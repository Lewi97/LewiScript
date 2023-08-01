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

	inline auto get_type(std::span<LeObject> args, MemoryManager& mem) -> LeObject
	{
		if (args.size() != 1)
			throw(ferr::too_many_arguments(args.size(), 1, "type"));

		return args.front()->type_name();
	}

	/* Is a specific symbol reserved */
	inline auto is_reserved(Symbol symbol) -> bool
	{
		const auto hash = hashing::Hasher::hash(symbol);

		switch (hash)
		{
		case hashing::Hasher::hash("print"):
		case hashing::Hasher::hash("type"):
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
		case hashing::Hasher::hash("type"):
			return global::mem->emplace<ImportedFunction>(get_type, "type");
		//case hashing::Hasher::hash("range"):
		default:
			throw(ferr::make_exception("Tried accessing non existent global"));
			return nullptr;
		}
	}
}
