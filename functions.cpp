#include "functions.h"

template <typename Exc>
void AssertFunctionOfLength(ObjectVector& vector, size_t length,
                            FunctionRef<bool(size_t, size_t)> func) {
    if (!func(vector.size(), length)) {
        throw Exc(" ");
    }
}

template <typename Exc>
void AssertLength(ObjectVector& vector, size_t length) {
    AssertFunctionOfLength<Exc>(vector, length, [](size_t lhv, size_t rhv) { return lhv == rhv; });
}

template <typename Exc>
void AssertLengthLessEq(ObjectVector& vector, size_t length) {
    AssertFunctionOfLength<Exc>(vector, length, [](size_t lhv, size_t rhv) { return lhv <= rhv; });
}

template <typename Exc>
void AssertLengthMoreEq(ObjectVector& vector, size_t length) {
    AssertFunctionOfLength<Exc>(vector, length, [](size_t lhv, size_t rhv) { return lhv >= rhv; });
}

std::shared_ptr<Object> IsBoolean(ObjectVector& input) {
    AssertLength<RuntimeError>(input, 1);
    auto s = input[0];
    if (Is<Bool>(s)) {
        return std::make_shared<Bool>(true);
    }
    return std::make_shared<Bool>(false);
}

std::shared_ptr<Object> NotBoolean(ObjectVector& input) {
    AssertLength<RuntimeError>(input, 1);
    std::shared_ptr<Object> s = input[0];
    if (!s) {
        return std::make_shared<Bool>(true);
    }
    return std::make_shared<Bool>(!(s.get()->operator bool()));
}

std::shared_ptr<Object> AndBoolean(ObjectVector& input) {
    std::shared_ptr<Object> ans = std::make_shared<Bool>(true);
    for (auto& i : input) {
        if (!i) {
            throw RuntimeError(" ");
        }
        auto obj_i = i->Evaluate(input.GetScope());
        if (!(obj_i.get()->operator bool())) {
            return obj_i;
        }
        ans = obj_i;
    }
    return ans;
}

std::shared_ptr<Object> OrBoolean(ObjectVector& input) {
    std::shared_ptr<Object> ans = std::make_shared<Bool>(false);
    for (auto& i : input) {
        if (!i) {
            throw RuntimeError(" ");
        }
        auto obj_i = i->Evaluate(input.GetScope());
        if ((obj_i.get()->operator bool())) {
            return obj_i;
        }
        ans = obj_i;
    }
    return ans;
}

std::shared_ptr<Object> IsInteger(ObjectVector& input) {
    AssertLength<RuntimeError>(input, 1);

    return std::make_shared<Bool>(Is<Number>(input[0]));
}

std::shared_ptr<Object> IntegerComparisonWrapper(ObjectVector& list,
                                                 FunctionRef<bool(int, int)> comp) {
    if (list.empty()) {
        return std::make_shared<Bool>(true);
    }
    if (list.size() == 1) {
        if (!Is<Number>(list[0]->Evaluate(list.GetScope()))) {
            throw RuntimeError(" ");
        }
        return std::make_shared<Bool>(true);
    }
    std::shared_ptr<Number> last_element = As<Number>(list[0]->Evaluate(list.GetScope()));
    for (size_t i = 1; i < list.size(); ++i) {
        if (!list[i]) {
            throw RuntimeError(" ");
        }
        std::shared_ptr<Object> cur_obj = list[i]->Evaluate(list.GetScope());
        if (!Is<Number>(cur_obj) || !cur_obj) {
            throw RuntimeError(" ");
        }
        std::shared_ptr<Number> cur_element = As<Number>(cur_obj);
        if (!comp(last_element->GetValue(), cur_element->GetValue())) {
            return std::make_shared<Bool>(false);
        }
        last_element = std::move(cur_element);
    }
    return std::make_shared<Bool>(true);
}

std::shared_ptr<Object> EqInteger(ObjectVector& s) {
    return IntegerComparisonWrapper(s, [](int lhv, int rhv) { return lhv == rhv; });
}

std::shared_ptr<Object> BiggerInteger(ObjectVector& s) {
    return IntegerComparisonWrapper(s, [](int lhv, int rhv) { return lhv > rhv; });
}

std::shared_ptr<Object> LessInteger(ObjectVector& s) {
    return IntegerComparisonWrapper(s, [](int lhv, int rhv) { return lhv < rhv; });
}

std::shared_ptr<Object> BiggerEqInteger(ObjectVector& s) {
    return IntegerComparisonWrapper(s, [](int lhv, int rhv) { return lhv >= rhv; });
}

std::shared_ptr<Object> LessEqInteger(ObjectVector& s) {
    return IntegerComparisonWrapper(s, [](int lhv, int rhv) { return lhv <= rhv; });
}

std::shared_ptr<Object> IntegerOperationsWrapper(ObjectVector& list,
                                                 FunctionRef<int(int, int)> operation,
                                                 FunctionRef<int()> default_value) {
    if (list.empty()) {
        return std::make_shared<Number>(default_value());
    }
    if (!list[0]) {
        throw RuntimeError(" ");
    }
    int ans = As<Number>(list[0]->Evaluate(list.GetScope()))->GetValue();
    for (size_t i = 1; i < list.size(); ++i) {
        if (!list[i]) {
            throw RuntimeError(" ");
        }
        ans = operation(ans, As<Number>(list[i]->Evaluate(list.GetScope()))->GetValue());
    }
    return std::make_shared<Number>(ans);
}

std::shared_ptr<Object> PlusInteger(ObjectVector& s) {
    return IntegerOperationsWrapper(
        s, [](int lhv, int rhv) { return lhv + rhv; }, []() { return 0; });
}

std::shared_ptr<Object> MinusInteger(ObjectVector& s) {
    return IntegerOperationsWrapper(
        s, [](int lhv, int rhv) { return lhv - rhv; },
        []() {
            throw RuntimeError(" ");
            return 0;
        });
}

std::shared_ptr<Object> ProductInteger(ObjectVector& s) {
    return IntegerOperationsWrapper(
        s, [](int lhv, int rhv) { return lhv * rhv; }, []() { return 1; });
}

std::shared_ptr<Object> DivisionInteger(ObjectVector& s) {
    return IntegerOperationsWrapper(
        s, [](int lhv, int rhv) { return lhv / rhv; },
        []() {
            throw RuntimeError(" ");
            return 1;
        });
}

std::shared_ptr<Object> MinInteger(ObjectVector& s) {
    return IntegerOperationsWrapper(
        s, [](int lhv, int rhv) { return std::min(lhv, rhv); },
        []() {
            throw RuntimeError(" ");
            return 0;
        });
}

std::shared_ptr<Object> MaxInteger(ObjectVector& s) {
    return IntegerOperationsWrapper(
        s, [](int lhv, int rhv) { return std::max(lhv, rhv); },
        []() {
            throw RuntimeError(" ");
            return 0;
        });
}

std::shared_ptr<Object> AbsInteger(ObjectVector& s) {
    AssertLength<RuntimeError>(s, 1);
    return std::make_shared<Number>(std::abs(As<Number>(s[0]->Evaluate(s.GetScope()))->GetValue()));
}

std::shared_ptr<Object> IsPairList(ObjectVector& list) {
    AssertLength<RuntimeError>(list, 1);
    std::shared_ptr<Object> s = list[0]->Evaluate(list.GetScope());
    if (!Is<Cell>(s)) {
        return std::make_shared<Bool>(false);
    }
    auto cell = As<Cell>(s);
    return std::make_shared<Bool>(cell->GetFirst() != nullptr);
}

std::shared_ptr<Object> IsListList(ObjectVector& list) {
    AssertLength<RuntimeError>(list, 1);
    std::shared_ptr<Object> s = list[0]->Evaluate(list.GetScope());
    if (!Is<Cell>(s)) {
        return std::make_shared<Bool>(true);
    }
    auto cell = As<Cell>(s);
    std::shared_ptr<Cell> cur_elem = cell;
    while (true) {
        if (cur_elem->GetSecond() == nullptr) {
            return std::make_shared<Bool>(true);
        }
        if (!Is<Cell>(cur_elem->GetSecond())) {
            return std::make_shared<Bool>(false);
        }
        cur_elem = As<Cell>(cur_elem->GetSecond());
    }
}

std::shared_ptr<Object> IsNullList(ObjectVector& list) {
    AssertLength<RuntimeError>(list, 1);
    std::shared_ptr<Object> obj = list[0]->Evaluate(list.GetScope());
    if (!Is<Cell>(obj)) {
        return std::make_shared<Bool>(true);
    }
    std::shared_ptr<Cell> cell = As<Cell>(obj);
    return std::make_shared<Bool>(cell->GetFirst() == nullptr && cell->GetSecond() == nullptr);
}

std::shared_ptr<Object> ConsList(ObjectVector& list) {
    AssertLength<RuntimeError>(list, 2);
    std::shared_ptr<Cell> ans = std::make_shared<Cell>();
    ans->GetFirst() = list[0]->Evaluate(list.GetScope());
    ans->GetSecond() = list[1]->Evaluate(list.GetScope());
    return ans;
}

std::shared_ptr<Object> CarList(ObjectVector& list) {
    AssertLength<RuntimeError>(list, 1);
    std::shared_ptr<Object> s = list[0]->Evaluate(list.GetScope());
    auto cell = As<Cell>(s);
    if (cell->GetFirst() == nullptr) {
        throw RuntimeError(" ");
    }
    return cell->GetFirst();
}

std::shared_ptr<Object> CdrList(ObjectVector& list) {
    AssertLength<RuntimeError>(list, 1);
    std::shared_ptr<Object> s = list[0]->Evaluate(list.GetScope());
    auto cell = As<Cell>(s);
    return cell->GetSecond();
}

std::shared_ptr<Object> ListList(ObjectVector& list) {
    std::shared_ptr<Cell> ans = std::make_shared<Cell>();
    std::shared_ptr<Cell> cur_pos = ans;
    if (list.empty()) {
        return nullptr;
    }
    ans->GetFirst() = list[0]->Evaluate(list.GetScope());
    ans->GetSecond() = std::make_shared<Cell>();
    for (size_t j = 1; j < list.size(); ++j) {
        auto& i = list[j];
        cur_pos = As<Cell>(cur_pos->GetSecond());
        cur_pos->GetFirst() = i->Evaluate(list.GetScope());
        cur_pos->GetSecond() = std::make_shared<Cell>();
    }
    cur_pos->GetSecond() = nullptr;
    return ans;
}

std::shared_ptr<Object> ListRefList(ObjectVector& list) {
    AssertLength<RuntimeError>(list, 2);
    std::shared_ptr<Cell> cell = As<Cell>(list[0]->Evaluate(list.GetScope()));
    std::shared_ptr<Cell> cur_cell = cell;
    std::shared_ptr<Number> pos = As<Number>(list[1]->Evaluate(list.GetScope()));
    int64_t counter = 0;
    while (counter < pos->GetValue()) {
        if (cur_cell->GetSecond() == nullptr) {
            throw RuntimeError(" ");
        }
        ++counter;
        cur_cell = As<Cell>(cur_cell->GetSecond());
    }
    if (cur_cell->GetFirst() == nullptr) {
        throw RuntimeError(" ");
    }
    return cur_cell->GetFirst();
}

std::shared_ptr<Object> ListTailList(ObjectVector& list) {
    AssertLength<RuntimeError>(list, 2);
    std::shared_ptr<Cell> cell = As<Cell>(list[0]->Evaluate(list.GetScope()));
    std::shared_ptr<Cell> cur_cell = cell;
    std::shared_ptr<Number> pos = As<Number>(list[1]->Evaluate(list.GetScope()));
    int64_t counter = 1;
    while (counter < pos->GetValue()) {
        if (cur_cell->GetSecond() == nullptr) {
            throw RuntimeError(" ");
        }
        ++counter;
        cur_cell = As<Cell>(cur_cell->GetSecond());
    }
    return cur_cell->GetSecond();
}

std::shared_ptr<Object> If(ObjectVector& list) {
    AssertLengthLessEq<SyntaxError>(list, 3);
    AssertLengthMoreEq<SyntaxError>(list, 2);
    std::shared_ptr<Object> condition = list[0]->Evaluate(list.GetScope());
    if (condition.get()->operator bool()) {
        return list[1]->Evaluate(list.GetScope());
    } else {
        if (list.size() == 2) {
            return nullptr;
        } else {
            return list[2]->Evaluate(list.GetScope());
        }
    }
}

std::shared_ptr<Object> Define(ObjectVector& list) {
    AssertLengthMoreEq<SyntaxError>(list, 1);
    if (Is<Symbol>(list[0])) {
        AssertLength<SyntaxError>(list, 2);
        std::shared_ptr<Object> variable = list[1]->Evaluate(list.GetScope());
        list.GetScope()->AddVariable(As<Symbol>(list[0])->GetName(), variable);
        return nullptr;
    } else if (Is<Cell>(list[0])) {
        ObjectVector def_list = EvaluateList(As<Cell>(list[0]));
        AssertLengthMoreEq<SyntaxError>(list, 1);
        std::shared_ptr<Symbol> lambda_name = As<Symbol>(def_list[0]);
        ObjectVector lambda_input = ObjectVectorBase(def_list.begin() + 1, def_list.end());
        ObjectVectorBase lambda_body = ObjectVectorBase(list.begin() + 1, list.end());
        list.GetScope()->AddVariable(
            lambda_name->GetName(),
            std::make_shared<LambdaCreator>(list.GetScope(), lambda_input, lambda_body));
        return nullptr;
    } else {
        throw SyntaxError(" ");
    }
}

std::shared_ptr<Object> Set(ObjectVector& list) {
    AssertLength<SyntaxError>(list, 2);
    std::shared_ptr<Symbol> name = As<Symbol>(list[0]);
    std::shared_ptr<Object> new_def = list[1];
    list.GetScope()->SetVariable(name->GetName(), new_def->Evaluate(list.GetScope()));
    return nullptr;
}

std::shared_ptr<Object> SetCar(ObjectVector& list) {
    AssertLength<SyntaxError>(list, 2);
    std::shared_ptr<Cell> variable = As<Cell>(list[0]->Evaluate(list.GetScope()));
    variable->GetFirst() = list[1]->Evaluate(list.GetScope());
    return nullptr;
}

std::shared_ptr<Object> SetCdr(ObjectVector& list) {
    AssertLength<SyntaxError>(list, 2);
    std::shared_ptr<Cell> variable = As<Cell>(list[0]->Evaluate(list.GetScope()));
    variable->GetSecond() = list[1]->Evaluate(list.GetScope());
    return nullptr;
}

std::shared_ptr<Object> CreateLambda(ObjectVector& list) {
    if (list.size() < 2) {
        throw SyntaxError(" ");
    }
    ObjectVector obj_vec = EvaluateList(list[0]);
    obj_vec.GetScope() = list.GetScope();
    ObjectVectorBase base = ObjectVectorBase(list.begin() + 1, list.end());
    return Lambda::CreateLambda(obj_vec, base);
}

std::shared_ptr<Object> Quote(ObjectVector& list) {
    AssertLength<SyntaxError>(list, 1);
    return list[0];
}

std::shared_ptr<Object> IsSymbol(ObjectVector& list) {
    AssertLength<SyntaxError>(list, 1);
    return std::make_shared<Bool>(Is<Symbol>(list[0]->Evaluate(list.GetScope())));
}

void InsertBooleanFunctions() {
    FunctionsKeeper& instance = FunctionsKeeper::Instance();
    instance.InsertFunction("boolean?", IsBoolean);
    instance.InsertFunction("not", NotBoolean);
    instance.InsertFunction("and", AndBoolean);
    instance.InsertFunction("or", OrBoolean);
}

void InsertIntegerFunctions() {
    FunctionsKeeper& instance = FunctionsKeeper::Instance();
    instance.InsertFunction("number?", IsInteger);
    instance.InsertFunction("=", EqInteger);
    instance.InsertFunction("<", LessInteger);
    instance.InsertFunction(">", BiggerInteger);
    instance.InsertFunction(">=", BiggerEqInteger);
    instance.InsertFunction("<=", LessEqInteger);
    instance.InsertFunction("+", PlusInteger);
    instance.InsertFunction("-", MinusInteger);
    instance.InsertFunction("*", ProductInteger);
    instance.InsertFunction("/", DivisionInteger);
    instance.InsertFunction("min", MinInteger);
    instance.InsertFunction("max", MaxInteger);
    instance.InsertFunction("abs", AbsInteger);
}

void InsertListFunctions() {
    FunctionsKeeper& instance = FunctionsKeeper::Instance();
    instance.InsertFunction("pair?", IsPairList);
    instance.InsertFunction("list?", IsListList);
    instance.InsertFunction("null?", IsNullList);
    instance.InsertFunction("cons", ConsList);
    instance.InsertFunction("car", CarList);
    instance.InsertFunction("cdr", CdrList);
    instance.InsertFunction("list", ListList);
    instance.InsertFunction("list-ref", ListRefList);
    instance.InsertFunction("list-tail", ListTailList);
}

void InsertOtherFunctions() {
    FunctionsKeeper& instance = FunctionsKeeper::Instance();
    instance.InsertFunction("quote", Quote);
    instance.InsertFunction("if", If);
    instance.InsertFunction("define", Define);
    instance.InsertFunction("set!", Set);
    instance.InsertFunction("set-car!", SetCar);
    instance.InsertFunction("set-cdr!", SetCdr);
    instance.InsertFunction("lambda", CreateLambda);
    instance.InsertFunction("symbol?", IsSymbol);
}

void InitializeFunctionKeeper() {
    FunctionsKeeper& instance = FunctionsKeeper::Instance();
    InsertBooleanFunctions();
    InsertIntegerFunctions();
    InsertListFunctions();
    InsertOtherFunctions();
}