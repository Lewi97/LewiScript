#pragma once

#include "Builtin.h"
#include "Function.h"
#include "getters.h"

#include <vector>
#include <span>
#include <unordered_map>

namespace le
{
	// class VirtualMachine;

	struct Class
		: RuntimeValue
	{
		Class() { type = Type::Class; }

		String name{};
		std::unordered_map<Symbol, LeObject> members{};
		
		auto type_name() -> String override
		{
			return name;
		}

		auto member_access(LeObject self, const String& query) -> LeObject override
		{
			if (auto member = has_member(query))
			{
				return member;
			}
			throw(ferr::invalid_member(query, name));
		}
		
		auto access_assign(LeObject query, LeObject new_val) -> LeObject override
		{
			if (query->type != Type::String)
				throw(ferr::invalid_access(type_name(), query->type_name()));

			auto& str = getters::get_string_ref(query, "access assign");
			members[str] = query; /* Create if we dont have it, else assign it */
			return query;
		}

		/* Can be null */
		auto has_member(Symbol str) -> LeObject
		{
			if (auto res = members.find(str);
				res != members.end())
			{
				return res->second;
			}
			return nullptr;
		}
	};

	constexpr auto size__class = sizeof(Class);

}
