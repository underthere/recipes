//
// Created on 2024/6/28.
//

#pragma once

#include <climits>
#include <cstdint>
#include <functional>
#include "NoCopy.h"

namespace underthere {

using Func = std::function<void()>;

class Executor: public NoCopy {
public:
    static constexpr int8_t LOWEST_PRIORITY = -128;
    static constexpr int8_t NORMAL_PRIORITY = 0;
    static constexpr int8_t HIGHEST_PRIORITY = 127;
    virtual ~Executor() = default;
    virtual auto schedule(Func&& func, int8_t priority = NORMAL_PRIORITY) -> void = 0;
};

}  // namespace underthere
