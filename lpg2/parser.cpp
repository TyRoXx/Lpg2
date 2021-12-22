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
    std::optional<non_comment> peek_next_non_comment(scanner &tokens)
    {
        while (!tokens.is_at_the_end())
        {
            std::optional<token> peeked = tokens.peek();
            if (!peeked)
            {
                return std::nullopt;
            }

            std::optional<non_comment> result =
                std::visit(overloaded{
                               [](comment const &) -> std::optional<non_comment> { return std::nullopt; },
                               [](auto value) -> std::optional<non_comment> { return value; },
                           },
                           peeked.value());

            if (result.has_value())
            {
                return result;
            }
            std::optional<token> popped = tokens.pop();
            assert(popped);
        }
        return std::nullopt;
    }

    std::optional<non_comment> pop_next_non_comment(scanner &tokens)
    {
        std::optional<non_comment> peeked = peek_next_non_comment(tokens);
        if (!peeked.has_value())
        {
            return std::nullopt;
        }
        std::optional<token> popped = tokens.pop();
        assert(popped);
        return peeked;
    }

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
                              [](string_literal) { throw std::invalid_argument("unexpected string"); },
                              [](comment) { throw std::invalid_argument("unexpected comment"); }},
                   expect_token(tokens));
    }

    identifier expect_identifier(scanner &tokens)
    {
        return std::visit(
            overloaded{
                [](identifier &&identifier_) -> identifier { return std::move(identifier_); },
                [](special_character) -> identifier { throw std::invalid_argument("unexpected special character"); },
                [](string_literal) -> identifier { throw std::invalid_argument("unexpected string"); },
                [](comment) -> identifier { throw std::invalid_argument("unexpected comment"); }},
            expect_token(tokens));
    }

    declaration parser::parse_declaration()
    {
        identifier name = expect_identifier(tokens);
        expect_special_character(tokens, special_character::assign);
        expression initializer = parse_expression();
        return declaration{name, std::make_unique<expression>(std::move(initializer))};
    }

    expression parser::parse_expression()
    {
        std::optional<non_comment> const next_token = pop_next_non_comment(tokens);
        if (!next_token)
        {
            throw std::invalid_argument("unexpected character.");
        }

        expression left_side =
            std::visit(overloaded{[this](identifier const &callee) -> expression {
                                      if (callee.content == "let")
                                      {
                                          return expression{parse_declaration()};
                                      }
                                      return expression{callee};
                                  },
                                  [this](special_character character) -> expression {
                                      switch (character)
                                      {
                                      case special_character::left_parenthesis:
                                          return parse_parentheses();
                                      case special_character::right_parenthesis:
                                          throw std::invalid_argument("Can not have a closing parenthesis here.");
                                      case special_character::slash:
                                          throw std::invalid_argument("Can not have a slash here.");
                                      case special_character::assign:
                                          throw std::invalid_argument("Can not have an assignment operator here.");
                                      }
                                      LPG_UNREACHABLE();
                                  },
                                  [](string_literal const &literal) -> expression { return expression{literal}; }},
                       *next_token);

        std::optional<non_comment> right_side = peek_next_non_comment(tokens);
        if (!right_side)
        {
            return left_side;
        }

        return std::visit(
            overloaded{[&left_side](identifier const &) -> expression { return std::move(left_side); },
                       [&left_side, this](special_character character) -> expression {
                           switch (character)
                           {
                           case special_character::left_parenthesis:
                               return parse_call(std::move(left_side));
                           case special_character::right_parenthesis:
                               return std::move(left_side);
                           case special_character::slash:
                               throw std::invalid_argument("Can't have a slash here");
                           case special_character::assign:
                               throw std::invalid_argument("Can not have an assignment operator here.");
                           }
                           LPG_UNREACHABLE();
                       },
                       [&left_side](string_literal const &) -> expression { return std::move(left_side); }},
            *right_side);
    }

    sequence parser::parse_sequence()
    {
        sequence result;
        while (!tokens.is_at_the_end())
        {
            std::optional<non_comment> maybe_token = peek_next_non_comment(tokens);
            if (!maybe_token)
            {
                break;
            }
            result.elements.push_back(parse_expression());
        }
        return result;
    }

    expression parser::parse_parentheses()
    {
        expression result = parse_expression();
        expect_special_character(tokens, special_character::right_parenthesis);
        return result;
    }

    expression parser::parse_call(expression callee)
    {
        // popping off the left parenthesis
        (void)tokens.pop();
        expression argument = parse_expression();
        expect_special_character(tokens, special_character::right_parenthesis);
        return expression{
            call{std::make_unique<expression>(std::move(callee)), std::make_unique<expression>(std::move(argument))}};
    }
} // namespace lpg
