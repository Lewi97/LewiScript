#include "common.h"

namespace le
{
	auto to_keyword(const StringView& view) -> Token::Type
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

#define LE_TO_STRING(type) case Token::Type::type: return #type
	auto to_string(Token::Type type) -> StringView
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
	
	auto operator<<(std::ostream& out, const Token& token) ->std::ostream&
	{
		out << "{ " << to_string(token.type) << ' ' << std::quoted(token.raw) << " column: " << token.column << " line: " << token.line << " }";
		return out;
	}

	auto precedence(const Token::Type type) -> int
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

	auto to_operator(const StringView& view) -> Token::Type
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