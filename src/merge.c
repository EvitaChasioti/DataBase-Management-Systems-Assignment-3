#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "merge.h"
#include "chunk.h"

// Function to compare two records based on name and surname
static bool compare_records(Record* rec1, Record* rec2) {
    // Compare based on name first
    int nameComparison = strcmp(rec1->name, rec2->name);

    // If names are the same, compare based on surname
    if (nameComparison == 0) {
        int surnameComparison = strcmp(rec1->surname, rec2->surname);
        return (surnameComparison > 0); // Swap if rec1's surname comes after rec2's surname
    }

    return (nameComparison > 0); // Swap if rec1's name comes after rec2's name
}

// Find the position of the minimum record in an array of records
int get_min_record_position(Record* records, int* chunk_completed, int size) {
    if (size <= 0) {
        // Return -1 to indicate an empty array or handle the error appropriately
        return -1;
    }

    // Initialize minPosition with the position of the first record in the array
    int minPosition;
    for (int i = 0; i < size; i++)
        if (!chunk_completed[i])
            minPosition = i;

    // Iterate through the array to find the position of the minimum record using compare_records
    for (int i = 0; i < size; ++i) {
        if (!chunk_completed[i] && compare_records(&records[minPosition], &records[i])) {
            minPosition = i;
        }
    }

    return minPosition;
}

// Check if all chunks have been completed
int should_stop_insertion(int* completed, int size) {
    for (int i = 0; i < size; i++) {
        if (!completed[i])
            return 0;
    }
    
    return 1;
}

// Merge chunks from an input file into an output file
void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    printf("----11111----\n");
    //HP_PrintAllEntries(input_FileDesc);

    int blocks_num = chunkSize;
    CHUNK_Iterator input_iterator = CHUNK_CreateIterator(input_FileDesc, blocks_num);
    CHUNK_Iterator output_iterator = CHUNK_CreateIterator(output_FileDesc, blocks_num * bWay);

    int help = 0;
    printf("Chunk Size: %d\nbWay: %d\n", chunkSize, bWay);

    // Calculate the number of iterations needed for the merge
    int iterations = HP_GetIdOfLastBlock(input_FileDesc) / (chunkSize * bWay);
    if (HP_GetIdOfLastBlock(input_FileDesc) % (chunkSize * bWay) != 0)
        iterations++;

    for (int iteration = 0; iteration < iterations; iteration++) {
        printf("Another loop\n");
        printf("%d < %d\n", iteration, iterations);

        // Handle the last iteration with a potentially incomplete set of blocks
        if (iterations != 1 && iteration == iterations - 1) {
            int rest_blocks = HP_GetIdOfLastBlock(input_FileDesc) - input_iterator.current + 1;
            printf("Rest Blocks: %d\n", rest_blocks);

            // Adjust bWay based on the remaining blocks
            if (chunkSize * bWay != rest_blocks) {
                int new_bWay = rest_blocks / chunkSize; 
                if (new_bWay != 0) {
                    bWay = new_bWay;
                    if (rest_blocks > chunkSize)
                        bWay++;
                }
                else
                    bWay--;
            }
        }

        // Allocate arrays and structures needed for the merge
        int* min_chunks_records_pos = (int*)malloc(sizeof(int) * bWay);
        int* max_records_in_chunks = (int*)malloc(sizeof(int) * bWay);
        Record* min_chunks_records = (Record*)malloc(sizeof(Record) * bWay);
        CHUNK* chunks = (CHUNK*)malloc(sizeof(CHUNK) * bWay);
        int* chunk_completed = (int*)malloc(sizeof(int) * bWay);

        int output_records = 1;

        // Create arrays with the min records of each chunk and their position
        printf("BWAY: %d\n", bWay);
        for (int i = 0; i < bWay; i++) {
            CHUNK current_chunk;
            CHUNK_GetNext(&input_iterator, &current_chunk);

            if (current_chunk.from_BlockId == 1 && current_chunk.to_BlockId == HP_GetIdOfLastBlock(input_FileDesc)) {
                return;
            }

            //printf("%d %d\n", help, input_FileDesc);
            CHUNK_GetIthRecordInChunk(&current_chunk, 1, &min_chunks_records[i]);
            min_chunks_records_pos[i] = 1;
            max_records_in_chunks[i] = current_chunk.recordsInChunk;
            chunks[i] = current_chunk;
            chunk_completed[i] = 0;
        }
        printf("--- %d, %d\n", max_records_in_chunks[0], max_records_in_chunks[1]);

        // Perform the merge until all chunks are completed
        while (!should_stop_insertion(chunk_completed, bWay)){
            // Get the position of the minimum record in the array
            int min_record_pos = get_min_record_position(min_chunks_records, chunk_completed, bWay);
            Record min_record = min_chunks_records[min_record_pos];
            
            // Write the minimum record to the output chunk
            //printf("Min Id: %d\n", min_record.id);
            HP_InsertEntry(output_FileDesc, min_record);

            // Move to the next record in the chunk or mark the chunk as completed
            if (min_chunks_records_pos[min_record_pos] < max_records_in_chunks[min_record_pos])
                min_chunks_records_pos[min_record_pos]++;
            else 
                chunk_completed[min_record_pos] = 1;

            // Update the min_chunks_records array with the next record in the chunk
            Record new_record;
            CHUNK_GetIthRecordInChunk(&chunks[min_record_pos], min_chunks_records_pos[min_record_pos], &new_record);
            min_chunks_records[min_record_pos] = new_record;
        }

        // Free allocated memory
        free(min_chunks_records_pos); free(max_records_in_chunks); free(min_chunks_records); free(chunks); free(chunk_completed);
        
        help++;
    }
}
