#pragma once

#include "common.h"
#include "Builtin.h"

namespace le
{
    /*
    * Base class for modules
    */
    struct Module :
        public RuntimeValue
    {
        Module() { type = Type::Module; }

        /*
        * import some_module
        */
        virtual auto load(String module_name) -> LeObject = 0;       
    };
}
