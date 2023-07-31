#pragma once

#include "Builtin.h"
#include <concepts>
#include "String.h"
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
		auto member_access(LeObject self, LeObject query) -> LeObject override
		{
			using This = std::remove_reference_t<decltype(*this)>;

			auto& str = static_cast<StringValue*>(query.get())->string;
			if (str == "next")
			{
				return 
					global::mem->emplace<MemberFunction<This>>(self, [](This& iter, std::span<LeObject>&, struct VirtualMachine&)
					{
						return iter.function_next(*static_cast<_Owner*>(iter.owner.get()));
					});
			}
			throw(ferr::invalid_member(str));
			return LeObject{};
		}

		auto call(std::span<LeObject>&, class VirtualMachine&) -> LeObject override
		{
			return function_next(*static_cast<_Owner*>(owner.get()));
		}
	};

}

