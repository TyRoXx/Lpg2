#include "program.h"
#include "tokenizer.h"
#include <cassert>
#include <variant>
#include <vector>

namespace lpg
{
    struct print
    {
        std::string message;
    };

    struct sequence
    {
        std::vector<print> elements;
    };

    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };

    template <class... Ts>
    overloaded(Ts...)->overloaded<Ts...>;

    token expect_token(scanner &tokens)
    {
        if (tokens.is_at_the_end())
        {
            throw std::invalid_argument("unexpected end of input");
        }
        std::optional<token> head = tokens.pop();
        if (!head)
        {
            throw std::invalid_argument("expected token");
        }
        return std::move(*head);
    }

    void expect_special_character(scanner &tokens, special_character expected)
    {
        std::visit(overloaded{[](identifier const &) { throw std::invalid_argument("unexpected identifier"); },
                              [expected](special_character found) {
                                  if (expected == found)
                                  {
                                      return;
                                  }
                                  throw std::invalid_argument("unexpected special character");
                              },
                              [](string_literal) { throw std::invalid_argument("unexpected string"); }},
                   expect_token(tokens));
    }

    print compile_print(scanner &tokens)
    {
        expect_special_character(tokens, special_character::left_parenthesis);
        return std::visit(
            overloaded{[](identifier const &) -> print { throw std::invalid_argument("unexpected identifier"); },
                       [](special_character) -> print { throw std::invalid_argument("unexpected special character"); },
                       [&tokens](string_literal message) -> print {
                           expect_special_character(tokens, special_character::right_parenthesis);
                           return print{std::string(message.inner_content)};
                       }},
            expect_token(tokens));
    }

    std::optional<sequence> compile(std::string_view source)
    {
        sequence result;
        scanner tokens{source};
        for (;;)
        {
            if (tokens.is_at_the_end())
            {
                break;
            }
            std::optional<token> head = tokens.pop();
            if (!head)
            {
                return std::nullopt;
            }
            result.elements.emplace_back(std::visit(
                overloaded{
                    [&tokens](identifier const &callee) -> print {
                        if (callee.content == "print")
                        {
                            return compile_print(tokens);
                        }
                        else
                        {
                            throw std::invalid_argument("unknown function");
                        }
                    },
                    [](special_character) -> print { throw std::invalid_argument("unexpected special character"); },
                    [](string_literal) -> print { throw std::invalid_argument("unexpected string"); }},
                *head));
        }
        return std::move(result);
    }
}

lpg::run_result lpg::run(std::string_view source)
{
    std::optional<sequence> program = compile(source);
    if (!program)
    {
        return run_result{std::nullopt};
    }
    std::string output;
    for (print const &element : program->elements)
    {
        output += element.message;
    }
    return run_result{std::move(output)};
}
