#pragma once

#include <vector>
#include <charconv>
#include <format>
#include <bitset>

#include "format_errs.h"
#include "Lexer.h"
#include "Statements.h"
#include "Operators.h"

namespace le
{	
	struct AST
	{
		String error{};
		std::vector<PStatement> body{};
	};

	class Parser
	{
	protected:
		AST _ast{};
		Lexer* _lexer{ nullptr };
		String _error_state{};

		/* Expects AT LEAST ONE argument */
		auto parse_comma_list(Token::Type delim = Token::Type::Comma) -> std::vector<PExpression>
		{
			auto container = std::vector<PExpression>{};

			auto elem = parse_assignment_expr();
			container.emplace_back(std::move(elem));
			while (_lexer->current().type == delim)
			{
				_lexer->eat(); /* Skip comma */
				elem = parse_assignment_expr();
				container.emplace_back(std::move(elem));
			}

			return container;
		}

		auto parse_array() -> PExpression
		{
			auto expr = std::make_unique<ArrayExpression>();

			if (_lexer->current().type != Token::Type::CloseSquareBracket)
			{
				expr->container = parse_comma_list();
			}

			return expr;
		}

		/*
		* NEED TO TEST:
		* a.b[0].c // check correct parsing order
		* a.b.c.d() // validate chaining
		*/
		auto parse_member_expr() -> PExpression
		{
			auto target = parse_primary_expr();
			while (_lexer->current().type == Token::Type::Dot)
			{
				_lexer->advance(); /* Skip dot */
				expect(Token::Type::Identifier);
				target = make_member_expression(target, _lexer->eat() /* Skip identifier */);
			}

			return target;
		}

		auto parse_access_expr() -> PExpression
		{
			auto target = parse_member_expr();

			while (
				_lexer->current().type == Token::Type::OpenSquareBracket
				)
			{
				_lexer->advance(); /* Skip open square bracket */
				auto query = parse_assignment_expr();
				target = make_accessor_expression(target, query);
				if (_lexer->current().type == Token::Type::CloseSquareBracket)
				{
					_lexer->advance(); /* Skip close square bracket */
				}
				else
				{
					throw(ferr::unexpected_token(Token::Type::CloseSquareBracket, _lexer->current()));
				}

				return target;
			}

			return target;
		}

		auto parse_call_expr() -> PExpression
		{ /* '(list[2])(50,50)' can be a valid call expr, hence why our target is a primary expr */
			auto target = parse_access_expr();

			/* While loop because we want chaining aka 'get_func("add")(3, 4)' */
			while (_lexer->current().type == Token::Type::OpenParen)
			{
				auto call_expr = std::make_unique<CallExpression>();
				call_expr->target = std::move(target);
				/* Make sure it isnt a function with no args */
				if (/* Skip open paren */_lexer->advance().type != Token::Type::CloseParen)
				{
					call_expr->args = parse_comma_list();
				}
				
				target = std::move(call_expr);
				_lexer->advance(); /* Skip close paren */
			}

			return target;
		}

		auto parse_multiplicative_expr() -> PExpression
		{
			auto left = parse_call_expr();

			while (
				left and
				is_operator(_lexer->current()) and
				precedence(_lexer->current().type) == precedences::multiplicative
				)
			{
				auto op = _lexer->eat();
				auto right = parse_access_expr();
				left = make_binary_operation(left, right, op);
			}

			return left;
		}

		auto parse_additive_expr() -> PExpression
		{
			auto left = parse_multiplicative_expr();

			while (
				left and
				is_operator(_lexer->current()) and
				precedence(_lexer->current().type) == precedences::additive
				)
			{
				auto op = _lexer->eat();
				auto right = parse_multiplicative_expr();
				left = make_binary_operation(left, right, op);
			}

			return left;
		}

		auto parse_relational_expr() -> PExpression
		{
			auto left = parse_additive_expr();

			while (
				left and
				is_operator(_lexer->current()) and
				precedence(_lexer->current().type) == precedences::relational
				)
			{
				auto op = _lexer->eat();
				auto right = parse_relational_expr();
				left = make_binary_operation(left, right, op);
			}

			return left;
		}

		auto parse_equality_expr() -> PExpression
		{
			auto left = parse_relational_expr();

			while (
				left and
				is_operator(_lexer->current()) and
				precedence(_lexer->current().type) == precedences::equality
				)
			{
				auto op = _lexer->eat();
				auto right = parse_relational_expr();
				left = make_binary_operation(left, right, op);
			}

			return left;
		}

		auto parse_assignment_expr() -> PExpression
		{
			auto left = parse_equality_expr();

			while (
				left and
				is_operator(_lexer->current()) and
				precedence(_lexer->current().type) == precedences::assignment
				)
			{
				/* Skip assig operator for now its always '=' but in the future it could be '+=' '-=' etc... */
				_lexer->advance(); 
				auto right = parse_assignment_expr();
				left = make_assignment_expression(left, right);
			}

			return left;
		}
		
		/* Checks if current token == type*/
		auto expect(Token::Type type) -> void
		{
			if (type == _lexer->current().type) return;
			throw(ferr::unexpected_token(type, _lexer->current()));
		}

		/* Checks if expression is of type type*/
		auto expect(Statement::Type type, PExpression& got) -> void
		{
			if (type == got->type) return;
			throw(ferr::unexpected_expression(String(to_string(type)), String(to_string(got->type))));
		}

		auto parse_function_args() -> std::vector<Symbol>
		{
			auto args = std::vector<Symbol>{};
			expect(Token::Type::OpenParen);
			_lexer->advance(); /* Skip open paren */
			/* We are here: 'arg1, arg2)' */
			if (_lexer->current().type == Token::Type::Identifier)
			{
				args.push_back(_lexer->eat().raw); /* Eat and continue */
				while (_lexer->current().type == Token::Type::Comma)
				{
					_lexer->advance(); /* Skip comma */
					expect(Token::Type::Identifier);
					args.push_back(_lexer->eat().raw); /* Eat and continue */
				}
			}
			expect(Token::Type::CloseParen);
			_lexer->advance(); /* Skip close paren */
			return args;
		}

		/* Basic block statement parse that uses keyword end and skips it */
		auto parse_block_statement() -> std::unique_ptr<BlockStatement>
		{
			auto block = parse_block_statement(Token::Type::KeywordEnd);
			expect(Token::Type::KeywordEnd); _lexer->advance(); /* Skip keyword end */
			return block;
		}

		/* Not skipping end delim requires type(delims) == Token::Type */
		auto parse_block_statement(auto... delims) -> std::unique_ptr<BlockStatement>
		{
			static_assert(sizeof...(delims) > 0);

			auto block = std::make_unique<BlockStatement>();
			
			while ((... and (_lexer->current().type != delims)))
			{
				auto expr = parse_statement();
				block->body.push_back(std::move(expr));
				
				if (_lexer->current().type == Token::Type::EoF) break;
			}
			
			return block;
		}

		/* fn name(arg1, arg2) || fn(arg1, arg2) */
		auto parse_function_declaration() -> PExpression
		{
			auto fn = std::make_unique<FunctionDeclaration>();
			/* Case: fn name(arg1, arg2) */
			if (_lexer->current().type == Token::Type::Identifier)
			{
				fn->name = _lexer->eat().raw; /* Eat identifier and returns its raw value, no need to parse this */
			}
			
			fn->args = parse_function_args();
			expect(Token::Type::Colon);
			_lexer->advance(); /* Skip colon */
			
			auto block = parse_block_statement();
			block->accepted_escape_reasons = BlockStatement::EscapeReason::Return;
			fn->body = std::move(block);

			return fn;
		}

		auto parse_primary_expr() -> PExpression
		{
			switch (_lexer->current().type)
			{
			case Token::Type::Identifier:
				return make_identifier(_lexer->eat());
			case Token::Type::StringLiteral:
				return make_string_literal(_lexer->eat());
			case Token::Type::NumericLiteral:
				return make_numeric_literal(_lexer->eat(), _error_state);
			case Token::Type::KeywordFn:
			{
				_lexer->eat(); /* Skip keyword */
				auto expr = parse_function_declaration();
				return expr;
			}
			case Token::Type::OpenSquareBracket:
			{
				_lexer->advance(); /* Skip OpenSquareBracket */
				auto expr = parse_array();
				if (auto old = _lexer->eat()/* Get and skip OpenSquareBracket */; old.type != Token::Type::CloseSquareBracket)
				{
					throw(ferr::unexpected_token(Token::Type::CloseSquareBracket, old));
				}
				return expr;
			}
			case Token::Type::OpenParen:
			{
				_lexer->advance(); /* Skip openparen */
				auto expr = parse_assignment_expr();
				if (auto old = _lexer->eat()/* Get and skip closeparen */; old.type != Token::Type::CloseParen)
				{
					throw(ferr::unexpected_token(Token::Type::CloseParen, old));
				}
				return expr;
			}
			//case Token::Type::NewLine: /* FIX THIS SHIT */
			/*	return nullptr; */ /* This shouldnt be expected till at the end of the expr */
			default:
				/* Unary operators */
				if (is_operator(_lexer->current()))
				{
					auto op = _lexer->eat();
					auto target = parse_primary_expr();
					return make_unary_operation(target, op);
				}

				throw(ferr::unexpected_token(_lexer->current()));
				return nullptr;
			}
		}

		auto parse_assignment_statement() -> PStatement
		{
			auto assignment = std::make_unique<VarAssignment>();
			auto target = _lexer->eat();
			
			if (target.type != Token::Type::Identifier)
			{
				throw(ferr::unexpected_token(Token::Type::Identifier, target));
				return nullptr;
			}

			if (_lexer->eat().type != Token::Type::OperatorEquals)
			{
				throw(ferr::unexpected_token(Token::Type::OperatorEquals, target));
				return nullptr;
			}

			assignment->target = target.raw;
			assignment->right = parse_assignment_expr();
			
			return assignment;
		}

		/*
		* if test: ...
		* elif test: ...
		* else: ...
		* end
		*/
		auto parse_if_statement() -> PStatement
		{
			auto if_stmt = std::make_unique<IfStatement>();
			if_stmt->test = parse_assignment_expr();
			expect(Token::Type::Colon); _lexer->advance();
			if_stmt->consequent = parse_block_statement(Token::Type::KeywordEnd, Token::Type::KeywordElif, Token::Type::KeywordElse);
			
			switch (_lexer->current().type)
			{
			case Token::Type::KeywordElif:
				_lexer->advance(); /* Skip elif keyword */
				if_stmt->alternative = parse_if_statement();
				break;
			case Token::Type::KeywordElse:
				_lexer->advance(); /* Skip keyword */
				expect(Token::Type::Colon); _lexer->advance();
				if_stmt->alternative = parse_block_statement();
				break;
			case Token::Type::KeywordEnd: 
				_lexer->advance(); /* Skip end */
				break;
			default:
				throw (ferr::make_exception(std::format("If statement ended on unexpected type: {}", to_string(_lexer->current().type))));
			}

			return if_stmt;
		}

		/* 
		* while [expr]: 
		*	[body]
		*	end
		*/
		auto parse_while_loop() -> PStatement
		{
			auto while_loop = std::make_unique<WhileLoop>();
			while_loop->expr = parse_assignment_expr();
			expect(Token::Type::Colon); _lexer->advance();

			auto block = parse_block_statement();
			block->accepted_escape_reasons = BlockStatement::EscapeReason::Break | BlockStatement::EscapeReason::Continue;
			while_loop->body = std::move(block);

			return while_loop;
		}

		auto parse_statement() -> PStatement
		{
			switch (_lexer->current().type)
			{
			case Token::Type::KeywordWhile:
				_lexer->advance(); /* Skip while keyword */
				return parse_while_loop();
			case Token::Type::KeywordIf:
				_lexer->advance(); /* Skip keyword */
				return parse_if_statement();
			case Token::Type::KeywordVar: 
				_lexer->advance(); /* Skip keyword */
				return parse_assignment_statement();
			case Token::Type::KeywordContinue:
				_lexer->advance(); /* Skip keyword */
				return std::make_unique<ContinueStatement>();
			case Token::Type::KeywordBreak:
				_lexer->advance(); /* Skip keyword */
				return std::make_unique<BreakStatement>();
			case Token::Type::KeywordReturn:
				_lexer->advance(); /* Skip keyword */
				if (_lexer->current().type == Token::Type::KeywordEnd) /* No expr */
					return std::make_unique<ReturnExpression>();
				return std::make_unique<ReturnExpression>(parse_assignment_expr());
			case Token::Type::KeywordImport: 
				_lexer->advance(); /* Skip keyword */
				expect(Token::Type::Identifier);
				return std::make_unique<ImportStatement>(_lexer->eat().raw);
			default: /* No statement, try expression */
				return parse_assignment_expr();
			}
		}

	public:
		auto parse(Lexer& lexer) -> AST&&
		{
			_error_state.clear();
			_lexer = &lexer;
			_lexer->advance();

			try
			{
				while (not _lexer->empty())
				{
					//if (_lexer->current().type == Token::Type::NewLine) 
					//{
					//	_lexer->advance(); /* Skip newline and start parsing new expr */
					//	continue;
					//}

					_ast.body.push_back(parse_statement());
					
					//if (not _lexer->empty() /*and _lexer->current().type != Token::Type::NewLine*/)
					//{ /* if the parser was forced to stop at some point there is an uncaught syntax error somewhere */
					//	throw(ferr::unexpected_token(_lexer->current()));
					//}
					//else
					//{
					//	_lexer->advance(); /* Skip newline and start parsing new expr */
					//}
				}
			}
			catch (const Exception& exception)
			{
				_ast.error = exception.what();
			}

			return std::move(_ast);
		}
	};

	inline auto to_string(const le::Statement* expr) -> String
	{
		if (not expr) return "Empty";

		auto string = String{};

		switch (expr->type)
		{
		case Statement::Type::NumericLiteralExpression:
		{
			const auto num = static_cast<const NumericLiteral*>(expr);
			string = std::format("( Type: {} Value: {} )", to_string(expr->type), num->value); break;
		}
		case Statement::Type::BinaryExpression:
		{
			const auto binop = static_cast<const BinaryOperation*>(expr);
			string = std::format("( Type: {} Left: {} Operator: {} Right: {} )"
				, to_string(expr->type)
				, to_string(binop->left.get())
				, to_string(binop->op.type)
				, to_string(binop->right.get())); break;
		}
		case Statement::Type::IdentifierExpression:
		{
			const auto id = static_cast<const Identifier*>(expr);
			string = std::format("( Type: {} Name: {} )", to_string(id->type), id->name); break;
		}
		case Statement::Type::VarAssignmentStatement:
		{
			const auto assignment = static_cast<const VarAssignment*>(expr);
			string = std::format("( Type: {} Target: {} Right: {} )"
				, to_string(expr->type)
				, assignment->target
				, to_string(assignment->right.get())); break;
		}
		case Statement::Type::UnaryOperation:
		{
			const auto operation = static_cast<const UnaryOperation*>(expr);
			string = std::format("( Type: {} Operator: {} Right: {} )"
				, to_string(expr->type)
				, to_string(operation->op.type)
				, to_string(operation->target.get())); break;
		}
		case Statement::Type::CallExpression:
		{
			const auto call = static_cast<const CallExpression*>(expr);
			auto args = String{};
			for (const auto& e : call->args)
				args += std::format("{} ", to_string(e->type));
			string = std::format("( Type: {} Target: {} Args: {} )", to_string(call->type), to_string(call->target.get()), args); break;
		}
		case Statement::Type::FunctionDeclarationExpression:
		{
			const auto func = static_cast<const FunctionDeclaration*>(expr);
			auto args = String{};
			for (const auto& a : func->args)
				args += std::format("{} ", a);
			auto block = String{};
			for (const auto& e : func->body->body)
				block += std::format("{} ", to_string(e->type));
			string = std::format("( Type: {} Name: {} Args: {} Block: {} )", to_string(func->type), func->name, args, block); break;
		}
		case Statement::Type::AssignmentExpression:
		{
			const auto assig = static_cast<const AssignmentExpression*>(expr);
			string = std::format("( Type: {} Target: {} Right: {} )"
				, to_string(assig->type)
				, to_string(assig->target.get())
				, to_string(assig->right.get())
			);
			break;
		}
		case Statement::Type::AccessorExpression:
		{
			const auto accessor = static_cast<const AccessorExpression*>(expr);
			string = std::format("( Type: {} Target: {} Query: {} )", to_string(expr->type), to_string(accessor->target.get()), to_string(accessor->query.get())); break;
		}
		case Statement::Type::ArrayExpression:
		{
			const auto arr = static_cast<const ArrayExpression*>(expr);
			string = "( [";
			for (const auto& e : arr->container)
			{
				string += to_string(e.get());
				string += ", ";
			}
			string += "] )";
			break;
		}
		case Statement::Type::ElifStatement: case Statement::Type::IfStatement:
		{
			const auto statement = static_cast<const IfStatement*>(expr);
			string = std::format("( Type: {} Test: {} Consequent: {} Alternative: {} )"
				, to_string(statement->type)
				, to_string(statement->test.get())
				, to_string(statement->consequent.get())
				, statement->alternative ? to_string(statement->alternative.get()) : String("Null")
			); break;
		}
		case Statement::Type::WhileLoop:
		{
			const auto loop = static_cast<const WhileLoop*>(expr);
			string = std::format("( Type: {} Expr: {} Body: {} )", to_string(loop->type), to_string(loop->expr.get()), to_string(loop->body.get()));
			break;
		}
		case Statement::Type::StringLiteralExpression:
		{
			const auto literal = static_cast<const StringLiteral*>(expr);
			string = std::format("( Type: {} String: {} )", to_string(literal->type), literal->string); break;
		}
		case Statement::Type::ImportStatement:
		{
			const auto literal = static_cast<const ImportStatement*>(expr);
			string = std::format("( Type: {} Target: {} )", to_string(literal->type), literal->target); break;
		}
		default:
			string = std::format("Type: {} does not have a string representation yet.", to_string(expr->type));
		}

		return string;
	}
}

inline auto operator<<(std::ostream& out, const le::Statement* expr) -> std::ostream&
{
	using namespace le;

	return out << to_string(expr);
}