//
// Created on 2024/6/26.
//

#pragma once

#include <cstdlib>
#include <functional>
#include <memory>

typedef void* vthread_frame_t;
typedef void* vthread_stack_pointer_t;
struct vthread_transfer_t {
    void* ctx;
    void* data;
};

extern "C" {
auto _vthread_make_fcontext(vthread_frame_t sp, std::size_t size,
                           void (*fn)(vthread_transfer_t))
    -> vthread_stack_pointer_t;

auto _vthread_jump_fcontext(vthread_stack_pointer_t to,
                           void* vp) -> vthread_transfer_t;
}

class vthread;
struct jump_buf_link;

struct jump_buf_link {
    vthread_frame_t fcontext{nullptr};
    jump_buf_link* link = nullptr;
    vthread* thread{nullptr};

    auto switch_in() -> void;
    auto switch_out() -> void;
    auto final_switch_out() -> void;
};

class vthread {
    struct stack_deleter {
        auto operator()(char* ptr) const noexcept -> void { delete[] ptr; }
    };
    using vstack = std::unique_ptr<char[], stack_deleter>;
    auto make_stack() -> vstack {
        auto stack = vstack(new char[stack_size_]);
        return stack;
    }

public:
    explicit vthread(std::function<void()> func, size_t stack_size = 1024 * 512)
        : func_(std::move(func)), stack_size_(stack_size) {
        setup();
    }

    auto main() -> void {
#if defined(__aarch64__)
        asm(".cfi_undefined x30");
#elif defined(__x86_64__)
        asm(".cfi_undefined rip");
#endif
        func_();
        done_ = true;
        context_.final_switch_out();
    }

    static auto s_main(vthread_transfer_t t) -> void {
        auto q = reinterpret_cast<vthread*>(t.data);
        q->context_.link->fcontext = t.ctx;
        q->main();
    }

    auto setup() -> void {
        context_.fcontext = _vthread_make_fcontext(
            stack_.get() + stack_size_, stack_size_, vthread::s_main);
        context_.thread = this;
        /*
         * TODO! Find out why? Phenomenon: If `switch_in` is called in the
         * constructor, it will cause a segmentation fault if `switch_out` is
         * called more than once.
         * */
        // context_.switch_in();
    }

    auto switch_in() -> void { context_.switch_in(); }
    auto switch_out() -> void { context_.switch_out(); }

    std::size_t stack_size_{1024 * 512};
    vstack stack_{make_stack()};
    std::function<auto()->void> func_;
    jump_buf_link context_;

    bool joined_{false};
    bool done_{false};
};

auto get_current() -> vthread*;