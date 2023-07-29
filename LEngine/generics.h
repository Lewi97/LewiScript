#pragma once

#include <concepts>

namespace le
{
	template <class _Type, class... _Others>
	concept any_of = (std::same_as<_Type, _Others> or ...);
}
