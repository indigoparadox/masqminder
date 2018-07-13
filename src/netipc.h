
#ifndef IPC_H
#define IPC_H

#include <stdint.h>

#include "bstrlib/bstrlib.h"
#include "mem.h"

#define IPC_BUFFER_LEN 80

typedef enum IPC_END {
   IPC_END_SERVER,
   IPC_END_CLIENT
} IPC_END;

struct CONNECTION {
   BOOL listening;
   IPC_END type;
   BOOL local_client;
   uint16_t port;
   void* (*callback)( void* client );
   void* arg;
   int socket;
};

void ipc_setup();
void ipc_shutdown();
struct CONNECTION* ipc_alloc();
void ipc_free( struct CONNECTION** n );
BOOL ipc_connected( struct CONNECTION* n );
BOOL ipc_listen( struct CONNECTION* n, uint16_t port );
BOOL ipc_connect( struct CONNECTION* n, const bstring server, uint16_t port );
BOOL ipc_accept( struct CONNECTION* n_server, struct CONNECTION* n );
int ipc_write( struct CONNECTION* n, const bstring buffer );
int ipc_read( struct CONNECTION* n, bstring buffer );
void ipc_stop( struct CONNECTION* n );
IPC_END ipc_get_type( struct CONNECTION* n );
BOOL ipc_is_listening( struct CONNECTION* n );
uint16_t ipc_get_port( struct CONNECTION* n );

#endif /* IPC_H */
