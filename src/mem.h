
#ifndef MEM_H
#define MEM_H

#include <stdlib.h>

typedef enum {
   FALSE = 0,
   TRUE = 1
} BOOL;

#define mem_alloc( count, type ) \
   (type*)mem_alloc_internal( count, sizeof( type ) )
#define mem_realloc( ptr, count, type ) \
   (type*)mem_realloc_internal( ptr, count, sizeof( type ) )
#define mem_free( ptr ) \
   ((ptr) = mem_free_internal( ptr ))

void* mem_alloc_internal( size_t count, size_t sz );
void* mem_realloc_internal( void* ptr, size_t count, size_t sz );
int mem_buffer_grow(
   char** buffer, size_t* len, size_t new_len
)
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
void* mem_free_internal( void* ptr );

#endif /* MEM_H */
