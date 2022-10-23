#pragma once
#include <cassert>
#include <compare>
#include <optional>
#include <ostream>
#include <string_view>
#include <variant>

namespace lpg::syntax
{
    struct source_location
    {
        size_t line = 0;
        size_t column = 0;

        source_location() noexcept = default;
        source_location(size_t line, size_t column) noexcept;
        std::weak_ordering operator<=>(source_location const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const source_location &value);

    struct identifier_token
    {
        std::string_view content;

        std::weak_ordering operator<=>(identifier_token const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const identifier_token &value);

    enum class special_character
    {
        left_parenthesis,
        right_parenthesis,
        left_brace,
        right_brace,
        slash,
        assign
    };

    std::ostream &operator<<(std::ostream &out, special_character value);

    struct string_literal
    {
        std::string_view inner_content;

        std::weak_ordering operator<=>(string_literal const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const string_literal &value);

    struct comment
    {
        std::string_view inner_content;

        std::weak_ordering operator<=>(comment const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const comment &value);

    using token_content = std::variant<identifier_token, special_character, string_literal, comment>;

    struct token
    {
        token_content content;
        source_location location;

        token(token_content content, source_location location) noexcept;
        std::weak_ordering operator<=>(token const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, const token &value);

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
        source_location next_location;

        std::optional<token> peeked;
        bool has_failed = false;

        explicit scanner(std::string_view source)
            : next(source.begin())
            , end(source.end())
        {
        }

        [[nodiscard]] std::optional<token> pop();
        [[nodiscard]] std::optional<token> peek();
    };
} // namespace lpg::syntax

namespace lpg
{
    template <class T>
    [[nodiscard]] std::string format(T const &value)
    {
        std::ostringstream buffer;
        buffer << value;
        return buffer.str();
    }
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
