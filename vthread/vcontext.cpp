//
// Created on 2024/6/26.
//

#include "vcontext.h"
#include <cassert>


thread_local jump_buf_link g_unthreaded_context;
thread_local jump_buf_link* g_current_context = nullptr;

auto jump_buf_link::switch_in() -> void {
    link = std::exchange(g_current_context, this);
    if (!link) {
        link = &g_unthreaded_context;
    }
    auto t = _vthread_jump_fcontext(fcontext, thread);
    fcontext = t.ctx;
}

auto jump_buf_link::switch_out() -> void {
    g_current_context = link;
    auto t = _vthread_jump_fcontext(link->fcontext, thread);
    fcontext = t.ctx;
}

auto jump_buf_link::final_switch_out() -> void {
    g_current_context = link;
    _vthread_jump_fcontext(link->fcontext, thread);
    assert(false);
}

auto get_current() -> vthread* { return g_current_context->thread; }
