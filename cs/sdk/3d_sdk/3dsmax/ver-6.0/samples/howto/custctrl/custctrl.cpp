/*===========================================================================*\
 |    File: custctrl.cpp
 |
 | Purpose: To Demonstrate the use of the MAX Custom Controls.
 |
 |          This plug-in puts up a rollup page in the utility panel with
 |          most of the custom controls.  The comments in the source code
 |          document what is happening.  Also see the Advanced Topics 
 |          section for a description of the custom controls.
 |          Note:  This plug-in does not use Parameter Maps to manage 
 |          the user interface.  Parameter Maps can simplify UI code
 |          for many plug-ins.  See the Advanced Topics section on 
 |          Parameter Maps for more details.
 |
 | History: Mark Meier, Begun 09/28/95.
 |          MM, 04/16/96, Modified to make it a Utililty plug-in.
 |          MM, 12/20/96, Modified to use the proper C string functions. For 
 |          example, sprintf() was replaced by _stprintf().
 |          See the Advanced Topics section on Character Strings for details
 |          on the methods used to work with the MBCS character strings
 |          used by MAX.
 |          MM, 06/97, Added new demo code of dialogs and new APIs in R2.
\*===========================================================================*/
#include "max.h"		// Main MAX include file
#include "utilapi.h"	// Utility plug-in include file.
#include "resource.h"	// Resource editor include file
#include "tcbgraph.h"	// TCB Graph include file
#include "stdmat.h"		// So we can create a standard material
#include "hsv.h"		// This is so the modal color picker API can be used
#include "arcdlg.h"		// Arcball dialog include file
#include "simpobj.h"	// To get the GenBox object created by the arcdlg

/*===========================================================================*\
 | Misc defines, etc.
\*===========================================================================*/
#define MY_CATEGORY _T("How To")
#define MY_CLASSNAME _T("Custom Controls")
#define MY_CLASS_ID Class_ID(0x211f19ca, 0x67900b9e)
#define MY_LIBDESCRIPTION _T("Custom Control Tester by Mark Meier")

#define IMAGE_W 20 
#define IMAGE_H 18 
#define ICON_W 16 // Icon width - "standard" is 16
#define ICON_H 15 // Icon height - "standard" is 15
#define TB_BUT_WIDTH 22
#define TB_BUT_HEIGHT 20

#define ID_TB_1 1000 // Toolbar IDs
#define ID_TB_2 1010
#define ID_TB_3 1020

HINSTANCE hInstance;

/*===========================================================================*\
 | This is the class used by the Arc Rotate modeless dialog
\*===========================================================================*/
class MyArcDlgCB : public ArcballCallback {
	INode *node;
	Interface *ip;
	Quat qstart;
	ArcballDialog *ad;
	public:
		void StartDrag();
		void Drag(Quat q, BOOL buttonUp);
		void CancelDrag();
		void EndDrag();
		void BeingDestroyed();
		
		MyArcDlgCB() {node = NULL; ad = NULL; ip = NULL;}
		~MyArcDlgCB() {if (ad) ad->DeleteThis();}
		void SetIP(Interface *intp) {ip = intp;}
		void SetNode(INode *n) {node = n;}
		INode *GetNode() {return node;}
		void SetDialog(ArcballDialog *d) {ad = d;}
		ArcballDialog *GetDialog() {return ad;}
		void MakeMatrix(Quat q, Matrix3 &tm);
};

/*===========================================================================*\
 | This is the main plug-in class
\*===========================================================================*/
class CustCtrlUtil : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hCustCtrl, hMaxDlg;
		TCBGraphParams gp;

		// These are the handles to the custom controls in the rollup page.
		ISpinnerControl *ccSpin;
		IColorSwatch *ccColorSwatch;
		ICustEdit *ccEdit;
		ICustEdit *ccTCBEdit;
		ISpinnerControl *ccTCBSpin;
		ICustButton *ccButtonC;
		ICustButton *ccButtonP;
		ICustButton *ccFlyOff;
		ICustStatus *ccStatus;
		ICustToolbar *ccToolbar;
		ICustImage *ccImage;

		ISliderControl *ccSlider;
		ICustEdit	   *ccSliderEdit;
		ISliderControl *ccSlider1;
		ICustEdit	   *ccSliderEdit1;
		ISliderControl *ccSlider2;
		ICustEdit	   *ccSliderEdit2;

		HIMAGELIST hImageFO;
		HIMAGELIST hImageIM;
		HIMAGELIST hImageTB;
		MyArcDlgCB arcb;

		// --- Inherited Virtual Methods From UtilityObj ---
		void BeginEditParams(Interface *ip, IUtil *iu);
		void EndEditParams(Interface *ip, IUtil *iu);
		void DeleteThis() {}

		// --- Methods From CustCtrlUtil ---
		CustCtrlUtil();
		~CustCtrlUtil() {}
		void OnInitDialog(HWND hWnd);
};
static CustCtrlUtil theUtility;

/*===========================================================================*\
 | Dialog Procs
\*===========================================================================*/
// This is the dialog proc for the CUSTCTRL rollup page.
INT_PTR CALLBACK CustCtrlDlgProc(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	TSTR buf;
	COLORREF color;
	TCHAR s[32];
	int index;

	CustCtrlUtil *u = (CustCtrlUtil *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!u && message != WM_INITDIALOG ) return FALSE;

	switch (message) {
		case WM_INITDIALOG: {
			u = (CustCtrlUtil *)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)u);
			theUtility.OnInitDialog(hWnd);
		 	return TRUE;
		}
		case WM_DESTROY: {
			// Release all our Custom Controls
			ReleaseISpinner(u->ccSpin);
			ReleaseIColorSwatch(u->ccColorSwatch);
			ReleaseICustEdit(u->ccEdit);
			ReleaseICustEdit(u->ccTCBEdit);
			ReleaseISpinner(u->ccTCBSpin);
			ReleaseICustButton(u->ccButtonP);
			ReleaseICustButton(u->ccButtonC);
			ReleaseICustButton(u->ccFlyOff);
			ReleaseICustStatus(u->ccStatus);
			ReleaseICustToolbar(u->ccToolbar);
			ReleaseICustImage(u->ccImage);
			ReleaseISlider(u->ccSlider);
			ReleaseICustEdit(u->ccSliderEdit);
			ReleaseISlider(u->ccSlider1);
			ReleaseICustEdit(u->ccSliderEdit1);
			ReleaseISlider(u->ccSlider2);
			ReleaseICustEdit(u->ccSliderEdit2);

			ImageList_Destroy(u->hImageFO);
			ImageList_Destroy(u->hImageIM);
			ImageList_Destroy(u->hImageTB);
			break;
		}
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				// The user clicked on the Toolbar Enable checkbox.
				case IDC_TBCHECK: {
					if (IsDlgButtonChecked(hWnd, IDC_TBCHECK)) {
						// Enable the toolbar controls
						u->ccToolbar->Enable();
						// Update the images in the toolbar
						u->ccToolbar->GetICustButton(ID_TB_1)->Enable();
						u->ccToolbar->GetICustButton(ID_TB_2)->Enable();
						u->ccToolbar->GetICustButton(ID_TB_3)->Enable();
					} 
					else {
						// Disable the toolbar controls
						u->ccToolbar->Disable();
						// Update the images in the toolbar
						u->ccToolbar->GetICustButton(ID_TB_1)->Disable();
						u->ccToolbar->GetICustButton(ID_TB_2)->Disable();
						u->ccToolbar->GetICustButton(ID_TB_3)->Disable();
					};
					break;
				}
				case IDC_FLYOFF: {
					switch (HIWORD(wParam)) { // Notification codes 
						case BN_FLYOFF:
							// This notification code is sent when the user 
							// chooses a new fly off.
							index = u->ccFlyOff->GetCurFlyOff();
							_stprintf(s, _T("Fly Off Index: %d"), index);
							SetWindowText(GetDlgItem(hWnd, IDC_FLYOFFINDEX), s);
							break;
					};
					break;
				}
				case IDC_BUTTON_PUSH: {
					switch (HIWORD(wParam)) { // Notification codes 
						case BN_BUTTONDOWN:
							// Pick button is pressed.
							SetWindowText(GetDlgItem(hWnd, IDC_BUTTON_TEXT), _T("IN"));
							break;
					
						case BN_BUTTONUP:
							// Pick button is released.
							SetWindowText(GetDlgItem(hWnd, IDC_BUTTON_TEXT), _T("OUT"));
							break;

						case BN_RIGHTCLICK:
							// User has right clicked on Pick button.
							SetWindowText(GetDlgItem(hWnd, IDC_BUTTON_TEXT), _T("RIGHT"));
							break;
					};
					break;
				}
				default:
					break;
			}
			break;
		}
		case WM_NOTIFY: {
			// This is where we provide the tooltip text for the toolbar buttons...
			LPNMHDR hdr = (LPNMHDR)lParam;
			if (hdr->code == TTN_NEEDTEXT) {
				LPTOOLTIPTEXT lpttt;
				lpttt = (LPTOOLTIPTEXT)hdr;				
				switch (lpttt->hdr.idFrom) {
					case ID_TB_1:
						lpttt->lpszText = _T("Do Nothing Up");
						break;
					case ID_TB_2:
						lpttt->lpszText = _T("Do Nothing Down");
						break;
					case ID_TB_3:
						lpttt->lpszText = _T("Do Nothing Lock");
						break;
					case IDC_BUTTON_CHECK:
						if (u->ccButtonC->IsChecked())
							lpttt->lpszText = _T("Button Checked");
						else 
							lpttt->lpszText = _T("Button Un-Checked");
						break;
				};
			};
			break;
		}
		case WM_CUSTEDIT_ENTER: {
			// This message is sent when the user presses ENTER on
			// a custom edit control...
			break;
		}
		case CC_SPINNER_CHANGE: {
			switch (LOWORD(wParam)) { // Switch on ID
				case IDC_SPIN_SPINNER:
					// Update our seperate text indicator of the value.
					_stprintf(s, _T("Value: %.1f"), 
						((ISpinnerControl *)lParam)->GetFVal());
					SetWindowText(GetDlgItem(hWnd, IDC_SPINEDIT_VALUE), s);
					break;
				case IDC_SPIN_TCBSPIN:
					// Update the TCB Graph
					u->gp.tens = (((ISpinnerControl *)lParam)->GetFVal()/50.0f);
					HWND hGraph = GetDlgItem(hWnd, IDC_TCB_GRAPH);
					EnableWindow(hGraph, TRUE);
					SendMessage(hGraph,WM_SETTCBGRAPHPARAMS,0,(LPARAM)&u->gp);
					UpdateWindow(hGraph);
					break;
			};
			break;
		}
		case CC_SPINNER_BUTTONDOWN: {
			switch (LOWORD(wParam)) {
				case IDC_SPIN_SPINNER:
					// Indicate the down state of the spinner button.
					// Note this message is ALSO sent when the user enters a 
					// value by typing and then presses ENTER.  You will
					// see the DN indicator come up in the Spin MSG: line
					// if you do press ENTER.
					_stprintf(s, _T("Spin MSG: DN"));
					SetWindowText(GetDlgItem(hWnd, IDC_SPIN_STATUS), s);
					break;
			};
			break;
		}
		case CC_SPINNER_BUTTONUP: {
			switch (LOWORD(wParam)) {
				case IDC_SPIN_SPINNER:
					// Indicate the up state of the spinner button.
					_stprintf(s, _T("Spin MSG: UP"));
					SetWindowText(GetDlgItem(hWnd, IDC_SPIN_STATUS), s);
					break;
			};
			break;
		}
		case CC_COLOR_CHANGE: {
			// This message is sent when the user is changing the color
			// using the ColorSwatch control. lParam contains a pointer
			// to the ColorSwatch control.
			// What we do here is update the status control with the 
			// RGB values we get back from the ColorSwatch control.
			color = ((IColorSwatch *) lParam)->GetColor();
			_stprintf(s, _T("(%d, %d, %d)"), (int) GetRValue(color),
				(int) GetGValue(color), (int) GetBValue(color));
			u->ccStatus->SetText(s);
			break;
		}
		case CC_SLIDER_BUTTONUP:
				break;
		case CC_SLIDER_BUTTONDOWN:
				break;
		case CC_SLIDER_CHANGE: {

				int isind;
				switch (LOWORD(wParam)) { // Switch on ID
				case IDC_SLIDER:
					// Update our seperate text indicator of the value.
					_stprintf(s, _T("%.1f"), 
						((ISliderControl *)lParam)->GetFVal());
					SetWindowText(GetDlgItem(hWnd, IDC_SLIDE_VALUE), s);
					break;
				case IDC_SLIDER1:
					_stprintf(s, _T("%d"), 
						((ISliderControl *)lParam)->GetIVal());
					SetWindowText(GetDlgItem(hWnd, IDC_SLIDE1_VALUE), s);
					isind = ((ISliderControl *)lParam)->IsIndeterminate();
					// Update the TCB Graph
					if ( isind )
						((ISliderControl *)lParam)->SetIndeterminate(FALSE);
					else
						((ISliderControl *)lParam)->SetIndeterminate(TRUE);
					break;
				case IDC_SLIDER2:
					((ISliderControl *)lParam)->SetKeyBrackets(TRUE);
					break;
				}
				break;
		}
		default:
			return FALSE;
	};
	return TRUE;
}

// This is the dialog proc for the MAXDLG rollup page.
INT_PTR CALLBACK MaxDlgDlgProc(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	TSTR buf;

	CustCtrlUtil *u = (CustCtrlUtil *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!u && message != WM_INITDIALOG ) return FALSE;

	switch (message) {
		case WM_INITDIALOG:
			u = (CustCtrlUtil *)lParam;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)u);
			SetWindowText(GetDlgItem(hWnd, IDC_INFO1), _T(""));
			SetWindowText(GetDlgItem(hWnd, IDC_INFO2), _T(""));
			SetWindowText(GetDlgItem(hWnd, IDC_INFO3), _T(""));
		 	return TRUE;

		case WM_DESTROY:
			if (u->arcb.GetDialog()) {
				u->arcb.GetDialog()->DeleteThis();
				u->arcb.SetDialog(NULL);
			}
			break;

		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDC_COLOR_DLG: {
					COLORREF color;
					int ok = HSVDlg_Do(u->ip->GetMAXHWnd(), &color, NULL, NULL, _T("Pick a Color"));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO1), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO2), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO3), _T(""));
					if (ok) {
						buf.printf(_T("Red=%d"), GetRValue(color));
						SetWindowText(GetDlgItem(hWnd, IDC_INFO1), buf.data());
						buf.printf(_T("Green=%d"), GetGValue(color));
						SetWindowText(GetDlgItem(hWnd, IDC_INFO2), buf.data());
						buf.printf(_T("Blue=%d"), GetBValue(color));
						SetWindowText(GetDlgItem(hWnd, IDC_INFO3), buf.data());
					}
					break;
				}
				case IDC_TVP_DLG: {
					TSTR str;
					TrackViewPick tvp;
					BOOL ok = u->ip->TrackViewPickDlg(u->ip->GetMAXHWnd(), &tvp);
					SetWindowText(GetDlgItem(hWnd, IDC_INFO1), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO2), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO3), _T(""));
					if (ok) {
						tvp.anim->GetClassName(str);
						buf.printf(_T("Anim=%s"), str);
						SetWindowText(GetDlgItem(hWnd, IDC_INFO1), buf.data());
						if (tvp.client) {
							tvp.client->GetClassName(str);
							buf.printf(_T("Client=%s"), str);
							SetWindowText(GetDlgItem(hWnd, IDC_INFO2), buf.data());
						}
						buf.printf(_T("subNum=%d"), tvp.subNum);
						SetWindowText(GetDlgItem(hWnd, IDC_INFO3), buf.data());
					}
					break;
				}
				case IDC_EXCL_DLG: {
					ExclList nl;
					int ok = u->ip->DoExclusionListDialog(&nl);
					SetWindowText(GetDlgItem(hWnd, IDC_INFO1), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO2), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO3), _T(""));
					if (ok) {
						buf.printf(_T("# Picked=%d"), nl.Count());
						SetWindowText(GetDlgItem(hWnd, IDC_INFO1), buf.data());
					}
					break;
				}
				case IDC_MTL_DLG: {
					BOOL newMat, cancel;
					MtlBase *m = u->ip->DoMaterialBrowseDlg(u->ip->GetMAXHWnd(),
						BROWSE_MATSONLY|BROWSE_INCNONE, newMat, cancel);
					SetWindowText(GetDlgItem(hWnd, IDC_INFO1), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO2), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO3), _T(""));
					if (!cancel) {
						buf.printf(_T("%s"), m->GetName());
						SetWindowText(GetDlgItem(hWnd, IDC_INFO1), buf.data());
					}
					break;
				}
				case IDC_HBN_DLG: {
					BOOL ok = u->ip->DoHitByNameDialog();
					SetWindowText(GetDlgItem(hWnd, IDC_INFO1), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO2), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO3), _T(""));
					if (ok) {
						buf.printf(_T("# selected=%d"), u->ip->GetSelNodeCount());
						SetWindowText(GetDlgItem(hWnd, IDC_INFO1), buf.data());
					}
					break;
				}
				case IDC_NCP_DLG: {
					DWORD color;
					BOOL ok = u->ip->NodeColorPicker(u->ip->GetMAXHWnd(), color);
					SetWindowText(GetDlgItem(hWnd, IDC_INFO1), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO2), _T(""));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO3), _T(""));
					if (ok) {
						buf.printf(_T("Red=%d"), GetRValue(color));
						SetWindowText(GetDlgItem(hWnd, IDC_INFO1), buf.data());
						buf.printf(_T("Green=%d"), GetGValue(color));
						SetWindowText(GetDlgItem(hWnd, IDC_INFO2), buf.data());
						buf.printf(_T("Blue=%d"), GetBValue(color));
						SetWindowText(GetDlgItem(hWnd, IDC_INFO3), buf.data());
					}
					break;
				}
				case IDC_ARC_DLG: {
					if (u->arcb.GetDialog())
						u->arcb.GetDialog()->DeleteThis();
					GenBoxObject *gb = (GenBoxObject *)CreateInstance(GEOMOBJECT_CLASS_ID, Class_ID(BOXOBJ_CLASS_ID, 0));
					gb->SetParams(10.0f, 20.0f, 30.0f);
					INode *node = u->ip->CreateObjectNode(gb);
					TSTR name(_T("MyDemoBox"));
					u->ip->MakeNameUnique(name);
					node->SetName(name);
					u->ip->SelectNode(node);
					u->ip->ExecuteMAXCommand(MAXCOM_ZOOMEXT_SEL_ALL);

					u->arcb.SetIP(u->ip);
					u->arcb.SetNode(node);
					ArcballDialog *ad = CreateArcballDialog(&u->arcb, 
						u->ip->GetMAXHWnd(), _T("Arc Rotate Node"));
					u->arcb.SetDialog(ad);
				}
				break;
				case IDC_CHOOSE_DIR: {
					TCHAR dir[256];
					TCHAR desc[256];
					_tcscpy(desc, _T("Desc String"));
					u->ip->ChooseDirectory(u->ip->GetMAXHWnd(), 
						_T("Title String"), dir, desc);
					buf.printf(_T("Dir and Label"));
					SetWindowText(GetDlgItem(hWnd, IDC_INFO1), buf.data());
					buf.printf(_T("%s"), dir);
					SetWindowText(GetDlgItem(hWnd, IDC_INFO2), buf.data());
					buf.printf(_T("%s"), desc);
					SetWindowText(GetDlgItem(hWnd, IDC_INFO3), buf.data());
				}
				break;
			};
			break;
		}
		default:
			return FALSE;
	};
	return TRUE;
}

/*===========================================================================*\
 | Inheritec virtual methods from UtilityObj
\*===========================================================================*/
void CustCtrlUtil::BeginEditParams(Interface *ip, IUtil *iu) {
	this->iu = iu;
	this->ip = ip;

	// Add the rollup page for the CUSTCTRL dialog 
	hCustCtrl = ip->AddRollupPage(
		hInstance, 
		MAKEINTRESOURCE(IDD_CUSTCTRL),
		CustCtrlDlgProc,
		_T("MAX Custom Controls"), 
		(LPARAM)this);		

	// Add the rollup page for the MAXDLG dialog 
	hMaxDlg = ip->AddRollupPage(
		hInstance, 
		MAKEINTRESOURCE(IDD_MAXDLG),
		MaxDlgDlgProc,
		_T("MAX Standard Dialogs"), 
		(LPARAM)this);		
}
	
void CustCtrlUtil::EndEditParams(Interface *ip, IUtil *iu) {
	// Delete the rollup pages
	ip->DeleteRollupPage(hCustCtrl);		
	ip->DeleteRollupPage(hMaxDlg);		
	hCustCtrl = NULL;				
	hMaxDlg = NULL;
}

/*===========================================================================*\
 | Methods of CustCtrlUtil
\*===========================================================================*/
CustCtrlUtil::CustCtrlUtil() { 
	iu = NULL;
	ip = NULL;
	hCustCtrl = NULL;
	hMaxDlg = NULL; 
}

// Initialize all the Custom Controls
void CustCtrlUtil::OnInitDialog(HWND hWnd) {
	// Init the TCB values
	gp.tens = 0.0f;
	gp.bias = 0.0f;
	gp.cont = 0.0f;
	gp.easeFrom = 0.0f;
	gp.easeTo = 0.0f;

	// Initialize the Custom Edit Control
	ccEdit = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT));
	ccEdit->SetText(_T("Hello world"));

 	// Initialize the Custom Spinner Control
 	ccSpin = GetISpinner(GetDlgItem(hWnd, IDC_SPIN_SPINNER));
	ccSpin->SetLimits(0.0f, 100.0f, FALSE);
	ccSpin->SetResetValue(100.0f); // Right click reset value
	ccSpin->SetValue(100.0f, FALSE); // FALSE = don't send notify yet.
	ccSpin->LinkToEdit(GetDlgItem(hWnd, IDC_SPIN_EDIT), 
		EDITTYPE_UNIVERSE);
	/*
	 * NOTE: the above code could be replaced with the following 
	 * simplified call when using integer or float spinners:

	ccSpin = SetupFloatSpinner(hWnd, IDC_SPIN_SPINNER,
		IDC_SPIN_EDIT, 0.0f, 100.0f, 100.0f);

	 * The integer version is called SetupIntSpinner()...
	*/

	ccSlider  = SetupFloatSlider(hWnd, IDC_SLIDER,IDC_SLIDER_EDIT, 0.0f, 100.0f, 30.0f, 10);
	ccSlider->SetResetValue(25.0f);
	
	ccSlider1 = SetupIntSlider(hWnd, IDC_SLIDER1, IDC_SLIDER_EDIT1, 0, 100, 50, 10);
	ccSlider1->SetResetValue(25);

	// Initialize the Custom Slider Control
	ccSlider2 = GetISlider(GetDlgItem(hWnd, IDC_SLIDER2));
	ccSlider2->SetLimits (0.0f, 100.0f, FALSE);
	ccSlider2->SetResetValue(50.0f); //Right click reset value
	ccSlider2->SetValue(50.0f, FALSE); // FALSE = don't send notify yet
	ccSlider2->LinkToEdit(GetDlgItem(hWnd, IDC_SLIDER_EDIT2), EDITTYPE_UNIVERSE);
	ccSlider2->SetNumSegs(10);

	//The previous calls could be replaced with the following simplified call

	//	ccSlider2 = SetupUniverseSlider(hWnd, IDC_SLIDER2,IDC_SLIDER_EDIT2, 0.0f, 100.0f, 50.0f, 10);
	
	SetSliderDragNotify(FALSE);

	// Init the TCB Graph spinner
	ccTCBSpin = SetupFloatSpinner(hWnd, IDC_SPIN_TCBSPIN,
		IDC_SPIN_TCBEDIT, 0.0f, 50.0f, 0.0f);
	ccTCBSpin->SetResetValue(0.0f);


	// Initialize the Custom Button Control-Push Button
	ccButtonP = GetICustButton(GetDlgItem(hWnd, IDC_BUTTON_PUSH));
	ccButtonP->SetType(CBT_PUSH);
	ccButtonP->SetRightClickNotify(TRUE);
	ccButtonP->SetButtonDownNotify(TRUE);
	ccButtonP->SetTooltip(TRUE, &(_T("Push Button Tooltip")));
	SetWindowText(GetDlgItem(hWnd, IDC_BUTTON_TEXT), _T("OUT"));// Indicator

	// Initialize the Custom Button Control-Check Button
	ccButtonC = GetICustButton(GetDlgItem(hWnd, IDC_BUTTON_CHECK));
	ccButtonC->SetType(CBT_CHECK);
	// Set the color when pressed - GREEN_WASH is defined in custcont.h
	ccButtonC->SetHighlightColor(GREEN_WASH);
	// Set tooltip to use text based on program state
	ccButtonC->SetTooltip(TRUE, LPSTR_TEXTCALLBACK);

	// Initialize the Custom Button Control-Fly Off
	ccFlyOff = GetICustButton(GetDlgItem(hWnd, IDC_FLYOFF));
	// These FlyOffData indicies control the appearance of the flyoff
	// buttons for each of four states:
	// { Out&Enabled, In&Enabled, Out&Disabled, In&Disabled }
	// This flyoff treats each case as the same and thus all the indicies
	// are the same.
	FlyOffData fod[3] = { // We have three different icons.
		{ 0,0,0,0 },
		{ 1,1,1,1 },
		{ 2,2,2,2 },
		};
	ccFlyOff->SetFlyOff(3, fod, ip->GetFlyOffTime(), 0 /* Initial value */, 
		FLY_RIGHT);
	// Here we load up the images for the flyoff...
	hImageFO = ImageList_Create(IMAGE_W, IMAGE_H, TRUE, 3, 3);
	HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FLYOFFIMAGE));
	HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FLYOFFMASK));
	ImageList_Add(hImageFO, hBitmap, hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	ccFlyOff->SetImage(hImageFO, 0,0,0,0, IMAGE_W, IMAGE_H);

	// Initialize the Custom Image Control
	// Our image is 80x50 pixels...
	ccImage = GetICustImage(GetDlgItem(hWnd, IDC_IMAGE));
	hImageIM = ImageList_Create(80, 50, TRUE, 1, 0);
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_IMAGE));
	hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_IMAGEMASK));
	ImageList_Add(hImageIM, hBitmap, hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	// Normal one...
	ccImage->SetImage(hImageIM, 0, 80, 50);

	// Initialize the Custom Toolbar Control
	ccToolbar = GetICustToolbar(GetDlgItem(hWnd, IDC_TOOLBAR));
	ccToolbar->SetBottomBorder(TRUE);
	ccToolbar->SetTopBorder(TRUE);
	hImageTB = ImageList_Create(16, 15, TRUE, 8, 0);
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TB));
	hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TBMASK));
	ImageList_Add(hImageTB, hBitmap, hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	ccToolbar->SetImage(hImageTB);

	// Add a little space at the left edge
	ccToolbar->AddTool(ToolSeparatorItem(3));
	// Add two push buttons
	// These buttons have two images each
	// An Enabled state and a Disabled state.
	// The indicies point to a specific 16 pixel chunk of the resource image.
	ccToolbar->AddTool(
		ToolButtonItem(CTB_PUSHBUTTON,  0,0,2,2,  16,15,
			TB_BUT_WIDTH,TB_BUT_HEIGHT,ID_TB_1));	
	ccToolbar->AddTool(
		ToolButtonItem(CTB_PUSHBUTTON,  1,1,3,3,  16,15,
			TB_BUT_WIDTH,TB_BUT_HEIGHT,ID_TB_2));	
	// A little more space
	ccToolbar->AddTool(ToolSeparatorItem(6));
	// Add a check button.  This button uses four different images for 
	// the possible states:
	// Out&Enabled, In&Enabled, Out&Disabled, In&Disabled
	// The indicies point to a specific 16 pixel chunk of the resource image.
	ccToolbar->AddTool(
		ToolButtonItem(CTB_CHECKBUTTON,  4,5,6,7,  16,15,
			TB_BUT_WIDTH,TB_BUT_HEIGHT,ID_TB_3));	
	// These methods allow the button to be highlighted in a specific color.
	// GREEN_WASH is the standard color for MAX buttons.
	ICustButton *getCustButton;
	getCustButton = ccToolbar->GetICustButton(ID_TB_3);
	getCustButton->SetCheckHighlight(TRUE);
	getCustButton->SetHighlightColor(GREEN_WASH);
	ReleaseICustButton(getCustButton);

	// Set the TB Enabled check box to true by default.
	CheckDlgButton(hWnd,IDC_TBCHECK, TRUE);

	// Initialize the Custom Status Control
	ccStatus = GetICustStatus(GetDlgItem(hWnd, IDC_STATUS));
	ccStatus->SetTextFormat(STATUSTEXT_CENTERED);
	ccStatus->SetText(_T("(0,255,128)"));

	// Initialize the Color Swatch Control
	ccColorSwatch = GetIColorSwatch(
		GetDlgItem(hWnd, IDC_COLORSWATCH),
		RGB(0,255,128), _T("ColorSwatch Dialog Box"));
	ccColorSwatch->SetColor(RGB(0,255,128), TRUE);
}

/*===========================================================================*\
 | Arc rotate callback methods
\*===========================================================================*/
void MyArcDlgCB::StartDrag() {
	Matrix3 tm = node->GetNodeTM(ip->GetTime());
	qstart = Quat(tm);
}

void MyArcDlgCB::Drag(Quat q, BOOL buttonUp) {
	Matrix3 tm;
	MakeMatrix(qstart*q, tm);
	node->SetNodeTM(ip->GetTime(), tm);
	ip->RedrawViews(ip->GetTime());
}

void MyArcDlgCB::CancelDrag() {
	Matrix3 tm;
	MakeMatrix(qstart, tm);
	node->SetNodeTM(ip->GetTime(), tm);
	ip->RedrawViews(ip->GetTime());
}

void MyArcDlgCB::EndDrag() {
}

void MyArcDlgCB::BeingDestroyed() {
	theUtility.ip->DeleteNode(node);
	ad = NULL;
}

void MyArcDlgCB::MakeMatrix(Quat q, Matrix3 &tm) {
	float ang[3];
	tm.IdentityMatrix();
	QuatToEuler(q, ang);
	tm.RotateX(ang[0]);
	tm.RotateY(ang[1]);
	tm.RotateZ(ang[2]);
}

/*===========================================================================*\
 | Class Descriptor
\*===========================================================================*/
class UtilityClassDesc : public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theUtility;}
	const TCHAR *	ClassName() {return MY_CLASSNAME;}
	SClass_ID		SuperClassID() {return SClass_ID(UTILITY_CLASS_ID);}
	Class_ID		ClassID() {return MY_CLASS_ID;}
	const TCHAR* 	Category() {return MY_CATEGORY;}
};
static UtilityClassDesc utilityCDesc;

/*===========================================================================*\
 | Lib and DLL Functions
\*===========================================================================*/
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {	
	hInstance = hinstDLL;
	if (! controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitTCBGraph(hInstance);
		InitCommonControls();
	}
	return(TRUE);
}

__declspec(dllexport) const TCHAR *LibDescription() { 
	return MY_LIBDESCRIPTION;
}

__declspec(dllexport) int LibNumberClasses() { 
	return 1;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i) { 
	return &utilityCDesc; 
}

__declspec(dllexport) ULONG LibVersion() { 
	return VERSION_3DSMAX; 
}

