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

    void fail_on_error(lpg::parse_error result)
    {
        FAIL(result);
    }
} // namespace

TEST_CASE("print_run_result")
{
    CHECK(format(lpg::run_result{""}) == "0: ");
    CHECK(format(lpg::run_result{lpg::evaluate_error(lpg::evaluate_error_type::not_callable)}) == "1: 0 ");
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
    CHECK(lpg::run_result{lpg::evaluate_error{lpg::evaluate_error_type::unknown_identifier, "hello"}} ==
          lpg::run(R"aaa(hello("ABC"))aaa", fail_on_error));
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

TEST_CASE("variable_redeclaration")
{
    CHECK(lpg::run_result{lpg::evaluate_error{lpg::evaluate_error_type::redeclaration, "a"}} ==
          lpg::run(R"(let a = "Hello world"
let a = "Hello world")",
                   fail_on_error));
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
