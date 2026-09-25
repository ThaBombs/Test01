#ifndef GUARD_PORT_TEST_PLAYER_H
#define GUARD_PORT_TEST_PLAYER_H

#include "gba/types.h"

void PortTestPlayer_Init(void);
void PortTestPlayer_Update(void);
void PortTestPlayer_BeginWarpExitStep(u8 direction);
void PortTestPlayer_SetFacingDirection(u8 direction);

#endif
