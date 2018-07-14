
#ifndef HTTP_H
#define HTTP_H

#include "bstrlib/bstrlib.h"
#include "netipc.h"

struct HTTP_NODE;

union HTTP_CONTENTS {
   bstring data;
   struct HTTP_NODE* first_child;
};

struct HTTP_NODE {
   bstring name;
   union HTTP_CONTENTS contents;
   struct HTTP_NODE* next;
};

struct HTTP_TREE {


   struct HTTP_NODE* root_dir;
};

void http_handle_connection(
   struct CONNECTION* client, struct HTTP_TREE* tree );
struct HTTP_NODE* http_tree_get_path( struct HTTP_TREE* tree, bstring path );
void http_tree_add_directory(
   struct HTTP_NODE* parent, bstring name, bstring contents );
void http_tree_add_file(
   struct HTTP_NODE* parent, bstring name, bstring contents );

#endif /* HTTP_H */
