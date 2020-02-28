#include <cassert>
#include <optional>
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
        right_parenthesis
    };

    struct string_literal
    {
        std::string_view inner_content;
    };

    using token = std::variant<identifier, special_character, string_literal>;

    inline bool is_identifier_letter(char c)
    {
        return c >= 'a' && c <= 'z';
    }

    struct scanner
    {
        char const *next;
        char const *end;

        explicit scanner(std::string_view source)
            : next(source.data())
            , end(source.end())
        {
        }

        bool is_at_the_end() const
        {
            return (next == end);
        }

        [[nodiscard]] std::optional<token> pop();
    };
}