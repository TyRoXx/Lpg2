#define CATCH_CONFIG_MAIN
#include "lpg2/interpreter.h"
#include <catch2/catch_test_macros.hpp>

namespace
{
    void fail_on_parse_error(lpg::syntax::parse_error result)
    {
        FAIL(result);
    }

    void fail_on_semantic_error(lpg::semantics::semantic_error result)
    {
        FAIL(result);
    }
} // namespace

TEST_CASE("print_run_result")
{
    CHECK(format(lpg::run_result{""}) == "0: ");
    CHECK(format(lpg::run_result{lpg::evaluate_error(lpg::evaluate_error_type::local_initialized_twice)}) == "1: 1");
}

TEST_CASE("empty")
{
    CHECK(lpg::run_result{""} == lpg::run("", fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("print_nothing")
{
    CHECK(lpg::run_result{""} == lpg::run(R"aaa(print(""))aaa", fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("print_hello_world")
{
    CHECK(lpg::run_result{"Hello, world!"} ==
          lpg::run(R"aaa(print("Hello, world!"))aaa", fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("print_twice")
{
    CHECK(lpg::run_result{"ab"} ==
          lpg::run(R"aaa(print("a")print("b"))aaa", fail_on_parse_error, fail_on_semantic_error));
}

namespace
{
    void expect_semantic_errors(std::string_view const &source,
                                std::vector<lpg::semantics::semantic_error> const &expected_errors)
    {
        lpg::syntax::sequence const parsed = compile(source, fail_on_parse_error);
        std::vector<lpg::semantics::semantic_error> got_errors;
        lpg::semantics::sequence const checked = lpg::semantics::check_types(
            parsed, [&got_errors](lpg::semantics::semantic_error error) { got_errors.emplace_back(std::move(error)); });
        CHECK(expected_errors == got_errors);
    }
} // namespace

TEST_CASE("unknown_function")
{
    expect_semantic_errors(R"aaa(hello("ABC"))aaa",
                           {lpg::semantics::semantic_error{"Unknown identifier", lpg::syntax::source_location{0, 0}}});
}

TEST_CASE("unknown_argument")
{
    expect_semantic_errors(R"aaa(print(uuu))aaa",
                           {lpg::semantics::semantic_error{"Unknown identifier", lpg::syntax::source_location{0, 6}}});
}

TEST_CASE("argument_type_mismatch")
{
    // TODO
    // CHECK(lpg::run_result{lpg::evaluate_error{lpg::evaluate_error_type::not_callable}} ==
    // lpg::run(R"aaa(print(print))aaa", fail_on_error));
}

TEST_CASE("parentheses")
{
    CHECK(lpg::run_result{"Hello, world!"} ==
          lpg::run(R"aaa((print("Hello, world!")))aaa", fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("variable_declaration")
{
    CHECK(lpg::run_result{""} == lpg::run(R"(let a = "Hello world")", fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("variable_access")
{
    CHECK(lpg::run_result{"Hello world"} == lpg::run(R"(let a = "Hello world"
print(a))",
                                                     fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("block_empty")
{
    CHECK(lpg::run_result{""} == lpg::run(R"({})", fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("block_non_empty")
{
    CHECK(lpg::run_result{"hello"} == lpg::run(R"({print("hello")})", fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("block_nested_simple")
{
    CHECK(lpg::run_result{"hello"} == lpg::run(R"({{print("hello")}})", fail_on_parse_error, fail_on_semantic_error));
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
                                             fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("block_returns_value")
{
    CHECK(lpg::run_result{"ab"} == lpg::run(R"(
let b = {
    print("a")
    "b"
}
print(b))",
                                            fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("void_variable")
{
    CHECK(lpg::run_result{""} == lpg::run(R"(
let v = {}
)",
                                          fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("variable_redeclaration")
{
    expect_semantic_errors(R"(let a = "Hello world"
let a = "Hello world")",
                           {lpg::semantics::semantic_error{
                               "Local variable with this name already exists", lpg::syntax::source_location{1, 4}}});
}

TEST_CASE("trailing_new_line")
{
    CHECK(lpg::run_result{"Hello world"} == lpg::run(R"(let a = "Hello world"
print(a)
)",
                                                     fail_on_parse_error, fail_on_semantic_error));
}
