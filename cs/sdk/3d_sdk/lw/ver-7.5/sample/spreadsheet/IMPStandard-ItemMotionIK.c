/*
 * IMPStandardBanks-ItemMotionIK.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 * Goal_Ghost()
 *  Ghosts the cell if the item doesn't have an IK goal.
 */
int Goal_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 )
    return IMPGHOST_BLANK;

  return ((item_info->goal( id ) != LWITEM_NULL) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED );
}

/*
 *  Unaffected By IK
 */
void * UnaffectedByIK_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = item_info->flags( id ) & LWITEMF_UNAFFECT_BY_IK;
  return &value_int;
}

void * UnaffectedByIK_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  unsigned int old_state = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = *(int *)value;

  if( apply ) {
    int test = item_info->flags( id ) & LWITEMF_UNAFFECT_BY_IK;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "UnaffectedByIK" );
    }
  }

  return &value_int;
}

IMPColumn col_UnaffectedByIK = {
  "Unaffected by IK of Decendants",          /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Unaffected by IK of Decendants, Motion Options, IK and Modifiers", /* Comment */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  UnaffectedByIK_Query,                      /* Query function          */
  UnaffectedByIK_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  IK Goal
 */
void * IKGoal_Query( int column, int row, LWItemID id, LWTime time ) {
  return item_info->goal( id );
}

void * IKGoal_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "GoalItem %x", value );
    command( buffer );
  }

  return value;
}

int IKGoal_Test( int column, int row, LWItemID applied_id, LWItemID queried_id ) {
  if( queried_id == LWITEM_NULL )
    return 1;

  if( item_info->type( queried_id ) == LWI_OBJECT )
    return 1;

  return 0;
}

IMPColumn col_IKGoal = {
  "Goal Object",                             /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_ITEM,                           /* column type             */
  "Goal Object, Motion Options, IK and Modifiers", /* Comment           */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  IKGoal_Query,                              /* Query function          */
  IKGoal_Evaluate,                           /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  IKGoal_Test,                               /* Test Item function      */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *   Goal Strength
 */
void * GoalStrength_Query( int column, int row, LWItemID id, LWTime time ) {
  if( row != 0 ) {
    value_float = 0.0;
    return &value_float;
  }

  value_float = item_info->goalStrength( id );
  return &value_float;
}

void * GoalStrength_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  if( row != 0 ) {
    value_float = 0.0;
    return &value_float;
  }

  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 150 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "GoalStrength %g", value_float );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_GoalStrength = {
  "Goal Strength",                           /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Goal Strength, Motion Options, IK and Modifiers", /* Comment         */
  NULL,                                      /* No envelope function    */
  Goal_Ghost,                                /* Ghosted function        */
  GoalStrength_Query,                        /* Query function          */
  GoalStrength_Evaluate,                     /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Full-time IK
 */
void * FullTimeIK_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = item_info->flags(id ) & LWITEMF_FULLTIME_IK;
  return &value_int;
}

void * FullTimeIK_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  unsigned int old_state = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = *(int *)value;

  if( apply ) {
    int test = item_info->flags( id ) & LWITEMF_FULLTIME_IK;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "FullTimeIK" );
    }
  }

  return &value_int;
}

IMPColumn col_FullTimeIK = {
  "Full-time IK",                            /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Full-time IK, Motion Options, IK and Modifiers", /* Comment          */
  NULL,                                      /* No envelope function    */
  Goal_Ghost,                                /* Ghost function          */
  FullTimeIK_Query,                          /* Query function          */
  FullTimeIK_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Match Goal Orientation
 */
void * MatchGoal_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = item_info->flags( id ) & LWITEMF_GOAL_ORIENT;
  return &value_int;
}

void * MatchGoal_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  unsigned int old_state = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = *(int *)value;

  if( apply ) {
    int test = item_info->flags( id ) & LWITEMF_GOAL_ORIENT;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "MatchGoalOrientation" );
    }
  }

  return &value_int;
}

IMPColumn col_MatchGoal = {
  "Match Goal Orientation",                  /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Match Goal Orientation, Motion Options, IK and Modifiers", /* Comment */
  NULL,                                      /* No envelope function    */
  Goal_Ghost,                                /* Ghost function          */
  MatchGoal_Query,                           /* Query function          */
  MatchGoal_Evaluate,                        /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Keep Goal In Reach
 */
void * KeepGoalInReach_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = item_info->flags( id ) & LWITEMF_REACH_GOAL;
  return &value_int;
}

void * KeepGoalInReach_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  unsigned int old_state = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = *(int *)value;

  if( apply ) {
    int test = item_info->flags( id ) & LWITEMF_REACH_GOAL;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "KeepGoalInReach" );
    }
  }

  return &value_int;
}

IMPColumn col_KeepGoalInReach = {
  "Keep Goal In Reach",                      /* title                   */
  COLWIDTH_TOGGLE,                          /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Keep Goal In Reach, Motion Options, IK and Modifiers", /* Comment    */
  NULL,                                      /* No envelope function    */
  Goal_Ghost,                                /* Ghost function          */
  KeepGoalInReach_Query,                     /* Query function          */
  KeepGoalInReach_Evaluate,                  /* Evaluate function       */
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
IMPColumn *col_motionOptionsIK[] = {
  &col_UnaffectedByIK,
  &col_IKGoal,
  &col_GoalStrength,
  &col_FullTimeIK,
  &col_MatchGoal,
  &col_KeepGoalInReach,
  NULL };

IMPBank bank_motionOptionsIK = {
  MakeBankID( '_', 'M', 'I', 'K' ),          /* id:  Standard (_) Motion Options: IK                  */
  "Motion Options: IK",                      /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_motionOptionsIK,                       /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

