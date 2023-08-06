#include "MemberFunctions.h"
#include "VM.h"
#include "ByteCode.h"

auto le::BuiltinMemberFunction::call(std::span<LeObject>& args, VirtualMachine& vm) -> LeObject
{
	return vm.run(_function, args, _this);
}

auto le::BuiltinMemberFunction::type_name() -> String
{
	return std::format("{}::{}", _this->type_name(), _function.name);
}
