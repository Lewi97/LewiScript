#include "getters.h"
#include "Number.h"
#include "format_errs.h"

namespace le::getters
{
    template<typename _T>
    static auto as(const auto& ptr) -> _T&
    {
        return *static_cast<_T*>(ptr.get());
    }
}

auto le::getters::get_number(const LeObject& obj, const char* context) -> Number
{
    if (obj->type != RuntimeValue::Type::NumericLiteral)
        throw(ferr::invalid_conversion(obj->type_name(), "Number", context));

    return as<NumberValue>(obj).number;
}
