#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <stdlib.h> 
#include <time.h> 
#include <stdio.h> 
#include <math.h> 
#include <time.h>
#include <sys/time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../include/Hashmap.h"
#include "../include/cJSON.h"
#include "../include/Population.h"
#include "../include/Util.h"
#include "../include/Display.h"
#include "../include/MapGen.h"

#define HEX_COUNT_X 50
#define HEX_COUNT_Y 25

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

PopulationUnit p;
TileDefinition** tile_definitions = NULL;
TileState** g_game_board = NULL;
struct hashmap* g_texture_map = NULL;
uint8_t tile_definition_size = 0;

char* message = "Hello EcoSim!";

void setup_logging(){
    // Set Log Priorities
    SDL_SetLogPriority(LOG_CAT_MAIN, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_DISPLAY, SDL_LOG_PRIORITY_TRACE);
    SDL_SetLogPriority(LOG_CAT_POPULATION, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_UTIL, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_MAPGEN, SDL_LOG_PRIORITY_DEBUG);

    // TODO: Add timestamp to log lines
    SDL_SetLogOutputFunction(log_with_timestamp, NULL);
}

int load_textures(TileDefinition** tile_definitions, uint8_t tile_definition_size, struct hashmap** texture_map){
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start load_textures()");
    int exit_status = SDL_APP_FAILURE;

    (*texture_map) = hashmap_new(sizeof(TextureHashMapRecord), tile_definition_size, 0, 0, texture_hash_map_hash, texture_hash_map_compare, NULL, NULL);    

    uint8_t tile_definition_index = 0;
    for (tile_definition_index = 0; tile_definition_index < tile_definition_size; tile_definition_index++){
        SDL_Surface* bmp = SDL_LoadBMP(tile_definitions[tile_definition_index]->texture);
        if (!bmp){
            SDL_LogError(LOG_CAT_DISPLAY, "Could not load %s.", tile_definitions[tile_definition_index]->texture);
            goto cleanup;
        }
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, bmp);
        SDL_DestroySurface(bmp);
        if (!texture){
            SDL_LogError(LOG_CAT_DISPLAY, "Could not create texture for %s.", tile_definitions[tile_definition_index]->texture);
            goto cleanup;            
        }
        
        hashmap_set((*texture_map), &(TextureHashMapRecord){ 
            .name = tile_definitions[tile_definition_index]->name,
            .texture = texture
        });        
    }
    
    exit_status = SDL_APP_CONTINUE;

cleanup: 
    if (exit_status != SDL_APP_CONTINUE){
        hashmap_free(g_texture_map);
    }
    SDL_LogTrace(LOG_CAT_DISPLAY, "End load_textures()");
    return exit_status;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	
    srand(time(NULL));

    setup_logging();
    
    /* Create the window */
    SDL_LogDebug(LOG_CAT_MAIN, "Creating Window and Renderer.");
    if (!SDL_CreateWindowAndRenderer("Eco Sim", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_LogError(LOG_CAT_MAIN, "Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    /* Read Tile Definitions */
    SDL_LogDebug(LOG_CAT_MAIN, "Reading Tile Definiitons.");
    if (read_tile_definition(&tile_definitions, &tile_definition_size) != SDL_APP_CONTINUE){
		SDL_LogError(LOG_CAT_MAIN, "Error during read tile definition.");
		return SDL_APP_FAILURE;
    }

    /* Load Textures */
    SDL_LogDebug(LOG_CAT_MAIN, "Loading Textures.");    
    if (load_textures(tile_definitions, tile_definition_size, &g_texture_map) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Couldn't load textures: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Generate Map */
    if(generate_board(&g_game_board, tile_definitions, tile_definition_size, HEX_COUNT_X, HEX_COUNT_Y) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Error while generating board: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Create Population Unit */
	if (create_population_unit(&p) != 0){
		SDL_LogError(LOG_CAT_MAIN, "Error during create population.");
		return SDL_APP_FAILURE;
	}

    log_pop_unit(&p);

    /* Draw Initial Map*/
    if(draw_tile_map(renderer, g_game_board, g_texture_map, tile_definition_size, HEX_COUNT_X, HEX_COUNT_Y) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Error while drawing tile map: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
        
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate){   
	return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result){    
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    hashmap_free(g_texture_map);
    free(p.base_needs);
    free(tile_definitions);
}