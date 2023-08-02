#pragma once

#include "Builtin.h"
#include "Function.h"

#include <vector>
#include <unordered_map>

namespace le
{
	class Class
		: RuntimeValue
	{
		Class() { type = Type::Class; }

		String name{};
		std::unordered_map<String, LeObject> members{};
		std::vector<Function> functions{};

		auto type_name() -> String override
		{
			return name;
		}


	};

	constexpr auto size__class = sizeof(Class);

}

