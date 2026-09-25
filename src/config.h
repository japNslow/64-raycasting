#ifndef CONFIG_H
#define CONFIG_H

/* Screen dimensions for C64 */
#define SCREEN_COLS         40
#define SCREEN_ROWS         25
#define VIEW_HEIGHT         19  /* Rows 0..18 for 3D view */
#define HUD_START_ROW       19  /* Rows 19..24 for HUD */
#define HUD_ROWS            6

/* Raycasting settings */
#define FOV_ANGLES          42  /* ~60 degrees in 256-angle circle */
#define HALF_VIEW_HEIGHT    9   /* Center horizon (row 9) */
#define MAX_RAY_STEPS       24  /* Max DDA steps per ray */

/* C64 Memory Locations */
#define SCREEN_RAM          ((unsigned char*)0x0400)
#define COLOR_RAM           ((unsigned char*)0xD800)
#define VIC_BORDER          (*(volatile unsigned char*)0xD020)
#define VIC_BG              (*(volatile unsigned char*)0xD021)
#define JOYSTICK_PORT2      (*(volatile unsigned char*)0xDC00)

/* C64 VIC-II Color codes */
#define C64_BLACK           0
#define C64_WHITE           1
#define C64_RED             2
#define C64_CYAN            3
#define C64_PURPLE          4
#define C64_GREEN           5
#define C64_BLUE            6
#define C64_YELLOW          7
#define C64_ORANGE          8
#define C64_BROWN           9
#define C64_LIGHTRED        10
#define C64_DARKGRAY        11
#define C64_GRAY            12
#define C64_LIGHTGREEN      13
#define C64_LIGHTBLUE       14
#define C64_LIGHTGRAY       15

/* PETSCII Screen codes */
#define CH_SPACE            0x20
#define CH_SOLID            0xA0
#define CH_DITHER1          0x66
#define CH_DITHER2          0x67
#define PETSCII_HLINE       0x40
#define PETSCII_VLINE       0x5D
#define CH_DOT              0x2E

#endif /* CONFIG_H */
