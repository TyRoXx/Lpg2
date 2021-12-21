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

    using token = std::variant<identifier, special_character, string_literal, comment>;

    inline bool is_identifier_letter(char c)
    {
        return c >= 'a' && c <= 'z';
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

        bool is_at_the_end() const
        {
            return (next == end) && !peeked;
        }

        [[nodiscard]] std::optional<token> pop();
        [[nodiscard]] std::optional<token> peek();
    };

    template <class... T>
    std::ostream &operator<<(std::ostream &out, const std::variant<T...> &value)
    {
        std::visit([&out, &value](const auto &element) { out << value.index() << ": " << element; }, value);
        return out;
    }
} // namespace lpg
