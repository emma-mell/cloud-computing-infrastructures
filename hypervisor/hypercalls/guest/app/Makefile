BIN := app

LD_SCRIPT := app.ld

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

MUSL_GIT_URL := https://gitlabev.imtbs-tsp.eu/csc5004/musl.git
MUSL_BRANCH := simple-hv-1.2.6
MUSL_DIR := $(shell pwd)/musl
CC := $(MUSL_DIR)/musl_installation/bin/musl-gcc
CFLAGS := -fno-stack-protector -mno-default -mfpmath=387 -O0

.PHONY: all clean deepclean

$(BIN): $(OBJS) $(LD_SCRIPT) $(CC)
	$(CC) $(CFLAGS) -T $(LD_SCRIPT) -static -o $@ $(OBJS)

%.o: %.c $(CC)
	$(CC) $(CFLAGS) -static -c $<

$(CC):
	git clone $(MUSL_GIT_URL) --branch $(MUSL_BRANCH) $(MUSL_DIR)
	cd musl &&\
		./configure CFLAGS="$(CFLAGS)" --disable-shared --prefix=$(MUSL_DIR)/musl_installation 
	make -C $(MUSL_DIR) -j install

clean:
	rm --force $(OBJS) $(BIN)

deepclean: clean
	rm --force --recursive $(MUSL_DIR)
