#if !defined(AFX_SDKAPWZAW_H__F0FBC91F_C4DD_11D0_9E39_080009B37D22__INCLUDED_)
#define AFX_SDKAPWZAW_H__F0FBC91F_C4DD_11D0_9E39_080009B37D22__INCLUDED_

// SDKAPWZaw.h : header file

extern CStringArray ClassList;

class CDialogChooser;

// All function calls made by mfcapwz.dll to this custom AppWizard (except for
//  GetCustomAppWizClass-- see SDKAPWZ.cpp) are through this class.  You may
//  choose to override more of the CCustomAppWiz virtual functions here to
//  further specialize the behavior of this custom AppWizard.
class CSDKAPWZAppWiz : public CCustomAppWiz
{
public:
	virtual CAppWizStepDlg* Next(CAppWizStepDlg* pDlg);
	virtual CAppWizStepDlg* Back(CAppWizStepDlg* pDlg);
		
	virtual void InitCustomAppWiz();
	virtual void ExitCustomAppWiz();
	virtual void CustomizeProject(IBuildProject* pProject);

protected:
	CDialogChooser* m_pChooser;
};

// This declares the one instance of the CSDKAPWZAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global SDKAPWZaw.  (Its definition is in SDKAPWZaw.cpp.)
extern CSDKAPWZAppWiz SDKAPWZaw;
extern BOOL lookup_key(CString key, CString& val=CString(""));
extern void set_key(CString key, CString val);
extern BOOL remove_key(CString key);

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SDKAPWZAW_H__F0FBC91F_C4DD_11D0_9E39_080009B37D22__INCLUDED_)
