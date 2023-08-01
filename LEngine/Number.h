#pragma once

#include "GlobalState.h"
#include "Builtin.h"
#include "Boolean.h"
#include "format_errs.h"

#include <charconv>

namespace le
{
	struct NumberValue : RuntimeValue
	{
		explicit NumberValue(Number n = Number{}) : number(n) { type = Type::NumericLiteral; }
		Number number{};

		auto type_name() -> String override
		{
			return "Number";
		}

		auto make_string() -> String override
		{
			auto str = std::array<char, 12>{};
			
			std::to_chars(str.data(), str.data() + str.size(), number);
			auto itr = std::find(str.begin(), str.end(), ' ');
			
			if (itr == str.end()) 
				return String(str.begin(), str.end());
			else	
				return String(str.begin(), itr);
		}

		auto apply_operation(Token::Type op, LeObject other) -> LeObject override
		{
			auto lval = number;
			auto rval = Number{};
			auto result = global::mem->emplace<NumberValue>();

			switch (other->type)
			{
				case Type::NumericLiteral:
					rval = static_cast<const NumberValue*>(other.get())->number; break;
				case Type::Boolean:
					rval = static_cast<Number>(static_cast<const Boolean*>(other.get())->val); break;
				default:
					throw(ferr::invalid_operation(op, to_string(type), to_string(other->type)));
			}

			switch (op)
			{
			case Token::Type::OperatorPlus:		result->number = lval + rval; break;
			case Token::Type::OperatorMinus:	result->number = lval - rval; break;
			case Token::Type::OperatorDivide:	result->number = lval / rval; break;
			case Token::Type::OperatorMultiply: result->number = lval * rval; break;
			case Token::Type::OperatorGET:  return Boolean::make_bool(lval >= rval);
			case Token::Type::OperatorGT:	return Boolean::make_bool(lval >  rval);
			case Token::Type::OperatorLET:	return Boolean::make_bool(lval <= rval);
			case Token::Type::OperatorLT:	return Boolean::make_bool(lval <  rval);
			case Token::Type::OperatorEq:	return Boolean::make_bool(lval == rval);
			case Token::Type::OperatorNEq:	return Boolean::make_bool(lval != rval);
			default:
				throw(ferr::invalid_operation(op, to_string(type), to_string(other->type))); break;
			}

			return result;
		}

		auto apply_operation(Token::Type op) -> LeObject override
		{
			auto result = global::mem->emplace<NumberValue>();

			switch (op)
			{
			case Token::Type::OperatorPlus:		result->number = +number; break;
			case Token::Type::OperatorMinus:	result->number = -number; break;
			default:
				throw(ferr::invalid_operation(op, to_string(type))); break;
			}

			return result;
		}

		inline static auto make_number_val(Number n) -> LeObject
		{
			return global::mem->emplace<NumberValue>(n);
		}
	
		auto to_native_bool() const -> bool override
		{
			return number != 0.0;
		}
	};

	inline auto to_numeric_index(const RuntimeValue& val) -> u64
	{
		if (val.type != RuntimeValue::Type::NumericLiteral)
			throw(ferr::make_exception(std::format("Cannot create numeric index from {}", to_string(val.type))));

		auto index = static_cast<const NumberValue*>(&val)->number;
		const auto is_integer = floor(index) == index; /* floor(50.5) == 50 so 50.5 == 50 is false */
		if (not is_integer)
		{
			throw(ferr::make_exception(std::format("Cannot use non integral numbers as indices, number is {}", index)));
		}

		return static_cast<size_t>(index);
	}

}

