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
#include "cell.h"

void add_initial_cells(Cell cells[MAX_CELLS]) {
    int i, j, k;
    for (i = 0; i < AREA_WIDTH / CELL_SPACE; i++) {
        for (j = 0; j < AREA_HEIGHT / CELL_SPACE; j++) {
            Cell tmp_cell;
            tmp_cell.x = i*CELL_SPACE + CELL_SPACE/2;
            tmp_cell.y = j*CELL_SPACE + CELL_SPACE/2;
            tmp_cell.x_err = 0;
            tmp_cell.y_err = 0;
            tmp_cell.x_vel = 0;
            tmp_cell.y_vel = 0;
            tmp_cell.rot = 0;
            tmp_cell.rot_vel = 0;
            tmp_cell.mov_counter = 0;
            tmp_cell.rot_counter = 0;
            tmp_cell.pause_motion = 0;
            tmp_cell.e = 500000;
            tmp_cell.age = 0;
            for (k = 0; k < NUM_TYPES; k++) {
                tmp_cell.type_counts[k] = 0;
            }
            tmp_cell.virus = NULL;
            tmp_cell.num_organelles = rand() % 3 + 5;
            tmp_cell.organelles = malloc(sizeof(Organelle) * tmp_cell.num_organelles);
            for (k = 0; k < tmp_cell.num_organelles; k++) {
                Organelle tmp_organelle;
                if (k) {
                    tmp_organelle.r = 4 + rand() % 4;
                } else {
                    tmp_organelle.r = 7 + rand() % 3;
                }
                tmp_organelle.angle = rand() % 64 * M_PI / 32;
                tmp_organelle.type = rand() % NUM_TYPES;
                if (k) {
                    tmp_organelle.parent_id = rand() % k;
                } else {
                    tmp_organelle.parent_id = -1;
                }
                tmp_organelle.x = 0;
                tmp_organelle.y = 0;
                tmp_organelle.num_children = 0;
                tmp_organelle.children = NULL;
                tmp_cell.organelles[k] = tmp_organelle;
            }
            tmp_cell.state = 0;
            tmp_cell.state_counter = 0;
            set_secondary_variables(&tmp_cell);
            cells[i * (AREA_HEIGHT / CELL_SPACE) + j] = tmp_cell;
        }
    }
}

void set_organelle_loc(Organelle *cur_organelle, double parent_x, double parent_y, int parent_r, double cell_rot, int cell_e) {
    int cur_r = energy_scale(cur_organelle->r, cell_e);
    double cur_x = parent_x + (parent_r + cur_r) * cos(cur_organelle->angle + cell_rot);
    double cur_y = parent_y + (parent_r + cur_r) * sin(cur_organelle->angle + cell_rot);
    cur_organelle->x = cur_x;
    cur_organelle->y = cur_y;
    int i;
    for (i = 0; i < cur_organelle->num_children; i++) {
        set_organelle_loc(cur_organelle->children[i], cur_x, cur_y, cur_r, cell_rot, cell_e);
    }
}

void set_secondary_variables(Cell *cell) {
    int i, j;
    for (i = 0; i < cell->num_organelles; i++) {
        cell->organelles[i].num_children = 0;
    }
    for (i = 1; i < cell->num_organelles; i++) {
        cell->organelles[cell->organelles[i].parent_id].num_children++;
    }
    for (i = 0; i < cell->num_organelles; i++) {
        cell->organelles[i].children = malloc(cell->organelles[i].num_children * sizeof(*cell->organelles[i].children));
        int cur_child = 0;
        for (j = 1; j < cell->num_organelles; j++) {
            if (cell->organelles[j].parent_id == i) {
                cell->organelles[i].children[cur_child] = cell->organelles + j;
                cur_child++;
            }
        }
    }
    for (i = 0; i < NUM_TYPES; i++) {
        cell->type_counts[i] = 0;
    }
    set_organelle_loc(cell->organelles, 0, 0, -cell->organelles->r, 0, 1000000);
    cell->r = 0;
    for (i = 0; i < cell->num_organelles; i++) {
        int cur_r = ceil(sqrt(cell->organelles[i].x*cell->organelles[i].x +
                    cell->organelles[i].y*cell->organelles[i].y)) + cell->organelles[i].r;
        if (cur_r > cell->r) {
            cell->r = cur_r;
        }
        cell->type_counts[cell->organelles[i].type] += cell->organelles[i].r;
    }
    cell->primary_type = 0;
    for (i = 0; i < NUM_TYPES; i++) {
        if (cell->type_counts[i] >= cell->type_counts[cell->primary_type]) {
            cell->primary_type = i;
        }
    }
    cell->weight = 1;
    cell->weight += cell->type_counts[0] * 3 / 2;
    cell->weight += cell->type_counts[1] * 3 / 2;
    cell->weight += cell->type_counts[2] * 3 / 2;
    cell->weight += cell->type_counts[3] / 5;
    cell->weight += cell->type_counts[4] * 5 / 9;
    cell->weight += cell->type_counts[5] * 5 / 9;
    cell->weight += cell->type_counts[6] * 5 / 9;
    cell->weight += cell->type_counts[7];
    cell->weight += cell->type_counts[8] * 4 / 5;
    cell->organelles_set = 0;
    cell->drawn = 0;
    cell->pause_motion = 0;
}

void save_cell(FILE *fp, Cell *cell) {
    int i;
    fprintf(fp, "%d %d %d %d %lf %lf %lf %lf %d %d %ld %d %d %d %d\n",
            cell->x, cell->y, cell->x_err, cell->y_err,
            cell->x_vel, cell->y_vel, cell->rot, cell->rot_vel,
            cell->mov_counter, cell->rot_counter, cell->e,
            cell->age, cell->state, cell->state_counter,
            cell->num_organelles);
    for (i = 0; i < cell->num_organelles; i++) {
        fprintf(fp, "%lf %d %d %d\n",
                cell->organelles[i].angle,
                cell->organelles[i].r,
                cell->organelles[i].type,
                cell->organelles[i].parent_id);
    }
}

void load_cell(FILE *fp, Cell *cell) {
    int i;
    fscanf(fp, "%d %d %d %d %lf %lf %lf %lf %d %d %ld %d %d %d %d\n",
            &cell->x, &cell->y, &cell->x_err, &cell->y_err,
            &cell->x_vel, &cell->y_vel,
            &cell->rot, &cell->rot_vel,
            &cell->mov_counter, &cell->rot_counter, &cell->e,
            &cell->age, &cell->state, &cell->state_counter,
            &cell->num_organelles);
    cell->organelles = malloc(cell->num_organelles * sizeof(Organelle));
    for (i = 0; i < cell->num_organelles; i++) {
        fscanf(fp, "%lf %d %d %d\n",
                &cell->organelles[i].angle,
                &cell->organelles[i].r,
                &cell->organelles[i].type,
                &cell->organelles[i].parent_id);
    }
    cell->virus = NULL;
}

void free_cell(Cell *cell) {
    int i;
    for (i = 0; i < cell->num_organelles; i++) {
        free(cell->organelles[i].children);
    }
    free(cell->organelles);
    if (cell->virus) {
        free_cell(cell->virus);
        free(cell->virus);
    }
}

SDL_Color get_type_color(int type) {
    switch (type) {
        case 0:
            return (SDL_Color){0, 255, 0};
        case 1:
            return (SDL_Color){255, 0, 255};
        case 2:
            return (SDL_Color){255, 91, 0};
        case 3:
            return (SDL_Color){0, 255, 255};
        case 4:
            return (SDL_Color){255, 0, 0};
        case 5:
            return (SDL_Color){0, 0, 255};
        case 6:
            return (SDL_Color){255, 255, 0};
        case 7:
            return (SDL_Color){255, 255, 255};
        case 8:
            return (SDL_Color){127, 127, 127};
        case 9:
            return (SDL_Color){127, 0, 192};
    }
    return (SDL_Color){0, 0, 0};
}

Uint32 map_type_color(int type, SDL_PixelFormat *format) {
    switch (type) {
        case 0:
            return SDL_MapRGB(format, 0, 255, 0);
        case 1:
            return SDL_MapRGB(format, 255, 0, 255);
        case 2:
            return SDL_MapRGB(format, 255, 91, 0);
        case 3:
            return SDL_MapRGB(format, 0, 255, 255);
        case 4:
            return SDL_MapRGB(format, 255, 0, 0);
        case 5:
            return SDL_MapRGB(format, 0, 0, 255);
        case 6:
            return SDL_MapRGB(format, 255, 255, 0);
        case 7:
            return SDL_MapRGB(format, 255, 255, 255);
        case 8:
            return SDL_MapRGB(format, 127, 127, 127);
        case 9:
            return SDL_MapRGB(format, 127, 0, 192);
    }
    return 0;
}

Uint32 map_state_color(int state, SDL_PixelFormat *format) {
    switch (state) {
        case 0:
            return 0;
        case 1:
            return SDL_MapRGB(format, 127, 91, 0);
        case 2:
            return SDL_MapRGB(format, 91, 0, 16);
        case 3:
            return SDL_MapRGB(format, 0, 127, 0);
        case 4:
            return SDL_MapRGB(format, 255, 0, 0);
        case 5:
            return SDL_MapRGB(format, 0, 0, 255);
        case 6:
            return SDL_MapRGB(format, 255, 255, 0);
        case 7:
            return SDL_MapRGB(format, 255, 255, 255);
        case 8:
            return SDL_MapRGB(format, 127, 127, 127);
        case 9:
            return SDL_MapRGB(format, 127, 0, 192);
        case 10:
            return SDL_MapRGB(format, 255, 255, 128);
    }
    return 0;
}

int energy_scale(int r, long e) {
    long capped_e = e;
    if (capped_e > 1000000) {
        capped_e = 1000000;
    } else if (capped_e < -500000) {
        capped_e = -500000;
    }
    return (long)r * (capped_e + 500000) / 1500000;
}

void adjust_cells(Cell cells[MAX_CELLS], int num_cells, Cell **cells_in_regions[][Y_REGIONS], int num_cells_in_regions[][Y_REGIONS],
        unsigned long long substances[3], int elapsed) {

    int i, j, k, l;
    for (i = 0; i < num_cells; i++) {
        // activate movement organelles
        if (cells[i].mov_counter > 0) {
            cells[i].mov_counter -= elapsed;
        } else {
            cells[i].x_vel += cos(M_PI * (rand() % 256) / 128) * cells[i].type_counts[3] * (rand() % (CELL_SPEED / 2) + CELL_SPEED) / cells[i].weight;
            cells[i].y_vel += sin(M_PI * (rand() % 256) / 128) * cells[i].type_counts[3] * (rand() % (CELL_SPEED / 2) + CELL_SPEED) / cells[i].weight;
            cells[i].mov_counter = CELL_MOV_DELAY_MAX - rand() % (CELL_MOV_DELAY_MAX - CELL_MOV_DELAY_MIN);
        }
        if (cells[i].rot_counter > 0) {
            cells[i].rot_counter -= elapsed;
        } else {
            cells[i].rot_vel += (rand() % CELL_ROT_SPEED - CELL_ROT_SPEED / 2) * M_PI * cells[i].type_counts[3] / cells[i].weight / 3;
            cells[i].rot_counter = CELL_ROT_DELAY_MAX - rand() % (CELL_ROT_DELAY_MAX - CELL_ROT_DELAY_MIN);
        }

        // apply movement friction
        double total_friction = pow(FRICTION, elapsed);
        double last_x_vel = cells[i].x_vel;
        cells[i].x_vel *= total_friction;
        int dx = (total_friction - 1) * last_x_vel / LN_FRICTION;
        double last_y_vel = cells[i].y_vel;
        cells[i].y_vel *= total_friction;
        int dy = (total_friction - 1) * last_y_vel / LN_FRICTION;

        if (!cells[i].pause_motion) {
            // move cell
            cells[i].x_err += dx;
            cells[i].x += cells[i].x_err / 1000;
            cells[i].x_err -= cells[i].x_err / 1000 * 1000;
            cells[i].y_err += dy;
            cells[i].y += cells[i].y_err / 1000;
            cells[i].y_err -= cells[i].y_err / 1000 * 1000;

            // apply rotational friction
            double last_rot_vel = cells[i].rot_vel;
            cells[i].rot_vel *= total_friction;
            double drot = (total_friction - 1) * last_rot_vel / LN_FRICTION;

            // rotate cell
            cells[i].rot += drot / 1000;
            if (cells[i].rot < 0) {
                cells[i].rot += M_PI * 2;
            } else if (cells[i].rot >= M_PI * 2) {
                cells[i].rot -= M_PI * 2;
            }
        }
    }

    for (i = 0; i < X_REGIONS; i++) {
        for (j = 0; j < Y_REGIONS; j++) {
            for (k = 0; k < num_cells_in_regions[i][j]; k++) {
                for (l = k + 1; l < num_cells_in_regions[i][j]; l++) {
                    handle_cell_collisions(cells_in_regions[i][j][k], cells_in_regions[i][j][l]);
                }
            }
        }
    }


    for (i = 0; i < X_REGIONS; i++) {
        for (j = 0; j < Y_REGIONS; j += Y_REGIONS - 1) {
            for (k = 0; k < num_cells_in_regions[i][j]; k++) {
                handle_wall_collisions(cells_in_regions[i][j][k]);
            }
        }
    }
    for (i = 0; i < X_REGIONS; i += X_REGIONS - 1) {
        for (j = 0; j < Y_REGIONS; j++) {
            for (k = 0; k < num_cells_in_regions[i][j]; k++) {
                handle_wall_collisions(cells_in_regions[i][j][k]);
            }
        }
    }

    for (i = 0; i < num_cells; i++) {
        // apply energy changes
        if (cells[i].state_counter) {
            int state_elapsed;
            if (cells[i].state_counter > elapsed) {
                state_elapsed = elapsed;
            } else {
                state_elapsed = cells[i].state_counter;
            }
            switch (cells[i].state) {
                case 0:
                    break;
                case 1:
                    cells[i].e -= EAT_LOSS_RATE * state_elapsed;
                    substances[2] += (EAT_LOSS_RATE - EAT_GAIN_RATE) * state_elapsed;
                    break;
                case 2:
                    cells[i].e -= EAT_LOSS_RATE * state_elapsed;
                    substances[1] += (EAT_LOSS_RATE - EAT_GAIN_RATE) * state_elapsed;
                    break;
                case 3:
                    cells[i].e -= EAT_LOSS_RATE * state_elapsed;
                    substances[0] += (EAT_LOSS_RATE - EAT_GAIN_RATE) * state_elapsed;
                    break;
                case 4:
                case 5:
                case 6:
                    cells[i].e += EAT_GAIN_RATE * state_elapsed;
                    break;
                case 7:
                    cells[i].e -= VIRUS_DONATION_RATE * state_elapsed;
                    break;
                case 8:
                    cells[i].e += ANTIVIRUS_GAIN_RATE * state_elapsed;
                    break;
                case 9:
                    cells[i].e += VIRUS_DONATION_RATE * state_elapsed;
                    break;
                case 10:
                    cells[i].e -= ANTIVIRUS_LOSS_RATE * state_elapsed;
                    substances[rand()%3] += (ANTIVIRUS_LOSS_RATE - ANTIVIRUS_GAIN_RATE) * state_elapsed;
                    break;
            }
            cells[i].state_counter -= state_elapsed;
            if (cells[i].state_counter <= 0) {
                cells[i].state = 0;
            }
        } else {
            // regular energy changes only when not interacting
            for (j = 0; j < 3; j++) {
                int synthesis = cells[i].type_counts[j] * elapsed * SYNTHESIS_FACTOR / SYNTHESIS_DIVISOR +
                    cells[i].type_counts[j] * substances[j] / SYNTHESIS_SUBSTANCE_DIVISOR * elapsed;
                if (synthesis > substances[j]) {
                    synthesis = substances[j];
                }
                substances[j] -= synthesis;
                int out_flow = synthesis / 7;
                substances[(j + 1) % 3] += out_flow;
                cells[i].e += synthesis - out_flow;
            }

            long energy_loss = cells[i].weight * cells[i].weight * num_cells * elapsed / 49000 +
                cells[i].weight * num_cells * elapsed / 2000 +
                num_cells * elapsed / 35;
            cells[i].age += elapsed;
            if (cells[i].age > CELL_MAX_AGE) {
                energy_loss *= cells[i].age / CELL_MAX_AGE;
            }
            if (energy_loss > 0) {
            	cells[i].e -= energy_loss;
            } else {
            	energy_loss = 0;
            }
            unsigned long long substance_total = 0;
            for (j = 0; j < 3; j++) {
                substance_total += substances[j];
            }
            long substance_added = 0;
            for (j = 0; j < 3; j++) {
                substance_added += energy_loss * substances[j] / substance_total;
                substances[j] += energy_loss * substances[j] / substance_total;
            }
            if (energy_loss > substance_added) substances[rand()%3] += energy_loss - substance_added;

        }
        cells[i].organelles_set = 0;
        cells[i].drawn = 0;
    }
}

void handle_cell_collisions(Cell *a_cell, Cell *b_cell) {
    int i, j;
    int dx = a_cell->x - b_cell->x;
    int dy = a_cell->y - b_cell->y;
    int rs = energy_scale(a_cell->r, a_cell->e) + energy_scale(b_cell->r, b_cell->e);
    if (dx * dx + dy * dy < rs * rs) {
        if (!a_cell->organelles_set) {
            set_organelle_loc(a_cell->organelles, 0, 0, energy_scale(-a_cell->organelles->r, a_cell->e), a_cell->rot, a_cell->e);
            a_cell->organelles_set = 1;
        }
        if (!b_cell->organelles_set) {
            set_organelle_loc(b_cell->organelles, 0, 0, energy_scale(-b_cell->organelles->r, b_cell->e), b_cell->rot, b_cell->e);
            b_cell->organelles_set = 1;
        }
        int a_collision_min_dist2 = rs * rs;
        int b_collision_min_dist2 = rs * rs;
        for (i = 0; i < a_cell->num_organelles; i++) {
            for (j = 0; j < b_cell->num_organelles; j++) {
                dx = (a_cell->organelles[i].x + a_cell->x) - (b_cell->organelles[j].x + b_cell->x);
                dy = (a_cell->organelles[i].y + a_cell->y) - (b_cell->organelles[j].y + b_cell->y);
                rs = energy_scale(a_cell->organelles[i].r, a_cell->e) + energy_scale(b_cell->organelles[j].r, b_cell->e);
                if (dx * dx + dy * dy < rs * rs) {
                    if (!(a_cell->state || b_cell->state)) {
                        handle_organelle_interaction(a_cell, b_cell, a_cell->organelles[i].type, b_cell->organelles[j].type);
                        handle_organelle_interaction(b_cell, a_cell, b_cell->organelles[j].type, a_cell->organelles[i].type);
                    }
                    int a_cur_dist2 = a_cell->organelles[i].x * a_cell->organelles[i].x +
                        a_cell->organelles[i].y * a_cell->organelles[i].y;
                    if (a_cur_dist2 < a_collision_min_dist2) {
                        a_collision_min_dist2 = a_cur_dist2;
                        a_cell->x_vel = dx * CELL_HARDNESS;
                        a_cell->y_vel = dy * CELL_HARDNESS;
                        a_cell->rot_vel = -atan2(dy, dx) * CELL_HARDNESS / 6;
                    }
                    int b_cur_dist2 = b_cell->organelles[j].x * b_cell->organelles[j].x +
                        b_cell->organelles[j].y * b_cell->organelles[j].y;
                    if (b_cur_dist2 < b_collision_min_dist2) {
                        b_collision_min_dist2 = b_cur_dist2; 
                        b_cell->x_vel = -dx * CELL_HARDNESS;
                        b_cell->y_vel = -dy * CELL_HARDNESS;
                        b_cell->rot_vel = atan2(dy, dx) * CELL_HARDNESS / 6;
                    }
                    if (!(dx || dy)) {
                        a_cell->x_vel = rand() % 3 - 1;
                        a_cell->y_vel = rand() % 3 - 1;
                        b_cell->x_vel = rand() % 3 - 1;
                        b_cell->y_vel = rand() % 3 - 1;
                    }
                }
            }
        }
    }
}

void handle_organelle_interaction(Cell *a_cell, Cell *b_cell, int a_type, int b_type) {
    int i;
    if (a_type == 4 && b_cell->e > 0 &&
            !(b_type == 4 || b_type == 5 || b_type == 7 || b_type == 8)) {
        a_cell->state = 4;
        b_cell->state = 1;
        a_cell->state_counter = MAX_STATE_DURATION;
        b_cell->state_counter = MAX_STATE_DURATION;
    } else if (a_type == 5 && b_cell->e > 0 &&
            !(b_type == 5 || b_type == 6 || b_type == 7 || b_type == 8)) {
        a_cell->state = 5;
        b_cell->state = 2;
        a_cell->state_counter = MAX_STATE_DURATION;
        b_cell->state_counter = MAX_STATE_DURATION;
    } else if (a_type == 6 && b_cell->e > 0 &&
            !(b_type == 4 || b_type == 6 || b_type == 7 || b_type == 8)) {
        a_cell->state = 6;
        b_cell->state = 3;
        a_cell->state_counter = MAX_STATE_DURATION;
        b_cell->state_counter = MAX_STATE_DURATION;
    } else if (a_type == 7 && !b_cell->virus && a_cell->e > INFECTION_MIN_ENERGY &&
            !(b_type == 3 || b_type == 7 || b_type == 8)) {
        a_cell->state = 7;
        b_cell->state = 9;
        a_cell->state_counter = MAX_STATE_DURATION;
        b_cell->state_counter = MAX_STATE_DURATION;
        b_cell->virus = malloc(sizeof(Cell));
        *b_cell->virus = *a_cell;
        b_cell->virus->organelles = malloc(b_cell->virus->num_organelles * sizeof(Organelle));
        for (i = 0; i < b_cell->virus->num_organelles; i++) {
            b_cell->virus->organelles[i] = a_cell->organelles[i];
            b_cell->virus->organelles[i].num_children = 0;
            b_cell->virus->organelles[i].children = NULL;
        }
        set_secondary_variables(b_cell->virus);
        b_cell->virus->virus = NULL;
    } else if (a_type == 8 && b_cell->e > 0 &&
            (b_type == 7 || b_cell->virus)) {
        a_cell->state = 8;
        b_cell->state = 10;
        a_cell->state_counter = MAX_STATE_DURATION;
        b_cell->state_counter = MAX_STATE_DURATION;
    }
    if ((b_cell->state == 1 || b_cell->state == 2 || b_cell->state == 3) &&
            b_cell->virus && !a_cell->virus && rand() % 2) {
        a_cell->virus = malloc(sizeof(Cell));
        *a_cell->virus = *b_cell->virus;
        a_cell->virus->organelles = malloc(a_cell->virus->num_organelles * sizeof(Organelle));
        for (i = 0; i < a_cell->virus->num_organelles; i++) {
            a_cell->virus->organelles[i] = b_cell->virus->organelles[i];
            a_cell->virus->organelles[i].num_children = 0;
            a_cell->virus->organelles[i].children = NULL;
        }
        set_secondary_variables(a_cell->virus);
        a_cell->virus->virus = NULL;
    }
    if ((b_cell->state == 1 || b_cell->state == 2 || b_cell->state == 3) &&
            b_cell->state_counter * EAT_LOSS_RATE > b_cell->e) {
        a_cell->state_counter = (b_cell->e + EAT_LOSS_RATE - 1) / EAT_LOSS_RATE;
        b_cell->state_counter = (b_cell->e + EAT_LOSS_RATE - 1) / EAT_LOSS_RATE;
    }
    if (a_cell->state == 7 &&
            a_cell->state_counter * VIRUS_DONATION_RATE > a_cell->e) {
        b_cell->state_counter = (a_cell->e + VIRUS_DONATION_RATE - 1) / VIRUS_DONATION_RATE;
        a_cell->state_counter = (a_cell->e + VIRUS_DONATION_RATE - 1) / VIRUS_DONATION_RATE;
    }
    if (b_cell->state == 10 &&
            b_cell->state_counter * ANTIVIRUS_LOSS_RATE > b_cell->e) {
        a_cell->state_counter = (b_cell->e + ANTIVIRUS_LOSS_RATE - 1) / ANTIVIRUS_LOSS_RATE;
        b_cell->state_counter = (b_cell->e + ANTIVIRUS_LOSS_RATE - 1) / ANTIVIRUS_LOSS_RATE;
    }
}

void handle_wall_collisions(Cell *cell) {
    if (cell->x - energy_scale(cell->r, cell->e) < 0) {
        if (!cell->organelles_set) {
            set_organelle_loc(cell->organelles, 0, 0, energy_scale(-cell->organelles->r, cell->e), cell->rot, cell->e);
            cell->organelles_set = 1;
        }
        int dir_r = 0;
        int i;
        for (i = 0; i < cell->num_organelles; i++) {
            if (-(cell->organelles[i].x - energy_scale(cell->organelles[i].r, cell->e)) > dir_r) {
                dir_r = -(cell->organelles[i].x - energy_scale(cell->organelles[i].r, cell->e));
            }
        }
        if (cell->x - dir_r < 0) {
            cell->x_vel = 0;
            cell->x_err = 0;
            cell->x = dir_r;
        }
    } else if (cell->x + energy_scale(cell->r, cell->e) >= AREA_WIDTH) {
        if (!cell->organelles_set) {
            set_organelle_loc(cell->organelles, 0, 0, energy_scale(-cell->organelles->r, cell->e), cell->rot, cell->e);
            cell->organelles_set = 1;
        }
        int dir_r = 0;
        int i;
        for (i = 0; i < cell->num_organelles; i++) {
            if (cell->organelles[i].x + energy_scale(cell->organelles[i].r, cell->e) > dir_r) {
                dir_r = cell->organelles[i].x + energy_scale(cell->organelles[i].r, cell->e);
            }
        }
        if (cell->x + dir_r >= AREA_WIDTH) {
            cell->x_vel = 0;
            cell->x_err = 0;
            cell->x = AREA_WIDTH - dir_r - 1;
        }
    }
    if (cell->y - energy_scale(cell->r, cell->e) < 0) {
        if (!cell->organelles_set) {
            set_organelle_loc(cell->organelles, 0, 0, energy_scale(-cell->organelles->r, cell->e), cell->rot, cell->e);
            cell->organelles_set = 1;
        }
        int dir_r = 0;
        int i;
        for (i = 0; i < cell->num_organelles; i++) {
            if (-(cell->organelles[i].y - energy_scale(cell->organelles[i].r, cell->e)) > dir_r) {
                dir_r = -(cell->organelles[i].y - energy_scale(cell->organelles[i].r, cell->e));
            }
        }
        if (cell->y - dir_r < 0) {
            cell->y_vel = 0;
            cell->y_err = 0;
            cell->y = dir_r;
        }
    } else if (cell->y + energy_scale(cell->r, cell->e) >= AREA_HEIGHT) {
        if (!cell->organelles_set) {
            set_organelle_loc(cell->organelles, 0, 0, energy_scale(-cell->organelles->r, cell->e), cell->rot, cell->e);
            cell->organelles_set = 1;
        }
        int dir_r = 0;
        int i;
        for (i = 0; i < cell->num_organelles; i++) {
            if (cell->organelles[i].y + energy_scale(cell->organelles[i].r, cell->e) > dir_r) {
                dir_r = cell->organelles[i].y + energy_scale(cell->organelles[i].r, cell->e);
            }
        }
        if (cell->y + dir_r >= AREA_HEIGHT) {
            cell->y_vel = 0;
            cell->y_err = 0;
            cell->y = AREA_HEIGHT - dir_r - 1;
        }
    }
}

void census_cells(Cell cells[MAX_CELLS], int *num_cells, Cell **selected_cell, unsigned long long substances[3], int *hud_update) {
    int i, j;
    for (i = 0; i < *num_cells; i++) {
        if (cells[i].e >= 1000000 && *num_cells < MAX_CELLS) {
            int empty_x[6];
            int empty_y[6];
            // first look for an empty space
            int num_empty = 0;
            double d;
            for (d = 0; d < 2*M_PI; d+=M_PI/3) {
                int space_x = cells[i].x + cos(d) * (2*cells[i].r + 1);
                int space_y = cells[i].y + sin(d) * (2*cells[i].r + 1);
                int space_occupied = 0;
                if (space_x - cells[i].r < 0 || space_x + cells[i].r >= AREA_WIDTH ||
                        space_y - cells[i].r < 0 || space_y + cells[i].r >= AREA_HEIGHT) {
                    space_occupied = 1;
                }
                for (j = 0; j < *num_cells; j++) {
                    int dx = space_x - cells[j].x;
                    int dy = space_y - cells[j].y;
                    int rs = cells[i].r + cells[j].r;
                    if (dx * dx + dy * dy < rs * rs) {
                        space_occupied = 1;
                    }
                }
                if (!space_occupied) {
                    empty_x[num_empty] = space_x;
                    empty_y[num_empty] = space_y;
                    num_empty++;
                }
            }
            // if space exists, spawn child
            if (num_empty) {
            	// pick a random space to spawn
                int spawn_space = rand() % num_empty;
                int spawn_x = empty_x[spawn_space];
                int spawn_y = empty_y[spawn_space];
                int mutation = 0;
                if (rand() % 101 < MUTATION_CHANCE) {
                    mutation = rand() % 6 + 1;
                }
                Cell *parent_cell = cells + i;
                if (parent_cell->virus && rand() % 2) {
                    parent_cell = parent_cell->virus;
                }
                Cell tmp_cell;
                tmp_cell.x = spawn_x;
                tmp_cell.y = spawn_y;
                tmp_cell.x_err = 0;
                tmp_cell.y_err = 0;
                tmp_cell.x_vel = 0;
                tmp_cell.y_vel = 0;
                tmp_cell.rot = cells[i].rot;
                tmp_cell.rot_vel = 0;
                tmp_cell.mov_counter = 0;
                tmp_cell.rot_counter = 0;
                tmp_cell.pause_motion = 0;
                tmp_cell.drawn = 0;
                tmp_cell.e = cells[i].e / 2;
                cells[i].e -= tmp_cell.e;
                tmp_cell.age = 0;
                if (parent_cell->virus && rand() % 2) {
                    // pass on virus
                    tmp_cell.virus = malloc(sizeof(Cell));
                    *tmp_cell.virus = *parent_cell->virus;
                    tmp_cell.virus->organelles = malloc(tmp_cell.virus->num_organelles * sizeof(Organelle));
                    for (j = 0; j < tmp_cell.virus->num_organelles; j++) {
                        tmp_cell.virus->organelles[j] = parent_cell->virus->organelles[j];
                        tmp_cell.virus->organelles[j].num_children = 0;
                        tmp_cell.virus->organelles[j].children = NULL;
                    }
                    set_secondary_variables(tmp_cell.virus);
                } else {
                    tmp_cell.virus = NULL;
                }
                tmp_cell.num_organelles = parent_cell->num_organelles;
                switch (mutation) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        // mutation doesn't change number of organelles
                        tmp_cell.organelles = malloc(tmp_cell.num_organelles * sizeof(Organelle));
                        break;
                    case 5:
                        // adding an organelle
                        tmp_cell.organelles = malloc((tmp_cell.num_organelles + 1) * sizeof(Organelle));
                        break;
                    case 6:
                        // removing an organelle
                        if (tmp_cell.num_organelles > 1) {
                            tmp_cell.num_organelles--;
                        }
                        tmp_cell.organelles = malloc(tmp_cell.num_organelles * sizeof(Organelle));
                        break;
                }
                for (j = 0; j < tmp_cell.num_organelles; j++) {
                    tmp_cell.organelles[j] = parent_cell->organelles[j];
                    tmp_cell.organelles[j].num_children = 0;
                    tmp_cell.organelles[j].children = NULL;
                }
                tmp_cell.state = 0;
                tmp_cell.state_counter = 0;
                for (j = 0; j < NUM_TYPES; j++) {
                    tmp_cell.type_counts[j] = 0; 
                }
                switch (mutation) {
                    case 1:
                        {
                            // mutate an organelle's radius
                            Organelle *mut_organelle = tmp_cell.organelles + rand() % tmp_cell.num_organelles;
                            int new_r;
                            if (mut_organelle == tmp_cell.organelles) {
                                new_r = 7 + rand() % 3;
                                while (new_r == mut_organelle->r) {
                                    new_r = 7 + rand() % 3;
                                }
                            } else {
                                new_r = 4 + rand() % 4;
                                while (new_r == mut_organelle->r) {
                                    new_r = 4 + rand() % 4;
                                }
                            }
                            mut_organelle->r = new_r;
                        }
                        break;
                    case 2:
                        {
                            // mutate an organelle's angle
                            Organelle *mut_organelle = tmp_cell.organelles + rand() % tmp_cell.num_organelles;
                            int new_angle = rand() % 60 * M_PI / 32;
                            if (new_angle >= mut_organelle->angle - M_PI / 16) {
                                new_angle += M_PI / 8;
                            }
                            mut_organelle->angle = new_angle;
                        }
                        break;
                    case 3:
                        {
                            // mutate an organelle's type
                            Organelle *mut_organelle = tmp_cell.organelles + rand() % tmp_cell.num_organelles;
                            int new_type = rand() % (NUM_TYPES - 1);
                            if (new_type >= mut_organelle->type) {
                                new_type++;
                            }
                            mut_organelle->type = new_type;
                        }
                        break;
                    case 4:
                        {
                            // replace all organelles of one type with another
                            int old_type;
                            int cell_has_old_type = 0;
                            while (!cell_has_old_type) {
                                old_type = rand() % NUM_TYPES;
                                cell_has_old_type = 0;
                                for (j = 0; j < tmp_cell.num_organelles; j++) {
                                    if (tmp_cell.organelles[j].type == old_type) {
                                        cell_has_old_type = 1;
                                    }
                                }
                            }
                            int new_type = rand() % (NUM_TYPES - 1);
                            if (new_type >= old_type) {
                                new_type++;
                            }
                            for (j = 0; j < tmp_cell.num_organelles; j++) {
                                if (tmp_cell.organelles[j].type == old_type) {
                                    tmp_cell.organelles[j].type = new_type;
                                }
                            }
                        }
                        break;
                    case 5:
                        {
                            // add an organelle
                            Organelle tmp_organelle;
                            tmp_organelle.r = 4 + rand() % 4;
                            tmp_organelle.angle = M_PI * (rand() % 64) / 32;
                            tmp_organelle.type = rand() % NUM_TYPES;
                            tmp_organelle.parent_id = rand() % tmp_cell.num_organelles;
                            tmp_organelle.x = 0;
                            tmp_organelle.y = 0;
                            tmp_organelle.children = NULL;
                            tmp_organelle.num_children = 0;
                            tmp_cell.organelles[tmp_cell.num_organelles] = tmp_organelle;
                            tmp_cell.num_organelles++;
                        }
                        break;
                }
                set_secondary_variables(&tmp_cell);
                cells[*num_cells] = tmp_cell;
                (*num_cells)++;
            }
        } else if (*num_cells < 10) {
            // give energy
            int s = rand()%3;
            while (cells[i].e < 1000000) {
                if (substances[s] > 1000000) {
                    substances[s] -= 1000000;
                    cells[i].e += 1000000;
                } else {
                    s = rand()%3;
                }
            }
        } else if ((cells[i].e <= 0 && !cells[i].state) || cells[i].state == -1) {
            // kill the cell
            int s = rand()%3;
            while (cells[i].e) {
                if (substances[s] > -cells[i].e) {
             		// substance is enough to add negative energy
           	    	substances[s] += cells[i].e;
           		    cells[i].e = 0;
               	} else {
              		// negative energy too great
               		s = rand()%3;
              	}
            }
            (*num_cells)--;
            if (*selected_cell == cells + *num_cells) {
                *selected_cell = cells + i;
            } else if (*selected_cell == cells + i) {
                *selected_cell = NULL;
                *hud_update = 1;
            }
            free_cell(cells + i);
            cells[i] = cells[*num_cells];
        }
    }
}

void draw_cells(SDL_Surface *s, SDL_Rect view, Cell **cells_in_regions[][Y_REGIONS], int num_cells_in_regions[][Y_REGIONS], Cell *selected_cell) {
    int i, j, k, l;
    int right_region = (view.x + view.w - 1) / (AREA_WIDTH / X_REGIONS);
    int bottom_region = (view.y + view.h - 1) / (AREA_HEIGHT / Y_REGIONS);
    int left_region = view.x / (AREA_WIDTH / X_REGIONS);
    int top_region = view.y / (AREA_HEIGHT / Y_REGIONS);
    for (i = left_region; i <= right_region; i++) {
        for (j = top_region; j <= bottom_region; j++) {
            for (k = 0; k < num_cells_in_regions[i][j]; k++) {
                if (!cells_in_regions[i][j][k]->organelles_set) {
                    set_organelle_loc(cells_in_regions[i][j][k]->organelles, 0, 0,
                            energy_scale(-cells_in_regions[i][j][k]->organelles->r, cells_in_regions[i][j][k]->e),
                            cells_in_regions[i][j][k]->rot, cells_in_regions[i][j][k]->e);
                    cells_in_regions[i][j][k]->organelles_set = 1;
                }
                if (!cells_in_regions[i][j][k]->drawn) {
                    for (l = 0; l < cells_in_regions[i][j][k]->num_organelles; l++) {
                        if (cells_in_regions[i][j][k]->state) {
                            draw_circle(s, cells_in_regions[i][j][k]->organelles[l].x + cells_in_regions[i][j][k]->x - view.x,
                                    cells_in_regions[i][j][k]->organelles[l].y + cells_in_regions[i][j][k]->y - view.y,
                                    energy_scale(cells_in_regions[i][j][k]->organelles[l].r, cells_in_regions[i][j][k]->e),
                                    map_state_color(cells_in_regions[i][j][k]->state, s->format));
                        } else {
                            draw_circle(s, cells_in_regions[i][j][k]->organelles[l].x + cells_in_regions[i][j][k]->x - view.x,
                                    cells_in_regions[i][j][k]->organelles[l].y + cells_in_regions[i][j][k]->y - view.y,
                                    energy_scale(cells_in_regions[i][j][k]->organelles[l].r, cells_in_regions[i][j][k]->e),
                                    map_type_color(cells_in_regions[i][j][k]->organelles[l].type, s->format));
                        }
                    }
                    if (cells_in_regions[i][j][k]->virus) {
                        SDL_Rect r;
                        r.x = cells_in_regions[i][j][k]->x - view.x -
                            energy_scale(cells_in_regions[i][j][k]->organelles->r, cells_in_regions[i][j][k]->e);
                        r.y = cells_in_regions[i][j][k]->y - view.y;
                        r.w = energy_scale(cells_in_regions[i][j][k]->organelles->r * 2, cells_in_regions[i][j][k]->e);
                        r.h = 1;
                        SDL_FillRect(s, &r, map_type_color(cells_in_regions[i][j][k]->virus->primary_type, s->format));
                        r.x = cells_in_regions[i][j][k]->x - view.x;
                        r.y = cells_in_regions[i][j][k]->y - view.y -
                            energy_scale(cells_in_regions[i][j][k]->organelles->r, cells_in_regions[i][j][k]->e);
                        r.w = 1;
                        r.h = energy_scale(cells_in_regions[i][j][k]->organelles->r * 2, cells_in_regions[i][j][k]->e);
                        SDL_FillRect(s, &r, map_type_color(cells_in_regions[i][j][k]->virus->primary_type, s->format));
                    }
                    cells_in_regions[i][j][k]->drawn = 1;
                }
            }
        }
    }
    if (selected_cell) {
        draw_circle(s, selected_cell->x - view.x, selected_cell->y - view.y,
                energy_scale(selected_cell->r, selected_cell->e), SDL_MapRGB(s->format, 192, 192, 192));
    }
}
