#include "port_test_npc.h"
#include "port_test_dialogue.h"
#include "port_test_overworld.h"

#include "global.h"
#include "field_camera.h"
#include "main.h"
#include "sprite.h"
#include "constants/event_objects.h"

#define PORT_NPC_MAX 8
#define PORT_NPC_PAL_TAG_1 0x7F11
#define PORT_NPC_PAL_TAG_2 0x7F12

enum
{
    PORT_NPC_FACE_SOUTH,
    PORT_NPC_FACE_NORTH,
    PORT_NPC_FACE_WEST,
    PORT_NPC_FACE_EAST,
};

static const u32 sPortTwinGfx[] =
    INCGFX_U32("graphics/object_events/pics/people/twin.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u32 sPortFatManGfx[] =
    INCGFX_U32("graphics/object_events/pics/people/fat_man.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u32 sPortBoy2Gfx[] =
    INCGFX_U32("graphics/object_events/pics/people/boy_2.png", ".4bpp", "-mwidth 2 -mheight 4");

static const u16 sPortNpc1Palette[] =
    INCGFX_U16("graphics/object_events/palettes/npc_1.pal", ".gbapal");
static const u16 sPortNpc2Palette[] =
    INCGFX_U16("graphics/object_events/palettes/npc_2.pal", ".gbapal");

static const struct SpriteFrameImage sPortTwinFrames[] =
{
    overworld_ascending_frames(sPortTwinGfx, 2, 4),
};
static const struct SpriteFrameImage sPortFatManFrames[] =
{
    overworld_ascending_frames(sPortFatManGfx, 2, 4),
};
static const struct SpriteFrameImage sPortBoy2Frames[] =
{
    overworld_ascending_frames(sPortBoy2Gfx, 2, 4),
};

static const struct OamData sPortNpcOam =
{
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
    .priority = 2,
};

static const union AnimCmd sPortNpcFaceSouth[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcFaceNorth[] =
{
    ANIMCMD_FRAME(1, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcFaceWest[] =
{
    ANIMCMD_FRAME(2, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcFaceEast[] =
{
    ANIMCMD_FRAME(2, 16, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sPortNpcAnims[] =
{
    [PORT_NPC_FACE_SOUTH] = sPortNpcFaceSouth,
    [PORT_NPC_FACE_NORTH] = sPortNpcFaceNorth,
    [PORT_NPC_FACE_WEST] = sPortNpcFaceWest,
    [PORT_NPC_FACE_EAST] = sPortNpcFaceEast,
};

static const struct SpritePalette sPortNpcPalette1 =
{
    .data = sPortNpc1Palette,
    .tag = PORT_NPC_PAL_TAG_1,
};
static const struct SpritePalette sPortNpcPalette2 =
{
    .data = sPortNpc2Palette,
    .tag = PORT_NPC_PAL_TAG_2,
};

static const struct SpriteTemplate sPortTwinTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_2,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortTwinFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortFatManTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_1,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortFatManFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortBoy2Template =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_1,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortBoy2Frames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

struct PortNpcRuntime
{
    const struct ObjectEventTemplate *event;
    u8 spriteId;
};

static struct PortNpcRuntime sPortNpcs[PORT_NPC_MAX];
static u8 sPortNpcCount;

static const struct SpriteTemplate *GetPortNpcSpriteTemplate(u16 graphicsId)
{
    switch (graphicsId)
    {
    case OBJ_EVENT_GFX_TWIN:
        return &sPortTwinTemplate;
    case OBJ_EVENT_GFX_FAT_MAN:
        return &sPortFatManTemplate;
    case OBJ_EVENT_GFX_BOY_2:
        return &sPortBoy2Template;
    default:
        return NULL;
    }
}

static s16 GetCameraTileCompensation(s16 offset, s16 speed)
{
    if (offset == 0)
        return 0;
    if (speed > 0)
        return 16 - offset;
    if (speed < 0)
        return -16 - offset;
    return 0;
}

static void UpdateNpcScreenPosition(struct PortNpcRuntime *npc)
{
    if (npc->spriteId >= MAX_SPRITES || npc->event == NULL)
        return;

    const s16 playerX = gSaveBlock1Ptr->pos.x;
    const s16 playerY = gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS;
    const s16 cameraX = GetCameraTileCompensation(
        gFieldCamera.x,
        gFieldCamera.movementSpeedX);
    const s16 cameraY = GetCameraTileCompensation(
        gFieldCamera.y,
        gFieldCamera.movementSpeedY);

    struct Sprite *sprite = &gSprites[npc->spriteId];
    sprite->x = DISPLAY_WIDTH / 2
        + (npc->event->x - playerX) * 16
        + cameraX;
    sprite->y = DISPLAY_HEIGHT / 2
        + (npc->event->y - playerY) * 16
        + cameraY;

    sprite->invisible =
        sprite->x < -16 || sprite->x > DISPLAY_WIDTH + 16
        || sprite->y < -32 || sprite->y > DISPLAY_HEIGHT + 32;
}

static void DestroyPortNpcs(void)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        if (sPortNpcs[i].spriteId < MAX_SPRITES)
            DestroySprite(&gSprites[sPortNpcs[i].spriteId]);
    }

    memset(sPortNpcs, 0, sizeof(sPortNpcs));
    sPortNpcCount = 0;
}

void PortTestNpc_LoadMap(void)
{
    DestroyPortNpcs();

    const struct MapEvents *events = gMapHeader.events;
    if (events == NULL || events->objectEvents == NULL)
        return;

    LoadSpritePalette(&sPortNpcPalette1);
    LoadSpritePalette(&sPortNpcPalette2);

    for (u32 i = 0; i < events->objectEventCount && sPortNpcCount < PORT_NPC_MAX; ++i)
    {
        const struct ObjectEventTemplate *event = &events->objectEvents[i];
        const struct SpriteTemplate *template =
            GetPortNpcSpriteTemplate(event->graphicsId);
        if (template == NULL)
            continue;

        const u8 spriteId = CreateSprite(
            template,
            DISPLAY_WIDTH / 2,
            DISPLAY_HEIGHT / 2,
            1);
        if (spriteId >= MAX_SPRITES)
            continue;

        sPortNpcs[sPortNpcCount].event = event;
        sPortNpcs[sPortNpcCount].spriteId = spriteId;
        ++sPortNpcCount;
    }

    PortTestNpc_Update();
}

void PortTestNpc_Update(void)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
        UpdateNpcScreenPosition(&sPortNpcs[i]);
}

bool32 PortTestNpc_BlocksTile(s16 x, s16 y)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        const struct ObjectEventTemplate *event = sPortNpcs[i].event;
        if (event != NULL && event->x == x && event->y == y)
            return TRUE;
    }

    return FALSE;
}

bool32 PortTestNpc_TryInteractAt(s16 x, s16 y, s16 playerX, s16 playerY)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        struct PortNpcRuntime *npc = &sPortNpcs[i];
        const struct ObjectEventTemplate *event = npc->event;
        if (event == NULL || event->x != x || event->y != y)
            continue;

        u8 faceAnim = PORT_NPC_FACE_SOUTH;
        if (playerY < event->y)
            faceAnim = PORT_NPC_FACE_NORTH;
        else if (playerY > event->y)
            faceAnim = PORT_NPC_FACE_SOUTH;
        else if (playerX < event->x)
            faceAnim = PORT_NPC_FACE_WEST;
        else if (playerX > event->x)
            faceAnim = PORT_NPC_FACE_EAST;

        if (npc->spriteId < MAX_SPRITES)
            StartSpriteAnimIfDifferent(&gSprites[npc->spriteId], faceAnim);

        if (event->script != NULL)
            return PortTestDialogue_Open(event->script);

        return TRUE;
    }

    return FALSE;
}
