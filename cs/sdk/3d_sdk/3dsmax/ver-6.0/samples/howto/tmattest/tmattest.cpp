/*===========================================================================*\
 |    File: tmattest.cpp
 |
 | Purpose: To allow the user to visualize the way transformation matrices
 |          are constructed in 3D Studio MAX.
 |
 | History: Mark Meier, Began 11/96.
 |          MM, 03/97, Modified for use with R2.
 |          MM, 04/97, Bug fixes, added viewing Object Offset Transformation.
\*===========================================================================*/
/*===========================================================================*\
 | Include Files
\*===========================================================================*/
#include "MAX.H"		// Main MAX include file
#include "utilapi.h"	// Utility Plug-in include file
#include "iparamm.h"	// Parameter Map include file
#include "resource.h"	// Resource editor include file

/*===========================================================================*\
 | Misc Defines
\*===========================================================================*/
#define MYUTIL_CLASS_ID Class_ID(0x6f5d6227, 0x47dd46ab)
#define CLASSNAME _T("Transform Test")
#define CATEGORY _T("How To")
#define LIBDESCRIPTION _T("Transformation Matrix Tester by Mark Meier")
#define UTIL_MSG_TITLE _T("Transform Tester")
#define UNDO_STRING TSTR(_T("Modify Node TM"))

// Parameter Map indices
#define PM_DO_TRANS 0
#define PM_DO_SCALE 1
#define PM_DO_X_ROTATE 2
#define PM_DO_Y_ROTATE 3
#define PM_DO_Z_ROTATE 4

#define PM_X_TRANS 5
#define PM_Y_TRANS 6
#define PM_Z_TRANS 7

#define PM_X_SCALE 8
#define PM_Y_SCALE 9
#define PM_Z_SCALE 10

#define PM_ROTATE 11
#define PM_AXIS_CHOICE 12

#define PM_DO_AUTO 13

// This is the DLL instance handle
HINSTANCE hInstance;

/*===========================================================================*\
 | Class definitions
\*===========================================================================*/
class Utility : public UtilityObj, public IParamArray {
	public:
	IUtil *iu;
	Interface *ip;

	// Variables for display of matrix values in a dialog
	float m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
	TCHAR viewTitle[128];

	// Custom controls used in rotation dialog
	ISpinnerControl *rotSpin;
	ICustEdit *rotEdit;

	// Parameter maps
	IParamMap *pmapMain;
	IParamMap *pmapScale;
	IParamMap *pmapTrans;
	IParamMap *pmapRot;

	HWND hViewTM;

	// Variables set by the parameter map
	int do_trans, do_scale, do_x_rotate, do_y_rotate, do_z_rotate, do_auto;
	int axis_choice;
	float x_trans, y_trans, z_trans;
	float x_scale, y_scale, z_scale;
	float angle;
		
	float x_rotate,y_rotate,z_rotate; // 'angle' gets assigned to one of these

	// Node in the scene we update by settings its Node TM.
	INode *node;

	// Methods from UtilityObj
	void BeginEditParams(Interface *ip, IUtil *iu);
	void EndEditParams(Interface *ip, IUtil *iu);
	void SelectionSetChanged(Interface *ip, IUtil *iu);
	void DeleteThis() {}  // Null since we use a single static instance

	// Methods from IParamArray
	BOOL SetValue(int i, TimeValue t, int v);
	BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
	BOOL SetValue(int i, TimeValue t, float v);
	BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);

	// Local methods...
	Utility();
	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	void ViewTrans();
	void ViewScale();
	void ViewXRotate();
	void ViewYRotate();
	void ViewZRotate();
	void ViewComposite();

	void ViewWorldRotation();
	void ViewLocalRotation();

	void UpdateNodeTM();
	void UpdateTM(Matrix3 &tmat);
	void LoadViewMatVars(Matrix3 mat);
	void ResetDefaultValues();
	void UpdateSinCosDisplay();
	void RedrawRollups();

	void ViewNodeTM();
	void ViewObjectTM();
	void ViewObjectOffset();
	void ViewObjTMBeforeWSM();
	void ViewObjTMAfterWSM();
	void ViewParentTM();
	void ViewTargetTM();
	void ViewObjectStateTM();
 	void ViewTMController();
	void SetMatrixTitleString(TCHAR *s, Interval v, INode *n);
};

// This is the static instance of the Utility plug-in.
static Utility theUtility;

// This is the restore object that contains the information allowing us
// to Undo or Redo the operation of setting the node TM.
class NodeTMRestore : public RestoreObj {
	public:
	Utility *uo;
	INode *undoNode, *redoNode;
	Matrix3 undoTM, redoTM;
	TimeValue undoTime, redoTime;

	// Constructor
	NodeTMRestore(Utility *u, INode *n, Matrix3 m, TimeValue t) { 
		uo = u; undoNode = n; undoTM = m; undoTime = t;
	}

	// Called when Undo is selected
	void Restore(int isUndo) {
		if (!isUndo)
			// This is NOT an undo operation ... we shouldn't get any of these
			// (but if we do, I'd like to know about it <g>)
			assert(0);
		// Make sure this is the same node...
		if (theUtility.node == undoNode) {
			//	1) Save the current node for a redo
			//	2) Save the current TM
			//	3) Save the current time
			redoTM = uo->node->GetNodeTM(undoTime);
			redoNode = uo->node;
			redoTime = undoTime;

			// Undo the operation
			undoNode->SetNodeTM(undoTime, undoTM);
			// Update the rollups and the viewports
			uo->RedrawRollups();
			uo->ip->RedrawViews(uo->ip->GetTime());
		}									
	}

	// Called when Redo is selected
	void Redo() {
		// Make sure this is the same node...
		if (uo->node == redoNode) {
			//	1) Save the current node
			//	2) Save the current TM
			//	3) Save the current time
			undoTM = uo->node->GetNodeTM(undoTime);
			undoNode = uo->node;
			undoTime = uo->ip->GetTime();

			// Redo the operation
			redoNode->SetNodeTM(redoTime, redoTM);
			// Update the rollups and the viewports
			uo->RedrawRollups();
			uo->ip->RedrawViews(uo->ip->GetTime());
		}
	}

	// Called to return the size in bytes of this RestoreObj
	int Size() {
		return sizeof(NodeTMRestore);
	}
};

/*===========================================================================*\
 | Parameter Map User Interface Descriptors and Dialog Procs
\*===========================================================================*/
// The following are arrays used by the Parameter Map mechanism to 
// describe the user interface controls.
static ParamUIDesc descMainParam[] = {
	// Check boxes
	ParamUIDesc(PM_DO_X_ROTATE, TYPE_SINGLECHEKBOX, IDC_CHECK_USE_X),
	ParamUIDesc(PM_DO_Y_ROTATE, TYPE_SINGLECHEKBOX, IDC_CHECK_USE_Y),
	ParamUIDesc(PM_DO_Z_ROTATE, TYPE_SINGLECHEKBOX, IDC_CHECK_USE_Z),
	ParamUIDesc(PM_DO_TRANS, TYPE_SINGLECHEKBOX, IDC_CHECK_USE_TR),
	ParamUIDesc(PM_DO_SCALE, TYPE_SINGLECHEKBOX, IDC_CHECK_USE_SC),
	ParamUIDesc(PM_DO_AUTO, TYPE_SINGLECHEKBOX, IDC_CHECK_AUTO),
};
#define MAIN_DESC_LENGTH 6

static ParamUIDesc descTransParam[] = {
	// Float spinners
	ParamUIDesc(PM_X_TRANS, EDITTYPE_UNIVERSE, IDC_SEDIT_XTR, IDC_SPIN_XTR,
		-1000000.0f, 1000000.0f, 1.0f),
	ParamUIDesc(PM_Y_TRANS, EDITTYPE_UNIVERSE, IDC_SEDIT_YTR, IDC_SPIN_YTR,
		-1000000.0f, 1000000.0f, 1.0f),
	ParamUIDesc(PM_Z_TRANS, EDITTYPE_UNIVERSE, IDC_SEDIT_ZTR, IDC_SPIN_ZTR,
		-1000000.0f, 1000000.0f, 1.0f),
};
#define TRANS_DESC_LENGTH 3

static ParamUIDesc descScaleParam[] = {
	// Float spinners
	ParamUIDesc(PM_X_SCALE, EDITTYPE_FLOAT, IDC_SEDIT_XSC, IDC_SPIN_XSC,
		0.001f, 100.0f, 1.0f),
	ParamUIDesc(PM_Y_SCALE, EDITTYPE_FLOAT, IDC_SEDIT_YSC, IDC_SPIN_YSC,
		0.001f, 100.0f, 1.0f),
	ParamUIDesc(PM_Z_SCALE, EDITTYPE_FLOAT, IDC_SEDIT_ZSC, IDC_SPIN_ZSC,
		0.001f, 100.0f, 1.0f),
};
#define SCALE_DESC_LENGTH 3

static int axisIDs[] = { IDC_RADIOX, IDC_RADIOY, IDC_RADIOZ };

static ParamUIDesc descRotateParam[] = {
	ParamUIDesc(PM_AXIS_CHOICE, TYPE_RADIO, axisIDs, 3)
};
#define ROT_DESC_LENGTH 1

// The following are Dialog Proc used to process the user interface
// controls not handled directly by the parameter maps
// This is the dialog proc for the matrix viewer
static BOOL CALLBACK ViewDlgProc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	TCHAR s[32];
	switch (msg) {
		case WM_INITDIALOG:
			CenterWindow(hWnd, GetParent(hWnd));
			SetWindowText(GetDlgItem(hWnd, IDC_VIEW_TITLE), 
				theUtility.viewTitle);

			_stprintf(s, _T("%.3f"), theUtility.m00);
			SetWindowText(GetDlgItem(hWnd, IDC_M00), s);
			_stprintf(s, _T("%.3f"), theUtility.m10);
			SetWindowText(GetDlgItem(hWnd, IDC_M10), s);
			_stprintf(s, _T("%.3f"), theUtility.m20);
			SetWindowText(GetDlgItem(hWnd, IDC_M20), s);
			_stprintf(s, _T("%.3f"), theUtility.m30);
			SetWindowText(GetDlgItem(hWnd, IDC_M30), s);
			_stprintf(s, _T("%.3f"), theUtility.m01);
			SetWindowText(GetDlgItem(hWnd, IDC_M01), s);
			_stprintf(s, _T("%.3f"), theUtility.m11);
			SetWindowText(GetDlgItem(hWnd, IDC_M11), s);
			_stprintf(s, _T("%.3f"), theUtility.m21);
			SetWindowText(GetDlgItem(hWnd, IDC_M21), s);
			_stprintf(s, _T("%.3f"), theUtility.m31);
			SetWindowText(GetDlgItem(hWnd, IDC_M31), s);
			_stprintf(s, _T("%.3f"), theUtility.m02);
			SetWindowText(GetDlgItem(hWnd, IDC_M02), s);
			_stprintf(s, _T("%.3f"), theUtility.m12);
			SetWindowText(GetDlgItem(hWnd, IDC_M12), s);
			_stprintf(s, _T("%.3f"), theUtility.m22);
			SetWindowText(GetDlgItem(hWnd, IDC_M22), s);
			_stprintf(s, _T("%.3f"), theUtility.m32);
			SetWindowText(GetDlgItem(hWnd, IDC_M32), s);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hWnd,1);
					break;
				}
			break;
		
		default:
			return FALSE;
		}
	return TRUE;
}

class MainUserDlgProc : public ParamMapUserDlgProc {
	public:
		Utility *uo;
		MainUserDlgProc(Utility *u) {uo = u;}
		BOOL DlgProc(TimeValue t,IParamMap *map,
			HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
};

BOOL MainUserDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
			uo->Init(hWnd);			
			return FALSE;
		
		case WM_DESTROY:
			uo->Destroy(hWnd);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_VIEW_CURRENT:
					// Initialize and put up the matrix viewer dialog box
					uo->ViewComposite();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						hWnd, ViewDlgProc);
					return TRUE;

				case IDC_NODE_TM:
					uo->UpdateNodeTM();
					return TRUE;

				case IDC_RESET: {
					// Verify..
					TSTR buf;
					buf.printf(_T("Really reset ALL transformation and dialog values?"));
					int res = MessageBox(uo->ip->GetMAXHWnd(), buf, 
						UTIL_MSG_TITLE, MB_YESNO|MB_ICONQUESTION);
					if (res == IDNO) break;

					// Hold the previous value of do_auto so we know
					// if we need to redraw after the values are reset
					int hold_do_auto = uo->do_auto;
					// Reset all values to their defaults...
					uo->ResetDefaultValues();
					// Make sure all the controls get redrawn
					uo->RedrawRollups();
					// If Auto is on, update the node TM to reflect this change
					if (hold_do_auto) {
						uo->UpdateNodeTM();
					}
					return TRUE;
				}
			}	
			break;

		default:
			return FALSE;
		}
	return FALSE; 
}

static INT_PTR CALLBACK ViewTMDlgProc(HWND hWnd, UINT msg, 
	WPARAM wParam, LPARAM lParam) {
 	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_NODETM: {
					theUtility.ViewNodeTM();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						GetCOREInterface()->GetMAXHWnd(), ViewDlgProc);
							 }
					return TRUE;

				case IDC_OBJECTTM:
					theUtility.ViewObjectTM();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						GetCOREInterface()->GetMAXHWnd(), ViewDlgProc);
					return TRUE;

				case IDC_OBJ_OFFSET:
					theUtility.ViewObjectOffset();
					return TRUE;

				case IDC_OBJTMBEFORE:
					theUtility.ViewObjTMBeforeWSM();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						GetCOREInterface()->GetMAXHWnd(), ViewDlgProc);
					return TRUE;

				case IDC_OBJTMAFTER:
					theUtility.ViewObjTMAfterWSM();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						GetCOREInterface()->GetMAXHWnd(), ViewDlgProc);
					return TRUE;

				case IDC_PARENTTM:
					theUtility.ViewParentTM();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						GetCOREInterface()->GetMAXHWnd(), ViewDlgProc);
					return TRUE;

				case IDC_TARGETTM:
					theUtility.ViewTargetTM();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						GetCOREInterface()->GetMAXHWnd(), ViewDlgProc);
					return TRUE;

				case IDC_OBJECTSTATE:
					theUtility.ViewObjectStateTM();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						GetCOREInterface()->GetMAXHWnd(), ViewDlgProc);
					return TRUE;

				case IDC_TM:
					theUtility.ViewTMController();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						GetCOREInterface()->GetMAXHWnd(), ViewDlgProc);
					return TRUE;
			}	
			break;

		default:
			return FALSE;
		}
	return FALSE; 
}

class TransUserDlgProc : public ParamMapUserDlgProc {
	public:
		Utility *uo;
		TransUserDlgProc(Utility *u) {uo = u;}
		BOOL DlgProc(TimeValue t,IParamMap *map,
			HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
};

BOOL TransUserDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_BUTTON_EDITTR:
					// Initialize and put up the matrix viewer dialog box
					uo->ViewTrans();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						hWnd, ViewDlgProc);
				break;				
			}	
			break;
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			// If Auto is on, update the node TM to reflect this change
			if (uo->do_auto) {
				uo->UpdateNodeTM();
			}
		default:
			return FALSE;
	}
	return FALSE; 
}

class ScaleUserDlgProc : public ParamMapUserDlgProc {
	public:
		Utility *uo;
		ScaleUserDlgProc(Utility *u) {uo = u;}
		BOOL DlgProc(TimeValue t,IParamMap *map,
			HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
};

BOOL ScaleUserDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_BUTTON_EDITSC:
					// Initialize and put up the matrix viewer dialog box
					uo->ViewScale();
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						hWnd, ViewDlgProc);
				break;				
			}	
			break;
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			// If Auto is on, update the node TM to reflect this change
			if (uo->do_auto) {
				uo->UpdateNodeTM();
			}
		default:
			return FALSE;
	}
	return FALSE;
}

class RotateUserDlgProc : public ParamMapUserDlgProc {
	public:
		Utility *uo;
		RotateUserDlgProc(Utility *u) {uo = u;}
		BOOL DlgProc(TimeValue t,IParamMap *map,
			HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
};

BOOL RotateUserDlgProc::DlgProc(TimeValue t, IParamMap *map,
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_BUTTON_VIEW:
					// Initialize and put up the matrix viewer dialog box
					switch (uo->axis_choice) {
						case 0: // X
							uo->ViewXRotate();
							break;
						case 1: // Y
							uo->ViewYRotate();
							break;
						case 2: // Z
							uo->ViewZRotate();
							break;
					}
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_VIEW),
						hWnd, ViewDlgProc);
					break;
				case IDC_RADIOX:
					// Update matrix display for X
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE1), _T("|  1  0  0  |"));
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE2), _T("|  0  c  s  |"));
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE3), _T("|  0 -s  c  |"));
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_BORDER), _T("X Rotation Matrix Layout"));
					uo->rotSpin->SetValue(uo->x_rotate, FALSE);
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_BORDER), _T("X Rotation Matrix Layout"));
					uo->UpdateSinCosDisplay();
					break;
				case IDC_RADIOY:
					// Update matrix display for Y
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE1), _T("|  c  0 -s  |"));
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE2), _T("|  0  1  0  |"));
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE3), _T("|  s  0  c  |"));
					uo->rotSpin->SetValue(uo->y_rotate, FALSE);
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_BORDER), _T("Y Rotation Matrix Layout"));
					uo->UpdateSinCosDisplay();
					break;
				case IDC_RADIOZ:
					// Update matrix display for Z
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE1), _T("|  c  s  0  |"));
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE2), _T("| -s  c  0  |"));
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_MATLINE3), _T("|  0  0  1  |"));
					uo->rotSpin->SetValue(uo->z_rotate, FALSE);
					SetWindowText(GetDlgItem(uo->pmapRot->GetHWnd(), 
						IDC_BORDER), _T("Z Rotation Matrix Layout"));
					uo->UpdateSinCosDisplay();
					break;
			}
		break;
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			// There is only one spinner/edit control -- the angle 
			switch(uo->axis_choice) {
			case 0: // X
				uo->x_rotate = uo->rotSpin->GetFVal(); 
				break;
			case 1: // Y
				uo->y_rotate = uo->rotSpin->GetFVal(); 
				break;
			case 2: // Z
				uo->z_rotate = uo->rotSpin->GetFVal(); 
				break;
			}
			uo->pmapRot->Invalidate();
			uo->UpdateSinCosDisplay();
			// If Auto is on, update the node TM to reflect this change
			if (uo->do_auto) {
				uo->UpdateNodeTM();
			}
			break;
		default:
			return FALSE;
	}
	return FALSE; 
}

/*===========================================================================*\
 | Utility Plug-In Methods
\*===========================================================================*/
// From UtilityObj
void Utility::BeginEditParams(Interface *ip, IUtil *iu) {
	this->iu = iu;
	this->ip = ip;
	// Create the rollups in the command panel...
	hViewTM = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_VIEW_TM),
		ViewTMDlgProc, _T("View Various TMs"), (LPARAM)0, 0);		

	pmapMain = CreateCPParamMap(descMainParam, MAIN_DESC_LENGTH,
		this, ip, hInstance, MAKEINTRESOURCE(IDD_MAIN), _T("Transformations"),
		0);

	pmapTrans = CreateCPParamMap(descTransParam, TRANS_DESC_LENGTH,
		this, ip, hInstance, MAKEINTRESOURCE(IDD_TRANS), _T("Translate"),
		APPENDROLL_CLOSED);

	pmapScale = CreateCPParamMap(descScaleParam, SCALE_DESC_LENGTH,
		this, ip, hInstance, MAKEINTRESOURCE(IDD_SCALE), _T("Scale"),
		APPENDROLL_CLOSED);

	pmapRot = CreateCPParamMap(descRotateParam, ROT_DESC_LENGTH,
		this, ip, hInstance, MAKEINTRESOURCE(IDD_ROT), _T("Rotate"),
		APPENDROLL_CLOSED);

	// Indicate we'll use our own dialog procs to handle 
	// some of the buttons...
	pmapMain->SetUserDlgProc(new MainUserDlgProc(this));
	pmapTrans->SetUserDlgProc(new TransUserDlgProc(this));
	pmapScale->SetUserDlgProc(new ScaleUserDlgProc(this));
	pmapRot->SetUserDlgProc(new RotateUserDlgProc(this));

	// Verify that at least one node is selected...if not the 'Update Node TM'
	// button will not be enabled.
	SelectionSetChanged(ip, iu);
}
	
// This method is called when we are done using the Utility plug-in
void Utility::EndEditParams(Interface *ip, IUtil *iu) {
	// Delete the parameter maps
	if (pmapMain) { DestroyCPParamMap(pmapMain); pmapMain = NULL; }
	if (pmapTrans) { DestroyCPParamMap(pmapTrans); pmapTrans = NULL; }
	if (pmapScale) { DestroyCPParamMap(pmapScale); pmapScale = NULL; }
	if (pmapRot) { DestroyCPParamMap(pmapRot); pmapRot = NULL; }

	// Delete the rollup pages
	ip->DeleteRollupPage(hViewTM);

	// Set all our pointers to null since they are no longer valid
	// outside the BeginEditParams/EndEditParams interval
	this->iu = NULL;
	this->ip = NULL;
}

// Verify that at least one node is still selected...
void Utility::SelectionSetChanged(Interface *ip, IUtil *iu) {
	BOOL onOff;
	if (onOff = !(ip->GetSelNodeCount() < 1)) {
		// The user is behaving, enable the update node button
		node = ip->GetSelNode(0);
		EnableWindow(GetDlgItem(pmapMain->GetHWnd(), IDC_NODE_TM), 1);
	}
	else {
		// Not even one node ... Disable the button to update the node TM.
		node = NULL;
	}
	EnableWindow(GetDlgItem(pmapMain->GetHWnd(), IDC_NODE_TM), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_TM), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_NODETM), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_OBJECTTM), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_OBJ_OFFSET), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_OBJTMBEFORE), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_OBJTMAFTER), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_PARENTTM), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_TARGETTM), onOff);
	EnableWindow(GetDlgItem(hViewTM, IDC_OBJECTSTATE), onOff);
}

// From IParamArray
// These methods allow the parameter map to get and set the variables
// in our class.  Based on the index passed, retrieve or set the 
// corresponding variable.
BOOL Utility::SetValue(int i, TimeValue t, int v) {
	switch (i) {
		case PM_DO_TRANS: do_trans = v; break;
		case PM_DO_SCALE: do_scale = v; break;
		case PM_DO_X_ROTATE: do_x_rotate = v; break;
		case PM_DO_Y_ROTATE: do_y_rotate = v; break;
		case PM_DO_Z_ROTATE: do_z_rotate = v; break;
		case PM_DO_AUTO: do_auto = v; break;
		case PM_AXIS_CHOICE: axis_choice = v; break;
	}
	return TRUE;
}

BOOL Utility::GetValue(int i, TimeValue t, int &v, Interval &ivalid) {
	switch (i) {
		case PM_DO_TRANS: v = do_trans; break;
		case PM_DO_SCALE: v = do_scale; break;
		case PM_DO_X_ROTATE: v = do_x_rotate; break;
		case PM_DO_Y_ROTATE: v = do_y_rotate; break;
		case PM_DO_Z_ROTATE: v = do_z_rotate; break;
		case PM_DO_AUTO: v = do_auto; break;
		case PM_AXIS_CHOICE: v = axis_choice; break;
	}
	return TRUE;
}

BOOL Utility::SetValue(int i, TimeValue t, float v) {
	switch (i) {
		case PM_X_TRANS: x_trans = v; break;
		case PM_Y_TRANS: y_trans = v; break;
		case PM_Z_TRANS: z_trans = v; break;
		case PM_X_SCALE: x_scale = v; break;
		case PM_Y_SCALE: y_scale = v; break;
		case PM_Z_SCALE: z_scale = v; break;
	}
	return TRUE;
}

BOOL Utility::GetValue(int i, TimeValue t, float &v, Interval &ivalid) {
	switch (i) {
		case PM_X_TRANS: v = x_trans; break;
		case PM_Y_TRANS: v = y_trans; break;
		case PM_Z_TRANS: v = z_trans; break;
		case PM_X_SCALE: v = x_scale; break;
		case PM_Y_SCALE: v = y_scale; break;
		case PM_Z_SCALE: v = z_scale; break;
	}
	return TRUE;
}

/*===========================================================================*\
 | The Guts...Local methods of Utility
\*===========================================================================*/
// Constructor
Utility::Utility() {
	iu = NULL;
	ip = NULL;	

	ResetDefaultValues();
}

void Utility::Init(HWND hWnd) {
	rotEdit = GetICustEdit(GetDlgItem(pmapRot->GetHWnd(), IDC_SEDIT_ROT));
 	rotSpin = GetISpinner(GetDlgItem(pmapRot->GetHWnd(), IDC_SPIN_ROT));
	rotSpin->SetLimits(-360.0f, 360.0f, FALSE);
	rotSpin->SetScale(1.0f);
	rotSpin->SetResetValue(0.0f); // Right click reset value
	rotSpin->SetValue(0.0f, FALSE); // FALSE = don't send notify yet.
	rotSpin->LinkToEdit(GetDlgItem(pmapRot->GetHWnd(), IDC_SEDIT_ROT), 
		EDITTYPE_FLOAT);
}

void Utility::Destroy(HWND hWnd) {
	ReleaseISpinner(rotSpin);
	ReleaseICustEdit(rotEdit);
}

// Make sure all the user interface controls get redrawn
void Utility::RedrawRollups() {
	pmapMain->Invalidate();
	pmapScale->Invalidate();
	pmapTrans->Invalidate();
	pmapRot->Invalidate();
	UpdateSinCosDisplay();
}

// This updates the sine and cosine values displayed in the rotation
// rollup pages to reflect their current values.
void Utility::UpdateSinCosDisplay() {
	TCHAR s[32];
	switch (axis_choice) {
		case 0: // X
			_stprintf(s, _T("c=%.3f"), cos(DegToRad(x_rotate)));
			SetWindowText(GetDlgItem(pmapRot->GetHWnd(), IDC_C),s);
			_stprintf(s, _T("s=%.3f"), sin(DegToRad(x_rotate)));
			SetWindowText(GetDlgItem(pmapRot->GetHWnd(), IDC_S),s);
		break;
		case 1: // Y
			_stprintf(s, _T("c=%.3f"), cos(DegToRad(y_rotate)));
			SetWindowText(GetDlgItem(pmapRot->GetHWnd(), IDC_C),s);
			_stprintf(s, _T("s=%.3f"), sin(DegToRad(y_rotate)));
			SetWindowText(GetDlgItem(pmapRot->GetHWnd(), IDC_S),s);
		break;
		case 2: // Z
			_stprintf(s, _T("c=%.3f"), cos(DegToRad(z_rotate)));
			SetWindowText(GetDlgItem(pmapRot->GetHWnd(), IDC_C),s);
			_stprintf(s, _T("s=%.3f"), sin(DegToRad(z_rotate)));
			SetWindowText(GetDlgItem(pmapRot->GetHWnd(), IDC_S),s);
		break;
	};
}

// This method loads up some variables in the class in preparation
// for displaying them in the view matrix dialog box.
void Utility::LoadViewMatVars(Matrix3 mat) {
	MRow* m = mat.GetAddr();
	m00 = m[0][0];
	m01 = m[0][1];
	m02 = m[0][2];
	m10 = m[1][0];
	m11 = m[1][1];
	m12 = m[1][2];
	m20 = m[2][0];
	m21 = m[2][1];
	m22 = m[2][2];
	m30 = m[3][0];
	m31 = m[3][1];
	m32 = m[3][2];
}

void Utility::ResetDefaultValues() {
	// Init values used by the parameter maps...
	do_trans = do_scale = do_x_rotate = do_y_rotate = do_z_rotate = 1;
	do_auto = 0;
	axis_choice = 0; // X
	x_trans = y_trans = z_trans = 0.0f;
	x_scale = y_scale = z_scale = 1.0f;
	x_rotate = y_rotate = z_rotate = 0.0f;
	_tcscpy(viewTitle, _T("4x3 Transformation Matrix Values"));
}

void Utility::ViewTrans() {
	Point3 p(x_trans, y_trans, z_trans);
	Matrix3 tmat = TransMatrix(p);
	_tcscpy(viewTitle, _T("Translation Matrix Values"));
	LoadViewMatVars(tmat);
}

void Utility::ViewScale() {
	Point3 p(x_scale, y_scale, z_scale);
	Matrix3 tmat = ScaleMatrix(p);
	/*
	MRow* m = tmat.GetAddr();
	m[0][0] = x_scale;
	m[1][1] = y_scale;
	m[2][2] = z_scale;
	*/
	_tcscpy(viewTitle, _T("Scale Matrix Values"));
	LoadViewMatVars(tmat);
}

void Utility::ViewXRotate() {
	Matrix3 tmat = RotateXMatrix(DegToRad(x_rotate));
	_tcscpy(viewTitle, _T("X Rotation Matrix Values"));
	LoadViewMatVars(tmat);
}

void Utility::ViewYRotate() {
	Matrix3 tmat = RotateYMatrix(DegToRad(y_rotate));
	_tcscpy(viewTitle, _T("Y Rotation Matrix Values"));
	LoadViewMatVars(tmat);
}

void Utility::ViewZRotate() {
	Matrix3 tmat = RotateZMatrix(DegToRad(z_rotate));
	_tcscpy(viewTitle, _T("Z Rotation Matrix Values"));
	LoadViewMatVars(tmat);
}

void Utility::ViewComposite() {
	Matrix3 tmat(1); // Identity matrix
	UpdateTM(tmat);
	_tcscpy(viewTitle, _T("4x3 Transformation Matrix Values"));
	LoadViewMatVars(tmat);
}

// This method modifies the matrix passed based on the user's settings
void Utility::UpdateTM(Matrix3 &tmat) {
	Point3 p;
	// If there is a selected node start with its world space transformation
	if (node) {
		tmat = node->GetNodeTM(ip->GetTime());
	}
	// Based on the check boxes update the Matrix3
	if (do_x_rotate) {
		tmat.NoRot();
		Point3 p1;
		p1 = tmat.GetTrans();
		Point3 p2(-p1.x, -p1.y, -p1.z);
		tmat.SetTrans(p2);
		tmat.RotateX(DegToRad(x_rotate));
		tmat.SetTrans(p1);
		tmat.SetIdentFlags(tmat.GetIdentFlags() & ~ROT_IDENT);
	}
	if (do_y_rotate) {
		if (! do_x_rotate) tmat.NoRot();
		Point3 p1;
		p1 = tmat.GetTrans();
		Point3 p2(-p1.x, -p1.y, -p1.z);
		tmat.SetTrans(p2);
		tmat.RotateY(DegToRad(y_rotate));
		tmat.SetTrans(p1);
		tmat.SetIdentFlags(tmat.GetIdentFlags() & ~ROT_IDENT);
	}
	if (do_z_rotate) {
		if (! (do_x_rotate || do_y_rotate)) tmat.NoRot();
		Point3 p1;
		p1 = tmat.GetTrans();
		Point3 p2(-p1.x, -p1.y, -p1.z);
		tmat.SetTrans(p2);
		tmat.RotateZ(DegToRad(z_rotate));
		tmat.SetTrans(p1);
		tmat.SetIdentFlags(tmat.GetIdentFlags() & ~ROT_IDENT);
	}
	if (do_trans) {
		p.x = x_trans;
		p.y = y_trans;
		p.z = z_trans;
		tmat.SetTrans(p);
		tmat.SetIdentFlags(tmat.GetIdentFlags() & ~POS_IDENT);
	}
	if (do_scale) {
		//p.x = x_scale;
		//p.y = y_scale;
		//p.z = z_scale;
		//tmat.SetScale(p);
		MRow* m = tmat.GetAddr();
		m[0][0] = x_scale;
		m[1][1] = y_scale;
		m[2][2] = z_scale;
		tmat.SetIdentFlags(tmat.GetIdentFlags() & ~SCL_IDENT);
	}
}

void Utility::UpdateNodeTM() {
	Matrix3 tmat(1); // Identity matrix

	// Prepare for a potential Undo by putting a Restore Object 
	// onto the undo system stack
	theHold.Begin();
	theHold.Put(new NodeTMRestore(
		this, node, node->GetNodeTM(ip->GetTime()), ip->GetTime()));
	theHold.Accept(UNDO_STRING);

	// Update the NodeTM (make sure we are not in amimate mode)
	UpdateTM(tmat);
	SuspendAnimate();
	AnimateOff();
	node->SetNodeTM(ip->GetTime(), tmat);
	ResumeAnimate();

	ip->RedrawViews(ip->GetTime());
}

void Utility::ViewNodeTM() {
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat(1);
	tmat = n->GetNodeTM(ip->GetTime(), &valid);
	LoadViewMatVars(tmat);
	SetMatrixTitleString(_T("GetNodeTM()"), valid, n);
}

void Utility::ViewObjectTM() {
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat(1);
	tmat = n->GetObjectTM(ip->GetTime(), &valid);
	LoadViewMatVars(tmat);
	SetMatrixTitleString(_T("GetObjectTM()"), valid, n);
}

void Utility::ViewObjectOffset() {
	TSTR buf;
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 

	Point3 p = n->GetObjOffsetPos();
	buf.printf("(%.2f, %.2f, %.2f)", p.x, p.y, p.z);
	MessageBox(NULL, buf.data(), _T("Object Offset Position"), MB_OK);

	ScaleValue s = n->GetObjOffsetScale();
	buf.printf("(%.2f, %.2f, %.2f)", s.s.x, s.s.y, s.s.z);
	MessageBox(NULL, buf.data(), _T("Object Offset Scale"), MB_OK);

	float ang[3];
	Quat q = n->GetObjOffsetRot();
	QuatToEuler(q, ang);
	buf.printf("(%.2f, %.2f, %.2f)", 
		RadToDeg(ang[0]), RadToDeg(ang[1]), RadToDeg(ang[2]));
	MessageBox(NULL, buf.data(), _T("Object Offset Rotation"), MB_OK);
}

void Utility::ViewObjTMBeforeWSM() {
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat(1);
	tmat = n->GetObjTMBeforeWSM(ip->GetTime(), &valid);
	LoadViewMatVars(tmat);
	SetMatrixTitleString(_T("GetObjTMBeforeWSM()"), valid, n);
}

void Utility::ViewObjTMAfterWSM() {
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat(1);
	tmat = n->GetObjTMAfterWSM(ip->GetTime(), &valid);
	LoadViewMatVars(tmat);
	SetMatrixTitleString(_T("GetObjTMAfterWSM()"), valid, n);
}

void Utility::ViewParentTM() {
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat(1);
	tmat = n->GetParentTM(ip->GetTime());
	LoadViewMatVars(tmat);
	SetMatrixTitleString(_T("GetParentTM()"), NEVER, n);
}

void Utility::ViewTargetTM() {
	int haveIt;
	Matrix3 tmat(1);
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 

	haveIt = n->GetTargetTM(ip->GetTime(), tmat);
	if (haveIt) {
		LoadViewMatVars(tmat);
		SetMatrixTitleString(_T("GetTargetTM()"), NEVER, n); // *** never
	}
	else {
		LoadViewMatVars(tmat);
		SetMatrixTitleString(_T("***Node has no Target***"), NEVER, n);
	}
}

void Utility::ViewObjectStateTM() {
	Interval tmValid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat(1);

	ObjectState os = n->EvalWorldState(ip->GetTime());
	Matrix3 *mat = os.GetTM();
	tmValid = os.tmValid();

	if (mat) {
		tmat = *mat;
		SetMatrixTitleString(_T("ObjectState.GetTM()"), tmValid, n);
	}
	else {
		SetMatrixTitleString(_T("GetTM() returned NULL"), tmValid, n);
	}

	LoadViewMatVars(tmat);
}

void Utility::ViewTMController() {
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat(1);
	Control *pos, *rot, *scl;
	Point3 p;
	Quat q;

	ViewLocalRotation();
	ViewWorldRotation();

	// Pre-multiply in position, rotation, scale order
	TimeValue t = ip->GetTime();
	pos = node->GetTMController()->GetPositionController();
	pos->GetValue(t, &tmat, valid, CTRL_RELATIVE);
	rot = node->GetTMController()->GetRotationController();
	rot->GetValue(t, &tmat, valid, CTRL_RELATIVE);
	scl = node->GetTMController()->GetScaleController();
	scl->GetValue(t, &tmat, valid, CTRL_RELATIVE);

	LoadViewMatVars(tmat);
	SetMatrixTitleString(_T("Show TM Controller"), valid, n);
}

void Utility::ViewWorldRotation() {
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat;

	Control *c = node->GetTMController();
	tmat = n->GetParentTM(ip->GetTime());
	c->GetValue(ip->GetTime(), &tmat, valid, CTRL_RELATIVE);

	Quat q(tmat);
	float ang[3];
	QuatToEuler(q, ang);

	TSTR buf;
	buf.printf(_T("XRot=%.1f, YRot=%.1f, ZRot=%.1f"), 
		RadToDeg(ang[0]), RadToDeg(ang[1]), RadToDeg(ang[2]));

	MessageBox(NULL, buf.data(), _T("World Rotation"), MB_OK);
}

void Utility::ViewLocalRotation() {
	Interval valid = FOREVER;
	INode *n = ip->GetSelNode(0); 
	Matrix3 tmat;
	
	Control *c = node->GetTMController();
	tmat.IdentityMatrix();
	c->GetValue(ip->GetTime(), &tmat, valid, CTRL_RELATIVE);

	Quat q(tmat);
	float ang[3];
	QuatToEuler(q, ang);

	TSTR buf;
	buf.printf(_T("XRot=%.1f, YRot=%.1f, ZRot=%.1f"), 
		RadToDeg(ang[0]), RadToDeg(ang[1]), RadToDeg(ang[2]));

	MessageBox(NULL, buf.data(), _T("Local Rotation"), MB_OK);
}

void Utility::SetMatrixTitleString(TCHAR *s, Interval v, INode *n) {
	TSTR start, end;

	// Save symbolic TimeValue names, not integers if we can...
	if (v.Start() == TIME_NegInfinity) {
		start = _T("TIME_NegInfinity");
	}
	else if (v.Start() == TIME_PosInfinity) {
		start = _T("TIME_PosInfinity");
	}
	else  {
		TimeToString(v.Start(), start);
	}

	if (v.End() == TIME_NegInfinity) {
		end = _T("TIME_NegInfinity");
	}
	else if (v.End() == TIME_PosInfinity) {
		end = _T("TIME_PosInfinity");
	}
	else {
		TimeToString(v.End(), end);
	}

	_stprintf(viewTitle, _T("%s->%s: s=%s, e=%s"), n->GetName(), 
		s, start.data(), end.data());
}

/*===========================================================================*\
 | Class Descriptor
\*===========================================================================*/
class UtilityClassDesc : public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theUtility;}
	const TCHAR *	ClassName() {return CLASSNAME;}
	SClass_ID		SuperClassID() {return SClass_ID(UTILITY_CLASS_ID);}
	Class_ID		ClassID() {return MYUTIL_CLASS_ID;}
	const TCHAR* 	Category() {return CATEGORY;}
};
// A single static instance of our class descriptor.
static UtilityClassDesc utilityDesc;

/*===========================================================================*\
 | DLL/Lib Functions
\*===========================================================================*/
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {	
	hInstance = hinstDLL;
	if (! controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	return(TRUE);
}

__declspec(dllexport) const TCHAR *LibDescription() {
	return LIBDESCRIPTION;
}

__declspec(dllexport) int LibNumberClasses() { 
	return 1; 
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i) { 
	return &utilityDesc; 
}

__declspec(dllexport) ULONG LibVersion() { 
	return VERSION_3DSMAX; 
}
