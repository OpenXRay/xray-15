/*
 * IMPStandardBanks-ObjectProperties.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 * Object_Ghost()
 *  Ghosts the cell if the ID points to anything except an object.
 */
int Object_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 )
    return IMPGHOST_BLANK;

  return ((item_info->type( id ) == LWI_OBJECT) ? IMPGHOST_ENABLED : IMPGHOST_BLANK );
}

/*
 *  Subdivision Order
 */
void * SubdivisionOrder_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->subdivOrder( id );
  return &value_int;
}

void * SubdivisionOrder_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *((int *)value);
  if( value_int < 0 )  value_int = 0;
  if( value_int > 5 )  value_int = 5;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "SubdivisionOrder %d", value_int );
    command( buffer );
  }

  return &value_int;
}

int SubdivisionOrder_ListCount( int column, int row, LWItemID id ) {
  return 6;
}

const char *subdivision_order_list[] = {
  "First", "After Morphing", "After Bones", "After Displacement", "After Motion", "Last" };

const char * SubdivisionOrder_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 6) )
    return subdivision_order_list[ index ];

  return "";
}

IMPColumn col_SubdivisionOrder = {
  "Subdivision Order",                       /* title                   */
  135,                                       /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Subdivision Order, Object Properties, Geometry", /* Comment           */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  SubdivisionOrder_Query,                    /* Query function          */
  SubdivisionOrder_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  SubdivisionOrder_ListCount,                /* List Count function     */
  SubdivisionOrder_ListName,                 /* List Name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Display Subpatch Level
 */
void * DisplaySubpatch_Query( int column, int row, LWItemID id, LWTime time ) {
  int render;
  object_info->patchLevel( id, &value_int, &render );
  return &value_int;
}

void * DisplaySubpatch_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *((int *)value);
  value_int = (value_int < 0) ? 0 : value_int;

  if( apply ) {
    int display, render;
    object_info->patchLevel( id, &display, &render );

    sprintf( command_buffer, "SelectItem %x", id );
    command( command_buffer );

    sprintf( command_buffer, "SubPatchLevel %d %d", value_int, render );
    command( command_buffer );
  }

  return &value_int;
}

IMPColumn col_DisplaySubpatch = {
  "Display Subpatch Level",                  /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_INTEGER,                        /* column type             */
  "Display Subpatch Level, Object Properties, Geometry", /* Comment     */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  DisplaySubpatch_Query,                     /* Query function          */
  DisplaySubpatch_Evaluate,                  /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Render Subpatch Level
 */
void * RenderSubpatch_Query( int column, int row, LWItemID id, LWTime time ) {
  int display;
  object_info->patchLevel( id, &display, &value_int );
  return &value_int;
}

void * RenderSubpatch_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *((int *)value);
  value_int = (value_int < 0) ? 0 : value_int;

  if( apply ) {
    int display, render;
    object_info->patchLevel( id, &display, &render );

    sprintf( command_buffer, "SelectItem %x", id );
    command( command_buffer );

    sprintf( command_buffer, "SubPatchLevel %d %d", display, value_int );
    command( command_buffer );
  }

  return &value_int;
}

IMPColumn col_RenderSubpatch = {
  "Render Subpatch Level",                   /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_INTEGER,                        /* column type             */
  "Render Subpatch Level, Object Properties, Geometry", /* Comment      */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  RenderSubpatch_Query,                      /* Query function          */
  RenderSubpatch_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Display Metaball Resolution
 */
void * DisplayMetaball_Query( int column, int row, LWItemID id, LWTime time ) {
  double render;
  object_info->metaballRes( id, &value_float, &render );
  return &value_float;
}

void * DisplayMetaball_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_float = *((double *)value);
  value_float = (value_float < 0.0) ? 0.0 : value_float;

  if( apply ) {
    double display, render;
    object_info->metaballRes( id, &display, &render );

    sprintf( command_buffer, "SelectItem %x", id );
    command( command_buffer );

    sprintf( command_buffer, "MetaballResolution %g %g", value_float, render );
    command( command_buffer );
  }

  return &value_float;
}

IMPColumn col_DisplayMetaball = {
  "Display Metaball Resolution",             /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Display Metaball Resolution, Object Properties, Geometry", /* Comment*/
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  DisplayMetaball_Query,                     /* Query function          */
  DisplayMetaball_Evaluate,                  /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};


/*
 *  Render Metaball Resolution
 */
void * RenderMetaball_Query( int column, int row, LWItemID id, LWTime time ) {
  double display;
  object_info->metaballRes( id, &display, &value_float );
  return &value_float;
}

void * RenderMetaball_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_float = *((double *)value);
  value_float = (value_float < 0.0) ? 0.0 : value_float;

  if( apply ) {
    double display, render;
    object_info->metaballRes( id, &display, &render );

    sprintf( command_buffer, "SelectItem %x", id );
    command( command_buffer );

    sprintf( command_buffer, "MetaballResolution %g %g", display, value_float );
    command( command_buffer );
  }

  return &value_float;
}

IMPColumn col_RenderMetaball = {
  "Render Metaball Resolution",              /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Render Metaball Resolution, Object Properties, Geometry", /* Comment */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  RenderMetaball_Query,                      /* Query function          */
  RenderMetaball_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  The Bank
 */
IMPColumn *col_objectPropertiesGeometry[] = {
  &col_SubdivisionOrder,
  &col_DisplaySubpatch,
  &col_RenderSubpatch,
  &col_DisplayMetaball,
  &col_RenderMetaball,
  NULL };

IMPBank bank_objectPropertiesGeometry = {
  MakeBankID( '_', 'O', 'P', 'G' ),          /* id:  Standard (_) Object Properties: Geometry         */
  "Object Properties: Geometry",             /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_objectPropertiesGeometry,              /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};
