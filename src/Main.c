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
#include "../include/Globals.h"

PopulationUnit p;

char* message = "Hello EcoSim!";

void setup_logging(){
    // Set Log Priorities
    SDL_SetLogPriority(LOG_CAT_MAIN, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_DISPLAY, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_POPULATION, SDL_LOG_PRIORITY_INFO);
    SDL_SetLogPriority(LOG_CAT_UTIL, SDL_LOG_PRIORITY_INFO);
    SDL_SetLogPriority(LOG_CAT_MAPGEN, SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(LOG_CAT_HEXMATH, SDL_LOG_PRIORITY_INFO);

    // TODO: Add timestamp to log lines
    SDL_SetLogOutputFunction(log_with_timestamp, NULL);
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	
    srand(time(NULL));

    setup_logging();
    
    /* Create the window */
    SDL_LogDebug(LOG_CAT_MAIN, "Creating Window and Renderer.");
    if (!SDL_CreateWindowAndRenderer("Eco Sim", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN, &g_window, &g_renderer)) {
        SDL_LogError(LOG_CAT_MAIN, "Couldn't create g_window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    /* Read Tile Definitions */
    SDL_LogDebug(LOG_CAT_MAIN, "Reading Tile Definiitons.");
    if (read_tile_definition(&g_arr_tile_definitions, &g_arr_tile_definitions_size) != SDL_APP_CONTINUE){
		SDL_LogError(LOG_CAT_MAIN, "Error during read tile definition.");
		return SDL_APP_FAILURE;
    }

    /* Load Textures */
    SDL_LogDebug(LOG_CAT_MAIN, "Loading Textures.");    
    if (load_textures(g_arr_tile_definitions, g_arr_tile_definitions_size) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Couldn't load textures: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    /* Generate Map */
    if(generate_map(HEX_GRID_RADIUS) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Error while generating map: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Create Population Unit */
	if (create_population_unit(&p) != 0){
		SDL_LogError(LOG_CAT_MAIN, "Error during create population.");
		return SDL_APP_FAILURE;
	}

    log_pop_unit(&p);

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
        
    switch(event->type){
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;

        case SDL_EVENT_KEY_DOWN:
            switch(event->key.key){
                case SDLK_ESCAPE:
                    return SDL_APP_SUCCESS;
                default:
                    return SDL_APP_CONTINUE;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            switch(event->button.button){
                case 1: // Left-Click
                    SDL_LogDebug(LOG_CAT_MAIN, "Left-Click");
                    handle_left_click(g_renderer);
                    break;
                case 2: // Middle-Click
                    SDL_LogDebug(LOG_CAT_MAIN, "Middle-Click");
                    break;
                case 3: // Right-Click
                    SDL_LogDebug(LOG_CAT_MAIN, "Right-Click");
                    break;
            }            
            return SDL_APP_CONTINUE;
        default:
            return SDL_APP_CONTINUE;
    }
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate){    
    
    // Update mouse coordinates
    SDL_GetGlobalMouseState(&g_mouse_pos_x, &g_mouse_pos_y);

    // Calculate delta time in seconds
    uint32_t frame_start = SDL_GetTicks();

    static uint32_t last_time = 0;
    if (last_time == 0) last_time = frame_start;
    float delta_time = (frame_start - last_time) / 1000.0f;
    last_time = frame_start;
    
    int result = frame_update(delta_time);
    
    uint32_t frame_end = SDL_GetTicks();
    uint32_t frame_duration = frame_end - frame_start;

    if (frame_duration < TARGET_FRAME_TIME_MS){
        SDL_Delay(TARGET_FRAME_TIME_MS - frame_duration);
    }

    return result;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result){    
    if (g_renderer) SDL_DestroyRenderer(g_renderer);
    if (g_window) SDL_DestroyWindow(g_window);
    
    clear_display_state();    
    free(p.base_needs);
    cleanup_globals();
}