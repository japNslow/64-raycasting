#include <c64.h>
#include "render.h"
#include "raycast.h"
#include "map.h"
#include "tables.h"

/* Precomputed offsets for each cell row: r * 320 */
static const unsigned int cell_row_offset[SCREEN_ROWS] = {
       0,  320,  640,  960, 1280, 1600, 1920, 2240,
    2560, 2880, 3200, 3520, 3840, 4160, 4480, 4800,
    5120, 5440, 5760, 6080, 6400, 6720, 7040, 7360, 7680
};

/* Precomputed screen row offsets: r * 40 */
static const unsigned int screen_row_offset[SCREEN_ROWS] = {
       0,   40,   80,  120,  160,  200,  240,  280,
     320,  360,  400,  440,  480,  520,  560,  600,
     640,  680,  720,  760,  800,  840,  880,  920,  960
};

void init_renderer(void) {
    unsigned int i;

    /* 1. Select VIC-II Bank 1 ($4000-$7FFF) via CIA2 Port A ($DD00) */
    CIA2_PRA = (CIA2_PRA & 0xFC) | 0x02;

    /* 2. Set Screen RAM at $4000 (offset 0), Bitmap at $6000 (offset $2000 in bank) */
    VIC.addr = 0x08;

    /* 3. Enable Bitmap mode (BMM) and Multicolor mode (MCM) */
    VIC.ctrl1 |= 0x20;
    VIC.ctrl2 |= 0x10;

    /* 4. Set Border and Background colors to Black */
    VIC.bordercolor = C64_BLACK;
    VIC.bgcolor0 = C64_BLACK;

    /* 5. Clear Screen RAM ($4000), Color RAM ($D800), and Bitmap RAM ($6000) */
    for (i = 0; i < 1000; ++i) {
        SCREEN_RAM[i] = 0x00;
        COLOR_RAM[i] = 0x00;
    }
    for (i = 0; i < 8000; ++i) {
        BITMAP_RAM[i] = 0x00;
    }
}

void render_frame(void) {
    unsigned char c;

    for (c = 0; c < NUM_RAYS; ++c) {
        unsigned char wall_h = ray_hits[c].height;
        unsigned char half_h = wall_h >> 1;
        signed int top = HALF_BITMAP_HEIGHT - half_h;
        signed int bot = HALF_BITMAP_HEIGHT + half_h;
        unsigned char actual_wall_h;
        unsigned char tile_raw = ray_hits[c].tile;
        unsigned char tile_idx = (tile_raw > 0 && tile_raw <= 5) ? (tile_raw - 1) : 0;
        unsigned char side = ray_hits[c].side;
        unsigned char screen_col;
        unsigned char color_ram_col;
        unsigned int v_step;
        unsigned int v_acc;
        unsigned char r;
        unsigned int col_offset_bmp;

        if (top < 0) top = 0;
        if (bot > 200) bot = 200;
        actual_wall_h = (unsigned char)(bot - top);

        /* Palette for this wall */
        screen_col = side ? tile_color_screen_dark[tile_idx] : tile_color_screen[tile_idx];
        color_ram_col = tile_color_ram[tile_idx];

        v_step = v_step_table[actual_wall_h];
        v_acc = 0;

        col_offset_bmp = (unsigned int)c << 3;

        /* Iterate through 25 cell rows */
        for (r = 0; r < SCREEN_ROWS; ++r) {
            signed int scanline_start = (signed int)(r << 3);
            signed int scanline_end = scanline_start + 7;
            unsigned char* bptr = BITMAP_RAM + cell_row_offset[r] + col_offset_bmp;
            unsigned int cell_offset = screen_row_offset[r] + c;

            if (scanline_end < top) {
                /* Entire cell is Ceiling */
                bptr[0] = 0x00;
                bptr[1] = 0x00;
                bptr[2] = 0x00;
                bptr[3] = 0x00;
                bptr[4] = 0x00;
                bptr[5] = 0x00;
                bptr[6] = 0x00;
                bptr[7] = 0x00;
                SCREEN_RAM[cell_offset] = 0x00;
                COLOR_RAM[cell_offset] = 0x00;
            } else if (scanline_start >= bot) {
                /* Entire cell is Floor */
                unsigned char floor_pat = (r > 20) ? 0x55 : 0x50;
                bptr[0] = floor_pat;
                bptr[1] = 0x00;
                bptr[2] = floor_pat;
                bptr[3] = 0x00;
                bptr[4] = floor_pat;
                bptr[5] = 0x00;
                bptr[6] = floor_pat;
                bptr[7] = 0x00;
                SCREEN_RAM[cell_offset] = (C64_BROWN << 4) | C64_DARKGRAY;
                COLOR_RAM[cell_offset] = C64_BROWN;
            } else {
                /* Cell contains Wall */
                unsigned char line;
                SCREEN_RAM[cell_offset] = screen_col;
                COLOR_RAM[cell_offset] = color_ram_col;

                for (line = 0; line < 8; ++line) {
                    signed int y = scanline_start + line;
                    if (y < top) {
                        bptr[line] = 0x00; /* Ceiling */
                    } else if (y >= bot) {
                        bptr[line] = 0x55; /* Floor */
                    } else {
                        unsigned char v = (unsigned char)(v_acc >> 8);
                        v_acc += v_step;
                        if (v > 15) v = 15;
                        bptr[line] = bmp_textures[tile_idx][v];
                    }
                }
            }
        }
    }
}

void render_fullscreen_map(void) {
    unsigned char mx, my, line;
    signed char px = (signed char)(player_x >> FP_SHIFT);
    signed char py = (signed char)(player_y >> FP_SHIFT);
    unsigned int i;

    /* Clear bitmap to black */
    for (i = 0; i < 8000; ++i) {
        BITMAP_RAM[i] = 0x00;
    }
    for (i = 0; i < 1000; ++i) {
        SCREEN_RAM[i] = 0x00;
        COLOR_RAM[i] = 0x00;
    }

    /* Render 16x16 map centered in 40x25 cells: cols 12..27, rows 4..19 */
    for (my = 0; my < MAP_HEIGHT; ++my) {
        for (mx = 0; mx < MAP_WIDTH; ++mx) {
            unsigned char col = 12 + mx;
            unsigned char row = 4 + my;
            unsigned char t = game_map[my][mx];
            unsigned char* bptr = BITMAP_RAM + cell_row_offset[row] + ((unsigned int)col << 3);
            unsigned int cell_offset = screen_row_offset[row] + col;

            if ((signed char)mx == px && (signed char)my == py) {
                /* Player circle */
                SCREEN_RAM[cell_offset] = (C64_WHITE << 4) | C64_WHITE;
                COLOR_RAM[cell_offset] = C64_WHITE;
                bptr[0] = 0x00;
                bptr[1] = 0x55;
                bptr[2] = 0xFF;
                bptr[3] = 0xFF;
                bptr[4] = 0xFF;
                bptr[5] = 0xFF;
                bptr[6] = 0x55;
                bptr[7] = 0x00;
            } else if (t != TILE_EMPTY) {
                unsigned char tidx = (t > 0 && t <= 5) ? (t - 1) : 0;
                SCREEN_RAM[cell_offset] = tile_color_screen[tidx];
                COLOR_RAM[cell_offset] = tile_color_ram[tidx];
                /* Solid block with border */
                bptr[0] = 0x55;
                for (line = 1; line < 7; ++line) {
                    bptr[line] = 0xAA;
                }
                bptr[7] = 0x55;
            } else {
                /* Floor dot in center */
                SCREEN_RAM[cell_offset] = (C64_DARKGRAY << 4) | C64_BLACK;
                COLOR_RAM[cell_offset] = C64_BLACK;
                bptr[3] = 0x50;
                bptr[4] = 0x50;
            }
        }
    }
}
