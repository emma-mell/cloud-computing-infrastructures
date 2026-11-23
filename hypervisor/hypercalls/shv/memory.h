#ifndef MEMORY_H
#define MEMORY_H

#include "memory_map.h"

#include <stdint.h>

// Macros to extract indices in Intel's 4-level page.
// Get PUD index in Page GLobal Directory (PGD).
#define pgd_index(addr) addr >> (12 + 3 * 9) & 0x1FF
// Get PMD index in Page Upper Directory (PUD).
#define pud_index(addr) addr >> (12 + 2 * 9) & 0x1FF
// Get PT index in Page Middle Directory (PMD).
#define pmd_index(addr) addr >> (12 + 1 * 9) & 0x1FF
// Get PTE index in Page Table.
#define pte_index(addr) addr >> (12 + 0 * 9) & 0x1FF

// Entry bit "page is present".
#define PRESENT 0x1
// Entry bit "RW access".
#define RW 0x2
// Entry bit "userspace accessible".
#define USER 0x4

// Tell whether the page pointed to by the PTE is present.
#define present(entry) ((entry) & PRESENT)
// Get page frame number from a pointer to a PTE in any page table level.
#define ptep2pfn(pointer) (*(pointer) & ~(0xFFF))

/* Maximum GPA allocated to the guest (exclusive).
 * This is actually the first invalid page after the maximum GPA.
 */
#define MAX_PAGE MEMORY_TOP_GPA

void memory_map(uint8_t *mem, uint64_t virt_addr, uint64_t phys_addr);
/* Map the memory range [from; to_exc[ in the guest's userspace (1-to-1
 * mapping). */
#define memory_map_range(mem, from, to_exc)                                    \
  for (uint64_t a = (from); a < (to_exc); a += 0x1000)                         \
  memory_map(mem, a, a)

uint64_t get_pages(int nb_page);

#endif
