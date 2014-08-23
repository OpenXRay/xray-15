/*
======================================================================
objseq.c

A simple object replacement demo.

Ernie Wright  10 Apr 00

At frame n, 0 < n <= count, replace the object with file[ n - 1 ].
The user picks the object files from a MULTILOAD file requester, and
we sort the filenames in strcmp() order.
====================================================================== */

#include <lwserver.h>
#include <lwobjrep.h>
#include <lwhost.h>
#include <stdlib.h>
#include <string.h>


/* our instance data */

typedef struct {
   int count;
   char **files;
} RepDat;


/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   return calloc( 1, sizeof( RepDat ));
}


/*
======================================================================
Destroy()

Handler callback.  Free resources allocated by Create().
====================================================================== */

XCALL_( static void )
Destroy( RepDat *dat )
{
   int i;

   if( dat ) {
      if ( dat->files ) {
         for ( i = 0; i < dat->count; i++ )
            if ( dat->files[ i ] )
               free( dat->files[ i ] );
         free( dat->files );
      }
      free( dat );
   }
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.  If your instance data contains
allocated resources, note that a simple *to = *from is insufficient.
====================================================================== */

XCALL_( static LWError )
Copy( RepDat *to, RepDat *from )
{
   int i;

   to->count = from->count;
   for ( i = 0; i < from->count; i++ ) {
      to->files[ i ] = malloc( strlen( from->files[ i ] ) + 1 );
      if ( !to->files[ i ] )
         return "Couldn't allocate instance data.";
      strcpy( to->files[ i ], from->files[ i ] );
   }
   return NULL;
}


/*
======================================================================
Load()

Handler callback.  Read instance data.
====================================================================== */

XCALL_( static LWError )
Load( RepDat *dat, const LWLoadState *ls )
{
   char buf[ 260 ];
   int i;

   LWLOAD_I4( ls, &dat->count, 1 );
   if ( dat->count > 0 ) {
      dat->files = calloc( dat->count, sizeof( char * ));
      if ( dat->files ) {
         for ( i = 0; i < dat->count; i++ ) {
            LWLOAD_STR( ls, buf, 260 );
            dat->files[ i ] = malloc( strlen( buf ) + 1 );
            strcpy( dat->files[ i ], buf );
         }
      }
   }

   return NULL;
}


/*
======================================================================
Save()

Handler callback.  Write instance data.
====================================================================== */

XCALL_( static LWError )
Save( RepDat *dat, const LWSaveState *ss )
{
   int i;

   LWSAVE_I4( ss, &dat->count, 1 );
   for ( i = 0; i < dat->count; i++ )
      LWSAVE_STR( ss, dat->files[ i ] );
   return NULL;
}


/*
======================================================================
Evaluate()

Handler callback.  For frames 1 to dat->count, we replace the current
file with files[ frame - 1 ].
====================================================================== */

XCALL_( static void )
Evaluate( RepDat *dat, LWObjReplacementAccess *access )
{
   if (( access->newFrame != access->curFrame ) &&
       ( access->newFrame > 0 ) &&
       ( access->newFrame <= dat->count ))
      access->newFilename = dat->files[ access->newFrame - 1 ];
   else
      access->newFilename = NULL;
}


/*
======================================================================
Handler()

Handler activation function.  Check the version and fill in the
callback fields of the handler structure.
====================================================================== */

XCALL_( static int )
Handler( long version, GlobalFunc *global, LWObjReplacementHandler *local,
   void *serverData)
{
   if ( version != LWOBJREPLACEMENT_VERSION )
      return AFUNC_BADVERSION;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->copy    = Copy;
   local->evaluate      = Evaluate;

   return AFUNC_OK;
}


/* from files.c */

char **get_filelist( LWFileActivateFunc *filereq, char *title, char *filetype,
   char *name, char *path, char *node, int bufsize, int *n );

static LWFileActivateFunc *filereq;


/*
======================================================================
Options()

Interface callback.  Display a MULTILOAD file dialog and sort the
array of filenames selected by the user.
====================================================================== */

XCALL_( static LWError )
Options( RepDat *dat )
{
   static char name[ 260 ], path[ 260 ], node[ 260 ];

   dat->files = get_filelist( filereq, "Select Objects", "Objects",
      name, path, node, 260, &dat->count );

   if ( dat->files )
      qsort( dat->files, dat->count, sizeof( char * ), strcmp );

   return NULL;
}


/*
======================================================================
Interface()

Interface activation function.
====================================================================== */

XCALL_( static int )
Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   filereq = global( LWFILEACTIVATEFUNC_GLOBAL, GFUSE_TRANSIENT );
   if ( !filereq ) return AFUNC_BADGLOBAL;

   local->panel = NULL;
   local->options = Options;
   local->command = NULL;

   return AFUNC_OK;
}


/*
======================================================================
Server description.  The convention is to stick this at the end of the
source file containing the activation functions.
====================================================================== */

ServerRecord ServerDesc[] = {
   { LWOBJREPLACEMENT_HCLASS, "Demo_ObjSequence", Handler },
   { LWOBJREPLACEMENT_ICLASS, "Demo_ObjSequence", Interface },
   { NULL }
};
