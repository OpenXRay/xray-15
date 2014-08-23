// MaxClientDlg.h : header file
//

#if !defined(AFX_MAXCLIENTDLG_H__4E380CA1_5DB1_11D2_91CB_0060081C257E__INCLUDED_)
#define AFX_MAXCLIENTDLG_H__4E380CA1_5DB1_11D2_91CB_0060081C257E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#import "..\..\..\..\..\exe\stdplugs\comsrv.gup"  no_namespace named_guids exclude("_SYSTEMTIME")

class CMaxRendererEvents;

//#define SINKDECL void __stdcall
//#define SINKDECL HRESULT

//-----------------------------------------------------------------------------
// CMaxClientDlg dialog

class CMaxClientDlg : public CDialog {
// Construction
public:
	CMaxClientDlg(CWnd* pParent = NULL);
	~CMaxClientDlg();
	
	IMaxBitmapInfo*					pBif;
	IMaxRenderer*					pMax;
	
	CComPtr<IUnknown>				ptrEventsUnk;
	CComObject<CMaxRendererEvents>* pMaxEvents;

#if defined( DESIGN_VER )
	HBITMAP		hDefPreview;
#endif // DESIGN_VER
	HBITMAP		hPreview;
	BYTE*		pPreview;
	long		preview_curline;
	bool		bInitialized,bMaxFileLoaded,bCameraDefined,bRendering;
	DWORD		cookie;

	bool	CreateObjectInstance	(REFCLSID rclsid,REFIID riid,LPVOID *ppv);
	void	UpdateServerEdit		( );
	void	ResetCamera				(bool b = true);
	void	UpdatePreview			(long Done, long Total);
	void	UpdateControls			( );
	void	StartSink				( );
	void	StopSink				( );

// Dialog Data
	//{{AFX_DATA(CMaxClientDlg)
	enum { IDD = IDD_MAXCLIENT_DIALOG };
	CEdit	m_ServerControl;
	CButton	m_LocalButton;
	CButton	m_CreateButton;
	CStatic	m_PreviewWindow;
	CEdit	m_ScriptTextControl;
	CButton	m_ScriptButtonControl;
	CButton	m_CancelRenderControl;
	CButton	m_ListCameraControl;
	CProgressCtrl	m_ProgressControl;
	CStatic	m_MessagesControl;
	CComboBox	m_CamerasControl;
	CButton	m_RenderButton;
	CEdit	m_MaxFileEdit;
	CButton	m_LoadButton;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaxClientDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMaxClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnCreateInstance();
	afx_msg void OnLoadMaxFile();
	afx_msg void OnListcameras();
	afx_msg void OnRender();
	afx_msg void OnCancelRender();
	afx_msg void OnScript();
	afx_msg void OnLocal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//-----------------------------------------------------------------------------
//-- Client Connection Point

class CMaxRendererEvents : 
		public IDispatchImpl<_IMaxRendererEvents,&IID__IMaxRendererEvents,&LIBID_COMSRVLib>,
		public CComObjectRoot {
	
	public:

		CMaxRendererEvents(){};

	BEGIN_COM_MAP(CMaxRendererEvents)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(_IMaxRendererEvents)
	END_COM_MAP()

	//-- Sink Methods

	STDMETHODIMP OnEnumCameras			( BSTR CameraName );
    STDMETHODIMP OnRenderProgress		( long Done, long Total );
    STDMETHODIMP OnRenderMessage		( BSTR Message );
    STDMETHODIMP OnRenderDone			( void );

	STDMETHODIMP raw_OnEnumCameras		( BSTR CameraName )			{ return OnEnumCameras		( CameraName ); }
	STDMETHODIMP raw_OnRenderProgress	( long Done, long Total )	{ return OnRenderProgress	( Done, Total ); }
	STDMETHODIMP raw_OnRenderMessage	( BSTR Message )			{ return OnRenderMessage	( Message ); }
	STDMETHODIMP raw_OnRenderDone		( void )					{ return OnRenderDone		( ); }

	//-- Local Helpers

	CMaxClientDlg *dlg;
	void	SetParentIface (CMaxClientDlg *p)	{ dlg = p; }

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAXCLIENTDLG_H__4E380CA1_5DB1_11D2_91CB_0060081C257E__INCLUDED_)
