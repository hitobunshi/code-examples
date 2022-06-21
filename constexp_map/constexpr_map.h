#pragma once

#include <utility>
#include <exception>
#include <stdexcept>
#include <optional>
#include <array>

template <class K, class V, int MaxSize = 8>
class ConstexprMap {
private:
    std::array<std::pair<K, V>, MaxSize> data_;
    size_t size_ = 0;

public:
    constexpr ConstexprMap() = default;

    constexpr V& operator[](const K& key) {
        for (size_t i = 0; i < size_; ++i) {
            auto& [elem_key, elem_value] = data_[i];
            if (elem_key == key) {
                return elem_value;
            }
        }
        if (size_ == MaxSize) {
            throw std::runtime_error("ConstexprMap Overflow");
        }
        data_[size_] = std::make_pair(key, V());
        ++size_;
        return data_[size_ - 1].second;
    }

    constexpr const V& operator[](const K& key) const {
        for (size_t i = 0; i < size_; ++i) {
            const auto& [elem_key, elem_value] = data_[i];
            if (elem_key == key) {
                return elem_value;
            }
        }
        throw std::runtime_error("ConstexprMap Overflow");
    }

    constexpr bool Erase(const K& key) {
        for (auto it = data_.begin(); it != data_.end(); ++it) {
            if (it->first == key) {
                for (auto jt = it; jt + 1 != data_.end(); ++jt) {
                    jt->swap(*(jt + 1));
                }
                --size_;
                return true;
            }
        }
        return false;
    }

    constexpr bool Find(const K& key) const {
        for (auto it = data_.begin(); it != data_.end(); ++it) {
            if (it->first == key) {
                return true;
            }
        }
        return false;
    }

    constexpr size_t Size() const {
        return size_;
    }

    constexpr std::pair<K, V>& GetByIndex(size_t pos) {
        if (pos >= size_) {
            throw std::runtime_error("ConstexprMap Overflow");
        }
        size_t i = 0;
        for (auto& elem : data_) {
            if (i == pos) {
                return elem;
            }
            ++i;
        }
    }

    constexpr const std::pair<K, V>& GetByIndex(size_t pos) const {
        if (pos >= data_.size()) {
            throw std::runtime_error("ConstexprMap Overflow");
        }
        size_t i = 0;
        for (const auto& elem : data_) {
            if (i == pos) {
                return elem;
            }
            ++i;
        }
    }
};
