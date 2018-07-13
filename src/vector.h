#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <stdlib.h>

#include "mem.h"

typedef enum _VECTOR_SORT_ORDER {
   VECTOR_SORT_A_LIGHTER = -1,
   VECTOR_SORT_A_B_EQUAL = 0,
   VECTOR_SORT_A_HEAVIER = 1
} VECTOR_SORT_ORDER;

struct VECTOR {
   uint16_t sentinal;
   void** data;
   size_t size;
   size_t count;
   BOOL scalar;
   int32_t* scalar_data;
   int lock_count;
};

#define VECTOR_SENTINAL 1212
#define VECTOR_SIZE_MAX 10000

#define VECTOR_ERR_FULL -1

typedef void* (*vector_search_cb)( size_t* idx, void* parent, void* iter, void* arg );
typedef BOOL (*vector_delete_cb)( size_t* idx, void* parent, void* iter, void* arg );
typedef VECTOR_SORT_ORDER (*vector_sorter_cb)( void* a, void* b );

#define vector_new( v ) \
   v = mem_alloc( 1, struct VECTOR ); \
   if( NULL == v ) { \
      goto cleanup; \
   } \
   vector_init( v );

void vector_init( struct VECTOR* v );
void vector_cleanup_force( struct VECTOR* v );
void vector_cleanup( struct VECTOR* v );
void vector_free_force( struct VECTOR** v );
void vector_free( struct VECTOR** v );
int vector_insert( struct VECTOR* v, size_t index, void* data )
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
int vector_add( struct VECTOR* v, void* data )
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
void vector_add_scalar( struct VECTOR* v, int32_t value, BOOL allow_dupe );
void vector_set( struct VECTOR* v, size_t index, void* data, BOOL force );
void vector_set_scalar( struct VECTOR* v, size_t index, int32_t value );
void* vector_get( const struct VECTOR* v, size_t index );
int32_t vector_get_scalar( const struct VECTOR* v, size_t index );
int32_t vector_get_scalar_value( const struct VECTOR* v, int32_t value );
size_t vector_remove_cb( struct VECTOR* v, vector_delete_cb callback, void* arg );
void vector_remove( struct VECTOR* v, size_t index );
size_t vector_remove_all( struct VECTOR* v );
void vector_remove_scalar( struct VECTOR* v, size_t index );
size_t vector_remove_scalar_value( struct VECTOR* v, int32_t value );
size_t vector_count( const struct VECTOR* v );
void vector_lock( struct VECTOR* v, BOOL lock );
void* vector_iterate( struct VECTOR* v, vector_search_cb callback, void* arg );
void* vector_iterate_nolock(
   struct VECTOR* v, vector_search_cb callback, void* parent, void* arg
);
void* vector_iterate_r( struct VECTOR* v, vector_search_cb callback, void* arg );
struct VECTOR* vector_iterate_v(
   struct VECTOR* v, vector_search_cb callback, void* parent, void* arg
);
void vector_sort_cb( struct VECTOR* v, vector_sorter_cb );
BOOL vector_is_valid( const struct VECTOR* v );

#endif /* VECTOR_H */
