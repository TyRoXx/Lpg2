#pragma once
#include "tokenizer.h"
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace lpg::syntax
{
    using non_comment_content = std::variant<identifier_token, special_character, string_literal, keyword>;

    struct non_comment
    {
        non_comment_content content;
        source_location location;
    };

    std::ostream &operator<<(std::ostream &out, const non_comment &value);
    bool operator==(const non_comment &left, const non_comment &right) noexcept;

    struct print
    {
        std::string message;
    };

    std::ostream &operator<<(std::ostream &out, const print &value);
    bool operator==(const print &left, const print &right) noexcept;

    struct expression;

    struct call
    {
        std::unique_ptr<expression> callee;
        std::unique_ptr<expression> argument;
    };

    std::ostream &operator<<(std::ostream &out, const call &value);
    bool operator==(const call &left, const call &right) noexcept;

    struct sequence
    {
        std::vector<expression> elements;
        source_location location;

        sequence(std::vector<expression> elements, source_location location);
    };

    std::ostream &operator<<(std::ostream &out, const sequence &value);
    bool operator==(const sequence &left, const sequence &right) noexcept;

    struct identifier
    {
        std::string_view content;
        source_location location;

        identifier(std::string_view content, source_location location);
        std::weak_ordering operator<=>(identifier const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const identifier &value);

    struct declaration
    {
        identifier name;
        std::unique_ptr<expression> initializer;
    };

    std::ostream &operator<<(std::ostream &out, const declaration &value);
    bool operator==(const declaration &left, const declaration &right) noexcept;

    struct string_literal_expression
    {
        string_literal literal;
        source_location location;

        std::weak_ordering operator<=>(string_literal_expression const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const string_literal_expression &value);

    struct keyword_expression
    {
        keyword which;
        source_location location;

        std::weak_ordering operator<=>(keyword_expression const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const keyword_expression &value);

    struct expression
    {
        std::variant<string_literal_expression, identifier, call, sequence, declaration, keyword_expression> value;
    };

    std::ostream &operator<<(std::ostream &out, const expression &value);
    bool operator==(const expression &left, const expression &right) noexcept;
    [[nodiscard]] source_location get_location(expression const &tree);

    std::optional<non_comment> peek_next_non_comment(scanner &tokens);
    std::optional<non_comment> pop_next_non_comment(scanner &tokens);

    struct parse_error
    {
        std::string error_message;
        source_location where;

        parse_error(std::string error_message, source_location where);
    };

    bool operator==(const parse_error &left, const parse_error &right) noexcept;
    std::ostream &operator<<(std::ostream &out, const parse_error &value);

    struct parser
    {
        parser(scanner tokens, std::function<void(parse_error)> on_error)
            : tokens(std::move(tokens))
            , on_error(std::move(on_error))
        {
        }
        scanner tokens;
        std::function<void(parse_error)> on_error;

        sequence parse_sequence(bool is_in_braces, source_location const &start_location);

    private:
        std::optional<expression> parse_expression();
        std::optional<expression> parse_parentheses();
        std::optional<expression> parse_braces(source_location const &start_location);
        std::optional<expression> parse_call(expression callee);
        std::optional<declaration> parse_declaration();

        std::tuple<std::optional<identifier_token>, source_location> expect_identifier();
        bool expect_special_character(special_character special_character);
    };

    [[nodiscard]] sequence compile(std::string_view source, std::function<void(parse_error)> on_error);
} // namespace lpg::syntax
