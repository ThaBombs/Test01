#ifndef POKEEMERALD_PORT_GBA_RENDERER_H
#define POKEEMERALD_PORT_GBA_RENDERER_H

#include <stdbool.h>
#include <stdint.h>

#define PORT_GBA_FRAME_WIDTH 240
#define PORT_GBA_FRAME_HEIGHT 160

void PortGbaRenderer_RenderCompat(uint32_t *pixels);
bool PortGbaRenderer_RenderSurface(uint32_t *pixels, int width, int height, int stridePixels);
bool PortGbaRenderer_SelfTest(void);

#endif // POKEEMERALD_PORT_GBA_RENDERER_H
