#define CATCH_CONFIG_MAIN
#include "lpg2/parser.h"
#include <catch2/catch_test_macros.hpp>

namespace
{
    void expect_compilation_error(const std::string program, const std::vector<std::string> expected_errors,
                                  lpg::sequence const &expected_program)
    {
        std::vector<std::string> error_messages;
        auto const on_error = [&error_messages](lpg::parse_error error) -> void {
            error_messages.push_back(error.error_message);
        };
        lpg::sequence output = lpg::compile(program, on_error);
        CHECK(expected_program == output);
        CHECK(expected_errors == error_messages);
    }
} // namespace

TEST_CASE("block_missing_closing_brace")
{
    std::vector<lpg::expression> block;
    block.emplace_back(lpg::expression{lpg::sequence{}});
    expect_compilation_error("{", {"Missing closing brace '}' before end of file"}, lpg::sequence{std::move(block)});
}

TEST_CASE("invalid_string_position")
{
    expect_compilation_error(R"(let a "Hello world")", {"Expected something else"}, lpg::sequence{});
}

TEST_CASE("only_let")
{
    expect_compilation_error(
        "let", {"Expected identifier but got end of stream", "Expected variable name but found end of file"},
        lpg::sequence{});
}

TEST_CASE("let_followed_by_non_identifier")
{

    expect_compilation_error("let =", {"Expected variable name but found end of file"}, lpg::sequence{});
}

TEST_CASE("declaration_missing_assignment")
{
    expect_compilation_error("let a", {"Expected special character but got end of stream"}, lpg::sequence{});
}

TEST_CASE("declaration_with_incorrect_operator")
{
    expect_compilation_error(
        "let a )", {"Expected a different special character", "Expected something else"}, lpg::sequence{});
}

TEST_CASE("unterminated_string")
{
    expect_compilation_error(R"("Hello world)", {"Tokenization failed"}, lpg::sequence{});
}

TEST_CASE("mismatching_closing_parenthesis")
{
    expect_compilation_error(")", {"Can not have a closing parenthesis here."}, lpg::sequence{});
}

TEST_CASE("only_slash")
{
    expect_compilation_error("/", {"Can not have a slash here."}, lpg::sequence{});
}

TEST_CASE("line_beginning_with_assign_operator")
{
    expect_compilation_error("=", {"Can not have an assignment operator here."}, lpg::sequence{});
}

TEST_CASE("identifier_followed_by_special_character")
{
    expect_compilation_error("a =", {"Can not have an assignment operator here."}, lpg::sequence{});
}

TEST_CASE("identifier_followed_by_slash")
{
    expect_compilation_error("a /", {"Can not have a slash here."}, lpg::sequence{});
}

TEST_CASE("invalid_content_inside_parenthese")
{
    expect_compilation_error(
        "(a /)",
        {"Can not have a slash here.", "Can not parse expression inside parenthese.", "Can not have a slash here."},
        lpg::sequence{});
}

TEST_CASE("parse_argument_error")
{
    expect_compilation_error(
        "f(", {"Unexpected end of stream", "Could not parse argument of the function"}, lpg::sequence{});
}

TEST_CASE("missing_initializer_for_declaration")
{
    expect_compilation_error(
        R"(let a = )", {"Unexpected end of stream", "Invalid initializer value for identifier: a"}, lpg::sequence{});
}

TEST_CASE("print_parse_error")
{
    std::ostringstream s;
    s << lpg::parse_error{"content"};
    CHECK(s.str() == "content");
}
