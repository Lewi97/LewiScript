#include "TypeFactory.h"
#include "String.h"
#include "GlobalState.h"
#include "Number.h"

auto le::make::make_string(String string) -> LeObject
{
    return global::mem->emplace<StringValue>(string);
}

auto le::make::make_number(Number number) -> LeObject
{
    return global::mem->emplace<NumberValue>(number);
}

auto le::make::make_null() -> LeObject
{
    return global::null;
}
