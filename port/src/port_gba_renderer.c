#include "port_gba_renderer.h"

#include <stddef.h>
#include <string.h>

#include "gba/types.h"
#include "gba/io_reg.h"

static uint8_t sPriority[PORT_GBA_FRAME_WIDTH * PORT_GBA_FRAME_HEIGHT];
static uint32_t sTestFrame[PORT_GBA_FRAME_WIDTH * PORT_GBA_FRAME_HEIGHT];

static uint16_t Read16(uintptr_t address)
{
    uint16_t value;
    memcpy(&value, (const void *)address, sizeof(value));
    return value;
}

static uint32_t Color555ToRgbx(uint16_t color)
{
    const uint32_t r5 = color & 0x1Fu;
    const uint32_t g5 = (color >> 5) & 0x1Fu;
    const uint32_t b5 = (color >> 10) & 0x1Fu;

    const uint32_t r = (r5 << 3) | (r5 >> 2);
    const uint32_t g = (g5 << 3) | (g5 >> 2);
    const uint32_t b = (b5 << 3) | (b5 >> 2);

    // ANativeWindow RGBX_8888 is R,G,B,X in byte order. On little-endian
    // Android this corresponds to 0xFFBBGGRR as a uint32_t.
    return 0xFF000000u | (b << 16) | (g << 8) | r;
}

static void GetTextBgSize(uint16_t bgcnt, int *widthTiles, int *heightTiles)
{
    switch ((bgcnt >> 14) & 3u)
    {
    default:
    case 0:
        *widthTiles = 32;
        *heightTiles = 32;
        break;
    case 1:
        *widthTiles = 64;
        *heightTiles = 32;
        break;
    case 2:
        *widthTiles = 32;
        *heightTiles = 64;
        break;
    case 3:
        *widthTiles = 64;
        *heightTiles = 64;
        break;
    }
}

static uint16_t GetTextMapEntry(uint16_t bgcnt, int tileX, int tileY)
{
    int widthTiles;
    int heightTiles;
    GetTextBgSize(bgcnt, &widthTiles, &heightTiles);

    tileX %= widthTiles;
    tileY %= heightTiles;
    if (tileX < 0)
        tileX += widthTiles;
    if (tileY < 0)
        tileY += heightTiles;

    const int blockX = tileX / 32;
    const int blockY = tileY / 32;
    const int blocksAcross = widthTiles / 32;
    const int blockIndex = blockY * blocksAcross + blockX;
    const int withinBlock = (tileY & 31) * 32 + (tileX & 31);

    const uintptr_t screenBase =
        VRAM + (uintptr_t)(((bgcnt >> 8) & 0x1Fu) * BG_SCREEN_SIZE);
    return Read16(screenBase
        + (uintptr_t)blockIndex * BG_SCREEN_SIZE
        + (uintptr_t)withinBlock * sizeof(uint16_t));
}

static uint8_t GetTextBgPixel(
    uint16_t bgcnt,
    uint16_t mapEntry,
    int pixelX,
    int pixelY)
{
    const bool is8bpp = (bgcnt & BGCNT_256COLOR) != 0;
    const uint32_t tileIndex = mapEntry & 0x03FFu;

    if ((mapEntry & 0x0400u) != 0)
        pixelX = 7 - pixelX;
    if ((mapEntry & 0x0800u) != 0)
        pixelY = 7 - pixelY;

    const uintptr_t charBase =
        VRAM + (uintptr_t)(((bgcnt >> 2) & 3u) * BG_CHAR_SIZE);

    if (is8bpp)
    {
        const uintptr_t tile = charBase + (uintptr_t)tileIndex * TILE_SIZE_8BPP;
        return *(const uint8_t *)(tile + (uintptr_t)pixelY * 8u + (uintptr_t)pixelX);
    }

    const uintptr_t tile = charBase + (uintptr_t)tileIndex * TILE_SIZE_4BPP;
    const uint8_t packed =
        *(const uint8_t *)(tile + (uintptr_t)pixelY * 4u + (uintptr_t)(pixelX >> 1));
    return (pixelX & 1) ? (packed >> 4) : (packed & 0x0Fu);
}

static void RenderTextBackground(
    int bg,
    uint16_t bgcnt,
    uint32_t *pixels)
{
    const uint8_t priority = bgcnt & 3u;
    const uint16_t hofs = *(volatile uint16_t *)(REG_ADDR_BG0HOFS + (uintptr_t)bg * 4u) & 0x01FFu;
    const uint16_t vofs = *(volatile uint16_t *)(REG_ADDR_BG0VOFS + (uintptr_t)bg * 4u) & 0x01FFu;
    const bool is8bpp = (bgcnt & BGCNT_256COLOR) != 0;

    int widthTiles;
    int heightTiles;
    GetTextBgSize(bgcnt, &widthTiles, &heightTiles);
    const int widthPixels = widthTiles * 8;
    const int heightPixels = heightTiles * 8;

    for (int y = 0; y < PORT_GBA_FRAME_HEIGHT; ++y)
    {
        const int worldY = (y + vofs) % heightPixels;
        const int tileY = worldY >> 3;
        const int pixelY = worldY & 7;

        for (int x = 0; x < PORT_GBA_FRAME_WIDTH; ++x)
        {
            const int worldX = (x + hofs) % widthPixels;
            const int tileX = worldX >> 3;
            const int pixelX = worldX & 7;
            const uint16_t entry = GetTextMapEntry(bgcnt, tileX, tileY);
            const uint8_t colorIndex = GetTextBgPixel(bgcnt, entry, pixelX, pixelY);

            if (colorIndex == 0)
                continue;

            const size_t pos = (size_t)y * PORT_GBA_FRAME_WIDTH + (size_t)x;
            if (priority > sPriority[pos])
                continue;

            uint16_t paletteColor;
            if (is8bpp)
            {
                paletteColor = Read16(BG_PLTT + (uintptr_t)colorIndex * 2u);
            }
            else
            {
                const uint32_t bank = (entry >> 12) & 0x0Fu;
                paletteColor = Read16(
                    BG_PLTT + (uintptr_t)(bank * 16u + colorIndex) * 2u);
            }

            pixels[pos] = Color555ToRgbx(paletteColor);
            sPriority[pos] = priority;
        }
    }
}

static void GetSpriteSize(uint16_t attr0, uint16_t attr1, int *width, int *height)
{
    static const uint8_t sizes[3][4][2] = {
        {{8,8}, {16,16}, {32,32}, {64,64}},
        {{16,8}, {32,8}, {32,16}, {64,32}},
        {{8,16}, {8,32}, {16,32}, {32,64}},
    };

    const unsigned shape = (attr0 >> 14) & 3u;
    const unsigned size = (attr1 >> 14) & 3u;

    if (shape >= 3u)
    {
        *width = 0;
        *height = 0;
        return;
    }

    *width = sizes[shape][size][0];
    *height = sizes[shape][size][1];
}

static uint8_t GetSpritePixel(
    uint16_t attr0,
    uint16_t attr1,
    uint16_t attr2,
    int width,
    int localX,
    int localY)
{
    const bool is8bpp = (attr0 & 0x2000u) != 0;
    const bool affine = (attr0 & 0x0100u) != 0;

    if (!affine)
    {
        if ((attr1 & 0x1000u) != 0)
            localX = width - 1 - localX;

        int height;
        int ignoredWidth;
        GetSpriteSize(attr0, attr1, &ignoredWidth, &height);
        if ((attr1 & 0x2000u) != 0)
            localY = height - 1 - localY;
    }

    const int tileX = localX >> 3;
    const int tileY = localY >> 3;
    const int pixelX = localX & 7;
    const int pixelY = localY & 7;
    uint32_t tileIndex = attr2 & 0x03FFu;

    if ((REG_DISPCNT & DISPCNT_OBJ_1D_MAP) != 0)
    {
        const int tilesAcross = width >> 3;
        const int unitsPerTile = is8bpp ? 2 : 1;
        tileIndex += (uint32_t)(tileY * tilesAcross + tileX) * (uint32_t)unitsPerTile;
    }
    else
    {
        tileIndex += (uint32_t)tileY * 32u + (uint32_t)tileX * (is8bpp ? 2u : 1u);
    }

    const uintptr_t tile = OBJ_VRAM0 + (uintptr_t)tileIndex * TILE_SIZE_4BPP;

    if (is8bpp)
        return *(const uint8_t *)(tile + (uintptr_t)pixelY * 8u + (uintptr_t)pixelX);

    const uint8_t packed =
        *(const uint8_t *)(tile + (uintptr_t)pixelY * 4u + (uintptr_t)(pixelX >> 1));
    return (pixelX & 1) ? (packed >> 4) : (packed & 0x0Fu);
}

static void RenderSprites(uint32_t *pixels)
{
    // Reverse iteration means lower OAM indices win when sprites overlap.
    for (int object = 127; object >= 0; --object)
    {
        const uintptr_t base = OAM + (uintptr_t)object * 8u;
        const uint16_t attr0 = Read16(base);
        const uint16_t attr1 = Read16(base + 2u);
        const uint16_t attr2 = Read16(base + 4u);

        const bool affine = (attr0 & 0x0100u) != 0;
        if (!affine && (attr0 & 0x0200u) != 0)
            continue;

        // Affine transform matrices are a later compatibility milestone.
        // We still render affine objects untransformed so their tiles are
        // visible during bring-up.
        int width;
        int height;
        GetSpriteSize(attr0, attr1, &width, &height);
        if (width == 0 || height == 0)
            continue;

        int originX = attr1 & 0x01FFu;
        int originY = attr0 & 0x00FFu;
        if (originX >= 256)
            originX -= 512;
        if (originY >= 160)
            originY -= 256;

        const uint8_t priority = (attr2 >> 10) & 3u;
        const bool is8bpp = (attr0 & 0x2000u) != 0;
        const uint32_t paletteBank = (attr2 >> 12) & 0x0Fu;

        for (int localY = 0; localY < height; ++localY)
        {
            const int screenY = originY + localY;
            if (screenY < 0 || screenY >= PORT_GBA_FRAME_HEIGHT)
                continue;

            for (int localX = 0; localX < width; ++localX)
            {
                const int screenX = originX + localX;
                if (screenX < 0 || screenX >= PORT_GBA_FRAME_WIDTH)
                    continue;

                const uint8_t colorIndex =
                    GetSpritePixel(attr0, attr1, attr2, width, localX, localY);
                if (colorIndex == 0)
                    continue;

                const size_t pos =
                    (size_t)screenY * PORT_GBA_FRAME_WIDTH + (size_t)screenX;
                if (priority > sPriority[pos])
                    continue;

                uint16_t paletteColor;
                if (is8bpp)
                {
                    paletteColor = Read16(
                        OBJ_PLTT + (uintptr_t)colorIndex * 2u);
                }
                else
                {
                    paletteColor = Read16(
                        OBJ_PLTT
                        + (uintptr_t)(paletteBank * 16u + colorIndex) * 2u);
                }

                pixels[pos] = Color555ToRgbx(paletteColor);
                sPriority[pos] = priority;
            }
        }
    }
}

void PortGbaRenderer_RenderCompat(uint32_t *pixels)
{
    if (pixels == NULL)
        return;

    const uint32_t backdrop = Color555ToRgbx(Read16(BG_PLTT));
    for (size_t i = 0; i < PORT_GBA_FRAME_WIDTH * PORT_GBA_FRAME_HEIGHT; ++i)
    {
        pixels[i] = backdrop;
        sPriority[i] = 4;
    }

    const unsigned mode = REG_DISPCNT & 7u;

    // Text backgrounds. Mode 0 has four; mode 1 keeps BG0/BG1 as text.
    const int textBgCount = mode == 0 ? 4 : (mode == 1 ? 2 : 0);
    for (int priority = 3; priority >= 0; --priority)
    {
        for (int bg = textBgCount - 1; bg >= 0; --bg)
        {
            if ((REG_DISPCNT & (DISPCNT_BG0_ON << bg)) == 0)
                continue;

            const uint16_t bgcnt =
                *(volatile uint16_t *)(REG_ADDR_BG0CNT + (uintptr_t)bg * 2u);
            if ((bgcnt & 3u) != (unsigned)priority)
                continue;

            RenderTextBackground(bg, bgcnt, pixels);
        }
    }

    if ((REG_DISPCNT & DISPCNT_OBJ_ON) != 0)
        RenderSprites(pixels);
}

bool PortGbaRenderer_RenderSurface(uint32_t *pixels, int width, int height, int stridePixels)
{
    if (pixels == NULL || width <= 0 || height <= 0 || stridePixels < width)
        return false;

    // Keep the diagnostic runtime visible until the game has actually begun
    // configuring display state or the backdrop palette.
    if (REG_DISPCNT == 0 && Read16(BG_PLTT) == 0)
        return false;

    PortGbaRenderer_RenderCompat(sTestFrame);

    const uint32_t black = 0xFF000000u;
    for (int y = 0; y < height; ++y)
    {
        uint32_t *row = pixels + (size_t)y * (size_t)stridePixels;
        for (int x = 0; x < width; ++x)
            row[x] = black;
    }

    int destWidth = width;
    int destHeight = (destWidth * PORT_GBA_FRAME_HEIGHT) / PORT_GBA_FRAME_WIDTH;
    if (destHeight > height)
    {
        destHeight = height;
        destWidth = (destHeight * PORT_GBA_FRAME_WIDTH) / PORT_GBA_FRAME_HEIGHT;
    }

    const int offsetX = (width - destWidth) / 2;
    const int offsetY = (height - destHeight) / 2;

    for (int y = 0; y < destHeight; ++y)
    {
        const int srcY = (y * PORT_GBA_FRAME_HEIGHT) / destHeight;
        uint32_t *row =
            pixels + (size_t)(offsetY + y) * (size_t)stridePixels + (size_t)offsetX;

        for (int x = 0; x < destWidth; ++x)
        {
            const int srcX = (x * PORT_GBA_FRAME_WIDTH) / destWidth;
            row[x] = sTestFrame[(size_t)srcY * PORT_GBA_FRAME_WIDTH + (size_t)srcX];
        }
    }

    return true;
}

bool PortGbaRenderer_SelfTest(void)
{
    memset(gPortVram, 0, VRAM_SIZE);
    memset(gPortPaletteRam, 0, PLTT_SIZE);
    memset(gPortOam, 0, OAM_SIZE);

    // Hide every object first.
    for (int object = 0; object < 128; ++object)
    {
        const uintptr_t base = OAM + (uintptr_t)object * 8u;
        *(uint16_t *)base = 0x0200u;
    }

    // BG tile 0: solid palette index 1.
    memset((void *)VRAM, 0x11, TILE_SIZE_4BPP);
    *(uint16_t *)(VRAM + BG_SCREEN_SIZE) = 0;
    *(uint16_t *)BG_PLTT = 0x0000u;
    *(uint16_t *)(BG_PLTT + 2u) = 0x001Fu; // red

    REG_BG0CNT = (1u << 8); // screen base block 1, char base block 0
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;
    REG_DISPCNT = DISPCNT_MODE_0 | DISPCNT_BG0_ON;

    PortGbaRenderer_RenderCompat(sTestFrame);
    const bool backgroundOk =
        sTestFrame[0] == Color555ToRgbx(0x001Fu);

    // OBJ tile 0: solid OBJ palette index 1.
    memset((void *)OBJ_VRAM0, 0x11, TILE_SIZE_4BPP);
    *(uint16_t *)OBJ_PLTT = 0x0000u;
    *(uint16_t *)(OBJ_PLTT + 2u) = 0x03E0u; // green
    *(uint16_t *)OAM = 0x0000u;
    *(uint16_t *)(OAM + 2u) = 0x0000u;
    *(uint16_t *)(OAM + 4u) = 0x0000u;
    REG_DISPCNT = DISPCNT_MODE_0 | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP;

    PortGbaRenderer_RenderCompat(sTestFrame);
    const bool spriteOk =
        sTestFrame[0] == Color555ToRgbx(0x03E0u);

    return backgroundOk && spriteOk;
}
