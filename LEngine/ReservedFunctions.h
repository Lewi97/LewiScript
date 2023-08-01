#pragma once

#include "common.h"
#include "GlobalState.h"
#include "Builtin.h"
#include "CPPLeFunction.h"
#include "hashing.h"

/*
* These are the types and functions that are always active in the namespace
*/
namespace le::lib::reserved
{
	inline auto print(std::span<LeObject> args, MemoryManager& mem) -> LeObject
	{
		for (const auto& obj : args)
		{
			std::cout << obj->make_string() << ' ';
		}

		std::cout << '\n';

		return mem.emplace<NullValue>();
	}

	/* Is a specific symbol reserved */
	inline auto is_reserved(Symbol symbol) -> bool
	{
		const auto hash = hashing::Hasher::hash(symbol);

		switch (hash)
		{
		case hashing::Hasher::hash("print"):
		case hashing::Hasher::hash("range"):
			return true;
		default:
			return false;
		}
	}

	inline auto get(Symbol symbol) -> LeObject
	{
		const auto hash = hashing::Hasher::hash(symbol);

		switch (hash)
		{
		case hashing::Hasher::hash("print"):
			return global::mem->emplace<ImportedFunction>(print, "print");
		//case hashing::Hasher::hash("range"):
		default:
			throw(ferr::make_exception("Tried accessing non existent global"));
			return nullptr;
		}
	}
}
