#pragma once

#include <functional>
#include <utility>
#include <tuple>
#include <memory>
#include <type_traits>

template <class F, class... Args1>
constexpr auto BindFront(F&& f, Args1&&... first) {
    return [ f = std::forward<F>(f),
             ... first = std::forward<Args1>(first) ]<class... Args2>(Args2 && ... args) mutable {
        return f(std::move(first)..., std::forward<Args2>(args)...);
    };
}
