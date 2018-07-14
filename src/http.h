
#ifndef HTTP_H
#define HTTP_H

#include "bstrlib/bstrlib.h"
#include "netipc.h"

void http_handle_connection( struct CONNECTION* client, bstring body );

#endif /* HTTP_H */
