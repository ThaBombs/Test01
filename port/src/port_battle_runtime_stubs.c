#include "global.h"

#include "battle.h"
#include "data.h"
#include "evolution_scene.h"
#include "frontier_util.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon_storage_system.h"
#include "rtc.h"
#include "trainer.h"
#include "constants/pokedex.h"
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


/*
 * Small exact/support definitions kept here to avoid pulling large unrelated
 * UI objects into the first Android battle closure. Battle mechanics
 * themselves (effects, AI history, Dynamax and Z-Moves) are linked from their
 * real Emerald source modules.
 */

static const union AffineAnimCmd sAndroidAffineAnim_Battler_Normal[] =
{
    AFFINEANIMCMD_FRAME(0x100, 0x100, 0, 0),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_Flipped[] =
{
    AFFINEANIMCMD_FRAME(-0x100, 0x100, 0, 0),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_Emerge[] =
{
    AFFINEANIMCMD_FRAME(0x28, 0x28, 0, 0),
    AFFINEANIMCMD_FRAME(0x12, 0x12, 0, 12),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_Return[] =
{
    AFFINEANIMCMD_FRAME(-0x2, -0x2, 0, 18),
    AFFINEANIMCMD_FRAME(-0x10, -0x10, 0, 15),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_HorizontalSquishLoop[] =
{
    AFFINEANIMCMD_FRAME(0xA0, 0x100, 0, 0),
    AFFINEANIMCMD_FRAME(0x4, 0x0, 0, 8),
    AFFINEANIMCMD_FRAME(-0x4, 0x0, 0, 8),
    AFFINEANIMCMD_JUMP(1),
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_Grow[] =
{
    AFFINEANIMCMD_FRAME(0x2, 0x2, 0, 20),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_Shrink[] =
{
    AFFINEANIMCMD_FRAME(-0x2, -0x2, 0, 20),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_BigToSmall[] =
{
    AFFINEANIMCMD_FRAME(0x100, 0x100, 0, 0),
    AFFINEANIMCMD_FRAME(-0x10, -0x10, 0, 9),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_GrowLarge[] =
{
    AFFINEANIMCMD_FRAME(0x4, 0x4, 0, 63),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_TipRight[] =
{
    AFFINEANIMCMD_FRAME(0x0, 0x0, -3, 5),
    AFFINEANIMCMD_FRAME(0x0, 0x0, 3, 5),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_SpinShrink[] =
{
    AFFINEANIMCMD_FRAME(-0x4, -0x4, 4, 63),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_TipLeft[] =
{
    AFFINEANIMCMD_FRAME(0x0, 0x0, 3, 5),
    AFFINEANIMCMD_FRAME(0x0, 0x0, -3, 5),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_RotateUpAndBack[] =
{
    AFFINEANIMCMD_FRAME(0x0, 0x0, -5, 20),
    AFFINEANIMCMD_FRAME(0x0, 0x0, 0, 20),
    AFFINEANIMCMD_FRAME(0x0, 0x0, 5, 20),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAndroidAffineAnim_Battler_Spin[] =
{
    AFFINEANIMCMD_FRAME(0x0, 0x0, 9, 110),
    AFFINEANIMCMD_END,
};

const union AffineAnimCmd *const gAffineAnims_BattleSpritePlayerSide[] =
{
    [BATTLER_AFFINE_NORMAL] = sAndroidAffineAnim_Battler_Normal,
    [BATTLER_AFFINE_EMERGE] = sAndroidAffineAnim_Battler_Emerge,
    [BATTLER_AFFINE_RETURN] = sAndroidAffineAnim_Battler_Return,
    sAndroidAffineAnim_Battler_HorizontalSquishLoop,
    sAndroidAffineAnim_Battler_Grow,
    sAndroidAffineAnim_Battler_Shrink,
    sAndroidAffineAnim_Battler_GrowLarge,
    sAndroidAffineAnim_Battler_TipRight,
    sAndroidAffineAnim_Battler_BigToSmall,
};

const union AffineAnimCmd *const gAffineAnims_BattleSpriteOpponentSide[] =
{
    [BATTLER_AFFINE_NORMAL] = sAndroidAffineAnim_Battler_Normal,
    [BATTLER_AFFINE_EMERGE] = sAndroidAffineAnim_Battler_Emerge,
    [BATTLER_AFFINE_RETURN] = sAndroidAffineAnim_Battler_Return,
    sAndroidAffineAnim_Battler_HorizontalSquishLoop,
    sAndroidAffineAnim_Battler_Grow,
    sAndroidAffineAnim_Battler_Shrink,
    sAndroidAffineAnim_Battler_SpinShrink,
    sAndroidAffineAnim_Battler_TipLeft,
    sAndroidAffineAnim_Battler_RotateUpAndBack,
    sAndroidAffineAnim_Battler_BigToSmall,
    sAndroidAffineAnim_Battler_Spin,
};

const union AffineAnimCmd *const gAffineAnims_BattleSpriteContest[] =
{
    [BATTLER_AFFINE_NORMAL] = sAndroidAffineAnim_Battler_Flipped,
    [BATTLER_AFFINE_EMERGE] = sAndroidAffineAnim_Battler_Emerge,
    [BATTLER_AFFINE_RETURN] = sAndroidAffineAnim_Battler_Return,
    sAndroidAffineAnim_Battler_HorizontalSquishLoop,
    sAndroidAffineAnim_Battler_Grow,
    sAndroidAffineAnim_Battler_Shrink,
    sAndroidAffineAnim_Battler_SpinShrink,
    sAndroidAffineAnim_Battler_TipLeft,
    sAndroidAffineAnim_Battler_RotateUpAndBack,
    sAndroidAffineAnim_Battler_BigToSmall,
    sAndroidAffineAnim_Battler_Spin,
};

const s8 gPokeblockFlavorCompatibilityTable[NUM_NATURES * FLAVOR_COUNT] =
{
     0,  0,  0,  0,  0,
     1,  0,  0,  0, -1,
     1,  0, -1,  0,  0,
     1, -1,  0,  0,  0,
     1,  0,  0, -1,  0,
    -1,  0,  0,  0,  1,
     0,  0,  0,  0,  0,
     0,  0, -1,  0,  1,
     0, -1,  0,  0,  1,
     0,  0,  0, -1,  1,
    -1,  0,  1,  0,  0,
     0,  0,  1,  0, -1,
     0,  0,  0,  0,  0,
     0, -1,  1,  0,  0,
     0,  0,  1, -1,  0,
    -1,  1,  0,  0,  0,
     0,  1,  0,  0, -1,
     0,  1, -1,  0,  0,
     0,  0,  0,  0,  0,
     0,  1,  0, -1,  0,
    -1,  0,  0,  1,  0,
     0,  0,  0,  1, -1,
     0,  0, -1,  1,  0,
     0, -1,  0,  1,  0,
     0,  0,  0,  0,  0,
};

mapsec_u8_t GetCurrentRegionMapSectionId(void)
{
    return gMapHeader.regionMapSectionId;
}

u16 GetUnownLetterByPersonality(u32 personality)
{
    if (!personality)
        return 0;
    return GET_UNOWN_LETTER(personality);
}

u8 StorageGetCurrentBox(void)
{
    return gPokemonStoragePtr->currentBox;
}

u32 GetBoxMonDataAt(u8 boxId, u8 boxPosition, s32 request)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        return GetBoxMonData(&gPokemonStoragePtr->boxes[boxId][boxPosition], request);
    return 0;
}

struct BoxPokemon *GetBoxedMonPtr(u8 boxId, u8 boxPosition)
{
    if (boxId < TOTAL_BOXES_COUNT && boxPosition < IN_BOX_COUNT)
        return &gPokemonStoragePtr->boxes[boxId][boxPosition];
    return NULL;
}

static u8 sAndroidPCBoxToSendMon;

void SetPCBoxToSendMon(u8 boxId)
{
    sAndroidPCBoxToSendMon = boxId;
}

u8 GetPCBoxToSendMon(void)
{
    return sAndroidPCBoxToSendMon;
}

s8 GetSetPokedexFlag(enum NationalDexOrder nationalDexNo, u8 caseID)
{
    u32 index, bit, mask;
    s8 retVal = 0;

    nationalDexNo--;
    index = nationalDexNo / 8;
    bit = nationalDexNo % 8;
    mask = 1 << bit;

    switch (caseID)
    {
    case FLAG_GET_SEEN:
        retVal = ((gSaveBlock1Ptr->dexSeen[index] & mask) != 0);
        break;
    case FLAG_GET_CAUGHT:
        retVal = ((gSaveBlock1Ptr->dexCaught[index] & mask) != 0);
        break;
    case FLAG_SET_SEEN:
        gSaveBlock1Ptr->dexSeen[index] |= mask;
        break;
    case FLAG_SET_CAUGHT:
        gSaveBlock1Ptr->dexCaught[index] |= mask;
        break;
    }

    return retVal;
}

struct Pokemon *GetFirstLiveMon(void)
{
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_HP) != 0)
            return mon;
    }
    return &gParties[B_TRAINER_PLAYER][0];
}

enum TimeOfDay GetTimeOfDay(void)
{
    return TIME_DAY;
}

const struct Trainer *GetDebugAiTrainer(void)
{
    return NULL;
}

void GetFrontierTrainerName(u8 *dst, u16 trainerId)
{
    (void)trainerId;
    if (dst != NULL)
        dst[0] = EOS;
}

bool8 InBattlePike(void)
{
    return FALSE;
}

void BeginEvolutionScene(struct Pokemon *mon, enum Species postEvoSpecies, bool32 canStopEvo, u8 partyId)
{
    (void)mon;
    (void)postEvoSpecies;
    (void)canStopEvo;
    (void)partyId;
}
