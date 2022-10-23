#include "interpreter.h"
#include "overloaded.h"
#include "type_checker.h"
#include <boost/outcome/result.hpp>

namespace lpg
{
    bool operator==(evaluate_error const &left, evaluate_error const &right)
    {
        return (left.type == right.type);
    }

    std::ostream &operator<<(std::ostream &out, evaluate_error const &error)
    {
        return out << static_cast<int>(error.type);
    }

    namespace
    {
        struct void_
        {
        };

        using value = std::variant<std::string, semantics::builtin_functions, void_>;

        struct interpreter final
        {
            std::vector<std::optional<value>> locals;
            std::string print_output;

            [[nodiscard]] std::optional<evaluate_error> initialize_local(semantics::local_id const id,
                                                                         value initializer)
            {
                if (id.value >= locals.size())
                {
                    locals.resize(id.value + 1);
                }
                std::optional<value> &local = locals[id.value];
                if (local)
                {
                    return evaluate_error{evaluate_error_type::local_initialized_twice};
                }
                local = std::move(initializer);
                return std::nullopt;
            }

            [[nodiscard]] boost::outcome_v2::result<value, evaluate_error> read_local(semantics::local_id const id)
            {
                std::optional<value> const &local = locals[id.value];
                if (!local)
                {
                    return evaluate_error{evaluate_error_type::read_uninitialized_local};
                }
                return *local;
            }
        };

        [[nodiscard]] std::optional<evaluate_error> run_sequence(interpreter &context,
                                                                 semantics::sequence const &sequence);

        [[nodiscard]] std::optional<evaluate_error> run_instruction(interpreter &context,
                                                                    semantics::instruction const &instruction)
        {
            return std::visit(
                overloaded{
                    [&context](semantics::builtin const &builtin_instruction) -> std::optional<evaluate_error> {
                        return context.initialize_local(builtin_instruction.destination, builtin_instruction.function);
                    },
                    [&context](semantics::call const &call_instruction) -> std::optional<evaluate_error> {
                        boost::outcome_v2::result<value, evaluate_error> const maybe_callee =
                            context.read_local(call_instruction.callee);
                        if (maybe_callee.has_error())
                        {
                            return maybe_callee.assume_error();
                        }
                        boost::outcome_v2::result<value, evaluate_error> const maybe_argument =
                            context.read_local(call_instruction.argument);
                        if (maybe_argument.has_error())
                        {
                            return maybe_argument.assume_error();
                        }
                        semantics::builtin_functions const *const builtin =
                            std::get_if<semantics::builtin_functions>(&maybe_callee.assume_value());
                        if (!builtin)
                        {
                            return evaluate_error{evaluate_error_type::not_callable};
                        }
                        switch (*builtin)
                        {
                        case semantics::builtin_functions::print: {
                            std::string const *const message = std::get_if<std::string>(&maybe_argument.assume_value());
                            if (!message)
                            {
                                return evaluate_error{evaluate_error_type::invalid_argument_type};
                            }
                            context.print_output += *message;
                            break;
                        }
                        }
                        return std::nullopt;
                    },
                    [&context](
                        semantics::string_literal const &string_literal_instruction) -> std::optional<evaluate_error> {
                        return context.initialize_local(
                            string_literal_instruction.destination, string_literal_instruction.value);
                    },
                    [&context](semantics::sequence const &sequence_instruction) -> std::optional<evaluate_error> {
                        return run_sequence(context, sequence_instruction);
                    },
                    [&context](
                        semantics::void_literal const &void_literal_instruction) -> std::optional<evaluate_error> {
                        return context.initialize_local(void_literal_instruction.destination, void_{});
                    },
                    [](semantics::poison const &poison_instruction) -> std::optional<evaluate_error> {
                        (void)poison_instruction;
                        return evaluate_error{evaluate_error_type::poison_reached};
                    }},
                instruction);
        }

        [[nodiscard]] std::optional<evaluate_error> run_sequence(interpreter &context,
                                                                 semantics::sequence const &sequence)
        {
            for (semantics::instruction const &element : sequence.elements)
            {
                std::optional<evaluate_error> error = run_instruction(context, element);
                if (error)
                {
                    return error;
                }
            }
            return std::nullopt;
        }
    } // namespace

    run_result run(std::string_view source, std::function<void(syntax::parse_error)> on_syntax_error,
                   semantics::semantic_error_handler on_semantic_error)
    {
        assert(on_syntax_error);
        assert(on_semantic_error);
        syntax::sequence parsed = compile(source, move(on_syntax_error));
        semantics::sequence const checked = semantics::check_types(parsed, move(on_semantic_error));
        interpreter context;
        if (std::optional<evaluate_error> error = run_sequence(context, checked))
        {
            return std::move(*error);
        }
        return std::move(context.print_output);
    }
} // namespace lpg
