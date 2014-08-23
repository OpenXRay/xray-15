/*
 * IMPStandardBanks-ObjectPropertiesMorphing.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 * Morph_Ghost()
 *  Ghosts the cell if the morph target is NULL points to anything except an object.
 */
int Morph_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_OBJECT) )
    return IMPGHOST_BLANK;

  return ((object_info->morphTarget( id ) == LWITEM_NULL) ? IMPGHOST_DISABLED : IMPGHOST_ENABLED);
}

/*
 * MorphFlags_Ghost()
 *  Ghosts the cell if the morph target is NULL points to anything except an object.
 */
int MorphFlags_Ghost( int column, int row, LWItemID id ) {
  double amount;
  if( (row != 0) || (item_info->type( id ) != LWI_OBJECT) )
    return IMPGHOST_BLANK;

  if( object_info->morphTarget( id ) == LWITEM_NULL )
    return IMPGHOST_DISABLED;

  amount = object_info->morphAmount( id, 0.0 );
  return (((amount > -0.00000001) && (amount < 0.00000001)) ? IMPGHOST_DISABLED : IMPGHOST_ENABLED);
}

/*
 *  Morph Target
 */
void * MorphTarget_Query( int column, int row, LWItemID id, LWTime time ) {
  return object_info->morphTarget( id );
}

void * MorphTarget_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "MorphTarget %x", value );
    command( buffer );
  }

  return value;
}

int MorphTarget_Test( int column, int row, LWItemID applied_id, LWItemID queried_id ) {
  if( queried_id == LWITEM_NULL )
    return 1;

  if( applied_id == queried_id )
    return 0;

  return (item_info->type( queried_id ) == LWI_OBJECT) ? 1 : 0;
}

IMPColumn col_MorphTarget = {
  "Morph Target",                            /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_ITEM,                           /* column type             */
  "Morph Target, Object Properties, Deformations", /* Comment           */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  MorphTarget_Query,                         /* Query function          */
  MorphTarget_Evaluate,                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  MorphTarget_Test,                          /* Test Item function      */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Morph Amount
 */
void * MorphAmount_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = object_info->morphAmount( id, time ) * 100.0;
  return &value_float;
}

void * MorphAmount_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_float = *((double *)value);

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "MorphAmount %g", value_float/100.0 );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_MorphAmount = {
  "Morph Amount",                            /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Morph Amount, Object Properties, Deformations", /* Comment           */
  NULL,                                      /* No envelope function    */
  Morph_Ghost,                               /* Ghosted function        */
  MorphAmount_Query,                         /* Query function          */
  MorphAmount_Evaluate,                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Morph Surfaces
 */
void * MorphSurfaces_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->flags( id ) & LWOBJF_MORPH_SURFACES;
  return &value_int;
}

void * MorphSurfaces_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *((int *)value);

  if( apply ) {
    int test = object_info->flags( id ) & LWOBJF_MORPH_SURFACES;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      sprintf( buffer, "MorphSurfaces" );      /* TODO:  Fix this when API support exists */
      command( buffer );
    }
  }

  return &value_int;
}

IMPColumn col_MorphSurfaces = {
  "Morph Surfaces",                          /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Morph Surfaces, Object Properties, Deformations", /* Comment         */
  NULL,                                      /* No envelope function    */
  MorphFlags_Ghost,                          /* Ghosted function        */
  MorphSurfaces_Query,                       /* Query function          */
  MorphSurfaces_Evaluate,                    /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Multi Target/Single Envelope
 */
void * MTSE_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->flags( id ) & LWOBJF_MORPH_MTSE;
  return &value_int;
}

void * MTSE_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *((int *)value);

  if( apply ) {
    int test = object_info->flags( id ) & LWOBJF_MORPH_MTSE;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      sprintf( buffer, "MorphMTSE" );
      command( buffer );
    }
  }

  return &value_int;
}

IMPColumn col_MTSE = {
  "MTSE",                                    /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Multi Target/Single Envelope, Object Properties, Deformations", /* Comment */
  NULL,                                      /* No envelope function    */
  MorphFlags_Ghost,                          /* Ghosted function        */
  MTSE_Query,                                /* Query function          */
  MTSE_Evaluate,                             /* Evaluate function       */
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
IMPColumn *col_objectPropertiesMorphing[] = {
  &col_MorphTarget,
  &col_MorphAmount,
  &col_MorphSurfaces,
  &col_MTSE,
  NULL };

IMPBank bank_objectPropertiesMorphing = {
  MakeBankID( '_', 'O', 'P', 'M' ),          /* id:  Standard (_) Object Properties: Morphing         */
  "Object Properties: Morphing",             /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_objectPropertiesMorphing,              /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

