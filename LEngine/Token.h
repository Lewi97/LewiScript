#pragma once

#include "common.h"

#include <format>
#include <iostream>

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
			KeywordAs,

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
		out << std::format("{} {} '{}' column: {} line: {} {}", '{', to_string(token.type), token.raw, token.column, token.line, '}');
		return out;
	}
	inline constexpr auto null_token() -> Token { return Token{ Token::Type::Null, StringView{}, -1, -1 }; }
	inline constexpr auto eof_token() -> Token { return Token{ Token::Type::EoF, StringView{}, -1, -1 }; }
}
