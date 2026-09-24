#ifndef GUARD_PORT_TEST_BATTLE_H
#define GUARD_PORT_TEST_BATTLE_H

#include "gba/types.h"

void PortTestBattle_RequestFirstBattle(void);
bool32 PortTestBattle_TryStartPending(void);
bool32 PortTestBattle_IsPending(void);

#endif
