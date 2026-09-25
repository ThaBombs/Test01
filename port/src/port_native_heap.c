#include <stdlib.h>

#include "global.h"
#include "load_save.h"
#include "decompress.h"
#include "malloc.h"

ALIGNED(4) u8 gHeap[HEAP_SIZE] = {0};

void InitHeap(void *heapStart, u32 heapSize)
{
    // Native builds are not constrained to the GBA's fixed EWRAM heap.
    // Keep the API so existing game code remains unchanged.
    (void)heapStart;
    (void)heapSize;
}


void MoveSaveBlocks_ResetHeap(void)
{
    // The GBA routine temporarily copies and relocates save blocks inside
    // EWRAM before resetting its fixed heap. Android keeps stable native save
    // pointers and uses the host allocator, so relocation is neither required
    // nor safe here. Preserve the Emerald call boundary and reset the native
    // heap API in place.
    InitHeap(gHeap, HEAP_SIZE);
}

void *AllocUnchecked_(u32 size, const char *location)
{
    (void)location;
    return malloc(size == 0 ? 1u : (size_t)size);
}

void *Alloc_(u32 size, const char *location)
{
    void *memory = AllocUnchecked_(size, location);
    if (memory == NULL)
        abort();
    return memory;
}

void *AllocZeroedUnchecked_(u32 size, const char *location)
{
    (void)location;
    return calloc(1u, size == 0 ? 1u : (size_t)size);
}

void *AllocZeroed_(u32 size, const char *location)
{
    void *memory = AllocZeroedUnchecked_(size, location);
    if (memory == NULL)
        abort();
    return memory;
}

void Free(void *pointer)
{
    free(pointer);
}

void PrintHeap(void)
{
}

const struct MemBlock *HeapHead(void)
{
    return NULL;
}

const char *MemBlockLocation(const struct MemBlock *block)
{
    (void)block;
    return NULL;
}


void *malloc_and_decompress(const void *src, u32 *size)
{
    const u32 decompressedSize = GetDecompressedDataSize(src);
    if (size != NULL)
        *size = decompressedSize;

    void *buffer = Alloc(decompressedSize);
    if (buffer != NULL)
        DecompressDataWithHeaderWram(src, buffer);

    return buffer;
}
