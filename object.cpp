#include "object.h"

void CollectEvaluations(std::shared_ptr<Object> list, ObjectVector& eval) {
    auto empty = std::make_shared<Cell>();
    empty->GetFirst() = nullptr;
    empty->GetSecond() = nullptr;
    if (Is<Cell>(list)) {
        auto cell_list = As<Cell>(list);
        if (cell_list->GetFirst() != nullptr) {
            eval.push_back(cell_list->GetFirst());
        } else {
            eval.push_back(nullptr);
        }
        if (cell_list->GetSecond() != nullptr) {
            CollectEvaluations(cell_list->GetSecond(), eval);
        }
    } else {
        eval.push_back(list);
    }
}

ObjectVector EvaluateList(const std::shared_ptr<Object>& list) {
    ObjectVector res;
    CollectEvaluations(list, res);
    return res;
}

void Scope::AddVariable(const std::string& name, std::shared_ptr<Object> variable) {
    auto iter = variables_.find(name);
    if (iter == variables_.end()) {
        variables_.insert({name, std::move(variable)});
    } else {
        variables_[name] = variable;
    }
}

void Scope::SetVariable(const std::string& name, std::shared_ptr<Object> variable) {
    auto iter = variables_.find(name);
    if (iter == variables_.end()) {
        if (parent_scope_) {
            return parent_scope_->SetVariable(name, variable);
        }
        throw NameError(" ");
    } else {
        variables_[name] = variable;
    }
}

std::shared_ptr<Object> Scope::GetVariable(const std::string& name) {
    auto iter = variables_.find(name);
    if (iter == variables_.end()) {
        if (parent_scope_) {
            return parent_scope_->GetVariable(name);
        }
        return Function::CreateFunction(name);
    } else {
        return iter->second;
    }
}

std::shared_ptr<Scope>& Scope::GetParentScope() {
    return parent_scope_;
}

bool Scope::HasVariable(const std::string& name) {
    auto iter = variables_.find(name);
    if (iter == variables_.end()) {
        if (parent_scope_) {
            return parent_scope_->HasVariable(name);
        }
        return Function::HasFunction(name);
    } else {
        return true;
    }
}
