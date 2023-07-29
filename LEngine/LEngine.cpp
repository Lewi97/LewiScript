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

/*
* TODO:
* member functions for C++ types
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
* import le scripts
* TEST:
* is it faster to also allocate ast nodes into the memory manager
* is it faster to make type of an object a virtual function, a type erased object in memory or to leave it as is
* is it faster to use a memory arena that holds all the pools over dynamically allocating all pools
* is it faster to use a custom allocator that allocates everything into the memory manager for objects like Array that dynamically hold pointers
* is it faster to use a specialized hash map to identify keywords
* is it faster to internally change identifiers from string to ints to make variable lookups faster
* * fix 'add(5,5) add' from properly parsing, 
    need to enforce a new token of type new line to be expected at any correctly parsed expression|statement 
*/

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
* TEST:
* if expr: expr end
* if expr: expr elif expr: expr else: expr
* if expr: expr elif expr: expr elif expr: expr else: expr
* if expr else expr
* 
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

