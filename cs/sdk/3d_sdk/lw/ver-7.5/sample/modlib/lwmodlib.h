/*
======================================================================
lwmodlib.h

Definitions and prototypes for the Modeler SDK library.

Ernie Wright  7 Jan 00
====================================================================== */

#ifndef LWMODLIB_H
#define LWMODLIB_H

#include <lwserver.h>
#include <lwcmdseq.h>
#include <lwmodeler.h>
#include <lwhost.h>
#include <lwdisplay.h>
#include <lwsurf.h>


typedef struct st_ModData {
   GlobalFunc         *global;
   LWModCommand       *local;
   MeshEditOp         *edit;
   DynaValue           result;
   int                 cmderror;
   EltOpSelect         opsel;
   float               version;
   int                 build;
   int                 product;
   unsigned long       serialno;
   unsigned long       locale;
   DynaMonitorFuncs   *monf;
   LWMonitor          *mon;
   LWStateQueryFuncs  *query;
   LWSurfaceFuncs     *surface;
   LWFontListFuncs    *font;
   HostDisplayInfo    *hdi;
   LWMessageFuncs     *message;
   LWFileReqFunc      *filereq;
   LWDirInfoFunc      *dirinfo;
} ModData;


#ifdef CS_MACROS

#define LOOKUP(cmd) if (!ccode) ccode = md->local->lookup(md->local->data, cmd)
#define EXECUTE(ac,av) \
   md->cmderror = md->local->execute(md->local->data, ccode, ac, av, \
      md->opsel, &md->result)
#define OK (md->cmderror == CSERR_NONE)

#endif


/* Init */

ModData *csInit( GlobalFunc *global, LWModCommand *local );
void csDone( void );

int csMeshBegin( int pntbufsize, int polbufsize, EltOpSelect select );
void csMeshDone( EDError ederror, int selm );

ModData *meInit( GlobalFunc *global, MeshEditBegin *local, int pntbufsize,
   int polbufsize, EltOpSelect select );
void meDone( EDError ederror, int selm );

ModData *getModData( void );

/* General */

int csNew( int clear );
int csUndo( void );
int csRedo( void );
int csDelete( void );
int csCut( void );
int csCopy( void );
int csPaste( void );

const char *mgGetLWDirectory( char *title );
int mgFileLoadReq( char *caption, char *node, char *path, char *name, int len );
int mgFileSaveReq( char *caption, char *node, char *path, char *name, int len );
int csLoad( char *filename );
int csSave( char *filename );

unsigned int mgGetLayerMask( EltOpLayer oplyr );
int mgGetLayerCount( void );
const char *mgGetLayerList( EltOpLayer oplyr, const char *str );
int csSetLayer( char *layer );
int csSetALayer( char *layer );
int csSetBLayer( char *layer );

unsigned int mgGetBoundingBox( EltOpLayer oplyr, double *box );
const char *mgGetCurrentObject( void );

void mgMonitorBegin( char *title, char *caption, int total );
int mgMonitorStep( int step );
void mgMonitorDone( void );

void mgMessage( int severity, char *line1, char *line2 );

int csCmdSeq( char *name, char *arg );
int csPlugin( char *module, char *class, char *name, char *username );

/* Selection */

int mePointSelect( LWPntID id, int select );
int csSelPoint_ID( EDPointScanFunc *callback, void *userdata, EltOpLayer layers );
int csSelPoint_Volume( char *action, double *lo, double *hi );
int csSelPoint_Connect( char *action );
int csSelPoint_Stat( char *action, char *stat, int npol );
int mePolygonSelect( LWPolID id, int select );
int csSelPolygon_ID( EDPolyScanFunc *callback, void *userdata, EltOpLayer layers );
int csSelPolygon_Volume( char *action, char *criterion, double *lo, double *hi );
int csSelPolygon_Connect( char *action );
int csSelPolygon_Stat( char *action, char *stat, int nvert );
int csSelPolygon_Surface( char *action, char *surf );
int csSelPolygon_Faces( char *action );
int csSelPolygon_Curves( char *action );
int csSelPolygon_Nonplanar( char *action, double limit );
int csSelInvert( void );
int csSelHide( char *state );
int csSelUnhide( void );
int csInvertHide( void );

/* Make */

int csMakeBall( double *radius, int nsides, int nsegments, double *center );
int csMakeDisc( double *radius, double top, double bottom, char *axis,
   int nsides, int nsegments, double *center );
int csMakeCone( double *radius, double top, double bottom, char *axis,
   int nsides, int nsegments, double *center );
int csMakeBox( double *lowcorner, double *highcorner, int *nsegments );
int csMakeTesBall( double *radius, int level, double *center );

/* Point Commands */

int csFixedFlex( char *axis, double start, double end, char *ease );
int csAutoFlex( char *axis, char *direction, char *ease );
int csDeformRegion( double *radius, double *center, char *axis );
int csMove( double *offset );
int csShear( double *offset );
int csMagnet( double *offset );
int csRotate( double angle, char *axis, double *center );
int csTwist( double angle, char *axis, double *center );
int csVortex( double angle, char *axis, double *center );
int csScale( double *factor, double *center );
int csTaper( double *factor, double *center );
int csPole( double *factor, double *center );
int csBend( double angle, double direction, double *center );
int csJitter( double *radius, char *type, double *center );
int csSmooth( int iterations, double strength );
int csQuantize( double *size );
int csMergePoints( double d );
int csWeldPoints( void );
int csWeldAverage( void );

/* Polygon Commands */

int csFlip( void );
int csTriple( void );
int csFreezeCurves( void );
int csAlignPols( void );
int csRemovePols( void );
int csUnifyPols( void );
int csChangePart( char *part );
int csSubdivide( char *mode, double maxangle );
int csFracSubdivide( char *mode, double fractal, double maxangle );
int csMorphPols( int nsegments );
int csMergePols( void );
int csSplitPols( void );
int csSkinPols( void );
int csSmoothCurves( void );
int csToggleCCStart( void );
int csToggleCCEnd( void );
int csTogglePatches( void );
int csMake4Patch( double perpendicular, double parallel );

/* Sweeps */

int csLathe( char *axis, int nsides, double *center, double endangle,
   double startangle, double offset );
int csExtrude( char *axis, double extent, int nsegments );
int csMirror( char *axis, double plane );
int csPathClone( char *filename, double step, double start, double end );
int csPathExtrude( char *filename, double step, double start, double end );
int csRailClone( int nsegments, char *divs, char *flags, double strength );
int csRailExtrude( int nsegments, char *divs, char *flags, double strength );

/* Special Tools */

int csAxisDrill( char *operation, char *axis, char *surface );
int csSolidDrill( char *operation, char *surface );
int csBoolean( char *operation );
int csBevel( double inset, double shift );
int csShapeBevel( int npairs, double *insh );
int csSmoothShift( double offset, double maxangle );
int csSmoothScale( double offset );

/* Text */

int csMakeText( char *text, int index, char *cornertype, double spacing,
   double scale, char *axis, double *pos );
int mgGetFontCount( void );
int mgGetFontIndex( char *name );
const char *mgGetFontName( int index );
int mgLoadFont( char *filename );
void mgClearFont( int index );

/* Surfaces */

int csSetDefaultSurface( const char *surfname );
const char *mgGetDefaultSurface( void );
int csChangeSurface( const char *surface );

LWSurfaceID mgGetFirstSurface( void );
LWSurfaceID mgGetNextSurface( LWSurfaceID sid );
LWSurfaceID *mgGetSurfacesByName( const char *name, const char *objname );
LWSurfaceID *mgGetSurfacesByObject( const char *name );
const char *mgGetSurfaceName( LWSurfaceID sid );
const char *mgGetSurfaceObject( LWSurfaceID sid );

LWSurfaceID mgCreateSurface( const char *objname, const char *surfname );

int csSetSurfEdSurface( const char *surfname, const char *objname );

int mgGetSurfaceInt( LWSurfaceID sid, const char *ch );
int csSetSurfaceInt( const char *ch, int val );
double mgGetSurfaceFlt( LWSurfaceID sid, const char *ch );
int csSetSurfaceFlt( const char *ch, double val );
void mgGetSurfaceColor( LWSurfaceID sid, const char *ch, double color[ 3 ] );
int csSetSurfaceColor( const char *ch, double color[ 3 ] );

LWEnvelopeID mgGetSurfaceEnvelope( LWSurfaceID sid, const char *ch );
LWTextureID mgGetSurfaceTexture( LWSurfaceID sid, const char *ch );
LWImageID mgGetSurfaceImage( LWSurfaceID sid, const char *ch );

int csAddSurfaceShader( const char *shader );
int csRemSurfaceShader( const char *shader );
int csCopySurface( LWSurfaceID to, LWSurfaceID from );
int csRenameSurface( const char *newname );

int csOpenSurfEdWindow( void );
int csCloseSurfEdWindow( void );
int csSetSurfEdWindowPos( int x, int y );

/* Mesh Editing */

int mePointCount( EltOpLayer layers, int mode );
int mePolyCount( EltOpLayer layers, int mode );

int mePointScan( EDPointScanFunc *callback, void *userdata, EltOpLayer layers );
EDError mePolyScan( EDPolyScanFunc *callback, void *userdata, EltOpLayer layers );

EDPointInfo *mePointInfo( LWPntID id );
EDPolygonInfo *mePolyInfo( LWPolID id );
int mePolyNormal( LWPolID id, double norm[ 3 ] );

LWPntID meAddPoint( double *coord );
LWPntID meAddPointInterp( double *coord, int npoints, LWPntID *ptarray,
   double *weight );
LWPolID meAddPoly( LWID type, LWPolID proto, char *surfname, int npoints,
   LWPntID *ptarray );
LWPolID meAddCurve( char *surfname, int npoints, LWPntID *ptarray, int flags );
EDError meAddQuad( LWPntID pt1, LWPntID pt2, LWPntID pt3, LWPntID pt4 );
EDError meAddTri( LWPntID pt1, LWPntID pt2, LWPntID pt3 );
EDError meAddPatch( int nr, int nc, int lr, int lc, EDBoundCv *r0,
   EDBoundCv *r1, EDBoundCv *c0, EDBoundCv *c1 );

EDError meRemovePoint( LWPntID id );
EDError meRemovePoly( LWPolID id );

EDError meMovePoint( LWPntID id, double *coord );
EDError meSetPolySurface( LWPolID id, char *surfname );
EDError meSetPolyPoints( LWPolID id, int npoints, LWPntID *ptarray );
EDError meSetPolyFlags( LWPolID id, int mask, int value );

EDError meSetPolyTag( LWPolID id, LWID tagid, const char *tag );
const char *meGetPolyTag( LWPolID id, LWID tagid );
EDError meSetPointVMap( LWPntID id, LWID vmapid, const char *vmapname,
   int nval, float *val );
void *meSelectPointVMap( void *data, LWID vmapid, const char *vmapname );
int meGetPointVMap( LWPntID id, float *vmap );
EDError meInitUV( float uv[ 2 ] );


#endif  /* LWMODLIB_H */
