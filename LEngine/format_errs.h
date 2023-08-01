#pragma once

#include <format>
#include "common.h"
#include "Token.h"

namespace le::ferr
{

	inline auto make_exception(String message) -> Exception
	{
		return Exception(message.c_str());
	}

	inline auto make_exception(const char* message) -> Exception
	{
		return Exception(message);
	}

	inline auto too_many_arguments(size_t args, size_t expected, String callable_name) -> Exception
	{
		return make_exception(std::format("Called {} with {} arguments expected {}", callable_name, args, expected));
	}

	inline auto unrecognized_character(char c) -> Exception
	{
		return make_exception(std::format("Unrecognized character '{}' detected", c));
	}

	inline auto invalid_member(String member) -> Exception
	{
		return make_exception(std::format("Invalid member '{}'", member));
	}

	inline auto failed_assignment(String left, String right) -> Exception
	{
		return make_exception(
			std::format("Failed to assign {} to {}"
				, right, left)
		);
	}
	
	inline auto unexpected_token(const Token& got) -> Exception
	{
		return make_exception(
			std::format("Unexpected token {} at line {} column {} raw {}"
			, to_string(got.type)
			, got.line
			, got.column
			, got.raw)
			);
	}

	inline auto unexpected_expression(String expected, String got) -> Exception
	{
		return make_exception(
			std::format("Expected expression {} got {} instead"
				, expected
				, got)
		);
	}

	inline auto index_out_of_range(size_t idx) -> Exception
	{
		return make_exception(
			std::format("Index {} out of range", idx)
				);
	}

	inline auto variable_not_declared(StringView name) -> Exception
	{
		return make_exception(
			std::format("Variable {} has not been declared", name)
		);
	}

	inline auto variable_already_declared(StringView name) -> Exception
	{
		return make_exception(
			std::format("Variable {} already exists", name)
		);
	}

	inline auto bad_overload(Token::Type op, String target) -> Exception
	{
		return make_exception(
			std::format("Bad overload {} on {}", to_string(op), target)
		);
	}

	inline auto unexpected_token(Token::Type expected, const Token& got) -> Exception
	{
		return make_exception(
			std::format("Expected token {} got ({}: {}) at line {} column {}"
			, to_string(expected)
			, to_string(got.type)
			, got.raw
			, got.line
			, got.column)
			);
	}

	inline auto empty_reference(String action) -> Exception
	{
		return make_exception(std::format("Tried to \"{}\" on an empty reference. References should never be empty", action));
	}

	inline auto not_implemented() { return make_exception("This feature has not yet been implemented"); }

	inline auto invalid_operation(Token::Type operation, String left, String right) -> Exception
	{
		return make_exception(
			std::format("Invalid operation {} between {} and {}"
				, to_string(operation)
				, left, right)
		);
	}

	inline auto invalid_operation(Token::Type operation, String right) -> Exception
	{
		return make_exception(
			std::format("Invalid unary operation {} on {}"
				, to_string(operation)
				, right)
		);
	}

	inline auto invalid_access(String accessee, String accessor) -> Exception
	{
		return make_exception(
			std::format("Invalid access on {} with {}"
				, accessee
				, accessor)
		);
	}
}
