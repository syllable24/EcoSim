#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>

#include "../include/Util.h"
#include "../include/cJSON.h"

// Custom log categories
#define LOG_CAT_UTIL SDL_LOG_CATEGORY_CUSTOM
#define LOG_CAT_DISPLAY SDL_LOG_CATEGORY_CUSTOM + 1
#define LOG_CAT_POPULATION SDL_LOG_CATEGORY_CUSTOM + 2
#define LOG_CAT_MAIN SDL_LOG_CATEGORY_CUSTOM + 3

// Resource File paths
#define RESOURCE_FILE_COUNT 1
static const char* resource_file_names[RESOURCE_FILE_COUNT] = {
	"./res/NeedDefinition.json"
};

const char* get_resource_file_name(ResourceFiles file);

void trim(char* line);

int read_file_content(const char* filename, char** target_buffer, size_t* target_size);

uint32_t determine_rand_val(int min, int max){
	return rand() % (max - min + 1) + min;	
}

uint8_t determine_rand_percent(){
	return rand() % (100 + 1);	
}

int read_definition_from_res_file(ResourceFiles file, char*** target, uint8_t* target_size){
	SDL_LogTrace(LOG_CAT_UTIL, "Start read_definition_from_res_file.");
	int exit_status = 1;

	char* raw_file_content = NULL;
	size_t raw_file_content_size = 0;
	
	cJSON* need_config = NULL;
	int need_array_size = 0;

	*target = NULL;
	*target_size = 0;	

	const char* filename = get_resource_file_name(file);

	if (!filename || !*filename){
		SDL_LogError(LOG_CAT_UTIL, "Error received invalid filename (NULL or empty).");
		goto cleanup;
	}

	if (!target || !target_size){
		SDL_LogError(LOG_CAT_UTIL, "Error received invalid target or target_size (NULL).");		
		goto cleanup;
	}	
	
	if (read_file_content(filename, &raw_file_content, &raw_file_content_size) != 0){
		SDL_LogError(LOG_CAT_UTIL, "Error while reading raw file content.");
		goto cleanup;
	}

	SDL_LogDebug(LOG_CAT_UTIL, "Raw %s content: \n%s\n", filename, raw_file_content);

	// Parse JSON content		
	need_config = cJSON_Parse(raw_file_content);
	if (!need_config){
		const char* error_ptr = cJSON_GetErrorPtr();
		SDL_LogError(LOG_CAT_UTIL, "Failed to parse JSON in '%s': %s\n", filename, error_ptr ? error_ptr : "Unknown error");		
		goto cleanup;
	}
	
	const cJSON* all_base_needs = cJSON_GetObjectItemCaseSensitive(need_config, "BASE_NEEDS");
	if(!all_base_needs || !cJSON_IsArray(all_base_needs)){
		SDL_LogError(LOG_CAT_UTIL, "Invalid need configuration content structure: Could not find element 'BASE_NEEDS'.");
		goto cleanup;
	}
	
	need_array_size = cJSON_GetArraySize(all_base_needs);
	if (need_array_size <= 0){		
		SDL_LogError(LOG_CAT_UTIL, "BASE_NEEDS array is empty.");
		goto cleanup;
	}

	if (need_array_size > UINT8_MAX){
		SDL_LogError(LOG_CAT_UTIL, "BASE_NEEDS array larger than UINT8_MAX.");
		goto cleanup;
	}

	// Setup target array
	*target = (char**) malloc(need_array_size * sizeof(char*));
	if (!*target){
		SDL_LogError(LOG_CAT_UTIL, "Memory allocation failed for target array (%d entries, %zu bytes).", need_array_size, need_array_size * sizeof(char*));
		goto cleanup;
	}
	
	// Initialize pointers
	for (int i = 0; i < need_array_size; i++){
		(*target)[i] = NULL;
	}

	for(int i = 0; i < need_array_size; i++){
		cJSON* array_item = cJSON_GetArrayItem(all_base_needs, i);
		char* string_value = cJSON_GetStringValue(array_item);
		if (!string_value || !cJSON_IsString(array_item)){
			SDL_LogError(LOG_CAT_UTIL, "Invalid value in BASE_NEEDS array.");
			goto cleanup;
		}
		
		(*target)[i] = strdup(string_value);
		if (!(*target)[i]) {
			SDL_LogError(LOG_CAT_UTIL, "Failed to duplicate string into target array.");			
			goto cleanup;
		}
	}

	*target_size = (uint8_t) need_array_size;
	exit_status = 0;

cleanup:
    if (exit_status != 0 && *target) {
        for (int i = 0; i < need_array_size; i++) {
            free((*target)[i]);
        }
        free(*target);
        *target = NULL;
    }
    free(raw_file_content);
    cJSON_Delete(need_config);
	SDL_LogTrace(LOG_CAT_UTIL, "End read_definition_from_res_file()");
    return exit_status;
}

int read_file_content(const char* filename, char** target_buffer, size_t* target_size){
	SDL_LogTrace(LOG_CAT_UTIL, "Start read_file_content()");	
	
	int exit_status = 1;
	FILE* resFile = NULL;
	*target_buffer = NULL;

	if(!filename || !*filename){
		SDL_LogError(LOG_CAT_UTIL, "Received invalid filename (NULL or empty).");		
		goto cleanup;
	}

	if(!target_buffer || !target_size){		
		SDL_LogError(LOG_CAT_UTIL, "Received invalid target_buffer or target_size.\n");
		goto cleanup;
	}

	SDL_LogDebug(LOG_CAT_UTIL, "Try opening file %s.\n", filename);
	resFile = fopen(filename, "rb"); // Open in read binary mode for fread()
	if (resFile == NULL){
		SDL_LogError(LOG_CAT_UTIL, "Error opening file %s.", filename);
		goto cleanup;
	}
	
	SDL_LogDebug(LOG_CAT_UTIL, "Reading from file %s.\n", filename);

	// Read file size 
	if (fseek(resFile, 0, SEEK_END) != 0){
		SDL_LogError(LOG_CAT_UTIL, "Could not seek end of file: %s", filename);				
		goto cleanup;
	}

	// Get filesize value
	long size = ftell(resFile);
	if (size < 0){
		SDL_LogError(LOG_CAT_UTIL, "Could not determine file size.\n");		
		goto cleanup;
	}

	// Rewind File
	if (fseek(resFile, 0, SEEK_SET) != 0){
		SDL_LogError(LOG_CAT_UTIL, "Error could rewind file: %s", filename);
		goto cleanup;
	}
	
	// Allocate file content buffer.
	*target_buffer = (char*) malloc(size + 1);
	if (!*target_buffer){
		SDL_LogError(LOG_CAT_UTIL, "Memory allocation failed for target_buffer (%d entries, %zu bytes).", size, size * sizeof(char*));
		goto cleanup;
	}

	// Read file content into target_buffer
	size_t bytes_read = fread(*target_buffer, 1, size, resFile);
	if (bytes_read != (size_t)size){
		SDL_LogError(LOG_CAT_UTIL, "Could not read entire resource file. Read %d bytes, expected %d bytes.", bytes_read, (size_t)size);
		goto cleanup;
	}

	(*target_buffer)[size] = '\0';
	*target_size = size;	
	
	exit_status = 0;

cleanup:
	if (exit_status != 0){
		free(*target_buffer);
		*target_buffer = NULL;
	}
	fclose(resFile);
	SDL_LogTrace(LOG_CAT_UTIL, "End read_file_content()");
	return exit_status;
}

const char* get_resource_file_name(ResourceFiles file){	
	if (file >= 0 && file < RESOURCE_FILE_COUNT){
		return resource_file_names[file];
	}
	return "";
}

void trim(char* line){
	// Remove trailing newline
	line[strcspn(line, "\n")] = '\0';
	
	// Trim leading and trailing whitespaces
	char* start = line;
    while (*start == ' ' || *start == '\t') start++;
    char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t')) end--;
    *(end + 1) = '\0';
    memmove(line, start, end - start + 2);
}