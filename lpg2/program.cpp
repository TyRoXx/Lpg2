#include "program.h"
#include "overloaded.h"
#include "parser.h"
#include "tokenizer.h"
#include <cassert>
#include <variant>
#include <vector>

namespace lpg
{
    sequence compile(std::string_view source)
    {
        scanner tokens{source};
        return parse_sequence(tokens);
    }
}

lpg::value lpg::evaluate_call(call const &function, std::string &output)
{
    value const callee = evaluate(*function.callee, output);
    value const argument = evaluate(*function.argument, output);

    std::visit(
        overloaded{
            [&output](builtin_functions const callee, std::string const &argument) {
                switch (callee)
                {
                case builtin_functions::print:
                    output += argument;
                    break;
                }
            },
            [](auto const &, auto const &) {
                throw std::invalid_argument("Wrong call arguments. Or function is not callable");
            },
        },
        callee, argument);
    return nullptr;
}

void lpg::evaluate_sequence(sequence const &to_evaluate, std::string &output)
{
    for (expression const &element : to_evaluate.elements)
    {
        evaluate(element, output);
    }
}

lpg::value lpg::evaluate(expression const &to_evaluate, std::string &output)
{
    return std::visit(
        overloaded{[](string_literal constant) -> value { return value{std::string{constant.inner_content}}; },
                   [](identifier name) -> value {
                       if (name.content == "print")
                       {
                           return builtin_functions::print;
                       }
                       throw std::invalid_argument("Unknown function");
                   },
                   [&output](call const &function) -> value { return evaluate_call(function, output); },
                   [&output](sequence const &list) -> value {
                       evaluate_sequence(list, output);
                       return nullptr;
                   }},
        to_evaluate.value);
}

lpg::run_result lpg::run(std::string_view source)
{
    std::optional<sequence> program = compile(source);
    if (!program)
    {
        return run_result{std::nullopt};
    }
    std::string output;
    evaluate_sequence(*program, output);
    return run_result{std::move(output)};
}
