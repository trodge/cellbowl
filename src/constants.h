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
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define AREA_WIDTH 3600
#define AREA_HEIGHT 2160
#define X_REGIONS 10
#define Y_REGIONS 6

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define HUD_HEIGHT 150
#define VIEW_HEIGHT (SCREEN_HEIGHT - HUD_HEIGHT)
#define SCREEN_DEPTH 32

#define MAX_CELLS 1200
#define CELL_SPACE 180

#define NUM_TYPES 9

#define SUBSTANCE_START 2240000000ull

#define SYNTHESIS_FACTOR 6
#define SYNTHESIS_DIVISOR 7
#define SYNTHESIS_SUBSTANCE_DIVISOR 1200000000ull

#endif
