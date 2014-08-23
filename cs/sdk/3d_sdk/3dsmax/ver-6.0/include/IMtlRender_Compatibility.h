 /**********************************************************************
 
	FILE:           IMtlRender_Compatibility.h

	DESCRIPTION:    Public interface for determining compatibility
					between a material/texmap and a renderer.

					The interface also allows 3rd party material/map plugins to
					use custom icons in the material/map browser.

	CREATED BY:     Daniel Levesque, Discreet

	HISTORY:        Created 14 April 2003

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/
#ifndef _IMTLRENDER_COMPATIBILITY_H_
#define _IMTLRENDER_COMPATIBILITY_H_

#include <Windows.h>
#include <max.h>
#include <baseinterface.h>

class Renderer;
class MtlBase;

#define IMTLRENDER_COMPATIBILITY_MTLBASE_INTERFACEID Interface_ID(0x5537445b, 0x70a97e02)
#define IMTLRENDER_COMPATIBILITY_RENDERER_INTERFACEID Interface_ID(0x25d24114, 0xdbe505f)

//==============================================================================
// class IMtlRender_Compability_MtlBase
//
// Implementation instructions:
//   To be subclassed by a ClassDesc or ClassDesc2 of a Texmap or Mtl plugin.
//   The subclass needs to call Init(*this) in its constructor.
// 
// Description:
//   This interface is used to determine whether a material/map flags itself as
//   being compatible with a specific renderer plugin. If the material/map flags
//   itself as being compatible with a renderer, then it is deemed compatible with
//   that renderer regardless of what class IMtlRender_Compatibility_Renderer
//   might say.
//
//   Default compatibility: If neither the renderer nor the material/map implements
//   the compatibility interface, they are considered compatible by default.
//
//   Material/map browser icon: This interface may also be used to provide custom
//   icons for the material/map browser.
//
//==============================================================================
class IMtlRender_Compatibility_MtlBase : public FPStaticInterface {

public:

	// Initialization method. MUST be called from the constructor of the subclass. i.e. "Init(*this);".
	void Init(
		ClassDesc& classDesc	// Reference to sub-class instance
	);

	// Returns wethere this material/map is compatible with the given renderer.
	// (Use the class ID of the renderer to determine compatibility).
	virtual bool IsCompatibleWithRenderer(
		ClassDesc& rendererClassDesc		// Class descriptor of a Renderer plugin
	) = 0;

	// Provides custom icons for the material/map browser. Returns true if custom
	// icons are provided.
	// ImageList owner: The derived class has complete ownership over the image list.
	// The derived class handles both creation and deletion of the image list.
	virtual bool GetCustomMtlBrowserIcon(
		HIMAGELIST& hImageList,		// The image list from which the icons are extracted. The images should have a mask.
		int& inactiveIndex,			// Index (into image list) of icon to be displayed when the material/map has the "Show Maps in Viewport" flag turned OFF.
		int& activeIndex,			// Index (into image list) of icon to be displayed when the material/map has the "Show Maps in Viewport" flag turned ON.
		int& disabledIndex			// Index (into image list) of icon to be displayed when the material/map is NOT COMPATIBLE with the current renderer.
	) 
	{ 
		return false; 
	}
};

// Given the class descriptor of a Mtl/Texmap plugin, this returns its compatibility interface (if it exists).
inline IMtlRender_Compatibility_MtlBase* Get_IMtlRender_Compability_MtlBase(ClassDesc& mtlBaseClassDesc) {

	return static_cast<IMtlRender_Compatibility_MtlBase*>(mtlBaseClassDesc.GetInterface(IMTLRENDER_COMPATIBILITY_MTLBASE_INTERFACEID));
}

//==============================================================================
// class IMtlRender_Compatibility_Renderer
//
// Implementation instructions:
//   To be subclassed by a ClassDesc or ClassDesc2 of a Renderer plugin.
//   The subclass needs to call Init(*this) in its constructor.
// 
// Description:
//   This interface is used to determine whether a Renderer flags itself as
//   being compatible with a specific material/map plugin. If the Renderer flags
//   itself as being compatible with a material/map, then it is deemed compatible with
//   that material/map regardless of what class IMtlRender_Compatibility_MtlBase
//   might say.
//
//   Default compatibility: If neither the renderer nor the material/map implements
//   the compatibility interface, they are considered compatible by default.
//
//==============================================================================
class IMtlRender_Compatibility_Renderer : public FPStaticInterface  {

public:

	// Initialization method. MUST be called from the constructor of the subclass. i.e. "Init(*this);".
	void Init(
		ClassDesc& classDesc	// Reference to sub-class instance
	);

	// Returns wethere this material/map is compatible with the given renderer.
	// (Use the class ID of the renderer to determine compatibility).
	virtual bool IsCompatibleWithMtlBase(
		ClassDesc& mtlBaseClassDesc		// Class descriptor of Mtl or Texmap plugin
	) = 0;
};

// Given the class descriptor of a Renderer plugin, this returns its compatibility interface (if it exists).
inline IMtlRender_Compatibility_Renderer* Get_IMtlRender_Compatibility_Renderer(ClassDesc& rendererClassDesc) {

	return static_cast<IMtlRender_Compatibility_Renderer*>(rendererClassDesc.GetInterface(IMTLRENDER_COMPATIBILITY_RENDERER_INTERFACEID));
}

////////////////////////////////////////////////////////////////////////////////
// bool AreMtlAndRendererCompatible(ClassDesc&, ClassDesc&)
//
// Returns whether the given Mtl/Texmap plugin is compatible with the given
// Renderer plugin. Always use this function to determine compatiblity.
//
inline bool AreMtlAndRendererCompatible(ClassDesc& mtlBaseClassDesc, ClassDesc& rendererClassDesc) {

	IMtlRender_Compatibility_MtlBase* mtlBaseCompatibility = Get_IMtlRender_Compability_MtlBase(mtlBaseClassDesc);
	IMtlRender_Compatibility_Renderer* rendererCompatibility = Get_IMtlRender_Compatibility_Renderer(rendererClassDesc);

	if((mtlBaseCompatibility == NULL) && (rendererCompatibility == NULL)) {
		// No compatibility info: compatible by default
		return true;
	}
	else if((mtlBaseCompatibility != NULL) && mtlBaseCompatibility->IsCompatibleWithRenderer(rendererClassDesc)) {
		// Material says it's compatible with the renderer: compatible
		return true;
	}
	else if((rendererCompatibility != NULL) && rendererCompatibility->IsCompatibleWithMtlBase(mtlBaseClassDesc)) {
		// Renderer says it's compatible with the material: compatible
		return true;
	}
	else {
		// Neither material nor renderer says it's compatible: incompatible
		return false;
	}
}

//==============================================================================
// class IMtlRender_Compability_MtlBase inlined methods
//==============================================================================

inline void IMtlRender_Compatibility_MtlBase::Init(ClassDesc& classDesc) {

	LoadDescriptor(IMTLRENDER_COMPATIBILITY_MTLBASE_INTERFACEID, _T("IMtlRender_Compability_MtlBase"), 0, &classDesc, 0, end);
}

//==============================================================================
// class IMtlRender_Compatibility_Renderer inlined methods
//==============================================================================

inline void IMtlRender_Compatibility_Renderer::Init(ClassDesc& classDesc) {

	LoadDescriptor(IMTLRENDER_COMPATIBILITY_RENDERER_INTERFACEID, _T("IMtlRender_Compability_Renderer"), 0, &classDesc, 0, end);
}

#endif