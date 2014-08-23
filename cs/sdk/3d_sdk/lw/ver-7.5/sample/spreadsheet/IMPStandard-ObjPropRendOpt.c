/*
 * IMPStandardBanks-ObjectPropertiesRenderOptions.c
 */

#include "IMPStandard.h"

#include <stdio.h>
#include <string.h>

/*
 *  Object Dissolve
 */
void * ObjectDissolve_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = object_info->dissolve( id, time ) * 100.0;
  return &value_float;
}

void * ObjectDissolve_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ObjectDissolve %g", value_float/100.0 );
    command( buffer );
  }

  return &value_float;
}

/*
 * ObjectDissolve_Envelope():
 */
LWChannelID *ObjectDissolve_Envelope( int column, int row, void * id, IMPEnvRequests request ) {
  return EditSingleEnvelopeOfItem( id, request, "Dissolve" );
}

IMPColumn col_ObjectDissolve = {
  "Object Dissolve",                         /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Object Dissolve, Object Properties, Rendering", /* Comment           */
  ObjectDissolve_Envelope,                   /* Envelope function       */
  Object_Ghost,                              /* Ghosted function        */
  ObjectDissolve_Query,                      /* Query function          */
  ObjectDissolve_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Object Polygon Size
 */
void * ObjectPolygonSize_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = object_info->polygonSize( id, time ) * 100.0;
  return &value_float;
}

void * ObjectPolygonSize_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "PolygonSize %g", value_float/100.0 );
    command( buffer );       /* TODO:  Fix this when this command is available */
  }

  return &value_float;
}

IMPColumn col_ObjectPolygonSize = {
  "Polygon Size",                            /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Polygon Size, Object Properties, Rendering", /* Comment              */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjectPolygonSize_Query,                   /* Query function          */
  ObjectPolygonSize_Evaluate,                /* Evaluate function       */
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
IMPColumn *col_ObjectPropertiesRenderOptions[] = {
  &col_ObjectDissolve,
  &col_ObjectPolygonSize,
  NULL };

IMPBank bank_objectPropertiesRenderOptions = {
  MakeBankID( '_', 'O', 'P', 'R' ),          /* id:  Standard (_) Object Properties: Morphing         */
  "Object Properties: Render Options",       /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_ObjectPropertiesRenderOptions,         /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

