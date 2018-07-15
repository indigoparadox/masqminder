
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "netipc.h"
#include "http.h"
#include "vector.h"
#include "htmtree.h"

//#define ERR_ALLOCATION 0x02

//struct tagbstring str_ctype = bsStatic( "HTTP/1.1 200 OK\r\nServer: MasqMinder\r\nContent-Length:Connection: close\r\nContent-Type: text/html\r\n\r\n" );
//struct tagbstring str_hello = bsStatic( "<!DOCTYPE html>\n<html><body>Hello World!</body></html>\n" );

struct tagbstring str_html = bsStatic( "html" );
struct tagbstring str_tag_head = bsStatic( "head" );
struct tagbstring str_title = bsStatic( "title" );
struct tagbstring str_tag_body = bsStatic( "body" );
struct tagbstring str_tag_h1 = bsStatic( "h1" );
struct tagbstring str_id = bsStatic( "id" );
struct tagbstring str_tag_doctype = bsStatic( "!DOCTYPE" );

static volatile int running = 1;

/* void handle_sig( int sig_num ) {
   signal( SIGINT, handle_sig );
   running = 0;
} */

struct html_tree* build_doc() {
   struct html_tree* doc = NULL;

   doc = mem_alloc( 1, struct html_tree );
   html_tree_new_tag( doc, &str_tag_doctype );
   html_tree_new_attr( doc->current, &str_html, NULL );
   html_tree_up( doc );
   html_tree_new_tag( doc, &str_html );
   html_tree_new_tag( doc, &str_tag_head );
   html_tree_new_tag( doc, &str_title );
   doc->current->data = bfromcstr( "Test Page" );
   html_tree_up( doc );
   html_tree_up( doc );
   html_tree_new_tag( doc, &str_tag_body );
   html_tree_new_tag( doc, &str_tag_h1 );
   doc->current->data = bfromcstr( "Test Page" );
   html_tree_new_attr( doc->current, &str_id, &str_title );
   html_tree_up( doc );
   html_tree_up( doc );

   return doc;
}

struct HTTP_TREE* build_tree( struct html_tree* index ) {
   struct HTTP_TREE* tree = NULL;

   tree = mem_alloc( 1, struct HTTP_TREE );
   tree->root_dir = mem_alloc( 1, struct HTTP_NODE );
   tree->root_dir->type = HTTP_TYPE_DIRECTORY;
   http_tree_add_file( tree->root_dir, bfromStatic( "index.html" ), index );

   return tree;
};

int main( int argc, char** argv ) {
   struct CONNECTION client = { 0 };
   struct CONNECTION n = { 0 };
   struct HTTP_TREE* tree = NULL;
   int retval = 0;
   int fork_res = 0;
   pid_t pid = 0;
   struct html_tree* doc = NULL;
   bstring doc_b = NULL;

   pid = getpid();

   signal( SIGCHLD, SIG_IGN );
   //signal( SIGINT, handle_sig );

   if( ipc_listen( &n, 5100 ) ) {
      while( running ) {
         /* Accept new clients */
         if( ipc_accept( &n, &client ) ) {
#ifdef USE_FORK
            fork_res = fork();
            if( 0 < fork_res ) {
#endif /* USE_FORK */
               printf( "Parent: Accepted connection: %d\n", fork_res );
#ifdef USE_FORK
               ipc_stop( &client );
               continue;
            } else if( 0 > fork_res ) {
               fprintf( stderr, "Fork error: %d\n", fork_res );
               continue;
            }
#endif /* USE_FORK */

            pid = getpid();
            doc = build_doc();
            tree = build_tree( doc );
            http_handle_connection( &client, tree );
#ifdef USE_FORK
            goto cleanup;
#else
            ipc_stop( &client );
#endif /* USE_FORK */
         }
      }
   }

cleanup:

   printf( "Ending process: %d\n", pid );
   fflush( stdout );

   html_tree_cleanup( doc->root );
   if( NULL != tree ) {
      http_tree_free_directory( tree->root_dir );
      mem_free( tree );
   }
   mem_free( doc );
   bdestroy( doc_b );
   ipc_stop( &n );
   //getchar();

   return retval;
}
