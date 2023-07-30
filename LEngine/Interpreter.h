#pragma once

/* std */
#include <variant>
#include <optional>
#include <unordered_map>
#include <stack>

/* Project */
#include "GlobalState.h"
#include "MemoryManager.h"
#include "hashing.h"
#include "Parser.h"

/* Builtin types */
#include "Builtin.h"
#include "Number.h"
#include "String.h"
#include "Storage.h"
#include "Array.h"
#include "Function.h"
#include "Boolean.h"
#include "DllModule.h"

/* Lib */
#include "le_io.h"

namespace le
{
	/*
	* @deprecate
	* [DEPRECATED]
	* I will currently no longer be working on this interpreter in favour of the bytecode interpreter found in "VM.h".
	* As the project gains complexity and features i have found a simple tree interpreter like this lacking in speed and flexibility.
	* So i will be moving my efforts to the compiler and bytecode interpreter.
	* 
	*/
	class Interpreter
	{
	private:
		using BlockEscapeReason = BlockStatement::EscapeReason;
		struct BlockHandler
		{
			BlockEscapeReason escape_reason { BlockEscapeReason::None };
			/*
			* Holds a stack of priority keyword acceptable blocks like While/For/Function blocks.
			* That way it knows which keyword to call for whom.
			* // Stack = []
			* fn print_10x(arg):
			* * var x = 10 // Stack = [addressof(print_10x)]
			* * While x > 0:
			* * * print(arg) // Stack = [addressof(while x > 0), addressof(print_10x)]
			* * * x = x - 1
			* * end
			* // Stack = [addressof(print_10x)]
			* end
			* // Stack = []
			*/
			std::vector<BlockStatement*> _breakable_code_blocks{};

			auto can_a_block_handle_escape_reason() const -> bool
			{
				for (const auto block : _breakable_code_blocks)
					if (block->accepted_escape_reasons & escape_reason) 
						return true;
				return false;
			}
			auto can_current_block_handle_keyword() const -> bool { return _breakable_code_blocks.back()->accepted_escape_reasons & escape_reason; }
			auto empty() const -> bool { return _breakable_code_blocks.empty(); }
			auto open_block(BlockStatement* block) -> void { _breakable_code_blocks.push_back(block); }
			auto close_block() -> void { return _breakable_code_blocks.pop_back(); }
		} _block_handler{};
	protected:
		String _error_state{};
		MemoryManager _mem{};
		Storage _storage{};

		constexpr inline static auto max_loops = 1ull << 16;

		auto solve(const Token::Type op, LeObject right) -> LeObject
		{
			return right->apply_operation(op);
		}

		auto solve(LeObject& lval, LeObject& rval, const Token::Type op) -> LeObject
		{
			return lval->apply_operation(op, rval);
		}

		auto call_builtin_function(Function& func, std::vector<PExpression>& args) -> LeObject
		{
			/* hook up args */
			auto& block = *func.body;
			block.block_arg_names = &func.args;
			block.block_args = &args;

			return evaluate(func.body);
		}

		/* Evaluate and assign a list of arguments */
		auto init_var_list(std::vector<PExpression>& args, const std::vector<String>& arg_names) -> void
		{
			const auto expected_args = arg_names.size();
			const auto provided_args = args.size();
			if (expected_args != provided_args)
			{
				throw(ferr::make_exception(std::format("{} arguments provided to function, expected {} got {}"
					, expected_args > provided_args ? "Not enough" : "Too many"
					, expected_args, provided_args)));
			}

			auto arg_expr_iter = args.begin();
			for (auto& arg_name : arg_names)
			{
				_storage.declare(arg_name, evaluate(arg_expr_iter->get()));
				arg_expr_iter++;
			}
		}

		auto verify_block_keyword_context() const -> void
		{
			/* Check if escape reason is allowed in current context */
			if (not _block_handler.can_a_block_handle_escape_reason())
			{
				throw(ferr::make_exception(std::format("Keyword '{}' cannot be used in the current context", to_string(_block_handler.escape_reason))));
			}
		}

		auto evaluate(Statement* stmt) -> LeObject
		{
			auto val = LeObject{};

			if (not stmt) return val;

			switch (stmt->type)
			{
			case Statement::Type::BreakStatement:
				_block_handler.escape_reason = BlockEscapeReason::Break;
				verify_block_keyword_context(); break;
			case Statement::Type::ContinueStatement:
				_block_handler.escape_reason = BlockEscapeReason::Continue;
				verify_block_keyword_context(); break;
			case Statement::Type::ReturnExpression:
			{
				auto expr = static_cast<ReturnExpression*>(stmt);
				val = evaluate(expr->expr.get());
				_block_handler.escape_reason = BlockEscapeReason::Return;
				verify_block_keyword_context();
				break;
			}
			case Statement::Type::FunctionDeclarationExpression:
			{
				auto expr = static_cast<FunctionDeclaration*>(stmt);
				auto f = _mem.emplace<Function>();
				f->args.reserve(expr->args.size());
				for (const auto& arg : expr->args) f->args.emplace_back(arg);

				f->body = expr->body.get();
				if (not expr->name.empty())
				{ /* Declare as variable, in the future it is to be const */
					_storage.declare(expr->name, f);
					val = _storage.get(expr->name);
				}
				else
				{
					val = f;
				}
				break;
			}
			case Statement::Type::ImportStatement:
			{
				auto imp = static_cast<ImportStatement*>(stmt);
				if (imp->target.ends_with(".dll"))
				{
					auto dllmod = _mem.emplace<DllModule>(String(imp->target));
					dllmod->load(String(imp->target));
					imp->target.remove_suffix(4); /* len(".dll") == 4 */
					_storage.declare(imp->target, dllmod);
					val = _storage.get(imp->target);
				}
				else
				{
					throw(ferr::make_exception("Importing other Le script's is currently unsupported"));
				}

				break;
			}
			case Statement::Type::BlockStatement:
			{ /* In the future we could optimize by only opening scopes when something has been declared in said scope */
				_storage.open_scope();

				auto block = static_cast<BlockStatement*>(stmt);
				if (block->block_args and block->block_arg_names)
					/* Blocks can hold args to handle and simplify cases such as function calls or for loops to init their args */
				{
					init_var_list(*block->block_args, *block->block_arg_names);
				}

				_block_handler.open_block(block);

				while (true)
				{
					for (auto& e : block->body)
					{
						val = evaluate(e.get());

						if (_block_handler.escape_reason != BlockEscapeReason::None)
							break;
					}
					
					if (_block_handler.escape_reason & BlockEscapeReason::None) break;
					/* Check if the current block can handle the keyword */
					if (not _block_handler.can_current_block_handle_keyword()) break;
					/* Reset escape reason back to none */
					auto old_escape_reason = std::exchange(_block_handler.escape_reason, BlockEscapeReason::None);
					block->was_escaped_with = old_escape_reason;
					if (old_escape_reason & BlockEscapeReason::Break or old_escape_reason & BlockEscapeReason::Return) break;
					if (old_escape_reason & BlockEscapeReason::Continue) continue;
					if (old_escape_reason & BlockEscapeReason::Break) break;
				}

				_block_handler.close_block();

				_storage.close_scope();

				break;
			}
			case Statement::Type::CallExpression:
			{
				auto expr = static_cast<CallExpression*>(stmt);
				auto call_target = evaluate(expr->target.get());
				
				if (call_target->type == RuntimeValue::Type::Function)
				{
					auto func = static_cast<Function*>(call_target.get());
					val = call_builtin_function(*func, expr->args);
				}
				else
				{
					auto provided_args = expr->args.size();
					auto evaluated_args = std::vector<LeObject>();
					evaluated_args.reserve(provided_args);

					for (auto& arg : expr->args)
					{
						evaluated_args.emplace_back(evaluate(arg.get()));
					}
					auto span = std::span(evaluated_args);
					//call_target->call(span);
				}

				break;
			}
			case Statement::Type::BinaryExpression:
			{
				auto expr = static_cast<BinaryOperation*>(stmt);
				auto left = evaluate(expr->left.get());
				auto right = evaluate(expr->right.get());
				val = solve(left, right, expr->op.type); break;
			}
			case Statement::Type::StringLiteralExpression:
			{
				auto expr = static_cast<StringLiteral*>(stmt);
				val = _mem.emplace<StringValue>(expr->string); break;
			}
			case Statement::Type::WhileLoop:
			{
				auto expr = static_cast<WhileLoop*>(stmt);
				auto loop_count = 0ull;
				auto block = static_cast<BlockStatement*>(expr->body.get());
				while (loop_count < max_loops)
				{
					auto result = evaluate(expr->expr.get());
					if (not result->to_native_bool()) break;
					evaluate(expr->body.get());
					/* This could be done a bit neater */
					if (block->was_escaped_with & (BlockEscapeReason::Break | BlockEscapeReason::Return))
						break;
					loop_count++;
				}
				block->was_escaped_with = BlockEscapeReason::None;
				if (loop_count >= max_loops)
				{
					throw(ferr::make_exception("Loop exceeded max iterations"));
				}

				break;
			}
			case Statement::Type::AssignmentExpression:
			{
				auto expr = static_cast<AssignmentExpression*>(stmt);
				auto rhs = evaluate(expr->right.get());
				switch (expr->target->type)
				{
				case Statement::Type::AccessorExpression:
				{
					auto accessor = static_cast<AccessorExpression*>(expr->target.get());
					auto target = evaluate(accessor->target.get());
					val = target->access_assign(evaluate(accessor->query.get()), rhs);
					break;
				}
				case Statement::Type::IdentifierExpression:
					_storage.assign(static_cast<Identifier*>(expr->target.get())->name, rhs);
					val = _storage.get(static_cast<Identifier*>(expr->target.get())->name);
					break;
				default:
					throw(ferr::failed_assignment(String(to_string(expr->target->type)), to_string(rhs->type)));
				}
				break;
			}
			case Statement::Type::NumericLiteralExpression:
			{
				auto expr = static_cast<NumericLiteral*>(stmt);
				val = _mem.emplace<NumberValue>(expr->value); break;
			}
			case Statement::Type::AccessorExpression:
			{
				auto expr = static_cast<AccessorExpression*>(stmt);
				auto target = evaluate(expr->target.get());
				auto query = evaluate(expr->query.get());
				val = target->access(query); break;
			}
			case Statement::Type::UnaryOperation:
			{
				auto expr = static_cast<UnaryOperation*>(stmt);
				auto right = evaluate(expr->target.get());
				val = solve(expr->op.type, right); break;
			}
			case Statement::Type::ArrayExpression:
			{
				auto expr = static_cast<ArrayExpression*>(stmt);
				val = _mem.emplace<Array>();
				auto& arr = *static_cast<Array*>(val.get());
				arr.data.reserve(expr->container.size());
				for (auto& ex : expr->container)
				{
					arr.data.emplace_back(evaluate(ex.get()));
				} 
				break;
			}
			case Statement::Type::IdentifierExpression:
			{
				auto expr = static_cast<Identifier*>(stmt);
				val = _storage.get(expr->name); break;
			}
			case Statement::Type::VarAssignmentStatement:
			{
				auto assignment = static_cast<VarAssignment*>(stmt);
				if (_storage.has(assignment->target))
				{
					throw(ferr::make_exception(std::format("Variable {} has already been declared", assignment->target)));
				}
				auto value = evaluate(assignment->right.get());
				_storage.declare(assignment->target, value);
				val = _storage.get(assignment->target); break;
			}
			case Statement::Type::ElifStatement:
			case Statement::Type::IfStatement:
			{
				auto if_stmt = static_cast<IfStatement*>(stmt);
				auto test = evaluate(if_stmt->test.get());
				if (test->to_native_bool())
				{
					val = evaluate(if_stmt->consequent.get());
				}
				else
				{
					val = evaluate(if_stmt->alternative.get());
				}
				break;
			}
			default:
				throw(ferr::make_exception(std::format("Unknown statement: {}", to_string(stmt->type)))); break;
			}

			if (not val)
				val = _mem.emplace<NullValue>();

			return val;
		}

		auto init_global_lib_funcs() -> void
		{
			//_storage.assign("print", _mem.emplace<ImportedFunction>(&lib::io::print));
		}
	public:
		Interpreter() = default;

		auto run(AST& ast) -> std::variant<LeObject, String>
		{ /* We are hoping that the next time this runs all objects will have gone out of scope and cleared it for us */
			global::mem = &_mem;

			_error_state.clear();
			auto result = LeObject{};
			
			_storage.clear();
			_storage.open_scope();

			init_global_lib_funcs();

			try
			{
				for (auto& expr : ast.body)
				{
					result = evaluate(expr.get());
				}
			}
			catch (const Exception& exception)
			{
				_storage.clear();
				return String(exception.what());
			}

			_storage.close_scope();

			return result;
		}
	};
}

