#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

// Initial size of the guest's memory.
#define GUEST_INIT_MEM_SZ 0xF000000

// GPA of guest page table (first valid page after IVT).
#define CR3 0x1000

// First page allocated to the user space stack (i.e., top-most stack page).
#define USER_STACK_TOP_GPA 0x2000

// These GPAs must match section descriptions in corresponding linker scripts.
#define BOOT_GPA 0x8000
#define APP_GPA 0x9000

// First available page for get_pages.
#define ALLOC_AREA_GPA 0xb000

// First guest invalid page above its memory mapping.
#define MEMORY_TOP_GPA GUEST_INIT_MEM_SZ

/* Bottom (high address) of the user stack of the guest app.
 * Technically it can start at 0x8000 (then stack grows downwards on x86_64),
 * but the guest app thinks it is called by the Linux kernel and expects its
 * arguments (argc and argv), on the stack. In other words, it expects argc and
 * argv before its (base) stack pointer, which are both 8B, thus the stack must
 * start at least 16B below BOOT_GPA.
 * Here we are simply skipping a page.
 */
#define APP_STACK_GPA (BOOT_GPA - 0x1000)

#endif
