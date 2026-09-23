#include "port_test_overworld.h"

#include "global.h"
#include "main.h"
#include "overworld.h"
#include "constants/maps.h"

void PortGame_StartTestOverworld(void)
{
    // The first Android overworld milestone deliberately avoids the full
    // NewGameInitData dependency graph. The bootstrap save blocks start zeroed,
    // so provide only the player identity state needed by the field loader.
    gSaveBlock2Ptr->playerGender = 0;
    gSaveBlock2Ptr->playerName[0] = 0xFF; // Emerald string terminator

    // WARP_ID_NONE with invalid coordinates makes Emerald choose the center of
    // the destination map, avoiding a fragile hard-coded Littleroot tile.
    SetWarpDestination(
        MAP_GROUP(MAP_LITTLEROOT_TOWN),
        MAP_NUM(MAP_LITTLEROOT_TOWN),
        WARP_ID_NONE,
        -1,
        -1);
    WarpIntoMap();

    ResetInitialPlayerAvatarState();
    gFieldCallback = NULL;
    gFieldCallback2 = NULL;
    SetMainCallback2(CB2_LoadMap);
}
