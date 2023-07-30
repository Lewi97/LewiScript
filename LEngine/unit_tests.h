#pragma once

#include "common.h"
#include "Parser.h"
#include "Lexer.h"
#include "Interpreter.h"
#include "Runner.h"

#define LE_UNIT_TEST_BEGIN(name, expect) \
	inline auto unit_test_##name() -> void \
	{   const auto test_name = #name; const auto expected = expect; \
		const auto source = 

#define LE_UNIT_TEST_END() run(source, test_name, expected); } 
#define LE_REGISTER_UNIT_TEST(name) unit_test_##name,

namespace le::unit_test
{
	auto run(StringView source, StringView test_name, String expected) -> void;

	LE_UNIT_TEST_BEGIN(variable_assignment, "50")
		R"(
var a = 50
a
)";
	LE_UNIT_TEST_END()


		LE_UNIT_TEST_BEGIN(relational_operators, "[True, True, True, True, True, True, True]")
		R"(
[ 5 > 0, 5 >= 5, 5 >= 4, 5 < 6, 5 <= 5, 5 == 5, 5 != 6 ]
)";
		LE_UNIT_TEST_END();


		LE_UNIT_TEST_BEGIN(if_statements, "Yes i am")
			R"(
	var a = "Am i the best?"
	if a[0] == "A":
		a = "Yes i am"
	else:
		a = "No i am not"
	end
	a
)";
		LE_UNIT_TEST_END();

		LE_UNIT_TEST_BEGIN(if_statements_alternative, "No i am not")
			R"(
	var a = "Am i the best?"
	if a[0] != "A":
		a = "Yes i am"
	else:
		a = "No i am not"
	end
	a
)";
		LE_UNIT_TEST_END();

		LE_UNIT_TEST_BEGIN(if_statements_elif, "Equality operators!")
			R"(
	var a = "Am i the best?"
	if a[0] != "A":
		a = "Yes i am"
	elif a == "Am i the best?":
		a = "Equality operators!"
	else:
		a = "No i am not"
	end
	a
)";
		LE_UNIT_TEST_END();

		LE_UNIT_TEST_BEGIN(nested_if_statements, "5")
			R"(
	var a = "Lewi"
	if a[0] == "L":
		if a[1] == "e":
			if a[2] == "w":
				if a[3] == "i":
					a = 5
				end
			end
		end
	end
	a
)";
		LE_UNIT_TEST_END();

		LE_UNIT_TEST_BEGIN(while_loop, "10")
			R"(
	var iteration = 0
	while iteration < 10:
		iteration = iteration + 1
	end
	iteration
)";
		LE_UNIT_TEST_END();

		LE_UNIT_TEST_BEGIN(nested_while_loop, "20")
			R"(
	var iteration = 0
	var iteration2 = 0
	while iteration < 10:
		while iteration2 < 10:
			iteration2 = iteration2 + 1
		end
		iteration = iteration + 1
	end
	iteration + iteration2
)";
		LE_UNIT_TEST_END();
		
		LE_UNIT_TEST_BEGIN(scope_test, "53")
			R"(
	var arg = 13
	var arg1 = 15
	fn func(arg1):
		var arg = arg1 + 5
		arg + 10
	end
	
	arg + func(10) + arg1
)";
		LE_UNIT_TEST_END();
		
		LE_UNIT_TEST_BEGIN(fibonacci_test, "13")
			R"(
	fn fibo(n):
		if n > 1: 
			return fibo(n - 1) + fibo(n - 2) end
		return n 
	end

	fibo(7)
)";
		LE_UNIT_TEST_END();

		LE_UNIT_TEST_BEGIN(return_test, "7")
			R"(
	fn func(n):
		if n > 0:
			return n 
		end
		return n
	end

	func(7)
)";
		LE_UNIT_TEST_END();

		LE_UNIT_TEST_BEGIN(nested_while_break_test, "15")
			R"(
	var x = 0
	var i = 5
	var j = 5

	while i > 0:
		i = i - 1
		while 1:
			j = j - 1
			x = x + 1
			if x > 5:
				break
			end
		end
		x = x + 1
	end
	x
)";
		LE_UNIT_TEST_END();

		LE_UNIT_TEST_BEGIN(static_var_test, "13")
			R"(
	fn function(arg):
		function.static_var + arg
	end
	function.static_var = 10
	function(3)
)";
		LE_UNIT_TEST_END();

	static inline auto _unit_tests = std::vector<void(*)()>
	{
		LE_REGISTER_UNIT_TEST(variable_assignment)
		LE_REGISTER_UNIT_TEST(relational_operators)
		LE_REGISTER_UNIT_TEST(if_statements)
		LE_REGISTER_UNIT_TEST(if_statements_alternative)
		LE_REGISTER_UNIT_TEST(if_statements_elif)
		LE_REGISTER_UNIT_TEST(nested_if_statements)
		LE_REGISTER_UNIT_TEST(while_loop)
		LE_REGISTER_UNIT_TEST(nested_while_loop)
		LE_REGISTER_UNIT_TEST(scope_test)
		LE_REGISTER_UNIT_TEST(return_test)
		LE_REGISTER_UNIT_TEST(nested_while_break_test)
		LE_REGISTER_UNIT_TEST(fibonacci_test)
		LE_REGISTER_UNIT_TEST(static_var_test)
	};
	
	inline auto run(StringView source, StringView test_name, String expected) -> void
	{
		/* abc\0\0 == abc\0 */
		const auto cmp_till_null = [](const String& a, const String& b) -> bool
		{
			/* find end, we expect it to always have one */
			auto a_end = std::find(a.data(), a.data() + a.size(), '\0');
			auto b_end = std::find(b.data(), b.data() + b.size(), '\0');

			return StringView(a.data(), a_end) == StringView(b.data(), b_end);
		};

		// le::print_bytecode(source, "__unit_tests__");
		auto res = le::run_with_vm(source, "__unit_tests__");
		
		if (res)
		{
			if (cmp_till_null(res->make_string(), expected))
			{
				std::cout << "[SUCCESS]";
			}
			else
			{
				std::cout << std::format("[FAILED] got '{}' expected '{}'", res->make_string(), expected);
			}
		}
		else
		{
			std::cout << "[FAILED] got nullptr ";
		}
		
		std::cout << " at test '" << test_name << "'\n";
	}

	inline auto start() -> void
	{
		for (auto f : _unit_tests)
			f();
	}
}

