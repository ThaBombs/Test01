#include "global.h"
#include "constants/characters.h"

#include "battle.h"
#include "battle_factory.h"
#include "battle_main.h"
#include "data.h"
#include "evolution_scene.h"
#include "frontier_util.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "naming_screen.h"
#include "load_save.h"
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
#include "menu_helpers.h"
#include "pokeblock.h"
#include "battle_dynamax.h"
#include "battle_z_move.h"
#include "debug.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "follower_npc.h"
#include "link.h"
#include "link_rfu.h"
#include "party_menu.h"
#include "safari_zone.h"
#include "save.h"
#include "sound.h"
#include "string_util.h"
#include "trainer_hill.h"
#include "trainer_tower.h"
#include "tv.h"
#include "constants/moves.h"
#include "constants/battle_pyramid.h"
#include "constants/map_types.h"
#include "constants/trainers.h"

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
 * Remaining non-core battle support for the Android milestone.
 *
 * Shared arithmetic, money and line-breaking helpers are linked from Emerald
 * directly. The definitions below are only for surrounding modes/UIs that the
 * first Android battle cannot enter yet: Frontier variants, e-Reader, contest,
 * follower battles, caught-mon naming/storage UI and overworld follower
 * metadata carried by gSpeciesInfo.
 */

EWRAM_DATA u64 gDebugAIFlags = 0;

const struct OamData gObjectEventBaseOam_32x32 = {0};
const struct OamData gObjectEventBaseOam_64x64 = {0};
const struct SubspriteTable sOamTables_32x32[] = {{0}};
const struct SubspriteTable sOamTables_64x64[] = {{0}};
const union AnimCmd *const sAnimTable_Following[] = {NULL};
const union AnimCmd *const sAnimTable_Following_Asym[] = {NULL};

const struct CompressedSpriteSheet gSpriteSheet_CategoryIcons = {0};
const struct SpritePalette gSpritePal_CategoryIcons = {0};

enum MapBattleScene GetCurrentMapBattleScene(void)
{
    return MAP_BATTLE_SCENE_NORMAL;
}

bool32 IsNPCFollowerWildBattle(void)
{
    return FALSE;
}

void LoadContestBgAfterMoveAnim(void)
{
}

u64 GetAiScriptsInBattleFactory(void)
{
    return 0;
}

u16 GetBattlePyramidPickupItemId(void)
{
    return ITEM_NONE;
}

void IncrementGameStat(u8 statId)
{
    (void)statId;
}

bool32 CanThrowBall(void)
{
    return (gBattleTypeFlags & BATTLE_TYPE_TRAINER) == 0;
}

void CreateBattlerSprite(enum BattlerId battler)
{
    (void)battler;
}

u8 GetMoveSlotToReplace(void)
{
    return MAX_MON_MOVES;
}

void ShowSelectMovePokemonSummaryScreen(struct Pokemon *mons, u8 monIndex, void (*callback)(void), u16 newMove)
{
    (void)mons;
    (void)monIndex;
    (void)newMove;
    if (callback != NULL)
        SetMainCallback2(callback);
}

void ReshowBattleScreenAfterMenu(void)
{
    SetMainCallback2(BattleMainCB2);
}

void ReshowBlankBattleScreenAfterMenu(void)
{
    SetMainCallback2(BattleMainCB2);
}

void SavePlayerParty(void)
{
}

void LoadPlayerParty(void)
{
}

static u8 sAndroidBoxName[] = {EOS};

u8 *GetBoxNamePtr(u8 boxId)
{
    (void)boxId;
    return sAndroidBoxName;
}

bool32 ShouldShowBoxWasFullMessage(void)
{
    return FALSE;
}

u8 DisplayCaughtMonDexPage(enum Species species, bool32 isShiny, u32 personality)
{
    (void)species;
    (void)isShiny;
    (void)personality;
    return 0;
}

void DoNamingScreen(u8 templateNum, u8 *destBuffer, u16 monSpeciesOrPlayerGender, u16 monGender, u32 monPersonality, MainCallback returnCallback)
{
    (void)templateNum;
    (void)destBuffer;
    (void)monSpeciesOrPlayerGender;
    (void)monGender;
    (void)monPersonality;
    if (returnCallback != NULL)
        SetMainCallback2(returnCallback);
}

void CopyFrontierBrainTrainerName(u8 *dst)
{
    if (dst != NULL)
        dst[0] = EOS;
}

void CopyFrontierTrainerText(u8 whichText, u16 trainerId)
{
    (void)whichText;
    (void)trainerId;
    gStringVar4[0] = EOS;
}

enum TrainerClassID GetFrontierBrainTrainerClass(void)
{
    return TRAINER_CLASS_PKMN_TRAINER_1;
}

enum TrainerClassID GetFrontierOpponentClass(u16 trainerId)
{
    (void)trainerId;
    return TRAINER_CLASS_PKMN_TRAINER_1;
}

void GetTrainerTowerOpponentWinText(u8 *dest, u8 opponentIdx)
{
    (void)opponentIdx;
    if (dest != NULL)
        dest[0] = EOS;
}

void GetTrainerTowerOpponentLoseText(u8 *dest, u8 opponentIdx)
{
    (void)opponentIdx;
    if (dest != NULL)
        dest[0] = EOS;
}

void GetTrainerTowerOpponentName(u8 *text)
{
    if (text != NULL)
        text[0] = EOS;
}

u8 GetTrainerTowerOpponentClass(void)
{
    return TRAINER_CLASS_PKMN_TRAINER_1;
}

void CopyTrainerHillTrainerText(u8 which, u16 trainerId)
{
    (void)which;
    (void)trainerId;
    gStringVar4[0] = EOS;
}

void GetTrainerHillTrainerName(u8 *dst, u16 trainerId)
{
    (void)trainerId;
    if (dst != NULL)
        dst[0] = EOS;
}

enum TrainerClassID GetTrainerHillOpponentClass(u16 trainerId)
{
    (void)trainerId;
    return TRAINER_CLASS_PKMN_TRAINER_1;
}

u8 GetEreaderTrainerClassId(void)
{
    return TRAINER_CLASS_PKMN_TRAINER_1;
}

void GetEreaderTrainerName(u8 *dst)
{
    if (dst != NULL)
        dst[0] = EOS;
}


/*
 * Peripheral menu/link adapters still referenced by the full battle source
 * graph. Ordinary Android single-player battles do not enter these modes yet.
 */
EWRAM_DATA u8 gLastViewedMonIndex = 0;

const u8 gText_PkmnTransferredSomeonesPC[] = {EOS};
const u8 gText_PkmnTransferredLanettesPC[] = {EOS};
const u8 gText_PkmnTransferredSomeonesPCBoxFull[] = {EOS};
const u8 gText_PkmnTransferredLanettesPCBoxFull[] = {EOS};
const u8 gText_PkmnSentToPCAfterCatch[] = {EOS};

u8 GetFrontierTrainerFrontSpriteId(u16 trainerId)
{
    (void)trainerId;
    return TRAINER_PIC_BRENDAN;
}

u8 GetLRKeysPressedAndHeld(void)
{
    return 0;
}

bool8 MenuHelpers_IsLinkActive(void)
{
    return FALSE;
}

bool8 MenuHelpers_ShouldWaitForLinkRecv(void)
{
    return FALSE;
}

bool16 RunTextPrintersRetIsActive(u8 textPrinterId)
{
    (void)textPrinterId;
    return FALSE;
}

void ReshowBattleScreenDummy(void)
{
}

void CB2_BagMenuFromBattle(void)
{
    SetMainCallback2(BattleMainCB2);
}

void GoToBagMenu(u8 location, u8 pocket, MainCallback exitCallback)
{
    (void)location;
    (void)pocket;
    SetMainCallback2(exitCallback != NULL ? exitCallback : BattleMainCB2);
}

void DoWallyTutorialBagMenu(void)
{
    SetMainCallback2(BattleMainCB2);
}

void InitOldManBag(void)
{
}

void GoToBattlePyramidBagMenu(u8 location, MainCallback exitCallback)
{
    (void)location;
    SetMainCallback2(exitCallback != NULL ? exitCallback : BattleMainCB2);
}

void OpenPokeblockCaseInBattle(void)
{
    SetMainCallback2(BattleMainCB2);
}

void Task_TryUseSoftboiledOnPartyMon(u8 taskId)
{
    (void)taskId;
}

void PutBattleUpdateOnTheAir(u8 opponentLinkPlayerId, enum Move move, enum Species speciesPlayer, enum Species speciesOpponent)
{
    (void)opponentLinkPlayerId;
    (void)move;
    (void)speciesPlayer;
    (void)speciesOpponent;
}

void TryPutBattleSeminarOnAir(enum Species foeSpecies, enum Species species, u8 moveIndex, const u16 *movePtr, enum Move betterMove)
{
    (void)foeSpecies;
    (void)species;
    (void)moveIndex;
    (void)movePtr;
    (void)betterMove;
}

void CB2_ReturnToField(void)
{
}

void MoveCoords(enum Direction direction, s16 *x, s16 *y)
{
    switch (direction)
    {
    case DIR_SOUTH:
        (*y)++;
        break;
    case DIR_NORTH:
        (*y)--;
        break;
    case DIR_WEST:
        (*x)--;
        break;
    case DIR_EAST:
        (*x)++;
        break;
    }
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
