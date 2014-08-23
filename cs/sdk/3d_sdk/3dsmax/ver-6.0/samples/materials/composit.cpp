/**********************************************************************
 *<
	FILE: composit.cpp

	DESCRIPTION: A compositor texture map.

	CREATED BY: Rolf Berteig

	HISTORY: UPdated to Param2 1/11/98 Peter Watje

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include <bmmlib.h>
#include "iparamm2.h"
#include "macrorec.h"

#include <d3dx8.h>

#include "IHardwareMaterial.h"

#define MAXTEXHANDLES 3

extern HINSTANCE hInstance;

static Class_ID compClassID(COMPOSITE_CLASS_ID,0);

class Composite;
class CompositeDlg;

#define NDLG 6


class CompositeDlg: public ParamDlg {
	public:
		HWND hwmedit;	 	// window handle of the materials editor dialog
		IMtlParams *ip;
		Composite *theTex;	 
		HWND hPanel; 		// Rollup pane		
		HWND hScroll;
		BOOL valid;
		ICustButton *iBut[NDLG];
		TexDADMgr dadMgr;
				
		CompositeDlg(HWND hwMtlEdit, IMtlParams *imp, Composite *m); 
		~CompositeDlg();
		BOOL PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );		
		void VScroll(int code, short int cpos );
		void LoadDialog(BOOL draw);  // stuff params into dialog
		void ReloadDialog();
		void UpdateMtlDisplay();		
		void UpdateSubTexNames();
		void ActivateDlg(BOOL onOff);
		void Invalidate();
		void Destroy(HWND hWnd);
		void SetNumMaps();
		void DragAndDrop(int ifrom, int ito);

		// methods inherited from ParamDlg:
		Class_ID ClassID() {return compClassID;  }
		void SetThing(ReferenceTarget *m);
		ReferenceTarget* GetThing() {return (ReferenceTarget *)theTex;}
		void DeleteThis() { delete this;  }	
		void SetTime(TimeValue t);
		int FindSubTexFromHWND(HWND hw);
	};



class Composite: public MultiTex { 
	public:			
		Tab<Texmap*> subTex;
		Tab<BOOL>mapOn;
		Interval ivalid;
		int offset;
		int rollScroll;
		CompositeDlg *paramDlg;

		BOOL Param1;
		IParamBlock2 *pblock;   // ref #0		
		
		TexHandle *texHandle[MAXTEXHANDLES];
		int useSubForTex[MAXTEXHANDLES];
		int numTexHandlesUsed;
		Interval texHandleValid;

	
		Composite();
		~Composite() {
			DiscardTexHandles(); 
			}
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void ClampOffset();
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) {Interval v; Update(t,v); return ivalid;}
		void NotifyChanged();		
		void SetNumSubTexmaps(int n) { SetNumMaps(n); }
		void SetNumMaps(int n);

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		
		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);		

		// Methods to access texture maps of material
		int NumSubTexmaps() {return subTex.Count();}
		Texmap* GetSubTexmap(int i) {return subTex[i];}		
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);		

		Class_ID ClassID() {return compClassID;}
		SClass_ID SuperClassID() {return TEXMAP_CLASS_ID;}
		void GetClassName(TSTR& s) {s=GetString(IDS_RB_COMPOSITE);}
		void DeleteThis() {delete this;}

		int NumSubs() {return subTex.Count();}
		Animatable* SubAnim(int i) {return subTex[i];}
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) {return subNum+1;}

		// From ref
 		int NumRefs() {return subTex.Count()+1;}
		RefTargetHandle GetReference(int i) {
						if (i==0) return pblock;
						else return subTex[i-1];
						}
		void SetReference(int i, RefTargetHandle rtarg) {
						if(i==0) pblock = (IParamBlock2*) rtarg;
						else subTex[i-1] = (Texmap*)rtarg;
						}

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		int RemapRefOnLoad(int iref) ;

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock


		// Multiple map in vp support -- DS 5/4/00
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff);
		BOOL SupportsMultiMapsInViewport() { return TRUE; }
		void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb);
		void DiscardTexHandles() {
			for (int i=0; i<MAXTEXHANDLES; i++) {
				if (texHandle[i]) {
					texHandle[i]->DeleteThis();
					texHandle[i] = NULL;
					}
				}
			texHandleValid.SetEmpty();
			}
		// From Texmap
		bool IsLocalOutputMeaningful( ShadeContext& sc );

	};

class CompositeClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return GetAppID() != kAPP_VIZR;}
	void *			Create(BOOL loading) {return new Composite;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_COMPOSITE_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() {return TEXMAP_CLASS_ID;}
	Class_ID 		ClassID() {return compClassID;}
	const TCHAR* 	Category() {return TEXMAP_CAT_COMP;}
// PW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("compositeTexture"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};
static CompositeClassDesc compCD;
ClassDesc* GetCompositeDesc() {return &compCD;}
// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { comptex_params, };  // pblock ID
// multi_params param IDs
enum 
{ 
	comptex_tex, comptex_ons

};

// per instance gradient block
static ParamBlockDesc2 comptex_param_blk ( comptex_params, _T("parameters"),  0, &compCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_COMPOSITEMAP, IDS_DS_BASIC, 0, 0, NULL, 
	// params
	comptex_tex,	_T("mapList"),	TYPE_TEXMAP_TAB, 2,	P_OWNERS_REF + P_VARIABLE_SIZE,	IDS_DS_TEXMAP,	
		p_refno,	1, 
		end,
	comptex_ons,	_T("mapEnabled"), TYPE_BOOL_TAB, 2,	P_VARIABLE_SIZE,				IDS_JW_MAP1ENABLE,
		p_default,	TRUE,
		end,

	end
);


static INT_PTR CALLBACK PanelDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
	{
	CompositeDlg *theDlg = (CompositeDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (msg==WM_INITDIALOG) {
		theDlg = (CompositeDlg*)lParam;
		theDlg->hPanel = hWnd;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
		}	
	if (theDlg) return theDlg->PanelProc(hWnd,msg,wParam,lParam);
	else return FALSE;
	}

int CompositeDlg::FindSubTexFromHWND(HWND hw) {
	for (int i=0; i<NDLG; i++) {
		if (hw == iBut[i]->GetHwnd()) return i+theTex->offset;
		}	
	return -1;
	}

void CompositeDlg::DragAndDrop(int ifrom, int ito) {
	theTex->CopySubTexmap(hPanel,ifrom+theTex->offset, ito+theTex->offset);
	theTex->NotifyChanged();
	}


//-------------------------------------------------------------------

CompositeDlg::CompositeDlg(HWND hwMtlEdit, IMtlParams *imp, Composite *m) 
	{
	dadMgr.Init(this);
	hwmedit  = hwMtlEdit;
	ip       = imp;
	hPanel   = NULL;
	theTex   = m; 	
	valid    = FALSE;	
	for (int i=0; i<NDLG; i++) iBut[i] = NULL;
	hPanel   = ip->AddRollupPage( 
		hInstance,
		MAKEINTRESOURCE(IDD_COMPOSITEMAP),
		PanelDlgProc, 
		GetString(IDS_RB_COMPOSITEPARAMS),
		(LPARAM)this);	
	}

void CompositeDlg::Destroy(HWND hWnd) {
	for (int i=0; i<NDLG; i++) {
		ReleaseICustButton(iBut[i]);
		iBut[i] = NULL; 
		}
	}

void CompositeDlg::Invalidate()
	{
	valid = FALSE;
	Rect rect;
	rect.left = rect.top = 0;
	rect.right = rect.bottom = 10;
	InvalidateRect(hPanel,&rect,FALSE);
	}

void CompositeDlg::ReloadDialog() 
	{
	Interval valid;
	theTex->Update(ip->GetTime(), valid);
	LoadDialog(FALSE);
	}

void CompositeDlg::SetTime(TimeValue t) 
	{
	Interval valid;	
	theTex->Update(ip->GetTime(), valid);
	LoadDialog(FALSE);
	InvalidateRect(hPanel,NULL,0);	
	}

CompositeDlg::~CompositeDlg() 
	{
	theTex->paramDlg = NULL;	
	SetWindowLongPtr(hPanel, GWLP_USERDATA, NULL);	
	hPanel =  NULL;
	}


void CompositeDlg::VScroll(int code, short int cpos ) {
	switch (code) {
		case SB_LINEUP: 	theTex->offset--;		break;
		case SB_LINEDOWN:	theTex->offset++;		break;
		case SB_PAGEUP:		theTex->offset -= NDLG;	break;
		case SB_PAGEDOWN:	theTex->offset += NDLG;	break;
		
		case SB_THUMBPOSITION: 
		case SB_THUMBTRACK:
			theTex->offset = cpos;
			break;
		}
	theTex->ClampOffset();
	UpdateSubTexNames();						
	LoadDialog(ip->GetTime());
	}

static int mapIDs[] = {IDC_COMP_TEX1,IDC_COMP_TEX2,IDC_COMP_TEX3,IDC_COMP_TEX4,IDC_COMP_TEX5,IDC_COMP_TEX6};
static int labelIDs[] = {IDC_COMP_LABEL1,IDC_COMP_LABEL2,IDC_COMP_LABEL3,IDC_COMP_LABEL4,IDC_COMP_LABEL5,IDC_COMP_LABEL6};
static int mapOnIDs[] = {IDC_MAPON1,IDC_MAPON2,IDC_MAPON3,IDC_MAPON4,IDC_MAPON5,IDC_MAPON6};


BOOL CompositeDlg::PanelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
	{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG: {			
			hScroll	= GetDlgItem(hWnd,IDC_COMP_SCROLL);
			SetScrollRange(hScroll,SB_CTL,0,theTex->NumSubTexmaps()-NDLG,FALSE);
			SetScrollPos(hScroll,SB_CTL,theTex->offset,TRUE);
			EnableWindow(hScroll,theTex->NumSubTexmaps()>NDLG);
			for (int i=0; i<NDLG; i++) {
				iBut[i] = GetICustButton(GetDlgItem(hWnd,IDC_COMP_TEX1+i));
				iBut[i]->SetDADMgr(&dadMgr);
//				if (i-theTex->offset<theTex->mapOn.Count())
//					SetCheckBox(hWnd, mapOnIDs[i], theTex->mapOn[i-theTex->offset]);
				}
			return TRUE;
			}
			
		case WM_PAINT:
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;

		case WM_VSCROLL:
			VScroll(LOWORD(wParam),(short int)HIWORD(wParam));
			break;
			
		case WM_COMMAND: 		    		 	
			switch (id) {		
				case IDC_COMP_TEX1: PostMessage(hwmedit,WM_TEXMAP_BUTTON,theTex->offset,(LPARAM)theTex); break;
				case IDC_COMP_TEX2: PostMessage(hwmedit,WM_TEXMAP_BUTTON,theTex->offset+1,(LPARAM)theTex); break;
				case IDC_COMP_TEX3: PostMessage(hwmedit,WM_TEXMAP_BUTTON,theTex->offset+2,(LPARAM)theTex); break;
				case IDC_COMP_TEX4: PostMessage(hwmedit,WM_TEXMAP_BUTTON,theTex->offset+3,(LPARAM)theTex); break;
				case IDC_COMP_TEX5: PostMessage(hwmedit,WM_TEXMAP_BUTTON,theTex->offset+4,(LPARAM)theTex); break;
				case IDC_COMP_TEX6: PostMessage(hwmedit,WM_TEXMAP_BUTTON,theTex->offset+5,(LPARAM)theTex); break;

				case IDC_MAPON1:							
				case IDC_MAPON2:							
				case IDC_MAPON3:							
				case IDC_MAPON4:							
				case IDC_MAPON5:							
				case IDC_MAPON6:
					theTex->pblock->SetValue(comptex_ons,0,GetCheckBox(hWnd, id),id-IDC_MAPON1+theTex->offset);
//					theTex->mapOn[id-IDC_MAPON1+theTex->offset] = GetCheckBox(hWnd, id);
					theTex->NotifyChanged();
					break;							
									
				case IDC_COMP_SETNUM:
					SetNumMaps();
					break;
				}			
			break;
				
		case WM_DESTROY:
			Destroy(hWnd);
			break;
    	}
	return FALSE;
	}

void CompositeDlg::UpdateSubTexNames() 
	{
	for (int i=theTex->offset; i<theTex->subTex.Count(); i++) {
		if (i-theTex->offset>=NDLG) break;

		Texmap *m = theTex->subTex[i];
		TSTR nm;
		if (m) 	nm = m->GetFullName();
		else 	nm = GetString(IDS_DS_NONE);
		TSTR buf;
		buf.printf(_T("%s %d:"),GetString(IDS_RB_MAP2),i+1);
		iBut[i-theTex->offset]->SetText(nm.data());
		SetDlgItemText(hPanel, labelIDs[i-theTex->offset], buf);
//		SetCheckBox(hPanel, mapOnIDs[i-theTex->offset], theTex->mapOn[i]);
		int on;
		Interval iv;
		theTex->pblock->GetValue(comptex_ons,0,on,iv,i);
		SetCheckBox(hPanel, mapOnIDs[i-theTex->offset], on);
		}
	}


void CompositeDlg::LoadDialog(BOOL draw) 
	{	
	if (theTex) {		
		theTex->ClampOffset();
		
		SetScrollRange(hScroll,SB_CTL,0,theTex->subTex.Count()-NDLG,FALSE);
		SetScrollPos(hScroll,SB_CTL,theTex->offset,TRUE);
		EnableWindow(hScroll,theTex->NumSubTexmaps()>NDLG);

		if (theTex->subTex.Count()>NDLG) {
			EnableWindow(GetDlgItem(hPanel,IDC_COMP_UP),theTex->offset>0);
			EnableWindow(GetDlgItem(hPanel,IDC_COMP_PAGEUP),theTex->offset>0);
			EnableWindow(GetDlgItem(hPanel,IDC_COMP_DOWN),theTex->offset+NDLG<theTex->subTex.Count());
			EnableWindow(GetDlgItem(hPanel,IDC_COMP_PAGEDOWN),theTex->offset+NDLG<theTex->subTex.Count());
		} else {
			EnableWindow(GetDlgItem(hPanel,IDC_COMP_UP),FALSE);
			EnableWindow(GetDlgItem(hPanel,IDC_COMP_PAGEUP),FALSE);
			EnableWindow(GetDlgItem(hPanel,IDC_COMP_DOWN),FALSE);
			EnableWindow(GetDlgItem(hPanel,IDC_COMP_PAGEDOWN),FALSE);
			}

		Interval valid;
		theTex->Update(ip->GetTime(),valid);		
		UpdateSubTexNames();
		TSTR buf;
		buf.printf(_T("%d"),theTex->subTex.Count());
		SetDlgItemText(hPanel,IDC_COMP_NUMMAPS,buf);
		for (int i=0; i<min(theTex->subTex.Count(),NDLG); i++) {
			ShowWindow(GetDlgItem(hPanel,mapIDs[i]),SW_SHOW);
			ShowWindow(GetDlgItem(hPanel,labelIDs[i]),SW_SHOW);
			ShowWindow(GetDlgItem(hPanel,mapOnIDs[i]),SW_SHOW);
//			SetCheckBox(hPanel, mapOnIDs[i], theTex->mapOn[i+theTex->offset]);
			}
		for (; i<NDLG; i++) {
			ShowWindow(GetDlgItem(hPanel,mapIDs[i]),SW_HIDE);
			ShowWindow(GetDlgItem(hPanel,labelIDs[i]),SW_HIDE);
			ShowWindow(GetDlgItem(hPanel,mapOnIDs[i]),SW_HIDE);
			}
		}
	}

void CompositeDlg::SetThing(ReferenceTarget *m) 
	{
	assert (m->ClassID()==compClassID);
	assert (m->SuperClassID()==TEXMAP_CLASS_ID);
	if (theTex) theTex->paramDlg = NULL;
	theTex = (Composite*)m;	
	if (theTex) theTex->paramDlg = this;
	LoadDialog(TRUE);
	}

void CompositeDlg::UpdateMtlDisplay() {	
	ip->MtlChanged();  
	}

void CompositeDlg::ActivateDlg(BOOL onOff) {	
	}


static INT_PTR CALLBACK NumMapsDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			ISpinnerControl *spin = 
				SetupIntSpinner(
					hWnd,IDC_COMP_NUMMAPSSPIN,IDC_COMP_NUMMAPS,
					2,1000,(int)lParam);
			ReleaseISpinner(spin);
			CenterWindow(hWnd,GetParent(hWnd));
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					ISpinnerControl *spin = 
						GetISpinner(GetDlgItem(hWnd,IDC_COMP_NUMMAPSSPIN));
					EndDialog(hWnd,spin->GetIVal());
					ReleaseISpinner(spin);
					break;
					}

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}
		
void CompositeDlg::SetNumMaps()
	{
	int res = DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_COMP_SETNUM),
		hPanel,
		NumMapsDlgProc,
		(LPARAM)theTex->subTex.Count());
	if (res>=0) {
		theTex->SetNumMaps(res);
		LoadDialog(TRUE);
		}
	}

//-----------------------------------------------------------------------------
//  Composite
//-----------------------------------------------------------------------------

#define COMPOSITE_VERSION 2

void Composite::Init() 
	{	
	macroRecorder->Disable();
	ivalid.SetEmpty();
	offset = 0;
	subTex.Resize(0);
//	mapOn.Resize(0);
	SetNumMaps(2);
	macroRecorder->Enable();
	}

void Composite::Reset() 
	{	
	DeleteAllRefsFromMe();
	compCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	}

void Composite::ClampOffset() {
	if (offset+NDLG>subTex.Count()) offset = subTex.Count()-NDLG;
	if (offset<0) offset=0;
	}

void Composite::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Composite::Composite() 
	{	
	for (int i=0; i<MAXTEXHANDLES; i++) {
		texHandle[i] = NULL;
		}
	texHandleValid.SetEmpty();
	paramDlg  = NULL;
	pblock = NULL;
	Param1 = FALSE;
	compCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}

void Composite::SetNumMaps(int n)
	{
	int ct = subTex.Count();
	if (n!=ct) {
		if (n<ct) {
			for (int i=n; i<ct; i++) {
				// Tell mtledit to deactivate texture map in UI
				if (subTex[i])
					subTex[i]->DeactivateMapsInTree();
				ReplaceReference(i+1,NULL);
				}
			}
		
		subTex.SetCount(n);
		// [dl | 01oct2003] Bug#525010. Initialize the new elements in the tab or pblock->SetCount()
		// may try to deference an invalid pointer from this tab.
		for(int j = ct; j < n; ++j) {
			subTex[j] = NULL;
		}

//		mapOn.SetCount(n);
		pblock->SetCount(comptex_tex,n);
		macroRec->Disable();  // JBW 4/21/99, only record one count change
		pblock->SetCount(comptex_ons,n);
		macroRec->Enable();

		if (n>ct) {
			for (int i=ct; i<subTex.Count(); i++) {
				subTex[i] = NULL;				
				pblock->SetValue(comptex_ons,0,TRUE,i);
//				mapOn[i] = TRUE;
				}
			}		
		NotifyChanged();
		}
	}

//need to remap references since we added a paramblock
int Composite::RemapRefOnLoad(int iref) 
{
if (Param1) iref += 1;
return iref;
}


static AColor black(0.0f,0.0f,0.0f,0.0f);

AColor Composite::EvalColor(ShadeContext& sc) {	
	AColor c;
	if (sc.GetCache(this,c)) 
		return c; 
	if (gbufID) sc.SetGBufferID(gbufID);
	AColor res(0,0,0);	
	for (int i=0; i<subTex.Count(); i++) {
//		int on;
		Interval iv;
//		pblock->GetValue(comptex_ons,0,on,iv,i);

		if (!subTex[i]||!mapOn[i]) continue;
//		if (!subTex[i]||!on) continue;
		res = CompOver(subTex[i]->EvalColor(sc),res);
		}
	sc.PutCache(this,res); 
	return res;
	}


Point3 Composite::EvalNormalPerturb(ShadeContext& sc) 
	{
	Point3 p(0,0,0);
	if (gbufID) sc.SetGBufferID(gbufID);
    BOOL c = FALSE;
	for (int i=0; i<subTex.Count(); i++) {
//		int on;
		Interval iv;
//		pblock->GetValue(comptex_ons,0,on,iv,i);
		if (!subTex[i]||!mapOn[i]) continue;
//		if (!subTex[i]||!on) continue;
		Point3 d = subTex[i]->EvalNormalPerturb(sc);
		if (!c) {
			p = d;
			c = 1;
			}	
		else {
			// composite perturbations using alpha -- DS 4/4/97
			AColor col = subTex[i]->EvalColor(sc);
			p = (1.0f-col.a)*p + d;
			}
		}
	return p;	
	}

RefTargetHandle Composite::Clone(RemapDir &remap) 
	{
	Composite *mnew = new Composite();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff	
	mnew->ivalid.SetEmpty();	
	mnew->subTex.SetCount(subTex.Count());
	mnew->ReplaceReference(0,remap.CloneRef(pblock));

	mnew->offset = offset;
	mnew->mapOn.SetCount(subTex.Count()); //DS 3/8/99  this seems necessary due to the param block 2 changes.

	for (int i = 0; i<subTex.Count(); i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i]) {
			mnew->ReplaceReference(i+1,remap.CloneRef(subTex[i]));
		//	GetCOREInterface()->AssignNewName(mnew->subTex[i]);
			}
//		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* Composite::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
	{
	CompositeDlg *dm = new CompositeDlg(hwMtlEdit, imp, this);
	dm->LoadDialog(TRUE);	
	paramDlg = dm;
	return dm;	
	}


void Composite::Update(TimeValue t, Interval& valid) 
	{
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();		
		int n = pblock->Count(comptex_ons);
		if (n!=mapOn.Count()) mapOn.SetCount(n);
		for (int i=0; i<subTex.Count(); i++) {
			pblock->GetValue(comptex_ons,0,mapOn[i],valid,i);
	
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}

void Composite::SetSubTexmap(int i, Texmap *m) {
	if (i>=subTex.Count()) {
		int n = subTex.Count();
		subTex.SetCount(i+1);
		pblock->SetCount(comptex_tex,i+1);

		for (int j=n; j<=i; j++)
			subTex[j] = NULL;
		}
	ReplaceReference(i+1,m);
	ivalid.SetEmpty();
	if (paramDlg)
		paramDlg->UpdateSubTexNames();
	}

TSTR Composite::GetSubTexmapSlotName(int i) {
	TSTR buf;
	buf.printf("%s %d",GetString(IDS_RB_MAP2),i+1);
	return buf;
	}
	 
TSTR Composite::SubAnimName(int i) {	
	return GetSubTexmapTVName(i);
	}

RefResult Composite::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:			
			if (paramDlg) 
				paramDlg->Invalidate();
			if (pblock->LastNotifyParamID() == comptex_tex && pblock->Count(comptex_tex) != subTex.Count())
				SetNumMaps(pblock->Count(comptex_tex));
			else if (pblock->LastNotifyParamID() == comptex_ons && pblock->Count(comptex_ons) != subTex.Count())
				SetNumMaps(pblock->Count(comptex_ons));
			DiscardTexHandles(); // DS 5/4/00
			ivalid.SetEmpty();
			if (paramDlg&&Active())
				paramDlg->ip->MtlChanged();
			break;
		
		case REFMSG_GET_PARAM_DIM:
			return REF_STOP; 
		
		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name= GetSubTexmapSlotName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 		0x4000
#define PARAM2_CHUNK 		0x4010
#define SUBTEX_COUNT_CHUNK	0x0010
#define MAPOFF_CHUNK 0x1000


IOResult Composite::Save(ISave *isave) { 
	IOResult res;
	ULONG nb;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();
	
	int c = subTex.Count();
	isave->BeginChunk(SUBTEX_COUNT_CHUNK);
	isave->Write(&c,sizeof(c),&nb);
	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();

/*	for (int i=0; i<subTex.Count(); i++) {
		if (mapOn[i]==0) {
			isave->BeginChunk(MAPOFF_CHUNK+i);
			isave->EndChunk();
			}
		}
*/

	return IO_OK;
	}	
	  

//watje
class CompTexPostLoadCallback:public  PostLoadCallback
{
public:
	Composite      *s;
	Tab<BOOL> ons;

	int Param1;
	CompTexPostLoadCallback(Composite *r, BOOL b, Tab<BOOL> bl) {s=r;Param1 = b;ons = bl;}
	void proc(ILoad *iload);
};

void CompTexPostLoadCallback::proc(ILoad *iload)
{
	if (Param1)
		{
		s->pblock->SetCount(comptex_ons,ons.Count());
		s->pblock->SetCount(comptex_tex,ons.Count());
		for (int i=0; i<s->subTex.Count(); i++) {
			s->pblock->SetValue(comptex_ons,0,ons[i],i);

			}
		}
	delete this;
}

IOResult Composite::Load(ILoad *iload) { 
	IOResult res;	
	ULONG nb;
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		int id = iload->CurChunkID();
		if (id>=MAPOFF_CHUNK&&id<=MAPOFF_CHUNK+0x1000) {
			mapOn[id-MAPOFF_CHUNK] = FALSE; 
			}
		else 
		switch(id)  {
			case SUBTEX_COUNT_CHUNK: {
				int c;
				iload->Read(&c,sizeof(c),&nb);
				subTex.SetCount(c);
				mapOn.SetCount(c);
				for (int i=0; i<c; i++)  {
					subTex[i] = NULL;
					mapOn[i] = TRUE;
					}
				break;
				}

			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}	
	CompTexPostLoadCallback* comptexplcb = new CompTexPostLoadCallback(this,Param1,mapOn);
	iload->RegisterPostLoadCallback(comptexplcb);

	return IO_OK;
	}


// DDS 5/4/00  Support for multiple texture display in viewports.
void Composite::ActivateTexDisplay(BOOL onoff) {
	if (!onoff) {
		DiscardTexHandles();
		}
	}

#define TX_MODULATE 0
#define TX_ALPHABLEND 1

struct TexOp {
	UBYTE colorOp;
	UBYTE colorAlphaSource;
	UBYTE colorScale;
	UBYTE alphaOp;
	UBYTE alphaAlphaSource;
	UBYTE alphaScale;
	};

static TexOp txops[2] = {
	{ GW_TEX_MODULATE,    GW_TEX_TEXTURE, GW_TEX_SCALE_1X, GW_TEX_LEAVE,  GW_TEX_TEXTURE, GW_TEX_SCALE_1X }, 
	{ GW_TEX_ALPHA_BLEND, GW_TEX_TEXTURE, GW_TEX_SCALE_1X, GW_TEX_LEAVE,  GW_TEX_TEXTURE, GW_TEX_SCALE_1X }, 
	};

static void SetTexOps(TextureInfo *ti, int type) {
	ti->colorOp = txops[type].colorOp;
	ti->colorAlphaSource = txops[type].colorAlphaSource;
	ti->colorScale = txops[type].colorScale;
	ti->alphaOp = txops[type].alphaOp;
	ti->alphaAlphaSource = txops[type].alphaAlphaSource;
	ti->alphaScale = txops[type].alphaScale;
	}

static Color whiteCol(1.0f, 1.0f, 1.0f);

static void SetHWTexOps(IHardwareMaterial *pIHWMat, int ntx, int type)
{
	pIHWMat->SetTextureColorArg(ntx, 1, D3DTA_TEXTURE);
	pIHWMat->SetTextureColorArg(ntx, 2, D3DTA_CURRENT);
	pIHWMat->SetTextureAlphaArg(ntx, 1, D3DTA_TEXTURE);
	pIHWMat->SetTextureAlphaArg(ntx, 2, D3DTA_CURRENT);
	switch (type) {
	case TX_MODULATE:
	default:
		pIHWMat->SetTextureColorOp(ntx, D3DTOP_MODULATE);
		pIHWMat->SetTextureAlphaOp(ntx, D3DTOP_SELECTARG2);
		pIHWMat->SetDiffuseColor(whiteCol);
		pIHWMat->SetAmbientColor(whiteCol);
		break;
	case TX_ALPHABLEND:
		pIHWMat->SetTextureColorOp(ntx, D3DTOP_BLENDTEXTUREALPHA);
		pIHWMat->SetTextureAlphaOp(ntx, D3DTOP_SELECTARG2);
		break;
	}
	pIHWMat->SetTextureTransformFlag(ntx, D3DTTFF_COUNT2);
}

void Composite::SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb)
{
	Interval valid;
	Texmap *sub[MAXTEXHANDLES];
	int texOp;

	IHardwareMaterial *pIHWMat = (IHardwareMaterial *)GetProperty(PROPID_HARDWARE_MATERIAL);
	if (pIHWMat) {
		// This is only true if Direct3D is in use
		if (texHandleValid.InInterval(t)) {
			pIHWMat->SetNumTexStages(numTexHandlesUsed);
			int nt = numTexHandlesUsed;
			if (numTexHandlesUsed == 1) {
				texOp = TX_MODULATE;
			}
			else {
				texOp = TX_ALPHABLEND;
			}
			for (int i = 0; i < nt; i++) {
				if (texHandle[i]) {
					pIHWMat->SetTexture(i, texHandle[i]->GetHandle());
					// Kludge to pass in the TextureStage number
					mtl->texture[0].useTex = i;
					cb.GetGfxTexInfoFromTexmap(t, mtl->texture[0], subTex[useSubForTex[i]]); 		
					SetHWTexOps(pIHWMat, i, texOp);
				}
			}
			return;
		}
		else {
			DiscardTexHandles();
		}

		int forceW = 0;
		int forceH = 0;

		int nsupport = cb.NumberTexturesSupported();

		nsupport = (nsupport > MAXTEXHANDLES) ? MAXTEXHANDLES : nsupport;

		numTexHandlesUsed = 0;

		for (int i = 0; i < MAXTEXHANDLES; i++) {
			texHandle[i] = NULL;
			sub[i] = NULL;
		}

		int nmaps = 0;
		for (i = 0; i < subTex.Count(); i++) {
			if (mapOn[i]) {
				if (subTex[i]) {
					sub[nmaps] = subTex[i];
					useSubForTex[nmaps] = i;
					if (++nmaps >= nsupport) {
						break;
					}
				}
			}
		}

		pIHWMat->SetNumTexStages(nmaps);
		for (i = 0; i < nmaps; i++) {
			pIHWMat->SetTexture(i, (DWORD_PTR)NULL);
		}

		numTexHandlesUsed  = nmaps;

		texHandleValid.SetInfinite();
		
		if (numTexHandlesUsed == 1) {
			texOp = TX_MODULATE;
		}
		else {
			texOp = TX_ALPHABLEND;
		}
		for (i = 0; i < numTexHandlesUsed; i++) {
			// Kludge to pass in the TextureStage number
			mtl->texture[0].useTex = i;
			cb.GetGfxTexInfoFromTexmap(t, mtl->texture[0], sub[i]);
			BITMAPINFO *bmi = sub[i]->GetVPDisplayDIB(t, cb, valid, FALSE, 0, 0);
			texHandle[i] = cb.MakeHandle(bmi);
			pIHWMat->SetTexture(i, texHandle[i]->GetHandle());
			SetHWTexOps(pIHWMat, i, texOp);
		}
	}
	else {
		if (texHandleValid.InInterval(t)) {
			mtl->texture.SetCount(numTexHandlesUsed);
			int nt = numTexHandlesUsed;
			for (int i=0; i<nt; i++) {
				if (texHandle[i]) {
					mtl->texture[i].textHandle = texHandle[i]->GetHandle();
					cb.GetGfxTexInfoFromTexmap(t, mtl->texture[i], subTex[useSubForTex[i]] ); 		
					SetTexOps(&mtl->texture[i],numTexHandlesUsed==1?TX_MODULATE:TX_ALPHABLEND);
				}
			}
			return;
		}
		else {
			DiscardTexHandles();
		}

		int forceW = 0;
		int forceH = 0;

		int nsupport = cb.NumberTexturesSupported();

		nsupport = (nsupport > MAXTEXHANDLES) ? MAXTEXHANDLES : nsupport;

		numTexHandlesUsed = 0;

		for (int i=0; i<MAXTEXHANDLES; i++) {
			texHandle[i] = NULL;
			sub[i]=NULL;
			}

		
		int nmaps = 0;
		for (i=0; i<subTex.Count(); i++) {
			if (mapOn[i]) {
				if (subTex[i]) {
					sub[nmaps] = subTex[i];
					useSubForTex[nmaps] = i;
					if (++nmaps>=nsupport)
						break;
					}
				}
			}

		mtl->texture.SetCount(nmaps);
		for (i=0; i<nmaps; i++) {
			mtl->texture[i].textHandle = NULL;
			}

		numTexHandlesUsed  = nmaps;

		texHandleValid.SetInfinite();
		
		for (i=0; i<nmaps; i++) {
			cb.GetGfxTexInfoFromTexmap(t, mtl->texture[i], sub[i] ); 		
			BITMAPINFO *bmi = sub[i]->GetVPDisplayDIB(t,cb,valid,FALSE,0,0); 
			texHandle[i] = cb.MakeHandle(bmi); 
			mtl->texture[i].textHandle = texHandle[i]->GetHandle();
			SetTexOps(&mtl->texture[i],nmaps==1?TX_MODULATE:TX_ALPHABLEND);
			}
		mtl->texture.SetCount(nmaps);
	}
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// This map is not meaningful unless all of its submaps are on. 
//
bool Composite::IsLocalOutputMeaningful( ShadeContext& sc )
{
	for ( int i = 0; i < NumSubTexmaps(); i++ ) 
	{
		if ( SubTexmapOn( i ) && ( GetSubTexmap( i ) != NULL ) )
			return true;
	}
	
	return false;
}
