#define _GNU_SOURCE

#include "hypercall.h"

#include "memory.h"

#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Handle a system call from the guest app as a hypercall.
 *
 * Returns 1 if the guest should keep executing, 0 otherwise.
 */
int hypercall(struct kvm_regs *regs, uint8_t *memory, int vcpufd) {
  switch (HC_NUM) {
  case __NR_exit: // exit
  case __NR_exit_group: // exit_group
    // STUDENT CODE HERE
    printf("[HC] exit(%#x)\n", ARG1);
    return 0;
  case __NR_arch_prctl: // arch_prctl
  case __NR_set_tid_address: // set_tid_address
    /* They are used by musl to set up threading: ignore. */
    fprintf(stderr, "ignored hypercall %lld\n", HC_NUM);
    break;
  default:
    fprintf(stderr, "unsupported hypercall %lld\n", HC_NUM);
    break;
  }

  /* Update guest registers after hypercall operation. */
  ioctl(vcpufd, KVM_SET_REGS, regs);

  return 1;
}
