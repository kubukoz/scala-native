#if defined(SCALANATIVE_GC_IMMIX)

#include "Bytemap.h"
#include <stdio.h>
#include "immix_commix/utils/MathUtils.h"

extern void assertOr(int condition, char *message);
#ifdef PD_DEBUG
extern void pd_log_error(char *str, ...);
#endif

// [t=437] bytemap: 0x9007fbf0, firstAddress: 0x9011c000, size: 10485760,
// ALLOCATION_ALIGNMENT: 16, bytemap->size: 655360, Bytemap_index: 1310720z
// [t=451] Bytemap_index(bytemap, (word_t *)((ubyte_t *)(firstAddress) + size) -
// ALLOCATION_ALIGNMENT) < bytemap->size

void Bytemap_Init(Bytemap *bytemap, word_t *firstAddress, size_t size) {
    bytemap->firstAddress = firstAddress;
    bytemap->size = size / ALLOCATION_ALIGNMENT;
    bytemap->end = &bytemap->data[bytemap->size];
    bool cond =
        Bytemap_index(bytemap, (word_t *)((ubyte_t *)(firstAddress) + size) -
                                   ALLOCATION_ALIGNMENT) < bytemap->size;

    if (!cond) {
#ifdef PD_DEBUG
        pd_log_error(
            "bytemap: %p, firstAddress: %p, size: %d, ALLOCATION_ALIGNMENT: "
            "%d, bytemap->size: %d, Bytemap_index: %zu",
            bytemap, firstAddress, size, ALLOCATION_ALIGNMENT, bytemap->size,
            Bytemap_index(bytemap,
                          (word_t *)((ubyte_t *)(firstAddress) + size)));
#endif
    }

    assertOr(cond,
             "Bytemap_index(bytemap, (word_t *)((ubyte_t *)(firstAddress) + "
             "size) - ALLOCATION_ALIGNMENT) < bytemap->size");
}

#endif
