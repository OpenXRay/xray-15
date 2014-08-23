/*
======================================================================
files.c

Functions for a MULTILOAD file request.

Ernie Wright  10 Apr 00
====================================================================== */

#include <lwserver.h>
#include <lwhost.h>
#include <stdlib.h>
#include <string.h>


typedef struct st_FNode {
   char            *name;
   struct st_FNode *next;
} FNode;


LWFileReqLocal frloc;
FNode *head = NULL, *tail;


static int list_add( char *name )
{
   FNode *p;

   p = malloc( sizeof( FNode ));
   if ( !p ) return 0;

   p->name = malloc( strlen( name ) + 1 );
   if ( !p->name ) { free( p ); return 0; }
   strcpy( p->name, name );
   p->next = NULL;

   if ( !head )
      head = tail = p;
   else {
      tail->next = p;
      tail = p;
   }

   return 1;
}


static int list_count( void )
{
   FNode *p = head;
   int i;

   for ( i = 0; p; i++ ) p = p->next;
   return i;
}


static void list_free( void )
{
   FNode *p = head, *q;

   while ( p ) {
      q = p->next;
      free( p );
      p = q;
   }
}


static char **list_array( int *n )
{
   char **array;
   FNode *p = head;
   int i;

   *n = list_count();
   if ( *n < 1 ) return NULL;

   if ( array = calloc( *n, sizeof( char * )))
      for ( i = 0; i < *n; i++ ) {
         array[ i ] = p->name;
         p = p->next;
      }

   list_free();
   return array;
}


/*
======================================================================
pickname()

MULTILOAD filename callback.  The file dialog server calls this for
each selected filename.  It returns 0 if everything's okay and any
non-zero number to tell the dialog to stop sending names.
====================================================================== */

static int pickname( void )
{
   if ( !list_add( frloc.fullName ))
      return -1;
   return 0;
}


/*
======================================================================
get_filelist()

Display a MULTILOAD file dialog.  Returns an array of names and sets
the name count if successful.  If the user cancels the dialog or an
error occurs, returns NULL.
====================================================================== */

char **get_filelist( LWFileActivateFunc *filereq, char *title, char *filetype,
   char *name, char *path, char *node, int bufsize, int *count )
{
   char **a = NULL;
   int result;

   frloc.reqType  = FREQ_MULTILOAD;
   frloc.title    = title;
   frloc.fileType = filetype;
   frloc.path     = path;
   frloc.baseName = node;
   frloc.fullName = name;
   frloc.bufLen   = bufsize;
   frloc.pickName = pickname;

   result = filereq( LWFILEREQ_VERSION, &frloc );

   if ( result == AFUNC_OK && frloc.result )
      a = list_array( count );

   return a;
}
