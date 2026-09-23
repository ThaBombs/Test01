#ifndef GUARD_PORT_TEST_OVERWORLD_H
#define GUARD_PORT_TEST_OVERWORLD_H

#include "gba/types.h"

// The current Android camera/player slice is visually two rows below Emerald's
// raw map-event Y coordinate space. Keep the conversion here so map data stays
// byte-for-byte aligned with the original source.
#define PORT_EVENT_PLAYER_Y_OFFSET 2

void PortGame_StartTestOverworld(void);
void PortGame_LoadTestMap(u16 mapGroup, u16 mapNum, s16 focusX, s16 focusY);
bool32 PortGame_TryTestWarpAt(s16 x, s16 y);
void PortGame_OpenTestOptions(void);
void PortGame_ReturnToTestOverworld(void);

#endif
