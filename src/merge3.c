#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "merge.h"
#include "chunk.h"

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    // Create iterators for input and output chunks
    CHUNK_Iterator inputIterator = CHUNK_CreateIterator(input_FileDesc, chunkSize * bWay);
    CHUNK_Iterator outputIterator = CHUNK_CreateIterator(output_FileDesc, chunkSize);

    // Allocate memory for b chunks
    CHUNK* chunks = (CHUNK*)malloc(sizeof(CHUNK) * bWay);

    // Check for memory allocation failure
    if (chunks == NULL) {
        // Handle the error, for example, by returning from the function or taking appropriate action.
        return;
    }

    // Merge b chunks at a time
    int should_stop = 0;
    while (!should_stop) {
        // Load the first block of each of the b chunks
        BF_Block* currentBlocks[bWay];
        int recordsInBlocks[bWay];

        // Getting the first block of each chunk
        for (int i = 0; i < bWay; i++) {
            CHUNK_GetNext(&inputIterator, &chunks[i])

            BF_Block_Init(&currentBlocks[i]);
            printf("bf 1\n");
            CALL_BF(BF_GetBlock(chunks[i].file_desc, chunks[i].from_BlockId, currentBlocks[i]));
            recordsInBlocks[i] = HP_GetRecordCounter(chunks[i].file_desc, chunks[i].from_BlockId);
            BF_Block_SetDirty(currentBlocks[i]);
            CALL_BF(BF_UnpinBlock(currentBlocks[i]));
        }

        // Perform the merging of b chunks
        int records_counter[bWay];
        for (int i = 0; i < bWay; i++)
            records_counter[i] = 0;

        while (1) {
            int minIndex = -1;
            Record minRecord;

            // Find the chunk with the minimum record
            for (int i = 0; i < bWay; i++) {
                if (recordsInBlocks[i] > 0) {
                    Record currentRecord;
                    HP_GetRecord(chunks[i].file_desc, chunks[i].from_BlockId, records_counter[i], &currentRecord);

                    if (minIndex == -1 || shouldSwap(&currentRecord, &minRecord)) {
                        minIndex = i;
                        minRecord = currentRecord;
                    }
                }
            }

            if (minIndex == -1) {
                // No more records in any block of the b chunks
                break;
            }

            // Write the minimum record to the output chunk
            CHUNK_UpdateIthRecord(&chunks[0], chunks[0].recordsInChunk, minRecord);

            // Move to the next record in the selected chunk
            recordsInBlocks[minIndex]--;
            records_counter[minIndex]++;

            // If the current block is exhausted, move to the next block
            if (recordsInBlocks[minIndex] == 0) {
                BF_Block_Destroy(&currentBlocks[minIndex]);
                chunks[minIndex].from_BlockId++;
                BF_Block_Init(&currentBlocks[minIndex]);
                printf("bf 2\n");
                CALL_BF(BF_GetBlock(chunks[minIndex].file_desc, chunks[minIndex].from_BlockId, currentBlocks[minIndex]));
                recordsInBlocks[minIndex] = HP_GetRecordCounter(chunks[minIndex].file_desc, chunks[minIndex].from_BlockId);
            }
        }

        // Update the output chunk with the merged records
        CHUNK_GetNext(&outputIterator, &chunks[0]);

        // Free the blocks used for the input chunks
        for (int i = 0; i < bWay; i++) {
            BF_Block_Destroy(&currentBlocks[i]);
        }

        if (chunks[i] == -1)
            should_stop = 1;
    }

    // Free the allocated memory
    free(chunks);
}
