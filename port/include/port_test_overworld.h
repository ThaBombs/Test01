#ifndef GUARD_PORT_TEST_OVERWORLD_H
#define GUARD_PORT_TEST_OVERWORLD_H

#include "gba/types.h"

// The simplified Android renderer keeps the custom player sprite one map row
// above gSaveBlock1Ptr->pos.y. Convert between the camera focus coordinate and
// the tile under the player's feet instead of shifting individual event types.
#define PORT_PLAYER_MAP_Y_BIAS 1

void PortGame_StartTestOverworld(void);
void PortGame_LoadTestMap(u16 mapGroup, u16 mapNum, s16 focusX, s16 focusY);
bool32 PortGame_TryTestWarpAt(s16 x, s16 y);
bool32 PortGame_TryTestCoordEventAt(s16 x, s16 y);
void PortGame_OpenTestOptions(void);
void PortGame_ReturnToTestOverworld(void);

#endif
