#pragma once

#include <cassert>
#include <compare>
#include <optional>
#include <ostream>
#include <string_view>
#include <variant>

namespace lpg
{
    struct identifier
    {
        std::string_view content;
        std::weak_ordering operator<=>(identifier const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const lpg::identifier &value);

    enum class special_character
    {
        left_parenthesis,
        right_parenthesis,
        left_brace,
        right_brace,
        slash,
        assign
    };

    std::ostream &operator<<(std::ostream &out, lpg::special_character value);

    struct string_literal
    {
        std::string_view inner_content;
        std::weak_ordering operator<=>(string_literal const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const lpg::string_literal &value);

    struct comment
    {
        std::string_view inner_content;
    };

    std::ostream &operator<<(std::ostream &out, const comment &value);

    using token = std::variant<identifier, special_character, string_literal, comment>;

    inline bool is_identifier_letter(char c)
    {
        return c >= 'a' && c <= 'z';
    }

    inline bool is_whitespace(char c)
    {
        return c == ' ' || c == '\n';
    }

    struct scanner
    {
        std::string_view::iterator next;
        std::string_view::iterator end;

        std::optional<token> peeked;

        explicit scanner(std::string_view source)
            : next(source.begin())
            , end(source.end())
        {
        }

        [[nodiscard]] std::optional<token> pop();
        [[nodiscard]] std::optional<token> peek();
    };
} // namespace lpg

namespace std
{
    // technically we are not allowed to overload these in namespace std, but it works, and these operators should have
    // existed in the standard anyway
    template <class... T>
    std::ostream &operator<<(std::ostream &out, const std::variant<T...> &value)
    {
        std::visit([&out, &value](const auto &element) { out << value.index() << ": " << element; }, value);
        return out;
    }

    template <class T>
    std::ostream &operator<<(std::ostream &out, const std::optional<T> &value)
    {
        if (value)
        {
            return out << *value;
        }
        return out << "nullopt";
    }
} // namespace std
