#pragma once

#include <type_traits>
#include "Error.h"
#include "Future.h"
#include "FutureState.h"
#include "Result.h"
#include "ScopeGuard.h"

namespace underthere {

template <typename T>
class Promise {
public:
public:
    auto valid() -> bool { return _state != nullptr; }
    auto getFuture() -> Result<Future<T>, Error> {
        if (!valid() || futureCreated) {
            return Error(Error{});
        }
        ScopeGuard guard([this] { futureCreated = true; });
        return Future<T>();
    }
    auto setValue() -> void
        requires(std::is_void_v<T>);
    auto setValue(T&& value) -> void;
    auto setValue(Result<T, Error>&& value) -> void;
    auto setException() -> void;

private:
    std::shared_ptr<FutureState<T>> _state = nullptr;
    bool futureCreated = false;
};
}  // namespace underthere