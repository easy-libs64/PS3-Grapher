#include <tiny3d.h>
#include <ppu-types.h>
#include <stdio.h>
#include <string.h>

#define RGBA(r, g, b, alpha) (r << 24) | (g << 16) | (b << 8) | alpha


void Grapher_Init(f32 start_x, f32 end_x, f32 min_y, f32 max_y, u32 dots_count);
void Grapher_show_axes();
void Grapher_append_Dot(f32 val_y);
void Grapher_show(f32 dot_size, u32 color);
void Grapher_clear();
void Grapher_DeInit();
