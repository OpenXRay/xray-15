/*
 * IMPStandardBanks-ItemMotionMisc.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 *  Parent
 */
void * Parent_Query( int column, int row, LWItemID id, LWTime time ) {
  return item_info->parent( id );
}

void * Parent_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  /* Special case (none) for bones */
  if( (item_info->type( id ) == LWI_BONE) && (value == LWITEM_NULL) ) {
    for( value = item_info->parent( id );          /* Find the first ancestor that is an object and use that as our parent */
        ((item_info->type( value ) != LWI_OBJECT) && (item_info->type( value ) != (int)LWITEM_NULL));
        value = item_info->parent( value ) ) { ; }
  }

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ParentItem %x", value );
    command( buffer );
  }

  return value;
}

int Parent_Test( int column, int row, LWItemID applied_id, LWItemID queried_id ) {
  if( queried_id == LWITEM_NULL )
    return 1;

  if( applied_id == queried_id )
    return 0;

  if( item_info->type( applied_id ) == LWI_BONE ) {           /* Bones can only be parented to other bones or their owner*/
    /* If this is an object, see if it's the bone's owner */
    LWItemID queried_owner = LWITEM_NULL, applied_owner = LWITEM_NULL;
    for( applied_owner = item_info->parent( applied_id );     /* Find the first ancestor that is an object */
         ((item_info->type( applied_owner ) != LWI_OBJECT) && (item_info->type( applied_owner ) != (int)LWITEM_NULL));
         applied_owner = item_info->parent( applied_owner ) ) { ; }

    if( item_info->type( queried_id ) == LWI_OBJECT ) {
      if( applied_owner == queried_id )
        return 1;
    }

    /* Not the owner; return only bones belonging to the same object */
    if( item_info->type( queried_id ) != LWI_BONE )           /* Not a bone */
      return 0;

    for( queried_owner = item_info->parent( applied_id );     /* Find the first ancestor that is an object */
         ((item_info->type( queried_owner ) != LWI_OBJECT) && (item_info->type( queried_owner ) != (int)LWITEM_NULL));
         queried_owner = item_info->parent( queried_owner ) ) { ; }

    /* if the owners are the same, the bone is OK */
    return ((queried_owner == applied_owner) ? 1 : 0);
  }

  return 1;
}

IMPColumn col_Parent = {
  "Parent",                                  /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_ITEM,                           /* column type             */
  "Parent, Motion Options",                  /* Comment                 */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  Parent_Query,                              /* Query function          */
  Parent_Evaluate,                           /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  Parent_Test,                               /* Test Item function      */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Target
 */
void * Target_Query( int column, int row, LWItemID id, LWTime time ) {
  return item_info->target( id );
}

void * Target_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "TargetItem %x", value );
    command( buffer );
  }

  return value;
}

int Target_Test( int column, int row, LWItemID applied_id, LWItemID queried_id ) {
  if( queried_id == LWITEM_NULL )
    return 1;

  if( applied_id == queried_id )
    return 0;

  return 1;
}

IMPColumn col_Target = {
  "Target",                                  /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_ITEM,                           /* column type             */
  "Target, Motion Options",                  /* Comment                 */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  Target_Query,                              /* Query function          */
  Target_Evaluate,                           /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  Target_Test,                               /* Test Item function      */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Align Look Ahead
 */
IMPGhostModes AlignLookAhead_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 ) {
    return IMPGHOST_BLANK;
  } else {
    int controllers[3];
    item_info->controller( id, LWIP_ROTATION, controllers );

    return ( ((controllers[0] == LWMOTCTL_ALIGN_TO_PATH) ||
              (controllers[1] == LWMOTCTL_ALIGN_TO_PATH) ||
              (controllers[2] == LWMOTCTL_ALIGN_TO_PATH) ) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED );
  }
}

void * AlignLookAhead_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = item_info->lookAhead( id );
  return &value_float;
}

void * AlignLookAhead_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "PathAlignLookAhead %g", value_float );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_AlignLookAhead = {
  "Path Align Look Ahead",                   /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_TIME,                           /* column type             */
  "Align to Path Look Ahead, Motion Options, IK and Modifiers", /* Comment */
  NULL,                                      /* No envelope function    */
  AlignLookAhead_Ghost,                      /* Ghost function          */
  AlignLookAhead_Query,                      /* Query function          */
  AlignLookAhead_Evaluate,                   /* Evaluate function       */
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
IMPColumn *col_motionOptionsMisc[] = {
  &col_Parent,
  &col_Target,
  &col_AlignLookAhead,
  NULL };

IMPBank bank_motionOptionsMisc = {
  MakeBankID( '_', 'M', 'O', 'M' ),                   /* id:  Standard (_) Motion Options Misc                 */
  "Motion Options: General",                          /* Bank Title                                            */
  IMPBASE_ITEM,                                       /* Item base type                                        */
  col_motionOptionsMisc,                              /* Columns in bank                                       */
  NULL,                                               /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                               /* No begin process function                             */
  NULL,                                               /* No end process function                               */
};


