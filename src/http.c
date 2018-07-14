
#include "http.h"

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

static int http_write( struct CONNECTION* client, bstring line ) {
   int written = 0;
#ifdef DEBUG
   pid_t pid = 0;

   pid = getpid();
#endif /* DEBUG */
   written = ipc_write( client, line );
#ifdef DEBUG
   if( '\n' == bchar( line, blength( line ) - 1 ) ) {
      printf( "%d: Written: %d: %s", pid, written, bdata( line ) );
   } else {
      printf( "%d: Written: %d: %s\n", pid, written, bdata( line ) );
   }
#endif /* DEBUG */
   return written;
}

static int http_handle_get( struct CONNECTION* client, bstring body ) {
   int header_i = 1;
   int written = 0;
   bstring header_b = NULL;

   header_b = bfromcstr( "" );

   do {

      switch( header_i ) {
      case 1:
         bassignformat( header_b, "HTTP/1.1 200 OK\r\n" );
         written += http_write( client, header_b );
         header_i++;
         break;

      case 2:
         bassignformat( header_b, "Server: MasqMinder\r\n" );
         written += http_write( client, header_b );
         header_i++;
         break;

      case 3:
         bassignformat( header_b,
            "Content-Length: %d\r\n", blength( body ) );
         written += http_write( client, header_b );
         header_i++;
         break;

      case 4:
         bassignformat( header_b, "Connection: close\r\n" );
         written += http_write( client, header_b );
         header_i++;
         break;

      case 5:
         bassignformat( header_b, "Content-Type: text/html\r\n\r\n" );
         written += http_write( client, header_b );
         header_i++;
         break;

      case 6:
         written += http_write( client, body );
         goto cleanup;
      }
   } while( /* 0 < read || 0 < written */ 1 );

cleanup:
   bdestroy( header_b );
   return written;
}

void http_handle_connection( struct CONNECTION* client, bstring body ) {
   bstring buffer = NULL;
   int written = 0,
      read = 0;
#ifdef DEBUG
   pid_t pid = 0;
#endif /* DEBUG */

   buffer = bfromcstr( "" );

#ifdef DEBUG
   pid = getpid();
#endif /* DEBUG */

   do {
      bassigncstr( buffer, "" );
      read = ipc_read( client, buffer );
#ifdef DEBUG
      printf( "%d: Read %d: %s", pid, read, bdata( buffer ) );
#endif /* DEBUG */

      if( 0 == strncmp( bdata( buffer ), "GET", 3 ) ) {
         written = http_handle_get( client, body );
         goto cleanup;
      }
   } while( /* 0 < read || 0 < written */ 1 );

   /* do {
      bassigncstr( buffer, "" );
      read = ipc_read( n_iter_client, buffer );
      printf( "Read: %s\n", bdata( buffer ) );
   } while( 0 < read ); */

cleanup:
   ipc_stop( client );
   bdestroy( buffer );

   return;
}

