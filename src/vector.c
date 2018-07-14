
#ifdef DEBUG
#include <stdio.h>
#include <assert.h>
#endif /* DEBUG */

#include "vector.h"

#include "mem.h"

void vector_init( struct VECTOR* v ) {
   v->data = NULL;
   v->size = 0;
   v->count = 0;
   v->scalar_data = NULL;
   v->sentinal = VECTOR_SENTINAL;
}

void vector_cleanup_force( struct VECTOR* v ) {
   if( NULL == v || !vector_is_valid( v ) ) {
      goto cleanup;
   }
   v->count = 0;
   vector_cleanup( v );
cleanup:
   return;
}

void vector_cleanup( struct VECTOR* v ) {
   if( NULL == v || !vector_is_valid( v ) || 0 < v->count ) {
      goto cleanup;
   }

   if( FALSE != v->scalar ) {
      mem_free( v->scalar_data );
   } else {
      mem_free( v->data );
   }

cleanup:
   return;
}

void vector_free_force( struct VECTOR** v ) {
   if( NULL == *v || !vector_is_valid( *v ) ) {
      goto cleanup;
   }
   (*v)->count = 0;
   vector_free( v );
cleanup:
   return;
}

void vector_free( struct VECTOR** v ) {
   if( NULL != *v ) {
      vector_cleanup( *v );
      mem_free( *v );
      *v = NULL;
   }
}

static void vector_reset( struct VECTOR* v ) {
   mem_free( v->scalar_data );
   mem_free( v->data );
   v->scalar_data = NULL;
   v->scalar = FALSE;
   v->count = 0;
}

static int vector_grow( struct VECTOR* v, size_t new_size ) {
   size_t old_size = v->size,
      i;
   void* new_data = NULL;
   int retval = 0;

   new_data = mem_realloc( v->data, new_size, void* );
   if( NULL == new_data ) {
      retval = 1;
      goto cleanup;
   }
   v->data = new_data;
   v->size = new_size;
   for( i = old_size ; i < v->size ; i++ ) {
      v->data[i] = NULL;
   }
cleanup:
   return retval;
}

static int vector_grow_scalar( struct VECTOR* v, size_t new_size ) {
   size_t old_size = v->size,
      i;
   int32_t* new_data;
   int retval = 0;

   v->size = new_size;
   new_data = mem_realloc( v->scalar_data, v->size, int32_t );
   if( NULL == new_data ) {
      retval = 1;
      goto cleanup;
   }
   v->scalar_data = new_data;

   if( NULL == v->scalar_data ) {
      retval = 1;
      goto cleanup;
   }
   for( i = old_size ; i < v->size ; i++ ) {
      v->scalar_data[i] = 0;
   }
cleanup:
   return retval;
}

/** \brief Insert an item into the vector at the specified position and push
 *         subsequent items later.
 * \param[in] v      Vector to manipulate.
 * \param[in] index  Index to push the item at. Vector will be extended to
 *                   include this index if it does not already.
 * \param[in] data   Pointer to the item to add.
 */
int vector_insert( struct VECTOR* v, size_t index, void* data ) {
   BOOL ok = FALSE;
   size_t i;
   int err = -1;

   if( NULL == v || !vector_is_valid( v ) || FALSE != v->scalar ) {
      goto cleanup;
   }

   vector_lock( v, TRUE );
   ok = TRUE;

   if( VECTOR_SIZE_MAX <= index ) {
      fprintf( stderr, "Error: Vector full!\n" );
      err = VECTOR_ERR_FULL;
      goto cleanup;
   }

   if( 0 == v->size ) {
      v->size = 10;
      v->data = mem_alloc( v->size, void* );
      if( NULL == v->data ) {
         goto cleanup;
      }
   }

   while( v->size <= index || v->count == v->size - 1 ) {
      if( 0 != vector_grow( v, v->size * 2 ) ) {
         goto cleanup;
      }
   }

   for( i = v->count ; index < i ; i-- ) {
      v->data[i] = v->data[i - 1];
   }
   v->data[index] = data;
   v->count++;
   err = index;

cleanup:
   if( FALSE != ok ) {
      vector_lock( v, FALSE );
   }

   return err;
}

int vector_add( struct VECTOR* v, void* data ) {
   BOOL ok = FALSE;
   int err = 0;


   if( NULL == v || !vector_is_valid( v ) || FALSE != v->scalar ) {
      goto cleanup;
   }

   vector_lock( v, TRUE );
   ok = TRUE;

   if( 0 == v->size ) {
      v->size = 10;
      v->data = mem_alloc( v->size, void* );
      if( NULL == v->data ) {
         goto cleanup;
      }
   }

   if( v->size == v->count ) {
      if( VECTOR_SIZE_MAX <= v->size * 2 ) {
         fprintf( stderr, "Error: Vector full!\n" );
         err = VECTOR_ERR_FULL;
         goto cleanup;
      }
      if( 0!= vector_grow( v, v->size * 2 ) ) {
         goto cleanup;
      }
   }

   if( VECTOR_SIZE_MAX <= v->count + 1 ) {
      fprintf( stderr, "Error: Vector full!\n" );
      err = VECTOR_ERR_FULL;
      goto cleanup;
   }

   v->data[v->count] = data;
   err = v->count;
   v->count++;

cleanup:
   if( FALSE != ok ) {
      vector_lock( v, FALSE );
   }

   return err;
}

void vector_add_scalar( struct VECTOR* v, int32_t value, BOOL allow_dupe ) {
   size_t i;
   BOOL ok = FALSE;

   if( NULL == v || !vector_is_valid( v ) || TRUE != v->scalar ) {
      goto cleanup;
   }

   vector_lock( v, TRUE );
   ok = TRUE;

   v->scalar = TRUE;

   if( 0 == v->size || NULL == v->scalar_data ) {
      v->size = 10;
      v->scalar_data = mem_alloc( v->size, int32_t );
      if( NULL == v->scalar_data ) {
         goto cleanup;
      }
   }

   if( FALSE == allow_dupe ) {
      for( i = 0 ; NULL != v->scalar_data && v->count > i ; i++ ) {
         if( v->scalar_data[i] == value ) {
            fprintf(
               stderr,
               "Warning: Attempted to add duplicate %d to scalar vector.\n",
               value
            );
            goto cleanup;
         }
      }
   }

   if( v->size == v->count ) {
      if( 0 != vector_grow_scalar( v, v->size * 2 ) ) {
         goto cleanup;
      }
   }

   v->scalar_data[v->count] = value;
   v->count++;

cleanup:
   if( TRUE == ok ) {
      vector_lock( v, FALSE );
   }
   return;
}

void vector_set( struct VECTOR* v, size_t index, void* data, BOOL force ) {
   size_t new_size = v->size;

   if( NULL == v || !vector_is_valid( v ) || FALSE != v->scalar ) {
      goto cleanup;
   }

   vector_lock( v, TRUE );

   if( 0 == v->size ) {
      v->size = 10;
      v->data = (void**)mem_alloc( v->size, void* );
      if( NULL == v->data ) {
         goto cleanup;
      }
   }

   if( FALSE == force ) {
      if( index > v->count ) {
         goto cleanup;
      }
   } else if( index + 1 > v->size ) {
      while( v->size < index + 1 ) {
         new_size = v->size * 2;
         if( 0 != vector_grow( v, new_size ) ) {
            goto cleanup;
         }
      }
   }

   v->data[index] = data;

   /* TODO: Is this the right thing to do? */
   if( v->count <= index ) {
      v->count = index + 1;
   }

cleanup:
   vector_lock( v, FALSE );
   return;
}

void vector_set_scalar( struct VECTOR* v, size_t index, int32_t value ) {
   BOOL ok = FALSE;

   if( NULL == v || !vector_is_valid( v ) || TRUE != v->scalar ) {
      goto cleanup;
   }
   ok = TRUE;

   v->scalar = TRUE;

   if( 0 == v->size ) {
      v->size = index + 1;
      v->scalar_data = mem_alloc( v->size, int32_t );
      if( NULL == v->scalar_data ) {
         goto cleanup;
      }
   }

   if( index >= v->size ) {
      if( 0 != vector_grow_scalar( v, index + 1 ) ) {
         goto cleanup;
      }
   }

   v->scalar_data[index] = value;

cleanup:
   if( TRUE == ok ) {
      vector_lock( v, FALSE );
   }
   return;
}

void* vector_get( const struct VECTOR* v, size_t index ) {
   void* retptr = NULL;

   if( NULL == v || !vector_is_valid( v ) || FALSE != v->scalar ) {
      goto cleanup;
   }

   if( v->count <= index ) {
      goto cleanup;
   }

   retptr = v->data[index];

cleanup:

   return retptr;
}

int32_t vector_get_scalar( const struct VECTOR* v, size_t index ) {
   int32_t retval = -1;

   if( NULL == v || TRUE != v->scalar ) {
      goto cleanup;
   }

   if( index > v->count ) {
      goto cleanup;
   }

   retval = v->scalar_data[index];

cleanup:

   return retval;
}

int32_t vector_get_scalar_value( const struct VECTOR* v, int32_t value ) {
   int32_t retval = -1;
   size_t i;

   if( NULL == v ) {
      goto cleanup;
   }

   if( v->count <= 0 ) {
      goto cleanup;
   }

   for( i = 0 ; v->count > i ; i++ ) {
      if( value == v->scalar_data[i] ) {
         retval = value;
         goto cleanup;
      }
   }

cleanup:

   return retval;
}

/* Use a callback to delete items. The callback frees the item or decreases   *
 * its refcount as applicable.                                                */
size_t vector_remove_cb( struct VECTOR* v, vector_delete_cb callback, void* arg ) {
   size_t i, j;
   size_t removed = 0;

   /* FIXME: Delete dynamic arrays and reset when empty. */

   if( NULL == v || !vector_is_valid( v ) || FALSE != v->scalar ) {
      goto cleanup;
   }

   vector_lock( v, TRUE );

   for( i = 0 ; v->count > i ; i++ ) {

      /* The delete callback should call the object-specific free() function, *
       * which decreases its refcount naturally. So there's no need to do it  *
       * manually here.                                                       */
      if( NULL == callback || FALSE != callback( NULL, v, v->data[i], arg ) ) {
         removed++;
         for( j = i ; v->count - 1 > j ; j++ ) {
            v->data[j] = v->data[j + 1];
            v->data[j + 1] = NULL;
         }
         i--;
         v->count--;
      }
   }

#ifdef DEBUG
   if( NULL == arg ) {
      assert( 0 == v->count );
   }
#endif /* DEBUG */

   vector_lock( v, FALSE );

cleanup:
   return removed;
}

size_t vector_remove_all( struct VECTOR* v ) {
   return vector_remove_cb( v, NULL, NULL );
}

void vector_remove( struct VECTOR* v, size_t index ) {
   size_t i;
   BOOL ok = FALSE;

   /* FIXME: Delete dynamic arrays and reset when empty. */

   if( NULL == v || !vector_is_valid( v ) || FALSE != v->scalar ) {
      goto cleanup;
   }

   vector_lock( v, TRUE );
   ok = TRUE;

   if( index > v->count ) {
      goto cleanup;
   }

   for( i = index; v->count - 1 > i ; i++ ) {
      v->data[i] = v->data[i + 1];
      v->data[i + 1] = NULL;
   }

   v->count--;

cleanup:
   if( FALSE != ok ) {
      vector_lock( v, FALSE );
   }
   return;
}

void vector_remove_scalar( struct VECTOR* v, size_t index ) {
   size_t i;

   if( NULL == v || !vector_is_valid( v ) || TRUE != v->scalar ) {
      goto cleanup;
   }

   vector_lock( v, TRUE );

   if( 1 >= v->count ) {
      /* Delete dynamic arrays and reset if now empty. */
      vector_reset( v );
   } else {
      for( i = index; v->count - 1 > i ; i++ ) {
         v->scalar_data[i] = v->scalar_data[i + 1];
      }
      v->count -= 1;
   }

   vector_lock( v, FALSE );

cleanup:
   return;
}

size_t vector_remove_scalar_value( struct VECTOR* v, int32_t value ) {
   size_t i;
   size_t difference = 0;

   if( NULL == v || !vector_is_valid( v ) || TRUE != v->scalar ) {
      goto cleanup;
   }

   vector_lock( v, TRUE );

   if( 1 >= v->count ) {
      /* Delete dynamic arrays and reset if now empty. */
      vector_reset( v );
   } else {
      for( i = 0; v->count - difference > i ; i++ ) {
         if( v->scalar_data[i] == value ) {
            difference++;
         }
         v->scalar_data[i] = v->scalar_data[i + difference];
      }
      v->count -= difference;
   }

   vector_lock( v, FALSE );

cleanup:
   return difference;
}

size_t vector_count( const struct VECTOR* v ) {
   if( NULL == v || !vector_is_valid( v ) ) {
      goto cleanup;
   }
   return v->count;
cleanup:
   return 0;
}

void vector_lock( struct VECTOR* v, BOOL lock ) {
   #ifdef USE_THREADS
   #error Locking mechanism undefined!
   #elif defined( DEBUG )
   if( TRUE == lock ) {
      assert( 0 == v->lock_count );
      v->lock_count++;
   } else {
      assert( 1 == v->lock_count );
      v->lock_count--;
   }
   #endif /* USE_THREADS */
}

#if 0

/** \brief Iterate through the given vector with the given callback.
 * \param[in] v         The vector through which to iterate.
 * \param[in] callback  The callback to run on each item.
 * \param[in] arg       The argument to pass the callback.
 * \return If one of the callbacks returns an item, the iteration loop will
 *         break and return this item. Otherwise, NULL will be returned.
 */
void* vector_iterate( struct VECTOR* v, vector_search_cb callback, void* arg ) {
   void* cb_return = NULL;

   scaffold_check_null( v );
   scaffold_assert( vector_is_valid( v ) );
   /* TODO: This can work for scalars too, can't it? */
   scaffold_assert( FALSE == v->scalar );

   vector_lock( v, TRUE );
   cb_return = vector_iterate_nolock( v, callback, v, arg );
   vector_lock( v, FALSE );

cleanup:
   return cb_return;
}

void* vector_iterate_nolock(
   struct VECTOR* v, vector_search_cb callback, void* parent, void* arg
) {
   void* cb_return = NULL;
   void* current_iter = NULL;
   size_t i;
   struct CONTAINER_IDX idx = { 0 };
   size_t v_count;

   scaffold_check_null( v );
   scaffold_assert( vector_is_valid( v ) );
   /* TODO: This can work for scalars too, can't it? */
   scaffold_assert( FALSE == v->scalar );

   idx.type = CONTAINER_IDX_NUMBER;

   /* If a parent wasn't explicitly provided, use the vector, itself. */
   if( NULL == parent ) {
      parent = v;
   }

   v_count = vector_count( v );
   for( i = 0 ; v_count > i ; i++ ) {
      current_iter = vector_get( v, i );
      idx = i;
      cb_return = callback( &idx, parent, current_iter, arg );
      if( NULL != cb_return ) {
         break;
      }
   }

cleanup:
   return cb_return;
}

/** \brief Iterate through the given vector with the given callback in reverse.
 * \param[in] v         The vector through which to iterate.
 * \param[in] callback  The callback to run on each item.
 * \param[in] arg       The argument to pass the callback.
 * \return If one of the callbacks returns an item, the iteration loop will
 *         break and return this item. Otherwise, NULL will be returned.
 */
void* vector_iterate_r( struct VECTOR* v, vector_search_cb callback, void* arg ) {
   void* cb_return = NULL;
   void* current_iter = NULL;
   size_t i;
   struct CONTAINER_IDX idx = { 0 };

   if( NULL == v ) {
      goto cleanup;
   }

   idx.type = CONTAINER_IDX_NUMBER;

   vector_lock( v, TRUE );
   for( i = vector_count( v ) ; 0 < i ; i-- ) {
      current_iter = vector_get( v, i - 1 );
      idx.value.index = i - 1;
      cb_return = callback( &idx, v, current_iter, arg );
      if( NULL != cb_return ) {
         break;
      }
   }
   vector_lock( v, FALSE );

cleanup:
   return cb_return;
}

struct VECTOR* vector_iterate_v(
   struct VECTOR* v, vector_search_cb callback, void* parent, void* arg
) {
   struct VECTOR* found = NULL;
   void* current_iter = NULL;
   void* cb_return = NULL;
   BOOL ok = FALSE;
   size_t i;
   struct CONTAINER_IDX idx = { 0 };
   int add_err = 0;
   size_t v_count;

   scaffold_check_null( v );
   scaffold_assert( vector_is_valid( v ) );

   vector_lock( v, TRUE );
   ok = TRUE;

   idx.type = CONTAINER_IDX_NUMBER;

   if( NULL == parent ) {
      parent = v;
   }

   /* Linear probing */
   v_count = vector_count( v );
   for( i = 0 ; v_count > i ; i++ ) {
      current_iter = vector_get( v, i );
      idx.value.index = i;
      cb_return = callback( &idx, parent, current_iter, arg );
      if( NULL != cb_return ) {
         if( NULL == found ) {
            vector_new( found );
         }
         add_err = vector_add( found, cb_return );
         if( 0 > add_err ) {
            scaffold_print_debug(
               &module, "Insufficient space for results vector.\n"
            );
            goto cleanup;
         }
      }
   }

cleanup:
   if( TRUE == ok ) {
      vector_lock( v, FALSE );
   }
   return found;
}

void vector_sort_cb( struct VECTOR* v, vector_sorter_cb callback ) {
   void* previous_iter = NULL;
   void* current_iter = NULL;
   size_t i;
   VECTOR_SORT_ORDER o;
   BOOL ok = FALSE;
   size_t v_count;

   scaffold_check_null( v );
   scaffold_assert( vector_is_valid( v ) );

   if( 2 > vector_count( v ) ) {
      /* Not enough to sort! */
      goto cleanup;
   }

   /* TODO: This can work for scalars too, can't it? */
   scaffold_assert( FALSE == v->scalar );

   vector_lock( v, TRUE );
   ok = TRUE;

   v_count = vector_count( v );
   for( i = 1 ; v_count > i ; i++ ) {
      current_iter = vector_get( v, i );
      previous_iter = vector_get( v, (i - 1) );
      o = callback( previous_iter, current_iter );
      switch( o ) {
      case VECTOR_SORT_A_B_EQUAL:
         break;

      case VECTOR_SORT_A_LIGHTER:
         break;

      case VECTOR_SORT_A_HEAVIER:
         v->data[i] = previous_iter;
         v->data[i - 1] = current_iter;
         break;
      }
   }

cleanup:
   if( FALSE != ok ) {
      vector_lock( v, FALSE );
   }
   scaffold_assert( 0 == v->lock_count );
   return;
}

#endif

BOOL vector_is_valid( const struct VECTOR* v ) {
   return NULL != v && VECTOR_SENTINAL == v->sentinal;
}
