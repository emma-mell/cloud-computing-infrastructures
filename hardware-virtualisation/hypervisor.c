#include <stdio.h>
#include <linux/kvm.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

/* Guest code.
*
* - add registers al and bl (initialized by the SHV code) into al
* - convert the result digit into ASCII by adding '0'
* - output it to serial port 0x3f8, followed by a new line
* - halt
*/
static const uint8_t guest_code[] = {
  0xba, 0xf8, 0x03, /* mov $0x3f8, %dx */
  0x00, 0xd8, /* add %bl, %al */
  0x04, '0', /* add $'0', %al */
  0xee, /* out %al, (%dx) */
  0xb0, '\n', /* mov $'\n', %al */
  0xee, /* out %al, (%dx) */
  0xf4, /* hlt */
};

int main () {
  int kvm_fd;
  if ((kvm_fd = open("/dev/kvm", O_RDWR)) == -1) {
    err(EXIT_FAILURE, "failed opening file /dev/kvm");
  } 

  if (ioctl(kvm_fd, KVM_GET_API_VERSION, NULL) != 12) {
    err(EXIT_FAILURE, "failed checking kvm version");
  }

  int vm_fd;
  if ((vm_fd = ioctl(kvm_fd, KVM_CREATE_VM, 0, kvm_fd)) == -1) {
    err(EXIT_FAILURE, "failed creating the vm");
  }

  void* guest_mem;
  
  if ((guest_mem = mmap(NULL, 0x1000, PROT_WRITE|PROT_READ, MAP_SHARED, vm_fd, 0)) == NULL) {
    err(EXIT_FAILURE, "failed making the maping with mmap");
  }

  memcpy(guest_mem, guest_code, sizeof(guest_code));

  /* slot is just an ID for memory regions managed by KVM.
   * guest_phys_addr is the address where the memory is mapped in the
   * guest's memory space.
   * guest_mem is the address in the hypervisor's memory space, of the
   * allocated memory.
   */
  struct kvm_userspace_memory_region region = {
    .slot = 0,
    .guest_phys_addr = 0x1000,
    .memory_size = 0x1000,
    .userspace_addr = (uint64_t)guest_mem,
  };

  if (ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, region) == -1) {
    err(EXIT_FAILURE, "failed giving memory region to the guest");
  }

  int vcpu_fd;
  if ((vcpu_fd != ioctl(vm_fd, KVM_CREATE_VCPU, 0)) == -1) {
    err(EXIT_FAILURE, "failed creating a virtual cpu");
  }

  uint8_t size;
  if ((size = ioctl(kvm_fd, KVM_GET_VCPU_MMAP_SIZE, NULL)) >= sizeof(struct kvm_run)) {
    err(EXIT_FAILURE, "failed checking that there is enough memory space to map a virtual cpu");
  }

  void* addr;
  if ((addr = mmap(NULL, size, PROT_WRITE|PROT_READ, MAP_SHARED, vcpu_fd, 0)) == NULL) {
    err(EXIT_FAILURE, "failed mapping vcpu in memory");
  }

  struct kvm_sregs srgs;
  if (ioctl(vcpu_fd, KVM_GET_SREGS, &srgs) == -1) {
    err(EXIT_FAILURE, "failed getting special register");
  }

  srgs.cs.base = 0;
  srgs.cs.selector = 0;

  if (ioctl(vcpu_fd, KVM_SET_SREGS, &srgs) == -1) {
    err(EXIT_FAILURE, "failed setting special register");
  }

  struct kvm_regs rgs = {0};
  rgs.rip = 0x1000;
  rgs.rax = 0x01;
  rgs.rbx = 0x10;
  rgs.rflags = 0x2;

  if (ioctl(vcpu_fd, KVM_SET_REGS, &rgs) == -1) {
    err(EXIT_FAILURE, "failed setting register");
  }
  

  return EXIT_SUCCESS;
}
