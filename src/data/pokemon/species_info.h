#include "constants/abilities.h"
#include "constants/teaching_types.h"
#include "species_info/shared_dex_text.h"
#include "species_info/shared_front_pic_anims.h"

// Macros for ease of use.

#define EVOLUTION(...) (const struct Evolution[]) { __VA_ARGS__, { EVOLUTIONS_END }, }
#define CONDITIONS(...) ((const struct EvolutionParam[]) { __VA_ARGS__, {CONDITIONS_END} })

#define ANIM_FRAMES(...) (const union AnimCmd *const[]) { sAnim_GeneralFrame0, (const union AnimCmd[]) { __VA_ARGS__ ANIMCMD_END, }, }

#if P_FOOTPRINTS
#define FOOTPRINT(sprite) .footprint = gMonFootprint_## sprite,
#else
#define FOOTPRINT(sprite)
#endif

#if B_ENEMY_MON_SHADOW_STYLE >= GEN_4 && P_GBA_STYLE_SPECIES_GFX == FALSE
#define SHADOW(x, y, size)  .enemyShadowXOffset = x, .enemyShadowYOffset = y, .enemyShadowSize = size,
#define NO_SHADOW           .suppressEnemyShadow = TRUE,
#else
#define SHADOW(x, y, size)  .enemyShadowXOffset = 0, .enemyShadowYOffset = 0, .enemyShadowSize = 0,
#define NO_SHADOW           .suppressEnemyShadow = FALSE,
#endif

#define SIZE_32x32 1
#define SIZE_64x64 0

// Set .compressed = OW_GFX_COMPRESS
#define COMP OW_GFX_COMPRESS

#if OW_POKEMON_OBJECT_EVENTS
#if OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE
#define OVERWORLD_PAL(...)                                  \
    .overworldPalette = DEFAULT(NULL, __VA_ARGS__),         \
    .overworldShinyPalette = DEFAULT_2(NULL, __VA_ARGS__),
#if P_GENDER_DIFFERENCES
#define OVERWORLD_PAL_FEMALE(...)                                 \
    .overworldPaletteFemale = DEFAULT(NULL, __VA_ARGS__),         \
    .overworldShinyPaletteFemale = DEFAULT_2(NULL, __VA_ARGS__),
#else
#define OVERWORLD_PAL_FEMALE(...)
#endif //P_GENDER_DIFFERENCES
#else
#define OVERWORLD_PAL(...)
#define OVERWORLD_PAL_FEMALE(...)
#endif //OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE

#define OVERWORLD_DATA(picTable, _size, shadow, _tracks, _anims)                                                                     \
{                                                                                                                                       \
    .tileTag = TAG_NONE,                                                                                                                \
    .paletteTag = OBJ_EVENT_PAL_TAG_DYNAMIC,                                                                                            \
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,                                                                                     \
    .size = (_size == SIZE_32x32 ? 512 : 2048),                                                                                         \
    .width = (_size == SIZE_32x32 ? 32 : 64),                                                                                           \
    .height = (_size == SIZE_32x32 ? 32 : 64),                                                                                          \
    .paletteSlot = PALSLOT_NPC_1,                                                                                                       \
    .shadowSize = shadow,                                                                                                               \
    .inanimate = FALSE,                                                                                                                 \
    .compressed = COMP,                                                                                                                 \
    .tracks = _tracks,                                                                                                                  \
    .oam = (_size == SIZE_32x32 ? &gObjectEventBaseOam_32x32 : &gObjectEventBaseOam_64x64),                                             \
    .subspriteTables = (_size == SIZE_32x32 ? sOamTables_32x32 : sOamTables_64x64),                                                     \
    .anims = _anims,                                                                                                                    \
    .images = picTable,                                                                                                                 \
}

#define OVERWORLD(objEventPic, _size, shadow, _tracks, _anims, ...)                                 \
    .overworldData = OVERWORLD_DATA(objEventPic, _size, shadow, _tracks, _anims),                   \
    OVERWORLD_PAL(__VA_ARGS__)

#if P_GENDER_DIFFERENCES
#define OVERWORLD_FEMALE(objEventPic, _size, shadow, _tracks, _anims, ...)                          \
    .overworldDataFemale = OVERWORLD_DATA(objEventPic, _size, shadow, _tracks, _anims),             \
    OVERWORLD_PAL_FEMALE(__VA_ARGS__)
#else
#define OVERWORLD_FEMALE(...)
#endif //P_GENDER_DIFFERENCES

#else
#define OVERWORLD(...)
#define OVERWORLD_FEMALE(...)
#define OVERWORLD_PAL(...)
#define OVERWORLD_PAL_FEMALE(...)
#endif //OW_POKEMON_OBJECT_EVENTS

// Maximum value for a female Pokémon is 254 (MON_FEMALE) which is 100% female.
// 255 (MON_GENDERLESS) is reserved for genderless Pokémon.
#define PERCENT_FEMALE(percent) min(254, ((percent * 255) / 100))

#define MON_TYPES(type1, ...) { type1, DEFAULT(type1, __VA_ARGS__) }
#define MON_EGG_GROUPS(group1, ...) { group1, DEFAULT(group1, __VA_ARGS__) }

#define FLIP    0
#define NO_FLIP 1

const struct SpeciesInfo gSpeciesInfo[] =
{
    [SPECIES_NONE] =
    {
        .speciesName = _("??????????"),
        .cryId = CRY_PORYGON,
        .natDexNum = NATIONAL_DEX_NONE,
        .categoryName = _("Unknown"),
        .height = 0,
        .weight = 0,
        .description = gFallbackPokedexText,
        .pokemonScale = 256,
        .pokemonOffset = 0,
        .trainerScale = 256,
        .trainerOffset = 0,
        .frontPic = gMonFrontPic_CircledQuestionMark,
        .frontPicSize = MON_COORDS_SIZE(40, 40),
        .frontPicYOffset = 12,
        .frontAnimFrames = sAnims_TwoFramePlaceHolder,
        .frontAnimId = ANIM_V_SQUISH_AND_BOUNCE,
        .backPic = gMonBackPic_CircledQuestionMark,
        .backPicSize = MON_COORDS_SIZE(40, 40),
        .backPicYOffset = 12,
        .backAnimId = BACK_ANIM_NONE,
        .palette = gMonPalette_CircledQuestionMark,
        .shinyPalette = gMonShinyPalette_CircledQuestionMark,
        .iconSprite = gMonIcon_QuestionMark,
        .iconPalIndex = 0,
        .pokemonJumpType = PKMN_JUMP_TYPE_NONE,
        FOOTPRINT(QuestionMark)
        SHADOW(-1, 0, SHADOW_SIZE_M)
    #if OW_POKEMON_OBJECT_EVENTS
        .overworldData = {
            .tileTag = TAG_NONE,
            .paletteTag = OBJ_EVENT_PAL_TAG_SUBSTITUTE,
            .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
            .size = 512,
            .width = 32,
            .height = 32,
            .paletteSlot = PALSLOT_NPC_1,
            .shadowSize = SHADOW_SIZE_M,
            .inanimate = FALSE,
            .compressed = COMP,
            .tracks = TRACKS_FOOT,
            .oam = &gObjectEventBaseOam_32x32,
            .subspriteTables = sOamTables_32x32,
            .anims = sAnimTable_Following,
            .images = sPicTable_Substitute,
        },
    #endif
        .levelUpLearnset = sNoneLevelUpLearnset,
        .teachableLearnset = sNoneTeachableLearnset,
        .eggMoveLearnset = sNoneEggMoveLearnset,
    },

    #include "species_info/gen_1_families.h"
    #include "species_info/gen_2_families.h"
    #include "species_info/gen_3_families.h"
    #include "species_info/gen_4_families.h"
    #include "species_info/gen_5_families.h"
    #include "species_info/gen_6_families.h"
    #include "species_info/gen_7_families.h"
    #include "species_info/gen_8_families.h"
    #include "species_info/gen_9_families.h"


    // -------------------------------------------------------------------------
    // Mireglen custom species.
    // Graphics currently reuse the development asset slots from the prototype;
    // the species IDs themselves are now permanent custom IDs.
    // -------------------------------------------------------------------------
    [SPECIES_WICKRAB] =
    {
        .baseHP = 50, .baseAttack = 55, .baseDefense = 60, .baseSpeed = 45, .baseSpAttack = 65, .baseSpDefense = 55,
        .types = MON_TYPES(TYPE_FIRE, TYPE_WATER),
        .catchRate = 45, .expYield = 62, .evYield_SpAttack = 1,
        .genderRatio = PERCENT_FEMALE(12.5), .eggCycles = 20, .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_SLOW, .eggGroups = MON_EGG_GROUPS(EGG_GROUP_WATER_3, EGG_GROUP_MINERAL),
        .abilities = { ABILITY_TIDAL_FLAME, ABILITY_NONE, ABILITY_TIDAL_FLAME },
        .bodyColor = BODY_COLOR_RED, .speciesName = _("Wickrab"), .cryId = CRY_TORCHIC,
        .natDexNum = NATIONAL_DEX_NONE, .categoryName = _("Candle Crab"), .height = 4, .weight = 25,
        .description = COMPOUND_STRING("Its waxy shell keeps a tiny flame lit.\nSea spray makes the flame hiss with steam.\nIt gathers warm stones in tidal pools\nand sleeps with them beneath its shell."),
        .pokemonScale = 566, .pokemonOffset = 19, .trainerScale = 256, .trainerOffset = 0,
        .frontPic = gMonFrontPic_Torchic, .frontPicSize = MON_COORDS_SIZE(64, 64), .frontPicYOffset = 0,
        .frontAnimFrames = sAnims_TwoFramePlaceHolder, .frontAnimId = ANIM_V_JUMPS_SMALL,
        .backPic = gMonBackPic_Torchic, .backPicSize = MON_COORDS_SIZE(64, 64), .backPicYOffset = 0,
        .backAnimId = BACK_ANIM_CONCAVE_ARC_SMALL,
        .palette = gMonPalette_Torchic, .shinyPalette = gMonShinyPalette_Torchic,
        .iconSprite = gMonIcon_Torchic, .iconPalIndex = 0,
        .levelUpLearnset = sTorchicLevelUpLearnset, .teachableLearnset = sTorchicTeachableLearnset,
        .evolutions = EVOLUTION({EVO_LEVEL, 16, SPECIES_CANDELAW}),
    },
    [SPECIES_CANDELAW] =
    {
        .baseHP = 65, .baseAttack = 75, .baseDefense = 80, .baseSpeed = 55, .baseSpAttack = 85, .baseSpDefense = 70,
        .types = MON_TYPES(TYPE_FIRE, TYPE_WATER),
        .catchRate = 45, .expYield = 142, .evYield_Attack = 1, .evYield_SpAttack = 1,
        .genderRatio = PERCENT_FEMALE(12.5), .eggCycles = 20, .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_SLOW, .eggGroups = MON_EGG_GROUPS(EGG_GROUP_WATER_3, EGG_GROUP_MINERAL),
        .abilities = { ABILITY_TIDAL_FLAME, ABILITY_NONE, ABILITY_TIDAL_FLAME },
        .bodyColor = BODY_COLOR_RED, .speciesName = _("Candelaw"), .cryId = CRY_COMBUSKEN,
        .natDexNum = NATIONAL_DEX_NONE, .categoryName = _("Boiler Crab"), .height = 9, .weight = 195,
        .description = COMPOUND_STRING("Heated seawater circulates through its shell.\nIts claws vent jets of pressurized steam.\nThe hotter its central candle burns,\nthe more forcefully it can strike."),
        .pokemonScale = 343, .pokemonOffset = 5, .trainerScale = 256, .trainerOffset = 0,
        .frontPic = gMonFrontPic_Combusken, .frontPicSize = MON_COORDS_SIZE(64, 64), .frontPicYOffset = 0,
        .frontAnimFrames = sAnims_TwoFramePlaceHolder, .frontAnimId = ANIM_V_JUMPS_H_JUMPS,
        .backPic = gMonBackPic_Combusken, .backPicSize = MON_COORDS_SIZE(64, 64), .backPicYOffset = 0,
        .backAnimId = BACK_ANIM_CONCAVE_ARC_LARGE,
        .palette = gMonPalette_Combusken, .shinyPalette = gMonShinyPalette_Combusken,
        .iconSprite = gMonIcon_Combusken, .iconPalIndex = 0,
        .levelUpLearnset = sCombuskenLevelUpLearnset, .teachableLearnset = sCombuskenTeachableLearnset,
        .evolutions = EVOLUTION({EVO_LEVEL, 36, SPECIES_CRABRAZIER}),
    },
    [SPECIES_CRABRAZIER] =
    {
        .baseHP = 85, .baseAttack = 95, .baseDefense = 105, .baseSpeed = 65, .baseSpAttack = 115, .baseSpDefense = 90,
        .types = MON_TYPES(TYPE_FIRE, TYPE_WATER),
        .catchRate = 45, .expYield = 265, .evYield_Attack = 3,
        .genderRatio = PERCENT_FEMALE(12.5), .eggCycles = 20, .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_SLOW, .eggGroups = MON_EGG_GROUPS(EGG_GROUP_WATER_3, EGG_GROUP_MINERAL),
        .abilities = { ABILITY_TIDAL_FLAME, ABILITY_NONE, ABILITY_TIDAL_FLAME },
        .bodyColor = BODY_COLOR_RED, .speciesName = _("Crabrazier"), .cryId = CRY_BLAZIKEN,
        .natDexNum = NATIONAL_DEX_NONE, .categoryName = _("Brazier Crab"), .height = 19, .weight = 520,
        .description = COMPOUND_STRING("Its shell has become a living sea brazier.\nWaves crashing over it erupt into steam.\nSailors once followed its distant glow\nthrough storms to find safe harbor."),
        .pokemonScale = 256, .pokemonOffset = 0, .trainerScale = 301, .trainerOffset = 4,
        .frontPic = gMonFrontPic_Blaziken, .frontPicSize = MON_COORDS_SIZE(64, 64), .frontPicYOffset = 0,
        .frontAnimFrames = sAnims_TwoFramePlaceHolder, .frontAnimId = ANIM_H_SHAKE,
        .backPic = gMonBackPic_Blaziken, .backPicSize = MON_COORDS_SIZE(64, 64), .backPicYOffset = 0,
        .backAnimId = BACK_ANIM_SHAKE_GLOW_RED,
        .palette = gMonPalette_Blaziken, .shinyPalette = gMonShinyPalette_Blaziken,
        .iconSprite = gMonIcon_Blaziken, .iconPalIndex = 0,
        .levelUpLearnset = sBlazikenLevelUpLearnset, .teachableLearnset = sBlazikenTeachableLearnset,
    },
    [SPECIES_MUDKIP_MIREGLEN] =
    {
        .baseHP = 50, .baseAttack = 65, .baseDefense = 55, .baseSpeed = 40, .baseSpAttack = 55, .baseSpDefense = 50,
        .types = MON_TYPES(TYPE_POISON, TYPE_GROUND),
        .catchRate = 45, .expYield = 62, .evYield_Attack = 1,
        .genderRatio = PERCENT_FEMALE(12.5), .eggCycles = 20, .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_SLOW, .eggGroups = MON_EGG_GROUPS(EGG_GROUP_MONSTER, EGG_GROUP_WATER_1),
        .abilities = { ABILITY_POISON_TOUCH, ABILITY_WATER_ABSORB, ABILITY_CORROSION },
        .bodyColor = BODY_COLOR_PURPLE, .speciesName = _("Mudkip"), .cryId = CRY_MUDKIP,
        .natDexNum = NATIONAL_DEX_NONE, .categoryName = _("Toxic Mud"), .height = 4, .weight = 76,
        .description = COMPOUND_STRING("It filters foul marsh water through its gills.\nToxic minerals gather in the mud on its head.\nIts glowing gills reveal how polluted\nthe ground around its nest has become."),
        .pokemonScale = 535, .pokemonOffset = 20, .trainerScale = 256, .trainerOffset = 0,
        .frontPic = gMonFrontPic_Mudkip, .frontPicSize = MON_COORDS_SIZE(64, 64), .frontPicYOffset = 0,
        .frontAnimFrames = sAnims_TwoFramePlaceHolder, .frontAnimId = ANIM_CIRCULAR_STRETCH_TWICE,
        .backPic = gMonBackPic_Mudkip, .backPicSize = MON_COORDS_SIZE(64, 64), .backPicYOffset = 0,
        .backAnimId = BACK_ANIM_H_SLIDE,
        .palette = gMonPalette_Mudkip, .shinyPalette = gMonShinyPalette_Mudkip,
        .iconSprite = gMonIcon_Mudkip, .iconPalIndex = 0,
        .levelUpLearnset = sMudkipLevelUpLearnset, .teachableLearnset = sMudkipTeachableLearnset,
        .evolutions = EVOLUTION({EVO_LEVEL, 16, SPECIES_MARSHTOMP_MIREGLEN}),
    },
    [SPECIES_MARSHTOMP_MIREGLEN] =
    {
        .baseHP = 70, .baseAttack = 85, .baseDefense = 75, .baseSpeed = 45, .baseSpAttack = 65, .baseSpDefense = 70,
        .types = MON_TYPES(TYPE_POISON, TYPE_GROUND),
        .catchRate = 45, .expYield = 142, .evYield_Attack = 2,
        .genderRatio = PERCENT_FEMALE(12.5), .eggCycles = 20, .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_SLOW, .eggGroups = MON_EGG_GROUPS(EGG_GROUP_MONSTER, EGG_GROUP_WATER_1),
        .abilities = { ABILITY_POISON_TOUCH, ABILITY_WATER_ABSORB, ABILITY_CORROSION },
        .bodyColor = BODY_COLOR_PURPLE, .speciesName = _("Marshtomp"), .cryId = CRY_MARSHTOMP,
        .natDexNum = NATIONAL_DEX_NONE, .categoryName = _("Bog Toxin"), .height = 7, .weight = 280,
        .pokemonScale = 340, .pokemonOffset = 7, .trainerScale = 256, .trainerOffset = 0,
        .frontPic = gMonFrontPic_Marshtomp, .frontPicSize = MON_COORDS_SIZE(64, 64), .frontPicYOffset = 0,
        .frontAnimFrames = sAnims_TwoFramePlaceHolder, .frontAnimId = ANIM_V_STRETCH,
        .backPic = gMonBackPic_Marshtomp, .backPicSize = MON_COORDS_SIZE(64, 64), .backPicYOffset = 0,
        .backAnimId = BACK_ANIM_CONCAVE_ARC_SMALL,
        .palette = gMonPalette_Marshtomp, .shinyPalette = gMonShinyPalette_Marshtomp,
        .iconSprite = gMonIcon_Marshtomp, .iconPalIndex = 0,
        .levelUpLearnset = sMarshtompLevelUpLearnset, .teachableLearnset = sMarshtompTeachableLearnset,
        .evolutions = EVOLUTION({EVO_LEVEL, 36, SPECIES_SWAMPERT_MIREGLEN}),
    },
    [SPECIES_SWAMPERT_MIREGLEN] =
    {
        .baseHP = 105, .baseAttack = 115, .baseDefense = 95, .baseSpeed = 55, .baseSpAttack = 85, .baseSpDefense = 90,
        .types = MON_TYPES(TYPE_POISON, TYPE_GROUND),
        .catchRate = 45, .expYield = 268, .evYield_Attack = 3,
        .genderRatio = PERCENT_FEMALE(12.5), .eggCycles = 20, .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_SLOW, .eggGroups = MON_EGG_GROUPS(EGG_GROUP_MONSTER, EGG_GROUP_WATER_1),
        .abilities = { ABILITY_POISON_TOUCH, ABILITY_WATER_ABSORB, ABILITY_CORROSION },
        .bodyColor = BODY_COLOR_PURPLE, .speciesName = _("Swampert"), .cryId = CRY_SWAMPERT,
        .natDexNum = NATIONAL_DEX_NONE, .categoryName = _("Toxic Bog"), .height = 15, .weight = 819,
        .pokemonScale = 256, .pokemonOffset = 0, .trainerScale = 256, .trainerOffset = 0,
        .frontPic = gMonFrontPic_Swampert, .frontPicSize = MON_COORDS_SIZE(64, 64), .frontPicYOffset = 0,
        .frontAnimFrames = sAnims_TwoFramePlaceHolder, .frontAnimId = ANIM_H_SHAKE,
        .backPic = gMonBackPic_Swampert, .backPicSize = MON_COORDS_SIZE(64, 64), .backPicYOffset = 0,
        .backAnimId = BACK_ANIM_SHAKE_GLOW_BLUE,
        .palette = gMonPalette_Swampert, .shinyPalette = gMonShinyPalette_Swampert,
        .iconSprite = gMonIcon_Swampert, .iconPalIndex = 0,
        .levelUpLearnset = sSwampertLevelUpLearnset, .teachableLearnset = sSwampertTeachableLearnset,
    },

    [SPECIES_EGG] =
    {
        .frontPic = gMonFrontPic_Egg,
        .frontPicSize = MON_COORDS_SIZE(24, 24),
        .frontPicYOffset = 20,
        .backPic = gMonFrontPic_Egg,
        .backPicSize = MON_COORDS_SIZE(24, 24),
        .backPicYOffset = 20,
        .palette = gMonPalette_Egg,
        .shinyPalette = gMonPalette_Egg,
        .iconSprite = gMonIcon_Egg,
        .iconPalIndex = 1,
    },

    /* You may add any custom species below this point based on the following structure: */

    /*
    [SPECIES_NONE] =
    {
        .baseHP        = 1,
        .baseAttack    = 1,
        .baseDefense   = 1,
        .baseSpeed     = 1,
        .baseSpAttack  = 1,
        .baseSpDefense = 1,
        .types = MON_TYPES(TYPE_MYSTERY),
        .catchRate = 255,
        .expYield = 67,
        .evYield_HP = 1,
        .evYield_Defense = 1,
        .evYield_SpDefense = 1,
        .genderRatio = PERCENT_FEMALE(50),
        .eggCycles = 20,
        .friendship = STANDARD_FRIENDSHIP,
        .growthRate = GROWTH_MEDIUM_FAST,
        .eggGroups = MON_EGG_GROUPS(EGG_GROUP_NO_EGGS_DISCOVERED),
        .abilities = { ABILITY_NONE, ABILITY_CURSED_BODY, ABILITY_DAMP },
        .bodyColor = BODY_COLOR_BLACK,
        .speciesName = _("??????????"),
        .cryId = CRY_NONE,
        .natDexNum = NATIONAL_DEX_NONE,
        .categoryName = _("Unknown"),
        .height = 0,
        .weight = 0,
        .description = COMPOUND_STRING(
            "This is a newly discovered Pokémon.\n"
            "It is currently under investigation.\n"
            "No detailed information is available\n"
            "at this time."),
        .pokemonScale = 256,
        .pokemonOffset = 0,
        .trainerScale = 256,
        .trainerOffset = 0,
        .frontPic = gMonFrontPic_CircledQuestionMark,
        .frontPicSize = MON_COORDS_SIZE(64, 64),
        .frontPicYOffset = 0,
        .frontAnimFrames = sAnims_None,
        //.frontAnimId = ANIM_V_SQUISH_AND_BOUNCE,
        .backPic = gMonBackPic_CircledQuestionMark,
        .backPicSize = MON_COORDS_SIZE(64, 64),
        .backPicYOffset = 7,
#if P_GENDER_DIFFERENCES
        .frontPicFemale = gMonFrontPic_CircledQuestionMark,
        .frontPicSizeFemale = MON_COORDS_SIZE(64, 64),
        .backPicFemale = gMonBackPic_CircledQuestionMarkF,
        .backPicSizeFemale = MON_COORDS_SIZE(64, 64),
        .paletteFemale = gMonPalette_CircledQuestionMarkF,
        .shinyPaletteFemale = gMonShinyPalette_CircledQuestionMarkF,
        .iconSpriteFemale = gMonIcon_QuestionMarkF,
        .iconPalIndexFemale = 1,
#endif //P_GENDER_DIFFERENCES
        .backAnimId = BACK_ANIM_NONE,
        .palette = gMonPalette_CircledQuestionMark,
        .shinyPalette = gMonShinyPalette_CircledQuestionMark,
        .iconSprite = gMonIcon_QuestionMark,
        .iconPalIndex = 0,
        FOOTPRINT(QuestionMark)
        .levelUpLearnset = sNoneLevelUpLearnset,
        .teachableLearnset = sNoneTeachableLearnset,
        .evolutions = EVOLUTION({EVO_LEVEL, 100, SPECIES_NONE},
                                {EVO_ITEM, ITEM_MOOMOO_MILK, SPECIES_NONE}),
        //.formSpeciesIdTable = sNoneFormSpeciesIdTable,
        //.formChangeTable = sNoneFormChangeTable,
        //.perfectIVCount = NUM_STATS,
    },
    */
};

const struct EggData gEggDatas[EGG_ID_COUNT] =
{
#include "egg_data.h"
};
