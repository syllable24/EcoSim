
#include <stdlib.h> 
#include <stdint.h>
#include <stdio.h> 

#include "../include/Population.h"
#include "../include/Util.h"

char** g_pop_base_needs = NULL; // Read from File ./res/NeedDefinition.cfg

uint32_t g_next_pop_id = 0;

int add_base_needs(PopulationUnit* p);

int create_population_unit(PopulationUnit* p){
	printf("Start create_population_unit.\n");
	
	if (p == NULL){
		printf("ERR: Received null pointer.\n");
		return 1;
	}
	
	Need* needs = (Need*) malloc(MAX_NEEDS * sizeof(Need));
	if (needs == NULL){
		printf("malloc failed.\n");
		return 1;
	}

	// Initialize needs array to safe defaults
    for (int i = 0; i < MAX_NEEDS; i++) {
        needs[i].name[0] = '\0';
        needs[i].affinity = 0;
        needs[i].satisfaction = 0;
    }

	printf("Init pop.\n");
	p->pop_id = g_next_pop_id++;
    p->pop_count = determine_rand_val(MIN_START_POP_COUNT, MAX_START_POP_COUNT);
    p->need_count = 0;
    p->base_needs = needs;	
	p->luxury_needs = NULL;
	
	/* ADD BASE NEEDS */
	if (add_base_needs(p) != 0){
		printf("ERR during add_base_needs().\n");
		return 1;
	}
	
	printf("End create_population_unit.\n");
	return 0;
}
 
int add_base_needs(PopulationUnit* p){
	printf("Start add_base_needs().\n");
	
	uint8_t base_need_count = 0;

	if (g_pop_base_needs == NULL){
		if (read_definition_from_res_file(NEED_DEFINITION, &g_pop_base_needs, &base_need_count) != 0){
			printf("Error during read need definition.\n");
			return 1;
		}
	}
	
	for (uint8_t i = 0; i < base_need_count; i++){
		Need n;
		if (create_need(&n, g_pop_base_needs[i]) != 0){
			printf("Error during create need.\n");
			return 1;
		}

		if(add_need(p, &n) != 0){
			printf("Error during add need.\n");
			free(p->base_needs);
			return 1;
		}
	}

	printf("End add_base_needs().\n");
	return 0;
}

int add_need(PopulationUnit* p, Need* n) {
    printf("Start add_need().\n");
    
    // Check for NULL pointers
    if (p == NULL || p->base_needs == NULL || n == NULL) {
        printf("Error: NULL pointer in add_need().\n");
        return 1; // Error code
    }
    
    // Check if there's space for a new need
    if (p->need_count >= MAX_NEEDS) {
        printf("Error: Cannot add need, array is full.\n");
        return 1; // Error code
    }
    
    // Add the new need
    p->base_needs[p->need_count] = *n;
    p->need_count++;
    
    printf("End add_need().\n");
    return 0; // Success
}

int create_need(Need* n, char* name){
	printf("Start create_need().\n");
	
	if (n == NULL || name == NULL){
		printf("ERR: Received null pointer.\n");
		return 1;
	}
	
	printf("Init Need: %s.\n", name);
	strcpy(n->name, name);
	n->affinity = determine_rand_percent();
	n->satisfaction = determine_rand_percent();
		
	printf("End create_need().\n");
	return 0;
}

