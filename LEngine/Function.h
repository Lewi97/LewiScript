#pragma once

#include "common.h"
#include "Builtin.h"
#include "String.h"
#include "RuntimeStrings.h"
#include "ByteCode.h"

#include <unordered_map>

/*
* The builtin function type.
* 
* Usage:
* Functions are by nature lambda's and have two ways of being declared. An immutable and mutable way.
* NOTE: The immutable way is not yet implemented
* 
* fn 'identifier'('args' ...):
*	'block'
* end
* 
* is equivalent to:
* 
* var 'identifier' = fn('args' ...): 
*	[block]
* end
* 
* Static variables:
* Builtin functions also allow for static variables, the current way might be a bit unintuitive:
* 
* fn function(arg1, arg2):
*   function.my_static_var + 10
* end
* function.my_static_var = 50
* 
* Static vars are declared and accessed as if they are a member variable of the function.
* The can be declared inside or outside of functions. Might restrict it to outside declaration later.
*/

namespace le
{
	struct Function : RuntimeValue
	{
		Function() { type = Type::Function; }

		auto make_string() -> String override { return "Function"; }

		struct BlockStatement* body{ nullptr };
		std::vector<String> args{};
		std::unordered_map<StringView, LeObject> static_vars{};

		auto get_static_var(LeObject& index) -> LeObject&
		{
			if (index->type != Type::String)
			{
				throw(ferr::invalid_access(to_string(type), to_string(index->type)));
			}

			auto string = static_cast<StringValue*>(index.get());
			return static_vars[string->string];
		};

		auto access(LeObject index) -> LeObject override
		{
			return get_static_var(index);
		}

		auto access_assign(LeObject index, LeObject new_val) -> LeObject override
		{
			get_static_var(index) = new_val;
			return new_val;
		}
	};

	/*
	* To be used by the virtual machine
	*/
	struct CompiledFunction : RuntimeValue
	{
		explicit CompiledFunction(Frame frame)
			: function_frame(frame)
		{ type = Type::Function; }

		
		Frame function_frame;

		auto type_name() -> String override
		{
			return "Function";
		}

		auto make_string() -> String override 
		{ 
			return "Function " + function_frame.name;
		}

		auto call(std::span<LeObject>& args, class VirtualMachine& vm) -> LeObject override;
	};
}

