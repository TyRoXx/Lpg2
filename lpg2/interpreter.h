#pragma once

#include "parser.h"
#include <map>
#include <optional>
#include <ostream>
#include <string_view>
#include <variant>

namespace lpg
{
    enum class evaluate_error_type
    {
        not_callable,
        unknown_identifier,
        redeclaration
    };

    struct evaluate_error
    {
        evaluate_error_type type;
        std::string identifier;
    };

    bool operator==(evaluate_error const &left, evaluate_error const &right);
    std::ostream &operator<<(std::ostream &out, evaluate_error const &error);

    using run_result = std::variant<std::string, evaluate_error>;

    enum class builtin_functions
    {
        print
    };

    using value = std::variant<std::string, builtin_functions, std::nullptr_t>;
    using local_variable_map = std::map<std::string, value>;

    [[nodiscard]] run_result run(std::string_view source, std::function<void(parse_error)> on_error);
} // namespace lpg
