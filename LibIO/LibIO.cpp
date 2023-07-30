// LibIO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "LibIO.h"
#include "../LEngine/MemoryManager.h"

auto le::lib::io::print(std::span<LeObject> args, MemoryManager& mem) -> LeObject
{
	for (const auto& obj : args)
	{
		std::cout << obj->make_string() << ' ';
	}

	std::cout << '\n';

	return mem.emplace<NullValue>();
}
