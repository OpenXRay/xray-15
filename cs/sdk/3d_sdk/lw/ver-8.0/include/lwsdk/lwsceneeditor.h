/*
 * LWSDK Header File
 * Copyright 2003, NewTek, Inc.
 *
 * lwsceneeditor.h -- LightWave Scene Editor services
 */

#ifndef LWSDK_SCENEEDITOR_H
#define LWSDK_SCENEEDITOR_H

#include <lwsdk/lwtypes.h>
#include <lwsdk/lwpanel.h>
#include <lwsdk/lwrender.h>

#define LWSCENEEDITOR_GLOBAL "SceneEditorGlobal"

/*
 *  The following are predefined column types.  IDs starting with undescores
 *   are reserved for future use.  If you want to make a custom type, be sure
 *   to provide a custom interface and custom draw callbank.
 */
                                                         /* Datatype   - Description                                                                                      */
#define SECOLTYPE_TOGGLE    LWID_( '_', 'T', 'G', 'L' )  /*  int       -  Checkbox or similar toggle (Unseen By Camera, Cast Shadows)                                     */
#define SECOLTYPE_INTEGER   LWID_( '_', 'I', 'N', 'T' )  /*  int       -  Integer based control (Subpatch Level, Area Light Quality                                       */
#define SECOLTYPE_PERCENT   LWID_( '_', 'P', 'C', 'T' )  /*  double    -  Floating point number displayed as a percentage.  1.0 is 100% (Light Intensity, Blur Lrngth)    */
#define SECOLTYPE_DISTANCE  LWID_( '_', 'D', 'S', 'T' )  /*  double    -  Floating point number displayed as a distance in meters (Focal Distance, Light Range)           */
#define SECOLTYPE_ANGLE     LWID_( '_', 'A', 'N', 'G' )  /*  double    -  Floating point number displayed as an angle in degrees (Heading Limits, Light Cone Angle)       */
#define SECOLTYPE_TIME      LWID_( '_', 'S', 'E', 'C' )  /*  double    -  Floating point number displayed as a time in seconds                                            */
#define SECOLTYPE_FLOAT     LWID_( '_', 'F', 'L', 'T' )  /*  double    -  Floating point number (Anti-Aliasing Threshold, Metaball Resolution)                            */
#define SECOLTYPE_LIST      LWID_( '_', 'L', 'S', 'T' )  /*  int       -  Pre-determined list of items.  Must provide list callbacks (Item Visibility, Subdivision Order) */
#define SECOLTYPE_FILE      LWID_( '_', 'F', 'I', 'L' )  /*  char *    -  Text string representing a file (PFX File)                                                      */
#define SECOLTYPE_STRING    LWID_( '_', 'S', 'T', 'R' )  /*  char *    -  Text string (Item Name, Comment)                                                                */
#define SECOLTYPE_COLOR     LWID_( '_', 'C', 'L', 'R' )  /*  float[3]  -  RGB Color Triple (Light Color, Edge Color)                                                      */
#define SECOLTYPE_ITEM      LWID_( '_', 'I', 'T', 'M' )  /*  LWItemID  -  ID of an item (IK Goal, Parent).                                                                */
#define SECOLTYPE_IMAGE     LWID_( '_', 'I', 'M', 'G' )  /*  LWImageID -  ID of an image (Projection Image)                                                               */
#define SECOLTYPE_VMAP      LWID_( '_', 'V', 'M', 'P' )  /*  SEVMap * -  Name of a VMAP (e.g. Bone Weight Map).
	testItem() should return a zero-terminated arrow of allowed VMap types (ie: { LWVMAP_WGHT, 0 }) if the test value is LWI_NULL
	*/

typedef struct st_SEVMap {      /* Used by SECOLTYPE_VMAPs, since both a type and a name are needed to identify a VMap */
  LWID        type;             /* VMap Type.  See lwmeshes.h for common types.                                         */
  const char *name;             /* VMap Name.                                                                           */
} SEVMap;

typedef enum en_SEColumnRequests { /* Your envelope() function will get one of these requests:                                                              */
  SECOLUMNREQUEST_STATE,               /* Request the enveloped state.  This should be one of SEENV_NOT_ENVELOPABLE, SEENV_NOT_ENVELOPED or SEENV_ENVELOPED     */
  SECOLUMNREQUEST_APPLY,               /* This cell can be enveloped, but currently is not.  Return SEENV_NOT_ENVELOPED or SEENV_ENVELOPED                    */
  SECOLUMNREQUEST_REMOVE,              /* This cell is currently enveloped.  Return SEENV_NOT_ENVELOPED or SEENV_ENVELOPED                                    */
} SEColumnRequests;

/* These flags indicate the behavior and drawing state of a cell (a specific value within a column) */
#define SECELLFLAG_ENABLED (1<<0) /* Cell can be interacted with by user and is editable at the moment */
#define SECELLFLAG_EXISTS (1<<1)  /* Cell exists, rather than blank.  Cells that exist and are enabled can be selected */
#define SECELLFLAG_READONLY (1<<2) /* cell can be viewed and interacted with but it not editable */
#define SECELLFLAG_CANHAVECHANNEL (1<<3) /* cell can have a channel attached (envelopable) */
#define SECELLFLAG_CANHAVETEXTURE (1<<4) /* cell can have a texture applied (texturable) */

/* These are data types for matching with item types internally supported for selection. */
typedef enum en_SEBaseTypes {
  SEBASETYPE_UNKNOWN = 0, /* item should be NULL (not used)*/
  SEBASETYPE_SCENE,	/* item is name of scene (char*) (not used)*/
  SEBASETYPE_ITEM,	/* item is a LWItemID */
  SEBASETYPE_SURFACEGROUP, /* item is name of group (char*) */
  SEBASETYPE_SURFACE, /* item is a LWSurfaceID */
  SEBASETYPE_CHANNELGROUP, /* item is a LWChanGroupID */
  SEBASETYPE_CHANNEL, /* item is a LWChannelID */
  SEBASETYPE_IMAGE, /* item is a LWImageID (not used)*/
  SEBASETYPE_TEXTURECONTEXT, /* item is a LWTxtrContextID (not used)*/
  SEBASETYPE_TEXTURE, /* item is a LWTextureID (not used)*/
  SEBASETYPE_TEXTURELAYER, /* item is a LWTLayerID (not used)*/
  SEBASETYPE_ENVELOPE, /* item is a LWEnvelopeID (not used)*/
  SEBASETYPE_VIEWPORT, /* item is a view port 0-based index (not used)*/
  SEBASETYPE_PARTICLESYSTEM, /* item is a LWPSysID (not used)*/
  SEBASETYPE_AUDIO, /* item is an audio track LWAudioTrackID (not used)*/
  SEBASETYPE_VIDEO, /* item is an video track LWVideoTrackID (not used)*/
} SEBaseTypes;

/* This structure holds an item identifier and its associated value.
 * It is used in the column apply function.
 */
typedef struct st_SEApply {
	void *item;
	void *value;
} SEApply;

/*
 *  Provides details about a specific column within a bank
 *  A column can only operate on one SEBASETYPE
 */
typedef struct st_SEColumn {
  char                *title;
  /* Column Title.*/

  unsigned long        datatype;
  /* Column Type.  Use LWID_() to generate one, or use one of the predefined column types above */
  
  char                *description;                                                                                 
  /* Column Comment.  This will be displayed on the status line when a cell is selected */
  
  LWChannelID *        (*channels)    ( void *item, SEColumnRequests request);
  /* This gains access to an envelope that may be attached to this cell.
   * the return value is a NULL-terminated array of channel ids associated with this cell.
   * The request parameter is used to determine if a channel should be added or removed on simply query the existence of a channel.
   * Channels differ from envelopes in that they also can contain modification plugins and exist within a channel hierarchy.
   * Envelopes are the lower level storage of key frames.  A channel contains an envelope.
   * Color channels are an example of 3 channels in the array.  Most other types will only return zero or one channel.
   * Zero channels can return NULL for the array or NULL as the first element in the array.
   */

  unsigned long        (*flags)       ( void *item);
  /* Flags Function.  SE calls this to decide how to draw the control and how it should interact with the keyboard and mouse.
   * This should return the various listed flag bits.  Can be NULL if always ENABLED and EXISTS and NOT READONLY
   * The id passed in is data dependent on the type of data this column can work with.
   * For 'Layout' types, it is an LWItemID; for 'Channel' types, it is a LWChannelID; for 'Surface' types, it is a LWSurfaceID.
   */
  
  void                 (*query)       ( void *item, LWTime time, void *value);
  /* Query Function.  Scene Editor calls this to get the current value(s) of this cell */

  unsigned long        (*apply)    ( LWTime time, unsigned long numitems, SEApply *items, void (*progress_increment)( void)); 
  /* Apply Function.  Scene Editor calls this to change the value of one or more items in this column.
   * This can occur when multiple cells are selected.  This function pointer can be NULL if the flags() function
   * never returns SECELLFLAG_ENABLED and is thus never evaluated.
   * The items parameter is really an array of item/value combinations.
   * This function is not called when in preview mode.
   * This can be slow if this causes a lot of dependent updates.  It is up to the function to decide the quickest
   * approach to setting the appropriate values.
   * The return value is the number of items successfully altered.
   * So, this would be zero if no items were changed.
   */
  
  int                  (*compare)     ( void *value1, void *value2 );                               
  /* Comparision Function.  Scene Editor calls this when sorting by column.
   * Return value is simialr to QSort:  0 means identical, -1 means value1 < value2, and 1 means value1 > value2
   * Can be NULL
   */

/* choice list construction */
  int                  (*listCount)   ( void *item );
  /* For SECOLTYPE_LIST's, returns the number of items in the list; for other types this can be NULL.
   * If item is LWI_NULL, return all possible items                                          
   */
  
  const char *         (*listName)    ( void *item, int index );                            
  /* For SECOLTYPE_LIST's, returns the name of the index item in the list; for other types this can be NULL.
   * If item is LWI_NULL or row is -1, return a complete list of all possible options
   */

  int                  (*testItem)    ( void *item, void *value);
  /* For SECOLTYPE_ITEM, SECOLTYPE_IMAGE and SECOLTYPE_VMAP's, used to ask if this item is OK for a particular item.
   * Use this to limit the item list to only lights, for example
   * This can also be used to limit the allowed range of a value to between 0.0 to 1.0
   * returns 1 if the quereied item is valid, even if the queried item was altered.
   */

  void                 (*jumpTo)      ( void *item);
  /* Jump To function.  This should present the standard interface for editing this option.
   * For example, a Controller and Limits option could open the Motion Options panel
   */

  int                  (*customDraw)  ( void *value, LWRasterID raster, long left, long top, unsigned long width, unsigned long height, unsigned long flags);
  /* Provides a raster that you can draw custom cell contents within.
   * a value of NULL is used when the column title should be drawn instead of an actual cell.
   * Return 0 to have Scene Editor draw the control itself; otherwise 1,
   */

/*  LWXPanelID           customPanel;*/
  /* XPanel interface used by IMPCOLTYPE_CUSTOM columns; others can leave this blank */

  /* texture access */
  LWTextureID *        (*texture)    ( void *item, SEColumnRequests request);
  /* This gains access to a texture array that may exist for the cell in the column.
   * the request mode can ask to add a texture, remove an existing texture, or find out if a texture currently exists (or is even possible)
   */
} SEColumn;

/*
 *  Provides details about a bank
 *  A bank acts on a specific type of data: item, channel, or surface
 *  Therefore, all columns in a bank are intended to on that same type of data.
 */
typedef struct st_SEBank
{
  unsigned long  id;                                               /* Bank ID.  Use the LWID_() macro to convert a string into an ID.                          */
  char          *category;                                         /* name of category this bank belongs to.  can be NULL.  Banks with same category name are grouped together */
  char          *title;                                            /* Bank Title, for display purposes.  Used in the Bank Popup. Also used to organize column selection */
  SEBaseTypes    base;                                             /* Bank base.  Determines what the bank acts on (layout items, channels, surfaces)          */
  SEColumn     **columns;                                          /* NULL-terminated array of columns in the bank                                             */
/*  int          (*numRows      ) ( unsigned long bank_id, void *id);*//* Num Rows Function.  Scene Editor calls this to ask how many rows this bank needs for this item ID */
  void         (*beginProcess ) ( unsigned long bank_id);          /* Called before Scene Editor calls any query/evaluate functions.  Use this to get bank-specific globals, init arrays, etc. */
  void         (*endProcess   ) ( unsigned long bank_id);          /* Called after Scene Editor has called all the query/evalute functions.  Use this to free whatever you got during beginProcess() */
} SEBank;

/*
 *  This is returned by the Scene Editor Global for use by your plug-in
 *  Banks need to be registered so that the scene editor can manipulate the data in your plug-in server.
 *  Each bank should will on a specific type of data.  Currently, only Layout Items, Channels, and Surfaces are supported.
 *  So, as long as your plug-in data has a direct association with these data types, the scene editor can work with it.
 *  A master bank differs from a normal bank in that it will automatically be applied when the scene editor is first activated (used)
 */
typedef struct st_SceneEditorGlobal
{
	int (*registerBank)           ( SEBank *bank);                 /* Register a new bank with the Scene Editor.  You cannot register the same ID twice          */
	int (*unregisterBank)         ( unsigned long bank_id);        /* Unregister an existing bank from the Scene Editor.                                         */
	int (*registerBankMaster)     ( const char *master_name);      /* Register a new bank master with Scene Editor.  This plug-in will be launched automatically when the scene Editor is first activated.  Returns 1 on succes, -1 if already added and 0 on error */
	int (*unregisterBankMaster)   ( const char *master_name);      /* Unregister a bank master.  Returns 1 on succes, -1 if already added and 0 on error                                                                                */
	unsigned long (*numInstances) ( void);                         /* return number of instances of the Scene Editor running */
	int (*command)                ( const char *command_string, void *command_return_data);   /* Issue a command to the Scene Editor global service.  Returns 0 on error */
	/* the return data is dependent on the command, and it allows the command to return data back to the caller.
	  It can be NULL if not required. */
	/* available commands include:
		SE_Refresh <instance>: Ask a Scene Editor instance to refresh it's interface
		SE_OpenWindow <instance>: attempts to open the Scene Editor instance window.  It will fail if the window does not open, but success if it was already open.
		SE_CloseWindow( instance>: attempts to close the Scene Editor instance window.  It will success if the window was already closed.
	*/
} SceneEditorGlobal;

#endif


