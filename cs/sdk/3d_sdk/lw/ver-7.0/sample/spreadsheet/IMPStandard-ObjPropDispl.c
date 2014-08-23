/*
 * IMPStandardBanks-ObjectDisplacement.c
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 * BoneFalloff_Ghost()
 *  Ghosts the cell if the Bump Displacement if false.
 */
int BoneFalloff_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_OBJECT) )
    return IMPGHOST_BLANK;

  return ((item_info->first( LWI_BONE, id ) == LWITEM_NULL) ? IMPGHOST_DISABLED : IMPGHOST_ENABLED);
}

/*
 *   Bone Falloff Type
 */
const char * bone_falloff_list[] = {
  "Inverse Distance",
  "Inverse Distance ^ 2",
  "Inverse Distance ^ 4",
  "Inverse Distance ^ 8",
  "Inverse Distance ^ 16",
  "Inverse Distance ^ 32",
  "Inverse Distance ^ 64",
  "Inverse Distance ^ 128" };

void * BoneFalloff_Query( int column, int row, LWItemID id, LWTime time ) {
  LWItemID bone;

  bone = item_info->first( LWI_BONE, id );
  if( bone == LWITEM_NULL )
    value_int = -1;
  else
    value_int = bone_info->falloff( id ) - 1;

  return &value_int;
}

void * BoneFalloff_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;
  if( value_int < 0 )  value_int = 0;
  if( value_int > 7 )  value_int = 7;

  if( apply ) {
    LWItemID bone = item_info->first( LWI_BONE, id );
    if( bone != LWITEM_NULL ) {
      char buffer[ 100 ];

      sprintf( buffer, "SelectItem %x", bone );
      command( buffer );

      sprintf( buffer, "BoneFalloffType %d", value_int + 1 );
      command( buffer );
    }
  }

  return &value_int;
}

int BoneFalloff_ListCount( int column, int row, LWItemID id ) {
  return 8;
}

const char * BoneFalloff_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 8) )
    return bone_falloff_list[ index ];

  return "";
}

IMPColumn col_BoneFalloff = {
  "Bone Falloff Type",                       /* title                   */
  135,                                       /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Bane Falloff Type, Bone Properties",      /* Comment                 */
  NULL,                                      /* No envelope function    */
  BoneFalloff_Ghost,                         /* Ghosted function        */
  BoneFalloff_Query,                         /* Query function          */
  BoneFalloff_Evaluate,                      /* Evaluate function       */
  NULL,                                      /* No compare function     */
  BoneFalloff_ListCount,                     /* List Count function     */
  BoneFalloff_ListName,                      /* List Name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};


/*
 *  The Bank
 */
IMPColumn *col_objectPropertiesDisplacement[] = {
  &col_BoneFalloff,
  NULL };

IMPBank bank_objectPropertiesDisplacement = {
  MakeBankID( '_', 'O', 'P', 'D' ),          /* id:  Standard (_) Object Properties: Morphing         */
  "Object Properties: Displacement",         /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_objectPropertiesDisplacement,          /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};
