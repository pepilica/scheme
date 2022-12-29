#pragma once

#include <utility>
#include <functional>
#include <type_traits>

union FunctionStorage {
    const void* ptr;
    void (*idk)();
};

template <typename F, typename R, typename... Args>
R CallFunctor(FunctionStorage storage, Args... args) {
    auto f = reinterpret_cast<const F*>(storage.ptr);
    if constexpr (std::is_void_v<R>) {
        (*f)(std::forward<Args>(args)...);
    } else {
        return (*f)(std::forward<Args>(args)...);
    }
}

template <typename F, typename R, typename... Args>
R CallFunction(FunctionStorage storage, Args... args) {
    auto f = reinterpret_cast<F>(storage.idk);
    if constexpr (std::is_void_v<R>) {
        f(std::forward<Args>(args)...);
    } else {
        return f(std::forward<Args>(args)...);
    }
}

template <typename Signature>
class FunctionRef;

template <typename ReturnType, typename... Args>
class FunctionRef<ReturnType(Args...)> {
public:
    using Invoker = ReturnType (*)(FunctionStorage, Args...);

    template <typename FunctionPtr>
    FunctionRef(FunctionPtr* ptr) : invoker_(CallFunction<FunctionPtr*, ReturnType, Args...>) {
        storage_.idk = reinterpret_cast<decltype(storage_.idk)>(ptr);
    }

    template <typename Functor>
    FunctionRef(const Functor& functor) : invoker_(CallFunctor<Functor, ReturnType, Args...>) {
        storage_.ptr = &functor;
    }

    ReturnType operator()(Args... args) const {
        return invoker_(storage_, std::forward<Args>(args)...);
    }

private:
    FunctionStorage storage_;
    Invoker invoker_;
};
