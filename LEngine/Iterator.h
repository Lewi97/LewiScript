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
		using Owner = std::shared_ptr<_Owner>;

		Iterator(Owner owner_, _Function func) 
			: owner(owner_), function_next(func)
		{ type = Type::Iterator; }

		Owner owner{};
		_Function function_next{};
		auto member_access(LeObject self, LeObject query) -> LeObject override
		{
			using This = std::remove_reference_t<decltype(*this)>;

			auto& str = static_cast<StringValue*>(query.get())->string;
			if (str == "next")
			{
				return 
					MemberFunction<This>(self, [](This& iter, std::span<LeObject>&, struct VirtualMachine&)
					{
						return iter.function_next(*iter.owner);
					});
			}
			throw(ferr::invalid_member(str));
			return LeObject{};
		}
	};

}

