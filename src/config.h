#ifndef CONFIG_H
#define CONFIG_H

/* Full screen 40x25 with 20 fast double-width rays (~25-30 FPS) */
#define SCREEN_COLS         40
#define SCREEN_ROWS         25
#define VIEW_HEIGHT         25
#define HALF_VIEW_HEIGHT    12
#define NUM_RAYS            20  /* 20 rays * 2 columns = 40 screen columns */

/* Raycasting settings */
#define FOV_ANGLES          42  /* ~60 degrees in 256-angle circle */
#define MAX_RAY_STEPS       16  /* 16 steps is plenty for 16x16 map */

/* C64 Memory Locations */
#define SCREEN_RAM          ((unsigned char*)0x0400)
#define COLOR_RAM           ((unsigned char*)0xD800)
#define VIC_BORDER          (*(volatile unsigned char*)0xD020)
#define VIC_BG              (*(volatile unsigned char*)0xD021)
#define JOYSTICK_PORT2      (*(volatile unsigned char*)0xDC00)

/* CIA1 Keyboard Matrix Registers */
#define CIA1_PRA            (*(volatile unsigned char*)0xDC00)
#define CIA1_PRB            (*(volatile unsigned char*)0xDC01)

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

/* PETSCII Screen codes for 3D textures */
#define TEX_SPACE           0x20
#define TEX_SOLID           0xA0
#define TEX_DITHER1         0x66
#define TEX_DITHER2         0x67
#define TEX_HLINE           0x40
#define TEX_VLINE           0x5D
#define TEX_DOT             0x2E
#define TEX_KNOB            0x51
#define TEX_CROSS           0x5A
#define TEX_FLOOR_LINE      0x64
#define TEX_FLOOR_TILE      0x63

#endif /* CONFIG_H */
