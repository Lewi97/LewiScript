#pragma once

#include "common.h"
#include "Builtin.h"

namespace le::make
{
	auto make_string(String string) -> LeObject;
	auto make_number(Number number) -> LeObject;
	auto make_null() -> LeObject;
}

