#include "../lpg2/parser.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("scan_nothing")
{
    auto s = lpg::scanner("");
    CHECK(!s.peek());
}

TEST_CASE("scan_invalid_token")
{
    auto s = lpg::scanner("+");
    std::optional<lpg::token> const t = s.peek();
    CHECK(!t.has_value());
}

TEST_CASE("scan_peek_token")
{
    auto s = lpg::scanner("\"Hello\"");
    std::optional<lpg::token> const t = s.peek();
    CHECK(t.has_value());
    lpg::string_literal literal = std::get<lpg::string_literal>(t.value());
    CHECK(literal.inner_content == "Hello");
}

TEST_CASE("scan_string")
{
    auto s = lpg::scanner("\"Hello\"");
    std::optional<lpg::token> const t = s.pop();
    CHECK(t.has_value());
    lpg::string_literal const string = std::get<lpg::string_literal>(t.value());
    CHECK(string.inner_content == "Hello");
}

TEST_CASE("scan_invalid_string")
{
    auto s = lpg::scanner("\"Hello");

    std::optional<lpg::token> t;
    CHECK_THROWS_AS(t = s.pop(), std::invalid_argument);
    CHECK(!t.has_value());
}

TEST_CASE("scan_parentheses")
{
    auto s = lpg::scanner("()");
    std::optional<lpg::token> const first_paren = s.pop();

    CHECK(s.peek());
    CHECK(first_paren.has_value());
    lpg::special_character const character = std::get<lpg::special_character>(first_paren.value());
    CHECK(character == lpg::special_character::left_parenthesis);

    auto const second_paren = s.pop();

    CHECK(!s.peek());
    CHECK(second_paren.has_value());
    lpg::special_character const character1 = std::get<lpg::special_character>(second_paren.value());
    CHECK(character1 == lpg::special_character::right_parenthesis);
}

TEST_CASE("scan_identifier")
{
    auto s = lpg::scanner("test");
    std::optional<lpg::token> const t = s.pop();
    CHECK(!s.peek());
    CHECK(t.has_value());
    lpg::identifier id = std::get<lpg::identifier>(t.value());
    CHECK(id.content == "test");
}

TEST_CASE("scan_slash")
{
    auto s = lpg::scanner("/ 2");
    std::optional<lpg::token> const t = s.pop();
    CHECK(!s.peek());
    CHECK(t.has_value());
    lpg::special_character const slash = std::get<lpg::special_character>(t.value());
    CHECK(slash == lpg::special_character::slash);
}

TEST_CASE("scan_slash_end_of_file")
{
    auto s = lpg::scanner("/");
    std::optional<lpg::token> const t = s.pop();
    CHECK(!s.peek());
    CHECK(t.has_value());
    lpg::special_character const slash = std::get<lpg::special_character>(t.value());
    CHECK(slash == lpg::special_character::slash);
}

TEST_CASE("scan_comment_end_of_file")
{
    auto s = lpg::scanner("//Just a comment");
    std::optional<lpg::token> const t = s.pop();
    CHECK(!s.peek());
    CHECK(t.has_value());

    lpg::comment comment = std::get<lpg::comment>(t.value());
    CHECK(comment.inner_content == "Just a comment");
}

TEST_CASE("scan_comment_end_of_line")
{
    auto s = lpg::scanner("//Just a comment\n");
    std::optional<lpg::non_comment> const t = lpg::peek_next_non_comment(s);
    CHECK(!s.peek());
    CHECK(!t.has_value());
}

TEST_CASE("scan_comment")
{
    auto s = lpg::scanner("//Just a comment\ntest");
    std::optional<lpg::non_comment> const t = lpg::peek_next_non_comment(s);
    CHECK(s.peek());
    CHECK(t.has_value());

    lpg::identifier id = std::get<lpg::identifier>(t.value());
    CHECK(id.content == "test");
}

TEST_CASE("print_special_character")
{
    std::ostringstream s;
    s << lpg::special_character::left_parenthesis;
    CHECK(s.str() == "0");
}

TEST_CASE("print_identifier")
{
    std::ostringstream s;
    s << lpg::identifier{"name"};
    CHECK(s.str() == "name");
}

TEST_CASE("print_string_literal")
{
    std::ostringstream s;
    s << lpg::string_literal{"content"};
    CHECK(s.str() == "\"content\"");
}

TEST_CASE("print_comment")
{
    std::ostringstream s;
    s << lpg::comment{"content"};
    CHECK(s.str() == "/*content*/");
}

TEST_CASE("ignore_spaces")
{
    auto s = lpg::scanner("let a");
    std::optional<lpg::non_comment> const let_token = lpg::pop_next_non_comment(s);
    CHECK(s.peek());
    CHECK(let_token.value() == lpg::non_comment{lpg::identifier{"let"}});

    std::optional<lpg::non_comment> const id_token = lpg::pop_next_non_comment(s);
    CHECK(!s.peek());
    CHECK(id_token.value() == lpg::non_comment{lpg::identifier{"a"}});
}

TEST_CASE("scan_assign")
{
    auto s = lpg::scanner("=");
    std::optional<lpg::non_comment> const let_token = lpg::pop_next_non_comment(s);
    CHECK(!s.peek());
    CHECK(let_token.value() == lpg::non_comment{lpg::special_character::assign});
}
