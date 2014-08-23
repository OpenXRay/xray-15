/*
 *   IMPStandardBanks.h
 */

#include <lwserver.h>
#include <lwhost.h>
#include <lwmonitor.h>
#include <lwrender.h>
#include <lwio.h>
#include <lwdyna.h>

#include <lwmaster.h>

#include <IMPGlobal.h>

/* LW Comamnd Func, so we can issue commands easilly */
typedef int LWCommandFunc( const char *cmd );

/* Master Class Functions                                                       */
XCALL_ (int)     IMPStandardBanks_Activate( long version, GlobalFunc *_global, void *_local, void *serverData );

LWInstance       IMPStandardBanks_Create(   void *data, void *context, LWError *error );
void             IMPStandardBanks_Destroy(  LWInstance _inst );
LWError          IMPStandardBanks_Copy(     LWInstance _to, LWInstance _from );
LWError          IMPStandardBanks_Load(     LWInstance _inst, const LWLoadState *loader );
LWError          IMPStandardBanks_Save(     LWInstance _inst, const LWSaveState *saver );
const char     * IMPStandardBanks_Describe( LWInstance _inst );

double           IMPStandardBanks_Event(    LWInstance _inst, const LWMasterAccess *access );
unsigned int     IMPStandardBanks_Flags(    LWInstance _inst );

/* Rxternals */
extern GlobalFunc     *global;

extern LWCommandFunc  *command;

extern LWItemInfo     *item_info;
extern LWBoneInfo     *bone_info;
extern LWCameraInfo   *camera_info;
extern LWObjectInfo   *object_info;
extern LWLightInfo    *light_info;
extern LWRasterFuncs  *raster_funcs;
extern LWChannelInfo  *chan_info;

extern int             value_int;
extern double          value_color[3];
extern double          value_float;
extern char            command_buffer[];
extern LWChannelID     value_channels[4];
extern IMPVMap         value_vmap;

/* Banks */
extern IMPBank bank_bonePropertiesBasic;
extern IMPBank bank_bonePropertiesInfluence;
extern IMPBank bank_bonePropertiesEffect;
extern IMPBank bank_objectPropertiesGeometry;
extern IMPBank bank_objectPropertiesMorphing;
extern IMPBank bank_objectPropertiesDisplacement;
extern IMPBank bank_objectPropertiesRenderOptions;
extern IMPBank bank_objectPropertiesRenderFlags;
extern IMPBank bank_objectPropertiesEdges;
extern IMPBank bank_lightPropertiesBase;
extern IMPBank bank_lightPropertiesFlags;
extern IMPBank bank_lightPropertiesTypeSpecific;
extern IMPBank bank_lightPropertiesShadows;
extern IMPBank bank_motionOptionsMisc;
extern IMPBank bank_motionOptionsIK;
extern IMPBank bank_motionLimitsHeading;
extern IMPBank bank_motionLimitsPitch;
extern IMPBank bank_motionLimitsBank;
extern IMPBank bank_itemFlags;
extern IMPBank bank_itemInfo;
extern IMPBank bank_tag;
extern IMPBank bank_channelValues;

#ifdef _DEBUG
  #define ADD_TEST_BANK
#endif
#ifdef ADD_TEST_BANK
  extern IMPBank bank_test;
#endif

/* Constants */
#ifndef PI
#define PI 3.14159265358979323846
#endif

#define COLWIDTH_TOGGLE       50
#define COLWIDTH_LIST        120
#define COLWIDTH_STRING      120
#define COLWIDTH_NUMERIC      65
#define COLWIDTH_NUMERIC_ENV  90
#define COLWIDTH_COLOR       130
#define COLWIDTH_COLOR_ENV   150

/* External Functions */
extern double DegreesToRadians( double degrees );
extern double RadiansToDegrees( double radians );

extern int Bone_Ghost(        int column, int row, LWItemID id );
extern int Camera_Ghost(      int column, int row, LWItemID id );
extern int Object_Ghost(      int column, int row, LWItemID id );
extern int Light_Ghost(       int column, int row, LWItemID id );
extern int RowZeroOnly_Ghost( int column, int row, LWItemID id );

extern LWChannelID * EditSingleEnvelopeOfItem( void * id, IMPEnvRequests request, char *name );
