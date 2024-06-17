#pragma once

#include "../Awaitable.h"
#include "../expected_wrapper.h"

/* what I want
 *
 * // n is the buffer size, strategy is the enqueue strategy [drop head | drop tail | block]
 * Channel<T>  chann(n, strategy);
 *
 * auto [receiver, sender] = chann.split();
 *
 * // async write & read with optional timeout
 * co_await sender.write(value, timeout); // return Expected<void, error>, error may be EOC (end of channel) or timeout
 * Excepted<T, std::error_code> val = co_await receiver.next(timeout); // return Excepted<T, error>, error may be EOC (end of channel) or timeout
 *
 * // sender close, receiver will get EOC
 * sender.close();
 *
 *
 */

namespace underthere {
namespace channels {
using timeout_t = std::chrono::milliseconds;
enum class Strategy {
  DropHead,
  DropTail,
  Block
};

template<typename T>
class Queue {
 public:
  virtual ~Queue() = default;
  virtual auto push(T &&value, std::optional<timeout_t> timeout = std::nullopt) -> void = 0;
  virtual auto push(const T &value, std::optional<timeout_t> timeout = std::nullopt) -> void = 0;
  virtual auto pop() -> T = 0;
  virtual auto empty() const -> bool = 0;
  virtual auto full() const -> bool = 0;
};

template<typename T>
class DropHeadQueue : public Queue<T> {};

template<typename T>
class DropTailQueue : public Queue<T> {};

template<typename T>
class BlockQueue : public Queue<T> {
 public:
  explicit BlockQueue(size_t n) : n_(n) {}
  auto push(T &&value, std::optional<timeout_t> timeout = std::nullopt) -> void override {
    // TODO
  }
  auto push(const T &value, std::optional<timeout_t> timeout = std::nullopt) -> void override {
    // TODO
  }
  auto pop() -> T override {
    // TODO
  }
  auto empty() const -> bool override {
    // TODO
  }
  auto full() const -> bool override {
    // TODO
  }
 private:
  size_t n_;
};

template<typename T>
inline static auto makeQueue(size_t n, Strategy strategy) -> std::unique_ptr<Queue<T>> {
  switch (strategy) {
    case Strategy::DropHead:return std::make_unique<DropHeadQueue<T>>(n);
    case Strategy::DropTail:return std::make_unique<DropTailQueue<T>>(n);
    case Strategy::Block:return std::make_unique<BlockQueue<T>>(n);
  }
  return nullptr;
}

template<typename T>
class ChannelBridge {
 public:
  virtual ~ChannelBridge() = default;
  virtual auto closed() const -> bool = 0;
  // can only be called by sender
  virtual auto senderPush(T &&value, std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<void,
                                                                                                            std::error_code>> = 0;
  virtual auto senderPush(const T &value, std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<void,
                                                                                                                 std::error_code>> = 0;
  virtual auto senderClose() -> void = 0;
  // can only be called by receiver
  virtual auto receiverPop(std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<T,
                                                                                                  std::error_code>> = 0;
};

template<typename T>
class ChannelBridgeImpl : public virtual ChannelBridge<T> {
 public:
  ChannelBridgeImpl(size_t n, Strategy strategy);
  virtual ~ChannelBridgeImpl() = default;
  virtual auto closed() const -> bool override;
  virtual auto senderPush(T &&value, std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<void,
                                                                                                            std::error_code>> override;
  virtual auto senderPush(const T &value, std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<void,
                                                                                                                 std::error_code>> override;
  virtual auto senderClose() -> void override;
  virtual auto receiverPop(std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<T,
                                                                                                  std::error_code>> override;
 private:
  std::unique_ptr<Queue<T>> queue_;
};

template<typename T>
class Sender {
 public:
  explicit Sender(std::shared_ptr<ChannelBridge<T>> bridge);
  virtual ~Sender() = default;
  auto write(T &&value, std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<void, std::error_code>>;
  auto write(const T &value, std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<void,
                                                                                                    std::error_code>>;
  auto close() -> void;

 private:
  std::shared_ptr<ChannelBridge<T>> bridge_;
};

template<typename T>
class Receiver {
 public:
  explicit Receiver(std::shared_ptr<ChannelBridge<T>> bridge);
  virtual ~Receiver() = default;
  auto next(std::optional<timeout_t> timeout = std::nullopt) -> Awaitable<Expected<T, std::error_code>>;

 private:
  std::shared_ptr<ChannelBridge<T>> bridge_;
};

template<typename T>
class Channel {
 public:
  Channel(size_t n, Strategy strategy);
  virtual ~Channel() = default;
  virtual auto split() -> std::pair<Receiver<T>, Sender<T>> = 0;

 private:
  std::shared_ptr<ChannelBridge<T>> bridge_;
};

// -------- Implementation --------

template<typename T>
ChannelBridgeImpl<T>::ChannelBridgeImpl(size_t n, underthere::channels::Strategy strategy) {
  queue_ = makeQueue<T>(n, strategy);
}

template<typename T>
auto ChannelBridgeImpl<T>::closed() const -> bool {
  return {};
}

template<typename T>
auto ChannelBridgeImpl<T>::senderPush(T &&value, std::optional<timeout_t> timeout) -> Awaitable<Expected<void,
                                                                                                         std::error_code>> {
  co_return {};
}

template<typename T>
auto ChannelBridgeImpl<T>::senderPush(const T &value, std::optional<timeout_t> timeout) -> Awaitable<Expected<void,
                                                                                                              std::error_code>> {
  co_return {};
}

template<typename T>
auto ChannelBridgeImpl<T>::senderClose() -> void {
  // TODO
}

template<typename T>
auto ChannelBridgeImpl<T>::receiverPop(std::optional<timeout_t> timeout) -> Awaitable<Expected<T, std::error_code>> {
  co_return {};
}

template<typename T>
Sender<T>::Sender(std::shared_ptr<ChannelBridge<T>> bridge) : bridge_(std::move(bridge)) {}

template<typename T>
auto Sender<T>::write(T &&value, std::optional<timeout_t> timeout) -> Awaitable<Expected<void, std::error_code>> {
  return bridge_->senderPush(std::forward<T>(value), timeout);
}

template<typename T>
auto Sender<T>::write(const T &value, std::optional<timeout_t> timeout) -> Awaitable<Expected<void, std::error_code>> {
  return bridge_->senderPush(value, timeout);
}

template<typename T>
auto Sender<T>::close() -> void {
  bridge_->senderClose();
}

template<typename T>
Receiver<T>::Receiver(std::shared_ptr<ChannelBridge<T>> bridge) : bridge_(std::move(bridge)) {}

template<typename T>
auto Receiver<T>::next(std::optional<timeout_t> timeout) -> Awaitable<Expected<T, std::error_code>> {
  return bridge_->receiverPop(timeout);
}

template<typename T>
Channel<T>::Channel(size_t n, Strategy strategy) {
  bridge_ = std::make_shared<ChannelBridgeImpl<T>>(n, strategy);
}

template<typename T>
auto Channel<T>::split() -> std::pair<Receiver<T>, Sender<T>> {
  return {Receiver<T>(bridge_), Sender<T>(bridge_)};
}

}
} // underthere
