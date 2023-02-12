#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct chunk_meta {
  size_t size;               //the size of this chunk
  int free;                  //it is free or not
  struct chunk_meta * next;  //double linkedlist to next chunk
  struct chunk_meta * prev;  //double linkedlist to previous chunk
};
typedef struct chunk_meta chunk;

void printChunk(chunk * chk);
void printFreeStatus();

void * allocate_space(size_t size);
void * reuse_chunk(chunk * ptr, size_t size);
void * ff_find_free_chunk(size_t size);
void * bf_find_free_chunk(size_t size);
void extend_chunk(chunk * ptr);
void remove_chunk(chunk * ptr);
chunk * split_chunk(size_t size, chunk * chk);
void mergeRight(chunk * chk);
void mergeLeft(chunk * chk);

//First Fit malloc/free
void * ff_malloc(size_t size);
void ff_free(void * ptr);

//Best Fit malloc/free
void * bf_malloc(size_t size);
void bf_free(void * ptr);

unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
