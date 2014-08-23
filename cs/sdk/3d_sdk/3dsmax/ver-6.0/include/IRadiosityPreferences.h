/*==============================================================================

  file:	        IRadiosityPreferences.h

  author:       Daniel Levesque
  
  created:	    7 September 2001
  
  description:
        
        Interface for accessing radiosity preferences.


  modified:		September 14, 2001 David Cunningham
						- added use/compute radiosity file setting access

                September 17, 2001 Daniel Levesque
                        - addition of Get/Set 'Save Geometry' methods

				May 20, 2003 Alexandre Cossette
						- addition of Get/Set 'Update Data When Required on Start'

				June 9, 2003 Alexandre Cossette
						- removed 'Display Warning on GI Props Change'


© 2001 Autodesk
==============================================================================*/

#ifndef _IRADIOSITYPREFERENCES_H_
#define _IRADIOSITYPREFERENCES_H_

#include "ifnpub.h"

#define IRADIOSITYPREFERENCES_INTERFACE Interface_ID(0x54442e40, 0x401621a4)

//==============================================================================
// class IRadiosityPreferences
//
// This class defines the interface for accessing the Radiosity Preferences
// from the 'Radiosity' tab in the preferences dialog.
//
// It also provides access to the use/compute radiosity controls found in the
// Rendering dialog.
//
// To get the a pointer to the interface, call:
//     static_cast<IRadiosityPreferences*>(GetCOREInterface(IRADIOSITYPREFERENCES_INTERFACE))
//
//==============================================================================
class IRadiosityPreferences : public FPStaticInterface {

public:

	// Global user settings that are stored in a .ini file

    // Automatically process refine steps stored in objects
    virtual BOOL GetAutoProcessObjectRefine() const = 0;
    virtual void SetAutoProcessObjectRefine(BOOL val) = 0;
    //Display reflectance/transmittance information in the material editor
    virtual BOOL GetDisplayReflectanceInMEditor() const = 0;
    virtual void SetDisplayReflectanceInMEditor(BOOL val) = 0;
    // Display radiosity in the viewport
    virtual BOOL GetDisplayInViewport() const = 0;
    virtual void SetDisplayInViewport(BOOL val) = 0;
    // Display warning on reset
    virtual BOOL GetDisplayResetWarning() const = 0;
    virtual void SetDisplayResetWarning(BOOL val) = 0;
    // Automatically update solution data when required
    virtual BOOL GetUpdateDataWhenRequiredOnStart() const = 0;
    virtual void SetUpdateDataWhenRequiredOnStart(BOOL val) = 0;
    // Save the geometry along with the solution in the .max file, for faster load times.
    virtual BOOL GetSaveScene() const = 0;
    virtual void SetSaveScene(BOOL val) = 0;

    /**
     * The functions below provide access to current radiosity settings.
     * These properties are saved on a per file basis, so they
     * are not written into the .ini file.
     */
    // Use radiosity when rendering
    virtual BOOL GetUseRadiosity() const = 0;
    virtual void SetUseRadiosity(BOOL val) = 0;
    // Automatically compute radiosity when rendering, if necessary.
    virtual BOOL GetComputeRadiosity() const = 0;
    virtual void SetComputeRadiosity(BOOL val) = 0;
};


#endif