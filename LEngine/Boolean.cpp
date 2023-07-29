#include "Boolean.h"
#include "Number.h"

namespace le
{
	auto Boolean::apply_operation(Token::Type op, LeObject other) -> LeObject
	{
		auto number = NumberValue(static_cast<Number>(val));
		return number.apply_operation(op, other);
	}
}