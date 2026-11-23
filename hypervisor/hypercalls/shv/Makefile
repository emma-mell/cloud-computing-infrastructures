BIN := shv

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

.PHONY: clean

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm --force $(OBJS) $(BIN)
