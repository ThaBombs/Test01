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

static struct PortRuntimeState sPortState;

static uint32_t Rgb(uint8_t r, uint8_t g, uint8_t b)
{
    // ANativeWindow RGBX_8888 is stored as R,G,B,X bytes in memory.
    // On little-endian Android devices that corresponds to 0xFFBBGGRR.
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

    if (PortGbaRenderer_RenderSurface(pixels, width, height, stridePixels))
        return;

    // This deliberately renders at the Android surface resolution rather than
    // forcing a 240x160 GBA framebuffer. The actual game renderer will be
    // migrated behind this API in later milestones.
    for (int y = 0; y < height; ++y)
    {
        uint32_t *row = pixels + (size_t)y * (size_t)stridePixels;
        uint8_t g = (uint8_t)(22 + (y * 36) / (height > 1 ? height - 1 : 1));
        for (int x = 0; x < width; ++x)
        {
            uint8_t b = (uint8_t)(44 + (x * 42) / (width > 1 ? width - 1 : 1));
            row[x] = Rgb(18, g, b);
        }
    }

    const int margin = width < height ? width / 20 : height / 20;
    const int cardLeft = margin;
    const int cardTop = margin;
    const int cardRight = width - margin;
    const int cardBottom = height - margin;

    FillRect(pixels, width, height, stridePixels,
             cardLeft, cardTop, cardRight, cardBottom, Rgb(24, 32, 43));

    const int line = (height / 120) + 1;
    FillRect(pixels, width, height, stridePixels,
             cardLeft, cardTop, cardRight, cardTop + line, Rgb(109, 211, 255));
    FillRect(pixels, width, height, stridePixels,
             cardLeft, cardBottom - line, cardRight, cardBottom, Rgb(109, 211, 255));

    // Moving native-rendered marker: visible proof that the C loop is running.
    const int travel = (cardRight - cardLeft) - 2 * margin;
    const int markerX = cardLeft + margin
        + (travel > 0 ? (int)(sPortState.frameCount % (uint64_t)travel) : 0);
    const int markerY = cardTop + (cardBottom - cardTop) / 3;
    const int markerSize = margin > 8 ? margin / 2 : 8;
    FillRect(pixels, width, height, stridePixels,
             markerX - markerSize, markerY - markerSize,
             markerX + markerSize, markerY + markerSize,
             Rgb(255, 200, 86));

    // Pointer/touch feedback.
    if (sPortState.input.pointerDown)
    {
        const int px = (int)sPortState.input.pointerX;
        const int py = (int)sPortState.input.pointerY;
        const int touchSize = markerSize * 2;
        FillRect(pixels, width, height, stridePixels,
                 px - touchSize, py - line,
                 px + touchSize, py + line,
                 Rgb(255, 255, 255));
        FillRect(pixels, width, height, stridePixels,
                 px - line, py - touchSize,
                 px + line, py + touchSize,
                 Rgb(255, 255, 255));
    }

    // Eight bars expose the normalized GBA-style input state without coupling
    // the Android layer to the old memory-mapped KEYINPUT register.
    const uint32_t buttonBits[] = {
        PORT_BUTTON_A,
        PORT_BUTTON_B,
        PORT_BUTTON_SELECT,
        PORT_BUTTON_START,
        PORT_BUTTON_RIGHT,
        PORT_BUTTON_LEFT,
        PORT_BUTTON_UP,
        PORT_BUTTON_DOWN,
        PORT_BUTTON_R,
        PORT_BUTTON_L,
    };

    const int barGap = line * 2;
    const int barWidth = ((cardRight - cardLeft) - (11 * barGap)) / 10;
    const int barTop = cardBottom - margin * 2;
    const int barBottom = cardBottom - margin;

    for (int i = 0; i < 10; ++i)
    {
        const int left = cardLeft + barGap + i * (barWidth + barGap);
        const uint32_t color = (sPortState.input.buttons & buttonBits[i])
            ? Rgb(109, 211, 255)
            : Rgb(50, 65, 82);
        FillRect(pixels, width, height, stridePixels,
                 left, barTop, left + barWidth, barBottom, color);
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
