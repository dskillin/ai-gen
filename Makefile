CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
LDFLAGS = -lcurl

all: generate_narration text_to_speech

generate_narration: generate_narration.c
	$(CC) $(CFLAGS) -o generate_narration generate_narration.c $(LDFLAGS)

text_to_speech: text_to_speech.c
	$(CC) $(CFLAGS) -o text_to_speech text_to_speech.c $(LDFLAGS)

clean:
	rm -f generate_narration text_to_speech

.PHONY: all clean
