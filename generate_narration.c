/**
 * generate_narration.c
 * 
 * This program uses ollama with the mixtral model to generate a narration
 * based on a prompt, and then converts the narration to speech using espeak.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROMPT_FILE ".augie/narration_prompt.txt"
#define NARRATION_FILE ".augie/narration.txt"
#define AUDIO_FILE ".augie/narration.mp3"
#define MAX_PROMPT_SIZE 8192
#define MAX_RESPONSE_SIZE 4096

/**
 * Read the contents of a file into a buffer
 */
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size > MAX_PROMPT_SIZE) {
        fprintf(stderr, "File too large: %s\n", filename);
        fclose(file);
        return NULL;
    }
    
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, file_size, file);
    buffer[read_size] = '\0';
    
    fclose(file);
    return buffer;
}

/**
 * Write a string to a file
 */
int write_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error opening file for writing: %s\n", filename);
        return 0;
    }
    
    fputs(content, file);
    fclose(file);
    return 1;
}

/**
 * Generate narration using ollama with mixtral model
 */
char* generate_narration(const char* prompt) {
    char command[MAX_PROMPT_SIZE + 100];
    char* response = (char*)malloc(MAX_RESPONSE_SIZE);
    if (!response) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    // Create a temporary file for the prompt
    char temp_prompt_file[] = "/tmp/prompt_XXXXXX";
    int fd = mkstemp(temp_prompt_file);
    if (fd == -1) {
        fprintf(stderr, "Failed to create temporary file\n");
        free(response);
        return NULL;
    }
    
    // Write prompt to temporary file
    FILE* temp_file = fdopen(fd, "w");
    fputs(prompt, temp_file);
    fclose(temp_file);
    
    // Create command to run ollama
    snprintf(command, sizeof(command), 
             "ollama run mixtral \"$(cat %s)\" --nowordwrap", 
             temp_prompt_file);
    
    // Execute command and capture output
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr, "Failed to run ollama command\n");
        free(response);
        unlink(temp_prompt_file);
        return NULL;
    }
    
    size_t total_read = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe) && total_read < MAX_RESPONSE_SIZE - 1) {
        size_t len = strlen(buffer);
        if (total_read + len >= MAX_RESPONSE_SIZE - 1) {
            len = MAX_RESPONSE_SIZE - 1 - total_read;
        }
        memcpy(response + total_read, buffer, len);
        total_read += len;
    }
    
    response[total_read] = '\0';
    pclose(pipe);
    unlink(temp_prompt_file);
    
    return response;
}

/**
 * Convert text to speech using espeak
 */
int text_to_speech(const char* text, const char* output_file) {
    char command[MAX_RESPONSE_SIZE + 100];
    
    // Create a temporary file for the text
    char temp_text_file[] = "/tmp/speech_XXXXXX";
    int fd = mkstemp(temp_text_file);
    if (fd == -1) {
        fprintf(stderr, "Failed to create temporary file\n");
        return 0;
    }
    
    // Write text to temporary file
    FILE* temp_file = fdopen(fd, "w");
    fputs(text, temp_file);
    fclose(temp_file);
    
    // Create command to run espeak
    snprintf(command, sizeof(command), 
             "espeak -v en+f3 -s 130 -p 50 -f %s -w %s", 
             temp_text_file, output_file);
    
    // Execute command
    int result = system(command);
    unlink(temp_text_file);
    
    return result == 0;
}

int main() {
    printf("Reading prompt from %s...\n", PROMPT_FILE);
    char* prompt = read_file(PROMPT_FILE);
    if (!prompt) {
        return 1;
    }
    
    printf("Generating narration using mixtral...\n");
    char* narration = generate_narration(prompt);
    free(prompt);
    
    if (!narration) {
        return 1;
    }
    
    printf("Writing narration to %s...\n", NARRATION_FILE);
    if (!write_file(NARRATION_FILE, narration)) {
        free(narration);
        return 1;
    }
    
    printf("Converting narration to speech...\n");
    if (!text_to_speech(narration, AUDIO_FILE)) {
        fprintf(stderr, "Failed to convert text to speech\n");
        free(narration);
        return 1;
    }
    
    printf("Narration generated successfully!\n");
    printf("Text: %s\n", NARRATION_FILE);
    printf("Audio: %s\n", AUDIO_FILE);
    
    free(narration);
    return 0;
}
