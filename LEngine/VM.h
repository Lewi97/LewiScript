#pragma once

#include "GlobalState.h"
#include "ByteCode.h"
#include "MemoryManager.h"
#include "Number.h"
#include "Function.h"
#include "Array.h"

#include <variant>
#include <stack>

namespace le
{
	class VirtualMachine
	{
		struct VarStorage
		{
			VarStorage() : data(1ull, nullptr) {}
			std::vector<LeObject> data{};
			auto store(size_t index, LeObject object) -> void
			{
				if (index >= data.size())
					data.resize(data.capacity() * 2);
				data.at(index) = object;
			}
			auto load(size_t index) -> LeObject
			{
				if (index >= data.size()) throw(ferr::make_exception("Invalid load index"));
				return data.at(index);
			}
		};

	protected:
		using ProgramCounter = decltype(Code::code)::const_iterator;
		using Stack = std::stack<LeObject>;
		using FunctionArgs = std::vector<LeObject>;

		struct Scope
		{
			ProgramCounter end{};
			VarStorage variables{};
			Stack stack{};
		};
		
		std::vector<Scope> _scopes{};
		
		Code const* _current_code{ nullptr };
		ProgramCounter _pc{};
		/* Reusable vector for pushing function args */
		FunctionArgs _function_args{};
		LeObject _null_val{};

		/* @return returns previous scope */
		auto open_scope(ProgramCounter end) -> void
		{ /* Internally std::stack uses a deque which should not invalidate the reference */
			_scopes.push_back(Scope{ .end = end });
		}

		auto open_begin_scope(ProgramCounter end) -> void
		{
			_scopes.push_back(Scope{ .end = end });
		}

		auto close_scope() -> void
		{
			_scopes.pop_back();
		}

		auto get_global_string(size_t index) -> String
		{
			if (index > _current_code->global_strings.size())
				throw(ferr::make_exception("Tried accessing out of bounds string global"));
			return _current_code->global_strings.at(index);
		}

		auto storage() -> VarStorage& { return _scopes.back().variables; }
		auto scope() -> Scope& { return _scopes.back(); }
		auto stack() -> Stack& { return _scopes.back().stack; }

		auto load(u64 index) -> LeObject
		{
			/* Look through all of them */
			return storage().load(index);
		}

		auto pop() -> LeObject
		{
			auto old = stack().top(); stack().pop();
			return old;
		}

		auto push(LeObject object) -> void
		{
			stack().push(object);
		}
		
		auto pop(Scope& scope) -> LeObject
		{
			auto old = scope.stack.top(); scope.stack.pop();
			return old;
		}

		auto push(LeObject object, Scope& scope) -> void
		{
			scope.stack.push(object);
		}


		auto jump(i64 delta) -> void
		{ 
			_pc += delta;
		}

		auto iterate_pc() -> void { _pc++; }
		auto halt() -> void { _pc = scope().end; }

#define LE_NEXT_INSTRUCTION iterate_pc(); break
#define LE_JUMP(delta) jump(delta); break
		auto evaluate(const Instruction& instr) -> void
		{
			switch (instr.op)
			{
			case OpCode::Halt: halt(); break;
			case OpCode::ReturnExpr:
			{ /* By evaluating the expr, its result should be on top */
				halt(); break;
			}
			case OpCode::Return:
			{ /* Empty return, we clear the stack so we dont return anything */
				scope().stack = std::stack<LeObject>{};
				halt(); break;
			}
			case OpCode::Call:
			{
				auto args_count = _function_args.size();
				for (auto i{ 0 }; i < instr.operand.uinteger; i++)
					_function_args.push_back(pop());
				
				auto args = std::span(_function_args.begin() + args_count, _function_args.end());
				auto ret_val = pop()->call(args, *this);
				push(ret_val);
				_function_args.erase(_function_args.begin() + args_count, _function_args.end());

				LE_NEXT_INSTRUCTION;
			}
			case OpCode::PushFunction:
			{
				auto& function = _current_code->functions.at(instr.operand.uinteger);
				push(global::mem->emplace<CompiledFunction>(function));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::Jump:
			{
				LE_JUMP(instr.operand.integer);
			}
			case OpCode::JumpIfFalse:
			{
				if (not pop()->to_native_bool())
				{
					LE_JUMP(instr.operand.integer);
				}
				else
				{
					LE_NEXT_INSTRUCTION;
				}
			}
			case OpCode::JumpIfTrue:
			{
				if (pop()->to_native_bool())
				{
					LE_JUMP(instr.operand.integer);
				}
				else
				{
					LE_NEXT_INSTRUCTION;
				}
			}
			case OpCode::AccessAssign:
			{
				auto target = pop();
				auto query = pop();
				auto rhs = pop();
				target->access_assign(query, rhs);
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::Access:
			{
				auto target = pop();
				auto query = pop();
				push(target->access(query));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::MakeArray:
			{
				const auto array_size = instr.operand.uinteger;
				auto array = global::mem->emplace<Array>();
				array->data.resize(array_size, nullptr);
				/* TOS is last element, add in reverse */
				std::for_each(array->data.rbegin(), array->data.rend(),
					[this](LeObject& obj)
					{
						obj = pop();
					});
				push(array);
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::PushString:
			{
				push(global::mem->emplace<StringValue>(get_global_string(instr.operand.uinteger)));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::Load:
			{
				push(load(instr.operand.uinteger));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::Store:
			{
				storage().store(instr.operand.uinteger, pop());
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::PushReal:
			{
				push(global::mem->emplace<NumberValue>(instr.operand.real));
				LE_NEXT_INSTRUCTION;
			}
			/* Operators */
			/* Arithmetic */
			case OpCode::Add: case OpCode::Mul: 
			case OpCode::Div: case OpCode::Sub:
			/* Relational */
			case OpCode::GT: case OpCode::GET:
			case OpCode::EQ: case OpCode::NEQ:
			case OpCode::LT: case OpCode::LET:
			{ /* Lhs is second on stack so we have to call apply on second */
				auto rhs = pop();
				push(pop()->apply_operation(to_token_type(instr.op), rhs));
				LE_NEXT_INSTRUCTION;
			}
			default:
				throw(ferr::make_exception("Unexpected Opcode encountered"));
			}
		}
#undef LE_NEXT_INSTRUCTION
#undef LE_JUMP

	public:
		VirtualMachine()
		{
			_null_val = global::mem->emplace<NullValue>();
		}

		auto run(const Frame& frame, std::span<LeObject>& args) -> LeObject
		{
			auto old_pc = _pc;
			_pc = frame.code.cbegin();
			
			auto end = frame.code.cend();

			open_scope(end);
			for (auto argc{ 0ull }; auto& arg : args)
			{ 
				storage().store(argc++, arg);
			}

			while (_pc != end)
			{
				evaluate(*_pc);
			}

			auto return_val = _null_val;
			if (not stack().empty())
			{
				return_val = pop();
			}

			close_scope();

			_pc = old_pc;

			return return_val;
		}

		auto run(const Code& code) -> std::variant<LeObject, String>
		{
			_current_code = &code;
			_pc = _current_code->code.cbegin();

			try
			{
				auto end = _current_code->code.cend();
				open_begin_scope(end);
				while(_pc != end)
				{
					//std::cout << std::format("Current: {}\n",to_string(_pc->op));
					
					evaluate(*_pc);
				}

				if (stack().empty())
				{
					close_scope();
					return _null_val;
				}
				else
				{
					auto ret = pop();
					close_scope();
					return ret;
				}
			}
			catch (const std::exception& e)
			{
				return String(e.what());
			}
		}
	};
}

