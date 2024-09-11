#pragma once


#include "Error.h"
#include "Result.h"

namespace underthere {
template <typename T>
class FutureState {
public:
    auto hasResult() -> bool;
    auto hasException() -> bool;

    auto setResult(Result<T, Error>&& value) -> void;
};
}