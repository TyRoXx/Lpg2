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
        poison_reached,
        local_initialized_twice,
        read_uninitialized_local,
        not_callable,
        invalid_argument_type
    };

    struct evaluate_error
    {
        evaluate_error_type type;
    };

    bool operator==(evaluate_error const &left, evaluate_error const &right);
    std::ostream &operator<<(std::ostream &out, evaluate_error const &error);

    using run_result = std::variant<std::string, evaluate_error>;

    [[nodiscard]] run_result run(std::string_view source, std::function<void(syntax::parse_error)> on_syntax_error,
                                 semantics::semantic_error_handler on_semantic_error);
} // namespace lpg
