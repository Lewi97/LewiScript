#include "ByteCode.h"
#include "Function.h"

namespace le
{
	auto to_string(const Code& code) -> String
	{
		auto string = to_string(code.code, code);
		string += "\nGlobals:\n";
		for (auto count{ 0ull }; const auto & global : code.globals)
		{
			string += std::format("{}: ", count++);
			switch (global->type)
			{
			case RuntimeValue::Type::Function:
			{
				const auto& function = static_cast<const CompiledFunction*>(global.get())->function_frame;
				string += std::format("\t(Function '{}')\n{}\n", function.name, to_string(function, code));
				break;
			}
			case RuntimeValue::Type::String:
				string += std::format("String('{}')\n", global->make_string());
				break;
			default:
				string += std::format("(Type: '{}')", global->make_string());
			}
		}
		return string;
	}
}