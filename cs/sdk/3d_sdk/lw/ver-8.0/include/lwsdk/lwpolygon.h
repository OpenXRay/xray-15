/*
 * LWSDK Header File
 * Copyright 2004, NewTek, Inc.
 *
 * POLYGON.H -- LightWave Polygon Handlers
 *
 * This header defines the polygon handler.  A polygon type is defined by
 * one of these structures.  The struct contains its four-character type
 * code, some flags, and a private data pointer to be used by the polygon
 * type implementation, plus a set of callback functions used to implement
 * the generic behaviors of this type of polygon.
 */

#ifndef LWSDK_POLYGON_H
#define LWSDK_POLYGON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lwsdk/lwtool.h>
#include <lwsdk/lwmeshes.h>
#include <lwsdk/lwhandler.h>
#include <lwsdk/lwrender.h>

/* Meshes */
/* Polygons are converted to a surface mesh using the 'genMesh' function.  The*/
/* optional 'tstMesh' function can also be provided to allow the client to    */
/* test whether a mesh will be generated for the given polygon.  If there is  */
/* no mesh then the client can save a lot of setup time.  The 'genMesh'       */
/* function returns its data in a struct with the following fields.           */

#define LWPMI_NORMALS   1
#define LWPMI_TEXTURE   2

typedef struct st_LWGPolMeshInfo *LWPolyMeshInfoID;
typedef struct st_LWGPolMeshInfo {
  int            flag;
/* This flag will be set for you to indicate which additional mesh data is being requested.
 * When set to LWPMI_NORMALS, please compute and supply a 'norm' array.
 * When set to LWPMI_TEXTURE, please compute and supply texture data via 'itex' and 'wtex'.
 * You should always specify the 'vrts' and 'pols' arrays.
 */

  int            type;
/* The type is 3 or 4, where 3 indicates that this is a triangle mesh  */
/* and 4 indicates that it is a quad mesh.  A type of zero indicates no mesh. */

  int            nvrt;
/* Number of vertices in the polygon's mesh */

  int            npol;
/* Number of surface polygons this specific polygon instance has generated */
  
  int            ntex;
/* Texture information is organized into a set of tuples; one tuple associated with each vertex
 * in the mesh of polygons.
 * Each point in the mesh is associated with an n-tuple of polygon vertices
 * and weights, with the tuple order specified by 'ntex.'
 * NOTE: Still need to understand this one.
 */

  const int     *pols;
/* The polygons in the mesh are specified by indices into the
 * vertex array 'vrts'.  There are 'npol' polygons and the indicies are grouped into
 * triples or quads based on the mesh type.
 */

  const float   *vrts;
/* The vertices of the polygon mesh are given by an array of floats organized into 'nvrt' triples. */

  const float   *norm;
/* The normals of the polygon mesh are given by an array of floats organized into 'nvrt' triples.
 * NOTE: These probably should be normalized direction vectors (x,y,z)
 * This only is needed when 'flag' is set the LWPMI_NORMALS.
 */

  const int     *itex;
/* The 'itex' array holds the indices of the polygon vertices used in a texture.
 */

  const float   *wtex;
/* The 'wtex' array holds the weight of each texture-polygon vertex.
 */
  } LWGPolMeshInfo;

/************************************************************************/

/**
 *  Access structure much like the Meshinfo structure described elsewhere
 */
typedef struct st_LWPolygonTypeAccess *LWPolygonTypeAccessID;
typedef struct st_LWPolygonTypeAccess {
  void *priv;
  void *       (*pntVLookup)  (LWPolygonTypeAccessID, LWID, const char *);
  int          (*pntVIDGet)   (LWPolygonTypeAccessID, LWPntID, float *vector, void *vmapid);
  void         (*pntOtherPos) (LWPolygonTypeAccessID, LWPntID, LWFVector pos);
  int          (*scanPolys)   (LWPolygonTypeAccessID, LWPolScanFunc *, void *);
  unsigned long(*polType)     (LWPolygonTypeAccessID, LWPolID);
  int          (*polSize)     (LWPolygonTypeAccessID, LWPolID);
  LWPntID      (*polVertex)   (LWPolygonTypeAccessID, LWPolID, int);
  const char * (*polTag)      (LWPolygonTypeAccessID, LWPolID, LWID);
} LWPolygonTypeAccess;

/************************************************************************/

#define LWGPTF_SURFACE  1
#define LWGPTF_FLAT     2
#define LWGPTF_LINEAR   4

/* The change flag contains a set of bits defining the types of changes that
can occur to layer data.  A geometric change is a change to the number of
points and polygons or their interconnections.  A positional change is a
change in the location of points.  A surface change is a change to the
surface attributes of polygons.  A VMap change is a change to the content
of any VMaps. */

#define SYNC_GEOMETRY   1
#define SYNC_POSITION   2
#define SYNC_SURFACE    4
#define SYNC_VMAP       8
#define SYNC_TAGS       16

/* Polygon Type
 * This polygon type structure is used as the interface between
 * the host and your custom polygon definition.
 */

typedef struct st_LWPolyType *LWPolyTypeID;
typedef struct st_LWPolyType {

  unsigned long type;
/* Four Character Type Code. Use LWID_ in lwtypes.h to create.
 * There are several 'built-in' types that can not be used for custom polygon types.
 */
 
  int           flags;
/* This contains bits that determine what kind of polygon this is.
 * If the LWGPTF_SURFACE bit is set, then this type of polygon renders as a surface
 * and a mesh should be generated.  If LWGPTF_FLAT is set, then the surface is
 * intended to be flat.  If LWGPTF_LINEAR is set, then the polygon type is a linear
 * curve and the order of the points determines absolute direction rather
 * than sidedness.
 */

  void	      (*display )( void *instance, LWPolID pol, const LWWireDrawAccess *access, LWPolygonTypeAccessID ptinfo );
/* This is called when an instance of your custom polygon needs to be drawn in a view port
 * using basic drawing functions.  
 * The specific polygon to displayed and the means to obtain to the vertexes are given along
 * with the means to draw into the view port.
 * NOTES: The specifics about the view port may be needed as well.
 *        More advanced drawing capabilities like textured quads may be added later.
 */

  void	      (*genMesh )(  void *instance, LWPolID pol, LWPolyMeshInfoID mesh, LWPolygonTypeAccessID ptinfo );
/* This is called when an instance of your custom polygon needs to generate a standard surface mesh
 * which can consist of vertices, triangles or quads, normal vectors, and texture weightings.
 * The polygon specifies the custom polygon for which a mesh is needed.
 * The vertex_instance allows access to the vertices for the existing custom polygon.
 * The mesh is a structure that you fill in. (see definition in this header above.)
 */

  int	      (*tstMesh )( void *instance, LWPolID pol, LWPolygonTypeAccessID ptinfo );
/* This is called to test if the given custom polygon instance is to be used.
 * Return 0 if not, 1 if yes.
 */

  int           index;                                  /* Private Data, GCore Usage. */

  void         *data;
  
  void        (*startup )( LWPolyTypeID );
/* The 'startup' and 'shutdown' functions are used by the host when
 * adding and removing the polygon type.  The private data should be managed
 * by these functions.
 */
  
  void        (*shutdown)( LWPolyTypeID polygon_type);
/* Called Once When Plugin Unloaded.
 * 
 */

  void *      (*alloc   )( LWPolygonTypeAccessID ptinfo );
/* This is called when an instance of your custom polygon type is being created.
 * Any necessary memory allocation and setup should be done here.
 * 
 * A polygon type gets a chance to allocate some data for every one of the
 * vertex instances in the system.  The 'alloc' function is called with the
 * vertex instance and the polygon type and should return any allocated data
 * necessary to implement the polygon behaviors for that instance.  The 'free'
 * function is obviously called to destroy that data.  The type implementation
 * can access the data by indexing into an array stored in each vertex
 * instance, at the 'index' offset.
 */

  void        (*free    )( void *data );
/* This is called when an instance of your custom polygon type is being destroyed.
 * This is your chance for polygon instance specific memory deallocation.
 * 'data' is a pointer to the memory allocated via the 'alloc' callback.
 */

/* This is called before the mesh is being updated
 */
  int         (*update)  (void *, int lnum, int change, LWPolygonTypeAccessID ptinfo );

} LWPolyType;


#define LWPOLYGON_HCLASS      "PolygonHandler"
#define LWPOLYGONFUNCS_GLOBAL "PolygonServices"        
#define LWPOLYGON_VERSION     4

typedef struct st_LWPolygonHandler {
  LWInstanceFuncs *inst;
  LWItemFuncs     *item;
  } LWPolygonHandler;

#ifdef __cplusplus
};
#endif
#endif
