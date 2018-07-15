
#ifndef HTTP_H
#define HTTP_H

#include "libvcol.h"
#include "netipc.h"

enum HTTP_TYPE {
   HTTP_TYPE_NONE,
   HTTP_TYPE_FILE_BIN,
   HTTP_TYPE_FILE_HTML,
   HTTP_TYPE_DIRECTORY
};

union HTTP_CONTENTS {
   struct html_tree* doc;
   void* bytes;
   struct HASHMAP* children;
};

struct HTTP_NODE {
   enum HTTP_TYPE type;
   union HTTP_CONTENTS contents;
};

struct HTTP_TREE {
   bstring default_filename;
   struct HTTP_NODE* root_dir;
};

void http_handle_connection(
   struct CONNECTION* client, struct HTTP_TREE* tree );
struct HTTP_NODE* http_node_get_path( struct HTTP_NODE* parent, bstring path );
struct HTTP_NODE* http_tree_get_path( struct HTTP_TREE* tree, bstring path );
struct HTTP_NODE*
   http_tree_add_directory( struct HTTP_NODE* parent, bstring name );
struct HTTP_NODE* http_tree_add_file(
   struct HTTP_NODE* parent, const bstring name, struct html_tree* contents );
void http_tree_free_directory( struct HTTP_NODE* dir );
void http_tree_free_file( struct HTTP_NODE* fil );

#endif /* HTTP_H */
