#include "global.h"

#include "battle.h"
#include "battle_setup.h"
#include "battle_pyramid_bag.h"
#include "contest.h"
#include "item_menu.h"
#include "item_use.h"
#include "pokeblock.h"
#include "battle_dynamax.h"
#include "battle_z_move.h"
#include "debug.h"
#include "event_data.h"
#include "follower_npc.h"
#include "link.h"
#include "link_rfu.h"
#include "party_menu.h"
#include "safari_zone.h"
#include "sound.h"
#include "constants/moves.h"
#include "constants/battle_pyramid.h"

// Android battle-runtime compatibility shims.
//
// The APK now links Emerald's real battle engine and native-width battle
// scripts. A few optional GBA subsystems are not live on Android yet
// (link/RFU multiplayer, follower battle partners, Safari bookkeeping, audio
// ducking and battle gimmick UI). Keep those edges inert so ordinary
// single-player battles can run through the real engine. Replace each shim
// with its source subsystem as that subsystem is ported.

EWRAM_DATA bool8 gIsDebugBattle = FALSE;

u8 gNumSafariBalls = 0;
enum Item gSpecialVar_ItemId = ITEM_NONE;
u16 gSpecialVar_ContestRank = 0;
enum ContestCategories gSpecialVar_ContestCategory = 0;
struct PyramidBagMenuState gPyramidBagMenuState = {0};

u16 *const gSpecialVars[] =
{
    &gSpecialVar_0x8000,
    &gSpecialVar_0x8001,
    &gSpecialVar_0x8002,
    &gSpecialVar_0x8003,
    &gSpecialVar_0x8004,
    &gSpecialVar_0x8005,
    &gSpecialVar_0x8006,
    &gSpecialVar_0x8007,
    &gSpecialVar_0x8008,
    &gSpecialVar_0x8009,
    &gSpecialVar_0x800A,
    &gSpecialVar_0x800B,
    &gSpecialVar_Facing,
    &gSpecialVar_Result,
    (u16 *)&gSpecialVar_ItemId,
    &gSpecialVar_LastTalked,
    &gSpecialVar_ContestRank,
    (u16 *)&gSpecialVar_ContestCategory,
    &gSpecialVar_MonBoxId,
    &gSpecialVar_MonBoxPos,
    &gSpecialVar_Unused_0x8014,
    (u16 *)&gTrainerBattleParameter.params.opponentA,
};

bool8 gReceivedRemoteLinkPlayers = FALSE;
u16 gBlockRecvBuffer[MAX_RFU_PLAYERS][BLOCK_BUFFER_SIZE / 2];
struct LinkPlayer gLinkPlayers[MAX_RFU_PLAYERS];
u8 gWirelessCommType;


u8 GetMultiplayerId(void)
{
    return 0;
}

u8 BitmaskAllOtherLinkPlayers(void)
{
    return 0;
}

bool8 SendBlock(u8 unused, const void *src, u16 size)
{
    (void)unused;
    (void)src;
    (void)size;
    return FALSE;
}

u8 GetBlockReceivedStatus(void)
{
    return 0;
}

void ResetBlockReceivedFlags(void)
{
}

bool8 IsLinkTaskFinished(void)
{
    return TRUE;
}

void SetLinkStandbyCallback(void)
{
}

void SetCloseLinkCallback(void)
{
}

bool8 IsLinkRfuTaskFinished(void)
{
    return TRUE;
}

bool32 PlayerHasFollowerNPC(void)
{
    return FALSE;
}

bool32 FollowerNPCIsBattlePartner(void)
{
    return FALSE;
}


bool32 GetSafariZoneFlag(void)
{
    return FALSE;
}

struct Pokeblock *SafariZoneGetActivePokeblock(void)
{
    return NULL;
}

s16 PokeblockGetGain(u8 nature, const struct Pokeblock *pokeblock)
{
    (void)nature;
    (void)pokeblock;
    return 0;
}

u8 CurrentBattlePyramidLocation(void)
{
    return PYRAMID_LOCATION_NONE;
}

void ItemUseOutOfBattle_Medicine(u8 taskId)
{
    (void)taskId;
}

void ItemUseOutOfBattle_SacredAsh(u8 taskId)
{
    (void)taskId;
}

void ItemUseOutOfBattle_PPRecovery(u8 taskId)
{
    (void)taskId;
}

void ItemUseOutOfBattle_CannotUse(u8 taskId)
{
    (void)taskId;
}
