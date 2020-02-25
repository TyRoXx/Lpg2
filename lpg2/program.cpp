#include "program.h"
#include <cassert>
#include <variant>
#include <vector>

namespace lpg
{
    struct identifier
    {
        std::string_view content;
    };

    enum class special_character
    {
        left_parenthesis,
        right_parenthesis
    };

    struct string_literal
    {
        std::string_view inner_content;
    };

    using token = std::variant<identifier, special_character, string_literal>;

    bool is_identifier_letter(char c)
    {
        return c >= 'a' && c <= 'z';
    }

    struct scanner
    {
        char const *next;
        char const *end;

        explicit scanner(std::string_view source)
            : next(source.data())
            , end(source.end())
        {
        }

        bool is_at_the_end() const
        {
            return (next == end);
        }

        [[nodiscard]] std::optional<token> pop()
        {
            assert(!is_at_the_end());
            char const head = *next;
            if (head == '(')
            {
                ++next;
                return token{special_character::left_parenthesis};
            }
            if (head == ')')
            {
                ++next;
                return token{special_character::right_parenthesis};
            }
            if (head == '"')
            {
                ++next;
                char const *literal_begin = next;
                for (;;)
                {
                    if (next == end)
                    {
                        throw std::invalid_argument("invalid string literal");
                    }
                    if (*next == '"')
                    {
                        break;
                    }
                    ++next;
                }
                char const *literal_end = next;
                ++next;
                return token{string_literal{std::string_view(literal_begin, literal_end - literal_begin)}};
            }
            if (is_identifier_letter(head))
            {
                char const *identifier_begin = next;
                ++next;
                while ((next != end) && is_identifier_letter(*next))
                {
                    ++next;
                }
                return token{identifier{std::string_view(identifier_begin, next - identifier_begin)}};
            }
            return std::nullopt;
        }
    };

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
