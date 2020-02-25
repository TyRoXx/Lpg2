#pragma once

#include <optional>
#include <ostream>
#include <string_view>

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

    [[nodiscard]] run_result run(std::string_view source);
}
