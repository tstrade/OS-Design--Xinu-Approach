    .text
    .global th_ctxsw

th_ctxsw:   push {r0-r11, lr}   ; Push regs 0 - 11 and lr
            push {lr}           ; Push return address
            mrs r2, cpsr        ; Obtain status from coprocessor
            push {r2}           ;   and push onto stack
            str sp, [r0]        ; Save old SP
            ldr sp, [r1]        ; Pick up new SP
            pop {r0}            ; Use status as argument and
            bl restore          ;   call restore to restore it
            pop {lr}            ; Pick up return address
            pop {r0-r12}        ; Restore other registers
            mov pc, r12         ; Return to new thread