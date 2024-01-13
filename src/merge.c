#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "merge.h"
#include "chunk.h"

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

int get_min_record_position(Record* records, int size) {
    if (size <= 0) {
        // Return -1 to indicate an empty array or handle the error appropriately
        return -1;
    }

    // Initialize minPosition with the position of the first record in the array
    int minPosition = 0;

    // Iterate through the array to find the position of the minimum record using compare_records
    for (int i = 1; i < size; ++i) {
        if (compare_records(&records[minPosition], &records[i])) {
            minPosition = i;
        }
    }

    return minPosition;
}

int should_stop_insertion(int* array1, int* array2, int size) {
    for (int i = 0; i < size; i++)
        if (array1[i] != array2[i])
            return 0;
    
    return 1;
}

// void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
//     printf("--------\n");
//     CHUNK_Iterator input_iterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
//     CHUNK_Iterator output_iterator = CHUNK_CreateIterator(output_FileDesc, chunkSize * bWay);

//     int* min_chunks_records_pos = (int*)malloc(sizeof(int) * bWay);
//     int* max_records_in_chunks = (int*)malloc(sizeof(int) * bWay);
//     Record* min_chunks_records = (Record*)malloc(sizeof(Record) * bWay);
//     CHUNK* chunks = (CHUNK*)malloc(sizeof(CHUNK) * bWay);

//     int output_records = 1;

//     // Creating the arrays with the min records of each chunk and their position
//     for (int i = 0; i < bWay; i++) {
//         CHUNK current_chunk;
//         CHUNK_GetNext(&input_iterator, &current_chunk);

//         CHUNK_GetIthRecordInChunk(&current_chunk, 1, &min_chunks_records[i]);
//         min_chunks_records_pos[i] = 1;
//         max_records_in_chunks[i] = current_chunk.recordsInChunk;
//         chunks[i] = current_chunk;
//     }
    
//     while (!should_stop_insertion(min_chunks_records_pos, max_records_in_chunks, bWay)){
//         // Getting the minimum record in the array
//         int min_record_pos = get_min_record_position(min_chunks_records, bWay);
//         Record min_record = min_chunks_records[min_record_pos];
        
//         // Writing the minimum record to the output chunk
//         HP_InsertEntry(output_FileDesc, min_record);

//         if (min_chunks_records_pos[min_record_pos] < max_records_in_chunks[min_record_pos])
//             min_chunks_records_pos[min_record_pos]++;

//         CHUNK_GetIthRecordInChunk(&chunks[min_record_pos], min_chunks_records_pos[min_record_pos], &min_chunks_records[min_record_pos]);

//         printf("----\n");
//         for (int i = 0; i < bWay; i++) {
//             printf(min_chunks_records[i])
//         }
//     }

//     HP_PrintAllEntries(output_FileDesc);
// }

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
     // Create iterators for input and output chunks
    CHUNK_Iterator inputIterator = CHUNK_CreateIterator(input_FileDesc, chunkSize * bWay);
    CHUNK_Iterator outputIterator = CHUNK_CreateIterator(output_FileDesc, chunkSize);
    while (1) {
        // Allocate memory for b chunks
        CHUNK* chunks = (CHUNK*)malloc(sizeof(CHUNK) * bWay);
        CHUNK* output_chunks = (CHUNK*)malloc(sizeof(CHUNK) * (bWay/2));

        int output_chunk_counter = 0;
        int output_record_counter = 0;

        // Check for memory allocation failure
        if (chunks == NULL) {
            // Handle the error, for example, by returning from the function or taking appropriate action.
            return;
        }
        int* records_counter=(int*)malloc(sizeof(int)*bWay);
        Record* min_records=(Record*)malloc(sizeof(Record)*bWay);

        // Initialize the structures
        while(1){
            for (int i = 0; i < bWay; i++) {
                CHUNK_GetNext(&inputIterator, &chunks[i]);
                records_counter[i] = chunks[i].recordsInChunk;
                CHUNK_GetIthRecordInChunk(&chunks[i], chunks[i].recordsInChunk-records_counter[i] + 1, &min_records[i]);
            }

            for (int i = 0; i < (bWay/2); i++) {
                CHUNK_GetNext(&outputIterator, &output_chunks[i]);
            }

            Record min_record = min_records[0];
            int min_pos = 0;
            for (int i = 1; i < bWay; i++) {
                if (compare_records(&min_record, &min_records[i])) {
                    min_record = min_records[i];
                    min_pos = i;
                }
            }

            HP_InsertEntry(output_FileDesc, min_record);
            output_record_counter++;
            records_counter[min_pos]--;

            if (output_record_counter == (bWay/2)) {
                output_record_counter = 0;
                output_chunk_counter++;
            }
            int should_stop=1;
            for(int i=0;i<bWay;i++) {
                if(records_counter[i]!=0){
                    should_stop=0;
                    break;
                }
            }
            if(should_stop){
                break;
            }
        }

        free(min_records);
        free(records_counter);
        free(output_chunks);
        free(chunks);

        if (HP_GetIdOfLastBlock(input_FileDesc) == inputIterator.lastBlocksID)
            break;
    }
}
