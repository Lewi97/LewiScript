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

	auto to_string(Token::Type type) -> StringView;

	auto operator<<(std::ostream& out, const Token& token)->std::ostream&;

	inline constexpr auto null_token() -> Token { return Token{ Token::Type::Null, StringView{}, -1, -1 }; }
	inline constexpr auto eof_token() -> Token { return Token{ Token::Type::EoF, StringView{}, -1, -1 }; }
		
// #define LE_SCRIPT_RESERVED_KEYWORD_LIST "var", "fn", "end", "if", "elif", "else", "while", "import"

	auto to_keyword(const StringView& view) -> Token::Type;

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
		using Underlying = std::underlying_type_t<Token::Type>;
		return static_cast<Underlying>(token.type) >= static_cast<Underlying>(Token::Type::OperatorEquals);
	}

	namespace precedences
	{
		enum {
			assignment = 1,
			additive,
			multiplicative,
			relational,
			equality,
		};
	}

	auto precedence(const Token::Type type) -> int;
	auto to_operator(const StringView& view) -> Token::Type;
}

