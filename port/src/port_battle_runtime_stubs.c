#include "global.h"

#include "battle.h"
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

// Android battle-runtime compatibility shims.
//
// The APK now links Emerald's real battle engine and native-width battle
// scripts. A few optional GBA subsystems are not live on Android yet
// (link/RFU multiplayer, follower battle partners, Safari bookkeeping, audio
// ducking and battle gimmick UI). Keep those edges inert so ordinary
// single-player battles can run through the real engine. Replace each shim
// with its source subsystem as that subsystem is ported.

u8 gNumSafariBalls;
EWRAM_DATA bool8 gIsDebugBattle = FALSE;

bool8 gReceivedRemoteLinkPlayers = FALSE;
u16 gBlockRecvBuffer[MAX_RFU_PLAYERS][BLOCK_BUFFER_SIZE / 2];
struct LinkPlayer gLinkPlayers[MAX_RFU_PLAYERS];
u8 gWirelessCommType;

u16 gSpecialVar_Result;
u8 gSelectedMonPartyId;

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

void ShowPartyMenuToShowcaseMultiBattleParty(void)
{
}

bool32 PlayerHasFollowerNPC(void)
{
    return FALSE;
}

bool32 FollowerNPCIsBattlePartner(void)
{
    return FALSE;
}

void Task_DuckBGMForPokemonCry(u8 taskId)
{
    (void)taskId;
}

enum Move GetMaxMove(enum BattlerId battler, enum Move baseMove)
{
    (void)battler;
    return baseMove;
}

bool32 TryChangeZTrigger(enum BattlerId battler, u32 moveIndex)
{
    (void)battler;
    (void)moveIndex;
    return FALSE;
}

enum Move GetUsableZMove(enum BattlerId battler, enum Move move)
{
    (void)battler;
    (void)move;
    return MOVE_NONE;
}

bool32 MoveSelectionDisplayZMove(enum Move zmove, enum BattlerId battler)
{
    (void)zmove;
    (void)battler;
    return FALSE;
}
