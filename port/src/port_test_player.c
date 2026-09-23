#include "port_test_player.h"

#include "global.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "sprite.h"

#define PORT_PLAYER_PAL_TAG 0x7F01

enum
{
    PORT_ANIM_FACE_SOUTH,
    PORT_ANIM_FACE_NORTH,
    PORT_ANIM_FACE_WEST,
    PORT_ANIM_FACE_EAST,
    PORT_ANIM_WALK_SOUTH,
    PORT_ANIM_WALK_NORTH,
    PORT_ANIM_WALK_WEST,
    PORT_ANIM_WALK_EAST,
};

static const u32 sPortBrendanWalking[] =
    INCGFX_U32("graphics/object_events/pics/people/brendan/walking.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u16 sPortBrendanPalette[] =
    INCGFX_U16("graphics/object_events/palettes/brendan.pal", ".gbapal");

static const struct SpriteFrameImage sPortBrendanFrames[] =
{
    overworld_ascending_frames(sPortBrendanWalking, 2, 4),
};

static const struct OamData sPortBrendanOam =
{
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
    .priority = 2,
};

static const union AnimCmd sFaceSouth[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sFaceNorth[] =
{
    ANIMCMD_FRAME(1, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sFaceWest[] =
{
    ANIMCMD_FRAME(2, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sFaceEast[] =
{
    ANIMCMD_FRAME(2, 16, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sWalkSouth[] =
{
    ANIMCMD_FRAME(3, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_FRAME(4, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sWalkNorth[] =
{
    ANIMCMD_FRAME(5, 4),
    ANIMCMD_FRAME(1, 4),
    ANIMCMD_FRAME(6, 4),
    ANIMCMD_FRAME(1, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sWalkWest[] =
{
    ANIMCMD_FRAME(7, 4),
    ANIMCMD_FRAME(2, 4),
    ANIMCMD_FRAME(8, 4),
    ANIMCMD_FRAME(2, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sWalkEast[] =
{
    ANIMCMD_FRAME(7, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(2, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(8, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(2, 4, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sPortBrendanAnims[] =
{
    sFaceSouth,
    sFaceNorth,
    sFaceWest,
    sFaceEast,
    sWalkSouth,
    sWalkNorth,
    sWalkWest,
    sWalkEast,
};

static const struct SpritePalette sPortBrendanSpritePalette =
{
    .data = sPortBrendanPalette,
    .tag = PORT_PLAYER_PAL_TAG,
};

static const struct SpriteTemplate sPortBrendanTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_PLAYER_PAL_TAG,
    .oam = &sPortBrendanOam,
    .anims = sPortBrendanAnims,
    .images = sPortBrendanFrames,
    .affineAnims = NULL,
    .callback = NULL,
};

static u32 sPortPlayerSpriteId = MAX_SPRITES;
static u8 sPortFacing = PORT_ANIM_FACE_SOUTH;
static u8 sPortWalkTimer;
static u8 sPortRepeatTimer;

static bool32 TryMoveCameraFocus(s16 dx, s16 dy)
{
    const s16 targetX = gSaveBlock1Ptr->pos.x + MAP_OFFSET + dx;
    const s16 targetY = gSaveBlock1Ptr->pos.y + MAP_OFFSET + dy;

    // The map buffer encodes ordinary impassable terrain in its collision bit.
    // Undefined/out-of-map cells also report collision, keeping this test slice
    // safely inside Littleroot until map connections are linked later.
    if (MapGridGetCollisionAt(targetX, targetY) != 0)
        return FALSE;

    gSaveBlock1Ptr->pos.x += dx;
    gSaveBlock1Ptr->pos.y += dy;
    DrawWholeMapView();
    return TRUE;
}

static void FaceAndMaybeMove(u16 keys)
{
    s16 dx = 0;
    s16 dy = 0;
    u8 faceAnim;
    u8 walkAnim;

    if (keys & DPAD_UP)
    {
        dy = -1;
        faceAnim = PORT_ANIM_FACE_NORTH;
        walkAnim = PORT_ANIM_WALK_NORTH;
    }
    else if (keys & DPAD_DOWN)
    {
        dy = 1;
        faceAnim = PORT_ANIM_FACE_SOUTH;
        walkAnim = PORT_ANIM_WALK_SOUTH;
    }
    else if (keys & DPAD_LEFT)
    {
        dx = -1;
        faceAnim = PORT_ANIM_FACE_WEST;
        walkAnim = PORT_ANIM_WALK_WEST;
    }
    else if (keys & DPAD_RIGHT)
    {
        dx = 1;
        faceAnim = PORT_ANIM_FACE_EAST;
        walkAnim = PORT_ANIM_WALK_EAST;
    }
    else
    {
        return;
    }

    sPortFacing = faceAnim;
    if (TryMoveCameraFocus(dx, dy))
    {
        StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], walkAnim);
        sPortWalkTimer = 8;
    }
    else
    {
        StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], faceAnim);
        sPortWalkTimer = 0;
    }

    sPortRepeatTimer = 8;
}

void PortTestPlayer_Init(void)
{
    ResetSpriteData();
    FreeAllSpritePalettes();
    ClearSpriteCopyRequests();

    LoadSpritePalette(&sPortBrendanSpritePalette);
    sPortPlayerSpriteId = CreateSprite(
        &sPortBrendanTemplate,
        DISPLAY_WIDTH / 2,
        DISPLAY_HEIGHT / 2,
        0);

    sPortFacing = PORT_ANIM_FACE_SOUTH;
    sPortWalkTimer = 0;
    sPortRepeatTimer = 0;
    StartSpriteAnim(&gSprites[sPortPlayerSpriteId], sPortFacing);

    SetGpuRegBits(
        REG_OFFSET_DISPCNT,
        DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);

    AnimateSprites();
    BuildOamBuffer();
    LoadOam();
    ProcessSpriteCopyRequests();
}

void PortTestPlayer_Update(void)
{
    if (sPortPlayerSpriteId >= MAX_SPRITES)
        return;

    if (sPortRepeatTimer != 0)
        --sPortRepeatTimer;

    if (gMain.newKeys & DPAD_ANY)
    {
        FaceAndMaybeMove(gMain.newKeys);
    }
    else if ((gMain.heldKeys & DPAD_ANY) && sPortRepeatTimer == 0)
    {
        FaceAndMaybeMove(gMain.heldKeys);
    }

    if (sPortWalkTimer != 0)
    {
        --sPortWalkTimer;
        if (sPortWalkTimer == 0)
            StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], sPortFacing);
    }

    AnimateSprites();
    BuildOamBuffer();
    LoadOam();
    ProcessSpriteCopyRequests();
}
