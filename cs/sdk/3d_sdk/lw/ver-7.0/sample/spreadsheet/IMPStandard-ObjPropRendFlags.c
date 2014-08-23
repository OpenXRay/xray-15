/*
 *  IMPStandardBanks-ObjectPropertiesRenderFlags
 */

#include "IMPStandard.h"

#include <stdio.h>

/*
 * FogLevel_Ghost()
 *  Ghosts the cell if the ID points to anything except a light that isn't distant.
 */
int FogLevel_Ghost( int column, int row, LWItemID id ) {
  if( (item_info->type( id ) != LWI_OBJECT) || (row != 0) )
    return IMPGHOST_BLANK;

  return ((object_info->flags( id ) & LWOBJF_UNAFFECT_BY_FOG) ? IMPGHOST_BLANK : IMPGHOST_ENABLED);
}


/*
 *  Shadow Flags
 */
const int    obj_shadow_bits[]     = { LWOSHAD_SELF, LWOSHAD_CAST, LWOSHAD_RECEIVE };
const char * obj_shadow_commands[] = { "SelfShadow", "CastShadow", "ReceiveShadow" };

void * ObjShadowFlags_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->shadowOpts( id ) & obj_shadow_bits[ column ] ;
  return &value_int;
}

void * ObjShadowFlags_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = (int)(object_info->shadowOpts( id ) & obj_shadow_bits[ column ] );

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( obj_shadow_commands[ column ] );
    }
  }

  return &value_int;
}

IMPColumn col_SelfShadow = {
  "Self Shadow",                             /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Self Shadowing, Object Properties, Rendering", /* Comment            */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjShadowFlags_Query,                      /* Query function          */
  ObjShadowFlags_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_CastShadows = {
  "Cast Shadows",                            /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Cast Shadows, Object Properties, Rendering", /* Comment              */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjShadowFlags_Query,                      /* Query function          */
  ObjShadowFlags_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_ReceiveShadows = {
  "Receive Shadows",                         /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Receive Shadows, Object Properties, Rendering", /* Comment           */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjShadowFlags_Query,                      /* Query function          */
  ObjShadowFlags_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Object Flags
 */
const int    obj_flag_bits[]     = { LWOBJF_UNSEEN_BY_RAYS, LWOBJF_UNSEEN_BY_CAMERA };
const char * obj_flag_commands[] = { "UnseenByRays", "UnseenByCamera" };

void * ObjFlags_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->flags( id ) & obj_flag_bits[ (column - 3) ] ;
  return &value_int;
}

void * ObjFlags_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = (int)(object_info->flags( id ) & obj_flag_bits[ (column - 3) ]);

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( obj_flag_commands[ (column - 3) ] );
    }
  }

  return &value_int;
}

IMPColumn col_UnseenByRays = {
  "Unseen by Rays",                          /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Unseen by Rays, Object Properties, Rendering", /* Comment            */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjFlags_Query,                            /* Query function          */
  ObjFlags_Evaluate,                         /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_UnseenByCamera = {
  "Unseen by Camera",                        /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Unseen by Camera, Object Properties, Rendering", /* Comment          */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjFlags_Query,                            /* Query function          */
  ObjFlags_Evaluate,                         /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Unaffected By Fog
 */
void * ObjNoFog_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = object_info->flags( id ) & LWOBJF_UNAFFECT_BY_FOG;
  return &value_int;
}

void * ObjNoFog_Evaluate( int column, int row, LWItemID id, LWTime time, void *value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = object_info->flags( id ) & LWOBJF_UNAFFECT_BY_FOG;

    if( ((value_int == 0) && (test != 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( "UnaffectedByFog" );       /* TODO:  Fix this when this command is available */
    }
  }

  return &value_int;
}

IMPColumn col_UnaffectedByFog = {
  "Unaffected by Fog",                       /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Unaffected by Fog, Object Properties, Rendering", /* Comment         */
  NULL,                                      /* No envelope function    */
  Object_Ghost,                              /* Ghosted function        */
  ObjNoFog_Query,                            /* Query function          */
  ObjNoFog_Evaluate,                         /* Evaluate function       */
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
IMPColumn *col_objectPropertiesRenderFlags[] = {
  &col_SelfShadow,
  &col_CastShadows,
  &col_ReceiveShadows,
  &col_UnseenByRays,
  &col_UnseenByCamera,
  &col_UnaffectedByFog,
  NULL };

IMPBank bank_objectPropertiesRenderFlags = {
  MakeBankID( '_', 'O', 'P', 'F' ),          /* id:  Standard (_) Object Properties: render Flags     */
  "Object Properties: Render Flags",         /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_objectPropertiesRenderFlags,           /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};
