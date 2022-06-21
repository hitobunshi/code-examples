#pragma once

#include <utility>
#include <type_traits>

template <typename T, std::size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
class CompressedPairElement {
public:
    T value;

    CompressedPairElement() : value() {
    }

    CompressedPairElement(const T& val) : value(val) {
    }

    CompressedPairElement(T&& val) : value(std::forward<T>(val)) {
    }

    const T& Get() const {
        return value;
    }

    T& Get() {
        return value;
    }
};

template <typename T, std::size_t I>
class CompressedPairElement<T, I, true> : public T {
public:
    CompressedPairElement() : T() {
    }

    CompressedPairElement(const T& val) : T(val) {
    }

    CompressedPairElement(T&& val) : T(std::forward<T>(val)) {
    }

    const CompressedPairElement& Get() const {
        return *this;
    }

    CompressedPairElement& Get() {
        return *this;
    }
};

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S>
class CompressedPair : public CompressedPairElement<F, 0>, public CompressedPairElement<S, 1> {
private:
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;

public:
    CompressedPair() {
    }

    CompressedPair(const F& first, const S& second) : First(first), Second(second) {
    }
    CompressedPair(F&& first, const S& second) : First(std::forward<F>(first)), Second(second) {
    }
    CompressedPair(const F& first, S&& second) : First(first), Second(std::forward<S>(second)) {
    }
    CompressedPair(F&& first, S&& second)
        : First(std::forward<F>(first)), Second(std::forward<S>(second)) {
    }

    F& GetFirst() {
        return First::Get();
    }

    const F& GetFirst() const {
        return First::Get();
    }

    S& GetSecond() {
        return Second::Get();
    }

    const S& GetSecond() const {
        return Second::Get();
    };
};