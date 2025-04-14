#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <stdlib.h> 
#include <time.h> 
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "../include/Population.h"
#include "../include/Util.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define HEX_RADIUS 20.0f  // Radius (center to vertex)
#define HEX_MASK_SIZE (2.0f * HEX_RADIUS)  // Hex mask texture size
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

// Calculate the six vertices of a flat-top hexagon
void get_hexagon_vertices(SDL_FPoint* points, float center_x, float center_y, float radius) {
    for (int i = 0; i < 6; i++) {
        float angle = (float)(M_PI / 3.0 * i);
        points[i].x = center_x + radius * cosf(angle);
        points[i].y = center_y + radius * sinf(angle);
    }
}

// Draw a hexagon outline by connecting vertices
void draw_hexagon_outline(SDL_Renderer* renderer, SDL_FPoint* vertices) {
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;
        SDL_RenderLine(
            renderer, 
            vertices[i].x, vertices[i].y,
            vertices[next].x, vertices[next].y
        );
    }
}

int draw_hexagon_texture(SDL_Texture* texture, SDL_FPoint points[6], float center_x, float center_y){
    // Define hexagon vertices and texture coordinates
    SDL_Vertex vertices[7];     

    // Center vertex
    vertices[0].position.x = center_x;
    vertices[0].position.y = center_y;
    vertices[0].tex_coord.x = 0.5f; // Center of texture
    vertices[0].tex_coord.y = 0.5f;
    vertices[0].color.r = 1.0f;
    vertices[0].color.g = 1.0f;
    vertices[0].color.b = 1.0f;
    vertices[0].color.a = 1.0f;

    // Outer vertices
    for (int i = 0; i < 6; i++) {
        vertices[i + 1].position = points[i];
        // Map texture coordinates to fit hexagon
        float tex_angle = (float)(M_PI / 3.0 * i);
        vertices[i + 1].tex_coord.x = 0.5f + 0.4f * cosf(tex_angle);
        vertices[i + 1].tex_coord.y = 0.5f + 0.4f * sinf(tex_angle);
        vertices[i + 1].color.r = 1.0f;
        vertices[i + 1].color.g = 1.0f;
        vertices[i + 1].color.b = 1.0f;
        vertices[i + 1].color.a = 1.0f;
    }

    // Define triangle indices for fan
    int indices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1 };

    if (SDL_RenderGeometry(renderer, texture, vertices, 7, indices, 18) < 0) {
        printf("RenderGeometry failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

int load_textures(){
    SDL_Surface* grass_bmp = SDL_LoadBMP("img/green-grass-texture.bmp");
    if (grass_bmp == NULL){
        SDL_Log("Could not load green-grass-texture.bmp.");
        return SDL_APP_FAILURE;
    }
    
    grass_texture = SDL_CreateTextureFromSurface(renderer, grass_bmp);
    SDL_DestroySurface(grass_bmp);
    if (grass_texture == NULL){
        SDL_Log("Could not load texture green-grass-texture.png.");
        return SDL_APP_FAILURE;
    }
    
    SDL_Surface* ice_bmp = SDL_LoadBMP("img/Ice.bmp");
    if (ice_bmp == NULL){
        SDL_Log("Could not load ice.bmp.");
        return SDL_APP_FAILURE;
    }
    
    ice_texture = SDL_CreateTextureFromSurface(renderer, ice_bmp);
    SDL_DestroySurface(ice_bmp);
    if (ice_texture == NULL){
        SDL_Log("Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    SDL_Surface* stone_bmp = SDL_LoadBMP("img/Stone.bmp");
    if (stone_bmp == NULL){
        SDL_Log("Could not load stone.bmp.");
        return SDL_APP_FAILURE;
    }
    
    stone_texture = SDL_CreateTextureFromSurface(renderer, stone_bmp);
    SDL_DestroySurface(stone_bmp);
    if (stone_texture == NULL){
        SDL_Log("Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    SDL_Surface* desert_bmp = SDL_LoadBMP("img/Desert.bmp");
    if (desert_bmp == NULL){
        SDL_Log("Could not load desert.bmp.");
        return SDL_APP_FAILURE;
    }
    
    desert_texture = SDL_CreateTextureFromSurface(renderer, desert_bmp);
    SDL_DestroySurface(desert_bmp);
    if (desert_texture == NULL){
        SDL_Log("Could not load texture ice.bmp.");
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
	srand(time(NULL));

    /* Create Population Unit */
	if (create_population_unit(&p) != 0){
		printf("Error during create population.\n");
		return 1;
	}

	printf("POP Count: %d \n", p.pop_count);
	printf("Need Count: %d \n", p.need_count);
	
	for (int i = 0; i < p.need_count; i++){
		printf("Base Need %d Name: %s\n", i, p.base_needs[i].name);
		printf("Base Need %d affinity: %d\n", i, p.base_needs[i].affinity);
		printf("Base Need %d satisfaction: %d\n", i, p.base_needs[i].satisfaction);	
	}
    
    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Eco Sim", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Load Textures */
    if (load_textures() != SDL_APP_CONTINUE){
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
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

            if (draw_hexagon_texture(texture, points, center_x, center_y) != SDL_APP_CONTINUE){
                SDL_Log("Error during draw_hexagon_texture: %s.\n", SDL_GetError());
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
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    free(p.base_needs);
}