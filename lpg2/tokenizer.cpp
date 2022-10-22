#include "tokenizer.h"
#include <stdexcept>

lpg::source_location::source_location(size_t line, size_t column) noexcept
    : line(line)
    , column(column)
{
}

std::ostream &lpg::operator<<(std::ostream &out, const source_location &value)
{
    return out << (value.line + 1) << ":" << (value.column + 1);
}
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

lpg::token::token(token_content content, source_location location) noexcept
    : content(std::move(content))
    , location(location)
{
}

std::ostream &lpg::operator<<(std::ostream &out, const token &value)
{
    return out << value.content << "(" << value.location << ")";
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
        if (head == '\n')
        {
            ++next_location.line;
            next_location.column = 0;
        }

        next++;
        if (next == end)
        {
            return std::nullopt;
        }
        head = *next;
    }

    if (head == '(')
    {
        peeked = token{special_character::left_parenthesis, next_location};
        ++next;
        ++next_location.column;
        return peeked;
    }
    if (head == ')')
    {
        peeked = token{special_character::right_parenthesis, next_location};
        ++next;
        ++next_location.column;
        return peeked;
    }
    if (head == '{')
    {
        peeked = token{special_character::left_brace, next_location};
        ++next;
        ++next_location.column;
        return peeked;
    }
    if (head == '}')
    {
        peeked = token{special_character::right_brace, next_location};
        ++next;
        ++next_location.column;
        return peeked;
    }
    if (head == '=')
    {
        peeked = token{special_character::assign, next_location};
        ++next;
        ++next_location.column;
        return peeked;
    }
    if (head == '/')
    {
        source_location const token_location = next_location;
        ++next;
        ++next_location.column;

        if ((next != end) && (*next == '/'))
        {
            ++next;
            ++next_location.column;
            auto const comment_begin = next;
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
                ++next_location.column;
            }
            auto const comment_end = next;
            peeked = token{comment{std::string_view(&*comment_begin, comment_end - comment_begin)}, token_location};
            return peeked;
        }

        peeked = token{special_character::slash, token_location};
        return peeked;
    }
    if (head == '"')
    {
        // only update next if the string literal is valid, so that you can see where the invalid literal began
        auto i = next;
        source_location const string_location = next_location;
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
        next_location.column += (next - literal_begin);
        peeked = token{string_literal{std::string_view(&*literal_begin, literal_end - literal_begin)}, string_location};
        return peeked;
    }
    if (is_identifier_letter(head))
    {
        std::string_view::iterator identifier_begin = next;
        source_location const identifier_location = next_location;
        ++next;
        ++next_location.column;
        while ((next != end) && is_identifier_letter(*next))
        {
            ++next;
            ++next_location.column;
        }
        peeked = token{identifier{std::string_view(&*identifier_begin, next - identifier_begin)}, identifier_location};
        return peeked;
    }
    peeked = std::nullopt;
    return peeked;
}
