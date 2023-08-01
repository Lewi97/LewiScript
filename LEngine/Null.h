#pragma once

#include "Builtin.h"

namespace le
{
    struct NullValue :
        RuntimeValue
    {
		NullValue()
		{
			type = Type::Null;
		}

		auto type_name() -> String override
		{
			return "Null";
		}

		auto to_native_bool() const -> bool override
		{
			return false;
		}

		auto make_string() -> String override
		{
			return String("Null");
		}
    };
}

