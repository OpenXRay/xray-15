/*
======================================================================
general.c

Ernie Wright  6 Jan 00

This module, part of the ModLib library, contains functions that
affect or operate on the general Modeler environment.
====================================================================== */

#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int csNew( int clear )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_INTEGER;
   argv.intv.value = clear;

   LOOKUP( "NEW" );
   EXECUTE( 1, &argv );
   return OK;
}


int csUndo( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "UNDO" );
   EXECUTE( 0, NULL );
   return OK;
}


int csRedo( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "REDO" );
   EXECUTE( 0, NULL );
   return OK;
}


int csDelete( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "DELETE" );
   EXECUTE( 0, NULL );
   return OK;
}


int csCut( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "CUT" );
   EXECUTE( 0, NULL );
   return OK;
}


int csCopy( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "COPY" );
   EXECUTE( 0, NULL );
   return OK;
}


int csPaste( void )
{
   static LWCommandCode ccode;
   ModData *md = getModData();

   assert( md->edit == NULL );

   LOOKUP( "PASTE" );
   EXECUTE( 0, NULL );
   return OK;
}


const char *mgGetLWDirectory( char *title )
{
   ModData *md = getModData();

   if ( !md->dirinfo ) return NULL;
   return md->dirinfo( title );
}


int mgFileLoadReq( char *caption, char *node, char *path, char *name, int len )
{
   ModData *md = getModData();
   char *tnode;
   int ok;

   tnode = malloc( len + 1 );
   if ( !tnode ) return 0;

   tnode[ 0 ] = '<';
   strcpy( &tnode[ 1 ], node );

   ok = md->filereq( caption, tnode, path, name, len );
   strcpy( node, tnode );
   free( tnode );

   return ok;
}


int mgFileSaveReq( char *caption, char *node, char *path, char *name, int len )
{
   ModData *md = getModData();
   char *tnode;
   int ok;

   tnode = malloc( len + 1 );
   if ( !tnode ) return 0;

   tnode[ 0 ] = '>';
   strcpy( &tnode[ 1 ], node );

   ok = md->filereq( caption, tnode, path, name, len );
   strcpy( node, tnode );
   free( tnode );

   return ok;
}


int csLoad( char *filename )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_STRING;
   argv.str.buf = filename;
   argv.str.bufLen = 0;

   LOOKUP( "LOAD" );
   EXECUTE( 1, &argv );
   return OK;
}


int csSave( char *filename )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_STRING;
   argv.str.buf = filename;
   argv.str.bufLen = 0;

   LOOKUP( "SAVE" );
   EXECUTE( 1, &argv );
   return OK;
}


unsigned int mgGetLayerMask( EltOpLayer oplyr )
{
   ModData *md = getModData();
   return md->query->layerMask( oplyr );
}


int mgGetLayerCount( void )
{
   ModData *md = getModData();
   return md->query->numLayers();
}


const char *mgGetLayerList( EltOpLayer oplyr, const char *obj )
{
   ModData *md = getModData();
   return md->query->layerList( oplyr, obj );
}


int csSetLayer( char *layer )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_STRING;
   argv.str.buf = layer;
   argv.str.bufLen = 0;

   LOOKUP( "SETLAYER" );
   EXECUTE( 1, &argv );
   return OK;
}


int csSetALayer( char *layer )
{
   return csSetLayer( layer );
}


int csSetBLayer( char *layer )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_STRING;
   argv.str.buf = layer;
   argv.str.bufLen = 0;

   LOOKUP( "SETBLAYER" );
   EXECUTE( 1, &argv );
   return OK;
}


const char *mgGetCurrentObject( void )
{
   ModData *md = getModData();
   return md->query->object();
}


unsigned int mgGetBoundingBox( EltOpLayer oplyr, double *box )
{
   ModData *md = getModData();
   return md->query->bbox( oplyr, box );
}


void mgMessage( int severity, char *line1, char *line2 )
{
   ModData *md = getModData();

   switch ( severity ) {
      case 0:
         md->message->info( line1, line2 );
         break;
      case 1:
         md->message->warning( line1, line2 );
         break;
      case 2:
         md->message->error( line1, line2 );
         break;
      default:
         break;
   }
}


void mgMonitorBegin( char *title, char *caption, int total )
{
   ModData *md = getModData();

   if ( md->mon ) return;
   if ( md->mon = md->monf->create( title, caption ))
      md->mon->init( md->mon->data, total );
}


int mgMonitorStep( int step )
{
   ModData *md = getModData();

   if ( !md->mon ) return 0;
   return md->mon->step( md->mon->data, step );
}


void mgMonitorDone( void )
{
   ModData *md = getModData();

   if ( !md->mon ) return;
   md->mon->done( md->mon->data );
   md->monf->destroy( md->mon );
   md->mon = NULL;
}


int csCmdSeq( char *name, char *arg )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 2 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = name;
   argv[ 0 ].str.bufLen = 0;

   if ( arg ) {
      argv[ 1 ].type = DY_STRING;
      argv[ 1 ].str.buf = arg;
      argv[ 1 ].str.bufLen = 0;
   }
   else argv[ 1 ].type = DY_NULL;

   LOOKUP( "CMDSEQ" );
   EXECUTE( 2, argv );
   return OK;
}


int csPlugin( char *module, char *class, char *name, char *username )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv[ 4 ];

   assert( md->edit == NULL );

   argv[ 0 ].type = DY_STRING;
   argv[ 0 ].str.buf = module;
   argv[ 0 ].str.bufLen = 0;

   if ( class ) {
      argv[ 1 ].type = DY_STRING;
      argv[ 1 ].str.buf = class;
      argv[ 1 ].str.bufLen = 0;
   }
   else argv[ 1 ].type = DY_NULL;

   if ( name ) {
      argv[ 2 ].type = DY_STRING;
      argv[ 2 ].str.buf = name;
      argv[ 2 ].str.bufLen = 0;
   }
   else argv[ 2 ].type = DY_NULL;

   if ( username ) {
      argv[ 3 ].type = DY_STRING;
      argv[ 3 ].str.buf = username;
      argv[ 3 ].str.bufLen = 0;
   }
   else argv[ 3 ].type = DY_NULL;

   LOOKUP( "PLUGIN" );
   EXECUTE( 4, argv );
   return OK;
}
