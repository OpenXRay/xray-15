/*
======================================================================
me.c

Ernie Wright  7 Jan 00

This module, part of the ModLib library, collects together the mesh
editing functions.  These can only be called after calling meInit()
(if you're a MeshDataEdit plug-in) or csMeshBegin() (if you're a
CommandSequence plug-in).  Many of these functions can't be called
during an OPSEL_MODIFY mesh edit.
====================================================================== */

#include <stdlib.h>
#include <assert.h>
#include "lwmodlib.h"


int mePointCount( EltOpLayer layers, int mode )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->pointCount( md->edit->state, layers, mode );
}


int mePolyCount( EltOpLayer layers, int mode )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->polyCount( md->edit->state, layers, mode );
}


int mePointScan( EDPointScanFunc *callback, void *userdata, EltOpLayer layers )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->pointScan( md->edit->state, callback, userdata, layers );
}


EDError mePolyScan( EDPolyScanFunc *callback, void *userdata, EltOpLayer layers )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->polyScan( md->edit->state, callback, userdata, layers );
}


EDPointInfo *mePointInfo( LWPntID id )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->pointInfo( md->edit->state, id );
}


EDPolygonInfo *mePolyInfo( LWPolID id )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->polyInfo( md->edit->state, id );
}


int mePolyNormal( LWPolID id, double norm[ 3 ] )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->polyNormal( md->edit->state, id, norm );
}


LWPntID meAddPoint( double *coord )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->addPoint( md->edit->state, coord );
}


LWPntID meAddPointInterp( double *coord, int npoints, LWPntID *ptarray, double *weight )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->addIPnt( md->edit->state, coord, npoints, ptarray, weight );
}


LWPolID meAddPoly( LWID type, LWPolID proto, char *surfname, int npoints,
   LWPntID *ptarray )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->addPoly( md->edit->state, type, proto, surfname, npoints,
      ptarray );
}


LWPolID meAddCurve( char *surfname, int npoints, LWPntID *ptarray, int flags )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->addCurve( md->edit->state, surfname, npoints, ptarray, flags );
}


EDError meAddQuad( LWPntID pt1, LWPntID pt2, LWPntID pt3, LWPntID pt4 )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->addQuad( md->edit->state, pt1, pt2, pt3, pt4 );
}


EDError meAddTri( LWPntID pt1, LWPntID pt2, LWPntID pt3 )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->addTri( md->edit->state, pt1, pt2, pt3 );
}


EDError meAddPatch( int nr, int nc, int lr, int lc, EDBoundCv *r0,
   EDBoundCv *r1, EDBoundCv *c0, EDBoundCv *c1 )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->addPatch( md->edit->state, nr, nc, lr, lc, r0, r1, c0, c1 );
}


EDError meRemovePoint( LWPntID id )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->remPoint( md->edit->state, id );
}


EDError meRemovePoly( LWPolID id )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->remPoly( md->edit->state, id );
}


EDError meMovePoint( LWPntID id, double *coord )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->pntMove( md->edit->state, id, coord );
}


EDError meSetPolySurface( LWPolID id, char *surfname )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->polSurf( md->edit->state, id, surfname );
}


EDError meSetPolyPoints( LWPolID id, int npoints, LWPntID *ptarray )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->polPnts( md->edit->state, id, npoints, ptarray );
}


EDError meSetPolyFlags( LWPolID id, int mask, int value )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->polFlag( md->edit->state, id, mask, value );
}


EDError meSetPolyTag( LWPolID id, LWID tagid, const char *tag )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->polTag( md->edit->state, id, tagid, tag );
}


const char *meGetPolyTag( LWPolID id, LWID tagid )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->polyTag( md->edit->state, id, tagid );
}


EDError meSetPointVMap( LWPntID id, LWID vmapid, const char *vmapname,
   int nval, float *val )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->pntVMap( md->edit->state, id, vmapid, vmapname, nval, val );
}


void *meSelectPointVMap( void *data, LWID vmapid, const char *vmapname )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->pointVSet( md->edit->state, data, vmapid, vmapname );
}


int meGetPointVMap( LWPntID id, float *vmap )
{
   ModData *md = getModData();

   assert( md->edit != NULL );
   return md->edit->pointVGet( md->edit->state, id, vmap );
}


EDError meInitUV( float uv[ 2 ] )
{
   ModData *md = getModData();

   assert(( md->edit != NULL ) && !( md->opsel & OPSEL_MODIFY ));
   return md->edit->initUV( md->edit->state, uv );
}
