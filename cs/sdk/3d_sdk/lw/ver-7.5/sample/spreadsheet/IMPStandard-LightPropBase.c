/*
 * IMPStandardBanks-ObjectPropertiesMorphing.c
 */

#include <stdio.h>
#include <string.h>
#include "IMPStandard.h"

/*
 * Light_Ghost()
 *  Ghosts the cell if the ID points to anything except a light.
 */
int Light_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 )
    return IMPGHOST_BLANK;

  return ((item_info->type( id ) == LWI_LIGHT) ? IMPGHOST_ENABLED : IMPGHOST_BLANK );
}

/*
 *  Light Type
 */
void * LightType_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = light_info->type( id );
  return &value_int;
}

void * LightType_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *(int *)value;

  if( value_int < 0 )  value_int = 0;
  if( value_int > 4 )  value_int = 4;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    switch( value_int ) {
      case 0:  command( "DistantLight" );  break;
      case 1:  command( "PointLight"   );  break;
      case 2:  command( "Spotlight"    );  break;
      case 3:  command( "LinearLight"  );  break;
      case 4:  command( "AreaLight"    );  break;
    }
  }

  return &value_int;
}

int LightType_ListCount( int column, int row, LWItemID id ) {
  return 5;
}

const char *light_type_list[] = {
  "Distant", "Point", "Spot", "Linear", "Area" };

const char * LightType_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 5) )
    return light_type_list[ index ];

  return "";
}

IMPColumn col_LightType = {
  "Light Type",                              /* title                   */
  70,                                        /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Light Type, Light Properties",            /* Comment                 */
  NULL,                                      /* No envelope function    */
  Light_Ghost,                               /* Ghosted function        */
  LightType_Query,                           /* Query function          */
  LightType_Evaluate,                        /* Evaluate function       */
  NULL,                                      /* No compare function     */
  LightType_ListCount,                       /* List count function     */
  LightType_ListName,                        /* List name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom xpanel        */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Light Color
 */
void * LightColor_Query( int column, int row, LWItemID id, LWTime time ) {
  light_info->rawColor( id, time, value_color );
  return &value_color;
}

void * LightColor_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_color[0] = ((double *)value)[0];
  value_color[1] = ((double *)value)[1];
  value_color[2] = ((double *)value)[2];

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "LightColor %g %g %g", value_color[0], value_color[1], value_color[2] );
    command( buffer );
  }

  return &value_color;
}

/*
 * LightColor_Envelope():
 *  This isn't too pretty; it scans the entire channel group for the item
 *   and tries to find a channel with the a matching name, then adds or removes
 *   the channel, if requested.  The return value is a NULL-terminated array of
 *   the channel IDs, or IMPENV_NOT_ENVELOPED if any of the channels used aren't
 *   currently enveloped.  For errors IMPENV_NOT_ENVELOPABLE is returned.  This
 *   return value should represent the current state of the envelopes after an
 *   add/remove requests.
 *  Note this returns a null-terminated array of channel IDs, not a single
 *   channel, or it returns IMPENV_NOT_ENVELOPED or IMPENV_NOT_ENVELOPABLE.
 *   These channels should be considered "linked" like those in a color
 *   control (three channels, but only one "E" button on the interface), or
 *   else just a single channel (like for Object Dissolve).
 */
LWChannelID *LightColor_Envelope( int column, int row, void * id, IMPEnvRequests request ) {
  LWChanGroupID group;
  LWChannelID   channel;
  int           count = 0;

  group = item_info->chanGroup( id );
  if( group == NULL )
    return IMPENV_NOT_ENVELOPABLE;

  value_channels[0] = NULL;
  value_channels[1] = NULL;
  value_channels[2] = NULL;
  value_channels[3] = NULL;

  // Remove the envelope, if requested
  if( request == IMPENVREQ_REMOVE ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    command( "RemoveEnvelope Color.R" );
    command( "RemoveEnvelope Color.G" );
    command( "RemoveEnvelope Color.B" );
    return IMPENV_NOT_ENVELOPED;
  }

  // See if we should add the envelope
  if( request == IMPENVREQ_APPLY ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    command( "AddEnvelope Color.R" );
    command( "AddEnvelope Color.G" );
    command( "AddEnvelope Color.B" );
  }

  for( channel = chan_info->nextChannel( group, NULL ); channel != NULL; channel = chan_info->nextChannel( group, channel ) ) {
    if( (strcmp( chan_info->channelName( channel ), "Color.R" ) == 0) ||
        (strcmp( chan_info->channelName( channel ), "Color.G" ) == 0) ||
        (strcmp( chan_info->channelName( channel ), "Color.B" ) == 0) ) {

      // Default case;  Return the channel ID
      value_channels[ count++ ] = channel;
      if( count > 2 )
        return value_channels;
    }
  }

  // No envelope; return
  return IMPENV_NOT_ENVELOPED;
}

IMPColumn col_LightColor = {
  "Light Color",                             /* title                   */
  COLWIDTH_COLOR_ENV,                       /* default width in pixels */
  IMPCOLTYPE_COLOR,                          /* column type             */
  "Light Color, Light Properties",           /* Comment                 */
  LightColor_Envelope,                       /* Envelope function       */
  Light_Ghost,                               /* Ghosted function        */
  LightColor_Query,                          /* Query function          */
  LightColor_Evaluate,                       /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Light Intensity
 */
void * LightIntensity_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = light_info->intensity( id, time ) * 100.0;
  return &value_float;
}

void * LightIntensity_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_float = *(double *)value;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "LightIntensity %g", value_float/100.0 );
    command( buffer );
  }

  return &value_float;
}

/*
 * LightIntensity_Envelope():
 */
LWChannelID *LightIntensity_Envelope( int column, int row, void * id, IMPEnvRequests request ) {
  return EditSingleEnvelopeOfItem( id, request, "Intensity" );
}

IMPColumn col_LightIntensity = {
  "Light Intensity",                         /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_PERCENT,                        /* column type             */
  "Light Intensity, Light Properties",       /* Comment                 */
  LightIntensity_Envelope,                   /* Envelope function       */
  Light_Ghost,                               /* Ghosted function        */
  LightIntensity_Query,                      /* Query function          */
  LightIntensity_Evaluate,                   /* Evaluate function       */
  NULL,                                      /* No compare function     */
  NULL,                                      /* No list count function  */
  NULL,                                      /* No list name function   */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Light Falloff Type
 */
int LightFalloffType_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 )
    return IMPGHOST_BLANK;

  if( item_info->type( id ) != LWI_LIGHT )
    return IMPGHOST_BLANK;

  return ((light_info->type( id ) == LWLIGHT_DISTANT ) ? IMPGHOST_DISABLED : IMPGHOST_ENABLED );
}

void * LightFalloffType_Query( int column, int row, LWItemID id, LWTime time ) {
  value_int = light_info->falloff( id );

  return &value_int;
}

void * LightFalloffType_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_int = *(int *)value;

  if( value_int < 0 )  value_int = 0;
  if( value_int > 3 )  value_int = 3;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "LightFalloffType %d", value_int );
    command( buffer );
  }

  return &value_int;
}

int LightFalloffType_ListCount( int column, int row, LWItemID id ) {
  return 4;
}

const char *light_falloff_type_list[] = {
  "Off", "Linear", "Inverse Distance", "Inverse Distance ^ 2" };

const char * LightFalloffType_ListName( int column, int row, LWItemID id, int index ) {
  if( (index >= 0) && (index < 4) )
    return light_falloff_type_list[ index ];

  return "";
}

IMPColumn col_LightFalloffType = {
  "Light Falloff Type",                      /* title                   */
  COLWIDTH_LIST,                             /* default width in pixels */
  IMPCOLTYPE_LIST,                           /* column type             */
  "Light Intensity Falloff, Light Properties", /* Comment               */
  NULL,                                      /* No envelope function    */
  LightFalloffType_Ghost,                    /* Ghosted function        */
  LightFalloffType_Query,                    /* Query function          */
  LightFalloffType_Evaluate,                 /* Evaluate function       */
  NULL,                                      /* No compare function     */
  LightFalloffType_ListCount,                /* List count function     */
  LightFalloffType_ListName,                 /* List name function      */
  NULL,                                      /* No test item function   */
  NULL,                                      /* No custom draw function */
  NULL,                                      /* No jump to Function     */
  NULL,                                      /* No custom xpanel        */
};

/*
 *  Light Range
 */
int LightRange_Ghost( int column, int row, LWItemID id ) {
  if( row != 0 )
    return IMPGHOST_BLANK;

  if( item_info->type( id ) != LWI_LIGHT )
    return IMPGHOST_BLANK;

  if( item_info->type( id ) == LWLIGHT_DISTANT )
    return IMPGHOST_DISABLED;

  return ((light_info->falloff( id ) == LWLFALL_OFF ) ? IMPGHOST_DISABLED : IMPGHOST_ENABLED );
}

void * LightRange_Query( int column, int row, LWItemID id, LWTime time ) {
  value_float = light_info->range( id, time );
  /* TODO:  Add envelope support */

  return &value_float;
}

void * LightRange_Evaluate( int column, int row, LWItemID id, LWTime time, void * value, int apply ) {
  value_float = *(double *)value;
  if( value_float < 0.0 )
    value_float = 0.0;

  if( apply ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    /* TODO:  Add envelope support */
    sprintf( buffer, "LightRange %g", value_float );
    command( buffer );
  }

  return &value_float;
}

/*
 * LightRange_Envelope():
 *  This isn't too pretty; it scans the entire channel group for the item
 *   and tries to find a channel with the a matching name, then adds or removes
 *   the channel, if requested.  The return value is a NULL-terminated array of
 *   the channel IDs, or IMPENV_NOT_ENVELOPED if any of the channels used aren't
 *   currently enveloped.  For errors IMPENV_NOT_ENVELOPABLE is returned.  This
 *   return value should represent the current state of the envelopes after an
 *   add/remove requests.
 *  Note this returns a null-terminated array of channel IDs, not a single
 *   channel, or it returns IMPENV_NOT_ENVELOPED or IMPENV_NOT_ENVELOPABLE.
 *   These channels should be considered "linked" like those in a color
 *   control (three channels, but only one "E" button on the interface), or
 *   else just a single channel (like for Object Dissolve).
 */
LWChannelID *LightRange_Envelope( int column, int row, void * id, IMPEnvRequests request ) {
  return EditSingleEnvelopeOfItem( id, request, "Range" );
}

IMPColumn col_LightRange = {
  "Light Range",                             /* title                   */
  COLWIDTH_NUMERIC_ENV,                      /* default width in pixels */
  IMPCOLTYPE_DISTANCE,                       /* column type             */
  "Light Range, Light Properties",           /* Comment                 */
  LightRange_Envelope,                       /* Envelope function       */
  LightRange_Ghost,                          /* Ghosted function        */
  LightRange_Query,                          /* Query function          */
  LightRange_Evaluate,                       /* Evaluate function       */
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
IMPColumn *col_lightPropertiesBase[] = {
  &col_LightType,
  &col_LightColor,
  &col_LightIntensity,
  &col_LightFalloffType,
  &col_LightRange,
  NULL };

IMPBank bank_lightPropertiesBase = {
  MakeBankID( '_', 'L', 'P', 'B' ),          /* id:  Standard (_) Light Properties: Base              */
  "Light Properties: Basic",                 /* Bank Title                                            */
  IMPBASE_ITEM,                              /* Item base type                                        */
  col_lightPropertiesBase,                   /* Columns in bank                                       */
  NULL,                                      /* Num Rows callbanks; Always one row, so set it to NULL */
  NULL,                                      /* No begin process function                             */
  NULL,                                      /* No end process function                               */
};

