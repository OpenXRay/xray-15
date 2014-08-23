// VIZDesignAssetServer.cpp : Implementation of CComsrvApp and DLL registration.

#include "stdafx.h"
#include "comsrv.h"
#include "VIZDesignAssetServer.h"
#include "AnimDataObject.h"

/////////////////////////////////////////////////////////////////////////////
//
HRESULT CVIZDesignAssetServer::FinalConstruct()
{
	HRESULT hr;
//	hr = CAnimDataObject::_CreatorClass::CreateInstance(GetControllingUnknown(), IID_IUnknown, (void**) &m_spunkDataObject.p);

	CComAggObject<CAnimDataObject> *pAnimDO;
	hr = CComAggObject<CAnimDataObject>::CreateInstance(GetControllingUnknown(), &pAnimDO);
	if(SUCCEEDED(hr))
	{
		pAnimDO->QueryInterface(IID_IUnknown, (void **)&m_spunkDataObject.p);

		//private initiliazation
		Interface *ip = Max();
		ReferenceTarget *root = ip->GetRootNode();

		RefResult res = pAnimDO->m_contained.MakeRefByID(FOREVER, 0, root);
	}


	return hr;
}

void CVIZDesignAssetServer::FinalRelease()
{
	m_spunkDataObject.Release();
}

STDMETHODIMP CVIZDesignAssetServer::GetObject(LPOLESTR,unsigned long,struct IBindCtx *,const struct _GUID &,void ** )
{
	//strategy
	/*
	Our pseudoobjects are a subset of the animatables in the scene
	Conceptually this method should create or retrieve a COM wrapper around an animatable

	The strings being passed will be of the form NodeHandle|SubAnimIndex

	Initial assumptions:
	we'll cache an instance of the wrapped root node.
	we'll cache an instance of the single hot object.
	Request for a new hot object we'll recycle 
	*/
	HRESULT hr =  MK_E_NOOBJECT;

	return hr;
}


