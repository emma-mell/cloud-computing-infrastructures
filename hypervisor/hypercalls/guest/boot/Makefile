BIN := boot

SRCS := $(wildcard *.asm)
OBJS := $(SRCS:.asm=.o)

LD_SCRIPT := boot.ld

AS := nasm
ASFLAGS := -f elf64
LDFLAGS := -m elf_x86_64

.PHONY: clean

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) -T $(LD_SCRIPT) $^ -o $@ 

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm --force $(OBJS) $(BIN)
