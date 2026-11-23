#ifndef VMM_H
#define VMM_H

#include <stdint.h>

#define DEV_KVM "/dev/kvm"

/* IO port where hypercalls are expected.
 *
 * The syscall wrapper in guest/boot/boot.asm translates a guest app syscall
 * into an out instruction to this port.
 */
#define HC_IO_PORT 0x118

// Guest memory mapped in the host.
extern uint8_t *guest_mem;

int create_vm(void);
int launch_vm(uint64_t boot_gpa, uint64_t app_gpa, uint64_t app_stack_gpa);

#endif
