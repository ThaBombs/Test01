#include "global.h"
#include "overworld.h"
#include "constants/layouts.h"
#include "constants/map_types.h"
#include "constants/region_map_sections.h"
#include "constants/weather.h"
#include "constants/maps.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/event_bg.h"

const u16 ALIGNED(4) gPortTilesetPalettes_General[][16] =
{
    INCGFX_U16("data/tilesets/primary/general/palettes/00.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/01.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/02.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/03.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/04.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/05.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/06.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/07.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/08.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/09.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/10.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/11.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/12.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/13.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/14.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/15.pal", ".gbapal"),
};
const u32 gPortTilesetTiles_General[] =
    INCGFX_U32("data/tilesets/primary/general/tiles.png", ".4bpp");
const u16 gPortMetatiles_General[] =
    INCBIN_U16("data/tilesets/primary/general/metatiles.bin");
const u16 gPortMetatileAttributes_General[] =
    INCBIN_U16("data/tilesets/primary/general/metatile_attributes.bin");

const u16 ALIGNED(4) gPortTilesetPalettes_Petalburg[][16] =
{
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/00.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/01.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/02.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/03.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/04.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/05.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/06.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/07.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/08.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/09.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/10.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/11.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/12.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/13.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/14.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/petalburg/palettes/15.pal", ".gbapal"),
};
const u32 gPortTilesetTiles_Petalburg[] =
    INCGFX_U32("data/tilesets/secondary/petalburg/tiles.png", ".4bpp", "-num_tiles 159 -Wnum_tiles");
const u16 gPortMetatiles_Petalburg[] =
    INCBIN_U16("data/tilesets/secondary/petalburg/metatiles.bin");
const u16 gPortMetatileAttributes_Petalburg[] =
    INCBIN_U16("data/tilesets/secondary/petalburg/metatile_attributes.bin");

const u16 ALIGNED(4) gPortTilesetPalettes_Building[][16] =
{
    INCGFX_U16("data/tilesets/primary/building/palettes/00.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/01.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/02.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/03.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/04.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/05.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/06.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/07.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/08.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/09.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/10.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/11.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/12.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/13.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/14.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/building/palettes/15.pal", ".gbapal"),
};
const u32 gPortTilesetTiles_Building[] =
    INCGFX_U32("data/tilesets/primary/building/tiles.png", ".4bpp", "-num_tiles 502 -Wnum_tiles");
const u16 gPortMetatiles_Building[] =
    INCBIN_U16("data/tilesets/primary/building/metatiles.bin");
const u16 gPortMetatileAttributes_Building[] =
    INCBIN_U16("data/tilesets/primary/building/metatile_attributes.bin");

const u16 ALIGNED(4) gPortTilesetPalettes_BrendansMaysHouse[][16] =
{
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/00.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/01.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/02.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/03.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/04.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/05.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/06.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/07.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/08.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/09.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/10.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/11.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/12.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/13.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/14.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/brendans_mays_house/palettes/15.pal", ".gbapal"),
};
const u32 gPortTilesetTiles_BrendansMaysHouse[] =
    INCGFX_U32("data/tilesets/secondary/brendans_mays_house/tiles.png", ".4bpp");
const u16 gPortMetatiles_BrendansMaysHouse[] =
    INCBIN_U16("data/tilesets/secondary/brendans_mays_house/metatiles.bin");
const u16 gPortMetatileAttributes_BrendansMaysHouse[] =
    INCBIN_U16("data/tilesets/secondary/brendans_mays_house/metatile_attributes.bin");

const u16 ALIGNED(4) gPortTilesetPalettes_Lab[][16] =
{
    INCGFX_U16("data/tilesets/secondary/lab/palettes/00.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/01.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/02.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/03.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/04.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/05.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/06.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/07.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/08.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/09.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/10.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/11.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/12.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/13.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/14.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/secondary/lab/palettes/15.pal", ".gbapal"),
};
const u32 gPortTilesetTiles_Lab[] =
    INCGFX_U32("data/tilesets/secondary/lab/tiles.png", ".4bpp", "-num_tiles 500 -Wnum_tiles");
const u16 gPortMetatiles_Lab[] =
    INCBIN_U16("data/tilesets/secondary/lab/metatiles.bin");
const u16 gPortMetatileAttributes_Lab[] =
    INCBIN_U16("data/tilesets/secondary/lab/metatile_attributes.bin");

static const struct Tileset sPortTilesetGeneral =
{
    .isCompressed = FALSE,
    .isSecondary = FALSE,
    .tiles = gPortTilesetTiles_General,
    .palettes = gPortTilesetPalettes_General,
    .metatiles = gPortMetatiles_General,
    .metatileAttributes = gPortMetatileAttributes_General,
    .callback = NULL,
};

static const struct Tileset sPortTilesetPetalburg =
{
    .isCompressed = FALSE,
    .isSecondary = TRUE,
    .tiles = gPortTilesetTiles_Petalburg,
    .palettes = gPortTilesetPalettes_Petalburg,
    .metatiles = gPortMetatiles_Petalburg,
    .metatileAttributes = gPortMetatileAttributes_Petalburg,
    .callback = NULL,
};

static const struct Tileset sPortTilesetBuilding =
{
    .isCompressed = FALSE,
    .isSecondary = FALSE,
    .tiles = gPortTilesetTiles_Building,
    .palettes = gPortTilesetPalettes_Building,
    .metatiles = gPortMetatiles_Building,
    .metatileAttributes = gPortMetatileAttributes_Building,
    .callback = NULL,
};

static const struct Tileset sPortTilesetBrendansMaysHouse =
{
    .isCompressed = FALSE,
    .isSecondary = TRUE,
    .tiles = gPortTilesetTiles_BrendansMaysHouse,
    .palettes = gPortTilesetPalettes_BrendansMaysHouse,
    .metatiles = gPortMetatiles_BrendansMaysHouse,
    .metatileAttributes = gPortMetatileAttributes_BrendansMaysHouse,
    .callback = NULL,
};

static const struct Tileset sPortTilesetLab =
{
    .isCompressed = FALSE,
    .isSecondary = TRUE,
    .tiles = gPortTilesetTiles_Lab,
    .palettes = gPortTilesetPalettes_Lab,
    .metatiles = gPortMetatiles_Lab,
    .metatileAttributes = gPortMetatileAttributes_Lab,
    .callback = NULL,
};

static const u16 ALIGNED(4) sPortLittlerootBorder[] =
    INCBIN_U16("data/layouts/LittlerootTown/border.bin");
static const u16 ALIGNED(4) sPortLittlerootMap[] =
    INCBIN_U16("data/layouts/LittlerootTown/map.bin");

static const u16 ALIGNED(4) sPortBrendanHouse1FBorder[] =
    INCBIN_U16("data/layouts/LittlerootTown_BrendansHouse_1F/border.bin");
static const u16 ALIGNED(4) sPortBrendanHouse1FMap[] =
    INCBIN_U16("data/layouts/LittlerootTown_BrendansHouse_1F/map.bin");
static const u16 ALIGNED(4) sPortMayHouse1FBorder[] =
    INCBIN_U16("data/layouts/LittlerootTown_MaysHouse_1F/border.bin");
static const u16 ALIGNED(4) sPortMayHouse1FMap[] =
    INCBIN_U16("data/layouts/LittlerootTown_MaysHouse_1F/map.bin");
static const u16 ALIGNED(4) sPortBirchLabBorder[] =
    INCBIN_U16("data/layouts/LittlerootTown_ProfessorBirchsLab/border.bin");
static const u16 ALIGNED(4) sPortBirchLabMap[] =
    INCBIN_U16("data/layouts/LittlerootTown_ProfessorBirchsLab/map.bin");

static const struct WarpEvent sPortLittlerootWarps[] =
{
    { .x = 14, .y = 8,  .elevation = 0, .warpId = 1, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F),          .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F) },
    { .x = 5,  .y = 8,  .elevation = 0, .warpId = 1, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F),     .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F) },
    { .x = 7,  .y = 16, .elevation = 0, .warpId = 0, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB), .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB) },
};

static const u8 sPortText_LittlerootTownSign[] =
    _("LITTLEROOT TOWN");
static const u8 sPortText_BirchLabSign[] =
    _("PROF. BIRCH'S POKEMON LAB");
static const u8 sPortText_BrendanHouseSign[] =
    _("BRENDAN'S HOUSE");
static const u8 sPortText_MayHouseSign[] =
    _("PROF. BIRCH'S HOUSE");

static const u8 sPortText_LittlerootTwin[] =
    _("If you go into the grass, wild\nPOKéMON will jump out!");
static const u8 sPortText_LittlerootFatMan[] =
    _("The power of science is staggering!");
static const u8 sPortText_LittlerootBoy[] =
    _("PROF. BIRCH studies POKéMON in his LAB\nand out in the wild.");
static const u8 sPortText_NorthExitWarning[] =
    _("It's dangerous to leave town without a POKéMON!");

static const struct CoordEvent sPortLittlerootCoordEvents[] =
{
    { .x = 10, .y = 1, .elevation = 3, .trigger = 0, .index = 0, .script = sPortText_NorthExitWarning },
    { .x = 11, .y = 1, .elevation = 3, .trigger = 0, .index = 0, .script = sPortText_NorthExitWarning },
};

static const struct ObjectEventTemplate sPortLittlerootObjectEvents[] =
{
    {
        .localId = 1,
        .graphicsId = OBJ_EVENT_GFX_TWIN,
        .kind = 0,
        .x = 16,
        .y = 10,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_WANDER_AROUND,
        .movementRangeX = 1,
        .movementRangeY = 2,
        .trainerType = 0,
        .trainerRange_berryTreeId = 0,
        .script = sPortText_LittlerootTwin,
        .flagId = 0,
    },
    {
        .localId = 2,
        .graphicsId = OBJ_EVENT_GFX_FAT_MAN,
        .kind = 0,
        .x = 12,
        .y = 13,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_WANDER_AROUND,
        .movementRangeX = 2,
        .movementRangeY = 1,
        .trainerType = 0,
        .trainerRange_berryTreeId = 0,
        .script = sPortText_LittlerootFatMan,
        .flagId = 0,
    },
    {
        .localId = 3,
        .graphicsId = OBJ_EVENT_GFX_BOY_2,
        .kind = 0,
        .x = 14,
        .y = 17,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_WANDER_AROUND,
        .movementRangeX = 2,
        .movementRangeY = 1,
        .trainerType = 0,
        .trainerRange_berryTreeId = 0,
        .script = sPortText_LittlerootBoy,
        .flagId = 0,
    },
};

static const struct BgEvent sPortLittlerootBgEvents[] =
{
    { .x = 15, .y = 13, .elevation = 0, .kind = 0, .bgUnion.script = sPortText_LittlerootTownSign },
    { .x = 6,  .y = 17, .elevation = 0, .kind = 0, .bgUnion.script = sPortText_BirchLabSign },
    { .x = 7,  .y = 8,  .elevation = 3, .kind = 0, .bgUnion.script = sPortText_BrendanHouseSign },
    { .x = 12, .y = 8,  .elevation = 3, .kind = 0, .bgUnion.script = sPortText_MayHouseSign },
};

static const struct MapEvents sPortLittlerootEvents =
{
    .objectEventCount = ARRAY_COUNT(sPortLittlerootObjectEvents),
    .warpCount = ARRAY_COUNT(sPortLittlerootWarps),
    .coordEventCount = ARRAY_COUNT(sPortLittlerootCoordEvents),
    .bgEventCount = ARRAY_COUNT(sPortLittlerootBgEvents),
    .objectEvents = sPortLittlerootObjectEvents,
    .warps = sPortLittlerootWarps,
    .coordEvents = sPortLittlerootCoordEvents,
    .bgEvents = sPortLittlerootBgEvents,
};

static const struct WarpEvent sPortBrendanHouse1FWarps[] =
{
    { .x = 9, .y = 8, .elevation = 0, .warpId = 1, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN), .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN) },
    { .x = 8, .y = 8, .elevation = 0, .warpId = 1, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN), .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN) },
};

static const struct MapEvents sPortBrendanHouse1FEvents =
{
    .warpCount = ARRAY_COUNT(sPortBrendanHouse1FWarps),
    .warps = sPortBrendanHouse1FWarps,
};

static const struct WarpEvent sPortMayHouse1FWarps[] =
{
    { .x = 1, .y = 8, .elevation = 0, .warpId = 0, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN), .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN) },
    { .x = 2, .y = 8, .elevation = 0, .warpId = 0, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN), .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN) },
};

static const struct MapEvents sPortMayHouse1FEvents =
{
    .warpCount = ARRAY_COUNT(sPortMayHouse1FWarps),
    .warps = sPortMayHouse1FWarps,
};

static const struct WarpEvent sPortBirchLabWarps[] =
{
    { .x = 6, .y = 12, .elevation = 0, .warpId = 2, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN), .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN) },
    { .x = 7, .y = 12, .elevation = 0, .warpId = 2, .mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN), .mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN) },
};

static const u8 sPortText_BirchLabAide[] =
    _("Hunh? PROF. BIRCH?\nThe PROF's away on fieldwork.\nErgo, he isn't here.\nHe prefers fieldwork to desk work.");
static const u8 sPortText_BirchLabMachine[] =
    _("It's a serious-looking machine.\nThe PROF must use this for research.");
static const u8 sPortText_BirchLabPc[] =
    _("It's a PC used for research.\nBetter not mess around with it.");
static const u8 sPortText_BirchLabBookshelf[] =
    _("It's crammed with books on POKéMON.");
static const u8 sPortText_BirchLabBook[] =
    _("It's a book that's too hard to read.");

static const struct ObjectEventTemplate sPortBirchLabObjectEvents[] =
{
    {
        .localId = 1,
        .graphicsId = OBJ_EVENT_GFX_SCIENTIST_1,
        .kind = 0,
        .x = 9,
        .y = 8,
        .elevation = 3,
        .movementType = MOVEMENT_TYPE_WANDER_AROUND,
        .movementRangeX = 1,
        .movementRangeY = 1,
        .trainerType = 0,
        .trainerRange_berryTreeId = 0,
        .script = sPortText_BirchLabAide,
        .flagId = 0,
    },
};

static const struct BgEvent sPortBirchLabBgEvents[] =
{
    { .x = 10, .y = 7,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_NORTH, .bgUnion.script = sPortText_BirchLabMachine },
    { .x = 11, .y = 7,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_NORTH, .bgUnion.script = sPortText_BirchLabMachine },
    { .x = 7,  .y = 1,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabBook },
    { .x = 8,  .y = 1,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabBook },
    { .x = 1,  .y = 1,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabBookshelf },
    { .x = 0,  .y = 7,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabBookshelf },
    { .x = 1,  .y = 7,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabBookshelf },
    { .x = 2,  .y = 7,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabBookshelf },
    { .x = 3,  .y = 7,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabBookshelf },
    { .x = 4,  .y = 1,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabPc },
    { .x = 3,  .y = 1,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabPc },
    { .x = 1,  .y = 10, .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabPc },
    { .x = 1,  .y = 9,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabPc },
    { .x = 11, .y = 10, .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabPc },
    { .x = 11, .y = 9,  .elevation = 0, .kind = BG_EVENT_PLAYER_FACING_ANY,   .bgUnion.script = sPortText_BirchLabPc },
};

static const struct MapEvents sPortBirchLabEvents =
{
    .objectEventCount = ARRAY_COUNT(sPortBirchLabObjectEvents),
    .warpCount = ARRAY_COUNT(sPortBirchLabWarps),
    .bgEventCount = ARRAY_COUNT(sPortBirchLabBgEvents),
    .objectEvents = sPortBirchLabObjectEvents,
    .warps = sPortBirchLabWarps,
    .bgEvents = sPortBirchLabBgEvents,
};

const struct MapLayout gPortLittlerootLayout =
{
    .width = 20,
    .height = 20,
    .border = sPortLittlerootBorder,
    .map = sPortLittlerootMap,
    .primaryTileset = &sPortTilesetGeneral,
    .secondaryTileset = &sPortTilesetPetalburg,
    .isFrlg = FALSE,
    .borderWidth = 2,
    .borderHeight = 2,
};

const struct MapHeader gPortLittlerootHeader =
{
    .mapLayout = &gPortLittlerootLayout,
    .events = &sPortLittlerootEvents,
    .mapScripts = NULL,
    .connections = NULL,
    .music = 0,
    .mapLayoutId = LAYOUT_LITTLEROOT_TOWN,
    .regionMapSectionId = MAPSEC_LITTLEROOT_TOWN,
    .weather = WEATHER_NONE,
    .mapType = MAP_TYPE_TOWN,
    .floorNumber = 0,
    .nightMusic = 0,
    .allowCycling = TRUE,
    .allowEscaping = FALSE,
    .allowRunning = TRUE,
    .showMapName = FALSE,
    .cave = FALSE,
    .battleType = 0,
};

const struct MapLayout gPortBrendanHouse1FLayout =
{
    .width = 11,
    .height = 9,
    .border = sPortBrendanHouse1FBorder,
    .map = sPortBrendanHouse1FMap,
    .primaryTileset = &sPortTilesetBuilding,
    .secondaryTileset = &sPortTilesetBrendansMaysHouse,
    .isFrlg = FALSE,
    .borderWidth = 2,
    .borderHeight = 2,
};

const struct MapLayout gPortMayHouse1FLayout =
{
    .width = 11,
    .height = 9,
    .border = sPortMayHouse1FBorder,
    .map = sPortMayHouse1FMap,
    .primaryTileset = &sPortTilesetBuilding,
    .secondaryTileset = &sPortTilesetBrendansMaysHouse,
    .isFrlg = FALSE,
    .borderWidth = 2,
    .borderHeight = 2,
};

const struct MapLayout gPortBirchLabLayout =
{
    .width = 13,
    .height = 13,
    .border = sPortBirchLabBorder,
    .map = sPortBirchLabMap,
    .primaryTileset = &sPortTilesetBuilding,
    .secondaryTileset = &sPortTilesetLab,
    .isFrlg = FALSE,
    .borderWidth = 2,
    .borderHeight = 2,
};

const struct MapHeader gPortBrendanHouse1FHeader =
{
    .mapLayout = &gPortBrendanHouse1FLayout,
    .events = &sPortBrendanHouse1FEvents,
    .mapScripts = NULL,
    .connections = NULL,
    .music = 0,
    .mapLayoutId = LAYOUT_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F,
    .regionMapSectionId = MAPSEC_LITTLEROOT_TOWN,
    .weather = WEATHER_NONE,
    .mapType = MAP_TYPE_INDOOR,
    .allowRunning = FALSE,
};

const struct MapHeader gPortMayHouse1FHeader =
{
    .mapLayout = &gPortMayHouse1FLayout,
    .events = &sPortMayHouse1FEvents,
    .mapScripts = NULL,
    .connections = NULL,
    .music = 0,
    .mapLayoutId = LAYOUT_LITTLEROOT_TOWN_MAYS_HOUSE_1F,
    .regionMapSectionId = MAPSEC_LITTLEROOT_TOWN,
    .weather = WEATHER_NONE,
    .mapType = MAP_TYPE_INDOOR,
    .allowRunning = FALSE,
};

const struct MapHeader gPortBirchLabHeader =
{
    .mapLayout = &gPortBirchLabLayout,
    .events = &sPortBirchLabEvents,
    .mapScripts = NULL,
    .connections = NULL,
    .music = 0,
    .mapLayoutId = LAYOUT_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB,
    .regionMapSectionId = MAPSEC_LITTLEROOT_TOWN,
    .weather = WEATHER_NONE,
    .mapType = MAP_TYPE_INDOOR,
    .allowRunning = FALSE,
};

const struct MapHeader *const Overworld_GetMapHeaderByGroupAndId(u16 mapGroup, u16 mapNum)
{
    if (mapGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN)
     && mapNum == MAP_NUM(MAP_LITTLEROOT_TOWN))
        return &gPortLittlerootHeader;
    if (mapGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F)
     && mapNum == MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F))
        return &gPortBrendanHouse1FHeader;
    if (mapGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F)
     && mapNum == MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F))
        return &gPortMayHouse1FHeader;
    if (mapGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB)
     && mapNum == MAP_NUM(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB))
        return &gPortBirchLabHeader;
    return &gPortLittlerootHeader;
}
