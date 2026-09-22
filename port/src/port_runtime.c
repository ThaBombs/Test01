#include "port_runtime.h"
#include "port_gba_host.h"
#include "port_gba_renderer.h"

#include <stddef.h>

struct PortRuntimeState
{
    struct PortInputState input;
    uint64_t frameCount;
    double elapsedSeconds;
    bool gbaHostReady;
};

struct TouchLayout
{
    int dpadX;
    int dpadY;
    int dpadRadius;
    int dpadDead;
    int aX;
    int aY;
    int bX;
    int bY;
    int faceRadius;
    int lLeft;
    int lTop;
    int lRight;
    int lBottom;
    int rLeft;
    int rTop;
    int rRight;
    int rBottom;
    int selectLeft;
    int selectTop;
    int selectRight;
    int selectBottom;
    int startLeft;
    int startTop;
    int startRight;
    int startBottom;
};

static struct PortRuntimeState sPortState;

static int MinInt(int a, int b)
{
    return a < b ? a : b;
}

static int AbsInt(int value)
{
    return value < 0 ? -value : value;
}

static uint32_t Rgb(uint8_t r, uint8_t g, uint8_t b)
{
    // ANativeWindow RGBX_8888 is stored as R,G,B,X bytes in memory.
    return 0xFF000000u | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
}

static void FillRect(
    uint32_t *pixels,
    int width,
    int height,
    int stridePixels,
    int left,
    int top,
    int right,
    int bottom,
    uint32_t color)
{
    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;
    if (right > width)
        right = width;
    if (bottom > height)
        bottom = height;

    for (int y = top; y < bottom; ++y)
    {
        uint32_t *row = pixels + (size_t)y * (size_t)stridePixels;
        for (int x = left; x < right; ++x)
            row[x] = color;
    }
}

static uint32_t BlendRgbx(uint32_t dst, uint32_t src, uint8_t alpha)
{
    const uint32_t inv = 255u - alpha;
    const uint32_t dr = dst & 0xFFu;
    const uint32_t dg = (dst >> 8) & 0xFFu;
    const uint32_t db = (dst >> 16) & 0xFFu;
    const uint32_t sr = src & 0xFFu;
    const uint32_t sg = (src >> 8) & 0xFFu;
    const uint32_t sb = (src >> 16) & 0xFFu;

    const uint32_t r = (dr * inv + sr * alpha) / 255u;
    const uint32_t g = (dg * inv + sg * alpha) / 255u;
    const uint32_t b = (db * inv + sb * alpha) / 255u;
    return 0xFF000000u | (b << 16) | (g << 8) | r;
}

static void BlendFillRect(
    uint32_t *pixels,
    int width,
    int height,
    int stridePixels,
    int left,
    int top,
    int right,
    int bottom,
    uint32_t color,
    uint8_t alpha)
{
    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;
    if (right > width)
        right = width;
    if (bottom > height)
        bottom = height;

    for (int y = top; y < bottom; ++y)
    {
        uint32_t *row = pixels + (size_t)y * (size_t)stridePixels;
        for (int x = left; x < right; ++x)
            row[x] = BlendRgbx(row[x], color, alpha);
    }
}

static void BlendCircle(
    uint32_t *pixels,
    int width,
    int height,
    int stridePixels,
    int centerX,
    int centerY,
    int radius,
    uint32_t color,
    uint8_t alpha)
{
    const int radiusSquared = radius * radius;
    for (int y = centerY - radius; y <= centerY + radius; ++y)
    {
        if (y < 0 || y >= height)
            continue;

        uint32_t *row = pixels + (size_t)y * (size_t)stridePixels;
        const int dy = y - centerY;
        for (int x = centerX - radius; x <= centerX + radius; ++x)
        {
            if (x < 0 || x >= width)
                continue;

            const int dx = x - centerX;
            if (dx * dx + dy * dy <= radiusSquared)
                row[x] = BlendRgbx(row[x], color, alpha);
        }
    }
}

static struct TouchLayout GetTouchLayout(int width, int height)
{
    const int minDim = MinInt(width, height);
    struct TouchLayout layout = {0};

    layout.dpadX = width * 12 / 100;
    layout.dpadY = height * 70 / 100;
    layout.dpadRadius = minDim / 8;
    layout.dpadDead = layout.dpadRadius / 3;

    layout.faceRadius = minDim / 16;
    layout.aX = width * 88 / 100;
    layout.aY = height * 65 / 100;
    layout.bX = width * 79 / 100;
    layout.bY = height * 76 / 100;

    const int shoulderHalfW = minDim / 10;
    const int shoulderHalfH = minDim / 28;
    layout.lLeft = width * 8 / 100 - shoulderHalfW;
    layout.lRight = width * 8 / 100 + shoulderHalfW;
    layout.rLeft = width * 92 / 100 - shoulderHalfW;
    layout.rRight = width * 92 / 100 + shoulderHalfW;
    layout.lTop = layout.rTop = height * 10 / 100 - shoulderHalfH;
    layout.lBottom = layout.rBottom = height * 10 / 100 + shoulderHalfH;

    const int systemHalfW = minDim / 13;
    const int systemHalfH = minDim / 30;
    const int systemY = height * 91 / 100;
    layout.selectLeft = width * 43 / 100 - systemHalfW;
    layout.selectRight = width * 43 / 100 + systemHalfW;
    layout.startLeft = width * 57 / 100 - systemHalfW;
    layout.startRight = width * 57 / 100 + systemHalfW;
    layout.selectTop = layout.startTop = systemY - systemHalfH;
    layout.selectBottom = layout.startBottom = systemY + systemHalfH;

    return layout;
}

static bool PointInRect(float x, float y, int left, int top, int right, int bottom)
{
    return x >= (float)left && x < (float)right
        && y >= (float)top && y < (float)bottom;
}

static bool PointInCircle(float x, float y, int centerX, int centerY, int radius)
{
    const float dx = x - (float)centerX;
    const float dy = y - (float)centerY;
    return dx * dx + dy * dy <= (float)(radius * radius);
}

uint32_t PortRuntime_ButtonsForTouch(float x, float y, int width, int height)
{
    if (width <= 0 || height <= 0)
        return 0;

    const struct TouchLayout layout = GetTouchLayout(width, height);
    uint32_t buttons = 0;

    const int dx = (int)x - layout.dpadX;
    const int dy = (int)y - layout.dpadY;
    if (AbsInt(dx) <= layout.dpadRadius && AbsInt(dy) <= layout.dpadRadius)
    {
        if (dx < -layout.dpadDead)
            buttons |= PORT_BUTTON_LEFT;
        else if (dx > layout.dpadDead)
            buttons |= PORT_BUTTON_RIGHT;

        if (dy < -layout.dpadDead)
            buttons |= PORT_BUTTON_UP;
        else if (dy > layout.dpadDead)
            buttons |= PORT_BUTTON_DOWN;
    }

    if (PointInCircle(x, y, layout.aX, layout.aY, layout.faceRadius))
        buttons |= PORT_BUTTON_A;
    if (PointInCircle(x, y, layout.bX, layout.bY, layout.faceRadius))
        buttons |= PORT_BUTTON_B;

    if (PointInRect(x, y, layout.lLeft, layout.lTop, layout.lRight, layout.lBottom))
        buttons |= PORT_BUTTON_L;
    if (PointInRect(x, y, layout.rLeft, layout.rTop, layout.rRight, layout.rBottom))
        buttons |= PORT_BUTTON_R;
    if (PointInRect(x, y, layout.selectLeft, layout.selectTop, layout.selectRight, layout.selectBottom))
        buttons |= PORT_BUTTON_SELECT;
    if (PointInRect(x, y, layout.startLeft, layout.startTop, layout.startRight, layout.startBottom))
        buttons |= PORT_BUTTON_START;

    return buttons;
}

static uint8_t GlyphRow(char ch, int row)
{
    static const uint8_t glyphA[7] = {14, 17, 17, 31, 17, 17, 17};
    static const uint8_t glyphB[7] = {30, 17, 17, 30, 17, 17, 30};
    static const uint8_t glyphC[7] = {14, 17, 16, 16, 16, 17, 14};
    static const uint8_t glyphE[7] = {31, 16, 16, 30, 16, 16, 31};
    static const uint8_t glyphL[7] = {16, 16, 16, 16, 16, 16, 31};
    static const uint8_t glyphR[7] = {30, 17, 17, 30, 20, 18, 17};
    static const uint8_t glyphS[7] = {15, 16, 16, 14, 1, 1, 30};
    static const uint8_t glyphT[7] = {31, 4, 4, 4, 4, 4, 4};

    const uint8_t *glyph = NULL;
    switch (ch)
    {
    case 'A': glyph = glyphA; break;
    case 'B': glyph = glyphB; break;
    case 'C': glyph = glyphC; break;
    case 'E': glyph = glyphE; break;
    case 'L': glyph = glyphL; break;
    case 'R': glyph = glyphR; break;
    case 'S': glyph = glyphS; break;
    case 'T': glyph = glyphT; break;
    default: return 0;
    }

    return glyph[row];
}

static int TextPixelWidth(const char *text, int scale)
{
    int count = 0;
    while (text[count] != '\0')
        ++count;
    if (count == 0)
        return 0;
    return (count * 6 - 1) * scale;
}

static void DrawText5x7(
    uint32_t *pixels,
    int width,
    int height,
    int stridePixels,
    int left,
    int top,
    const char *text,
    int scale,
    uint32_t color,
    uint8_t alpha)
{
    int cursorX = left;
    for (const char *p = text; *p != '\0'; ++p)
    {
        for (int row = 0; row < 7; ++row)
        {
            const uint8_t bits = GlyphRow(*p, row);
            for (int col = 0; col < 5; ++col)
            {
                if ((bits & (1u << (4 - col))) == 0)
                    continue;

                BlendFillRect(
                    pixels, width, height, stridePixels,
                    cursorX + col * scale,
                    top + row * scale,
                    cursorX + (col + 1) * scale,
                    top + (row + 1) * scale,
                    color, alpha);
            }
        }
        cursorX += 6 * scale;
    }
}

static void DrawCenteredText(
    uint32_t *pixels,
    int width,
    int height,
    int stridePixels,
    int centerX,
    int centerY,
    const char *text,
    int scale,
    uint32_t color,
    uint8_t alpha)
{
    const int textWidth = TextPixelWidth(text, scale);
    const int textHeight = 7 * scale;
    DrawText5x7(
        pixels, width, height, stridePixels,
        centerX - textWidth / 2,
        centerY - textHeight / 2,
        text, scale, color, alpha);
}

static void DrawTouchControls(uint32_t *pixels, int width, int height, int stridePixels)
{
    const struct TouchLayout layout = GetTouchLayout(width, height);
    const uint32_t idle = Rgb(22, 26, 32);
    const uint32_t active = Rgb(108, 205, 255);
    const uint32_t label = Rgb(245, 248, 252);
    const uint32_t held = sPortState.input.buttons;
    const uint8_t idleAlpha = 145;
    const uint8_t activeAlpha = 205;

    const int arm = layout.dpadRadius;
    const int thick = arm * 2 / 5;

    BlendFillRect(pixels, width, height, stridePixels,
                  layout.dpadX - thick, layout.dpadY - arm,
                  layout.dpadX + thick, layout.dpadY + arm,
                  idle, idleAlpha);
    BlendFillRect(pixels, width, height, stridePixels,
                  layout.dpadX - arm, layout.dpadY - thick,
                  layout.dpadX + arm, layout.dpadY + thick,
                  idle, idleAlpha);

    if (held & PORT_BUTTON_UP)
        BlendFillRect(pixels, width, height, stridePixels,
                      layout.dpadX - thick, layout.dpadY - arm,
                      layout.dpadX + thick, layout.dpadY - layout.dpadDead,
                      active, activeAlpha);
    if (held & PORT_BUTTON_DOWN)
        BlendFillRect(pixels, width, height, stridePixels,
                      layout.dpadX - thick, layout.dpadY + layout.dpadDead,
                      layout.dpadX + thick, layout.dpadY + arm,
                      active, activeAlpha);
    if (held & PORT_BUTTON_LEFT)
        BlendFillRect(pixels, width, height, stridePixels,
                      layout.dpadX - arm, layout.dpadY - thick,
                      layout.dpadX - layout.dpadDead, layout.dpadY + thick,
                      active, activeAlpha);
    if (held & PORT_BUTTON_RIGHT)
        BlendFillRect(pixels, width, height, stridePixels,
                      layout.dpadX + layout.dpadDead, layout.dpadY - thick,
                      layout.dpadX + arm, layout.dpadY + thick,
                      active, activeAlpha);

    BlendCircle(pixels, width, height, stridePixels,
                layout.aX, layout.aY, layout.faceRadius,
                (held & PORT_BUTTON_A) ? active : idle,
                (held & PORT_BUTTON_A) ? activeAlpha : idleAlpha);
    BlendCircle(pixels, width, height, stridePixels,
                layout.bX, layout.bY, layout.faceRadius,
                (held & PORT_BUTTON_B) ? active : idle,
                (held & PORT_BUTTON_B) ? activeAlpha : idleAlpha);
    DrawCenteredText(pixels, width, height, stridePixels,
                     layout.aX, layout.aY, "A", 3, label, 230);
    DrawCenteredText(pixels, width, height, stridePixels,
                     layout.bX, layout.bY, "B", 3, label, 230);

    BlendFillRect(pixels, width, height, stridePixels,
                  layout.lLeft, layout.lTop, layout.lRight, layout.lBottom,
                  (held & PORT_BUTTON_L) ? active : idle,
                  (held & PORT_BUTTON_L) ? activeAlpha : idleAlpha);
    BlendFillRect(pixels, width, height, stridePixels,
                  layout.rLeft, layout.rTop, layout.rRight, layout.rBottom,
                  (held & PORT_BUTTON_R) ? active : idle,
                  (held & PORT_BUTTON_R) ? activeAlpha : idleAlpha);
    DrawCenteredText(pixels, width, height, stridePixels,
                     (layout.lLeft + layout.lRight) / 2,
                     (layout.lTop + layout.lBottom) / 2,
                     "L", 2, label, 230);
    DrawCenteredText(pixels, width, height, stridePixels,
                     (layout.rLeft + layout.rRight) / 2,
                     (layout.rTop + layout.rBottom) / 2,
                     "R", 2, label, 230);

    BlendFillRect(pixels, width, height, stridePixels,
                  layout.selectLeft, layout.selectTop, layout.selectRight, layout.selectBottom,
                  (held & PORT_BUTTON_SELECT) ? active : idle,
                  (held & PORT_BUTTON_SELECT) ? activeAlpha : idleAlpha);
    BlendFillRect(pixels, width, height, stridePixels,
                  layout.startLeft, layout.startTop, layout.startRight, layout.startBottom,
                  (held & PORT_BUTTON_START) ? active : idle,
                  (held & PORT_BUTTON_START) ? activeAlpha : idleAlpha);
    DrawCenteredText(pixels, width, height, stridePixels,
                     (layout.selectLeft + layout.selectRight) / 2,
                     (layout.selectTop + layout.selectBottom) / 2,
                     "SELECT", 1, label, 230);
    DrawCenteredText(pixels, width, height, stridePixels,
                     (layout.startLeft + layout.startRight) / 2,
                     (layout.startTop + layout.startBottom) / 2,
                     "START", 1, label, 230);
}

void PortRuntime_Init(void)
{
    sPortState = (struct PortRuntimeState){0};
    PortGbaHost_Init();
    sPortState.gbaHostReady = PortGbaHost_SelfTest();
}

void PortRuntime_Step(const struct PortInputState *input, double deltaSeconds)
{
    if (input != NULL)
        sPortState.input = *input;

    PortGbaHost_SetButtons(sPortState.input.buttons);

    if (deltaSeconds < 0.0)
        deltaSeconds = 0.0;
    if (deltaSeconds > 0.25)
        deltaSeconds = 0.25;

    sPortState.elapsedSeconds += deltaSeconds;
    ++sPortState.frameCount;
}

void PortRuntime_Render(uint32_t *pixels, int width, int height, int stridePixels)
{
    if (pixels == NULL || width <= 0 || height <= 0 || stridePixels < width)
        return;

    const bool gameRendered =
        PortGbaRenderer_RenderSurface(pixels, width, height, stridePixels);

    if (!gameRendered)
    {
        for (int y = 0; y < height; ++y)
        {
            uint32_t *row = pixels + (size_t)y * (size_t)stridePixels;
            for (int x = 0; x < width; ++x)
                row[x] = Rgb(15, 21, 30);
        }

        const int marker = MinInt(width, height) / 20;
        const int travel = width - marker * 4;
        const int x = marker * 2
            + (travel > 0 ? (int)(sPortState.frameCount % (uint64_t)travel) : 0);
        FillRect(pixels, width, height, stridePixels,
                 x - marker, height / 2 - marker,
                 x + marker, height / 2 + marker,
                 Rgb(255, 199, 82));
    }

    DrawTouchControls(pixels, width, height, stridePixels);
}

uint64_t PortRuntime_GetFrameCount(void)
{
    return sPortState.frameCount;
}

bool PortRuntime_IsGbaHostReady(void)
{
    return sPortState.gbaHostReady;
}
