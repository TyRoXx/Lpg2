#include "../lpg2/parser.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("scan_nothing")
{
    auto s = lpg::syntax::scanner("");
    CHECK(!s.peek());
    CHECK(!s.has_failed);
}

TEST_CASE("scan_invalid_token")
{
    auto s = lpg::syntax::scanner("+");
    std::optional<lpg::syntax::token> const t = s.peek();
    CHECK(!t.has_value());
    CHECK(!s.has_failed);
}

TEST_CASE("scan_peek_token")
{
    auto s = lpg::syntax::scanner("\"Hello\"");
    lpg::syntax::token const t = s.peek().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    lpg::syntax::string_literal literal = std::get<lpg::syntax::string_literal>(t.content);
    CHECK(literal.inner_content == "Hello");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_string")
{
    auto s = lpg::syntax::scanner("\"Hello\"");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    lpg::syntax::string_literal const string = std::get<lpg::syntax::string_literal>(t.content);
    CHECK(string.inner_content == "Hello");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_invalid_string")
{
    auto s = lpg::syntax::scanner("\"Hello");
    CHECK(std::nullopt == s.pop());
    CHECK(s.has_failed);
}

TEST_CASE("scan_new_line")
{
    auto s = lpg::syntax::scanner("\n\"Hello\"");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(1, 0) == t.location);
    lpg::syntax::string_literal const string = std::get<lpg::syntax::string_literal>(t.content);
    CHECK(string.inner_content == "Hello");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_parentheses")
{
    auto s = lpg::syntax::scanner("()");
    lpg::syntax::token const first_paren = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == first_paren.location);

    CHECK(s.peek());
    lpg::syntax::special_character const character = std::get<lpg::syntax::special_character>(first_paren.content);
    CHECK(character == lpg::syntax::special_character::left_parenthesis);

    lpg::syntax::token const second_paren = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 1) == second_paren.location);

    CHECK(!s.peek());
    lpg::syntax::special_character const character1 = std::get<lpg::syntax::special_character>(second_paren.content);
    CHECK(character1 == lpg::syntax::special_character::right_parenthesis);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_identifier")
{
    auto s = lpg::syntax::scanner("test");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::syntax::identifier_token id = std::get<lpg::syntax::identifier_token>(t.content);
    CHECK(id.content == "test");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_true")
{
    auto s = lpg::syntax::scanner("true");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::syntax::keyword const keyword = std::get<lpg::syntax::keyword>(t.content);
    CHECK(keyword == lpg::syntax::keyword::true_);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_false")
{
    auto s = lpg::syntax::scanner("false");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::syntax::keyword const keyword = std::get<lpg::syntax::keyword>(t.content);
    CHECK(keyword == lpg::syntax::keyword::false_);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_slash")
{
    auto s = lpg::syntax::scanner("/ 2");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::syntax::special_character const slash = std::get<lpg::syntax::special_character>(t.content);
    CHECK(slash == lpg::syntax::special_character::slash);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_equals")
{
    auto s = lpg::syntax::scanner("== 2");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::syntax::special_character const equals = std::get<lpg::syntax::special_character>(t.content);
    CHECK(equals == lpg::syntax::special_character::equals);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_slash_end_of_file")
{
    auto s = lpg::syntax::scanner("/");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    CHECK(!s.peek());
    lpg::syntax::special_character const slash = std::get<lpg::syntax::special_character>(t.content);
    CHECK(slash == lpg::syntax::special_character::slash);
    CHECK(!s.has_failed);
}

TEST_CASE("scan_comment_end_of_file")
{
    auto s = lpg::syntax::scanner("//Just a comment");
    lpg::syntax::token const t = s.pop().value();
    CHECK(lpg::syntax::source_location(0, 0) == t.location);
    CHECK(!s.peek());

    lpg::syntax::comment comment = std::get<lpg::syntax::comment>(t.content);
    CHECK(comment.inner_content == "Just a comment");
    CHECK(!s.has_failed);
}

TEST_CASE("scan_comment_end_of_line")
{
    auto s = lpg::syntax::scanner("//Just a comment\n");
    std::optional<lpg::syntax::non_comment> const t = peek_next_non_comment(s);
    CHECK(!s.peek());
    CHECK(!t.has_value());
    CHECK(!s.has_failed);
}

TEST_CASE("scan_comment")
{
    auto s = lpg::syntax::scanner("//Just a comment\ntest");
    lpg::syntax::non_comment const t = peek_next_non_comment(s).value();
    CHECK(lpg::syntax::source_location(0, 16) == t.location);
    CHECK(s.peek());

    lpg::syntax::identifier_token id = std::get<lpg::syntax::identifier_token>(t.content);
    CHECK(id.content == "test");
    CHECK(!s.has_failed);
}

TEST_CASE("print_special_character")
{
    std::ostringstream s;
    s << lpg::syntax::special_character::left_parenthesis;
    CHECK(s.str() == "0");
}

TEST_CASE("print_identifier_token")
{
    std::ostringstream s;
    s << lpg::syntax::identifier_token{"name"};
    CHECK(s.str() == "name");
}

TEST_CASE("print_string_literal")
{
    std::ostringstream s;
    s << lpg::syntax::string_literal{"content"};
    CHECK(s.str() == "\"content\"");
}

TEST_CASE("print_comment")
{
    std::ostringstream s;
    s << lpg::syntax::comment{"content"};
    CHECK(s.str() == "/*content*/");
}

TEST_CASE("ignore_spaces")
{
    auto s = lpg::syntax::scanner("let a");
    lpg::syntax::non_comment const let_token = pop_next_non_comment(s).value();
    CHECK(s.peek());
    CHECK(let_token ==
          lpg::syntax::non_comment{lpg::syntax::identifier_token{"let"}, lpg::syntax::source_location{0, 0}});

    lpg::syntax::non_comment const id_token = pop_next_non_comment(s).value();
    CHECK(!s.peek());
    CHECK(id_token == lpg::syntax::non_comment{lpg::syntax::identifier_token{"a"}, lpg::syntax::source_location{0, 4}});
    CHECK(!s.has_failed);
}

TEST_CASE("scan_assign")
{
    auto s = lpg::syntax::scanner("=");
    lpg::syntax::non_comment const let_token = pop_next_non_comment(s).value();
    CHECK(!s.peek());
    CHECK(let_token ==
          lpg::syntax::non_comment{lpg::syntax::special_character::assign, lpg::syntax::source_location{0, 0}});
    CHECK(!s.has_failed);
}
