#include "program.h"
#include "overloaded.h"
#include "parser.h"
#include "tokenizer.h"
#include <variant>

namespace lpg
{
    sequence compile(std::string_view source, std::function<void(parse_error)> on_error)
    {
        scanner tokens{source};
        parser parser(tokens, on_error);
        return parser.parse_sequence();
    }

    namespace
    {
        value evaluate(expression const &to_evaluate, local_variable_map &locals, std::string &output);

        value evaluate_call(call const &function, local_variable_map &locals, std::string &output)
        {
            value const callee = evaluate(*function.callee, locals, output);
            value const argument = evaluate(*function.argument, locals, output);

            std::visit(overloaded{
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

        void evaluate_sequence(sequence const &to_evaluate, local_variable_map &locals, std::string &output)
        {
            for (expression const &element : to_evaluate.elements)
            {
                evaluate(element, locals, output);
            }
        }

        value evaluate(expression const &to_evaluate, local_variable_map &locals, std::string &output)
        {
            return std::visit(
                overloaded{
                    [](string_literal constant) -> value { return value{std::string{constant.inner_content}}; },
                    [&locals](identifier name) -> value {
                        if (name.content == "print")
                        {
                            return builtin_functions::print;
                        }
                        auto const found = locals.find(std::string(name.content));
                        if (found == locals.end())
                        {
                            throw std::invalid_argument("Unknown identifier " + std::string(name.content));
                        }
                        return found->second;
                    },
                    [&output, &locals](call const &function) -> value {
                        return evaluate_call(function, locals, output);
                    },
                    [&output, &locals](sequence const &list) -> value {
                        evaluate_sequence(list, locals, output);
                        return nullptr;
                    },
                    [&output, &locals](declaration const &declaration_) -> value {
                        output += "Declaring ";
                        output += declaration_.name.content;
                        output += "\n";
                        value initial_value = evaluate(*declaration_.initializer, locals, output);
                        bool const inserted =
                            locals.insert(std::make_pair(declaration_.name.content, std::move(initial_value))).second;
                        if (!inserted)
                        {
                            throw std::invalid_argument("Redeclaration of " + std::string(declaration_.name.content));
                        }
                        return nullptr;
                    }},
                to_evaluate.value);
        }
    } // namespace
} // namespace lpg

void on_error(lpg::parse_error result)
{
}

lpg::run_result lpg::run(std::string_view source)
{
    sequence program = compile(source, on_error);
    local_variable_map locals;
    std::string output;
    evaluate_sequence(program, locals, output);
    return run_result{std::move(output)};
}
