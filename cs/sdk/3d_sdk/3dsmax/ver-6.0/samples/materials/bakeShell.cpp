/**********************************************************************
 *<
	FILE: BakeShell.cpp

	DESCRIPTION:  A selector for an original & a baked material

	CREATED BY: Kells Elmquist

	HISTORY:

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "stdmat.h"
#include "iparamm2.h"

extern HINSTANCE hInstance;

static Class_ID bakeShellClassID(BAKE_SHELL_CLASS_ID,0);

#define NSUBMTL 2

#define PB_REF		0
#define ORIG_REF	1
#define BAKED_REF	2

class BakeShellDlgProc;

class BakeShell : public Mtl, public IReshading  {	
	public:
		IParamBlock2			*mpBlock; 			// ref #0
		Mtl						*mpOrigMtl;			// ref #1
		Mtl						*mpBakedMtl;		// ref #2		
		Interval				 mValidInt;
		ReshadeRequirements		 mReshadeRQ;
		
		static BakeShellDlgProc *mpParamDlg;

		// methods
		BakeShell(BOOL loading);
		void NotifyChanged() {NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);}
		Mtl *UseMtl();

		// From MtlBase and Mtl
		void SetAmbient(Color c, TimeValue t) {}		
		void SetDiffuse(Color c, TimeValue t) {}		
		void SetSpecular(Color c, TimeValue t) {}
		void SetShininess(float v, TimeValue t) {}				
		
		Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
	    Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
		Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
		float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
		float GetShininess(int mtlNum=0, BOOL backFace=FALSE);		
		float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
		float WireSize(int mtlNum=0, BOOL backFace=FALSE);
				
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void EnableStuff();
		
		void Shade(ShadeContext& sc);
		float EvalDisplacement(ShadeContext& sc); 
		Interval DisplacementValidity(TimeValue t); 
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t);
		
		Class_ID ClassID()			{ return bakeShellClassID; }
		SClass_ID SuperClassID()	{ return MATERIAL_CLASS_ID;}
		void GetClassName(TSTR& s)	{ s=GetString(IDS_KE_BAKE_SHELL); }  

		void DeleteThis()			{ delete this; }	

		Sampler*  GetPixelSampler(int mtlNum, BOOL backFace );

	// Methods to access sub-materials of meta-materials
		int NumSubMtls(){ return 2; }
		Mtl* GetSubMtl(int i){ return i? mpBakedMtl : mpOrigMtl;}
		void SetSubMtl(int i, Mtl *m);
		int VPDisplaySubMtl();
		TSTR GetSubMtlSlotName(int i) {
			return TSTR( i ? 
				 GetString(IDS_KE_BAKED_MTL):GetString(IDS_KE_ORIG_MTL)
			);
		}
		// Methods to access sub texture maps of material or texmap
		int NumSubTexmaps()					 { return 0; }
		Texmap* GetSubTexmap(int i)			 { return NULL; } 
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i)	 { return _T( "" ); }

		int NumSubs()						 { return 3; } 
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum)		 { return subNum; }

		// From ref
		int NumRefs()						 { return 3; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags); 

		// IO
		IOResult Save(ISave *isave); 
		IOResult Load(ILoad *iload); 

		// direct ParamBlock access
		int	NumParamBlocks()				{ return 1; }		// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i)	{ return mpBlock; }	// return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (mpBlock->ID() == id) ? mpBlock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		// begin - reshading code
		BOOL SupportsRenderElements(){ return TRUE; }
		ULONG Requirements(int subMtlNum); 
		ReshadeRequirements GetReshadeRequirements() { return mReshadeRQ; } // mjm - 06.02.00
		void PreShade(ShadeContext& sc, IReshadeFragment* pFrag);
		void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip);
		// end - reshading code

		// From Mtl
		bool IsOutputConst( ShadeContext& sc, int stdID );
		bool EvalColorStdChannel( ShadeContext& sc, int stdID, Color& outClr );
		bool EvalMonoStdChannel( ShadeContext& sc, int stdID, float& outVal );

		void* GetInterface(ULONG id);
};

// the one instance of the paramDlg
BakeShellDlgProc* BakeShell::mpParamDlg;


class BakeShellClassDesc:public ClassDesc2 {
	public:
	int		 		IsPublic(){ return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading){ return new BakeShell(loading); }
	const TCHAR *	ClassName() {return GetString(IDS_KE_BAKE_SHELL); } 
	SClass_ID		SuperClassID() {return MATERIAL_CLASS_ID;}
	Class_ID 		ClassID() {return bakeShellClassID;}
	const TCHAR* 	Category() {return _T("");}
	//  internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("bakeShell"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};

static BakeShellClassDesc _bakeShellCD;
ClassDesc* GetBakeShellDesc(){ return &_bakeShellCD; }

enum { bakeShell_params };  // mpBlock ID
// BakeShell_params param IDs


enum 
{ 
	bakeShell_vp_n_mtl, bakeShell_render_n_mtl,
	bakeShell_orig_mtl, bakeShell_baked_mtl
};

class ShellPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BakeShell* mtl = (BakeShell*)owner;
		switch (id)
		{
			// use item data to unscramble sorted lists
			case bakeShell_orig_mtl: {
				mtl->ReplaceReference( ORIG_REF, v.r );

			} break;

			case bakeShell_baked_mtl: {
				mtl->ReplaceReference( BAKED_REF, v.r );
			} break;

		} // end, switch
	} // end, set accessor

};

static ShellPBAccessor shellPBAccessor;


static ParamBlockDesc2 bakeShell_param_blk ( bakeShell_params, _T("parameters"),  0, &_bakeShellCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PB_REF, 
	//rollout
	IDD_BAKE_SHELL, IDS_KE_BAKE_SHELL_PARAMS, 0, 0, NULL, 
	// params
	bakeShell_orig_mtl,		_T("originalMaterial"),	TYPE_MTL, P_OWNERS_REF,	IDS_KE_ORIG_MTL,
		p_refno,		ORIG_REF,
		p_submtlno,		0,		
		p_ui,			TYPE_MTLBUTTON, IDC_ORIG_MTL,
		end,
	bakeShell_baked_mtl, _T("bakedMaterial"), TYPE_MTL, P_OWNERS_REF, IDS_KE_BAKED_MTL,
		p_refno,		BAKED_REF,
		p_submtlno,		1,		
		p_ui,			TYPE_MTLBUTTON, IDC_BAKED_MTL,
		end,
	bakeShell_vp_n_mtl, _T("viewportMtlIndex"), TYPE_INT,	0,	IDS_KE_VP_MTL,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_BAKESHELL_VP_USE1, IDC_BAKESHELL_VP_USE2,
		end,
	bakeShell_render_n_mtl, _T("renderMtlIndex"), TYPE_INT,	0,	IDS_KE_REND_MTL,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_BAKESHELL_REND_USE1, IDC_BAKESHELL_REND_USE2,
		end,

	end
);


//dialog stuff to get the Set Ref button
class BakeShellDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		BakeShell *bakeShell;		
		BOOL valid;
		HWND hPanel; 
		BakeShellDlgProc(BakeShell *m) {
			bakeShell = m;
			valid   = FALSE;
		}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}

};


BOOL BakeShellDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	Rect rect;
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
	bakeShell = (BakeShell*)map->GetParamBlock()->GetOwner(); 
	switch (msg) {
		case WM_PAINT: {
//			em->EnableAffectRegion (t);
//			PAINTSTRUCT ps;
//			HDC hdc = BeginPaint(hWnd,&ps);
//			DrawCurve(hWnd,hdc,bakeShell);
//			EndPaint(hWnd,&ps);
			return FALSE;
			}

		case WM_COMMAND:  
//		    switch (id) {
//				case IDC_BakeShell_MAP:
//					bakeShell->EnableStuff(); 
//					break;
//				}
			break;							
		case CC_SPINNER_CHANGE:
//			bakeShell->Update(GetCOREInterface()->GetTime(),FOREVER);
//			GetClientRectP(GetDlgItem(hWnd,IDC_MIXCURVE),&rect);
//			InvalidateRect(hWnd,&rect,FALSE);
			return FALSE;
			break;

		default:
			return FALSE;
		}
	return FALSE;
	}



//--- Texture Baking Shell Material -------------------------------------------------



BakeShell::BakeShell(BOOL loading) : mReshadeRQ(RR_None) // mjm - 06.02.00
{	
	mpBlock = NULL;
	mpOrigMtl = mpBakedMtl = NULL;	
	mValidInt.SetEmpty();
	if (!loading) {
		_bakeShellCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
		Init();
	}
}

void BakeShell::Init()
{
	_bakeShellCD.Reset(this, TRUE);	// reset all pb2's
	ReplaceReference( ORIG_REF, NewDefaultStdMat() );

	// this shd be the hw material eventually
	ReplaceReference( BAKED_REF, NewDefaultStdMat() );

	GetCOREInterface()->AssignNewName(mpOrigMtl);
	GetCOREInterface()->AssignNewName(mpBakedMtl);
}

void BakeShell::Reset()
{
	Init();
}

void* BakeShell::GetInterface(ULONG id)
{
	if( id == IID_IReshading )
		return (IReshading*)( this );
	else if ( id == IID_IValidityToken )
		return (IValidityToken*)( this );
	else
		return Mtl::GetInterface(id);
}

int BakeShell::VPDisplaySubMtl() 
{
	int m;
	mpBlock->GetValue(bakeShell_vp_n_mtl, 0, m, FOREVER);
	return m; 
} 

Mtl *BakeShell::UseMtl() 
{
	int n;
	mpBlock->GetValue(bakeShell_vp_n_mtl,0, n, FOREVER);
	if (n==0 && mpOrigMtl) 
		return mpOrigMtl;
	if (n==1 && mpBakedMtl)
		return mpBakedMtl;

	return mpOrigMtl? mpOrigMtl : mpBakedMtl;
}

Color BakeShell::GetAmbient(int mtlNum, BOOL backFace) { 
	return UseMtl()?UseMtl()->GetAmbient(mtlNum,backFace):Color(0,0,0);
}		
Color BakeShell::GetDiffuse(int mtlNum, BOOL backFace){ 
	return UseMtl()?UseMtl()->GetDiffuse(mtlNum,backFace):Color(0,0,0);
}				
Color BakeShell::GetSpecular(int mtlNum, BOOL backFace){
	return UseMtl()?UseMtl()->GetSpecular(mtlNum,backFace):Color(0,0,0);
}		
float BakeShell::GetXParency(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetXParency(mtlNum,backFace):0.0f;
}
float BakeShell::GetShininess(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetXParency(mtlNum,backFace):0.0f;
}		
float BakeShell::GetShinStr(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetXParency(mtlNum,backFace):0.0f;
}

float BakeShell::WireSize(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->WireSize(mtlNum,backFace):0.0f;
}

		
ParamDlg* BakeShell::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{
// JBW: the main difference here is the automatic creation of a ParamDlg by the new
// ClassDesc2 function CreateParamDlgs().  This mirrors the way BeginEditParams()
// can be redirected to the ClassDesc2 for automatic ParamMap2 management.  In this 
// case a special subclass of ParamDlg, AutoMParamDlg, defined in IParamm2.h, is 
// created.  It can act as a 'master' ParamDlg to which you can add any number of 
// secondary dialogs and it will make sure all the secondary dialogs are kept 
// up-to-date and deleted as necessary.  

	// create the rollout dialogs
	IAutoMParamDlg* masterDlg = _bakeShellCD.CreateParamDlgs(hwMtlEdit, imp, this);
	mpParamDlg = new BakeShellDlgProc(this);
	bakeShell_param_blk.SetUserDlgProc( mpParamDlg );
	Update(imp->GetTime(), FOREVER);
	return masterDlg;
}

BOOL BakeShell::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	return FALSE;
}


void BakeShell::SetSubTexmap(int i, Texmap *m){}

void BakeShell::SetSubMtl(int i, Mtl *m)
{
	ReplaceReference(i+1,m);
	if (i==0) {
		bakeShell_param_blk.InvalidateUI(bakeShell_orig_mtl);
		mValidInt.SetEmpty();
	}	
	else if (i==1) {
		bakeShell_param_blk.InvalidateUI(bakeShell_baked_mtl);
		mValidInt.SetEmpty();
	}	
}


static Color black(0,0,0);

ULONG BakeShell::Requirements(int subMtlNum) 
{
	 int useMtl;
	 mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
	 Mtl *mtl = (useMtl == 0 && mpOrigMtl) ? mpOrigMtl : mpBakedMtl;
	 ULONG req = 0;
	 if (mtl != NULL)
		req = mtl->Requirements(subMtlNum);
	 return req | LocalRequirements(subMtlNum);
}


// my version 12.4.00,  kae need to save mix & get it
void BakeShell::PreShade(ShadeContext& sc, IReshadeFragment* pFrag)
{
	IReshading* pReshading = NULL;

	int lenChan = pFrag->NTextures(); // indx of len channel
	pFrag->AddIntChannel(0);//placeholder

	int mtlLength = 0;
	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );

	if ( useMtl == 0 && mpOrigMtl ){
		pReshading = (IReshading*)(mpOrigMtl->GetInterface(IID_IReshading));
	} else if( mpBakedMtl ){
		pReshading = (IReshading*)(mpBakedMtl->GetInterface(IID_IReshading));
	}
	if( pReshading ) {
		pReshading->PreShade(sc, pFrag);
		mtlLength = pFrag->NTextures() - lenChan - 1;
	}
	// save the length into the int channel
	pFrag->SetIntChannel( lenChan, mtlLength );
}

void BakeShell::PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams*)
{
	IReshading* pReshading;
	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );

	Mtl *mtl = useMtl == 0 ? mpOrigMtl : mpBakedMtl;
	int mtlLength = pFrag->GetIntChannel(nextTexIndex++);
 
	ShadeOutput out1;
	if( mtl ){
		sc.ResetOutput();
		pReshading = (IReshading*)(mtl->GetInterface(IID_IReshading));
		if( pReshading ) 
			pReshading->PostShade(sc, pFrag, nextTexIndex);
	} else {
		// i think this is always an error
		nextTexIndex += mtlLength;
	}
}

Sampler*  BakeShell::GetPixelSampler(int mtlNum, BOOL backFace )
{
	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
	Mtl *mtl = (useMtl == 0) ? mpOrigMtl : mpBakedMtl;

	Sampler* pSampler = NULL;
	if( mtl )
		pSampler = mtl->GetPixelSampler( mtlNum, backFace );
	
	if ( !pSampler ){
		mtl = (useMtl == 1) ? mpOrigMtl : mpBakedMtl; // other mtl
		pSampler = mtl->GetPixelSampler( mtlNum, backFace );
	}
	
	return pSampler;
}

// if this function changes, please also check SupportsReShading, PreShade and PostShade
// end - ke/mjm - 03.16.00 - merge reshading code
// [attilas|24.5.2000] if this function changes, please also check EvalColorStdChannel
void BakeShell::Shade(ShadeContext& sc)
{

	if( gbufID )
		sc.SetGBufferID(gbufID);

	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
	Mtl *mtl = useMtl == 0 ? mpOrigMtl : mpBakedMtl;
	if( mtl )
		mtl->Shade(sc);
}

void BakeShell::Update(TimeValue t, Interval& valid)
{	
	mValidInt = FOREVER;
	if (mpOrigMtl) mpOrigMtl->Update(t,valid);
	if (mpBakedMtl) mpBakedMtl->Update(t,valid);
//	mpBlock->GetValue(bakeShell_mix,t,u,mValidInt);
//	valid &= mValidInt;
}

Interval BakeShell::Validity(TimeValue t)
{
	Interval valid = FOREVER;		
	if (mpOrigMtl) valid &= mpOrigMtl->Validity(t);
	if (mpBakedMtl) valid &= mpBakedMtl->Validity(t);
	return valid;
}

Animatable* BakeShell::SubAnim(int i)
{
	switch (i) {
		case 0: return mpBlock;
		case 1: return mpOrigMtl;
		case 2: return mpBakedMtl;
		default: return NULL;
	}
}

TSTR BakeShell::SubAnimName(int i)
{
	switch (i) {
		case 0: return GetString(IDS_DS_PARAMETERS);
		case 1: return GetSubMtlTVName(0); 
		case 2: return GetSubMtlTVName(1); 
		default: return _T("");
	}
}

RefTargetHandle BakeShell::GetReference(int i)
{
	switch (i) {
		case 0: return mpBlock;
		case 1: return mpOrigMtl;
		case 2: return mpBakedMtl;
		default: return NULL;
	}
}

void BakeShell::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
		case 0: mpBlock = (IParamBlock2*)rtarg; break;
		case 1: mpOrigMtl = (Mtl*)rtarg; break;
		case 2: mpBakedMtl = (Mtl*)rtarg; break;
	}
}

RefTargetHandle BakeShell::Clone(RemapDir &remap)
{
	BakeShell *mtl = new BakeShell(FALSE);
	*((MtlBase*)mtl) = *((MtlBase*)this);  // copy superclass stuff
	mtl->ReplaceReference( PB_REF, remap.CloneRef(mpBlock) );
	if (mpOrigMtl) mtl->ReplaceReference( ORIG_REF,remap.CloneRef(mpOrigMtl));
	if (mpBakedMtl) mtl->ReplaceReference( BAKED_REF,remap.CloneRef(mpBakedMtl));
	BaseClone(this, mtl, remap);	// ???? needed
	return mtl;
}

RefResult BakeShell::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			mValidInt.SetEmpty();
			if (hTarget == mpBlock)
			{
				ParamID changing_param = mpBlock->LastNotifyParamID();
				bakeShell_param_blk.InvalidateUI(changing_param);

				if((changing_param == bakeShell_vp_n_mtl)
					||(changing_param == bakeShell_render_n_mtl)
					||(changing_param == bakeShell_orig_mtl)
					||(changing_param == bakeShell_baked_mtl)
					)
					mReshadeRQ = RR_NeedReshade;
			}

//			if (hTarget==mpBlock) mValidInt.SetEmpty();

			if (hTarget != NULL) {
				switch (hTarget->SuperClassID()) {
					case MATERIAL_CLASS_ID: {
						IReshading* r = static_cast<IReshading*>(hTarget->GetInterface(IID_IReshading));
						mReshadeRQ = (r == NULL)? RR_None : r->GetReshadeRequirements();
					} break;
				}
			}
			break;
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			mReshadeRQ = RR_NeedPreshade;
			NotifyChanged();
			break;

	}// end, switch
	return REF_SUCCEED;
}

/////////////////////////////////////////////////////////////////
//
//	Save & Load
//
// Note: ALL Materials and texmaps must have a Save and Load to save and load
// the MtlBase info.
#define HEADER_CHUNK 0x4000
#define VERSION_CHUNK 0x1000

#define CURRENT_BAKESHELL_VERSION 0x1

IOResult BakeShell::Save(ISave *isave) 
{ 
	IOResult res;

	// Save common stuff
	isave->BeginChunk(HEADER_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(VERSION_CHUNK);
	int version = CURRENT_BAKESHELL_VERSION;
	ULONG	nBytes;
	isave->Write( &version, sizeof(version), &nBytes );			
	isave->EndChunk();

	return IO_OK;
}	
	  

class BakeShellPostLoad : public PostLoadCallback {
	public:
		BakeShell *n;
		BakeShellPostLoad(BakeShell *ns) {n = ns;}
		void proc(ILoad *iload) { 
			// your goop here
			delete this; 
		} 
};


IOResult BakeShell::Load(ILoad *iload) 
{ 
	IOResult res;
	int version;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch( iload->CurChunkID() ){
			case HEADER_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case VERSION_CHUNK:
				ULONG nb;
				res = iload->Read(&version, sizeof(version), &nb);
				break;
		} // end switch
		
		iload->CloseChunk();
	
		if( res != IO_OK ) 
			return res;
	
	} // end, while still chunks

	// if need be register version converter
	// if( version < CURRENT_BAKESHELL_VERSION )
	//		iload->RegisterPostLoadCallback( new BakeShellPostLoad(this) );

	return IO_OK;
}

/////////////////////////////////////////////////////////////////
//
//	Channel Evaluations
//

float BakeShell::EvalDisplacement(ShadeContext& sc) 
{
	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
	Mtl *mtl = (useMtl == 0) ? mpOrigMtl : mpBakedMtl;

	if( mtl ){
		return mtl->EvalDisplacement(sc);
	}
	else {
		mtl = (useMtl == 0) ? mpBakedMtl : mpOrigMtl;	// other material
		return mtl ? mtl->EvalDisplacement(sc) : 0.0f;
	}
}

Interval BakeShell::DisplacementValidity(TimeValue t)
{
	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
	Mtl *mtl = (useMtl == 0) ? mpOrigMtl : mpBakedMtl;

	Interval iv;
	iv.SetInfinite();

	if( mtl ){
		iv &= mtl->DisplacementValidity( t );
	}
	else {
		// missing render mtl
		mtl = (useMtl == 0) ? mpBakedMtl : mpOrigMtl;	// other material
		if( mtl )
			iv &= mtl->DisplacementValidity( t );
	}

	return iv;
} 

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// A BakeShell is constant if the render submtl is constant
//
bool BakeShell::IsOutputConst
( 
	ShadeContext& sc,	// describes context of evaluation
	int stdID			// must be ID_AM, etc from stdmat.h
)
{
	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
	Mtl *mtl = (useMtl == 0) ? mpOrigMtl : mpBakedMtl;

	if ( mtl && !mtl->IsOutputConst( sc, stdID ) ) 
		return false;

	return true;
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Evaluates the material on a single texmap channel. 
// For a mono channel, the value is copied in all 3 components of the 
// output color.
// 
bool BakeShell::EvalColorStdChannel
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID,				// must be ID_AM, ect
	Color& outClr			// output var
)
{
	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
	Mtl *mtl = (useMtl == 0) ? mpOrigMtl : mpBakedMtl;

	bool bRes = false;

	if ( mtl ){
		bRes = mtl->EvalColorStdChannel(sc, stdID, outClr);
	} 
	else {
		// missing render mtl
		mtl = (useMtl == 0) ? mpBakedMtl : mpOrigMtl;	// other material
		if( mtl )
			bRes = mtl->EvalColorStdChannel(sc, stdID, outClr);
	} 

	return bRes;
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Evaluates the material on a single texmap channel. 
//
bool BakeShell::EvalMonoStdChannel
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID,				// must be ID_AM, ect
	float& outVal			// output var
)
{
	int useMtl;
	mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
	Mtl *mtl = (useMtl == 0) ? mpOrigMtl : mpBakedMtl;

	bool bRes = false;

	if ( mtl ){
		bRes = mtl->EvalMonoStdChannel(sc, stdID, outVal);
	} 
	else {
		// missing render mtl
		mtl = (useMtl == 0) ? mpBakedMtl : mpOrigMtl;	// other material
		if( mtl )
			bRes = mtl->EvalMonoStdChannel(sc, stdID, outVal);
	} 

	return bRes;
}	

void BakeShell::EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) 
{
	if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return;
	if (flags&FILE_ENUM_SKIP_VPRENDER_ONLY)	
	{
		int useMtl;
		mpBlock->GetValue( bakeShell_render_n_mtl, 0, useMtl, FOREVER );
		Mtl *mtl = useMtl == 0 ? mpOrigMtl : mpBakedMtl;
		if( mtl )
			mtl->EnumAuxFiles(nameEnum,flags);
		Animatable::EnumAuxFiles(nameEnum,flags); // pick up cust attributes
	}
	else
		Mtl::EnumAuxFiles( nameEnum, flags ); // will pick up both materials as references
}
