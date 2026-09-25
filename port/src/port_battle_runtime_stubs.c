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
#include "battle_debug.h"
#include "battle_tower.h"
#include "cable_club.h"
#include "evolution_graphics.h"
#include "field_control_avatar.h"
#include "field_move.h"
#include "field_specials.h"
#include "field_weather.h"
#include "fldeff.h"
#include "region_map.h"
#include "start_menu.h"
#include "strings.h"
#include "trade.h"
#include "union_room.h"
#include "berry_powder.h"
#include "bike.h"
#include "coins.h"
#include "event_scripts.h"
#include "field_effect.h"
#include "field_screen_effect.h"
#include "fishing.h"
#include "easy_chat.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "follower_npc.h"
#include "link.h"
#include "link_rfu.h"
#include "party_menu.h"
#include "player_pc.h"
#include "pokemon_jump.h"
#include "safari_zone.h"
#include "save.h"
#include "script.h"
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


/* Exact/lightweight shared helpers used by ordinary battle calculations. */
u16 GetNationalPokedexCount(u8 caseID)
{
    u16 count = 0;

    for (u16 i = 0; i < NATIONAL_DEX_COUNT; i++)
    {
        switch (caseID)
        {
        case FLAG_GET_SEEN:
            if (GetSetPokedexFlag(i + 1, FLAG_GET_SEEN))
                count++;
            break;
        case FLAG_GET_CAUGHT:
            if (GetSetPokedexFlag(i + 1, FLAG_GET_CAUGHT))
                count++;
            break;
        }
    }

    return count;
}

enum MapType GetCurrentMapType(void)
{
    return gMapHeader.mapType;
}

u8 gDisableMapMusicChangeOnMapLoad = 0;

/* Dormant side-mode/UI adapters not entered by the first Android battle. */
const u8 SecretBase_Text_Trainer0Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer1Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer2Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer3Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer4Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer5Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer6Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer7Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer8Defeated[] = {EOS};
const u8 SecretBase_Text_Trainer9Defeated[] = {EOS};

enum TrainerPicID GetFrontierBrainTrainerPicIndex(void)
{
    return TRAINER_PIC_BRENDAN;
}

u8 GetTrainerTowerTrainerFrontSpriteId(void)
{
    return TRAINER_PIC_BRENDAN;
}

u8 GetTrainerHillTrainerFrontSpriteId(u16 trainerId)
{
    (void)trainerId;
    return TRAINER_PIC_BRENDAN;
}

u8 GetEreaderTrainerFrontSpriteId(void)
{
    return TRAINER_PIC_BRENDAN;
}

bool32 IsSpeciesAllowedInPokemonJump(enum Species species)
{
    (void)species;
    return FALSE;
}

u8 GetContestEntryEligibility(struct Pokemon *pkmn)
{
    (void)pkmn;
    return 0;
}

void CB2_BattleDebugMenu(void)
{
    SetMainCallback2(BattleMainCB2);
}

void CB2_ChooseBall(void)
{
    SetMainCallback2(BattleMainCB2);
}

struct PlayerPCItemPageStruct gPlayerPCItemPageInfo = {0};

void DoEasyChatScreen(u8 type, u16 *words, MainCallback exitCallback, u8 displayedPersonType)
{
    (void)type;
    (void)words;
    (void)displayedPersonType;
    if (exitCallback != NULL)
        SetMainCallback2(exitCallback);
}


/*
 * Android field/bag boundary.
 *
 * Keep Emerald's real item-use and battle-item rules linked, but do not drag
 * the full GBA field UI, fishing minigame, event-script runner, bike UI, or
 * Battle Pyramid bag into the first Android battle milestone.
 */

static struct BagMenu sAndroidBagMenu;
struct BagMenu *gBagMenu = &sAndroidBagMenu;

static struct PyramidBagMenu sAndroidPyramidBagMenu;
struct PyramidBagMenu *gPyramidBagMenu = &sAndroidPyramidBagMenu;

void (*gFieldCallback)(void) = NULL;

const u8 EventScript_AccessPokemonBoxLink[] = {EOS};
const u8 BattleFrontier_OutsideEast_EventScript_WaterSudowoodo[] = {EOS};
const u8 BerryTree_EventScript_ItemUseWailmerPail[] = {EOS};

const u8 MoveRelearner_Text_LevelUpMoveLWR[] = {EOS};
const u8 MoveRelearner_Text_EggMoveLWR[] = {EOS};
const u8 MoveRelearner_Text_TMMoveLWR[] = {EOS};
const u8 MoveRelearner_Text_TutorMoveLWR[] = {EOS};

void LockPlayerFieldControls(void)
{
}

void UnlockPlayerFieldControls(void)
{
}

void ScriptUnfreezeObjectEvents(void)
{
}

void FieldCB_ReturnToFieldNoScript(void)
{
}

void FadeInFromBlack(void)
{
    FadeScreen(FADE_FROM_BLACK, 0);
}

void CleanupOverworldWindowsAndTilemaps(void)
{
    /*
     * Android's overworld renderer owns its tilemap/window lifetime. The GBA
     * implementation frees GBA-side overworld buffers here; there are no
     * equivalent owned buffers to release in this bootstrap path.
     */
}

void ScriptContext_SetupScript(const u8 *ptr)
{
    (void)ptr;
}

void ObjectEventSetGraphicsId(struct ObjectEvent *objectEvent, u16 graphicsId)
{
    if (objectEvent != NULL)
        objectEvent->graphicsId = graphicsId;
}

u8 GetObjectEventIdByPosition(u16 x, u16 y, u8 elevation)
{
    (void)x;
    (void)y;
    (void)elevation;
    return OBJECT_EVENTS_COUNT;
}

bool32 Overworld_IsBikingAllowed(void)
{
    return FALSE;
}

bool8 IsBikingDisallowedByPlayer(void)
{
    return TRUE;
}

void GetOnOffBike(u8 transitionFlags)
{
    (void)transitionFlags;
}

bool32 FollowerNPCCanBike(void)
{
    return FALSE;
}

void FollowerNPC_HandleBike(void)
{
}

bool32 CheckFollowerNPCFlag(u32 flag)
{
    (void)flag;
    return FALSE;
}

void ResetInitialPlayerAvatarState(void)
{
}

void Overworld_ResetStateAfterDigEscRope(void)
{
}

void StartEscapeRopeFieldEffect(void)
{
}

u16 GetCoins(void)
{
    return 0;
}

u32 GetBerryPowder(void)
{
    return 0;
}

void ReadMail(struct Mail *mail, MainCallback exitCallback, bool8 hasText)
{
    (void)mail;
    (void)hasText;
    if (exitCallback != NULL)
        SetMainCallback2(exitCallback);
}

void CB2_ReturnToBagMenuPocket(void)
{
    SetMainCallback2(BattleMainCB2);
}

void DisplayItemMessage(u8 taskId, u8 fontId, const u8 *str, TaskFunc callback)
{
    (void)fontId;
    (void)str;
    if (callback != NULL)
        callback(taskId);
}

void CloseItemMessage(u8 taskId)
{
    (void)taskId;
}

void Task_FadeAndCloseBagMenu(u8 taskId)
{
    MainCallback next = gBagMenu != NULL ? gBagMenu->newScreenCallback : NULL;
    if (next != NULL)
        SetMainCallback2(next);
    DestroyTask(taskId);
}

void DisplayItemMessageInBattlePyramid(u8 taskId, const u8 *str, TaskFunc callback)
{
    (void)str;
    if (callback != NULL)
        callback(taskId);
}

void Task_CloseBattlePyramidBagMessage(u8 taskId)
{
    (void)taskId;
}

void CloseBattlePyramidBag(u8 taskId)
{
    MainCallback next = gPyramidBagMenu != NULL ? gPyramidBagMenu->newScreenCallback : NULL;
    if (next != NULL)
        SetMainCallback2(next);
    DestroyTask(taskId);
}

void OpenPokeblockCase(u8 caseId, void (*callback)(void))
{
    (void)caseId;
    if (callback != NULL)
        SetMainCallback2(callback);
}

void SetPokemonAnglerSpecies(enum Species species)
{
    (void)species;
}

/* Fishing minigame stays outside the battle archive for now. */
void StartFishing(u8 rod)
{
    (void)rod;
}

void UpdateChainFishingStreak(void)
{
}

u32 CalculateChainFishingShinyRolls(void)
{
    return 0;
}

bool32 ShouldUseFishingEnvironmentInBattle(void)
{
    return FALSE;
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


/* Android battle-closure boundary adapters (link/RFU/field-only side paths). */

/*
 * These symbols are referenced from real Emerald battle objects, but belong to
 * surrounding GBA-only field/link/trading UIs that are not reachable from the
 * first Android single-player battle. Keep the real battle modules linked and
 * close only those peripheral edges here.
 */

s16 CompactPartySlots(void)
{
    s16 firstEmpty = -1;
    u16 i, last;

    for (i = 0, last = 0; i < PARTY_SIZE; i++)
    {
        enum Species species = GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES);
        if (species != SPECIES_NONE)
        {
            if (i != last)
                gParties[B_TRAINER_PLAYER][last] = gParties[B_TRAINER_PLAYER][i];
            last++;
        }
        else if (firstEmpty == -1)
        {
            firstEmpty = i;
        }
    }

    for (; last < PARTY_SIZE; last++)
        ZeroMonData(&gParties[B_TRAINER_PLAYER][last]);

    return firstEmpty;
}

void AppendToList(u8 *list, u8 *pos, u8 newEntry)
{
    list[*pos] = newEntry;
    (*pos)++;
}

bool8 InMultiPartnerRoom(void)
{
    return FALSE;
}

static bool32 AndroidFieldMoveUnavailable(enum FieldMove fieldMove)
{
    (void)fieldMove;
    return FALSE;
}

const struct FieldMoveUnlock gFieldMoveUnlocks[FIELD_MOVE_UNLOCK_COUNT] =
{
    [0 ... FIELD_MOVE_UNLOCK_COUNT - 1] = {AndroidFieldMoveUnavailable, NULL},
};

const struct FieldMoveInfo gFieldMoveInfo[FIELD_MOVES_COUNT] =
{
    [0 ... FIELD_MOVES_COUNT - 1] = {
        .unlockType = CANT_UNLOCK,
        .moveID = MOVE_NONE,
        .hideIfLocked = TRUE,
    },
};

static struct RfuGameData sAndroidHostRfuGameData;
struct RfuGameCompatibilityData gRfuPartnerCompatibilityData = {0};
enum Species gUnionRoomOfferedSpecies = SPECIES_NONE;
enum Type gUnionRoomRequestedMonType = TYPE_NONE;

struct RfuGameData *GetHostRfuGameData(void)
{
    return &sAndroidHostRfuGameData;
}

int CanRegisterMonForTradingBoard(struct RfuGameCompatibilityData player, enum Species species2, enum Species species, bool8 isModernFatefulEncounter)
{
    (void)player;
    (void)species2;
    (void)species;
    (void)isModernFatefulEncounter;
    return 0;
}

int GetUnionRoomTradeMessageId(struct RfuGameCompatibilityData player, struct RfuGameCompatibilityData partner, enum Species playerSpecies2, enum Species partnerSpecies, enum Type requestedType, enum Species playerSpecies, bool8 isModernFatefulEncounter)
{
    (void)player;
    (void)partner;
    (void)playerSpecies2;
    (void)partnerSpecies;
    (void)requestedType;
    (void)playerSpecies;
    (void)isModernFatefulEncounter;
    return 0;
}

enum CanTradeMon CanSpinTradeMon(struct Pokemon *mon, u16 monIdx)
{
    (void)mon;
    (void)monIdx;
    return (enum CanTradeMon)0;
}

void ShowPokemonSummaryScreen(u8 mode, void *mons, u8 monIndex, u8 maxMonIndex, void (*callback)(void))
{
    (void)mode;
    (void)mons;
    (void)monIndex;
    (void)maxMonIndex;
    if (callback != NULL)
        SetMainCallback2(callback);
}

void ChooseMonForSoftboiled(u8 taskId)
{
    DestroyTask(taskId);
}

u8 *GetMapNameGeneric(u8 *dest, mapsec_u16_t mapSecId)
{
    (void)mapSecId;
    if (dest != NULL)
        dest[0] = EOS;
    return dest;
}

void CB2_OpenFlyMap(void)
{
}

bool8 (*gFieldCallback2)(void) = NULL;

void UpdatePocketItemList(enum Pocket pocketId)
{
    (void)pocketId;
}

void UpdatePocketListPosition(u8 pocketId)
{
    (void)pocketId;
}

void UpdatePyramidBagList(void)
{
}

void UpdatePyramidBagCursorPos(void)
{
}

void StartSweetScentFieldEffect(void)
{
}

const u8 EventScript_RegionMap[] = {EOS};

u8 GetObjectEventIdByLocalIdAndMap(u8 localId, u8 mapNum, u8 mapGroupId)
{
    (void)localId;
    (void)mapNum;
    (void)mapGroupId;
    return OBJECT_EVENTS_COUNT;
}

void ObjectEventClearHeldMovementIfFinished(struct ObjectEvent *objectEvent)
{
    (void)objectEvent;
}

void ObjectEventClearHeldMovement(struct ObjectEvent *objectEvent)
{
    (void)objectEvent;
}

void UnfreezeObjectEvent(struct ObjectEvent *objectEvent)
{
    (void)objectEvent;
}

u8 ObjectEventCheckHeldMovementStatus(struct ObjectEvent *objectEvent)
{
    (void)objectEvent;
    return 0;
}

void BagMenu_YesNo(u8 taskId, u8 windowType, const struct YesNoFuncTable *funcTable)
{
    (void)taskId;
    (void)windowType;
    (void)funcTable;
}

void LoadWirelessStatusIndicatorSpriteGfx(void)
{
}

void GetBattleTowerTrainerLanguage(u8 *dst, u16 trainerId)
{
    (void)trainerId;
    if (dst != NULL)
        *dst = 0;
}

void CreateWirelessStatusIndicatorSprite(u8 x, u8 y)
{
    (void)x;
    (void)y;
}

void TrySetLinkBattleTowerEnemyPartyLevel(void)
{
}

void TryPutPokemonTodayOnAir(void)
{
}

void TryPutBreakingNewsOnAir(void)
{
}

void Task_ReconnectWithLinkPlayers(u8 taskId)
{
    DestroyTask(taskId);
}

const u8 gText_LinkStandby3[] = {EOS};
const u8 BattleFrontier_BattleTowerBattleRoom_Text_RecordCouldntBeSaved[] = {EOS};

void SetWirelessCommType1(void)
{
}

void OpenLink(void)
{
}

void Task_WaitForLinkPlayerConnection(u8 taskId)
{
    DestroyTask(taskId);
}

struct ObjectEvent *GetFollowerObject(void)
{
    return NULL;
}

u8 GetLinkPlayerCount_2(void)
{
    return 1;
}

bool8 IsLinkMaster(void)
{
    return TRUE;
}

void CheckShouldAdvanceLinkState(void)
{
}

void LoadEvoSparkleSpriteAndPal(void)
{
}

u8 EvolutionSparkles_SpiralUpward(u16 palNum)
{
    (void)palNum;
    return 0;
}

u8 EvolutionSparkles_ArcDown(void)
{
    return 0;
}

u8 CycleEvolutionMonSprite(u8 preEvoSpriteId, u8 postEvoSpriteId)
{
    (void)preEvoSpriteId;
    (void)postEvoSpriteId;
    return 0;
}

u8 EvolutionSparkles_CircleInward(void)
{
    return 0;
}

u8 EvolutionSparkles_SprayAndFlash(enum Species species)
{
    (void)species;
    return 0;
}

void Overworld_PlaySpecialMapMusic(void)
{
}

bool8 ObjectEventIsMovementOverridden(struct ObjectEvent *objectEvent)
{
    (void)objectEvent;
    return FALSE;
}

bool8 ObjectEventSetHeldMovement(struct ObjectEvent *objectEvent, u8 specialAnimId)
{
    (void)objectEvent;
    (void)specialAnimId;
    return FALSE;
}

u8 GetWalkInPlaceFastMovementAction(u32 direction)
{
    (void)direction;
    return 0;
}

enum Collision GetCollisionAtCoords(struct ObjectEvent *objectEvent, s16 x, s16 y, enum Direction dir)
{
    (void)objectEvent;
    (void)x;
    (void)y;
    (void)dir;
    return (enum Collision)0;
}

u8 gSelectedObjectEvent = 0;

u8 GetObjectEventBerryTreeId(u8 objectEventId)
{
    (void)objectEventId;
    return 0;
}

const u8 *GetObjectEventScriptPointerPlayerFacing(void)
{
    return NULL;
}

const u8 BerryTreeScript[] = {EOS};

bool32 MapHasNaturalLight(enum MapType mapType)
{
    (void)mapType;
    return FALSE;
}

void UpdateAltBgPalettes(u16 palettes)
{
    (void)palettes;
}

struct TimeBlendSettings gTimeBlend = {0};

bool32 IsOverworldLinkActive(void)
{
    return FALSE;
}

bool32 IsLinkRecvQueueAtOverworldMax(void)
{
    return FALSE;
}

bool32 Overworld_IsRecvQueueAtMax(void)
{
    return FALSE;
}

static const struct OamData sAndroidCategoryIconOam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x16),
    .size = SPRITE_SIZE(32x16),
    .priority = 1,
};

static const union AnimCmd sAndroidCategoryIconAnim[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END,
};

static const union AnimCmd *const sAndroidCategoryIconAnims[] =
{
    sAndroidCategoryIconAnim,
    sAndroidCategoryIconAnim,
    sAndroidCategoryIconAnim,
    sAndroidCategoryIconAnim,
};

const struct SpriteTemplate gSpriteTemplate_CategoryIcons =
{
    .tileTag = TAG_NONE,
    .paletteTag = TAG_NONE,
    .oam = &sAndroidCategoryIconOam,
    .anims = sAndroidCategoryIconAnims,
    .callback = SpriteCallbackDummy,
};
