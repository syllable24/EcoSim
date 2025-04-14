#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#ifndef DISPLAY_H
#define DISPLAY_H

void get_hexagon_vertices(SDL_FPoint* points, float center_x, float center_y, float radius);

void draw_hexagon_outline(SDL_Renderer* renderer, SDL_FPoint* vertices);

int draw_hexagon_texture(SDL_Renderer* renderer, SDL_Texture* texture, SDL_FPoint points[6], float center_x, float center_y);

#endif