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

    void expect_compilation_error(const std::string program, const std::vector<std::string> expected_errors)
    {
        std::vector<std::string> error_messages;
        auto const on_error = [&error_messages](lpg::parse_error error) -> void {
            error_messages.push_back(error.error_message);
        };
        lpg::sequence output = lpg::compile(program, on_error);
        BOOST_TEST(output.elements.empty());
        BOOST_CHECK_EQUAL_COLLECTIONS(
            expected_errors.begin(), expected_errors.end(), error_messages.begin(), error_messages.end());
    }

    void fail_on_error(lpg::parse_error result)
    {
        BOOST_FAIL(result);
    }
} // namespace

BOOST_AUTO_TEST_CASE(print_run_result)
{
    BOOST_TEST(format(lpg::run_result{""}) == "success: ");
    BOOST_TEST(format(lpg::run_result{std::nullopt}) == "error");
}

BOOST_AUTO_TEST_CASE(empty)
{
    BOOST_TEST(lpg::run_result{""} == lpg::run("", fail_on_error));
}

BOOST_AUTO_TEST_CASE(print_nothing)
{
    BOOST_TEST(lpg::run_result{""} == lpg::run(R"aaa(print(""))aaa", fail_on_error));
}

BOOST_AUTO_TEST_CASE(print_hello_world)
{
    BOOST_TEST(lpg::run_result{"Hello, world!"} == lpg::run(R"aaa(print("Hello, world!"))aaa", fail_on_error));
}

BOOST_AUTO_TEST_CASE(print_twice)
{
    BOOST_TEST(lpg::run_result{"ab"} == lpg::run(R"aaa(print("a")print("b"))aaa", fail_on_error));
}

BOOST_AUTO_TEST_CASE(unknown_function)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"aaa(hello("ABC"))aaa", fail_on_error), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(argument_type_mismatch)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"aaa(print(print))aaa", fail_on_error), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(parentheses)
{
    BOOST_TEST(lpg::run_result{"Hello, world!"} == lpg::run(R"aaa((print("Hello, world!")))aaa", fail_on_error));
}

BOOST_AUTO_TEST_CASE(variable_declaration)
{
    BOOST_TEST(lpg::run_result{"Declaring a\n"} == lpg::run(R"(let a = "Hello world")", fail_on_error));
}

BOOST_AUTO_TEST_CASE(variable_access)
{
    BOOST_TEST(lpg::run_result{"Declaring a\nHello world"} == lpg::run(R"(let a = "Hello world"
print(a))",
                                                                       fail_on_error));
}

BOOST_AUTO_TEST_CASE(variable_redeclaration)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"(let a = "Hello world"
let a = "Hello world")",
                                        fail_on_error),
                      std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(invalid_string_position)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"(let a "Hello world")", fail_on_error), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(unterminated_string)
{
    BOOST_CHECK_THROW(auto a = lpg::run(R"("Hello world)", fail_on_error), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(mismatching_closing_parenthesis)
{
    BOOST_CHECK_THROW(auto a = lpg::run(")", fail_on_error), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(only_slash)
{
    BOOST_CHECK_THROW(auto a = lpg::run("/", fail_on_error), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(line_beginning_with_assign_operator)
{
    BOOST_CHECK_THROW(auto a = lpg::run("=", fail_on_error), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(identifier_followed_by_special_character)
{
    expect_compilation_error("a =", {"Can not have an assignment operator here."});
}

BOOST_AUTO_TEST_CASE(identifier_followed_by_slash)
{
    expect_compilation_error("a /", {"Can not have a slash here."});
}

BOOST_AUTO_TEST_CASE(invalid_content_inside_parenthese)
{
    expect_compilation_error("(a /)", {"Can not have a slash here.", "Can not parse expression inside parenthese.",
                                       "Can not have a slash here."});
}

BOOST_AUTO_TEST_CASE(parse_argument_error)
{
    expect_compilation_error("f(", {"Unexpected end of stream", "Could not parse argument of the function"});
}

BOOST_AUTO_TEST_CASE(missing_initializer_for_declaration)
{
    expect_compilation_error(
        R"(let a = )", {"Unexpected end of stream", "Invalid initializer value for identifier: a"});
}

BOOST_AUTO_TEST_CASE(trailing_new_line)
{
    BOOST_TEST(lpg::run_result{"Declaring a\nHello world"} == lpg::run(R"(let a = "Hello world"
print(a)
)",
                                                                       fail_on_error));
}

BOOST_AUTO_TEST_CASE(variant_printing)
{
    std::ostringstream buffer;
    buffer << std::variant<int, float>(12);
    BOOST_TEST("0: 12" == buffer.str());
}

BOOST_AUTO_TEST_CASE(optional_printing)
{
    std::ostringstream buffer;
    buffer << std::optional<int>(12);
    BOOST_TEST("12" == buffer.str());
}

BOOST_AUTO_TEST_CASE(optional_printing_nullopt)
{
    std::ostringstream buffer;
    buffer << std::optional<int>();
    BOOST_TEST("nullopt" == buffer.str());
}

BOOST_AUTO_TEST_CASE(print_parse_error)
{
    std::ostringstream s;
    s << lpg::parse_error{"content"};
    BOOST_TEST(s.str() == "content");
}
