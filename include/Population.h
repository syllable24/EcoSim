#ifndef POPULATION_H
#define POPULATION_H

#include <string.h>
#include <stdint.h>

#include "Util.h"

#define MIN_START_POP_COUNT 2500
#define MAX_START_POP_COUNT 5000
#define MAX_NEEDS 1024
#define BASE_NEED_COUNT 5

typedef struct {
	char name[MAX_NAME_LENGTH];
	uint8_t affinity;    // 0 - 100%. How important is the need to to pop?
	uint8_t satisfaction;// 0 - 100%. Current need satisfaction. 
} Need;

typedef struct {
	char name[MAX_NAME_LENGTH];	
	char covers_need[MAX_NAME_LENGTH];
	uint8_t input_satisfaction;  // 0 - 100%. How well are the production requirements met?
	uint8_t throughput_skill;    // 0 - 100%. How skilled is the pop with producing?
	uint8_t output_satisfaction; // 0 - 100%. How well are the produced goods selling?
} Production;

typedef struct {
	char name[MAX_NAME_LENGTH];		
	Production* product_lines;
} ProductionSegment;

typedef struct {
	uint32_t pop_id;
	uint32_t pop_count;	
	Need* base_needs;
	uint32_t need_count;
	Need* luxury_needs;
	uint32_t luxury_need_count;

	ProductionSegment production_segment;
} PopulationUnit;

extern char** g_pop_base_needs;

int create_population_unit(PopulationUnit* p);

int add_need(PopulationUnit* p, Need* n);

int create_need(Need* n, char* name);

void log_pop_unit(PopulationUnit* p);

#endif