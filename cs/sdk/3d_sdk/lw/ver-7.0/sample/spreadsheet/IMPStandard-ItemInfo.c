/*
 * IMPStandardBanks-ItemInfo.c
 */

#include "IMPStandard.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*
 * ItemName_Ghost()
 *  Makes a label cells if this is an object.
 */
int ItemName_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 )
    return IMPGHOST_BLANK;

  if( item_info->type( id ) == LWI_OBJECT ) {
    // Handle objects/nulls specially
    LWMeshInfo *mesh_info = object_info->meshInfo( id, 0 );
    if( mesh_info != NULL ) {
      int retval = ( (mesh_info->numPoints( mesh_info ) == 1) || (mesh_info->numPolygons( mesh_info ) == 0) ) ? IMPGHOST_ENABLED : IMPGHOST_DISABLED;

      if( mesh_info->destroy != NULL )
        mesh_info->destroy( mesh_info );

      return retval;
    }
  }

  return IMPGHOST_ENABLED;
}

/*
 * ItemID_Ghost()
 */
int ItemID_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 )
    return IMPGHOST_BLANK;

  return IMPGHOST_LABEL;
}

/*
 * ObjectStats_Ghost()
 *  Makes a label cells if this is an object.
 */
int ObjectStats_Ghost( int column, int row, LWItemID id ) {
  if( (row != 0) || (item_info->type( id ) != LWI_OBJECT) )
    return IMPGHOST_BLANK;

  return IMPGHOST_LABEL;
}

/*
 *  Item Name
 */
void * ItemName_Query( int column, int row, LWItemID id, LWTime time ) {
  static char   buffer[ 512 ];
         char * suffix;
         int    i;

  strcpy( buffer, item_info->name( id ) );

  /* Remove the item clone number, if found */
  suffix = strrchr( buffer, '(' );
  if( (suffix != NULL) && (suffix != buffer) ) {
    if( suffix[-1] == ' ' ) {                   /* There should be a space before the opening paren   */
      for( i=1; suffix[i] != '\0'; i++ ) {
        if( suffix[i] == ')' ) {                /* If a closing paren is found, set the space to NUL  */
          suffix[-1] = '\0';
          break;
        }

        if( !isdigit( suffix[i] ) )             /* If this isn't a digit, break out (only digits should be between the parens) */
          break;
      }
    }
  }

  return buffer; 
}

void * ItemName_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  if( apply ) {
    char buffer[ 512 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "Rename %s", (char *)value );
    command( buffer );
  }

  return value;
}

IMPColumn col_ItemName = {
  "Item Name",                               /* title                   */
  COLWIDTH_STRING,                           /* default width in pixels */
  IMPCOLTYPE_STRING,                         /* column type             */
  "Item Name",                               /* Comment                 */
  NULL,                                      /* No envelope function    */
  ItemName_Ghost,                            /* Ghosted function        */
  ItemName_Query,                            /* Query function          */
  ItemName_Evaluate,                         /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Object Path
 */
void * ObjectPath_Query( int column, int row, LWItemID id, LWTime time ) {
  if( item_info->type( id ) != LWI_OBJECT )
    return NULL;

  return (char *)object_info->filename( id ); 
}

void * ObjectPath_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  if( item_info->type( id ) != LWI_OBJECT )
    return NULL;

  if( apply ) {
    char buffer[ 512 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "ReplaceWithObject %s", (char *)value );
    command( buffer );
  }

  return value;
}

IMPColumn col_ObjectPath = {
  "Object Filename",                         /* title                   */
  COLWIDTH_STRING,                           /* default width in pixels */
  IMPCOLTYPE_FILE,                           /* column type             */
  "Object Filename",                         /* Comment                 */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjectPath_Query,                          /* Query function          */
  ObjectPath_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Item ID
 */
void * ItemID_Query( int column, int row, LWItemID id, LWTime time ) {
  static char buffer[20];
  sprintf( buffer, "%x", id );

  return buffer;
}

IMPColumn col_ItemID = {
  "Item ID",                                 /* title                   */
  70,                                        /* default width in pixels */
  IMPCOLTYPE_STRING,                         /* column type             */
  "Item ID",                                 /* Comment                 */
  NULL,                                      /* No envelope function    */
  ItemID_Ghost,                              /* Ghosted function        */
  ItemID_Query,                              /* Query function          */
  NULL,                                      /* No evaluate function    */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Polygon Count
 */
void * PolygonCount_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->numPolygons( id );
  return &value_int;
}

IMPColumn col_PolygonCount = {
  "Polygon Count",                           /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_INTEGER,                        /* column type             */
  "Polygon Count",                           /* Comment                 */
  NULL,                                      /* No envelope function    */
  ObjectStats_Ghost,                         /* Ghosted function        */
  PolygonCount_Query,                        /* Query function          */
  NULL,                                      /* No evaluate function    */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Point Count
 */
void * PointCount_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->numPoints( id );
  return &value_int;
}

IMPColumn col_PointCount = {
  "Point Count",                             /* title                   */
  COLWIDTH_NUMERIC,                          /* default width in pixels */
  IMPCOLTYPE_INTEGER,                        /* column type             */
  "Point Count",                             /* Comment                 */
  NULL,                                      /* No envelope function    */
  ObjectStats_Ghost,                         /* Ghosted function        */
  PointCount_Query,                          /* Query function          */
  NULL,                                      /* No evaluate function    */
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
IMPColumn *col_itemInfo[] = {
  &col_ItemName,
  &col_ObjectPath,
  &col_PolygonCount,
  &col_PointCount,
  &col_ItemID,
  NULL };

IMPBank bank_itemInfo = {
  MakeBankID( '_', 'I', 'N', 'S' ),          /* id:  Standard (_) Item Names and Statistics           */
  "Item Names and Statistics",               /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_itemInfo,                              /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

