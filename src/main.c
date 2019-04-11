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
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <inttypes.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "cell.h"
#include "graph.h"
#include "draw.h"
#include "constants.h"

#define SCROLL_SPEED 1024
#define LIQUID_SCROLL 5
#define NUM_HIST_MODES 3

void draw_hud(SDL_Surface *s, TTF_Font *font, SDL_Color text_color, SDL_Rect view,
		unsigned long total_elapsed, unsigned long long substances[3],
        Cell *cells, int num_cells, Cell *selected_cell, int selected_state,
        int hud_update, int ms_since_last_update, int frames_since_last_update) {
    int i;
    SDL_Rect r;
    // draw a black rectangle over the mini-map
    r.x = 0;
    r.y = 0;
    r.w = (AREA_WIDTH * HUD_HEIGHT + AREA_HEIGHT - 1) / AREA_HEIGHT;
    r.h = HUD_HEIGHT;
    SDL_FillRect(s, &r, SDL_MapRGB(s->format, 0, 0, 0));
    // draw grey rectangle over currently viewed area on mini-map
    r.x = view.x * HUD_HEIGHT / AREA_HEIGHT;
    r.y = view.y * HUD_HEIGHT / AREA_HEIGHT;
    r.w = (view.w * HUD_HEIGHT + AREA_HEIGHT - 1) / AREA_HEIGHT;
    r.h = (view.h * HUD_HEIGHT + AREA_HEIGHT - 1) / AREA_HEIGHT;
    SDL_FillRect(s, &r, SDL_MapRGB(s->format, 32, 32, 32));
    // draw grey lines defining regions grid on mini-map
    for (i = 1; i < X_REGIONS; i++) {
        r.x = i * AREA_WIDTH / X_REGIONS * HUD_HEIGHT / AREA_HEIGHT;
        r.y = 0;
        r.w = 1;
        r.h = HUD_HEIGHT;
        SDL_FillRect(s, &r, SDL_MapRGB(s->format, 32, 32, 32));
    }
    for (i = 1; i < Y_REGIONS; i++) {
        r.x = 0;
        r.y = i * AREA_HEIGHT / Y_REGIONS * HUD_HEIGHT / AREA_HEIGHT;
        r.w = (AREA_WIDTH * HUD_HEIGHT + AREA_HEIGHT - 1) / AREA_HEIGHT;
        r.h = 1;
        SDL_FillRect(s, &r, SDL_MapRGB(s->format, 32, 32, 32));
    }
    //SDL_LockSurface(s);
    // Draw circles representing cells on minimap
    for (i = 0; i < num_cells; i++) {
       if (cells[i].state) {
            draw_circle(s, cells[i].x * HUD_HEIGHT / AREA_HEIGHT, cells[i].y * HUD_HEIGHT / AREA_HEIGHT,
                    energy_scale(3, cells[i].e), map_state_color(cells[i].state, s->format));
        } else {
            draw_circle(s, cells[i].x * HUD_HEIGHT / AREA_HEIGHT, cells[i].y * HUD_HEIGHT / AREA_HEIGHT,
                    energy_scale(3, cells[i].e), map_type_color(cells[i].primary_type, s->format));
        }
        if (cells[i].virus) {
        	// Draw crosses representing infecting virus
            r.x = cells[i].x * HUD_HEIGHT / AREA_HEIGHT - energy_scale(3, cells[i].e);
            r.y = cells[i].y * HUD_HEIGHT / AREA_HEIGHT;
            r.w = energy_scale(6, cells[i].e);
            r.h = 1;
            SDL_FillRect(s, &r, map_type_color(cells[i].virus->primary_type, s->format));
            r.x = cells[i].x * HUD_HEIGHT / AREA_HEIGHT;
            r.y = cells[i].y * HUD_HEIGHT / AREA_HEIGHT - energy_scale(3, cells[i].e);
            r.w = 1;
            r.h = energy_scale(6, cells[i].e);
            SDL_FillRect(s, &r, map_type_color(cells[i].virus->primary_type, s->format));
        }
    }
    if (selected_cell) {
        draw_circle(s, selected_cell->x * HUD_HEIGHT / AREA_HEIGHT, selected_cell->y * HUD_HEIGHT / AREA_HEIGHT,
                energy_scale(3, selected_cell->e) + 1, SDL_MapRGB(s->format, 255, 255, 255));
    }
    // draw the line between mini-map and HUD
    r.x = (AREA_WIDTH * HUD_HEIGHT + AREA_HEIGHT - 1) / AREA_HEIGHT;
    r.y = 0;
    r.h = HUD_HEIGHT;
    r.w = 1;
    SDL_FillRect(s, &r, SDL_MapRGB(s->format, 83, 93, 108));
    // fill in the region to the right of mini-map
    r.x += 1;
    r.w = 4;
    SDL_FillRect(s, &r, SDL_MapRGB(s->format, 0, 0, 0));
    if (hud_update || ms_since_last_update > 1000) {
    	r.x += 4;
        r.w = SCREEN_WIDTH - r.x;
        SDL_FillRect(s, &r, SDL_MapRGB(s->format, 0, 0, 0));
        if (selected_cell) {
            draw_text(s, font, r.x + 48, 3, 0, -1, text_color, "Cell Info");
            for (i = 0; i < (NUM_TYPES + 1) / 2; i++) {
                draw_text(s, font, r.x + 6, 31 + 14*i, -1, -1, get_type_color(i), "%2d", selected_cell->type_counts[i]);
            }
            for (i = (NUM_TYPES + 1) / 2; i < NUM_TYPES; i++) {
                draw_text(s, font, r.x + 84, 31 + 14*(i - (NUM_TYPES + 1) / 2), 1, -1, get_type_color(i), "%2d", selected_cell->type_counts[i]);
            }
            draw_text(s, font, r.x + 6, HUD_HEIGHT - 4, -1, 1, text_color, "Energy:%5ld", selected_cell->e / 1000);
            draw_text(s, font, r.x + 6, HUD_HEIGHT - 18, -1, 1, text_color, "Age:%8d", selected_cell->age / 1000);
            draw_text(s, font, r.x + 6, HUD_HEIGHT - 32, -1, 1, text_color, "Weight:%5d", selected_cell->weight);
            int cell_display_x = ((r.x + 92) * 3 + SCREEN_WIDTH - 260) / 4;
            set_organelle_loc(selected_cell->organelles, 0, 0, -selected_cell->organelles->r, 0, 1000000); 
            for (i = 0; i < selected_cell->num_organelles; i++) {
                draw_circle(s, cell_display_x + selected_cell->organelles[i].x, HUD_HEIGHT / 2 + selected_cell->organelles[i].y,
                        selected_cell->organelles[i].r, map_type_color(selected_cell->organelles[i].type, s->format));
            }
            if (selected_cell->virus) {
                draw_text(s, font, SCREEN_WIDTH - 210, 3, 0, -1, text_color, "Virus Info");
                for (i = 0; i < (NUM_TYPES + 1) / 2; i++) {
                    draw_text(s, font, SCREEN_WIDTH - 246, 31 + 14*i, -1, -1, get_type_color(i), "%2d", selected_cell->virus->type_counts[i]);
                }
                for (i = (NUM_TYPES + 1) / 2; i < NUM_TYPES; i++) {
                    draw_text(s, font, SCREEN_WIDTH - 168, 31 + 14*(i - (NUM_TYPES + 1) / 2), 1, -1, get_type_color(i), "%2d", selected_cell->virus->type_counts[i]);
                }
                draw_text(s, font, SCREEN_WIDTH - 168, HUD_HEIGHT - 4, 1, 1, text_color, "Weight:%5d", selected_cell->virus->weight);
                int virus_display_x = (r.x + 92 + (SCREEN_WIDTH - 260) * 3) / 4;
                for (i = 0; i < selected_cell->virus->num_organelles; i++) {
                    draw_circle(s, virus_display_x + selected_cell->virus->organelles[i].x, HUD_HEIGHT / 2 + selected_cell->virus->organelles[i].y,
                            selected_cell->virus->organelles[i].r, map_type_color(selected_cell->virus->organelles[i].type, s->format));
                }
            }
        }
        for (i = 0; i < 3; i++) {
            draw_text(s, font, SCREEN_WIDTH - 6, 4 + 14*i, 1, -1, get_type_color(i),
                    "Substance %1d:%8lu", i, substances[i] / 10000);
        }
        draw_text(s, font, SCREEN_WIDTH - 6, 60, 1, -1, text_color, "Cells:%4d", num_cells);
        unsigned long energy_sum = 0;
        for (i = 0; i < 3; i++) {
            energy_sum += substances[i];
        }
        for (i = 0; i < num_cells; i++) {
            energy_sum += cells[i].e;
        }
        draw_text(s, font, SCREEN_WIDTH - 6, 74, 1, -1, text_color, "Energy Sum:%8ld", energy_sum / 1000);
        draw_text(s, font, SCREEN_WIDTH - 6, 102, 1, -1, text_color, "Selected State:%2d", selected_state);
        draw_text(s, font, SCREEN_WIDTH - 6, 116, 1, -1, text_color, "Time: %4lu:%02lu:%02lu",
                total_elapsed / 3600000, total_elapsed % 3600000 / 60000, total_elapsed % 60000 / 1000);
        if (ms_since_last_update) {
            draw_text(s, font, SCREEN_WIDTH - 6, HUD_HEIGHT - 4, 1, 1, text_color,
                    "FPS:%4d", 1000 * frames_since_last_update / ms_since_last_update);
        }
    }
}

void assign_cells_to_regions(Cell *cells, int num_cells, Cell **cells_in_regions[][Y_REGIONS],
        int num_cells_in_regions[][Y_REGIONS], int cells_allocated_in_regions[][Y_REGIONS]) {
    int i, j, k;

    // reset count of cells in regions
    for (i = 0; i < X_REGIONS; i++) {
        for (j = 0; j < Y_REGIONS; j++) {
            num_cells_in_regions[i][j] = 0;
        }
    }

    // count cells in regions, allocate space for cell pointers, add pointers
    for (i = 0; i < num_cells; i++) {
        int right_region = (cells[i].x + energy_scale(cells[i].r, cells[i].e) - 1) / (AREA_WIDTH / X_REGIONS);
        int bottom_region = (cells[i].y + energy_scale(cells[i].r, cells[i].e) - 1) / (AREA_HEIGHT / Y_REGIONS);
        int left_region = (cells[i].x - energy_scale(cells[i].r, cells[i].e)) / (AREA_WIDTH / X_REGIONS);
        int top_region = (cells[i].y - energy_scale(cells[i].r, cells[i].e)) / (AREA_HEIGHT / Y_REGIONS);
        if (right_region >= X_REGIONS) {
            right_region = X_REGIONS - 1;
        }
        if (bottom_region >= Y_REGIONS) {
            bottom_region = Y_REGIONS - 1;
        }
        for (j = left_region; j <= right_region; j++) {
            for (k = top_region; k <= bottom_region; k++) {
                num_cells_in_regions[j][k] += 1;
                if (num_cells_in_regions[j][k] > cells_allocated_in_regions[j][k]) {
                    cells_allocated_in_regions[j][k] += num_cells_in_regions[j][k] * 4;
                    cells_in_regions[j][k] = realloc(cells_in_regions[j][k],
                        cells_allocated_in_regions[j][k] * sizeof(*cells_in_regions[j][k]));
                }
                cells_in_regions[j][k][num_cells_in_regions[j][k] - 1] = &cells[i];
            }
        }
    }

    // deallocate extra space for pointers to cells in regions
    for (i = 0; i < X_REGIONS; i++) {
        for (j = 0; j < Y_REGIONS; j++) {
             if (num_cells_in_regions[i][j] * 8 < cells_allocated_in_regions[i][j]) {
                cells_allocated_in_regions[i][j] = num_cells_in_regions[i][j];
                cells_in_regions[i][j] = realloc(cells_in_regions[i][j],
                        cells_allocated_in_regions[i][j] * sizeof(*cells_in_regions[i][j]));
            }
        }
    }
}

void save_state(int slot_num, unsigned long total_elapsed, unsigned long long substances[3], Cell cells[MAX_CELLS], int num_cells,
        History *now) {
    int i;
    FILE *fp;
    char filename[6];
    sprintf(filename, "state%1d", slot_num);
    fp = fopen(filename, "w");
    fprintf(fp, "%ld\n", total_elapsed);
    for (i = 0; i < 3; i++) {
        fprintf(fp, "%" SCNu64 "\n", substances[i]);
    }
    fprintf(fp, "%d\n", num_cells);
    for (i = 0; i < num_cells; i++) {
        save_cell(fp, cells + i);
        if (cells[i].virus) {
            fprintf(fp, "1\n");
            save_cell(fp, cells[i].virus);
        } else {
            fprintf(fp, "0\n");
        }
    }
    save_hist(fp, now);
    fclose(fp);
}

void load_state(int slot_num, unsigned long *total_elapsed, unsigned long long substances[3], Cell cells[MAX_CELLS], int *num_cells,
        History **now, History **oldest) {
    int i;
    FILE *fp;
    char filename[6];
    sprintf(filename, "state%1d", slot_num);
    fp = fopen(filename, "r");
    if (fp) {
        for (i = 0; i < *num_cells; i++) {
            free_cell(cells + i);
        }
        free_hist(*now, *oldest);
        fscanf(fp, "%lu\n", total_elapsed);
        for (i = 0; i < 3; i++) {
            fscanf(fp, "%" SCNu64 "\n", substances + i);
        }
        fscanf(fp, "%d\n", num_cells);
        for (i = 0; i < *num_cells; i++) {
            load_cell(fp, cells + i);
            set_secondary_variables(cells + i);
            int cell_infected;
            fscanf(fp, "%d\n", &cell_infected);
            if (cell_infected) {
                cells[i].virus = malloc(sizeof(Cell));
                load_cell(fp, cells[i].virus);
                set_secondary_variables(cells[i].virus);
            }
        }
        load_hist(fp, now, oldest);
        fclose(fp);
    }
}

void handle_events(int *done, int *view_x_vel, int *view_y_vel, int *view_x_goal, int *view_y_goal,
        int *view_drag, SDL_Rect view, unsigned long *total_elapsed, unsigned long long substances[3],
        Cell cells[MAX_CELLS], int *num_cells,
        Cell **cells_in_regions[][Y_REGIONS], int num_cells_in_regions[][Y_REGIONS],
        Cell **selected_cell, int *cell_drag,
        int *hist_mode, History **now, History **oldest,
        int *selected_state, int *hud_update) {
    int i, j;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        *view_x_vel = -SCROLL_SPEED;
                        break;
                    case SDLK_UP:
                        *view_y_vel = -SCROLL_SPEED;
                        break;
                    case SDLK_RIGHT:
                        *view_x_vel = SCROLL_SPEED;
                        break;
                    case SDLK_DOWN:
                        *view_y_vel = SCROLL_SPEED;
                        break;
                    case SDLK_DELETE:
                        if (*selected_cell && !(*selected_cell)->state) {
                            (*selected_cell)->state = -1;
                        }
                        break;
                    case SDLK_j:
                        if (*hist_mode) {
                            (*hist_mode)--;
                        } else {
                            (*hist_mode) = NUM_HIST_MODES;
                        }
                        break;
                    case SDLK_k:
                        if (*hist_mode < NUM_HIST_MODES) {
                            (*hist_mode)++;
                        } else {
                            (*hist_mode) = 0;
                        }
                        break;
                    case SDLK_0:
                        *selected_state = 0;
                        *hud_update = 1;
                        break;
                    case SDLK_1:
                        *selected_state = 1;
                        *hud_update = 1;
                        break;
                    case SDLK_2:
                        *selected_state = 2;
                        *hud_update = 1;
                        break;
                    case SDLK_3:
                        *selected_state = 3;
                        *hud_update = 1;
                        break;
                    case SDLK_4:
                        *selected_state = 4;
                        *hud_update = 1;
                        break;
                    case SDLK_5:
                        *selected_state = 5;
                        *hud_update = 1;
                        break;
                    case SDLK_6:
                        *selected_state = 6;
                        *hud_update = 1;
                        break;
                    case SDLK_7:
                        *selected_state = 7;
                        *hud_update = 1;
                        break;
                    case SDLK_8:
                        *selected_state = 8;
                        *hud_update = 1;
                        break;
                    case SDLK_9:
                        *selected_state = 9;
                        *hud_update = 1;
                        break;
                    case SDLK_r:
                        for (i = 0; i < *num_cells; i++) {
                            free_cell(cells + i);
                        }
                        free_hist(*now, *oldest);
                        *total_elapsed = 0;
                        for (i = 0; i < 3; i++) {
                            substances[i] = SUBSTANCE_START;
                        }
                        *num_cells = (AREA_WIDTH / CELL_SPACE) * (AREA_HEIGHT / CELL_SPACE);
                        add_initial_cells(cells);
                        int total_counts[NUM_TYPES];
                        for (i = 0; i < NUM_TYPES; i++) {
                            total_counts[i] = 0;
                            for (j = 0; j < *num_cells; j++) {
                                total_counts[i] += cells[j].type_counts[i];
                            }
                        }
                        create_hist(now, *total_elapsed, *num_cells, total_counts, substances, oldest);
                        *selected_cell = NULL;
                        *hud_update = 1;
                        break;
                    case SDLK_s:
                        save_state(*selected_state, *total_elapsed, substances, cells, *num_cells, *now);
                        break;
                    case SDLK_f:
                        load_state(*selected_state, total_elapsed, substances, cells, num_cells, now, oldest);
                        *selected_cell = NULL;
                        *hud_update = 1;
                        break;
                    case SDLK_ESCAPE:
                        *done = 1;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        *view_x_vel = 0;
                        break;
                    case SDLK_UP:
                        *view_y_vel = 0;
                        break;
                    case SDLK_RIGHT:
                        *view_x_vel = 0;
                        break;
                    case SDLK_DOWN:
                        *view_y_vel = 0;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.y <= view.h) {
                    int clicked_area_x = event.button.x + view.x;
                    int clicked_area_y = event.button.y + view.y;
                    int clicked_region_x = clicked_area_x * X_REGIONS / AREA_WIDTH;
                    int clicked_region_y = clicked_area_y * Y_REGIONS / AREA_HEIGHT;
                    int found_one = 0;
                    int i;
                    for (i = 0; i < num_cells_in_regions[clicked_region_x][clicked_region_y]; i++) {
                        int dx = clicked_area_x - cells_in_regions[clicked_region_x][clicked_region_y][i]->x;
                        int dy = clicked_area_y - cells_in_regions[clicked_region_x][clicked_region_y][i]->y;
                        int cell_r = energy_scale(cells_in_regions[clicked_region_x][clicked_region_y][i]->r,
                                cells_in_regions[clicked_region_x][clicked_region_y][i]->e);
                        if (dx * dx + dy * dy < cell_r * cell_r) {
                            if (*selected_cell == cells_in_regions[clicked_region_x][clicked_region_y][i]) {
                                *cell_drag = 1;
                                (*selected_cell)->pause_motion = 1;
                            }
                            *selected_cell = cells_in_regions[clicked_region_x][clicked_region_y][i];
                            found_one = 1;
                        }
                    }
                    if (!found_one) {
                        *selected_cell = NULL;
                    }
                    *hud_update = 1;
                } else if (event.button.x < (AREA_WIDTH * HUD_HEIGHT + AREA_HEIGHT - 1) / AREA_HEIGHT) {
                    *view_x_goal = event.button.x * AREA_HEIGHT / HUD_HEIGHT;
                    *view_y_goal = (event.button.y - view.h) * AREA_HEIGHT / HUD_HEIGHT;
                    *view_drag = 1;
                }
                break;
            case SDL_MOUSEMOTION:
                if (*selected_cell && *cell_drag && event.motion.y <= view.h) {
                    (*selected_cell)->x = event.motion.x + view.x;
                    (*selected_cell)->y = event.motion.y + view.y;
                } else if (*view_drag && event.motion.x < (AREA_WIDTH * HUD_HEIGHT + AREA_HEIGHT - 1) / AREA_HEIGHT &&
                        event.motion.y > view.h) {
                    *view_x_goal = event.motion.x * AREA_HEIGHT / HUD_HEIGHT;
                    *view_y_goal = (event.motion.y - view.h) * AREA_HEIGHT / HUD_HEIGHT;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                *cell_drag = 0;
                if (*selected_cell) {
                    (*selected_cell)->pause_motion = 0;
                }
                *view_drag = 0;
                break;
            case SDL_QUIT:
                *done = 1;
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    int i, j;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Cell Bowl",
    		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    // create surface and texture to draw to before window
    SDL_Surface *screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH,
    		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Rect view; // rectangle representing current position
    view.w = SCREEN_WIDTH;
    view.h = SCREEN_HEIGHT - HUD_HEIGHT;
    view.x = AREA_WIDTH / 2 - view.w / 2;
    view.y = AREA_HEIGHT / 2 - view.h / 2;
    int view_x_vel = 0;
    int view_y_vel = 0;
    int view_x_goal = AREA_WIDTH / 2;
    int view_y_goal = AREA_HEIGHT / 2;
    int view_drag = 0;
    TTF_Init();
    TTF_Font *font;
    font = TTF_OpenFont("Terminus.ttf", 14);
    SDL_Color text_color = {192, 192, 192};

    int selected_state = 0;
    unsigned long total_elapsed = 0;

    srand(time(NULL));

    unsigned long long substances[3];
    for (i = 0; i < 3; i++) {
        substances[i] = SUBSTANCE_START;
    }

    int num_cells = (AREA_WIDTH / CELL_SPACE) * (AREA_HEIGHT / CELL_SPACE);
    Cell cells[MAX_CELLS];
    add_initial_cells(cells);
    Cell **cells_in_regions[X_REGIONS][Y_REGIONS];
    int num_cells_in_regions[X_REGIONS][Y_REGIONS];
    int cells_allocated_in_regions[X_REGIONS][Y_REGIONS];
    for (i = 0; i < X_REGIONS; i++) {
        for (j = 0; j < Y_REGIONS; j++) {
            cells_allocated_in_regions[i][j] = num_cells / (X_REGIONS * Y_REGIONS) * 3;
            cells_in_regions[i][j] = malloc(sizeof(*cells_in_regions[i][j]) * cells_allocated_in_regions[i][j]);
        }
    }
    Cell *selected_cell = NULL;
    int cell_drag = 0;

    SDL_Surface *hud = SDL_CreateRGBSurface(0, SCREEN_WIDTH, HUD_HEIGHT, SCREEN_DEPTH,
    		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    History *now, *oldest;
    int total_counts[NUM_TYPES];
    for (i = 0; i < NUM_TYPES; i++) {
        total_counts[i] = 0;
        for (j = 0; j < num_cells; j++) {
            total_counts[i] += cells[j].type_counts[i];
        }
    }
    create_hist(&now, total_elapsed, num_cells, total_counts, substances, &oldest);
    int hist_mode = 0;

    // measure time elapsed since last update to keep movement smooth
    int cur_elapsed, last_elapsed, hud_update, ms_since_last_update, frames_since_last_update, done;
    last_elapsed = SDL_GetTicks();
    cur_elapsed = last_elapsed;
    hud_update = 1;
    ms_since_last_update = 0;
    frames_since_last_update = 0;
    done = 0;
    while (!done) {
        handle_events(&done, &view_x_vel, &view_y_vel, &view_x_goal, &view_y_goal, &view_drag, view, &total_elapsed, substances,
                cells, &num_cells, cells_in_regions, num_cells_in_regions, &selected_cell, &cell_drag,
                &hist_mode, &now, &oldest, &selected_state, &hud_update);
        view.x += (view_x_goal - (view.x + view.w / 2)) / LIQUID_SCROLL;
        view_x_goal += view_x_vel * cur_elapsed / 1000;
        view.y += (view_y_goal - (view.y + view.h / 2)) / LIQUID_SCROLL;
        view_y_goal += view_y_vel * cur_elapsed / 1000;
        if (view.x < 0) {
            view.x = 0;
            view_x_goal = view.w / 2;
        } else if (view.x + view.w >= AREA_WIDTH) {
            view.x = AREA_WIDTH - view.w - 1;
            view_x_goal = AREA_WIDTH - view.w / 2 - 1;
        }
        if (view.y < 0) {
            view.y = 0;
            view_y_goal = view.h / 2;
        } else if (view.y + view.h >= AREA_HEIGHT) {
            view.y = AREA_HEIGHT - view.h - 1;
            view_y_goal = AREA_HEIGHT - view.h / 2 - 1;
        }

        cur_elapsed = SDL_GetTicks() - last_elapsed;
        last_elapsed = SDL_GetTicks();
        ms_since_last_update += cur_elapsed;
        frames_since_last_update++;
        if (total_elapsed + cur_elapsed > ULONG_MAX) {
            total_elapsed -= ULONG_MAX;
        }
        total_elapsed += cur_elapsed;

        assign_cells_to_regions(cells, num_cells, cells_in_regions, num_cells_in_regions, cells_allocated_in_regions);

        adjust_cells(cells, num_cells, cells_in_regions, num_cells_in_regions, substances, cur_elapsed);

        SDL_Rect r;
        r.x = 0;
        r.y = 0;
        r.w = view.w;
        r.h = view.h;
        SDL_FillRect(screen, &r, 0); // draw black on the screen
        
        if (!hist_mode) {
            draw_cells(screen, view, cells_in_regions, num_cells_in_regions, selected_cell);
        } else {
            draw_hist(screen, now, hist_mode, oldest);
        }

        draw_hud(hud, font, text_color, view, total_elapsed, substances, cells, num_cells, selected_cell,
                selected_state, hud_update, ms_since_last_update, frames_since_last_update);
        hud_update = 0;


        census_cells(cells, &num_cells, &selected_cell, substances, &hud_update);


        if (num_cells == MAX_CELLS) {
            printf("Cell Limit Hit\n");
        }

        r.y = view.h;
        SDL_BlitSurface(hud, NULL, screen, &r);

        r.y -= 1;
        r.h = 1;
        SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 83, 93, 108));

        if (ms_since_last_update > 1000) {
            ms_since_last_update = 0;
            frames_since_last_update = 0;
            for (i = 0; i < NUM_TYPES; i++) {
                total_counts[i] = 0;
                for (j = 0; j < num_cells; j++) {
                    total_counts[i] += cells[j].type_counts[i];
                }
            }
            update_hist(&now, total_elapsed, num_cells, total_counts, substances, &oldest);
        }
        SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(7);
    }

    save_state(0, total_elapsed, substances, cells, num_cells, now);

    // free memory mainly for valgrind
    free_hist(now, oldest);
    for (i = 0; i < X_REGIONS; i++) {
        for (j = 0; j < Y_REGIONS; j++) {
            free(cells_in_regions[i][j]);
        }
    }
    for (i = 0; i < num_cells; i++) {
        free_cell(cells + i);
    }
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_FreeSurface(hud);
    SDL_FreeSurface(screen);
    SDL_Quit();
    return 0;
}
