CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -g
SRCS := $(wildcard src/*.c)
TARGET := divc

all: $(TARGET)

$(TARGET): $(SRCS)
	@$(CC) $(CFLAGS) -o $@ $^

test: $(TARGET)
	./$(TARGET) test/test.dc

clean:
	rm $(TARGET)
