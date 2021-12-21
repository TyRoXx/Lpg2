#pragma once

#include "tokenizer.h"
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

    struct expression
    {
        std::variant<string_literal, identifier, call, sequence> value;
    };

    std::optional<non_comment> peek_next_non_comment(scanner &tokens);
    std::optional<non_comment> pop_next_non_comment(scanner &tokens);

    token expect_token(scanner &tokens);

    expression parse_expression(scanner &tokens);

    sequence parse_sequence(scanner &tokens);

    expression parse_parentheses(scanner &tokens);

    expression parse_call(expression callee, scanner &tokens);

    void expect_special_character(scanner &tokens, special_character expected);
}
