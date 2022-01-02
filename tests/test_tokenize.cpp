#include "../lpg2/parser.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(scan_nothing)
{
    auto s = lpg::scanner("");
    BOOST_TEST(!s.peek());
}

BOOST_AUTO_TEST_CASE(scan_invalid_token)
{
    auto s = lpg::scanner("+");
    std::optional<lpg::token> const t = s.peek();
    BOOST_TEST(!t.has_value());
}

BOOST_AUTO_TEST_CASE(scan_peek_token)
{
    auto s = lpg::scanner("\"Hello\"");
    std::optional<lpg::token> const t = s.peek();
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

    BOOST_TEST(s.peek());
    BOOST_TEST(first_paren.has_value());
    lpg::special_character const character = std::get<lpg::special_character>(first_paren.value());
    BOOST_TEST(character == lpg::special_character::left_parenthesis);

    auto const second_paren = s.pop();

    BOOST_TEST(!s.peek());
    BOOST_TEST(second_paren.has_value());
    lpg::special_character const character1 = std::get<lpg::special_character>(second_paren.value());
    BOOST_TEST(character1 == lpg::special_character::right_parenthesis);
}

BOOST_AUTO_TEST_CASE(scan_identifier)
{
    auto s = lpg::scanner("test");
    std::optional<lpg::token> const t = s.pop();
    BOOST_TEST(!s.peek());
    BOOST_TEST(t.has_value());
    lpg::identifier id = std::get<lpg::identifier>(t.value());
    BOOST_TEST(id.content == "test");
}

BOOST_AUTO_TEST_CASE(scan_slash)
{
    auto s = lpg::scanner("/ 2");
    std::optional<lpg::token> const t = s.pop();
    BOOST_TEST(!s.peek());
    BOOST_TEST(t.has_value());
    lpg::special_character const slash = std::get<lpg::special_character>(t.value());
    BOOST_TEST(slash == lpg::special_character::slash);
}

BOOST_AUTO_TEST_CASE(scan_slash_end_of_file)
{
    auto s = lpg::scanner("/");
    std::optional<lpg::token> const t = s.pop();
    BOOST_TEST(!s.peek());
    BOOST_TEST(t.has_value());
    lpg::special_character const slash = std::get<lpg::special_character>(t.value());
    BOOST_TEST(slash == lpg::special_character::slash);
}

BOOST_AUTO_TEST_CASE(scan_comment_end_of_file)
{
    auto s = lpg::scanner("//Just a comment");
    std::optional<lpg::token> const t = s.pop();
    BOOST_TEST(!s.peek());
    BOOST_TEST(t.has_value());

    lpg::comment comment = std::get<lpg::comment>(t.value());
    BOOST_TEST(comment.inner_content == "Just a comment");
}

BOOST_AUTO_TEST_CASE(scan_comment_end_of_line)
{
    auto s = lpg::scanner("//Just a comment\n");
    std::optional<lpg::non_comment> const t = lpg::peek_next_non_comment(s);
    BOOST_TEST(!s.peek());
    BOOST_TEST(!t.has_value());
}

BOOST_AUTO_TEST_CASE(scan_comment)
{
    auto s = lpg::scanner("//Just a comment\ntest");
    std::optional<lpg::non_comment> const t = lpg::peek_next_non_comment(s);
    BOOST_TEST(s.peek());
    BOOST_TEST(t.has_value());

    lpg::identifier id = std::get<lpg::identifier>(t.value());
    BOOST_TEST(id.content == "test");
}

BOOST_AUTO_TEST_CASE(print_special_character)
{
    std::ostringstream s;
    s << lpg::special_character::left_parenthesis;
    BOOST_TEST(s.str() == "0");
}

BOOST_AUTO_TEST_CASE(print_identifier)
{
    std::ostringstream s;
    s << lpg::identifier{"name"};
    BOOST_TEST(s.str() == "name");
}

BOOST_AUTO_TEST_CASE(print_string_literal)
{
    std::ostringstream s;
    s << lpg::string_literal{"content"};
    BOOST_TEST(s.str() == "\"content\"");
}

BOOST_AUTO_TEST_CASE(print_comment)
{
    std::ostringstream s;
    s << lpg::comment{"content"};
    BOOST_TEST(s.str() == "/*content*/");
}

BOOST_AUTO_TEST_CASE(ignore_spaces)
{
    auto s = lpg::scanner("let a");
    std::optional<lpg::non_comment> const let_token = lpg::pop_next_non_comment(s);
    BOOST_TEST(s.peek());
    BOOST_TEST(let_token.value() == lpg::non_comment{lpg::identifier{"let"}});

    std::optional<lpg::non_comment> const id_token = lpg::pop_next_non_comment(s);
    BOOST_TEST(!s.peek());
    BOOST_TEST(id_token.value() == lpg::non_comment{lpg::identifier{"a"}});
}

BOOST_AUTO_TEST_CASE(scan_assign)
{
    auto s = lpg::scanner("=");
    std::optional<lpg::non_comment> const let_token = lpg::pop_next_non_comment(s);
    BOOST_TEST(!s.peek());
    BOOST_TEST(let_token.value() == lpg::non_comment{lpg::special_character::assign});
}
