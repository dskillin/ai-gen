/**
 * Narration Generator
 * 
 * This program reads a prompt from a file and uses the mixtral model
 * via ollama to generate a cinematic narration. The narration is then
 * saved to a text file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PROMPT_SIZE 4096
#define MAX_CMD_SIZE 5120
#define PROMPT_FILE "narration_prompt.txt"
#define OUTPUT_FILE "narration.txt"

/**
 * Read the contents of a file into a buffer
 */
char* read_file(const char* filename, size_t* size) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(*size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, *size, file);
    buffer[read_size] = '\0';
    
    fclose(file);
    return buffer;
}

/**
 * Call ollama to generate narration using mixtral model
 */
int generate_narration(const char* prompt, const char* output_file) {
    char cmd[MAX_CMD_SIZE];
    
    // Escape double quotes in the prompt
    char* escaped_prompt = (char*)malloc(strlen(prompt) * 2 + 1);
    if (!escaped_prompt) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }
    
    char* dst = escaped_prompt;
    for (const char* src = prompt; *src; src++) {
        if (*src == '"') {
            *dst++ = '\\';
        }
        *dst++ = *src;
    }
    *dst = '\0';
    
    // Construct the command to call ollama
    snprintf(cmd, MAX_CMD_SIZE,
             "ollama run mixtral \"%s\" > %s",
             escaped_prompt, output_file);
    
    printf("Generating narration with mixtral...\n");
    
    // Execute the command
    int status = system(cmd);
    
    free(escaped_prompt);
    
    if (status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Error: Failed to execute ollama command\n");
        return 1;
    }
    
    printf("Narration generated and saved to %s\n", output_file);
    return 0;
}

int main() {
    size_t prompt_size;
    char* prompt = read_file(PROMPT_FILE, &prompt_size);
    
    if (!prompt) {
        return 1;
    }
    
    if (prompt_size > MAX_PROMPT_SIZE) {
        fprintf(stderr, "Error: Prompt is too large (max size: %d)\n", MAX_PROMPT_SIZE);
        free(prompt);
        return 1;
    }
    
    int result = generate_narration(prompt, OUTPUT_FILE);
    
    free(prompt);
    return result;
}
