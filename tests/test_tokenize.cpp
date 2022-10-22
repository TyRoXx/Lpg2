#include "../lpg2/parser.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("scan_nothing")
{
    auto s = lpg::scanner("");
    CHECK(!s.peek());
    CHECK(!s.has_failed);
}

TEST_CASE("scan_invalid_token")
{
    auto s = lpg::scanner("+");
    std::optional<lpg::token> const t = s.peek();
    CHECK(!t.has_value());
    CHECK(!s.has_failed);
}

TEST_CASE("scan_peek_token")
{
    auto s = lpg::scanner("\"Hello\"");
    lpg::token const t = s.peek().value();
    CHECK(lpg::source_location(0, 0) == t.location);
    lpg::string_literal literal = std::get<lpg::string_literal>(t.content);
    CHECK(literal.inner_content == "Hello");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_string")
{
    auto s = lpg::scanner("\"Hello\"");
    lpg::token const t = s.pop().value();
    CHECK(lpg::source_location(0, 0) == t.location);
    lpg::string_literal const string = std::get<lpg::string_literal>(t.content);
    CHECK(string.inner_content == "Hello");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_invalid_string")
{
    auto s = lpg::scanner("\"Hello");
    CHECK(std::nullopt == s.pop());
    CHECK(s.has_failed);
}

TEST_CASE("scan_new_line")
{
    auto s = lpg::scanner("\n\"Hello\"");
    lpg::token const t = s.pop().value();
    CHECK(lpg::source_location(1, 0) == t.location);
    lpg::string_literal const string = std::get<lpg::string_literal>(t.content);
    CHECK(string.inner_content == "Hello");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_parentheses")
{
    auto s = lpg::scanner("()");
    lpg::token const first_paren = s.pop().value();
    CHECK(lpg::source_location(0, 0) == first_paren.location);

    CHECK(s.peek());
    lpg::special_character const character = std::get<lpg::special_character>(first_paren.content);
    CHECK(character == lpg::special_character::left_parenthesis);

    lpg::token const second_paren = s.pop().value();
    CHECK(lpg::source_location(0, 1) == second_paren.location);

    CHECK(!s.peek());
    lpg::special_character const character1 = std::get<lpg::special_character>(second_paren.content);
    CHECK(character1 == lpg::special_character::right_parenthesis);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_identifier")
{
    auto s = lpg::scanner("test");
    lpg::token const t = s.pop().value();
    CHECK(lpg::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::identifier id = std::get<lpg::identifier>(t.content);
    CHECK(id.content == "test");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_slash")
{
    auto s = lpg::scanner("/ 2");
    lpg::token const t = s.pop().value();
    CHECK(lpg::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::special_character const slash = std::get<lpg::special_character>(t.content);
    CHECK(slash == lpg::special_character::slash);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_slash_end_of_file")
{
    auto s = lpg::scanner("/");
    lpg::token const t = s.pop().value();
    CHECK(lpg::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::special_character const slash = std::get<lpg::special_character>(t.content);
    CHECK(slash == lpg::special_character::slash);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_comment_end_of_file")
{
    auto s = lpg::scanner("//Just a comment");
    lpg::token const t = s.pop().value();
    CHECK(lpg::source_location(0, 0) == t.location);
    CHECK(!s.peek());

    lpg::comment comment = std::get<lpg::comment>(t.content);
    CHECK(comment.inner_content == "Just a comment");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_comment_end_of_line")
{
    auto s = lpg::scanner("//Just a comment\n");
    std::optional<lpg::non_comment> const t = lpg::peek_next_non_comment(s);
    CHECK(!s.peek());
    CHECK(!t.has_value());
    CHECK(!s.has_failed);
}

TEST_CASE("scan_comment")
{
    auto s = lpg::scanner("//Just a comment\ntest");
    lpg::non_comment const t = lpg::peek_next_non_comment(s).value();
    CHECK(lpg::source_location(0, 16) == t.location);
    CHECK(s.peek());

    lpg::identifier id = std::get<lpg::identifier>(t.content);
    CHECK(id.content == "test");
    CHECK(!s.has_failed);
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
    lpg::non_comment const let_token = lpg::pop_next_non_comment(s).value();
    CHECK(s.peek());
    CHECK(let_token == lpg::non_comment{lpg::identifier{"let"}, lpg::source_location{0, 0}});

    lpg::non_comment const id_token = lpg::pop_next_non_comment(s).value();
    CHECK(!s.peek());
    CHECK(id_token == lpg::non_comment{lpg::identifier{"a"}, lpg::source_location{0, 3}});
    CHECK(!s.has_failed);
}

TEST_CASE("scan_assign")
{
    auto s = lpg::scanner("=");
    lpg::non_comment const let_token = lpg::pop_next_non_comment(s).value();
    CHECK(!s.peek());
    CHECK(let_token == lpg::non_comment{lpg::special_character::assign, lpg::source_location{0, 0}});
    CHECK(!s.has_failed);
}
