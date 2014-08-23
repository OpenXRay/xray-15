// Copyright (C) 2000 by Autodesk, Inc.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
///////////////////////////////////////////////////////////////////////////////
// XREFUTIL.CPP
//
// DESCR:
//     Simple Utility plug-in sample that shows how to do some of the MAX
//     XRef Scene and Object functionality via SDK APIs.  
//
//     Does not do:
//     -- undo/redo handling
//     -- extensive error checking
//     
//
// CHANGE LOG:
//     03/2000 : DY : Created 
//		07 Nov 2000 : rjc : updated to support IXRefObjectManager
//
///////////////////////////////////////////////////////////////////////////////

#include "xrefutil.h"

#define ERROR_TITLE "XRef Util Error"

///////////////////////////////////////
// XrefutilClassDesc -- our required ClassDesc
///////////////////////////////////////

class XrefutilClassDesc:public ClassDesc2 
{
public:
    int             IsPublic()                    { return 1; }
    void *          Create(BOOL loading = FALSE)  { return &theXrefutil; }
    const TCHAR *   ClassName()                   { return GetString(IDS_CLASS_NAME); }
    SClass_ID       SuperClassID()                { return UTILITY_CLASS_ID; }
    Class_ID        ClassID()                     { return XREFUTIL_CLASS_ID; }
    const TCHAR *   Category()                    { return GetString(IDS_CATEGORY); }
    const TCHAR *   InternalName()                { return _T("Xrefutil"); }
    HINSTANCE       HInstance()                   { return hInstance; }
};

static XrefutilClassDesc XrefutilDesc;

ClassDesc2* GetXrefutilDesc() 
{
    return &XrefutilDesc;
}

static BOOL CALLBACK XrefutilDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        theXrefutil.Init(hWnd);
        break;
		
    case WM_DESTROY:
        theXrefutil.Destroy(hWnd);
        break;
		
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
			
        case IDC_CMD_XS_ADD:
            // -- Add a new XRef Scene into the current scene --
            theXrefutil.AddNewXrefScene(hWnd);
            break;
			
        case IDC_CMD_XS_CNV:
            // -- Convert selected nodes into an XRef'd scene --
            theXrefutil.ConvertSelectedToXrefScene(hWnd);
            break;
			
        case IDC_CMD_XS_RFS:
            // -- Refresh/reload all scene xrefs --
            theXrefutil.RefreshAllXrefScenes(hWnd);
            break;
			
        case IDC_CMD_XS_MRG:
            // -- merge in all scene xrefs --
            theXrefutil.MergeAllXrefScenes(hWnd);
            break;
			
			
        case IDC_CMD_XO_ADD:
            // -- Add a new XRef Object into the current scene --
            theXrefutil.AddNewXrefObject(hWnd);
            break;
			
        case IDC_CMD_XO_CNVSEL:
            // -- Convert selected object into an XRef Object --
            theXrefutil.ConvertSelectedToXrefObject(hWnd);
            break;
			
        case IDC_CMD_XO_EXP:
            // -- fake "export" of all xref objects in scene --
            theXrefutil.ExportXrefObjects(hWnd);
            break;
			
			
			// Adding support for IXRefObjectManager -- 7 Nov 2000 rudy cazabon
		case IDC_CMD_IXOMGR_ADD:
			// Use the IXRefObjectManager interface to bring XRefs into a scene
			theXrefutil.AddNewXrefScene_IXROMGR(hWnd);
			break;
			
		case IDC_CMD_IXOMGR_REFRESH:
			// Refresh/reload an given XRef object
			break;
			
		case IDC_CMD_IXOMGR_GETALL:
			// Given a .max file bring in all XRef objects in there
			theXrefutil.GetAllXRefsObjects_IXROMGR(hWnd);
			break;
			
        case IDC_CLOSE:
            theXrefutil.m_pUtil->CloseUtility();
            break;
			
        }
		
		
        break;
		
		default:
			return FALSE;
    }
    return TRUE;
}

///////////////////////////////////////
// some more (Xrefutil specific) NodeEnumerator derivations
///////////////////////////////////////

class UnflaggedNodeNamer : public NodeEnumerator
{
public:
    Tab<TSTR *> * m_namelist;
	
    UnflaggedNodeNamer() : m_namelist(NULL) {};
    virtual bool Proc(INode * pNode);
};


bool UnflaggedNodeNamer::Proc(INode * pNode)
{
    if (!m_namelist) return false;
    // otherwise, grab the list of node names and stick them in namelist
    if (! pNode->TestAFlag(A_WORK1)) {
        TSTR * pName = new TSTR(pNode->GetName()); // consumer must delete
        m_namelist->Append(1, &pName, 5);
    }
    return true;
}

class UnflaggedNodeDeleter : public NodeEnumerator
{
public:
    virtual bool Proc(INode * pNode);
};


bool UnflaggedNodeDeleter::Proc(INode * pNode)
{
    // NOTE: should be called with Enumerate(procfirst = false) !
    // Always do safety check so that we don't delete root accidentally
    if (!pNode->IsRootNode() && !pNode->TestAFlag(A_WORK1)) {
        // pNode->DeleteThis();
        // theXrefutil.m_pInterface->DeleteNode(pnode, FALSE);
        pNode->Delete(0,TRUE);
    }
	
    return true;
}

class XRefObjFinder : public NodeEnumerator
{
public:
    TSTR * m_buffer;
	
    XRefObjFinder() : m_buffer(NULL) {};
    virtual bool Proc(INode * pNode);
};

bool XRefObjFinder::Proc(INode * pNode)
{
    TSTR workstring;
	
    // NOTE: We dump all info into one big string
    // Not super realistic, and probably won't work with
    // a scene with lots of XRef objects, but in a real exporter
    // situation, you'd be dumping out to a file pointer anyway
	
    if (!pNode || !m_buffer) return false;
    Object *obj = pNode->GetObjectRef();
    if (obj && obj->SuperClassID()==SYSTEM_CLASS_ID && 
		obj->ClassID()==Class_ID(XREFOBJ_CLASS_ID,0)) {
        IXRefObject *ix = (IXRefObject *)obj;
        // if pNode refs an XRef object, let's pull some info out of it
        workstring.printf("Node <%s> XREF <%s> filename:<%s> proxy:<%s> anim-off:<%s> update-mats:<%s> \x0D\x0A",
            pNode->GetName(), ix->GetCurObjName(), ix->GetCurFileName(), 
            (ix->GetUseProxy()) ? "T" : "F",
            (ix->GetIgnoreAnim()) ? "T" : "F",
            (ix->GetUpdateMats()) ? "T" : "F");
        m_buffer->append(workstring);
    }
    return true;
}


// Helper class derived from AnimEnum used to delete (keyframed) animation info

class DeleteAllAnimEnum : public AnimEnum 
{
public: 
    BOOL delAtFrame0;
	
    DeleteAllAnimEnum(BOOL delAt0) : AnimEnum(SCOPE_ALL) {delAtFrame0 = delAt0;}
    int proc(Animatable *anim, Animatable *client, int subNum) {
        if (delAtFrame0) {
            // Evaluate the controller at frame 0 before
            // deleting. This will cause its cache to take on
            // its value at frame 0.
            Control *cont = GetControlInterface(anim);
            if (cont) {
                Point3 p;
				Point4 p4;
                Quat q;
                ScaleValue s;
                float f;
                Interval valid;
                switch (cont->SuperClassID()) {
                case CTRL_FLOAT_CLASS_ID:
                    cont->GetValue(0,&f,valid); break;
                case CTRL_POSITION_CLASS_ID:
				case CTRL_POINT3_CLASS_ID:
                    cont->GetValue(0,&p,valid); break;
                case CTRL_ROTATION_CLASS_ID:
                    cont->GetValue(0,&q,valid); break;
                case CTRL_SCALE_CLASS_ID:
                    cont->GetValue(0,&s,valid); break;
                case CTRL_POINT4_CLASS_ID:
                    cont->GetValue(0,&p4,valid); break;
                }
                cont->SetORT(ORT_CONSTANT,ORT_BEFORE);
                cont->SetORT(ORT_CONSTANT,ORT_AFTER);
            }
        }
        anim->DeleteTime(FOREVER,0);
        anim->EditTimeRange(Interval(0,0),EDITRANGE_LINKTOKEYS);
        return ANIM_ENUM_PROCEED;
    }
};


///////////////////////////////////////
// Xrefutil class implementation -- our main utility plugin class
///////////////////////////////////////

///////////////////
// Construction/destruction

Xrefutil::Xrefutil()
: m_pUtil(NULL)
, m_pInterface(NULL)
, m_hPanel(NULL)
, m_proxyholder(false)
, m_ignoreanimholder(false)
{
}

Xrefutil::~Xrefutil()
{
    
}

///////////////////
// UtilityObj overrides

void Xrefutil::BeginEditParams(Interface *ip,IUtil *iu) 
{
    m_pUtil = iu;
    m_pInterface = ip;
    m_hPanel = ip->AddRollupPage(hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		XrefutilDlgProc,
		GetString(IDS_PARAMS),
		0);
}

void Xrefutil::EndEditParams(Interface *ip,IUtil *iu) 
{
    m_pUtil = NULL;
    m_pInterface = NULL;
    ip->DeleteRollupPage(m_hPanel);
    m_hPanel = NULL;
}

void Xrefutil::DeleteThis()
{
    // since there's only one static instance of the
    // UtilityObj, we don't do anything here
}


///////////////////
// utility methods

void Xrefutil::Init(HWND hWnd)
{
}

void Xrefutil::Destroy(HWND hWnd)
{
}

bool Xrefutil::DoOpenSaveDialog(TSTR &fileName, bool bOpen)
{
	
    // Does a standard Win32 CommonDlg Save-As or Open dialog
    // for a .MAX file
    //
    // (doesn't use registered custom dlg in MAX, as
    // in truth, only MAX can access this cache currently,
    // although you'll get any registered dialog if
    // you call Interface::FileSave or something similar,
    // but we just want to get a filename, not save yet)
	
    OPENFILENAME    ofn;
    TCHAR           szFilter[]=__TEXT("3D Studio MAX (*.MAX)\0*.MAX\0\0");     
    TCHAR           fname[512];
	
    _tcscpy(fname,fileName);
    
    // set up that OPENFILENAME struct
    ::memset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize      = sizeof(OPENFILENAME);
    ofn.hwndOwner        = m_hPanel;
    ofn.nFilterIndex     = 1L;
    ofn.lpstrFilter      = szFilter;
    ofn.lpstrCustomFilter = (LPTSTR)NULL;
    ofn.lpstrFile        = fname;
    ofn.nMaxFile         = sizeof(fname) / sizeof(TCHAR);
    ofn.lpstrFileTitle   = NULL;
    ofn.nMaxFileTitle    = 0;
    ofn.lpstrInitialDir  = m_pInterface->GetDir(APP_SCENE_DIR);
    ofn.lpstrTitle       = (LPCSTR)NULL;
    if (bOpen) {
        ofn.Flags            = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES;
    } else {
        ofn.Flags            = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
    }
    ofn.lpstrDefExt      = _TEXT("MAX");
	
    if (bOpen) {
        if (GetOpenFileName(&ofn)) {
			
            // NOTE: More error checking needs to be done for this
            // to be practical -- e.g. we shouldn't allow the user to
            // select the currently open file.  
			
            fileName = TSTR(ofn.lpstrFile); // full path and file 
            return true; // success
			
        } else {
            // user canceled
            return false;
        }
		
    } else {   
        if (GetSaveFileName(&ofn)) {
			
            // NOTE: More error checking needs to be done for this
            // to be practical -- e.g. we shouldn't allow the user to
            // select the currently open file.  
			
            fileName = TSTR(ofn.lpstrFile); // full path and file 
            return true; // success
			
        } else {
            // user canceled
            return false;
        }
    }
    return false; // failure
}


extern HINSTANCE hInstance;

static BOOL CALLBACK PickObjDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int i, ix;
    UINT chkval;
	
    switch (message) {
		
    case WM_INITDIALOG: 
        // (populate the listbox)
        SendDlgItemMessage(hWnd,IDC_LIST_NODES,LB_RESETCONTENT,0,0);
        for (i = 0; i < theXrefutil.m_objnamesholder.Count(); i++) {
            TSTR oname = *(theXrefutil.m_objnamesholder[i]);
            ix = SendDlgItemMessage(hWnd,IDC_LIST_NODES,LB_ADDSTRING,0,(LPARAM)(TCHAR*)oname);
            SendDlgItemMessage(hWnd,IDC_LIST_NODES,LB_SETITEMDATA,ix,i);
        }
        CenterWindow(hWnd,GetParent(hWnd));
        theXrefutil.m_picknameholder = "";
        break;
		
    case WM_COMMAND:
		
        switch (LOWORD(wParam)) {
			
        case IDCANCEL:
            theXrefutil.m_picknameholder = "";
            EndDialog(hWnd,0);
            break;
			
        case IDOK:
            // on okay, we just copy over user-picked info into our
            // utility object -- slightly unusual, but it's a modal dialog
            // and the sample continues the xref object logic fully in
            // AddNewXrefObject
            ix = ::SendDlgItemMessage(hWnd,IDC_LIST_NODES,LB_GETCURSEL,0,0);
            if (ix != LB_ERR) {
                theXrefutil.m_picknameholder = *(theXrefutil.m_objnamesholder[ix]);
            }
            chkval = ::IsDlgButtonChecked(hWnd, IDC_CHK_PROXY);
            if (chkval == BST_CHECKED) theXrefutil.m_proxyholder = true;
            else theXrefutil.m_proxyholder = false;
            chkval = ::IsDlgButtonChecked(hWnd, IDC_CHK_NOANIM);
            if (chkval == BST_CHECKED) theXrefutil.m_ignoreanimholder = true;
            else theXrefutil.m_ignoreanimholder = false;
			
            EndDialog(hWnd,1);
            break;
			
        default:
            return FALSE;
			
        }
		
		default:
			return FALSE;
    }
	
    return TRUE;
	
}


bool Xrefutil::DoPickObjDialog()
{
    // populate IDD_PICKOBJ IDC_LIST_NODES with objnames,
    // show dialog modal, get and return results
    int dlgres = 0;
    
    dlgres = DialogBox(hInstance, 
		MAKEINTRESOURCE(IDD_PICKOBJ),
		m_hPanel,
		PickObjDlgProc);
    if (dlgres == 1) return true;
    return false; // failure
}


static BOOL CALLBACK ExportDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TSTR * contents = NULL;
	
    switch (message) {
		
    case WM_INITDIALOG: 
        // (populate the editbox)
        contents = (TSTR *) lParam;
        ::SetDlgItemText(hWnd, IDC_EDIT_EXPRES, (TCHAR*)(*contents));
        CenterWindow(hWnd,GetParent(hWnd));
        break;
		
    case WM_COMMAND:
		
        switch (LOWORD(wParam)) {
			
        case IDOK:
            EndDialog(hWnd,1);
            break;
			
        default:
            return FALSE;
			
        }
		
		default:
			return FALSE;
    }
	
    return TRUE;
	
}


///////////////////
// xref methods

// (XRef Scene methods)

void Xrefutil::AddNewXrefScene(HWND hWnd)
{
    // *** AddNewXrefScene ***
    //
    // Add new XRef'd Scene to current scene -- simply get the
    // name of the .MAX file, and use RootNode::AddNewXRefFile
    // to hook that xref'd scene into the current scene hierarchy
    //
    // Note: Don't confuse XRef scenes with XRef objects.
    // XRef scenes live as special children of the (single) root
    // node of the current scene, basically as "XRef'd root nodes"
    // of the external scene.  Note that merged-in XRef Root Nodes
    // can come from original scenes that already had another scene
    // xref'd in, and so-on.  See the SDK documentation on the 
    // various XRef Scene APIs on the INode object, keeping in mind
    // that these APIs only function when the INode is in fact the
    // root node of the scene.
    //
    
    TSTR filename = "";
    INode * pRootNode = m_pInterface->GetRootNode();
    if (!pRootNode) {
        // well, this is actually _really_ bad, but we just exit
        return;
    }
    if (!DoOpenSaveDialog(filename, true)) {
        // either cancel or fail, just return
        return;
    }    
    pRootNode->AddNewXRefFile(filename, TRUE);
    m_pInterface->RedrawViews(m_pInterface->GetTime());
}


void Xrefutil::ConvertSelectedToXrefScene(HWND hWnd)
{
    // *** ConvertSelectedToXrefScene ***
    //
    // Relatively simple -- take the selected nodes, save them out
    // to a new .MAX file, delete the selected nodes from the 
    // current scene, and then do a RootNode->AddNewXRefFile
    // with the just-saved MAX file.
	
    INode * pNode = NULL;
    TSTR filename = "";
    int i;
	
    INode * pRootNode = m_pInterface->GetRootNode();
    if (!pRootNode) {
        // well, this is actually _really_ bad, but we just exit
        return;
    }
    if (m_pInterface->GetSelNodeCount() == 0) {
        ::MessageBox(hWnd, GetString(IDS_ERR3), ERROR_TITLE, MB_ICONSTOP | MB_OK);
        return;
    }
	
    Tab<INode *> nodetab;
    nodetab.ZeroCount();
	nodetab.Shrink();
    for (i = 0; i < m_pInterface->GetSelNodeCount(); i++) {
        pNode = m_pInterface->GetSelNode(i);
        nodetab.Append(1, &pNode, 5);
    }
	
    if (!DoOpenSaveDialog(filename)) {
        // either cancel or fail, just return
        return;
    }
	
    m_pInterface->FileSaveSelected(filename);
	
    // delete selected nodes, don't refresh yet
    for (i = 0; i < nodetab.Count(); i++) {
        nodetab[i]->Delete(0,TRUE);
    }
	
    // add in the nodes we saved out as an xref'd scene
    pRootNode->AddNewXRefFile(filename, TRUE);
    
    m_pInterface->RedrawViews(m_pInterface->GetTime());
	
}


void Xrefutil::RefreshAllXrefScenes(HWND hWnd)
{
    // *** RefreshAllXrefScenes ***
    // 
    // Refreshes all Xref'd Scenes -- mimics 
    // MAX's "Update Now" button on XRef Scene dialog,
    // with all xref'd scenes selected.
    //
    // Simply walk through all XRef'd scenes (hanging off root)
    // and use RootNode::ReloadXRef() to reload each one.
	
    INode * pNode = NULL;
    int i, numxrefscenes;
	
    INode * pRootNode = m_pInterface->GetRootNode();
    if (!pRootNode) {
        // well, this is actually _really_ bad, but we just exit
        return;
    }
    numxrefscenes = pRootNode->GetXRefFileCount();
    for (i = 0; i < numxrefscenes; i++) {
        pRootNode->ReloadXRef(i);
    }
}


void Xrefutil::MergeAllXrefScenes(HWND hWnd)
{
    // *** MergeAllXrefScenes ***
    //
    // Merges ("binds") all XRef Scenes into the
    // current scene.  Just using RootNode::BindXRefFile
    // will merge in the real scene objects from the xref'd
    // scene as real modifiable objects, and delete the
    // xref scene link from the scene hierarchy.
	
    INode * pNode = NULL;
    int i, numxrefscenes;
	
    INode * pRootNode = m_pInterface->GetRootNode();
    if (!pRootNode) {
        // well, this is actually _really_ bad, but we just exit
        return;
    }
    numxrefscenes = pRootNode->GetXRefFileCount();
    for (i = 0; i < numxrefscenes; i++) {
        pRootNode->BindXRefFile(i);
    }
	
}


// (XRef Object methods)



// Function name	: Xrefutil::NodeToXref
// Description	    : This method does a straightforward conversion of a Node to an XRef
// Return type		: void 
// Argument         : INode * pNode
// Argument         : TSTR &filename
// Argument         : bool bProxy
// Argument         : bool bIgnoreAnim
void Xrefutil::NodeToXref(INode * pNode, TSTR &filename, bool bProxy, bool bIgnoreAnim)
{
    IXRefObject * pXRef = (IXRefObject *)m_pInterface->CreateInstance(SYSTEM_CLASS_ID, 
        Class_ID(XREFOBJ_CLASS_ID,0));
    TSTR obName = TSTR(pNode->GetName());
    pXRef->Init(filename, obName, pNode->GetObjectRef(), bProxy);
    pNode->SetObjectRef(pXRef);
    // also, set visual queue that we're ignoring anim if we did
    if (bIgnoreAnim)
        pXRef->SetIgnoreAnim(TRUE,FALSE);
}


// Function name	: Xrefutil::DeleteAllAnimation
// Description	    : Self-explanatory...for all anims in the object, simply drop them
// Return type		: void 
// Argument         : ReferenceTarget *ref
void Xrefutil::DeleteAllAnimation(ReferenceTarget *ref)
{
    DeleteAllAnimEnum en(TRUE);
    ref->EnumAnimTree(&en,NULL,0);
}



// Function name	: Xrefutil::AddNewXrefObject
// Description	    : ...please read below...
// Return type		: void 
// Argument         : HWND hWnd
void Xrefutil::AddNewXrefObject(HWND hWnd)
{
    // *** AddNewXrefObject ***
    //
    // Tries to mimic the functionality of MAX's Add New XRef Object
    // 
    // Does the following:
    // 1) Gets a source .MAX filename from user to get the XRef Object from
    // 1.5) Flags current nodes for later
    // 2) Merges in this file, but doesn't display end results (yet)
    // 3) Walks list of file nodes (not flagged), and lets user pick one to XRef
    // 4) Blows away all merged (not flagged) nodes except picked node.  Converts
    //    the chosen node into an XRef node (see ConvertSelectedToXrefObject below)
    //    with default settings
	
    INode * pNode = NULL;
    TSTR filename = "";
    TSTR pickedname = "";
    TSTR * workname = NULL;
	
    INode * pRootNode = m_pInterface->GetRootNode();
    if (!pRootNode) {
        // well, this is actually _really_ bad, but we just exit
        return;
    }
    if (!DoOpenSaveDialog(filename, true)) {
        // either cancel or fail, just return
        return;
    }
	
    // in preparation for merge, flag current scene nodes
    NodeFlagger newFlagger(A_WORK1);
    newFlagger.Enumerate(pRootNode);
	
    // merge in user-picked file into current scene
    // NOTE: We just skip anything in xref'd file that has the same name
    // as an object in the current scene
    if (! m_pInterface->MergeFromFile(filename, TRUE, FALSE, FALSE, MERGE_DUPS_SKIP, NULL)) {
        // error, merge failed
        newFlagger.set_clear(true);
        newFlagger.Enumerate(pRootNode);
        return;
    }
	
    // walk scene and build list of non-flagged nodes
    m_objnamesholder.ZeroCount();
	m_objnamesholder.Shrink();
    UnflaggedNodeNamer newNamer;
    newNamer.m_namelist = &m_objnamesholder;
    newNamer.Enumerate(pRootNode);
    UnflaggedNodeDeleter newDeleter;
	
    // present list of nodes to user, sep. modal dialog
    if (DoPickObjDialog() && m_picknameholder.length() > 0) {
        pNode = m_pInterface->GetINodeByName(m_picknameholder);
        if (pNode) {
            if (m_ignoreanimholder && pNode->IsAnimated()) {
                // if animation is ignored, we basically go through
                // the node and delete all the keys for the node's controllers
                // Note that this won't remove animation for procedural controllers
                DeleteAllAnimation(pNode);
            }
            NodeToXref(pNode, filename, m_proxyholder, m_ignoreanimholder);
            // flag this converted node so we keep it
            pNode->SetAFlag(A_WORK1);
        }
    }
	
    // deleted non-flagged nodes, un-flag original nodes, and return
    newDeleter.Enumerate(pRootNode);
    newFlagger.set_clear(true);
    newFlagger.Enumerate(pRootNode);
    for (int delme = 0; delme < m_objnamesholder.Count(); delme++) { 
        // (clean up TSTRs)
        delete m_objnamesholder[delme];
    }
	
    m_pInterface->RedrawViews(m_pInterface->GetTime());
	
}



// Function name	: Xrefutil::ConvertSelectedToXrefObject
// Description	    : ...please read below...
// Return type		: void 
// Argument         : HWND hWnd
void Xrefutil::ConvertSelectedToXrefObject(HWND hWnd)
{
    // *** ConvertSelectedToXrefObject ***
    //
    // Takes single selection of scene object and
    // "converts" it to XRef Object with default settings.  This is 
    // done via the following steps:
    //
    // 1) Save selected object into new MAX file of user's specification
    // 2) Create a new XRefObject
    // 3) XRefObject makes reference to the objectref of the selected node (OSM-level only)
    // 4) INode object reference changed to XRefObject
    //
    // converting multiple selected objects is left as an exercise
    // but should be quite straight-forward...
	
    INode * pNode = NULL;
    TSTR filename = "";
	
    if (m_pInterface->GetSelNodeCount() != 1) {
        ::MessageBox(hWnd, GetString(IDS_ERR1), ERROR_TITLE, MB_ICONSTOP | MB_OK);
        return;
    }
    pNode = m_pInterface->GetSelNode(0);
    if (!pNode) {
        ::MessageBox(hWnd, GetString(IDS_ERR2), ERROR_TITLE, MB_ICONSTOP | MB_OK);
        return;
    }
	
    if (!DoOpenSaveDialog(filename)) {
        // either cancel or fail, just return
        return;
    }
    m_pInterface->FileSaveSelected(filename);
	
    // One caveat : If the object (not node) has any ReferenceMakers watching it
    // that are expecting geom pipeline msgs, you may need to remove/reset these
    // references.  After the conversion, the original object is effectively
    // hidden from the pipeline.
	
    NodeToXref(pNode, filename, false);
	
    // leave all XRef settings at defaults for this sample
	
    m_pInterface->RedrawViews(m_pInterface->GetTime());
	
}


void Xrefutil::ExportXrefObjects(HWND hWnd)
{
    // *** ExportXrefObjects ***
    //
    // Does a "fake" export of all the XRef objects in the scene.
    // Basically, walks the scene looking for XRef objects, pulls
    // some information from them, and spits this out into a text
    // format, which is then displayed in a modal dialog.
    //
    // In reality, this sort of process would be more applicable
    // to a MAX Exporter plug-in. During SceneExport::DoExport
    // the INodes would be walked, looking for XRef objects, and
    // the exporter could then export whatever was needed from
    // them.
    //
    // The important distinction between XRef scenes and XRef
    // objects is again shown here -- XRef objects, to be manipulated
    // must be found in the scene via ClassID (like any typical
    // scene object), and then treated as IXRefObject pointers.
    // Do NOT use the XRef methods off of INode, since these only
    // apply to xref scenes and only work from the scene root INode.
	
    TSTR * resbuffer = NULL;
	
    // walk the scene starting at the root, looking for xref objects
    INode * pRootNode = m_pInterface->GetRootNode();
    if (!pRootNode) {
        // well, this is actually _really_ bad, but we just exit
        return;
    }
    resbuffer = new TSTR; 
    if (!resbuffer) return;
	
    XRefObjFinder newfinder;
    newfinder.m_buffer = resbuffer;
    // (see XRefObjFinder::Proc above for details)
    newfinder.Enumerate(pRootNode);
	
    // finally, display results in dialog
    int dlgres = DialogBoxParam(hInstance, 
		MAKEINTRESOURCE(IDD_DLGEXPORT),
		m_hPanel,
		ExportDlgProc,
		(LPARAM)resbuffer);
	
    delete resbuffer;
}


////////////////////////////////////////////////////////////////////////////////
//////////  IObjXRefManager methods
////////////////////////////////////////////////////////////////////////////////



// Function name	: Xrefutil::AddNewXrefScene_IXROMGR
// Description	    : The purpose of this method is to provide a show of how to 
//						bring in xref objects by way of the IXRefObjectManager
//						interface.  As pertaining to this sample, the 
//						functionality it provides replaces most of the scene
//						scene enumeration and ::NodeToXref() call.
// Return type		: void 
// Argument         : HWND hWnd
void Xrefutil::AddNewXrefScene_IXROMGR(HWND hWnd)
{
	IXRefObject *pXRef;  // a temporary XRef object to handle the incoming one
	
	//  This is just a copy of the same variables from ::AddNewXrefObject
	INode * pNode = NULL;
    TSTR filename = "";
    TSTR pickedname = "";
    TSTR * workname = NULL;
	
	
	// getting the interface pointer
	m_pIobjrefmgr = static_cast<IObjXRefManager *>(GetCOREInterface(OBJXREFMANAGER_INTERFACE));
	
	//  if we don't get a good pointer to an IObjXRefManager _IS_REALLY_ bad!
	//  I am opting to just leave the method. 
	if(!m_pIobjrefmgr) {
		return;
	}
	
	// ********************************************************************************
	// ********** BELOW THIS LINE EVERYTHING IS COPIED FROM ::AddNewXrefObject ********
	// ********************************************************************************
	
    INode * pRootNode = m_pInterface->GetRootNode();
    if (!pRootNode) {
        // well, this is actually _really_ bad, but we just exit
        return;
    }
    if (!DoOpenSaveDialog(filename, true)) {
        // either cancel or fail, just return
        return;
    }
	
	// in preparation for merge, flag current scene nodes
    NodeFlagger newFlagger(A_WORK1);
    newFlagger.Enumerate(pRootNode);
	
	// merge in user-picked file into current scene
	// NOTE: We just skip anything in xref'd file that has the same name
	// as an object in the current scene
    if (! m_pInterface->MergeFromFile(filename, TRUE, FALSE, FALSE, MERGE_DUPS_SKIP, NULL)) {
        // error, merge failed
        newFlagger.set_clear(true);
        newFlagger.Enumerate(pRootNode);
        return;
    }
	
    // walk scene and build list of non-flagged nodes
    m_objnamesholder.ZeroCount();
	m_objnamesholder.Shrink();
    UnflaggedNodeNamer newNamer;
    newNamer.m_namelist = &m_objnamesholder;
    newNamer.Enumerate(pRootNode);
    UnflaggedNodeDeleter newDeleter;
	
    // present list of nodes to user, sep. modal dialog
    if (DoPickObjDialog() && m_picknameholder.length() > 0) {
        pNode = m_pInterface->GetINodeByName(m_picknameholder);
        if (pNode) {
            if (m_ignoreanimholder && pNode->IsAnimated()) {
                // if animation is ignored, we basically go through
                // the node and delete all the keys for the node's controllers
                // Note that this won't remove animation for procedural controllers
                DeleteAllAnimation(pNode);
            }
			// Woohoo!  we get to use the IXRefObjectManager interface
			pXRef = m_pIobjrefmgr->AddXRefObject(filename, pNode->GetName(), m_proxyholder);
			pNode->SetObjectRef(pXRef);
			
			// also, set visual queue that we're ignoring anim if we did
			if (m_ignoreanimholder)
				pXRef->SetIgnoreAnim(TRUE,FALSE);
			// flag this converted node so we keep it
			pNode->SetAFlag(A_WORK1);
        }
    }
	
    // deleted non-flagged nodes, un-flag original nodes, and return
    newDeleter.Enumerate(pRootNode);
    newFlagger.set_clear(true);
    newFlagger.Enumerate(pRootNode);
    for (int delme = 0; delme < m_objnamesholder.Count(); delme++) { 
        // (clean up TSTRs)
        delete m_objnamesholder[delme];
    }
	
    m_pInterface->RedrawViews(m_pInterface->GetTime());
	
	// ********************************************************************************
	// ********** ABOVE THIS LINE EVERYTHING IS COPIED FROM ::AddNewXrefObject ********
	// ********************************************************************************
}


// Function name	: Xrefutil::RefreshXRefObject_IXROMGR
// Description	    : 
// Return type		: void 
// Argument         : HWND hWnd
void Xrefutil::RefreshXRefObject_IXROMGR(HWND hWnd)
{
	
}


// Function name	: Xrefutil::GetAllXRefsObjects_IXROMGR
// Description	    : 
// Return type		: void 
// Argument         : HWND hWnd
void Xrefutil::GetAllXRefsObjects_IXROMGR(HWND hWnd)
{
	Tab<IXRefObject*> pXRef;  // a temporary XRef object to handle the incoming one
	
	// ********************************************************************************
	// ********** BELOW THIS LINE EVERYTHING IS COPIED FROM ::AddNewXrefObject ********
	// ********************************************************************************
	
	//  This is just a copy of the same variables from ::AddNewXrefObject
	INode * pNode = NULL;
    TSTR filename = "";
    TSTR pickedname = "";
    TSTR * workname = NULL;
	char buffer[8];	
	
	// getting the interface pointer
	m_pIobjrefmgr = static_cast<IObjXRefManager *>(GetCOREInterface(OBJXREFMANAGER_INTERFACE));
	
	//  if we don't get a good pointer to an IObjXRefManager _IS_REALLY_ bad!
	//  I am opting to just leave the method. 
	if(!m_pIobjrefmgr) {
		return;
	}
	
    INode * pRootNode = m_pInterface->GetRootNode();
    if (!pRootNode) {
        // well, this is actually _really_ bad, but we just exit
        return;
    }
    if (!DoOpenSaveDialog(filename, true)) {
        // either cancel or fail, just return
        return;
    }
	
	m_pInterface->MergeFromFile(filename, TRUE, FALSE, FALSE, MERGE_DUPS_SKIP, NULL);
	
	// Woohoo!  we get to use the IXRefObjectManager interface
	
	int num = m_pIobjrefmgr->GetNumXRefObjects(filename);
	MessageBox(NULL,itoa(num, buffer, 10),itoa(num, buffer, 10),MB_OK);
	
	m_pIobjrefmgr->GetAllXRefObjects(pXRef);
	
	
	
    m_pInterface->RedrawViews(m_pInterface->GetTime());
	
	// ********************************************************************************
	// ********** ABOVE THIS LINE EVERYTHING IS COPIED FROM ::AddNewXrefObject ********
	// ********************************************************************************
}
