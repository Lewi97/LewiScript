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
			return 
		}

		auto make_string() -> String override
		{
			return String("Null");
		}
    };
}

