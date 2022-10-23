#include "interpreter.h"
#include "overloaded.h"
#include "parser.h"
#include "tokenizer.h"
#include <boost/outcome/basic_result.hpp>
#include <boost/outcome/policy/fail_to_compile_observers.hpp>
#include <boost/outcome/try.hpp>
#include <variant>

namespace lpg
{
    bool operator==(evaluate_error const &left, evaluate_error const &right)
    {
        return (left.type == right.type) && (left.identifier == right.identifier);
    }

    std::ostream &operator<<(std::ostream &out, evaluate_error const &error)
    {
        return out << static_cast<int>(error.type) << " " << error.identifier;
    }

    namespace
    {
        using evaluate_result = boost::outcome_v2::basic_result<value, evaluate_error,
                                                                boost::outcome_v2::policy::fail_to_compile_observers>;

        [[nodiscard]] evaluate_result evaluate(syntax::expression const &to_evaluate, local_variable_map &locals,
                                               std::string &output);

        [[nodiscard]] evaluate_result evaluate_call(syntax::call const &function, local_variable_map &locals,
                                                    std::string &output)
        {
            BOOST_OUTCOME_TRY(value const callee, evaluate(*function.callee, locals, output));
            BOOST_OUTCOME_TRY(value const argument, evaluate(*function.argument, locals, output));

            return std::visit(overloaded{
                                  [&output](semantics::builtin_functions const callee,
                                            std::string const &argument) -> evaluate_result {
                                      switch (callee)
                                      {
                                      case semantics::builtin_functions::print:
                                          output += argument;
                                          break;
                                      }
                                      return void_{};
                                  },
                                  [](auto const &, auto const &) -> evaluate_result {
                                      return evaluate_error{evaluate_error_type::not_callable};
                                  },
                              },
                              callee, argument);
        }

        [[nodiscard]] evaluate_result evaluate_sequence(syntax::sequence const &to_evaluate, local_variable_map &locals,
                                                        std::string &output)
        {
            std::optional<value> sequence_result;
            for (syntax::expression const &element : to_evaluate.elements)
            {
                evaluate_result result = evaluate(element, locals, output);
                if (result.has_error())
                {
                    return result.assume_error();
                }
                sequence_result = std::move(result.assume_value());
            }
            if (sequence_result)
            {
                return std::move(*sequence_result);
            }
            return value(void_{});
        }

        evaluate_result evaluate(syntax::expression const &to_evaluate, local_variable_map &locals, std::string &output)
        {
            return std::visit(
                overloaded{
                    [](syntax::string_literal constant) -> evaluate_result {
                        return value{std::string{constant.inner_content}};
                    },
                    [&locals](syntax::identifier name) -> evaluate_result {
                        if (name.content == "print")
                        {
                            return semantics::builtin_functions::print;
                        }
                        auto const found = locals.find(std::string(name.content));
                        if (found == locals.end())
                        {
                            return evaluate_error{evaluate_error_type::unknown_identifier, std::string(name.content)};
                        }
                        return found->second;
                    },
                    [&output, &locals](syntax::call const &function) -> evaluate_result {
                        return evaluate_call(function, locals, output);
                    },
                    [&output, &locals](syntax::sequence const &list) -> evaluate_result {
                        return evaluate_sequence(list, locals, output);
                    },
                    [&output, &locals](syntax::declaration const &declaration_) -> evaluate_result {
                        BOOST_OUTCOME_TRY(value initial_value, evaluate(*declaration_.initializer, locals, output));
                        bool const inserted =
                            locals.insert(std::make_pair(declaration_.name.content, std::move(initial_value))).second;
                        if (!inserted)
                        {
                            return evaluate_error{
                                evaluate_error_type::redeclaration, std::string(declaration_.name.content)};
                        }
                        return value(void_{});
                    }},
                to_evaluate.value);
        }
    } // namespace

    run_result run(std::string_view source, std::function<void(syntax::parse_error)> on_error)
    {
        assert(on_error);
        syntax::sequence program = compile(source, move(on_error));
        local_variable_map locals;
        std::string output;
        evaluate_result const result = evaluate_sequence(program, locals, output);
        if (result.has_error())
        {
            return result.assume_error();
        }
        return std::move(output);
    }
} // namespace lpg
