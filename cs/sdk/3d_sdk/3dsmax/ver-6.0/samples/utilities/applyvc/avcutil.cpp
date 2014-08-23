/**********************************************************************
*<
FILE:			AVCUtil.cpp
DESCRIPTION:	Vertex Color Utility
CREATED BY:		Christer Janson
HISTORY:		Created Monday, June 02, 1997

  *>	Copyright (c) 1997 Kinetix, All Rights Reserved.
**********************************************************************/

#include "ApplyVC.h"
#include "modstack.h"
#include "Radiosity.h"

static ApplyVCUtil theApplyVC;

#define CFGFILE		_T("APPLYVC.CFG")
#define CFGVERSION	4

//==============================================================================
// class ApplyVCClassDesc
//==============================================================================
class ApplyVCClassDesc:public ClassDesc {
public:
    int 			IsPublic() {return 1;}
    void *			Create(BOOL loading = FALSE) {return &theApplyVC;}
    const TCHAR *	ClassName() {return GetString(IDS_AVCU_CNAME);}
    SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
    Class_ID		ClassID() {return APPLYVC_UTIL_CLASS_ID;}
    const TCHAR* 	Category() {return _T("");}
};

static ApplyVCClassDesc ApplyVCUtilDesc;
ClassDesc* GetApplyVCUtilDesc() {return &ApplyVCUtilDesc;}

//==============================================================================
// class ProgressDialog
//==============================================================================
class ProgressDialog {
public:
    void Begin();
    void End();
    BOOL Progress(int done, int total); //return TRUE if the user canceled

	void SetText(const TCHAR* text);
	static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    HWND hWnd;
    BOOL canceled;
	Interface* ip;
};

INT_PTR CALLBACK ProgressDialog::DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ProgressDialog* dialog = NULL;
	if(uMsg == WM_INITDIALOG) {
		dialog = (ProgressDialog*)(lParam);
		dialog->hWnd = hDlg;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		CenterWindow(hDlg, GetCOREInterface()->GetMAXHWnd());
		SendDlgItemMessage(hDlg, IDC_PROGRESSDLG_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 1000));
	}
	else
		dialog = (ProgressDialog*)(GetWindowLongPtr(hDlg, GWLP_USERDATA));

	// Check if 'canceled' was pressed.
	if( (dialog!=NULL) && (uMsg == WM_COMMAND) && (LOWORD(wParam) == IDCANCEL))
		dialog->canceled = TRUE;

	return FALSE;
}

void ProgressDialog::Begin() {
	ip = GetCOREInterface();
	canceled = FALSE;
	HWND hWnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_PROGRESSDLG), 
		ip->GetMAXHWnd(), DialogProc, (LPARAM)(this));
    DbgAssert(hWnd != NULL);
}

void ProgressDialog::End() {
    if(hWnd!=NULL) EndDialog( hWnd, !canceled );
    hWnd = NULL;
}

BOOL ProgressDialog::Progress(int done, int total)
{
    if(total > 0)
		SendDlgItemMessage(hWnd, IDC_PROGRESSDLG_PROGRESS, PBM_SETPOS, (done*1000)/total, 0);

    if(!ip->CheckMAXMessages())
        return FALSE;

    return canceled;
}

void ProgressDialog::SetText(const TCHAR* text) {
	DbgAssert(hWnd!=NULL);
	SetDlgItemText(hWnd, IDC_PROGRESSDLG_STATUS, text);
    ip->CheckMAXMessages();
}

//==============================================================================
// class ProgCallback
//
// Progress callback class for the VC evaluation functions
//==============================================================================

class ProgCallback : public EvalColProgressCallback {
public:
	ProgCallback(ApplyVCUtil* util) { u = util; }

	void Begin() { dialog.Begin(); canceled = FALSE; }
	void End() { dialog.End(); }
    BOOL progress(float prog) { //return TRUE if the user cancelled
        static int p=-1;
        // Eliminate ugly flashing of progress bar by only
        // calling update if the value has changed.
        if (p!=int(prog*100.f)) {
			canceled = dialog.Progress( int(prog*100.f), 100 );
            p = int(prog*100.0f);
        }
		return canceled;
    }
	void SetProgressText(const TCHAR* text) {dialog.SetText(text);}

private:
    UtilityObj* u;
	ProgressDialog dialog;
	BOOL canceled;
};

//==============================================================================
// class DerivedObjTab
//==============================================================================
class DerivedObjTab : public Tab<IDerivedObject*> {
public:
    BOOL InList(IDerivedObject *dob) {
        for (int i=0; i<Count(); i++) {
            if ((*this)[i]==dob) return TRUE;
        }
        return FALSE;
    }
};



static INT_PTR CALLBACK ApplyVCDlgProc(
                                       HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL initialized = FALSE;

    switch (msg) {
    case WM_INITDIALOG:
        theApplyVC.Init(hWnd);
		initialized = TRUE;
        break;
        
    case WM_DESTROY:
        theApplyVC.Destroy(hWnd);
		initialized = FALSE;
        break;
        
    case WM_COMMAND:
		//First, handle only the commands that require a UI update...
        switch (LOWORD(wParam)) {
		case IDC_VCUTIL_CHAN_COLOR:		theApplyVC.currentOptions.mapChannel = 0;	break;
		case IDC_VCUTIL_CHAN_ILLUM:		theApplyVC.currentOptions.mapChannel = -1;	break;
		case IDC_VCUTIL_CHAN_ALPHA:		theApplyVC.currentOptions.mapChannel = -2;	break;
		case IDC_VCUTIL_CHAN_MAP:
		case IDC_VCUTIL_CHAN_MAP_EDIT:
		case IDC_VCUTIL_CHAN_MAP_SPIN:
			theApplyVC.currentOptions.mapChannel = theApplyVC.iMapChanSpin->GetIVal();
			break;
        case IDC_VCUTIL_LIGHTING:		theApplyVC.currentOptions.lightingModel = LightingModel::kLightingOnly;		break;
        case IDC_VCUTIL_SHADED:			theApplyVC.currentOptions.lightingModel = LightingModel::kShadedLighting;	break;
        case IDC_VCUTIL_DIFFUSE:		theApplyVC.currentOptions.lightingModel = LightingModel::kShadedOnly;		break;
		case IDC_VCUTIL_COLORBYFACE:	theApplyVC.currentOptions.mixVertColors = false;	break;
		case IDC_VCUTIL_COLORBYVERT:	theApplyVC.currentOptions.mixVertColors = true;		break;
        case IDC_MIX:					theApplyVC.currentOptions.mixVertColors = (IsDlgButtonChecked(hWnd, IDC_MIX) == BST_CHECKED);			break;
        case IDC_CASTSHADOWS:			theApplyVC.currentOptions.castShadows = (IsDlgButtonChecked(hWnd, IDC_CASTSHADOWS) == BST_CHECKED);		break;
        case IDC_USEMAPS:				theApplyVC.currentOptions.useMaps = (IsDlgButtonChecked(hWnd, IDC_USEMAPS) == BST_CHECKED);				break;
        case IDC_USERADIOSITY:			theApplyVC.currentOptions.useRadiosity = (IsDlgButtonChecked(hWnd, IDC_USERADIOSITY) == BST_CHECKED);	break;
        case IDC_RADIOSITYONLY:			theApplyVC.currentOptions.radiosityOnly = (IsDlgButtonChecked(hWnd, IDC_RADIOSITYONLY) == BST_CHECKED);	break;
        }

		//Next, handle commands which do NOT require a UI update
		switch (LOWORD(wParam)) {
        case IDC_CLOSEBUTTON:
            theApplyVC.iu->CloseUtility();
            break;
        case IDC_VCUTIL_APPLY:
            theApplyVC.ApplySelected();
            break;
        //case IDC_VCUTIL_UPDATEALL:
        //    theApplyVC.UpdateAll();
        //    break;
		case IDC_VCUTIL_EDITCOLORS:
			theApplyVC.EditColors();
			break;
		default: // For all other commands -> Update the UI
			theApplyVC.UpdateUI();
			break;
		}

        break;
	case CC_SPINNER_CHANGE:
		//ignore this message if called before InitUI() is completed
		if( LOWORD(wParam)==IDC_VCUTIL_CHAN_MAP_SPIN && initialized ) {
			theApplyVC.currentOptions.mapChannel = theApplyVC.iMapChanSpin->GetIVal();
			theApplyVC.UpdateUI();
		}
		break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        theApplyVC.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
        break;
        
    default:
        return FALSE;
    }
    return TRUE;
}	

void ApplyVCUtil::RadiosityPluginChanged(void* param, NotifyInfo* info) {

    ApplyVCUtil* util = static_cast<ApplyVCUtil*>(param);
    DbgAssert(util != NULL);
    if(util != NULL)
        util->UpdateEnableStates();
}

ApplyVCUtil::ApplyVCUtil()
//Interface descriptor for IAssignVertexColors
:IAssignVertexColors (
	IASSIGNVERTEXCOLORS_INTERFACE_ID, _T("AssignVertexColors"), 0, NULL, FP_CORE,
	ApplyVCUtil::fnIdApplyNodes, _T("ApplyNodes"), 0, TYPE_INT, 0, 2,
		_T("nodes"), 0, TYPE_INODE_TAB,
		_T("vertexPaint"), 0, TYPE_REFTARG, f_keyArgDefault, NULL,
	end
)
//ApplyVCUtil constructor
{
    iu = NULL;
    ip = NULL;	
    hPanel = NULL;
	recentMod = NULL;
    currentOptions.mapChannel = 0, mapChannelSpin = 3; //map channel spinner defaults to 3
    currentOptions.lightingModel = kShadedLighting;
    currentOptions.mixVertColors = false;
    currentOptions.castShadows = false;
    currentOptions.useMaps = false;
    currentOptions.useRadiosity = false;
    currentOptions.radiosityOnly = false;

    // Register a notification callback for radiosity plugin changes
    RegisterNotification(RadiosityPluginChanged, this, NOTIFY_RADIOSITY_PLUGIN_CHANGED);
}

ApplyVCUtil::~ApplyVCUtil()
{
    UnRegisterNotification(RadiosityPluginChanged, this, NOTIFY_RADIOSITY_PLUGIN_CHANGED);
}

void ApplyVCUtil::SaveOptions()
{
    TSTR f;
    
    f = ip->GetDir(APP_PLUGCFG_DIR);
    f += _T("\\");
    f += CFGFILE;
    
    FILE* out = fopen(f, "w");
    if (out) {
        fputc(CFGVERSION, out);
        int dummy;
        dummy = currentOptions.lightingModel;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.mixVertColors;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.castShadows;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.useMaps;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.useRadiosity;
        fprintf(out, "%d\n", dummy);
        dummy = currentOptions.radiosityOnly;
        fprintf(out, "%d\n", dummy);
        fclose(out);
    }
}

void ApplyVCUtil::LoadOptions()
{
    TSTR f;
    
    f = ip->GetDir(APP_PLUGCFG_DIR);
    f += _T("\\");
    f += CFGFILE;
    
    FILE* in = fopen(f, "r");
    if (in) {
        int version;
        version = fgetc(in);	// Version
        if(version == CFGVERSION) {
            int dummy;
            fscanf(in, "%d\n", &dummy);
            currentOptions.lightingModel = static_cast<LightingModel>(dummy);
            fscanf(in, "%d\n", &dummy);
            currentOptions.mixVertColors = (dummy != 0);
            fscanf(in, "%d\n", &dummy);
            currentOptions.castShadows = (dummy != 0);
            fscanf(in, "%d\n", &dummy);
            currentOptions.useMaps = (dummy != 0);
            fscanf(in, "%d\n", &dummy);
            currentOptions.useRadiosity = (dummy != 0);
            fscanf(in, "%d\n", &dummy);
            currentOptions.radiosityOnly = (dummy != 0);
        }
        fclose(in);
    }
}

void ApplyVCUtil::GetOptions( IAssignVertexColors::Options& options ) {
	options = currentOptions;
}

void ApplyVCUtil::SetOptions( IAssignVertexColors::Options& options ) {
	currentOptions = options;
	UpdateUI();
}

void ApplyVCUtil::BeginEditParams(Interface *ip,IUtil *iu) 
{
    this->iu = iu;
    this->ip = ip;
    ip->AddRollupPage(
        hInstance,
        MAKEINTRESOURCE(IDD_VCUTIL_PANEL),
        ApplyVCDlgProc,
        GetString(IDS_AVCU_PANELTITLE),
        0);
	recentMod=NULL;
	LoadOptions();
	UpdateUI();
}

void ApplyVCUtil::EndEditParams(Interface *ip,IUtil *iu) 
{
    SaveOptions();
    
    this->iu = NULL;
    this->ip = NULL;
    ip->DeleteRollupPage(hPanel);
    hPanel = NULL;
}

void ApplyVCUtil::Init(HWND hWnd)
{
	hPanel = hWnd;

	HWND hMapChanEdit = GetDlgItem(hWnd,IDC_VCUTIL_CHAN_MAP_EDIT);
	iMapChanEdit = GetICustEdit( hMapChanEdit );
	iMapChanSpin = GetISpinner( GetDlgItem(hWnd,IDC_VCUTIL_CHAN_MAP_SPIN) );
	iMapChanSpin->LinkToEdit( hMapChanEdit, EDITTYPE_POS_INT );
	iMapChanSpin->SetLimits( 1, 99 );
	iMapChanSpin->SetValue( mapChannelSpin, FALSE );
}

void ApplyVCUtil::Destroy(HWND hWnd)
{
	mapChannelSpin = iMapChanSpin->GetIVal(); //save for the next time we open the UI
	ReleaseICustEdit( iMapChanEdit );
	ReleaseISpinner( iMapChanSpin );
}

void ApplyVCUtil::UpdateUI() {
	if( hPanel==NULL ) return;

	int channelRadioButtonID;
    switch( currentOptions.mapChannel ){
    case -2:	channelRadioButtonID = IDC_VCUTIL_CHAN_ALPHA;	break;
    case -1:	channelRadioButtonID = IDC_VCUTIL_CHAN_ILLUM;	break;
    case 0:		channelRadioButtonID = IDC_VCUTIL_CHAN_COLOR;	break;
	default:	channelRadioButtonID = IDC_VCUTIL_CHAN_MAP;		break;
    }
	iMapChanEdit->Enable( currentOptions.mapChannel>0 );
	iMapChanSpin->Enable( currentOptions.mapChannel>0 );
	int spinValue = (currentOptions.mapChannel>0? currentOptions.mapChannel : iMapChanSpin->GetIVal());
	if( currentOptions.mapChannel>0 ) iMapChanSpin->SetValue( spinValue, FALSE );
	TSTR chanName;
	if( GetChannelName( spinValue, chanName ) )
		SetDlgItemText( hPanel, IDC_VCUTIL_CHAN_NAME, chanName.data() );
	else SetDlgItemText( hPanel, IDC_VCUTIL_CHAN_NAME, GetString(IDS_MAPNAME_NONE) );

    int radioButtonID;
    switch(currentOptions.lightingModel){
    case kLightingOnly:
        radioButtonID = IDC_VCUTIL_LIGHTING;
        break;
    case kShadedLighting:
        radioButtonID = IDC_VCUTIL_SHADED;
        break;
    case kShadedOnly:
        radioButtonID = IDC_VCUTIL_DIFFUSE;       
        break;
    }

    CheckRadioButton(hPanel, IDC_VCUTIL_CHAN_COLOR, IDC_VCUTIL_CHAN_MAP, channelRadioButtonID);
    CheckRadioButton(hPanel, IDC_VCUTIL_LIGHTING, IDC_VCUTIL_DIFFUSE, radioButtonID);
	CheckRadioButton(hPanel, IDC_VCUTIL_COLORBYFACE, IDC_VCUTIL_COLORBYVERT, 
		currentOptions.mixVertColors? IDC_VCUTIL_COLORBYVERT : IDC_VCUTIL_COLORBYFACE );

    CheckDlgButton(hPanel, IDC_MIX, currentOptions.mixVertColors ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hPanel, IDC_CASTSHADOWS, currentOptions.castShadows ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hPanel, IDC_USEMAPS, currentOptions.useMaps ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hPanel, IDC_USERADIOSITY, currentOptions.useRadiosity ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hPanel, IDC_RADIOSITYONLY, currentOptions.radiosityOnly ? BST_CHECKED : BST_UNCHECKED);

	//FIXME: the way the state of the Edit button is controlled, is a hack...
	//but keeping of track of the modifier to determine if/when it's still valid would be a lot of overhead
	ICustButton* editBtn = GetICustButton( GetDlgItem(hPanel,IDC_VCUTIL_EDITCOLORS) );
	editBtn->Enable( recentMod!=NULL );
	ReleaseICustButton(editBtn);

    UpdateEnableStates();  
}

void ApplyVCUtil::UpdateEnableStates() {
    
    if (hPanel != NULL) {
        switch(currentOptions.lightingModel) {
        case kLightingOnly:
        case kShadedLighting:
            {
                // Enable shadow casting
                EnableWindow(GetDlgItem(hPanel, IDC_CASTSHADOWS), true);
                // Enable radiosity, if present in the scene
                bool enableRadiosity = (GetCompatibleRadiosity() != NULL);
                EnableWindow(GetDlgItem(hPanel, IDC_USERADIOSITY), enableRadiosity);
                EnableWindow(GetDlgItem(hPanel, IDC_RADIOSITYONLY), (enableRadiosity && currentOptions.useRadiosity));
            }        
            break;
        case kShadedOnly:
            // Disable shadow casting
            EnableWindow(GetDlgItem(hPanel, IDC_CASTSHADOWS), true);
            EnableWindow(GetDlgItem(hPanel, IDC_USERADIOSITY), false);
            EnableWindow(GetDlgItem(hPanel, IDC_RADIOSITYONLY), false);
            break;
        }
    }
}

//***************************************************************************
//
// Apply / Update selected
//
//***************************************************************************

void ApplyVCUtil::ApplySelected()
{
	Tab<INode*> nodes;
	for (int i=0; i<ip->GetSelNodeCount(); i++) {
		INode* node = ip->GetSelNode(i);
		nodes.Append( 1, &node );
	}

	ApplyNodes( &nodes );

	ip->RedrawViews(ip->GetTime());
}


DWORD WINAPI dummy(LPVOID arg) {
    return(0);
}

int ApplyVCUtil::ApplyNodes( Tab<INode*>* nodes, ReferenceTarget* dest ) {
	DbgAssert( nodes!=NULL );
	if( ip==NULL ) ip=GetCOREInterface(); //hack

	Modifier* mod=NULL; //modifier to apply to the nodes, if needed
	IVertexPaint* ivertexPaint; //the interface to the modifier
	if( dest==NULL ) {
		mod = (Modifier*)CreateInstance(OSM_CLASS_ID, PAINTLAYERMOD_CLASS_ID);
		if( mod!=NULL ) {
			ivertexPaint = (IVertexPaint*)mod->GetInterface(IVERTEXPAINT_INTERFACE_ID);
			ivertexPaint->SetOptions( currentOptions );
		}
	}
	else ivertexPaint = (IVertexPaint*)dest->GetInterface(IVERTEXPAINT_INTERFACE_ID);

	if( ivertexPaint==NULL ) {
		if( mod!=NULL ) mod->DeleteThis();
		return 0;
	}

	theHold.Begin();
	if( mod!=NULL ) { // need to put the vertex paint modifier on the nodes
		AddModifier( *nodes, mod );
		recentMod = mod;
	}

	for( int i=0; i<nodes->Count(); i++ ) {
		INode* node = nodes->operator[]( i );
		if( node!=NULL ) {
			BOOL result;
			result = ApplyNode( node, ivertexPaint );
		}
	}

	theHold.Accept( GetString(IDS_HOLDMSG_APPLYVTXCOLOR) ); //FIXME: localize
	UpdateUI();
	return 1;
}

BOOL ApplyVCUtil::ApplyNode(INode* node, IVertexPaint* ivertexPaint, bool noUpdate)
{
    ProgCallback fn(this);
    
    if (!node)
        return TRUE;
    
    ObjectState os = node->EvalWorldState(ip->GetTime());
    if (!os.obj)
        return TRUE;
    
    if (!os.obj->CanConvertToType(triObjectClassID))
        return TRUE;
    
    FaceColorTab faceColors;
    ColorTab mixedVertexColors;
	int result = TRUE;
    
    // Update the vertex colors if needed
    if (!noUpdate) {
		theHold.Suspend();
		fn.Begin();
		fn.SetProgressText( (TSTR(GetString(IDS_PROGRESSMSG_RENDERING)) + TSTR( node->GetName() ) + TSTR(_T("..."))).data() );
        
		if (currentOptions.mixVertColors) {
			result = calcMixedVertexColors(node, ip->GetTime(), currentOptions, mixedVertexColors, &fn);
		}
		else {
			result = calcFaceColors(node, ip->GetTime(), currentOptions, faceColors, &fn);
		}
        
		fn.End();
		theHold.Resume();
    }
	if( !result ) return FALSE;
    
	//If the object to hold the colors has not been specified, create a new Vertex Paint mod
	if( ivertexPaint==NULL ) {    
		// Retrieve the modifier, and create it if it does not exist on this node.
		ApplyVCMod* mod;
		if (!(mod = (ApplyVCMod*)GetModifier(node, PAINTLAYERMOD_CLASS_ID))) {
			mod = (ApplyVCMod*)CreateInstance(OSM_CLASS_ID, PAINTLAYERMOD_CLASS_ID);
	        
			Object* obj = node->GetObjectRef();
			IDerivedObject* dobj = CreateDerivedObject(obj);
			dobj->AddModifier(mod);
			node->SetObjectRef(dobj);
		}	 
		IVertexPaint* ivertexPaint = (IVertexPaint*)mod->GetInterface(IVERTEXPAINT_INTERFACE_ID);
	}

	if( ivertexPaint!=NULL ) {
		if (currentOptions.mixVertColors)
			ivertexPaint->SetColors( node, mixedVertexColors );
		else
			ivertexPaint->SetColors( node, faceColors );
	}

	//-- now obsolete - MAB - 05/15/03
    // Update the modifier with the new vertex colors
    //if (!noUpdate) {
    //    if (currentOptions.mixVertColors) {
    //        mod->SetMixedColors(mixedVertexColors);
    //    }
    //    else {
    //        mod->SetColors(faceColors);
    //    }
    //}
    

    // [dl | 15feb2002] Note: This first loop was commented out, and I can't
    // figure out why. I'm un-commenting it, since it seems necessary to
    // prevent memory leaks.
    int i;
    for (i=0; i<mixedVertexColors.Count(); i++) {
        delete mixedVertexColors[i];
    }
    for (i=0; i<faceColors.Count(); i++) {
        delete faceColors[i];
    }
    
    return TRUE;
}

//***************************************************************************
//
// Update all
//
//***************************************************************************

//void ApplyVCUtil::UpdateAll()
//{   
//    CheckForAndMakeUnique();
//    
//    for (int i=0; i<ip->GetRootNode()->NumberOfChildren(); i++)
//    {
//        if (!UpdateAllEnum(ip->GetRootNode()->GetChildNode(i))) {
//            break;
//        }
//    }
//    
//    ip->RedrawViews(ip->GetTime());
//}
//
//BOOL ApplyVCUtil::UpdateAllEnum(INode* node)
//{
//    for (int i=0; i<node->NumberOfChildren(); i++) {
//        if (!UpdateAllEnum(node->GetChildNode(i))) {
//            return FALSE;
//        }
//    }
//    
//    if (GetModifier(node, PAINTLAYERMOD_CLASS_ID)) {
//        ApplyNode(node);
//    }
//    
//    return TRUE;
//}


//***************************************************************************
//
// Edit Colors
//
//***************************************************************************

void ApplyVCUtil::EditColors() {
	ip->SetCommandPanelTaskMode( TASK_MODE_MODIFY );
	//FIXME: after opening the mod panel, this should also
	//ensure that the recentMod modifier is opened for editing
}


//***************************************************************************
//
// Utility methods
//
//***************************************************************************

// Traverse the pipeline for this node and return the first modifier with a
// specified ClassID

Modifier* ApplyVCUtil::GetModifier(INode* node, Class_ID modCID)
{
    Object* obj = node->GetObjectRef();
    
    if (!obj)
        return NULL;
    
    ObjectState os = node->EvalWorldState(0);
    if (os.obj && os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) {
        return NULL;
    }
    
    // For all derived objects (can be > 1)
    while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
        IDerivedObject* dobj = (IDerivedObject*)obj;
        int m;
        int numMods = dobj->NumModifiers();
        // Step through all modififers and verify the class id
        for (m=0; m<numMods; m++) {
            Modifier* mod = dobj->GetModifier(m);
            if (mod) {
                if (mod->ClassID() == modCID) {
                    // Match! Return it
                    return mod;
                }
            }
        }
        obj = dobj->GetObjRef();
    }
    
    return NULL;
}

void ApplyVCUtil::AddModifier(Tab<INode*>& nodes, Modifier* mod) {
	theHold.Begin();

	for( int i=0; i<nodes.Count(); i++ ) {
		if( nodes[i]==NULL ) continue;
		Object* obj = nodes[i]->GetObjectRef();
		IDerivedObject* dobj = CreateDerivedObject(obj);
		//dobj->SetAFlag(A_LOCK_TARGET); //FIXME: needed?
		dobj->AddModifier(mod);
		//dobj->ClearAFlag(A_LOCK_TARGET); //FIXME: needed?
		nodes[i]->SetObjectRef(dobj);
	}

	theHold.Accept( GetString(IDS_HOLDMSG_ADDMODIFIER) );
}

void ApplyVCUtil::DeleteModifier( INode* node, Modifier* mod ) {
	//for( int i=0; i<nodes.Count(); i++ ) {
	Object* obj =  node->GetObjectRef(); //nodes[i]->GetObjectRef();
	if (!obj) return;

	theHold.Begin();

	// For all derived objects (can be > 1)
	while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) {
		IDerivedObject* dobj = (IDerivedObject*)obj;
		int numMods = dobj->NumModifiers();
		// Step through all modififers
		for (int j=(numMods-1); j>=0; j--) {
			Modifier* curMod = dobj->GetModifier(j);
			if( curMod==mod )
				dobj->DeleteModifier(j);
		}
		obj = dobj->GetObjRef();
	}
	//}

	theHold.Accept( GetString(IDS_HOLDMSG_DELETEMODIFIER) ); //FIXME: localize
}

BOOL ApplyVCUtil::GetChannelName( int index, TSTR& name ) {
	Interface* ip = GetCOREInterface();
	int nodeCount = ip->GetSelNodeCount();

	TCHAR propKey[16];
	_stprintf(propKey, _T("MapChannel:%i"), index );

	//get the channel name for the first selected obejct
	INode* node;
	BOOL ok = (
		(nodeCount>0) &&
		((node=ip->GetSelNode(0))!=NULL) &&
		(node->GetUserPropString( propKey, name )) );

	//check the channel name for each other selected object...
	for( int i=1; ok && i<nodeCount; i++ ) {
		TSTR nextName; 
		INode* node = ip->GetSelNode(i);
		ok = node->GetUserPropString( propKey, nextName );
		//...we'll return a default value unless all the objects have a maching channel name
		if( nextName!=name ) ok=FALSE;
	}
	
	return ok;
}