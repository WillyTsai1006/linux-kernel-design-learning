CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11 -g

TARGET := build/process-identity
SOURCE := src/process_identity.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	mkdir -p build
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

clean:
	rm -rf build

