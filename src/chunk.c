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

    //printf("%d\n", iterator->file_desc);
    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->current;
    chunk->to_BlockId = iterator->current + iterator->blocksInChunk - 1;
    chunk->blocksInChunk = iterator->blocksInChunk;

    
    chunk->recordsInChunk = 0;  // Initialize to 0
    for (int id = chunk->from_BlockId; id <= chunk->to_BlockId; id++) {
        chunk->recordsInChunk += HP_GetRecordCounter(chunk->file_desc, id);
    }
    //printf("%d\n", chunk->recordsInChunk);
    iterator->current += iterator->blocksInChunk;
    iterator->lastBlocksID = iterator->current + iterator->blocksInChunk - 1;

    return 0;
}

int CHUNK_GetIthRecordInChunk(CHUNK* chunk,  int i, Record* record){
    CHUNK_RecordIterator record_iterator = CHUNK_CreateRecordIterator(chunk);

    for (int j = 0; j < i; j++) {
        CHUNK_GetNextRecord(&record_iterator, record);
    }

    return 0;
}

int CHUNK_UpdateIthRecord(CHUNK* chunk, int i, Record record){
    int records_in_block = HP_GetMaxRecordsInBlock(chunk->file_desc);
    int position = i % records_in_block;
    int block_id = i / records_in_block + chunk->from_BlockId;

    HP_UpdateRecord(chunk->file_desc, block_id, position, record);
    HP_Unpin(chunk->file_desc, block_id);

    return 0;
}

void CHUNK_Print(CHUNK chunk){
    Record currentRecord;
    
    for (int i = 1; i <= chunk.recordsInChunk; i++) {
        CHUNK_GetIthRecordInChunk(&chunk, i, &currentRecord);
        printRecord(currentRecord);
    }
}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {
    CHUNK_RecordIterator iterator;
    iterator.chunk = *chunk;
    iterator.currentBlockId = chunk->from_BlockId;
    iterator.cursor = 0;

    return iterator;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record* record){
    HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId, iterator->cursor, record);

    int records_in_block = HP_GetRecordCounter(iterator->chunk.file_desc, iterator->currentBlockId);

    if (records_in_block == iterator->cursor + 1) {
        iterator->cursor = 0;
        HP_Unpin(iterator->chunk.file_desc, iterator->currentBlockId);
        iterator->currentBlockId++;
    }
    else {
        iterator->cursor++;
    }

    return 1;
}
