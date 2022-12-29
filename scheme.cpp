#include "scheme.h"

std::string Interpreter::Run(const std::string& stream) {

    InitializeFunctionKeeper();

    std::stringstream ss{stream};
    Tokenizer tokenizer{&ss};

    std::string output_string;

    auto input_ast = Read(&tokenizer);

    while (!tokenizer.IsEnd()) {
        Read(&tokenizer);
    }

    if (!input_ast) {
        throw RuntimeError(" ");
    }

    if (!global_scope_) {
        global_scope_ = std::make_shared<Scope>();
    }

    if (Is<Cell>(input_ast)) {
        std::shared_ptr<Cell> cell_ast = As<Cell>(input_ast);
        std::shared_ptr<Object> first_arg = cell_ast->GetFirst();
        ObjectVector args;
        if (Is<Symbol>(first_arg)) {
            if (As<Symbol>(first_arg)->GetName() == "quote") {
                if (Is<Cell>(cell_ast->GetSecond())) {
                    args = ObjectVectorBase({As<Cell>(cell_ast->GetSecond())->GetFirst()});
                } else if (cell_ast->GetSecond()) {
                    args = ObjectVectorBase({cell_ast->GetSecond()});
                }
            } else if (cell_ast->GetSecond()) {
                args = EvaluateList(cell_ast->GetSecond());
            }
        } else {
            args = EvaluateList(cell_ast->GetSecond());
        }
        args.GetScope() = std::shared_ptr<Scope>(new Scope());
        args.GetScope() = global_scope_;
        if (first_arg == nullptr) {
            throw RuntimeError(" ");
        }
        std::shared_ptr<Object> evaluated_first_arg = first_arg->Evaluate(global_scope_);
        auto function = As<FunctionWrapper>(evaluated_first_arg);
        auto output = function->Apply(args);
        if (!output) {
            output_string += "()";
        } else {
            output_string += output->Serialize();
        }
    } else {
        output_string += input_ast->Evaluate(global_scope_)->Serialize();
    }
    return output_string;
}
