#define CATCH_CONFIG_MAIN
#include "lpg2/program.h"
#include <catch2/catch_test_macros.hpp>

namespace
{
    template <class T>
    std::string format(T const &value)
    {
        std::ostringstream buffer;
        buffer << value;
        return buffer.str();
    }

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

    void fail_on_error(lpg::parse_error result)
    {
        FAIL(result);
    }
} // namespace

TEST_CASE("print_run_result")
{
    CHECK(format(lpg::run_result{""}) == "0: ");
    CHECK(format(lpg::run_result{lpg::evaluate_error(lpg::evaluate_error_type::not_callable)}) == "1: 0");
}

TEST_CASE("empty")
{
    CHECK(lpg::run_result{""} == lpg::run("", fail_on_error));
}

TEST_CASE("print_nothing")
{
    CHECK(lpg::run_result{""} == lpg::run(R"aaa(print(""))aaa", fail_on_error));
}

TEST_CASE("print_hello_world")
{
    CHECK(lpg::run_result{"Hello, world!"} == lpg::run(R"aaa(print("Hello, world!"))aaa", fail_on_error));
}

TEST_CASE("print_twice")
{
    CHECK(lpg::run_result{"ab"} == lpg::run(R"aaa(print("a")print("b"))aaa", fail_on_error));
}

TEST_CASE("unknown_function")
{
    CHECK_THROWS_AS(lpg::run(R"aaa(hello("ABC"))aaa", fail_on_error), std::invalid_argument);
}

TEST_CASE("argument_type_mismatch")
{
    CHECK(lpg::run_result{lpg::evaluate_error{lpg::evaluate_error_type::not_callable}} ==
          lpg::run(R"aaa(print(print))aaa", fail_on_error));
}

TEST_CASE("parentheses")
{
    CHECK(lpg::run_result{"Hello, world!"} == lpg::run(R"aaa((print("Hello, world!")))aaa", fail_on_error));
}

TEST_CASE("variable_declaration")
{
    CHECK(lpg::run_result{"Declaring a\n"} == lpg::run(R"(let a = "Hello world")", fail_on_error));
}

TEST_CASE("variable_access")
{
    CHECK(lpg::run_result{"Declaring a\nHello world"} == lpg::run(R"(let a = "Hello world"
print(a))",
                                                                  fail_on_error));
}

TEST_CASE("block_empty")
{
    CHECK(lpg::run_result{""} == lpg::run(R"({})", fail_on_error));
}

TEST_CASE("block_non_empty")
{
    CHECK(lpg::run_result{"hello"} == lpg::run(R"({print("hello")})", fail_on_error));
}

TEST_CASE("block_nested_simple")
{
    CHECK(lpg::run_result{"hello"} == lpg::run(R"({{print("hello")}})", fail_on_error));
}

TEST_CASE("block_nested_complex")
{
    CHECK(lpg::run_result{"abc"} == lpg::run(R"(
{
    print("a")
    {
        print("b")
        {}
    }
    print("c")
})",
                                             fail_on_error));
}

TEST_CASE("block_missing_closing_brace")
{
    std::vector<lpg::expression> block;
    block.emplace_back(lpg::expression{lpg::sequence{}});
    expect_compilation_error("{", {"Missing closing brace '}' before end of file"}, lpg::sequence{std::move(block)});
}

TEST_CASE("variable_redeclaration")
{
    CHECK_THROWS_AS(lpg::run(R"(let a = "Hello world"
let a = "Hello world")",
                             fail_on_error),
                    std::invalid_argument);
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
    CHECK_THROWS_AS(lpg::run(R"("Hello world)", fail_on_error), std::invalid_argument);
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

TEST_CASE("trailing_new_line")
{
    CHECK(lpg::run_result{"Declaring a\nHello world"} == lpg::run(R"(let a = "Hello world"
print(a)
)",
                                                                  fail_on_error));
}

TEST_CASE("variant_printing")
{
    std::ostringstream buffer;
    buffer << std::variant<int, float>(12);
    CHECK("0: 12" == buffer.str());
}

TEST_CASE("optional_printing")
{
    std::ostringstream buffer;
    buffer << std::optional<int>(12);
    CHECK("12" == buffer.str());
}

TEST_CASE("optional_printing_nullopt")
{
    std::ostringstream buffer;
    buffer << std::optional<int>();
    CHECK("nullopt" == buffer.str());
}

TEST_CASE("print_parse_error")
{
    std::ostringstream s;
    s << lpg::parse_error{"content"};
    CHECK(s.str() == "content");
}
