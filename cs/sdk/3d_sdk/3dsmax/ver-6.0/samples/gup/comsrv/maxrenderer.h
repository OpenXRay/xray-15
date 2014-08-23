//-----------------------------------------------------------------------------
// ------------------------
// File ....: MaxRenderer.h
// ------------------------
// Author...: Gus J Grubba
// Date ....: September 1998
//
// IMaxRenderer Interface
//
//-----------------------------------------------------------------------------
      
#ifndef __MAXRENDERER_H_
#define __MAXRENDERER_H_

#include "resource.h"

//-- MAX Include files

#include <Max.h>
#include <bmmlib.h>
#include <guplib.h>
#include "mscom.h"

class CamListImp;
class maxRndProgressCB;

typedef struct tagEventProxys {
	IStream*	pStream;
	IDispatch*	p;
} EventProxys;

//-----------------------------------------------------------------------------
//--  Class Definition --------------------------------------------------------
//-----------------------------------------------------------------------------
// #> CProxy_IMaxRendererEvents
//

template <class T>
class CProxy_IMaxRendererEvents : public IConnectionPointImpl<T, &IID__IMaxRendererEvents, CComDynamicUnkArray> {
public:
	HRESULT Fire_OnEnumCameras(BSTR CameraName) {
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)	{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL) {
				VariantClear(&varResult);
				pvars[0] = CameraName;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnRenderProgress(LONG Done, LONG Total)	{
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[2];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)	{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL) {
				VariantClear(&varResult);
				pvars[1] = Done;
				pvars[0] = Total;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				pDispatch->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnRenderMessage(BSTR Message) {
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		CComVariant* pvars = new CComVariant[1];
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)	{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL) {
				VariantClear(&varResult);
				pvars[0] = Message;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	
	}
	HRESULT Fire_OnRenderDone() {
		CComVariant varResult;
		T* pT = static_cast<T*>(this);
		int nConnectionIndex;
		int nConnections = m_vec.GetSize();
		
		for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)	{
			pT->Lock();
			CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
			pT->Unlock();
			IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
			if (pDispatch != NULL)	{
				VariantClear(&varResult);
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				pDispatch->Invoke(0x4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		return varResult.scode;
	
	}
};

//-----------------------------------------------------------------------------
//--  Class Definition --------------------------------------------------------
//-----------------------------------------------------------------------------
// #> CMaxRenderer
//

class ATL_NO_VTABLE CMaxRenderer : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMaxRenderer, &CLSID_MaxRenderer>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CMaxRenderer>,
	public IDispatchImpl<IMaxRenderer, &IID_IMaxRenderer, &LIBID_COMSRVLib>,
	public CProxy_IMaxRendererEvents< CMaxRenderer >,
	public GUP_MSCOM {
public:
	CMaxRenderer();
	~CMaxRenderer();

DECLARE_REGISTRY_RESOURCEID(IDR_MAXRENDERER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMaxRenderer)
	COM_INTERFACE_ENTRY(IMaxRenderer)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()
BEGIN_CONNECTION_POINT_MAP(CMaxRenderer)
CONNECTION_POINT_ENTRY(IID__IMaxRendererEvents)
END_CONNECTION_POINT_MAP()

	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:

	//-----------------------------------------------------
	//-- IMaxRenderer Public Interface

	STDMETHOD(get_RenderFieldOrder)		(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_RenderFieldOrder)		(/*[in]*/ long newVal);
	STDMETHOD(get_RenderAtmosphere)		(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_RenderAtmosphere)		(/*[in]*/ BOOL newVal);
	STDMETHOD(get_RenderForceTwoSide)	(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_RenderForceTwoSide)	(/*[in]*/ BOOL newVal);
	STDMETHOD(get_RenderHidden)			(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_RenderHidden)			(/*[in]*/ BOOL newVal);
	STDMETHOD(get_RenderSuperBlack)		(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_RenderSuperBlack)		(/*[in]*/ BOOL newVal);
	STDMETHOD(get_RenderColorCheck)		(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_RenderColorCheck)		(/*[in]*/ BOOL newVal);
	STDMETHOD(get_RenderFieldRender)	(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_RenderFieldRender)	(/*[in]*/ BOOL newVal);
	STDMETHOD(get_AnimationEnd)			(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AnimationEnd)			(/*[in]*/ float newVal);
	STDMETHOD(get_AnimationStart)		(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AnimationStart)		(/*[in]*/ float newVal);

	STDMETHOD(ExecuteMAXScriptFile)		(/*[in]*/ BSTR FileName);
	STDMETHOD(ExecuteMAXScriptString)	(/*[in]*/ BSTR String);
	STDMETHOD(GetPreviewLine)			(/*[in]*/ long line, /*[in]*/ long width, /*[out, retval]*/ SAFEARRAY **psa);
	STDMETHOD(GetLine)					(/*[in]*/ MAXchannelTypes type, /*[in]*/ long line, /*[in]*/ BOOL linear, /*[out, retval]*/ SAFEARRAY **psa);
	STDMETHOD(SetRegion)				(/*[in]*/ short x,/*[in]*/ short y,/*[in]*/ short w,/*[in]*/ short h);
	STDMETHOD(CancelRenderer)			(void);
	STDMETHOD(CloseRenderer)			(void);
	STDMETHOD(OpenRenderer)				(/*[in]*/ BSTR CameraName, /*[in]*/ IMaxBitmapInfo* pMap, /*[in]*/ BOOL region);
	STDMETHOD(RenderFrame)				(/*[in]*/ float Time, /*[in]*/ float Duration);
	STDMETHOD(EnumCameras)				(void);
	STDMETHOD(LoadScene)				(/*[in]*/ BSTR SceneName);
	STDMETHOD(SaveScene)				(/*[in]*/ BSTR SceneName);
	STDMETHOD(ImportFile)				(/*[in]*/ BSTR FileName);
	 
	//-----------------------------------------------------
	//-- Local

	STDMETHODIMP	AllocAndGetLine	(long line, BOOL linear, BMM_Color_64 **pix);
	STDMETHODIMP	GetChannel		(long line, long depth, long channel, SAFEARRAY **psa);
	STDMETHODIMP	GetLine24x48	(long line, BOOL linear, BOOL b24, SAFEARRAY **psa);
	STDMETHODIMP	GetBGRLine24x48	(long line, BOOL linear, BOOL b24, SAFEARRAY **psa);
	STDMETHODIMP	GetLine32		(long line, BOOL linear, SAFEARRAY **psa);
	STDMETHODIMP	GetBGRLine32	(long line, BOOL linear, SAFEARRAY **psa);
	STDMETHODIMP	GetBGRLine64	(long line, BOOL linear, SAFEARRAY **psa);
	STDMETHODIMP	GetLine64		(long line, BOOL linear, SAFEARRAY **psa);
	
	void			ReadBitmapInfo	(IMaxBitmapInfo* pBif);
	void			CreateBitmap	( );
	void			DestroyBitmap	( );
	void			RenderThread	( );
	
	//-----------------------------------------------------
	//-- Data

	RECT					region_rect;
	bool					rendering,renderopen,renderregion;
	long					rndtime;
	float					rnddur;
	Bitmap*					rndmap;
	BitmapInfo				bi;
	CamListImp*				clp;
	maxRndProgressCB*		rndCB;
	HANDLE					RenderThreadHandle;
	DWORD					RenderThreadId,image_channels;
	Tab<EventProxys>		aProxy;

	//-----------------------------------------------------
	//-- Threaded Proxys

	HRESULT OnEnumCameras(BSTR CameraName) {
		CComVariant varResult;
		CComVariant* pvars = new CComVariant[1];
		for (int i = 0; i < aProxy.Count(); i++)	{
			if (aProxy[i].p != NULL) {
				VariantClear(&varResult);
				pvars[0] = CameraName;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				aProxy[i].p->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	}
	

	HRESULT OnRenderProgress(LONG Done, LONG Total)	{
		CComVariant varResult;
		CComVariant* pvars = new CComVariant[2];
		for (int i = 0; i < aProxy.Count(); i++)	{
			if (aProxy[i].p != NULL) {
				VariantClear(&varResult);
				pvars[1] = Done;
				pvars[0] = Total;
				DISPPARAMS disp = { pvars, NULL, 2, 0 };
				aProxy[i].p->Invoke(0x2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	}
	
	HRESULT OnRenderMessage(BSTR Message) {
		CComVariant varResult;
		CComVariant* pvars = new CComVariant[1];
		for (int i = 0; i < aProxy.Count(); i++)	{
			if (aProxy[i].p != NULL) {
				VariantClear(&varResult);
				pvars[0] = Message;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				aProxy[i].p->Invoke(0x3, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		delete[] pvars;
		return varResult.scode;
	}
	
	HRESULT OnRenderDone() {
		CComVariant varResult;
		for (int i = 0; i < aProxy.Count(); i++)	{
			if (aProxy[i].p != NULL) {
				VariantClear(&varResult);
				DISPPARAMS disp = { NULL, NULL, 0, 0 };
				aProxy[i].p->Invoke(0x4, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &varResult, NULL, NULL);
			}
		}
		return varResult.scode;
	}


};

//-----------------------------------------------------------------------------
//--  Class Definition --------------------------------------------------------
//-----------------------------------------------------------------------------
// #> CamListImp
//

class CamListImp: public ITreeEnumProc {
	public:
		TCHAR			name[128];
		CMaxRenderer*	cmax;
		INode*			camNode;
		bool			fire_back;
		CamListImp	( ) {Reset();}
		int				callback	( INode* node );
		void			Reset		( ) { 
			camNode		= NULL;
			cmax		= NULL;
			fire_back	= true;
		}
};

//-----------------------------------------------------------------------------
//--  Class Definition --------------------------------------------------------
//-----------------------------------------------------------------------------
// #> maxRendProgressCallback
//

class maxRndProgressCB : public RendProgressCallback {
	public:
		void	SetTitle		( const TCHAR *title );
		int		Progress		( int done, int total );
		CMaxRenderer* cmax;
		bool	abort;
};

#endif //__MAXRENDERER_H_

//-- EOF: MaxRenderer.h -------------------------------------------------------
