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

std::ostream &lpg::operator<<(std::ostream &out, const comment &value)
{
    return out << "/*" << value.inner_content << "*/";
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

    if (next == end)
    {
        return std::nullopt;
    }

    char head = *next;
    while (is_whitespace(head))
    {
        next++;
        if (next == end)
        {
            return std::nullopt;
        }
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
    if (head == '{')
    {
        ++next;
        peeked = token{special_character::left_brace};
        return peeked;
    }
    if (head == '}')
    {
        ++next;
        peeked = token{special_character::right_brace};
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
        // only update next if the string literal is valid, so that you can see where the invalid literal began
        auto i = next;
        ++i;
        std::string_view::iterator literal_begin = i;
        for (;;)
        {
            if (i == end)
            {
                has_failed = true;
                peeked = std::nullopt;
                return peeked;
            }
            if (*i == '"')
            {
                break;
            }
            ++i;
        }
        std::string_view::iterator literal_end = i;
        ++i;
        next = i;
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
