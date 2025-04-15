#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <stdlib.h> 
#include <time.h> 
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../include/cJSON.h"
#include "../include/Population.h"
#include "../include/Util.h"
#include "../include/Display.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define HEX_RADIUS 20.0f  // Radius (center to vertex)
#define HEX_COUNT_X 50
#define HEX_COUNT_Y 25

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* grass_texture = NULL;
static SDL_Texture* ice_texture = NULL;
static SDL_Texture* stone_texture = NULL;
static SDL_Texture* desert_texture = NULL;

PopulationUnit p;
char* message = "Hello EcoSim!";

int load_textures(){
    SDL_Surface* grass_bmp = SDL_LoadBMP("img/green-grass-texture.bmp");
    if (grass_bmp == NULL){
        SDL_LogError(LOG_CAT_MAIN, "Could not load green-grass-texture.bmp.");
        return SDL_APP_FAILURE;
    }
    
    grass_texture = SDL_CreateTextureFromSurface(renderer, grass_bmp);
    SDL_DestroySurface(grass_bmp);
    if (grass_texture == NULL){
        SDL_LogError(LOG_CAT_MAIN, "Could not load texture green-grass-texture.png.");
        return SDL_APP_FAILURE;
    }
    
    SDL_Surface* ice_bmp = SDL_LoadBMP("img/Ice.bmp");
    if (ice_bmp == NULL){
        SDL_LogError(LOG_CAT_MAIN, "Could not load ice.bmp.");
        return SDL_APP_FAILURE;
    }
    
    ice_texture = SDL_CreateTextureFromSurface(renderer, ice_bmp);
    SDL_DestroySurface(ice_bmp);
    if (ice_texture == NULL){
        SDL_LogError(LOG_CAT_MAIN, "Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    SDL_Surface* stone_bmp = SDL_LoadBMP("img/Stone.bmp");
    if (stone_bmp == NULL){
        SDL_LogError(LOG_CAT_MAIN, "Could not load stone.bmp.");
        return SDL_APP_FAILURE;
    }
    
    stone_texture = SDL_CreateTextureFromSurface(renderer, stone_bmp);
    SDL_DestroySurface(stone_bmp);
    if (stone_texture == NULL){
        SDL_LogError(LOG_CAT_MAIN, "Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    SDL_Surface* desert_bmp = SDL_LoadBMP("img/Desert.bmp");
    if (desert_bmp == NULL){
        SDL_LogError(LOG_CAT_MAIN, "Could not load desert.bmp.");
        return SDL_APP_FAILURE;
    }
    
    desert_texture = SDL_CreateTextureFromSurface(renderer, desert_bmp);
    SDL_DestroySurface(desert_bmp);
    if (desert_texture == NULL){
        SDL_LogError(LOG_CAT_MAIN, "Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	srand(time(NULL));

    /* Create Population Unit */
	if (create_population_unit(&p) != 0){
		SDL_LogError(LOG_CAT_MAIN, "Error during create population.");
		return 1;
	}

	SDL_LogDebug(LOG_CAT_MAIN, "POP Count: %d", p.pop_count);
	SDL_LogDebug(LOG_CAT_MAIN, "Need Count: %d", p.need_count);
	
	for (int i = 0; i < p.need_count; i++){
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d Name: %s", i, p.base_needs[i].name);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d affinity: %d", i, p.base_needs[i].affinity);
		SDL_LogDebug(LOG_CAT_MAIN, "Base Need %d satisfaction: %d", i, p.base_needs[i].satisfaction);	
	}
    
    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Eco Sim", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_LogError(LOG_CAT_MAIN, "Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Load Textures */
    if (load_textures() != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Setup and Clear screen     
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for (int index_x = 0; index_x < HEX_COUNT_X; index_x++) {
        for (int index_y = 0; index_y < HEX_COUNT_Y; index_y++) {
            float center_x = HEX_RADIUS * 1.5f * index_x + WINDOW_WIDTH / 8.0f;
            float center_y = HEX_RADIUS * sqrtf(3.0f) * (index_y + 0.5f * (index_x % 2)) + WINDOW_HEIGHT / 8.0f;

            // Select texture randomly
            SDL_Texture* texture = NULL;
            uint32_t result = determine_rand_val(0, 3);
            switch (result) {
                case 1: texture = ice_texture; break;
                case 2: texture = desert_texture; break;
                case 3: texture = stone_texture; break;
                default: texture = grass_texture; break;
            }

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
    if (grass_texture) SDL_DestroyTexture(grass_texture);
    if (ice_texture) SDL_DestroyTexture(ice_texture);
    if (stone_texture) SDL_DestroyTexture(stone_texture);
    if (desert_texture) SDL_DestroyTexture(desert_texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    free(p.base_needs);
}