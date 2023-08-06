#pragma once

#include "GlobalState.h"
#include "ByteCode.h"
#include "MemoryManager.h"
#include "Number.h"
#include "Function.h"
#include "Array.h"
#include "DllModule.h"
#include "Class.h"

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
		using Stack = std::vector<LeObject>;
		using FunctionArgs = std::vector<LeObject>;

		struct Scope
		{
			ProgramCounter end{};
			VarStorage variables{};
			Stack stack{};
		};
		
		std::vector<Scope> _scopes{};
		
		Code* _current_code{ nullptr };
		ProgramCounter _pc{};
		/* Reusable vector for pushing function args */
		FunctionArgs _function_args{};
		LeObject _null_val{};
		VarStorage _global_storage{};

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

		auto get_global(size_t index) -> LeObject
		{
			if (index > _current_code->globals.size())
				throw(ferr::make_exception("Tried accessing out of bounds global"));
			return _current_code->globals.at(index);
		}

		auto storage() -> VarStorage& { return _scopes.back().variables; }
		auto global_storage() -> VarStorage& { return _global_storage; }
		auto scope() -> Scope& { return _scopes.back(); }
		auto stack() -> Stack& { return _scopes.back().stack; }

		auto load(u64 index) -> LeObject
		{
			return storage().load(index);
		}

		auto load_global(u64 index) -> LeObject
		{
			return global_storage().load(index);
		}

		auto pop() -> LeObject
		{
			auto old = stack().back(); stack().pop_back();
			return old;
		}

		auto tos() -> LeObject&
		{
			return stack().back();
		}

		auto pop_no_ret() -> void
		{
			stack().pop_back();
		}
		
		auto push(LeObject object) -> void
		{
			stack().push_back(object);
		}
		
		/* 
		* Will reverse pop into target container 
		* This can be done neater
		*/
		auto reverse_pop_into_end_of(std::vector<LeObject>& target, u64 n) -> void
		{
			auto& s = stack();
			for (auto i{ s.end() - n }; i != s.end(); i++)
				target.push_back(*i);

			for (auto i{ 0 }; i < n; i++)
				pop_no_ret();
		}

		auto pop(Scope& scope) -> LeObject
		{
			auto old = scope.stack.back(); scope.stack.pop_back();
			return old;
		}

		auto push(LeObject object, Scope& scope) -> void
		{
			scope.stack.push_back(object);
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
			case OpCode::Noop: LE_NEXT_INSTRUCTION;
			case OpCode::ImportDll:
			{
				auto dll_name = pop()->make_string();
				push(global::mem->emplace<DllModule>(StringView(dll_name)));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::ReturnExpr:
			{ /* By evaluating the expr, its result should be on top */
				halt(); break;
			}
			case OpCode::Return:
			{ /* Empty return, we clear the stack so we dont return anything */
				scope().stack.clear();
				halt(); break;
			}
			case OpCode::UnaryOp:
			{
				push(pop()->apply_operation(static_cast<Token::Type>(instr.operand.integer)));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::Call:
			{
				auto args_count = _function_args.size();
			
				reverse_pop_into_end_of(_function_args, instr.operand.uinteger);
				auto args = std::span(_function_args.begin() + args_count, _function_args.end());
				auto ret_val = pop()->call(args, *this);
				push(ret_val);
				_function_args.erase(_function_args.begin() + args_count, _function_args.end());

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
			case OpCode::GetIter:
			{
				auto target = pop();
				push(target->iterator(target));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::ForLoop:
			{
				auto empty_span = std::span<LeObject>{};

				/* Look for the iterator, this is a somewhat bad and naive solution but will work for now */
				while (tos()->type != RuntimeValue::Type::Iterator)
					pop();

				/* Iterators call next on their call operator */
				auto iter_res = tos()->call(empty_span, *this);
				if (iter_res->type != RuntimeValue::Type::Null)
				{
					push(iter_res);
					LE_NEXT_INSTRUCTION;
				}
				else
				{
					pop(); /* Remove iterator from stack */
					LE_JUMP(instr.operand.integer);
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
			case OpCode::AccessMember:
			{
				auto target = pop();
				auto query = pop();
				push(target->member_access(target, static_cast<StringValue*>(query.get())->string));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::PushEmptyClass:
			{
				push(global::mem->emplace<Class>());
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::MakeMember:
			{
				auto target = pop();
				auto member = pop(); /* Could maybe make the operand of the opcode hold index to member in global string array */
				auto value = pop();

				if (target->type != RuntimeValue::Type::Class)
					throw(ferr::make_exception("Cannot assign a member to a non class type"));

				static_cast<Class*>(target.get())->make_member(target, getters::get_string_ref(member, "Assigning member to class"), value);

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
/* Pushes a global from the Code's global storage aka a string literal or function bytecode */
			case OpCode::PushGlobal:
			{
				push(get_global(instr.operand.uinteger));
				LE_NEXT_INSTRUCTION;
			}
/* Loads a global variable from the virtual machines global memory, aka a global variable */
			case OpCode::LoadGlobal:
			{
				push(load_global(instr.operand.uinteger));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::Load:
			{
				push(load(instr.operand.uinteger));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::StoreGlobal:
			{
				global_storage().store(instr.operand.uinteger, pop());
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::Store:
			{
				storage().store(instr.operand.uinteger, pop());
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::DupTos:
			{
				push(tos());
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::PushReal:
			{
				push(global::mem->emplace<NumberValue>(instr.operand.real));
				LE_NEXT_INSTRUCTION;
			}
			case OpCode::PushNull:
			{
				push(_null_val);
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
			_null_val = global::null;
		}

		auto run(const Frame& frame, std::span<LeObject>& args, LeObject this_ptr = nullptr) -> LeObject
		{
			auto old_pc = _pc;
			_pc = frame.code.cbegin();

			auto end = frame.code.cend();

			open_scope(end);

			auto argc{ 0ull };
			if (this_ptr)
				storage().store(argc++, this_ptr);
			for (auto & arg : args)
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

		auto run(Code& code) -> std::variant<LeObject, String>
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

#if(LE_TURN_ON_DEBUG_PRINTS)
				auto& stack_ = stack();
				auto count = 0ull;
				while(not stack_.empty())
				{
					auto obj = pop();
					LE_DEBUG_PRINT("{}: {}\n", count++, obj->make_string());
				}
#endif

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

