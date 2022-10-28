#include "parser.h"
#include "overloaded.h"
#include <stdexcept>

namespace lpg::syntax
{
    std::ostream &operator<<(std::ostream &out, const non_comment &value)
    {
        return out << value.content << "(" << value.location << ")";
    }

    bool operator==(const non_comment &left, const non_comment &right) noexcept
    {
        return (left.content == right.content) && (left.location == right.location);
    }

    std::ostream &operator<<(std::ostream &out, const print &value)
    {
        return out << "print " << value.message;
    }

    bool operator==(const print &left, const print &right) noexcept
    {
        return (left.message == right.message);
    }

    std::ostream &operator<<(std::ostream &out, const call &value)
    {
        if (value.callee && value.argument)
        {
            return out << *value.callee << "(" << *value.argument << ")";
        }
        return out << "call with missing callee or argument";
    }

    namespace
    {
        template <class T>
        bool PointeesEqual(const std::unique_ptr<T> &left, const std::unique_ptr<T> &right)
        {
            if (left)
            {
                if (right)
                {
                    return *left == *right;
                }
                return false;
            }
            return !right;
        }
    } // namespace

    bool operator==(const call &left, const call &right) noexcept
    {
        return PointeesEqual(left.callee, right.callee) && PointeesEqual(left.argument, right.argument);
    }

    sequence::sequence(std::vector<expression> elements, source_location location)
        : elements(move(elements))
        , location(location)
    {
    }

    std::ostream &operator<<(std::ostream &out, const sequence &value)
    {
        out << value.location << ':{';
        for (size_t i = 0; i < value.elements.size(); ++i)
        {
            if (i > 0)
            {
                out << "; ";
            }
            out << value.elements[i];
        }
        return out << '}';
    }

    bool operator==(const sequence &left, const sequence &right) noexcept
    {
        return (left.elements == right.elements) && (left.location == right.location);
    }

    std::ostream &operator<<(std::ostream &out, const declaration &value)
    {
        out << "let " << value.name;
        if (!value.initializer)
        {
            return out;
        }
        return out << " = " << *value.initializer;
    }

    identifier::identifier(std::string_view content, source_location location)
        : content(content)
        , location(location)
    {
    }

    std::ostream &operator<<(std::ostream &out, const identifier &value)
    {
        return out << value.content << "(" << value.location << ")";
    }

    bool operator==(const declaration &left, const declaration &right) noexcept
    {
        return (left.name == right.name) && PointeesEqual(left.initializer, right.initializer);
    }

    std::ostream &operator<<(std::ostream &out, const string_literal_expression &value)
    {
        return out << value.location << ":" << value.literal;
    }

    std::ostream &operator<<(std::ostream &out, const keyword_expression &value)
    {
        return out << value.location << ":" << value.which;
    }

    std::ostream &operator<<(std::ostream &out, const expression &value)
    {
        return out << value.value;
    }

    bool operator==(const expression &left, const expression &right) noexcept
    {
        return left.value == right.value;
    }

    source_location get_location(expression const &tree)
    {
        return std::visit(
            overloaded{[](string_literal_expression const &string) -> source_location { return string.location; },
                       [](identifier const &identifier_) -> source_location { return identifier_.location; },
                       [](call const &call_) -> source_location { return get_location(*call_.callee); },
                       [](sequence const &sequence_) -> source_location { return sequence_.location; },
                       [](declaration const &declaration_) -> source_location { return declaration_.name.location; },
                       [](keyword_expression const &keyword) -> source_location { return keyword.location; }},
            tree.value);
    }

    std::optional<non_comment> peek_next_non_comment(scanner &tokens)
    {
        for (;;)
        {
            std::optional<token> maybe_peeked = tokens.peek();
            if (!maybe_peeked)
            {
                return std::nullopt;
            }

            token &peeked = *maybe_peeked;
            std::optional<non_comment> result =
                std::visit(overloaded{
                               [](comment const &) -> std::optional<non_comment> { return std::nullopt; },
                               [&peeked](auto value) -> std::optional<non_comment> {
                                   return non_comment{std::move(value), peeked.location};
                               },
                           },
                           peeked.content);

            if (result.has_value())
            {
                return result;
            }
            std::optional<token> popped = tokens.pop();
            assert(popped);
        }
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

    parse_error::parse_error(std::string error_message, source_location where)
        : error_message(move(error_message))
        , where(where)
    {
    }

    bool operator==(const parse_error &left, const parse_error &right) noexcept
    {
        return (left.error_message == right.error_message) && (left.where == right.where);
    }

    std::ostream &operator<<(std::ostream &out, const parse_error &value)
    {
        return out << value.where << ": " << value.error_message;
    }

    std::tuple<std::optional<identifier_token>, source_location> parser::expect_identifier()
    {
        std::optional<token> token = tokens.pop();

        if (!token)
        {
            on_error(parse_error{"Expected identifier but got end of stream", tokens.next_location});
            return std::make_tuple(std::nullopt, tokens.next_location);
        }

        return std::make_tuple(std::visit(overloaded{
                                              [](identifier_token &&identifier_) -> std::optional<identifier_token> {
                                                  return std::move(identifier_);
                                              },
                                              [this, &token](auto &&) -> std::optional<identifier_token> {
                                                  on_error(parse_error({"Expected identifier", token->location}));
                                                  return std::nullopt;
                                              },
                                          },
                                          std::move(token->content)),
                               token->location);
    }

    std::optional<declaration> parser::parse_declaration()
    {
        auto [name, location] = expect_identifier();
        if (!name)
        {
            return std::nullopt;
        }

        const bool is_assignment = expect_special_character(special_character::assign);
        if (!is_assignment)
        {
            return std::nullopt;
        }

        std::optional<expression> initializer = parse_expression();
        if (!initializer)
        {
            on_error(parse_error{
                "Invalid initializer value for identifier: " + std::string(name.value().content), location});
            return std::nullopt;
        }
        return declaration{
            identifier{name->content, location}, std::make_unique<expression>(std::move(initializer.value()))};
    }

    bool parser::expect_special_character(special_character expected)
    {
        const std::optional<token> token = tokens.pop();
        if (!token)
        {
            on_error(parse_error{"Expected special character but got end of stream", tokens.next_location});
            return false;
        }
        if (std::holds_alternative<special_character>(token->content))
        {
            if (expected != std::get<special_character>(token->content))
            {
                on_error(parse_error{"Expected a different special character", token->location});
            }
            else
            {
                return true;
            }
        }
        on_error(parse_error{"Expected something else", token->location});
        return false;
    }

    std::optional<expression> parser::parse_expression()
    {
        std::optional<non_comment> const next_token = pop_next_non_comment(tokens);
        if (!next_token)
        {
            on_error(parse_error{"Unexpected end of stream", tokens.next_location});
            return std::nullopt;
        }

        std::optional<expression> left_side = std::visit(
            overloaded{
                [this, &next_token](identifier_token const &callee) -> std::optional<expression> {
                    if (callee.content == "let")
                    {
                        std::optional<declaration> declaration = parse_declaration();
                        if (!declaration)
                        {
                            return std::nullopt;
                        }
                        return expression{std::move(declaration.value())};
                    }
                    return expression{identifier{callee.content, next_token->location}};
                },
                [this, &next_token](special_character character) -> std::optional<expression> {
                    switch (character)
                    {
                    case special_character::left_parenthesis:
                        return parse_parentheses();
                    case special_character::right_parenthesis:
                        on_error(parse_error({"Can not have a closing parenthesis here.", next_token->location}));
                        return std::nullopt;
                    case special_character::left_brace:
                        return parse_braces(next_token->location);
                    case special_character::right_brace:
                        on_error(parse_error({"Can not have a closing parenthesis here.", next_token->location}));
                        return std::nullopt;
                    case special_character::slash:
                        on_error(parse_error({"Can not have a slash here.", next_token->location}));
                        return std::nullopt;
                    case special_character::assign:
                        on_error(parse_error({"Can not have an assignment operator here.", next_token->location}));
                        return std::nullopt;
                    }
                    LPG_UNREACHABLE();
                },
                [&next_token](string_literal const &literal) -> std::optional<expression> {
                    return expression{string_literal_expression{literal, next_token->location}};
                },
                [&next_token](keyword const keyword_) -> std::optional<expression> {
                    return expression{keyword_expression{keyword_, next_token->location}};
                }},
            next_token->content);

        std::optional<non_comment> right_side = peek_next_non_comment(tokens);
        if (!right_side)
        {
            return left_side;
        }

        return std::visit(
            overloaded{
                [&left_side](identifier_token const &) -> std::optional<expression> { return std::move(left_side); },
                [this, &left_side, &right_side](special_character character) -> std::optional<expression> {
                    switch (character)
                    {
                    case special_character::left_parenthesis:
                        return parse_call(std::move(left_side.value()));
                    case special_character::right_parenthesis:
                        return std::move(left_side);
                    case special_character::left_brace:
                        return std::move(left_side);
                    case special_character::right_brace:
                        return std::move(left_side);
                    case special_character::slash:
                        on_error(parse_error{"Can not have a slash here.", right_side->location});
                        return std::nullopt;
                    case special_character::assign:
                        on_error(parse_error{"Can not have an assignment operator here.", right_side->location});
                        return std::nullopt;
                    }
                    LPG_UNREACHABLE();
                },
                [&left_side](string_literal const &) -> std::optional<expression> { return std::move(left_side); },
                [&left_side](keyword const) -> std::optional<expression> { return std::move(left_side); }},
            right_side->content);
    }

    sequence parser::parse_sequence(const bool is_in_braces, source_location const &start_location)
    {
        sequence result{{}, start_location};
        for (;;)
        {
            std::optional<non_comment> maybe_token = peek_next_non_comment(tokens);
            if (!maybe_token)
            {
                if (is_in_braces)
                {
                    on_error(parse_error{"Missing closing brace '}' before end of file", tokens.next_location});
                }
                break;
            }
            if (is_in_braces)
            {
                special_character const *const token = std::get_if<special_character>(&maybe_token->content);
                if (token && (*token == special_character::right_brace))
                {
                    (void)tokens.pop();
                    break;
                }
            }
            std::optional<expression> expression = parse_expression();
            if (!expression)
            {
                break;
            }
            result.elements.push_back(std::move(expression.value()));
        }
        return result;
    }

    std::optional<expression> parser::parse_parentheses()
    {
        std::optional<expression> result = parse_expression();
        if (!result)
        {
            return std::nullopt;
        }
        expect_special_character(special_character::right_parenthesis);
        return result;
    }

    std::optional<expression> parser::parse_braces(source_location const &start_location)
    {
        return expression{parse_sequence(true, start_location)};
    }

    std::optional<expression> parser::parse_call(expression callee)
    {
        // popping off the left parenthesis
        token const parenthesis = tokens.pop().value();
        std::optional<expression> argument = parse_expression();
        if (!argument)
        {
            on_error(parse_error{"Could not parse argument of the function", parenthesis.location});
            return std::nullopt;
        }
        expect_special_character(special_character::right_parenthesis);
        return expression{call{std::make_unique<expression>(std::move(callee)),
                               std::make_unique<expression>(std::move(argument.value()))}};
    }

    sequence compile(std::string_view source, std::function<void(parse_error)> on_error)
    {
        parser parser(scanner{source}, on_error);
        sequence parsed = parser.parse_sequence(false, source_location{0, 0});
        if (parser.tokens.has_failed)
        {
            on_error(parse_error{"Tokenization failed", parser.tokens.next_location});
        }
        return parsed;
    }
} // namespace lpg::syntax
