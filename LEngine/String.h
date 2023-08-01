#pragma once

#include "Builtin.h"
#include "format_errs.h"
#include "Number.h"
#include "Boolean.h"
#include "Iterator.h"

/*
* Builtin string type, uses std::string for its implementation.
* 
* Usage:
* "text..."
* var 'identifier' = "text..."
*/

namespace le
{
	struct StringValue : RuntimeValue
	{
		StringValue()
		{ type = Type::String; }

		explicit StringValue(StringView string_)
			: string(string_)
		{ type = Type::String; }

		explicit StringValue(String string_)
			: string(string_)
		{ type = Type::String; }
		
		explicit StringValue(String::value_type c)
			: string(1, c)
		{ 
			type = Type::String; 
		}

		String string{};

		auto make_string() -> String override
		{
			return string;
		}

		auto type_name() -> String override
		{
			return "String";
		}

		auto bounds_check(size_t idx) const -> bool { return idx < string.size(); }

		auto access(LeObject index) -> LeObject override
		{
			auto idx = to_numeric_index(*index);
			return _make_small_string(idx);
		}

		auto _make_small_string(size_t idx) -> LeObject
		{
			/* Return a new string object, even if it just holds one character, SBO will avoid a heap allocation anyway */
			auto new_str = global::mem->emplace<StringValue>();
			
			if (bounds_check(idx))
			{
				auto c = string.at(idx);
				new_str->string.push_back(c);
			}
			else
			{
				throw(ferr::index_out_of_range(idx));
			}

			return new_str;
		}

		auto access_assign(LeObject index, LeObject rhs) -> LeObject override
		{
			throw(ferr::make_exception("Contents of string are immutable"));
			return LeObject{};
		}

		auto to_native_bool() const -> bool override
		{
			return not string.empty();
		}

		auto handle_string_op(Token::Type op, String& other) -> LeObject
		{
			switch (op)
			{
			case Token::Type::OperatorPlus:
			{
				auto new_str = global::mem->emplace<StringValue>();
				new_str->string = string + other;
				return new_str;
			}
			case Token::Type::OperatorEq: return Boolean::make_bool(string == other);
			case Token::Type::OperatorNEq: return Boolean::make_bool(string != other);
			default:
				throw(ferr::bad_overload(op, "String"));
			}
		}

		auto apply_operation(Token::Type op, LeObject other) -> LeObject override
		{
			switch (other->type)
			{
			case Type::String:
			{
				auto& other_string = static_cast<StringValue*>(other.get())->string;
				return handle_string_op(op, other_string);
			}
			default:
				throw(ferr::invalid_operation(op, to_string(type), to_string(other->type)));
			}

			return {};
		}

		auto iterator(LeObject self) -> LeObject override
		{
			auto iterator_next =
				[count = 0ull](StringValue& self) mutable -> LeObject
			{
				if (count < self.string.size())
					return self._make_small_string(count++);
				return global::null;
			};

			return global::mem->emplace<Iterator<StringValue, decltype(iterator_next)>>(self, iterator_next);
		}
	};

	constexpr auto size__string = sizeof(StringValue);
}

