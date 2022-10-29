#include "lpg2/formatter.h"
#include <catch2/catch_test_macros.hpp>

namespace
{
    void test_formatter_roundtrip(std::string_view const &source)
    {
        lpg::syntax::parser parser{
            lpg::syntax::scanner{source}, [](lpg::syntax::parse_error const &error) { FAIL(error); }};
        lpg::syntax::sequence const parsed = parser.parse_sequence(false, lpg::syntax::source_location{});
        std::ostringstream buffer;
        lpg::syntax::formatter formatter{buffer, 0};
        formatter.format(parsed);
        CHECK(source == buffer.str());
    }
} // namespace

TEST_CASE("format_empty_file")
{
    test_formatter_roundtrip("");
}

TEST_CASE("format_call")
{
    test_formatter_roundtrip("f()\n");
    test_formatter_roundtrip("f(a)\n");
    test_formatter_roundtrip("f(a, b)\n");
}

TEST_CASE("format_string")
{
    test_formatter_roundtrip("\"test\"\n");
}

TEST_CASE("format_declaration")
{
    test_formatter_roundtrip("let a = \"test\"\n");
}

TEST_CASE("format_keyword")
{
    test_formatter_roundtrip("true\n");
    test_formatter_roundtrip("false\n");
}

TEST_CASE("format_binary_expression")
{
    test_formatter_roundtrip("a == b\n");
}

TEST_CASE("format_binary_expression_literal")
{
    test_formatter_roundtrip("==\n");
}

TEST_CASE("format_block")
{
    test_formatter_roundtrip("let a = {\n"
                             "    print(\"test\")\n"
                             "}\n");
    test_formatter_roundtrip("let a = {\n"
                             "    {\n"
                             "        {\n"
                             "        }\n"
                             "        print(\"test\")\n"
                             "    }\n"
                             "}\n");
}
