#include <folly/channels/Channel.h>
#include <folly/experimental/coro/Task.h>
#include <folly/experimental/coro/Sleep.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Collect.h>
#include <folly/Format.h>
#include <iostream>
#include <optional>
#include <any>
#include <proxy/proxy.h>

namespace spec {
    PRO_DEF_MEMBER_DISPATCH(at, std::string(int));
    PRO_DEF_FACADE(Dict, at);
}

void test_print(pro::proxy<spec::Dict> dict) {
    std::cout << dict.at(1) << std::endl;
}

using namespace folly::coro;
using Sender = folly::channels::Sender<std::any>;
using Receiver = folly::channels::Receiver<std::any>;

struct C {
    C() { std::cout << "C()\n"; }

    C(const C &) { std::cout << "C(const C&)\n"; }

    C(C &&) noexcept { std::cout << "C(C&&)\n"; }

    ~C() { std::cout << "~C()\n"; }
};

auto data_gen(folly::channels::Sender<int> sender) -> Task<void> {
    for (int i = 0; i < 99999999; i++) {
        sender.write(i);
        std::cout << std::format("data_gen {}\n", i);
        // co_await folly::coro::sleep(std::chrono::milliseconds(100));
    }
    co_return;
}

auto data_recv(folly::channels::Receiver<int> receiver) -> Task<void> {
    co_return ;
    while (auto data = co_await receiver.next()) {
        std::cout << fmt::format("data: {}\n", data.value());
    }
    co_return;
}

auto async_main(int argc, char **argv) -> Task<int> {
    auto [r, s] = std::move(folly::channels::Channel<int>::create());
    auto t0 = data_gen(std::move(s));
    auto t1 = data_recv(std::move(r));
    co_await folly::coro::collectAll(std::move(t0), std::move(t1));
    co_return 0;
}

struct Caps {
};

struct Pad {
    virtual auto caps() const -> const Caps & = 0;

    virtual auto supports(const Caps &caps) const -> bool = 0;
    // virtual auto connected() const -> bool = 0;
};

template<typename T>
struct SrcPad : Pad {
    virtual auto caps() const -> const Caps & {
        return {};
    }

    std::optional<Sender> sender;
};

template<typename T>
struct SinkPad : Pad {
    virtual auto caps() const -> const Caps & {
        return {};
    }

    std::optional<Receiver> receiver;
};

template<typename T>
auto connect(SrcPad<T> &src, SinkPad<T> &sink) -> bool {
    if (!src.supports(sink.caps()) || !sink.supports(src.caps())) {
        return false;
    }
    auto [receiver, sender] = folly::channels::Channel<std::any>::create();
    src.sender = std::move(sender);
    sink.receiver = std::move(receiver);
    return true;
}


int main(int argc, char **argv) {
    auto executor = folly::getGlobalCPUExecutor();
    auto task = async_main(argc, argv);
    return blockingWait(std::move(task).scheduleOn(executor));

}
