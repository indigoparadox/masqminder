
#include "htmtree.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "mem.h"

#define html_bstring_debug( bstr_res ) \
   fprintf( stderr, __FILE__ ": %d: bstring error: %d\n", __LINE__, bstr_res )

struct html_tree_entity_def {
   struct tagbstring name;
   struct tagbstring character;
};

static struct tagbstring tag_name = bsStatic( "name" );

#define HTML_TREE_SINGLETON_COUNT 18

static struct tagbstring html_tree_singleton_tags[HTML_TREE_SINGLETON_COUNT] = {
   bsStatic( "!--" ),
   bsStatic( "!DOCTYPE" ),
   bsStatic( "area" ),
   bsStatic( "base" ),
   bsStatic( "br" ),
   bsStatic( "col" ),
   bsStatic( "command" ),
   bsStatic( "embed" ),
   bsStatic( "hr" ),
   bsStatic( "img" ),
   bsStatic( "input" ),
   bsStatic( "keygen" ),
   bsStatic( "link" ),
   bsStatic( "meta" ),
   bsStatic( "param" ),
   bsStatic( "source" ),
   bsStatic( "track" ),
   bsStatic( "wbr" )
};

static bstring tag_comment = &(html_tree_singleton_tags[0]);

#define HTML_TREE_ENTITY_COUNT 12

static struct html_tree_entity_def entities[HTML_TREE_ENTITY_COUNT] = {
   { bsStatic( "dash" ), bsStatic( "-" ) },
   { bsStatic( "copy" ), bsStatic( "©" ) },
   { bsStatic( "amp" ), bsStatic( "&" ) },
   { bsStatic( "nbsp" ), bsStatic( " " ) },
   { bsStatic( "lt" ), bsStatic( "<" ) },
   { bsStatic( "gt" ), bsStatic( ">" ) },
   { bsStatic( "quot" ), bsStatic( "\"" ) },
   { bsStatic( "apos" ), bsStatic( "'" ) },
   { bsStatic( "cent" ), bsStatic( "¢" ) },
   { bsStatic( "pound" ), bsStatic( "£" ) },
   { bsStatic( "yen" ), bsStatic( "¥" ) },
   { bsStatic( "reg" ), bsStatic( "®" ) }
};

struct html_tree_tag* html_tree_up( struct html_tree* tree ) {
   struct html_tree_tag* tag_out = NULL;

   if( NULL == tree ) {
      fprintf( stderr, "HTML error: Attempted to move up on NULL tree.\n" );
      goto cleanup;
   }

   if( NULL != tree->current->parent ) {
      tree->current = tree->current->parent;
   } else {
      fprintf( stderr, "HTML error: Attempted to move up past root.\n" );
      tree->current = NULL;
   }
   tag_out = tree->current;

cleanup:
   return tag_out;
}

struct html_tree_tag* html_tree_new_tag(
   struct html_tree* tree, bstring tag_name
) {
   struct html_tree_tag* new_tag = NULL;
   struct html_tree_tag* new_root = NULL;

   /* Create the new tag. */
   new_tag = mem_alloc( 1, struct html_tree_tag );
   if( NULL == new_tag ) {
      goto cleanup;
   }

   if( NULL == tag_name ) {
      new_tag->tag = bfromcstr( "" );
   } else {
      new_tag->tag = bstrcpy( tag_name );
   }
   if( NULL == new_tag->tag ) {
      mem_free( new_tag );
      new_tag = NULL;
      goto cleanup;
   }

   //new_tag->data = bfromcstr( "" );

   if( NULL == tree->root ) {
      new_root = mem_alloc( 1, struct html_tree_tag );
      //new_root->tag = bfromcstr( "root" );
      //new_root->data = bfromcstr( "" );

      tree->root = new_root;
      tree->root->first_child = new_tag;
      new_tag->parent = new_root;
      tree->current = tree->root->first_child;
   }

   if( NULL == tree->current ) {
      tree->current = tree->root;
   }

   if(
      new_tag != tree->current &&
      NULL == tree->current->first_child
   ) {
      /* If the new tag is not the root, it must be a child... */
      tree->current->first_child = new_tag;
      new_tag->parent = tree->current;
      tree->current = tree->current->first_child;
   } else if(
      new_tag != tree->current &&
      NULL != tree->current->first_child
   ) {
      /* If it's not the first child, it must be a sibling of that child. */
      new_tag->parent = tree->current;
      tree->current = tree->current->first_child;
      while( NULL != tree->current->next_sibling ) {
         tree->current = tree->current->next_sibling;
      }
      tree->current->next_sibling = new_tag;
      tree->current = tree->current->next_sibling;
   }

cleanup:
   return new_tag;
}

struct html_tree_attr* html_tree_new_attr(
   struct html_tree_tag* tag, bstring label, bstring value
) {
   struct html_tree_attr* attr_iter = NULL;
   struct html_tree_attr* new_attr = NULL;

   if( NULL == tag ) {
      fprintf( stderr, "HTML error: Attempted new attrib on NULL tag.\n" );
      goto cleanup;
   }

   new_attr = mem_alloc( 1, struct html_tree_attr );
   if( NULL == new_attr ) {
      goto cleanup;
   }

   if( NULL == label ) {
      new_attr->label = bfromcstr( "" );
   } else {
      new_attr->label = bstrcpy( label );
   }
   if( NULL == new_attr->label ) {
      /* TODO: Error. */
      mem_free( new_attr );
      new_attr = NULL;
      goto cleanup;
   }

   if( NULL == value ) {
      new_attr->value = bfromcstr( "" );
   } else {
      new_attr->value = bstrcpy( value );
   }
   if( NULL == new_attr->value ) {
      /* TODO: Error. */
      bdestroy( new_attr->label );
      mem_free( new_attr );
      new_attr = NULL;
      goto cleanup;
   }

   if( NULL == tag->attrs ) {
      tag->attrs = new_attr;
   } else {
      attr_iter = tag->attrs;
      while( NULL != attr_iter->next ) {
         attr_iter = attr_iter->next;
      }
      attr_iter->next = new_attr;
   }

cleanup:
   return new_attr;
}

static void html_tree_append_char( unsigned char c, struct html_tree* tree ) {
   struct html_tree_attr* attr_current = NULL;
   int bstr_res = 0;

   switch( tree->state ) {
      case HTML_TREE_IN_START_TAG:
         bstr_res = bconchar( tree->current->tag, c );
         if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         }
         break;

      case HTML_TREE_IN_DATA:
         if(
            NULL == tree->current->data &&
            NULL == tree->current->first_child
         ) {
            /* This must be a text element. */
            tree->current->data = bfromcstr( "" );
         } else if(
            NULL == tree->current->data &&
            NULL != tree->current->first_child
         ) {
            /* Tag with children, so make a text element. */
            html_tree_new_tag( tree, NULL );
            tree->current->data = bfromcstr( "" );
         }
         bstr_res = bconchar( tree->current->data, c );
         if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         }
         break;

      case HTML_TREE_IN_ATTR_LABEL:
         attr_current = tree->current->attrs;
         /* The current attr is always last. */
         while( NULL != attr_current->next ) {
            attr_current = attr_current->next;
         }
         bstr_res = bconchar( attr_current->label, c );
         if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         }
         break;

      case HTML_TREE_IN_ATTR_VALUE:
         attr_current = tree->current->attrs;
         /* The current attr is always last. */
         while( NULL != attr_current->next ) {
            attr_current = attr_current->next;
         }
         bstr_res = bconchar( attr_current->value, c );
         if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         }
         break;

      case HTML_TREE_IN_ENTITY:
         attr_current = tree->current->attrs;
         /* The current attr is always last. */
         while(
            NULL != attr_current &&
            0 != bstricmp( &tag_name, attr_current->label )
         ) {
            attr_current = attr_current->next;
         }
         if( NULL == attr_current ) {
            attr_current =
               html_tree_new_attr( tree->current, &tag_name, NULL );
         }
         bstr_res = bconchar( attr_current->value, c );
         if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         }
         break;
   }

cleanup:
   return;
}

static void html_tree_parse_char( unsigned char c, struct html_tree* tree ) {
   int i;
   short tag_is_singleton_or_entity = 0;

   switch( c ) {
      case '<':
         if(
            HTML_TREE_OPENING_TAG == tree->state ||
            HTML_TREE_IN_START_TAG == tree->state ||
            HTML_TREE_IN_END_TAG == tree->state ||
            HTML_TREE_IN_ATTR_LABEL == tree->state ||
            /* TODO: Should we allow these in attr values? */
            HTML_TREE_IN_ATTR_VALUE == tree->state
         ) {
            /* TODO: Error: Double-open tag. */
         } else {
            if(
               HTML_TREE_IN_DATA == tree->state &&
               NULL != tree->current &&
               0 == blength( tree->current->tag )
            ) {
               html_tree_up( tree );
            }

            tree->state = HTML_TREE_OPENING_TAG;
         }
         break;

      case '>':
         for( i = 0 ; HTML_TREE_SINGLETON_COUNT > i ; i++ ) {
            if( 0 == bstricmp(
               tree->current->tag,
               &(html_tree_singleton_tags[i])
            ) ) {
               tag_is_singleton_or_entity = 1;
            }
         }

         if(
            HTML_TREE_IN_END_TAG == tree->state ||
            tag_is_singleton_or_entity
         ) {
            /* Strip off the end -- in comments. */
            if( 0 == bstricmp( tree->current->tag, tag_comment ) ) {
               btrunc(
                  tree->current->attrs->label,
                  blength( tree->current->attrs->label ) - 2
               );
            }

            if( 0 == blength( tree->current->tag ) ) {
               if( NULL == html_tree_up( tree ) ) {
                  goto cleanup;
               }
            }

            tree->state = HTML_TREE_IN_DATA;
         } else if( HTML_TREE_IN_START_TAG == tree->state ) {
            tree->state = HTML_TREE_IN_DATA;
         } else if( HTML_TREE_OPENING_TAG == tree->state ) {
            /* Weird empty tag. */
            tree->state = HTML_TREE_IN_DATA;
         }
         break;

      case '\0':
         break;

      case '&':
         if( NULL == html_tree_up( tree ) ) {
            goto cleanup;
         }
         html_tree_new_tag( tree, NULL );
         tree->last_state = tree->state;
         tree->state = HTML_TREE_IN_ENTITY;
         break;

      case ';':
         if( HTML_TREE_IN_ENTITY == tree->state ) {
            for( i = 0 ; HTML_TREE_ENTITY_COUNT > i ; i++ ) {
               if( 0 == bstricmp(
                  &(entities[i].name),
                  tree->current->attrs->value
               ) ) {
                  bassign( tree->current->data, &(entities[i].character) );
               }
            }
            if( NULL == html_tree_up( tree ) ) {
               goto cleanup;
            }
            html_tree_new_tag( tree, NULL );
            tree->state = tree->last_state;
         } else {
            goto plain_char;
         }
         /* TODO: Handle other states? */
         break;

      case '"':
         if( HTML_TREE_IN_ATTR_LABEL == tree->state ) {
            tree->state = HTML_TREE_IN_ATTR_VALUE;
         } else if( HTML_TREE_IN_ATTR_VALUE == tree->state ) {
            /* Only the start tag has attrs, anyway. */
            tree->state = HTML_TREE_IN_START_TAG;
         } else {
            goto plain_char;
         }
         break;

      case '=':
         /* Don't append the '=' to the attr label. */
         if( HTML_TREE_IN_ATTR_LABEL != tree->state ) {
            html_tree_append_char( c, tree );
         } else {
            goto plain_char;
         }
         break;

      case '/':
         if( HTML_TREE_OPENING_TAG == tree->state ) {
            tree->state = HTML_TREE_IN_END_TAG;
         } else if( HTML_TREE_IN_START_TAG == tree->state ) {
            tree->state = HTML_TREE_IN_END_TAG;
         } else {
            goto plain_char;
         }
         break;

      case '\n':
      case '\r':
         /* Always ignore newlines. */
         break;

      case '\t':
      case ' ':
         /* Limit spaces to one. */
         if(
            ' ' == tree->last_char ||
            '\t' == tree->last_char
         ) {
            break;
         }

         if( HTML_TREE_IN_START_TAG == tree->state ) {
            /* Don't add spaces to tag tags. */
            break;
         }

         /* Fall through to default. */

      case '?':
         if(
            '?' == c &&
            HTML_TREE_OPENING_TAG == tree->state
         ) {
            html_tree_new_tag( tree, NULL );
            tree->state = HTML_TREE_IN_START_TAG;
            break;
         } else if(
            '?' == c &&
            HTML_TREE_IN_START_TAG == tree->state
         ) {
            tree->state = HTML_TREE_IN_END_TAG;
            break;
         }

         /* Fall through to default. */

      default:
plain_char:
         if( HTML_TREE_OPENING_TAG == tree->state ) {
            /* The character isn't '/' or '>', so we're in a tag! */
            html_tree_new_tag( tree, NULL );
            tree->state = HTML_TREE_IN_START_TAG;
         } else if(
            ' ' == tree->last_char &&
            HTML_TREE_IN_START_TAG == tree->state
         ) {
            html_tree_new_attr( tree->current, NULL, NULL );
            tree->state = HTML_TREE_IN_ATTR_LABEL;
         } else if(
            HTML_TREE_IN_DATA == tree->state &&
            /* TODO: Crash if text before <html>? */
            ((NULL != tree->current &&
            0 < blength( tree->current->tag )) ||
            NULL == tree->current)
         ) {
            html_tree_new_tag( tree, NULL );
         }
         html_tree_append_char( c, tree );
         break;
   }

cleanup:
   tree->last_char = c;
}

void html_tree_cleanup( struct html_tree_tag* tag ) {
   struct html_tree_tag* tag_iter = NULL;
   struct html_tree_attr* attr_iter = NULL;
   struct html_tree_tag* tag_last = NULL;
   struct html_tree_tag* tag_parent = NULL;
   bstring str_test = NULL;
   int attr_count = 0;
   int bstr_res = 0;

   tag_iter = tag;
   while( NULL != tag_iter ) {
      attr_iter = tag_iter->attrs;
      attr_count = 0;
      while( NULL != attr_iter ) {
         attr_count++;
         attr_iter = attr_iter->next;
      }

      if( NULL != tag_iter->first_child ) {
         html_tree_cleanup( tag_iter->first_child );
      } else {
         /* This tag has no children. */
         str_test = bstrcpy( tag_iter->data );
         /* bstr_res = */ btrimws( str_test );
         /* if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         } */
         if( 0 == blength( str_test ) && 0 == blength( tag_iter->tag ) && 0 == attr_count ) {
            /* This tag has no data or attrs. */
            if( NULL != tag_last ) {
               tag_last->next_sibling = tag_iter->next_sibling;
               html_tree_free_tag( tag_iter );
               tag_iter = tag_last->next_sibling;
               continue;
            } else if( NULL != tag_iter->parent ) {
               /* No prior sibling. */
               tag_parent = tag_iter->parent;
               tag_parent->first_child = tag_iter->next_sibling;
               html_tree_free_tag( tag_iter );
               tag_iter = tag_parent->first_child;
               continue;
            }
         }
      }

      tag_last = tag_iter;
      tag_iter = tag_iter->next_sibling;
   }

cleanup:
   return;
}

int html_tree_parse_string( bstring html_string, struct html_tree* out ) {
   int i;

   if( NULL == out || NULL != out->root ) {
      fprintf( stderr, "Parse error: NULL or existing root.\n" );
      goto cleanup;
   }

   for( i = 0 ; blength( html_string ) > i ; i++ ) {
      html_tree_parse_char( bchar( html_string, i ), out );
   }

   html_tree_cleanup( out->root );

cleanup:
   return i;
}

void html_tree_free_attr( struct html_tree_attr* attr ) {
   bdestroy( attr->label );
   bdestroy( attr->value );
   free( attr );
}

void html_tree_free_tag( struct html_tree_tag* tag ) {
   struct html_tree_attr* attr_iter = NULL;
   struct html_tree_attr* attr_next = NULL;

   attr_iter = tag->attrs;
   while( NULL != attr_iter ) {
      attr_next = attr_iter->next;
      html_tree_free_attr( attr_iter );
      attr_iter = attr_next;
   }

   bdestroy( tag->data );
   bdestroy( tag->tag );

   free( tag );
}

bstring html_attr_to_bstr( const struct html_tree_attr* attr ) {
   size_t estimated_length = 3; /* ="" */
   bstring str_out = NULL;
   int bstr_res = 0;

   estimated_length += blength( attr->label );
   estimated_length += blength( attr->value );

   str_out = bfromcstralloc( estimated_length, " " );
   if( NULL == str_out ) {
      /* TODO */
      goto cleanup;
   }

   bstr_res = bconcat( str_out, attr->label );
   if( 0 != bstr_res ) {
      html_bstring_debug( bstr_res );
      goto cleanup;
   }

   /* Value is optional. */
   if( NULL != attr->value && 0 < blength( attr->value ) ) {
      bstr_res = bcatcstr( str_out, "=\"" );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }

      bstr_res = bconcat( str_out, attr->value );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }

      bstr_res = bcatcstr( str_out, "\"" );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }
   }

cleanup:
   return str_out;
}

size_t html_tag_length( const struct html_tree_tag* tag ) {
   size_t estimated_length = 0;
   struct html_tree_tag* child_iter = NULL;
   struct html_tree_attr* attr_iter = NULL;

   if( NULL != tag->tag ) {
      estimated_length += 3; /* < /> */
      estimated_length += blength( tag->tag );
   }

   attr_iter = tag->attrs;
   while( NULL != attr_iter ) {
      estimated_length += blength( attr_iter->label );

      estimated_length += 1; /* Space */

      if( NULL != attr_iter->value ) {
         estimated_length += 3; /* ="" */
         estimated_length += blength( attr_iter->value );
      }

      attr_iter = attr_iter->next;
   }

   if( NULL != tag->first_child ) {
      estimated_length += 3; /* <>\n, the closing / is handled above. */
      estimated_length += blength( tag->tag ); /* Closing tag. */

      /* Cycle through and count all child tags. */
      child_iter = tag->first_child;
      do {
         estimated_length += html_tag_length( child_iter );
         child_iter = child_iter->next_sibling;
      } while( NULL != child_iter );
   } else if( NULL != tag->data ) {
      estimated_length += blength( tag->data );
   }

   return estimated_length;
}

bstring html_tag_to_bstr( const struct html_tree_tag* tag ) {
   bstring str_out = NULL,
      str_tmp = NULL;
   size_t estimated_length = 0;
   struct html_tree_attr* attr_iter = NULL;
   struct html_tree_tag* child_iter = NULL;
   int bstr_res = 0;

   /* Allocate enough space for the final product. */
   estimated_length = html_tag_length( tag );
   str_out = bfromcstralloc( estimated_length, "" );
   if( NULL == str_out ) {
      goto cleanup;
   }

   if( NULL != tag->tag ) {
      bstr_res = bconchar( str_out, '<' );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }
      bstr_res = bconcat( str_out, tag->tag );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }

      attr_iter = tag->attrs;
      while( NULL != attr_iter ) {
         str_tmp = html_attr_to_bstr( attr_iter );
         if( NULL == str_tmp ) {
            /* TODO: Error. */
            goto cleanup;
         }
         bstr_res = bconcat( str_out, str_tmp );
         bdestroy( str_tmp );
         if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         }

         attr_iter = attr_iter->next;
      }
   }

   if( NULL != tag->tag && NULL == tag->data && NULL == tag->first_child ) {
      /* Singleton with no child data, close it. */
      if( '!' == bchar( tag->tag, 0 ) ) {
         /* Special cases, like "<!--" and "<!DOCYPE". */
         bstr_res = bcatcstr( str_out, ">\n" );
      } else {
         /* All other cases. */
         bstr_res = bcatcstr( str_out, " />" );
      }
      goto cleanup; /* We're done, no matter what. */

   } else if( NULL != tag->tag && NULL != tag->data) {
      /* Close start tag, append child text. */
      bstr_res = bconchar( str_out, '>' );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }

      bstr_res = bconcat( str_out, tag->data );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }

   } else {
      /* Close start tag, append children. */
      if( NULL != tag->tag ) {
         bstr_res = bcatcstr( str_out, ">\n" );
         if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         }
      }

      child_iter = tag->first_child;
      while( NULL != child_iter ) {
         str_tmp = html_tag_to_bstr( child_iter );
         bstr_res = bconcat( str_out, str_tmp );
         bdestroy( str_tmp );
         if( 0 != bstr_res ) {
            html_bstring_debug( bstr_res );
            goto cleanup;
         }

         child_iter = child_iter->next_sibling;
      }
   }

   if( NULL != tag->tag ) {
      /* There was inner content, so append an end tag. */
      bstr_res = bcatcstr( str_out, "</" );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }

      bstr_res = bconcat( str_out, tag->tag );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }

      bstr_res = bcatcstr( str_out, ">\n" );
      if( 0 != bstr_res ) {
         html_bstring_debug( bstr_res );
         goto cleanup;
      }
   }

cleanup:
   return str_out;
}

bstring html_tree_to_bstr( struct html_tree* tree ) {
   if( NULL != tree->root ) {
      return html_tag_to_bstr( tree->root );
   } else {
      return NULL;
   }
}

struct html_tree_tag* html_tree_get_root( struct html_tree* tree ) {
   if( NULL != tree ) {
      return tree->root;
   }
   return NULL;
}
