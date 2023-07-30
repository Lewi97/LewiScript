#pragma once

#include "common.h"
#include "Builtin.h"
#include "Number.h"
#include "String.h"
#include "MemberFunctions.h"

#include <vector>

namespace le
{
	struct Array : RuntimeValue
	{
		Array() = default;
		explicit Array(u64 size)
		{
			data.reserve(size);
		}

		std::vector<LeObject> data{};

		auto make_string() -> String override
		{
			if (data.empty()) return String("[]");

			auto string = String{"["};

			for (auto& elem : data)
			{
				string += elem->make_string();
				string += ", ";
			}

			string.pop_back(); /* remove ',' */
			string.pop_back(); /* remove ' ' */
		
			string += ']';
			return string;
		}

		auto access(LeObject index) -> LeObject override
		{
			auto idx = to_numeric_index(*index);
			return at(idx);
		}

		auto member_access(LeObject self, LeObject query) -> LeObject
		{
			auto& str = static_cast<StringValue*>(query.get())->string;
			if (str == "append")
			{
				return global::mem->emplace<MemberFunction<Array>>(self,
					[](Array& self, std::span<LeObject>& args, struct VirtualMachine&)->LeObject
					{
						self.data.append_range(args);
						return self.data.back();
					}
				);
			}
			if (str == "size")
			{
				return global::mem->emplace<MemberFunction<Array>>(self,
					[](Array& self, std::span<LeObject>& args, struct VirtualMachine&)->LeObject
					{
						return global::mem->emplace<NumberValue>(self.data.size());
					}
				);
			}
			throw(ferr::invalid_member(str));
		}

		auto at(size_t idx) -> LeObject&
		{
			if (idx >= data.size())
				throw(ferr::index_out_of_range(idx));

			return data.at(idx);
		}

		auto access_assign(LeObject index, LeObject rhs) -> LeObject override
		{
			auto idx = to_numeric_index(*index);
			return (at(idx) = rhs);
		}
	};

	inline constexpr auto size__array = sizeof(Array);
}

