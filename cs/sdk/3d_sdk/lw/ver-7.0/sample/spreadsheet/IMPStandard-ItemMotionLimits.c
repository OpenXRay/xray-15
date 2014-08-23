/*
 * IMPStandardBanks-ItemMotionLimits.c
 */

#include "IMPStandard.h"

#include <stdio.h>

double DegreesToRadians( double degrees ) {
  return (degrees * (PI/180));
}

extern double RadiansToDegrees( double radians ) {
  return (radians * 180/PI);
}

int RowZeroOnly_Ghost( int column, int row, LWItemID id ) {
  return ((row == 0) ? IMPGHOST_ENABLED : IMPGHOST_BLANK );
}

const char *heading_pitch_list[] = {
  "Keyframes", "Point at Target", "Align to Path", "Inverse Kinematics" };

const char *bank_list[] = {
  "Keyframes", "Inverse Kinematics" };

/*
 *  Heading Controller
 */
void * HeadingController_Query( int column, int row, LWItemID id, LWTime time  ) {
  int type[3];
  item_info->controller( id, LWIP_ROTATION, type );

  value_int = type[ 0 ];

  return &value_int;
}

void * HeadingController_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;
  if( value_int < 0 )
    value_int = 0;

  if( value_int > 3)
    value_int = 3;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "HController %d", value_int );
    command( buffer );
  }

  return &value_int;
}

int HeadingController_ListCount( int column, int row, LWItemID id ) {
  return 4;
}

const char * HeadingController_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 4) )
    return heading_pitch_list[ index ];

  return "";
}

/*
 *  Pitch Controller
 */
void * PitchController_Query( int column, int row, LWItemID id, LWTime time ) {
  int type[3];
  item_info->controller( id, LWIP_ROTATION, type );

  value_int = type[ 1 ];

  return &value_int;
}

void * PitchController_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;
  if( value_int < 0 )
    value_int = 0;

  if( value_int > 3)
    value_int = 3;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "PController %d", value_int );
    command( buffer );
  }

  return &value_int;
}

int PitchController_ListCount( int column, int row, LWItemID id ) {
  return 4;
}

const char * PitchController_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 4) )
    return heading_pitch_list[ index ];

  return "";
}

/*
 *  Heading/Pitch/Bank Controller
 */
void * BankController_Query( int column, int row, LWItemID id, LWTime time ) {
  int type[3];
  item_info->controller( id, LWIP_ROTATION, type );

  value_int = type[ 2 ];

  return &value_int;
}

void * BankController_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;
  if( value_int < 0 )
    value_int = 0;

  if( value_int > 1)
    value_int = 1;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "BController %d", value_int );
    command( buffer );
  }

  return &value_int;
}

int BankController_ListCount( int column, int row, LWItemID id ) {
  return ((column == 10) ? 2 : 4 );
}

const char * BankController_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 2) )
    return bank_list[ index ];

  return "";
}

IMPColumn col_HeadingController = {
  "Heading Controller",                      /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Heading Controller, Motion Options, Controllers and Limits", /* Comment*/
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  HeadingController_Query,                   /* Query function          */
  HeadingController_Evaluate,                /* Evaluate function       */
  NULL,                                      /* No compare function     */
  HeadingController_ListCount,               /* List Count function     */
  HeadingController_ListName,                /* List Name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_PitchController = {
  "Pitch Controller",                        /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Pitch Controller, Motion Options, Controllers and Limits", /* Comment*/
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  PitchController_Query,                     /* Query function          */
  PitchController_Evaluate,                  /* Evaluate function       */
  NULL,                                      /* No compare function     */
  PitchController_ListCount,                 /* List Count function     */
  PitchController_ListName,                  /* List Name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BankController = {
  "Bank Controller",                         /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Bank Controller, Motion Options, Controllers and Limits", /* Comment */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  BankController_Query,                      /* Query function          */
  BankController_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  BankController_ListCount,                  /* List Count function     */
  BankController_ListName,                   /* List Name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Heading Limits
 */
void * HeadingLimits_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector min_vec, max_vec;
  value_int = (int)item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = value_int & LWVECF_0;
  return &value_int;
}

void * HeadingLimits_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  unsigned int old_state = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = *(int *)value;

  if( apply && (value_int != (int)(old_state & LWVECF_0)) ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    command( "LimitH" );
  }

  return &value_int;
}

/*
 *  Pitch Limits
 */
void * PitchLimits_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector min_vec, max_vec;
  value_int = (int)item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = value_int & LWVECF_1;
  return &value_int;
}

void * PitchLimits_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  unsigned int old_state = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = *(int *)value;

  if( apply && (value_int != (int)(old_state & LWVECF_1)) ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    command( "LimitP" );
  }

  return &value_int;
}

/*
 *  Bank Limits
 */
void * BankLimits_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector min_vec, max_vec;
  value_int = (int)item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = value_int & LWVECF_2;
  return &value_int;
}

void * BankLimits_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  unsigned int old_state = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_int = *(int *)value;

  if( apply && (value_int != (int)(old_state & LWVECF_2)) ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    command( "LimitB" );
  }

  return &value_int;
}

IMPColumn col_HeadingLimits = {
  "Heading Limits",                          /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Heading Limits, Motion Options, Controllers and Limits", /* Comment  */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  HeadingLimits_Query,                       /* Query function          */
  HeadingLimits_Evaluate,                    /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_PitchLimits = {
  "Pitch Limits",                            /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Pitch Limits, Motion Options, Controllers and Limits", /* Comment    */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  PitchLimits_Query,                         /* Query function          */
  PitchLimits_Evaluate,                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BankLimits = {
  "Bank Limits",                             /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Bank Limits, Motion Options, Controllers and Limits", /* Comment     */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  BankLimits_Query,                          /* Query function          */
  BankLimits_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Heading Min/Max Limit
 */
IMPGhostModes HeadingMinMaxLimit_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 ) {
    return IMPGHOST_BLANK;
  } else {
    LWDVector min_vec, max_vec;
    unsigned int flags = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

    return ((flags & LWVECF_0) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED);
  }
}

void * HeadingMinMaxLimit_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector min_vec, max_vec;
  item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  if( column == 2 )
    value_float = RadiansToDegrees( min_vec[ 0 ] );
  else
    value_float = RadiansToDegrees( max_vec[ 0 ] );

  return &value_float;
}

void * HeadingMinMaxLimit_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    if( column == 2 )
      sprintf( buffer, "HLimits %g %g", value_float, RadiansToDegrees( max_vec[ 0 ] ) );
    else
      sprintf( buffer, "HLimits %g %g", RadiansToDegrees( min_vec[ 0 ] ), value_float );
    command( buffer );
  }

  return &value_float;
}

/*
 *  Pitch Min/Max Limit
 */
IMPGhostModes PitchMinMaxLimit_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 ) {
    return IMPGHOST_BLANK;
  } else {
    LWDVector min_vec, max_vec;
    unsigned int flags = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

    return ((flags & LWVECF_1) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED);
  }
}

void * PitchMinMaxLimit_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector min_vec, max_vec;
  item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  if( column == 2 )
    value_float = RadiansToDegrees( min_vec[ 1 ] );
  else
    value_float = RadiansToDegrees( max_vec[ 1 ] );

  return &value_float;
}

void * PitchMinMaxLimit_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    if( column == 2 )
      sprintf( buffer, "PLimits %g %g", value_float, RadiansToDegrees( max_vec[ 1 ] ) );
    else
      sprintf( buffer, "PLimits %g %g", RadiansToDegrees( min_vec[ 1 ] ), value_float );
    command( buffer );
  }

  return &value_float;
}

/*
 *  Bank Min/Max Limit
 */
IMPGhostModes BankMinMaxLimit_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 ) {
    return IMPGHOST_BLANK;
  } else {
    LWDVector min_vec, max_vec;
    unsigned int flags = item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

    return ((flags & LWVECF_2) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED);
  }
}

void * BankMinMaxLimit_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector min_vec, max_vec;
  item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  if( column == 2 )
    value_float = RadiansToDegrees( min_vec[ 2 ] );
  else
    value_float = RadiansToDegrees( max_vec[ 2 ] );

  return &value_float;
}

void * BankMinMaxLimit_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  LWDVector min_vec, max_vec;
  item_info->limits( id, LWIP_ROTATION, min_vec, max_vec );

  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    if( column == 2 )
      sprintf( buffer, "BLimits %g %g", value_float, RadiansToDegrees( max_vec[ 2 ] ) );
    else
      sprintf( buffer, "BLimits %g %g", RadiansToDegrees( min_vec[ 2 ] ), value_float );
    command( buffer );
  }

  return &value_float;
}


IMPColumn col_MinimumHeading = {
  "Minimum Heading",                         /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Minimum Heading, Motion Options, Controllers and Limits", /* Comment */
  NULL,                                      /* No envelope function    */
  HeadingMinMaxLimit_Ghost,                  /* Ghost function          */
  HeadingMinMaxLimit_Query,                  /* Query function          */
  HeadingMinMaxLimit_Evaluate,               /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_MaximumHeading = {
  "Maximum Heading",                         /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Maximum Heading, Motion Options, Controllers and Limits", /* Comment */
  NULL,                                      /* No envelope function    */
  HeadingMinMaxLimit_Ghost,                  /* Ghost function          */
  HeadingMinMaxLimit_Query,                  /* Query function          */
  HeadingMinMaxLimit_Evaluate,               /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_MinimumPitch = {
  "Minimum Pitch",                           /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Minimum Pitch, Motion Options, Controllers and Limits", /* Comment   */
  NULL,                                      /* No envelope function    */
  PitchMinMaxLimit_Ghost,                    /* Ghost function          */
  PitchMinMaxLimit_Query,                    /* Query function          */
  PitchMinMaxLimit_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_MaximumPitch = {
  "Maximum Pitch",                           /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Maximum Pitch, Motion Options, Controllers and Limits", /* Comment   */
  NULL,                                      /* No envelope function    */
  PitchMinMaxLimit_Ghost,                    /* Ghost function          */
  PitchMinMaxLimit_Query,                    /* Query function          */
  PitchMinMaxLimit_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_MinimumBank = {
  "Minimum Bank",                            /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Minimum Bank, Motion Options, Controllers and Limits", /* Comment    */
  NULL,                                      /* No envelope function    */
  BankMinMaxLimit_Ghost,                     /* Ghost function          */
  BankMinMaxLimit_Query,                     /* Query function          */
  BankMinMaxLimit_Evaluate,                  /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_MaximumBank = {
  "Maximum Bank",                            /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Maximum Bank, Motion Options, Controllers and Limits", /* Comment    */
  NULL,                                      /* No envelope function    */
  BankMinMaxLimit_Ghost,                     /* Ghost function          */
  BankMinMaxLimit_Query,                     /* Query function          */
  BankMinMaxLimit_Evaluate,                  /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};


/*
 *  Heading Stiffness
 */
void * HeadingStiffness_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector stiffs;
  item_info->stiffness( id, LWIP_ROTATION, stiffs );

  value_float = stiffs[0];
  return &value_float;
}

void * HeadingStiffness_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "HStiffness %g", value_float );
    command( buffer );
  }

  return &value_float;
}

/*
 *  Pitch Stiffness
 */
void * PitchStiffness_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector stiffs;
  item_info->stiffness( id, LWIP_ROTATION, stiffs );

  value_float = stiffs[1];
  return &value_float;
}

void * PitchStiffness_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "PStiffness %g", value_float );
    command( buffer );
  }

  return &value_float;
}

/*
 *  Bank Stiffness
 */
void * BankStiffness_Query( int column, int row, LWItemID id, LWTime time ) {
  LWDVector stiffs;
  item_info->stiffness( id, LWIP_ROTATION, stiffs );

  value_float = stiffs[2];
  return &value_float;
}

void * BankStiffness_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "BStiffness %g", value_float );
    command( buffer );
  }

  return &value_float;
}

IMPColumn col_HeadingStiffness = {
  "Heading Stiffness",                       /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Heading Stiffness, Motion Options, Controllers and Limits", /* Comment*/
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  HeadingStiffness_Query,                    /* Query function          */
  HeadingStiffness_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_PitchStiffness = {
  "Pitch Stiffness",                         /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Pitch Stiffness, Motion Options, Controllers and Limits", /* Comment */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  PitchStiffness_Query,                      /* Query function          */
  PitchStiffness_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_BankStiffness = {
  "Bank Stiffness",                          /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Bank Stiffness, Motion Options, Controllers and Limits", /* Comment  */
  NULL,                                      /* No envelope function    */
  RowZeroOnly_Ghost,                         /* Ghost function          */
  BankStiffness_Query,                       /* Query function          */
  BankStiffness_Evaluate,                    /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  The Banks
 */
IMPColumn *col_limits_heading[] = {
  &col_HeadingController,
  &col_HeadingLimits,
  &col_MinimumHeading,
  &col_MaximumHeading,
  &col_HeadingStiffness,
  NULL };

IMPBank bank_motionLimitsHeading = {
  MakeBankID( '_', 'C', 'L', 'H' ),                   /* id:  Standard (_) Controllers & Limits, Heading       */
  "Motion Options: Controllers and Limits, Heading",  /* Bank Title                                            */
  IMPBASE_ITEM,                                       /* Item base type                                        */
  col_limits_heading,                                 /* Columns in bank                                       */
  NULL,                                               /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                               /* No begin process function                             */
  NULL,                                               /* No end process function                               */
};


IMPColumn *col_limits_pitch[] = {
  &col_PitchController,
  &col_PitchLimits,
  &col_MinimumPitch,
  &col_MaximumPitch,
  &col_PitchStiffness,
  NULL };

IMPBank bank_motionLimitsPitch = {
  MakeBankID( '_', 'C', 'L', 'P' ),                   /* id:  Standard (_) Controllers & Limits, Pitch         */
  "Motion Options: Controllers and Limits, Pitch",    /* Bank Title                                            */
  IMPBASE_ITEM,                                       /* Item base type                                        */
  col_limits_pitch,                                   /* Columns in bank                                       */
  NULL,                                               /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                               /* No begin process function                             */
  NULL,                                               /* No end process function                               */
};

IMPColumn *col_limits_bank[] = {
  &col_BankController,
  &col_BankLimits,
  &col_MinimumBank,
  &col_MaximumBank,
  &col_BankStiffness,
  NULL };

IMPBank bank_motionLimitsBank = {
  MakeBankID( '_', 'C', 'L', 'B' ),                   /* id:  Standard (_) Controllers & Limits, Bank          */
  "Motion Options: Controllers and Limits, Bank",     /* Bank Title                                            */
  IMPBASE_ITEM,                                       /* Item base type                                        */
  col_limits_bank,                                    /* Columns in bank                                       */
  NULL,                                               /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                               /* No begin process function                             */
  NULL,                                               /* No end process function                               */
};
