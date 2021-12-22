#pragma once

#include "tokenizer.h"
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace lpg
{
    using non_comment = std::variant<identifier, special_character, string_literal>;

    struct print
    {
        std::string message;
    };

    struct expression;

    struct call
    {
        std::unique_ptr<expression> callee;
        std::unique_ptr<expression> argument;
    };

    struct sequence
    {
        std::vector<expression> elements;
    };

    struct declaration
    {
        identifier name;
        std::unique_ptr<expression> initializer;
    };

    struct expression
    {
        std::variant<string_literal, identifier, call, sequence, declaration> value;
    };

    std::optional<non_comment> peek_next_non_comment(scanner &tokens);
    std::optional<non_comment> pop_next_non_comment(scanner &tokens);

    token expect_token(scanner &tokens);
    void expect_special_character(scanner &tokens, special_character expected);

    struct parse_error
    {
        std::string error_message;
    };

    struct parser
    {
        parser(scanner tokens, std::function<void(parse_error)> on_error)
            : tokens(std::move(tokens))
            , on_error(std::move(on_error))
        {
        }
        scanner tokens;
        std::function<void(parse_error)> on_error;

        sequence parse_sequence();

    private:
        std::optional<expression> parse_expression();
        std::optional<expression> parse_parentheses();
        std::optional<expression> parse_call(expression callee);
        std::optional<declaration> parse_declaration();
    };
} // namespace lpg
