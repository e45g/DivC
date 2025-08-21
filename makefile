CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -g
SRCS := $(wildcard src/*.c)
TARGET := divc

all: $(TARGET)

$(TARGET): $(SRCS)
	@$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(TARGET)
