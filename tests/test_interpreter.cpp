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

TEST_CASE("function_pointer")
{
    CHECK(lpg::run_result{"Hello, world!"} == lpg::run(R"aaa(
let p = print
print("Hello, world!")
)aaa",
                                                       fail_on_parse_error, fail_on_semantic_error));
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

TEST_CASE("trailing_new_line")
{
    CHECK(lpg::run_result{"Hello world"} == lpg::run(R"(let a = "Hello world"
print(a)
)",
                                                     fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("true")
{
    CHECK(lpg::run_result{""} == lpg::run(R"(
let b = true
)",
                                          fail_on_parse_error, fail_on_semantic_error));
}

TEST_CASE("false")
{
    CHECK(lpg::run_result{""} == lpg::run(R"(
let b = false
)",
                                          fail_on_parse_error, fail_on_semantic_error));
}
