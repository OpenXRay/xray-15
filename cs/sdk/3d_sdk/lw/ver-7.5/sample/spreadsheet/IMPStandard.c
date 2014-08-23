/*
 * IMPStandardBanks.c
 */

#include "IMPStandard.h"
#include <string.h>
#include <stdio.h>

#define IMPSTANDARDBANKS_NAME ".SpreadsheetStandardBanks"

/*
 * Server Record
 */
ServerUserName IMPStandardBanks_Usernames[] = { {"Spreadsheet Standard Banks",  LANGID_USENGLISH | SRVTAG_USERNAME },  { "Spreadsheet Standard Banks", LANGID_USENGLISH | SRVTAG_BUTTONNAME }, { (const char *)NULL } };

ServerRecord ServerDesc[] = {
  { LWMASTER_HCLASS, IMPSTANDARDBANKS_NAME, IMPStandardBanks_Activate },  /* The . in the name hides the plug-in from the list */
  NULL
};

/* Global Variables */
int             instance_count = 0;     /* Number of instances of this are plug-in enabled.  This should usually be 0 or 1. */
unsigned long   sysid;

GlobalFunc     *global         = NULL;  /* Global Function */

IMPGlobal      *imp            = NULL;  /* Misc. Globals */
LWMessageFuncs *message        = NULL;
LWCommandFunc  *command        = NULL;

LWItemInfo     *item_info      = NULL;
LWBoneInfo     *bone_info      = NULL;
LWCameraInfo   *camera_info    = NULL;
LWObjectInfo   *object_info    = NULL;
LWLightInfo    *light_info     = NULL;
LWRasterFuncs  *raster_funcs   = NULL;
LWChannelInfo  *chan_info      = NULL;
LWChannelID     value_channels[4];

/* Return Value Variables (for Query/Process functions) */
int             value_int;
double          value_color[3];
double          value_float;
char            command_buffer[ 512 ];
IMPVMap         value_vmap;

/*
 * IMPStandardBanks_Activate():
 *  Most of this isn't really important; we just want to register ourselves with IMP.
 *   Thus, most of the handlers don't actually do anything.
 */
XCALL_ (int) IMPStandardBanks_Activate( long version, GlobalFunc *_global,
                                        void *_local, void *serverData ) {
  LWMasterHandler *local;

  if(version != LWMASTER_VERSION)
    return(AFUNC_BADVERSION);

  local = (LWMasterHandler *)_local;

  if( local->inst != NULL ) {
    local->inst->create     = IMPStandardBanks_Create;
    local->inst->destroy    = IMPStandardBanks_Destroy;
    local->inst->load       = IMPStandardBanks_Load;
    local->inst->save       = IMPStandardBanks_Save;
    local->inst->copy       = IMPStandardBanks_Copy;
    local->inst->descln     = IMPStandardBanks_Describe;
  }

  if( local->item != NULL ) {
    local->item->useItems   = NULL;
    local->item->changeID   = NULL;
  }

  local->type  = LWMAST_SCENE;
  local->event = IMPStandardBanks_Event;
  local->flags = IMPStandardBanks_Flags;

  global = _global;

  return AFUNC_OK;
}


/*
 * IMPStandardBanks_Create():
 *  This creates a new instance and registers the banks with IMP, and increments
 *   the global instance_count.  If instance_count is already one when this function
 *   is entered, the banks will not be re-registered, but the plug-in will not fail,
 *   which would be distressing to the user.  Technically, it hasn't failed, but
 *   should only be applied once. Duplicate plug-ins will be removed in the event
 *   function.
 */
LWInstance IMPStandardBanks_Create( void *data, void *context, LWError *error ) {
  sysid = ( unsigned long )global( LWSYSTEMID_GLOBAL, GFUSE_TRANSIENT );
  instance_count++;

  /* Only run in Layout  */
  if( (sysid & LWSYS_TYPEBITS) != LWSYS_LAYOUT )
    return (LWInstance)instance_count;

  /* See how many instances are currently applied.  If it's not 1, do nothing */
  if( instance_count > 1 )
    return (LWInstance)instance_count;

  /* Fetch some globals */
  message      = (LWMessageFuncs   *)global( LWMESSAGEFUNCS_GLOBAL,  GFUSE_ACQUIRE );
  command      = (LWCommandFunc    *)global( "LW Command Interface", GFUSE_ACQUIRE );
  imp          = (IMPGlobal        *)global( IMP_GLOBAL,             GFUSE_ACQUIRE );

  item_info    = (LWItemInfo       *)global( LWITEMINFO_GLOBAL,      GFUSE_ACQUIRE );
  bone_info    = (LWBoneInfo       *)global( LWBONEINFO_GLOBAL,      GFUSE_ACQUIRE );
  camera_info  = (LWCameraInfo     *)global( LWCAMERAINFO_GLOBAL,    GFUSE_ACQUIRE );
  object_info  = (LWObjectInfo     *)global( LWOBJECTINFO_GLOBAL,    GFUSE_ACQUIRE );
  light_info   = (LWLightInfo      *)global( LWLIGHTINFO_GLOBAL,     GFUSE_ACQUIRE );
  raster_funcs = (LWRasterFuncs    *)global( LWRASTERFUNCS_GLOBAL,   GFUSE_ACQUIRE );
  chan_info    = (LWChannelInfo    *)global( LWCHANNELINFO_GLOBAL,   GFUSE_ACQUIRE );

  /* Register the banks with IMP */
  imp->registerBank( &bank_bonePropertiesBasic );
  imp->registerBank( &bank_bonePropertiesInfluence );
  imp->registerBank( &bank_bonePropertiesEffect );
  imp->registerBank( &bank_lightPropertiesBase );
  imp->registerBank( &bank_lightPropertiesFlags );
  imp->registerBank( &bank_lightPropertiesTypeSpecific );
  imp->registerBank( &bank_lightPropertiesShadows );
  imp->registerBank( &bank_objectPropertiesGeometry );
  imp->registerBank( &bank_objectPropertiesMorphing );
  imp->registerBank( &bank_objectPropertiesDisplacement );
  imp->registerBank( &bank_objectPropertiesRenderOptions );
  imp->registerBank( &bank_objectPropertiesRenderFlags );
  imp->registerBank( &bank_objectPropertiesEdges );
  imp->registerBank( &bank_motionOptionsMisc );
  imp->registerBank( &bank_motionOptionsIK );
  imp->registerBank( &bank_motionLimitsHeading );
  imp->registerBank( &bank_motionLimitsPitch );
  imp->registerBank( &bank_motionLimitsBank );
  imp->registerBank( &bank_itemFlags );
  imp->registerBank( &bank_itemInfo );
  imp->registerBank( &bank_tag );
  imp->registerBank( &bank_channelValues );

  #ifdef ADD_TEST_BANK
    imp->registerBank( &bank_test );
  #endif

  /* We don't actually allocate anything, but we have to return something, so how about the instance count */
  return (LWInstance) instance_count;
}

/*
 * IMPStandardBanks_Destroy():
 *  This decrements the instance count and releases the globals if no one is
 *   using them anymore (instance_count == 0).
 */
void IMPStandardBanks_Destroy( LWInstance _inst ) {
  instance_count--;
  if( (sysid & LWSYS_TYPEBITS) != LWSYS_LAYOUT )
    return;

  if( instance_count == 0 ) {
    imp->unregisterBank( bank_bonePropertiesBasic.id );
    imp->unregisterBank( bank_bonePropertiesInfluence.id );
    imp->unregisterBank( bank_bonePropertiesEffect.id );
    imp->unregisterBank( bank_lightPropertiesBase.id );
    imp->unregisterBank( bank_lightPropertiesFlags.id );
    imp->unregisterBank( bank_lightPropertiesTypeSpecific.id );
    imp->unregisterBank( bank_lightPropertiesShadows.id );
    imp->unregisterBank( bank_objectPropertiesGeometry.id );
    imp->unregisterBank( bank_objectPropertiesMorphing.id );
    imp->unregisterBank( bank_objectPropertiesDisplacement.id );
    imp->unregisterBank( bank_objectPropertiesRenderOptions.id );
    imp->unregisterBank( bank_objectPropertiesRenderFlags.id );
    imp->unregisterBank( bank_objectPropertiesEdges.id );
    imp->unregisterBank( bank_motionOptionsMisc.id );
    imp->unregisterBank( bank_motionOptionsIK.id );
    imp->unregisterBank( bank_motionLimitsHeading.id );
    imp->unregisterBank( bank_motionLimitsPitch.id );
    imp->unregisterBank( bank_motionLimitsBank.id );
    imp->unregisterBank( bank_itemFlags.id );
    imp->unregisterBank( bank_itemInfo.id );
    imp->unregisterBank( bank_tag.id );
    imp->unregisterBank( bank_channelValues.id );

    #ifdef ADD_TEST_BANK
      imp->unregisterBank( bank_test.id );
    #endif

    imp->refresh();

    global( LWMESSAGEFUNCS_GLOBAL,  GFUSE_RELEASE );
    global( "LW Command Interface", GFUSE_RELEASE );
    global( IMP_GLOBAL,             GFUSE_RELEASE );

    global( LWITEMINFO_GLOBAL,      GFUSE_RELEASE );
    global( LWBONEINFO_GLOBAL,      GFUSE_RELEASE );
    global( LWCAMERAINFO_GLOBAL,    GFUSE_RELEASE );
    global( LWOBJECTINFO_GLOBAL,    GFUSE_RELEASE );
    global( LWLIGHTINFO_GLOBAL,     GFUSE_RELEASE );
    global( LWRASTERFUNCS_GLOBAL,   GFUSE_RELEASE );
    global( LWCHANNELINFO_GLOBAL,   GFUSE_RELEASE );

    message      = NULL;
    command      = NULL;
    imp          = NULL;

    item_info    = NULL;
    object_info  = NULL;
    light_info   = NULL;
    raster_funcs = NULL;
    chan_info    = NULL;
  }
}

/*
 * IMPStandardBanks_Copy():
 *  This doesn't actually do anything (there's nothing to copy), so just return NULL
 */
LWError IMPStandardBanks_Copy( LWInstance _to, LWInstance _from ) {
  return NULL;
}

/*
 * IMPStandardBanks_Load():
 *  This doesn't actually do anything (there's nothing to load), so just return NULL
 */
LWError IMPStandardBanks_Load( LWInstance _inst, const LWLoadState *loader ) {
  return NULL;
}

/*
 * IMPStandardBanks_Save():
 *  This doesn't actually do anything (there's nothing to save), so just return NULL
 */
LWError IMPStandardBanks_Save( LWInstance _inst, const LWSaveState *saver ) {
  return NULL;
}

/*
 * IMPStandardBanks_Describe():
 *  Since this is a hidden plug-in, this line will probably never be seen, but as
 *   long as we're here...
 */
const char * IMPStandardBanks_Describe( LWInstance _inst ) {
  return "IMP Standard Banks";
}

const char * test_commands[] = {
  /* Object Properties:  Geometry */
  "SubdivisionOrder",
  "SubPatchLevel",
  "MetaballResolution",

  /* Object Properties:  Morphing */
  "MorphTarget",
  "MorphAmount",

  /* Bone Properties:  Basic */
  "BoneActive",
  "BoneRestPosition",
  "BoneRestRotation",
  "BoneRestLength",

  /* Bone Properties:  Influence */
  "BoneWeightMapName",
  "BoneWeightMapOnly",
  "BoneNormalization",
  "BoneStrength",
  "BoneLimitedRange",

  /* Object Properties:  Render Flags */
  "SelfShadow",
  "CastShadow",
  "ReceiveShadow",
  "UnseenByRays",
  "UnseenByCamera",

  /* Object Properties:  Render Options */
  "ObjectDissolve",

  /* Object Properties:  Displacement */
  "BoneFalloffType",

  /* Object Properties:  Edges */
  "PolygonEdgeFlags",
  "PolygonEdgeColor",
  "EdgeZScale",

  /* Light Properties:  Basic */
  "DistantLight",
  "PointLight",
  "Spotlight",
  "LinearLight",
  "AreaLight",
  "LightIntensity",
  "LightColor",
  "LightFalloffType",
  "LightRange",

  /* Light Properties:  Flags */
  "AffectDiffuse",
  "AffectSpecular",
  "AffectCaustics",
  "AffectOpenGL",
  "LensFlare",
  "VolumetricLighting",

  /* Light Properties:  Shadows */
  "ShadowType",
  "CacheShadowMap",
  "ShadowMapSize",
  "ShadowMapFuzziness",

  /* Light Properties:  Type */
  "LightQuality",
  "LightConeAngle",
  "LightEdgeAngle",
  "ProjectionImage"

  /* Motion Controllers and Limits */
  "HController",
  "PController",
  "BController",
  "HLimits",
  "PLimits",
  "BLimits",

  /* IK and Modifiers */
  "UnaffectedByIK",
  "GoalItem",
  "GoalStrength",
  "FullTimeIK",

  /* Item Flags */
  "ItemActive",
  "ItemVisibility",
  "ItemLock",
  "ItemColor",

  /* Item Names */
  "Rename",
  "ReplaceWithObject",

  NULL
};

/*
 * IMPStandardBanks_Event():
 *  This function has two purposes:  Detect and remove any duplicate instances
 *   of the IMPStandardBanks Master, and detect specific commands that should
 *   cause IMP's interface to be refreshed.
 */
double IMPStandardBanks_Event( LWInstance _inst, const LWMasterAccess *access ) {
  /* See if we got a command we like */
  int i;
  if( access->eventCode == LWEVNT_COMMAND ) {
    for( i=0; test_commands[i] != NULL; i++ ) {
      if( strncmp( access->eventData, test_commands[i], strlen( test_commands[i] ) ) == 0 ) {
        imp->refresh();
        break;
      }
    }
  }

  /* Find and remove duplicate instances */
  if( instance_count > 1 ) {
    LWItemInfo * iteminfo = (LWItemInfo *)global( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT );
    const char * server_name;
    int          first_found = 0;
    char         buffer[ 256 ];
    int          i;

    // Find all duplicate instances of IMPStandardBanks and remove them
    server_name = (const char *)-1;
    for( i=1; server_name != NULL; i++ ) {
      server_name = (*iteminfo->server)( NULL, LWMASTER_HCLASS, i );
      if( server_name == NULL )
        break;

      if( strcmp( server_name, IMPSTANDARDBANKS_NAME ) == 0 ) {
        if( first_found == 0 ) {
          first_found = 1;
        } else {
          sprintf( buffer, "RemoveServer %s %i", LWMASTER_HCLASS, i );
          (*access->evaluate)( access->data, buffer );
          i--;
        }
      }
    }
  }

  return 0.0;
}

/*
 * IMPStandardBanks_Flags():
 *  As with all Master Flags() functions, this just returns 0
 */
unsigned int IMPStandardBanks_Flags( LWInstance _inst ) {
  return 0;
}

/*
 * EditSingleEnvelopeOfItem():
 *  Used by many of the simpler envelopable columns.
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
extern LWChannelID * EditSingleEnvelopeOfItem( void * id, IMPEnvRequests request, char *name ) {
  LWChanGroupID group;
  LWChannelID   channel;
  int           count = 0;

  group = item_info->chanGroup( id );
  if( group == NULL )
    return IMPENV_NOT_ENVELOPABLE;

  value_channels[0] = NULL;
  value_channels[1] = NULL;

  // Remove the envelope, if requested
  if( request == IMPENVREQ_REMOVE ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "RemoveEnvelope %s", name );
    command( buffer );
    return IMPENV_NOT_ENVELOPED;
  }

  // See if we should add the envelope
  if( request == IMPENVREQ_APPLY ) {
    char buffer[ 100 ];
    sprintf( buffer, "SelectItem %x", id );
    command( buffer );

    sprintf( buffer, "AddEnvelope %s", name );
    command( buffer );
  }

  for( channel = chan_info->nextChannel( group, NULL ); channel != NULL; channel = chan_info->nextChannel( group, channel ) ) {
    if( strcmp( chan_info->channelName( channel ), name ) == 0) {
      value_channels[ 0 ] = channel;
      return value_channels;
    }
  }

  // No envelope; return
  return IMPENV_NOT_ENVELOPED;
}
