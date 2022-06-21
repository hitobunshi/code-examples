#pragma once

#include <vector>
#include <string>

struct State {
    int ref_count_;
    std::vector<std::string> storage_;

    // rule of 5
    State();
    State(const State& other);
    State(State&& other) = delete;
    State& operator=(const State& other) = delete;
    State& operator=(State&& other) = delete;
};

class COWVector {
public:
    COWVector();
    ~COWVector();

    COWVector(const COWVector& other);
    COWVector& operator=(const COWVector& other);

    size_t Size() const;

    void Resize(size_t size);

    const std::string& Get(size_t at) const;
    const std::string& Back() const;

    void PushBack(const std::string& value);

    void Set(size_t at, const std::string& value);

private:
    void CopyState();
    State* state_;
};

// State

State::State() : ref_count_(1), storage_() {
}

State::State(const State& other) : ref_count_(1), storage_(other.storage_) {
}

// COWVector

COWVector::COWVector() : state_(new State()) {
}

COWVector::~COWVector() {
    --state_->ref_count_;
    if (state_->ref_count_ == 0) {
        delete state_;
    }
}

COWVector::COWVector(const COWVector& other) : state_(other.state_) {
    ++state_->ref_count_;
}

COWVector& COWVector::operator=(const COWVector& other) {
    --state_->ref_count_;
    if (state_->ref_count_ == 0) {
        delete state_;
    }
    state_ = other.state_;
    ++state_->ref_count_;
    return *this;
}

size_t COWVector::Size() const {
    return state_->storage_.size();
}

void COWVector::Resize(size_t size) {
    CopyState();
    state_->storage_.resize(size);
}

const std::string& COWVector::Get(size_t at) const {
    return state_->storage_[at];
}

const std::string& COWVector::Back() const {
    return state_->storage_.back();
}

void COWVector::PushBack(const std::string& value) {
    CopyState();
    state_->storage_.push_back(value);
}

void COWVector::Set(size_t at, const std::string& value) {
    CopyState();
    state_->storage_[at] = value;
}

void COWVector::CopyState() {
    if (state_->ref_count_ > 1) {
        --state_->ref_count_;
        State* new_state = new State(*state_);
        state_ = new_state;
    }
}