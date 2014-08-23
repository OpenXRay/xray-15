/*
======================================================================
surface.c

Ernie Wright  23 Jan 00

This module, part of the ModLib library, collects together the surface
management functions and commands.
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CS_MACROS
#include "lwmodlib.h"


int csSetDefaultSurface( const char *surfname )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_SURFACE;
   argv.str.buf = surfname;
   argv.str.bufLen = 0;

   LOOKUP( "SURFACE" );
   EXECUTE( 1, &argv );
   return OK;
}


const char *mgGetDefaultSurface( void )
{
   ModData *md = getModData();
   return md->query->surface();
}


int csChangeSurface( const char *surface )
{
   static LWCommandCode ccode;
   ModData *md = getModData();
   DynaValue argv;

   assert( md->edit == NULL );

   argv.type = DY_STRING;
   argv.str.buf = surface;
   argv.str.bufLen = 0;

   LOOKUP( "CHANGESURFACE" );
   EXECUTE( 1, &argv );
   return OK;
}


LWSurfaceID mgGetFirstSurface( void )
{
   ModData *md = getModData();
   return md->surface->first();
}


LWSurfaceID mgGetNextSurface( LWSurfaceID sid )
{
   ModData *md = getModData();
   return md->surface->next( sid );
}


LWSurfaceID *mgGetSurfacesByName( const char *name, const char *objname )
{
   ModData *md = getModData();
   return md->surface->byName( name, objname );
}


LWSurfaceID *mgGetSurfacesByObject( const char *name )
{
   ModData *md = getModData();
   return md->surface->byObject( name );
}


const char *mgGetSurfaceName( LWSurfaceID sid )
{
   ModData *md = getModData();
   return md->surface->name( sid );
}


const char *mgGetSurfaceObject( LWSurfaceID sid )
{
   ModData *md = getModData();
   return md->surface->sceneObject( sid );
}


LWSurfaceID mgCreateSurface( const char *objname, const char *surfname )
{
   ModData *md = getModData();
   return md->surface->create( objname, surfname );
}


int csSetSurfEdSurface( const char *surfname, const char *objname )
{
   ModData *md = getModData();
   char cmd[ 256 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_SetSurf \"%s\" \"%s\"", surfname, objname );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}


int csRenameSurface( const char *newname )
{
   ModData *md = getModData();
   char cmd[ 128 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_Rename \"%s\"", newname );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}


int mgGetSurfaceInt( LWSurfaceID sid, const char *ch )
{
   ModData *md = getModData();
   return md->surface->getInt( sid, ch );
}


int csSetSurfaceInt( const char *ch, int val )
{
   ModData *md = getModData();
   char cmd[ 128 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_SetInt %s %d", ch, val );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}


double mgGetSurfaceFlt( LWSurfaceID sid, const char *ch )
{
   ModData *md = getModData();
   double *flt;

   flt = md->surface->getFlt( sid, ch );
   return *flt;
}


int csSetSurfaceFlt( const char *ch, double val )
{
   ModData *md = getModData();
   char cmd[ 128 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_SetFloat %s %g", ch, val );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}


void mgGetSurfaceColor( LWSurfaceID sid, const char *ch, double color[ 3 ] )
{
   ModData *md = getModData();
   double *flt;

   flt = md->surface->getFlt( sid, ch );
   color[ 0 ] = flt[ 0 ];
   color[ 1 ] = flt[ 1 ];
   color[ 2 ] = flt[ 2 ];
}


int csSetSurfaceColor( const char *ch, double color[ 3 ] )
{
   ModData *md = getModData();
   char cmd[ 128 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_SetColor %s %g %g %g", ch,
      color[ 0 ], color[ 1 ], color[ 2 ] );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}


LWEnvelopeID mgGetSurfaceEnvelope( LWSurfaceID sid, const char *ch )
{
   ModData *md = getModData();
   return md->surface->getEnv( sid, ch );
}


LWTextureID mgGetSurfaceTexture( LWSurfaceID sid, const char *ch )
{
   ModData *md = getModData();
   return md->surface->getTex( sid, ch );
}


LWImageID mgGetSurfaceImage( LWSurfaceID sid, const char *ch )
{
   ModData *md = getModData();
   return md->surface->getImg( sid, ch );
}


int csAddSurfaceShader( const char *shader )
{
   ModData *md = getModData();
   char cmd[ 128 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_AddShader %s", shader );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}


int csRemSurfaceShader( const char *shader )
{
   ModData *md = getModData();
   char cmd[ 128 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_RemShader %s", shader );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}


int csCopySurface( LWSurfaceID to, LWSurfaceID from )
{
   ModData *md = getModData();
   char cmd[ 256 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_Copy %x %x", to, from );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}


int csOpenSurfEdWindow( void )
{
   ModData *md = getModData();

   assert( md->edit == NULL );

   md->cmderror = md->local->evaluate( md->local->data, "Surf_OpenWindow" );
   return !md->cmderror;
}


int csCloseSurfEdWindow( void )
{
   ModData *md = getModData();

   assert( md->edit == NULL );

   md->cmderror = md->local->evaluate( md->local->data, "Surf_CloseWindow" );
   return !md->cmderror;
}


int csSetSurfEdWindowPos( int x, int y )
{
   ModData *md = getModData();
   char cmd[ 80 ];

   assert( md->edit == NULL );

   sprintf( cmd, "Surf_SetWindowPos %d %d", x, y );
   md->cmderror = md->local->evaluate( md->local->data, cmd );
   return !md->cmderror;
}
