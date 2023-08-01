#pragma once

#include "GlobalState.h"
#include "Builtin.h"

namespace le
{
    struct Boolean :
        public RuntimeValue
    {
		bool val{ false };

		explicit Boolean(bool v) : val(v) { type = Type::Boolean; }

		auto type_name() -> String override
		{
			return "Boolean";
		}

		auto make_string() -> String override
		{
			if (val) return "True";
			return "False";
		}

		static auto make_bool(bool val) -> LeObject
		{
			return global::mem->emplace<Boolean>(val);
		}

		auto to_native_bool() const -> bool override
		{
			return val;
		}

		/* we handle boolean operations as if boolean was a number instead of a bool because bools are implicitly convertible to number */
		auto apply_operation(Token::Type op, LeObject other) -> LeObject override;

		auto apply_operation(Token::Type op) -> LeObject override
		{
			if (op == Token::Type::OperatorNot)
			{
				return global::mem->emplace<Boolean>(not val);
			}
			throw(ferr::invalid_operation(op, "boolean"));
			return LeObject{};
		}
    };


}

