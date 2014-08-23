/*
 *  IMPStandardBanks-Test.c
 */

#include "IMPStandard.h"

#include <stdio.h>

#define IMPTEST_CUSTOM   MakeBankID( 'T', 'E', 'S', 'T' )

/*
 *  Test
 */
void * Test_Query( int column, int row, LWItemID id, LWTime time ) {
  switch( column ) {
    case 0:                     /* Toggle (1 (on) or 0 (off)) */
      value_int = row;
      return &value_int;
      
    case 1:                     /* Any signed integer */
      value_int = 2 * (row + 1);
      return &value_int;

    case 2:                     /* Percentage (ie: 100% = 100.0) */
      value_float = 50.0 * (row + 1);
      return &value_float;

    case 3:                     /* Distance (ie: 5m = 5.0) */
      value_float = 1.0 * (row + 1);
      return &value_float;

    case 4:                     /* Angle (ie: 90 degrees = 90.0) */
      value_float = 90.0 * (row + 1);
      return &value_float;

    case 5:                     /* Time (ie: 30 seconds = 30.0) */
      value_float = 5 * (row + 1);
      return &value_float;

    case 6:                     /* Any double-percision floating point number */
      value_float = PI * (row + 1);
      return &value_float;

    case 7:                     /* Integer representing an index into a list of items */
      value_int = 1 * (row + 1);
      return &value_int;

    case 8:                     /* ASCII String */
      return "String";

    case 9:                     /* 3-element integer array representing RGB colors (ie: 200, 200, 200 = (int[] = { 200, 200, 200 })) */
      value_color[0] = (130.0 + row) / 255.0;
      value_color[1] = (170.0 + row) / 255.0;
      value_color[2] = (230.0 + row) / 255.0;
      return &value_color;

    case 10:                     /* Item ID */
      return id;

    case 11:                     /* Anything.  Will be passed to Evaluate() */
      return NULL;
 }

  return NULL;
}

void * Test_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  /* This doesn't do anything; I just wanted to test the drawing stuff */
  return value;
}

int Test_ListCount( int column, int row, LWItemID id ) {
  return 6;
}

const char *test_list[] = {
  "Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot" };

const char * Test_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 6) )
    return test_list[ index ];

  return "";
}

int Test_ItemTest( int column, int row, LWItemID applied_id, LWItemID queried_id ) {
  return ( (item_info->type( queried_id ) == LWI_LIGHT) ? 1 : 0 );
}


IMPColumn col_TestToggle = {
  "Test Toggle",                             /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Test Toggle",                             /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestInteger = {
  "Test Integer",                            /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_INTEGER,                        /* column type             */
  "Test Integer",                            /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestPercent = {
  "Test Percent",                            /* title                   */
  70,                                        /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Test Percent",                            /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestDistance = {
  "Test Distance",                           /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_DISTANCE,                       /* column type             */
  "Test Distance",                           /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestAngle = {
  "Test Angle",                              /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_ANGLE,                          /* column type             */
  "Test Angle",                              /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestTime = {
  "Test Time",                               /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_TIME,                           /* column type             */
  "Test Time",                               /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestFloat = {
  "Test Float",                              /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_FLOAT,                          /* column type             */
  "Test Float",                              /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestList = {
  "Test List",                               /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Test List",                               /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  Test_ListCount,                            /* List Count function     */
  Test_ListName,                             /* List Name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestString = {
  "Test String",                             /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_STRING,                         /* column type             */
  "Test String",                             /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestColor = {
  "Test Color",                              /* title                   */
  130,                                       /* default width in pixels */
  IMPCOLTYPE_COLOR,                          /* column type             */
  "Test Color",                              /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestItem = {
  "Test Item (Lights)",                      /* title                   */
  100,                                       /* default width in pixels */
  IMPCOLTYPE_ITEM,                           /* column type             */
  "Test Item",                               /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  Test_ItemTest,                             /* Item Test function      */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_TestCustom = {
  "Test Custom",                             /* title                   */
  50,                                        /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Test Custom",                             /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghost function       */
  Test_Query,                                /* Query function          */
  Test_Evaluate,                             /* Evaluate function       */
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
int Test_NumRows( long bank_id, LWItemID id ) {
  return 2;
}

IMPColumn *col_test[] = {
  &col_TestToggle,
  &col_TestInteger,
  &col_TestPercent,
  &col_TestDistance,
  &col_TestAngle,
  &col_TestTime,
  &col_TestFloat,
  &col_TestList,
  &col_TestString,
  &col_TestColor,
  &col_TestItem,
  &col_TestCustom,
  NULL };

IMPBank bank_test = {
  MakeBankID( '_', 'T', 'S', 'T' ),          /* id:  Standard (_) Object                              */
  "Test",                                    /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_test,                                  /* Columns in bank                                       */
  Test_NumRows,                              /* Num Rows callbanks                                    */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

