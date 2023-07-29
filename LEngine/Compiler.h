#pragma once

#include "Statements.h"
#include "ByteCode.h"
#include "generics.h"
#include "VarMap.h"
#include "GlobalState.h"

#include <unordered_map>

namespace le
{
	/*
	* The implementation of the compiler.
	* Use this to compile pieces of code using another function that controls the Code object.
	* The seperation between compiler and code object is done intentionally for flexibility reasons.
	Eg. Compiling a specific piece of code would otherwise require the compiler to generate a new global strings map while that might not be the intention.
	*/
	class ImplCompiler
	{
	protected:
		Code* _code_obj{};
		ByteCode _code{};
		VarMap _vars{};
		size_t _depth{}; /* Scope depth */

		auto in_global_namespace() const -> bool { return _depth == 0; }
		auto register_global(Symbol name) -> size_t
		{
			return _code_obj->global_names.store(name);
		}
		auto is_global(const Symbol& name) const -> bool
		{
			return _code_obj->global_names.has(name);
		}
		auto get_global(const Symbol& name) const -> size_t
		{
			return _code_obj->global_names.get(name);
		}

		auto instruction_count() const -> i64 { return _code.size(); }
		auto emit(Instruction instruction) -> void
		{
			_code.push_back(instruction);
		}

		auto emit_and_get_index(Instruction instruction) -> size_t
		{
			_code.push_back(instruction);
			return _code.size() - 1;
		}

		auto instruction_at(size_t index) -> Instruction& { return _code.at(index); }

		auto last_instruction() -> Instruction&
		{
			return _code.back();
		}

		template<
			std::derived_from<RuntimeValue> _Val,
			typename... _Args>
		auto store_global(_Args&&... args) -> size_t
			requires std::constructible_from<_Val, _Args...>
		{
			auto old_size = _code_obj->globals.size();
			_code_obj->globals.push_back(global::mem->emplace<_Val>(std::forward<_Args>(args)...));
			return old_size;
		}

		auto store_function(ByteCode code, int argc, String name) -> size_t
		{
			auto frame = Frame{};

			if (name.empty())
				frame = Frame(std::move(code), String("Lambda"), argc);
			else
				frame = Frame(std::move(code), std::move(name), argc);

			return store_global<CompiledFunction>(frame);
		}

		template<typename _Type>
		auto as(auto ptr) -> _Type&
		{
			return *static_cast<_Type*>(ptr);
		}

		auto store(Symbol name) -> size_t
		{
			return _vars.store(name);
		}

		auto get(Symbol name) -> size_t
		{
			return _vars.get(name);
		}

		auto generate(Statement* statement) -> void
		{
			using SType = Statement::Type;

			if (not statement) return;

			/* Helpers, handy for instructions that rely on stack order */

			/* 'expr[expr]' */
			auto access_expr_order_helper =
				[this](PExpression& target, PExpression& query)
			{
				generate(query.get()); /* Set query at second to TOS */
				generate(target.get()); /* Set target TOS */
			};

			/* 'expr[expr] = expr' */
			auto access_assign_expr_order_helper =
				[this, access_expr_order_helper](PExpression& target, PExpression& query, PExpression& rhs)
			{
				generate(rhs.get()); /* Set rhs at third to TOS */
				access_expr_order_helper(target, query);
			};

			/* Generator */
			switch (statement->type)
			{
			case SType::ReturnExpression:
			{
				auto& return_expr = as<ReturnExpression>(statement);
				if (return_expr.expr)
				{
					generate(return_expr.expr.get());
					emit(Instruction(OpCode::ReturnExpr));
				}
				else
				{
					emit(Instruction(OpCode::Return));
				}
				break;
			}
			case SType::BlockStatement:
			{
				auto& block = as<BlockStatement>(statement);
				for (auto& expr : block.body)
					generate(expr.get());
				break;
			}
			case SType::CallExpression:
			{
				auto& call_expr = as<CallExpression>(statement);
				generate(call_expr.target.get());
				for (auto& expr : call_expr.args)
					generate(expr.get());
				emit(Instruction(OpCode::Call, call_expr.args.size()));
				break;
			}
			case SType::FunctionDeclarationExpression:
			{
				auto& function_decl = as<FunctionDeclaration>(statement);
				auto compiler = ImplCompiler(_depth + 1ull);
				
				for (auto& arg : function_decl.args)
					compiler.add_local(arg);

				const auto is_lambda = function_decl.name.empty();
				const auto push_global_index = emit_and_get_index(Instruction(OpCode::PushGlobal));

				if (not is_lambda)
				{
					if (in_global_namespace())
					{
						emit(Instruction(OpCode::StoreGlobal, register_global(function_decl.name)));
					}
					else
					{
						emit(Instruction(OpCode::Store, store(function_decl.name)));
					}
				}
				/* Have to first declare globals incase the function body refers to itself (maybe we should compile functions last?) */
				auto res = compiler.compile(function_decl.body->body, *_code_obj);
				instruction_at(push_global_index).operand.uinteger = store_function(res.first, function_decl.args.size(), String(function_decl.name));

				break;
			}
			case SType::IfStatement:
			{
				auto& if_statement = as<IfStatement>(statement);

				/* Generate the test */
				auto instr_count_pre_test = instruction_count();
				generate(if_statement.test.get());
				auto jump_to_next_set = emit_and_get_index(Instruction(OpCode::JumpIfFalse));
				generate(if_statement.consequent.get());

				/* no alternative */
				if (not if_statement.alternative)
				{
					instruction_at(jump_to_next_set).operand.integer = instruction_count() - jump_to_next_set;
				}
				/* elif or else case */
				else if (if_statement.alternative->type == SType::IfStatement)
				{
					auto jump_to_end = emit_and_get_index(Instruction(OpCode::Jump));
					auto instr_count = instruction_count();
					instruction_at(jump_to_next_set).operand.integer = instr_count - jump_to_next_set;
					generate(if_statement.alternative.get());
					instruction_at(jump_to_end).operand.integer = instruction_count() - instr_count + 1 /* account for jump instr */;
				}
				else if (if_statement.alternative->type == SType::BlockStatement)
				{
					auto jump_to_end = emit_and_get_index(Instruction(OpCode::Jump));
					auto instr_count = instruction_count();
					generate(if_statement.alternative.get());
					instruction_at(jump_to_next_set).operand.integer = instruction_count() - jump_to_next_set;
					instruction_at(jump_to_end).operand.integer = instruction_count() - instr_count + 1 /* account for jump instr */;
				}

				break;
			}
			case SType::WhileLoop:
			{
				auto& loop_statement = as<WhileLoop>(statement);
				auto pre_condition_instr_count = instruction_count();
				generate(loop_statement.expr.get()); /* expr first */
				auto post_condition_instr_count = instruction_count();
				auto pre_condition_jump_index = emit_and_get_index(Instruction(OpCode::JumpIfFalse));
				generate(loop_statement.body.get());
				auto post_block_instruction_count = instruction_count();
				instruction_at(pre_condition_jump_index).operand.integer = post_block_instruction_count - post_condition_instr_count + 1 /* Jump past the last jump instruction */;
				emit(Instruction(OpCode::Jump, pre_condition_instr_count - post_block_instruction_count));
				break;
			}
			case SType::AssignmentExpression:
			{
				auto& assignment_expr = as<AssignmentExpression>(statement);
				auto& target = assignment_expr.target;
				auto& rhs = assignment_expr.right;

				if (target->type == SType::AccessorExpression)
				{
					auto& access_expr = as<AccessorExpression>(target.get());
					access_assign_expr_order_helper(access_expr.target, access_expr.query, rhs);
					emit(Instruction(OpCode::AccessAssign));
				}
				/* Normal assignment */
				else if (target->type == SType::IdentifierExpression)
				{
					auto& identifier = as<Identifier>(target.get());
					auto var_index = _vars.get(identifier.name);
					generate(rhs.get());
					emit(Instruction(OpCode::Store, var_index));
				}
				else
				{
					throw(ferr::failed_assignment(String(to_string(target->type)), String(to_string(rhs->type))));
				}

				break;
			}
			case SType::AccessorExpression:
			{
				auto& access_expr = as<AccessorExpression>(statement);
				access_expr_order_helper(access_expr.target, access_expr.query);
				emit(Instruction(OpCode::Access));
				break;
			}
			case SType::ArrayExpression:
			{
				auto& array_expr = as<ArrayExpression>(statement);
				for (auto& expr : array_expr.container)
				{
					generate(expr.get());
				}
				emit(Instruction(OpCode::MakeArray, array_expr.container.size()));
				break;
			}
			case SType::VarAssignmentStatement:
			{
				auto& assignment = as<VarAssignment>(statement);
				auto index = store(assignment.target);
				generate(assignment.right.get());
				emit(Instruction(OpCode::Store, index));
				break;
			}
			case SType::IdentifierExpression:
			{
				auto& identifier = as<Identifier>(statement);
				if (is_global(identifier.name))
				{
					emit(Instruction(OpCode::LoadGlobal, get_global(identifier.name)));
				}
				else
				{
					emit(Instruction(OpCode::Load, _vars.get(identifier.name)));
				}

				break;
			}
			case SType::NumericLiteralExpression:
			{
				auto inst = Instruction{ OpCode::PushReal };
				inst.operand.real = as<NumericLiteral>(statement).value;
				emit(inst);
				break;
			}
			case SType::StringLiteralExpression:
			{
				auto& string = as<StringLiteral>(statement);
				auto string_global_index = store_global<StringValue>(String(string.string));
				emit(Instruction(OpCode::PushGlobal, string_global_index));
				break;
			}
			case SType::BinaryExpression:
			{
				auto& binop = as<BinaryOperation>(statement);
				generate(binop.left.get());
				generate(binop.right.get());
				switch (binop.op.type)
				{
					/* Arithmetic */
				case Token::Type::OperatorPlus: emit(Instruction{ OpCode::Add }); break;
				case Token::Type::OperatorMinus: emit(Instruction{ OpCode::Sub }); break;
				case Token::Type::OperatorMultiply: emit(Instruction{ OpCode::Mul }); break;
				case Token::Type::OperatorDivide: emit(Instruction{ OpCode::Div }); break;
					/* Relational */
				case Token::Type::OperatorEq: emit(Instruction{ OpCode::EQ }); break;
				case Token::Type::OperatorNEq: emit(Instruction{ OpCode::NEQ }); break;
				case Token::Type::OperatorLT: emit(Instruction{ OpCode::LT }); break;
				case Token::Type::OperatorLET: emit(Instruction{ OpCode::LET }); break;
				case Token::Type::OperatorGET: emit(Instruction{ OpCode::GET }); break;
				case Token::Type::OperatorGT: emit(Instruction{ OpCode::GT }); break;
				default:
					throw(ferr::unexpected_token(binop.op));
				}
				break;
			}
			default:
				throw(ferr::make_exception(std::format("Statement '{}' is not supported yet by the compiler", to_string(statement->type))));
			}
		}
	public:
		explicit ImplCompiler(size_t depth = 0ull)
			: _depth(depth)
		{}

		using Result = std::pair<ByteCode, VarMap>;

		auto add_local(Symbol name) -> void
		{
			store(name);
		}

		auto compile(AST& ast, Code& code_object)
			-> Result
		{
			return compile(ast.body, code_object);
		}

		auto compile(std::vector<PStatement>& ast, Code& code_object) 
			-> Result
		{
			_code_obj = &code_object;

			for (auto& statement : ast)
			{
				generate(statement.get());
			}
			emit(Instruction(OpCode::Halt));
			
			return std::make_pair(std::move(_code), std::move(_vars));
		}
	};

	constexpr auto impl__size = sizeof(ImplCompiler);

	class Compiler
	{
	public:
		Compiler() = default;

		auto emit_bytecode(AST& ast) -> std::variant<Code, String>
		try
		{
			Code code{};
			auto compiler = ImplCompiler(0ull);
			auto result = compiler.compile(ast, code);
			code.code = result.first;
			return code;
		}
		catch (const std::exception& e)
		{
			return String(e.what());
		}
	};
}

