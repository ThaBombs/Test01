#include "port_gba_bios.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "gba/types.h"
#include "gba/syscall.h"

// The modern headers wrap CpuSet/CpuFastSet in compile-time alignment macros.
// The host implementation must provide the underlying function symbols.
#ifdef CpuSet
#undef CpuSet
#endif
#ifdef CpuFastSet
#undef CpuFastSet
#endif

#define BIOS_COPY_COUNT_MASK 0x001FFFFFu
#define BIOS_SRC_FIXED       0x01000000u
#define BIOS_32BIT           0x04000000u

static uint32_t ReadU32(const void *ptr)
{
    uint32_t value;
    memcpy(&value, ptr, sizeof(value));
    return value;
}

static uint16_t ReadU16(const void *ptr)
{
    uint16_t value;
    memcpy(&value, ptr, sizeof(value));
    return value;
}

static void WriteU32(void *ptr, uint32_t value)
{
    memcpy(ptr, &value, sizeof(value));
}

static void WriteU16(void *ptr, uint16_t value)
{
    memcpy(ptr, &value, sizeof(value));
}

void CpuSet(const void *src, void *dest, u32 control)
{
    const uint32_t count = control & BIOS_COPY_COUNT_MASK;

    if (src == NULL || dest == NULL || count == 0)
        return;

    if ((control & BIOS_32BIT) != 0)
    {
        const uint8_t *source = (const uint8_t *)src;
        uint8_t *target = (uint8_t *)dest;

        if ((control & BIOS_SRC_FIXED) != 0)
        {
            const uint32_t value = ReadU32(source);
            for (uint32_t i = 0; i < count; ++i)
                WriteU32(target + i * sizeof(uint32_t), value);
        }
        else
        {
            memmove(target, source, (size_t)count * sizeof(uint32_t));
        }
    }
    else
    {
        const uint8_t *source = (const uint8_t *)src;
        uint8_t *target = (uint8_t *)dest;

        if ((control & BIOS_SRC_FIXED) != 0)
        {
            const uint16_t value = ReadU16(source);
            for (uint32_t i = 0; i < count; ++i)
                WriteU16(target + i * sizeof(uint16_t), value);
        }
        else
        {
            memmove(target, source, (size_t)count * sizeof(uint16_t));
        }
    }
}

void CpuFastSet(const void *src, void *dest, u32 control)
{
    const uint32_t count = control & BIOS_COPY_COUNT_MASK;

    if (src == NULL || dest == NULL || count == 0)
        return;

    const uint8_t *source = (const uint8_t *)src;
    uint8_t *target = (uint8_t *)dest;

    if ((control & BIOS_SRC_FIXED) != 0)
    {
        const uint32_t value = ReadU32(source);
        for (uint32_t i = 0; i < count; ++i)
            WriteU32(target + i * sizeof(uint32_t), value);
    }
    else
    {
        memmove(target, source, (size_t)count * sizeof(uint32_t));
    }
}

static void Lz77Decompress(const uint8_t *source, uint8_t *target)
{
    if (source == NULL || target == NULL || source[0] != 0x10)
        return;

    const uint32_t outputSize =
        (uint32_t)source[1]
        | ((uint32_t)source[2] << 8)
        | ((uint32_t)source[3] << 16);

    source += 4;
    uint32_t out = 0;

    while (out < outputSize)
    {
        uint8_t flags = *source++;

        for (int bit = 7; bit >= 0 && out < outputSize; --bit)
        {
            if ((flags & (1u << bit)) == 0)
            {
                target[out++] = *source++;
                continue;
            }

            const uint8_t first = *source++;
            const uint8_t second = *source++;
            const uint32_t length = (uint32_t)(first >> 4) + 3u;
            const uint32_t distance =
                (((uint32_t)first & 0x0Fu) << 8) + (uint32_t)second + 1u;

            if (distance > out)
                return;

            for (uint32_t i = 0; i < length && out < outputSize; ++i)
            {
                target[out] = target[out - distance];
                ++out;
            }
        }
    }
}

void LZ77UnCompWram(const u32 *src, void *dest)
{
    Lz77Decompress((const uint8_t *)src, (uint8_t *)dest);
}

void LZ77UnCompVram(const u32 *src, void *dest)
{
    // Host VRAM is normal byte-addressable memory, so it does not need the
    // halfword-write restriction of physical GBA VRAM.
    Lz77Decompress((const uint8_t *)src, (uint8_t *)dest);
}

static void RlDecompress(const uint8_t *source, uint8_t *target)
{
    if (source == NULL || target == NULL || source[0] != 0x30)
        return;

    const uint32_t outputSize =
        (uint32_t)source[1]
        | ((uint32_t)source[2] << 8)
        | ((uint32_t)source[3] << 16);

    source += 4;
    uint32_t out = 0;

    while (out < outputSize)
    {
        const uint8_t header = *source++;

        if ((header & 0x80u) != 0)
        {
            const uint32_t length = (uint32_t)(header & 0x7Fu) + 3u;
            const uint8_t value = *source++;
            for (uint32_t i = 0; i < length && out < outputSize; ++i)
                target[out++] = value;
        }
        else
        {
            const uint32_t length = (uint32_t)(header & 0x7Fu) + 1u;
            for (uint32_t i = 0; i < length && out < outputSize; ++i)
                target[out++] = *source++;
        }
    }
}

void RLUnCompWram(const u32 *src, void *dest)
{
    RlDecompress((const uint8_t *)src, (uint8_t *)dest);
}

void RLUnCompVram(const u32 *src, void *dest)
{
    RlDecompress((const uint8_t *)src, (uint8_t *)dest);
}

s32 Div(s32 num, s32 denom)
{
    if (denom == 0)
        return 0;
    return num / denom;
}

u16 Sqrt(u32 num)
{
    uint32_t result = 0;
    uint32_t bit = 1u << 30;

    while (bit > num)
        bit >>= 2;

    while (bit != 0)
    {
        if (num >= result + bit)
        {
            num -= result + bit;
            result = (result >> 1) + bit;
        }
        else
        {
            result >>= 1;
        }
        bit >>= 2;
    }

    return (u16)result;
}

u16 ArcTan2(s16 x, s16 y)
{
    const double tau = 6.283185307179586476925286766559;
    double angle = atan2((double)y, (double)x);

    if (angle < 0.0)
        angle += tau;

    return (u16)(angle * (65536.0 / tau));
}

void BgAffineSet(struct BgAffineSrcData *src, struct BgAffineDstData *dest, s32 count)
{
    if (src == NULL || dest == NULL || count <= 0)
        return;

    const double tau = 6.283185307179586476925286766559;

    for (s32 i = 0; i < count; ++i)
    {
        const double angle = ((double)src[i].alpha / 65536.0) * tau;
        const double cosine = cos(angle);
        const double sine = sin(angle);

        const s32 pa = (s32)lrint(((double)src[i].sx * cosine));
        const s32 pb = (s32)lrint(-((double)src[i].sx * sine));
        const s32 pc = (s32)lrint(((double)src[i].sy * sine));
        const s32 pd = (s32)lrint(((double)src[i].sy * cosine));

        dest[i].pa = (s16)pa;
        dest[i].pb = (s16)pb;
        dest[i].pc = (s16)pc;
        dest[i].pd = (s16)pd;
        dest[i].dx = src[i].texX - pa * src[i].scrX - pb * src[i].scrY;
        dest[i].dy = src[i].texY - pc * src[i].scrX - pd * src[i].scrY;
    }
}

void ObjAffineSet(struct ObjAffineSrcData *src, void *dest, s32 count, s32 offset)
{
    if (src == NULL || dest == NULL || count <= 0 || offset <= 0)
        return;

    const double tau = 6.283185307179586476925286766559;
    uint8_t *output = (uint8_t *)dest;

    for (s32 i = 0; i < count; ++i)
    {
        const double angle = ((double)src[i].rotation / 65536.0) * tau;
        const double cosine = cos(angle);
        const double sine = sin(angle);
        const s16 values[4] = {
            (s16)lrint((double)src[i].xScale * cosine),
            (s16)lrint(-((double)src[i].xScale * sine)),
            (s16)lrint((double)src[i].yScale * sine),
            (s16)lrint((double)src[i].yScale * cosine),
        };

        for (int j = 0; j < 4; ++j)
        {
            WriteU16(output, (uint16_t)values[j]);
            output += offset;
        }
    }
}

bool PortGbaBios_SelfTest(void)
{
    uint16_t source16[4] = {0x1111, 0x2222, 0x3333, 0x4444};
    uint16_t copied16[4] = {0};
    uint32_t fill32 = 0xAABBCCDDu;
    uint32_t copied32[4] = {0};

    CpuSet(source16, copied16, 4u);
    CpuSet(&fill32, copied32, BIOS_SRC_FIXED | BIOS_32BIT | 4u);

    // "ABABABAB" -> literals A/B, then one six-byte backreference.
    const uint8_t lzData[] = {
        0x10, 0x08, 0x00, 0x00,
        0x20,
        'A', 'B',
        0x30, 0x01
    };
    uint8_t lzOut[8] = {0};
    LZ77UnCompWram((const u32 *)lzData, lzOut);

    // Four literal bytes followed by four repeated bytes.
    const uint8_t rlData[] = {
        0x30, 0x08, 0x00, 0x00,
        0x03, 'T', 'E', 'S', 'T',
        0x81, '!'
    };
    uint8_t rlOut[8] = {0};
    RLUnCompWram((const u32 *)rlData, rlOut);

    const uint8_t expectedLz[8] = {'A','B','A','B','A','B','A','B'};
    const uint8_t expectedRl[8] = {'T','E','S','T','!','!','!','!'};

    return memcmp(source16, copied16, sizeof(source16)) == 0
        && copied32[0] == fill32
        && copied32[1] == fill32
        && copied32[2] == fill32
        && copied32[3] == fill32
        && memcmp(lzOut, expectedLz, sizeof(expectedLz)) == 0
        && memcmp(rlOut, expectedRl, sizeof(expectedRl)) == 0
        && Sqrt(144u) == 12u
        && Div(84, 7) == 12;
}
