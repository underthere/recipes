#include <iostream>
#include <coroutine>
#include <format>

template <typename T>
struct ReturnObject {
    struct promise_type {
        T value_;

        ~promise_type() {
            std::cout << "promise_type::~promise_type\n";
        }

        auto get_return_object() -> ReturnObject {
            return {
                    .h_ = std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        std::suspend_never initial_suspend() {
            std::cout << "promise_type::initial_suspend\n";
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            std::cout << "promise_type::final_suspend\n";
            return {};
        }

        void unhandled_exception() {}

        auto yield_value(T value) -> std::suspend_always {
            std::cout << std::format("promise_type::yield_value: {}\n", value); // "promise_type::yield_value: 0\n"
            value_ = value;
            return {};
        }

        auto return_value(T value) -> void {
            std::cout << std::format("promise_type::return_value: {}\n", value); // "promise_type::return_value: 100\n"
            value_ = value;
        }

        auto await_ready() -> bool {
            return false;
        }
    };

    std::coroutine_handle<promise_type> h_;

};

auto make_number() -> ReturnObject<int> {
    static int i {0};
    co_return i++;
}

auto counter4() -> ReturnObject<int> {
    for (int i = 0; i < 10; i++) {
        co_yield i;
    }
    std::cout << "counter4: done\n";
    co_return 100;
}


int main() {
    // std::coroutine_handle<ReturnObject3::promise_type> h = counter3();
    auto h = counter4().h_;

    while (!h.done()) {
        std::cout << std::format("counter4: {}\n", h.promise().value_);
        h();
    }
    std::cout << std::format("counter4 done with: {}\n", h.promise().value_);
    h.destroy();
}
