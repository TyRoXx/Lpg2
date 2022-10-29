#pragma once
#include "parser.h"

namespace lpg::syntax
{
    struct formatter final
    {
        std::ostream &output;
        size_t indentation_level = 0;

        void format(string_literal_expression const &value);
        void format(identifier const &value);
        void format(call const &value);
        void format(sequence const &value);
        void format(declaration const &value);
        void format(keyword_expression const &value);
        void format(binary_operator_expression const &value);
        void format(binary_operator_literal_expression const &value);
        void format_expression(expression const &value);
        void print_indentation(size_t const level);
    };
} // namespace lpg::syntax
