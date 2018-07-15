
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

void http_handle_connection(
   struct CONNECTION* client, struct HTTP_TREE* tree
) {
   bstring buffer = NULL,
      req_path = NULL;
   int written = 0,
      read = 0;
#ifdef DEBUG
   pid_t pid = 0;
#endif /* DEBUG */
   bstring body = NULL;
   struct HTTP_NODE* fil = NULL;

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
         fil = http_tree_get_path( tree, bfromStatic( "/index.html" ) );
         body = html_tree_to_bstr( fil->contents.doc );
         written = http_handle_get( client, body );
         goto cleanup;
      }
   } while( /* 0 < read || 0 < written */ 1 );

cleanup:
   ipc_stop( client );
   bdestroy( buffer );
   bdestroy( body );
   bdestroy( req_path );
   return;
}

struct HTTP_NODE* http_node_get_path( struct HTTP_NODE* parent, bstring path ) {
   int start_pos = 0;
   int end_pos = 0;
   struct HTTP_NODE* node_found = NULL;
   struct HTTP_NODE* node_next = NULL;
   bstring next_name = NULL;
   bstring search_path_next = NULL;
   int name_length = 0;

   if( NULL == parent || NULL == path ) {
      goto cleanup;
   }

   while( '/' == bchar( path, start_pos ) ) {
      start_pos++;
   }

   end_pos = bstrchrp( path, '/', start_pos );
   if( BSTR_ERR == end_pos ) {
      name_length = blength( path ) - start_pos;
   } else {
      name_length = end_pos - start_pos - 1;
   }
   next_name = bmidstr( path, start_pos, name_length );

   printf( "Descending to: %s\n", bdata( next_name ) );

   search_path_next = bmidstr( path, end_pos, blength( path ) - end_pos );

   node_found = hashmap_get( parent->contents.children, next_name );
   node_next = http_node_get_path( node_found, search_path_next );
   if( NULL == node_next ) {
      node_next = node_found;
   }

cleanup:
   bdestroy( next_name );
   bdestroy( search_path_next );
   return node_next;
}

struct HTTP_NODE* http_tree_get_path( struct HTTP_TREE* tree, bstring path ) {
   struct HTTP_NODE* node = NULL;

   if( NULL == tree || NULL == tree->root_dir ) {
      goto cleanup;
   }

   node = http_node_get_path( tree->root_dir, path );

cleanup:
   return node;
}

struct HTTP_NODE*
http_tree_add_directory( struct HTTP_NODE* parent, bstring name ) {
   struct HTTP_NODE* new_dir = NULL;

   if( NULL == parent || HTTP_TYPE_DIRECTORY != parent->type ) {
      /* TODO: Error. */
      goto cleanup;
   }

   if( NULL == parent->contents.children ) {
      hashmap_new( parent->contents.children );
   }

   new_dir = mem_alloc( 1, struct HTTP_NODE );
   new_dir->type = HTTP_TYPE_DIRECTORY;
   hashmap_new( new_dir->contents.children );

   if(
      HASHMAP_ERROR_ITEM_EXISTS ==
      hashmap_put( parent->contents.children, name, new_dir, FALSE )
   ) {
      /* Unable to replace existing directory. */
      http_tree_free_directory( new_dir );
      new_dir = FALSE;
   }

cleanup:
   return new_dir;
}

struct HTTP_NODE* http_tree_add_file(
   struct HTTP_NODE* parent, const bstring name, struct html_tree* contents
) {
   struct HTTP_NODE* new_file = NULL;

   if( NULL == parent || HTTP_TYPE_DIRECTORY != parent->type ) {
      /* TODO: Error. */
      goto cleanup;
   }

   if( NULL == parent->contents.children ) {
      hashmap_new( parent->contents.children );
   }

   new_file = mem_alloc( 1, struct HTTP_NODE );
   new_file->type = HTTP_TYPE_FILE_HTML;
   new_file->contents.doc = contents;

   if(
      HASHMAP_ERROR_ITEM_EXISTS ==
      hashmap_put( parent->contents.children, name, new_file, FALSE )
   ) {
      /* Unable to replace existing directory. */
      http_tree_free_file( new_file );
      new_file = FALSE;
   }

cleanup:
   return new_file;
}

void http_tree_free_directory( struct HTTP_NODE* dir ) {
   if( NULL == dir || HTTP_TYPE_DIRECTORY != dir->type ) {
      /* TODO: Error. */
      goto cleanup;
   }

   if( NULL != dir->contents.children ) {
      hashmap_free( &(dir->contents.children) );
   }
   mem_free( dir );

cleanup:
   return;
}

void http_tree_free_file( struct HTTP_NODE* fil ) {
   if( NULL == fil ) {
      /* TODO: Error. */
      goto cleanup;
   }

   switch( fil->type ) {
   case HTTP_TYPE_FILE_HTML:
      if( NULL != fil->contents.doc ) {
         html_tree_cleanup( html_tree_get_root( fil->contents.doc ) );
         mem_free( fil->contents.doc );
      }
      mem_free( fil );
   }

cleanup:
   return;
}
