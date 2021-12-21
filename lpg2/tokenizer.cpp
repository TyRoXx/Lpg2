#include "tokenizer.h"
#include <stdexcept>

std::ostream &lpg::operator<<(std::ostream &out, const lpg::identifier &value)
{
    return out << value.content;
}

std::ostream &lpg::operator<<(std::ostream &out, lpg::special_character value)
{
    out << static_cast<int>(value);
    return out;
}

std::ostream &lpg::operator<<(std::ostream &out, const lpg::string_literal &value)
{
    return out << '"' << value.inner_content << '"';
}

std::optional<lpg::token> lpg::scanner::pop()
{
    auto result = peek();
    peeked = std::nullopt;
    return result;
}

std::optional<lpg::token> lpg::scanner::peek()
{
    if (peeked)
    {
        return peeked;
    }

    if (is_at_the_end())
    {
        return std::nullopt;
    }

    char head = *next;
    while (head == ' ')
    {
        next++;
        head = *next;
    }
    if (head == '(')
    {
        ++next;
        peeked = token{special_character::left_parenthesis};
        return peeked;
    }
    if (head == ')')
    {
        ++next;
        peeked = token{special_character::right_parenthesis};
        return peeked;
    }
    if (head == '=')
    {
        ++next;
        peeked = token{special_character::assign};
        return peeked;
    }
    if (head == '/')
    {
        ++next;

        if ((next != end) && (*next == '/'))
        {
            ++next;
            auto const literal_begin = next;
            for (;;)
            {
                if (next == end)
                {
                    break;
                }
                if (*next == '\n')
                {
                    ++next;
                    break;
                }
                ++next;
            }
            auto const literal_end = next;
            peeked = token{comment{std::string_view(&*literal_begin, literal_end - literal_begin)}};
            return peeked;
        }

        peeked = token{special_character::slash};
        return peeked;
    }
    if (head == '"')
    {
        ++next;
        std::string_view::iterator literal_begin = next;
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
        std::string_view::iterator literal_end = next;
        ++next;
        peeked = token{string_literal{std::string_view(&*literal_begin, literal_end - literal_begin)}};
        return peeked;
    }
    if (is_identifier_letter(head))
    {
        std::string_view::iterator identifier_begin = next;
        ++next;
        while ((next != end) && is_identifier_letter(*next))
        {
            ++next;
        }
        peeked = token{identifier{std::string_view(&*identifier_begin, next - identifier_begin)}};
        return peeked;
    }
    peeked = std::nullopt;
    return peeked;
}
