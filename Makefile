CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c99 -g

SRCS    = assembler.c    \
          opcodes.c      \
          utils.c        \
          symbol_table.c \
          pre_processor.c\
          first_pass.c   \
          second_pass.c

OBJS    = $(SRCS:.c=.o)
TARGET  = assembler

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c defs.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) *.am *.ob *.ent *.ext
