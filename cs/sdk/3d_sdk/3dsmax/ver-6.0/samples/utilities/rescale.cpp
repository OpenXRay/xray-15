/**********************************************************************
 *<
	FILE: rescale.cpp

	DESCRIPTION:  A rescale utility

	CREATED BY: Dan Silva

	HISTORY: created 4/28/97

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"

#ifndef NO_UTILITY_RESCALE	// russom - 10/16/01

#define RESCALE_CLASS_ID		Class_ID(0xb1381a58,0x738933)
#define RESCALE_CNAME			GetString(IDS_DS_RESCALE)

class RescaleUtil : public UtilityObj, public MeshOpProgress {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		HWND hDlg;
		static float factor;
		static ISpinnerControl *fspin;
		static BOOL doSel;
		RescaleUtil();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}
		void SelectionSetChanged(Interface *ip,IUtil *iu);
		BOOL DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		void ReleaseControls();

		void DoRescale();

		// From MeshOpProgress
		void Init(int total);		
		BOOL Progress(int p);
	};

ISpinnerControl *RescaleUtil::fspin = NULL;
float RescaleUtil::factor = 1.0f;
BOOL RescaleUtil::doSel = FALSE;

static RescaleUtil theRescaleUtil;

class RescaleUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theRescaleUtil;}
	const TCHAR *	ClassName() {return RESCALE_CNAME;}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return RESCALE_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static RescaleUtilClassDesc rescaleUtilDesc;
ClassDesc* GetRescaleDesc() {return &rescaleUtilDesc;}


static INT_PTR CALLBACK RescaleUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theRescaleUtil.Init(hWnd);
			break;

		case WM_DESTROY:
			theRescaleUtil.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theRescaleUtil.iu->CloseUtility();
					break;
				case IDC_RESCALE:
					theRescaleUtil.DoRescale();
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
			theRescaleUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}	


RescaleUtil::RescaleUtil()
	{	
	factor = 1.0f;
	}

void RescaleUtil::BeginEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_RESCALE_PANEL),
		RescaleUtilDlgProc,
		GetString(IDS_DS_RESCALE),
		0);
	}
	
void RescaleUtil::EndEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void RescaleUtil::Init(HWND hWnd)
	{
	hPanel = hWnd;
//	SelectionSetChanged(ip,iu);	
	}

void RescaleUtil::Destroy(HWND hWnd)
	{		
	hPanel = NULL;
	}

void RescaleUtil::SelectionSetChanged(Interface *ip,IUtil *iu)
	{
	}

void RescaleUtil::ReleaseControls() {
	if (fspin) {
		ReleaseISpinner(fspin);
		fspin = NULL;
		}
	}

static BOOL doseltmp;

BOOL RescaleUtil::DlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	int id;
	switch (msg) {
		case WM_INITDIALOG:			
			fspin = SetupFloatSpinner(hWnd, IDC_RESCALE_SPIN, IDC_RESCALE_EDIT,0.00001f,100000.0f, factor);
			CenterWindow(hWnd, GetWindow(hWnd, GW_OWNER));        	
			CheckRadioButton( hWnd, IDC_RESC_SCENE,IDC_RESC_SEL, doSel+IDC_RESC_SCENE);
			doseltmp = doSel;
			break;

		case WM_DESTROY:
			ReleaseControls();
			break;

		case WM_COMMAND:			
			switch (id=LOWORD(wParam)) {				
				case IDC_RESC_SEL:
					doseltmp = 1;
					break;
				case IDC_RESC_SCENE:
					doseltmp = 0;
					break;
				case IDOK:
					doSel = doseltmp;
					factor = fspin->GetFVal();
					EndDialog(hWnd, TRUE);
					break;
	        	case IDCANCEL:
					EndDialog(hWnd, FALSE);
					break;
				}			
			break;

		default:
			return FALSE;
		}	
	return TRUE;
	}


static INT_PTR CALLBACK  RescaleDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	RescaleUtil *resc;
	if (msg==WM_INITDIALOG) {
		resc = (RescaleUtil*)lParam;
		resc->hDlg = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (resc = (RescaleUtil *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	int	res = resc->DlgProc(hwndDlg,msg,wParam,lParam);
	return res;
	}

class RescaleRestore: public RestoreObj {
	float fact;
	BOOL doSel;
	public:
		RescaleRestore::RescaleRestore(float f, BOOL sel) { fact = f; doSel = sel; } 
		void Restore(int isUndo) {
			Interface *ip = GetCOREInterface();
			ip->RescaleWorldUnits(1.0f/fact,doSel);
			ip->RedrawViews(ip->GetTime());
			}
		void Redo() {
			Interface *ip = GetCOREInterface();
			ip->RescaleWorldUnits(fact,doSel);
			ip->RedrawViews(ip->GetTime());
			}
		TSTR Description() { return TSTR("RescaleRestore"); }
	};


void RescaleUtil::DoRescale()
	{

	if (DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_RESCALE_DLG),
		hPanel,
		RescaleDlgProc,(LPARAM)this)) {
		ip->RescaleWorldUnits(factor,doSel);
		ip->RedrawViews(ip->GetTime());
		theHold.Begin();
		theHold.Put(new RescaleRestore(factor,doSel));
		theHold.Accept(GetString(IDS_DS_RESCALE));
		}

	}

void RescaleUtil::Init(int total)
	{
	}

BOOL RescaleUtil::Progress(int p)
	{
	return TRUE;
	}


#endif // NO_UTILITY_RESCALE
