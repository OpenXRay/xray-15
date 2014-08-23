//-----------------------------------------------------------------------------
// --------------------------
// File ....: MaxRenderer.cpp
// --------------------------
// Author...: Gus J Grubba
// Date ....: September 1998
//
// Implementation of CMaxRenderer
//
//-----------------------------------------------------------------------------
      
#include "stdafx.h"
#include "Comsrv.h"
#include "MaxRenderer.h"
#ifdef _DEBUG
#include <mmsystem.h>
#endif

#pragma warning(disable:4800)
extern TCHAR *GetString(int id);

//-----------------------------------------------------------------------------
// *> renderThread()
//

void renderThread( PVOID Parameter ) {
	CMaxRenderer* cmax = (CMaxRenderer *)Parameter;
	cmax->RenderThread();
}

//-----------------------------------------------------------------------------
// #> CamListImp::callback()
//

int CamListImp::callback(INode *node) {
	Interface *max = cmax->Max();
	const ObjectState& os = node->EvalWorldState(max->GetTime());
	Object* ob = os.obj;
	if (ob!=NULL) {
		if (ob->SuperClassID()==CAMERA_CLASS_ID) {
			if (fire_back) {
				USES_CONVERSION;
				LPOLESTR b_name = T2OLE(node->GetName());
				cmax->Fire_OnEnumCameras(b_name);
			} else {
				if (!camNode) {
					if (!_tcscmp(node->GetName(),name)) {
						camNode = node;
						return TREE_ABORT;
					}
				}   
			}

		}
	}
	return TREE_CONTINUE;
}

//-----------------------------------------------------------------------------
// #> maxRndProgressCB::SetTitle()
//

void maxRndProgressCB::SetTitle( const TCHAR *title ) {
	if (cmax) {
		USES_CONVERSION;
		LPOLESTR b_text = T2OLE(title);
		cmax->OnRenderMessage(b_text);
	}
}

//-----------------------------------------------------------------------------
// #> maxRndProgressCB::Progress()
//

int maxRndProgressCB::Progress( int done, int total ) {
	if (cmax && total)
		cmax->OnRenderProgress(done,total);
	if (abort)
		return (RENDPROG_ABORT);
	else   
		return (RENDPROG_CONTINUE);
}

//-----------------------------------------------------------------------------
// CMaxRenderer::InterfaceSupportsErrorInfo()

STDMETHODIMP CMaxRenderer::InterfaceSupportsErrorInfo(REFIID riid) {
	static const IID* arr[] = {
		&IID_IMaxRenderer
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++) {
		//JH 7/11/02 Explicitly scoping this to the global namespace
		//This allows the code to compile with the Nov '01 psdk in which the symbols is ambiguous
		if (::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::CMaxRenderer()
//

CMaxRenderer::CMaxRenderer() {
#ifndef RENDER_VER 
	Max()->SetNetServer();
	ShowWindow(MaxWnd(),SW_MINIMIZE);
#endif
	SetWindowLongPtr(MaxWnd(),GWL_STYLE,WS_VISIBLE | WS_CAPTION);
	clp			= NULL;
	rndCB		= NULL;
	rndmap		= NULL;
	rendering	= false;
	renderopen	= false;
	memset(&region_rect,0,sizeof(RECT));
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::~CMaxRenderer()
//

CMaxRenderer::~CMaxRenderer() {
	DestroyBitmap();
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_AnimationStart()

STDMETHODIMP CMaxRenderer::get_AnimationStart(float *pVal){
	Interface *max = Max();
	Interval i = max->GetAnimRange();
	*pVal = ((float)i.Start() / (float)GetTicksPerFrame());
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_AnimationStart()

STDMETHODIMP CMaxRenderer::put_AnimationStart(float newVal){
	Interface *max = Max();
	Interval i = max->GetAnimRange();
	i.SetStart((int)(newVal * (float)GetTicksPerFrame()));
	max->SetAnimRange(i);
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_AnimationEnd()

STDMETHODIMP CMaxRenderer::get_AnimationEnd(float *pVal){
	Interface *max = Max();
	Interval i = max->GetAnimRange();
	*pVal = ((float)i.End() / (float)GetTicksPerFrame());
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_AnimationEnd()

STDMETHODIMP CMaxRenderer::put_AnimationEnd(float newVal) {
	Interface *max = Max();
	Interval i = max->GetAnimRange();
	i.SetEnd((int)(newVal * (float)GetTicksPerFrame()));
	max->SetAnimRange(i);
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_RenderFieldRender()

STDMETHODIMP CMaxRenderer::get_RenderFieldRender(BOOL *pVal){
	Interface *max = Max();
	*pVal = max->GetRendFieldRender();
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_RenderFieldRender()

STDMETHODIMP CMaxRenderer::put_RenderFieldRender(BOOL newVal){
	Interface *max = Max();
	max->SetRendFieldRender(newVal);
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_RenderColorCheck()

STDMETHODIMP CMaxRenderer::get_RenderColorCheck(BOOL *pVal) {
	Interface *max = Max();
	*pVal = max->GetRendColorCheck();
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_RenderColorCheck()

STDMETHODIMP CMaxRenderer::put_RenderColorCheck(BOOL newVal){
	Interface *max = Max();
	max->SetRendColorCheck(newVal);
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_RenderSuperBlack()

STDMETHODIMP CMaxRenderer::get_RenderSuperBlack(BOOL *pVal){
	Interface *max = Max();
	*pVal = max->GetRendSuperBlack();
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_RenderSuperBlack()

STDMETHODIMP CMaxRenderer::put_RenderSuperBlack(BOOL newVal){
	Interface *max = Max();
	max->SetRendSuperBlack(newVal);
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_RenderHidden()

STDMETHODIMP CMaxRenderer::get_RenderHidden(BOOL *pVal){
	Interface *max = Max();
	*pVal = max->GetRendHidden();
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_RenderHidden()

STDMETHODIMP CMaxRenderer::put_RenderHidden(BOOL newVal){
	Interface *max = Max();
	max->SetRendHidden(newVal);
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_RenderForceTwoSide()

STDMETHODIMP CMaxRenderer::get_RenderForceTwoSide(BOOL *pVal){
	Interface *max = Max();
	*pVal = max->GetRendForce2Side();
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_RenderForceTwoSide()

STDMETHODIMP CMaxRenderer::put_RenderForceTwoSide(BOOL newVal){
	Interface *max = Max();
	max->SetRendForce2Side(newVal);
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_RenderAtmosphere()

STDMETHODIMP CMaxRenderer::get_RenderAtmosphere(BOOL *pVal){
	Interface *max = Max();
	*pVal = max->GetRendAtmosphere();
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_RenderAtmosphere()

STDMETHODIMP CMaxRenderer::put_RenderAtmosphere(BOOL newVal){
	Interface *max = Max();
	max->SetRendAtmosphere(newVal);
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::get_RenderFieldOrder()

STDMETHODIMP CMaxRenderer::get_RenderFieldOrder(long *pVal){
	Interface *max = Max();
	*pVal = max->GetRendFieldOrder();
	return S_OK;
}

//-----------------------------------------------------------------------------
// CMaxRenderer::put_RenderFieldOrder()

STDMETHODIMP CMaxRenderer::put_RenderFieldOrder(long newVal){
	Interface *max = Max();
	max->SetRendFieldOrder(newVal);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::LoadScene()
//

STDMETHODIMP CMaxRenderer::LoadScene(BSTR SceneName) {
	LPTSTR filename;
	USES_CONVERSION;
	filename = OLE2T(SceneName);
	Interface *max = Max();
	if (!max->LoadFromFile(filename)) {
		TCHAR errtxt[MAX_PATH];
		wsprintf(errtxt,GetString(IDS_NO_FILE_ERROR),filename);
		return Error(errtxt);
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::SaveScene()
//

STDMETHODIMP CMaxRenderer::SaveScene(BSTR SceneName) {
	LPTSTR filename;
	USES_CONVERSION;
	filename = OLE2T(SceneName);
	Interface *max = Max();
	if (!max->SaveToFile(filename)) {
		TCHAR errtxt[MAX_PATH];
		wsprintf(errtxt,GetString(IDS_NO_FILE_ERROR),filename);
		return Error(errtxt);
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::ImportFile()
//

STDMETHODIMP CMaxRenderer::ImportFile(BSTR FileName) {
	LPTSTR filename;
	USES_CONVERSION;
	filename = OLE2T(FileName);
	Interface *max = Max();
	if (!max->ImportFromFile(filename,TRUE)) {
		TCHAR errtxt[MAX_PATH];
		wsprintf(errtxt,GetString(IDS_NO_FILE_ERROR),filename);
		return Error(errtxt);
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::EnumCameras()
//

STDMETHODIMP CMaxRenderer::EnumCameras() {
	if (rendering)
		return Error(GetString(IDS_MAX_BUSY));
	clp = new CamListImp;
	clp->Reset();
	clp->cmax = this;
	#ifdef _DEBUG
	PlaySound("chimes.wav",NULL,SND_FILENAME|SND_ASYNC);
	#endif
	EnumTree(clp);
	delete clp;
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::RenderFrame()
//

STDMETHODIMP CMaxRenderer::RenderFrame(float Time, float Duration) {

	#ifdef _DEBUG
	PlaySound("chimes.wav",NULL,SND_FILENAME|SND_ASYNC);
	#endif

	if (rendering)
		return Error(GetString(IDS_MAX_BUSY));
	
	if (!renderopen)
		return Error(GetString(IDS_RENDERER_NOT_OPEN));

	if (renderregion) {
		if (!(region_rect.bottom+region_rect.right))
			return Error(GetString(IDS_RECT_NOT_SET));
	}
	
	CreateBitmap();

	Interface *max = Max();
	_ASSERTE(max);

	rndtime = (int)(Time * (float)GetTicksPerFrame());
	rnddur	= Duration;

	//-- Marshal Outgoing Interfaces

	CMaxRenderer* pT = static_cast<CMaxRenderer*>(this);
	int nConnectionIndex;
	int nConnections = m_vec.GetSize();
	for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++) {
		pT->Lock();
		CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
		pT->Unlock();
		IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
		EventProxys ep;
		ep.p = NULL;
		HRESULT hr = CoMarshalInterThreadInterfaceInStream(
			IID_IDispatch,
			pDispatch,
			&ep.pStream);
		_ASSERTE(SUCCEEDED(hr));
		aProxy.Append(1,&ep);
	}

	//-- Launch Render Thread

	SECURITY_ATTRIBUTES SecurityAttributes;
	SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	SecurityAttributes.lpSecurityDescriptor = NULL;  // Use default ACL
	SecurityAttributes.bInheritHandle       = FALSE; // No inheritance

	RenderThreadHandle = CreateThread(
			&SecurityAttributes, 
			0, 
			(LPTHREAD_START_ROUTINE)renderThread,
			(LPVOID)this,
			0, 
			&RenderThreadId);
										 
	if (RenderThreadHandle == NULL) {
		while (aProxy.Count()) {
			aProxy[0].pStream->Release();
			aProxy.Delete(0,1);
			aProxy.Shrink();
		}
		delete rndCB;
		return Error(GetString(IDS_THREAD_ERROR));
	} else
		CloseHandle(RenderThreadHandle);

	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::OpenRenderer()
//

STDMETHODIMP CMaxRenderer::OpenRenderer(BSTR CameraName, IMaxBitmapInfo* pBif, BOOL region) {

	if (renderopen)
		return Error(GetString(IDS_MAX_BUSY));

	renderregion = bool(region);

	Interface *max = Max();
	_ASSERTE(max);

	ReadBitmapInfo(pBif);

	//-- Define Camera

	clp = new CamListImp;
	clp->Reset();
	clp->fire_back	= false;
	clp->cmax		= this;
	USES_CONVERSION;
	LPTSTR a_name = OLE2T(CameraName);
	_tcscpy(clp->name,a_name);
	EnumTree(clp);

	if (!clp->camNode) {
		TCHAR errtxt[MAX_PATH];
		wsprintf(errtxt,GetString(IDS_NO_CAMERA_ERROR),a_name);
		delete clp;
		return Error(errtxt);
	}
	
	int res;

	if (region) {
		short w,h;
		pBif->get_Width(&w);
		pBif->get_Height(&h);
		res = max->OpenCurRenderer(clp->camNode,NULL,RENDTYPE_REGIONCROP,int(w),int(h));
	} else
		res = max->OpenCurRenderer(clp->camNode,NULL);

	if (!res) {
		delete clp;
		return Error(GetString(IDS_OPEN_RENDERER_ERROR));
	}

	delete clp;
	renderopen = true;
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::CloseRenderer()
//

STDMETHODIMP CMaxRenderer::CloseRenderer() {
	if (!renderopen)
		return Error(GetString(IDS_RENDERER_NOT_OPEN));
	if (rendering) {
		rndCB->abort = true;
		int dur = 50;
		while (--dur && rendering)
			Sleep(100);
	}
	renderopen = false;
	Interface *max = Max();
	ATLASSERT(max);
	max->CloseCurRenderer();
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::CancelRenderer()
//

STDMETHODIMP CMaxRenderer::CancelRenderer() {
	if (rendering)
		rndCB->abort = true;
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetPreviewLine()
//

STDMETHODIMP CMaxRenderer::GetPreviewLine(long line, long width, SAFEARRAY **psa) {
	_ASSERTE(psa);
	if (!rndmap)
		return Error(GetString(IDS_BITMAP_NOT_CREATED));
	BMM_Color_64 *pix;
	AllocAndGetLine(line,TRUE,&pix);
	int size = width * 3;
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	_ASSERTE(*psa);
	BYTE *data;
	SafeArrayAccessData(*psa,(void**)&data);
	_ASSERTE(data);
	float step = (float)((float)rndmap->Width() / (float)width);
	for (int i = 0; i < width; i++) {
		BMM_Color_64 *p = pix +	(long)(step * (float)i);
		*data++ = (BYTE)(p->b >> 8);
		*data++ = (BYTE)(p->g >> 8);
		*data++ = (BYTE)(p->r >> 8);
	}
	LocalFree(pix);
	SafeArrayUnaccessData(*psa);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::SetRegion()
//

STDMETHODIMP CMaxRenderer::SetRegion(short x,short y,short w,short h) {
	region_rect.top = y;
	region_rect.left = x;
	region_rect.bottom = y + h;
	region_rect.right = x + w;
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetLine()
//

STDMETHODIMP CMaxRenderer::GetLine(MAXchannelTypes type, long line, BOOL linear, SAFEARRAY **psa) {
	_ASSERTE(psa);
	if (!rndmap)
		return Error(GetString(IDS_BITMAP_NOT_CREATED));
	switch (type) {
		case TYPE_RGB24:
			return GetLine24x48(line,linear,TRUE,psa);
		case TYPE_RGB48:
			return GetLine24x48(line,linear,FALSE,psa);
		case TYPE_RGBA32:
			return GetLine32(line,linear,psa);
		case TYPE_RGBA64:
			return GetLine64(line,linear,psa);
		case TYPE_BGR24:
			return GetBGRLine24x48(line,linear,TRUE,psa);
		case TYPE_BGR48:
			return GetBGRLine24x48(line,linear,FALSE,psa);
		case TYPE_BGRA32:
			return GetBGRLine32(line,linear,psa);
		case TYPE_BGRA64:
			return GetBGRLine64(line,linear,psa);
		case TYPE_Z32:
			return GetChannel(line,sizeof(float),CHAN_Z,psa);
		case TYPE_MTLID8:
			return GetChannel(line,sizeof(BYTE),CHAN_MTL_ID,psa);
		case TYPE_NODEID16:
			return GetChannel(line,sizeof(WORD),CHAN_NODE_ID,psa);
		case TYPE_UV64:
			return GetChannel(line,sizeof(Point2),CHAN_UV,psa);
		case TYPE_NORMAL32:
			return GetChannel(line,sizeof(float),CHAN_NORMAL,psa);
		case TYPE_REALPIXDEP:
			return GetChannel(line,sizeof(RealPixel),CHAN_REALPIX,psa);
		case TYPE_COVERAGE8:
			return GetChannel(line,sizeof(BYTE),CHAN_COVERAGE,psa);
		case TYPE_BGRGB24:
			return GetChannel(line,3,CHAN_BG,psa);
		case TYPE_NODEIDX16:
			return GetChannel(line,sizeof(WORD),CHAN_NODE_RENDER_ID,psa);
	}
	return Error(GetString(IDS_UNDEFINED_TYPE));
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::AllocAndGetLine()
//

STDMETHODIMP CMaxRenderer::AllocAndGetLine(long line, BOOL linear, BMM_Color_64 **pix) {
	_ASSERTE(pix);
	*pix = (BMM_Color_64 *)LocalAlloc(LPTR,rndmap->Width() * sizeof(BMM_Color_64));
	_ASSERTE(*pix);
	if (linear)
		rndmap->GetLinearPixels(0,line,rndmap->Width(),*pix);
	else
		rndmap->GetPixels(0,line,rndmap->Width(),*pix);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetChannel()
//

STDMETHODIMP CMaxRenderer::GetChannel(long line, long depth, long channel, SAFEARRAY **psa) {
	int size = rndmap->Width() * depth;
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	_ASSERTE(*psa);
	BYTE *data;
	SafeArrayAccessData(*psa,(void**)&data);
	_ASSERTE(data);
	memset(data,0,size);
	if (rndmap->ChannelsPresent() & channel) {
		DWORD type;
		BYTE *b = (BYTE *)rndmap->GetChannel(channel,type);
		if (b) {
			BYTE *ptr = b + (line * size);
			memcpy(data,ptr,size);
		}
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetLine24x48()
//

STDMETHODIMP CMaxRenderer::GetLine24x48(long line, BOOL linear, BOOL b24, SAFEARRAY **psa) {
	BMM_Color_64 *pix;
	AllocAndGetLine(line,linear,&pix);
	int size = rndmap->Width() * 3;
	if (!b24)
		size = size << 1;
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	_ASSERTE(*psa);
	BYTE *data;
	SafeArrayAccessData(*psa,(void**)&data);
	_ASSERTE(data);
	BMM_Color_64 *p = pix;
	if (b24) {
		for (int i = 0; i < rndmap->Width(); i++, p++) {
			*data++ = (BYTE)(p->r >> 8);
			*data++ = (BYTE)(p->g >> 8);
			*data++ = (BYTE)(p->b >> 8);
		}
	} else {
		WORD *t = (WORD *)(void *)data;
		for (int i = 0; i < rndmap->Width(); i++, p++) {
			*t++ = p->r;
			*t++ = p->g;
			*t++ = p->b;
		}
	}
	LocalFree(pix);
	SafeArrayUnaccessData(*psa);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetBGRLine24x48()
//

STDMETHODIMP CMaxRenderer::GetBGRLine24x48(long line, BOOL linear, BOOL b24, SAFEARRAY **psa) {
	BMM_Color_64 *pix;
	AllocAndGetLine(line,linear,&pix);
	int size = rndmap->Width() * 3;
	if (!b24)
		size = size << 1;
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	_ASSERTE(*psa);
	BYTE *data;
	SafeArrayAccessData(*psa,(void**)&data);
	_ASSERTE(data);
	BMM_Color_64 *p = pix;
	if (b24) {
		for (int i = 0; i < rndmap->Width(); i++, p++) {
			*data++ = (BYTE)(p->b >> 8);
			*data++ = (BYTE)(p->g >> 8);
			*data++ = (BYTE)(p->r >> 8);
		}
	} else {
		WORD *t = (WORD *)(void *)data;
		for (int i = 0; i < rndmap->Width(); i++, p++) {
			*t++ = p->b;
			*t++ = p->g;
			*t++ = p->r;
		}
	}
	LocalFree(pix);
	SafeArrayUnaccessData(*psa);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetLine32()
//

STDMETHODIMP CMaxRenderer::GetLine32(long line, BOOL linear, SAFEARRAY **psa) {
	BMM_Color_64 *pix;
	AllocAndGetLine(line,linear,&pix);
	int size = rndmap->Width() * 4;
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	_ASSERTE(*psa);
	BYTE *data;
	SafeArrayAccessData(*psa,(void**)&data);
	_ASSERTE(data);
	BMM_Color_64 *p = pix;
	for (int i = 0; i < rndmap->Width(); i++, p++) {
		*data++ = (BYTE)(p->r >> 8);
		*data++ = (BYTE)(p->g >> 8);
		*data++ = (BYTE)(p->b >> 8);
		*data++ = (BYTE)(p->a >> 8);
	}
	LocalFree(pix);
	SafeArrayUnaccessData(*psa);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetBGRLine32()
//

STDMETHODIMP CMaxRenderer::GetBGRLine32(long line, BOOL linear, SAFEARRAY **psa) {
	BMM_Color_64 *pix;
	AllocAndGetLine(line,linear,&pix);
	int size = rndmap->Width() * 4;
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	_ASSERTE(*psa);
	BYTE *data;
	SafeArrayAccessData(*psa,(void**)&data);
	_ASSERTE(data);
	BMM_Color_64 *p = pix;
	for (int i = 0; i < rndmap->Width(); i++, p++) {
		*data++ = (BYTE)(p->b >> 8);
		*data++ = (BYTE)(p->g >> 8);
		*data++ = (BYTE)(p->r >> 8);
		*data++ = (BYTE)(p->a >> 8);
	}
	LocalFree(pix);
	SafeArrayUnaccessData(*psa);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetBGRLine64()
//

STDMETHODIMP CMaxRenderer::GetBGRLine64(long line, BOOL linear, SAFEARRAY **psa) {
	BMM_Color_64 *pix;
	AllocAndGetLine(line,linear,&pix);
	int size = rndmap->Width() * 4;
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	_ASSERTE(*psa);
	BYTE *data;
	SafeArrayAccessData(*psa,(void**)&data);
	_ASSERTE(data);
	BMM_Color_64 *p = pix;
	WORD *t = (WORD *)(void *)data;
	for (int i = 0; i < rndmap->Width(); i++, p++) {
		*t++ = p->b;
		*t++ = p->g;
		*t++ = p->r;
		*t++ = p->a;
	}
	LocalFree(pix);
	SafeArrayUnaccessData(*psa);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::GetLine64()
//

STDMETHODIMP CMaxRenderer::GetLine64(long line, BOOL linear, SAFEARRAY **psa) {
	int size = rndmap->Width() * 8;
	*psa = SafeArrayCreateVector(VT_UI1,0,size);
	_ASSERTE(*psa);
	BMM_Color_64 *pix;
	SafeArrayAccessData(*psa,(void**)&pix);
	_ASSERTE(pix);
	if (linear)
		rndmap->GetLinearPixels(0,line,rndmap->Width(),pix);
	else
		rndmap->GetPixels(0,line,rndmap->Width(),pix);
	SafeArrayUnaccessData(*psa);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::ExecuteMAXScriptFile()
//

STDMETHODIMP CMaxRenderer::ExecuteMAXScriptFile(BSTR FileName) {
	LPTSTR filename;
	USES_CONVERSION;
	filename = OLE2T(FileName);
	if (!ExecuteFileScript(filename))
		return Error(GetString(IDS_SCRIPT_ERROR));
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::ExecuteMAXScriptString()
//

STDMETHODIMP CMaxRenderer::ExecuteMAXScriptString(BSTR String) {
	LPTSTR string;
	USES_CONVERSION;
	string = OLE2T(String);
	if (!ExecuteStringScript(string))
		return Error(GetString(IDS_SCRIPT_ERROR));
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::ReadBitmapInfo()
//

void CMaxRenderer::ReadBitmapInfo(IMaxBitmapInfo* pBif) {
	short s;
	float f;
	pBif->get_Width(&s);
	bi.SetWidth(s);
	pBif->get_Height(&s);
	bi.SetHeight(s);
	pBif->get_Gamma(&f);
	bi.SetGamma(f);
	pBif->get_Aspect(&f);
	bi.SetAspect(f);
	BOOL b;
	pBif->get_ProcessGamma(&b);
	bi.SetFlags(MAP_HAS_ALPHA);
	bi.ResetCustomFlag(0xFFFFFFFF);
	if (b)
		bi.SetCustomFlag(BMM_CUSTOM_GAMMA);
	bi.SetType(BMM_TRUE_64);
	pBif->get_Channels((MAXGBufferFlags *)(void *)&image_channels);
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::CreateBitmap()
//

void CMaxRenderer::CreateBitmap( ) {
	long w,h;
	if (renderregion) {
		w = region_rect.right - region_rect.left + 1;
		h = region_rect.bottom - region_rect.top + 1;
	} else {
		w = bi.Width();
		h = bi.Height();
	}
	if (rndmap) {
		if (rndmap->Width() == w &&
			rndmap->Height() == h)
			return;
		DestroyBitmap();
	}
	BitmapInfo tbi = bi;
	tbi.SetWidth(short(w));
	tbi.SetHeight(short(h));
	rndmap = Bmi()->Create(&tbi);
	_ASSERTE(rndmap);
	if (image_channels)
		rndmap->PrepareGChannels(image_channels);
}

//-----------------------------------------------------------------------------
// #> CMaxRenderer::DestroyBitmap()
//

void CMaxRenderer::DestroyBitmap() {
	if (rndmap) {
		rndmap->DeleteThis();
		rndmap = NULL;
	}
}

//-----------------------------------------------------------------------------
// CMaxRenderer::RenderThread()

void CMaxRenderer::RenderThread() {

    HRESULT hRes = CoInitialize(NULL);
	_ASSERTE(SUCCEEDED(hRes));

	//-- Marshal Outgoing Interfaces
	
	for (int i = 0; i < aProxy.Count(); i++) {
		hRes = CoGetInterfaceAndReleaseStream(aProxy[i].pStream,IID_IDispatch,(void **)&aProxy[i].p);
		_ASSERTE(SUCCEEDED(hRes));
	}

	rendering		= true;
	rndCB			= new maxRndProgressCB;
	rndCB->abort	= false;
	rndCB->cmax		= this;
	Interface *max	= Max();
	_ASSERTE(max);

	RECT *r;
	if (renderregion)
		r = &region_rect;
	else
		r = NULL;

	max->CurRendererRenderFrame(rndtime,rndmap,rndCB,rnddur,NULL,r);
	rendering		= false;

	OnRenderDone();

	while (aProxy.Count()) {
		aProxy.Delete(0,1);
		aProxy.Shrink();
	}

	CoUninitialize();
	delete rndCB;

}

//-- EOF: MaxRenderer.cpp -----------------------------------------------------
