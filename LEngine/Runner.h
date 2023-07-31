#pragma once

#include "common.h"
#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Compiler.h"
#include "VM.h"

#include <sstream>
#include <fstream>

namespace le
{
    inline auto parse(std::string_view source, std::string_view fname) -> std::optional<AST>
    {
        auto lexer = Lexer();
        auto parser = Parser();

        lexer.tokenize(source);
        auto ast = parser.parse(lexer);

        if (not ast.error.empty())
        {
            std::cout << "[PARSE ERROR] " << ast.error << " in file '" << fname << "'\n";
            return {};
        }
        return ast;
    }

    namespace detail
    {
        template<typename _T, typename _Code>
        concept Evaluator = requires(_T t, _Code code) { t.run(code); };
    }

    template<typename _Code>
    inline auto evaluate(detail::Evaluator<_Code> auto& runner, _Code& ast) -> std::optional<LeObject>
    {
        std::cout << "\n[RUNTIME OUTPUT]\n";
        auto output = runner.run(ast);
        if (std::holds_alternative<String>(output))
        {
            std::cout << "[RUNTIME ERROR] " << std::get<String>(output) << '\n';
            return std::nullopt;
        }
        else
        {
            return std::get<LeObject>(output);
        }
    }

    template<typename _Evaluator, typename _Code>
    inline auto evaluate_with(_Code code) -> std::optional<LeObject>
    {
        auto evaluator = _Evaluator();
        return evaluate(evaluator, code);
    }

    inline auto run(std::string_view source, std::string_view fname) -> LeObject
    {
        return
            parse(source, fname)
            .and_then(evaluate_with<Interpreter, AST>)
            .value_or(nullptr);
    }

    inline auto compile(AST ast) -> std::optional<Code>
    {
        auto compiler = Compiler();
        auto code = compiler.emit_bytecode(ast);
        if (std::holds_alternative<String>(code))
        {
            std::cout << "[COMPILE ERROR] " << std::get<String>(code) << '\n';
            return std::nullopt;
        }
        else
        {
            return std::get<Code>(code);
        }
    }

    inline auto run_with_vm(std::string_view source, std::string_view fname) -> LeObject
    {
        return 
            parse(source, fname)
            .and_then(compile)
            .and_then(evaluate_with<VirtualMachine, Code>)
            .value_or(nullptr);
    }

    inline auto print_bytecode(std::string_view source, std::string_view fname) -> void
    {
        auto code = 
            parse(source, fname)
            .and_then(compile);
        
        if (code)
            std::cout << to_string(code.value());
    }

    inline auto run_file(std::string fname) -> void
    {
        auto fstream = std::ifstream(fname);
        auto buffer = std::stringstream{};
        buffer << fstream.rdbuf();
        auto str = buffer.str();

        run(str, fname);
    }
}

