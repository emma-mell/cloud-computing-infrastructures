#include "memory.h"

#include "memory_map.h"

#include <err.h>
#include <stdio.h>
#include <string.h>

uint64_t new_page = ALLOC_AREA_GPA;

/* Get new guest-accessible pages.
 *
 * They are already mapped in the EPT via the KVM userspace memory ragion, but
 * not mapped in the guest's userspace.
 */
uint64_t get_pages(int nb_page) {
  uint64_t ret = new_page;

  new_page += 0x1000 * nb_page;
  if (new_page >= MAX_PAGE)
    errx(1, "OOM");

  return ret;
}

/* Map pages in the guest's userspace.
 *
 * Also see memory_map_range.
 */
void memory_map(uint8_t *mem, uint64_t virt_addr, uint64_t phys_addr) {
  uint64_t *cr3, *pgd, *pud, *pmd, *pt, page;
  uint64_t *pgde_p, *pude_p, *pmde_p, *pte_p;

  cr3 = (uint64_t *)&mem[CR3];
  pgde_p = &cr3[pgd_index(virt_addr)];
  if (!present(*pgde_p)) {
    page = get_pages(1);
    memset(&mem[page], 0, 0x1000);
    *pgde_p = page | PRESENT | USER | RW;
  }

  pud = (uint64_t *)&mem[ptep2pfn(pgde_p)];
  pude_p = &pud[pud_index(virt_addr)];
  if (!present(*pude_p)) {
    page = get_pages(1);
    memset(&mem[page], 0, 0x1000);
    *pude_p = page | PRESENT | USER | RW;
  }

  pmd = (uint64_t *)&mem[ptep2pfn(pude_p)];
  pmde_p = &pmd[pmd_index(virt_addr)];
  if (!present(*pmde_p)) {
    page = get_pages(1);
    memset(&mem[page], 0, 0x1000);
    *pmde_p = page | PRESENT | USER | RW;
  }

  pt = (uint64_t *)&mem[ptep2pfn(pmde_p)];
  pte_p = &pt[pte_index(virt_addr)];
  if (!present(*pte_p)) {
    page = phys_addr;
    *pte_p = page | PRESENT | USER | RW;
  }
}
