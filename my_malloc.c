#include "my_malloc.h"

#include <assert.h>

#define META_SIZE sizeof(chunk)

chunk * free_region_Start = NULL;  //Start
chunk * free_region_End = NULL;    //End
size_t data_size = 0;
size_t free_size = 0;

void printChunk(chunk * chk) {
  printf("curr: %p\n", chk);
  printf("size: %lu\n", chk->size);
  printf("is free: %d\n", chk->free);
  printf("next: %p\n", chk->next);
  printf("prev: %p\n", chk->prev);
}

void printFreeStatus() {
  printf("Here is Free Status:\n");
  printf("chunk size: %lu\n", META_SIZE);
  printf("From: %p\n", free_region_Start);
  chunk * curr = free_region_Start;
  while (curr != NULL) {
    printChunk(curr);
    curr = curr->next;
  }
  printf("To: %p\n", free_region_End);
  printf("\n");
}

/*
Function: Request for space with size of 'size'.

When there is no free block, 
space should be allocated from the OS (request for space) 
using sbrk and add new block to the end of the struct chunk.
*/
void * allocate_space(size_t size) {
  data_size += size + META_SIZE;
  chunk * new;
  new = sbrk(0);
  void * request = sbrk(size + META_SIZE);
  assert((void *)new == request);
  if (request == (void *)-1) {
    return NULL;  // sbrk failed.
  }
  new->size = size;
  new->free = 0;
  new->next = NULL;
  new->prev = NULL;
  return (char *)new + META_SIZE;
}

/*
Function: extend free memory region in chunk
*/
void extend_chunk(chunk * ptr) {
  if (!free_region_Start || (ptr < free_region_Start)) {
    ptr->prev = NULL;
    ptr->next = free_region_Start;
    if (ptr->next) {
      ptr->next->prev = ptr;
    }
    else {
      free_region_End = ptr;
    }
    free_region_Start = ptr;
  }
  else {
    chunk * curr = free_region_Start;
    //find until ptr < curr->next
    while (curr->next && (ptr > curr->next)) {
      curr = curr->next;
    }

    //curr -> ptr -> curr->next
    ptr->prev = curr;
    ptr->next = curr->next;
    curr->next = ptr;

    if (ptr->next) {  //ptr is the last
      ptr->next->prev = ptr;
    }
    else {
      free_region_End = ptr;
    }
  }
}

/*
Function: remove targeted chunk
*/

void remove_chunk(chunk * ptr) {
  if (free_region_End == ptr) {
    if (free_region_Start == ptr) {
      free_region_Start = NULL;
      free_region_End = NULL;
    }
    else {
      free_region_End = ptr->prev;
      free_region_End->next = NULL;
    }
  }
  else if (free_region_Start == ptr) {
    free_region_Start = ptr->next;
    free_region_Start->prev = NULL;
  }
  else {
    ptr->prev->next = ptr->next;
    ptr->next->prev = ptr->prev;
  }
}

/*
Function: split the chunk.
One of First fit's short is that it may make a small size to
be set in a big chunk, thus when the remain chunk is large
enough, the chunk should be splitted.
*/
chunk * split_chunk(size_t size, chunk * chk) {
  chunk * splitChunk;
  splitChunk = (chunk *)((char *)chk + META_SIZE + size);
  splitChunk->size = chk->size - size - META_SIZE;
  splitChunk->free = 1;
  splitChunk->next = NULL;
  splitChunk->prev = NULL;
  return splitChunk;
}

/*
Function: split the chunk and return used chunk
*/
void * reuse_chunk(chunk * ptr, size_t size) {
  if (ptr->size >= size + META_SIZE) {  //find space that large enough
    chunk * split = split_chunk(size, ptr);

    ptr->size = size;

    extend_chunk(split);
    remove_chunk(ptr);
    free_size -= size + META_SIZE;
  }
  else {
    remove_chunk(ptr);
    free_size -= ptr->size + META_SIZE;
  }
  ptr->free = 0;
  ptr->next = NULL;
  ptr->prev = NULL;
  return (char *)ptr + META_SIZE;
}

/*
Function:checking Find a free chunk and return it straightforward. 
Iterate through linked list to see if there's a large enough free chunk.
If large enough, split it
*/
void * ff_find_free_chunk(size_t size) {
  chunk * ptr = free_region_Start;
  while (ptr) {
    if (ptr->size >= size) {
      return reuse_chunk(ptr, size);
    }
    ptr = ptr->next;
  }
  return NULL;
}

/*
Function:checking Find a free chunk and return it straightforward.
Iterate through linked list to see if there's a best fit free chunk.
*/
void * bf_find_free_chunk(size_t size) {
  chunk * ptr = free_region_Start;
  chunk * bf_ptr = NULL;  //record minimal best fit chunk
  while (ptr) {
    if (ptr->size >= size) {
      if (!bf_ptr || ptr->size < bf_ptr->size) {
        bf_ptr = ptr;
      }
      if (ptr->size == size) {
        return reuse_chunk(bf_ptr, size);
      }
    }
    ptr = ptr->next;
  }
  if (!bf_ptr) {
    return NULL;
  }
  else {
    return reuse_chunk(bf_ptr, size);
  }
}
/*
Function: Merge chunk
*/
void mergeRight(chunk * chk) {
  if (chk->next && ((char *)chk + chk->size + META_SIZE == (char *)chk->next)) {
    chk->size += META_SIZE + chk->next->size;
    remove_chunk(chk->next);
  }
}

void mergeLeft(chunk * chk) {
  if (chk->prev && ((char *)chk->prev + chk->prev->size + META_SIZE == (char *)chk)) {
    chk->prev->size += META_SIZE + chk->size;
    remove_chunk(chk);
  }
}

//First Fit malloc/free
void * ff_malloc(size_t size) {
  chunk * _chunk;
  if (size <= 0) {
    return NULL;
  }
  if (!free_region_Start) {
    _chunk = allocate_space(size);
    if (!_chunk) {
      return NULL;
    }
  }
  else {
    _chunk = ff_find_free_chunk(size);
    if (!_chunk) {  //there is no space large enough
      _chunk = allocate_space(size);
      if (!_chunk) {
        return NULL;
      }
    }
  }
  return _chunk;
}

void ff_free(void * ptr) {
  if (!ptr) {  //call free with a NULL ptr
    return;
  }
  chunk * pointer;
  pointer = (chunk *)((char *)ptr - META_SIZE);
  pointer->free = 1;
  free_size += pointer->size + META_SIZE;
  extend_chunk(pointer);

  //merge chunk
  mergeRight(pointer);
  mergeLeft(pointer);
}

void * bf_malloc(size_t size) {
  chunk * _chunk;
  if (size <= 0) {
    return NULL;
  }
  if (!free_region_Start) {
    _chunk = allocate_space(size);
    if (!_chunk) {
      return NULL;
    }
  }
  else {
    _chunk = bf_find_free_chunk(size);
    if (!_chunk) {  //there is no space
      _chunk = allocate_space(size);
      if (!_chunk) {
        return NULL;
      }
    }
  }
  return _chunk;
}

void bf_free(void * ptr) {
  return ff_free(ptr);
}

unsigned long get_data_segment_size() {
  return data_size;
}

unsigned long get_data_segment_free_space_size() {
  return free_size;
}
