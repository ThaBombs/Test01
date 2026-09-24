#include "global.h"
#include "constants/characters.h"

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
#include "battle_pyramid.h"
#include "battle_pyramid_bag.h"
#include "contest.h"
#include "item.h"
#include "item_menu.h"
#include "item_use.h"
#include "mail.h"
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
#include "save.h"
#include "sound.h"
#include "trainer_hill.h"
#include "trainer_tower.h"
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


/* Android-only peripheral compatibility data follows. Core battle sprite/data tables now come from Emerald's real data.c. */

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



/*
 * Link-only support adapters for peripheral Emerald subsystems that are not
 * entered by the Android single-player overworld/battle milestone.
 *
 * These do not replace battle rules. They satisfy data-table/function-pointer
 * edges for field items, multiplayer, Frontier side modes, recorded battles,
 * and contests until those surrounding UIs/modes are ported.
 */

u8 gTimeOfDay = TIME_DAY;
struct ContestResources *gContestResources = NULL;

void UpdateTimeOfDay(bool32 updateBlend)
{
    (void)updateBlend;
    gTimeOfDay = TIME_DAY;
}

#define ANDROID_FIELD_ITEM_STUB(func) \
    void func(u8 taskId)              \
    {                                 \
        ItemUseOutOfBattle_CannotUse(taskId); \
    }

ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_PPUp)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_AbilityCapsule)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_AbilityPatch)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Mint)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_RareCandy)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_DynamaxCandy)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_BlackWhiteFlute)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Repel)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Lure)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_EscapeRope)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Honey)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Mail)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_EvolutionStone)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_FormChange_ConsumedOnUse)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_ExpShare)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_ReduceEV)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_EnigmaBerry)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_TMHM)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_RotomCatalog)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_FormChange)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Fusion)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_ZygardeCube)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Bike)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Rod)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_Itemfinder)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_TownMap)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_PokemonBoxLink)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_CoinCase)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_PowderJar)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_WailmerPail)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_PokeblockCase)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_PokeFlute)
ANDROID_FIELD_ITEM_STUB(ItemUseOutOfBattle_ResetEVs)

#undef ANDROID_FIELD_ITEM_STUB

void InitTrainerTowerBattleStruct(void)
{
}

void FreeTrainerTowerBattleStruct(void)
{
}

void InitTrainerHillBattleStruct(void)
{
}

void FreeTrainerHillBattleStruct(void)
{
}

u8 GetPyramidRunMultiplier(void)
{
    return 1;
}

u8 GetLinkPlayerCount(void)
{
    return 1;
}

s32 GetFronterBrainSymbol(void)
{
    return 0;
}

u32 TryWriteSpecialSaveSector(u8 sector, u8 *src)
{
    (void)sector;
    (void)src;
    return SAVE_STATUS_OK;
}

bool8 ItemIsMail(enum Item itemId)
{
    return GetItemType(itemId) == ITEM_TYPE_MAIL;
}

bool8 IsSpeciesNotUnown(enum Species species)
{
    return species != SPECIES_UNOWN;
}

/*
 * Battle Palace nature-description text lives in map script data. The first
 * Android battle never enters that map/UI, but pokemon.c keeps pointers to the
 * strings in gNaturesInfo. Empty sentinel strings satisfy those dormant
 * references until the full event-script data archive is linked.
 */
const u8 BattleFrontier_Lounge5_Text_NatureGirlAttackHighAttackLow[] = {EOS};
const u8 BattleFrontier_Lounge5_Text_NatureGirlAttackHighDefenseLow[] = {EOS};
const u8 BattleFrontier_Lounge5_Text_NatureGirlAttackHighSupportLow[] = {EOS};
const u8 BattleFrontier_Lounge5_Text_NatureGirlDefenseHighAttackLow[] = {EOS};
const u8 BattleFrontier_Lounge5_Text_NatureGirlDefenseHighDefenseLow[] = {EOS};
const u8 BattleFrontier_Lounge5_Text_NatureGirlDefenseHighSupportLow[] = {EOS};
const u8 BattleFrontier_Lounge5_Text_NatureGirlSupportHighAttackLow[] = {EOS};
const u8 BattleFrontier_Lounge5_Text_NatureGirlSupportHighDefenseLow[] = {EOS};
const u8 BattleFrontier_Lounge5_Text_NatureGirlSupportHighSupportLow[] = {EOS};
