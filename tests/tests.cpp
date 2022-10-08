#define CATCH_CONFIG_MAIN
#include "lpg2/tokenizer.h"
#include <catch2/catch_test_macros.hpp>

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
