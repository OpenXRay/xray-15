/*
 * IMPStandardBanks-BonePropertiesBase.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 * Bone_Ghost()
 *  Ghosts the cell if the item isn't a bone.
 */
int Bone_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_BONE) )
    return IMPGHOST_BLANK;

  return IMPGHOST_ENABLED;
}

/*
 *  Bone Active
 */
void * BoneActive_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = bone_info->flags( id ) & LWBONEF_ACTIVE;
  return &value_int;
}

void * BoneActive_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = bone_info->flags( id ) & LWBONEF_ACTIVE;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "BoneActive" );
    }
  }

  return &value_int;
}

IMPColumn col_BoneActive = {
  "Bone Active",                             /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Bone Active, Bone Properties",            /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneActive_Query,                          /* Query function          */
  BoneActive_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};


/*
 *  Rest Position
 */
void * BoneRestPosition_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector temp;
  bone_info->restParam( id, LWIP_POSITION, temp );
  value_float = temp[ (column - 1) % 3 ];
  return &value_float;
}

void * BoneRestPosition_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    LWDVector temp;
    char buffer[ 150 ];

    bone_info->restParam( id, LWIP_POSITION, temp );
    temp[ (column - 1) % 3 ] = value_float;

    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "BoneRestPosition %g %g %g", temp[0], temp[1], temp[2] );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_BoneRestPositionX = {
  "Rest X Position",                         /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_DISTANCE,                       /* column type             */
  "Bone Rest Position, X Axis, Bone Properties", /* Comment             */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneRestPosition_Query,                    /* Query function          */
  BoneRestPosition_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BoneRestPositionY = {
  "Rest Y Position",                         /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_DISTANCE,                       /* column type             */
  "Bone Rest Position, Y Axis, Bone Properties", /* Comment             */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneRestPosition_Query,                    /* Query function          */
  BoneRestPosition_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BoneRestPositionZ = {
  "Rest Z Position",                         /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_DISTANCE,                       /* column type             */
  "Bone Rest Position, Z Axis, Bone Properties", /* Comment             */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneRestPosition_Query,                    /* Query function          */
  BoneRestPosition_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Rest Rotation
 */
void * BoneRestRotation_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector temp;
  bone_info->restParam( id, LWIP_ROTATION, temp );
  value_float = RadiansToDegrees( temp[ (column - 4) % 3 ] );
  return &value_float;
}

void * BoneRestRotation_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    LWDVector temp;
    char buffer[ 150 ];

    bone_info->restParam( id, LWIP_ROTATION, temp );

    temp[0] = RadiansToDegrees( temp[0] );
    temp[1] = RadiansToDegrees( temp[1] );
    temp[2] = RadiansToDegrees( temp[2] );

    temp[ (column - 4) % 3 ] = value_float;

    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "BoneRestRotation %g %g %g", temp[0], temp[1], temp[2] );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_BoneRestRotationH = {
  "Rest Heading",                            /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Bone Rest Rotation, Heading, Bone Properties", /* Comment            */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneRestRotation_Query,                    /* Query function          */
  BoneRestRotation_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BoneRestRotationP = {
  "Rest Pitch",                              /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Bone Rest Rotation, Pitch, Bone Properties", /* Comment              */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneRestRotation_Query,                    /* Query function          */
  BoneRestRotation_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BoneRestRotationB = {
  "Rest Bank",                               /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Bone Rest Rotation, Bank, Bone Properties", /* Comment               */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneRestRotation_Query,                    /* Query function          */
  BoneRestRotation_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Rest Length
 */
void * BoneRestLength_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = bone_info->restLength( id );
  return &value_float;
}

void * BoneRestLength_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 150 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "BoneRestLength %g ", value_float );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_BoneRestLength = {
  "Rest Length",                             /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Bone Rest Length, Bone Properties",       /* Comment                 */
  NULL,                                      /* No envelope function    */
  Bone_Ghost,                                /* Ghosted function        */
  BoneRestLength_Query,                      /* Query function          */
  BoneRestLength_Evaluate,                   /* Evaluate function       */
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
IMPColumn *col_bonePropertiesBasic[] = {
  &col_BoneActive,
  &col_BoneRestPositionX,
  &col_BoneRestPositionY,
  &col_BoneRestPositionZ,
  &col_BoneRestRotationH,
  &col_BoneRestRotationP,
  &col_BoneRestRotationB,
  &col_BoneRestLength,
 NULL };

IMPBank bank_bonePropertiesBasic = {
  MakeBankID( '_', 'B', 'P', 'B' ),          /* id:  Standard (_) Bone Properties: Basic              */
  "Bone Properties: Basic",                  /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_bonePropertiesBasic,                   /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

