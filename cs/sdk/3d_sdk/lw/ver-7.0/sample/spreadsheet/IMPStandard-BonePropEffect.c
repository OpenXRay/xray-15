/*
 * IMPStandardBanks-BonePropertiesEffect.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 * BoneJointCompensationAmount_Ghost()
 *  Ghosts the cell if the item isn't a camera.
 */
int BoneJointCompensationAmount_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_BONE) )
    return IMPGHOST_BLANK;

  return ((bone_info->flags( id ) /* & LWBONEF_JOINT_COMP_SELF */ /* TODO:  Fix when possible */) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED );
}

/*
 * BoneParentJointCompensationAmount_Ghost()
 *  Ghosts the cell if the item isn't a camera.
 */
int BoneParentJointCompensationAmount_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_BONE) )
    return IMPGHOST_BLANK;

  return ((bone_info->flags( id ) /* & LWBONEF_JOINT_COMP_PARENT */ /* TODO:  Fix when possible */) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED );
}

/*
 * BoneMuscleFlexingAmount_Ghost()
 *  Ghosts the cell if the item isn't a camera.
 */
int BoneMuscleFlexingAmount_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_BONE) )
    return IMPGHOST_BLANK;

  return ((bone_info->flags( id ) /* & LWBONEF_MUSCLE_FLEX_SELF */ /* TODO:  Fix when possible */) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED );
}

/*
 * BoneParentMuscleFlexingAmount_Ghost()
 *  Ghosts the cell if the item isn't a camera.
 */
int BoneParentMuscleFlexingAmount_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_BONE) )
    return IMPGHOST_BLANK;

  return ((bone_info->flags( id ) /* & LWBONEF_MUSCLE_FLEX_PARENT */ /* TODO:  Fix when possible */) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED );
}

/*
 *  Bone Joint Compensation
 */
void * BoneJointCompensation_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_JOINT_COMP;
  return &value_int;
}

void * BoneJointCompensation_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_JOINT_COMP;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneJointComp" ); /* TODO:  Fix when possible */
    }
  }

  return &value_int;
}

IMPColumn col_BoneJointCompensation = {
  "Joint Compensation",                      /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Joint Compensation, Bone Properties",     /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneJointCompensation_Query,               /* Query function          */
  BoneJointCompensation_Evaluate,            /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Bone Joint Compensation Amount
 */
void * BoneJointCompensationAmount_Query( int column, int row, LWItemID id, LWTime time ) {
  double self, parent;
  bone_info->jointComp( id, &self, &parent );
  value_float = ((column == 1) ? self : parent) * 100.0;
  return &value_float;
}

void * BoneJointCompensationAmount_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  double self, parent;
  value_float = *(double *)value;
  bone_info->jointComp( id, &self, &parent );
  if( column == 1 )
    self = value_float / 100.0;
  else
    parent = value_float / 100.0;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "BoneJointCompAmounts %g %g", self, parent );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_BoneJointCompensationAmount = {
  "Joint Compensation Amount",               /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Joint Compensation, Bone Properties",     /* Comment                 */
  NULL,                                      /* No envelope function    */
  BoneJointCompensationAmount_Ghost,         /* Ghosted function        */
  BoneJointCompensationAmount_Query,         /* Query function          */
  BoneJointCompensationAmount_Evaluate,      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BoneParentJointCompensationAmount = {
  "Parent Joint Compensation Amount",        /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Parent Joint Compensation, Bone Properties", /* Comment              */
  NULL,                                      /* No envelope function    */
  BoneParentJointCompensationAmount_Ghost,   /* Ghosted function        */
  BoneJointCompensationAmount_Query,         /* Query function          */
  BoneJointCompensationAmount_Evaluate,      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Bone Parent Joint Compensation
 */
void * BoneParentJointCompensation_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_JOINT_COMP_PAR;
  return &value_int;
}

void * BoneParentJointCompensation_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_JOINT_COMP_PAR;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneJointCompParent" );
    }
  }

  return &value_int;
}

IMPColumn col_BoneParentJointCompensation = {
  "Parent Joint Compensation",               /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Parent Joint Compensation, Bone Properties", /* Comment              */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneParentJointCompensation_Query,         /* Query function          */
  BoneParentJointCompensation_Evaluate,      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Bone Muscle Flexing
 */
void * BoneMuscleFlexing_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_MUSCLE_FLEX;
  return &value_int;
}

void * BoneMuscleFlexing_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_MUSCLE_FLEX;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneMuscleFlex" );
    }
  }

  return &value_int;
}

IMPColumn col_BoneMuscleFlexing = {
  "Muscle Flexing",                          /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Muscle Flexing, Bone Properties",         /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneMuscleFlexing_Query,                   /* Query function          */
  BoneMuscleFlexing_Evaluate,                /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Bone Muscle Flexing Amount
 */
void * BoneMuscleFlexingAmount_Query( int column, int row, LWItemID id, LWTime time ) {
  double self, parent;
  bone_info->muscleFlex( id, &self, &parent );
  value_float = ((column == 5) ? self : parent) * 100.0;
  return &value_float;
}

void * BoneMuscleFlexingAmount_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  double self, parent;
  value_float = *(double *)value;
  bone_info->muscleFlex( id, &self, &parent );
  if( column == 5 )
    self = value_float / 100.0;
  else
    parent = value_float / 100.0;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "BoneMuscleFlexAmounts %g %g", self, parent );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_BoneMuscleFlexingAmount = {
  "Joint Compensation Amount",               /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Joint Compensation, Bone Properties",     /* Comment                 */
  NULL,                                      /* No envelope function    */
  BoneMuscleFlexingAmount_Ghost,             /* Ghosted function        */
  BoneMuscleFlexingAmount_Query,             /* Query function          */
  BoneMuscleFlexingAmount_Evaluate,          /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BoneParentMuscleFlexingAmount = {
  "Parent Joint Compensation Amount",        /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Parent Joint Compensation, Bone Properties", /* Comment              */
  NULL,                                      /* No envelope function    */
  BoneParentMuscleFlexingAmount_Ghost,       /* Ghosted function        */
  BoneMuscleFlexingAmount_Query,             /* Query function          */
  BoneMuscleFlexingAmount_Evaluate,          /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Bone Parent Muscle Flexing
 */
void * BoneParentMuscleFlexing_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_MUSCLE_FLEX_PAR;
  return &value_int;
}

void * BoneParentMuscleFlexing_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_MUSCLE_FLEX_PAR;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneMuscleFlexParent" );
    }
  }

  return &value_int;
}

IMPColumn col_BoneParentMuscleFlexing = {
  "Parent Muscle Flexing",                   /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Parent Muscle Flexing, Bone Properties",  /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneParentMuscleFlexing_Query,             /* Query function          */
  BoneParentMuscleFlexing_Evaluate,          /* Evaluate function       */
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
IMPColumn *col_bonePropertiesEffect[] = {
  &col_BoneJointCompensation,
  &col_BoneJointCompensationAmount,
  &col_BoneParentJointCompensation,
  &col_BoneParentJointCompensationAmount,
  &col_BoneMuscleFlexing,
  &col_BoneMuscleFlexingAmount,
  &col_BoneParentMuscleFlexing,
  &col_BoneParentMuscleFlexingAmount,
  NULL };

IMPBank bank_bonePropertiesEffect = {
  MakeBankID( '_', 'B', 'P', 'E' ),          /* id:  Standard (_) Bone Properties: Effect             */
  "Bone Properties: Effect",                 /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_bonePropertiesEffect,                  /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};


