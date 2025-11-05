
#include <stdlib.h> 
#include <stdint.h>
#include <stdio.h> 
#include <SDL3/SDL.h>

#include "Globals.h"
#include "Population.h"
#include "Util.h"

char** g_pop_survival_needs = NULL; // Read from File ./res/NeedDefinition.cfg
uint8_t g_pop_survival_need_count = 0;

uint32_t g_next_pop_id = 0;

int add_survival_needs(PopulationUnit* p);

int create_population_unit(PopulationUnit* p){
	SDL_LogTrace(LOG_CAT_POPULATION, "Start create_population_unit.");
	
	if (p == NULL){
		SDL_LogError(LOG_CAT_POPULATION, "Received null PopulationUnit pointer.");		
		return 1;
	}
		
	SDL_LogDebug(LOG_CAT_POPULATION, "Init pop.");
	p->pop_id = g_next_pop_id++;
    p->pop_count = determine_rand_val(MIN_START_POP_COUNT, MAX_START_POP_COUNT);    		
	
	p->survival_needs = calloc(MAX_NEEDS, sizeof(Need));
	p->survival_need_count = 0;
    
	p->base_needs = calloc(MAX_NEEDS, sizeof(Need));;
	p->base_need_count = 0;

	p->luxury_needs = calloc(MAX_NEEDS, sizeof(Need));
	p->luxury_need_count = 0;
	
	if(!p->survival_needs || !p->base_needs || !p->luxury_needs){
		SDL_LogError(LOG_CAT_POPULATION, "Error during malloc need arrays.");
		free(p->survival_needs);
		free(p->base_needs);
		free(p->luxury_needs);
		return 1;
	}
	
	/* ADD SURVIVAL NEEDS */
	if (add_survival_needs(p) != 0){
		SDL_LogError(LOG_CAT_POPULATION, "Error during add_base_needs().");
		free(p->survival_needs);
		free(p->base_needs);
		free(p->luxury_needs);		
		return 1;
	}
	
	SDL_LogTrace(LOG_CAT_POPULATION, "End create_population_unit()");
	return 0;
}
 
int add_survival_needs(PopulationUnit* p){
	SDL_LogTrace(LOG_CAT_POPULATION, "Start add_base_needs().");		

	if (g_pop_survival_needs == NULL){
		if (read_definition_from_res_file(NEED_DEFINITION, &g_pop_survival_needs, &g_pop_survival_need_count) != 0){
			SDL_LogError(LOG_CAT_POPULATION, "Error during read need definition.");
			return 1;
		}
	}
	
	for (uint8_t i = 0; i < g_pop_survival_need_count; i++){
		Need n;
		if (create_need(&n, g_pop_survival_needs[i]) != 0){
			SDL_LogError(LOG_CAT_POPULATION, "Error during create_need().");
			return 1;
		}

		if(add_survival_need(p, &n) != 0){
			SDL_LogError(LOG_CAT_POPULATION, "Error during add_survival_need().");
			free(p->survival_needs);
			return 1;
		}
	}

	SDL_LogTrace(LOG_CAT_POPULATION, "End add_base_needs().");
	return 0;
}

int add_survival_need(PopulationUnit* p, Need* n) {
    SDL_LogTrace(LOG_CAT_POPULATION, "Start add_survival_need().");
    
    if (p == NULL || p->survival_needs == NULL || n == NULL) {
        SDL_LogError(LOG_CAT_POPULATION, "Received NULL pointer in add_survival_need().");
        return 1;
    }
    
    // Check if there's space for a new need
    if (p->survival_need_count >= MAX_NEEDS) {
        SDL_LogError(LOG_CAT_POPULATION, "Cannot add need, maximum size of %d needs reached.", MAX_NEEDS);
        return 1;
    }
    
    // Add the new need
    p->survival_needs[p->survival_need_count] = *n;
    p->survival_need_count++;
    
    SDL_LogTrace(LOG_CAT_POPULATION, "End add_survival_need().");
    return 0;
}

int create_need(Need* n, char* name){
	SDL_LogTrace(LOG_CAT_POPULATION, "Start create_need().");
	
	if (n == NULL || name == NULL){
		SDL_LogError(LOG_CAT_POPULATION, "Received null pointer.");
		return 1;
	}
	
	SDL_LogDebug(LOG_CAT_POPULATION, "Init Need: %s.", name);
	memset(n->name, 0, POP_TEXT_LENGTH); // Properly init name string
	strcpy(n->name, name);
	n->affinity = determine_rand_percent();
	n->satisfaction = determine_rand_percent();
		
	SDL_LogTrace(LOG_CAT_POPULATION, "End create_need().");
	return 0;
}

void log_pop_unit(PopulationUnit* p){
	SDL_LogDebug(LOG_CAT_MAIN, "POP Count: %d", p->pop_count);
	SDL_LogDebug(LOG_CAT_MAIN, "Survival Need Count: %d", p->survival_need_count);
	SDL_LogDebug(LOG_CAT_MAIN, "Base Need Count: %d", p->base_need_count);
	SDL_LogDebug(LOG_CAT_MAIN, "Luxury Need Count: %d", p->luxury_need_count);
	
	for (int i = 0; i < p->survival_need_count; i++){
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d Name: %s", i, p->survival_needs[i].name);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d affinity: %d", i, p->survival_needs[i].affinity);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d satisfaction: %d", i, p->survival_needs[i].satisfaction);	
	}

	for (int i = 0; i < p->base_need_count; i++){
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d Name: %s", i, p->base_needs[i].name);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d affinity: %d", i, p->base_needs[i].affinity);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d satisfaction: %d", i, p->base_needs[i].satisfaction);	
	}

	for (int i = 0; i < p->luxury_need_count; i++){
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d Name: %s", i, p->luxury_needs[i].name);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d affinity: %d", i, p->luxury_needs[i].affinity);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d satisfaction: %d", i, p->luxury_needs[i].satisfaction);	
	}	
}