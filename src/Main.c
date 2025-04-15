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

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define HEX_RADIUS 60.0f  // Radius (center to vertex)
#define HEX_COUNT_X 50
#define HEX_COUNT_Y 25

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

PopulationUnit p;
TileDefinition** tile_definitions = NULL;
SDL_Texture** g_textures = NULL;
uint8_t tile_definition_size = 0;

char* message = "Hello EcoSim!";

void setup_logging(){
    // Set Log Priorities
    SDL_SetLogPriority(LOG_CAT_MAIN, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_DISPLAY, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_POPULATION, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_UTIL, SDL_LOG_PRIORITY_DEBUG);

    // Add timestamp to log lines
    SDL_SetLogOutputFunction(log_with_timestamp, NULL);
}

int load_textures(TileDefinition** tile_definitions, uint8_t tile_definition_size, SDL_Texture*** textures_out){
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start load_textures()");
    int exit_status = SDL_APP_FAILURE;

    SDL_Texture** textures = malloc(tile_definition_size * sizeof(SDL_Texture*));
    if (!textures){
        SDL_LogError(LOG_CAT_DISPLAY, "Memory allocation failed for texture array (%d entries, %zu bytes)", tile_definition_size, tile_definition_size * sizeof(SDL_Texture*));
        goto cleanup;
    }

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
        textures[tile_definition_index] = texture;
    }
    
    *textures_out = textures;    
    exit_status = SDL_APP_CONTINUE;

cleanup: 
    if (exit_status != SDL_APP_CONTINUE){
        for (int i = 0; i < tile_definition_index; i++){
            SDL_DestroyTexture(textures[i]);
            textures[i] = NULL;
        }
        free(textures);
    }
    SDL_LogTrace(LOG_CAT_DISPLAY, "End load_textures()");
    return exit_status;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	
    srand(time(NULL));

    setup_logging();
    
    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Eco Sim", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_LogError(LOG_CAT_MAIN, "Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    SDL_LogDebug(LOG_CAT_MAIN, "Reading Tile Definiitons.");
    if (read_tile_definition(&tile_definitions, &tile_definition_size) != SDL_APP_CONTINUE){
		SDL_LogError(LOG_CAT_MAIN, "Error during read tile definition.");
		return SDL_APP_FAILURE;
    }

    /* Load Textures */
    SDL_LogDebug(LOG_CAT_MAIN, "Loading Textures.");    
    if (load_textures(tile_definitions, tile_definition_size, &g_textures) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Couldn't load textures: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Create Population Unit */
	if (create_population_unit(&p) != 0){
		SDL_LogError(LOG_CAT_MAIN, "Error during create population.");
		return SDL_APP_FAILURE;
	}

	SDL_LogDebug(LOG_CAT_MAIN, "POP Count: %d", p.pop_count);
	SDL_LogDebug(LOG_CAT_MAIN, "Need Count: %d", p.need_count);
	
	for (int i = 0; i < p.need_count; i++){
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d Name: %s", i, p.base_needs[i].name);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d affinity: %d", i, p.base_needs[i].affinity);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d satisfaction: %d", i, p.base_needs[i].satisfaction);	
	}

    // Setup and Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for (int index_x = 0; index_x < HEX_COUNT_X; index_x++) {
        for (int index_y = 0; index_y < HEX_COUNT_Y; index_y++) {
            float center_x = HEX_RADIUS * 1.5f * index_x + WINDOW_WIDTH / 8.0f;
            float center_y = HEX_RADIUS * sqrtf(3.0f) * (index_y + 0.5f * (index_x % 2)) + WINDOW_HEIGHT / 8.0f;

            // Select tile texture randomly
            SDL_Texture* texture = NULL;
            uint32_t result = determine_rand_val(0, tile_definition_size);
            texture = g_textures[result];            

            SDL_FPoint points[6];
            get_hexagon_vertices(points, center_x, center_y, HEX_RADIUS);

            if (draw_hexagon_texture(renderer, texture, points, center_x, center_y) != SDL_APP_CONTINUE){
                SDL_LogError(LOG_CAT_MAIN, "Error during draw_hexagon_texture: %s.", SDL_GetError());
                return SDL_APP_FAILURE;
            }

            // Draw hexagon outline
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            draw_hexagon_outline(renderer, points);
        }
    }

    // Present
    SDL_RenderPresent(renderer);   

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
    if(g_textures){
        for (int i = 0; i < tile_definition_size; i++){
            SDL_DestroyTexture(g_textures[i]);            
        }
        free(g_textures);
        g_textures = NULL;
    }

    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    free(p.base_needs);
    free(tile_definitions);
}