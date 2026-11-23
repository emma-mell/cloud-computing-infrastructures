#include "loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_content(FILE *fd, void *dst, int offset, int size) {
  fseek(fd, offset, SEEK_SET);
  fread(dst, 1, size, fd);

  return 0;
}

int get_hdr(FILE *fd, Elf64_Ehdr *hdr) {
  fseek(fd, 0, SEEK_SET);
  fread(hdr, sizeof(Elf64_Ehdr), 1, fd);

  return 0;
}

int get_shdr(FILE *fd, Elf64_Shdr *shdr, int nb) {
  fseek(fd, SH_OFF, SEEK_SET);
  fread(shdr, sizeof(Elf64_Shdr) * nb, 1, fd);

  return 0;
}

int get_phdr(FILE *fd, Elf64_Phdr *phdr) {
  fseek(fd, PH_OFF, SEEK_SET);
  fread(phdr, sizeof(Elf64_Phdr), 1, fd);

  return 0;
}

FILE *get_file(char *pathname) {
  Elf64_Ehdr hdr;
  char check[] = {0x7f, 'E', 'L', 'F'};
  FILE *fd = fopen(pathname, "rw");
  FILE *ret = NULL;
  if (!fd)
    goto out;

  get_hdr(fd, &hdr);

  if (memcmp(&hdr.e_ident, check, 4))
    goto out;
  ret = fd;
out:
  return ret;
}

int load_text_section(FILE *fd, int offset, uint32_t *memory) {
  Elf64_Ehdr hdr;
  Elf64_Phdr phdr;
  uint32_t *text_section = (uint32_t *)malloc(phdr.p_filesz);

  get_hdr(fd, &hdr);
  get_phdr(fd, &phdr);

  memset(text_section, 0, phdr.p_filesz);
  get_content(fd, text_section, phdr.p_offset, phdr.p_filesz);

  uint8_t *tmp = (uint8_t *)text_section;

  memcpy(&memory[offset], text_section, phdr.p_filesz);
  return 0;
}

int load_sections(FILE *fd, uint8_t *memory) {
  Elf64_Ehdr hdr;
  Elf64_Shdr *shdrs;
  uint8_t *section;
  uint32_t size, addr, offset;
  get_hdr(fd, &hdr);
  shdrs = (Elf64_Shdr *)malloc(sizeof(Elf64_Shdr) * hdr.e_shnum);
  get_content(fd, shdrs, hdr.e_shoff, hdr.e_shnum * sizeof(Elf64_Shdr));
  for (int i = 0; i < hdr.e_shnum; i++) {
    /* Check if section occupies memory during execution */
    if (!(shdrs[i].sh_flags & 0x2) || shdrs[i].sh_addr == 0)
      continue;
    size = shdrs[i].sh_size;
    addr = shdrs[i].sh_addr;
    offset = shdrs[i].sh_offset;
    section = (uint8_t *)malloc(size);
    memset(section, 0, size);
    /* check if section has data (bss) */
    if (!(shdrs[i].sh_type & 0x8)) {
      get_content(fd, section, offset, size);
    }
    memcpy(&memory[addr], section, size);
    free(section);
  }

  return 0;
}

int load_segments(FILE *fd, uint8_t *memory) {
  Elf64_Ehdr hdr;
  Elf64_Phdr *phdrs;
  uint8_t *section;
  uint32_t size, addr, p_offset;
  get_hdr(fd, &hdr);

  phdrs = (Elf64_Phdr *)malloc(sizeof(Elf64_Phdr) * hdr.e_phnum);
  get_content(fd, phdrs, hdr.e_phoff, hdr.e_phnum * sizeof(Elf64_Phdr));

  for (int i = 0; i < hdr.e_phnum; i++) {
    if (phdrs[i].p_type != PT_LOAD)
      continue;
    size = phdrs[i].p_memsz;
    addr = phdrs[i].p_paddr;
    p_offset = phdrs[i].p_offset;

    section = (uint8_t *)malloc(size);
    memset(section, 0, size);
    get_content(fd, section, p_offset, size);
    memcpy(&memory[addr], section, size);
    free(section);
  }
  return 0;
}
