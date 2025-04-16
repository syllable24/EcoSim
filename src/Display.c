#include <stdlib.h> 
#include <time.h> 
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>

#include "../include/Globals.h"
#include "../include/Util.h"
#include "../include/Display.h"
#include "../include/Hashmap.h"

struct hashmap* g_texture_map = NULL;

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

int draw_hexagon_texture(SDL_Renderer* renderer, SDL_Texture* texture, SDL_FPoint points[6], float center_x, float center_y){    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

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
    int indices[] = { 
        0, 1, 2, 
        0, 2, 3, 
        0, 3, 4, 
        0, 4, 5, 
        0, 5, 6, 
        0, 6, 1 
    };

    if (SDL_RenderGeometry(renderer, texture, vertices, 7, indices, 18) < 0) {
        SDL_LogError(LOG_CAT_DISPLAY, "RenderGeometry failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

int draw_tile_map(
    SDL_Renderer* renderer,    
    uint8_t map_size_x, 
    uint8_t map_size_y
){
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start draw_tile_map().");

    // Setup and Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);        

    // Draw Hex-Grid Background Texture
    SDL_FPoint top_left_menu = {
        .x = WINDOW_WIDTH / 8.0f,
        .y = WINDOW_HEIGHT / 8.0f
    };

    SDL_FRect border = {
        .x = top_left_menu.x,
        .y = top_left_menu.y,
        .w = WINDOW_WIDTH - (WINDOW_WIDTH / 16.0f),
        .h = WINDOW_HEIGHT - (WINDOW_HEIGHT / 16.0f)
    };
    
    const TextureHashMapRecord* background_rec = hashmap_get(g_texture_map, &(TextureHashMapRecord){.name="Hex Grid Background"});
    SDL_RenderTextureTiled(renderer, background_rec->texture, NULL, 1.0f, &border);    

    // Draw Hex Grid
    SDL_FPoint top_left_hex_grid = {
        .x = WINDOW_WIDTH / 6.0f,
        .y = WINDOW_HEIGHT / 6.0f
    };
   
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start Draw Hexes.");
        
    for (uint8_t index_x = 0; index_x < map_size_x; index_x++) {        
        for (uint8_t index_y = 0; index_y < map_size_y; index_y++) {
            
            // Validate game_board entry
            if (!g_game_board[index_x] || !g_game_board[index_x][index_y].tile_def || !g_game_board[index_x][index_y].tile_def->name) {
                SDL_LogError(LOG_CAT_DISPLAY, "Invalid tile at [%u][%u]: NULL tile_def or name", index_x, index_y);
                SDL_RenderPresent(renderer);
                return SDL_APP_FAILURE;
            }

            // Calculate Hex Texture center (staggered grid)
            float base_center_x = HEX_RADIUS * 1.5f * index_x + top_left_hex_grid.x;
            float base_center_y = HEX_RADIUS * sqrtf(3.0f) * (index_y + 0.5f * (index_x % 2)) + top_left_hex_grid.y;

            // Add camera offset
            float center_x = base_center_x + camera_offset_x;
            float center_y = base_center_y + camera_offset_y;

            SDL_LogTrace(LOG_CAT_DISPLAY, "Hex Center X: %05.02f Y: %05.02f.", center_x, center_y);

            // Skip off-screen hexagons
            float min_render_x = HEX_RADIUS + ((WINDOW_WIDTH / 8.0f) * 0.6f);
            float max_render_x = WINDOW_WIDTH + HEX_RADIUS;

            float min_render_y = (-(HEX_RADIUS + (WINDOW_HEIGHT / 8.0f)) * 0.6f);
            float max_render_y = WINDOW_HEIGHT + HEX_RADIUS;

            if (center_x < min_render_x || center_x > max_render_x ||
                center_y < -min_render_y || center_y > max_render_y) {
                continue;
            }

            // Get Hex Texture by name
            char* hex_def_name = g_game_board[index_x][index_y].tile_def->name;
            SDL_LogTrace(LOG_CAT_DISPLAY, "Hex def name: %s.", hex_def_name);
            const TextureHashMapRecord* rec = hashmap_get(g_texture_map, &(TextureHashMapRecord){.name=hex_def_name});
            if (!rec || !rec->texture) {
                SDL_LogError(LOG_CAT_DISPLAY, "No texture found for name '%s' at [%u][%u]", hex_def_name, index_x, index_y);
                SDL_RenderPresent(renderer);
                return SDL_APP_FAILURE;
            }            
            SDL_Texture* texture = rec->texture;

            // Calc Hex vertices
            SDL_FPoint points[6];
            get_hexagon_vertices(points, center_x, center_y, HEX_RADIUS);

            // Draw Hex Texture to screen
            if (draw_hexagon_texture(renderer, texture, points, center_x, center_y) != SDL_APP_CONTINUE){
                SDL_LogError(LOG_CAT_MAIN, "Error during draw_hexagon_texture: %s.", SDL_GetError());
                return SDL_APP_FAILURE;
            }

            // Draw hex outline
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            draw_hexagon_outline(renderer, points);
        }
    }

    // Draw Box around grid    
    SDL_LogTrace(LOG_CAT_DISPLAY, "Draw Board Borders.");

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderRect(renderer, &border);

    // DEBUG
    SDL_RenderDebugTextFormat(renderer, 0, 0, "Camera Offset: X: %04.02f, Y: %04.02f", camera_offset_x, camera_offset_y);

    // Present
    SDL_RenderPresent(renderer);

    SDL_LogTrace(LOG_CAT_DISPLAY, "End draw_tile_map().");
    return SDL_APP_CONTINUE;
}

int load_textures(SDL_Renderer* renderer, TileDefinition** tile_definitions, uint8_t tile_definition_size){
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start load_textures()");
    int exit_status = SDL_APP_FAILURE;

    g_texture_map = hashmap_new(sizeof(TextureHashMapRecord), tile_definition_size + 1, 0, 0, texture_hash_map_hash, texture_hash_map_compare, NULL, NULL);    

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
        
        hashmap_set(g_texture_map, &(TextureHashMapRecord){ 
            .name = tile_definitions[tile_definition_index]->name,
            .texture = texture
        });
    }

    // Load additional textures
    SDL_Surface* bmp = SDL_LoadBMP("./img/Wood_Background.bmp");
    if (!bmp){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load %s.", "./img/Wood_Background.bmp");
        goto cleanup;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, bmp);
    SDL_DestroySurface(bmp);
    if (!texture){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not create texture for %s.", "./img/Wood_Background.bmp");
        goto cleanup;            
    }
    
    hashmap_set(g_texture_map, &(TextureHashMapRecord){ 
        .name = "Hex Grid Background",
        .texture = texture
    });
    
    exit_status = SDL_APP_CONTINUE;

cleanup: 
    if (exit_status != SDL_APP_CONTINUE){
        hashmap_free(g_texture_map);
    }
    SDL_LogTrace(LOG_CAT_DISPLAY, "End load_textures()");
    return exit_status;
}

// Update camera offsets
void update_camera() {
    const float scroll_speed = (HEX_RADIUS * 2) / 30.0f;
    const bool* keys = SDL_GetKeyboardState(NULL);

    // Update offsets based on arrow keys
    if (keys[SDL_SCANCODE_UP]) {
        camera_offset_y += scroll_speed;
    }
    if (keys[SDL_SCANCODE_DOWN]) {
        camera_offset_y -= scroll_speed;
    }
    if (keys[SDL_SCANCODE_LEFT]) {
        camera_offset_x += scroll_speed;
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        camera_offset_x -= scroll_speed;
    }

    // Calculate map boundaries
    float total_map_width = HEX_RADIUS * 2.0f * HEX_COUNT_X;
    float total_map_heigth = HEX_RADIUS * sqrtf(3.0f) * HEX_COUNT_Y;

    float hex_grid_min_x = -(WINDOW_WIDTH);
    float hex_grid_max_x = WINDOW_WIDTH / 64.0f;
    
    float hex_grid_min_y = -(total_map_heigth - WINDOW_HEIGHT * 0.75f);
    float hex_grid_max_y = WINDOW_HEIGHT * 0.5f;

    // Clamp offsets
    camera_offset_x = fminf(fmaxf(camera_offset_x, hex_grid_min_x), hex_grid_max_x);
    camera_offset_y = fminf(fmaxf(camera_offset_y, hex_grid_min_y), hex_grid_max_y);        
}


int texture_hash_map_compare(const void *a, const void *b, void *udata){
    const TextureHashMapRecord* ua = a;
    const TextureHashMapRecord* ub = b;
    return strcmp(ua->name, ub->name);
}

bool texture_hash_map_iter(const void *item, void *udata){
    const TextureHashMapRecord* rec = item;    
    return true;
}

uint64_t texture_hash_map_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const TextureHashMapRecord* rec = item;
    return hashmap_sip(rec->name, strlen(rec->name), seed0, seed1);
}

void clear_display_state(){
    hashmap_free(g_texture_map);
}
