#define BOOST_TEST_MAIN
#include "lpg2/program.h"
#include <boost/test/unit_test.hpp>

namespace
{
    template <class T>
    std::string format(T const &value)
    {
        std::ostringstream buffer;
        buffer << value;
        return buffer.str();
    }
} // namespace

BOOST_AUTO_TEST_CASE(print_run_result)
{
    BOOST_TEST(format(lpg::run_result{""}) == "success: ");
    BOOST_TEST(format(lpg::run_result{std::nullopt}) == "error");
}

BOOST_AUTO_TEST_CASE(empty)
{
    BOOST_TEST(lpg::run_result{""} == lpg::run(""));
}

BOOST_AUTO_TEST_CASE(print_nothing)
{
    BOOST_TEST(lpg::run_result{""} == lpg::run(R"aaa(print(""))aaa"));
}

BOOST_AUTO_TEST_CASE(print_hello_world)
{
    BOOST_TEST(lpg::run_result{"Hello, world!"} == lpg::run(R"aaa(print("Hello, world!"))aaa"));
}

BOOST_AUTO_TEST_CASE(print_twice)
{
    BOOST_TEST(lpg::run_result{"ab"} == lpg::run(R"aaa(print("a")print("b"))aaa"));
}

BOOST_AUTO_TEST_CASE(unknown_function)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"aaa(hello("ABC"))aaa"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(argument_type_mismatch)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"aaa(print(print))aaa"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(parentheses)
{
    BOOST_TEST(lpg::run_result{"Hello, world!"} == lpg::run(R"aaa((print("Hello, world!")))aaa"));
}

BOOST_AUTO_TEST_CASE(variable_declaration)
{
    BOOST_TEST(lpg::run_result{"Declaring a\n"} == lpg::run(R"(let a = "Hello world")"));
}

BOOST_AUTO_TEST_CASE(variable_access)
{
    BOOST_TEST(lpg::run_result{"Declaring a\nHello world"} == lpg::run(R"(let a = "Hello world"
print(a))"));
}

BOOST_AUTO_TEST_CASE(variable_redeclaration)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"(let a = "Hello world"
let a = "Hello world")"),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(trailing_new_line)
{
    BOOST_TEST(lpg::run_result{"Declaring a\nHello world"} == lpg::run(R"(let a = "Hello world"
print(a)
)"));
}

BOOST_AUTO_TEST_CASE(variant_printing)
{
    using lpg::operator<<;
    std::ostringstream buffer;
    buffer << std::variant<int, float>(12);
    BOOST_TEST("0: 12" == buffer.str());
}
