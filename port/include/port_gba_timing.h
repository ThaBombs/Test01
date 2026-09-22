#ifndef POKEEMERALD_PORT_GBA_TIMING_H
#define POKEEMERALD_PORT_GBA_TIMING_H

#include <stdbool.h>

void PortGbaTiming_Init(void);
void PortGbaTiming_BeginVisibleFrame(void);
void PortGbaTiming_EnterVBlank(void);
void PortGbaTiming_LeaveVBlank(void);
void PortGbaTiming_WaitForNextFrame(void);
bool PortGbaTiming_SelfTest(void);

#endif // POKEEMERALD_PORT_GBA_TIMING_H
