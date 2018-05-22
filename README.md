# memory-allocator
Simulates C dynamic memory allocation.
The memory arena is then divided into smaller chunks. This smaller chunk will have two pieces:
  * first one is the management zone. It will have 3 ints:
    * next memory chunk starting index
    * previous memory chuck starting index
    * the size of the actual data zone
  * the second one is the data zone
