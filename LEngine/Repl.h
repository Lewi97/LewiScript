#pragma once

#include "Interpreter.h"

namespace le
{
	class Repl
	{
    protected:
        Lexer lexer{};
        Parser parser{};
        Interpreter interpreter{};
	public:
		auto run() -> void
		{
            for (auto current = String{}; current != "exit"; )
            {
                std::cout << "> ";
                std::getline(std::cin, current);
                if (current == "exit") return;
                if (current.empty()) continue;
                lexer.tokenize(current);

                if (auto loc = current.find_first_of(' '); loc != String::npos)
                {
                    const auto command = std::string_view(current.data(), current.data() + loc);
                    if (command == "debug_tokens")
                    {
                        auto view = StringView(current);
                        view.remove_prefix(loc);
                        lexer.tokenize(view);
                        while (not lexer.empty())
                        {
                            std::cout << lexer.advance();
                        }
                        std::cout << '\n';
                        continue;
                    }
                    else if (command == "debug_ast")
                    {
                        auto view = StringView(current);
                        view.remove_prefix(loc);
                        lexer.tokenize(view);
                        auto ast = parser.parse(lexer);
                        if (not ast.error.empty())
                        {
                            std::cout << ast.error << '\n';
                        }
                        else
                        {
                            for (const auto& expr : ast.body)
                            {
                                std::cout << to_string(expr.get()) << '\n';
                            }
                        }
                        continue;
                    }
                }

                auto ast = parser.parse(lexer);
                if (not ast.error.empty())
                {
                    std::cout << ast.error << '\n';
                    continue;
                }
            
                auto err = interpreter.run(ast);
                if (std::holds_alternative<le::String>(err))
                {
                    std::cout << std::get<le::String>(err) << '\n';
                    continue;
                }
                
                std::cout << std::get<le::LeObject>(err)->make_string() << '\n';
            }
		}
	};
}

