//{{AFX_INCLUDES()
#include "picclip.h"
//}}AFX_INCLUDES
#if !defined(AFX_CSTM1DLG_H__F0FBC92D_C4DD_11D0_9E39_080009B37D22__INCLUDED_)
#define AFX_CSTM1DLG_H__F0FBC92D_C4DD_11D0_9E39_080009B37D22__INCLUDED_

// cstm1dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg dialog


class CCustom1Dlg : public CAppWizStepDlg
{
// Construction
public:
	CCustom1Dlg();
	virtual BOOL OnDismiss();
	CBitmap bGeo, bShp, *pGeoBMap, *pShpBMap;
// Dialog Data
	//{{AFX_DATA(CCustom1Dlg)
	enum { IDD = IDD_CUSTOM1 };
	CStatic	m_PicFrame;
	CListBox	m_PlugTypes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustom1Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DrawBitmaps();
	// Generated message map functions
	//{{AFX_MSG(CCustom1Dlg)
	afx_msg void OnSelchangePluginTypes();
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSTM1DLG_H__F0FBC92D_C4DD_11D0_9E39_080009B37D22__INCLUDED_)
