#include "formatter.h"

namespace lpg::syntax
{
    void formatter::format(string_literal_expression const &value)
    {
        output << "\"" << value.literal.inner_content << "\"";
    }

    void formatter::format(identifier const &value)
    {
        output << value.content;
    }

    void formatter::format(call const &value)
    {
        format_expression(*value.callee);
        output << "(";
        for (size_t i = 0; i < value.arguments.size(); ++i)
        {
            if (i > 0)
            {
                output << ", ";
            }
            format_expression(*value.arguments[i]);
        }
        output << ")";
    }

    void formatter::format(sequence const &value)
    {
        if (indentation_level > 0)
        {
            output << "{\n";
        }
        ++indentation_level;
        for (size_t i = 0; i < value.elements.size(); ++i)
        {
            print_indentation(indentation_level - 1);
            format_expression(value.elements[i]);
            output << "\n";
        }
        --indentation_level;
        if (indentation_level > 0)
        {
            print_indentation(indentation_level - 1);
            output << "}";
        }
    }

    void formatter::format(declaration const &value)
    {
        output << "let " << value.name.content << " = ";
        format_expression(*value.initializer);
    }

    void formatter::format(bool_literal_expression const &value)
    {
        output << value.literal;
    }

    void formatter::format(binary_operator_expression const &value)
    {
        format_expression(*value.left);
        output << " " << value.which << " ";
        format_expression(*value.right);
    }

    void formatter::format(binary_operator_literal_expression const &value)
    {
        output << value.which;
    }

    void formatter::format_expression(expression const &value)
    {
        std::visit(
            [this](auto const &expression_) {
                this->format(expression_);
            },
            value.value);
    }

    void formatter::print_indentation(size_t const level)
    {
        for (size_t i = 0; i < level; ++i)
        {
            constexpr std::string_view indentation = "    ";
            output << indentation;
        }
    }
} // namespace lpg::syntax
