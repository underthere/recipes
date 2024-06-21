.text
.balign 16
.global __vthread_jump
.global __vthread_make_context

__vthread_make_context:


finish:
    mov x0, #0
    bl __exit

