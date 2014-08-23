#if !defined(AFX_CSTM2DLG_H__F0FBC92F_C4DD_11D0_9E39_080009B37D22__INCLUDED_)
#define AFX_CSTM2DLG_H__F0FBC92F_C4DD_11D0_9E39_080009B37D22__INCLUDED_

// cstm2dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustom2Dlg dialog

class CCustom2Dlg : public CAppWizStepDlg
{
// Construction
public:
	void DrawBitmaps();
	CBitmap bPic, *pPicBMap;
	CCustom2Dlg();
	virtual BOOL OnDismiss();

// Dialog Data
	//{{AFX_DATA(CCustom2Dlg)
	enum { IDD = IDD_CUSTOM2 };
	CStatic	m_PicFrame;
	CListBox	m_ClassTypes;
	CString	m_ClassName;
	CString	m_BaseClass;
	CString	m_Category;
	CString	m_LibDesc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustom2Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCustom2Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSTM2DLG_H__F0FBC92F_C4DD_11D0_9E39_080009B37D22__INCLUDED_)
