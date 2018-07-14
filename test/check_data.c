
#include "check_data.h"

struct tagbstring str_key_hit = bsStatic( "This key should be a hit." );
struct tagbstring str_key_miss = bsStatic( "This key should be a miss." );
struct tagbstring str_key_also1 = bsStatic( "This key isn't relevant." );
struct tagbstring str_key_also2 = bsStatic( "This key is not relevant." );
struct tagbstring str_key_also3 = bsStatic( "This key ain't relevant." );

#include <stdlib.h>

void free_blob( BLOB* blob ) {
    mem_free( blob->data );
    mem_free( blob );
}

BLOB* create_blob( uint32_t sent_s, uint16_t ptrn, size_t c, uint32_t sent_e ) {
   size_t i;
   BLOB* blob;

   blob = (BLOB*)mem_alloc( 1, BLOB );

   blob->sentinal_start = sent_s;
   blob->data_len = c;
   blob->data = (uint16_t*)calloc( c, sizeof( uint16_t ) );
   for( i = 0 ; c > i ; i++ ) {
      blob->data[i] = ptrn;
   }
   blob->sentinal_end = sent_e;

   return blob;
}
