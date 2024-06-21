//
// Created on 2024/6/21.
//

#include <cstdlib>

struct vthread_context_t {
    void* sp;
    void* lr;
    void* regs[10];
};

extern "C" {
void _vthread_make_context(void* sp, void (*fn)(void*), void* arg);
void _vthread_jump(vthread_context_t* from, vthread_context_t* to);
}

#include <iostream>

class Coroutine {
public:
    Coroutine(void (*fn)(void*), void* arg, size_t stack_size) {
        stack_ = new char[stack_size];
        fcontext_.sp = stack_ + stack_size;
        fcontext_.lr = reinterpret_cast<void*>(fn);
        _vthread_make_context(fcontext_.sp, fn, arg);
    }

    ~Coroutine() {
        delete[] stack_;
    }

    void resume() {
        _vthread_jump(&main_context_, &fcontext_);
    }

    void yield() {
        _vthread_jump(&fcontext_, &main_context_);
    }

private:
    vthread_context_t fcontext_;
    vthread_context_t main_context_;
    char* stack_;
};

void coro_f(void* arg) {
    auto coro = reinterpret_cast<Coroutine*>(arg);
    std::cout << "coro_f: start" << std::endl;
    coro->yield();
    std::cout << "coro_f: end" << std::endl;
}

int main() {
    Coroutine coro(coro_f, nullptr, 1024*128);
    std::cout << "main: start" << std::endl;
    coro.resume();
    std::cout << "main: end" << std::endl;
    return 0;
}