#pragma once

#include "Token.h"

namespace le
{
	inline auto to_keyword(const StringView& view) -> Token::Type
	{ /* We gotta clean this up lmao */
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
		if (view == "as")
			return Token::Type::KeywordAs;
		if (view == "for")
			return Token::Type::KeywordFor;
		if (view == "in")
			return Token::Type::OperatorIn;
		if (view == "Null")
			return Token::Type::KeywordNull;
		if (view == "class")
			return Token::Type::KeywordClass;
		return Token::Type::Null;
	}
}
