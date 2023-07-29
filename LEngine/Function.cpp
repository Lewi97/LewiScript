#include "Function.h"
#include "VM.h"

namespace le
{
	auto CompiledFunction::call(std::span<LeObject>& args, VirtualMachine& vm) -> LeObject
	{
		return vm.run(function_frame, args);
	}
}