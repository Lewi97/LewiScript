#pragma once

#include "Builtin.h"
#include <concepts>
#include "RuntimeStrings.h"
#include "MemberFunctions.h"

namespace le
{

	namespace detail
	{
		template<typename _Function, typename _Owner>
		concept iterate_function = std::invocable<_Function, _Owner&> and
			requires(_Function func, _Owner& owner)
		{
			{ func(owner) } -> std::convertible_to<LeObject>;
		};
	}

	template<
		std::derived_from<RuntimeValue> _Owner ,
		detail::iterate_function<_Owner> _Function
	>
	struct Iterator
		: RuntimeValue
	{
		Iterator(LeObject owner_, _Function func) 
			: owner(owner_), function_next(func)
		{ type = Type::Iterator; }

		LeObject owner{};
		_Function function_next{};
		
		auto type_name() -> LeObject override
		{
			return strings::make_string("Iterator");
		}

		auto member_access(LeObject self, const String& member) -> LeObject override
		{
			using This = std::remove_reference_t<decltype(*this)>;

			if (member == "next")
			{
				return 
					global::mem->emplace<MemberFunction<This>>(self, [](This& iter, std::span<LeObject>&, struct VirtualMachine&)
					{
						return iter.function_next(*static_cast<_Owner*>(iter.owner.get()));
					});
			}
			throw(ferr::invalid_member(member));
			return LeObject{};
		}

		auto call(std::span<LeObject>&, class VirtualMachine&) -> LeObject override
		{
			return function_next(*static_cast<_Owner*>(owner.get()));
		}
	};

}

