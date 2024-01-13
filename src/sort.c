#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"

bool shouldSwap(Record *rec1, Record *rec2) {
    // Compare based on name first
    int nameComparison = strcmp(rec1->name, rec2->name);

    // If names are the same, compare based on surname
    if (nameComparison == 0) {
        int surnameComparison = strcmp(rec1->surname, rec2->surname);
        return (surnameComparison > 0); // Swap if rec1's surname comes after rec2's surname
    }

    return (nameComparison > 0); // Swap if rec1's name comes after rec2's name
}

void sort_FileInChunks(int file_desc, int numBlocksInChunk){
    CHUNK_Iterator chunk_iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    int lastBlockId=HP_GetIdOfLastBlock(file_desc);

    CHUNK chunk;

    for (int i = 0; i < (int)(lastBlockId / numBlocksInChunk); i++) {
        CHUNK_GetNext(&chunk_iterator, &chunk);
        sort_Chunk(&chunk);
        
        // printf("--New Chunk--\n");
        // CHUNK_Print(chunk);
    }
}

void sort_Chunk(CHUNK* chunk) {
    // Allocate memory for an array of records
    Record *records = (Record *)malloc(sizeof(Record) * chunk->recordsInChunk);

    // Check for memory allocation failure
    if (records == NULL) {
        // Handle the error, for example, by returning from the function or taking appropriate action.
        return;
    }

    // Initialize the iterator for the chunk
    CHUNK_RecordIterator iterator = CHUNK_CreateRecordIterator(chunk);


    // Populate the records array with records from the chunk
    for (int i = 0; i < chunk->recordsInChunk; i++) {
        CHUNK_GetNextRecord(&iterator, &records[i]);
    }
    
    // Use the shouldSwap function to perform the sorting
    for (int i = 0; i < chunk->recordsInChunk - 1; i++) {
        for (int j = 0; j < chunk->recordsInChunk - i - 1; j++) {
            if (shouldSwap(&records[j], &records[j + 1])) {
                // Swap records if shouldSwap returns true
                Record temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
            }
        }
    }

    // printf("--- New ---\n");
    // for (int i = 0; i < chunk->recordsInChunk; i++) {
    //     printRecord(records[i]);
    // }

    printf("Sorting chunk with from: %d, to: %d\n", chunk->from_BlockId, chunk->to_BlockId);

    // Update the chunk with the sorted records
    iterator = CHUNK_CreateRecordIterator(chunk);
    for (int i = 0; i < chunk->recordsInChunk; i++) {
        CHUNK_UpdateIthRecord(chunk, i, records[i]);
    }

    // Free the allocated memory
    free(records);
}
