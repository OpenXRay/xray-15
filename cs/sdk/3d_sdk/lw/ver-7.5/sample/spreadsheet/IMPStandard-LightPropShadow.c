/*
 * IMPStandardBanks-ObjectPropertiesMorphing.c
 */

#include <stdio.h>
#include <string.h>
#include "IMPStandard.h"

/*
 * ShadowMap_Ghost()
 *  Ghosts the cell if the light isn't shadow mapped.
 */
int ShadowMap_Ghost( int column, int row, LWItemID id ) {
  if( (item_info->type( id ) != LWI_LIGHT) || (row != 0) )
    return IMPGHOST_BLANK;

  return ((light_info->shadowType( id ) == LWLSHAD_MAP) ? IMPGHOST_ENABLED : IMPGHOST_BLANK );
}

/*
 * ShadowMapAngle_Ghost()
 *  Ghosts the cell if the light isn't shadow mapped.
 */
int ShadowMapAngle_Ghost( int column, int row, LWItemID id ) {
  if( (item_info->type( id ) != LWI_LIGHT) || (row != 0) )
    return IMPGHOST_BLANK;

  /* TODO:  Fix when possible */
  if( light_info->shadowType( id ) != LWLSHAD_MAP )
    return IMPGHOST_BLANK;

  return (!(light_info->flags /*& LWLF_FIT_CONE*/) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED );
}

/*
 *  Shadow Type
 */
void * ShadowType_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = light_info->shadowType( id );
  return &value_int;
}

void * ShadowType_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *(int *)value;

  if( value_int < 0 )  value_int = 0;
  if( value_int > 2 )  value_int = 2;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ShadowType %d", value_int );
    command( buffer );
  }

  return &value_int;
}

int ShadowType_ListCount( int column, int row, LWItemID id ) {
  return 3;
}

const char *shadow_type_list[] = {
  "Off", "Ray Trace", "Shadow Map" };

const char * ShadowType_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 3) )
    return shadow_type_list[ index ];

  return "";
}

IMPColumn col_ShadowType = {
  "Shadow Type",                             /* title                   */
  100,                                       /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Shadow Type, Light Properties",           /* Comment                 */
  NULL,                                      /* No envelope function    */
  Light_Ghost,                               /* Ghosted function        */
  ShadowType_Query,                          /* Query function          */
  ShadowType_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  ShadowType_ListCount,                      /* List count function     */
  ShadowType_ListName,                       /* List name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom xpanel        */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Cache Shadow Map
 */
void * CacheShadowMap_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = light_info->flags( id ) & LWLFL_CACHE_SHAD_MAP;
  return &value_int;
}

void * CacheShadowMap_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = light_info->flags( id ) & LWLFL_CACHE_SHAD_MAP;
    if( ((value_int == 0) && (test =! 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "CacheShadowMap" );
    }
  }

  return &value_int;
}

IMPColumn col_CacheShadowMap = {
  "Cache Shadow Map",                        /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Cache Shadow Map, Light Properites",      /* Comment                 */
  NULL,                                      /* No envelope function    */
  ShadowMap_Ghost,                           /* Ghosted function        */
  CacheShadowMap_Query,                      /* Query function          */
  CacheShadowMap_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};


/*
 *  Shadow Map Size
 */
void * ShadowMapSize_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = light_info->shadMapSize( id );
  return &value_int;
}

void * ShadowMapSize_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ShadowMapSize %d", value_int );
    command( buffer );
  }

  return &value_int;
}

IMPColumn col_ShadowMapSize = {
  "Shadow Map Size",                         /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_INTEGER,                        /* column type             */
  "Shadow Map Size, Light Properties",       /* Comment            */
  NULL,                                      /* No envelope function    */
  ShadowMap_Ghost,                           /* Ghosted function        */
  ShadowMapSize_Query,                       /* Query function          */
  ShadowMapSize_Evaluate,                    /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Shadow Map Fuzziness
 */
void * ShadowMapFuzziness_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = light_info->shadMapFuzz( id, time );
  return &value_float;
}

void * ShadowMapFuzziness_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  if( item_info->type( id ) != LWI_LIGHT ) {
    value_float = 0.0;
    return &value_float;
  }

  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ShadowMapFuzziness %g", value_float );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_ShadowMapFuzziness = {
  "Shadow Map Fuzziness",                    /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Shadow Map Fuzziness, Light Properties",  /* Comment                 */
  NULL,                                      /* No envelope function    */
  ShadowMap_Ghost,                           /* Ghosted function        */
  ShadowMapFuzziness_Query,                  /* Query function          */
  ShadowMapFuzziness_Evaluate,               /* Evaluate function       */
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
void * ShadowMapAngle_Query( int column, int row, LWItemID id, LWTime time ) {
  if( item_info->type( id ) != LWI_LIGHT ) {
    value_float = 0.0;
    return &value_float;
  }

  value_float = RadiansToDegrees( light_info->shadMapAngle( id, time ) );
  return &value_float;
}

void * ShadowMapAngle_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  if( item_info->type( id ) != LWI_LIGHT ) {
    value_float = 0.0;
    return &value_float;
  }

  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ShadowMapAngle %g", value_float );
    command( buffer );
  }

  return &value_float;
}

/*
 * ShadowMapAngle_Envelope():
 */
LWChannelID *ShadowMapAngle_Envelope( int column, int row, void * id, IMPEnvRequests request ) {
  return EditSingleEnvelopeOfItem( id, request, "ShadowMapAngle" );
}

IMPColumn col_ShadowMapAngle = {
  "Shadow Map Angle",                        /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Shadow Map Angle, Light Properties",      /* Comment                 */
  ShadowMapAngle_Envelope,                   /* Envelope function       */
  ShadowMapAngle_Ghost,                      /* Ghosted function        */
  ShadowMapAngle_Query,                      /* Query function          */
  ShadowMapAngle_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Fit Cone
 */
void * FitCone_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = light_info->flags( id ) & LWLFL_FIT_CONE;
  return &value_int;
}

void * FitCone_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = light_info->flags( id ) & LWLFL_FIT_CONE;
    if( ((value_int == 0) && (test =! 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "FitCone" );  /* TODO:  Fix when possible */
    }
  }

  return &value_int;
}

IMPColumn col_FitCone = {
  "Fit Cone",                                /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Fit Cone, Light Properites",              /* Comment                 */
  NULL,                                      /* No envelope function    */
  ShadowMap_Ghost,                           /* Ghosted function        */
  FitCone_Query,                             /* Query function          */
  FitCone_Evaluate,                          /* Evaluate function       */
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
IMPColumn *col_lightPropertiesShadows[] = {
  &col_ShadowType,
  &col_CacheShadowMap,
  &col_ShadowMapSize,
  &col_ShadowMapFuzziness,
  &col_FitCone,
  &col_ShadowMapAngle,
  NULL };

IMPBank bank_lightPropertiesShadows = {
  MakeBankID( '_', 'L', 'P', 'S' ),          /* id:  Standard (_) Light Properties: Shadows           */
  "Light Properties: Shadows",               /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_lightPropertiesShadows,                /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};


