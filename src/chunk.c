#include <merge.h>
#include <stdio.h>
#include "chunk.h"

// Structure for representing an iterator for CHUNKs
CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator iterator;
    
    // Set the file descriptor for the iterator
    iterator.file_desc = fileDesc;
    
    // Start iterating from block 1
    iterator.current = 1;
    
    // Set the last block ID to 0 (initialized state)
    iterator.lastBlocksID = blocksInChunk;
    
    // Set the number of blocks in each chunk
    iterator.blocksInChunk = blocksInChunk;

    return iterator;
}

// Get the next CHUNK from the iterator
int CHUNK_GetNext(CHUNK_Iterator* iterator, CHUNK* chunk) {
    if (iterator->current > iterator->lastBlocksID) {
        // No more chunks to iterate
        return -1;
    }

    // Copy file descriptor and range of blocks to the CHUNK structure
    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->current;
    chunk->to_BlockId = iterator->current + iterator->blocksInChunk - 1;

    // Adjust to_BlockId if it exceeds the last block ID in the file
    if (chunk->to_BlockId > HP_GetIdOfLastBlock(chunk->file_desc))
        chunk->to_BlockId -= (chunk->to_BlockId - HP_GetIdOfLastBlock(chunk->file_desc));

    // Set the number of blocks in the chunk
    chunk->blocksInChunk = iterator->blocksInChunk;
    
    // Initialize the number of records in the chunk to 0
    chunk->recordsInChunk = 0;

    // Calculate the total number of records in the chunk
    for (int id = chunk->from_BlockId; id <= chunk->to_BlockId; id++) {
        chunk->recordsInChunk += HP_GetRecordCounter(chunk->file_desc, id);
    }

    // Update iterator for the next CHUNK
    iterator->current += iterator->blocksInChunk;
    iterator->lastBlocksID = iterator->current + iterator->blocksInChunk - 1;

    return 0;
}

// Get the ith record in the given chunk
int CHUNK_GetIthRecordInChunk(CHUNK* chunk, int i, Record* record) {
    // Create a record iterator for the given chunk
    CHUNK_RecordIterator record_iterator = CHUNK_CreateRecordIterator(chunk);

    // Iterate to the ith record in the chunk
    for (int j = 0; j < i; j++) {
        CHUNK_GetNextRecord(&record_iterator, record);
    }

    return 0;
}

// Update the ith record in the given chunk
int CHUNK_UpdateIthRecord(CHUNK* chunk, int i, Record record) {
    // Calculate the block and position of the ith record
    int records_in_block = HP_GetMaxRecordsInBlock(chunk->file_desc);
    int position = i % records_in_block;
    int block_id = i / records_in_block + chunk->from_BlockId;

    // Update the record in the block
    HP_UpdateRecord(chunk->file_desc, block_id, position, record);
    
    // Unpin the block after updating the record
    HP_Unpin(chunk->file_desc, block_id);

    return 0;
}

// Print all records in the given chunk
void CHUNK_Print(CHUNK chunk) {
    Record currentRecord;

    // Iterate through all records in the chunk and print each record
    for (int i = 1; i <= chunk.recordsInChunk; i++) {
        CHUNK_GetIthRecordInChunk(&chunk, i, &currentRecord);
        printRecord(currentRecord);
    }
}

// Structure for representing an iterator for records within a CHUNK
CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK* chunk) {
    CHUNK_RecordIterator iterator;
    
    // Set the chunk for the iterator
    iterator.chunk = *chunk;
    
    // Start iterating from the first block in the chunk
    iterator.currentBlockId = chunk->from_BlockId;
    
    // Set the initial cursor position to 0
    iterator.cursor = 0;

    return iterator;
}

// Get the next record from the record iterator
int CHUNK_GetNextRecord(CHUNK_RecordIterator* iterator, Record* record) {
    // Get the record at the current cursor position
    HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId, iterator->cursor, record);

    // Get the total number of records in the current block
    int records_in_block = HP_GetRecordCounter(iterator->chunk.file_desc, iterator->currentBlockId);

    // If cursor reaches the end of the block, move to the next block
    if (records_in_block == iterator->cursor + 1) {
        iterator->cursor = 0;
        HP_Unpin(iterator->chunk.file_desc, iterator->currentBlockId);
        iterator->currentBlockId++;
    } else {
        // Move to the next record within the same block
        iterator->cursor++;
    }

    return 1;
}
