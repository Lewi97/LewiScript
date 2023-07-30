#pragma once

#include "common.h"

namespace le
{
	struct Token
	{
		enum class Type : i64
		{
			Null,
			NumericLiteral,
			StringLiteral,
			Identifier,
			EoF,
			Comma, /* , */
			Dot, /* . */
			OpenParen, /* ( */
			CloseParen, /* ) */
			OpenSquareBracket, /* [ */
			CloseSquareBracket, /* ] */
			Colon, /* : */
			NewLine,

			/* Keywords */
			KeywordFn,
			KeywordVar,
			KeywordEnd,
			KeywordIf,
			KeywordElif,
			KeywordElse,
			KeywordWhile,
			KeywordImport,
			KeywordBreak,
			KeywordContinue,
			KeywordReturn,

			/* Operators */
			OperatorEquals,
			OperatorPlus,
			OperatorMinus,
			OperatorMultiply,
			OperatorDivide,
			OperatorGT, /* > */
			OperatorGET, /* >= */
			OperatorLT, /* < */
			OperatorLET, /* <= */
			OperatorEq, /* == */
			OperatorNEq, /* != */
			OperatorNot, /* ! */
		};

		Type type{};
		StringView raw{};

		/* Location in source */
		i32 line{};
		i32 column{};
	};

	constexpr auto size__token = sizeof(Token);

#define LE_TO_STRING(type) case Token::Type::type: return #type
	inline auto to_string(Token::Type type) -> StringView
	{
		switch (type)
		{
			LE_TO_STRING(Null);
			LE_TO_STRING(Dot);
			LE_TO_STRING(OperatorGT);
			LE_TO_STRING(OperatorGET);
			LE_TO_STRING(OperatorLT);
			LE_TO_STRING(OperatorLET);
			LE_TO_STRING(OperatorEq);
			LE_TO_STRING(OperatorNEq);
			LE_TO_STRING(OperatorNot);
			LE_TO_STRING(NewLine);
			LE_TO_STRING(KeywordVar);
			LE_TO_STRING(StringLiteral);
			LE_TO_STRING(NumericLiteral);
			LE_TO_STRING(Identifier);
			LE_TO_STRING(Comma);
			LE_TO_STRING(EoF);
			LE_TO_STRING(KeywordEnd);
			LE_TO_STRING(KeywordIf);
			LE_TO_STRING(KeywordElse);
			LE_TO_STRING(KeywordElif);
			LE_TO_STRING(OpenParen);
			LE_TO_STRING(CloseParen);
			LE_TO_STRING(OpenSquareBracket);
			LE_TO_STRING(CloseSquareBracket);
			LE_TO_STRING(Colon);
			LE_TO_STRING(KeywordFn);
			LE_TO_STRING(OperatorEquals);
			LE_TO_STRING(OperatorPlus);
			LE_TO_STRING(OperatorMinus);
			LE_TO_STRING(OperatorMultiply);
			LE_TO_STRING(OperatorDivide);
		}
		return "Unknown";
	}
#undef LE_TO_STRING(type) case Token::Type::type: return #type

	inline auto operator<<(std::ostream& out, const Token& token)->std::ostream&
	{
		out << "{ " << to_string(token.type) << ' ' << std::quoted(token.raw) << " column: " << token.column << " line: " << token.line << " }";
		return out;
	}
	inline constexpr auto null_token() -> Token { return Token{ Token::Type::Null, StringView{}, -1, -1 }; }
	inline constexpr auto eof_token() -> Token { return Token{ Token::Type::EoF, StringView{}, -1, -1 }; }

	// #define LE_SCRIPT_RESERVED_KEYWORD_LIST "var", "fn", "end", "if", "elif", "else", "while", "import"

	inline auto is_operator(const Token& token) -> bool
	{
		using Underlying = std::underlying_type_t<Token::Type>;
		return static_cast<Underlying>(token.type) >= static_cast<Underlying>(Token::Type::OperatorEquals);
	}

	inline auto to_keyword(const StringView& view) -> Token::Type
	{
		if (view == "var")
			return Token::Type::KeywordVar;
		if (view == "fn")
			return Token::Type::KeywordFn;
		if (view == "end")
			return Token::Type::KeywordEnd;
		if (view == "if")
			return Token::Type::KeywordIf;
		if (view == "elif")
			return Token::Type::KeywordElif;
		if (view == "else")
			return Token::Type::KeywordElse;
		if (view == "while")
			return Token::Type::KeywordWhile;
		if (view == "import")
			return Token::Type::KeywordImport;
		if (view == "break")
			return Token::Type::KeywordBreak;
		if (view == "continue")
			return Token::Type::KeywordContinue;
		if (view == "return")
			return Token::Type::KeywordReturn;
		/*if (view == "class")
			return Token::Type::KeywordImport;*/
		return Token::Type::Null;
	}

	inline auto to_operator(const StringView& view) -> Token::Type
	{
		if (view.size() == 1)
		{
			switch (view.at(0))
			{
			case '+': return Token::Type::OperatorPlus;
			case '-': return Token::Type::OperatorMinus;
			case '/': return Token::Type::OperatorDivide;
			case '*': return Token::Type::OperatorMultiply;
			case '=': return Token::Type::OperatorEquals;
			case '!': return Token::Type::OperatorNot;
			case '>': return Token::Type::OperatorGT;
			case '<': return Token::Type::OperatorLT;
			}
		}
		else if (view.size() == 2)
		{
			constexpr auto op_hash = [](char a, char b) constexpr -> auto { return (a << 8) | b; };
			switch (op_hash(view[0], view[1]))
			{
			case op_hash('>', '='): return Token::Type::OperatorGET;
			case op_hash('<', '='): return Token::Type::OperatorLET;
			case op_hash('!', '='): return Token::Type::OperatorNEq;
			case op_hash('=', '='): return Token::Type::OperatorEq;
			}
		}

		return Token::Type::Null;
	}
	
	inline auto is_operator(char c) -> bool
	{
		switch (c)
		{
		case '+': case '-':
		case '*': case '/':
		case '=': case '>':
		case '<': case '!':
			return true;
		}
		return false;
	}

	inline auto precedence(Token::Type type) -> int
	{
		using op_t = Token::Type;
		switch (type)
		{
		case op_t::OperatorEquals:
			return precedences::assignment;

		case op_t::OperatorPlus:
		case op_t::OperatorMinus:
			return precedences::additive;

		case op_t::OperatorMultiply:
		case op_t::OperatorDivide:
			return precedences::multiplicative;

		case op_t::OperatorGET:
		case op_t::OperatorGT:
		case op_t::OperatorLT:
		case op_t::OperatorLET:
			return precedences::relational;

		case op_t::OperatorEq:
		case op_t::OperatorNEq:
			return precedences::equality;
		}
		return -1;
	}
}
