#if !defined(AFX_CSTM3DLG_H__F0FBC931_C4DD_11D0_9E39_080009B37D22__INCLUDED_)
#define AFX_CSTM3DLG_H__F0FBC931_C4DD_11D0_9E39_080009B37D22__INCLUDED_

// cstm3dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustom3Dlg dialog

class CCustom3Dlg : public CAppWizStepDlg
{
// Construction
public:
	void DrawBitmaps();
	CBitmap bPic, *pPicBMap;
	CCustom3Dlg();
	virtual BOOL OnDismiss();
	BOOL GetBrowseDirectory(TCHAR* title, CString& path);

// Dialog Data
	//{{AFX_DATA(CCustom3Dlg)
	enum { IDD = IDD_CUSTOM3 };
	CButton	m_ctlExtCh;
	CButton	m_CtlMaps;
	CStatic	m_PicFrame;
	CString	m_SdkPath;
	CString	m_PlgPath;
	CString	m_ExePath;
	BOOL	m_Comments;	
	BOOL	m_HybConfig;
	BOOL	m_ParamMaps;
	BOOL	m_ExtensionChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustom3Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCustom3Dlg)
	afx_msg void OnSdkBrowse();
	afx_msg void OnPluginBrowse();
	afx_msg void OnExeBrowse();
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSTM3DLG_H__F0FBC931_C4DD_11D0_9E39_080009B37D22__INCLUDED_)
