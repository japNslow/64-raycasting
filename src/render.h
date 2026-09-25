#ifndef RENDER_H
#define RENDER_H

#include "config.h"

void init_renderer(void);
void render_frame(void);
void render_hud(unsigned int score, unsigned char health, unsigned char fps);
void render_fullscreen_map(void);

#endif /* RENDER_H */
