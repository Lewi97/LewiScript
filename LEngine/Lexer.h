#pragma once

#include "common.h"
#include "iter_tools.h"
#include "format_errs.h"
#include "Token.h"
#include "Operators.h"
#include "Keywords.h"

namespace le
{

	class Lexer
	{
	protected:
		using Iterator = StringView::iterator;

		StringView _source{};
		Iterator _source_iter{};

		Token _current{ eof_token() };
		i32 _current_line{};
		i32 _current_column{};

		bool _empty{ true };

		auto advance_line() -> void
		{
			_current_line++;
			_current_column = 0;
		}

		auto advance_column() -> void
		{
			_current_column++;
		}

		/* Reads data to token raw property while predicate is true 
		@return End of token iterator */
		template<std::predicate<char> _Pred>
		auto parse_token_while(Iterator itr, Token& token, _Pred&& predicate) -> Iterator
		{
			auto end = iters::advance_while(itr, _source.end(), predicate);
			token.raw = StringView(itr, end);
			return end;
		}

		/* Reads data to token raw property while predicate is true
		@return End of token iterator */
		auto parse_token_between(Iterator itr, Token& token, char guard) -> Iterator
		{
			if (*itr == guard) 
			{
				advance_column();
				itr++;
			}
			auto end = std::find(itr, _source.end(), guard);
			if (itr == _source.end())
				throw(ferr::make_exception(std::format("Unclosed {} expected {} at line {} column {}"
				, to_string(token.type), guard, _current_line, _current_column)));
			token.raw = StringView(itr, end); /* end == guard so subtract 1 */
			_current_column += static_cast<i32>(std::distance(itr, ++end)); /* increment end over guard */
			return end;
		}

		auto parse_next_token() -> Token
		{
			auto token = Token{};
			auto itr = _source_iter;				

			if (itr == _source.end())
			{
				token.type = Token::Type::EoF;
				token.column = _current_column;
				token.line = _current_line;
				_empty = true;
				return token;
			}

			while (itr != _source.end())
			{
				auto c = *itr;

				token.column = _current_column;
				token.line = _current_line;		
				
				if (isspace(c))
				{
					itr++;
					if (c == '\n')
					{
						advance_line();
					}
					else
					{
						advance_column();
					}
					continue;
				}
				else if (isalpha(c))
				{
					itr = parse_token_while(itr, token,
						[this](char c) -> bool
						{
							advance_column();
							return static_cast<bool>(isalpha(c)) or static_cast<bool>(isdigit(c)) or c == '_';
						});
					const auto keyword = to_keyword(token.raw);
					token.type = keyword != Token::Type::Null ? keyword : Token::Type::Identifier;
				}
				else if (isdigit(c))
				{
					itr = parse_token_while(itr, token,
						[this](char c) -> bool
						{
							advance_column();
							return static_cast<bool>(isdigit(c)) or c == '.';
						});
					token.type = Token::Type::NumericLiteral;
				}
				else if (is_operator(c))
				{
					itr = parse_token_while(itr, token,
						[this](char c) -> bool
						{
							advance_column();
							return is_operator(c);
						});
					token.type = to_operator(token.raw);
				}
				else
				{
					/* Single characters */
					switch (c)
					{
					case '(':
						token.type = Token::Type::OpenParen;
						token.raw = StringView(itr, itr + 1); itr++; break;
					case ')':
						token.type = Token::Type::CloseParen;
						token.raw = StringView(itr, itr + 1); itr++; break;
					case '[':
						token.type = Token::Type::OpenSquareBracket;
						token.raw = StringView(itr, itr + 1); itr++; break;
					case ']':
						token.type = Token::Type::CloseSquareBracket;
						token.raw = StringView(itr, itr + 1); itr++; break;
					case '"':
						token.type = Token::Type::StringLiteral;
						itr = parse_token_between(itr, token, '"'); break;
					case ',':
						token.type = Token::Type::Comma;
						token.raw = StringView(itr, itr + 1); itr++; break;
					case ':':
						token.type = Token::Type::Colon;
						token.raw = StringView(itr, itr + 1); itr++; break;
					case '.':
						token.type = Token::Type::Dot;
						token.raw = StringView(itr, itr + 1); itr++; break;
					default: /* unrecognized */
						itr++; advance_column();
					}
				}

				if (token.type == Token::Type::Null) 
					continue;
				break;
			}

			if (itr == _source.end())
			{
				token.type = Token::Type::EoF;
				token.column = _current_column;
				token.line = _current_line;
				_empty = true;
				return token;
			}

			_source_iter = itr;

			return token;
		}
	public:
		Lexer() = default;

		auto tokenize(StringView source) -> void
		{
			_source = source;
			
			_current_column = 0;
			_current_line = 0;

			if (source.empty()) return;

			_source_iter = source.begin();
			_empty = _source.empty();
		}

		/* Get current token */
		auto current() -> Token&
		{
			return _current;
		}

		/* Parse next token and return reference to it */
		auto advance() -> Token&
		{
			if (not empty())
			{
				_current = parse_next_token();
			}

			return current();
		}

		/* Advance and return previous token */
		auto eat() -> Token
		{
			const auto old = current();
			advance();
			return old;
		}

		auto empty() -> bool
		{
			return _empty;
		}
	};
}
