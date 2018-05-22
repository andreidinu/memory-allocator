# Memory Allocator
Simulates C dynamic memory allocation.
The memory arena is then divided into smaller chunks. This smaller chunk will have two pieces:
  * first one is the management zone. It will have 3 ints:
    * next memory chunk starting index
    * previous memory chuck starting index
    * the size of the actual data zone
  * second one is the data zone where user's info is stored
  
**alloc_arena(<N>)**:
 * Reserves the memory arena.
 
**free_arena(<N>)**: 
 * Frees the memory arena.
 
**dump(<N>)**:
 * Iterate the arena byte by byte and outputs the value. It will show 16 bytes on each line.
 
**fill(<INDEX> <SIZE> <VALUE>)**:
 * Starting from (arena + "index"), will change value of "size" bytes into "value". If "size" exceeds the memory arena
 size, it will change all the bytes till the end of arena and then the function will terminate.
 
**alloc_block(<SIZE>)**:
 * Searches for a free block of size "size". First it will look if it can fits between start index and first block. If it can't
 fits, it will search through the entire arena until it reaches the last block of memory (the one with index 0 for the next block).
 If it did not fit, I try to insert it at the end of the arena. If neither this does work, the function will return 0.

**free_block(<INDEX>)**:
 * Frees the memory block at index "index" by changing the management zone values of the blocks on its left and right.
 
**show_free(<N>)**:
 * Iterates through the memory arena using the management zone indices. It counts the free and busy blocks.
 
**show_usage(<N>)**:
 * Iterates through the memory arena and counts the busy bytes, busy blocks, free bytes and free blocks. Output:
   * busy blocks (used bytes)
   * % efficiency -> used bytes / reserved bytes
   * % fragmeted -> free blocks / busy blocks
   
**show_allocations(<N>)**:
 * Iterate through the memory arena. Outputs the size of all encountered blocks (free or busy).
