#pragma once
#include "type_checker.h"
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

    struct void_
    {
    };

    using value = std::variant<std::string, semantics::builtin_functions, void_>;
    using local_variable_map = std::map<std::string, value>;

    [[nodiscard]] run_result run(std::string_view source, std::function<void(syntax::parse_error)> on_error);
} // namespace lpg
