#pragma once

#include "Builtin.h"
#include "Function.h"
#include "getters.h"
#include "MemberFunctions.h"

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
		std::unordered_map<String, LeObject> members{};
		
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
			throw(ferr::invalid_member(query, query));
		}

		auto make_member(LeObject self, const String& member, LeObject assign) -> void
		{
			if (assign->type == Type::Function)
				assign = global::mem->emplace<BuiltinMemberFunction>(self, static_cast<CompiledFunction*>(assign.get())->function_frame);

			members.insert(std::pair{ member, assign });
		}
		
		auto access_assign(LeObject query, LeObject new_val) -> LeObject override
		{
			if (query->type != Type::String)
				throw(ferr::invalid_access(type_name(), query->type_name()));
			
			auto& str = getters::get_string_ref(query, "access assign");
			if (not has_member(str))
				throw(ferr::invalid_member(str));

			return members.at(str) = new_val;
		}

		/* Can be null */
		auto has_member(const String& str) -> LeObject
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
