#ifndef POKEEMERALD_PORT_RUNTIME_H
#define POKEEMERALD_PORT_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

enum PortButton
{
    PORT_BUTTON_A      = 1u << 0,
    PORT_BUTTON_B      = 1u << 1,
    PORT_BUTTON_SELECT = 1u << 2,
    PORT_BUTTON_START  = 1u << 3,
    PORT_BUTTON_RIGHT  = 1u << 4,
    PORT_BUTTON_LEFT   = 1u << 5,
    PORT_BUTTON_UP     = 1u << 6,
    PORT_BUTTON_DOWN   = 1u << 7,
    PORT_BUTTON_R      = 1u << 8,
    PORT_BUTTON_L      = 1u << 9,
    PORT_BUTTON_FAST_FORWARD = 1u << 10,
};

struct PortInputState
{
    uint32_t buttons;
    float pointerX;
    float pointerY;
    bool pointerDown;
};

void PortRuntime_Init(void);
void PortRuntime_SetStoragePath(const char *path);
void PortRuntime_Step(const struct PortInputState *input, double deltaSeconds);
void PortRuntime_Render(uint32_t *pixels, int width, int height, int stridePixels);
uint32_t PortRuntime_ButtonsForTouch(float x, float y, int width, int height);
void PortRuntime_SetTouchScalePercent(int percent);
int PortRuntime_GetTouchScalePercent(void);
void PortRuntime_BeginTouchLayoutEdit(void);
void PortRuntime_EndTouchLayoutEdit(void);
bool PortRuntime_IsTouchLayoutEditing(void);
void PortRuntime_TouchEditorPointer(float x, float y, bool down, int width, int height);
uint64_t PortRuntime_GetFrameCount(void);
bool PortRuntime_IsGbaHostReady(void);
bool PortRuntime_IsFastForwardEnabled(void);
int PortRuntime_GetFastForwardMultiplier(void);
void PortRuntime_SetFastForwardMultiplier(int multiplier);
void PortRuntime_CycleFastForwardMultiplier(void);

#endif // POKEEMERALD_PORT_RUNTIME_H
