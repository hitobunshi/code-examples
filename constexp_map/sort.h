#pragma once

#include <constexpr_map.h>

#include <type_traits>
#include <array>
#include <algorithm>

template<class K, class V, int S>
constexpr auto Sort(ConstexprMap<K, V, S> map) {
    ConstexprMap<K, V, S> res = map;
    if constexpr (std::is_integral_v<K>) {
        for (size_t i = 0; i < res.Size() - 1; ++i) {
            for (size_t j = 0; j < res.Size() - 1; ++j) {
                if (res.GetByIndex(j).first < res.GetByIndex(j + 1).first) {
                    std::swap(res.GetByIndex(j), res.GetByIndex(j + 1));
                }
            }
        }
    } else {
        for (size_t i = 0; i < res.Size() - 1; ++i) {
            for (size_t j = 0; j < res.Size() - 1; ++j) {
                if (res.GetByIndex(j + 1).first < res.GetByIndex(j).first) {
                    std::swap(res.GetByIndex(j), res.GetByIndex(j + 1));
                }
            }
        }
    }
    return res;
}
