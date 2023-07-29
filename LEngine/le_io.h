#pragma once

#include <span>

#include "GlobalState.h"
#include "builtin.h"

namespace le::lib::io
{
	auto print(std::span<LeObject> args) -> LeObject
	{
		for (const auto& obj : args)
		{
			std::cout << obj->make_string() << ' ';
		}

		std::cout << '\n';

		return global::mem->emplace<NullValue>();
	}
}

