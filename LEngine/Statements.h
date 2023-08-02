#pragma once

#include "common.h"
#include "generics.h"

#include <vector>
#include <charconv>
#include <format>

namespace le
{
	using PStatement = std::unique_ptr<struct Statement>;

	struct Statement
	{
		enum class Type
		{
			/* Statements */
			IfStatement,
			ElifStatement,
			ElseStatement,
			ImportStatement,
			BreakStatement,
			ContinueStatement,
			ClassDeclaration, /* class Class: */

			VarAssignmentStatement, /* var a = 1 */
			AssignmentStatement, /* a = 1 */
			BlockStatement,

			/* Loops */
			WhileLoop,
			ForLoop,

			/* Expressions */
			NullExpression,
			BinaryExpression,
			NumericLiteralExpression,
			StringLiteralExpression,
			IdentifierExpression,
			UnaryOperation, /* Should be unary expression */
			AccessorExpression,
			MemberExpression,
			ArrayExpression,
			FunctionDeclarationExpression, /* Expr so we can assign it to variables */
			MemberFunctionDeclaration, 
			CallExpression,
			AssignmentExpression, /* a := 1 */
			ReturnExpression
		};

		Type type{};
	};


#define LE_STATEMENT_TYPE_TO_STRING_CASE(type) case Statement::Type::type: return #type
	inline auto to_string(Statement::Type type) -> StringView
	{
		switch (type)
		{
			LE_STATEMENT_TYPE_TO_STRING_CASE(IfStatement);
			LE_STATEMENT_TYPE_TO_STRING_CASE(AssignmentExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(ElifStatement);
			LE_STATEMENT_TYPE_TO_STRING_CASE(ElseStatement);
			LE_STATEMENT_TYPE_TO_STRING_CASE(FunctionDeclarationExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(CallExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(BlockStatement);
			LE_STATEMENT_TYPE_TO_STRING_CASE(AccessorExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(StringLiteralExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(VarAssignmentStatement);
			LE_STATEMENT_TYPE_TO_STRING_CASE(AssignmentStatement);
			LE_STATEMENT_TYPE_TO_STRING_CASE(BinaryExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(IdentifierExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(NumericLiteralExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(ArrayExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(MemberExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(UnaryOperation);
			LE_STATEMENT_TYPE_TO_STRING_CASE(NullExpression);
			LE_STATEMENT_TYPE_TO_STRING_CASE(ClassDeclaration);
			LE_STATEMENT_TYPE_TO_STRING_CASE(MemberFunctionDeclaration);
		}
		return "Unknown";
	}
#undef LE_STATEMENT_TYPE_TO_STRING_CASE

	struct Expression : Statement {};
	using PExpression = std::unique_ptr<Expression>;

	/* a := 10 */
	struct AssignmentExpression : Expression
	{
		AssignmentExpression() { type = Type::AssignmentExpression; }

		PExpression target{}; /* Access, identifier, member expressions */
		PExpression right{};
	};

	/* a = 10 */
	struct AssignmentStatement : Expression
	{
		AssignmentStatement() { type = Type::AssignmentStatement; }

		PExpression target{}; /* Access, identifier, member expressions */
		PExpression right{};
	};

	/* class 'identifier': */
	struct ClassDeclaration : Statement
	{
		ClassDeclaration() { type = Type::ClassDeclaration; }
		
		Symbol name{};
		std::vector<PStatement> members{};
	};

	/* import [identifier] */
	struct ImportStatement : Statement
	{
		explicit ImportStatement(Symbol target_ = "") : target(target_) { type = Type::ImportStatement; }

		Symbol target{};
		Symbol alias{}; /* Can be null */
	};

	/* Execute while iter does not return null */
	struct ForLoop : Statement
	{
		ForLoop() { type = Type::ForLoop; }

		Symbol var{}; /* for 'var' in 'target' */
		PExpression target{};
		PStatement body{};
	};

	/* Execute body while 'expr' evaluates to true */
	struct WhileLoop : Statement
	{
		WhileLoop() { type = Type::WhileLoop; }

		PExpression expr{};
		PStatement body{};
	};

	struct VarAssignment : Statement
	{
		VarAssignment() { type = Type::VarAssignmentStatement; }

		Symbol target{};
		PExpression right{};
	};

	struct IfStatement : Statement
	{
		IfStatement() { type = Type::IfStatement; }

		PExpression test{};
		PStatement consequent{};
		PStatement alternative{};
	};

	struct BlockStatement : Statement
	{
		/* Some blocks can be escaped by one of the following keywords */
		enum EscapeReason : unsigned {
			None = 0b0001u,
			Break = 0b0010u,
			Continue = 0b0100u,
			Return = 0b1000u
		};

		BlockStatement() { type = Type::BlockStatement; }

		std::vector<PStatement> body{};

		/* To be initialized at the start of the scope, for example when opening function scope */
		std::vector<String>* block_arg_names{ nullptr };
		std::vector<PExpression>* block_args{ nullptr };
		/* Whitelist of keywords that can interact with this block, these are set by the parser */
		std::underlying_type_t<EscapeReason> accepted_escape_reasons{ EscapeReason::None };
		EscapeReason was_escaped_with{ EscapeReason::None };
	};

	inline constexpr auto a = sizeof(BlockStatement);

	inline constexpr auto to_string(BlockStatement::EscapeReason reason) -> std::string
	{
		switch (reason)
		{
		case BlockStatement::EscapeReason::Break: return "break";
		case BlockStatement::EscapeReason::Continue: return "continue";
		case BlockStatement::EscapeReason::Return: return "return";
		default: return "UnknownEscape";
		}
	}

	struct BreakStatement : Statement
	{
		BreakStatement() { type = Type::BreakStatement; }
	};

	struct ContinueStatement : Statement
	{
		ContinueStatement() { type = Type::ContinueStatement; }
	};

	struct ReturnExpression : Expression
	{
		explicit ReturnExpression(PExpression expr_ = nullptr) : expr(std::move(expr_)) { type = Type::ReturnExpression; }
		PExpression expr{};
	};

	struct FunctionDeclaration : Expression
	{
		FunctionDeclaration() { type = Type::FunctionDeclarationExpression; }

		Symbol name{}; /* Can be empty for lambda's */
		std::vector<Symbol> args{};
		std::unique_ptr<BlockStatement> body{};
	};

	/* A function with a special identifier so we know to pass it a this pointer */
	/* Currently unused */
	//struct MemberFunctionDeclaration : FunctionDeclaration
	//{
	//	MemberFunctionDeclaration() { type = Type::MemberFunctionDeclaration; }
	//};

	struct CallExpression : Expression
	{
		CallExpression() { type = Type::CallExpression; }

		PExpression target{}; /* Name of func */
		std::vector<PExpression> args{};
	};

	struct ArrayExpression : Expression
	{
		ArrayExpression() { type = Type::ArrayExpression; }

		std::vector<PExpression> container{};
	};

	struct AccessorExpression : Expression
	{
		AccessorExpression() { type = Type::AccessorExpression; }

		PExpression target{};
		PExpression query{};
	};

	struct MemberExpression : Expression
	{
		MemberExpression() { type = Type::MemberExpression; }

		PExpression target{};
		PExpression query{};
	};

	inline auto make_accessor_expression(PExpression& target, PExpression& query) -> PExpression
	{
		auto accessor = std::make_unique<AccessorExpression>();
		accessor->target = std::move(target);
		accessor->query = std::move(query);
		return accessor;
	}

	struct NullLiteral : Expression
	{
		NullLiteral() { type = Type::NullExpression; }
	};

	struct BinaryOperation : Expression
	{
		BinaryOperation() { type = Type::BinaryExpression; }

		PExpression left{};
		PExpression right{};
		Token op{};
	};

	inline auto make_binary_operation(PExpression& left, PExpression& right, const Token& op) -> PExpression
	{
		auto binop = std::make_unique<BinaryOperation>();
		binop->left = std::move(left);
		binop->right = std::move(right);
		binop->op = op;
		return binop;
	}

	template<typename _Expr>
	inline auto make_assignment(PExpression& target, PExpression& right) -> PExpression
		requires any_of<_Expr, AssignmentExpression, AssignmentStatement>
	{
		auto expr = std::make_unique<_Expr>();
		expr->target = std::move(target);
		expr->right = std::move(right);
		return expr;
	}

	struct StringLiteral : Expression
	{
		StringLiteral() { type = Type::StringLiteralExpression; }
		explicit StringLiteral(StringView view)
			: string(view)
		{
			type = Type::StringLiteralExpression;
		}
		StringView string{};
	};

	struct NumericLiteral : Expression
	{
		NumericLiteral(Number value = {}) : value(value) { type = Type::NumericLiteralExpression; }
		Number value{};
	};

	struct UnaryOperation : Expression
	{
		UnaryOperation() { type = Type::UnaryOperation; }

		PExpression target{};
		Token op{};
	};

	inline auto make_unary_operation(PExpression& target, const Token& op) -> PExpression
	{
		auto unary_op = std::make_unique<UnaryOperation>();
		unary_op->op = op;
		unary_op->target = std::move(target);
		return unary_op;
	}

	struct Identifier : Expression
	{
		Identifier(Symbol name)
			: name(name) {
			type = Type::IdentifierExpression;
		}

		Symbol name{};
	};

	inline auto make_identifier(const Token& token) -> std::unique_ptr<Identifier>
	{
		return std::make_unique<Identifier>(token.raw);
	}

	inline auto make_string_literal(const Token& token) -> std::unique_ptr<StringLiteral>
	{
		return std::make_unique<StringLiteral>(token.raw);
	}

	inline auto make_member_expression(PExpression& target, const Token& member) -> PExpression
	{
		auto accessor = std::make_unique<MemberExpression>();
		accessor->target = std::move(target);
		accessor->query = make_string_literal(member);
		return accessor;
	}

	/* TODO error handling */
	inline auto make_numeric_literal(const Token& token, String& error) -> std::unique_ptr<NumericLiteral>
	{
		auto expr = std::make_unique<NumericLiteral>();

		auto [ptr, ec] = std::from_chars(token.raw.data(), token.raw.data() + token.raw.size(), expr->value);
		if (ec == std::errc::invalid_argument)
			error = std::format("Failed to parse number at line {} column {}, number: '{}'", token.line, token.column, token.raw);
		else if (ec == std::errc::result_out_of_range)
			error = std::format("The given number is too large at line {} column {}, number: '{}'", token.line, token.column, token.raw);

		if (error.empty())
			return expr;
		return nullptr;
	}
}

