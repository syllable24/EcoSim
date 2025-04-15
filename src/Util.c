#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/Util.h"
#include "../include/cJSON.h"

#define RESOURCE_FILE_COUNT 1
#define BUFFER_SIZE 1024

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
	printf("Start read_definition_from_res_file.\n");
	int exit_status = 1;

	char* raw_file_content = NULL;
	size_t raw_file_content_size = 0;
	
	cJSON* need_config = NULL;
	int need_array_size = 0;

	*target = NULL;
	*target_size = 0;	

	const char* filename = get_resource_file_name(file);

	if (!filename || !*filename){
		printf("Error received invalid filename (NULL or empty).\n");		
		goto cleanup;
	}

	if (!target || !target_size){
		printf("Error received invalid target or target_size (NULL).\n");
		goto cleanup;
	}	
	
	if (read_file_content(filename, &raw_file_content, &raw_file_content_size) != 0){
		printf("Error during raw file content read.\n");		
		goto cleanup;
	}

	printf("Raw file content: \n%s\n", raw_file_content);

	// Parse JSON content		
	need_config = cJSON_Parse(raw_file_content);
	if (!need_config){
		const char* error_ptr = cJSON_GetErrorPtr();
		printf("Error failed to parse JSON in '%s': %s\n", filename, error_ptr ? error_ptr : "Unknown error");
		goto cleanup;
	}
	
	const cJSON* all_base_needs = cJSON_GetObjectItemCaseSensitive(need_config, "BASE_NEEDS");
	if(!all_base_needs || !cJSON_IsArray(all_base_needs)){
		fprintf(stderr, "Error: Invalid need configuration structure.");		
		goto cleanup;
	}
	
	need_array_size = cJSON_GetArraySize(all_base_needs);
	if (need_array_size <= 0){
		fprintf(stderr, "Error: Base needs array is empty.\n");
		goto cleanup;
	}

	if (need_array_size > UINT8_MAX){
		fprintf(stderr, "Error: Base needs array larger than UINT8_MAX.\n");
		goto cleanup;
	}

	// Setup target array
	*target = (char**) malloc(need_array_size * sizeof(char*));
	if (!*target){		
		fprintf(stderr, "Malloc error.");
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
			fprintf(stderr, "Invalid item in need_array.");
			goto cleanup;
		}
		
		(*target)[i] = strdup(string_value);
		if (!(*target)[i]) {
			fprintf(stderr, "Failed to duplicate string into target array.\n");
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
	printf("End read_definition_from_res_file()\n");
    return exit_status;
}

int read_file_content(const char* filename, char** target_buffer, size_t* target_size){
	printf("Start read_file_content()\n");
	
	int exit_status = 1;
	FILE* resFile = NULL;
	*target_buffer = NULL;

	if(!filename || !*filename){
		printf("Error received invalid filename.\n");		
		goto cleanup;
	}

	if(!target_buffer || !target_size){
		printf("Error received invalid target_buffer or target_size.\n");		
		goto cleanup;	
	}

	printf("Try opening file %s.\n", filename);
	resFile = fopen(filename, "rb"); // Open in read binary mode for fread()
	if (resFile == NULL){
		printf("Error opening file.\n");		
		goto cleanup;
	}
	
	printf("Reading from file %s.\n", filename);

	// Read file size 
	if (fseek(resFile, 0, SEEK_END) != 0){
		printf("Error could not seek end of file: %s", filename);				
		goto cleanup;
	}

	// Get filesize value
	long size = ftell(resFile);
	if (size < 0){
		printf("Error could not determine resource file size.\n");		
		goto cleanup;
	}

	// Rewind File
	if (fseek(resFile, 0, SEEK_SET) != 0){
		printf("Error could rewind file: %s", filename);
		goto cleanup;
	}
	
	// Allocate file content buffer.
	*target_buffer = (char*) malloc(size + 1);
	if (!*target_buffer){
		printf("Malloc error.\n");		
		goto cleanup;
	}

	// Read file content into target_buffer
	size_t bytes_read = fread(*target_buffer, 1, size, resFile);
	if (bytes_read != (size_t)size){
		printf("Could not read entire resource file.\n");		
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
	printf("End read_file_content()\n");
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