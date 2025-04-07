CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
TARGET = generate_narration

all: $(TARGET)

$(TARGET): generate_narration.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
