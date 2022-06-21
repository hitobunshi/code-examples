#pragma once

#include <utility>
#include <iostream>

template <typename Callback>
class CallbackStorage {
private:
    alignas(Callback) char callback_buffer_[sizeof(Callback)];
    bool valid_;

public:
    explicit CallbackStorage(Callback callback) : valid_(true) {
        new (GetCallbackBuffer()) Callback(std::move(callback));
    }

    void* GetCallbackBuffer() {
        return static_cast<void*>(callback_buffer_);
    }

    Callback& GetCallback() {
        return *reinterpret_cast<Callback*>(callback_buffer_);
    }

    ~CallbackStorage() {
    }

    void Invalidate() {
        valid_ = false;
    }

    bool IsValid() {
        return valid_;
    }
};

template <typename Callback>
class Defer final {
private:
    CallbackStorage<Callback> storage_;

public:
    Defer(Callback callback) : storage_(std::move(callback)) {
    }

    ~Defer() {
        Invoke();
    }

    void Cancel() {
        if (storage_.IsValid()) {
            storage_.GetCallback().~Callback();
            storage_.Invalidate();
        }
    }

    void Invoke() {
        if (storage_.IsValid()) {
            std::move(storage_.GetCallback())();
            storage_.Invalidate();
        }
    }
};
