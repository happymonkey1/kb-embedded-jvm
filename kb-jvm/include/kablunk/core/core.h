//
// Created by happymonkey1 on 5/26/24.
//

#ifndef KABLUNK_CORE_H
#define KABLUNK_CORE_H

#define KB_CORE

#include <cstdint>
#include <optional>

namespace kb {
using u32 = uint32_t;
using i32 = int32_t;
using u64 = uint64_t;
using i64 = int64_t;

template <typename T>
using option = ::std::optional<T>;
}

#endif //KABLUNK_CORE_H
