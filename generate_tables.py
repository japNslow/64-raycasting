import math

# Character codes
TEX_SPACE       = 0x20
TEX_SOLID       = 0xA0
TEX_DITHER1     = 0x66
TEX_DITHER2     = 0x67
TEX_HLINE       = 0x40
TEX_VLINE       = 0x5D
TEX_DOT         = 0x2E
TEX_KNOB        = 0x51
TEX_CROSS       = 0x5A
TEX_FLOOR_LINE  = 0x64
TEX_FLOOR_TILE  = 0x63

# C64 Colors
C64_BLACK       = 0
C64_WHITE       = 1
C64_RED         = 2
C64_CYAN        = 3
C64_PURPLE      = 4
C64_GREEN       = 5
C64_BLUE        = 6
C64_YELLOW      = 7
C64_ORANGE      = 8
C64_BROWN       = 9
C64_LIGHTRED    = 10
C64_DARKGRAY    = 11
C64_GRAY        = 12
C64_LIGHTGREEN  = 13
C64_LIGHTBLUE   = 14
C64_LIGHTGRAY   = 15

with open('src/tables.c', 'w', encoding='utf-8') as f:
    f.write('/* Generated LUT tables for ultra-fast C64 Raycasting */\n')
    f.write('#include "tables.h"\n\n')

    # 1. Sin table
    f.write('const signed int sin_table[256] = {\n')
    for i in range(256):
        angle = 2.0 * math.pi * i / 256.0
        val = int(round(math.sin(angle) * 256.0))
        if i % 8 == 0: f.write('    ')
        f.write(f'{val:6d},')
        if i % 8 == 7: f.write('\n')
    f.write('};\n\n')

    # 2. Delta table (delta_x = 256 / |cos(a)|)
    # Clamp to 2048 to prevent 16-bit multiplication overflow
    f.write('const unsigned int delta_x_table[256] = {\n')
    for i in range(256):
        angle = 2.0 * math.pi * i / 256.0
        ca = abs(math.cos(angle))
        if ca < 0.05:
            d = 2048
        else:
            d = int(round(256.0 / ca))
            if d > 2048: d = 2048
        if i % 8 == 0: f.write('    ')
        f.write(f'{d:6d},')
        if i % 8 == 7: f.write('\n')
    f.write('};\n\n')

    # 3. Ray angle offsets for 20 rays (double-width columns across 40-col screen)
    fov_deg = 60.0
    ray_offsets = []
    cos_rays = []
    for i in range(20):
        camera_x = 2.0 * i / 19.0 - 1.0
        ray_ang_rad = math.atan(camera_x * math.tan(math.radians(fov_deg / 2.0)))
        ray_ang_256 = int(round(ray_ang_rad * 256.0 / (2.0 * math.pi)))
        ray_offsets.append(ray_ang_256)
        cos_rays.append(int(round(math.cos(ray_ang_rad) * 256.0)))

    f.write('const signed char ray_angle_offset[NUM_RAYS] = {\n    ')
    for i, ro in enumerate(ray_offsets):
        f.write(f'{ro:4d},')
        if i % 10 == 9 and i != 19: f.write('\n    ')
    f.write('\n};\n\n')

    f.write('const unsigned char cos_ray_table[NUM_RAYS] = {\n    ')
    for i, cr in enumerate(cos_rays):
        val = min(255, cr)
        f.write(f'{val:4d},')
        if i % 10 == 9 and i != 19: f.write('\n    ')
    f.write('\n};\n\n')

    # 4. Height table (for 25 rows)
    f.write('const unsigned char height_table[256] = {\n')
    for dist in range(256):
        if dist < 6:
            h = 25
        else:
            h = int(round(480.0 / float(dist)))
            if h > 25: h = 25
            if h < 1: h = 1
        if dist % 8 == 0: f.write('    ')
        f.write(f'{h:4d},')
        if dist % 8 == 7: f.write('\n')
    f.write('};\n\n')

    # 5. v_step_table: (8 << 8) / wall_h for wall_h from 0 to 25
    f.write('const unsigned int v_step_table[26] = {\n    ')
    for h in range(26):
        step = 0 if h == 0 else int(round((8 * 256) / h))
        f.write(f'{step:5d},')
        if h % 8 == 7 and h != 25: f.write('\n    ')
    f.write('\n};\n\n')

    # 6. Precomputed textures: 5 tiles x 8 rows x 4 cols
    # Tile 0: Grey Stone
    # Tile 1: Blue Stone with Gold Medallion
    # Tile 2: Wood Paneling
    # Tile 3: Red Brick
    # Tile 4: Steel Door with Brass Knob
    tex_chars = [
        # Tile 0: Grey Stone
        [
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE],
            [TEX_SOLID, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_SOLID, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE]
        ],
        # Tile 1: Blue Stone
        [
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_VLINE, TEX_CROSS, TEX_CROSS, TEX_VLINE],
            [TEX_VLINE, TEX_CROSS, TEX_CROSS, TEX_VLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE]
        ],
        # Tile 2: Wood
        [
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE],
            [TEX_VLINE, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_VLINE, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_VLINE, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_VLINE, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_VLINE, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_VLINE, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE]
        ],
        # Tile 3: Red Brick
        [
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_SOLID],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_SOLID],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_SOLID],
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE],
            [TEX_SOLID, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_SOLID, TEX_SOLID, TEX_VLINE, TEX_SOLID],
            [TEX_SOLID, TEX_SOLID, TEX_VLINE, TEX_SOLID]
        ],
        # Tile 4: Steel Door
        [
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_VLINE, TEX_CROSS, TEX_CROSS, TEX_VLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_VLINE, TEX_SOLID, TEX_KNOB,  TEX_VLINE],
            [TEX_VLINE, TEX_SOLID, TEX_SOLID, TEX_VLINE],
            [TEX_VLINE, TEX_CROSS, TEX_CROSS, TEX_VLINE],
            [TEX_HLINE, TEX_HLINE, TEX_HLINE, TEX_HLINE]
        ]
    ]

    tex_colors = [
        # Tile 0: Grey Stone
        [
            [C64_WHITE,    C64_WHITE,    C64_LIGHTGRAY, C64_DARKGRAY],
            [C64_WHITE,    C64_LIGHTGRAY,C64_LIGHTGRAY, C64_DARKGRAY],
            [C64_WHITE,    C64_LIGHTGRAY,C64_LIGHTGRAY, C64_DARKGRAY],
            [C64_WHITE,    C64_LIGHTGRAY,C64_LIGHTGRAY, C64_DARKGRAY],
            [C64_DARKGRAY, C64_DARKGRAY, C64_DARKGRAY,  C64_DARKGRAY],
            [C64_LIGHTGRAY,C64_LIGHTGRAY,C64_DARKGRAY,  C64_GRAY],
            [C64_LIGHTGRAY,C64_LIGHTGRAY,C64_DARKGRAY,  C64_GRAY],
            [C64_DARKGRAY, C64_DARKGRAY, C64_DARKGRAY,  C64_DARKGRAY]
        ],
        # Tile 1: Blue Stone
        [
            [C64_BLUE,     C64_BLUE,     C64_BLUE,      C64_BLUE],
            [C64_BLUE,     C64_LIGHTBLUE,C64_LIGHTBLUE, C64_BLUE],
            [C64_BLUE,     C64_LIGHTBLUE,C64_LIGHTBLUE, C64_BLUE],
            [C64_BLUE,     C64_YELLOW,   C64_YELLOW,    C64_BLUE],
            [C64_BLUE,     C64_YELLOW,   C64_YELLOW,    C64_BLUE],
            [C64_BLUE,     C64_LIGHTBLUE,C64_LIGHTBLUE, C64_BLUE],
            [C64_BLUE,     C64_LIGHTBLUE,C64_LIGHTBLUE, C64_BLUE],
            [C64_BLUE,     C64_BLUE,     C64_BLUE,      C64_BLUE]
        ],
        # Tile 2: Wood
        [
            [C64_BROWN,    C64_BROWN,    C64_BROWN,     C64_BROWN],
            [C64_BROWN,    C64_ORANGE,   C64_BROWN,     C64_ORANGE],
            [C64_BROWN,    C64_ORANGE,   C64_BROWN,     C64_ORANGE],
            [C64_BROWN,    C64_ORANGE,   C64_BROWN,     C64_ORANGE],
            [C64_BROWN,    C64_ORANGE,   C64_BROWN,     C64_ORANGE],
            [C64_BROWN,    C64_ORANGE,   C64_BROWN,     C64_ORANGE],
            [C64_BROWN,    C64_ORANGE,   C64_BROWN,     C64_ORANGE],
            [C64_BROWN,    C64_BROWN,    C64_BROWN,     C64_BROWN]
        ],
        # Tile 3: Red Brick
        [
            [C64_WHITE,    C64_WHITE,    C64_WHITE,     C64_WHITE],
            [C64_WHITE,    C64_LIGHTRED, C64_LIGHTRED,  C64_LIGHTRED],
            [C64_WHITE,    C64_LIGHTRED, C64_LIGHTRED,  C64_LIGHTRED],
            [C64_WHITE,    C64_LIGHTRED, C64_LIGHTRED,  C64_LIGHTRED],
            [C64_WHITE,    C64_WHITE,    C64_WHITE,     C64_WHITE],
            [C64_LIGHTRED, C64_LIGHTRED, C64_WHITE,     C64_LIGHTRED],
            [C64_LIGHTRED, C64_LIGHTRED, C64_WHITE,     C64_LIGHTRED],
            [C64_LIGHTRED, C64_LIGHTRED, C64_WHITE,     C64_LIGHTRED]
        ],
        # Tile 4: Steel Door
        [
            [C64_LIGHTGRAY,C64_LIGHTGRAY,C64_LIGHTGRAY, C64_LIGHTGRAY],
            [C64_LIGHTGRAY,C64_DARKGRAY, C64_DARKGRAY,  C64_LIGHTGRAY],
            [C64_LIGHTGRAY,C64_GRAY,     C64_GRAY,      C64_LIGHTGRAY],
            [C64_LIGHTGRAY,C64_DARKGRAY, C64_DARKGRAY,  C64_LIGHTGRAY],
            [C64_LIGHTGRAY,C64_DARKGRAY, C64_YELLOW,    C64_LIGHTGRAY],
            [C64_LIGHTGRAY,C64_DARKGRAY, C64_DARKGRAY,  C64_LIGHTGRAY],
            [C64_LIGHTGRAY,C64_GRAY,     C64_GRAY,      C64_LIGHTGRAY],
            [C64_LIGHTGRAY,C64_LIGHTGRAY,C64_LIGHTGRAY, C64_LIGHTGRAY]
        ]
    ]

    f.write('const unsigned char tex_chars[5][8][4] = {\n')
    for t in range(5):
        f.write('    { /* Tile ' + str(t+1) + ' */\n')
        for v in range(8):
            f.write('        { ' + ', '.join(f'0x{c:02X}' for c in tex_chars[t][v]) + ' },\n')
        f.write('    },\n')
    f.write('};\n\n')

    f.write('const unsigned char tex_colors[5][8][4] = {\n')
    for t in range(5):
        f.write('    { /* Tile ' + str(t+1) + ' */\n')
        for v in range(8):
            f.write('        { ' + ', '.join(f'{c:2d}' for c in tex_colors[t][v]) + ' },\n')
        f.write('    },\n')
    f.write('};\n\n')

    # 7. dark_colors table (for Y-side shading)
    dark_colors = [
        0,   # 0 Black -> Black
        15,  # 1 White -> Light Gray
        0,   # 2 Red -> Black
        6,   # 3 Cyan -> Blue
        0,   # 4 Purple -> Black
        0,   # 5 Green -> Black
        0,   # 6 Blue -> Black
        8,   # 7 Yellow -> Orange
        9,   # 8 Orange -> Brown
        0,   # 9 Brown -> Black
        2,   # 10 Light Red -> Red
        0,   # 11 Dark Gray -> Black
        11,  # 12 Medium Gray -> Dark Gray
        5,   # 13 Light Green -> Green
        6,   # 14 Light Blue -> Blue
        12   # 15 Light Gray -> Medium Gray
    ]
    f.write('const unsigned char dark_colors[16] = {\n    ' + ', '.join(f'{c:2d}' for c in dark_colors) + '\n};\n\n')

    # 8. fog_colors table (for distance fading)
    fog_colors = [
        0, 11, 0, 6, 0, 0, 0, 9, 9, 0, 2, 0, 11, 0, 6, 11
    ]
    f.write('const unsigned char fog_colors[16] = {\n    ' + ', '.join(f'{c:2d}' for c in fog_colors) + '\n};\n')

print('Generated ultra-fast tables.c successfully')
