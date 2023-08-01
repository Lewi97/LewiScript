#pragma once

#include <concepts>

#include "Builtin.h"

namespace le
{
    template<typename _This>
    struct MemberFunction
        : RuntimeValue
    {
        using Function = LeObject(*)(_This&, std::span<LeObject>&, class VirtualMachine&);

        MemberFunction(LeObject object, Function func)
            : _this(object)
            , _function(func)
        {}

        auto call(std::span<LeObject>& args, class VirtualMachine& vm) -> LeObject override
        {
            return _function(*static_cast<_This*>(_this.get()), args, vm);
        }

        auto type_name() -> String override
        {
            return "MemberFunction";
        }
    private:
        LeObject _this{};
        Function _function{};
    };
}

