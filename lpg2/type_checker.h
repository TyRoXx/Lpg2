#pragma once
#include "parser.h"

namespace lpg::semantics
{
    struct local_id final
    {
        size_t value;
    };

    enum class builtin_functions
    {
        print,
        equals_string
    };

    struct builtin final
    {
        local_id destination;
        builtin_functions function;
    };

    struct call final
    {
        local_id result;
        local_id callee;
        std::vector<local_id> arguments;
    };

    struct string_literal final
    {
        local_id destination;
        std::string value;
    };

    struct void_literal final
    {
        local_id destination;
    };

    struct poison final
    {
        local_id destination;
    };

    struct boolean_literal final
    {
        local_id destination;
        bool value;
    };

    struct sequence;

    using instruction = std::variant<builtin, call, string_literal, sequence, void_literal, poison, boolean_literal>;

    struct sequence final
    {
        std::vector<instruction> elements;
    };

    struct semantic_error final
    {
        std::string message;
        syntax::source_location location;

        std::weak_ordering operator<=>(semantic_error const &other) const noexcept = default;
    };

    std::ostream &operator<<(std::ostream &out, semantic_error const &error);

    using semantic_error_handler = std::function<void(semantic_error)>;

    [[nodiscard]] sequence check_types(syntax::sequence const &input, semantic_error_handler on_error);
} // namespace lpg::semantics
