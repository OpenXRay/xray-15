/****
 * lwvbshelf.h
 ****
 * COPYRIGHT (C) 1999 NewTek, Inc.
 ****
 */

#ifndef LWVBSHELF_H
#define LWVBSHELF_H

/* Standard */
#include <stdio.h>
#include <stdlib.h>

#include <lwdialog.h>
#include <lwglobsrv.h>
#include <lwtypes.h>


/****
 * Defines
 ****
 */
#define LWBRFUNCS_GLOBAL "LWBrowseRequest"

/****
 * TypeDefs
 ****
 */
typedef struct st_BrowseClient *LWBRCltID;
typedef char **LWBRStrList;

/* callbacks */
typedef int   LWBRPreMultiFunc ( void *userdata, char *fullname,
                                 char *datafile, char **parms );
typedef char *LWBRFileThumFunc ( void *userdata, char *filename,
                                 int *thum_w, int *thum_h );
typedef char *LWBRPreThumFunc  ( void *userdata, int *thum_w,
                                 int *thum_h );
typedef int   LWBRLoadSettings ( void *userdata, char *datafile,
                                 LWBRStrList parms );
typedef int   LWBRShelfClosed  ( void *userdata );

/* services API */
typedef struct st_LWBRFUNCS {
  LWBRCltID (*subscribe)      ( char *str, char *substr, void *userdata,
                                char *path, char *filetype,
                                LWBRFileThumFunc *filethum,
                                LWBRPreThumFunc  *prethum,
                                LWBRPreMultiFunc *premulti,
                                LWBRLoadSettings *loadsettings,
                                LWBRShelfClosed  *closedNotify );
  int     (*unsubscribe)      ( LWBRCltID );
  /* These allow the client to set the callbacks
   * or userdata pointer independently or modify
   * them at a later date
   */
  void    (*setFileThumFunc)  ( LWBRCltID clt, LWBRFileThumFunc *func );
  void    (*setPreThumFunc)   ( LWBRCltID clt, LWBRPreThumFunc  *func );
  void    (*setPreMultiFunc)  ( LWBRCltID clt, LWBRPreMultiFunc *func );
  void    (*setLoadSettings)  ( LWBRCltID clt, LWBRLoadSettings *func );
  void    (*setShelfClosed)   ( LWBRCltID clt, LWBRShelfClosed  *func );
  void    (*setUserData)      ( LWBRCltID clt, void *userdata );
  /* These are used to load/save a preset file
     * using the Visual Browser as a File Requester
     */
  int     (*presetSave)       ( LWBRCltID clt, char *title,
                                char *filename, char *datafile,
                                LWBRStrList parms );
  int     (*presetLoad)       ( LWBRCltID clt, char *title,
                                char *filename, char *datafile,
                                LWBRStrList *parms );
  int     (*presetMulti)      ( LWBRCltID clt, char *title );
  /* The following are used to open and close the non-modal preset shelf.
   * shelfFocus sets focus to the specified client
   */
  void    (*shelfOpen)        ( LWBRCltID clt );
  void    (*shelfClose)       ( LWBRCltID clt );
  int     (*shelfIsOpen)      ( LWBRCltID clt );
  void    (*shelfFocus)       ( LWBRCltID clt );
  /* This adds a preset to the shelf and will trigger a call to
     * the LWBRPreThumFunc (if set) to generate a thumbnail image
     */
  int     (*shelfInclude)     ( LWBRCltID clt, char *datafilename,
                                LWBRStrList parms );
} LWBRFuncs;

/****
 * Generally, preset files are created by the shelf using the Visual
 * Browser shelfInclude method above.
 *
 * Alternatively, LWPresets can be created and accessed by programs
 * using the following API.
 *
 * A preset contains essentially three sections.
 * The first is some "information" about the preset - who created it,
 * the name of the preset, etc. The second section contains an "image"
 * used to preview the preset in the Visual Browser or the Presets Shelf.
 * The final section contains "settings" which makes the preset file a preset.
 *
 * Information ID Strings
 *  The information section consists of strings (char*) and each info string
 *  is identified by one of the LWPST_ISTR_* defined values.
 *    PST_NAME    - Name of the preset (displayed to user)
 *                  and is independent of the actual filename.
 *    PST_CMNT    - A description of the preset (displayed to user)
 *    PST_OID     - "Owner" ID indicating who created the preset
 *    PST_OSUBID  - Owner's Sub-ID which allows a single owner to
 *                  create multiple types of distinct presets.
 *  Any of these info strings can be omitted. However, some filtering and
 *  type checking is performed based on the OID and OSUBID strings and
 *  owners of presets (e.g., plugins) should validate these fields before
 *  blindly attempting to load the preset data.
 *
 * Preview Image:
 *  The preview image is a standard LWImageID and is automatically scaled
 *  if necessary.
 *
 * Preset Settings:
 *  The preset settings is typically the same data which a plugin would load
 *  or save in the plugin's usual load/save operations. This is, in fact,
 *  the recommended methodology of storing preset data. Plugins, for example,
 *  should create temporary load or save states (see lwio.h) and then call
 *  their normal plugin load and save routines.
 *
 *  While not mandatory, it is recommended preset settings be stored in
 *  binary format (LWIO_BINARY) for size and performance reasons.
 *
 *  The settings data is given and obtained from the preset instance
 *  using files.
 *
 *  To obtain the preset settings data, the client calls get_pdata
 *  (with an optional directory location) which returns a filename of
 *  where the settings are stored. It is left to the client to remove
 *  the file when done.
 *
 *  Storing the setting is handled in reverse where the client saves
 *  its settings to a file, gives the preset instance the name of the file.
 *  The client can remove the file immediately.
 *
 * Parameter Options:
 *  Presets can also store a NULL terminated list of parameter
 *  strings. If these exist, users can be given the option of loading
 *  all or only some of the settings in the preset. The prompt_parms
 *  method obtains the user selections and returns only the sub-set of
 *  parameter strings the user enabled. It is then left to the program
 *  to conditionally load only the user selected settings.
 *
 ****
 */

#define LWPSTFUNCS_GLOBAL     "LWPresetFuncs"

#define LWPST_ISTR_NAME       0
#define LWPST_ISTR_CMNT       2
#define LWPST_ISTR_OID        3
#define LWPST_ISTR_OSUBID     4

typedef void *LWPSTID;

typedef struct st_LWPSTFUNCS {
  /* Load methods */
  LWPSTID        (*load)          ( char *filename );
  const char    *(*get_string)    ( LWPSTID pst, int strid );
  LWImageID      (*get_img)       ( LWPSTID pst );
  const char    *(*get_pdata)     ( LWPSTID pst );
    LWBRStrList    (*prompt_parms)  ( LWPSTID pst );

  /* Save functions */
  LWPSTID        (*create)        ();
  void           (*set_string)    ( LWPSTID pst, int istrid, char *name );
  void           (*set_img)       ( LWPSTID pst, LWImageID image );
  void           (*set_pdata)     ( LWPSTID pst, char *datafile );
  void           (*set_parms)     ( LWPSTID pst, LWBRStrList parms );
  void           (*save)          ( LWPSTID pst, char *filename );

  /* Destroy instance generated from either a load or create */
    void           (*destroy)       ( LWPSTID pst );

} LWPSTFuncs;

/* Close the LWVBSHELF_H header */
#endif
