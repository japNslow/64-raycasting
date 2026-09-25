#include <conio.h>
#include "config.h"
#include "fixed.h"
#include "tables.h"
#include "map.h"
#include "raycast.h"
#include "render.h"

#define MOVE_SPEED  45   /* Smooth movement speed in 8.8 fixed units */
#define TURN_SPEED  6    /* Smooth turning speed (~8.5 deg per frame) */

/* SID chip registers for sound effects */
#define SID_V1_FREQ_LO  (*(volatile unsigned char*)0xD400)
#define SID_V1_FREQ_HI  (*(volatile unsigned char*)0xD401)
#define SID_V1_CTRL     (*(volatile unsigned char*)0xD404)
#define SID_V1_AD       (*(volatile unsigned char*)0xD405)
#define SID_V1_SR       (*(volatile unsigned char*)0xD406)
#define SID_VOL         (*(volatile unsigned char*)0xD418)

static unsigned char key_w = 0;
static unsigned char key_s = 0;
static unsigned char key_a = 0;
static unsigned char key_d = 0;
static unsigned char key_q = 0;
static unsigned char key_e = 0;
static unsigned char key_space = 0;
static unsigned char key_m = 0;
static unsigned char key_x = 0;

static void play_step_sound(void) {
    SID_VOL = 15;
    SID_V1_AD = 0x05;     /* Fast attack, short decay */
    SID_V1_SR = 0x00;     /* Zero sustain */
    SID_V1_FREQ_LO = 0x20;
    SID_V1_FREQ_HI = 0x04; /* Low thump sound */
    SID_V1_CTRL = 0x81;    /* Noise waveform + Gate bit on */
}

static void try_move(fixed dx, fixed dy) {
    fixed new_x = player_x + dx;
    fixed new_y = player_y + dy;
    unsigned char tx = (unsigned char)(new_x >> FP_SHIFT);
    unsigned char ty = (unsigned char)(new_y >> FP_SHIFT);
    unsigned char cur_x = (unsigned char)(player_x >> FP_SHIFT);
    unsigned char cur_y = (unsigned char)(player_y >> FP_SHIFT);

    /* Wall sliding collision detection with bounds checking */
    if (tx < MAP_WIDTH && game_map[cur_y][tx] == TILE_EMPTY) {
        player_x = new_x;
    }
    if (ty < MAP_HEIGHT && game_map[ty][cur_x] == TILE_EMPTY) {
        player_y = new_y;
    }
}

/* Fast direct CIA1 keyboard matrix scan for continuous, zero-delay WASD */
static void scan_keyboard(void) {
    unsigned char val;
    unsigned char joy;

    key_w = key_s = key_a = key_d = key_q = key_e = key_space = key_m = key_x = 0;

    /* Row 1 ($FD): W (bit 1), A (bit 2), S (bit 5), E (bit 6) */
    CIA1_PRA = 0xFD;
    val = CIA1_PRB;
    if (!(val & 0x02)) key_w = 1;
    if (!(val & 0x04)) key_a = 1;
    if (!(val & 0x20)) key_s = 1;
    if (!(val & 0x40)) key_e = 1;

    /* Row 2 ($FB): D (bit 2), X (bit 7) */
    CIA1_PRA = 0xFB;
    val = CIA1_PRB;
    if (!(val & 0x04)) key_d = 1;
    if (!(val & 0x80)) key_x = 1;

    /* Row 4 ($EF): M (bit 4) */
    CIA1_PRA = 0xEF;
    val = CIA1_PRB;
    if (!(val & 0x10)) key_m = 1;

    /* Row 7 ($7F): Space (bit 4), Q (bit 6) */
    CIA1_PRA = 0x7F;
    val = CIA1_PRB;
    if (!(val & 0x10)) key_space = 1;
    if (!(val & 0x40)) key_q = 1;

    /* Restore CIA1 Port A */
    CIA1_PRA = 0x7F;

    /* Also check Joystick in Port 2 */
    joy = JOYSTICK_PORT2;
    if (!(joy & 0x01)) key_w = 1;     /* Up */
    if (!(joy & 0x02)) key_s = 1;     /* Down */
    if (!(joy & 0x04)) key_a = 1;     /* Left */
    if (!(joy & 0x08)) key_d = 1;     /* Right */
    if (!(joy & 0x10)) key_space = 1; /* Fire */

    /* KERNAL keyboard buffer fallback */
    while (kbhit()) {
        char ch = cgetc();
        if (ch == 'w' || ch == 'W' || ch == 145) key_w = 1;
        else if (ch == 's' || ch == 'S' || ch == 17) key_s = 1;
        else if (ch == 'a' || ch == 'A' || ch == 157) key_a = 1;
        else if (ch == 'd' || ch == 'D' || ch == 29) key_d = 1;
        else if (ch == 'q' || ch == 'Q') key_q = 1;
        else if (ch == 'e' || ch == 'E') key_e = 1;
        else if (ch == 'm' || ch == 'M') key_m = 1;
        else if (ch == ' ') key_space = 1;
        else if (ch == 'x' || ch == 'X') key_x = 1;
    }
}

int main(void) {
    unsigned char map_mode = 0;
    unsigned char last_key_m = 0;
    unsigned char last_key_space = 0;
    unsigned char running = 1;

    init_renderer();
    init_raycaster();

    /* Clear keyboard buffer */
    while (kbhit()) {
        cgetc();
    }

    while (running) {
        if (map_mode) {
            render_fullscreen_map();
        } else {
            raycast_all();
            render_frame();
        }

        /* Read smooth continuous keyboard and joystick input */
        scan_keyboard();

        if (key_x) {
            running = 0;
            break;
        }

        /* Toggle 3D tactical map */
        if (key_m && !last_key_m) {
            map_mode = !map_mode;
        }
        last_key_m = key_m;

        /* Move Forward (W) */
        if (key_w) {
            fixed dx = (fixed)(((signed long)COS_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
            fixed dy = (fixed)(((signed long)SIN_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
            try_move(dx, dy);
            play_step_sound();
        }

        /* Move Backward (S) */
        if (key_s) {
            fixed dx = (fixed)(((signed long)COS_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
            fixed dy = (fixed)(((signed long)SIN_LOOKUP(player_angle) * MOVE_SPEED) >> FP_SHIFT);
            try_move(-dx, -dy);
            play_step_sound();
        }

        /* Turn Left (A) */
        if (key_a) {
            player_angle = (unsigned char)(player_angle - TURN_SPEED);
        }

        /* Turn Right (D) */
        if (key_d) {
            player_angle = (unsigned char)(player_angle + TURN_SPEED);
        }

        /* Strafe Left (Q) */
        if (key_q) {
            unsigned char strafe_ang = (unsigned char)(player_angle - 64);
            fixed dx = (fixed)(((signed long)COS_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
            fixed dy = (fixed)(((signed long)SIN_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
            try_move(dx, dy);
        }

        /* Strafe Right (E) */
        if (key_e) {
            unsigned char strafe_ang = (unsigned char)(player_angle + 64);
            fixed dx = (fixed)(((signed long)COS_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
            fixed dy = (fixed)(((signed long)SIN_LOOKUP(strafe_ang) * MOVE_SPEED) >> FP_SHIFT);
            try_move(dx, dy);
        }

        /* Open Door (Space) */
        if (key_space && !last_key_space) {
            unsigned char front_ang = player_angle;
            fixed fx = player_x + (fixed)(((signed long)COS_LOOKUP(front_ang) * 300) >> FP_SHIFT);
            fixed fy = player_y + (fixed)(((signed long)SIN_LOOKUP(front_ang) * 300) >> FP_SHIFT);
            unsigned char ftx = (unsigned char)(fx >> FP_SHIFT);
            unsigned char fty = (unsigned char)(fy >> FP_SHIFT);

            if (ftx < MAP_WIDTH && fty < MAP_HEIGHT) {
                if (game_map[fty][ftx] == TILE_DOOR) {
                    game_map[fty][ftx] = TILE_EMPTY;
                    play_step_sound();
                }
            }
        }
        last_key_space = key_space;
    }

    return 0;
}
