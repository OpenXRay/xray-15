//-----------------------------------------------------------------------------
// -----------------------
// File ....: BitmapInfo.h
// -----------------------
// Author...: Gus J Grubba
// Date ....: September 1998
//
// Declaration of the CMaxBitmapInfo
//
//-----------------------------------------------------------------------------

#ifndef __BITMAPINFO_H_
#define __BITMAPINFO_H_

#include "resource.h"       // main symbols

//-----------------------------------------------------------------------------
//--  Class Definition --------------------------------------------------------
//-----------------------------------------------------------------------------
// CMaxBitmapInfo

class ATL_NO_VTABLE CMaxBitmapInfo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMaxBitmapInfo, &CLSID_MaxBitmapInfo>,
	public IDispatchImpl<IMaxBitmapInfo, &IID_IMaxBitmapInfo, &LIBID_COMSRVLib> {
public:
	CMaxBitmapInfo();

DECLARE_REGISTRY_RESOURCEID(IDR_BITMAPINFO)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMaxBitmapInfo)
	COM_INTERFACE_ENTRY(IMaxBitmapInfo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:

	//-----------------------------------------------------
	//-- IMaxBitmapInfo Public Interface

	STDMETHOD(get_Channels)		(/*[out, retval]*/ MAXGBufferFlags *pVal);
	STDMETHOD(put_Channels)		(/*[in]*/ MAXGBufferFlags newVal);
	STDMETHOD(get_Aspect)		(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Aspect)		(/*[in]*/ float newVal);
	STDMETHOD(get_Gamma)		(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Gamma)		(/*[in]*/ float newVal);
	STDMETHOD(get_ProcessGamma)	(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_ProcessGamma)	(/*[in]*/ BOOL newVal);
	STDMETHOD(get_Height)		(/*[out, retval]*/ short *pVal);
	STDMETHOD(put_Height)		(/*[in]*/ short newVal);
	STDMETHOD(get_Width)		(/*[out, retval]*/ short *pVal);
	STDMETHOD(put_Width)		(/*[in]*/ short newVal);

private:

	//-----------------------------------------------------
	//-- Local

	short	width,height;
	float	gamma,aspect;
	BOOL	process_gamma;
	DWORD	channels;
	
};

#endif //__BITMAPINFO_H_

//-- EOF: BitmapInfo.h --------------------------------------------------------
