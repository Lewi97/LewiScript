#pragma once

#include "Builtin.h"
#include <functional>
#include "GlobalState.h"

namespace le
{
    /*
    * Class for simple cpp functions that share the FFI interface
    */
    struct ImportedFunction :
        public RuntimeValue
    {
        explicit ImportedFunction(FFI_FUNC f, String string = "") 
            : func(f)
            , name(string)
        {
            if (f == nullptr)
                throw(ferr::make_exception("Tried to create an imported function from a nullptr"));

            type = Type::Custom;
        }

        FFI_FUNC func{};
        String name{};

        auto type_name() -> String override
        {
            return "ImportedFunction";
        }

        auto make_string() -> String override
        {
            return name;
        }

        /*
        * func is assumed to be not null
        */
        auto call(std::span<LeObject>& args, class VirtualMachine&) -> LeObject override
        { 
            return func(args, *global::mem);
        }
    };

}
