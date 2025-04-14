#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/Util.h"

#define RESOURCE_FILE_COUNT 1
#define LINE_BUFFER_SIZE 1024

static const char* resource_file_names[RESOURCE_FILE_COUNT] = {
	"./res/NeedDefinition.cfg"
};

const char* get_resource_file_name(ResourceFiles file);

void trim(char* line);

uint32_t determine_rand_val(int min, int max){
	return rand() % (max - min + 1) + min;	
}

uint8_t determine_rand_percent(){
	return rand() % (100 + 1);	
}

int read_definition_from_res_file(ResourceFiles file, char*** target, uint8_t* target_size){
	printf("Start read_definition_from_res_file.\n");

	const char* filename = get_resource_file_name(file);
	
	printf("Try opening file %s.\n", filename);
	FILE* resFile = fopen(filename, "r");
	if (resFile == NULL){
		perror("Error opening file.");
		return 1;
	}
	
	char line_buffer[LINE_BUFFER_SIZE];
	int need_index = 0;
	int found_header = 0;	
	int capacity = 10; // Initial guess

	*target = malloc(capacity * sizeof(char*));
	if (*target == NULL){
		fclose(resFile);
		perror("Malloc error.");
		return 1;
	}
	for (int i = 0; i < capacity; i++){
		(*target)[i] = NULL;
	}

	printf("Reading from file %s.\n", filename);
	while(fgets(line_buffer, sizeof(line_buffer), resFile) != NULL){								
		
		trim(line_buffer);		
		
		// Skip empty lines
		if (strlen(line_buffer) == 0){
			continue;
		}

		// Resource structure specific part
		if (!found_header){ 
			// Check for Header
			if (strstr(line_buffer, "BASE_NEEDS:") != NULL){
				printf("Found Header. Processing content line.\n");
				found_header = 1;
				continue;
			}
		}
		else {
			// Process Content
			printf("Removing quotes.\n");
			char* quote_start = strchr(line_buffer, '"');
			char* quote_end = strrchr(line_buffer, '"');

			if (quote_start && quote_end && quote_start < quote_end){
				*quote_end = '\0'; // Terminate string at trailing quote
				quote_start++;
				printf("Copy Value: '%s' to target.\n", quote_start);

				// Realloc in case capacity exceeded
				if (need_index >= capacity){
					capacity *= 2;
					char** temp = realloc(*target, capacity * sizeof(char*));
					if (temp == NULL){
						for (int i = 0; i < need_index; i++){
							free((*target)[i]);
						}	
						free((*target));
						fclose(resFile);
						printf("Malloc error.");
						return 1;
					}
					*target = temp;
					// Init newly allocated memory to NULL
					for (int i = need_index; i < capacity; i++) {
                        (*target)[i] = NULL;
                    }					
				}

				// Allocate memory for line 
				(*target)[need_index] = (char*) malloc((strlen(quote_start) + 1 * sizeof(char)));
				if((*target)[need_index] == NULL){
					printf("Malloc error.");
					for (int i = 0; i < need_index; i++){
						free((*target)[i]);
					}
					free(*target);
					fclose(resFile);
					return 1;
				}

				strcpy((*target)[need_index], quote_start);	
				need_index++;
			}
		}
	}

	*target_size = need_index;
	fclose(resFile);

	printf("Read %u entries.\n", *target_size);

	printf("End read_definition_from_res_file.\n");
	return 0;
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