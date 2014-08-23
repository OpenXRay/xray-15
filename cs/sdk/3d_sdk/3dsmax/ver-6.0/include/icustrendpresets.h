/////////////////////////////////////////////////////////////////////////
//
//
//	Render Preset Interface
//
//	Created 5/14/2003	Tom Burke
//

#ifndef	I_CUSTOM_RENDER_PRESETS_H
#define I_CUSTOM_RENDER_PRESETS_H

// includes
#include "sfx.h"
#include "itargetedio.h"
#include "irenderpresets.h"

//////////////////////////////////////////////////////////////
//
//		Render Presets compatibility interface
//		used by renderers to support renderer specific presets.
//		If the renderer does not support this interface, the entire
//		renderer will be saved.
//
//////////////////////////////////////////////////////////////
class ICustomRenderPresets
{
public:
	virtual int  RenderPresetsFileVersion() = 0;
	virtual BOOL RenderPresetsIsCompatible( int version ) = 0;

	virtual TCHAR * RenderPresetsMapIndexToCategory( int catIndex ) = 0;
	virtual int     RenderPresetsMapCategoryToIndex( TCHAR* category ) = 0;

	virtual int RenderPresetsPreSave( ITargetedIO * root, BitArray saveCategories ) = 0;
	virtual int RenderPresetsPostSave( ITargetedIO * root, BitArray loadCategories ) = 0;
	virtual int RenderPresetsPreLoad( ITargetedIO * root, BitArray saveCategories ) = 0;
	virtual int RenderPresetsPostLoad( ITargetedIO * root, BitArray loadCategories ) = 0;
};




#if 0
//////////////////////////////////////////////////////////////
//
//		RenderElementMgr base interface
//
#define IREND_ELEM_MGR_INTERFACE Interface_ID(0x95791767, 0x17651746)
class IRenderElementMgr : public FPMixinInterface
{
public:

	// called by system to add an element merged from another file
	virtual BOOL AppendMergedRenderElement(IRenderElement *pRenderElement) = 0;
	virtual BOOL AppendMergedRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element

	// adds/removes an instance of IRenderElement to the manager's list
	virtual BOOL AddRenderElement(IRenderElement *pRenderElement) = 0;
	virtual BOOL AddRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element
	virtual BOOL RemoveRenderElement(IRenderElement *pRenderElement) = 0;
	virtual BOOL RemoveRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element
	virtual void RemoveAllRenderElements() = 0;

	// returns number of render elements in manager's list
	virtual int NumRenderElements() = 0;

	// returns pointer to a specific render element in manager's list -- NULL if invalid index
	virtual IRenderElement *GetRenderElement(int index) = 0;

	// sets/gets whether element list should be active during a render
	virtual void SetElementsActive(BOOL elementsActive) = 0;
	virtual BOOL GetElementsActive() const = 0;

	// sets/gets whether elements should be displayed in their own viewer
	virtual void SetDisplayElements(BOOL displayElements) = 0;
	virtual BOOL GetDisplayElements() const = 0;

	// sets/gets whether element list should be exported to Combustion file format
	virtual void SetCombustionOutputEnabled(BOOL combustionOutEnabled) = 0;
	virtual BOOL GetCombustionOutputEnabled() const = 0;

	// sets/gets Combustion output path
	virtual void SetCombustionOutputPath(const TCHAR *combustionOutputPath) = 0;
	virtual void SetCombustionOutputPath(const TSTR& combustionOutputPath) = 0;
	virtual const TSTR& GetCombustionOutputPath() const = 0;
	virtual const TCHAR* GetCombustionOutputPathPtr() const = 0;

#ifndef NO_RENDER_ELEMENTS
	// function publishing
	enum
	{
		fps_AddRenderElement, fps_RemoveRenderElement, fps_RemoveAllRenderElements,
		fps_NumRenderElements, fps_GetRenderElement,
		fps_SetElementsActive, fps_GetElementsActive,
		fps_SetDisplayElements,	fps_GetDisplayElements,
		fps_SetCombustionOutputEnabled, fps_GetCombustionOutputEnabled,
		fps_SetCombustionOutputPath, fps_GetCombustionOutputPath,
		fps_SetRenderElementFilename, fps_GetRenderElementFilename,
	};
#endif // NO_RENDER_ELEMENTS

};
#endif // #ifdef 0

#endif // I_CUSTOM_RENDER_PRESETS_H