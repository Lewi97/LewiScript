#pragma once

#include <ranges>

#include "common.h"

namespace le::hashing
{
	template<size_t _Prime, size_t _Hash, std::unsigned_integral _Out>
	struct FNVHash
	{
		template<
			typename _String,
			typename _Val = _String::value_type>
		auto operator()(const _String& str) const -> _Out
			requires(std::ranges::range<_String> and std::same_as<char, _Val>)
		{
			return hash(str);
		}

		template<
			typename _String,
			typename _Val = _String::value_type>
		static auto hash(const _String& str) -> _Out
		{
			auto hash = _Hash;
			for (auto c : str)
			{
				hash ^= c;
				hash *= _Prime;
			}
			return hash;
		}
	};

	/* prime and hash val from boost */
	using Hasher = FNVHash<1099511628211u, 14695981039346656037u, hash_t>;
}

