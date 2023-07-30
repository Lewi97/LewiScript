#pragma once

#include <concepts>

#include "Builtin.h"

namespace le
{
    template<typename _This>
    struct MemberFunction
        : RuntimeValue
    {
        using Function = LeObject(*)(_This&, std::span<LeObject>&, struct VirtualMachine&);

        MemberFunction(LeObject object, Function func)
            : _this(object)
            , _function(func)
        {}

        auto call(std::span<LeObject>& args, struct VirtualMachine& vm) -> LeObject override
        {
            return _function(*static_cast<_This*>(_this.get()), args, vm);
        }
    private:
        LeObject _this{};
        Function _function{};
    };
}

