#include "program.h"
#include "overloaded.h"
#include "parser.h"
#include "tokenizer.h"
#include <boost/outcome/basic_result.hpp>
#include <boost/outcome/policy/fail_to_compile_observers.hpp>
#include <boost/outcome/try.hpp>
#include <variant>

bool lpg::operator==(evaluate_error const &left, evaluate_error const &right)
{
    return (left.type == right.type) && (left.identifier == right.identifier);
}

std::ostream &lpg::operator<<(std::ostream &out, evaluate_error const &error)
{
    return out << static_cast<int>(error.type) << " " << error.identifier;
}

namespace lpg
{
    sequence compile(std::string_view source, std::function<void(parse_error)> on_error)
    {
        scanner tokens{source};
        parser parser(tokens, on_error);
        return parser.parse_sequence(false);
    }

    namespace
    {
        using evaluate_result = boost::outcome_v2::basic_result<value, evaluate_error,
                                                                boost::outcome_v2::policy::fail_to_compile_observers>;

        [[nodiscard]] evaluate_result evaluate(expression const &to_evaluate, local_variable_map &locals,
                                               std::string &output);

        [[nodiscard]] evaluate_result evaluate_call(call const &function, local_variable_map &locals,
                                                    std::string &output)
        {
            BOOST_OUTCOME_TRY(value const callee, evaluate(*function.callee, locals, output));
            BOOST_OUTCOME_TRY(value const argument, evaluate(*function.argument, locals, output));

            return std::visit(
                overloaded{
                    [&output](builtin_functions const callee, std::string const &argument) -> evaluate_result {
                        switch (callee)
                        {
                        case builtin_functions::print:
                            output += argument;
                            break;
                        }
                        return nullptr;
                    },
                    [](auto const &, auto const &) -> evaluate_result {
                        return evaluate_error{evaluate_error_type::not_callable};
                    },
                },
                callee, argument);
        }

        [[nodiscard]] std::optional<evaluate_error> evaluate_sequence(sequence const &to_evaluate,
                                                                      local_variable_map &locals, std::string &output)
        {
            for (expression const &element : to_evaluate.elements)
            {
                evaluate_result const result = evaluate(element, locals, output);
                if (result.has_error())
                {
                    return result.assume_error();
                }
            }
            return std::nullopt;
        }

        evaluate_result evaluate(expression const &to_evaluate, local_variable_map &locals, std::string &output)
        {
            return std::visit(
                overloaded{
                    [](string_literal constant) -> evaluate_result {
                        return value{std::string{constant.inner_content}};
                    },
                    [&locals](identifier name) -> evaluate_result {
                        if (name.content == "print")
                        {
                            return builtin_functions::print;
                        }
                        auto const found = locals.find(std::string(name.content));
                        if (found == locals.end())
                        {
                            return evaluate_error{evaluate_error_type::unknown_identifier, std::string(name.content)};
                        }
                        return found->second;
                    },
                    [&output, &locals](call const &function) -> evaluate_result {
                        return evaluate_call(function, locals, output);
                    },
                    [&output, &locals](sequence const &list) -> evaluate_result {
                        std::optional<evaluate_error> const error = evaluate_sequence(list, locals, output);
                        if (error)
                        {
                            return *error;
                        }
                        return value(nullptr);
                    },
                    [&output, &locals](declaration const &declaration_) -> evaluate_result {
                        output += "Declaring ";
                        output += declaration_.name.content;
                        output += "\n";
                        BOOST_OUTCOME_TRY(value initial_value, evaluate(*declaration_.initializer, locals, output));
                        bool const inserted =
                            locals.insert(std::make_pair(declaration_.name.content, std::move(initial_value))).second;
                        if (!inserted)
                        {
                            throw std::invalid_argument("Redeclaration of " + std::string(declaration_.name.content));
                        }
                        return value(nullptr);
                    }},
                to_evaluate.value);
        }
    } // namespace
} // namespace lpg

lpg::run_result lpg::run(std::string_view source, std::function<void(parse_error)> on_error)
{
    assert(on_error);
    sequence program = compile(source, move(on_error));
    local_variable_map locals;
    std::string output;
    std::optional<evaluate_error> const error = evaluate_sequence(program, locals, output);
    if (error)
    {
        return *error;
    }
    return std::move(output);
}
