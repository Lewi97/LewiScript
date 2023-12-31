#pragma once

#include "common.h"
#include "GlobalState.h"
#include "Builtin.h"
#include "CPPLeFunction.h"
#include "hashing.h"
#include "TypeFactory.h"
#include "Range.h"
#include "Null.h"

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

		return make::make_string(args.front()->type_name());
	}

	inline auto get_iterator(std::span<LeObject> args, MemoryManager& mem) -> LeObject
	{
		if (args.size() != 1)
			throw(ferr::too_many_arguments(args.size(), 1, "iterator"));

		return args.front()->iterator(args.front());
	}

	inline auto get_string(std::span<LeObject> args, MemoryManager& mem) -> LeObject
	{
		if (args.size() != 1)
			throw(ferr::too_many_arguments(args.size(), 1, "String"));

		return make::make_string(args.front()->make_string());
	}

	/* Is a specific symbol reserved */
	inline auto is_reserved(Symbol symbol) -> bool
	{
		const auto hash = hashing::Hasher::hash(symbol);

		switch (hash)
		{
			/* Functions */
		case hashing::Hasher::hash("print"):
		case hashing::Hasher::hash("type"):
			/* Types */
		case hashing::Hasher::hash("Iterator"):
		case hashing::Hasher::hash("String"):
		case hashing::Hasher::hash("Range"):
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
		case hashing::Hasher::hash("Iterator"):
			return global::mem->emplace<ImportedFunction>(get_iterator, "Iterator");
		case hashing::Hasher::hash("String"):
			return global::mem->emplace<ImportedFunction>(get_string, "String");
		case hashing::Hasher::hash("Range"):
			return global::mem->emplace<ImportedFunction>(range_constructor, "Range");
		//case hashing::Hasher::hash("range"):
		default:
			throw(ferr::make_exception(std::format("Tried accessing non existent global '{}'", symbol)));
			return nullptr;
		}
	}
}
