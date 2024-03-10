#if defined(SCALANATIVE_GC_IMMIX)

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "Object.h"
#include "Block.h"
#include "immix_commix/Log.h"
#include "immix_commix/utils/MathUtils.h"
#include <string.h>

#ifdef PD_DEBUG
extern void pd_log_error(char *str, ...);
#endif

extern void assertOr(int condition, char *message);

word_t *Object_LastWord(Object *object) {
    size_t size = Object_Size(object);
    assertOr(size < LARGE_BLOCK_SIZE, "size < LARGE_BLOCK_SIZE");
    word_t *last = (word_t *)((ubyte_t *)object + size) - 1;
    return last;
}

Object *Object_getInnerPointer(Heap *heap, BlockMeta *blockMeta, word_t *word,
                               ObjectMeta *wordMeta) {
    int stride;
    word_t *blockStart;
    if (BlockMeta_ContainsLargeObjects(blockMeta)) {
        stride = MIN_BLOCK_SIZE / ALLOCATION_ALIGNMENT;
        BlockMeta *superblockStart =
            BlockMeta_GetSuperblockStart(heap->blockMetaStart, blockMeta);
        blockStart = BlockMeta_GetBlockStart(heap->blockMetaStart,
                                             heap->heapStart, superblockStart);
    } else {
        stride = 1;
        blockStart = Block_GetBlockStartForWord(word);
    }

    word_t *current = word;
    ObjectMeta *currentMeta = wordMeta;
    while (current >= blockStart && ObjectMeta_IsFree(currentMeta)) {
        current -= ALLOCATION_ALIGNMENT_WORDS * stride;
        currentMeta -= stride;
    }
    Object *object = (Object *)current;
    if (ObjectMeta_IsAllocated(currentMeta) &&
        (ubyte_t *)word < (ubyte_t *)current + Object_Size(object)) {
        return object;
    } else {
        return NULL;
    }
}

Object *Object_GetUnmarkedObject(Heap *heap, word_t *word) {
    BlockMeta *blockMeta =
        Block_GetBlockMeta(heap->blockMetaStart, heap->heapStart, word);

    if (BlockMeta_ContainsLargeObjects(blockMeta)) {
        word = (word_t *)((word_t)word & LARGE_BLOCK_MASK);
    } else {
        word = (word_t *)((word_t)word & ALLOCATION_ALIGNMENT_INVERSE_MASK);
    }

    ObjectMeta *wordMeta = Bytemap_Get(heap->bytemap, word);
    if (ObjectMeta_IsPlaceholder(wordMeta) || ObjectMeta_IsMarked(wordMeta)) {
        return NULL;
    } else if (ObjectMeta_IsAllocated(wordMeta)) {
        return (Object *)word;
    } else {
        return Object_getInnerPointer(heap, blockMeta, word, wordMeta);
    }
}

void Object_Mark(Heap *heap, Object *object, ObjectMeta *objectMeta) {
    // Mark the object itself
    ObjectMeta_SetMarked(objectMeta);

    BlockMeta *blockMeta = Block_GetBlockMeta(
        heap->blockMetaStart, heap->heapStart, (word_t *)object);
    if (!BlockMeta_ContainsLargeObjects(blockMeta)) {
        // Mark the block
        word_t *blockStart = Block_GetBlockStartForWord((word_t *)object);
        BlockMeta_Mark(blockMeta);

        // here's the thing: our object is an array. Shouldn't it be handled
        // elsewhere?
        // we're definitely getting here with an array.

        // Mark all Lines
        word_t *lastWord = Object_LastWord(object);

        if (blockMeta != Block_GetBlockMeta(heap->blockMetaStart,
                                            heap->heapStart, lastWord)) {

#ifdef PD_DEBUG
            pd_log_error("blockMeta LHS: %p, RHS: %p", blockMeta,
                         Block_GetBlockMeta(heap->blockMetaStart,
                                            heap->heapStart, lastWord));
            pd_log_error("class %p, size %d, name %p, field count: %d",
                         object->rtti->rt.cls, object->rtti->size,
                         object->rtti->rt.name, Object_Size(object));
            pd_log_error("first word: %p, last word: %p", object, lastWord);
#endif
        }

        assertOr(blockMeta == Block_GetBlockMeta(heap->blockMetaStart,
                                                 heap->heapStart, lastWord),
                 "blockMeta == Block_GetBlockMeta(heap->blockMetaStart, "
                 "heap->heapStart, lastWord)");
        LineMeta *firstLineMeta = Heap_LineMetaForWord(heap, (word_t *)object);
        LineMeta *lastLineMeta = Heap_LineMetaForWord(heap, lastWord);
        assertOr(firstLineMeta <= lastLineMeta,
                 "firstLineMeta <= lastLineMeta");
        for (LineMeta *lineMeta = firstLineMeta; lineMeta <= lastLineMeta;
             lineMeta++) {
            Line_Mark(lineMeta);
        }
    }
}

#endif
