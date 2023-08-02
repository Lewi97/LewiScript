#pragma once

#include "Builtin.h"


namespace le::getters
{
	auto get_number(const LeObject& obj, const char* context) -> Number;
	auto get_string_ref(const LeObject& obj, const char* context) -> String&;
}
