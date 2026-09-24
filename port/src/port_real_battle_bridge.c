#include "port_real_battle_bridge.h"
#include "port_test_overworld.h"

#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "main.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "starter_choose.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/pokemon.h"

static void PortRealBattle_ReturnFromEngine(void)
{
    const bool32 won = (gBattleOutcome == B_OUTCOME_WON);
    PortGame_ReturnFromFirstBattle(won);
}

bool32 PortRealBattle_StartFirstBattle(u8 starterChoice)
{
    if (starterChoice > 2)
        return FALSE;

    // The Android layer only supplies the choice and callback boundary.
    // Party creation, moves, stats and the Birch Zigzagoon are owned by the
    // original Emerald systems.
    ZeroPlayerPartyMons();
    if (ScriptGiveMon(GetStarterPokemon(starterChoice), 5, ITEM_NONE) == MON_CANT_GIVE)
        return FALSE;

    gBattleTypeFlags = BATTLE_TYPE_FIRST_BATTLE;
    gMain.savedCallback = PortRealBattle_ReturnFromEngine;

    FreeAllWindowBuffers();
    SetMainCallback2(CB2_InitBattle);
    return TRUE;
}
