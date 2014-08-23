/*
======================================================================
tree.h

Typedefs and prototypes for using tree.c.

Ernie Wright  15 Dec 00
====================================================================== */

#ifndef TREE_H
#define TREE_H

#ifndef LWPANEL_H
#include <lwpanel.h>
#endif

typedef struct st_Node {
   char           *name;
   void           *id;
   int             level;
   struct st_Node *parent;       /* pointer to the parent */
   struct st_Node *child;        /* an array of children */
   int             nchildren;
   int             flags;
} Node;

void  free_tree( Node *node, int n );
int   init_node( Node *node, const char *name, void *id, int nchildren );
int   tree_count( void *data, Node *node );
Node *tree_child( Node *root, Node *node, int i );
char *tree_info( void *data, Node *node, int *flags );
void  tree_event( LWControl *ctl, void *data );

#endif