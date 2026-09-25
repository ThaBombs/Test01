#ifndef GUARD_PORT_TEST_NPC_H
#define GUARD_PORT_TEST_NPC_H

#include "gba/types.h"

void PortTestNpc_LoadMap(void);
void PortTestNpc_Update(void);
void PortTestNpc_ApplyCameraDelta(s16 dx, s16 dy);
void PortTestNpc_UpdateMovement(s16 playerX, s16 playerY);
void PortTestNpc_SaveSceneState(void);
void PortTestNpc_ClearSavedSceneState(void);
bool32 PortTestNpc_BlocksTile(s16 x, s16 y);
bool32 PortTestNpc_TryInteractAt(s16 x, s16 y, s16 playerX, s16 playerY);

#endif
