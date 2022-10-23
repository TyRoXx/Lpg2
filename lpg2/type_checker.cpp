#include "type_checker.h"
#include "overloaded.h"
#include <map>

namespace lpg::semantics
{
    namespace
    {
        struct type_checker
        {
            local_id next_local = {0};
            semantic_error_handler on_error;
            std::map<std::string, local_id> named_local_variables;

            [[nodiscard]] local_id allocate_local()
            {
                local_id const result = next_local;
                ++next_local.value;
                return result;
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
            local_id const void_result = checker.allocate_local();
            output.elements.emplace_back(void_literal{void_result});
            return void_result;
        }

        [[nodiscard]] local_id check_expression(type_checker &checker, syntax::expression const &input,
                                                sequence &output)
        {
            return std::visit(
                overloaded{[&checker, &output](syntax::string_literal const &string_literal_input) -> local_id {
                               local_id const local = checker.allocate_local();
                               output.elements.emplace_back(
                                   string_literal{local, std::string(string_literal_input.inner_content)});
                               return local;
                           },
                           [&checker, &output](syntax::identifier const &identifier_input) -> local_id {
                               if (identifier_input.content == "print")
                               {
                                   local_id const destination = checker.allocate_local();
                                   output.elements.emplace_back(builtin{destination, builtin_functions::print});
                                   return destination;
                               }
                               auto const found =
                                   checker.named_local_variables.find(std::string(identifier_input.content));
                               if (found == checker.named_local_variables.end())
                               {
                                   checker.on_error(semantic_error{"Unknown identifier", identifier_input.location});
                                   local_id const poison_id = checker.allocate_local();
                                   output.elements.emplace_back(poison{poison_id});
                                   return poison_id;
                               }
                               return found->second;
                           },
                           [&checker, &output](syntax::call const &call_input) -> local_id {
                               local_id const callee = check_expression(checker, *call_input.callee, output);
                               local_id const argument = check_expression(checker, *call_input.argument, output);
                               local_id const result = checker.allocate_local();
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
                                   checker.on_error(semantic_error{"Local variable with this name already exists",
                                                                   declaration_input.name.location});
                               }
                               local_id const initializer =
                                   check_expression(checker, *declaration_input.initializer, output);
                               if (!name_exists)
                               {
                                   checker.named_local_variables.emplace(declaration_input.name.content, initializer);
                               }
                               local_id const void_id = checker.allocate_local();
                               output.elements.emplace_back(void_literal{void_id});
                               return void_id;
                           }},
                input.value);
        }
    } // namespace

    sequence check_types(syntax::sequence const &input, semantic_error_handler on_error)
    {
        type_checker checker{local_id{0}, move(on_error)};
        sequence result;
        (void)check_sequence(checker, input, result);
        return result;
    }
} // namespace lpg::semantics
