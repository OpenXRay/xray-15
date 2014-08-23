/**********************************************************************
 *<
    FILE: mr_blue.cpp

    DESCRIPTION:  A Mr. Blue helper implementation
 
    CREATED BY: Charles Thaeler
 
        BASED ON: tapehelp.cpp
 
    HISTORY: created 13 Feb. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/


#include "max.h"

#include "resource.h"
TCHAR *GetString(int id);
#include "target.h"

#include "mr_blue.h"
#include "bookmark.h"

 
//------------------------------------------------------

class MrBlueClassDesc:public ClassDesc {
public:
    int 			IsPublic() { return 1; }
    void *			Create(BOOL loading = FALSE) { return new MrBlueObject; }
    int 			BeginCreate(Interface *i);
    int 			EndCreate(Interface *i);
    const TCHAR *	ClassName() { return _T("VRML/VRBL"); }
    SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
    Class_ID 		ClassID() { return Class_ID(MR_BLUE_CLASS_ID1,
                                                    MR_BLUE_CLASS_ID2); }
    const TCHAR* 	Category() { return _T("VRML 1.0/VRBL");  }
    void			ResetClassParams(BOOL fileReset);
};

static MrBlueClassDesc mrBlueDesc;

ClassDesc* GetMrBlueDesc() { return &mrBlueDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for mrBlue class.
Mesh MrBlueObject::mesh;
short MrBlueObject::meshBuilt=0;
HWND MrBlueObject::hMrBlueActions = NULL;
HWND MrBlueObject::hMrBlueTriggers = NULL;
HWND MrBlueObject::hMrBlueParamsHL = NULL;
HWND MrBlueObject::hMrBlueParamsVPT = NULL;
HWND MrBlueObject::hMrBlueParamsAnim = NULL;
IObjParam *MrBlueObject::iObjParams;



// Topper no longer supports MrBlueMessage gdf 10-22-96
// VRBL_Action MrBlueObject::dlgAction = MrBlueMessage;
VRBL_Action MrBlueObject::dlgAction = HyperLinkJump;
BOOL MrBlueObject::dlgMouseEnabled = FALSE;
BOOL MrBlueObject::dlgProxDistEnabled = FALSE;
BOOL MrBlueObject::dlgBBoxEnabled = FALSE;
BOOL MrBlueObject::dlgLosEnabled = FALSE;
float MrBlueObject::dlgProxDist = 25.0f;
float MrBlueObject::dlgBBoxX = 25.0f;
float MrBlueObject::dlgBBoxY = 25.0f;
float MrBlueObject::dlgBBoxZ = 25.0f;
VRBL_LOS_Type MrBlueObject::dlgLosType = CanSee;
float MrBlueObject::dlgLosVptAngle = 30.0f;
float MrBlueObject::dlgLosObjAngle = 60.0f;
int MrBlueObject::dlgAnimPrevSel = -1;
AnimToggle MrBlueObject::dlgAnimState = Start;
ISpinnerControl *MrBlueObject::proxDistSpin = NULL;
ISpinnerControl *MrBlueObject::bboxXSpin = NULL;
ISpinnerControl *MrBlueObject::bboxYSpin = NULL;
ISpinnerControl *MrBlueObject::bboxZSpin = NULL;
ISpinnerControl *MrBlueObject::losVptAngSpin = NULL;
ISpinnerControl *MrBlueObject::losObjAngSpin = NULL;
ICustButton *MrBlueObject::animPickButton = NULL;
 
void MrBlueClassDesc::ResetClassParams(BOOL fileReset)
{
	// Topper no longer supports MrBlueMessage gdf 10-22-96
	// MrBlueObject::dlgAction = MrBlueMessage;
	MrBlueObject::dlgAction = HyperLinkJump;
    MrBlueObject::dlgMouseEnabled = FALSE;
    MrBlueObject::dlgProxDistEnabled = FALSE;
    MrBlueObject::dlgBBoxEnabled = FALSE;
    MrBlueObject::dlgLosEnabled = FALSE;
    MrBlueObject::dlgProxDist = 25.0f;
    MrBlueObject::dlgBBoxX = 25.0f;
    MrBlueObject::dlgBBoxY = 25.0f;
    MrBlueObject::dlgBBoxZ = 25.0f;
    MrBlueObject::dlgLosType = CanSee;
    MrBlueObject::dlgLosVptAngle = 30.0f;
    MrBlueObject::dlgLosObjAngle = 60.0f;
    MrBlueObject::dlgAnimPrevSel = -1;
    MrBlueObject::dlgAnimState = Start;
}

INT_PTR CALLBACK
MrBlueActionDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    MrBlueObject *th = (MrBlueObject *)GetWindowLongPtr( hDlg, GWLP_USERDATA );	

    if ( !th && message != WM_INITDIALOG ) return FALSE;

    switch ( message ) {
    case WM_INITDIALOG:
        th = (MrBlueObject *)lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG)th );
        SetDlgFont( hDlg, th->iObjParams->GetAppHFont() );

        switch(th->dlgAction) {
	    case MrBlueMessage:
            // Topper no longer supports MrBlueMessage gdf 10-22-96
			// CheckDlgButton( hDlg, IDC_MRBLUE_MRBLUE, TRUE );
			CheckDlgButton( hDlg, IDC_MRBLUE_HL, TRUE );
            break;
		case HyperLinkJump:
            CheckDlgButton( hDlg, IDC_MRBLUE_HL, TRUE );
            break;
        case SetViewpoint:
            CheckDlgButton( hDlg, IDC_MRBLUE_CAMERA, TRUE );
            break;
        case Animate:
            CheckDlgButton( hDlg, IDC_MRBLUE_ANIMATE, TRUE );
            break;
        }
        return TRUE;			

    case WM_DESTROY:
        return FALSE;

    case WM_MOUSEACTIVATE:
        th->iObjParams->RealizeParamPanel();
        return FALSE;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        th->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
        return FALSE;

    case WM_COMMAND:			
        switch( LOWORD(wParam) ) {
        case IDC_MRBLUE_MRBLUE:
        case IDC_MRBLUE_HL:
        case IDC_MRBLUE_CAMERA:
        case IDC_MRBLUE_ANIMATE:
            th->SetAction(th->GetDlgAction());
            th->SetRollUps(th->iObjParams);
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;
        }
        return FALSE;

    default:
        return FALSE;
    }
}

INT_PTR CALLBACK
MrBlueTriggerDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    MrBlueObject *th = (MrBlueObject *)GetWindowLongPtr( hDlg, GWLP_USERDATA );	
    if ( !th && message != WM_INITDIALOG ) return FALSE;

    switch ( message ) {
    case WM_INITDIALOG:
        th = (MrBlueObject *)lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG)th );
        SetDlgFont( hDlg, th->iObjParams->GetAppHFont() );
		
					
        CheckDlgButton( hDlg, IDC_MRBLUE_MOUSE, th->dlgMouseEnabled );
        EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_MOUSE), TRUE);
		


        CheckDlgButton( hDlg, IDC_MRBLUE_PROXIMITY, th->dlgProxDistEnabled );
        EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_PROXIMITY), TRUE);
		
        th->proxDistSpin = GetISpinner(GetDlgItem(hDlg,IDC_PROX_DIST_SPINNER));
        th->proxDistSpin->SetLimits( MIN_PROX_DIST, MAX_PROX_DIST, FALSE );
        th->proxDistSpin->SetScale( 0.5f );
        th->proxDistSpin->SetValue( th->GetProxDist(), FALSE );
        th->proxDistSpin->LinkToEdit( GetDlgItem(hDlg,IDC_PROX_DIST), EDITTYPE_POS_FLOAT );
        EnableWindow(GetDlgItem(hDlg, IDC_PROX_DIST), th->dlgProxDistEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_PROX_DIST_SPINNER), th->dlgProxDistEnabled);

        CheckDlgButton( hDlg, IDC_MRBLUE_BBOX, th->dlgBBoxEnabled );
        EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_BBOX), TRUE);
		
        th->bboxXSpin = GetISpinner(GetDlgItem(hDlg,IDC_BBOX_X_SPINNER));
        th->bboxXSpin->SetLimits( MIN_PROX_DIST, MAX_PROX_DIST, FALSE );
        th->bboxXSpin->SetScale( 0.5f );
        th->bboxXSpin->SetValue( th->dlgBBoxX, FALSE );
        th->bboxXSpin->LinkToEdit( GetDlgItem(hDlg,IDC_BBOX_X), EDITTYPE_POS_FLOAT );
        EnableWindow(GetDlgItem(hDlg, IDC_BBOX_X), th->dlgBBoxEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_BBOX_X_SPINNER), th->dlgBBoxEnabled);

        th->bboxYSpin = GetISpinner(GetDlgItem(hDlg,IDC_BBOX_Y_SPINNER));
        th->bboxYSpin->SetLimits( MIN_PROX_DIST, MAX_PROX_DIST, FALSE );
        th->bboxYSpin->SetScale( 0.5f );
        th->bboxYSpin->SetValue( th->dlgBBoxY, FALSE );
        th->bboxYSpin->LinkToEdit( GetDlgItem(hDlg,IDC_BBOX_Y), EDITTYPE_POS_FLOAT );
        EnableWindow(GetDlgItem(hDlg, IDC_BBOX_Y), th->dlgBBoxEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_BBOX_Y_SPINNER), th->dlgBBoxEnabled);

        th->bboxZSpin = GetISpinner(GetDlgItem(hDlg,IDC_BBOX_Z_SPINNER));
        th->bboxZSpin->SetLimits( MIN_PROX_DIST, MAX_PROX_DIST, FALSE );
        th->bboxZSpin->SetScale( 0.5f );
        th->bboxZSpin->SetValue( th->dlgBBoxZ, FALSE );
        th->bboxZSpin->LinkToEdit( GetDlgItem(hDlg,IDC_BBOX_Z), EDITTYPE_POS_FLOAT );
        EnableWindow(GetDlgItem(hDlg, IDC_BBOX_Z), th->dlgBBoxEnabled);
        EnableWindow(GetDlgItem(hDlg, IDC_BBOX_Z_SPINNER), th->dlgBBoxEnabled);


        CheckDlgButton( hDlg, IDC_MRBLUE_LOS, th->dlgLosEnabled );
        EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS), TRUE);

        switch (th->dlgLosType) {
        case CanSee:
            CheckDlgButton( hDlg, IDC_MRBLUE_LOS_SEE, TRUE );
            break;
        case CanSeeAndSeen:
            CheckDlgButton( hDlg, IDC_MRBLUE_LOS_SEEN, TRUE );
            break;
        }
        EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS_SEE), th->dlgLosEnabled );
        EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS_SEEN), th->dlgLosEnabled );

        th->losVptAngSpin = GetISpinner(GetDlgItem(hDlg,IDC_LOS_VPT_ANG_SPINNER));
        th->losVptAngSpin->SetLimits( MIN_LOS_VPT_ANG, MAX_LOS_VPT_ANG, FALSE );
        th->losVptAngSpin->SetScale( 0.5f );
        th->losVptAngSpin->SetValue( th->dlgLosVptAngle, FALSE );
        th->losVptAngSpin->LinkToEdit( GetDlgItem(hDlg,IDC_LOS_VPT_ANG), EDITTYPE_POS_FLOAT );
        EnableWindow(GetDlgItem(hDlg, IDC_LOS_VPT_ANG), th->dlgLosEnabled );
        EnableWindow(GetDlgItem(hDlg, IDC_LOS_VPT_ANG_SPINNER), th->dlgLosEnabled );

        th->losObjAngSpin = GetISpinner(GetDlgItem(hDlg,IDC_LOS_OBJ_ANG_SPINNER));
        th->losObjAngSpin->SetLimits( MIN_LOS_OBJ_ANG, MAX_LOS_OBJ_ANG, FALSE );
        th->losObjAngSpin->SetScale( 0.5f );
        th->losObjAngSpin->SetValue( th->dlgLosObjAngle, FALSE );
        th->losObjAngSpin->LinkToEdit( GetDlgItem(hDlg,IDC_LOS_OBJ_ANG), EDITTYPE_POS_FLOAT );
        EnableWindow(GetDlgItem(hDlg, IDC_LOS_OBJ_ANG),
                     th->dlgLosEnabled && (th->dlgLosType == CanSeeAndSeen));
        EnableWindow(GetDlgItem(hDlg, IDC_LOS_OBJ_ANG_SPINNER),
                     th->dlgLosEnabled && (th->dlgLosType == CanSeeAndSeen));

        return TRUE;			

    case WM_DESTROY:
        ReleaseISpinner( th->proxDistSpin );
        ReleaseISpinner( th->bboxXSpin );
        ReleaseISpinner( th->bboxYSpin );
        ReleaseISpinner( th->bboxZSpin );
        ReleaseISpinner( th->losObjAngSpin );
        ReleaseISpinner( th->losVptAngSpin );
        return FALSE;

    case CC_SPINNER_CHANGE:
        switch ( LOWORD(wParam) ) {
        case IDC_PROX_DIST_SPINNER:
            th->SetProxDist( th->proxDistSpin->GetFVal() );
            th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_INTERACTIVE);
            break;
        case IDC_BBOX_X_SPINNER:
            th->SetBBoxX( th->bboxXSpin->GetFVal() );
            th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_INTERACTIVE);
            break;
        case IDC_BBOX_Y_SPINNER:
            th->SetBBoxY( th->bboxYSpin->GetFVal() );
            th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_INTERACTIVE);
            break;
        case IDC_BBOX_Z_SPINNER:
            th->SetBBoxZ( th->bboxZSpin->GetFVal() );
            th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_INTERACTIVE);
            break;
        case IDC_LOS_VPT_ANG_SPINNER:
            th->SetLosVptAngle( th->losVptAngSpin->GetFVal() );
            th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_INTERACTIVE);
            break;
        case IDC_LOS_OBJ_ANG_SPINNER:
            th->SetLosObjAngle( th->losObjAngSpin->GetFVal() );
            th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_INTERACTIVE);
            break;
        }
        return TRUE;

    case CC_SPINNER_BUTTONUP:
        th->iObjParams->RedrawViews(th->iObjParams->GetTime(),REDRAW_END);
        return TRUE;

    case WM_MOUSEACTIVATE:
        th->iObjParams->RealizeParamPanel();
        return FALSE;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        th->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
        return FALSE;

    case WM_COMMAND:			
        switch( LOWORD(wParam) ) {
        case IDC_MRBLUE_MOUSE:
            th->SetMouseEnabled( IsDlgButtonChecked( hDlg, IDC_MRBLUE_MOUSE ) );
            if (th->dlgAction == HyperLinkJump && th->hMrBlueParamsHL) {
                WPARAM wParam;
                wParam = 666 << 16;   // HACK ALERT!!!!
                wParam ^= IDC_URL_DESC;
                SendMessage(th->hMrBlueParamsHL, WM_COMMAND, wParam, 0);
            }
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;
        case IDC_MRBLUE_PROXIMITY:
            th->SetProxDistEnabled( IsDlgButtonChecked( hDlg, IDC_MRBLUE_PROXIMITY ) );
            EnableWindow(GetDlgItem(hDlg, IDC_PROX_DIST), th->dlgProxDistEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_PROX_DIST_SPINNER), th->dlgProxDistEnabled);
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;
        case IDC_MRBLUE_BBOX:
            th->SetBBoxEnabled( IsDlgButtonChecked( hDlg, IDC_MRBLUE_BBOX ) );
            EnableWindow(GetDlgItem(hDlg, IDC_BBOX_X), th->dlgBBoxEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_BBOX_X_SPINNER), th->dlgBBoxEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_BBOX_Y), th->dlgBBoxEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_BBOX_Y_SPINNER), th->dlgBBoxEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_BBOX_Z), th->dlgBBoxEnabled);
            EnableWindow(GetDlgItem(hDlg, IDC_BBOX_Z_SPINNER), th->dlgBBoxEnabled);
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;
        case IDC_MRBLUE_LOS:
            th->SetLosEnabled( IsDlgButtonChecked( hDlg, IDC_MRBLUE_LOS ) );
            EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS_SEE), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS_SEEN), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_VPT_ANG), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_VPT_ANG_SPINNER), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_OBJ_ANG),
                         th->dlgLosEnabled && (th->dlgLosType == CanSeeAndSeen));
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_OBJ_ANG_SPINNER),
                         th->dlgLosEnabled && (th->dlgLosType == CanSeeAndSeen));
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;
        case IDC_MRBLUE_LOS_SEE:
            th->SetLosType(CanSee);
            EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS_SEE), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS_SEEN), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_VPT_ANG), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_VPT_ANG_SPINNER), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_OBJ_ANG),
                         th->dlgLosEnabled && (th->dlgLosType == CanSeeAndSeen));
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_OBJ_ANG_SPINNER),
                         th->dlgLosEnabled && (th->dlgLosType == CanSeeAndSeen));
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;
        case IDC_MRBLUE_LOS_SEEN:
            th->SetLosType(CanSeeAndSeen);
            EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS_SEE), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_MRBLUE_LOS_SEEN), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_VPT_ANG), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_VPT_ANG_SPINNER), th->dlgLosEnabled );
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_OBJ_ANG),
                         th->dlgLosEnabled && (th->dlgLosType == CanSeeAndSeen));
            EnableWindow(GetDlgItem(hDlg, IDC_LOS_OBJ_ANG_SPINNER),
                         th->dlgLosEnabled && (th->dlgLosType == CanSeeAndSeen));
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;
        }
        return FALSE;

    default:
        return FALSE;
    }
}

INT_PTR CALLBACK
MrBlueHLParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    MrBlueObject *th = (MrBlueObject *)GetWindowLongPtr( hDlg, GWLP_USERDATA );	
    if ( !th && message != WM_INITDIALOG ) return FALSE;

    switch ( message ) {
    case WM_INITDIALOG:
        th = (MrBlueObject *)lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG)th );
        SetDlgFont( hDlg, th->iObjParams->GetAppHFont() );

        SendMessage(GetDlgItem(hDlg,IDC_MRBLUE_URL), WM_SETTEXT, 0, (LPARAM)th->hlURL.data());
        EnableWindow(GetDlgItem(hDlg,IDC_MRBLUE_URL),TRUE);

        SendMessage(GetDlgItem(hDlg,IDC_URL_CAMERA), WM_SETTEXT, 0, (LPARAM)th->hlCamera.data());
        EnableWindow(GetDlgItem(hDlg,IDC_URL_CAMERA),TRUE);

        SendMessage(GetDlgItem(hDlg,IDC_URL_DESC), WM_SETTEXT, 0, (LPARAM)th->hlDesc.data());
        EnableWindow(GetDlgItem(hDlg,IDC_URL_DESC),th->dlgMouseEnabled);
        return TRUE;			

    case WM_DESTROY:
        return FALSE;

    case WM_MOUSEACTIVATE:
        th->iObjParams->RealizeParamPanel();
        return FALSE;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        th->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
        return FALSE;

    case WM_COMMAND:			
        switch( LOWORD(wParam) ) {
        case IDC_BOOKMARKS:
            // do bookmarks
            if (GetBookmarkURL(th->iObjParams, &th->hlURL, &th->hlCamera, &th->hlDesc)) {
                // get the new URL information;
                SendMessage(GetDlgItem(hDlg,IDC_MRBLUE_URL), WM_SETTEXT, 0,
                            (LPARAM)th->hlURL.data());
                SendMessage(GetDlgItem(hDlg,IDC_URL_CAMERA), WM_SETTEXT, 0,
                            (LPARAM)th->hlCamera.data());
                SendMessage(GetDlgItem(hDlg,IDC_URL_DESC), WM_SETTEXT, 0,
                            (LPARAM)th->hlDesc.data());
            }
            break;
        case IDC_MRBLUE_URL:
            switch(HIWORD(wParam)) {
            case EN_CHANGE: {
                int len = SendDlgItemMessage(hDlg, IDC_MRBLUE_URL, WM_GETTEXTLENGTH, 0, 0);
                TSTR temp;
                temp.Resize(len+1);
                SendDlgItemMessage(hDlg, IDC_MRBLUE_URL, WM_GETTEXT, len+1, (LPARAM)temp.data());
                th->hlURL = temp;
            }
            break;
            case EN_SETFOCUS:
                DisableAccelerators();					
                break;
            case EN_KILLFOCUS:
                EnableAccelerators();
                break;
            }
            break;
        case IDC_URL_CAMERA:
            switch(HIWORD(wParam)) {
            case EN_CHANGE: {
                int len = SendDlgItemMessage(hDlg, IDC_URL_CAMERA, WM_GETTEXTLENGTH, 0, 0);
                TSTR temp;
                temp.Resize(len+1);
                SendDlgItemMessage(hDlg, IDC_URL_CAMERA, WM_GETTEXT, len+1, (LPARAM)temp.data());
                th->hlCamera = temp;
            }
            break;
            case EN_SETFOCUS:
                DisableAccelerators();					
                break;
            case EN_KILLFOCUS:
                EnableAccelerators();
                break;
            }
            break;
        case IDC_URL_DESC:
            switch(HIWORD(wParam)) {
            case EN_CHANGE: {
                int len = SendDlgItemMessage(hDlg, IDC_URL_DESC, WM_GETTEXTLENGTH, 0, 0);
                TSTR temp;
                temp.Resize(len+1);
                SendDlgItemMessage(hDlg, IDC_URL_DESC, WM_GETTEXT, len+1, (LPARAM)temp.data());
                th->hlDesc = temp;
            }
            break;
            case EN_SETFOCUS:
                DisableAccelerators();					
                break;
            case EN_KILLFOCUS:
                EnableAccelerators();
                break;
            case 666:
                EnableWindow(GetDlgItem(hDlg, IDC_URL_DESC), th->dlgMouseEnabled);
                break;
            }
            break;
        }
        return FALSE;

    default:
        return FALSE;
    }
}

static void
GetCameras(INode *inode, Tab<INode*> *list)
{
    const ObjectState& os = inode->EvalWorldState(0);
    Object* ob = os.obj;
    if (ob != NULL) {
        if (ob->SuperClassID() == CAMERA_CLASS_ID) {
            list->Append(1, &inode);
        }
    }
    int count = inode->NumberOfChildren();
    for (int i = 0; i < count; i++)
        GetCameras(inode->GetChildNode( i), list);
}

INT_PTR CALLBACK
MrBlueVPTParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    MrBlueObject *th = (MrBlueObject *)GetWindowLongPtr( hDlg, GWLP_USERDATA );	
    if ( !th && message != WM_INITDIALOG ) return FALSE;

    switch ( message ) {
    case WM_INITDIALOG: {
        th = (MrBlueObject *)lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG)th );
        SetDlgFont( hDlg, th->iObjParams->GetAppHFont() );

        // Time to make a list of all the camera's in the scene
        Tab<INode*> cameras;
        GetCameras(th->iObjParams->GetRootNode(), &cameras);
        int c = cameras.Count();
        for (int i = 0; i < c; i++) {
            // add the name to the list
            TSTR name = cameras[i]->GetName();
            int ind = SendMessage(GetDlgItem(hDlg,IDC_MRBLUE_CAMERAS), CB_ADDSTRING,
                                  0, (LPARAM)name.data());
            SendMessage(GetDlgItem(hDlg,IDC_MRBLUE_CAMERAS), CB_SETITEMDATA,
                        ind, (LPARAM)cameras[i]);

        }
        if (th->GetVptCamera()) {
            TSTR name = th->GetVptCamera()->GetName();
            // try to set the current selecttion to the current camera
            SendMessage(GetDlgItem(hDlg,IDC_MRBLUE_CAMERAS), CB_SELECTSTRING,
                        0, (LPARAM)name.data());
        }
        SendMessage(GetDlgItem(hDlg,IDC_VPT_DESC), WM_SETTEXT, 0, (LPARAM)th->vptDesc.data());
        EnableWindow(GetDlgItem(hDlg,IDC_VPT_DESC),TRUE);
    }
    return TRUE;			

    case WM_DESTROY:
        return FALSE;

    case WM_MOUSEACTIVATE:
        th->iObjParams->RealizeParamPanel();
        return FALSE;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        th->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
        return FALSE;

    case WM_COMMAND:			
        switch( LOWORD(wParam) ) {
        case IDC_MRBLUE_CAMERAS:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                // we need to get the selected camera
                int index = SendMessage(GetDlgItem(hDlg,IDC_MRBLUE_CAMERAS),
					CB_GETCURSEL, 0, 0);
                if (index != CB_ERR) {
                    INode *rtarg = (INode *)SendMessage(GetDlgItem(hDlg,IDC_MRBLUE_CAMERAS),
                                                        CB_GETITEMDATA, (WPARAM)index, 0);
                    th->ReplaceReference(0, rtarg);
                } else th->ReplaceReference(0, NULL);
            }
            break;

        case IDC_VPT_DESC:
            switch(HIWORD(wParam)) {
            case EN_SETFOCUS:
                DisableAccelerators();					
                break;
            case EN_KILLFOCUS:
                EnableAccelerators();
                break;
            case EN_CHANGE:
                int len = SendDlgItemMessage(hDlg, IDC_VPT_DESC, WM_GETTEXTLENGTH, 0, 0);
                TSTR temp;
                temp.Resize(len+1);
                SendDlgItemMessage(hDlg, IDC_VPT_DESC, WM_GETTEXT, len+1, (LPARAM)temp.data());
                th->vptDesc = temp;
            }
            break;
        }
        return FALSE;

    default:
        return FALSE;
    }
}


class AnimObjPick : public PickModeCallback {
    MrBlueObject *obj;
public:		

    BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
    BOOL Pick(IObjParam *ip,ViewExp *vpt);	

    void EnterMode(IObjParam *ip);
    void ExitMode(IObjParam *ip);

    HCURSOR GetHitCursor(IObjParam *ip);
    void SetOBJ(MrBlueObject *o) { obj = o; }
};

static AnimObjPick theAnimPick;

BOOL
AnimObjPick::HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{
    INode *node = ip->PickNode(hWnd, m);
    if (node == NULL)
        return FALSE;
    // Don't allow any of our helpers
    Object* obj = node->EvalWorldState(0).obj;
    if (obj->SuperClassID() == HELPER_CLASS_ID &&
        obj->ClassID() == Class_ID(MR_BLUE_CLASS_ID1, MR_BLUE_CLASS_ID2))
        return FALSE;
    return TRUE;
}

void
AnimObjPick::EnterMode(IObjParam *ip)
{
    ip->PushPrompt(GetString(IDS_ANIM_PICK_MODE));
    SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
}

void
AnimObjPick::ExitMode(IObjParam *ip)
{
    ip->PopPrompt();
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}


BOOL
AnimObjPick::Pick(IObjParam *ip,ViewExp *vpt)
{
    if (vpt->HitCount() == 0)
        return FALSE;

    INode *node;
    if ((node = vpt->GetClosestHit()) != NULL) {
        // Check to see if we have a reference to this object already
        for (int i = 0; i < obj->animObjects.Count(); i++) {
            if (obj->animObjects[i]->node == node)
                return FALSE; // Can't click those we already have
        }

        MrBlueAnimObj *aobj = new MrBlueAnimObj(node);
        int id = obj->animObjects.Append(1, &aobj);

        RefResult ret = obj->MakeRefByID(FOREVER, id + 1, node);

        HWND hw = obj->hMrBlueParamsAnim;
        int ind = SendMessage(GetDlgItem(hw,IDC_ANIM_LIST), LB_ADDSTRING, 0, (LPARAM)aobj->listStr.data());
        SendMessage(GetDlgItem(hw,IDC_ANIM_LIST), LB_SETITEMDATA, (WPARAM)ind, (LPARAM)aobj);
        EnableWindow(GetDlgItem(hw, IDC_ANIM_DEL), (obj->animObjects.Count() > 0));
    }
    return FALSE;
}

HCURSOR
AnimObjPick::GetHitCursor(IObjParam *ip)
{
    return LoadCursor(hInstance, MAKEINTRESOURCE(IDC_ANIM_CURSOR));
}


INT_PTR CALLBACK
MrBlueAnimParamDialogProc( HWND hDlg, UINT message, WPARAM wParam,
                           LPARAM lParam )
{
    MrBlueObject *th = (MrBlueObject *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
    if ( !th && message != WM_INITDIALOG ) return FALSE;

    switch ( message ) {
    case WM_INITDIALOG: {
        th = (MrBlueObject *)lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)th );
        SetDlgFont( hDlg, th->iObjParams->GetAppHFont() );

        th->animPickButton = GetICustButton(GetDlgItem(hDlg,IDC_ANIM_PICK));
        th->animPickButton->SetType(CBT_CHECK);
        th->animPickButton->SetButtonDownNotify(TRUE);
        th->animPickButton->SetHighlightColor(GREEN_WASH);

        // Clear the type in box
        SendMessage(GetDlgItem(hDlg,IDC_ANIM_OBJ), WM_SETTEXT, 0,
                    (LPARAM)NULL);
        EnableWindow(GetDlgItem(hDlg,IDC_ANIM_OBJ), TRUE);

        switch(th->dlgAnimState) {
        case Start:
            CheckDlgButton( hDlg, IDC_ANIM_START, TRUE );
            break;
        case Stop:
            CheckDlgButton( hDlg, IDC_ANIM_STOP, TRUE );
            break;
        }
		
        // Now we need to fill in the list box IDC_ANIM_LIST
        int c = th->animObjects.Count();
        int ret = 0;
        for (int i = 0; i < c; i++) {
            MrBlueAnimObj *obj = th->animObjects[i];

            // for now just load the name, we might want to add the frame 
            // range as some point.
            ret = SendMessage(GetDlgItem(hDlg,IDC_ANIM_LIST), LB_ADDSTRING, 0,
                              (LPARAM)obj->listStr.data());
            SendMessage(GetDlgItem(hDlg,IDC_ANIM_LIST), LB_SETITEMDATA,
                        (WPARAM) ret, (LPARAM) obj);
        }
        EnableWindow(GetDlgItem(hDlg, IDC_ANIM_DEL),
                     (th->animObjects.Count() > 0));
        th->dlgAnimPrevSel = -1;
    }
    return TRUE;			

    case WM_DESTROY:
        th->iObjParams->ClearPickMode();
        th->previousMode = NULL;
        ReleaseICustButton( th->animPickButton );
        return FALSE;

    case WM_MOUSEACTIVATE:
        th->iObjParams->RealizeParamPanel();
        return FALSE;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        th->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
        return FALSE;

    case WM_COMMAND:			
        switch( LOWORD(wParam) ) {
        case IDC_ANIM_START:
            th->SetAnimState(Start);
            CheckDlgButton( hDlg, IDC_ANIM_START, TRUE );
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;

        case IDC_ANIM_STOP:
            th->SetAnimState(Stop);
            CheckDlgButton( hDlg, IDC_ANIM_STOP, TRUE );
            th->iObjParams->RedrawViews(th->iObjParams->GetTime());
            break;

        case IDC_ANIM_PICK: // Pick an object from the scene
            // Set the pickmode...
            switch (HIWORD(wParam)) {
            case BN_BUTTONDOWN:
                if (th->previousMode) {
                    // reset the command mode
                    th->iObjParams->SetCommandMode(th->previousMode);
                    th->previousMode = NULL;
                } else {
                    th->previousMode = th->iObjParams->GetCommandMode();
                    theAnimPick.SetOBJ(th);
                    th->iObjParams->SetPickMode(&theAnimPick);
                }
                break;
            }
            break;

        case IDC_ANIM_DEL: { // Delete the object from the list
            int index =	SendMessage(GetDlgItem(hDlg,IDC_ANIM_LIST),
                                    LB_GETCURSEL, 0, 0);
            if (index != LB_ERR) {
                char n[256];
                TSTR name;
                int len = SendDlgItemMessage(hDlg, IDC_ANIM_LIST,
                                             LB_GETTEXTLEN, 0, 0);
                SendDlgItemMessage(hDlg, IDC_ANIM_LIST, LB_GETTEXT,
                                   (WPARAM) index, (LPARAM)n);
                name = n;
                for (int i = 0; i < th->animObjects.Count(); i++) {
                    if (name == th->animObjects[i]->listStr) {
                        // remove the item from the list
                        SendDlgItemMessage(hDlg, IDC_ANIM_LIST,
                                           LB_DELETESTRING, (WPARAM) index,
                                           0);
                        th->dlgAnimPrevSel = -1;
                        // remove the object from the table
                        // Remember ref 0 is the target object!
                        th->DeleteReference(i+1); 
                        th->animObjects.Delete(i, 1);
                        break;
                    }
                }
                EnableWindow(GetDlgItem(hDlg, IDC_ANIM_DEL),
                             (th->animObjects.Count() > 0));
            }
        }
        break;
        case IDC_ANIM_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int sel = SendMessage(GetDlgItem(hDlg,IDC_ANIM_LIST),
                                      LB_GETCURSEL, 0, 0);
                if (th->dlgAnimPrevSel != -1) {
                    // save any editing
                    MrBlueAnimObj *pobj = (MrBlueAnimObj *)
                        SendDlgItemMessage(hDlg, IDC_ANIM_LIST,
                                           LB_GETITEMDATA,
                                           th->dlgAnimPrevSel, 0);
                }
                th->dlgAnimPrevSel = sel;
                MrBlueAnimObj *obj = (MrBlueAnimObj *)
                    SendDlgItemMessage(hDlg, IDC_ANIM_LIST,
                                       LB_GETITEMDATA, sel, 0);
                assert(obj);
                SendDlgItemMessage(hDlg, IDC_ANIM_OBJ, WM_SETTEXT, 0,
                                   (LPARAM)obj->listStr.data());
            }
            break;
        }
        return FALSE;

    default:
        return FALSE;
    }
}

VRBL_Action
MrBlueObject::GetDlgAction()
{
	// Topper no longer supports MrBlueMessage gdf 10-22-96
	/*
    if (IsDlgButtonChecked(hMrBlueActions, IDC_MRBLUE_MRBLUE))
        return MrBlueMessage;
	*/

    if (IsDlgButtonChecked(hMrBlueActions, IDC_MRBLUE_HL))
        return HyperLinkJump;

    if (IsDlgButtonChecked(hMrBlueActions, IDC_MRBLUE_CAMERA))
        return SetViewpoint;

    if (IsDlgButtonChecked(hMrBlueActions, IDC_MRBLUE_ANIMATE))
        return Animate;

	// Topper no longer supports MrBlueMessage gdf 10-22-96
    // return MrBlueMessage;
	return HyperLinkJump;

}

void
MrBlueObject::SetRollUps(IObjParam *ip)
{
    if (hMrBlueParamsHL) {
        ip->UnRegisterDlgWnd(hMrBlueParamsHL);
        ip->DeleteRollupPage(hMrBlueParamsHL);
        hMrBlueParamsHL = NULL;
    }
    if (hMrBlueParamsVPT) {
        ip->UnRegisterDlgWnd(hMrBlueParamsVPT);
        ip->DeleteRollupPage(hMrBlueParamsVPT);
        hMrBlueParamsVPT = NULL;
    }
    if (hMrBlueParamsAnim) {
        if (previousMode) { // if "Pick Objects" active
            // reset the command mode
            iObjParams->SetCommandMode(previousMode);
            previousMode = NULL;
        }
        ip->UnRegisterDlgWnd(hMrBlueParamsAnim);
        ip->DeleteRollupPage(hMrBlueParamsAnim);
        hMrBlueParamsAnim = NULL;
    }
    switch (GetAction()) {
		// Topper no longer supports MrBlueMessage gdf 10-22-96
    case MrBlueMessage:
        // break;
    case HyperLinkJump:
        hMrBlueParamsHL = ip->AddRollupPage(
            hInstance,
            MAKEINTRESOURCE(IDD_MRBLUE_PARAMS_HL),
            MrBlueHLParamDialogProc,
            GetString(IDS_HL_TITLE),
            (LPARAM)this );
 
        ip->RegisterDlgWnd(hMrBlueParamsHL);
        break;
    case SetViewpoint:
        hMrBlueParamsVPT = ip->AddRollupPage(
            hInstance,
            MAKEINTRESOURCE(IDD_MRBLUE_PARAMS_VPT),
            MrBlueVPTParamDialogProc,
            GetString(IDS_VPT_TITLE),
            (LPARAM)this );
 
        ip->RegisterDlgWnd(hMrBlueParamsVPT);
        break;
    case Animate:
        hMrBlueParamsAnim = ip->AddRollupPage(
            hInstance,
            MAKEINTRESOURCE(IDD_MRBLUE_ANIMATE),
            MrBlueAnimParamDialogProc,
            GetString(IDS_ANIM_TITLE),
            (LPARAM)this );
 
        ip->RegisterDlgWnd(hMrBlueParamsAnim);
        break;
    }
}

void
MrBlueObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
    iObjParams = ip;
    dlgAction = action;

    dlgMouseEnabled = mouseEnabled;

    dlgProxDistEnabled = proxDistEnabled;
    dlgProxDist = proxDist;

    dlgBBoxEnabled = bboxEnabled;
    dlgBBoxX = bboxX;
    dlgBBoxY = bboxY;
    dlgBBoxZ = bboxZ;

    dlgLosEnabled = losEnabled;
    dlgLosType = losType;
    dlgLosVptAngle = losVptAngle;
    dlgLosObjAngle = losObjAngle;

    animState = dlgAnimState;
		
    if ( !hMrBlueActions ) {
        hMrBlueActions = ip->AddRollupPage(
            hInstance,
            MAKEINTRESOURCE(IDD_MRBLUE_ACTIONS),
            MrBlueActionDialogProc,
            GetString(IDS_ACTION_TITLE),
            (LPARAM)this );
 
        ip->RegisterDlgWnd(hMrBlueActions);
 
        hMrBlueTriggers = ip->AddRollupPage(
            hInstance,
            MAKEINTRESOURCE(IDD_MRBLUE_TRIGGERS),
            MrBlueTriggerDialogProc,
            GetString(IDS_TRIGGERS_TITLE),
            (LPARAM)this );
 
        ip->RegisterDlgWnd(hMrBlueTriggers);

    } else {
        if (hMrBlueActions)
            SetWindowLongPtr( hMrBlueActions, GWLP_USERDATA, (LONG_PTR)this );
        if (hMrBlueTriggers)
            SetWindowLongPtr( hMrBlueTriggers, GWLP_USERDATA, (LONG_PTR)this );
        if (hMrBlueParamsHL)
            SetWindowLongPtr( hMrBlueParamsHL, GWLP_USERDATA, (LONG_PTR)this );
        if (hMrBlueParamsVPT)
            SetWindowLongPtr( hMrBlueParamsVPT, GWLP_USERDATA, (LONG_PTR)this );
        if (hMrBlueParamsAnim)
            SetWindowLongPtr( hMrBlueParamsAnim, GWLP_USERDATA, (LONG_PTR)this );

    }

    SetRollUps(ip);

}
		
void
MrBlueObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
{
	// Topper no longer supports MrBlueMessage gdf 10-22-96
	/*
    if (IsDlgButtonChecked( hMrBlueActions, IDC_MRBLUE_MRBLUE))
        action = dlgAction = MrBlueMessage;
    else
	*/
        if (IsDlgButtonChecked( hMrBlueActions, IDC_MRBLUE_HL))
            action = dlgAction = HyperLinkJump;
        else
            if (IsDlgButtonChecked( hMrBlueActions, IDC_MRBLUE_CAMERA))
                action = dlgAction = SetViewpoint;
            else
                if (IsDlgButtonChecked( hMrBlueActions, IDC_MRBLUE_ANIMATE))
                    action = dlgAction = Animate;

    mouseEnabled = dlgMouseEnabled = IsDlgButtonChecked(hMrBlueTriggers, IDC_MRBLUE_MOUSE);
    proxDistEnabled = dlgProxDistEnabled = IsDlgButtonChecked(hMrBlueTriggers, IDC_MRBLUE_PROXIMITY);
    proxDist = dlgProxDist = proxDistSpin->GetFVal();

    bboxEnabled = dlgBBoxEnabled = IsDlgButtonChecked(hMrBlueTriggers, IDC_MRBLUE_BBOX);
    bboxX = dlgBBoxX = bboxXSpin->GetFVal();
    bboxY = dlgBBoxY = bboxYSpin->GetFVal();
    bboxZ = dlgBBoxZ = bboxZSpin->GetFVal();
		
    losEnabled = dlgLosEnabled = IsDlgButtonChecked(hMrBlueTriggers, IDC_MRBLUE_LOS);
    if (IsDlgButtonChecked( hMrBlueTriggers, IDC_MRBLUE_LOS_SEE))
        losType = dlgLosType = CanSee;
    else
        if (IsDlgButtonChecked( hMrBlueTriggers, IDC_MRBLUE_LOS_SEEN))
            losType = dlgLosType = CanSeeAndSeen;
    losVptAngle = dlgLosVptAngle = losVptAngSpin->GetFVal();
    losObjAngle = dlgLosObjAngle = losObjAngSpin->GetFVal();

    if (IsDlgButtonChecked( hMrBlueParamsAnim, IDC_ANIM_START))
        animState = dlgAnimState = Start;
    else
        if (IsDlgButtonChecked( hMrBlueParamsAnim, IDC_ANIM_STOP))
            animState = dlgAnimState = Stop;

    if ( flags&END_EDIT_REMOVEUI ) {		
        if (hMrBlueActions) {
            ip->UnRegisterDlgWnd(hMrBlueActions);
            ip->DeleteRollupPage(hMrBlueActions);
            hMrBlueActions = NULL;
        }
        if (hMrBlueTriggers) {
            ip->UnRegisterDlgWnd(hMrBlueTriggers);
            ip->DeleteRollupPage(hMrBlueTriggers);
            hMrBlueTriggers = NULL;
        }
        if (hMrBlueParamsHL) {
            ip->UnRegisterDlgWnd(hMrBlueParamsHL);
            ip->DeleteRollupPage(hMrBlueParamsHL);
            hMrBlueParamsHL = NULL;
        }
        if (hMrBlueParamsVPT) {
            ip->UnRegisterDlgWnd(hMrBlueParamsVPT);
            ip->DeleteRollupPage(hMrBlueParamsVPT);
            hMrBlueParamsVPT = NULL;
        }
        if (hMrBlueParamsAnim) {
            ip->UnRegisterDlgWnd(hMrBlueParamsAnim);
            ip->DeleteRollupPage(hMrBlueParamsAnim);
            hMrBlueParamsAnim = NULL;
        }
    } else {
        if (hMrBlueActions)
            SetWindowLongPtr( hMrBlueActions, GWLP_USERDATA, 0 );
        if (hMrBlueTriggers)
            SetWindowLongPtr( hMrBlueTriggers, GWLP_USERDATA, 0 );
        if (hMrBlueParamsHL)
            SetWindowLongPtr( hMrBlueParamsHL, GWLP_USERDATA, 0 );
        if (hMrBlueParamsVPT)
            SetWindowLongPtr( hMrBlueParamsVPT, GWLP_USERDATA, 0 );
        if (hMrBlueParamsAnim)
            SetWindowLongPtr( hMrBlueParamsAnim, GWLP_USERDATA, 0 );
    }
	
    iObjParams = NULL;
}


void MrBlueObject::BuildMesh()
{
    if(meshBuilt)
        return;
    int nverts = 6;
    int nfaces = 8;
    mesh.setNumVerts(nverts);
    mesh.setNumFaces(nfaces);
 
    float radius = 7.0f;
 
    mesh.setVert(0, Point3( radius, radius, 0.0f));
    mesh.setVert(1, Point3( radius, -radius, 0.0f));
    mesh.setVert(2, Point3( -radius, -radius, 0.0f));
    mesh.setVert(3, Point3( -radius, radius, 0.0f));
    mesh.setVert(4, Point3( 0.0f, 0.0f, radius));
    mesh.setVert(5, Point3( 0.0f, 0.0f, -radius));

    mesh.faces[0].setVerts(1, 0, 4);
    mesh.faces[0].setEdgeVisFlags(1, 1, 0);
    mesh.faces[0].setSmGroup(0);

    mesh.faces[1].setVerts(2, 1, 4);
    mesh.faces[1].setEdgeVisFlags(1, 1, 0);
    mesh.faces[1].setSmGroup(0);

    mesh.faces[2].setVerts(3, 2, 4);
    mesh.faces[2].setEdgeVisFlags(1, 1, 0);
    mesh.faces[2].setSmGroup(0);

    mesh.faces[3].setVerts(0, 3, 4);
    mesh.faces[3].setEdgeVisFlags(1, 1, 0);
    mesh.faces[3].setSmGroup(0);

    mesh.faces[4].setVerts(0, 1, 5);
    mesh.faces[4].setEdgeVisFlags(0, 1, 0);
    mesh.faces[4].setSmGroup(0);

    mesh.faces[5].setVerts(1, 2, 5);
    mesh.faces[5].setEdgeVisFlags(0, 1, 0);
    mesh.faces[5].setSmGroup(0);

    mesh.faces[6].setVerts(2, 3, 5);
    mesh.faces[6].setEdgeVisFlags(0, 1, 0);
    mesh.faces[6].setSmGroup(0);

    mesh.faces[7].setVerts(3, 0, 5);
    mesh.faces[7].setEdgeVisFlags(0, 1, 0);
    mesh.faces[7].setSmGroup(0);

    mesh.buildNormals();
    mesh.EnableEdgeList(1);
    meshBuilt = 1;
}

void
MrBlueObject::UpdateUI(TimeValue t)
{
    if ( hMrBlueTriggers &&
         GetWindowLongPtr(hMrBlueTriggers,GWLP_USERDATA)==(LONG_PTR)this ) {
        proxDistSpin->SetValue( GetProxDist(), FALSE );
        bboxXSpin->SetValue( GetBBoxX(), FALSE );
        bboxYSpin->SetValue( GetBBoxY(), FALSE );
        bboxZSpin->SetValue( GetBBoxZ(), FALSE );
        losVptAngSpin->SetValue( GetLosVptAngle(), FALSE );
        losObjAngSpin->SetValue( GetLosObjAngle(), FALSE );

        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_PROX_DIST), GetProxDistEnabled());
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_PROX_DIST_SPINNER), GetProxDistEnabled());

        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_BBOX_X), GetBBoxEnabled());
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_BBOX_X_SPINNER), GetBBoxEnabled());
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_BBOX_Y), GetBBoxEnabled());
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_BBOX_Y_SPINNER), GetBBoxEnabled());
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_BBOX_Z), GetBBoxEnabled());
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_BBOX_Z_SPINNER), GetBBoxEnabled());


        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_LOS_DIST), GetLosEnabled() );
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_MRBLUE_LOS_SEE), GetLosEnabled() );
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_MRBLUE_LOS_SEEN), GetLosEnabled() );
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_LOS_DIST_SPINNER), GetLosEnabled() );
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_LOS_VPT_ANG), GetLosEnabled() );
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_LOS_VPT_ANG_SPINNER), GetLosEnabled() );
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_LOS_OBJ_ANG),
                     GetLosEnabled() && (GetLosType() == CanSeeAndSeen));
        EnableWindow(GetDlgItem(hMrBlueTriggers, IDC_LOS_OBJ_ANG_SPINNER),
                     GetLosEnabled() && (GetLosType() == CanSeeAndSeen));
    }
}



MrBlueObject::MrBlueObject() : HelperObject() 
{
    // Initialize the object from the dlg versions
    enable = 0;

    action = dlgAction;

    mouseEnabled = dlgMouseEnabled;

    proxDistEnabled = dlgProxDistEnabled;
    proxDist = dlgProxDist;

    bboxEnabled = dlgBBoxEnabled;
    bboxX = dlgBBoxX;
    bboxY = dlgBBoxY;
    bboxZ = dlgBBoxZ;

    losEnabled = dlgLosEnabled;
    losType = dlgLosType;
    losVptAngle = dlgLosVptAngle;
    losObjAngle = dlgLosObjAngle;

    animState = dlgAnimState;
    previousMode = NULL;

    vptCamera = NULL;

    BuildMesh();	
}

MrBlueObject::~MrBlueObject()
{
    DeleteAllRefsFromMe();
    int c = animObjects.Count();
    for (int i = 0; i < c; i++) {
        MrBlueAnimObj *obj = animObjects[i];
        delete obj;
    }
}




void
MrBlueObject::SetAction( VRBL_Action act )
{
    dlgAction = action = act;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetMouseEnabled( int onOff )
{
    dlgMouseEnabled = mouseEnabled = onOff;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetProxDistEnabled( int onOff )
{
    dlgProxDistEnabled = proxDistEnabled = onOff;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetProxDist( float dist )
{
    dlgProxDist = proxDist = dist;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetBBoxEnabled( int onOff )
{
    dlgBBoxEnabled = bboxEnabled = onOff;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetBBoxX( float dist )
{
    dlgBBoxX = bboxX = dist;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetBBoxY( float dist )
{
    dlgBBoxY = bboxY = dist;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetBBoxZ( float dist )
{
    dlgBBoxZ = bboxZ = dist;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetLosEnabled( int onOff )
{
    dlgLosEnabled = losEnabled = onOff;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetLosType( VRBL_LOS_Type type)
{
    dlgLosType = losType = type;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetLosVptAngle( float ang )
{
    dlgLosVptAngle = losVptAngle = ang;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetLosObjAngle( float ang )
{
    dlgLosObjAngle = losObjAngle = ang;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void
MrBlueObject::SetAnimState( AnimToggle state )
{
    dlgAnimState = animState = state;
    NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

class MrBlueObjCreateCallBack: public CreateMouseCallBack {
    MrBlueObject *ob;
public:
    int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
    void SetObj(MrBlueObject *obj) { ob = obj; }
};

int MrBlueObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
    ob->enable = 1;
    if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
        mat.SetTrans( vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE) );
        if (point==1 && msg==MOUSE_POINT) 
            return 0;
    }
    else
	if (msg == MOUSE_ABORT)
            return CREATE_ABORT;

    return TRUE;
}

static MrBlueObjCreateCallBack mrBlueCreateCB;

CreateMouseCallBack* MrBlueObject::GetCreateMouseCallBack() {
    mrBlueCreateCB.SetObj(this);
    return(&mrBlueCreateCB);
}

static int GetTargetPoint(TimeValue t, INode *inode, Point3& p) {
    Matrix3 tmat;
    if (inode->GetTargetTM(t,tmat)) {
        p = tmat.GetTrans();
        return 1;
    }
    else 
        return 0;
}



void
MrBlueObject::GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm)
{
    tm = inode->GetObjectTM(t);
    tm.NoScale();
    float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
    tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}


float
MrBlueObject::GetTDist(Matrix3 tm, Point3 pt)
{
    if (Length(tm.GetRow(2)) == 0.0)
        return 10.0f;

    return Length(tm.GetTrans() - pt)/Length(tm.GetRow(2));
}


void
MrBlueObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
    box = mesh.getBoundingBox(tm);
}

void
MrBlueObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
    Matrix3 tm = inode->GetObjectTM(t);
    Point3 loc = tm.GetTrans();
    Point3 pt;
    float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(loc) / 360.0f;
    box = mesh.getBoundingBox();
    box.Scale(scaleFactor);

    GetTargetPoint(t,inode,pt);
    float d = GetTDist(tm, pt);
    box += Point3(0.0f, 0.0f, -d);

    if (GetProxDistEnabled()) {
        Point3 x[32], y[32], z[32];
        GetProxDistPoints(GetProxDist(), x, y, z);
        for (int i = 0; i < 32; i++) {
            box += x[i];
            box += y[i];
            box += z[i];
        }
    }
    if (GetLosEnabled()) {
        Point3 v[32];
        GetLosVptConePoints(d, v);
        for (int i = 0; i < 32; i++)
            box += v[i];
        if (GetLosType() == CanSeeAndSeen) {
            GetLosObjConePoints(d, v);
            for (int i = 0; i < 32; i++)
                box += v[i];
        }
    }
    if (GetBBoxEnabled()) {
        Matrix3 itm = tm;
        itm.NoTrans();
        itm = Inverse(itm);
        float x = GetBBoxX(),
            y = GetBBoxY(),
            z = GetBBoxZ();
        box += itm*Point3( x,  y,  z);
        box += itm*Point3(-x,  y,  z);
        box += itm*Point3(-x, -y,  z);
        box += itm*Point3( x, -y,  z);
        box += itm*Point3( x,  y, -z);
        box += itm*Point3(-x,  y, -z);
        box += itm*Point3(-x, -y, -z);
        box += itm*Point3( x, -y, -z);
    }
}

void
MrBlueObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
    int i, nv;
    Matrix3 tm;
    Point3 pt;
    Point3 q[2];
    BuildMesh();        // 000830  --prs.
    GetMat(t,inode,vpt,tm);
    nv = mesh.getNumVerts();
    box.Init();
    for (i=0; i<nv; i++) 
        box += tm*mesh.getVert(i);


    tm = inode->GetObjectTM(t);

    GetTargetPoint(t,inode,pt);
    float d = GetTDist(tm, pt);
    box += tm*Point3(0.0f, 0.0f, -d);

    if (GetProxDistEnabled()) {
        Point3 x[32], y[32], z[32];
        GetProxDistPoints(GetProxDist(), x, y, z);
        for (int i = 0; i < 32; i++) {
            box += tm*x[i];
            box += tm*y[i];
            box += tm*z[i];
        }
    }
    if (GetLosEnabled()) {
        Point3 v[32];
        GetLosVptConePoints(d, v);
        for (int i = 0; i < 32; i++)
            box += tm*v[i];
        if (GetLosType() == CanSeeAndSeen) {
            GetLosObjConePoints(d, v);
            for (int i = 0; i < 32; i++)
                box += tm*v[i];
        }
    }
    if (GetBBoxEnabled()) {
        float x = GetBBoxX(),
            y = GetBBoxY(),
            z = GetBBoxZ();
        Point3 center = tm * Point3(0,0,0);
        box += center + Point3(-x, -y, -z); // since this is world space already...
        box += center + Point3(x, y, z);
    }
}

void
MrBlueObject::GetLinePoints(TimeValue t, Point3* q, float len)
{
    q[0] = Point3( 0.0f, 0.0f, 0.0f);				
    q[1] = Point3( 0.0f, 0.0f, -len);				
}

// From BaseObject
void
MrBlueObject::DrawLine(TimeValue t, INode* inode, GraphicsWindow *gw)
{
    Matrix3 tm = inode->GetObjectTM(t);
    gw->setTransform(tm);

    Point3 pt,v[2];
    GetTargetPoint(t,inode,pt);
    v[0] = Point3(0,0,0);
    v[1] = Point3(0.0f, 0.0f, -GetTDist(tm, pt));
    if(!inode->IsFrozen())
        gw->setColor( LINE_COLOR, 0.0f, 1.0f, 0.0f);
    gw->polyline( 2, v, NULL, NULL, FALSE, NULL );
}


void
MrBlueObject::GetProxDistPoints(float radius, Point3 *x, Point3 *y, Point3 *z)
{
    float dang = PI / ( 2.0f * 8.0f);
    float ang = 0.0f;
    for (int i = 0; i < 32; i++ ) {
        z[i].x = x[i].y = y[i].x = radius * (float) cos(ang);
        z[i].y = x[i].z = y[i].z = radius * (float) sin(ang);
        z[i].z = x[i].x = y[i].y = 0.0f;
        ang += dang;
    }
}

void
MrBlueObject::DrawProxDist(TimeValue t, INode* inode, GraphicsWindow *gw)
{
    Matrix3 tm = inode->GetObjectTM(t);
    gw->setTransform(tm);

    Point3 x[32], y[32], z[32];
    GetProxDistPoints(GetProxDist(), x, y, z);
	
    if(!inode->IsFrozen())
        gw->setColor( LINE_COLOR, 0.0f, 0.0f, 1.0f);
    gw->polyline(32, x, NULL, NULL, TRUE, NULL);
    gw->polyline(32, y, NULL, NULL, TRUE, NULL);
    gw->polyline(32, z, NULL, NULL, TRUE, NULL);
}

void
MrBlueObject::DrawBBox(TimeValue t, INode* inode, GraphicsWindow *gw)
{
    Matrix3 tm = inode->GetObjectTM(t);
    gw->setTransform(tm);

    float x = GetBBoxX()/2.0f,
        y = GetBBoxY()/2.0f,
        z = GetBBoxZ()/2.0f;

    Point3 top[4], bottom[4], side[2], center;

    center = tm * Point3(0,0,0);
    top[0] = center + Point3(x, y, z);
    top[1] = center + Point3(x, -y, z);
    top[2] = center + Point3(-x, -y, z);
    top[3] = center + Point3(-x, y, z);

    for (int i = 0; i < 4; i++) {
        bottom[i].x = top[i].x;
        bottom[i].y = top[i].y;
        bottom[i].z = center.z - z;
    }

    for (i = 0; i < 4; i++) {
        Matrix3 itm = Inverse(tm);
        Point3 temp;
        temp = top[i];
        top[i] = itm * temp;
        temp = bottom[i];
        bottom[i] = itm * temp;
    }

    if(!inode->IsFrozen())
        gw->setColor( LINE_COLOR, 1.0f, 0.0f, 1.0f);
    gw->polyline(4, top, NULL, NULL, TRUE, NULL);
    gw->polyline(4, bottom, NULL, NULL, TRUE, NULL);
    for (i = 0; i < 4; i++) {
        side[0] = top[i];
        side[1] = bottom[i];
        gw->polyline(2, side, NULL, NULL, FALSE, NULL);
    }
}

void
MrBlueObject::GetLosObjConePoints(float dist, Point3 *v)
{
    float radius;
    float length;
    float cone = GetLosObjAngle();
    if (cone < 45 || cone > 135) {
        // Constant length
        length = dist * 0.25f;
        radius = length * (float) tan(DegToRad(cone));
    } else {
        // Constant radius cone end
        radius = dist * 0.25f;
        length = radius / (float) tan(DegToRad(cone));
    }

    float dang = PI / ( 2.0f * 8.0f);
    float ang = 0.0f;

    for (int i = 0; i < 32; i++ ) {
        v[i].x = radius * (float) cos(ang);
        v[i].y = radius * (float) sin(ang);
        v[i].z = (cone > 135) ? length : -length;
        ang += dang;
    }
}

void
MrBlueObject::DrawLosObjCone(TimeValue t, INode* inode, GraphicsWindow *gw)
{
    Matrix3 tm = inode->GetObjectTM(t);
    gw->setTransform(tm);
    Point3 pt;
    GetTargetPoint(t,inode,pt);
    Point3 v[32];
    GetLosObjConePoints(GetTDist(tm, pt), v);

    Point3 leg1[3], leg2[3];
    leg1[1].x = leg1[1].y = leg1[1].z = 0.0f;
    leg1[0] = v[0];
    leg1[2] = v[16];
    leg2[0] = v[8];
    leg2[1].x = leg2[1].y = leg2[1].z = 0.0f;
    leg2[2] = v[24];

    if(!inode->IsFrozen())
        gw->setColor( LINE_COLOR, 1.0f, 0.0f, 0.0f);
    gw->polyline(32, v, NULL, NULL, TRUE, NULL);
    gw->polyline(3, leg1, NULL, NULL, FALSE, NULL);
    gw->polyline(3, leg2, NULL, NULL, FALSE, NULL);
}


void
MrBlueObject::GetLosVptConePoints(float dist, Point3 *v)
{
    float radius,
        length,
        cone = GetLosVptAngle();

    if (cone < 45) {
        // Constant length
        length = dist;
        radius = length * (float) tan(DegToRad(cone));
    } else {
        // Constant radius cone end
        radius = dist;
        length = radius / (float) tan(DegToRad(cone));
    }

    float dang = PI / ( 2.0f * 8.0f),
        ang = 0.0f;
    for (int i = 0; i < 32; i++ ) {
        v[i].x = radius * (float) cos(ang);
        v[i].y = radius * (float) sin(ang);
        v[i].z = (cone < 45) ? 0.0f : -(dist - length);
        ang += dang;
    }
}

void
MrBlueObject::DrawLosVptCone(TimeValue t, INode* inode, GraphicsWindow *gw)
{
    Matrix3 tm = inode->GetObjectTM(t);
    gw->setTransform(tm);
    Point3 pt;
    GetTargetPoint(t,inode,pt);
    float dist = GetTDist(tm, pt);
    Point3 v[32];
    GetLosVptConePoints(dist, v);
	
    Point3 leg1[3], leg2[3];
    leg1[0] = v[0];
    leg1[1].x = leg1[1].y = 0.0f;
    leg1[1].z = -dist;
    leg1[2] = v[16];
    leg2[0] = v[8];
    leg2[1].x = leg2[1].y = 0.0f;
    leg2[1].z = -dist;
    leg2[2] = v[24];

    if(!inode->IsFrozen())
        gw->setColor( LINE_COLOR, 1.0f, 1.0f, 0.0f);

    gw->polyline(32, v, NULL, NULL, TRUE, NULL);
    gw->polyline(3, leg1, NULL, NULL, FALSE, NULL);
    gw->polyline(3, leg2, NULL, NULL, FALSE, NULL);
}

int
MrBlueObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
    // Snap to a parent!
    if (!inode->GetParentNode()->IsRootNode()) {
        Box3 box;
        Matrix3 tm(TRUE);
        Object *obj = inode->GetParentNode()->EvalWorldState(t).obj;
        if (obj) {
            obj->GetDeformBBox(t, box, &inode->GetParentNode()->GetObjectTM(t));
            tm.SetTrans(box.Center());
        } else {
            tm.SetTrans(inode->GetParentNode()->GetObjectTM(t).GetTrans());
        }
        inode->SetNodeTM(t, tm);
    }

    Matrix3 m;
    GraphicsWindow *gw = vpt->getGW();

    GetMat(t,inode,vpt,m);
    gw->setTransform(m);
    DWORD rlim = gw->getRndLimits();

    gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL|(gw->getRndMode() & GW_Z_BUFFER));

    if (inode->Selected()) 
        gw->setColor( LINE_COLOR, 1.0f, 1.0f, 1.0f);
    else if(!inode->IsFrozen())
        gw->setColor( LINE_COLOR, 0.0f, 1.0f, 0.0f);

    mesh.render( gw, gw->getMaterial(),
                 (flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL);

    DrawLine(t,inode,gw);

    if (inode->Selected() || (inode->GetTarget() && inode->GetTarget()->Selected())) {
        if (GetProxDistEnabled())
            DrawProxDist(t,inode,gw);
        if (GetLosEnabled()) {
            DrawLosVptCone(t, inode, gw);
            if (GetLosType() == CanSeeAndSeen)
                DrawLosObjCone(t, inode, gw);
        }
        if (GetBBoxEnabled())
            DrawBBox(t,inode,gw);
    }
    gw->setRndLimits(rlim);
    return(0);
}



int
MrBlueObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
    HitRegion hitRegion;
    DWORD	savedLimits;
    int res;
    Matrix3 m;
    if (!enable) return  0;
    GraphicsWindow *gw = vpt->getGW();	
    MakeHitRegion(hitRegion,type,crossing,4,p);	
    gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
    GetMat(t,inode,vpt,m);
    gw->setTransform(m);
    gw->clearHitCode();
    res = mesh.select( gw, gw->getMaterial(), &hitRegion, flags & HIT_ABORTONHIT); 
    gw->setRndLimits(savedLimits);
    return res;
}

void MrBlueObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
{
    // Make sure the vertex priority is active and at least as important as the best snap so far
    if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
        Matrix3 tm = inode->GetObjectTM(t);	
        GraphicsWindow *gw = vpt->getGW();	
   	
        gw->setTransform(tm);

        Point3 screen3;
        Point2 screen2;
        IPoint2 iscreen2;
        Point3 vert(0.0f,0.0f,0.0f);

        gw->transPoint(&vert,&screen3);

        screen2.x = screen3.x;
        screen2.y = screen3.y;
        iscreen2.x = (int)(screen3.x + 0.5f);
        iscreen2.y = (int)(screen3.y + 0.5f);

        // Are we within the snap radius?
        int len = Length(iscreen2 - *p);
        if(len <= snap->strength) {
            // Is this priority better than the best so far?
            if(snap->vertPriority < snap->priority) {
                snap->priority = snap->vertPriority;
                snap->bestWorld = vert * tm;
                snap->bestScreen = screen2;
                snap->bestDist = len;
            }
            else // Closer than the best of this priority?
                if(len < snap->bestDist) {
                    snap->priority = snap->vertPriority;
                    snap->bestWorld = vert * tm;
                    snap->bestScreen = screen2;
                    snap->bestDist = len;
                }
        }
    }
}

// From GeomObject
int MrBlueObject::IntersectRay(TimeValue t, Ray& r, float& at) { return(0); }

//
// Reference Managment:
//

// This is only called if the object MAKES references to other things.
RefResult
MrBlueObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message ) 
{
    int i;
	
    switch (message) {
    case REFMSG_TARGET_DELETED:
        if (hTarget == vptCamera) {
            vptCamera = NULL;
            if (iObjParams)
                UpdateUI(iObjParams->GetTime());
        } 
        // Find the ID on the list and call ResetStr
        for (i = 0; i < animObjects.Count(); i++) {
            if (animObjects[i]->node == hTarget) {
                // Do I need to remove the reference? FIXME
                animObjects.Delete(i, 1); 
            }
        }
        break;
    case REFMSG_NODE_NAMECHANGE:
        // Find the ID on the list and call ResetStr
        for (i = 0; i < animObjects.Count(); i++) {
            if (animObjects[i]->node == hTarget) {
                // Found it
                animObjects[i]->ResetStr();
                break;
            }
        }
        break;
    }
    return REF_SUCCEED;
}

RefTargetHandle
MrBlueObject::GetReference(int i)
{
    if (i == 0)
        return (RefTargetHandle)vptCamera; // the 0 reference is the camera if it exists

    if (i > animObjects.Count())  
        return NULL;

    return animObjects[i-1]->node;
} 

void
MrBlueObject::SetReference(int i, RefTargetHandle rtarg)
{
    if (i == 0) {
        vptCamera = (INode *)rtarg;
    } else {
        if (i > animObjects.Count()) return;
        animObjects[i-1]->node = (INode *)rtarg;
        animObjects[i-1]->ResetStr();
    }
}

ObjectState MrBlueObject::Eval(TimeValue time)
{
    return ObjectState(this);
}

Interval MrBlueObject::ObjectValidity(TimeValue time)
{
    Interval ivalid;
    ivalid.SetInfinite();
    UpdateUI(time);
    return ivalid;	
}

class MrBlueCreationManager : public MouseCallBack, ReferenceMaker {
private:
    CreateMouseCallBack *createCB;	
    INode *mrBlueNode,*targNode;
    MrBlueObject *mrBlueObject;
    TargetObject *targObject;
    int attachedToNode;
    IObjCreate *createInterface;
    ClassDesc *cDesc;
    Matrix3 mat;  // the nodes TM relative to the CP
    IPoint2 pt0;
    int ignoreSelectionChange;

    void CreateNewObject();	

    int NumRefs() { return 1; }
    RefTargetHandle GetReference(int i) { return (RefTargetHandle)mrBlueNode; } 
    void SetReference(int i, RefTargetHandle rtarg) { mrBlueNode = (INode *)rtarg; }

    // StdNotifyRefChanged calls this, which can change the partID to new value 
    // If it doesnt depend on the particular message& partID, it should return
    // REF_DONTCARE
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
                               PartID& partID,  RefMessage message);

public:
    void Begin( IObjCreate *ioc, ClassDesc *desc );
    void End();
		
    MrBlueCreationManager()
    {
        ignoreSelectionChange = FALSE;
    }
    int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
};


#define CID_MRBLUEOBJCREATE	CID_USER + 6

class MrBlueCreateMode : public CommandMode {
    MrBlueCreationManager proc;
public:
    void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
    void End() { proc.End(); }

    int Class() { return CREATE_COMMAND; }
    int ID() { return CID_MRBLUEOBJCREATE; }
    MouseCallBack *MouseProc(int *numPoints) { *numPoints = 1000000; return &proc; }
    ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
    BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
    void EnterMode() {}
    void ExitMode() {}
    BOOL IsSticky() { return FALSE; }
};

static MrBlueCreateMode theMrBlueCreateMode;

void MrBlueCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc )
{
    createInterface = ioc;
    cDesc           = desc;
    attachedToNode  = FALSE;
    createCB		= NULL;
    mrBlueNode		= NULL;
    targNode		= NULL;
    mrBlueObject	= NULL;
    targObject		= NULL;
    CreateNewObject();
}

void MrBlueCreationManager::End()
{
    if ( mrBlueObject ) {
        mrBlueObject->EndEditParams( (IObjParam*)createInterface, 
                                     END_EDIT_REMOVEUI, NULL);
        if ( !attachedToNode ) {
            delete mrBlueObject;
            mrBlueObject = NULL;
        } else if ( mrBlueNode ) {
            // Get rid of the reference.
            DeleteReference(0);  // sets mrBlueNode = NULL
        }
    }	
}

RefResult MrBlueCreationManager::NotifyRefChanged(
    Interval changeInt, 
    RefTargetHandle hTarget, 
    PartID& partID,  
    RefMessage message) 
{
    switch (message) {
    case REFMSG_TARGET_SELECTIONCHANGE:
        if ( ignoreSelectionChange ) {
            break;
        }
        if ( mrBlueObject && mrBlueNode==hTarget ) {
            // this will set mrBlueNode== NULL;
            DeleteReference(0);
            goto endEdit;
        }
        // fall through

    case REFMSG_TARGET_DELETED:		
        if ( mrBlueObject && mrBlueNode==hTarget ) {
          endEdit:
            mrBlueObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
            mrBlueObject  = NULL;				
            mrBlueNode    = NULL;
            CreateNewObject();	
            attachedToNode = FALSE;
        }
        else if (targNode==hTarget) {
            targNode = NULL;
            targObject = NULL;
        }
        break;		
    }
    return REF_SUCCEED;
}


void MrBlueCreationManager::CreateNewObject()
{
    mrBlueObject = (MrBlueObject*)cDesc->Create();
	
    // Start the edit params process
    if ( mrBlueObject ) {
        mrBlueObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
    }	
}

			
int MrBlueCreationManager::proc( 
    HWND hwnd,
    int msg,
    int point,
    int flag,
    IPoint2 m )
{	
    int res;
    TSTR targName;	
    ViewExp *vpx = createInterface->GetViewport(hwnd); 
    assert( vpx );

    switch ( msg ) {
    case MOUSE_POINT:
        switch ( point ) {
        case 0:
            pt0 = m;
            assert( mrBlueObject );					
            if ( createInterface->SetActiveViewport(hwnd) ) {
                return FALSE;
            }

            if (createInterface->IsCPEdgeOnInView()) { 
                res = FALSE;
                goto done;
            }

            if ( attachedToNode ) {
                // send this one on its way
                mrBlueObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
						
                // Get rid of the reference.
                if (mrBlueNode)
                    DeleteReference(0);

                // new object
                CreateNewObject();   // creates mrBlueObject
            }

            theHold.Begin();	 // begin hold for undo
            mat.IdentityMatrix();

            // link it up
            mrBlueNode = createInterface->CreateObjectNode( mrBlueObject);
            attachedToNode = TRUE;
            assert( mrBlueNode );					
            createCB = mrBlueObject->GetCreateMouseCallBack();					
            createInterface->SelectNode( mrBlueNode );
					
            // Create target object and node
            targObject = (TargetObject *)createInterface->CreateInstance(GEOMOBJECT_CLASS_ID, Class_ID( TARGET_CLASS_ID, 0));
            assert(targObject);
            targNode = createInterface->CreateObjectNode( targObject);
            assert(targNode);
            targName = mrBlueNode->GetName();
            targName += _T(".Target");
            targNode->SetName(targName);

            // hook up camera to target using lookat controller.
            createInterface->BindToTarget(mrBlueNode,targNode);

            // Reference the new node so we'll get notifications.
            MakeRefByID( FOREVER, 0, mrBlueNode);

            // Position camera and target at first point then drag.
            mat.IdentityMatrix();
            mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
            createInterface->SetNodeTMRelConstPlane(mrBlueNode, mat);
            createInterface->SetNodeTMRelConstPlane(targNode, mat);
            mrBlueObject->Enable(1);

            ignoreSelectionChange = TRUE;
            createInterface->SelectNode( targNode,0);
            ignoreSelectionChange = FALSE;
            res = TRUE;
            break;
					
        case 1:
            if (Length(m-pt0)<2)
                goto abort;
            mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
            createInterface->SetNodeTMRelConstPlane(targNode, mat);
            ignoreSelectionChange = TRUE;
            createInterface->SelectNode( mrBlueNode);
            ignoreSelectionChange = FALSE;
					
            createInterface->RedrawViews(createInterface->GetTime());  

            theHold.Accept(IDS_DS_CREATE);	 

            res = FALSE;	// We're done
            break;
        }			
        break;

    case MOUSE_MOVE:
        mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
        createInterface->SetNodeTMRelConstPlane(targNode, mat);
        createInterface->RedrawViews(createInterface->GetTime());	   
        res = TRUE;
        break;

	case MOUSE_FREEMOVE:
		SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		break;

    case MOUSE_ABORT:
      abort:
    assert( mrBlueObject );
    mrBlueObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
    theHold.Cancel();	 // deletes both the camera and target.
    mrBlueNode = NULL;			
    targNode = NULL;	 	
    createInterface->RedrawViews(createInterface->GetTime()); 
    CreateNewObject();	
    attachedToNode = FALSE;
    res = FALSE;						
    }
	
  done:
    createInterface->ReleaseViewport(vpx); 
    return res;
}

int MrBlueClassDesc::BeginCreate(Interface *i)
{
    IObjCreate *iob = i->GetIObjCreate();
	
    theMrBlueCreateMode.Begin( iob, this );
    iob->PushCommandMode( &theMrBlueCreateMode );
	
    return TRUE;
}

int MrBlueClassDesc::EndCreate(Interface *i)
{
	
    theMrBlueCreateMode.End();
    i->RemoveMode( &theMrBlueCreateMode );

    return TRUE;
}


// IO
#define ACTION_CHUNK		0xac00
#define MOUSE_ENABLED_CHUNK	0xac01
#define PROX_ENABLED_CHUNK	0xac02
#define PROX_DIST_CHUNK		0xac03
#define BBOX_ENABLED_CHUNK	0xac04
#define BBOX_X_CHUNK		0xac05
#define BBOX_Y_CHUNK		0xac06
#define BBOX_Z_CHUNK		0xac07
#define LOS_ENABLED_CHUNK	0xac08
#define LOS_TYPE_CHUNK		0xac09
#define LOS_VPT_ANG_CHUNK	0xac0a
#define LOS_OBJ_ANG_CHUNK	0xac0b

#define HL_URL_CHUNK		0xac0c
#define HL_URL_CAM_CHUNK	0xac0d
#define HL_URL_DESC_CHUNK	0xac0e

#define VPT_DESC_CHUNK		0xac10

#define ANIM_STATE_CHUNK	0xac12

IOResult MrBlueObject::Save(ISave *isave)
{
    ULONG written;

    isave->BeginChunk(ACTION_CHUNK);
    isave->Write(&action, sizeof(VRBL_Action), &written);
    isave->EndChunk();

    if (mouseEnabled) {
        isave->BeginChunk(MOUSE_ENABLED_CHUNK);
        isave->EndChunk();
    }

    if (proxDistEnabled) {
        isave->BeginChunk(PROX_ENABLED_CHUNK);
        isave->EndChunk();
    }

    isave->BeginChunk(PROX_DIST_CHUNK);
    isave->Write(&proxDist, sizeof(float), &written);
    isave->EndChunk();

    if (bboxEnabled) {
        isave->BeginChunk(BBOX_ENABLED_CHUNK);
        isave->EndChunk();
    }

    isave->BeginChunk(BBOX_X_CHUNK);
    isave->Write(&bboxX, sizeof(float), &written);
    isave->EndChunk();

    isave->BeginChunk(BBOX_Y_CHUNK);
    isave->Write(&bboxY, sizeof(float), &written);
    isave->EndChunk();

    isave->BeginChunk(BBOX_Z_CHUNK);
    isave->Write(&bboxZ, sizeof(float), &written);
    isave->EndChunk();

    if (losEnabled) {
        isave->BeginChunk(LOS_ENABLED_CHUNK);
        isave->EndChunk();
    }

    isave->BeginChunk(LOS_TYPE_CHUNK);
    isave->Write(&losType, sizeof(VRBL_LOS_Type), &written);
    isave->EndChunk();
    isave->BeginChunk(LOS_VPT_ANG_CHUNK);
    isave->Write(&losVptAngle, sizeof(float), &written);
    isave->EndChunk();
    isave->BeginChunk(LOS_OBJ_ANG_CHUNK);
    isave->Write(&losObjAngle, sizeof(float), &written);
    isave->EndChunk();

    if (!hlURL.isNull()) {
        isave->BeginChunk(HL_URL_CHUNK);
#ifdef _UNICODE
        isave->WriteWString(hlURL.data());
#else
        isave->WriteCString(hlURL.data());
#endif
        isave->EndChunk();
    }

    if (!hlCamera.isNull()) {
        isave->BeginChunk(HL_URL_CAM_CHUNK);
#ifdef _UNICODE
        isave->WriteWString(hlCamera.data());
#else
        isave->WriteCString(hlCamera.data());
#endif
        isave->EndChunk();
    }

    if (!hlDesc.isNull()) {
        isave->BeginChunk(HL_URL_DESC_CHUNK);
#ifdef _UNICODE
        isave->WriteWString(hlDesc.data());
#else
        isave->WriteCString(hlDesc.data());
#endif
        isave->EndChunk();
    }

    if (!vptDesc.isNull()) {
        isave->BeginChunk(VPT_DESC_CHUNK);
#ifdef _UNICODE
        isave->WriteWString(vptDesc.data());
#else
        isave->WriteCString(vptDesc.data());
#endif
        isave->EndChunk();
    }


    int c = animObjects.Count();
    if (c > 0) {
        isave->BeginChunk(ANIM_STATE_CHUNK);
        isave->Write(&animState, sizeof(AnimToggle), &written);
        isave->Write(&c, sizeof(int), &written);
        isave->EndChunk();
    }
    return IO_OK;
}

IOResult
MrBlueObject::Load(ILoad *iload)
{
    ULONG nread;
    IOResult res;
    enable = TRUE;

    while (IO_OK==(res=iload->OpenChunk())) {
        switch(iload->CurChunkID()) {
        case ACTION_CHUNK:
            iload->Read(&action, sizeof(VRBL_Action), &nread );
			// Topper no longer supports MrBlueMessage gdf 10-22-96
			if ( action == MrBlueMessage ) action = HyperLinkJump;
            break;
        case MOUSE_ENABLED_CHUNK:
            mouseEnabled = 1;
            break;
        case PROX_ENABLED_CHUNK:
            proxDistEnabled = 1;
            break;
        case PROX_DIST_CHUNK:
            iload->Read(&proxDist, sizeof(float), &nread );
            break;
        case BBOX_ENABLED_CHUNK:
            bboxEnabled = 1;
            break;
        case BBOX_X_CHUNK:
            iload->Read(&bboxX, sizeof(float), &nread );
            break;
        case BBOX_Y_CHUNK:
            iload->Read(&bboxY, sizeof(float), &nread );
            break;
        case BBOX_Z_CHUNK:
            iload->Read(&bboxZ, sizeof(float), &nread );
            break;
        case LOS_ENABLED_CHUNK:
            losEnabled = 1;
            break;
        case LOS_TYPE_CHUNK:
            iload->Read(&losType, sizeof(VRBL_LOS_Type), &nread );
            break;
        case LOS_VPT_ANG_CHUNK:
            iload->Read(&losVptAngle, sizeof(float), &nread );
            break;
        case LOS_OBJ_ANG_CHUNK:
            iload->Read(&losObjAngle, sizeof(float), &nread );
            break;
        case HL_URL_CHUNK: {
            char *url;
#ifdef _UNICODE
            iload->ReadWStringChunk(&url);
#else
            iload->ReadCStringChunk(&url);
#endif
            hlURL = url;
        }
        break;
        case HL_URL_CAM_CHUNK: {
            char *cam;
#ifdef _UNICODE
            iload->ReadWStringChunk(&cam);
#else
            iload->ReadCStringChunk(&cam);
#endif
            hlCamera = cam;
        }
        break;
        case HL_URL_DESC_CHUNK: {
            char *desc;
#ifdef _UNICODE
            iload->ReadWStringChunk(&desc);
#else
            iload->ReadCStringChunk(&desc);
#endif
            hlDesc = desc;
        }
        break;
        case VPT_DESC_CHUNK: {
            char *desc;
#ifdef _UNICODE
            iload->ReadWStringChunk(&desc);
#else
            iload->ReadCStringChunk(&desc);
#endif
            vptDesc = desc;
        }
        break;
        case ANIM_STATE_CHUNK: {
            int c;
            iload->Read(&animState, sizeof(AnimToggle), &nread );
            iload->Read(&c, sizeof(int), &nread );
            for (int i = 0; i < c; i++) {
                MrBlueAnimObj *obj = new MrBlueAnimObj();
                animObjects.Append(1, &obj);
            }
        }
        break;
        }
        iload->CloseChunk();
        if (res!=IO_OK) 
            return res;
    }
    return IO_OK;
}



RefTargetHandle
MrBlueObject::Clone(RemapDir& remap)
{
    MrBlueObject* ts = new MrBlueObject();
    ts->animObjects.SetCount(animObjects.Count());
    ts->ReplaceReference(0, vptCamera);
    for(int i = 0; i < animObjects.Count(); i++) {
        ts->animObjects[i] = new MrBlueAnimObj;
        ts->ReplaceReference(i+1, animObjects[i]->node);
    }
    ts->action = action;
    ts->mouseEnabled = mouseEnabled;
    ts->proxDistEnabled = proxDistEnabled;
    ts->bboxEnabled = bboxEnabled;
    ts->losEnabled = losEnabled;
    ts->proxDist = proxDist;
    ts->bboxX = bboxX;
    ts->bboxY = bboxY;
    ts->bboxZ = bboxZ;
    ts->losType = losType;
    ts->losVptAngle = losVptAngle;
    ts->losObjAngle = losObjAngle;
    ts->hlURL = hlURL;
    ts->hlCamera = hlCamera;
    ts->hlDesc = hlDesc;
    ts->vptCamera = vptCamera;
    ts->vptDesc = vptDesc;
    ts->ivalid.SetEmpty();
	BaseClone(this, ts, remap);
    return ts;
}
