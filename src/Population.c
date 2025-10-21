
#include <stdlib.h> 
#include <stdint.h>
#include <stdio.h> 
#include <SDL3/SDL.h>

#include "Globals.h"
#include "Population.h"
#include "Util.h"

char** g_pop_base_needs = NULL; // Read from File ./res/NeedDefinition.cfg

uint32_t g_next_pop_id = 0;

int add_base_needs(PopulationUnit* p);

int create_population_unit(PopulationUnit* p){
	SDL_LogTrace(LOG_CAT_POPULATION, "Start create_population_unit.");
	
	if (p == NULL){
		SDL_LogError(LOG_CAT_POPULATION, "Received null PopulationUnit pointer.");		
		return 1;
	}
	
	Need* needs = (Need*) malloc(MAX_NEEDS * sizeof(Need));
	if (needs == NULL){
		SDL_LogError(LOG_CAT_POPULATION, "Memory allocation for needs array failed (%d entries, %zu bytes).", MAX_NEEDS, MAX_NEEDS * sizeof(Need));
		return 1;
	}

	// Initialize needs array to safe defaults
    for (int i = 0; i < MAX_NEEDS; i++) {
        needs[i].name[0] = '\0';
        needs[i].affinity = 0;
        needs[i].satisfaction = 0;
    }

	SDL_LogDebug(LOG_CAT_POPULATION, "Init pop.");
	p->pop_id = g_next_pop_id++;
    p->pop_count = determine_rand_val(MIN_START_POP_COUNT, MAX_START_POP_COUNT);
    p->need_count = 0;
    p->base_needs = needs;	
	p->luxury_needs = NULL;
	
	/* ADD BASE NEEDS */
	if (add_base_needs(p) != 0){
		SDL_LogError(LOG_CAT_POPULATION, "Error during add_base_needs().");
		return 1;
	}
	
	SDL_LogTrace(LOG_CAT_POPULATION, "End create_population_unit()");
	return 0;
}
 
int add_base_needs(PopulationUnit* p){
	SDL_LogTrace(LOG_CAT_POPULATION, "Start add_base_needs().");
	
	uint8_t base_need_count = 0;

	if (g_pop_base_needs == NULL){
		if (read_definition_from_res_file(NEED_DEFINITION, &g_pop_base_needs, &base_need_count) != 0){
			SDL_LogError(LOG_CAT_POPULATION, "Error during read need definition.");
			return 1;
		}
	}
	
	for (uint8_t i = 0; i < base_need_count; i++){
		Need n;
		if (create_need(&n, g_pop_base_needs[i]) != 0){
			SDL_LogError(LOG_CAT_POPULATION, "Error during create_need().");
			return 1;
		}

		if(add_need(p, &n) != 0){
			SDL_LogError(LOG_CAT_POPULATION, "Error during add_need().");
			free(p->base_needs);
			return 1;
		}
	}

	SDL_LogTrace(LOG_CAT_POPULATION, "End add_base_needs().");
	return 0;
}

int add_need(PopulationUnit* p, Need* n) {
    SDL_LogTrace(LOG_CAT_POPULATION, "Start add_need().");
    
    if (p == NULL || p->base_needs == NULL || n == NULL) {
        SDL_LogError(LOG_CAT_POPULATION, "Received NULL pointer in add_need().");
        return 1;
    }
    
    // Check if there's space for a new need
    if (p->need_count >= MAX_NEEDS) {
        SDL_LogError(LOG_CAT_POPULATION, "Cannot add need, maximum size of %d needs reached.", MAX_NEEDS);
        return 1;
    }
    
    // Add the new need
    p->base_needs[p->need_count] = *n;
    p->need_count++;
    
    SDL_LogTrace(LOG_CAT_POPULATION, "End add_need().");
    return 0;
}

int create_need(Need* n, char* name){
	SDL_LogTrace(LOG_CAT_POPULATION, "Start create_need().");
	
	if (n == NULL || name == NULL){
		SDL_LogError(LOG_CAT_POPULATION, "Received null pointer.");
		return 1;
	}
	
	SDL_LogDebug(LOG_CAT_POPULATION, "Init Need: %s.", name);
	strcpy(n->name, name);
	n->affinity = determine_rand_percent();
	n->satisfaction = determine_rand_percent();
		
	SDL_LogTrace(LOG_CAT_POPULATION, "End create_need().");
	return 0;
}

void log_pop_unit(PopulationUnit* p){
	SDL_LogDebug(LOG_CAT_MAIN, "POP Count: %d", p->pop_count);
	SDL_LogDebug(LOG_CAT_MAIN, "Need Count: %d", p->need_count);
	
	for (int i = 0; i < p->need_count; i++){
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d Name: %s", i, p->base_needs[i].name);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d affinity: %d", i, p->base_needs[i].affinity);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d satisfaction: %d", i, p->base_needs[i].satisfaction);	
	}
}