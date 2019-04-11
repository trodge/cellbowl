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
#include "graph.h"

void save_hist(FILE *fp, History *now) {
    while (now) {
        int i;
        fprintf(fp, "1\n");
        fprintf(fp, "%lu %d", now->total_elapsed, now->num_cells);
        for (i = 0; i < NUM_TYPES; i++) {
            fprintf(fp, " %d", now->total_counts[i]);
        }
        for (i = 0; i < 3; i++) {
            fprintf(fp, " %" PRIu64, now->substances[i]);
        }
        fprintf(fp, "\n");
        now = now->past_point;
    }
    fprintf(fp, "0\n");
}

void load_hist(FILE *fp, History **now, History **oldest) {
    int i;
    int next_hist;
    fscanf(fp, "%d\n", &next_hist);
    if (next_hist) {
        *now = malloc(sizeof(History));
        (*now)->past_point = NULL;
        (*now)->future_point = NULL;
        fscanf(fp, "%lu %d", &(*now)->total_elapsed, &(*now)->num_cells);
        for (i = 0; i < NUM_TYPES; i++) {
            fscanf(fp, " %d", &(*now)->total_counts[i]);
        }
        for (i = 0; i < 3; i++) {
            fscanf(fp, " %" SCNu64, &(*now)->substances[i]);
        }
        fscanf(fp, "\n");
        fscanf(fp, "%d\n", &next_hist);
        *oldest = *now;
    }
    while (next_hist) {
        (*oldest)->past_point = malloc(sizeof(History));
        (*oldest)->past_point->future_point = *oldest;
        *oldest = (*oldest)->past_point;
        (*oldest)->past_point = NULL;
        fscanf(fp, "%lu %d", &(*oldest)->total_elapsed, &(*oldest)->num_cells);
        for (i = 0; i < NUM_TYPES; i++) {
            fscanf(fp, " %d", &(*oldest)->total_counts[i]);
        }
        for (i = 0; i < 3; i++) {
            fscanf(fp, " %" SCNu64, &(*oldest)->substances[i]);
        }
        fscanf(fp, "\n");
        fscanf(fp, "%d\n", &next_hist);
    }
}


void create_hist(History **now, unsigned long total_elapsed, int num_cells, int total_counts[NUM_TYPES],
		unsigned long long substances[3], History **oldest) {
    *now = malloc(sizeof(History));
    (*now)->future_point = NULL;
    (*now)->past_point = NULL;
    (*now)->total_elapsed = total_elapsed;
    (*now)->num_cells = num_cells;
    int i;
    for (i = 0; i < NUM_TYPES; i++) {
        (*now)->total_counts[i] = total_counts[i];
    }
    for (i = 0; i < 3; i++) {
        (*now)->substances[i] = substances[i];
    }
    *oldest = *now;
}

void free_hist(History *now, History *oldest) {
    now = now->past_point;
    while (now) {
        free(now->future_point);
        now = now->past_point;
    }
    free(oldest);
}

void update_hist(History **now, unsigned long total_elapsed, int num_cells, int total_counts[NUM_TYPES],
		unsigned long long substances[3], History **oldest) {
    if (total_elapsed >= (*now)->total_elapsed + HIST_UPDATE_INTERVAL) {
        (*now)->future_point = malloc(sizeof(History));
        (*now)->future_point->past_point = (*now);
        *now = (*now)->future_point;
        (*now)->future_point = NULL;
        (*now)->total_elapsed = total_elapsed;
        (*now)->num_cells = num_cells;
        int i;
        for (i = 0; i < NUM_TYPES; i++) {
            (*now)->total_counts[i] = total_counts[i];
        }
        for (i = 0; i < 3; i++) {
            (*now)->substances[i] = substances[i];
        }
    }
}

void draw_hist(SDL_Surface *s, History *now, int mode, History *oldest) {
    int i;
    while (now) {
    	// calculate the time elapsed between oldest and now
    	long current_elapsed = now->total_elapsed - oldest->total_elapsed;
        if (current_elapsed < HIST_LEN * HIST_UPDATE_INTERVAL && now != oldest) {
        	if (now->past_point) {
        		// if past point not null, draw line from past point to now
    			int xi = (now->past_point->total_elapsed - oldest->total_elapsed) / HIST_LEN * SCREEN_WIDTH / HIST_UPDATE_INTERVAL;
    			int xf = (now->total_elapsed - oldest->total_elapsed) / HIST_LEN * SCREEN_WIDTH / HIST_UPDATE_INTERVAL;
    			if (xi > xf) printf("Backward line %lu to %lu\nBackward Line %d to %d\n", now->past_point->total_elapsed - oldest->total_elapsed, now->total_elapsed - oldest->total_elapsed, xi, xf);
                switch (mode) {
                    case 1:
                        draw_line(s, xi,
                        		VIEW_HEIGHT - now->past_point->num_cells * VIEW_HEIGHT / MAX_CELLS,
                                xf,
                                VIEW_HEIGHT - now->num_cells * VIEW_HEIGHT / MAX_CELLS,
                                SDL_MapRGB(s->format, 255, 255, 255));
                        break;
                    case 2:
                        for (i = 0; i < NUM_TYPES; i++) {
                            draw_line(s, xi,
                            		VIEW_HEIGHT - now->past_point->total_counts[i] * VIEW_HEIGHT / MAX_CELLS / 12,
                                    xf,
                                    VIEW_HEIGHT - now->total_counts[i] * VIEW_HEIGHT / MAX_CELLS / 12,
                                    map_type_color(i, s->format));
                        }
                        break;
                    case 3:
                        for (i = 0; i < 3; i++) {
                            draw_line(s, xi,
                            		VIEW_HEIGHT - now->past_point->substances[i] / (SUBSTANCE_START * 3 / VIEW_HEIGHT),
                            		xf,
                            		VIEW_HEIGHT - now->substances[i] / (SUBSTANCE_START * 3 / VIEW_HEIGHT),
                                    map_type_color(i, s->format));
                        }
                        break;
                }
        	}
        	now = now->past_point; // set now to past_point even if NULL
        } else if (oldest->future_point) {
        	// move future point up if total duration is too long
            oldest = oldest->future_point;
        } else {
        	break;
        }
    }
}
