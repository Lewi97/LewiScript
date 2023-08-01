#pragma once

#include "Token.h"

namespace le
{
	inline auto is_operator(const Token& token) -> bool
	{
		using Underlying = std::underlying_type_t<Token::Type>;
		return static_cast<Underlying>(token.type) >= static_cast<Underlying>(Token::Type::OperatorEquals);
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
			case ':': return Token::Type::Colon; /* The lexer will try and place this as a walrus operator so we just recognize it here */
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
			case op_hash(':', '='): return Token::Type::OperatorWalrus;
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
		case ':': 
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
		case op_t::OperatorWalrus:
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
