/*
 * IMPStandardBanks-BonePropertiesInfluence.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 *  Bone Weight Map
 */
void * BoneWeightMap_Query( int column, int row, LWItemID id, LWTime time ) {
  value_vmap.type = LWVMAP_WGHT;
  value_vmap.name = bone_info->weightMap( id );
  return &value_vmap;
}

void * BoneWeightMap_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  IMPVMap *vmap = (IMPVMap *)value;
  if( vmap->name != NULL ) {
    if( (vmap->type != LWVMAP_WGHT) ) {        /* Only allow Weight Maps */
      value_vmap.type = LWVMAP_WGHT;
      value_vmap.name = bone_info->weightMap( id );
      return &value_vmap;
    }
  }

  value_vmap.type = vmap->type;
  value_vmap.name = vmap->name;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );
    
    sprintf( buffer, "BoneWeightMapName %s", ((value_vmap.name == NULL) ? "(none)" : value_vmap.name) );
    command( buffer );
  }

  return &value_vmap;
}

LWID bones_from_vmap_types[] = { LWVMAP_WGHT, 0 };

int BoneWeightMap_Test( int column, int row, LWItemID applied_id, IMPVMap *vmap ) {
  if( vmap == NULL )                                /* Special case:  if vmap is NULL, return a list of VMap types we can use casted as an int */
    return (int)bones_from_vmap_types;

  if( (vmap->type == 0) || (vmap->name == NULL) )   /* (none) is OK */
    return 1;

  if( vmap->type != LWVMAP_WGHT )                   /* Weight Maps Only */
    return 0;

  return 1;
}

IMPColumn col_BoneWeightMap = {
  "Bone Weight Map",                         /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_VMAP,                           /* column type             */
  "Bone Weight Map, Bone Properties",        /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneWeightMap_Query,                       /* Query function          */
  BoneWeightMap_Evaluate,                    /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  BoneWeightMap_Test,                        /* Test item function      */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Use Weight Map Only
 */
void * BoneWeightMapOnly_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_WEIGHT_MAP_ONLY;
  return &value_int;
}

void * BoneWeightMapOnly_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_WEIGHT_MAP_ONLY;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneWeightMapOnly" );
    }
  }

  return &value_int;
}

IMPColumn col_BoneWeightMapOnly = {
  "Weight Map Only",                         /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Use Weight Map Only, Bone Properties",    /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneWeightMapOnly_Query,                   /* Query function          */
  BoneWeightMapOnly_Evaluate,                /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Use Weight Normalization
 */
void * BoneWeightNormalization_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_WEIGHT_MAP_ONLY;
  return &value_int;
}

void * BoneWeightNormalization_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_WEIGHT_MAP_ONLY;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneNormalization" );
    }
  }

  return &value_int;
}

IMPColumn col_BoneWeightNormalization = {
  "Weight Normalization",                    /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Use Weight Normalization, Bone Properties", /* Comment               */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneWeightNormalization_Query,             /* Query function          */
  BoneWeightNormalization_Evaluate,          /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Bone Strength
 */
void * BoneStrength_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = bone_info->strength( id ) * 100.0;
  return &value_float;
}

void * BoneStrength_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "BoneStrength %g", value_float/100.0 );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_BoneStrength = {
  "Bone Strength",                           /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Bone Strength, Bone Properties",          /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneStrength_Query,                        /* Query function          */
  BoneStrength_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Multiply Strength by Rest Length
 */
void * BoneMultiplyStrength_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_SCALE_STRENGTH;
  return &value_int;
}

void * BoneMultiplyStrength_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_SCALE_STRENGTH;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneStrengthMultiply" );
    }
  }

  return &value_int;
}

IMPColumn col_BoneMultiplyStrength = {
  "Multiply Strength by Rest Length",        /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Multiply Strength by Rest Length, Bone Properties", /* Comment       */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneMultiplyStrength_Query,                /* Query function          */
  BoneMultiplyStrength_Evaluate,             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Limited Range
 */
void * BoneLimitedRange_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_LIMITED_RANGE;
  return &value_int;
}

void * BoneLimitedRange_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_LIMITED_RANGE;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneUseLimitedRange" );        /* TODO:  Fix when API supports this */
    }
  }

  return &value_int;
}

IMPColumn col_BoneLimitedRange = {
  "Limited Range",                           /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Limited Range, Bone Properties",          /* Comment       */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneLimitedRange_Query,                    /* Query function          */
  BoneLimitedRange_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Bone Limited Range Min/Max
 */
void * BoneLimitedRangeMinMax_Query( int column, int row, LWItemID id, LWTime time ) {
  double inner, outer;
  bone_info->limits( id, &inner, &outer );
  value_float = (column % 2) ? outer : inner;
  return &value_float;
}

void * BoneLimitedRangeMinMax_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "%s %g", ((column == 6) ? "BoneMinRange" : "BoneMaxRange"), value_float );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_BoneLimitedRangeMin = {
  "Min Range",                               /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_DISTANCE,                       /* column type             */
  "Limited Range Minimum, Bone Properties",  /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneLimitedRangeMinMax_Query,              /* Query function          */
  BoneLimitedRangeMinMax_Evaluate,           /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BoneLimitedRangeMax = {
  "Max Range",                               /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_DISTANCE,                       /* column type             */
  "Limited Range Maximum, Bone Properties",  /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneLimitedRangeMinMax_Query,              /* Query function          */
  BoneLimitedRangeMinMax_Evaluate,           /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  The Bank
 */
IMPColumn *col_bonePropertiesInfluence[] = {
  &col_BoneWeightMap,
  &col_BoneWeightMapOnly,
  &col_BoneWeightNormalization,
  &col_BoneStrength,
  &col_BoneMultiplyStrength,
  &col_BoneLimitedRange,
  &col_BoneLimitedRangeMin,
  &col_BoneLimitedRangeMax,
  NULL };

IMPBank bank_bonePropertiesInfluence = {
  MakeBankID( '_', 'B', 'P', 'I' ),          /* id:  Standard (_) Bone Properties: Influence          */
  "Bone Properties: Influence",              /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_bonePropertiesInfluence,               /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

