// cstm2dlg.cpp : implementation file
//

#include "stdafx.h"
#include "SDKAPWZ.h"
#include "cstm2dlg.h"
#include "SDKAPWZaw.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CCustom2Dlg dialog


CCustom2Dlg::CCustom2Dlg()
	: CAppWizStepDlg(CCustom2Dlg::IDD)
{
	//{{AFX_DATA_INIT(CCustom2Dlg)
	m_ClassName = _T("");
	m_BaseClass = _T("");
	m_Category = _T("");
	m_LibDesc = _T("");
	//}}AFX_DATA_INIT
}


void CCustom2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CAppWizStepDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustom2Dlg)
	DDX_Control(pDX, IDC_PICTURE, m_PicFrame);
	DDX_Control(pDX, IDC_CLASS_TYPES, m_ClassTypes);
	DDX_Text(pDX, IDC_CLASS_NAME, m_ClassName);
	DDX_LBString(pDX, IDC_CLASS_TYPES, m_BaseClass);
	DDX_Text(pDX, IDC_CATEGORY, m_Category);
	DDX_Text(pDX, IDC_LIBDESC, m_LibDesc);
	//}}AFX_DATA_MAP
}

// This is called whenever the user presses Next, Back, or Finish with this step
//  present.  Do all validation & data exchange from the dialog in this function.
BOOL CCustom2Dlg::OnDismiss()
{
	if (!UpdateData(TRUE))
		return FALSE;

	// TODO: Set template variables based on the dialog's data.
	if(m_ClassName.IsEmpty())
	{
		MessageBox(_T("The class name cannot be null"), _T("Appwizard Error"));
		return FALSE;
	}
	
	// Initialize random generator
	srand( (unsigned)time( NULL ));
	
	//Generating a unique ClassID
	CString nClassID1, nClassID2,nExtClassID1,nExtClassID2,warpClassID1,warpClassID2;
	nClassID1.Format(_T("0x%x"), rand() + rand() + ((rand()+(rand()<<16))));
	nClassID2.Format(_T("0x%x"), rand() + rand() + ((rand()+(rand()<<16))));

	//needed for extension channels
	nExtClassID1.Format(_T("0x%x"), rand() + rand() + ((rand()+(rand()<<16))));
	nExtClassID2.Format(_T("0x%x"), rand() + rand() + ((rand()+(rand()<<16))));

	//needed for Space warps
	warpClassID1.Format(_T("0x%x"), rand() + rand() + ((rand()+(rand()<<16))));
	warpClassID2.Format(_T("0x%x"), rand() + rand() + ((rand()+(rand()<<16))));


	CString cnu = m_ClassName; cnu.MakeUpper();
	CString cnl = m_ClassName; cnl.MakeLower();
	set_key(_T("CLASS_NAME"),		m_ClassName);
	set_key(_T("CLASS_NAME_UPPER"),	cnu);
	set_key(_T("CLASS_NAME_LOWER"),	cnl);
	set_key(_T("SUPER_CLASS_NAME"),	m_BaseClass);
	set_key(_T("EXTCLASSID1"),			nExtClassID1);
	set_key(_T("EXTCLASSID2"),			nExtClassID2);
	set_key(_T("CLASSID1"),			nClassID1);
	set_key(_T("CLASSID2"),			nClassID2);

	set_key(_T("WARPID1"),			warpClassID1);
	set_key(_T("WARPID2"),			warpClassID2);


	set_key(_T("CATEGORY"),			m_Category);
	set_key(_T("LIBDESC"),			m_LibDesc);
	
	remove_key(_T("SIMPLE_TYPE"));
	remove_key(_T("SIMPLE_MANIP"));
	if( m_BaseClass == _T("GenCamera") ||
		m_BaseClass == _T("StdControl") ||
		m_BaseClass == _T("ConstObject") ||
		m_BaseClass == _T("GenLight") ||
		m_BaseClass == _T("SimpleMod2") ||
		m_BaseClass == _T("SimpleParticle") ||
		m_BaseClass == _T("SimpleObject2") ||
		m_BaseClass == _T("SimpleSpline") ||
		m_BaseClass == _T("SimpleShape") ||
		m_BaseClass == _T("SimpleWSMMod"))
		set_key(_T("SIMPLE_TYPE"), _T("YES"));
	
	if(m_BaseClass ==_T("SimpleManipulator"))
		set_key(_T("SIMPLE_MANIP"), _T("YES"));


	return TRUE;	// return FALSE if the dialog shouldn't be dismissed
}


BEGIN_MESSAGE_MAP(CCustom2Dlg, CAppWizStepDlg)
	//{{AFX_MSG_MAP(CCustom2Dlg)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCustom2Dlg message handlers

BOOL CCustom2Dlg::OnInitDialog() 
{
	CAppWizStepDlg::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString cname;
	lookup_key(_T("root"), cname);
	
	cname.SetAt(0, toupper(cname[0]));
	m_ClassName = cname;
	//m_Category = _T("Appwiz Tests");
	//m_LibDesc  = _T("Appwizard generated test plugin");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustom2Dlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CAppWizStepDlg::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	if(!bShow)
		return;
	
	m_ClassTypes.ResetContent();
	for(int i=0; i < ClassList.GetSize(); i++)
		m_ClassTypes.AddString(ClassList[i]);

	if(m_ClassTypes.GetCurSel() == LB_ERR)
		m_ClassTypes.SetCurSel(0);

	UpdateData(FALSE);
}

void CCustom2Dlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CAppWizStepDlg::OnPaint() for painting messages
	DrawBitmaps();
	HWND FinishButton = ::GetDlgItem(::GetParent(m_hWnd),440);
	::EnableWindow(FinishButton,false);
}

void CCustom2Dlg::DrawBitmaps()
{
	CDC cpDcPic, *cdc;
	RECT rec ;
	m_PicFrame.GetWindowRect(&rec) ;
	cdc = m_PicFrame.GetDC() ;
	cpDcPic.CreateCompatibleDC(cdc);
	cpDcPic.SelectObject(pPicBMap) ;
	cdc->BitBlt(0,0,rec.right - rec.left, rec.bottom-rec.top,&cpDcPic,0,0,SRCCOPY) ;
}

int CCustom2Dlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CAppWizStepDlg::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	bPic.LoadBitmap(IDB_MAX2) ;
	pPicBMap = &bPic ;
	return 0;
}
