#include "global.h"
#include "overworld.h"
#include "constants/layouts.h"
#include "constants/map_types.h"
#include "constants/region_map_sections.h"
#include "constants/weather.h"

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
    INCGFX_U32("data/tilesets/primary/general/tiles.png", ".4bpp.smol");
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
    INCGFX_U32("data/tilesets/secondary/petalburg/tiles.png", ".4bpp.fastSmol", "-num_tiles 159 -Wnum_tiles");
const u16 gPortMetatiles_Petalburg[] =
    INCBIN_U16("data/tilesets/secondary/petalburg/metatiles.bin");
const u16 gPortMetatileAttributes_Petalburg[] =
    INCBIN_U16("data/tilesets/secondary/petalburg/metatile_attributes.bin");

static const struct Tileset sPortTilesetGeneral =
{
    .isCompressed = TRUE,
    .isSecondary = FALSE,
    .tiles = gPortTilesetTiles_General,
    .palettes = gPortTilesetPalettes_General,
    .metatiles = gPortMetatiles_General,
    .metatileAttributes = gPortMetatileAttributes_General,
    .callback = NULL,
};

static const struct Tileset sPortTilesetPetalburg =
{
    .isCompressed = TRUE,
    .isSecondary = TRUE,
    .tiles = gPortTilesetTiles_Petalburg,
    .palettes = gPortTilesetPalettes_Petalburg,
    .metatiles = gPortMetatiles_Petalburg,
    .metatileAttributes = gPortMetatileAttributes_Petalburg,
    .callback = NULL,
};

static const u16 ALIGNED(4) sPortLittlerootBorder[] =
    INCBIN_U16("data/layouts/LittlerootTown/border.bin");
static const u16 ALIGNED(4) sPortLittlerootMap[] =
    INCBIN_U16("data/layouts/LittlerootTown/map.bin");

static const struct MapEvents sPortEmptyEvents = {0};

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
    .events = &sPortEmptyEvents,
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

const struct MapHeader *const Overworld_GetMapHeaderByGroupAndId(u16 mapGroup, u16 mapNum)
{
    if (mapGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN)
     && mapNum == MAP_NUM(MAP_LITTLEROOT_TOWN))
        return &gPortLittlerootHeader;
    return &gPortLittlerootHeader;
}
