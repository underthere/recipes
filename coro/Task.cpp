export module ud.coro.Task;


#include <asio.hpp>
#include <coroutine>


namespace ud {
    namespace coro {
        template <typename T>
        class Task;

        class TaskPromiseBase {
            class Awaiter {
            public:
                auto await_ready() noexcept -> bool { return false; };
                template <class Promise>
                auto await_suspend(std::coroutine_handle<Promise> h) noexcept -> std::coroutine_handle<> {
                    auto& promise = h.promise();

                };
            };
        };

        template <typename T>
        class Task {
            struct promise_type;
        };
    } // coro
} // ud

