/*
 * IMPGlobal.h
 */

#ifndef IMP_GLOBAL_HEADER
#define IMP_GLOBAL_HEADER

#include <lwpanel.h>
#include <lwrender.h>

#define IMP_GLOBAL "SpreadsheetSceneManagerGlobal"

#define MakeBankID( a, b, c, d )   ((long)(((a) << 24) | ((b) << 16) | ((c) << 8) | d))  /* Example:  long id = MakeBankID( 'T' 'E' 'S' 'T' ); */

/*
 *  The following are predefined column types.  IDs starting with undescores
 *   are reserved for future use.  If you want to make a custom type, be sure
 *   to provide a custom interface and custom draw callbank.
 */
                                                               /* Datatype   - Description                                                                                      */
#define IMPCOLTYPE_TOGGLE    MakeBankID( '_', 'T', 'G', 'L' )  /*  int       -  Checkbox or similar toggle (Unseen By Camera, Cast Shadows)                                     */
#define IMPCOLTYPE_INTEGER   MakeBankID( '_', 'I', 'N', 'T' )  /*  int       -  Integer based control (Subpatch Level, Area Light Quality                                       */
#define IMPCOLTYPE_PERCENT   MakeBankID( '_', 'P', 'C', 'T' )  /*  double    -  Floating point number displayed as a percentage.  1.0 is 100% (Light Intensity, Blur Lrngth)    */
#define IMPCOLTYPE_DISTANCE  MakeBankID( '_', 'D', 'S', 'T' )  /*  double    -  Floating point number displayed as a distance in meters (Focal Distance, Light Range)           */
#define IMPCOLTYPE_ANGLE     MakeBankID( '_', 'A', 'N', 'G' )  /*  double    -  Floating point number displayed as an angle in degrees (Heading Limits, Light Cone Angle)       */
#define IMPCOLTYPE_TIME      MakeBankID( '_', 'S', 'E', 'C' )  /*  double    -  Floating point number displayed as a time in seconds                                            */
#define IMPCOLTYPE_FLOAT     MakeBankID( '_', 'F', 'L', 'T' )  /*  double    -  Floating point number (Anti-Aliasing Threshold, Metaball Resolution)                            */
#define IMPCOLTYPE_LIST      MakeBankID( '_', 'L', 'S', 'T' )  /*  int       -  Pre-determined list of items.  Must provide list callbacks (Item Visibility, Subdivision Order) */
#define IMPCOLTYPE_FILE      MakeBankID( '_', 'F', 'I', 'L' )  /*  char *    -  Text string representing a file (PFX File)                                                      */
#define IMPCOLTYPE_STRING    MakeBankID( '_', 'S', 'T', 'R' )  /*  char *    -  Text string (Item Name, Comment)                                                                */
#define IMPCOLTYPE_COLOR     MakeBankID( '_', 'C', 'L', 'R' )  /*  float[3]  -  RGB Color Triple (Light Color, Edge Color)                                                      */
#define IMPCOLTYPE_ITEM      MakeBankID( '_', 'I', 'T', 'M' )  /*  LWItemID  -  ID of an item (IK Goal, Parent).                                                                */
#define IMPCOLTYPE_IMAGE     MakeBankID( '_', 'I', 'M', 'G' )  /*  LWImageID -  ID of an image (Projection Image)                                                               */
#define IMPCOLTYPE_VMAP      MakeBankID( '_', 'V', 'M', 'P' )  /*  IMPVMap * -  Name of a VMAP (Bone Weight Map).   testItem() should return a zero-terminated arrow of allowed VMap types (ie: { LWVMAP_WGHT, 0 }) if the test value is LWI_NULL */

typedef struct st_IMPVMap {      /* Used by IMPCOLTYPE_VMAPs, since both a type and a name are needed to identify a VMap */
  LWID         type;             /* VMap Type.  See lwmeshes.h for common types.                                         */
  const char * name;             /* VMap Name.                                                                           */
} IMPVMap;

typedef enum en_IMPGhostModes {  /* Your ghosted() function should return one of these values:                                                             */
  IMPGHOST_ENABLED,              /* Draws the cell normally                                                                                                */
  IMPGHOST_DISABLED,             /* Draws a disabled (ghosted) cell                                                                                        */
  IMPGHOST_LABEL,                /* Draws the cell normally, but it cannot be selected.  You could consider this a "read only" cell                        */
  IMPGHOST_BLANK,                /* Draws a blank (and unselectable) cell.  Useful if the column is only relevant to objects and this ID points to a light */
} IMPGhostModes;

#define  IMPENV_NOT_ENVELOPABLE  ((void **)0xFFFFFFFF)   /* This cell cannot be enveloped (Linear/Area Light Quality)                                                             */
#define  IMPENV_NOT_ENVELOPED    ((void **)NULL)         /* This cell can be enveloped, but currently is not                                                                      */

typedef enum en_IMPEnvRequests { /* Your envelope() function will get one of these requests:                                                              */
  IMPENVREQ_STATE,               /* Request the enveloped state.  This should be one of IMPENV_NOT_ENVELOPABLE, IMPENV_NOT_ENVELOPED or IMPENV_ENVELOPED  */
  IMPENVREQ_APPLY,               /* This cell can be enveloped, but currently is not.  Return IMPENV_NOT_ENVELOPED or IMPENV_ENVELOPED                    */
  IMPENVREQ_REMOVE,              /* This cell is currently enveloped.  Return IMPENV_NOT_ENVELOPED or IMPENV_ENVELOPED                                    */
} IMPEnvRequests;

/*
 * IMPColumn:
 *  Provides details about a specific column within a bank
 */
typedef struct st_IMPColumn {
  char *               title;                                                                                   /* Column Title.  If NULL, the custom draw function will be called with row index of -1.                 */
  int                  default_width;                                                                           /* Default width of the column in pixels                                                                 */

  long                 type;                                                                                    /* Column Type.  Use MakeBankID() to generate one, or use one of the predefined column types above       */
  char *               comment;                                                                                 /* Column Comment.  This will be displayed on the status line when a cell is selected (someday)          */

  LWChannelID *        (*envelope)    ( int column, int row, void * id, IMPEnvRequests request );               /* Envelope Function.  IMP handles enveloped cells differently from normal cells.  Can be NULL if not envelopable   */

  IMPGhostModes        (*ghosted)     ( int column, int row, void * id );                                       /* Ghosted Function.  IMP calls this to decide how to draw the control.  This should return one of the IMPGhostModes.  Can be NULL if always enabled                                          */
  void *               (*query)       ( int column, int row, void * id, LWTime time );                          /* Query Function.  IMP calls this to get the current state of this cell                                                                                                                      */
  void *               (*evaluate)    ( int column, int row, void * id, LWTime time, void * value, int apply ); /* Evaluate Function.  IMP calls this to change the value of a cell.  Can be NULL if never returns IMPGHOST_ENABLE                                                                            */
  int                  (*compare)     ( int column, void *value1, void *value2 );                               /* Comparision Function.  IMP calls this when sorting by column.  Return value is simialr to QSort:  0 means identical, -1 means value1 < value2, and 1 means value1 > value2.  Can be NULL   */

  int                  (*listCount)   ( int column, int row, void * id );                                       /* For IMPCOLTYPE_LIST's, returns the number of items in the list; for other types this can be NULL.  If item is LWI_NULL, return all possible items                                          */
  const char *         (*listName)    ( int column, int row, void * id, int index );                            /* For IMPCOLTYPE_LIST's, returns the name of the index item in the list; for other types this can be NULL.  If item is LWI_NULL or row is -1, return a complete list of all possible options */

  int                  (*testItem)    ( int column, int row, void * applied_id, void * queried_id );            /* For IMPCOLTYPE_ITEM, IMPCOLTYPE_IMAGE and IMPCOLTYPE_VMAP's, used to ask if this item id is OK for a particular item.  Use this to limit the item list to only lights, for example         */

  int                  (*customDraw)  ( int column, void *value, LWRasterID raster, int x, int y, int w, int h, IMPGhostModes ghosting );  /* Provides a raster that you can draw a custom icon in.  Return 0 to have IMP draw the control itself; otherwise, return the optimal width for the column (for autosizing purposes).  You can safely draw left of x and right w (it will be clipped by the column edge), but not above y or below y+h (it will corrupt other rows). */

  void                 (*jumpTo)      ( int column, int row, void * id );                                          /* Jump To function.  This should present the standard interface for editing this option.  For example, a Controller and Limits option could open the Motion Options panel */

  LWXPanelID           customPanel;                                                                                /* XPanel interface used by IMPCOLTYPE_CUSTOM columns; others can leave this blank                          */
} IMPColumn;

typedef enum en_IMPBaseModes {   /* The column's base should be one of these */
  IMPBASE_ITEM,                  /* Item-specific                            */
  IMPBASE_CHANNEL                /* Channel-specific                         */
} IMPBaseModes;

/*
 * IMPBank:
 *  Provides details about a bank
 */
typedef struct st_IMPBank {
  long           id;                                             /* Bank ID.  Use the MakeBankID() macro to convert a string into an ID.                     */
  char          *title;                                          /* Bank Title, as displayed in the Bank Popup                                               */

  IMPBaseModes   base;                                           /* Bank base.  Determines what the bank acts on (ie: items or columns)                      */
  IMPColumn    **columns;                                        /* NULL-terminated array of columns in the bank                                             */

  int          (*numRows      ) ( long bank_id, void * id );     /* Num Rows Function.  IMP calls this to ask how many rows this bank needs for this item ID */

  void         (*beginProcess ) ( long bank_id );                /* Called before IMP calls any query/evaluate functions.  Use this to get bank-specific globals, init arrays, etc.       */
  void         (*endProcess   ) ( long bank_id );                /* Called after IMP has called all the query/evalute functions.  Use this to free whatever you got during beginProcess() */
} IMPBank;

/*
 * IMPGlobal:
 *  This is returned by the IMP Global for use by your plug-in
 */
typedef struct st_IMPGlobal {
  int (*registerBank)         ( IMPBank *bank );                 /* Register a new bank with IMP.  You cannot register the same ID twice          */
  int (*unregisterBank)       ( long bank_id );                  /* Unregister an existing bank from IMP.                                         */

  int (*registerBankMaster)   ( const char *master_name );       /* Register a new bank master with IMP.  This plug-in will be launched automatically when IMP is activated.  Returns 1 on succes, -1 if already added and 0 on error */
  int (*unregisterBankMaster) ( const char *master_name );       /* Unregister a bank master.  Returns 1 on succes, -1 if already added and 0 on error                                                                                */

  int (*refresh)              ();                                /* Ask IMP to refresh it's interface                                             */
  int (*command)              ( const char *command_string );    /* Issue a command to IMP.  Returns 0 on error                                   */

} IMPGlobal;

#endif
