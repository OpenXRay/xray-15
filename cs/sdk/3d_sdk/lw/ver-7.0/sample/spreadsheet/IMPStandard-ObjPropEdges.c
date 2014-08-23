/*
 * IMPStandardBanks-ObjectPropertiesEdges.c
 */

#include "IMPStandard.h"

#include <stdio.h>

const int obj_edge_bits[] = { LWEDGEF_SILHOUETTE, LWEDGEF_UNSHARED, LWEDGEF_CREASE, LWEDGEF_SURFACE, LWEDGEF_OTHER, LWEDGEF_SHRINK_DIST };

/*
 * ObjEdgeOptions_Ghost()
 *  Ghosts the cell if all edge toggles are false.
 */
int ObjEdgeOptions_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_OBJECT) )
    return IMPGHOST_BLANK;

  return (object_info->edgeOpts( id ) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED);
}

/*
 *  Object Edges Toggles
 */
void * ObjEdgeFlags_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->edgeOpts( id ) & obj_edge_bits[ column/*/2*/ ];
  return &value_int;
}

void * ObjEdgeFlags_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = object_info->edgeOpts( id ) & obj_edge_bits[ column/*/2*/ ];

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      int state;
      char buffer[ 100 ];

      if( value_int == 0 )
        state = object_info->edgeOpts( id ) & ~obj_edge_bits[ column/*/2*/ ];
      else
        state = object_info->edgeOpts( id ) | obj_edge_bits[ column/*/2*/ ];

      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      sprintf( buffer, "PolygonEdgeFlags %d", state );
      command( buffer );
    }
  }

  return &value_int;
}

IMPColumn col_SilhouetteEdges = {
  "Silhouette Edges",                        /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Silhouette Edges, Object Properties, Edges", /* Comment              */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjEdgeFlags_Query,                        /* Query function          */
  ObjEdgeFlags_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_UnsharedEdges = {
  "Unshared Edges",                          /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Unshared Edges, Object Properties, Edges",/* Comment                 */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjEdgeFlags_Query,                        /* Query function          */
  ObjEdgeFlags_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_SharpCreases = {
  "Sharp Creases",                           /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Sharp Creases, Object Properties, Edges", /* Comment                 */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjEdgeFlags_Query,                        /* Query function          */
  ObjEdgeFlags_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_SurfaceBorders = {
  "Surface Borders",                         /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Surface Borders, Object Properties, Edges", /* Comment               */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjEdgeFlags_Query,                        /* Query function          */
  ObjEdgeFlags_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_OtherEdges = {
  "Other Edges",                             /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Other Edges, Object Properties, Edges",   /* Comment               */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjEdgeFlags_Query,                        /* Query function          */
  ObjEdgeFlags_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Object Edge Color
 */
void * ObjEdgeColor_Query( int column, int row, LWItemID id, LWTime time ) {

  object_info->edgeColor( id, time, value_color );
  return value_color;
}

void * ObjEdgeColor_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_color[0] = ((double *)value)[0];
  value_color[1] = ((double *)value)[1];
  value_color[2] = ((double *)value)[2];

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "PolygonEdgeColor %g %g %g", value_color[0], value_color[1], value_color[2] );
    command( buffer );
  }

  return value_color;
}

IMPColumn col_EdgeColor = {
  "Edge Color",                              /* title                   */
  130,                                       /* default width in pixels */
  IMPCOLTYPE_COLOR,                          /* column type             */
  "Edge Color, Object Properties, Edges",    /* Comment                 */
  NULL,                                      /* No envelope function    */
  ObjEdgeOptions_Ghost,                      /* Ghosted function        */
  ObjEdgeColor_Query,                        /* Query function          */
  ObjEdgeColor_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Shrink Edges With Distance
 */
void * ObjectShrinkEdgesWithDistance_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = ((object_info->edgeOpts( id ) & LWEDGEF_SHRINK_DIST) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED );
  return &value_int;
}

void * ObjectShrinkEdgesWithDistance_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = object_info->edgeOpts( id ) & LWEDGEF_SHRINK_DIST;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      int state;
      char buffer[ 100 ];

      if( value_int == 0 )
        state = object_info->edgeOpts( id ) & ~LWEDGEF_SHRINK_DIST;
      else
        state = object_info->edgeOpts( id ) | LWEDGEF_SHRINK_DIST;

      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      sprintf( buffer, "PolygonEdgeFlags %d", state );
      command( buffer );
    }
  }

  return &value_int;
}

IMPColumn col_ShrinkEdgesWithDistance = {
  "Shrink Edges With Distance",              /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Shrink Edges With Distance, Object Properties, Rendering", /* Comment */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjectShrinkEdgesWithDistance_Query,       /* Query function          */
  ObjectShrinkEdgesWithDistance_Evaluate,    /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};


/*
 *  The Bank
 */
IMPColumn *col_objectPropertiesEdges[] = {
  &col_SilhouetteEdges,
  &col_UnsharedEdges,
  &col_SharpCreases,
  &col_SurfaceBorders,
  &col_OtherEdges,
  &col_EdgeColor,
  &col_ShrinkEdgesWithDistance,
  NULL };

IMPBank bank_objectPropertiesEdges = {
  MakeBankID( '_', 'O', 'P', 'E' ),          /* id:  Standard (_) Object Properties: render Flags     */
  "Object Properties: Edges",                /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_objectPropertiesEdges,                 /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

