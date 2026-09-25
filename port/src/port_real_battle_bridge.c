#include "port_real_battle_bridge.h"
#include "port_test_overworld.h"

#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "main.h"
#include "pokemon.h"
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
    // Create the known first-battle party directly with Emerald's real
    // Pokemon routines. ScriptGiveMon() also performs PC/Pokedex bookkeeping,
    // which is unnecessary here and crosses Android subsystems that are not
    // part of the first-battle runtime yet.
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    CreateRandomMon(
        &gParties[B_TRAINER_PLAYER][0],
        GetStarterPokemon(starterChoice),
        5);
    gPartiesCount[B_TRAINER_PLAYER] = 1;

    gBattleTypeFlags = BATTLE_TYPE_FIRST_BATTLE;
    gMain.savedCallback = PortRealBattle_ReturnFromEngine;

    FreeAllWindowBuffers();
    SetMainCallback2(CB2_InitBattle);
    return TRUE;
}
