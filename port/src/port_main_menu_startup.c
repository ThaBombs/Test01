#include "port_main_menu_startup.h"

#include "global.h"
#include "load_save.h"
#include "main.h"
#include "main_menu.h"
#include "malloc.h"
#include "new_game.h"
#include "save.h"
#include "sound.h"

void PortGame_StartMainMenu(void)
{
    // The legacy copyright-screen boot callback used to perform this work
    // after its presentation finished. Android skips that presentation, but
    // still needs the game-state/save initialization before showing the menu.
    SetSaveBlocksPointers(GetSaveBlocksPointersBaseOffset());
    ResetMenuAndMonGlobals();
    Save_ResetSaveCounters();
    LoadGameSave(SAVE_NORMAL);

    if (gSaveFileStatus == SAVE_STATUS_EMPTY
     || gSaveFileStatus == SAVE_STATUS_CORRUPT)
        Sav2_ClearSetDefault();

    SetPokemonCryStereo(gSaveBlock2Ptr->optionsSound);
    InitHeap(gHeap, HEAP_SIZE);

    SetMainCallback2(CB2_InitMainMenu);
}
