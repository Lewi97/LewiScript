#pragma once

#include <string_view>
#include <ctype.h>
#include <array>
#include <iostream>
#include <iomanip>

#define LE_TURN_ON_DEBUG_PRINTS 0
#define LE_DEBUG_PRINT(format_str, ...) std::cout << std::format(format_str, __VA_ARGS__)

namespace le
{
	using Exception = std::exception;
	using StringView = std::string_view;
	using String = std::string;
	using i64 = std::int64_t;
	using u64 = std::uint64_t;
	using i32 = std::int32_t;
	using u32 = std::uint32_t;
	using u8 = std::uint8_t;
	using f32 = float;
	using f64 = double;
	using Number = f64;
	using hash_t = u64;
	using Symbol = StringView;

	namespace precedences
	{
		enum {
			assignment = 1,
			additive,
			multiplicative,
			relational,
			equality,
		};
	}
}

