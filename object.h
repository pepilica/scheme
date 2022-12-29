#pragma once

#include "error.h"
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include "function_ref.h"
#include <iostream>

class Scope;

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual std::string Serialize() = 0;
    virtual std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope = nullptr) = 0;
    virtual ~Object() = default;
    virtual operator bool() const {
        return true;
    }
};

class Scope {

public:
    Scope() : variables_(), parent_scope_() {
    }

    void AddVariable(const std::string& name, std::shared_ptr<Object> variable);
    void SetVariable(const std::string& name, std::shared_ptr<Object> variable);
    bool HasVariable(const std::string& name);
    std::shared_ptr<Object> GetVariable(const std::string& name);
    std::shared_ptr<Scope>& GetParentScope();

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> variables_;
    std::shared_ptr<Scope> parent_scope_;
};

using ObjectVectorBase = std::vector<std::shared_ptr<Object>>;

class ObjectVector : public ObjectVectorBase {
public:
    ObjectVector() : ObjectVectorBase(), scope_() {
    }

    ObjectVector(const ObjectVectorBase& vector) : ObjectVectorBase(vector), scope_() {
    }

    std::shared_ptr<Scope>& GetScope() {
        return scope_;
    }

private:
    std::shared_ptr<Scope> scope_ = {};
};

using FunctionSignature = std::shared_ptr<Object> (*)(ObjectVector&);

ObjectVector EvaluateList(const std::shared_ptr<Object>& list);

class FunctionsKeeper {
public:
    void InsertFunction(const std::string& s, FunctionSignature sign) {
        functions_.insert({s, sign});
    }

    FunctionSignature GetFunction(const std::string& s) {
        auto iter = functions_.find(s);
        if (iter == functions_.end()) {
            throw NameError(" ");
        }
        return iter->second;
    }

    static bool HasFunction(const std::string& s) {
        return (Instance().functions_.find(s) != Instance().functions_.end());
    }

    static FunctionsKeeper& Instance() {
        static FunctionsKeeper* keeper = new FunctionsKeeper{};
        return *keeper;
    }

private:
    std::unordered_map<std::string, FunctionSignature> functions_;
    static std::unique_ptr<FunctionsKeeper> functions_keeper;
    FunctionsKeeper() {
    }
};

class FunctionWrapper : public Object {
public:
    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope = nullptr) override {
        return shared_from_this();
    }

    virtual std::shared_ptr<Object> Apply(ObjectVector& args) const = 0;
};

class Function : public FunctionWrapper {

public:
    Function(FunctionSignature f) : func_(f) {
    }

    std::string Serialize() override {
        return "";
    }

    std::shared_ptr<Object> Apply(ObjectVector& args) const override {
        return func_(args);
    }

    static std::shared_ptr<Function> CreateFunction(const std::string& str) {
        FunctionsKeeper& keeper = FunctionsKeeper::Instance();
        return std::shared_ptr<Function>(new Function(keeper.GetFunction(str)));
    }

    static bool HasFunction(const std::string& str) {
        return FunctionsKeeper::HasFunction(str);
    }

private:
    FunctionRef<std::shared_ptr<Object>(ObjectVector&)> func_;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj);
template <class T>
bool Is(const std::shared_ptr<Object>& obj);

class LambdaCreator;

class Symbol : public Object {
public:
    const std::string& GetName() const {
        return symbol_;
    };

    std::string Serialize() override {
        return symbol_;
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope = nullptr) override {
        if (scope) {
            std::shared_ptr<Object> obj = scope->GetVariable(symbol_);
            while (Is<Symbol>(obj)) {
                obj = scope->GetVariable(As<Symbol>(obj)->GetName());
            }
            if (Is<LambdaCreator>(obj)) {
                return obj->Evaluate();
            }
            return obj;
        }
        return Function::CreateFunction(GetName());
    }

    Symbol(const std::string& s) : symbol_(s) {
    }

private:
    std::string symbol_;
};

class Lambda : public FunctionWrapper {

public:
    Lambda(std::shared_ptr<Scope> scope, const ObjectVector& vars, ObjectVectorBase& body)
        : order_(), scope_(std::make_shared<Scope>()), body_(body) {
        scope_->GetParentScope() = scope;
        std::copy_if(vars.begin(), vars.end(), std::back_inserter(order_),
                     [](std::shared_ptr<Object> ptr) { return ptr != nullptr; });
    }

    std::string Serialize() override {
        return "";
    }

    std::shared_ptr<Object> Apply(ObjectVector& args) const override {
        std::shared_ptr<Scope> cur_scope = scope_;
        ObjectVector args_redefined;
        std::copy_if(args.begin(), args.end(), std::back_inserter(args_redefined),
                     [](std::shared_ptr<Object> ptr) { return ptr != nullptr; });
        if (args_redefined.size() != order_.size()) {
            throw RuntimeError(" ");
        }
        ObjectVector vars;
        for (size_t i = 0; i < args_redefined.size(); ++i) {
            cur_scope->AddVariable(As<Symbol>(order_[i])->GetName(),
                                   args_redefined[i]->Evaluate(args.GetScope()));
        }
        std::shared_ptr<Object> res;
        for (auto& i : body_) {
            res = i->Evaluate(cur_scope);
        }
        return res;
    }

    static std::shared_ptr<Lambda> CreateLambda(ObjectVector& vars, ObjectVectorBase& body) {
        return std::make_shared<Lambda>(vars.GetScope(), vars, body);
    }

private:
    ObjectVectorBase order_;
    std::shared_ptr<Scope> scope_;
    std::vector<std::shared_ptr<Object>> body_;
};

class LambdaCreator : public Object {
public:
    LambdaCreator(std::shared_ptr<Scope> scope, const ObjectVector& vars, ObjectVectorBase& body)
        : order_(), scope_(std::make_shared<Scope>()), body_(body) {
        scope_->GetParentScope() = scope;
        std::copy_if(vars.begin(), vars.end(), std::back_inserter(order_),
                     [](std::shared_ptr<Object> ptr) { return ptr != nullptr; });
    }

    std::string Serialize() override {
        return "";
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope = nullptr) override {
        ObjectVector obj = order_;
        obj.GetScope() = scope_;
        return CreateLambda(obj, body_);
    }

    static std::shared_ptr<Lambda> CreateLambda(ObjectVector& vars, ObjectVectorBase& body) {
        return std::make_shared<Lambda>(vars.GetScope(), vars, body);
    }

private:
    ObjectVectorBase order_;
    std::shared_ptr<Scope> scope_;
    std::vector<std::shared_ptr<Object>> body_;
};

class Bool : public Object {

public:
    Bool(bool value) : value_(value) {
    }

    Bool(const std::string& s) {
        if (s == "#f") {
            value_ = false;
        } else {
            value_ = true;
        }
    }

    std::string Serialize() override {
        if (value_) {
            return "#t";
        }
        return "#f";
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope = nullptr) override {
        return std::make_shared<Bool>(value_);
    }

    operator bool() const override {
        return value_;
    }

private:
    bool value_;
};

class Number : public Object {
public:
    int GetValue() const {
        return value_;
    };

    std::string Serialize() override {
        return std::to_string(value_);
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope = nullptr) override {
        return std::make_shared<Number>(GetValue());
    }

    Number(int n) : value_(n) {
    }

private:
    int value_;
};

class Cell : public Object {
public:
    std::string Serialize() override {
        std::string ans = "(";
        ObjectVector cell;
        if (shared_from_this()) {
            cell = EvaluateList(shared_from_this());
        }
        ObjectVector me_in_vector = ObjectVectorBase({shared_from_this()});
        if (cell.empty()) {
            return "(())";
        }
        for (size_t j = 0; j < cell.size() - 1; ++j) {
            auto i = cell[j];
            if (j != 0) {
                ans += ' ';
            }
            if (!i) {
                ans += "()";
            } else {
                ans += i->Serialize();
            }
        }
        bool is_wrong = false;
        std::shared_ptr<Cell> cur = As<Cell>(shared_from_this());
        while (cur->GetSecond() != nullptr && Is<Cell>(cur->GetSecond())) {
            cur = As<Cell>(cur->GetSecond());
        }
        if (cur->GetSecond() == nullptr) {
            is_wrong = false;
        } else {
            is_wrong = true;
        }
        if (is_wrong) {
            ans += " .";
        }
        if (cell.size() > 1) {
            ans += " ";
        }
        if (!cell.empty()) {
            if (!cell.back()) {
                ans += "()";
            } else {
                ans += cell.back()->Serialize();
            }
        }
        ans += ")";
        return ans;
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope = nullptr) override {
        if (GetSecond() == nullptr && GetFirst() == nullptr) {
            throw RuntimeError(" ");
        }
        if (!GetFirst()) {
            throw RuntimeError(" ");
        }
        std::shared_ptr<Object> first_arg = GetFirst()->Evaluate(scope);
        std::shared_ptr<FunctionWrapper> func = As<FunctionWrapper>(first_arg);
        ObjectVector objects;
        if (Is<Symbol>(GetFirst())) {
            if (As<Symbol>(GetFirst())->GetName() == "quote") {
                auto second = As<Cell>(GetSecond());
                objects = ObjectVectorBase({second->GetFirst()});
            } else {
                objects = EvaluateList(GetSecond());
            }
        } else {
            objects = EvaluateList(GetSecond());
        }
        objects.GetScope() = scope;
        std::shared_ptr<Object> res = (func->Apply(objects));
        return res;
    }

    std::shared_ptr<Object> GetFirst() const {
        return cell_.first;
    };
    std::shared_ptr<Object>& GetFirst() {
        return cell_.first;
    };
    std::shared_ptr<Object> GetSecond() const {
        return cell_.second;
    };
    std::shared_ptr<Object>& GetSecond() {
        return cell_.second;
    };

private:
    std::pair<std::shared_ptr<Object>, std::shared_ptr<Object>> cell_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    if (!Is<T>(obj)) {
        throw RuntimeError(" ");
    }
    return std::dynamic_pointer_cast<T>(obj);
};

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    try {
        auto a = std::dynamic_pointer_cast<T>(obj);
        if (a.get() == nullptr) {
            return false;
        }
        return true;
    } catch (std::bad_cast&) {
        return false;
    }
}