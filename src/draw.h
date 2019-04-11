/*  This file is part of Cellbowl.

    Cellbowl is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cellbowl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cellbowl.  If not, see <https://www.gnu.org/licenses/>. 
    
    © Tom Rodgers 2010-2019
*/
#ifndef DRAW_H
#define DRAW_H

#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "constants.h"

void draw_text(SDL_Surface *s, TTF_Font *font, int x, int y, int x_align, int y_align, SDL_Color color, char *fmt, ...);
void draw_circle(SDL_Surface *s, int cx, int cy, int r, Uint32 color);
void draw_circle_symmetry_points(SDL_Surface *s, int cx, int cy, int x, int y, Uint32 color, int check_bounds);
void draw_line(SDL_Surface *s, int xi, int yi, int xf, int yf, Uint32 color);
void draw_pixel(SDL_Surface *s, int x, int y, Uint32 color);

#endif
