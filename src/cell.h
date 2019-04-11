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
#ifndef CELL_H
#define CELL_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#include "draw.h"
#include "constants.h"

#define CELL_SPEED 145
#define CELL_ROT_SPEED 14
#define CELL_MOV_DELAY_MAX 1600
#define CELL_MOV_DELAY_MIN 800
#define CELL_ROT_DELAY_MAX 1800
#define CELL_ROT_DELAY_MIN 1000
#define CELL_HARDNESS 2
#define CELL_MAX_AGE 120000
#define MAX_STATE_DURATION 500
#define EAT_GAIN_RATE 550
#define EAT_LOSS_RATE 1600
#define INFECTION_MIN_ENERGY 400000
#define VIRUS_DONATION_RATE 1600
#define ANTIVIRUS_GAIN_RATE 5400
#define ANTIVIRUS_LOSS_RATE 6000
#define MUTATION_CHANCE 6

#define FRICTION 0.9995
#define LN_FRICTION -0.00050012504168224286

typedef struct Organelle {
    // primary variables
    double angle; // angle by which this organelle is offset from its parent
    int r;
    int type;
    int parent_id;
    // secondary variables
    int x, y;
    int num_children; // number of organelles from the same cell that branch off this one
    struct Organelle **children;
} Organelle;

typedef struct Cell {
    // primary variables
    int x, y, x_err, y_err;
    double x_vel, y_vel;
    double rot, rot_vel;
    int mov_counter, rot_counter;
    long e;
    int age;
    int state, state_counter;
    int num_organelles;
    Organelle *organelles;
    struct Cell *virus;
    // secondary variables
    int r;
    int type_counts[NUM_TYPES];
    int primary_type;
    int weight; // used for energy loss and movement
    // tertiary variables
    int organelles_set, drawn;
    int pause_motion;
} Cell;


void add_initial_cells(Cell cells[MAX_CELLS]);
void set_organelle_loc(Organelle *cur_organelle, double parent_x, double parent_y, int parent_r, double cell_rot, int cell_e);
// recursively checks each organelle to find the distance of the outermost point of the cell
void set_secondary_variables(Cell *cell);
void save_cell(FILE *fp, Cell *cell);
void load_cell(FILE *fp, Cell *cell);
void free_cell(Cell *cell);
SDL_Color get_type_color(int type);
Uint32 map_type_color(int type, SDL_PixelFormat *format);
Uint32 map_state_color(int state, SDL_PixelFormat *format);
int energy_scale(int r, long e);
void adjust_cells(Cell cells[MAX_CELLS], int num_cells, Cell **cells_in_regions[][Y_REGIONS], int num_cells_in_regions[][Y_REGIONS],
        unsigned long long substances[3], int elapsed);
void handle_cell_collisions(Cell *a_cell, Cell *b_cell);
void handle_organelle_interaction(Cell *a_cell, Cell *b_cell, int a_type, int b_type);
void handle_wall_collisions(Cell *cell);
void census_cells(Cell cells[MAX_CELLS], int *num_cells, Cell **selected_cell, unsigned long long substances[3], int *hud_update);
void draw_cells(SDL_Surface *s, SDL_Rect view, Cell **cells_in_regions[][Y_REGIONS], int num_cells_in_regions[][Y_REGIONS], Cell *selected_cell);

#endif
