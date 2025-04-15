#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <SDL3/SDL.h>

#include "../include/Util.h"
#include "../include/cJSON.h"

// Resource File paths
#define RESOURCE_FILE_COUNT 2
static const char* resource_file_names[RESOURCE_FILE_COUNT] = {
	"./res/NeedDefinition.json",
	"./res/TileDefinition.json",
};

const char* get_resource_file_name(ResourceFiles file);

void trim(char* line);

int read_file_content(const char* filename, char** target_buffer, size_t* target_size);

bool validate_json_non_empty_string(const cJSON* json, const char *key, char** target_value);

bool validate_json_percent_number(const cJSON* json, const char *key, uint8_t* target_value);

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

int read_tile_definition(TileDefinition** target, uint8_t* target_size){
	SDL_LogTrace(LOG_CAT_UTIL, "Start read_tile_definition().");
	int exit_status = SDL_APP_FAILURE;

	char* raw_file_content = NULL;
	size_t raw_file_content_size = 0;
	
	cJSON* tile_defs = NULL;
	int tile_defs_array_size = 0;

	*target = NULL;
	*target_size = 0;	
	
	const char* filename = get_resource_file_name(TILE_DEFINITION);

	if (!filename || !*filename){
		SDL_LogError(LOG_CAT_UTIL, "Error received invalid filename (NULL or empty).");
		goto cleanup;
	}

	if (!target || !target_size){
		SDL_LogError(LOG_CAT_UTIL, "Error received invalid target or target_size (NULL).");		
		goto cleanup;
	}
	
	if (read_file_content(filename, &raw_file_content, &raw_file_content_size) != 0){
		SDL_LogError(LOG_CAT_UTIL, "Error while reading raw tile definition file content.");
		goto cleanup;
	}

	SDL_LogDebug(LOG_CAT_UTIL, "Raw %s content: \n%s\n", filename, raw_file_content);

	// Parse JSON content
	tile_defs = cJSON_Parse(raw_file_content);
	if (!tile_defs){
		const char* error_ptr = cJSON_GetErrorPtr();
		SDL_LogError(LOG_CAT_UTIL, "Failed to parse JSON in '%s': %s\n", filename, error_ptr ? error_ptr : "Unknown error");		
		goto cleanup;
	}
	
	const cJSON* all_tile_defs = cJSON_GetObjectItemCaseSensitive(tile_defs, "Tiles");
	if(!all_tile_defs || !cJSON_IsArray(all_tile_defs)){
		SDL_LogError(LOG_CAT_UTIL, "Invalid tile configuration content structure: Could not find element 'Tiles'.");
		goto cleanup;
	}
	
	tile_defs_array_size = cJSON_GetArraySize(all_tile_defs);
	if (tile_defs_array_size <= 0){		
		SDL_LogError(LOG_CAT_UTIL, "Tiles array is empty.");
		goto cleanup;
	}

	if (tile_defs_array_size > UINT8_MAX){
		SDL_LogError(LOG_CAT_UTIL, "Tiles array larger than UINT8_MAX.");
		goto cleanup;
	}

	// Setup target array
	SDL_LogDebug(LOG_CAT_UTIL, "Setup tile_definition target array.");
	*target = (TileDefinition*) malloc(tile_defs_array_size * sizeof(TileDefinition*));
	if (!*target){
		SDL_LogError(LOG_CAT_UTIL, "Memory allocation failed for target array (%d entries, %zu bytes).", tile_defs_array_size, tile_defs_array_size * sizeof(TileDefinition*));
		goto cleanup;
	}
	
	const cJSON* tile_def = NULL;
	int tile_index = 0;
	cJSON_ArrayForEach(tile_def, all_tile_defs){
		SDL_LogDebug(LOG_CAT_UTIL, "Parsing json array index %u.", tile_index);
		char* raw_name = "\0";
		char* raw_texture = "\0";
		uint8_t raw_base_water_quality = 0;
		uint8_t raw_base_light_quality = 0;
		uint8_t raw_base_air_quality = 0;
		uint8_t raw_base_soil_quality = 0;
		uint8_t raw_base_temperature_mod = 0;

		bool valid_definitions = 
			validate_json_non_empty_string(tile_def, "name", &raw_name)
			&& validate_json_non_empty_string(tile_def, "texture", &raw_texture)
			&& validate_json_percent_number(tile_def, "base_water_quality", &raw_base_water_quality) 
			&& validate_json_percent_number(tile_def, "base_light_quality", &raw_base_light_quality)
			&& validate_json_percent_number(tile_def, "base_air_quality", &raw_base_air_quality)
			&& validate_json_percent_number(tile_def, "base_soil_quality", &raw_base_soil_quality)
			&& validate_json_percent_number(tile_def, "base_temperature_mod", &raw_base_temperature_mod);

		if (!valid_definitions){
			goto cleanup;
		}

		TileDefinition currentDef = {
			.name = raw_name,
			.texture = raw_texture,
			.base_water_quality = raw_base_water_quality,
			.base_light_quality = raw_base_light_quality,
			.base_air_quality = raw_base_air_quality,
			.base_soil_quality = raw_base_soil_quality,
			.base_temperature_mod = raw_base_temperature_mod,
		};

		(*target)[tile_index] = currentDef;		
		tile_index++;
	}

	*target_size = (uint8_t) tile_defs_array_size;
	exit_status = SDL_APP_CONTINUE;

cleanup:
    if (exit_status != SDL_APP_CONTINUE && *target) {
        for (int i = 0; i < tile_defs_array_size; i++) {
            free((target)[i]);
        }
        free(*target);
        *target = NULL;
    }
    free(raw_file_content);
    cJSON_Delete(tile_defs);
	SDL_LogTrace(LOG_CAT_UTIL, "End read_tile_definition()");
    return exit_status;
}

bool validate_json_non_empty_string(const cJSON* json, const char *key, char** target_value) {
	SDL_LogTrace(LOG_CAT_UTIL, "Start validate_json_non_empty_string()");
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
	
    char* value = cJSON_GetStringValue(item);
	if (!value || strlen(value) == 0){
        SDL_LogError(LOG_CAT_UTIL, "Invalid tile definition in '%s': Empty or Null string.", key);
        return false;
    }

	*target_value = (char*) malloc(strlen(value) + 1);	
    if (!*target_value) {
        SDL_LogError(LOG_CAT_UTIL, "Memory allocation failed for '%s' string (%zu bytes).", key, strlen(value) + 1);
        return false;
    }

	strcpy(*target_value, value);
	SDL_LogTrace(LOG_CAT_UTIL, "End validate_json_non_empty_string()");
    return true;
}

bool validate_json_percent_number(const cJSON* json, const char *key, uint8_t* target_value) {
	SDL_LogTrace(LOG_CAT_UTIL, "Start validate_json_percent_number()");
    cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
	*target_value = 0;
    double value = cJSON_GetNumberValue(item);
	SDL_LogTrace(LOG_CAT_UTIL, "Raw Value: %3.0f", value);
    if (isnan(value) || value < 0 || value > 100) {
        SDL_LogError(LOG_CAT_UTIL, "Invalid tile definition in '%s': Out of range (0 - 100).", key);
        return false;
    }
	*target_value = (uint8_t) value;
	SDL_LogTrace(LOG_CAT_UTIL, "End validate_json_percent_number()");
    return true;
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

void log_with_timestamp(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    // Get category name
    const char* category_str;
    switch (category) {
        case LOG_CAT_DISPLAY:
            category_str = "DISPLAY";
            break;
        case LOG_CAT_MAIN:
            category_str = "MAIN";
            break;
        case LOG_CAT_POPULATION:
            category_str = "POPULATION";
            break;
        case LOG_CAT_UTIL:
            category_str = "UTIL";
            break;
        default:
            category_str = "DEFAULT";
            break;
    }

	const char* prio_str;
    switch (priority) {
        case SDL_LOG_PRIORITY_INVALID:
			prio_str = "INVALID";
            break;
        case SDL_LOG_PRIORITY_TRACE:
			prio_str = "TRACE";
            break;
        case SDL_LOG_PRIORITY_VERBOSE:
			prio_str = "VERBOSE";
            break;
        case SDL_LOG_PRIORITY_DEBUG:
			prio_str = "DEBUG";
            break;
		case SDL_LOG_PRIORITY_INFO:
			prio_str = "INFO";
            break;
		case SDL_LOG_PRIORITY_WARN:
			prio_str = "WARNING";
            break;
		case SDL_LOG_PRIORITY_ERROR:
			prio_str = "ERROR";
            break;
		case SDL_LOG_PRIORITY_CRITICAL:
			prio_str = "CRITICAL";
            break;
		case SDL_LOG_PRIORITY_COUNT:
			prio_str = "COUNT";
            break;			
        default:
			prio_str = "DEFAULT";
            break;
    }

    // Format log message
    char log_message[1024];
    snprintf(log_message, sizeof(log_message), "[%s] [%s] %s\n",
			category_str, prio_str, message);

    // Output to console (stderr)
    fputs(log_message, stderr);
    fflush(stderr);

    // Output to file if open
    if (userdata) {
        FILE* file = (FILE*)userdata;
        fputs(log_message, file);
        fflush(file);
    }
}