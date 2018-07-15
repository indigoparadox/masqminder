/*
 * Originally by Elliot C Back - http://elliottback.com/wp/hashmap-implementation-in-c/
 *
 * Modified by Pete Warden to fix a serious performance problem, support strings as keys
 * and removed thread synchronization - http://petewarden.typepad.com
 */

/**
 * @file
 * @brief Generic hashmap manipulation functions
 */

#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "bstrlib/bstrlib.h"
#include "vector.h"

struct HASHMAP;

typedef void* (*hashmap_cb)( const bstring idx, void* iter, void* arg );

enum HASHMAP_ERROR {
   HASHMAP_ERROR_NONE,
   HASHMAP_ERROR_FULL = -1,
   HASHMAP_ERROR_ITEM_EXISTS = -2
};

/* We need to keep keys and values */
struct HASHMAP_ELEMENT {
   uint16_t sentinal;
   bstring key;
   BOOL in_use;
   void* data;
#ifdef USE_ITERATOR_CACHE
   size_t iterator_index;
#endif /* USE_ITERATOR_CACHE */
};

/* A hashmap has some maximum size and current size,
 * as well as the data to hold. */
struct HASHMAP {
   uint16_t sentinal;
   int table_size;
   int size; /* Hashmap sizes can also be - error codes. */
   struct HASHMAP_ELEMENT* data;
   uint8_t lock_count;
   enum HASHMAP_ERROR last_error;
   BOOL rehashing;
#ifdef USE_ITERATOR_CACHE
   struct VECTOR iterators; /*!< List of hashes stored sequentially.  */
#endif /* USE_ITERATOR_CACHE */
};

#define hashmap_new( m ) \
   m = mem_alloc( 1, struct HASHMAP ); \
   if( NULL == m ) { \
      /* TODO: Error. */ \
      goto cleanup; \
   } \
   hashmap_init( m );

void hashmap_init( struct HASHMAP* m );
void* hashmap_iterate( struct HASHMAP* m, hashmap_cb callback, void* arg );
struct VECTOR* hashmap_iterate_v( struct HASHMAP* m, hashmap_cb callback, void* arg );
size_t hashmap_remove_cb( struct HASHMAP* m, hashmap_cb callback, void* arg );
enum HASHMAP_ERROR hashmap_put(
   struct HASHMAP* m, const bstring key, void* value, BOOL overwrite )
#ifdef USE_GNUC_EXTENSIONS
__attribute__ ((warn_unused_result))
#endif /* USE_GNUC_EXTENSIONS */
;
void* hashmap_get( struct HASHMAP* m, const bstring key );
BOOL hashmap_contains_key( struct HASHMAP* m, const bstring key );
BOOL hashmap_remove( struct HASHMAP* m, const bstring key );
size_t hashmap_remove_all( struct HASHMAP* m );
void hashmap_cleanup( struct HASHMAP* m );
void hashmap_free( struct HASHMAP** m );
size_t hashmap_count( const struct HASHMAP* m);
void hashmap_lock( struct HASHMAP* m, BOOL lock );
BOOL hashmap_is_valid( const struct HASHMAP* m);

#endif /* __HASHMAP_H__ */
