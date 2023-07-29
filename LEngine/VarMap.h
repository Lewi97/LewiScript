#pragma once

#include "common.h"
#include "format_errs.h"

#include <unordered_map>

namespace le
{
	/* Keeps track of variable name index translation */
	struct VarMap
	{
		using Index = size_t;
		using Map = std::unordered_map<Symbol, Index>;
		Index count{};
		Map map{};

		auto store(const Symbol& str) -> Index
		{
			if (has(str))
				throw(ferr::variable_already_declared(str));
			return map.insert({ str, count++ }).first->second;
		}
		auto get(const Symbol& str) -> Index
		{
			if (not has(str))
				throw(ferr::variable_not_declared(str));
			return map.at(str);
		}

		auto has(const Symbol& str) -> bool { return map.find(str) != map.end(); }
	};
}

