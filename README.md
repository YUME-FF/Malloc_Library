# Malloc Library

This is Zhe Fan's ECE650 Project Repository.

# Table of Contents

- [Malloc Library](#Malloc-Library)
  - [Implementation Description](#Implementation-Description)
    - [Malloc](#Malloc)
    - [Free](#Free)
  - [Performance Result Presentation and Analysis](#Performance-Result-Presentation-and-Analysis)

## Malloc Library

For this assignment, I will implement my own version of several memory allocation functions from the C standard library. This implementation is to be done in C code. The C standard library includes 4 malloc reallrelated library functions: malloc(), free(), calloc(), and oc(). In this assignment, I implement versions of malloc() and free(): 
```
void *malloc(size_t size); 
void free(void *ptr); 
```
Essentially, malloc() takes in a size (number of bytes) for a memory allocation, locates an address in the program’s data region where there is enough space to fit the specified number of bytes, and returns this address for use by the calling program. 
The free() function takes an address (that was returned by a previous malloc operation) and marks that data region as available again for use.

The task is to implement 2 versions of malloc and free, each based on a different strategy for
determining the memory region to allocate. The two strategies are:
1. **First Fit**: Examine the free space tracker (e.g. free list), and allocate an address from
the first free region with enough space to fit the requested allocation size.
2. **Best Fit**: Examine all of the free space information, and allocate an address from the
free region which has the smallest number of bytes greater than or equal to the
requested allocation size.

Details of requirement can be found in [Project1.pdf](Project_1_Malloc_Library/Project1.pdf)

### Implementation Description

#### Malloc

  For each space chunk, I define a struct using **Double LinkedList** to know the **size** of the chunk, whether or not it's **free**, and what the **next** and **previous** chunk.
  
```
struct chunk_meta {
  size_t size;               //the size of this chunk
  int free;                  //it is free or not
  struct chunk_meta * next;
  struct chunk_meta * prev;
};
typedef struct chunk_meta chunk
```
  
  I also define two variable to record the head and tail of free list:
  
```
chunk * free_region_Start = NULL;
chunk * free_region_End = NULL;
```
  
  For malloc, space should only be allocated when existing space is not enough. Thus given that we have this linked list structure, checking if we have a free chunk and return it. When we get a request of some size, we iterate through our linked list to see if there's a free chunk that's large enough.

 In details, when the chunk is large enough, I also need to reuse free space efficiently if possible. To implementaion that, given that I have LinkedList, we can using splitting and merging.
 
If we don't find a free block, we'll have to request space using sbrk to allocate space.

#### Free

For free(), first I upadate the size of the chunk and make the **free** = 1, then check the previous and next chunk if free or not, if free, then merge them.

### Performance Result Presentation and Analysis

<img src="https://user-images.githubusercontent.com/73271231/214691154-acdcb645-d944-4cda-a1bd-86cf6ab95125.png" width="800"/>

From the Table above, we can see that for small range size, the execution time of Best Fit is less than that of First Fit. This is due to that in the figure above, if we first want to use 2 free chunks, finally the 3rd request will return that there is not enough space, which leading to call the sbrk() to request space from OS. However, calling sbrk() is more time-consuming than directly using free chunks in small range of size. While Best Fit will find the free chunk with minimum suitable space so that it will use all free chunks efficiently.

For equal range size, the two methods show that they have close execution time and fragmentation. This is because for equal range size, two methods run in the same way. Best Fit behaves in the same way as First Fit does namely.

For large range size, though Best Fit is faster than First Fit in small range size, iterating to find the free chunk with minimum suitable space in a large range will cost more time than sbrk(). Thus First Fit is faster than Best Fit in large range size.