#include "type_checker.h"
#include "overloaded.h"
#include <map>

namespace lpg::semantics
{
    std::ostream &operator<<(std::ostream &out, semantic_error const &error)
    {
        return out << error.location << ":" << error.message;
    }

    namespace
    {
        enum class type
        {
            string,
            void_,
            print,
            poison
        };

        struct type_checker final
        {
            std::vector<type> locals;
            semantic_error_handler on_error;
            std::map<std::string, local_id> named_local_variables;

            [[nodiscard]] local_id allocate_local(type const local_type)
            {
                local_id const result{locals.size()};
                locals.emplace_back(local_type);
                return result;
            }

            [[nodiscard]] type type_of(local_id const local)
            {
                return locals[local.value];
            }
        };

        [[nodiscard]] local_id check_expression(type_checker &checker, syntax::expression const &input,
                                                sequence &output);

        [[nodiscard]] local_id check_sequence(type_checker &checker, syntax::sequence const &input, sequence &output)
        {
            std::optional<local_id> sequence_result;
            for (syntax::expression const &element : input.elements)
            {
                sequence_result = check_expression(checker, element, output);
            }
            if (sequence_result)
            {
                return *sequence_result;
            }
            local_id const void_result = checker.allocate_local(type::void_);
            output.elements.emplace_back(void_literal{void_result});
            return void_result;
        }

        [[nodiscard]] local_id check_expression(type_checker &checker, syntax::expression const &input,
                                                sequence &output)
        {
            return std::visit(
                overloaded{
                    [&checker, &output](syntax::string_literal_expression const &string_literal_input) -> local_id {
                        local_id const local = checker.allocate_local(type::string);
                        output.elements.emplace_back(
                            string_literal{local, std::string(string_literal_input.literal.inner_content)});
                        return local;
                    },
                    [&checker, &output](syntax::identifier const &identifier_input) -> local_id {
                        if (identifier_input.content == "print")
                        {
                            local_id const destination = checker.allocate_local(type::print);
                            output.elements.emplace_back(builtin{destination, builtin_functions::print});
                            return destination;
                        }
                        auto const found = checker.named_local_variables.find(std::string(identifier_input.content));
                        if (found == checker.named_local_variables.end())
                        {
                            checker.on_error(semantic_error{"Unknown identifier", identifier_input.location});
                            local_id const poison_id = checker.allocate_local(type::poison);
                            output.elements.emplace_back(poison{poison_id});
                            return poison_id;
                        }
                        return found->second;
                    },
                    [&checker, &output](syntax::call const &call_input) -> local_id {
                        local_id const callee = check_expression(checker, *call_input.callee, output);
                        if (checker.type_of(callee) != type::print)
                        {
                            checker.on_error(
                                semantic_error{"This value is not callable", get_location(*call_input.callee)});
                            local_id const poison_id = checker.allocate_local(type::poison);
                            output.elements.emplace_back(poison{poison_id});
                            return poison_id;
                        }
                        local_id const argument = check_expression(checker, *call_input.argument, output);
                        if (checker.type_of(argument) != type::string)
                        {
                            checker.on_error(
                                semantic_error{"Argument type mismatch", get_location(*call_input.argument)});
                            local_id const poison_id = checker.allocate_local(type::poison);
                            output.elements.emplace_back(poison{poison_id});
                            return poison_id;
                        }
                        local_id const result = checker.allocate_local(type::void_);
                        output.elements.emplace_back(call{result, callee, argument});
                        return result;
                    },
                    [&checker, &output](syntax::sequence const &sequence_input) -> local_id {
                        return check_sequence(checker, sequence_input, output);
                    },
                    [&checker, &output](syntax::declaration const &declaration_input) -> local_id {
                        bool const name_exists =
                            checker.named_local_variables.contains(std::string(declaration_input.name.content));
                        if (name_exists)
                        {
                            checker.on_error(semantic_error{
                                "Local variable with this name already exists", declaration_input.name.location});
                        }
                        local_id const initializer = check_expression(checker, *declaration_input.initializer, output);
                        if (!name_exists)
                        {
                            checker.named_local_variables.emplace(declaration_input.name.content, initializer);
                        }
                        local_id const void_id = checker.allocate_local(type::void_);
                        output.elements.emplace_back(void_literal{void_id});
                        return void_id;
                    }},
                input.value);
        }
    } // namespace

    sequence check_types(syntax::sequence const &input, semantic_error_handler on_error)
    {
        type_checker checker{{}, move(on_error)};
        sequence result;
        (void)check_sequence(checker, input, result);
        return result;
    }
} // namespace lpg::semantics
