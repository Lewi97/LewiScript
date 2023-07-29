#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Repl.h"
#include "unit_tests.h"
#include "Runner.h"

#include "Compiler.h"
#include "VM.h"

#include <fstream>
#include <sstream>

inline auto print_tokens(std::string_view source) -> void
{
    auto lexer = le::Lexer();
    lexer.tokenize(source);
    while (not lexer.empty())
    {
        std::cout << lexer.advance();
    }
}

auto print_ast(std::string_view source) -> void
{
    auto lexer = le::Lexer();
    auto parser = le::Parser();

    lexer.tokenize(source);
    auto ast = parser.parse(lexer);

    if (not ast.error.empty())
    {
        std::cout << ast.error << '\n';
        return;
    }

    for (const auto& expr : ast.body)
    {
        std::cout << to_string(expr.get()) << '\n';
    }
}

auto tree_interpreter_main(int argc, char** argv) -> void
{
    const auto source =
        R"(

print("Hello" + " world!")

)";



    if (argc > 1)
    {
        le::run_file(argv[1]);
    }

    //print_ast(source);

    le::unit_test::start();

    le::run(source, "__main__"); 
}
/*
* TODO:
* Find a better way to handle global objects like scripts and strings.
* Member functions 
* for loops and accompanying iterators
* or | and keywords
* null | true | false literals
* tuples
* tuple for loop assignments eg. for key,val in map:
* add arena's to allocate the pools into
* custom arena allocator
* allocate expressions in arena pools too
* custom classes
* add operators to classes
* named arguments
* var args
* reference self in lambda, this keyword in general
* f strings
* import keyword
*/
auto compiler_main(int, char**) -> void
{
    const auto source =
        R"( 
fn fibo(n):
	if n > 1: 
		return fibo(n - 1) + fibo(n - 2) 
    end
	return n 
end

fibo(7)
)";

    le::print_bytecode(source, "__main__");
    auto result = le::run_with_vm(source, "__main__");
    if (result)
        std::cout << "\n\nResult: " << result->make_string() << '\n';
}

int main(int argc, char** argv)
{
    auto global_memory_manager = le::MemoryManager();
    le::global::mem = &global_memory_manager;

    // tree_interpreter_main(argc, argv);
    compiler_main(argc, argv);
    return 0;
}

