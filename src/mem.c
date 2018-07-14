
#define MEM_C
#include "mem.h"

#include <stdio.h>

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
   void* ptr_out = NULL;

   if( 0 != mem_check_overflow( count, sz ) ) {
      fprintf( stderr, "Memory error: Overflow detected in allocation.\n" );
      goto cleanup;
   }

   ptr_out = calloc( count, sz );
   if( NULL == ptr_out ) {
      fprintf( stderr, "Memory error: Allocation failed.\n" );
      goto cleanup;
   }

cleanup:
   return ptr_out;
}

void* mem_realloc_internal( void* ptr, size_t count, size_t sz ) {
   void* ptr_out = NULL;

   if( 0 != mem_check_overflow( count, sz ) ) {
      fprintf( stderr, "Memory error: Overflow detected in reallocation.\n" );
      goto cleanup;
   }

   ptr_out = realloc( ptr, count * sz );
   if( NULL == ptr_out ) {
      fprintf( stderr, "Memory error: Reallocation failed.\n" );
      goto cleanup;
   }

cleanup:
   return ptr_out;
}

void* mem_free_internal( void* ptr ) {
   free( ptr );
   return NULL;
}
