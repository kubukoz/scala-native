#ifndef IMMIX_HEAP_H
#define IMMIX_HEAP_H

#include "shared/GCTypes.h"
#include "datastructures/Stack.h"
#include "datastructures/Bytemap.h"
#include "metadata/LineMeta.h"
#include "Stats.h"
#include "shared/ThreadUtil.h"

#ifdef PD_DEBUG
extern void pd_log_error(char *str, ...);
#endif

typedef struct {
    word_t *blockMetaStart;
    word_t *blockMetaEnd;
    word_t *lineMetaStart;
    word_t *lineMetaEnd;
    word_t *heapStart;
    word_t *heapEnd;
    size_t heapSize;
    size_t maxHeapSize;
    uint32_t blockCount;
    uint32_t maxBlockCount;
    Bytemap *bytemap;
    Stats *stats;
    mutex_t lock;
} Heap;

static inline bool Heap_IsWordInHeap(Heap *heap, word_t *word) {
    // #ifdef PD_DEBUG
    // pd_log_error("Heap_IsWordInHeap: word=%p, heapStart=%p, heapEnd=%p",
    // word, heap->heapStart, heap->heapEnd); #endif

    return word >= heap->heapStart && word < heap->heapEnd;
}

static inline LineMeta *Heap_LineMetaForWord(Heap *heap, word_t *word) {
    // assumes there are no gaps between lines
    assert(LINE_COUNT * LINE_SIZE == BLOCK_TOTAL_SIZE);
    assert(Heap_IsWordInHeap(heap, word));
    word_t lineGlobalIndex =
        ((word_t)word - (word_t)heap->heapStart) >> LINE_SIZE_BITS;
    assert(lineGlobalIndex >= 0);
    LineMeta *lineMeta = (LineMeta *)heap->lineMetaStart + lineGlobalIndex;
    assert(lineMeta < (LineMeta *)heap->lineMetaEnd);
    return lineMeta;
}

void Heap_Init(Heap *heap, size_t minHeapSize, size_t maxHeapSize);

bool Heap_isGrowingPossible(Heap *heap, uint32_t incrementInBlocks);
void Heap_Collect(Heap *heap, Stack *stack);
void Heap_Recycle(Heap *heap);
void Heap_Grow(Heap *heap, uint32_t increment);
void Heap_exitWithOutOfMemory(const char *details);
size_t Heap_getMemoryLimit();

#endif // IMMIX_HEAP_H
