#ifndef GUARD_PORT_TEST_OVERWORLD_H
#define GUARD_PORT_TEST_OVERWORLD_H

void PortGame_StartTestOverworld(void);
void PortGame_LoadTestMap(u16 mapGroup, u16 mapNum, s16 focusX, s16 focusY);
bool32 PortGame_TryTestWarpAt(s16 x, s16 y);
void PortGame_OpenTestOptions(void);
void PortGame_ReturnToTestOverworld(void);

#endif
