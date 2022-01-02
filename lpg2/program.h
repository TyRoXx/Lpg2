#pragma once

#include "parser.h"
#include <map>
#include <optional>
#include <ostream>
#include <string_view>
#include <variant>

namespace lpg
{
    struct run_result
    {
        std::optional<std::string> output;

        explicit run_result(std::optional<std::string> output)
            : output(move(output))
        {
        }

        friend bool operator==(run_result const &left, run_result const &right)
        {
            return (left.output == right.output);
        }

        friend std::ostream &operator<<(std::ostream &out, run_result const &value)
        {
            if (value.output)
            {
                return out << "success: " << *value.output;
            }
            return out << "error";
        }
    };

    enum class builtin_functions
    {
        print
    };

    using value = std::variant<std::string, builtin_functions, std::nullptr_t>;
    using local_variable_map = std::map<std::string, value>;

    [[nodiscard]] run_result run(std::string_view source, std::function<void(parse_error)> on_error);
    [[nodiscard]] lpg::sequence compile(std::string_view source, std::function<void(parse_error)> on_error);
} // namespace lpg
