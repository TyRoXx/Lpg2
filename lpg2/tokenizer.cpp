#include "tokenizer.h"

std::optional<lpg::token> lpg::scanner::pop()
{
    assert(!is_at_the_end());
    char const head = *next;
    if (head == '(')
    {
        ++next;
        return token{special_character::left_parenthesis};
    }
    if (head == ')')
    {
        ++next;
        return token{special_character::right_parenthesis};
    }
    if (head == '"')
    {
        ++next;
        char const *literal_begin = next;
        for (;;)
        {
            if (next == end)
            {
                throw std::invalid_argument("invalid string literal");
            }
            if (*next == '"')
            {
                break;
            }
            ++next;
        }
        char const *literal_end = next;
        ++next;
        return token{string_literal{std::string_view(literal_begin, literal_end - literal_begin)}};
    }
    if (is_identifier_letter(head))
    {
        char const *identifier_begin = next;
        ++next;
        while ((next != end) && is_identifier_letter(*next))
        {
            ++next;
        }
        return token{identifier{std::string_view(identifier_begin, next - identifier_begin)}};
    }
    return std::nullopt;
}