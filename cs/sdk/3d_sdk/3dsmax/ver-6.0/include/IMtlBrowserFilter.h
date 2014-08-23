 /**********************************************************************
 
	FILE:           IMtlBrowserFilter.h

	DESCRIPTION:    Public interface for filtering the contents of the
					material/map browser.

	CREATED BY:     Daniel Levesque, Discreet

	HISTORY:        Created 23 February 2003

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#ifndef _IMTLBROWSERFILTER_H_
#define _IMTLBROWSERFILTER_H_

#include <Windows.h>
#include <max.h>
#include <ifnpub.h>
#include <baseinterface.h>

#define IMTLBROWSERFILTER_MANAGER_INTERFACEID Interface_ID(0x371b4b10, 0x6c715bbc)
#define IMTLBROWSERFILTER_INTERFACEID Interface_ID(0x1d1f513b, 0x6f315a24)
#define ISUBMTLMAP_BROWSERFILTER_INTERFACEID Interface_ID(0x17304fc3, 0x1ad25941)

class IMtlBrowserFilter;
class MtlBase;
class ClassDesc;

//==============================================================================
// class IMtlBrowserFilter_Manager
//
// This is the manager for the mtl/map browser filters. It is used to add/remove
// and access filters for the mtl/map browser.
//==============================================================================
class IMtlBrowserFilter_Manager : public FPStaticInterface {

public:

	// Add a filter to the list. Returns false if filter could not be added because duplicate.
	virtual bool AddFilter(IMtlBrowserFilter& filter) = 0;
	// Remove a filter from the list. Returns false if filter could not be removed because not found.
	virtual bool RemoveFilter(IMtlBrowserFilter& filter) = 0;

	// Query the list of filters
	virtual int GetNumberFilters() = 0;
	virtual IMtlBrowserFilter* GetFilter(unsigned int i) = 0;
	// Returns whether the i-th filter is enabled
	virtual bool FilterEnabled(unsigned int i) = 0;
	// Enables/disables the i-th filter
	virtual void EnableFilter(unsigned int i, bool enable) = 0;
	// Get the name of the i-th filter
	virtual const TCHAR* GetFilterName(unsigned int i) = 0;

	// Run the given material/map through all the filters in the list, returning
	// true if and only if all filters accept to include that material/map.
	virtual bool Include(
		MtlBase& mtlBase,	// The material/texmap to be filtered
		DWORD flags			// The browser flags, as passed to Interface::DoMaterialBrowseDlg()
	) = 0;

	// Run the given class descriptor through all the filters in the list,
	// returning true if and only if all filters accept to include that classdesc.
	virtual bool Include(
		ClassDesc& classDesc,	// The material/map class descriptor to be filtered
		DWORD flags				// The browser flags, as passed to Interface::DoMaterialBrowseDlg()
	) = 0;
};

inline IMtlBrowserFilter_Manager* Get_IMtlBrowserFilter_Manager() {

	return static_cast<IMtlBrowserFilter_Manager*>(GetCOREInterface(IMTLBROWSERFILTER_MANAGER_INTERFACEID));
};

//==============================================================================
// class IMtlBrowserFilter
//
// This is the base class for all mtl/map browser filters.
//==============================================================================
class IMtlBrowserFilter : public BaseInterface {

public:

	// Returns the name of the filter
	virtual const TCHAR* FilterName() = 0;

	// Used to enable/disable this filter. The filter manager will not call
	// disabled filters.
	virtual bool Enabled() = 0;
	virtual void Enable(bool enable) = 0;

	// Called when this filter is added to the manager
	virtual void Registered() = 0;
	// Called when a filter is removed from the manager
	virtual void Unregistered() = 0;

	// Returns whether the given material/map should be included in the browser,
	// when browsing for existing materials/maps.
	virtual bool Include(
		MtlBase& mtlBase,	// The material/texmap to be filtered
		DWORD flags			// The browser flags, as passed to Interface::DoMaterialBrowseDlg()
	) = 0;

	// Returns whether the given material/map should be included in the browser,
	// when browsing for 'new'.
	virtual bool Include(
		ClassDesc& classDesc,	// The material/map class descriptor to be filtered
		DWORD flags				// The browser flags, as passed to Interface::DoMaterialBrowseDlg()
	) = 0;

	// -- from BaseInterface
	virtual Interface_ID GetID();
};

//==============================================================================
// class ISubMtlMap_BrowserFilter
//
// A Mtl/Texmap plugin which wishes to specify filters for its submaps/submaterials
// may implement this interface. The material editor will use the filters returned
// by this interface when browsing for sub materials or texmaps.
//==============================================================================
class ISubMtlMap_BrowserFilter : public BaseInterface {

public:

	// Returns a filter for the i-th sub-texmap (indices are identical to
	// those used with class ISubMap).
	virtual IMtlBrowserFilter* GetSubMapFilter(unsigned int i) = 0;

	// Returns a filter for the i-th sub-material (indices are identical to
	// those used with class Mtl). Note that this is only useful for Mtl plugins.
	// Texmap plugins should return NULL.
	virtual IMtlBrowserFilter* GetSubMtlFilter(unsigned int i) = 0;
	
	// -- from BaseInterface
	virtual Interface_ID GetID();

};

inline ISubMtlMap_BrowserFilter* Get_ISubMtlMap_BrowserFilter(InterfaceServer* iserver) {

	return static_cast<ISubMtlMap_BrowserFilter*>(iserver->GetInterface(ISUBMTLMAP_BROWSERFILTER_INTERFACEID));
}

//==============================================================================
// class IMtlBrowserFilter inlined methods
//==============================================================================

inline Interface_ID IMtlBrowserFilter::GetID() {

	return IMTLBROWSERFILTER_INTERFACEID;
}

//==============================================================================
// class ISubMapBrowserFilter inlined methods
//==============================================================================

inline Interface_ID ISubMtlMap_BrowserFilter::GetID() {

	return ISUBMTLMAP_BROWSERFILTER_INTERFACEID;
}

#endif