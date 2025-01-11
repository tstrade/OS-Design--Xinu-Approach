	.text
	.globl ctxsw

ctxsw:
	pushl %ebp           ; Push ebp onto stack
	movl %esp,%ebp	     ; Record current SP in ebp
	pushfl		     ; Push flags onto stack
	pushal		     ; Push general regs. on stack

	movl 8(%ebp),%eax 	; Get mem location to save old SP
	movl %esp,(%eax)	; Save old SP
	movl 12(%ebp),%eax	; Get location to restore new SP

	movl (%eax),%esp	; Pop new SP

	popal			; Restore general registers
	movl 4(%esp),%ebp	; Pick up ebp before restoring interrupts

	popfl			; Restore interrupt mask
	add $4,%esp		; Skip saved value of ebp
	ret			; Return to new process
