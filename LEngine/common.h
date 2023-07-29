#pragma once

#include <string_view>
#include <ctype.h>
#include <array>
#include <iostream>
#include <iomanip>

#define LE_TURN_ON_DEBUG_PRINTS 0
#define LE_DEBUG_PRINT(format_str, ...) std::cout << std::format(format_str, __VA_ARGS__)

namespace le
{
	using Exception = std::exception;
	using StringView = std::string_view;
	using String = std::string;
	using i64 = std::int64_t;
	using u64 = std::uint64_t;
	using i32 = std::int32_t;
	using u32 = std::uint32_t;
	using u8 = std::uint8_t;
	using f32 = float;
	using f64 = double;
	using Number = f64;
	using hash_t = u64;
	using Symbol = StringView;

	struct Token
	{
		enum class Type
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
	
#define LE_TOKEN_TYPE_TO_STRING_CASE(type) case Token::Type::type: return #type
	inline auto to_string(Token::Type type) -> StringView
	{
		switch (type)
		{
			LE_TOKEN_TYPE_TO_STRING_CASE(Null);
			LE_TOKEN_TYPE_TO_STRING_CASE(Dot);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorGT);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorGET);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorLT);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorLET);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorEq);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorNEq);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorNot);
			LE_TOKEN_TYPE_TO_STRING_CASE(NewLine);
			LE_TOKEN_TYPE_TO_STRING_CASE(KeywordVar);
			LE_TOKEN_TYPE_TO_STRING_CASE(StringLiteral);
			LE_TOKEN_TYPE_TO_STRING_CASE(NumericLiteral);
			LE_TOKEN_TYPE_TO_STRING_CASE(Identifier);
			LE_TOKEN_TYPE_TO_STRING_CASE(Comma);
			LE_TOKEN_TYPE_TO_STRING_CASE(EoF);
			LE_TOKEN_TYPE_TO_STRING_CASE(KeywordEnd);
			LE_TOKEN_TYPE_TO_STRING_CASE(KeywordIf);
			LE_TOKEN_TYPE_TO_STRING_CASE(KeywordElse);
			LE_TOKEN_TYPE_TO_STRING_CASE(KeywordElif);
			LE_TOKEN_TYPE_TO_STRING_CASE(OpenParen);
			LE_TOKEN_TYPE_TO_STRING_CASE(CloseParen);
			LE_TOKEN_TYPE_TO_STRING_CASE(OpenSquareBracket);
			LE_TOKEN_TYPE_TO_STRING_CASE(CloseSquareBracket);
			LE_TOKEN_TYPE_TO_STRING_CASE(Colon);
			LE_TOKEN_TYPE_TO_STRING_CASE(KeywordFn);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorEquals);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorPlus);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorMinus);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorMultiply);
			LE_TOKEN_TYPE_TO_STRING_CASE(OperatorDivide);
		}
		return "Unknown";
	}
#undef LE_TOKEN_TYPE_TO_STRING_CASE(type) case Token::Type::type: return #type

	inline auto operator<<(std::ostream& out, const Token& token) ->std::ostream&
	{
		out << "{ " << to_string(token.type) << ' ' << std::quoted(token.raw) << " column: " << token.column << " line: " << token.line << " }";
		return out;
	}

	inline constexpr auto null_token() -> Token { return Token{ Token::Type::Null, StringView{}, -1, -1 }; }
	inline constexpr auto eof_token() -> Token { return Token{ Token::Type::EoF, StringView{}, -1, -1 }; }
		
#define LE_SCRIPT_RESERVED_KEYWORD_LIST "var", "fn", "end", "if", "elif", "else", "while", "import"

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

	inline auto is_operator(const Token& token) -> bool
	{
		using Underlying = std::underlying_type<Token::Type>::type;
		return static_cast<Underlying>(token.type) >= static_cast<Underlying>(Token::Type::OperatorEquals);
	}

	inline constexpr auto assignment_prec = 1;
	inline constexpr auto additive_prec = 2;
	inline constexpr auto multiplicative_prec = 3;
	inline constexpr auto relational_prec = 4;
	inline constexpr auto equality_prec = 5;

	inline auto precedence(const Token::Type type) -> int
	{
		using op_t = Token::Type;
		switch (type)
		{
		case op_t::OperatorEquals: 
			return  assignment_prec;
		
		case op_t::OperatorPlus:  
		case op_t::OperatorMinus: 
			return additive_prec;
		
		case op_t::OperatorMultiply: 
		case op_t::OperatorDivide: 	 
			return multiplicative_prec;

		case op_t::OperatorGET: 
		case op_t::OperatorGT:	
		case op_t::OperatorLT:	
		case op_t::OperatorLET:	
			return relational_prec;

		case op_t::OperatorEq: 
		case op_t::OperatorNEq: 
			return equality_prec;
		}
		return -1;
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
}

