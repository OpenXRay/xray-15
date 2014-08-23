// MaxClientDlg.cpp : implementation file
//

#include "stdafx.h"

#include "../../../../include/udmIA64.h"
// WIN64 Cleanup: Shuler
// only contains datatypes to complete the Unified Data Model
// Remove this once MSVC70 is out.


#include "MaxClient.h"
#include "MaxClientDlg.h"
#include "..\..\..\..\include\buildver.h"
#include "resourceOverride.h"
extern CMaxClientApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NOCAMERA	"No Cameras" 

#define PREV_WIDTH		320
#define PREV_HEIGHT		240

#define OUTPUT_WIDTH	640
#define OUTPUT_HEIGHT	480

//#define RENDER_REGION

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD){
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::CMaxClientDlg()
//

CMaxClientDlg::CMaxClientDlg(CWnd* pParent)	: CDialog(CMaxClientDlg::IDD, pParent) {
	//{{AFX_DATA_INIT(CMaxClientDlg)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	bInitialized	= false;
	bMaxFileLoaded	= false;
	bCameraDefined	= false;
	bRendering		= false;
	pMaxEvents		= NULL;
	pPreview		= NULL;
	hPreview		= NULL;
	preview_curline	= 0;
	cookie			= 0;
	pMax			= NULL;
	pBif			= NULL;
#if defined( DESIGN_VER )
	hDefPreview = NULL;
#endif // DESIGN_VER
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::CMaxClientDlg()
//

CMaxClientDlg::~CMaxClientDlg( ) {
#if defined( DESIGN_VER )
	DeleteObject(hDefPreview);
	hDefPreview = NULL;
#endif // DESIGN_VER
}
	
//-----------------------------------------------------------------------------
// #> CMaxClientDlg::DoDataExchange()
//

void CMaxClientDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMaxClientDlg)
	DDX_Control(pDX, IDC_SERVERNAME, m_ServerControl);
	DDX_Control(pDX, IDC_LOCAL, m_LocalButton);
	DDX_Control(pDX, IDC_CREATE, m_CreateButton);
	DDX_Control(pDX, IDC_PREVIEW_WINDOW, m_PreviewWindow);
	DDX_Control(pDX, IDC_SCRIPTEDIT, m_ScriptTextControl);
	DDX_Control(pDX, IDC_SCRIPT, m_ScriptButtonControl);
	DDX_Control(pDX, IDC_CANCEL_RENDER, m_CancelRenderControl);
	DDX_Control(pDX, IDC_LISTCAMERAS, m_ListCameraControl);
	DDX_Control(pDX, IDC_RENDER_PROGRESS, m_ProgressControl);
	DDX_Control(pDX, IDC_RENDER_MESSAGES, m_MessagesControl);
	DDX_Control(pDX, IDC_CAMERAS, m_CamerasControl);
	DDX_Control(pDX, IDC_RENDER, m_RenderButton);
	DDX_Control(pDX, IDC_MAXFILENAME, m_MaxFileEdit);
	DDX_Control(pDX, IDC_LOAD, m_LoadButton);
	//}}AFX_DATA_MAP
}

//-----------------------------------------------------------------------------
// #> Message Map
//

BEGIN_MESSAGE_MAP(CMaxClientDlg, CDialog)
	//{{AFX_MSG_MAP(CMaxClientDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CREATE, OnCreateInstance)
	ON_BN_CLICKED(IDC_LOAD, OnLoadMaxFile)
	ON_BN_CLICKED(IDC_LISTCAMERAS, OnListcameras)
	ON_BN_CLICKED(IDC_RENDER, OnRender)
	ON_BN_CLICKED(IDC_CANCEL_RENDER, OnCancelRender)
	ON_BN_CLICKED(IDC_SCRIPT, OnScript)
	ON_BN_CLICKED(IDC_LOCAL, OnLocal)
	ON_BN_CLICKED(IDC_REMOTE, OnLocal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnInitDialog()
//

BOOL CMaxClientDlg::OnInitDialog() {
	
	CDialog::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL) {
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
//	SetIcon(m_hIcon, TRUE);
//	SetIcon(m_hIcon, FALSE);
#if defined( DESIGN_VER )
	// Set dlg title
	CString strVIZTitle;
	strVIZTitle.LoadString(IDS_APPTITLE);
	SetWindowText(strVIZTitle);
	// set preview image
	hDefPreview = static_cast<HBITMAP>(LoadImage(theApp.m_hInstance, 
												MAKEINTRESOURCE(IDB_PREVIEW),
												IMAGE_BITMAP,
												0, // desired width
												0, // desired height
												0 ));// load options
	m_PreviewWindow.SetBitmap(hDefPreview);
	m_PreviewWindow.InvalidateRect(NULL);
#endif // DESIGN_VER
	SetClassLongPtr(m_hWnd,GCLP_HICON,(LONG_PTR)m_hIcon);
	
	//-------------------------------------------

	CheckRadioButton(IDC_LOCAL,IDC_REMOTE,IDC_LOCAL);

	UpdateServerEdit();
	ResetCamera();
	UpdateControls();
	return TRUE;
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnSysCommand()
//

void CMaxClientDlg::OnSysCommand(UINT nID, LPARAM lParam) {
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialog::OnSysCommand(nID, lParam);
	}
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnPaint()
//
void CMaxClientDlg::OnPaint() {
	if (IsIconic())	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialog::OnPaint();
	}
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnQueryDragIcon()
//

HCURSOR CMaxClientDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::UpdateServerEdit()
//

void CMaxClientDlg::UpdateServerEdit() {
	BOOL flag = IsDlgButtonChecked(IDC_LOCAL);
	m_ServerControl.EnableWindow(!flag);
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::UpdateServerEdit()
//

void CMaxClientDlg::OnLocal() {
	UpdateServerEdit();	
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::UpdateControls()
//

void CMaxClientDlg::UpdateControls() {
	BOOL flag;
	flag = (BOOL)bInitialized;
	m_CreateButton.EnableWindow(!flag);
	if (flag) {
		m_ServerControl.EnableWindow(FALSE);
		m_LocalButton.EnableWindow(FALSE);
	}
	m_LoadButton.EnableWindow(flag);
	m_ScriptButtonControl.EnableWindow(flag);
	flag = (flag && bMaxFileLoaded);
	m_ListCameraControl.EnableWindow(flag);
	m_CamerasControl.EnableWindow(flag);
	flag = (flag && bMaxFileLoaded && bCameraDefined);
	m_RenderButton.EnableWindow(flag);
	m_CancelRenderControl.EnableWindow((BOOL)bRendering);
}

//-----------------------------------------------------------------------------
// #> CMaxRendererEvents::ResetCamera()
//

void CMaxClientDlg::ResetCamera(bool addnocamera) {
	bCameraDefined	= false;
	m_CamerasControl.ResetContent();
	if (addnocamera)
		m_CamerasControl.AddString(NOCAMERA);
	m_CamerasControl.SetCurSel(0);
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::CreateObjectInstance()
//

bool CMaxClientDlg::CreateObjectInstance(REFCLSID rclsid,REFIID riid,LPVOID *ppv) {

	HRESULT hr;

	//-- If local, simply use registry

	if (IsDlgButtonChecked(IDC_LOCAL)) {
		
		hr = CoCreateInstance(rclsid,NULL,CLSCTX_LOCAL_SERVER,riid,ppv);

		if (FAILED(hr)) {
			MessageBox("Error Creating Local Instance of object");
			return false;
		}

	//-- If remote, use given server name

	} else {

		TCHAR server_name[MAX_PATH];
		if (!GetDlgItemText(IDC_SERVERNAME,server_name,MAX_PATH)) {
			MessageBox("Need a server name if not set to Local");
			return false;
		} else {

			COSERVERINFO server;
			memset(&server,0,sizeof(COSERVERINFO));
			_bstr_t s_name(server_name);
			server.pwszName = s_name;
			MULTI_QI mq;
			memset(&mq,0,sizeof(MULTI_QI));
			mq.pIID = &riid;

			hr = CoCreateInstanceEx(rclsid,NULL,CLSCTX_REMOTE_SERVER | CLSCTX_LOCAL_SERVER,&server,1,&mq);

			if (FAILED(hr) || FAILED(mq.hr)) {
				TCHAR txt[256];
				wsprintf(txt,"Error Creating Remote Instance (hr:%0X)",hr);
				MessageBox(txt);
				return false;
			} else
				*ppv = mq.pItf;

		}

	}

	return true;

}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnCreateInstance()
//

void CMaxClientDlg::OnCreateInstance() {
	HCURSOR oldcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	if (CreateObjectInstance(CLSID_MaxRenderer,IID_IMaxRenderer,(void **)&pMax)) {
		if (CreateObjectInstance(CLSID_MaxBitmapInfo,IID_IMaxBitmapInfo,(void **)&pBif)) {
			bInitialized = true;
			StartSink();
		} else
			pMax->Release();
	}
	UpdateControls();
	SetCursor(oldcur);
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnLoadMaxFile()
//

void CMaxClientDlg::OnLoadMaxFile() {
	TCHAR a_name[MAX_PATH];
	if (!GetDlgItemText(IDC_MAXFILENAME,a_name,MAX_PATH))
		return;
	_bstr_t b_name(a_name);
	try {
		HRESULT hr = pMax->LoadScene(b_name);
		if (FAILED(hr)) {
			CString txt;
			txt.Format("Error loading %s",a_name);
			MessageBox(LPCTSTR(txt),"Load MAX Scene");
		} else {
			bMaxFileLoaded	= true;
			ResetCamera();
		}
	} catch(_com_error& e) {
		TRACE("Error Loading %s\n",a_name);
		TRACE("%s\n",e.ErrorMessage());
		if (e.ErrorInfo()) {
			TRACE("%s\n",e.Description());
		}
	}
	UpdateControls();
}

//-----------------------------------------------------------------------------
// #> CMaxRendererEvents::OnScript()
//

void CMaxClientDlg::OnScript() {
	TCHAR a_name[MAX_PATH];
	if (!GetDlgItemText(IDC_SCRIPTEDIT,a_name,MAX_PATH))
		return;
	_bstr_t b_name(a_name);
	try {
		HRESULT hr = pMax->ExecuteMAXScriptString(b_name);
		if (FAILED(hr)) {
			CString txt;
			txt.Format("Error executing %s",a_name);
			MessageBox(LPCTSTR(txt),"Execute MAX Script");
		}
	} catch(_com_error& e) {
		TRACE("Error Executing %s\n",a_name);
		TRACE("%s\n",e.ErrorMessage());
		if (e.ErrorInfo()) {
			TRACE("%s\n",e.Description());
		}
	}
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnRender()
//

void CMaxClientDlg::OnRender() {

	m_ProgressControl.SetRange(0,100);
	m_ProgressControl.SetPos(0);
	
	try {

		if (!pPreview)
			pPreview = (BYTE *)LocalAlloc(LPTR,PREV_WIDTH * PREV_HEIGHT * 3);
		_ASSERTE(pPreview);

		preview_curline = 0;

		//-- Define Output Bitmap

		pBif->Width				= OUTPUT_WIDTH;		//-- Final Image Width
		pBif->Height			= OUTPUT_HEIGHT;	//-- Final Image Height
		pBif->Aspect			= 1.0f;				//-- Final Image Aspect Ratio
		
		//-- Optional Settings
		
		pBif->Channels			= CHAN_NONE;		//-- No Special G Buffer Channels
		pBif->Gamma				= 1.2f;				//-- Gamma Setting
		pBif->ProcessGamma		= TRUE;				//-- Enable Gamma Processing

		//-- Get Camera Name
		
		CString CameraName;
		int idx = m_CamerasControl.GetCurSel();
		m_CamerasControl.GetLBText(idx,CameraName);

		//-- Open Renderer
		
		USES_CONVERSION;
		LPOLESTR b_name = T2OLE(LPCTSTR(CameraName));

#ifdef RENDER_REGION

		pMax->OpenRenderer(b_name,pBif,TRUE);		//-- Open Renderer with camera and Output Settings

#else

		pMax->OpenRenderer(b_name,pBif,FALSE);		//-- Open Renderer with camera and Output Settings

#endif

		//-- Frame Number
		//
		//   The frame number is a float. Time between frames can be specified
		//   with a resolution up to around 1/4000 of a frame.

		TCHAR framestr[MAX_PATH];
		GetDlgItemText(IDC_FRAMENUMBER,framestr,MAX_PATH);
		float frameno = (float)(atof(framestr));

		//-- Start Render

#ifdef RENDER_REGION

		pMax->SetRegion(80,80,OUTPUT_WIDTH-81,OUTPUT_HEIGHT-81);

#endif
		
		pMax->RenderFrame(frameno,1.0f);					//-- Frame Number and Duration
		bRendering = true;

	} catch(_com_error& e) {
		TRACE("Error Rendering Frame\n");
		TRACE("%s\n",e.ErrorMessage());
		if (e.ErrorInfo()) {
			TRACE("%s\n",e.Description());
		}
	}

	UpdateControls();

}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnCancelRender()
//

void CMaxClientDlg::OnCancelRender() {
	pMax->CancelRenderer();
	m_MessagesControl.SetWindowText("Cancel Pending...");
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::OnListcameras()
//

void CMaxClientDlg::OnListcameras() {
	ResetCamera(false);
	try {												//-- Exception Handling
		pMax->EnumCameras();							//-- Ask MAX to Enumerate Cameras
		if (m_CamerasControl.GetCount())				//-- If any Camera
			bCameraDefined = true;						//-- Enable rest of controls
		else
			ResetCamera();
		m_CamerasControl.SetCurSel(0);					//-- Set default to first Camera
	} catch(_com_error& e) {							//-- Handle exception
		TRACE("Error Enumerating Cameras\n");
		TRACE("%s\n",e.ErrorMessage());
		if (e.ErrorInfo()) {
			TRACE("%s\n",e.Description());
		}
	}
	UpdateControls();
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::UpdatePreview()
//

void CMaxClientDlg::UpdatePreview(long Done, long Total) {

	bool new_data = false;

	try {

#ifdef RENDER_REGION

		int output_height	= OUTPUT_HEIGHT - 81 - 80 + 1;
		int prev_height		= PREV_HEIGHT / 2;
		int prev_width		= PREV_WIDTH  / 2;
		int x_offset		= 80;
		int y_offset		= 80;

#else

		int output_height	= OUTPUT_HEIGHT;
		int prev_height		= PREV_HEIGHT;
		int prev_width		= PREV_WIDTH;
		int x_offset		= 0;
		int y_offset		= 0;

#endif

		//-- Scale vertical image

		int top		= (Done * prev_height) / Total;
		float step	= (float)((float)output_height / (float)prev_height);

		//-- Collect some lines

		for (int i = preview_curline; i < top; i++) {
			int line = (int)(step * (float)i);
			SAFEARRAY *psa = pMax->GetPreviewLine(line,prev_width);	//-- It will scale to the given width
			BYTE *data;
			SafeArrayAccessData(psa,(void**)&data);

			BYTE *target = ((prev_height - i - 1 + y_offset) * PREV_WIDTH * 3 + (x_offset * 3)) + pPreview;
			memcpy(target,data,prev_width * 3);

			SafeArrayUnaccessData(psa);
			SafeArrayDestroy(psa);
			new_data = true;
		}
		
		//-- Silly little progress white line

		if (top < prev_height) {
			BYTE *target = ((prev_height - top - 1 + y_offset) * PREV_WIDTH * 3 + (x_offset *3)) + pPreview;
			memset(target,255,prev_width * 3);
		}

		preview_curline = top;

	} catch(_com_error& e) {
		TRACE("Error Loading Bitmap\n");
		TRACE("%s\n",e.ErrorMessage());
		if (e.ErrorInfo()) {
			TRACE("%s\n",e.Description());
		}
	}

	if (new_data) {

		//-- Very ugly but it works...

		DeleteObject(hPreview);

		BITMAPINFO bmi;
		memset(&bmi,0,sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biBitCount	= 24;
		bmi.bmiHeader.biWidth		= PREV_WIDTH;
		bmi.bmiHeader.biHeight		= PREV_HEIGHT;
		bmi.bmiHeader.biPlanes		= 1;
		bmi.bmiHeader.biCompression	= BI_RGB;
		
		HDC hdc = ::GetDC(NULL);
		hPreview = CreateCompatibleBitmap(hdc,PREV_WIDTH,PREV_HEIGHT);
		SetDIBits(hdc,hPreview,0,PREV_HEIGHT,pPreview,&bmi,DIB_RGB_COLORS);

		m_PreviewWindow.SetBitmap(hPreview);
		m_PreviewWindow.InvalidateRect(NULL);
	}

}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::StopSink()
//

void CMaxClientDlg::StopSink() {
	if (pMaxEvents)
		AtlUnadvise(pMax,IID__IMaxRendererEvents,cookie);
	if (pMax) pMax = 0;
	if (ptrEventsUnk) ptrEventsUnk = 0;
	bInitialized	= false;
	bMaxFileLoaded	= false;
	ResetCamera();
	UpdateControls();
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::StartSink()
//

void CMaxClientDlg::StartSink() {
	if (!pMaxEvents) {
		pMaxEvents	= new CComObject<CMaxRendererEvents>;
		ptrEventsUnk= pMaxEvents;
		HRESULT hr	= AtlAdvise(pMax,ptrEventsUnk,IID__IMaxRendererEvents,&cookie);
		if (FAILED(hr)) {
			pMaxEvents = 0;
			StopSink();
		} else
			pMaxEvents->SetParentIface(this);
	}
}

//-----------------------------------------------------------------------------
// #> CMaxClientDlg::DestroyWindow()
//

BOOL CMaxClientDlg::DestroyWindow() {
	StopSink();
	if (hPreview)
		DeleteObject(hPreview);
	if (pPreview)
		LocalFree(pPreview);
	if (pBif)
		pBif->Release();
	if (pMax)
		pMax->Release();
	return CDialog::DestroyWindow();
}

//-----------------------------------------------------------------------------
// #> CMaxRendererEvents::OnEnumCameras()
//

STDMETHODIMP CMaxRendererEvents::OnEnumCameras( BSTR CameraName ) {
	LPTSTR cameraname;
	USES_CONVERSION;
	cameraname = OLE2T(CameraName);
	dlg->m_CamerasControl.AddString(cameraname);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRendererEvents::OnRenderProgress()
//

STDMETHODIMP CMaxRendererEvents::OnRenderProgress( long Done, long Total ) {
	dlg->m_ProgressControl.SetPos((Done*100)/Total);
	dlg->UpdatePreview(Done,Total);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRendererEvents::OnRenderMessage()
//

STDMETHODIMP CMaxRendererEvents::OnRenderMessage( BSTR Message ) {
	LPTSTR msg;
	USES_CONVERSION;
	msg = OLE2T(Message);
	dlg->m_MessagesControl.SetWindowText(msg);
	return S_OK;
}

//-----------------------------------------------------------------------------
// #> CMaxRendererEvents::OnRenderDone()
//

STDMETHODIMP CMaxRendererEvents::OnRenderDone( void ) {
	dlg->pMax->CloseRenderer();
	dlg->bRendering = false;
	dlg->UpdatePreview(100,100);
	dlg->m_ProgressControl.SetPos(0);
	dlg->m_MessagesControl.SetWindowText("Rendering Complete");
	dlg->UpdateControls();
	return S_OK;
}


