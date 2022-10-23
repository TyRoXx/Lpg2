#define CATCH_CONFIG_MAIN
#include "lpg2/parser.h"
#include <catch2/catch_test_macros.hpp>

namespace
{
    void expect_compilation_error(const std::string program,
                                  const std::vector<lpg::syntax::parse_error> expected_errors,
                                  lpg::syntax::sequence const &expected_program)
    {
        std::vector<lpg::syntax::parse_error> error_messages;
        auto const on_error = [&error_messages](lpg::syntax::parse_error error) -> void {
            error_messages.push_back(std::move(error));
        };
        lpg::syntax::sequence output = lpg::syntax::compile(program, on_error);
        CHECK(expected_program == output);
        CHECK(expected_errors == error_messages);
    }
} // namespace

TEST_CASE("block_missing_closing_brace")
{
    std::vector<lpg::syntax::expression> block;
    block.emplace_back(lpg::syntax::expression{lpg::syntax::sequence{}});
    expect_compilation_error(
        "{",
        {lpg::syntax::parse_error{"Missing closing brace '}' before end of file", lpg::syntax::source_location{0, 1}}},
        lpg::syntax::sequence{std::move(block)});
}

TEST_CASE("invalid_string_position")
{
    expect_compilation_error(R"(let a "Hello world")",
                             {lpg::syntax::parse_error{"Expected something else", lpg::syntax::source_location{0, 6}}},
                             lpg::syntax::sequence{});
}

TEST_CASE("only_let")
{
    expect_compilation_error(
        "let",
        {lpg::syntax::parse_error{"Expected identifier but got end of stream", lpg::syntax::source_location{0, 3}}},
        lpg::syntax::sequence{});
}

TEST_CASE("let_followed_by_non_identifier")
{
    expect_compilation_error(
        "let =", {lpg::syntax::parse_error{"Expected identifier", lpg::syntax::source_location{0, 4}}},
        lpg::syntax::sequence{});
}

TEST_CASE("declaration_missing_assignment")
{
    expect_compilation_error("let a",
                             {lpg::syntax::parse_error{"Expected special character but got end of stream",
                                                       lpg::syntax::source_location{0, 5}}},
                             lpg::syntax::sequence{});
}

TEST_CASE("declaration_with_incorrect_operator")
{
    expect_compilation_error(
        "let a )",
        {lpg::syntax::parse_error{"Expected a different special character", lpg::syntax::source_location{0, 6}},
         lpg::syntax::parse_error{"Expected something else", lpg::syntax::source_location{0, 6}}},
        lpg::syntax::sequence{});
}

TEST_CASE("unterminated_string")
{
    expect_compilation_error(R"("Hello world)",
                             {lpg::syntax::parse_error{"Tokenization failed", lpg::syntax::source_location{0, 0}}},
                             lpg::syntax::sequence{});
}

TEST_CASE("mismatching_closing_parenthesis")
{
    expect_compilation_error(
        ")", {lpg::syntax::parse_error{"Can not have a closing parenthesis here.", lpg::syntax::source_location{0, 0}}},
        lpg::syntax::sequence{});
}

TEST_CASE("only_slash")
{
    expect_compilation_error(
        "/", {lpg::syntax::parse_error{"Can not have a slash here.", lpg::syntax::source_location{0, 0}}},
        lpg::syntax::sequence{});
}

TEST_CASE("line_beginning_with_assign_operator")
{
    expect_compilation_error(
        "=",
        {lpg::syntax::parse_error{"Can not have an assignment operator here.", lpg::syntax::source_location{0, 0}}},
        lpg::syntax::sequence{});
}

TEST_CASE("identifier_followed_by_special_character")
{
    expect_compilation_error(
        "a =",
        {lpg::syntax::parse_error{"Can not have an assignment operator here.", lpg::syntax::source_location{0, 2}}},
        lpg::syntax::sequence{});
}

TEST_CASE("identifier_followed_by_slash")
{
    expect_compilation_error(
        "a /", {lpg::syntax::parse_error{"Can not have a slash here.", lpg::syntax::source_location{0, 2}}},
        lpg::syntax::sequence{});
}

TEST_CASE("invalid_content_inside_parentheses")
{
    expect_compilation_error(
        "(a /)",
        {lpg::syntax::parse_error{"Can not have a slash here.", lpg::syntax::source_location{0, 3}},
         lpg::syntax::parse_error{"Can not have a slash here.", lpg::syntax::source_location{0, 3}}},
        lpg::syntax::sequence{});
}

TEST_CASE("parse_argument_error")
{
    expect_compilation_error(
        "f(",
        {lpg::syntax::parse_error{"Unexpected end of stream", lpg::syntax::source_location{0, 2}},
         lpg::syntax::parse_error{"Could not parse argument of the function", lpg::syntax::source_location{0, 1}}},
        lpg::syntax::sequence{});
}

TEST_CASE("missing_initializer_for_declaration")
{
    expect_compilation_error(
        R"(let a = )",
        {lpg::syntax::parse_error{"Unexpected end of stream", lpg::syntax::source_location{0, 8}},
         lpg::syntax::parse_error{"Invalid initializer value for identifier: a", lpg::syntax::source_location{0, 4}}},
        lpg::syntax::sequence{});
}

TEST_CASE("print_parse_error")
{
    std::ostringstream s;
    s << lpg::syntax::parse_error{"content", lpg::syntax::source_location{1, 2}};
    CHECK(s.str() == "2:3: content");
}
