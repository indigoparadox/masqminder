
#define MEM_C
#include "mem.h"

#ifndef __palmos__
#include <stdlib.h>
#endif /* __palmos__ */

/** \brief Check to make sure the given count/size combination will not result
 *         in an overflow, creating a memory buffer with an unexpected size.
 * \param
 * \param
 * \return TRUE if there is an overflow, otherwise FALSE.
 */
static int mem_check_overflow( size_t count, size_t sz ) {
   size_t sz_check = count * sz;
   if( sz_check < count ) {
      return 1;
   }
   return 0;
}

void* mem_alloc_internal( size_t count, size_t sz ) {
   if( 0 == mem_check_overflow( count, sz ) ) {
      return calloc( count, sz );
   }
   return NULL;
}

void* mem_realloc_internal( void* ptr, size_t count, size_t sz ) {
   if( 0 == mem_check_overflow( count, sz ) ) {
      return realloc( ptr, count * sz );
   }
   return NULL;
}

void* mem_free_internal( void* ptr ) {
   free( ptr );
   return NULL;
}
