CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99
LDFLAGS = 

all: generate_narration text_to_speech

generate_narration: generate_narration.c
	$(CC) $(CFLAGS) -o generate_narration generate_narration.c $(LDFLAGS)

text_to_speech: text_to_speech.c
	$(CC) $(CFLAGS) -o text_to_speech text_to_speech.c $(LDFLAGS)

clean:
	rm -f generate_narration text_to_speech temp_narration.wav

run: all
	./generate_narration && ./text_to_speech

.PHONY: all clean run
