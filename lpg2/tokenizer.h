#pragma once

#include <cassert>
#include <optional>
#include <ostream>
#include <string_view>
#include <variant>

namespace lpg
{
    struct identifier
    {
        std::string_view content;
    };

    enum class special_character
    {
        left_parenthesis,
        right_parenthesis,
        slash
    };

    inline std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out, lpg::special_character value)
    {
        out << static_cast<int>(value);
        return out;
    }

    struct string_literal
    {
        std::string_view inner_content;
    };

    struct comment
    {
        std::string_view inner_content;
    };

    using token = std::variant<identifier, special_character, string_literal, comment>;

    inline bool is_identifier_letter(char c)
    {
        return c >= 'a' && c <= 'z';
    }

    struct scanner
    {
        char const *next;
        char const *end;

        std::optional<token> peeked;

        explicit scanner(std::string_view source)
            : next(source.data())
            , end(source.end())
        {
        }

        bool is_at_the_end() const
        {
            return (next == end) && !peeked;
        }

        [[nodiscard]] std::optional<token> pop();
        [[nodiscard]] std::optional<token> peek();
    };
}
