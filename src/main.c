
#include "netipc.h"
#include "http.h"

const struct tagbstring str_ctype = bsStatic( "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n" );
const struct tagbstring str_hello = bsStatic( "<!DOCTYPE html>\n<html><body>Hello World!</body></html>\n" );

int main( int argc, char** argv ) {
   BOOL running = TRUE;
   struct CONNECTION n;
   struct CONNECTION client;
   int written = 0;
   memset( &n, '\0', sizeof( struct CONNECTION ) );
   memset( &client, '\0', sizeof( struct CONNECTION ) );

   if( ipc_listen( &n, 5000 ) ) {
      while( running ) {
         if( ipc_accept( &n, &client ) ) {
            printf( "Accepted connection.\n" );
            printf( "Written: %d\n", ipc_write( &client, &str_ctype ) );
            printf( "Written: %d\n", ipc_write( &client, &str_hello ) );
            ipc_stop( &client );
         }
      }
   }

   ipc_stop( &n );

   return 0;
}
