#include <stdlib.h> 
#include <time.h> 
#include <stdio.h> 
#include <math.h> 
#include <SDL3/SDL.h>

#include "../include/Globals.h"
#include "../include/Util.h"
#include "../include/Display.h"
#include "../include/Hashmap.h"

uint32_t frame_last_time = 0;
int frame_count = 0;
bool g_tile_selected = false;
CubeCoord g_curr_selected_coords = {0,0,0};

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
void draw_hexagon_outline(SDL_FPoint* vertices) {
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;
        SDL_RenderLine(
            g_renderer, 
            vertices[i].x, vertices[i].y,
            vertices[next].x, vertices[next].y
        );
    }
}

int draw_hexagon_filled(SDL_FColor color, SDL_FPoint points[6], float center_x, float center_y){    
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 0);

    // Define hexagon vertices and texture coordinates
    SDL_Vertex vertices[7];     

    vertices[0].position.x = center_x;
    // Center vertex
    vertices[0].position.y = center_y;
    vertices[0].color = color;

    // Outer vertices
    for (int i = 0; i < 6; i++) {
        vertices[i + 1].position = points[i];
        vertices[i + 1].color = color;
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

    if (SDL_RenderGeometry(g_renderer, NULL, vertices, 7, indices, 18) < 0) {
        SDL_LogError(LOG_CAT_DISPLAY, "RenderGeometry failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

int draw_hexagon_texture(SDL_Texture* texture, SDL_FPoint points[6], float center_x, float center_y){    
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 0);

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

    if (SDL_RenderGeometry(g_renderer, texture, vertices, 7, indices, 18) < 0) {
        SDL_LogError(LOG_CAT_DISPLAY, "RenderGeometry failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

int draw_tile_map(uint8_t map_hex_radius, SDL_FRect border){
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start draw_tile_map().");

    // Setup and Clear screen
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 0);
    SDL_RenderClear(g_renderer);

    // Draw Hex-Grid Background Texture
    const TextureHashMapRecord* hex_grid_background_rec = hashmap_get(g_texture_map, &(TextureHashMapRecord){.name="Hex Grid Background"});    
    
    SDL_RenderTextureTiled(g_renderer, hex_grid_background_rec->texture, NULL, 1.0f, &border);

    // Draw Hex Grid   
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start Draw %d hexes.", hashmap_count(g_game_map));
    
    size_t iter = 0;
    void* item;
    while (hashmap_iter(g_game_map, &iter, &item)) {
        const TileState* curr_state = item;

        // Validate game_map entry
        if (!curr_state || !curr_state->tile_def || !curr_state->tile_def->name) {
            SDL_LogError(LOG_CAT_DISPLAY, "Invalid tile at [%"PRId64"][%"PRId64"][%"PRId64"]: NULL tile_def or name", 
                curr_state->coord.pos_q, curr_state->coord.pos_r, curr_state->coord.pos_s
            );
            SDL_RenderPresent(g_renderer);
            return SDL_APP_FAILURE;
        }

        // Get Hex Texture by name
        char* hex_def_name = curr_state->tile_def->name;

        if (draw_hexagon(curr_state, hex_def_name) != SDL_APP_CONTINUE){            
            SDL_LogError(LOG_CAT_DISPLAY, "Error during draw_hexagon().");
            return SDL_APP_FAILURE;
        }
    }

    // Draw Box around grid    
    SDL_LogTrace(LOG_CAT_DISPLAY, "Draw Board Borders.");

    SDL_LogTrace(LOG_CAT_DISPLAY, "End draw_tile_map().");
    return SDL_APP_CONTINUE;
}

int draw_hexagon(const TileState* curr_state, char* hex_def_name){
    SDL_FPoint top_left_menu = {
        .x = WINDOW_WIDTH / 8.0f,
        .y = WINDOW_HEIGHT / 8.0f
    };

    SDL_FPoint camera_offset = {
        .x = camera_offset_x,
        .y = camera_offset_y
    };
    
    // Calculate Hex Texture center (flat-top odd-q hex grid)        
    SDL_FPoint center = flat_top_hex_to_pixel(curr_state->coord, HEX_RADIUS, top_left_menu, camera_offset);
    
    SDL_LogTrace(LOG_CAT_DISPLAY, "Calculated Hex Center: X: %04.02f Y: %04.02f for [%d][%d][%d].", 
        center.x, center.y,
        curr_state->coord.pos_q, curr_state->coord.pos_r, curr_state->coord.pos_s
    );

    // Skip off-screen hexagons
    float min_render_px_x = HEX_RADIUS + ((WINDOW_WIDTH / 8.0f) * 0.5f);
    float max_render_px_x = WINDOW_WIDTH + HEX_RADIUS;

    float min_render_px_y = (-(HEX_RADIUS + (WINDOW_HEIGHT / 8.0f)) * 0.4f);
    float max_render_px_y = WINDOW_HEIGHT + HEX_RADIUS;
    
    if (center.x < min_render_px_x || center.x > max_render_px_x ||
        center.y < -min_render_px_y || center.y > max_render_px_y) {
        return SDL_APP_CONTINUE;
    }

    // Calc Hex vertices
    SDL_FPoint points[6];
    get_hexagon_vertices(points, center.x, center.y, HEX_RADIUS);

    //Determine Biome color
    int curr_biome = curr_state->tile_biome;
    RgbColor biome_color = get_biome_color(curr_biome - 1);
    SDL_FColor color = {
        .r = biome_color.r / 255.0f, 
        .g = biome_color.g / 255.0f, 
        .b = biome_color.b / 255.0f, 
        .a = biome_color.a / 255.0f
    };
   
    // Draw solid color Hex
    if (draw_hexagon_filled(color, points, center.x, center.y) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Error during draw_hexagon_filled: %s.", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Draw Textured Hex
    /*    
    SDL_LogTrace(LOG_CAT_DISPLAY, "Hex def name: %s.", hex_def_name);
    const TextureHashMapRecord* rec = hashmap_get(g_texture_map, &(TextureHashMapRecord){.name=hex_def_name});
    if (!rec || !rec->texture) {
        SDL_LogError(LOG_CAT_DISPLAY, "No texture found for name '%s'", hex_def_name);
        SDL_RenderPresent(renderer);
        return SDL_APP_FAILURE;
    }            
    SDL_Texture* texture = rec->texture;    
    if (draw_hexagon_texture(renderer, texture, points, center.x, center.y) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Error during draw_hexagon_texture: %s.", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    */

    // Draw hex outline    
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255); // Black Outline
    draw_hexagon_outline(points);

    // Draw Rivers
    if (curr_state->has_river == true){
        SDL_SetRenderDrawColor(g_renderer, 153, 153, 255, 255);
        if (!is_same_coord(&curr_state->river_source, &curr_state->coord)){
            SDL_FPoint river_source_center = flat_top_hex_to_pixel(curr_state->river_source, HEX_RADIUS, top_left_menu, camera_offset);
            SDL_RenderLine(g_renderer, river_source_center.x, river_source_center.y, center.x, center.y);
        } else {
            SDL_FRect rect = {
                .x = center.x - HEX_RADIUS / 8.0f,
                .y = center.y - HEX_RADIUS / 8.0f,
                .w = HEX_RADIUS / 4.0f,
                .h = HEX_RADIUS / 4.0f
            };
            SDL_RenderFillRect(g_renderer, &rect);
        }

        if (!is_same_coord(&curr_state->river_destination, &curr_state->coord)){
            SDL_FPoint river_dest_center = flat_top_hex_to_pixel(curr_state->river_destination, HEX_RADIUS, top_left_menu, camera_offset);        
            SDL_RenderLine(g_renderer, center.x, center.y, river_dest_center.x, river_dest_center.y);
        } else {
            SDL_FRect rect = {
                .x = center.x - HEX_RADIUS / 16.0f,
                .y = center.y - HEX_RADIUS / 16.0f,
                .w = HEX_RADIUS / 8.0f,
                .h = HEX_RADIUS / 8.0f
            };
            SDL_RenderFillRect(g_renderer, &rect);
        }                
    }

    // DEBUG INFO Coordinates
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderDebugTextFormat(g_renderer, center.x - (HEX_RADIUS/1.5f), center.y - 12, "[%d][%d][%d]", 
        curr_state->coord.pos_q, curr_state->coord.pos_r, curr_state->coord.pos_s
    );

    // Draw Red inner Hex for selected tiles
    if (g_tile_selected && is_same_coord(&curr_state->coord, &g_curr_selected_coords)){
        get_hexagon_vertices(points, center.x, center.y, HEX_RADIUS - 5.0f);
        SDL_SetRenderDrawColor(g_renderer, 255, 0, 0, 255); // Red outline
        draw_hexagon_outline(points);
    }

    return SDL_APP_CONTINUE;
}


int load_textures(TileDefinition** tile_definitions, uint8_t tile_definition_size){
    SDL_LogTrace(LOG_CAT_DISPLAY, "Start load_textures()");
    int exit_status = SDL_APP_FAILURE;

    g_texture_map = hashmap_new(sizeof(TextureHashMapRecord), tile_definition_size + 1, 0, 0, texture_hash_map_hash, texture_hash_map_compare, NULL, NULL);

    uint8_t tile_definition_index = 0;
    for (tile_definition_index = 0; tile_definition_index < tile_definition_size; tile_definition_index++){
        if(init_and_add_texture(tile_definitions[tile_definition_index]->name, tile_definitions[tile_definition_index]->texture) != SDL_APP_CONTINUE){
            goto cleanup;
        }
    }

    // Load additional textures
    if(init_and_add_texture("Hex Grid Background", "./img/Texturelabs_Paper_251L.bmp") != SDL_APP_CONTINUE){
        goto cleanup;
    }
    
    if(init_and_add_texture("Menu Background", "./img/Texturelabs_Paper_319M.bmp") != SDL_APP_CONTINUE){
        goto cleanup;
    }

    exit_status = SDL_APP_CONTINUE;

cleanup: 
    if (exit_status != SDL_APP_CONTINUE){
        hashmap_free(g_texture_map);
    }
    SDL_LogTrace(LOG_CAT_DISPLAY, "End load_textures()");
    return exit_status;
}

int init_and_add_texture(char* texture_id, char* texture_filename){
    SDL_Surface* bmp = SDL_LoadBMP(texture_filename);
    if (!bmp){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not load %s.", texture_filename);
        return SDL_APP_FAILURE;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, bmp);
    SDL_DestroySurface(bmp);
    if (!texture){
        SDL_LogError(LOG_CAT_DISPLAY, "Could not create texture for %s.", texture_filename);
        return SDL_APP_FAILURE;
    }
    
    hashmap_set(g_texture_map, &(TextureHashMapRecord){ 
        .name = texture_id,
        .texture = texture
    });

    return SDL_APP_CONTINUE;
}

void handle_left_click(){
    SDL_FPoint top_left_menu = {
        .x = WINDOW_WIDTH / 8.0f,
        .y = WINDOW_HEIGHT / 8.0f
    };
    SDL_FPoint camera_offset = {
        .x = camera_offset_x,
        .y = camera_offset_y
    };
    SDL_FPoint click = { 
        g_mouse_pos_x,
        g_mouse_pos_y
    };
    SDL_LogDebug(LOG_CAT_DISPLAY, "Clicked Grid (adjusted by camera offset) X: %04.02f Y: %04.02f", click.x, click.y);

    CubeCoord coords = flat_top_pixel_to_hex(click, HEX_RADIUS, top_left_menu, camera_offset);    
    const TileState* tile_state = hashmap_get(g_game_map, &(TileState){.coord = coords});
    if (!tile_state) {
        SDL_LogError(LOG_CAT_DISPLAY, "No Tile State found for coords [%"PRId64"][%"PRId64"][%"PRId64"]", 
            coords.pos_q, coords.pos_r, coords.pos_s
        );
        g_tile_selected = false;
        g_curr_selected_coords = (CubeCoord){0,0,0};
        return;
    }

    if (g_tile_selected && is_same_coord(&g_curr_selected_coords, &coords)){
        g_tile_selected = false;        
        g_curr_selected_coords = (CubeCoord){0,0,0};
    } else {
        g_tile_selected = true;        
        g_curr_selected_coords = coords;
    }

    log_tile_state_string(tile_state);
}

int frame_update(float delta_time) {
    // Update camera    
    const float scroll_speed = (WINDOW_WIDTH / (128 + 64)) * delta_time * 60.0f;
    const bool* keys = SDL_GetKeyboardState(NULL);

    // Update offsets based on arrow keys
    if (keys[SDL_SCANCODE_UP]) {
        camera_offset_y -= scroll_speed;
    }
    if (keys[SDL_SCANCODE_DOWN]) {
        camera_offset_y += scroll_speed;
    }
    if (keys[SDL_SCANCODE_LEFT]) {
        camera_offset_x -= scroll_speed;
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        camera_offset_x += scroll_speed;
    }

    // Calculate map boundaries
    float half_map_width = HEX_RADIUS * 2.0f * HEX_GRID_RADIUS;
    float half_map_heigth = HEX_RADIUS * sqrtf(3.0f) * HEX_GRID_RADIUS;

    SDL_FPoint top_left_menu = {
        .x = WINDOW_WIDTH / 8.0f,
        .y = WINDOW_HEIGHT / 8.0f
    };

    SDL_FRect grid_border = {
        .x = top_left_menu.x,
        .y = top_left_menu.y,
        .w = WINDOW_WIDTH - (WINDOW_WIDTH / 16.0f),
        .h = WINDOW_HEIGHT - (WINDOW_HEIGHT / 16.0f)
    };

    float hex_grid_min_x = -half_map_width + top_left_menu.x;
    float hex_grid_max_x = half_map_width / 4.0f;
    
    float hex_grid_min_y = (-half_map_heigth) - top_left_menu.y;
    float hex_grid_max_y = half_map_heigth / 1.5f;

    // Clamp offsets
    camera_offset_x = fminf(fmaxf(camera_offset_x, hex_grid_min_x), hex_grid_max_x);
    camera_offset_y = fminf(fmaxf(camera_offset_y, hex_grid_min_y), hex_grid_max_y);        

    /* Draw Map*/ 
    if(draw_tile_map(HEX_GRID_RADIUS, grid_border) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Error while drawing tile map: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Draw Menu
    if(draw_menu(grid_border) != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Error while drawing menu: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // DEBUG INFO
    if(draw_debug_info() != SDL_APP_CONTINUE){
        SDL_LogError(LOG_CAT_MAIN, "Error while drawing debug_info: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Present
    SDL_RenderPresent(g_renderer);

    return SDL_APP_CONTINUE;
}

int draw_debug_info(){
    SDL_RenderDebugTextFormat(g_renderer, 0, 0, "Camera Offset: X: %04.02f, Y: %04.02f", camera_offset_x, camera_offset_y);
    SDL_RenderDebugTextFormat(g_renderer, 0, 12, "Screen Mouse Pos: X: %04.02f, Y: %04.02f", g_mouse_pos_x, g_mouse_pos_y);    

    frame_count++;
    uint32_t now = SDL_GetTicks();
    if (now > frame_last_time + 1000) {        
        frame_count = 0;
        frame_last_time = now;
    }
    SDL_RenderDebugTextFormat(g_renderer, 0, 36, "FPS: %d", frame_count);

    return SDL_APP_CONTINUE;
}

int draw_menu(SDL_FRect border){
    SDL_FRect horizontal_menu = {
        .x = 0,
        .y = 0,
        .w = WINDOW_WIDTH,
        .h = WINDOW_HEIGHT / 8.0f
    };

    SDL_FRect vertical_menu = {
        .x = 0,
        .y = WINDOW_HEIGHT / 8.0f,
        .w = WINDOW_WIDTH / 8.0f,
        .h = WINDOW_HEIGHT - (WINDOW_HEIGHT / 8.0f)
    };

    const TextureHashMapRecord* menu_background_rec = hashmap_get(g_texture_map, &(TextureHashMapRecord){.name="Menu Background"});       
    SDL_RenderTexture(g_renderer, menu_background_rec->texture, NULL, &horizontal_menu);    
    SDL_RenderTexture(g_renderer, menu_background_rec->texture, NULL, &vertical_menu);

    SDL_SetRenderDrawColor(g_renderer, 0, 255, 0, 255); // GREEN
    SDL_RenderRect(g_renderer, &border);

    return SDL_APP_CONTINUE;
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
