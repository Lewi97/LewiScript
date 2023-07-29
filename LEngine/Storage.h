#pragma once

#include "common.h"
#include "Builtin.h"
#include "hashing.h"

#include <unordered_map>
#include <vector>

namespace le
{
	class Storage
	{
	protected:
		using VarMap = std::unordered_map<hash_t, LeObject>;
		std::vector<VarMap> _variables{};
		hashing::Hasher _hasher{};

		auto find(hash_t hash) -> LeObject*
		{
			for (auto itr = _variables.rbegin(); itr != _variables.rend(); itr++)
			{
				auto result = itr->find(hash);
				if (result != itr->end())
				{
					return &result->second;
				}
			}
			return nullptr;
		}
	public:
		Storage() = default;
		
		auto open_scope() -> void
		{
			_variables.push_back(VarMap{});
		}

		auto close_scope() -> void
		{
			_variables.pop_back();
		}

		auto top() -> VarMap& { return _variables.back(); }

		auto declare(const StringView& name, LeObject value) -> void
		{
			const auto hash = _hasher(name);
			top()[hash] = value;
		}

		auto assign(const StringView& name, LeObject value) -> void
		{
			const auto hash = _hasher(name);
			if (auto var = find(hash))
				*var = value;
			else
				top()[hash] = value;
		}

		auto get(const StringView& name) -> LeObject
		{
			if (auto result = find(_hasher(name)))
				return *result;
			else
				throw(ferr::variable_not_declared(name));
		}

		auto has(const StringView& name) -> bool
		{
			return top().find(_hasher(name)) != top().end();
		}

		auto has(hash_t hash) -> bool
		{
			return top().find(hash) != top().end();
		}

		auto clear() -> void { _variables.clear(); }
	};
}
