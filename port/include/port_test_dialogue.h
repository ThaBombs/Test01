#ifndef GUARD_PORT_TEST_DIALOGUE_H
#define GUARD_PORT_TEST_DIALOGUE_H

#include "gba/types.h"

void PortTestDialogue_Init(void);
bool32 PortTestDialogue_Open(const u8 *text);
bool32 PortTestDialogue_Update(void);
bool32 PortTestDialogue_IsOpen(void);

#endif
