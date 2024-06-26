//
// Created on 2024/6/21.
//

#include <iostream>

#include "vcontext.h"




auto ok() -> void {
    for (int i = 0; i < 10; ++i) {
        std::cout << "switch out" << std::endl;
        auto vt = get_current();
        vt->switch_out();
    }
    std::cout << "ok!" << std::endl;
}

auto ping() -> void {
    std::cout << "ping" << std::endl;
    vthread vt(ok);
    while (!vt.done_) {
        std::cout << "switch in" << std::endl;
        vt.switch_in();
    }
}

auto make_coro(std::function<void()> f) -> void {
    vthread vt(f);
    while (!vt.done_) {
        vt.switch_in();
    }
}

int main() {
    make_coro(ping);
    return 0;
}