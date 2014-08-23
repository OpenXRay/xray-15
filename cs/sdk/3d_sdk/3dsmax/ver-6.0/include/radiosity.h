/**********************************************************************
 *<
	FILE: radiosity.h

	DESCRIPTION: Definitions for radiosity plugin

	CREATED BY: Cleve Ard

	HISTORY:
        [d.levesque | 21August2001]
            Addition of IRadiosityEffectExtension interface.

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/


#ifndef __RADIOSITY_H__
#define __RADIOSITY_H__

#include <render.h>

typedef SFXParamDlg RadiosityParamDlg;

/*=====================================================================
 * Radiosity Interface class
 *===================================================================*/

class RadiosityEffect : public SpecialFX {
public:
	enum CompletionCode {
		PROCESS_COMPLETED,
		PROCESS_TIMED_OUT,
		PROCESS_CANCELED,
		PROCESS_ABORTED
	};

	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	      PartID& partID,  RefMessage message) {return REF_SUCCEED;}
	SClass_ID SuperClassID() {return RADIOSITY_CLASS_ID;}
	
	// Saves and loads name. These should be called at the start of
	// a plug-in's save and load methods.
	IOResult Save(ISave *isave) { return SpecialFX::Save(isave); }
	IOResult Load(ILoad *iload) { return SpecialFX::Load(iload); }

	virtual void SetActive(
		bool		active,
		TimeValue	t
	) {
		if (active ^ (TestAFlag(A_ATMOS_DISABLED) == 0)) {
			if (active) {
				ClearAFlag(A_ATMOS_DISABLED);
			}
			else {
				SetAFlag(A_ATMOS_DISABLED);
			}
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

	// Merge a radiosity solution into the current solution. This
	// method is called for the currently selected radiosity plugin, when
	// merging objects from a file that also has a radiosity plugin. The
	// class id identifies the class of the radiosity plugin being loaded.
	// The default behaviour does not merge the solutions.
	virtual IOResult Merge(ILoad* iload, const Class_ID& id) { return IO_OK; }

	// Put up a modal dialog that lets the user edit the plug-ins parameters.
	virtual RadiosityParamDlg *CreateParamDialog(IRendParams *ip)
		{return NULL;}

	// Implement this if you are using the ParamMap2 AUTO_UI system and the 
	// atmosphere has secondary dialogs that don't have the effect as their 'thing'.
	// Called once for each secondary dialog for you to install the correct thing.
	// Return TRUE if you process the dialog, false otherwise.
	virtual BOOL SetDlgThing(RadiosityParamDlg* dlg) { return FALSE; }

	// Render access

	// Should the light render it's illumination in the production render. This is
	// used to allow the radiosity plugin to supply lighting for lights
	// from its solution.
	virtual bool UseLight(INode* node, bool recalcSolution = false) { return true; }

	// Create light objects that the renderer can use to get the
	// radiosity contribution. NumLightDesc returns the number of
	// ObjLightDesc objects the radiosity plugin needs for
	// rendering. CreateLightDesc creates all of the ObjLightDesc
	// objects, and stores their addresses in buffer. Buffer must be
	// large enough to hold all of the addresses.
	virtual int NumLightDesc( ) const = 0;
	virtual void CreateLightDesc(ObjLightDesc** buffer) = 0;

	// Start the radiosity process running. This method may start
	// the process at the beginning, or where it stopped previously.
	// InteractiveRender is true, when RunProcess is called from an
	// interactive renderer. The radiosity plugin should use this
	// information to trade-off acceptable render quality and interactivity.
	virtual void RunProcess(
		TimeValue				t,
		RenderGlobalContext*	rgc,
		bool					interactiveRender
    ) = 0;

	// Stop the radiosity process. This method stops the global
	// illumination process. This can take quite a while, and if allowAbort is
	// true, may display a modeless dialog to allow the user to abort
	// processing.
	virtual void StopProcess(bool allowAbort = true) = 0;

	// Abort the radiosity process. This method aborts the global
	// illumination process quickly. If AbortProcess is called, RunProcess
	// may start at the beginning the next time it is called.
	virtual void AbortProcess() = 0;

	// Wait for radiosity process to complete. The RendContext is
	// used to provide progress reporting to the user, and to detect when the
	// cancel button is pressed. If the process doesn't complete in timeout
	// milliseconds, it is stopped and WaitForCompletion returns.

	// Completion criteria is set by the user in the radiosity plugin
	// UI. If the user cancels the render, the radiosity plugin should attempt
	// to stop the process, but allow the user to abort, when stopping
	// takes an excessive amount of time.
	virtual CompletionCode WaitForCompletion(
		RendContext*	rc = NULL,
		DWORD		timeout = INFINITE
	) = 0;

	// Called right before RunProcess. Return true to have the camPos
	// array built before running the process, return false to keep
	// the array from being build. SaveMem is true if the renderer
	// would like to save memory. Default returns false.
	virtual bool NeedsCamVerts(
		TimeValue				t,
		RenderGlobalContext*	rgc,
		bool					interactiveRender,
		bool					saveMem
	) { return false; }
};


/*=====================================================================
 * Radisity Core Interface class
 *===================================================================*/

// This class is used to get access to the radiosity plugin
// and UI.
#define RADIOSITY_INTERFACE	Interface_ID(0x6711e7a, 0x5b504baa)

class RadiosityInterface : public FPStaticInterface {
public:
	// Open and close the Radiosity Panel
	virtual void OpenRadiosityPanel()=0;
	virtual void CloseRadiosityPanel()=0;
	virtual void MinimizeRadiosityPanel()=0;

	// Get and Set the radiosity in the scene
	virtual RadiosityEffect* GetRadiosity() const = 0;
	virtual void SetRadiosity(RadiosityEffect* op) = 0;
};


/*=====================================================================
 * Class IRadiosityEffectExtension
 *
 * Provides extended functionality to class RadiosityEffect. To use
 * this functionality with a RadiosityEffect class, derive the class
 * from both RadiosityEffect and IRadiosityEffectExtension, and implement
 * RadiosityEffect::GetInterface() to return a pointer to this interface
 * on request.
 *===================================================================*/

#define IRADIOSITYEFFECT_EXTENSION_INTERFACE Interface_ID(0x703149db, 0x43ed63b8)

class IRadiosityEffectExtension : public BaseInterface {

public:

    // Returns whether the specified default light should be used by the scanline
    // renderer. The scanline renderer normally creates default lights when there
    // are no lights in the scene. A radiosity plugin could override this if it
    // uses objects other than lights as light sources (e.g. self-emitting
    // surfaces)
    virtual bool UseDefaultLight(const DefaultLight& defLight, bool recalcSolution = false) const = 0;

    // Returns whether the the radiosity plugin is interested in any
	// of the channels in the part id.
    virtual bool IsInterestedInChannels(PartID part) const { return true; }

    // -- from BaseInterface
    virtual Interface_ID GetID() { return IRADIOSITYEFFECT_EXTENSION_INTERFACE; }
};

#endif
