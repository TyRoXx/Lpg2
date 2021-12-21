#include "../lpg2/parser.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(scan_nothing)
{
    auto s = lpg::scanner("");
    BOOST_TEST(s.is_at_the_end());
}

BOOST_AUTO_TEST_CASE(scan_invalid_token)
{
    auto s = lpg::scanner("+");
    std::optional<lpg::token> const t = s.peek();

    BOOST_TEST(!s.is_at_the_end());
    BOOST_TEST(!t.has_value());
}

BOOST_AUTO_TEST_CASE(scan_peek_token)
{
    auto s = lpg::scanner("\"Hello\"");
    std::optional<lpg::token> const t = s.peek();

    BOOST_TEST(!s.is_at_the_end());
    BOOST_TEST(t.has_value());
    lpg::string_literal literal = std::get<lpg::string_literal>(t.value());
    BOOST_TEST(literal.inner_content == "Hello");
}

BOOST_AUTO_TEST_CASE(scan_string)
{
    auto s = lpg::scanner("\"Hello\"");
    std::optional<lpg::token> const t = s.pop();
    BOOST_TEST(t.has_value());
    lpg::string_literal const string = std::get<lpg::string_literal>(t.value());
    BOOST_TEST(string.inner_content == "Hello");
}

BOOST_AUTO_TEST_CASE(scan_invalid_string)
{
    auto s = lpg::scanner("\"Hello");

    std::optional<lpg::token> t;
    BOOST_CHECK_THROW(t = s.pop(), std::invalid_argument);
    BOOST_TEST(!t.has_value());
}

BOOST_AUTO_TEST_CASE(scan_parentheses)
{
    auto s = lpg::scanner("()");
    std::optional<lpg::token> const first_paren = s.pop();

    BOOST_TEST(!s.is_at_the_end());
    BOOST_TEST(first_paren.has_value());
    lpg::special_character const character = std::get<lpg::special_character>(first_paren.value());
    BOOST_TEST(character == lpg::special_character::left_parenthesis);

    auto const second_paren = s.pop();

    BOOST_TEST(s.is_at_the_end());
    BOOST_TEST(second_paren.has_value());
    lpg::special_character const character1 = std::get<lpg::special_character>(second_paren.value());
    BOOST_TEST(character1 == lpg::special_character::right_parenthesis);
}

BOOST_AUTO_TEST_CASE(scan_identifier)
{
    auto s = lpg::scanner("test");

    std::optional<lpg::token> const t = s.pop();
    BOOST_TEST(s.is_at_the_end());
    BOOST_TEST(t.has_value());
    lpg::identifier id = std::get<lpg::identifier>(t.value());
    BOOST_TEST(id.content == "test");
}

BOOST_AUTO_TEST_CASE(print_special_char)
{
    std::stringstream s;

    s << lpg::special_character::left_parenthesis;
    BOOST_TEST(s.str() == "0");
}