/**
 * Text to Speech Converter
 *
 * This program converts a text file to an MP3 audio file using
 * espeak for text-to-speech and ffmpeg for audio conversion.
 * The voice is configured to be female, thoughtful, and kind.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_CMD_SIZE 1024
#define INPUT_FILE "narration.txt"
#define OUTPUT_FILE "narration.mp3"
#define TEMP_WAV "temp_narration.wav"

/**
 * Check if a command is available
 */
int command_exists(const char* cmd) {
    char which_cmd[MAX_CMD_SIZE];
    snprintf(which_cmd, MAX_CMD_SIZE, "which %s > /dev/null 2>&1", cmd);
    return system(which_cmd) == 0;
}

/**
 * Convert text to speech using espeak
 */
int text_to_speech(const char* input_file, const char* temp_wav) {
    char cmd[MAX_CMD_SIZE];

    // Use espeak with a female voice, slower speed for thoughtfulness
    // -v en+f3: English female voice variant 3
    // -s 130: Slower speed (normal is 175)
    // -p 50: Pitch (higher for female voice)
    // -a 100: Amplitude
    // -g 10: Word gap (ms) for more thoughtful pacing
    snprintf(cmd, MAX_CMD_SIZE,
             "espeak -v en+f3 -s 130 -p 50 -a 100 -g 10 -f %s -w %s",
             input_file, temp_wav);

    printf("Converting text to speech...\n");

    int status = system(cmd);
    if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Error: Failed to execute espeak command\n");
        return 1;
    }

    return 0;
}

/**
 * Convert WAV to MP3 using ffmpeg
 */
int convert_to_mp3(const char* temp_wav, const char* output_file) {
    char cmd[MAX_CMD_SIZE];

    // Use ffmpeg to convert WAV to MP3 with good quality
    snprintf(cmd, MAX_CMD_SIZE,
             "ffmpeg -i %s -codec:a libmp3lame -qscale:a 2 %s",
             temp_wav, output_file);

    printf("Converting WAV to MP3...\n");

    int status = system(cmd);
    if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Error: Failed to execute ffmpeg command\n");
        return 1;
    }

    // Clean up temporary WAV file
    if (unlink(temp_wav) != 0) {
        fprintf(stderr, "Warning: Failed to remove temporary WAV file\n");
    }

    return 0;
}

int main() {
    // Check for required dependencies
    if (!command_exists("espeak")) {
        fprintf(stderr, "Error: espeak is not installed. Please install it with:\n");
        fprintf(stderr, "  sudo apt-get install espeak\n");
        return 1;
    }

    if (!command_exists("ffmpeg")) {
        fprintf(stderr, "Error: ffmpeg is not installed. Please install it with:\n");
        fprintf(stderr, "  sudo apt-get install ffmpeg\n");
        return 1;
    }

    // Check if input file exists
    if (access(INPUT_FILE, F_OK) != 0) {
        fprintf(stderr, "Error: Input file %s does not exist\n", INPUT_FILE);
        return 1;
    }

    // Convert text to speech
    if (text_to_speech(INPUT_FILE, TEMP_WAV) != 0) {
        return 1;
    }

    // Convert WAV to MP3
    if (convert_to_mp3(TEMP_WAV, OUTPUT_FILE) != 0) {
        return 1;
    }

    printf("Narration successfully converted to MP3: %s\n", OUTPUT_FILE);
    return 0;
}
