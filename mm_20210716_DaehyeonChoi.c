/*
    20210716 DaeHyeon Choi
    daehyeonchoi@postech.ac.kr
    implement by using Segragation list & best fit 
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* Basic constants and macros */
#define WSIZE 4             /* word size (bytes) */
#define DSIZE 8             /* doubleword size (bytes) */
#define CHUNKSIZE (1 << 12) /* extend heap size (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp) - DSIZE)))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


//segragation list
static unsigned int h[10]; 
static void *head[10]; 
static int64_t t[10];
static void *tail[10];
static char *heap_listp; 

//macros for free block 
#define PREV(bp) ((char *)(bp) + WSIZE) 
#define SUCC(bp) ((char *)(bp))          
#define GETP(p) (*(char **)p)
#define GET_PREV(bp) (GETP(PREV(bp)))
#define GET_SUCC(bp) (GETP(SUCC(bp)))


// functions for we need to implment & additional function 
int mm_init(void);
void *mm_malloc(size_t size);
void *mm_realloc(void *ptr, size_t size);
void mm_free(void *ptr);

static void *coalesce(void *bp);
static void *extend_heap(size_t words);
void *best_fit(size_t size);

//additional function 
static void make_seg_list();
void insert(void* pos, void *node);
void delete(void *pos);
static void place(void *bp, size_t asize);
int segragation(size_t size);
void insertseglist(void *bp);
void place_alloc(void *ptr, size_t size, size_t new_size);


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{

    make_seg_list(); //make segragate list 

    /*
    int i;

    for (i = 0; i < 10; i++) { //make segragation list for free block

        head[i] = &h[i];
        tail[i] = &t[i];

        GET_SUCC(head[i]) = tail[i];  //doubly linked list 
        GET_PREV(tail[i]) = head[i]; 

    }

    */

    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) return -1;
    PUT(heap_listp, 0);  //padding alignment
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1)); //prologue header
    PUT(heap_listp + 2 * WSIZE, PACK(DSIZE, 1));  // prologue footer 
    PUT(heap_listp + 3 * WSIZE, PACK(0, 1)); // epilogue header 

    heap_listp += DSIZE; // for prologue & epilogue header

    if (extend_heap(DSIZE) == NULL) return -1; // Extend the empty heap with a free block of CHUNKSIZE bytes

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    //exception: if size <= 0 
    if(size < 0) return NULL;
    else if(size == 0) return NULL;

    if(size <= DSIZE) asize = 2 * DSIZE; //DSIZE + overhead(for header + footer) 
    else asize = DSIZE * ((size + DSIZE + DSIZE -1) / DSIZE); // allocate to multiples of DSIZE


    if ((bp = best_fit(asize)) == NULL) { // call best_fit function
        extendsize = MAX(asize, CHUNKSIZE);
        if ((bp = extend_heap(extendsize / DSIZE)) == NULL) return NULL;
    }
    
    place(bp, asize); // If we can't find fit memeory, we need to get more memory and place the block 
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */

void mm_free(void *ptr)
{

  if (ptr == NULL) return; // exception 1: If ptr == NULL

  size_t size = GET_SIZE(HDRP(ptr));

  PUT(HDRP(ptr), PACK(size, 0)); // deallocate mark
  PUT(FTRP(ptr), PACK(size, 0));
  coalesce(ptr); 

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

    if(ptr == NULL) return mm_malloc(size); // exception 1: not allocated -> call 'mm_malloc'

    if(size == 0){ //exception 2: if size == 0 -> call 'mm_free'

      mm_free(ptr);
      return NULL;

    }

    if (size <= DSIZE) size = DSIZE * 2; //overhead + DSIZE
    else size = DSIZE * ((size + DSIZE + DSIZE - 1) / DSIZE); // of DSIZE

    size_t old_size = GET_SIZE(HDRP(ptr)); //save current size 
  
    if (old_size == size) return ptr; //exception 1: old_size == size 
    else if (old_size > size) { //if old_size > size, so we need to reduce it 

        if (old_size >= size + (DSIZE + DSIZE)) place_alloc(ptr, old_size, size);
        return ptr;

    }

    else { // if old_size < size, so we need to expand it 

        void *next = NEXT_BLKP(ptr); //next block pointer
        size_t next_alloc = GET_ALLOC(HDRP(next));
        size_t old_next = old_size;


        if (!next_alloc) { //if next block isn't allocated 

            old_next += GET_SIZE(HDRP(next));
            delete(next); 
            PUT(HDRP(ptr), PACK(old_next, 1));
            PUT(FTRP(ptr), PACK(old_next, 1));

            if (old_next - size >= (2* DSIZE)) place_alloc(ptr, old_next, size);
            if (old_next >= size) return ptr;

        }

        else {

            void* new_ptr = mm_malloc(size); //if next block is allcated 
            memcpy(new_ptr, ptr, old_size); // memory copying 
            mm_free(ptr); //free old_pointer 
            return new_ptr; //return new pointer

        }

    }

    //return ptr;

}

static void make_seg_list(){

    int i;

    for (i = 0; i < 10; i++) { //make segragation list for free block

        head[i] = &h[i];
        tail[i] = &t[i];

        GET_SUCC(head[i]) = tail[i];  //doubly linked list 
        GET_PREV(tail[i]) = head[i]; 

    }
}

static void *coalesce(void *bp){ //coalesce function 

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    if (prev_alloc && next_alloc) { // case 1: if previous block & next_block are both allocated 
    
        insertseglist(bp);
        return bp;

    }

    else if (prev_alloc && !next_alloc){  // case 2: else if previous block is allocated and next block is free 

        delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        //delete(NEXT_BLKP(bp)); 
        PUT(HDRP(bp), PACK(size, 0));          
        PUT(FTRP(bp), PACK(size, 0));
        insertseglist(bp);

    }
   
    else if (!prev_alloc && next_alloc){  // case 3: else if previous block is free and next block is allocated 

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

    }


    else { // case 4: if previous block and next block are both free

        delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

    }
    
    return bp;

}

static void *extend_heap(size_t words){

    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * DSIZE : words * DSIZE; // allocate "even number" of words to maintain alignment policy

    if(size < 16) size = 16; //exception: if size < 16 

    if ((int)(bp = mem_sbrk(size)) == -1) return NULL;

    // Initialize free block header, footer, epilogue header 
    PUT(HDRP(bp), PACK(size, 0));         // free block header 
    PUT(FTRP(bp), PACK(size, 0));         // free block footer 
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header 
    
    return coalesce(bp); // if the previous block was free, coalesce 
 
}


void *best_fit(size_t size){ //find position by using "best fit" method

    size_t temp_size = 2 << 30;
    void *here = NULL;
    size_t pos = segragation(size); //find position in least & available segragation list

    void *bp;
    void *temp; 
    
    while (pos != 10 && here == NULL) { //while for segragation list 

        for (bp = GET_SUCC(head[pos]); bp != tail[pos]; bp = GET_SUCC(bp)) { // find fit location in fit segragation list

            temp = HDRP(bp);
            if (temp_size > GET_SIZE(temp) - size) {

                temp_size = GET_SIZE(temp) - size;
                here = bp;
                if (GET_SIZE(temp) == size) return here;
            }
        }

        pos++;
    }

    return here; //return fit location 
}

void insert(void* pos, void *node){  // insert node before "pos" (of segragation list)
                                                                
    GET_SUCC(GET_PREV(pos)) = (char *)node; // pos.PREV -> node
    GET_PREV(node) = GET_PREV(pos);         // pos.PREV <-node   
    GET_PREV(pos) = (char *)node;           // node <- pos     
    GET_SUCC(node) = (char *)pos;           // node -> pos     

} 


void delete(void* pos){  // delete "pos" from segragation list    
                                                            
    GET_SUCC(GET_PREV(pos)) = GET_SUCC(pos);  // PREV -> NEXT 
    GET_PREV(GET_SUCC(pos)) = GET_PREV(pos);  // PREV <- NEXT   

} 


static void place(void *bp, size_t asize){ // function that puts block in the found location and padding the header and footer

    size_t csize = GET_SIZE(HDRP(bp));
    delete(bp);  

    if ((csize - asize) >= (2 * DSIZE)) {

        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        insertseglist(bp);  

    } 
    
    else {

        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));

    }
}


int segragation(size_t size){ //segragation by size of block

    int max_idx = 9;
    int i;

    for(i = 0; i < max_idx + 1; i++) if((size >> (3 + i)) == 0) return i; // if size <= 2 ^ (3+i) Byte, return i 

    return max_idx; // if size > 2^12 = 4096 Byte, return max_idx
}

void insertseglist(void *bp) {         

    void *p = GET_SUCC(head[segragation((GET_SIZE(HDRP(bp))))]); //find location p 
    insert(p, bp); //insert "bp" before p

}

void place_alloc(void *ptr, size_t size, size_t new_size){  // function that puts block in the found location (allocated) and padding the header and footer

    //padding for new size block 

    PUT(HDRP(ptr), PACK(new_size, 1));
    PUT(FTRP(ptr), PACK(new_size, 1));

    //padding for splited block 

    void *temp = NEXT_BLKP(ptr);

    PUT(HDRP(temp), PACK(size - new_size, 0));
    PUT(FTRP(temp), PACK(size - new_size, 0));

    coalesce(temp);

}

