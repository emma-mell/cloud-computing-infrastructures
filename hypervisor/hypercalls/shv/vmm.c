#include "vmm.h"

#include "hypercall.h"
#include "memory_map.h"

#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <err.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int kvmfd, vmfd, vcpufd;
struct kvm_run *vcpu_run;

uint8_t *guest_mem;

/** Handle reasons for VM to exit back to the HV.
 *
 * Returns 1 if the guest should keep executing, 0 otherwise.
 */
static int vmexit_handler(int exit_reason) {
  struct kvm_regs regs;
  struct kvm_sregs sregs;

  ioctl(vcpufd, KVM_GET_REGS, &regs);
  ioctl(vcpufd, KVM_GET_SREGS, &sregs);

  switch (exit_reason) {
  case KVM_EXIT_FAIL_ENTRY:
    errx(
        1, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = %#llx",
        (unsigned long long)vcpu_run->fail_entry.hardware_entry_failure_reason);
  case KVM_EXIT_INTERNAL_ERROR:
    errx(1, "KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x",
         vcpu_run->internal.suberror);
  case KVM_EXIT_SHUTDOWN:
    errx(1, "KVM_EXIT_SHUTDOWN: @rip=%#llx (triple fault or hardware error?)",
         regs.rip);
  default:
    errx(1, "unhandled exit_reason %#x: @rip=%#llx", vcpu_run->exit_reason,
         regs.rip);
  }

  return 1;
}

int create_vm(void) {
  int ret = 0;

  size_t mmap_size;
  struct kvm_sregs sregs;
  int nent = 128;
  struct kvm_cpuid2 *cpuid2;

  /* Open the KVM device. */
  kvmfd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
  if (kvmfd == -1)
    err(1, "/dev/kvm");

  /* Make sure we have the stable version of the API */
  ret = ioctl(kvmfd, KVM_GET_API_VERSION, NULL);
  if (ret == -1)
    err(1, "KVM_GET_API_VERSION");
  if (ret != 12)
    errx(1, "KVM_GET_API_VERSION %d, expected 12", ret);

  /* Create a KVM virtual machine. */
  vmfd = ioctl(kvmfd, KVM_CREATE_VM, (unsigned long)0);
  if (vmfd == -1)
    err(1, "KVM_CREATE_VM");

  /* Allocate initial guest memory. */
  guest_mem = mmap(NULL, GUEST_INIT_MEM_SZ, PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (!guest_mem)
    err(1, "allocating guest memory");

  /* Map it. */
  struct kvm_userspace_memory_region region = {
      .slot = 0,
      .guest_phys_addr = 0,
      .memory_size = GUEST_INIT_MEM_SZ,
      .userspace_addr = (uint64_t)guest_mem,
  };
  ret = ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region);
  if (ret == -1)
    err(1, "KVM_SET_USER_MEMORY_REGION");

  /* Spawn a vCPU for the VM.
   * This time we will enable long mode, i.e., expose a virtualized modern CPU
   * to the guest, so we need to fetch the host and KVM cpuid capabilities, and
   * set them to the VM.
   */
  cpuid2 = malloc(sizeof(struct kvm_cpuid2) +
                  nent * sizeof(struct kvm_cpuid_entry2));
  cpuid2->nent = nent;
  if (ioctl(kvmfd, KVM_GET_SUPPORTED_CPUID, cpuid2) < 0)
    err(1, "getting supported CPU ID");

  vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, (unsigned long)0);
  if (vcpufd == -1)
    err(1, "KVM_CREATE_VCPU");

  if (ioctl(vcpufd, KVM_SET_CPUID2, cpuid2) < 0)
    err(1, "cannot set cpuid things\n");

  /* Map the shared kvm_run structure and following data. */
  ret = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
  if (ret == -1)
    err(1, "KVM_GET_VCPU_MMAP_SIZE");
  mmap_size = ret;
  if (mmap_size < sizeof(*vcpu_run))
    errx(1, "KVM_GET_VCPU_MMAP_SIZE unexpectedly small");
  vcpu_run =
      mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);
  if (!vcpu_run)
    err(1, "mmap vcpu");

  /* Initialize CS to point at 0, via a read-modify-write of sregs.
   * CS is the Code Segment, back from when segmentation was a thing:
   * https://wiki.osdev.org/Segmentation.
   */
  ret = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
  if (ret == -1)
    err(1, "KVM_GET_SREGS");
  sregs.cs.base = 0;
  sregs.cs.selector = 0;
  ret = ioctl(vcpufd, KVM_SET_SREGS, &sregs);
  if (ret == -1)
    err(1, "KVM_SET_SREGS");

  return ret;
}

int launch_vm(uint64_t boot_gpa, uint64_t app_gpa, uint64_t app_stack_gpa) {
  int ret;

  /* Initialize registers: instruction pointer for our code, initial flags
   * required by x86 architecture, and bootstrapping information. */
  struct kvm_regs regs = {
      .rip = boot_gpa,      // initial value of the instruction pointer
      .rflags = 0x2,        // magically required by Intel's x86 architecture
      .r15 = app_gpa,       // address of guest application entrypoint
      .r14 = app_stack_gpa, // address of guest application stack
  };
  ret = ioctl(vcpufd, KVM_SET_REGS, &regs);
  if (ret == -1)
    err(1, "KVM_SET_REGS");

  /* Repeatedly run code and handle VM exits. */
  while (1) {
    if (ioctl(vcpufd, KVM_RUN, NULL) == -1)
      err(1, "KVM_RUN");
    if (vmexit_handler(vcpu_run->exit_reason) == 0)
      break;
  }
  return 0;
}
