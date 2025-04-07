/**
 * generate_narration.c
 * 
 * This program reads a prompt from a file and uses the Mixtral model via Ollama
 * to generate a cinematic narration based on the prompt. The narration is then
 * saved to a file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define PROMPT_FILE "prompt.txt"
#define OUTPUT_FILE "narration.txt"
#define OLLAMA_API "http://localhost:11434/api/generate"
#define MAX_PROMPT_SIZE 10000
#define MAX_RESPONSE_SIZE 20000

/* Structure to hold response data */
struct ResponseData {
    char *data;
    size_t size;
};

/* Callback function for curl to handle response data */
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    struct ResponseData *resp = (struct ResponseData *)userp;
    
    char *ptr = realloc(resp->data, resp->size + real_size + 1);
    if (!ptr) {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return 0;
    }
    
    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, real_size);
    resp->size += real_size;
    resp->data[resp->size] = 0;
    
    return real_size;
}

/* Function to read prompt from file */
char *read_prompt_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    char *buffer = (char *)malloc(MAX_PROMPT_SIZE);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for prompt\n");
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(buffer, 1, MAX_PROMPT_SIZE - 1, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return buffer;
}

/* Function to extract response text from JSON */
char *extract_response_text(const char *json_response) {
    char *response_text = (char *)malloc(MAX_RESPONSE_SIZE);
    if (!response_text) {
        fprintf(stderr, "Error: Failed to allocate memory for response text\n");
        return NULL;
    }
    
    response_text[0] = '\0';
    
    /* Process each line of the response */
    char *line = strdup(json_response);
    char *line_ptr = line;
    char *next_line;
    
    while ((next_line = strchr(line_ptr, '\n'))) {
        *next_line = '\0';
        
        /* Look for "response": in the JSON line */
        char *response_field = strstr(line_ptr, "\"response\":");
        if (response_field) {
            response_field += 11; /* Skip past "response": */
            
            /* Find the start and end of the response text */
            char *start = strchr(response_field, '\"');
            if (start) {
                start++; /* Skip the opening quote */
                char *end = strrchr(start, '\"');
                if (end) {
                    *end = '\0';
                    strcat(response_text, start);
                }
            }
        }
        
        line_ptr = next_line + 1;
    }
    
    free(line);
    return response_text;
}

/* Function to save text to file */
int save_to_file(const char *filename, const char *text) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", filename);
        return 0;
    }
    
    fprintf(file, "%s", text);
    fclose(file);
    return 1;
}

int main() {
    CURL *curl;
    CURLcode res;
    struct ResponseData response_data;
    
    /* Initialize response data */
    response_data.data = malloc(1);
    response_data.size = 0;
    
    /* Read prompt from file */
    char *prompt = read_prompt_file(PROMPT_FILE);
    if (!prompt) {
        return 1;
    }
    
    /* Initialize curl */
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (curl) {
        /* Prepare JSON payload */
        char *json_payload = (char *)malloc(MAX_PROMPT_SIZE + 100);
        if (!json_payload) {
            fprintf(stderr, "Error: Failed to allocate memory for JSON payload\n");
            free(prompt);
            curl_global_cleanup();
            return 1;
        }
        
        sprintf(json_payload, "{\"model\":\"mixtral\",\"prompt\":\"%s\",\"stream\":false}", prompt);
        
        /* Set curl options */
        curl_easy_setopt(curl, CURLOPT_URL, OLLAMA_API);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);
        
        /* Set headers */
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        /* Perform the request */
        printf("Sending request to Mixtral model...\n");
        res = curl_easy_perform(curl);
        
        /* Check for errors */
        if (res != CURLE_OK) {
            fprintf(stderr, "Error: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Response received. Processing...\n");
            
            /* Extract and save the response text */
            char *narration = extract_response_text(response_data.data);
            if (narration && narration[0] != '\0') {
                if (save_to_file(OUTPUT_FILE, narration)) {
                    printf("Narration successfully saved to %s\n", OUTPUT_FILE);
                }
                free(narration);
            } else {
                fprintf(stderr, "Error: Failed to extract narration from response\n");
            }
        }
        
        /* Clean up */
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(json_payload);
    }
    
    /* Global cleanup */
    curl_global_cleanup();
    free(prompt);
    free(response_data.data);
    
    return 0;
}
