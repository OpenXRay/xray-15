/*
 * IMPStandardBanks-LightPropertiesFlags.c
 */

#include <stdio.h>
#include "IMPStandard.h"

/*
 * LightCaustics_Ghost()
 *  Ghosts the cell if the ID points to anything except a light that isn't distant.
 */
int LightCaustics_Ghost( int column, int row, LWItemID id ) {
  if( (item_info->type( id ) != LWI_LIGHT) || (row != 0) )
    return IMPGHOST_BLANK;

  return ((light_info->type( id ) != LWLIGHT_DISTANT) ? IMPGHOST_ENABLED : IMPGHOST_BLANK );
}

/*
 *  Light Flags
 */
int          light_flag_bits[]     = { LWLFL_NO_DIFFUSE, LWLFL_NO_SPECULAR, LWLFL_NO_CAUSTICS, LWLFL_NO_OPENGL, LWLFL_LENS_FLARE, LWLFL_VOLUMETRIC };
const char * light_flag_commands[] = { "AffectDiffuse",  "AffectSpecular",  "AffectCaustics",  "AffectOpenGL",  "LensFlare",      "VolumetricLighting" };

void * LightFlags_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = light_info->flags( id );
  value_int &= light_flag_bits[ column ];
  if( column < 4 )
    value_int = !value_int;

  return &value_int;
}

void * LightFlags_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *(int *)value;

  if( apply ) {
    int test = light_info->flags( id )  & light_flag_bits[ column ];
    if( column < 4 )
      value_int = !value_int;

    if( ((value_int == 0) && (test =! 0)) ||
        ((value_int != 0) && (test == 0)) ) {

      char buffer[ 100 ];
      sprintf( buffer, "SelectItem %x", id );
      command( buffer );

      command( light_flag_commands[ column ] );
    }
  }

  return &value_int;
}

IMPColumn col_AffectDiffuse = {
  "Affect Diffuse",                          /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Affect Diffuse, Light Properites",        /* Comment      */
  NULL,                                      /* No envelope function    */
  Light_Ghost,                               /* Ghosted function        */
  LightFlags_Query,                          /* Query function          */
  LightFlags_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_AffectSpecular = {
  "Affect Specular",                         /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Affect Specular, Light Properites",       /* Comment      */
  NULL,                                      /* No envelope function    */
  Light_Ghost,                               /* Ghosted function        */
  LightFlags_Query,                          /* Query function          */
  LightFlags_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_AffectCaustics = {
  "Affect Caustics",                         /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Affect Caustics, Light Properites",       /* Comment      */
  NULL,                                      /* No envelope function    */
  LightCaustics_Ghost,                       /* Ghosted function        */
  LightFlags_Query,                          /* Query function          */
  LightFlags_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_AffectOpenGL = {
  "Affect OpenGL",                           /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Affect OpenGL, Light Properites",         /* Comment      */
  NULL,                                      /* No envelope function    */
  Light_Ghost,                               /* Ghosted function        */
  LightFlags_Query,                          /* Query function          */
  LightFlags_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_LensFlare = {
  "Lens Flare",                              /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Lens Flare, Light Properites",            /* Comment      */
  NULL,                                      /* No envelope function    */
  Light_Ghost,                               /* Ghosted function        */
  LightFlags_Query,                          /* Query function          */
  LightFlags_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

IMPColumn col_Volumetrics = {
  "Volumetric",                              /* title                   */
  COLWIDTH_TOGGLE,                           /* default width in pixels */
  IMPCOLTYPE_TOGGLE,                         /* column type             */
  "Volumetric Lighting, Light Properites",   /* Comment      */
  NULL,                                      /* No envelope function    */
  Light_Ghost,                               /* Ghosted function        */
  LightFlags_Query,                          /* Query function          */
  LightFlags_Evaluate,                       /* Evaluate function       */
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
IMPColumn *col_lightPropertiesFlags[] = {
  &col_AffectDiffuse,
  &col_AffectSpecular,
  &col_AffectCaustics,
  &col_AffectOpenGL,
  &col_LensFlare,
  &col_Volumetrics,
  NULL };

IMPBank bank_lightPropertiesFlags = {
  MakeBankID( '_', 'L', 'P', 'F' ),          /* id:  Standard (_) Light Properties: Flags             */
  "Light Properties: Flags",                 /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_lightPropertiesFlags,                  /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};


