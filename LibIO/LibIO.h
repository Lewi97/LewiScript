#pragma once

#include "../LEngine/Builtin.h"

#ifdef LIB_EXPORT
#define LE_LIBIO_API __declspec(dllexport)
#else
#define LE_LIBIO_API __declspec(dllimport)
#endif

namespace le::lib::io
{
	extern "C" {
		auto LE_LIBIO_API print(std::span<LeObject> args, struct MemoryManager&) -> LeObject;
	}
}

#undef LE_LIBIO_API