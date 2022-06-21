#pragma once

#include "sw_fwd.h" // forward declaration

#include <algorithm>
#include <any>
#include <utility>
#include <cstddef>
#include <iostream>
#include <type_traits>

struct SharedPtrObject {
    size_t use_count_;
    bool allocated_with_make_share_;

    SharedPtrObject(size_t use_count, bool allocated_with_make_share = false)
        : use_count_(use_count), allocated_with_make_share_(allocated_with_make_share) {
    }

    virtual ~SharedPtrObject() = 0;
};

SharedPtrObject::~SharedPtrObject() {
}

template <typename T>
struct SharedPtrObjectWithType : public SharedPtrObject {
    T* ptr_;

    SharedPtrObjectWithType(T* ptr, bool allocated_with_make_share = false)
        : SharedPtrObject(1, allocated_with_make_share), ptr_(ptr) {
    }

    ~SharedPtrObjectWithType() {
        if (ptr_ != nullptr && allocated_with_make_share_) {
            ptr_->~T();
        } else {
            delete ptr_;
        }
    }
};

template <typename T>
class SharedPtr {
private:
    SharedPtrObject* managed_ptr_;
    T* ptr_;
    char* buf_ = nullptr;

    template <typename Y>
    friend class SharedPtr;

    void Unshare();
    void IncrementUseCount();

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr();
    SharedPtr(std::nullptr_t);
    template <typename Y>
    explicit SharedPtr(Y* ptr) : managed_ptr_(new SharedPtrObjectWithType<Y>(ptr)), ptr_(ptr) {
    }

    SharedPtr(const SharedPtr& other);
    SharedPtr(SharedPtr&& other);

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) : managed_ptr_(other.managed_ptr_), ptr_(other.ptr_) {
        IncrementUseCount();
    }

    SharedPtr(T* ptr, char* block_location)
        : managed_ptr_(new (block_location) SharedPtrObjectWithType<T>(ptr, true)),
          ptr_(ptr),
          buf_(block_location) {
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other)
        : managed_ptr_(other.managed_ptr_), ptr_(other.ptr_), buf_(other.buf_) {
        IncrementUseCount();
        other.Unshare();
        other.ptr_ = nullptr;
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr)
        : managed_ptr_(other.managed_ptr_), ptr_(ptr), buf_(other.buf_) {
        IncrementUseCount();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other);
    SharedPtr& operator=(SharedPtr&& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset();

    template <typename Y>
    void Reset(Y* ptr) {
        Unshare();
        managed_ptr_ = new SharedPtrObjectWithType<Y>(ptr);
        ptr_ = ptr;
    }

    void Swap(SharedPtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const;
    T& operator*() const;
    T* operator->() const;
    size_t UseCount() const;
    explicit operator bool() const;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    char* buf = new char[sizeof(SharedPtrObjectWithType<T>) + sizeof(T)];
    auto block_location = buf;
    auto element_location = buf + sizeof(SharedPtrObjectWithType<T>);
    T* ptr = new (element_location) T(std::forward<Args>(args)...);
    return SharedPtr<T>(ptr, block_location);
}

template <typename T>
void SharedPtr<T>::Unshare() {
    if (managed_ptr_) {
        --managed_ptr_->use_count_;
        if (managed_ptr_->use_count_ == 0) {
            if (buf_) {
                managed_ptr_->~SharedPtrObject();
                delete[] buf_;
                buf_ = nullptr;
            } else {
                delete managed_ptr_;
            }
        }
        managed_ptr_ = nullptr;
    }
}

template <typename T>
void SharedPtr<T>::IncrementUseCount() {
    if (managed_ptr_) {
        ++managed_ptr_->use_count_;
    }
}

template <typename T>
SharedPtr<T>::SharedPtr() : managed_ptr_(nullptr), ptr_(nullptr) {
}

template <typename T>
SharedPtr<T>::SharedPtr(std::nullptr_t) : managed_ptr_(nullptr), ptr_(nullptr) {
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other)
    : managed_ptr_(other.managed_ptr_), ptr_(other.ptr_), buf_(other.buf_) {
    IncrementUseCount();
}

template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other)
    : managed_ptr_(other.managed_ptr_), ptr_(other.ptr_), buf_(other.buf_) {
    IncrementUseCount();
    other.Unshare();
    other.ptr_ = nullptr;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
    if (managed_ptr_ != other.managed_ptr_) {
        Unshare();
        managed_ptr_ = other.managed_ptr_;
        IncrementUseCount();
    }
    buf_ = other.buf_;
    ptr_ = other.ptr_;
    return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) {
    if (managed_ptr_ != other.managed_ptr_) {
        Unshare();
        managed_ptr_ = other.managed_ptr_;
        buf_ = other.buf_;
        IncrementUseCount();
        other.Unshare();
    }
    ptr_ = other.ptr_;
    other.ptr_ = nullptr;
    return *this;
}

template <typename T>
SharedPtr<T>::~SharedPtr() {
    Unshare();
}

template <typename T>
void SharedPtr<T>::Reset() {
    Unshare();
    ptr_ = nullptr;
}

template <typename T>
void SharedPtr<T>::Swap(SharedPtr& other) {
    std::swap(managed_ptr_, other.managed_ptr_);
    std::swap(ptr_, other.ptr_);
}

template <typename T>
T* SharedPtr<T>::Get() const {
    return ptr_;
}

template <typename T>
T& SharedPtr<T>::operator*() const {
    return *ptr_;
}

template <typename T>
T* SharedPtr<T>::operator->() const {
    return ptr_;
}

template <typename T>
size_t SharedPtr<T>::UseCount() const {
    if (managed_ptr_) {
        return managed_ptr_->use_count_;
    }
    return 0;
}

template <typename T>
SharedPtr<T>::operator bool() const {
    return ptr_ != nullptr;
}