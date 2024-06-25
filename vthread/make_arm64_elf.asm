.text
.balign 16
.global __vthread_jump
.global __vthread_make_context

 /*
 in args
 x0: asp
 x1: size
 x2: function

 out args
 x0: fctx

 asp = pointer of alloc stack

 ------------- asp

 ------------- asp - 0x10 <- x2

 ------------- asp - 0x18 <- finish

 ------------- asp - 0xb0 -> x0


 */

__vthread_make_context:
    and x0, x0, ~0xF

    ; reserve space for context-data on context-stack
    sub  x0, x0, #0xb0

    ; third arg of make_fcontext() == address of context-function
    ; store address as a PC to jump in
    str  x2, [x0, #0xa0]

    adr  x1, finish

    ; save address of finish as return-address for context-function
    ; will be entered after context-function returns (LR register)
    str  x1, [x0, #0x98]

    ret  lr ; return pointer to context-data (x0)

finish:
    ; exit code is zero
    mov  x0, #0
    ; exit application
    bl  __exit


/*
input
x0: to context
x1: thread context

output:
x0: transfer_t

-------- sp

-------- nsp + 0xa0 <- lr

    x19 - x30

-------- nsp + 0x40

    d8 - d15

-------- sp - 0xb0 = nsp -> x4


*/

__vthread_jump:
    ; prepare stack for GP + FPU
    sub  sp, sp, #0xb0

    ; save d8 - d15
    stp  d8,  d9,  [sp, #0x00]
    stp  d10, d11, [sp, #0x10]
    stp  d12, d13, [sp, #0x20]
    stp  d14, d15, [sp, #0x30]

    ; save x19-x30
    stp  x19, x20, [sp, #0x40]
    stp  x21, x22, [sp, #0x50]
    stp  x23, x24, [sp, #0x60]
    stp  x25, x26, [sp, #0x70]
    stp  x27, x28, [sp, #0x80]
    stp  fp,  lr,  [sp, #0x90]

    ; save LR as PC
    str  lr, [sp, #0xa0]

    ; store RSP (pointing to context-data) in X0
    mov  x4, sp

    ; restore RSP (pointing to context-data) from X1
    mov  sp, x0 ; sp <- new sp

    ; load d8 - d15
    ldp  d8,  d9,  [sp, #0x00]
    ldp  d10, d11, [sp, #0x10]
    ldp  d12, d13, [sp, #0x20]
    ldp  d14, d15, [sp, #0x30]

    ; load x19-x30
    ldp  x19, x20, [sp, #0x40]
    ldp  x21, x22, [sp, #0x50]
    ldp  x23, x24, [sp, #0x60]
    ldp  x25, x26, [sp, #0x70]
    ldp  x27, x28, [sp, #0x80]
    ldp  fp,  lr,  [sp, #0x90]

    ; return transfer_t from jump
    ; pass transfer_t as first arg in context function
    ; X0 == FCTX, X1 == DATA
    mov x0, x4 ; old sp -> x0 to return

    ; load pc
    ldr  x4, [sp, #0xa0]

    ; restore stack from GP + FPU
    add  sp, sp, #0xb0

    ret x4
