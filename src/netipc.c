
/* Need to include winsock stuff before windows.h in scaffold. */
#if defined( _WIN32 ) && defined( USE_NETWORK )
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment( lib, "Ws2_32.lib" )
#pragma comment( lib, "Mswsock.lib" )
#pragma comment( lib, "AdvApi32.lib" )
#endif /* _WIN32 && USE_NETWORK */

#include "netipc.h"
#include "libvcol.h"

#ifdef _GNU_SOURCE
#include <unistd.h>
#endif /* _GNU_SOURCE */

/* This block is really finicky in terms of order. */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <netdb.h>
#endif /* _WIN32 */
#include <sys/types.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* _WIN32 */

static void net_ipc_cleanup_socket( struct CONNECTION* n ) {
   if( 0 != n->socket ) {
      close( n->socket );
      n->socket = 0;
   }
}

void ipc_setup() {
#if defined( _WIN32 ) && defined( USE_NETWORK )
   int result = 0;
   WSADATA wsa_data = { 0 };

   result = WSAStartup( MAKEWORD(2,2), &wsa_data );
   if( 0 != result ) {
      scaffold_print_error(
         &module,
         "WSAStartup failed with error: %d\n", result
      );
      scaffold_error = SCAFFOLD_ERROR_CONNECTION_CLOSED;
   }

cleanup:
#endif /* _WIN32 && USE_NETWORK */
   return;
}

void ipc_shutdown() {
   /* TODO */
}

struct CONNECTION* ipc_alloc() {
   struct CONNECTION* n_out = NULL;

   n_out =  mem_alloc( 1, struct CONNECTION );
   if( NULL == n_out ) {
      fprintf( stderr, "Unable to alloc connection.\n" );
      goto cleanup;
   }
   n_out->local_client = FALSE;
   n_out->listening = FALSE;

cleanup:
   return n_out;
}

void ipc_free( struct CONNECTION** n ) {
   mem_free( *n );
   n = NULL;
}

BOOL ipc_connected( struct CONNECTION* n ) {
   if( NULL != n && 0 < n->socket ) {
      return TRUE;
   } else {
      return FALSE;
   }
}

BOOL ipc_connect( struct CONNECTION* n, const bstring server, uint16_t port ) {
   BOOL connected = FALSE;
   int connect_result;
#ifdef _WIN32
   u_long mode = 1;
#endif /* _WIN32 */
#ifndef USE_LEGACY_NETWORK
   bstring service;
   struct addrinfo hints = { 0 },
         * result;
#else
   unsigned long ul_addr;
   struct sockaddr_in dest = { 0 };
#endif /* !USE_LEGACY_NETWORK */

   n->local_client = TRUE;
   n->type = IPC_END_CLIENT;

#ifdef USE_LEGACY_NETWORK
   unsigned long ul_addr;
   struct sockaddr_in dest = { 0 };

   ul_addr = inet_addr( bdata( server ) );

   n->socket = socket(
      AF_INET,
      SOCK_STREAM,
      0
   );

   dest.sin_family = AF_INET;
   dest.sin_addr.s_addr = ul_addr;
   dest.sin_port = htons( port );

   connect_result = connect(
      n->socket,
      (const struct sockaddr*)&dest,
      sizeof( struct sockaddr_in )
   );

   if( 0 > connect_result ) {
      net_ipc_cleanup_socket( n );
      goto cleanup;
   }

   /* TODO: Check for error. */
#ifdef _WIN32
   ioctlsocket( n->socket, FIONBIO, &mode );
#else
   fcntl( n->socket, F_SETFL, O_NONBLOCK );
#endif /* _WIN32 */

cleanup:

#else

   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   service = bformat( "%d", port );

   connect_result = getaddrinfo(
      bdata( server ), bdata( service ), &hints, &result
   );
   if( 0 != connect_result ) {
      goto cleanup;
   }

   n->socket = socket(
      result->ai_family,
      result->ai_socktype,
      result->ai_protocol
   );

   connect_result = connect(
      n->socket,
      result->ai_addr,
      result->ai_addrlen
   );

   if( 0 > connect_result ) {
      fprintf( stderr, "Connect error: %s\n", strerror( errno ) );
      net_ipc_cleanup_socket( n );
      goto cleanup;
   }

   /* TODO: Check for error. */
#ifdef _WIN32
   ioctlsocket( n->socket, FIONBIO, &mode );
#else
   fcntl( n->socket, F_SETFL, O_NONBLOCK );
#endif /* _WIN32 */

   connected = TRUE;
   n->port = port;

cleanup:

   bdestroy( service );
   freeaddrinfo( result );
#endif /* USE_LEGACY_NETWORK */

   return connected;
}

int ipc_write( struct CONNECTION* n, const bstring buffer ) {
   int sent = -1;
   const char* buffer_chars;
   size_t dest_socket;
   size_t buffer_len,
      total_sent = 0;

   buffer_chars = bdata( buffer );
#ifdef DEBUG
   assert( NULL != buffer_chars );
#endif /* DEBUG */

   dest_socket = n->socket;
   buffer_len = blength( buffer );
   if( 0 == dest_socket ) {
      fprintf( stderr, "Invalid destination socket.\n" );
      goto cleanup;
   }

   do {
      sent = send(
         dest_socket,
         &(buffer_chars[total_sent]),
         buffer_len - total_sent,
#ifdef _WIN32
         0
#else
         MSG_NOSIGNAL
#endif /* _WIN32 */
      );
      total_sent += sent;
   } while( total_sent < buffer_len );

cleanup:
   return sent;
}

BOOL ipc_accept( struct CONNECTION* n_server, struct CONNECTION* n ) {
   BOOL connected = FALSE;
   unsigned int address_length;
   struct sockaddr_in address;
   #ifdef _WIN32
   u_long mode = 1;
   #endif /* _WIN32 */

   /* Accept and verify the client. */
   address_length = sizeof( address );
   n->socket = accept(
      n_server->socket, (struct sockaddr*)&address,
      &address_length
   );

   /* No connection incoming, this time! */
   if( 0 > n->socket && (EWOULDBLOCK == errno || EAGAIN == errno) ) {
      goto cleanup;
   }

   /* TODO: Check for error. */
#ifdef _WIN32
   ioctlsocket( n->socket, FIONBIO, &mode );
#else
   fcntl( n->socket, F_SETFL, O_NONBLOCK );
#endif /* _WIN32 */

   if( 0 > n->socket ) {
      fprintf( stderr, "Error while connecting on %d: %d\n", n->socket, errno );
      ipc_stop( n );
      n->socket = 0;
      goto cleanup;
   }

   connected = TRUE;
   n->local_client = FALSE;
   n->type = IPC_END_CLIENT;

cleanup:
   return connected;
}

BOOL ipc_listen( struct CONNECTION* n, uint16_t port ) {
   int connect_result;
   struct sockaddr_in address;

   if( TRUE == n->listening ) {
      fprintf( stderr, "Server already listening!\n" );
      goto cleanup;
   }

#ifdef _WIN32
   u_long mode = 1;
#endif /* _WIN32 */

   n->socket = socket( AF_INET, SOCK_STREAM, 0 );
   if( 0 > n->socket ) {
      goto cleanup;
   }

   /* TODO: Check for error. */
#ifdef _WIN32
   ioctlsocket( n->socket, FIONBIO, &mode );
#else
   fcntl( n->socket, F_SETFL, O_NONBLOCK );
#endif /* _WIN32 */

   /* Setup and bind the port, first. */
   address.sin_family = AF_INET;
   address.sin_port = htons( port );
   address.sin_addr.s_addr = INADDR_ANY;

   connect_result = bind(
      n->socket, (struct sockaddr*)&address, sizeof( address )
   );

   if( 0 > connect_result ) {
      fprintf( stderr, "Unable to open listening socket.\n" );
      net_ipc_cleanup_socket( n );
      goto cleanup;
   }

   /* If we could bind the port, then launch the serving connection. */
   connect_result = listen( n->socket, 5 );
   if( 0 > connect_result ) {
      fprintf( stderr, "Unable to listen on socket.\n" );
      net_ipc_cleanup_socket( n );
      goto cleanup;
   }

   n->listening = TRUE;
   n->local_client = FALSE;
   n->type = IPC_END_SERVER;

   /* "Now listening for connection on port %d... */

cleanup:
   return n->listening;
}

int ipc_read( struct CONNECTION* n, bstring buffer ) {
	int total_read_count = 0;
	int bstr_res;
   int last_read_count = 0;
   char read_char = '\0';

   while( '\n' != read_char ) {
      if( NULL == n ) {
         break;
      }
      last_read_count = recv( n->socket, &read_char, 1, 0 );

      if( 0 == last_read_count ) {
         fprintf(
            stderr, "Remote connection (%d) has been closed.\n", n->socket
         );
         close( n->socket );
         n->socket = 0;
         goto cleanup;
      }

      if( 0 > last_read_count ) {
         goto cleanup;
      }

      /* No error and something was read, so add it to the string. */
      total_read_count++;
      bstr_res = bconchar( buffer, read_char );
      if( 0 != bstr_res ) {
         goto cleanup;
      }
   }

cleanup:
   return total_read_count;
}

void ipc_stop( struct CONNECTION* n ) {
   net_ipc_cleanup_socket( n );
   n->listening = FALSE;
}

IPC_END ipc_get_type( struct CONNECTION* n ) {
   return n->type;
}

BOOL ipc_is_listening( struct CONNECTION* n ) {
   return n->listening;
}

uint16_t ipc_get_port( struct CONNECTION* n ) {
   return n->port;
}
