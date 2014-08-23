// VIZDesignAssetServer.h: Definition of the CVIZDesignAssetServer class
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIZDESIGNASSETSERVER_H__CD9E2B3A_491A_4AB7_971F_D7D8017FAF6E__INCLUDED_)
#define AFX_VIZDESIGNASSETSERVER_H__CD9E2B3A_491A_4AB7_971F_D7D8017FAF6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols
#include "oleidl.h" //JH adding can't find a type lib 
#include <Max.h>
#include <bmmlib.h>
#include <guplib.h>
#include "mscom.h"

/////////////////////////////////////////////////////////////////////////////
// CVIZDesignAssetServer

class CVIZDesignAssetServer : 
	public IDesignAssetServer,
	public IOleItemContainer,
	public CComObjectRoot,
	public CComCoClass<CVIZDesignAssetServer,&CLSID_VIZDesignAssetServer>,
	public GUP_MSCOM
{
public:
	CVIZDesignAssetServer() {}

DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVIZDesignAssetServer)
	COM_INTERFACE_ENTRY(IDesignAssetServer)
	COM_INTERFACE_ENTRY(IParseDisplayName)
	COM_INTERFACE_ENTRY(IOleItemContainer)
	COM_INTERFACE_ENTRY(IOleContainer)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IDataObject, m_spunkDataObject.p)
END_COM_MAP()

//DECLARE_NOT_AGGREGATABLE(CVIZDesignAssetServer) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 

	HRESULT FinalConstruct();
	void	FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_VIZDesignAssetServer)

// IDesignAssetServer
public:

// IParseDisplayName
public:
	STDMETHOD(ParseDisplayName(struct IBindCtx *,unsigned short *,unsigned long *,struct IMoniker ** ))
	{ATLASSERT(0); return E_NOTIMPL;}

// IOleContainer
public:
	STDMETHOD(EnumObjects(unsigned long,struct IEnumUnknown ** ))
	{return E_NOTIMPL;}

	STDMETHOD(LockContainer(int))
	{ATLASSERT(0); return E_NOTIMPL;}

// IOleItemContainer
public:


/*	partial note
	If pszItem names a pseudo-object, your implementation can ignore the dwSpeedNeeded parameter because a pseudo-object is running whenever its container is running. In this case, your implementation can simply query for the requested interface.
*/
	STDMETHOD(GetObject(LPOLESTR,unsigned long,struct IBindCtx *,const struct _GUID &,void ** ));

/*
If pszItem designates a pseudo-object, your implementation should return MK_E_NOSTORAGE, because pseudo-objects do not have their own independent storage. If pszItem designates an embedded object, or a portion of the document that has its own storage, your implementation should return the specified interface pointer on the appropriate storage object. 
*/
	STDMETHOD(GetObjectStorage(LPOLESTR,struct IBindCtx *,const struct _GUID &,void ** ))
	{return MK_E_NOSTORAGE;}

/*
Your implementation of IOleItemContainer::IsRunning should first determine whether pszItem identifies one of the container's objects. If it does not, your implementation should return MK_E_NOOBJECT. If the object is not loaded, your implementation should return S_FALSE. If it is loaded, your implementation can call the OleIsRunning function to determine whether it is running.

If pszItem names a pseudo-object, your implementation can simply return S_OK because a pseudo-object is running whenever its container is running. 
*/
	STDMETHOD(IsRunning(LPOLESTR))
	{return MK_E_NOOBJECT;}






private:
	CComPtr<IUnknown> m_spunkDataObject;
};

#endif // !defined(AFX_VIZDESIGNASSETSERVER_H__CD9E2B3A_491A_4AB7_971F_D7D8017FAF6E__INCLUDED_)
