#include "loader.h"
#include "memory.h"
#include "memory_map.h"
#include "vmm.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 3)
    errx(2, "expected 2 arguments: bootloader_filepath guest_app_filepath");

  if (create_vm())
    errx(EXIT_FAILURE, "creating the VM");

  FILE *f = get_file(argv[1]);
  if (!f)
    err(EXIT_FAILURE, "loading bootloader");
  load_sections(f, guest_mem);

  f = get_file(argv[2]);
  if (!f)
    err(EXIT_FAILURE, "loading guest application");
  load_sections(f, guest_mem);

  // Map the user space stack.
  memory_map_range(guest_mem, USER_STACK_TOP_GPA, BOOT_GPA);
  // Map the boot section as loaded above.
  memory_map_range(guest_mem, BOOT_GPA, APP_GPA);
  // Map the guest app as loaded above.
  memory_map_range(guest_mem, APP_GPA, ALLOC_AREA_GPA);

  launch_vm(BOOT_GPA, APP_GPA, APP_STACK_GPA);

  return 0;
}
