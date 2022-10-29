#include "lpg2/type_checker.h"
#include <catch2/catch_test_macros.hpp>

namespace
{
    void fail_on_parse_error(lpg::syntax::parse_error result)
    {
        FAIL(result);
    }

    void expect_semantic_errors(std::string_view const &source,
                                std::vector<lpg::semantics::semantic_error> const &expected_errors)
    {
        lpg::syntax::sequence const parsed = compile(source, fail_on_parse_error);
        std::vector<lpg::semantics::semantic_error> got_errors;
        lpg::semantics::sequence const checked =
            lpg::semantics::check_types(parsed, [&got_errors](lpg::semantics::semantic_error error) {
                got_errors.emplace_back(std::move(error));
            });
        CHECK(expected_errors == got_errors);
    }
} // namespace

TEST_CASE("unknown_function")
{
    expect_semantic_errors(
        R"aaa(hello("ABC"))aaa",
        {lpg::semantics::semantic_error{"Unknown identifier", lpg::syntax::source_location{0, 0}},
         lpg::semantics::semantic_error{"This value is not callable", lpg::syntax::source_location{0, 0}}});
}

TEST_CASE("unknown_argument")
{
    expect_semantic_errors(
        R"aaa(print(uuu))aaa",
        {lpg::semantics::semantic_error{"Unknown identifier", lpg::syntax::source_location{0, 6}},
         lpg::semantics::semantic_error{"Argument type mismatch", lpg::syntax::source_location{0, 6}}});
}

TEST_CASE("variable_redeclaration")
{
    expect_semantic_errors(R"(let a = "Hello world"
let a = "Hello world")",
                           {lpg::semantics::semantic_error{
                               "Local variable with this name already exists", lpg::syntax::source_location{1, 4}}});
}

TEST_CASE("argument_type_mismatch")
{
    expect_semantic_errors(R"aaa(print(print))aaa", {lpg::semantics::semantic_error{
                                                        "Argument type mismatch", lpg::syntax::source_location{0, 6}}});
}

TEST_CASE("not_comparable")
{
    expect_semantic_errors(
        R"aaa(
let b = true
let c = b == "string"
)aaa",
        {lpg::semantics::semantic_error{"These types are not comparable", lpg::syntax::source_location{2, 8}}});
}

TEST_CASE("not_callable")
{
    expect_semantic_errors(
        R"aaa(let a = "hello"
a("")
)aaa",
        {lpg::semantics::semantic_error{"This value is not callable", lpg::syntax::source_location{1, 0}}});
}
