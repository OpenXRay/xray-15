/*
 * IMPStandardBanks-Tags.c
 */

#include "IMPStandard.h"

#include <stdio.h>
#include <string.h>

/*
 *  Tag
 */
void * Tag_Query( int column, int row, LWItemID id, LWTime time ) {
  const char * tag = item_info->getTag( id, row + 1 );
  return (void *)((tag == NULL) ? "" : tag);
}

void * Tag_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  if( apply ) {
    row++;
    if( item_info->getTag( id, row ) == NULL )
      item_info->setTag( id, 0, value );
    else
      item_info->setTag( id, row, (char *)value );
  }

  return value;
}

IMPColumn col_Tag = {
  "Tags",                                    /* title                   */
  200,                                       /* default width in pixels */
  IMPCOLTYPE_STRING,                         /* column type             */
  "Tags",                                    /* Comment                 */
  NULL,                                      /* No envelope function    */
  NULL,                                      /* No ghosted function     */
  Tag_Query,                                 /* Query function          */
  Tag_Evaluate,                              /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 * Tag Rows
 */
int Tag_Rows( long bank_id, void * id ) {
  int i;
  for( i=1; item_info->getTag( id, i ) != NULL; i++ ) { ; }
  return i;   /* Note there is one extra here */
}

/*
 *  The Bank
 */
IMPColumn *col_tag[] = {
  &col_Tag,
  NULL };

IMPBank bank_tag = {
  MakeBankID( '_', 'T', 'A', 'G' ),          /* id:  Standard (_) Tags                                */
  "Tags",                                    /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_tag,                                   /* Columns in bank                                       */
  Tag_Rows,                                  /* Num Rows callback                                     */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};
