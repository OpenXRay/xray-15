/*
======================================================================
tree.c

Test the LWPanels tree control.

Ernie Wright  28 Jun 00
====================================================================== */

#include <lwserver.h>
#include <lwpanel.h>
#include <stdio.h>


/* The tree control doesn't enforce a specific data structure for the
   tree.  But we do need a structure that allows us to answer certain
   questions about the data. */

typedef struct st_Node {
   char           *name;
   struct st_Node *child[ 5 ];
   int             nchildren;
   int             flags;
} Node;


static int count( void *data, Node *node )
{
   if ( node )
      return node->nchildren;
   else
      return 1;
}


static Node *child( Node *root, Node *node, int i )
{
   if ( node )
      return node->child[ i ];
   else
      return root;
}


static char *info( void *data, Node *node, int *flags )
{
   if ( *flags )
      node->flags = *flags;
   else
      *flags = node->flags;

   return node->name;
}


static void event( LWControl *ectl, void *edata )
{
   LWValue ival = { LWT_INTEGER };
   Node *sel;

   ectl->get( ectl, CTL_VALUE, &ival );
   sel = ( Node * ) ival.intv.value;
}


static Node root;
static Node apple, orange, banana;
static Node mac, delicious, granny;
static Node navel;


static void build_tree( void )
{
   root.name           = "Fruit";
   root.child[ 0 ]     = &apple;
   root.child[ 1 ]     = &orange;
   root.child[ 2 ]     = &banana;
   root.nchildren      = 3;
   root.flags          = 0;

   apple.name          = "Apple";
   apple.child[ 0 ]    = &mac;
   apple.child[ 1 ]    = &delicious;
   apple.child[ 2 ]    = &granny;
   apple.nchildren     = 3;
   apple.flags         = 0;

   orange.name         = "Orange";
   orange.child[ 0 ]   = &navel;
   orange.nchildren    = 1;
   orange.flags        = 0;

   banana.name         = "Banana";
   banana.nchildren    = 0;

   mac.name            = "Macintosh";
   mac.nchildren       = 0;
   mac.flags           = 0;

   delicious.name      = "Red Delicious";
   delicious.nchildren = 0;
   delicious.flags     = 0;

   granny.name         = "Granny Smith";
   granny.nchildren    = 0;
   granny.flags        = 0;

   navel.name          = "Navel";
   navel.nchildren     = 0;
   navel.flags         = 0;
}


int open_treepan( LWPanelFuncs *panf )
{
   LWPanControlDesc desc;
   LWValue ival = { LWT_INTEGER };
   LWPanelID panel;
   LWControl *ctl;

   build_tree();

   if( !( panel = PAN_CREATE( panf, "Tree" )))
      return 0;

   ctl = TREE_CTL( panf, panel, "Food", 150, 200, info, count, child );
   CON_SETEVENT( ctl, event, &root );

   panf->open( panel, PANF_BLOCKING );
   PAN_KILL( panf, panel );

   return 1;
}
