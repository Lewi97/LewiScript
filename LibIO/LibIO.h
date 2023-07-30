#pragma once

#include "../LEngine/Builtin.h"

namespace le::lib::io
{
	auto print(std::span<LeObject> args, struct MemoryManager&) -> LeObject;
}

