/*
======================================================================
tree.c

Functions for LW Panels TREE_CTLs.

Ernie Wright  15 Dec 00
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"


/*
======================================================================
free_tree()

Free a node tree.  node points to a root-level node array, and n is
the number of nodes in the array.  The children are freed by calling
free_tree() recursively.
====================================================================== */

void free_tree( Node *node, int n )
{
   int i;

   if ( node ) {
      for ( i = 0; i < n; i++ )
         free_tree( node[ i ].child, node[ i ].nchildren );
      free( node );
   }
}


/*
======================================================================
init_node()

Fill in the fields of a Node structure.  If the node has children, the
child array is allocated and each child's parent field initialized.
====================================================================== */

int init_node( Node *node, const char *name, void *id, int nchildren )
{
   Node *parent;
   int i;

   node->nchildren = nchildren;
   node->id = id;

   node->name = malloc( strlen( name ) + 1 );
   if ( !node->name ) return 0;
   strcpy( node->name, name );

   node->level = 0;
   parent = node->parent;
   while ( parent ) {
      ++node->level;
      parent = parent->parent;
   }

   if ( nchildren > 0 ) {
      node->child = calloc( nchildren, sizeof( Node ));
      if ( !node->child ) return 0;
      for ( i = 0; i < nchildren; i++ )
         node->child[ i ].parent = node;
   }

   return 1;
}


/*
======================================================================
tree_count()

TREE_CTL callback.  Returns the number of child nodes.
====================================================================== */

int tree_count( void *data, Node *node )
{
   if ( node )
      return node->nchildren;
   else
      return 1;
}


/*
======================================================================
tree_child()

TREE_CTL callback.  Returns a pointer to the i-th child.
====================================================================== */

Node *tree_child( Node *root, Node *node, int i )
{
   if ( node )
      return &node->child[ i ];
   else
      return root;
}


/*
======================================================================
tree_info()

TREE_CTL callback.  Sets or gets flags for the node.
====================================================================== */

char *tree_info( void *data, Node *node, int *flags )
{
   if ( *flags )
      node->flags = *flags;
   else
      *flags = node->flags;

   return node->name;
}


/*
======================================================================
tree_event()

TREE_CTL callback.  Called when a node is selected.  This is just a
placeholder, used when selection events don't need to cause anything
else to happen.
====================================================================== */

void tree_event( LWControl *ctl, void *data )
{
   LWValue ival = { LWT_INTEGER };
   Node *sel;

   ctl->get( ctl, CTL_VALUE, &ival );
   sel = ( Node * ) ival.ptr.ptr;
}
