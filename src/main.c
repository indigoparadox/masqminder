
#include <stdio.h>

#include "netipc.h"
#include "http.h"
#include "vector.h"

#define ERR_ALLOCATION 0x02

struct tagbstring str_ctype = bsStatic( "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n" );
struct tagbstring str_hello = bsStatic( "<!DOCTYPE html>\n<html><body>Hello World!</body></html>\n" );

int main( int argc, char** argv ) {
   BOOL running = TRUE;
   struct CONNECTION n = { 0 },
      *n_new_client = NULL,
      *n_iter_client = NULL;
   struct VECTOR* clients = NULL;
   int written = 0;
   int retval = 0;
   size_t i = 0;

   memset( &n, '\0', sizeof( struct CONNECTION ) );

   //clients = mem_alloc( clients_size, struct CONNECTION );
   vector_new( clients );
   if( NULL == clients ) {
      fprintf( stderr, "Unable to allocate clients array.\n" );
      retval &= ERR_ALLOCATION;
      goto cleanup;
   }

   if( ipc_listen( &n, 5000 ) ) {
      while( running ) {
         /* Two discrete sections that can be threaded later. */

         /* Accept new clients */
         if( NULL == n_new_client ) {
            n_new_client = mem_alloc( 1, struct CONNECTION );
         }
         if( ipc_accept( &n, n_new_client ) ) {
            vector_add( clients, n_new_client );
            printf( "Accepted connection.\n" );
            n_new_client = NULL;
         }

         /* Handle existing clients. */
         for( i = 0 ; vector_count( clients ) > i ; i++ ) {
            n_iter_client = vector_get( clients, i );
            printf( "Written: %d\n", ipc_write( n_iter_client, &str_ctype ) );
            printf( "Written: %d\n", ipc_write( n_iter_client, &str_hello ) );
            ipc_stop( n_iter_client );
         }
         vector_remove_all( clients );
      }
   }


cleanup:
   ipc_stop( &n );
   vector_free_force( &clients );

   return retval;
}
