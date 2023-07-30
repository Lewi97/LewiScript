#pragma once

#include "Token.h"

namespace le
{
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
}
