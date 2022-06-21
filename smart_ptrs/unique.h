#pragma once

#include "compressed_pair.h"

#include <algorithm>
#include <memory>
#include <cstddef>
#include <type_traits>

template <typename T>
struct Slug {
    Slug() {
    }

    template <typename U>
    Slug(U&&) {
    }

    void operator()(T* ptr) {
        delete ptr;
    }
};

template <typename T>
struct Slug<T[]> {
    Slug() {
    }

    template <typename U>
    Slug(U&&) {
    }

    void operator()(T* ptr) {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
private:
    CompressedPair<T*, Deleter> data_;

    template <typename S, typename OtherDeleter>
    friend class UniquePtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr);

    template <typename U>
    UniquePtr(T* ptr, U&& deleter) : data_(ptr, std::forward<U>(deleter)) {
    }

    UniquePtr(const UniquePtr&) = delete;

    template <typename S, typename OtherDeleter = Slug<T>>
    UniquePtr(UniquePtr<S, OtherDeleter>&& other) noexcept
        : data_(static_cast<T*>(other.data_.GetFirst()),
                std::forward<OtherDeleter>(other.data_.GetSecond())) {
        other.data_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <typename S, typename OtherDeleter>
    UniquePtr& operator=(UniquePtr<S, OtherDeleter>&& other) noexcept {
        if (data_.GetFirst() == static_cast<T*>(other.data_.GetFirst())) {
            return *this;
        }
        data_.GetSecond()(data_.GetFirst());
        data_.GetFirst() = static_cast<T*>(other.data_.GetFirst());
        data_.GetSecond() = std::forward<OtherDeleter>(other.data_.GetSecond());
        other.data_.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t);

    UniquePtr& operator=(const UniquePtr&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release();
    void Reset(T* ptr = nullptr);
    void Swap(UniquePtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const;
    Deleter& GetDeleter();
    const Deleter& GetDeleter() const;
    explicit operator bool() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const;

    T* operator->() const;
};

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::UniquePtr(T* ptr) : data_(ptr, Deleter()) {
}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>& UniquePtr<T, Deleter>::operator=(std::nullptr_t) {
    data_.GetSecond()(data_.GetFirst());
    data_.GetFirst() = nullptr;
    return *this;
}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::~UniquePtr() {
    if (data_.GetFirst() != nullptr) {
        data_.GetSecond()(data_.GetFirst());
    }
}

template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::Release() {
    T* ret = data_.GetFirst();
    data_.GetFirst() = nullptr;
    return ret;
}

template <typename T, typename Deleter>
void UniquePtr<T, Deleter>::Reset(T* ptr) {
    T* old_ptr = data_.GetFirst();
    data_.GetFirst() = ptr;
    if (old_ptr != nullptr) {
        data_.GetSecond()(old_ptr);
    }
}

template <typename T, typename Deleter>
void UniquePtr<T, Deleter>::Swap(UniquePtr& other) {
    std::swap(data_.GetFirst(), other.data_.GetFirst());
    std::swap(data_.GetSecond(), other.data_.GetSecond());
}

template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::Get() const {
    return data_.GetFirst();
}

template <typename T, typename Deleter>
Deleter& UniquePtr<T, Deleter>::GetDeleter() {
    return data_.GetSecond();
}

template <typename T, typename Deleter>
const Deleter& UniquePtr<T, Deleter>::GetDeleter() const {
    return data_.GetSecond();
}

template <typename T, typename Deleter>
UniquePtr<T, Deleter>::operator bool() const {
    return data_.GetFirst() != nullptr;
}

template <typename T, typename Deleter>
std::add_lvalue_reference_t<T> UniquePtr<T, Deleter>::operator*() const {
    return *data_.GetFirst();
}

template <typename T, typename Deleter>
T* UniquePtr<T, Deleter>::operator->() const {
    return data_.GetFirst();
}

// Specialization for arrays

template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    CompressedPair<T*, Deleter> data_;

    template <typename S, typename OtherDeleter>
    friend class UniquePtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr);

    template <typename U>
    UniquePtr(T* ptr, U&& deleter) : data_(ptr, std::forward<U>(deleter)) {
    }

    UniquePtr(const UniquePtr&) = delete;

    template <typename S, typename OtherDeleter = Slug<T>>
    UniquePtr(UniquePtr<S, OtherDeleter>&& other) noexcept
        : data_(static_cast<T*>(other.data_.GetFirst()),
                std::forward<OtherDeleter>(other.data_.GetSecond())) {
        other.data_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <typename S, typename OtherDeleter>
    UniquePtr& operator=(UniquePtr<S, OtherDeleter>&& other) noexcept {
        if (data_.GetFirst() == static_cast<T*>(other.data_.GetFirst())) {
            return *this;
        }
        data_.GetSecond()(data_.GetFirst());
        data_.GetFirst() = static_cast<T*>(other.data_.GetFirst());
        data_.GetSecond() = std::forward<OtherDeleter>(other.data_.GetSecond());
        other.data_.GetFirst() = nullptr;
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t);

    UniquePtr& operator=(const UniquePtr&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release();
    void Reset(T* ptr = nullptr);
    void Swap(UniquePtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const;
    Deleter& GetDeleter();
    const Deleter& GetDeleter() const;
    explicit operator bool() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator[](size_t i) const;

    T* operator->() const;
};

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::UniquePtr(T* ptr) : data_(ptr, Deleter()) {
}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>& UniquePtr<T[], Deleter>::operator=(std::nullptr_t) {
    data_.GetSecond()(data_.GetFirst());
    data_.GetFirst() = nullptr;
    return *this;
}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::~UniquePtr() {
    if (data_.GetFirst() != nullptr) {
        data_.GetSecond()(data_.GetFirst());
    }
}

template <typename T, typename Deleter>
T* UniquePtr<T[], Deleter>::Release() {
    T* ret = data_.GetFirst();
    data_.GetFirst() = nullptr;
    return ret;
}

template <typename T, typename Deleter>
void UniquePtr<T[], Deleter>::Reset(T* ptr) {
    T* old_ptr = data_.GetFirst();
    data_.GetFirst() = ptr;
    if (old_ptr != nullptr) {
        data_.GetSecond()(old_ptr);
    }
}

template <typename T, typename Deleter>
void UniquePtr<T[], Deleter>::Swap(UniquePtr& other) {
    std::swap(data_.GetFirst(), other.data_.GetFirst());
    std::swap(data_.GetSecond(), other.data_.GetSecond());
}

template <typename T, typename Deleter>
T* UniquePtr<T[], Deleter>::Get() const {
    return data_.GetFirst();
}

template <typename T, typename Deleter>
Deleter& UniquePtr<T[], Deleter>::GetDeleter() {
    return data_.GetSecond();
}

template <typename T, typename Deleter>
const Deleter& UniquePtr<T[], Deleter>::GetDeleter() const {
    return data_.GetSecond();
}

template <typename T, typename Deleter>
UniquePtr<T[], Deleter>::operator bool() const {
    return data_.GetFirst() != nullptr;
}

template <typename T, typename Deleter>
std::add_lvalue_reference_t<T> UniquePtr<T[], Deleter>::operator[](size_t i) const {
    return data_.GetFirst()[i];
}

template <typename T, typename Deleter>
T* UniquePtr<T[], Deleter>::operator->() const {
    return data_.GetFirst();
}