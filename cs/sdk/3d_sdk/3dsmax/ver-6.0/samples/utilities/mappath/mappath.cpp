/**********************************************************************
 *<
	FILE..........: MapPath.cpp

	DESCRIPTION...: Bitmap Path Editor

	CREATED BY....: Christer Janson - Kinetix

	HISTORY.......: Created Thursday, October 16, 1997

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "MapPath.h"
#include "DataIndex.h"
#include <list.h>
#include <iassembly.h>
#include <iassemblymgr.h>

HINSTANCE		hInstance;
int				controlsInit = FALSE;
static TCHAR*	useFolder;

#define DX_RENDER_PARAMBLOCK 2
#define DX_RENDERBITMAP_PARAMID 2


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
			
	return (TRUE);
}


__declspec( dllexport ) const TCHAR* LibDescription()
{
	return _T("Map path editor utility (Discreet)");
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetRefCheckDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

static RefCheck theRefCheck;

class RefCheckClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theRefCheck;}
	const TCHAR *	ClassName() {return GetString(IDS_MAPPATH_EDITOR);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return REFCHECK_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
};

static RefCheckClassDesc RefCheckDesc;
ClassDesc* GetRefCheckDesc() {return &RefCheckDesc;}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


static INT_PTR CALLBACK RefCheckDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theRefCheck.Init(hWnd);
			break;

		case WM_DESTROY:
			theRefCheck.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CLOSEBUTTON:
					theRefCheck.iu->CloseUtility();
					break;
				case IDC_CHECKDEP:
					theRefCheck.DoDialog();
					break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theRefCheck.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}	

RefCheck::RefCheck()
{
	iu		= NULL;
	ip		= NULL;	
	hPanel	= NULL;
	hDialog	= NULL;
	//pDib	= NULL;
}

RefCheck::~RefCheck()
{
	if (hDialog) {
		DestroyWindow(hDialog);
	}
}

// The dialog procedure for the modeless dialogbox
INT_PTR CALLBACK dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RefCheck* util = (RefCheck*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
				RECT rect;
				util = (RefCheck*)lParam;
				util->hDialog = hWnd;
				SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
				CenterWindow(hWnd, util->ip->GetMAXHWnd());
				util->CheckDependencies();
				util->EnableEntry(hWnd, FALSE, 0);

				GetWindowRect(hWnd, &rect);
				util->SetMinDialogSize((rect.right - rect.left)*2/3, rect.bottom - rect.top);
				SendMessage(hWnd, WM_SETICON, ICON_SMALL, GetClassLong(util->ip->GetMAXHWnd(), GCLP_HICONSM));
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
				case IDOK:
					//DestroyWindow(hWnd);
					EndDialog(hWnd, 1);
					break;
				case IDC_SETPATH:
					theRefCheck.Update();
					break;
				case IDC_STRIP_SELECTED:
					theRefCheck.StripSelected();
					break;
				case IDC_STRIP_ALL:
					theRefCheck.StripAll();
					break;
				case IDC_INFO:
					theRefCheck.ShowInfo();
					break;
				case IDC_COPYMAPS:
					theRefCheck.CopyMaps();
					break;
				case IDC_MISSING:
					theRefCheck.SelectMissing();
					break;
				case IDC_ACTUALPATH:
					theRefCheck.SetActualPath();
					break;
				case IDC_DEPLIST:
					if (HIWORD(wParam) == LBN_DBLCLK) {
						theRefCheck.ShowInfo();
					}
					else if (HIWORD(wParam) == LBN_SELCHANGE) {
						theRefCheck.DoSelection();
					}
					break;
				case IDC_BROWSE:
					theRefCheck.BrowseDirectory();
					break;
				case IDC_PATHEDIT:
					// If we get a setfocus message, disable accelerators
					if(HIWORD(wParam) == EN_SETFOCUS)
						DisableAccelerators();
					// Otherwise enable them again
					else if(HIWORD(wParam) == EN_KILLFOCUS) {
						EnableAccelerators();
					}
					break;
			}
			break;
		case WM_CLOSE:
			//DestroyWindow(hWnd);
			EndDialog(hWnd, 1);
			break;
		case WM_DESTROY:
			util->hDialog = NULL;
			break;
		case WM_WINDOWPOSCHANGING:
			{
				if(IsIconic(hWnd)) {
					return FALSE;
				}

				// prevent the window from stretching horizontally
				WINDOWPOS *wp = (WINDOWPOS*)lParam;
				if (wp->cx < util->GetMinDialogWidth()) {
					wp->cx = util->GetMinDialogWidth();
				}
				if (wp->cy < util->GetMinDialogHeight()) {
					wp->cy = util->GetMinDialogHeight();
				}

				break;
			}
		case WM_SIZE:
			util->ResizeWindow(LOWORD(lParam), HIWORD(lParam));
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

void RefCheck::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;

	

	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		RefCheckDlgProc,
		GetString(IDS_PANELTITLE),
		0);
}
	
void RefCheck::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	
	DataIndex::Instance()->Reinitialize();
	distributionLister.fileList.ZeroCount();

	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void RefCheck::Init(HWND hWnd)
{
	CheckDlgButton(hWnd, IDC_INCLUDE_MEDIT, BST_CHECKED);
}

void RefCheck::Destroy(HWND hWnd)
{
}

void RefCheck::DoDialog()
{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_MAINDIALOG),
		ip->GetMAXHWnd(),
		dlgProc,
		(LPARAM)this);
}

// My own version of ScreenToClient (taking Rect instead of Point)
void ScreenToClient(HWND hWnd, RECT* rect)
{
	POINT pt;
	pt.x = rect->left;
	pt.y = rect->top;
	ScreenToClient(hWnd, &pt);
	rect->left = pt.x;
	rect->top = pt.y;
	pt.x = rect->right;
	pt.y = rect->bottom;
	ScreenToClient(hWnd, &pt);
	rect->right = pt.x;
	rect->bottom = pt.y;
}

// Resize the window
void RefCheck::ResizeWindow(int x, int y)
{
	HWND	pCtrl;
	RECT	lbRect;
	RECT	prevCtrlRect;
	int		offset;
	BOOL	bRepaint = FALSE;

	// Move the OK button
	pCtrl = GetDlgItem(hDialog, IDCANCEL);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	offset = lbRect.right - lbRect.left;
	lbRect.left = x-offset - 10;
	lbRect.right = lbRect.left + offset;
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);
	prevCtrlRect = lbRect;

	// Move the Info button
	pCtrl = GetDlgItem(hDialog, IDC_INFO);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	offset = lbRect.bottom - lbRect.top;
	lbRect.top = prevCtrlRect.top+28;
	lbRect.bottom = lbRect.top + offset;
	lbRect.right = prevCtrlRect.right;
	lbRect.left = prevCtrlRect.left;
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);
	prevCtrlRect = lbRect;

	// Move the CopyMaps button
	pCtrl = GetDlgItem(hDialog, IDC_COPYMAPS);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	offset = lbRect.bottom - lbRect.top;
	lbRect.top = prevCtrlRect.top+41;
	lbRect.bottom = lbRect.top + offset;
	lbRect.right = prevCtrlRect.right;
	lbRect.left = prevCtrlRect.left;
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);
	prevCtrlRect = lbRect;

	// Path editbox
	pCtrl = GetDlgItem(hDialog, IDC_PATHEDIT);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	offset = lbRect.bottom - lbRect.top;
	lbRect.top = y - offset - 6;
	lbRect.bottom = lbRect.top + offset;
	lbRect.right = prevCtrlRect.left - 35;
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);
	prevCtrlRect = lbRect;

	// Browse button
	pCtrl = GetDlgItem(hDialog, IDC_BROWSE);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	offset = lbRect.bottom - lbRect.top;
	lbRect.top = prevCtrlRect.top;
	lbRect.bottom = lbRect.top + offset;
	lbRect.left = prevCtrlRect.right + 3;
	lbRect.right = prevCtrlRect.right + 30;
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);


	// Path title text
	pCtrl = GetDlgItem(hDialog, IDC_PATHTITLE);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	offset = lbRect.bottom - lbRect.top;
	lbRect.top = prevCtrlRect.top+4;
	lbRect.bottom = lbRect.top + offset;
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);

	// Resize the listbox
	pCtrl = GetDlgItem(hDialog, IDC_DEPLIST);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	lbRect.bottom = prevCtrlRect.top - 3;
	lbRect.right = prevCtrlRect.right+30;
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);

	// Load the Close btn rect
	GetWindowRect(GetDlgItem(hDialog, IDCANCEL), &prevCtrlRect);
	ScreenToClient(hDialog, &prevCtrlRect);

	// SetMap button
	pCtrl = GetDlgItem(hDialog, IDC_SETPATH);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	lbRect.right = prevCtrlRect.right;
	lbRect.left = prevCtrlRect.left;
	// Load the edit box rect
	GetWindowRect(GetDlgItem(hDialog, IDC_PATHEDIT), &prevCtrlRect);
	ScreenToClient(hDialog, &prevCtrlRect);
	lbRect.top = prevCtrlRect.top;
	lbRect.bottom = prevCtrlRect.bottom;
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);
	prevCtrlRect = lbRect;

	// Strip all button
	pCtrl = GetDlgItem(hDialog, IDC_STRIP_ALL);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	lbRect.right = prevCtrlRect.right;
	lbRect.left = prevCtrlRect.left;
	offset = lbRect.bottom - lbRect.top;
	lbRect.bottom = prevCtrlRect.top - 4;
	lbRect.top = lbRect.bottom - offset;	
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);
	prevCtrlRect = lbRect;

	// Strip selected button
	pCtrl = GetDlgItem(hDialog, IDC_STRIP_SELECTED);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	lbRect.right = prevCtrlRect.right;
	lbRect.left = prevCtrlRect.left;
	offset = lbRect.bottom - lbRect.top;
	lbRect.bottom = prevCtrlRect.top - 4;
	lbRect.top = lbRect.bottom - offset;	
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);
	prevCtrlRect = lbRect;

	// Set Actual Path
	pCtrl = GetDlgItem(hDialog, IDC_ACTUALPATH);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	lbRect.right = prevCtrlRect.right;
	lbRect.left = prevCtrlRect.left;
	offset = lbRect.bottom - lbRect.top;
	lbRect.bottom = prevCtrlRect.top - 4;
	lbRect.top = lbRect.bottom - offset;	
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);
	prevCtrlRect = lbRect;

	// Missing Maps
	pCtrl = GetDlgItem(hDialog, IDC_MISSING);
	GetWindowRect(pCtrl, &lbRect);
	ScreenToClient(hDialog, &lbRect);
	lbRect.right = prevCtrlRect.right;
	lbRect.left = prevCtrlRect.left;
	offset = lbRect.bottom - lbRect.top;
	lbRect.bottom = prevCtrlRect.top - 4;
	lbRect.top = lbRect.bottom - offset;	
	MoveWindow(pCtrl, lbRect.left, lbRect.top, lbRect.right-lbRect.left, lbRect.bottom-lbRect.top, bRepaint);


	InvalidateRect(hDialog, NULL, TRUE);
}

class SceneAPIFindSceneProc : public DependentEnumProc {
	public:
		SceneAPIFindSceneProc(ReferenceMaker** anim) {
			scene = anim;
			*scene = NULL;
		}
		int proc(ReferenceMaker *ref) {
			switch (ref->SuperClassID()) {
				case REF_MAKER_CLASS_ID:
					if (ref->ClassID() == Class_ID(THE_SCENE_CLASS_ID, 0)) {
						*scene = ref;
					}
					break;
			}
			return 0;
		}
	private:
		ReferenceMaker** scene;
};

#define DXMATERIAL_DYNAMIC_UI Class_ID(0xef12512, 0x11351ed1)

bool IsDynamicDxMaterial(MtlBase * newMtl)
{

	DllDir * lookup = GetCOREInterface()->GetDllDirectory();
	ClassDirectory & dirLookup = lookup->ClassDir();

	ClassDesc * cd = dirLookup.FindClass(MATERIAL_CLASS_ID,newMtl->ClassID());
	if(cd && cd->SubClassID() == DXMATERIAL_DYNAMIC_UI)
		return true;
	else
		return false;


}

class ResMapEnum : public AnimEnum {
	public:	
		ResMapEnum(HWND dlg, Animatable* medit,RefCheck* rc) : AnimEnum(SCOPE_ALL)
		{
			hDialog = dlg;
			mtledit = medit;
			ref = rc;
		}
		int proc(Animatable *anim, Animatable *client, int subNum)
		{
			if (anim == mtledit) {
				return ANIM_ENUM_STOP;
			}
			else if (anim->SuperClassID() == TEXMAP_CLASS_ID) {
				if (anim->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
					BitmapTex* bmt = (BitmapTex*)anim;
					AddTexmap(bmt);
				}
			}
			else if(anim->SuperClassID()==MATERIAL_CLASS_ID)
			{
				if(IsDynamicDxMaterial((MtlBase *)anim))
				{
					MtlBase * mtl = (MtlBase*)anim;
					AddDxMtl(mtl);

				}
			}
				
			return ANIM_ENUM_PROCEED;
			
		}

		BOOL AddDxMtl(MtlBase * mtl)
		{
			int numItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);
			for (int i=0; i < numItems; i++) {
				DWORD_PTR ptr = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
				if (ptr == (DWORD_PTR)mtl) {
					return FALSE;
				}
			}
			IDxMaterial * idxm = (IDxMaterial*)mtl->GetInterface(IDXMATERIAL_INTERFACE);
			int bitmapCount = idxm->GetNumberOfEffectBitmaps();
			for(int j=0; j<bitmapCount;j++)
			{
				PBBitmap* bmap = idxm->GetEffectBitmap(j);

				int idx = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_ADDSTRING, 0, (LPARAM)bmap->bi.Name());
				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SETITEMDATA, idx, (LPARAM)mtl);
				// add an entry in the data type index
				DataIndex::Instance()->Store(idx, DataIndex::DXMATERIAL);

			}
			int idx = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_ADDSTRING, 0, (LPARAM)idxm->GetEffectFilename());
			SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SETITEMDATA, idx, (LPARAM)mtl);
			DataIndex::Instance()->Store(idx, DataIndex::DXMATERIAL);

			PBBitmap * bm;
			//the software override if 2 and the bitmap is 2...
//			mtl->GetParamBlock(DX_RENDER_PARAMBLOCK)->GetValue(DX_RENDERBITMAP_PARAMID,0,bm,FOREVER);
			bm = idxm->GetSoftwareRenderBitmap();
			if(bm)
			{
				int idx = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_ADDSTRING, 0, (LPARAM)bm->bi.Name());
				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SETITEMDATA, idx, (LPARAM)mtl);
				DataIndex::Instance()->Store(idx, DataIndex::DXMATERIAL);

			}
			return TRUE;

		}

		BOOL AddTexmap(BitmapTex* bmt)
		{
			int numItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);
			for (int i=0; i < numItems; i++) {
				DWORD_PTR ptr = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
				if (ptr == (DWORD_PTR)bmt) {
					return FALSE;
				}
			}

			int idx = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_ADDSTRING, 0, (LPARAM)bmt->GetMapName());
			SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SETITEMDATA, idx, (LPARAM)bmt);
			// add an entry in the data type index
			DataIndex::Instance()->Store(idx, DataIndex::TEXMAP);

			return TRUE;
		}
	private:
	HWND hDialog;
	Animatable* mtledit;
	RefCheck * ref;
};


void RefCheck::CheckDependencies()
{
	ReferenceMaker* scene;

	//reinitialize
	DataIndex::Instance()->Reinitialize();
	distributionLister.fileList.ZeroCount();

	scene = NULL;
	SceneAPIFindSceneProc dep(&scene);
	ip->GetRootNode()->EnumDependents(&dep);

	int numItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);
	int numSel = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSELCOUNT, 0, 0);
	int* selArray = new int[numSel];
	SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSELITEMS, numSel, (LPARAM)selArray);

	SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_RESETCONTENT, 0, 0);

	Animatable* pMEdit = NULL;
	if (!GetIncludeMedit()) {
		// Yikes!
		pMEdit = scene->SubAnim(7);
	}

	ResMapEnum resMapEnum(hDialog, pMEdit,this);
	scene->EnumAnimTree(&resMapEnum,NULL,0);

	//find lighting distributions
	
	EnumerateNodes(ip->GetRootNode());

	for(int i=0;i<distributionLister.fileList.Count(); i++)	{
		int idx = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_ADDSTRING, 0, (LPARAM)distributionLister.fileList[i]);
		// add an entry in the data type index
		DataIndex::Instance()->Store(idx, DataIndex::PHOTOMETRIC);
	}

	if (GetIncludeMatLib()) {
		MtlBaseLib matlib = ip->GetMaterialLibrary();
		matlib.EnumAnimTree(&resMapEnum,NULL,0);
	}

	if (numItems == SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0)) {
		for (int i=0; i<numSel; i++) {
			SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SETSEL, TRUE, selArray[i]);
		}
	}
	delete [] selArray;

	numSel = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSELCOUNT, 0, 0);
	if (numSel) {
		EnableEntry(hDialog, TRUE, numSel);
	}
	else {
		EnableEntry(hDialog, FALSE, numSel);
	}
}



void RefCheck::Update()
{
	BitmapTex*	bmTex = NULL;
	TCHAR		newPath[MAX_PATH];

	int numSel		= SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSELCOUNT, 0, 0);
	int numEntries	= SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);

	SendMessage(GetDlgItem(hDialog, IDC_PATHEDIT), WM_GETTEXT, MAX_PATH, (LPARAM)newPath);

	if (_tcslen(newPath) == 0) {
		return;
	}

	if (numSel == 1) {
		int idx = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCURSEL, 0, 0);
		if (idx != -1) {
			if(DataIndex::Instance()->Get(idx) == DataIndex::TEXMAP)	{
				bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, idx, 0);

				if (bmTex) {
					SetPath(newPath, bmTex);
					bmTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					SendMessage(GetDlgItem(hDialog, IDC_PATHEDIT), WM_SETTEXT, 0, (LPARAM)"");
				}
			}
			else if(DataIndex::Instance()->Get(idx) == DataIndex::PHOTOMETRIC)	{
				TCHAR entry[MAX_PATH];
				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, idx, (LPARAM) entry);
				TCHAR	newName[MAX_PATH];
				TCHAR	drive[_MAX_DRIVE];
				TCHAR	dir[_MAX_DIR];
				TCHAR	fname[_MAX_FNAME];
				TCHAR	ext[_MAX_EXT];
				_tsplitpath(entry, drive, dir, fname, ext);
				_tcscpy(newName, newPath);
				_tcscat(newName, fname);
				_tcscat(newName, ext);
				DataIndex::Instance()->SetPathOnLights(entry, newName);
			}
			else if(DataIndex::Instance()->Get(idx) == DataIndex::DXMATERIAL)	{

				MtlBase * mtl;
				mtl = (MtlBase*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, idx, 0);
				TCHAR entry[MAX_PATH];
				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, idx, (LPARAM) entry);
				TCHAR	newName[MAX_PATH];
				TCHAR	drive[_MAX_DRIVE];
				TCHAR	dir[_MAX_DIR];
				TCHAR	fname[_MAX_FNAME];
				TCHAR	ext[_MAX_EXT];
				_tsplitpath(entry, drive, dir, fname, ext);
				_tcscpy(newName, newPath);
				_tcscat(newName, fname);
				_tcscat(newName, ext);

				SetDxPath(mtl,entry,newName,ext);

			}

		}
	}
	else {
		for (int idx=0; idx < numEntries; idx++) {
			if (SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSEL, idx, 0) > 0) {
				if(DataIndex::Instance()->Get(idx) == DataIndex::TEXMAP)	{
					bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, idx, 0);
					if (bmTex) {
						SetPath(newPath, bmTex);
						bmTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
						
					}
				}
				else if(DataIndex::Instance()->Get(idx) == DataIndex::PHOTOMETRIC)	{
					TCHAR entry[MAX_PATH];
					SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, idx, (LPARAM) entry);
					TCHAR	newName[MAX_PATH];
					TCHAR	drive[_MAX_DRIVE];
					TCHAR	dir[_MAX_DIR];
					TCHAR	fname[_MAX_FNAME];
					TCHAR	ext[_MAX_EXT];
					_tsplitpath(entry, drive, dir, fname, ext);
					_tcscpy(newName, newPath);
					_tcscat(newName, fname);
					_tcscat(newName, ext);
					DataIndex::Instance()->SetPathOnLights(entry, newPath);
				}
				else if(DataIndex::Instance()->Get(idx) == DataIndex::DXMATERIAL)	{
					TCHAR entry[MAX_PATH];
					MtlBase * mtl;
					mtl = (MtlBase*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, idx, 0);
					SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, idx, (LPARAM) entry);
					TCHAR	newName[MAX_PATH];
					TCHAR	drive[_MAX_DRIVE];
					TCHAR	dir[_MAX_DIR];
					TCHAR	fname[_MAX_FNAME];
					TCHAR	ext[_MAX_EXT];
					_tsplitpath(entry, drive, dir, fname, ext);
					_tcscpy(newName, newPath);
					_tcscat(newName, fname);
					_tcscat(newName, ext);
					SetDxPath(mtl,entry,newName,ext);
				}
			}
		}
		SendMessage(GetDlgItem(hDialog, IDC_PATHEDIT), WM_SETTEXT, 0, (LPARAM)"");
	}

	CheckDependencies();
}

void RefCheck::StripSelected()
{
	TSTR szTitle = GetString(IDS_STRIPTITLE);
	if (MessageBox(hDialog, GetString(IDS_STRIPWARNING), szTitle, MB_OKCANCEL | MB_ICONWARNING) != IDOK) {
		return;
	}

	int numItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);
	for (int i=0; i < numItems; i++) {
		if (SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSEL, i, 0) > 0) {
			if(DataIndex::Instance()->Get(i) == DataIndex::TEXMAP)	{
				BitmapTex* bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
				if (bmTex) {
					TCHAR* texName = bmTex->GetMapName();
					if (texName && _tcscmp(texName, _T(""))) {
						TCHAR	newName[MAX_PATH];
						TCHAR	drive[_MAX_DRIVE];
						TCHAR	dir[_MAX_DIR];
						TCHAR	fname[_MAX_FNAME];
						TCHAR	ext[_MAX_EXT];
						_tsplitpath(texName, drive, dir, fname, ext);
						_tcscpy(newName, fname);
						_tcscat(newName, ext);
						bmTex->SetMapName(newName);
						bmTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					}
				}
			}
			else if(DataIndex::Instance()->Get(i) == DataIndex::PHOTOMETRIC)	{
				TCHAR distName[MAX_PATH];
				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM)distName);
				if (distName && _tcscmp(distName, _T(""))) {
					TCHAR	newName[MAX_PATH];
					TCHAR	drive[_MAX_DRIVE];
					TCHAR	dir[_MAX_DIR];
					TCHAR	fname[_MAX_FNAME];
					TCHAR	ext[_MAX_EXT];
					_tsplitpath(distName, drive, dir, fname, ext);
					_tcscpy(newName, fname);
					_tcscat(newName, ext);
					DataIndex::Instance()->SetPathOnLights(distName, newName);
				}
			}
			else if(DataIndex::Instance()->Get(i) == DataIndex::DXMATERIAL)	{
				TCHAR distName[MAX_PATH];
				MtlBase * mtl = (MtlBase*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
				if (mtl) {

					SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM)distName);
					if (distName && _tcscmp(distName, _T(""))) {
						TCHAR	newName[MAX_PATH];
						TCHAR	drive[_MAX_DRIVE];
						TCHAR	dir[_MAX_DIR];
						TCHAR	fname[_MAX_FNAME];
						TCHAR	ext[_MAX_EXT];
						_tsplitpath(distName, drive, dir, fname, ext);
						_tcscpy(newName, fname);
						_tcscat(newName, ext);
						SetDxPath(mtl,distName,newName,ext);
					}
				}
			}
				
		}
	}

	CheckDependencies();
}

void RefCheck::StripAll()
{
	TSTR szTitle = GetString(IDS_STRIPTITLE);
	if (MessageBox(hDialog, GetString(IDS_STRIP_ALL_WARNING), szTitle, MB_OKCANCEL | MB_ICONWARNING) != IDOK) {
		return;
	}

	int numItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);
	for (int i=0; i < numItems; i++) {
		if(DataIndex::Instance()->Get(i) == DataIndex::TEXMAP)	{
			BitmapTex* bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
			if (bmTex) {
				TCHAR* texName = bmTex->GetMapName();
				if (texName && _tcscmp(texName, _T(""))) {
					TCHAR	newName[MAX_PATH];
					TCHAR	drive[_MAX_DRIVE];
					TCHAR	dir[_MAX_DIR];
					TCHAR	fname[_MAX_FNAME];
					TCHAR	ext[_MAX_EXT];
					_tsplitpath(texName, drive, dir, fname, ext);
					_tcscpy(newName, fname);
					_tcscat(newName, ext);
					bmTex->SetMapName(newName);
					bmTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				}
			}
		}
		else if(DataIndex::Instance()->Get(i) == DataIndex::PHOTOMETRIC)	{
			TCHAR distName[MAX_PATH];
			SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM)distName);
			if (distName && _tcscmp(distName, _T(""))) {
				TCHAR	newName[MAX_PATH];
				TCHAR	drive[_MAX_DRIVE];
				TCHAR	dir[_MAX_DIR];
				TCHAR	fname[_MAX_FNAME];
				TCHAR	ext[_MAX_EXT];
				_tsplitpath(distName, drive, dir, fname, ext);
				_tcscpy(newName, fname);
				_tcscat(newName, ext);
				DataIndex::Instance()->SetPathOnLights(distName, newName);
			}
		}
		else if(DataIndex::Instance()->Get(i) == DataIndex::DXMATERIAL)	{
			TCHAR distName[MAX_PATH];
			MtlBase * mtl = (MtlBase*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
			if (mtl) {

				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM)distName);
				if (distName && _tcscmp(distName, _T(""))) {
					TCHAR	newName[MAX_PATH];
					TCHAR	drive[_MAX_DRIVE];
					TCHAR	dir[_MAX_DIR];
					TCHAR	fname[_MAX_FNAME];
					TCHAR	ext[_MAX_EXT];
					_tsplitpath(distName, drive, dir, fname, ext);
					_tcscpy(newName, fname);
					_tcscat(newName, ext);
					SetDxPath(mtl,distName,newName,ext);
				}
			}
		}
	}

	CheckDependencies();
}

void RefCheck::DoSelection()
{
	TCHAR	bmName[MAX_PATH];

	int numSel		= SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSELCOUNT, 0, 0);
	int numEntries	= SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);

	if (numSel == 0) {
		EnableEntry(hDialog, FALSE, 0);
		SendMessage(GetDlgItem(hDialog, IDC_PATHEDIT), WM_SETTEXT, 0, (LPARAM)"");
	}
	else if (numSel == 1) {
		EnableEntry(hDialog, TRUE, 1);
		int idx = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCURSEL, 0, 0);
		if (idx != -1) {
			if(DataIndex::Instance()->Get(idx) == DataIndex::TEXMAP)	{
				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETTEXT, idx, (LPARAM)bmName);
				StripMapName(bmName);
				SendMessage(GetDlgItem(hDialog, IDC_PATHEDIT), WM_SETTEXT, 0, (LPARAM)bmName);
				BitmapTex* bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, idx, 0);
				if (bmTex) {
					SendMessage(GetDlgItem(hDialog, IDC_MAPNAME), WM_SETTEXT, 0, (LPARAM)(char*)bmTex->GetName());
				}
			}
		}
	}
	else {
		EnableEntry(hDialog, TRUE, numSel);
		SendMessage(GetDlgItem(hDialog, IDC_PATHEDIT), WM_SETTEXT, 0, (LPARAM)"");
	}
}

void RefCheck::SetPath(TCHAR* path, BitmapTex* map)
{
	TCHAR*	texName = map->GetMapName();

	if (_tcscmp(texName, _T("")) == 0) {
		return;
		}

	TCHAR	newName[MAX_PATH];
	TCHAR	drive[_MAX_DRIVE];
	TCHAR	dir[_MAX_DIR];
	TCHAR	fname[_MAX_FNAME];
	TCHAR	ext[_MAX_EXT];
	_tsplitpath(texName, drive, dir, fname, ext);

	_tcscpy(newName, path);
	if (path[_tcslen(path)-1] != _T('\\')) {
		_tcscat(newName, _T("\\"));
	}
	_tcscat(newName, fname);
	_tcscat(newName, ext);

	map->SetMapName(newName);
}

void RefCheck::SetDxPath(MtlBase * mtl, TCHAR * path, TCHAR * newPath, TCHAR * ext)
{
	IDxMaterial * idxm = (IDxMaterial*)mtl->GetInterface(IDXMATERIAL_INTERFACE);

	bool found = false;

	if(!_tcscmp(ext,_T(".fx")))
	{
		TSTR name = TSTR(newPath);
        idxm->SetEffectFilename(name);
		found = true;
	}
	else
	{
		int bitmapCount = idxm->GetNumberOfEffectBitmaps();
		for(int j=0; j<bitmapCount;j++)
		{
			PBBitmap* bmap = idxm->GetEffectBitmap(j);

			if(!_tcscmp(path, bmap->bi.Name()))
			{
				PBBitmap newBitmap;
				TSTR newFile = TSTR(newPath);
				newBitmap.bi.SetName(newFile);
				idxm->SetEffectBitmap(j,&newBitmap);
				found = true;
			}
		}
	}

	//try the software override bitmap
	if(!found)
	{
		PBBitmap * bmap = NULL;
//		mtl->GetParamBlock(DX_RENDER_PARAMBLOCK)->GetValue(DX_RENDERBITMAP_PARAMID,0,bmap,FOREVER);
		bmap = idxm->GetSoftwareRenderBitmap();

		if(bmap && !_tcscmp(path, bmap->bi.Name()))
		{
			PBBitmap newBitmap;
			TSTR newFile = TSTR(newPath);
			newBitmap.bi.SetName(newFile);
//          mtl->GetParamBlock(DX_RENDER_PARAMBLOCK)->SetValue(DX_RENDERBITMAP_PARAMID,0,&newBitmap);
			idxm->SetSoftwareRenderBitmap(&newBitmap);

		}
	}
}

void RefCheck::StripMapName(TCHAR* path)
{
	TCHAR	drive[_MAX_DRIVE];
	TCHAR	dir[_MAX_DIR];
	TCHAR	fname[_MAX_FNAME];
	TCHAR	ext[_MAX_EXT];
	_tsplitpath(path, drive, dir, fname, ext);

	_tcscpy(path, drive);
	_tcscat(path, dir);
}

void RefCheck::EnableEntry(HWND hWnd, BOOL bEnable, int numSel)
{
	EnableWindow(GetDlgItem(hWnd, IDC_PATHEDIT), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_BROWSE), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_STRIP_SELECTED), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_SETPATH), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_COPYMAPS), bEnable);
	EnableWindow(GetDlgItem(hWnd, IDC_ACTUALPATH), bEnable);

	// Only enable this is one single entry is selected
	if (bEnable && numSel ==1) {
		EnableWindow(GetDlgItem(hWnd, IDC_INFO), TRUE);
	}
	else {
		EnableWindow(GetDlgItem(hWnd, IDC_INFO), FALSE);
	}

	EnableWindow(GetDlgItem(hWnd, IDC_MISSING), TRUE);
}

void RefCheck::BrowseDirectory()
{
	TCHAR newPath[MAX_PATH];

	if (ChooseDir(GetString(IDS_NEWPATH_TITLE), newPath)) {
		SendMessage(GetDlgItem(hDialog, IDC_PATHEDIT), WM_SETTEXT, 0, (LPARAM)newPath);
	}
}

// CCJ 10/15/99 
// Switched from custom path selection dialog to the standard one.
/*
static unsigned int	CALLBACK FileOpenHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM	lParam) 
{
	static int fakeDblClk;

	switch (message) 
	{
		case WM_INITDIALOG: 
			useFolder[0] = _T('\0');
			fakeDblClk = 0;
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			return	1;
		  
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_MY_OK:
					GetDlgItemText(hWnd, 1088, useFolder, MAX_PATH);

					EndDialog(hWnd, TRUE);
					break;

				case IDCANCEL:
					break;
			}
			break;
	}
	return 0;
}
*/

BOOL RefCheck::ChooseDir(TCHAR *title, TCHAR *dir)
{
	TCHAR	folderName[MAX_PATH];

	_tcscpy(folderName, dir);

	ip->ChooseDirectory(ip->GetMAXHWnd(), title, folderName, NULL);
	if (_tcscmp(folderName, _T("")) == 0) {
		// Cancel
		return FALSE;
		}

	_tcscpy(dir, folderName);

	return TRUE;

// CCJ 10/15/99 
/*
	TCHAR			fileName[MAX_PATH];
	TCHAR			folderName[MAX_PATH];
	TCHAR			folderName2[MAX_PATH];
	OPENFILENAME	ofn;	
	BOOL			done = FALSE;

	do {
		_tcscpy(fileName, dir);
		ofn.lStructSize       = sizeof(OPENFILENAME);
		ofn.hwndOwner         = hDialog;
		ofn.lpstrTitle        = title;
		ofn.lpstrFilter       = _T("All\0*.*\0");
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter    = 0;
		ofn.nFilterIndex      = 1;
		ofn.lpstrFile         = fileName;
		ofn.nMaxFile          = sizeof(fileName);
		ofn.lpstrFileTitle    = NULL;
		ofn.nMaxFileTitle     = 0;
		ofn.lpstrInitialDir   = dir;
		ofn.nFileOffset       = 0;
		ofn.nFileExtension    = 0;
		ofn.lpstrDefExt       = NULL;
		ofn.nFilterIndex      = 0;
		ofn.hInstance         = hInstance;	
		ofn.lpTemplateName	  =	MAKEINTRESOURCE(IDD_CHOOSE_DIR);
		ofn.lpfnHook		  =	(LPOFNHOOKPROC)FileOpenHookProc;
		ofn.lCustData		  =	0;
		ofn.Flags			  =	OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOVALIDATE |
									OFN_ENABLETEMPLATE | OFN_ENABLEHOOK |OFN_LONGNAMES;

		_tcscpy(folderName, dir);
		useFolder = folderName;
		useFolder2 = folderName2;
		
		if(!GetOpenFileName(&ofn)) 
		{
			return FALSE;
		}
	
		if(!SetCurrentDirectory(folderName)) 
		{
			TSTR errmsg = GetString(IDS_ERROR);
			MessageBox(hDialog, GetString(IDS_NODIR), errmsg, MB_OK);
		}
		else {
			done = TRUE;
		}
 	} while (!done);

	_tcscpy(dir, folderName);

	return TRUE;
*/

}

void RefCheck::SelectMissing()
{
	int numItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);
	int numSel = 0;

	// Clear selection
	SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SELITEMRANGE, FALSE, MAKELPARAM(0, numItems-1));

	SetCursor(LoadCursor(NULL,IDC_WAIT));

	for (int i=0; i < numItems; i++) {
		if(DataIndex::Instance()->Get(i) == DataIndex::TEXMAP)	{
			BitmapTex* bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
			if (bmTex) {
				TCHAR* texName = bmTex->GetMapName();
				if (texName && _tcscmp(texName, _T(""))) {
					TCHAR	newName[MAX_PATH];
					if (!FindMap(texName, newName)) {
						SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SETSEL, TRUE, i);
						numSel++;
					}
				}
			}
		}
		else if (DataIndex::Instance()->Get(i) == DataIndex::PHOTOMETRIC)	{
			TCHAR entry[MAX_PATH];
			TCHAR	newName[MAX_PATH];
			SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM) entry);
			if (entry && _tcscmp(entry, _T(""))) {
				if (!FindMap(entry, newName)) {
					SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SETSEL, TRUE, i);
					numSel++;
				}
			}
		}
		else if (DataIndex::Instance()->Get(i) == DataIndex::DXMATERIAL)	{
			TCHAR entry[MAX_PATH];
			TCHAR	newName[MAX_PATH];
			SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM) entry);
			if (entry && _tcscmp(entry, _T(""))) {
				if (!FindMap(entry, newName)) {
					SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_SETSEL, TRUE, i);
					numSel++;
				}
			}
		}


	}
	SetCursor(LoadCursor(NULL,IDC_ARROW));

	EnableEntry(hDialog, numSel > 0, numSel);
}

BOOL RefCheck::FindMap(TCHAR* mapName, TCHAR* newName)
{
	TCHAR	tmpPath[MAX_PATH];
	BOOL	bFound = FALSE;

	// Copy the map name to the new target name even if the map is not found.
	_tcscpy(newName, mapName);

	// Try full path
	if (!_access(mapName, 0)) {
		bFound = TRUE;
		_tcscpy(newName, mapName);
	}
	else {
		// Try MAX File directory

		TCHAR	drive[_MAX_DRIVE];
		TCHAR	dir[_MAX_DIR];
		TCHAR	fname[_MAX_FNAME];
		TCHAR	ext[_MAX_EXT];

		_tsplitpath(ip->GetCurFilePath(), drive, dir, fname, ext);
		_tcscpy(tmpPath, drive);
		_tcscat(tmpPath, dir);
		if (tmpPath[_tcslen(tmpPath)-1] != _T('\\')) {
			_tcscat(tmpPath, _T("\\"));
		}
		_tsplitpath(mapName, drive, dir, fname, ext);
		_tcscat(tmpPath, fname);
		_tcscat(tmpPath, ext);
		if (!_access(tmpPath, 0)) {
			bFound = TRUE;
			_tcscpy(newName, tmpPath);
		}
		else {
			// Try each of the map paths
			for (int d = 0; d < TheManager->GetMapDirCount(); d++) {
				_tcscpy(tmpPath, TheManager->GetMapDir(d));
				if (tmpPath[_tcslen(tmpPath)-1] != _T('\\')) {
					_tcscat(tmpPath, _T("\\"));
				}
				_tcscat(tmpPath, fname);
				_tcscat(tmpPath, ext);
				if (!_access(tmpPath, 0)) {
					_tcscpy(newName, tmpPath);
					bFound = TRUE;
					break;
				}
			}
		}
	}

	return bFound;
}

void RefCheck::SetActualPath()
{
	int numItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);
	int numSelItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSELCOUNT, 0, 0);

	SetCursor(LoadCursor(NULL,IDC_WAIT));

	int nFoundMapCount = 0;
	int nMissingMapCount = 0;
	for (int i=0; i < numItems; i++) {
		if (SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSEL, i, 0) > 0) {
			if(DataIndex::Instance()->Get(i) == DataIndex::TEXMAP)	{
				BitmapTex* bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
				if (bmTex) {
					TCHAR* texName = bmTex->GetMapName();
					if (texName && _tcscmp(texName, _T(""))) {
						TCHAR	newName[MAX_PATH];
						if (FindMap(texName, newName)) {
							bmTex->SetMapName(newName);
							bmTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
							nFoundMapCount++;
						}
						else {
							nMissingMapCount++;
						}
					}
				}
			}
			else if(DataIndex::Instance()->Get(i) == DataIndex::PHOTOMETRIC)	{
				TCHAR entry[MAX_PATH];
				TCHAR	newName[MAX_PATH];
				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM) entry);
				if (entry && _tcscmp(entry, _T(""))) {
					if (FindMap(entry, newName)) {
						DataIndex::Instance()->SetPathOnLights(entry, newName);
						nFoundMapCount++;
					}
				}
			}
			else if(DataIndex::Instance()->Get(i) == DataIndex::DXMATERIAL)	{
				MtlBase * mtl = (MtlBase*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
				if (mtl) {

					TCHAR entry[MAX_PATH];
					TCHAR	newName[MAX_PATH];
					SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM) entry);
					if (entry && _tcscmp(entry, _T(""))) {
						if (FindMap(entry, newName)) {
							IDxMaterial * idxm = (IDxMaterial*)mtl->GetInterface(IDXMATERIAL_INTERFACE);
							int bitmapCount = idxm->GetNumberOfEffectBitmaps();
							for(int j=0; j<bitmapCount;j++)
							{
								PBBitmap* bmap = idxm->GetEffectBitmap(j);

								if(!_tcscmp(entry, bmap->bi.Name()))
								{
									PBBitmap newBitmap;
									TSTR newFile = TSTR(newName);
									newBitmap.bi.SetName(newFile);
									idxm->SetEffectBitmap(j,&newBitmap);
								}
							}
							nFoundMapCount++;
						}
					}
				}
			}
		}
	}
	CheckDependencies();
	SetCursor(LoadCursor(NULL,IDC_ARROW));

	TSTR msg;
	msg.printf(GetString(IDS_FOUNDLOG), nFoundMapCount, nMissingMapCount);
	MessageBox(hDialog, msg, GetString(IDS_MESSAGE), MB_OK);
}

class CopyWarning {
public:
	TSTR	filename;
	int		retval;
};

INT_PTR CALLBACK CopyWarningDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	CopyWarning* obj = (CopyWarning*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (message) {
		case WM_INITDIALOG:
			obj = (CopyWarning*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));

			SendMessage(GetDlgItem(hWnd, IDC_FILENAME), WM_SETTEXT, 0, (LPARAM)(TCHAR*)obj->filename);
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_YES:
					obj->retval = COPYWARN_YES;
					EndDialog(hWnd, 1);
					break;
				case IDC_YESTOALL:
					obj->retval = COPYWARN_YESTOALL;
					EndDialog(hWnd, 1);
					break;
				case IDCANCEL:
					obj->retval = COPYWARN_NO;
					EndDialog(hWnd, 1);
					break;
				case IDC_NOTOALL:
					obj->retval = COPYWARN_NOTOALL;
					EndDialog(hWnd, 1);
					break;
			}
			return 1;
		case WM_CLOSE:
			EndDialog(hWnd, 0);			
			break;
	}
	return 0;
}

int RefCheck::CopyWarningPrompt(HWND hParent, TCHAR* filename)
{
	CopyWarning copyWarn;

	copyWarn.filename = filename;

	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_COPYWARNING),
		hParent,
		(DLGPROC)CopyWarningDlgProc,
		(LPARAM)&copyWarn);

	return copyWarn.retval;
}


INT_PTR CALLBACK CopyDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	RefCheck* util = (RefCheck*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (message) {
		case WM_INITDIALOG:
			util = (RefCheck*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			EnableWindow(util->hDialog, FALSE);

			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
					util->SetCopyQuitFlag(TRUE);
					DestroyWindow(hWnd);
					break;
			}
			return 1;
		case WM_CLOSE:
			DestroyWindow(hWnd);			
			break;
		case WM_DESTROY:
			EnableWindow(util->hDialog, TRUE);
			break;

	}
	return 0;
}


void RefCheck::CopyMaps()
{
	TCHAR	newPath[MAX_PATH];
	int		copyCount = 0;
	HWND	hCopyDlg;
	TSTR	copyLog;

	SetCopyQuitFlag(FALSE);

	if (ChooseDir(GetString(IDS_NEWPATH_TITLE), newPath)) {

		hCopyDlg = CreateDialogParam(
			hInstance,
			MAKEINTRESOURCE(IDD_COPYDLG),
			hDialog,
			CopyDlgProc,
			(LPARAM)this);

		if (hCopyDlg) {

			TCHAR	tmpPath[MAX_PATH];
			TCHAR	texName[MAX_PATH];

			if (newPath[_tcslen(newPath)-1] != _T('\\')) {
				_tcscat(newPath, _T("\\"));
			}

			int numItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETCOUNT, 0, 0);
			int numSelItems = SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSELCOUNT, 0, 0);

			SendMessage(GetDlgItem(hCopyDlg, IDC_COPYPROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, numSelItems-1));

			int		progress	= 0;
			int		copyStat	= COPYWARN_NO;
			BOOL	bOverwrite	= FALSE;
			BOOL	fileExists	= FALSE;

			for (int i=0; i < numItems; i++) {
				TCHAR tmpName[MAX_PATH];
				if (SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSEL, i, 0) > 0) {
					if(DataIndex::Instance()->Get(i) == DataIndex::TEXMAP)	{
						BitmapTex* bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, i, 0);
						if (bmTex)	
							_tcscpy(tmpName, bmTex->GetMapName()); 
					}
					else if(DataIndex::Instance()->Get(i) == DataIndex::PHOTOMETRIC)	{
						SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM) tmpName);
					}
					else if(DataIndex::Instance()->Get(i) == DataIndex::DXMATERIAL){
						SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, i, (LPARAM) tmpName);
					}
					else	
						return;
					if (tmpName && _tcscmp(tmpName, _T(""))) {
						TCHAR	drive[_MAX_DRIVE];
						TCHAR	dir[_MAX_DIR];
						TCHAR	fname[_MAX_FNAME];
						TCHAR	ext[_MAX_EXT];
						
							FindMap(tmpName, texName);

						_tsplitpath(texName, drive, dir, fname, ext);
						_tcscpy(tmpPath, newPath);
						_tcscat(tmpPath, fname);
						_tcscat(tmpPath, ext);

						//bmTex->SetMapName(newName);
						//bmTex->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
						progress++;

							SendMessage(GetDlgItem(hCopyDlg, IDC_CURRENTFILE), WM_SETTEXT, 0, (LPARAM)texName);

						if (GetCopyQuitFlag()) {
							break;
						}

						fileExists = !_access(tmpPath, 0);

						if (copyStat == COPYWARN_NOTOALL) {
							//
						}
						else if (copyStat == COPYWARN_YESTOALL) {
							//
						}
						else if (fileExists) {
							copyStat = CopyWarningPrompt(hCopyDlg, tmpPath);
						}

						if (copyStat == COPYWARN_NO || copyStat == COPYWARN_NOTOALL) {
							bOverwrite = FALSE;
						}
						else {
							bOverwrite = TRUE;
						}

						if (fileExists && bOverwrite == FALSE) {
							copyLog = GetString(IDS_COPYSKIP);
						}
							else if (!CopyFile(texName, tmpPath, !bOverwrite)) {
							copyLog = GetString(IDS_COPYFAIL);
							/*
							TSTR szTitle = GetString(IDS_COPYTITLE);
							if (MessageBox(hDialog, GetString(IDS_COPYERROR), szTitle, MB_OKCANCEL | MB_ICONWARNING) != IDOK) {
								break;
							}
							*/
						}
						else {
							copyLog = GetString(IDS_COPYOK);
							copyCount++;
						}
						copyLog += texName;
						SendMessage(GetDlgItem(hCopyDlg, IDC_COPYLOG), LB_ADDSTRING, 0, (LPARAM)(char*)copyLog);

					}
					SendMessage(GetDlgItem(hCopyDlg, IDC_COPYPROGRESS), PBM_SETPOS, progress, 0);
				}
			}
			if (GetCopyQuitFlag()) {
				copyLog.printf(GetString(IDS_COPYABORTED), copyCount);
			}
			else {
				copyLog.printf(GetString(IDS_COPYCOUNT), copyCount);
			}
			SendMessage(GetDlgItem(hCopyDlg, IDC_CURRENTFILE), WM_SETTEXT, 0, (LPARAM)(char*)copyLog);
			SendMessage(GetDlgItem(hCopyDlg, IDCANCEL), WM_SETTEXT, 0, (LPARAM)GetString(IDS_CLOSE));
		}
	}
}

INT_PTR CALLBACK InfoDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	RefCheck* util = (RefCheck*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (message) {
		case WM_INITDIALOG:
			util = (RefCheck*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			CenterWindow(hWnd,GetParent(hWnd));
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			util->HandleInfoDlg(hWnd);
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:				  
				case IDCANCEL:
					EndDialog(hWnd,1);
					break;
				case IDC_VIEW: {
						BitmapTex* i = util->GetInfoTex();
						if (i) {
							Bitmap* b = i->GetBitmap(util->ip->GetTime());
							if (b) {
								b->Display(i->GetName());
							}
						}
						else
						{
							Bitmap *bmap = NULL;
							BitmapInfo bi;
							bi.SetName(util->bitmapName);
							BMMRES status;
							bmap = TheManager->Load(&bi, &status);
							if(bmap)
								bmap->Display(util->bitmapName);

						}
					}
					break;
			}
			return 1;
	}
	return 0;
}

void RefCheck::ShowInfo()
{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_INFODLG),
		hDialog,
		(DLGPROC)InfoDlgProc,
		(LPARAM)this);

	/*
	if (pDib) {
		LocalFree(pDib);
		pDib = NULL;
	}
	*/
}



void RefCheck::HandleInfoDlg(HWND dlg)
{
	int idx;

	SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETSELITEMS, 1, (LPARAM)&idx);
	if(DataIndex::Instance()->Get(idx) == DataIndex::TEXMAP)	{
		// active view bitmap button
		EnableWindow(GetDlgItem(dlg, IDC_VIEW), true);
		BitmapTex* bmTex = (BitmapTex*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, idx, 0);
		if (bmTex) {
			SetInfoTex(bmTex);
			INodeTab nodeTab;
			FindNodesProc dep(&nodeTab);
			bmTex->EnumDependents(&dep);
			for (int i=0; i<nodeTab.Count(); i++) {
				SendMessage(GetDlgItem(dlg, IDC_NODELIST), LB_ADDSTRING, 0, (LPARAM)nodeTab[i]->GetName());
			}

			/*
			Bitmap* b = bmTex->GetBitmap(ip->GetTime());

			if (b) {
				ICustImage* iImage;
				pDib = b->ToDib();
				iImage = GetICustImage(GetDlgItem(dlg, IDC_IMAGE));
				// ...
				ReleaseICustImage(iImage);
			}
			*/
		}
	}
	else if( DataIndex::Instance()->Get(idx) == DataIndex::PHOTOMETRIC)	{
		// deactive view bitmap button
		EnableWindow(GetDlgItem(dlg, IDC_VIEW), false);
		if(DataIndex::Instance()->nodeTab.Count())	{
			for (int i=0; i<DataIndex::Instance()->nodeTab.Count(); i++) {
				EnumLightDistFileCallBack callBack;
				DataIndex::Instance()->nodeTab[i]->EnumAuxFiles(callBack, FILE_ENUM_ALL);
				TCHAR entry[MAX_PATH];
				SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, idx, (LPARAM) entry);
				for(int j=0;j<callBack.fileList.Count(); j++)	{
					if(!_tcscmp(entry, callBack.fileList[j]))	{
						SendMessage(GetDlgItem(dlg, IDC_NODELIST), LB_ADDSTRING, 0, (LPARAM)DataIndex::Instance()->nodeTab[i]->GetName());
						break;
					}
				}
				
			}
		}
	
	}
	else if( DataIndex::Instance()->Get(idx) == DataIndex::DXMATERIAL){
		TCHAR entry[MAX_PATH];
		MtlBase* mtl = (MtlBase*)SendMessage(GetDlgItem(hDialog, IDC_DEPLIST), LB_GETITEMDATA, idx, 0);
		SendMessage(GetDlgItem(hDialog, IDC_DEPLIST),LB_GETTEXT, idx, (LPARAM) entry);
		bitmapName = TSTR(entry);
		TCHAR	drive[_MAX_DRIVE];
		TCHAR	dir[_MAX_DIR];
		TCHAR	fname[_MAX_FNAME];
		TCHAR	ext[_MAX_EXT];

		_tsplitpath(entry, drive, dir, fname, ext);

		if(!_tcscmp(ext,_T(".fx")))
			EnableWindow(GetDlgItem(dlg, IDC_VIEW), false);

		if (mtl) {
			SetInfoTex(NULL);
			INodeTab nodeTab;
			FindNodesProc dep(&nodeTab);
			mtl->EnumDependents(&dep);
			for (int i=0; i<nodeTab.Count(); i++) {
				SendMessage(GetDlgItem(dlg, IDC_NODELIST), LB_ADDSTRING, 0, (LPARAM)nodeTab[i]->GetName());
			}
		}
	}
}

BOOL RefCheck::GetIncludeMatLib()
{
	return IsDlgButtonChecked(hPanel, IDC_USE_MATLIB) == BST_CHECKED;
}

BOOL RefCheck::GetIncludeMedit()
{
	return IsDlgButtonChecked(hPanel, IDC_INCLUDE_MEDIT) == BST_CHECKED;
}

// Enumerate the scene
void RefCheck::EnumerateNodes(INode *root)
{
	for (int k=0; k<root->NumberOfChildren(); k++)
	{
		INode *node = root->GetChildNode(k);
	//	MtlBase* mat = (MtlBase*)node->GetMtl();
	//	if (mat)
	//		CEnumMtlTree(mat,CEnym);
		// enumerate light distribution files
		Object* obj = node->GetObjectRef();
		if(obj && obj->IsSubClassOf(LIGHTSCAPE_LIGHT_CLASS))	{
			DataIndex::Instance()->AddLight(static_cast<LightscapeLight*>(obj));
			DataIndex::Instance()->nodeTab.Append(1, &node);
		}
      else if(obj && obj->IsSubClassOf(Class_ID(DERIVOB_CLASS_ID, 0)))	{
         Object* obj2 = obj->FindBaseObject();
         if(obj2 && obj2->IsSubClassOf(LIGHTSCAPE_LIGHT_CLASS))	{
			   DataIndex::Instance()->AddLight(static_cast<LightscapeLight*>(obj2));
			   DataIndex::Instance()->nodeTab.Append(1, &node);
		   }
      }

		node->EnumAuxFiles(distributionLister, FILE_ENUM_ALL);
		if(node->NumberOfChildren()>0) EnumerateNodes(node);
	}
}

