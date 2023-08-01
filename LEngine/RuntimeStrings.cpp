#include "RuntimeStrings.h"
#include "String.h"
#include "GlobalState.h"

auto le::strings::make_string(String string) -> LeObject
{
    return global::mem->emplace<StringValue>(string);
}
