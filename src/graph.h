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
#ifndef HIST_H
#define HIST_H
#include <stdlib.h>
#include <inttypes.h>
#include <SDL2/SDL.h>

#include "draw.h"
#include "cell.h"

#define HIST_UPDATE_INTERVAL 5000
#define HIST_LEN 800

typedef struct History {
    struct History* past_point;
    struct History* future_point;
    unsigned long total_elapsed;
    int num_cells;
    int total_counts[NUM_TYPES];
    unsigned long long substances[3];
} History;

void save_hist(FILE *fp, History *now);
void load_hist(FILE *fp, History **now, History **oldest);
void create_hist(History **now, unsigned long total_elapsed, int num_cells, int total_counts[NUM_TYPES], unsigned long long substances[3], History **oldest);
void free_hist(History *now, History *oldest);
void update_hist(History **now, unsigned long total_elapsed, int num_cells, int total_counts[NUM_TYPES], unsigned long long substances[3], History **oldest);
void draw_hist(SDL_Surface *s, History *now, int mode, History *oldest);
#endif
