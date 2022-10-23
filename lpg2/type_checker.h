#pragma once
#include "parser.h"

namespace lpg::semantics
{
    struct local_id final
    {
        uint32_t value;
    };

    enum class builtin_functions
    {
        print
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
        local_id argument;
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

    struct sequence;

    using instruction = std::variant<builtin, call, string_literal, sequence, void_literal, poison>;

    struct sequence final
    {
        std::vector<instruction> elements;
    };

    struct semantic_error final
    {
        std::string message;
        syntax::source_location location;
    };

    using semantic_error_handler = std::function<void(semantic_error)>;

    [[nodiscard]] sequence check_types(syntax::sequence const &input, semantic_error_handler on_error);
} // namespace lpg::semantics
