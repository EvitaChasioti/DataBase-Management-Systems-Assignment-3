#include <merge.h>
#include <stdio.h>
#include "chunk.h"


CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator iterator;
    iterator.file_desc = fileDesc;
    iterator.current = 1;  // Starting from block 1
    iterator.lastBlocksID = blocksInChunk;  // Initialize to 0
    iterator.blocksInChunk = blocksInChunk;

    return iterator;
}

int CHUNK_GetNext(CHUNK_Iterator* iterator, CHUNK* chunk) {
    if (iterator->current > iterator->lastBlocksID) {
        // No more chunks to iterate
        return -1;
    }

    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->current;
    chunk->to_BlockId = iterator->current + iterator->blocksInChunk - 1;
    chunk->blocksInChunk = iterator->blocksInChunk;
    
    chunk->recordsInChunk = 0;  // Initialize to 0
    for (int id = chunk->from_BlockId; id < chunk->to_BlockId; id++) {
        chunk->recordsInChunk += HP_GetRecordCounter(chunk->file_desc, id);
    }

    iterator->current += iterator->blocksInChunk;
    iterator->lastBlocksID = iterator->current + iterator->blocksInChunk - 1;

    return 0;
}

int CHUNK_GetIthRecordInChunk(CHUNK* chunk,  int i, Record* record){
    CHUNK_RecordIterator record_iterator = CHUNK_CreateRecordIterator(chunk);

    for (int j = 0; j < i; j++)
        if (CHUNK_GetNextRecord(&record_iterator, record) == -1)
            return -1;

    return 0;
}

int CHUNK_UpdateIthRecord(CHUNK* chunk,  int i, Record record){
    Record* current_record;
    if (CHUNK_GetIthRecordInChunk(chunk, i, current_record) == -1)
        return -1;

    current_record = &record;
    return 0;
}

void CHUNK_Print(CHUNK chunk){
    // Bariemai
}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {
    CHUNK_RecordIterator iterator;
    iterator.chunk = *chunk;
    iterator.currentBlockId = chunk->from_BlockId;
    iterator.cursor = 0;

    return iterator;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator,Record* record){
    if (HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId, iterator->cursor + 1, record) == -1)
        return -1;

    iterator->cursor++;
    return 1;
}
