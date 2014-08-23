// NotifyDlg.h : header file
//

#if !defined(AFX_NOTIFYDLG_H__25CB49CB_AB8D_11D0_9667_00A0249611DC__INCLUDED_)
#define AFX_NOTIFYDLG_H__25CB49CB_AB8D_11D0_9667_00A0249611DC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CNotifyDlg dialog

class CNotifyDlg : public CDialog
{
// Construction
public:
	CNotifyDlg(CWnd* pParent = NULL);	// standard constructor
	BOOL Browse(int type, TCHAR *filename);

// Dialog Data
	//{{AFX_DATA(CNotifyDlg)
	enum { IDD = IDD_SOUNDS };
	CString	m_CompletionTarget;
	CString	m_FailureTarget;
	CString	m_ProgressTarget;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNotifyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNotifyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnUpdateFailureEd();
	afx_msg void OnBrowseFailure();
	afx_msg void OnBrowseProgress();
	afx_msg void OnBrowseCompletion();
	afx_msg void OnPlayFailure();
	afx_msg void OnPlayCompletion();
	afx_msg void OnPlayProgress();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOTIFYDLG_H__25CB49CB_AB8D_11D0_9667_00A0249611DC__INCLUDED_)
