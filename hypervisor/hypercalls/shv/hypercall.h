#ifndef HYPERCALL_H
#define HYPERCALL_H

#include <linux/kvm.h>

#include <stdint.h>

/* Syscall number from the guest app, taken as the hypercall number. */
#define HC_NUM regs->rax
/* Return value of the hypercall, i.e. of the guest userspace app's syscall. */
#define HC_RET regs->rax

/* Arguments to the hypercall. */
#define ARG1 regs->rdi
#define ARG2 regs->rsi
/* A syscall's 3rd argument is given in rdx.
 * But our guest's syscall handler (that forwards it as a hypercall, see
 * guest/boot/boot.asm) moves the 3rd argument to r12, because it needs rdx for
 * the out instruction that performs the IO, interpreted as an HC.
 *
 * Note r11 cannot be used because RFLAGS is saved in r11 for the guest kernel
 * to modify RFLAGS for its context, and then be restored from r11 by sysret.
 */
#define ARG3 regs->r12
#define ARG4 regs->r10
#define ARG5 regs->r8
#define ARG6 regs->r9

int hypercall(struct kvm_regs *regs, uint8_t *memory, int vcpufd);

#endif
