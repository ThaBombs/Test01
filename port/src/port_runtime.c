#include "port_runtime.h"
#include "port_gba_host.h"
#include "port_gba_renderer.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

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
    int aRadius;
    int bRadius;
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
    int fastLeft;
    int fastTop;
    int fastRight;
    int fastBottom;
};

static struct PortRuntimeState sPortState;

enum
{
    TOUCH_CONTROL_DPAD,
    TOUCH_CONTROL_A,
    TOUCH_CONTROL_B,
    TOUCH_CONTROL_L,
    TOUCH_CONTROL_R,
    TOUCH_CONTROL_SELECT,
    TOUCH_CONTROL_START,
    // Keep this last so older 7-control layout files remain compatible.
    TOUCH_CONTROL_FAST_FORWARD,
    TOUCH_CONTROL_COUNT,
};

struct TouchControlConfig
{
    int xPermille;
    int yPermille;
    int sizePercent;
};

static struct TouchControlConfig sTouchControls[TOUCH_CONTROL_COUNT] =
{
    [TOUCH_CONTROL_DPAD]   = {120, 700, 125},
    [TOUCH_CONTROL_A]      = {880, 650, 125},
    [TOUCH_CONTROL_B]      = {790, 760, 125},
    [TOUCH_CONTROL_L]      = { 80, 100, 125},
    [TOUCH_CONTROL_R]      = {920, 100, 125},
    [TOUCH_CONTROL_SELECT] = {430, 910, 125},
    [TOUCH_CONTROL_START]  = {570, 910, 125},
    [TOUCH_CONTROL_FAST_FORWARD] = {790, 100, 125},
};

static bool sTouchLayoutEditing;
static int sFastForwardMultiplier = 2;
static bool sFastForwardToggleMode;
static bool sFastForwardToggled;
static bool sPreviousFastForwardDown;
static bool sEditorPointerDown;
static bool sEditorDragging;
static int sEditorSelected = TOUCH_CONTROL_DPAD;
static char sTouchLayoutPath[512];
static char sBattleDiagnosticPath[512];
static char sPreviousBattleDiagnostic[32];
static uint64_t sBattleDiagnosticVisibleUntil;

static int ClampInt(int value, int low, int high)
{
    if (value < low)
        return low;
    if (value > high)
        return high;
    return value;
}

static int ScaleTouchSizeFor(int control, int value)
{
    return (value * sTouchControls[control].sizePercent + 50) / 100;
}

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

    layout.dpadX = width * sTouchControls[TOUCH_CONTROL_DPAD].xPermille / 1000;
    layout.dpadY = height * sTouchControls[TOUCH_CONTROL_DPAD].yPermille / 1000;
    layout.dpadRadius = ScaleTouchSizeFor(TOUCH_CONTROL_DPAD, minDim / 8);
    layout.dpadDead = layout.dpadRadius / 3;

    layout.aX = width * sTouchControls[TOUCH_CONTROL_A].xPermille / 1000;
    layout.aY = height * sTouchControls[TOUCH_CONTROL_A].yPermille / 1000;
    layout.bX = width * sTouchControls[TOUCH_CONTROL_B].xPermille / 1000;
    layout.bY = height * sTouchControls[TOUCH_CONTROL_B].yPermille / 1000;
    layout.aRadius = ScaleTouchSizeFor(TOUCH_CONTROL_A, minDim / 16);
    layout.bRadius = ScaleTouchSizeFor(TOUCH_CONTROL_B, minDim / 16);

    const int lHalfW = ScaleTouchSizeFor(TOUCH_CONTROL_L, minDim / 10);
    const int lHalfH = ScaleTouchSizeFor(TOUCH_CONTROL_L, minDim / 28);
    const int rHalfW = ScaleTouchSizeFor(TOUCH_CONTROL_R, minDim / 10);
    const int rHalfH = ScaleTouchSizeFor(TOUCH_CONTROL_R, minDim / 28);
    const int lX = width * sTouchControls[TOUCH_CONTROL_L].xPermille / 1000;
    const int lY = height * sTouchControls[TOUCH_CONTROL_L].yPermille / 1000;
    const int rX = width * sTouchControls[TOUCH_CONTROL_R].xPermille / 1000;
    const int rY = height * sTouchControls[TOUCH_CONTROL_R].yPermille / 1000;
    layout.lLeft = lX - lHalfW;
    layout.lRight = lX + lHalfW;
    layout.rLeft = rX - rHalfW;
    layout.rRight = rX + rHalfW;
    layout.lTop = lY - lHalfH;
    layout.lBottom = lY + lHalfH;
    layout.rTop = rY - rHalfH;
    layout.rBottom = rY + rHalfH;

    const int selectHalfW = ScaleTouchSizeFor(TOUCH_CONTROL_SELECT, minDim / 13);
    const int selectHalfH = ScaleTouchSizeFor(TOUCH_CONTROL_SELECT, minDim / 30);
    const int startHalfW = ScaleTouchSizeFor(TOUCH_CONTROL_START, minDim / 13);
    const int startHalfH = ScaleTouchSizeFor(TOUCH_CONTROL_START, minDim / 30);
    const int selectX = width * sTouchControls[TOUCH_CONTROL_SELECT].xPermille / 1000;
    const int selectY = height * sTouchControls[TOUCH_CONTROL_SELECT].yPermille / 1000;
    const int startX = width * sTouchControls[TOUCH_CONTROL_START].xPermille / 1000;
    const int startY = height * sTouchControls[TOUCH_CONTROL_START].yPermille / 1000;
    layout.selectLeft = selectX - selectHalfW;
    layout.selectRight = selectX + selectHalfW;
    layout.startLeft = startX - startHalfW;
    layout.startRight = startX + startHalfW;
    layout.selectTop = selectY - selectHalfH;
    layout.selectBottom = selectY + selectHalfH;
    layout.startTop = startY - startHalfH;
    layout.startBottom = startY + startHalfH;

    const int fastHalfW = ScaleTouchSizeFor(TOUCH_CONTROL_FAST_FORWARD, minDim / 12);
    const int fastHalfH = ScaleTouchSizeFor(TOUCH_CONTROL_FAST_FORWARD, minDim / 28);
    const int fastX = width * sTouchControls[TOUCH_CONTROL_FAST_FORWARD].xPermille / 1000;
    const int fastY = height * sTouchControls[TOUCH_CONTROL_FAST_FORWARD].yPermille / 1000;
    layout.fastLeft = fastX - fastHalfW;
    layout.fastRight = fastX + fastHalfW;
    layout.fastTop = fastY - fastHalfH;
    layout.fastBottom = fastY + fastHalfH;

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

static void SaveTouchLayout(void)
{
    if (sTouchLayoutPath[0] == '\0')
        return;

    FILE *file = fopen(sTouchLayoutPath, "w");
    if (file == NULL)
        return;

    for (int i = 0; i < TOUCH_CONTROL_COUNT; ++i)
        fprintf(file, "%d %d %d\n",
                sTouchControls[i].xPermille,
                sTouchControls[i].yPermille,
                sTouchControls[i].sizePercent);
    fprintf(file, "FF %d\n", sFastForwardMultiplier);
    fprintf(file, "FFT %d\n", sFastForwardToggleMode ? 1 : 0);
    fclose(file);
}

static void LoadTouchLayout(void)
{
    if (sTouchLayoutPath[0] == '\0')
        return;

    FILE *file = fopen(sTouchLayoutPath, "r");
    if (file == NULL)
        return;

    struct TouchControlConfig loaded[TOUCH_CONTROL_COUNT];
    memcpy(loaded, sTouchControls, sizeof(loaded));

    int loadedCount = 0;
    for (int i = 0; i < TOUCH_CONTROL_COUNT; ++i)
    {
        struct TouchControlConfig candidate;
        if (fscanf(file, "%d %d %d",
                   &candidate.xPermille,
                   &candidate.yPermille,
                   &candidate.sizePercent) != 3)
            break;

        if (candidate.xPermille < 20 || candidate.xPermille > 980
         || candidate.yPermille < 20 || candidate.yPermille > 980
         || candidate.sizePercent < 60 || candidate.sizePercent > 200)
            break;

        loaded[i] = candidate;
        ++loadedCount;
    }

    // The previous format contained seven controls. Preserve those positions
    // and simply use the default FF button when upgrading an existing install.
    const int legacyControlCount = TOUCH_CONTROL_FAST_FORWARD;
    if (loadedCount >= legacyControlCount)
    {
        for (int i = 0; i < loadedCount; ++i)
            sTouchControls[i] = loaded[i];
    }

    if (loadedCount == TOUCH_CONTROL_COUNT)
    {
        char tag[8] = {0};
        int multiplier = 0;
        if (fscanf(file, "%7s %d", tag, &multiplier) == 2
         && strcmp(tag, "FF") == 0
         && multiplier >= 2 && multiplier <= 4)
            sFastForwardMultiplier = multiplier;

        int toggleMode = 0;
        if (fscanf(file, "%7s %d", tag, &toggleMode) == 2
         && strcmp(tag, "FFT") == 0)
            sFastForwardToggleMode = toggleMode != 0;
    }

    fclose(file);
}

void PortRuntime_SetStoragePath(const char *path)
{
    if (path == NULL || path[0] == '\0')
        return;

    snprintf(sTouchLayoutPath, sizeof(sTouchLayoutPath),
             "%s/touch_layout.cfg", path);
    snprintf(sBattleDiagnosticPath, sizeof(sBattleDiagnosticPath),
             "%s/battle_init_stage.txt", path);

    FILE *diagnostic = fopen(sBattleDiagnosticPath, "r");
    if (diagnostic != NULL)
    {
        if (fgets(sPreviousBattleDiagnostic, sizeof(sPreviousBattleDiagnostic), diagnostic) != NULL)
        {
            sPreviousBattleDiagnostic[strcspn(sPreviousBattleDiagnostic, "\r\n")] = '\0';
            sBattleDiagnosticVisibleUntil = sPortState.frameCount + 300;
        }
        fclose(diagnostic);
        // Consume the previous crash breadcrumb. A new battle attempt will
        // write a fresh stage if it crashes again.
        remove(sBattleDiagnosticPath);
    }

    LoadTouchLayout();
}

void PortRuntime_SetBattleDiagnosticStage(const char *stage)
{
    if (stage == NULL || sBattleDiagnosticPath[0] == '\0')
        return;

    FILE *diagnostic = fopen(sBattleDiagnosticPath, "w");
    if (diagnostic == NULL)
        return;
    fputs(stage, diagnostic);
    fputc('\n', diagnostic);
    fclose(diagnostic);
}

void PortRuntime_ClearBattleDiagnosticStage(void)
{
    if (sBattleDiagnosticPath[0] != '\0')
        remove(sBattleDiagnosticPath);
}

void PortRuntime_SetTouchScalePercent(int percent)
{
    percent = ClampInt(percent, 60, 200);
    for (int i = 0; i < TOUCH_CONTROL_COUNT; ++i)
        sTouchControls[i].sizePercent = percent;
    SaveTouchLayout();
}

int PortRuntime_GetTouchScalePercent(void)
{
    return sTouchControls[TOUCH_CONTROL_DPAD].sizePercent;
}

void PortRuntime_BeginTouchLayoutEdit(void)
{
    sTouchLayoutEditing = true;
    sEditorPointerDown = false;
    sEditorDragging = false;
}

void PortRuntime_EndTouchLayoutEdit(void)
{
    if (!sTouchLayoutEditing)
        return;
    sTouchLayoutEditing = false;
    sEditorPointerDown = false;
    sEditorDragging = false;
    SaveTouchLayout();
}

bool PortRuntime_IsTouchLayoutEditing(void)
{
    return sTouchLayoutEditing;
}

static int TouchControlAt(float x, float y, const struct TouchLayout *layout)
{
    if (PointInCircle(x, y, layout->aX, layout->aY, layout->aRadius))
        return TOUCH_CONTROL_A;
    if (PointInCircle(x, y, layout->bX, layout->bY, layout->bRadius))
        return TOUCH_CONTROL_B;
    if (PointInRect(x, y, layout->lLeft, layout->lTop, layout->lRight, layout->lBottom))
        return TOUCH_CONTROL_L;
    if (PointInRect(x, y, layout->rLeft, layout->rTop, layout->rRight, layout->rBottom))
        return TOUCH_CONTROL_R;
    if (PointInRect(x, y, layout->selectLeft, layout->selectTop, layout->selectRight, layout->selectBottom))
        return TOUCH_CONTROL_SELECT;
    if (PointInRect(x, y, layout->startLeft, layout->startTop, layout->startRight, layout->startBottom))
        return TOUCH_CONTROL_START;
    if (PointInRect(x, y, layout->fastLeft, layout->fastTop, layout->fastRight, layout->fastBottom))
        return TOUCH_CONTROL_FAST_FORWARD;
    if (PointInCircle(x, y, layout->dpadX, layout->dpadY, layout->dpadRadius))
        return TOUCH_CONTROL_DPAD;
    return -1;
}

void PortRuntime_TouchEditorPointer(float x, float y, bool down, int width, int height)
{
    if (!sTouchLayoutEditing || width <= 0 || height <= 0)
        return;

    if (!down)
    {
        sEditorPointerDown = false;
        sEditorDragging = false;
        return;
    }

    const bool justPressed = !sEditorPointerDown;
    sEditorPointerDown = true;

    const int chromeY = height * 7 / 100;
    const int chromeHalfW = MinInt(width, height) / 18;
    const int chromeHalfH = MinInt(width, height) / 28;
    const int minusX = width * 43 / 100;
    const int plusX = width * 57 / 100;
    const int okX = width * 90 / 100;

    if (justPressed)
    {
        if (PointInRect(x, y,
                        okX - chromeHalfW, chromeY - chromeHalfH,
                        okX + chromeHalfW, chromeY + chromeHalfH))
        {
            PortRuntime_EndTouchLayoutEdit();
            return;
        }

        if (PointInRect(x, y,
                        minusX - chromeHalfW, chromeY - chromeHalfH,
                        minusX + chromeHalfW, chromeY + chromeHalfH))
        {
            sTouchControls[sEditorSelected].sizePercent =
                ClampInt(sTouchControls[sEditorSelected].sizePercent - 10, 60, 200);
            SaveTouchLayout();
            return;
        }

        if (PointInRect(x, y,
                        plusX - chromeHalfW, chromeY - chromeHalfH,
                        plusX + chromeHalfW, chromeY + chromeHalfH))
        {
            sTouchControls[sEditorSelected].sizePercent =
                ClampInt(sTouchControls[sEditorSelected].sizePercent + 10, 60, 200);
            SaveTouchLayout();
            return;
        }

        const struct TouchLayout layout = GetTouchLayout(width, height);
        const int hit = TouchControlAt(x, y, &layout);
        if (hit >= 0)
        {
            sEditorSelected = hit;
            sEditorDragging = true;
        }
    }

    if (sEditorDragging)
    {
        sTouchControls[sEditorSelected].xPermille =
            ClampInt((int)(x * 1000.0f / (float)width), 20, 980);
        sTouchControls[sEditorSelected].yPermille =
            ClampInt((int)(y * 1000.0f / (float)height), 20, 980);
    }
}

uint32_t PortRuntime_ButtonsForTouch(float x, float y, int width, int height)
{
    if (width <= 0 || height <= 0 || sTouchLayoutEditing)
        return 0;

    const struct TouchLayout layout = GetTouchLayout(width, height);
    uint32_t buttons = 0;

    const int dx = (int)x - layout.dpadX;
    const int dy = (int)y - layout.dpadY;
    const int dist2 = dx * dx + dy * dy;
    const int dpadCaptureRadius = layout.dpadRadius * 145 / 100;
    if (dist2 <= dpadCaptureRadius * dpadCaptureRadius
     && dist2 >= layout.dpadDead * layout.dpadDead)
    {
        // Keep movement active through a generous ring just outside the drawn
        // stick. This makes small thumb drift behave as "keep going" rather
        // than dropping movement the instant the finger crosses the artwork.
        if (AbsInt(dx) > AbsInt(dy))
            buttons |= (dx < 0) ? PORT_BUTTON_LEFT : PORT_BUTTON_RIGHT;
        else
            buttons |= (dy < 0) ? PORT_BUTTON_UP : PORT_BUTTON_DOWN;
    }

    if (PointInCircle(x, y, layout.aX, layout.aY, layout.aRadius))
        buttons |= PORT_BUTTON_A;
    if (PointInCircle(x, y, layout.bX, layout.bY, layout.bRadius))
        buttons |= PORT_BUTTON_B;

    if (PointInRect(x, y, layout.lLeft, layout.lTop, layout.lRight, layout.lBottom))
        buttons |= PORT_BUTTON_L;
    if (PointInRect(x, y, layout.rLeft, layout.rTop, layout.rRight, layout.rBottom))
        buttons |= PORT_BUTTON_R;
    if (PointInRect(x, y, layout.selectLeft, layout.selectTop, layout.selectRight, layout.selectBottom))
        buttons |= PORT_BUTTON_SELECT;
    if (PointInRect(x, y, layout.startLeft, layout.startTop, layout.startRight, layout.startBottom))
        buttons |= PORT_BUTTON_START;
    if (PointInRect(x, y, layout.fastLeft, layout.fastTop, layout.fastRight, layout.fastBottom))
        buttons |= PORT_BUTTON_FAST_FORWARD;

    return buttons;
}

static uint8_t GlyphRow(char ch, int row)
{
    static const uint8_t glyphA[7] = {14, 17, 17, 31, 17, 17, 17};
    static const uint8_t glyphB[7] = {30, 17, 17, 30, 17, 17, 30};
    static const uint8_t glyphC[7] = {14, 17, 16, 16, 16, 17, 14};
    static const uint8_t glyphE[7] = {31, 16, 16, 30, 16, 16, 31};
    static const uint8_t glyphF[7] = {31, 16, 16, 30, 16, 16, 16};
    static const uint8_t glyphK[7] = {17, 18, 20, 24, 20, 18, 17};
    static const uint8_t glyphL[7] = {16, 16, 16, 16, 16, 16, 31};
    static const uint8_t glyphO[7] = {14, 17, 17, 17, 17, 17, 14};
    static const uint8_t glyphR[7] = {30, 17, 17, 30, 20, 18, 17};
    static const uint8_t glyphPlus[7] = {0, 4, 4, 31, 4, 4, 0};
    static const uint8_t glyphMinus[7] = {0, 0, 0, 31, 0, 0, 0};
    static const uint8_t glyphS[7] = {15, 16, 16, 14, 1, 1, 30};
    static const uint8_t glyphT[7] = {31, 4, 4, 4, 4, 4, 4};
    static const uint8_t glyphX[7] = {17, 17, 10, 4, 10, 17, 17};
    static const uint8_t glyph2[7] = {14, 17, 1, 2, 4, 8, 31};
    static const uint8_t glyph3[7] = {30, 1, 1, 14, 1, 1, 30};
    static const uint8_t glyph4[7] = {2, 6, 10, 18, 31, 2, 2};

    const uint8_t *glyph = NULL;
    switch (ch)
    {
    case 'A': glyph = glyphA; break;
    case 'B': glyph = glyphB; break;
    case 'C': glyph = glyphC; break;
    case 'E': glyph = glyphE; break;
    case 'F': glyph = glyphF; break;
    case 'K': glyph = glyphK; break;
    case 'L': glyph = glyphL; break;
    case 'O': glyph = glyphO; break;
    case 'R': glyph = glyphR; break;
    case '+': glyph = glyphPlus; break;
    case '-': glyph = glyphMinus; break;
    case 'S': glyph = glyphS; break;
    case 'T': glyph = glyphT; break;
    case 'X': glyph = glyphX; break;
    case '2': glyph = glyph2; break;
    case '3': glyph = glyph3; break;
    case '4': glyph = glyph4; break;
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

    const int stickRadius = layout.dpadRadius;
    const int knobRadius = stickRadius * 45 / 100;
    int knobX = layout.dpadX;
    int knobY = layout.dpadY;
    const int knobTravel = stickRadius * 48 / 100;

    if (held & PORT_BUTTON_LEFT)
        knobX -= knobTravel;
    else if (held & PORT_BUTTON_RIGHT)
        knobX += knobTravel;
    if (held & PORT_BUTTON_UP)
        knobY -= knobTravel;
    else if (held & PORT_BUTTON_DOWN)
        knobY += knobTravel;

    BlendCircle(pixels, width, height, stridePixels,
                layout.dpadX, layout.dpadY, stickRadius,
                idle, idleAlpha);
    BlendCircle(pixels, width, height, stridePixels,
                knobX, knobY, knobRadius,
                (held & (PORT_BUTTON_UP | PORT_BUTTON_DOWN | PORT_BUTTON_LEFT | PORT_BUTTON_RIGHT)) ? active : idle,
                (held & (PORT_BUTTON_UP | PORT_BUTTON_DOWN | PORT_BUTTON_LEFT | PORT_BUTTON_RIGHT)) ? activeAlpha : 205);

    BlendCircle(pixels, width, height, stridePixels,
                layout.aX, layout.aY, layout.aRadius,
                (held & PORT_BUTTON_A) ? active : idle,
                (held & PORT_BUTTON_A) ? activeAlpha : idleAlpha);
    BlendCircle(pixels, width, height, stridePixels,
                layout.bX, layout.bY, layout.bRadius,
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

    const bool fastForwardActive = PortRuntime_IsFastForwardEnabled();
    BlendFillRect(pixels, width, height, stridePixels,
                  layout.fastLeft, layout.fastTop, layout.fastRight, layout.fastBottom,
                  fastForwardActive ? active : idle,
                  fastForwardActive ? activeAlpha : idleAlpha);
    const char *fastLabel =
        sFastForwardMultiplier == 4 ? "4X"
      : sFastForwardMultiplier == 3 ? "3X"
      : "2X";
    DrawCenteredText(pixels, width, height, stridePixels,
                     (layout.fastLeft + layout.fastRight) / 2,
                     (layout.fastTop + layout.fastBottom) / 2,
                     fastLabel, 2, label, 230);

    if (sTouchLayoutEditing)
    {
        const int chromeY = height * 7 / 100;
        const int chromeHalfW = MinInt(width, height) / 18;
        const int chromeHalfH = MinInt(width, height) / 28;
        const int minusX = width * 43 / 100;
        const int plusX = width * 57 / 100;
        const int okX = width * 90 / 100;

        if (sEditorSelected == TOUCH_CONTROL_DPAD)
            BlendCircle(pixels, width, height, stridePixels, layout.dpadX, layout.dpadY, layout.dpadRadius + 6, active, 70);
        else if (sEditorSelected == TOUCH_CONTROL_A)
            BlendCircle(pixels, width, height, stridePixels, layout.aX, layout.aY, layout.aRadius + 6, active, 70);
        else if (sEditorSelected == TOUCH_CONTROL_B)
            BlendCircle(pixels, width, height, stridePixels, layout.bX, layout.bY, layout.bRadius + 6, active, 70);
        else if (sEditorSelected == TOUCH_CONTROL_L)
            BlendFillRect(pixels, width, height, stridePixels, layout.lLeft - 6, layout.lTop - 6, layout.lRight + 6, layout.lBottom + 6, active, 70);
        else if (sEditorSelected == TOUCH_CONTROL_R)
            BlendFillRect(pixels, width, height, stridePixels, layout.rLeft - 6, layout.rTop - 6, layout.rRight + 6, layout.rBottom + 6, active, 70);
        else if (sEditorSelected == TOUCH_CONTROL_SELECT)
            BlendFillRect(pixels, width, height, stridePixels, layout.selectLeft - 6, layout.selectTop - 6, layout.selectRight + 6, layout.selectBottom + 6, active, 70);
        else if (sEditorSelected == TOUCH_CONTROL_START)
            BlendFillRect(pixels, width, height, stridePixels, layout.startLeft - 6, layout.startTop - 6, layout.startRight + 6, layout.startBottom + 6, active, 70);
        else if (sEditorSelected == TOUCH_CONTROL_FAST_FORWARD)
            BlendFillRect(pixels, width, height, stridePixels, layout.fastLeft - 6, layout.fastTop - 6, layout.fastRight + 6, layout.fastBottom + 6, active, 70);

        BlendFillRect(pixels, width, height, stridePixels,
                      minusX - chromeHalfW, chromeY - chromeHalfH,
                      minusX + chromeHalfW, chromeY + chromeHalfH,
                      idle, 205);
        BlendFillRect(pixels, width, height, stridePixels,
                      plusX - chromeHalfW, chromeY - chromeHalfH,
                      plusX + chromeHalfW, chromeY + chromeHalfH,
                      idle, 205);
        BlendFillRect(pixels, width, height, stridePixels,
                      okX - chromeHalfW, chromeY - chromeHalfH,
                      okX + chromeHalfW, chromeY + chromeHalfH,
                      active, 205);
        DrawCenteredText(pixels, width, height, stridePixels, minusX, chromeY, "-", 2, label, 240);
        DrawCenteredText(pixels, width, height, stridePixels, plusX, chromeY, "+", 2, label, 240);
        DrawCenteredText(pixels, width, height, stridePixels, okX, chromeY, "OK", 2, label, 240);
    }
}

void PortRuntime_Init(void)
{
    sPortState = (struct PortRuntimeState){0};
    sFastForwardMultiplier = 2;
    sFastForwardToggleMode = false;
    sFastForwardToggled = false;
    sPreviousFastForwardDown = false;
    sTouchLayoutEditing = false;
    sEditorPointerDown = false;
    sEditorDragging = false;
    PortGbaHost_Init();
    sPortState.gbaHostReady = PortGbaHost_SelfTest();
}

void PortRuntime_Step(const struct PortInputState *input, double deltaSeconds)
{
    if (input != NULL)
        sPortState.input = *input;

    const bool fastForwardDown =
        (sPortState.input.buttons & PORT_BUTTON_FAST_FORWARD) != 0;
    if (sFastForwardToggleMode
     && fastForwardDown
     && !sPreviousFastForwardDown)
        sFastForwardToggled = !sFastForwardToggled;
    if (!sFastForwardToggleMode)
        sFastForwardToggled = false;
    sPreviousFastForwardDown = fastForwardDown;

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

    if (sPreviousBattleDiagnostic[0] != '\0'
     && sPortState.frameCount < sBattleDiagnosticVisibleUntil)
    {
        const int bannerY = height * 5 / 100;
        const int bannerHalfW = MinInt(width * 3 / 10, 180);
        const int bannerHalfH = 18;
        BlendFillRect(
            pixels, width, height, stridePixels,
            width / 2 - bannerHalfW, bannerY - bannerHalfH,
            width / 2 + bannerHalfW, bannerY + bannerHalfH,
            Rgb(120, 20, 20), 220);
        DrawCenteredText(
            pixels, width, height, stridePixels,
            width / 2, bannerY,
            sPreviousBattleDiagnostic, 2,
            Rgb(255, 255, 255), 255);
    }
}

uint64_t PortRuntime_GetFrameCount(void)
{
    return sPortState.frameCount;
}

bool PortRuntime_IsGbaHostReady(void)
{
    return sPortState.gbaHostReady;
}


bool PortRuntime_IsFastForwardEnabled(void)
{
    if (sFastForwardToggleMode)
        return sFastForwardToggled;
    return (sPortState.input.buttons & PORT_BUTTON_FAST_FORWARD) != 0;
}

bool PortRuntime_GetFastForwardToggleMode(void)
{
    return sFastForwardToggleMode;
}

void PortRuntime_SetFastForwardToggleMode(bool enabled)
{
    sFastForwardToggleMode = enabled;
    sFastForwardToggled = false;
    sPreviousFastForwardDown = false;
    SaveTouchLayout();
}

int PortRuntime_GetFastForwardMultiplier(void)
{
    return sFastForwardMultiplier;
}

void PortRuntime_SetFastForwardMultiplier(int multiplier)
{
    sFastForwardMultiplier = ClampInt(multiplier, 2, 4);
    SaveTouchLayout();
}

void PortRuntime_CycleFastForwardMultiplier(void)
{
    ++sFastForwardMultiplier;
    if (sFastForwardMultiplier > 4)
        sFastForwardMultiplier = 2;
    SaveTouchLayout();
}
