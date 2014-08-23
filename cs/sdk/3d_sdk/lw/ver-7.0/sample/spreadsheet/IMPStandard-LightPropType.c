/*
 * IMPStandardBanks-LightPropertiesTypeSpecific.c
 */

#include <stdio.h>
#include <string.h>

#include "IMPStandard.h"

extern IMPColumn col_LightType;

/*
 * SpotLight_Ghost()
 *  Ghosts the cell if the ID points to anything except a spot light.
 */
int SpotLight_Ghost( int column, int row, LWItemID id ) {
  if( (item_info->type( id ) != LWI_LIGHT) || (row != 0) )
    return IMPGHOST_BLANK;

  return ((light_info->type( id ) == LWLIGHT_SPOT) ? IMPGHOST_ENABLED : IMPGHOST_BLANK );
}

/*
 * AreaLinearLight_Ghost()
 *  Ghosts the cell if the ID points to anything except an area or linear light.
 */
int AreaLinearLight_Ghost( int column, int row, LWItemID id ) {
  if( (item_info->type( id ) != LWI_LIGHT) || (row != 0) )
    return IMPGHOST_BLANK;

  return (((light_info->type( id ) == LWLIGHT_LINEAR) || (light_info->type( id ) == LWLIGHT_AREA)) ? IMPGHOST_ENABLED : IMPGHOST_BLANK );
}

/*
 *  Linear/Area Light Quality
 */
void * LightQuality_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = light_info->quality( id );
  return &value_int;
}

void * LightQuality_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "LightQuality %d", value_int );
    command( buffer );
  }

  return &value_int;
}

IMPColumn col_LightQuality = {
  "Linear/Area Quality",                     /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_INTEGER,                        /* column type             */
  "Linear/Area Light Quality, Light Properties",  /* Comment            */
  NULL,                                      /* No envelope function    */
  AreaLinearLight_Ghost,                     /* Ghosted function        */
  LightQuality_Query,                        /* Query function          */
  LightQuality_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Spotlight Cone Angle
 */
void * SpotlightConeAngle_Query( int column, int row, LWItemID id, LWTime time ) {
  double edge;
  light_info->coneAngles( id, time, &value_float, &edge );
  value_float = RadiansToDegrees( value_float );
  return &value_float;
}

void * SpotlightConeAngle_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "LightConeAngle %g", value_float );
    command( buffer );
  }

  return &value_float;
}

/*
 * ConeAngle_Envelope():
 */
LWChannelID *ConeAngle_Envelope( int column, int row, void * id, IMPEnvRequests request ) {
  return EditSingleEnvelopeOfItem( id, request, "ConeAngle" );
}

IMPColumn col_SpotlightConeAngle = {
  "Cone Angle",                              /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Spotlight Cone Angle, Light Properties",  /* Comment                 */
  ConeAngle_Envelope,                        /* Envelope function       */
  SpotLight_Ghost,                           /* Ghosted function        */
  SpotlightConeAngle_Query,                  /* Query function          */
  SpotlightConeAngle_Evaluate,               /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Spotlight Edge Angle
 */
void * SpotlightEdgeAngle_Query( int column, int row, LWItemID id, LWTime time ) {
  double radius;
  light_info->coneAngles( id, time, &radius, &value_float );
  value_float = RadiansToDegrees( value_float );
  return &value_float;
}

void * SpotlightEdgeAngle_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "LightEdgeAngle %g", value_float );
    command( buffer );
  }

  return &value_float;
}

/*
 * EdgeAngle_Envelope():
 */
LWChannelID *EdgeAngle_Envelope( int column, int row, void * id, IMPEnvRequests request ) {
  return EditSingleEnvelopeOfItem( id, request, "EdgeAngle" );
}

IMPColumn col_SpotlightEdgeAngle = {
  "Soft Edge Angle",                         /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Spotlight Soft Edge Angle, Light Properties",  /* Comment            */
  EdgeAngle_Envelope,                        /* Envelope function       */
  SpotLight_Ghost,                           /* Ghosted function        */
  SpotlightEdgeAngle_Query,                  /* Query function          */
  SpotlightEdgeAngle_Evaluate,               /* Evaluate function       */
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
IMPColumn *col_lightPropertiesTypeSpecific[] = {
  &col_LightType,
  &col_LightQuality,
  &col_SpotlightConeAngle,
  &col_SpotlightEdgeAngle,
  NULL };

IMPBank bank_lightPropertiesTypeSpecific = {
  MakeBankID( '_', 'L', 'P', 'T' ),          /* id:  Standard (_) Light Properties: Type Specific     */
  "Light Properties: Type Specific",         /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_lightPropertiesTypeSpecific,           /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};


