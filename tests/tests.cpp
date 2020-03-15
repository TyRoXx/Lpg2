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
}

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

BOOST_AUTO_TEST_CASE(unkown_function)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"aaa(hello("ABC"))aaa"), std::invalid_argument);
}
