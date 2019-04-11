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
#include "draw.h"

void draw_text(SDL_Surface *s, TTF_Font *font, int x, int y, int x_align, int y_align, SDL_Color color, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (len <= 0) return;
    char *str = malloc(len + 1);
    if (str == NULL) return;
    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    va_end(ap);
    SDL_Surface *text = TTF_RenderText_Solid(font, str, color);
    free(str);
    SDL_Rect r;
    switch (x_align) {
        case -1:
            r.x = x;
            break;
        case 0:
            r.x = x - text->w / 2;
            break;
        case 1:
            r.x = x - text->w;
            break;
    }
    switch (y_align) {
        case -1:
            r.y = y;
            break;
        case 0:
            r.y = y - text->h / 2;
            break;
        case 1:
            r.y = y - text->h;
            break;
    }
    SDL_BlitSurface(text, NULL, s, &r);
    SDL_FreeSurface(text);
}

void draw_circle(SDL_Surface *s, int cx, int cy, int r, Uint32 color) {
    int x = 0;
    int y = r;
    int d = 1 - r;
    if (cx - r >= 0 && cx + r < s->w && cy - r >= 0 && cy + r < s->h) {
        draw_circle_symmetry_points(s, cx, cy, x, y, color, 0);
        while (x < y) {
            x++;
            if (d < 0) {
                d += (x << 1) + 1;
            } else {
                y--;
                d += ((x-y) << 1) + 1;
            }
            draw_circle_symmetry_points(s, cx, cy, x, y, color, 0);
        }
    } else if (cx + r >= 0 && cx - r < s->w && cy + r >= 0 && cy - r < s->h) {
        // circle is partially on surface, must check bounds
        draw_circle_symmetry_points(s, cx, cy, x, y, color, 1);
        while (x < y) {
            x++;
            if (d < 0) {
                d += (x << 1) + 1;
            } else {
                y--;
                d += ((x-y) << 1) + 1;
            }
            draw_circle_symmetry_points(s, cx, cy, x, y, color, 1);
        }
    }
}

void draw_circle_symmetry_points(SDL_Surface *s, int cx, int cy, int x, int y, Uint32 color, int check_bounds) {
    if (check_bounds) {
        if (cx + x >= 0 && cx + x < s->w && cy + y >= 0 && cy + y < s->h) {
            draw_pixel(s, (cx + x), (cy + y), color);
        }
        if (cx + x >= 0 && cx + x < s->w && cy - y >= 0 && cy - y < s->h) {
            draw_pixel(s, (cx + x), (cy - y), color);
        }
        if (cx - x >= 0 && cx - x < s->w && cy + y >= 0 && cy + y < s->h) {
            draw_pixel(s, (cx - x), (cy + y), color);
        }
        if (cx - x >= 0 && cx - x < s->w && cy - y >= 0 && cy - y < s->h) {
            draw_pixel(s, (cx - x), (cy - y), color);
        }
        if (cx + y >= 0 && cx + y < s->w && cy + x >= 0 && cy + x < s->h) {
            draw_pixel(s, (cx + y), (cy + x), color);
        }
        if (cx + y >= 0 && cx + y < s->w && cy - x >= 0 && cy - x < s->h) {
            draw_pixel(s, (cx + y), (cy - x), color);
        }
        if (cx - y >= 0 && cx - y < s->w && cy + x >= 0 && cy + x < s->h) {
            draw_pixel(s, (cx - y), (cy + x), color);
        }
        if (cx - y >= 0 && cx - y < s->w && cy - x >= 0 && cy - x < s->h) {
            draw_pixel(s, (cx - y), (cy - x), color);
        }
    } else {
        draw_pixel(s, (cx + x), (cy + y), color);
        draw_pixel(s, (cx + x), (cy - y), color);
        draw_pixel(s, (cx - x), (cy + y), color);
        draw_pixel(s, (cx - x), (cy - y), color);
        draw_pixel(s, (cx + y), (cy + x), color);
        draw_pixel(s, (cx + y), (cy - x), color);
        draw_pixel(s, (cx - y), (cy + x), color);
        draw_pixel(s, (cx - y), (cy - x), color);
    }
}

void draw_line(SDL_Surface *s, int xi, int yi, int xf, int yf, Uint32 color) {
    char steep = abs(yf - yi) > abs(xf - xi);
    if (steep) {
        int swap;
        swap = xi;
        xi = yi;
        yi = swap;
        swap = xf;
        xf = yf;
        yf = swap;
    }
    if (xi > xf) {
        int swap;
        swap = xi;
        xi = xf;
        xf = swap;
        swap = yi;
        yi = yf;
        yf = swap;
    }
    int dx = xf - xi;
    int dy = abs(yf - yi);
    int error = dx / 2;
    int ystep;
    int y = yi;
    if (yi < yf) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    int x;
    for (x = xi; x <= xf; x++) {
        if (steep) {
            if (x >= 0 && x < s->h && y >= 0 && y < s->w) {
                draw_pixel(s, y, x, color);
            }
        } else {
            if (x >= 0 && x < s->w && y >= 0 && y < s->h) {
                draw_pixel(s, x, y, color);
            }
        }
        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}

void draw_pixel(SDL_Surface *s, int x, int y, Uint32 color) {
    int bpp = s->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)s->pixels + y * s->pitch + x * bpp;

    switch (bpp) {
        case 1:
            *p = color;
            break;

        case 2:
            *(Uint16 *)p = color;
            break;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (color >> 16) & 0xff;
                p[1] = (color >> 8) & 0xff;
                p[2] = color & 0xff;
            }
            else {
                p[0] = color & 0xff;
                p[1] = (color >> 8) & 0xff;
                p[2] = (color >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = color;
            break;

       default:
            break;
    }
}
