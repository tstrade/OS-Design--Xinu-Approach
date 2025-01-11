        #include "../inclusions.h"

        ;; Interrupt dispatcher for clock interrupts (x86 version)

        .text
        .globl clkdisp          ; Clock interrupt dispatcher

clkdisp:
        pushal                  ; Save registers
        cli                     ; Disable further interrupts
        movb $EOI,%al           ; Reset interrupt
        outb %al,$OCWI_2

        call clkhandler

        sti                     ; Restore interrupt status
        popal                   ; Restore registers
        iret                    ; Return from interrupt
