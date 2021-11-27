#include "parser.h"
#include "overloaded.h"
#include <stdexcept>

#ifdef _MSC_VER
#define LPG_UNREACHABLE()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        __assume(false);                                                                                               \
    } while (false)
#else
#define LPG_UNREACHABLE() __builtin_unreachable()
#endif

namespace lpg
{
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

    expression parse_expression(scanner &tokens)
    {
        std::optional<token> const next_token = tokens.pop();
        if (!next_token)
        {
            throw std::invalid_argument("unexpected character.");
        }

        expression left_side =
            std::visit(overloaded{[](identifier const &callee) -> expression { return expression{callee}; },
                                  [&tokens](special_character character) -> expression {
                                      switch (character)
                                      {
                                      case special_character::left_parenthesis:
                                          return parse_parentheses(tokens);

                                      case special_character::right_parenthesis:
                                          throw std::invalid_argument("Can not have a closing parenthesis here.");
                                      }
                                      LPG_UNREACHABLE();
                                  },
                                  [](string_literal const &literal) -> expression { return expression{literal}; }},
                       *next_token);

        std::optional<token> right_side = tokens.peek();
        if (!right_side)
        {
            throw std::invalid_argument("No right side deteced");
        }

        return std::visit(
            overloaded{[&left_side](identifier const &) -> expression { return std::move(left_side); },
                       [&left_side, &tokens](special_character character) -> expression {
                           switch (character)
                           {
                           case special_character::left_parenthesis:
                               return parse_call(std::move(left_side), tokens);

                           case special_character::right_parenthesis:
                               return std::move(left_side);
                           }
                           LPG_UNREACHABLE();
                       },
                       [&left_side](string_literal const &) -> expression { return std::move(left_side); }},
            *right_side);
    }

    sequence parse_sequence(scanner &tokens)
    {
        sequence result;
        while (!tokens.is_at_the_end())
        {
            result.elements.push_back(parse_expression(tokens));
        }
        return result;
    }

    expression parse_parentheses(scanner &tokens)
    {
        expression result = parse_expression(tokens);
        expect_special_character(tokens, special_character::right_parenthesis);
        return result;
    }

    expression parse_call(expression callee, scanner &tokens)
    {
        // popping off the left parenthesis
        (void)tokens.pop();
        expression argument = parse_expression(tokens);
        expect_special_character(tokens, special_character::right_parenthesis);
        return expression{
            call{std::make_unique<expression>(std::move(callee)), std::make_unique<expression>(std::move(argument))}};
    }
} // namespace lpg
