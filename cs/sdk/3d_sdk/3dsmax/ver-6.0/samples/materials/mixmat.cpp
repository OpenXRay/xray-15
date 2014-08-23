/**********************************************************************
 *<
	FILE: mixmat.cpp

	DESCRIPTION:  A blend of two materials

	CREATED BY: Rolf Berteig

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
// begin - ke/mjm - 03.16.00 - merge reshading code
//#include "iReshade.h"
// end - ke/mjm - 03.16.00 - merge reshading code

extern HINSTANCE hInstance;

static Class_ID mixmatClassID(MIXMAT_CLASS_ID,0);

#define NSUBMTL 2
class MixMat;

#define PB_REF		0
#define SUB1_REF	1
#define SUB2_REF	2
#define MAP_REF		3

class MixMatDlgProc;


class MixMat : public Mtl, public IReshading  {	
	public:
		IParamBlock2 *pblock; 	// ref #0
		Mtl *sub1, *sub2;		// ref #1, 2		
		Texmap *map;			// ref #3
		BOOL mapOn[3];
		float u, crvA, crvB;
		BOOL useCurve;		
		Interval ivalid;
		ReshadeRequirements mReshadeRQ; // mjm - 06.02.00
		BOOL Param1;

		static MixMatDlgProc *paramDlg;
		// [attilas|29.5.2000]
		// min and max Mix tresholds
		static const float minMix;
		static const float maxMix;

		MixMat(BOOL loading);
		void NotifyChanged() {NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);}
		Mtl *UseMtl();
		float mixCurve(float x);

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
		
		Class_ID ClassID() {return mixmatClassID; }
		SClass_ID SuperClassID() {return MATERIAL_CLASS_ID;}
		void GetClassName(TSTR& s) {s=GetString(IDS_RB_BLENDMAT);}  

		void DeleteThis() {delete this;}	

		Sampler*  GetPixelSampler(int mtlNum, BOOL backFace );

	// Methods to access sub-materials of meta-materials
	   	int NumSubMtls() {return 2;}
		Mtl* GetSubMtl(int i) {return i?sub2:sub1;}
		void SetSubMtl(int i, Mtl *m);
		int VPDisplaySubMtl();
		TSTR GetSubMtlSlotName(int i) {return i?GetString(IDS_RB_MATERIALTWO):GetString(IDS_RB_MATERIALONE);}

		// Methods to access sub texture maps of material or texmap
		int NumSubTexmaps() {return 1;}
		Texmap* GetSubTexmap(int i) {return map;}
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i) {return GetString(IDS_RB_MASK);}

		int NumSubs() {return 4;} 
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) {return subNum;}

		// From ref
 		int NumRefs() {return 4;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		// IO
		IOResult Save(ISave *isave); 
		IOResult Load(ILoad *iload); 

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

// begin - ke/mjm - 03.16.00 - merge reshading code
		BOOL SupportsRenderElements(){ return TRUE; }
//		BOOL SupportsReShading(ShadeContext& sc);
		ReshadeRequirements GetReshadeRequirements() { return mReshadeRQ; } // mjm - 06.02.00
		void PreShade(ShadeContext& sc, IReshadeFragment* pFrag);
		void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip);
// end - ke/mjm - 03.16.00 - merge reshading code

		// From Mtl
		bool IsOutputConst( ShadeContext& sc, int stdID );
		bool EvalColorStdChannel( ShadeContext& sc, int stdID, Color& outClr );
		bool EvalMonoStdChannel( ShadeContext& sc, int stdID, float& outVal );

		void* GetInterface(ULONG id);
	};

MixMatDlgProc* MixMat::paramDlg;
const float MixMat::minMix = 0.0001f;
const float MixMat::maxMix = 0.9999f;



class MixMatClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return GetAppID() != kAPP_VIZR;}
	void *			Create(BOOL loading) {return new MixMat(loading);}
	const TCHAR *	ClassName() {return GetString(IDS_RB_BLENDMAT_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() {return MATERIAL_CLASS_ID;}
	Class_ID 		ClassID() {return mixmatClassID;}
	const TCHAR* 	Category() {return _T("");}
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("Blend"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};
static MixMatClassDesc mixmatCD;
ClassDesc* GetMixMatDesc() {return &mixmatCD;}


enum { mixmat_params };  // pblock ID
// mixmat_params param IDs


enum 
{ 
	mixmat_mix, mixmat_curvea, mixmat_curveb, mixmat_usecurve, mixmat_usemat,
	mixmat_map1, mixmat_map2, mixmat_mask,		
	mixmat_map1_on, mixmat_map2_on,  mixmat_mask_on // main grad params 
};


//JBW: here is the new ParamBlock2 descriptor. There is only one block for Gradients, a per-instance block.
// for the moment, some of the parameters a Tab<>s to test the Tab system.  Aslo note that all the References kept
// kept in a Gradient are mapped here, marked as P_OWNERS_REF so that the paramblock accesses and maintains them
// as references on owning Gradient.  You need to specify the refno for these owner referencetarget parameters.
// I even went so far as to expose the UVW mapping and Texture Output sub-objects this way so that they can be
// seen by the scripter and the schema-viewer

// per instance gradient block

static ParamBlockDesc2 mixmat_param_blk ( mixmat_params, _T("parameters"),  0, &mixmatCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PB_REF, 
	//rollout
	IDD_MIXMAT, IDS_DS_BLEND_PARAMS, 0, 0, NULL, 
	// params
	mixmat_mix, _T("mixAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE,	IDS_PW_MIXAMOUNT,
		p_default,		0.0f,
		p_range,		0.0f, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIXMAT_MIX, IDC_MIXMAT_MIXSPIN, 0.1f, 
		end,
	mixmat_curvea, _T("lower"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_LOWER,
		p_default,		0.25f,
		p_range,		0.0f, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIXA_EDIT, IDC_MIXA_SPIN, 0.01f, 
		end,
	mixmat_curveb, _T("upper"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_UPPER,
		p_default,		0.75f,
		p_range,		0.0f, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIXB_EDIT, IDC_MIXB_SPIN, 0.01f, 
		end,
	mixmat_usecurve,	_T("useCurve"), TYPE_BOOL,			0,				IDS_PW_USECURVE,
		p_default,		FALSE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MIX_USECURVE,
		end,
	mixmat_usemat, _T("interactive"), TYPE_INT,		0,				IDS_PW_INTERACTIVE,
		p_default,		0,
		p_range,		0,	2,
		p_ui,			TYPE_RADIO, 3, IDC_MIXMAT_USE1, IDC_MIXMAT_USE2,IDC_MIXMAT_USE3,
		end,
	mixmat_map1,		_T("map1"),		TYPE_MTL,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		SUB1_REF,
		p_submtlno,		0,		
		p_ui,			TYPE_MTLBUTTON, IDC_MIXMAT_MAT1,
		end,
	mixmat_map2,		_T("map2"),		TYPE_MTL,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		SUB2_REF,
		p_submtlno,		1,		
		p_ui,			TYPE_MTLBUTTON, IDC_MIXMAT_MAT2,
		end,
	mixmat_mask,		_T("mask"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_DS_MASK,
		p_refno,		MAP_REF,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MIXMAT_MAP,
		end,
	mixmat_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	mixmat_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	mixmat_mask_on,	_T("maskEnabled"), TYPE_BOOL,			0,				IDS_PW_MASKENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON3,
		end,

	end
);




//dialog stuff to get the Set Ref button
class MixMatDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		MixMat *mixmat;		
		BOOL valid;
		HWND hPanel; 
		MixMatDlgProc(MixMat *m) {
			mixmat = m;
			valid   = FALSE;
			}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			mixmat = (MixMat*)m;
			mixmat->EnableStuff();
			}

	};


static void DrawCurve (HWND hWnd,HDC hdc, MixMat *mixmat) {

	Rect rect, orect;
	GetClientRectP(GetDlgItem(hWnd,IDC_MIXCURVE),&rect);
	orect = rect;

	SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	
	SelectObject(hdc,GetStockObject(BLACK_PEN));
	MoveToEx(hdc,rect.left+1, rect.bottom-1,NULL); 
	float fx,fy;
	int ix,iy;
	float w = (float)rect.w()-2;
	float h = (float)rect.h()-2;
	for (ix =0; ix<w; ix++) {
		fx = (float(ix)+0.5f)/w;
		fy = mixmat->mixCurve(fx);
		iy = int(h*fy+0.5f);
		LineTo(hdc, rect.left+1+ix, rect.bottom-1-iy);
		}
	WhiteRect3D(hdc,orect,TRUE);

}


BOOL MixMatDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	Rect rect;
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
	mixmat = (MixMat*)map->GetParamBlock()->GetOwner(); 
	switch (msg) {
		case WM_PAINT: {
//			em->EnableAffectRegion (t);
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd,&ps);
			DrawCurve(hWnd,hdc,mixmat);
			EndPaint(hWnd,&ps);
			return FALSE;
			}

		case WM_COMMAND:  
		    switch (id) {
				case IDC_MIXMAT_MAP:
					mixmat->EnableStuff(); 
					break;
				}
			break;							
		case CC_SPINNER_CHANGE:
			mixmat->Update(GetCOREInterface()->GetTime(),FOREVER);
			GetClientRectP(GetDlgItem(hWnd,IDC_MIXCURVE),&rect);
			InvalidateRect(hWnd,&rect,FALSE);
			return FALSE;
			break;

		default:
			return FALSE;
		}
	return FALSE;
	}



//--- Blend Material -------------------------------------------------


static ParamBlockDesc pbdesc[] = {
	{TYPE_FLOAT, NULL, TRUE },	// Mix Amount
	{TYPE_FLOAT, NULL, TRUE },	// Curve A	
	{TYPE_FLOAT, NULL, TRUE },  // Curve B
	{TYPE_INT, NULL, FALSE },	// Use curve
	{TYPE_INT, NULL, FALSE }};  // Use mat
#define PBLOCK_LENGTH	5



static ParamBlockDescID pbdesc1[] = {
	{TYPE_FLOAT, NULL, TRUE ,mixmat_mix},	// Mix Amount
	{TYPE_FLOAT, NULL, TRUE ,mixmat_curvea},	// Curve A	
	{TYPE_FLOAT, NULL, TRUE ,mixmat_curveb},  // Curve B
	{TYPE_INT, NULL, FALSE,mixmat_usecurve },	// Use curve
	{TYPE_INT, NULL, FALSE,mixmat_usemat }};  // Use mat

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc1,5,0),	// Version 1 params
	};
#define NUM_OLDVERSIONS	1


MixMat::MixMat(BOOL loading) : mReshadeRQ(RR_None) // mjm - 06.02.00
	{	
	Param1 = FALSE;
	pblock = NULL;
	sub1 = sub2 = NULL;	
	map = NULL;
	ivalid.SetEmpty();
	for (int i=0; i<3; i++) 
		mapOn[i] = 1;
	if (!loading) {
		mixmatCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
		Init();
		}
	}

void MixMat::Init()
	{
	mixmatCD.Reset(this, TRUE);	// reset all pb2's
	pblock->SetValue(mixmat_curveb,0,0.75f);	
	ReplaceReference(SUB1_REF,NewDefaultStdMat());
	ReplaceReference(SUB2_REF,NewDefaultStdMat());
	GetCOREInterface()->AssignNewName(sub1);
	GetCOREInterface()->AssignNewName(sub2);
	}

void MixMat::Reset()
	{
	mixmatCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	DeleteReference(MAP_REF);
	}

void* MixMat::GetInterface(ULONG id)
{
	if( id == IID_IReshading )
		return (IReshading*)( this );
	else if ( id == IID_IValidityToken )
		return (IValidityToken*)( this );
	else
		return Mtl::GetInterface(id);
}

int MixMat::VPDisplaySubMtl() {
	int m;
	pblock->GetValue(mixmat_usemat,0,m,FOREVER);

	if(m==2)// Display the mask map
		return -1;
	else
	return m; 
	} 

Mtl *MixMat::UseMtl() 
	{
	int m;
	pblock->GetValue(mixmat_usemat,0,m,FOREVER);
	if (m==0 && sub1) return sub1;
	if (m==1 && sub2) return sub2;
	return sub1?sub1:sub2;
	}

Color MixMat::GetAmbient(int mtlNum, BOOL backFace) { 
	return UseMtl()?UseMtl()->GetAmbient(mtlNum,backFace):Color(0,0,0);
	}		
Color MixMat::GetDiffuse(int mtlNum, BOOL backFace){ 
	return UseMtl()?UseMtl()->GetDiffuse(mtlNum,backFace):Color(0,0,0);
	}				
Color MixMat::GetSpecular(int mtlNum, BOOL backFace){
	return UseMtl()?UseMtl()->GetSpecular(mtlNum,backFace):Color(0,0,0);
	}		
float MixMat::GetXParency(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetXParency(mtlNum,backFace):0.0f;
	}
float MixMat::GetShininess(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetXParency(mtlNum,backFace):0.0f;
	}		
float MixMat::GetShinStr(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetXParency(mtlNum,backFace):0.0f;
	}

float MixMat::WireSize(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->WireSize(mtlNum,backFace):0.0f;
	}

void MixMat::EnableStuff() {
	if (pblock) {
		IParamMap2 *pmap = pblock->GetMap();
		if (pmap) {
			Texmap *mp = mapOn[2]?map:NULL;
			pmap->Enable(mixmat_mix, mp? FALSE: TRUE);
			pmap->Enable(mixmat_usecurve, mp?TRUE:FALSE);
			}
		}
	} 
		
ParamDlg* MixMat::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
	{
// JBW: the main difference here is the automatic creation of a ParamDlg by the new
// ClassDesc2 function CreateParamDlgs().  This mirrors the way BeginEditParams()
// can be redirected to the ClassDesc2 for automatic ParamMap2 management.  In this 
// case a special subclass of ParamDlg, AutoMParamDlg, defined in IParamm2.h, is 
// created.  It can act as a 'master' ParamDlg to which you can add any number of 
// secondary dialogs and it will make sure all the secondary dialogs are kept 
// up-to-date and deleted as necessary.  

	// create the rollout dialogs
	IAutoMParamDlg* masterDlg = mixmatCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	paramDlg = new MixMatDlgProc(this);
	mixmat_param_blk.SetUserDlgProc(paramDlg);
	EnableStuff();
	Update(imp->GetTime(), FOREVER);
	return masterDlg;


	}

BOOL MixMat::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
//	paramDlg->mixmat = this;
	return FALSE;

}


void MixMat::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(MAP_REF,m);
	mixmat_param_blk.InvalidateUI(mixmat_mask);
	ivalid.SetEmpty();
	}

void MixMat::SetSubMtl(int i, Mtl *m)
{
	ReplaceReference(i+1,m);
	if (i==0)
		{
		mixmat_param_blk.InvalidateUI(mixmat_map1);
		ivalid.SetEmpty();
		}	
	else if (i==1)
		{
		mixmat_param_blk.InvalidateUI(mixmat_map2);
		ivalid.SetEmpty();
		}	

}



float MixMat::mixCurve(float x) 
	{
	if (x<crvA)  return 0.0f;
	if (x>=crvB) return 1.0f;
	x = (x-crvA)/(crvB-crvA);
	return (x*x*(3-2*x));
	}

static Color black(0,0,0);


// my version 12.4.00,  kae need to save mix & get it
void MixMat::PreShade(ShadeContext& sc, IReshadeFragment* pFrag)
{
	IReshading* pReshading;

	int lenChan = pFrag->NTextures(); // indx of len channel
	pFrag->AddIntChannel(0);//placeholder

	int mapLength = 0;
	// save mix
	if( map ) {
		float mix = map->EvalMono(sc);
		pFrag->AddFloatChannel( mix );
		mapLength = 1;
	}
	int sub1Length = 0;
	if (sub1){
		pReshading = (IReshading*)(sub1->GetInterface(IID_IReshading));
		if( pReshading ) {
			pReshading->PreShade(sc, pFrag);
			sub1Length = pFrag->NTextures() - lenChan - 1;
		}
	}
	int sub2Length = 0;
	if (sub2){
		pReshading = (IReshading*)(sub2->GetInterface(IID_IReshading));
		if( pReshading ) {
			pReshading->PreShade(sc, pFrag);
			sub2Length = pFrag->NTextures() - sub1Length - lenChan - 1;
		}
	}
	// code the two lengths into the int channel
	pFrag->SetIntChannel( lenChan, (sub1Length | (sub2Length<<8) | (mapLength<<31)) );
}

void MixMat::PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams*)
{
	IReshading* pReshading;
	Mtl *sm1 = mapOn[0] ? sub1 : NULL;
	Mtl *sm2 = mapOn[1] ? sub2 : NULL;
	Texmap *mp = mapOn[2] ? map : NULL;

	int sm1Length = pFrag->GetIntChannel(nextTexIndex++);
	int sm2Length = (sm1Length >> 8) & 0xff ;
	int mpLength = sm1Length >> 31; // top bit
	sm1Length &= 0xff; // get rid of other stuff
 
	float mix;
	if( mp && mpLength ){
		mix = pFrag->GetFloatChannel(nextTexIndex++);
		if (useCurve)
			mix = mixCurve(mix);
	} else {
		mix = u;
		nextTexIndex += mpLength;
	}

	ShadeOutput out1;
	out1.Reset();
	if (sm1){
		sc.ResetOutput();
		pReshading = (IReshading*)(sm1->GetInterface(IID_IReshading));
		if( pReshading ) 
			pReshading->PostShade(sc, pFrag, nextTexIndex);
		out1 = sc.out;
	} else {
		mix = 1.0f;
		nextTexIndex += sm1Length;
	}

	if (sm2){
		sc.ResetOutput();
		pReshading = (IReshading*)(sm2->GetInterface(IID_IReshading));
		if( pReshading ) 
			pReshading->PostShade(sc, pFrag, nextTexIndex);
	} else mix = 0.0f;

	sc.out.MixIn( out1, mix );
}

Sampler*  MixMat::GetPixelSampler(int mtlNum, BOOL backFace )
{
	Mtl *sm1 = mapOn[0]? sub1 : NULL;
	Mtl *sm2 = mapOn[1]? sub2 : NULL;

	Sampler* pSampler = NULL;
	if (sm1)
		pSampler = sm1->GetPixelSampler( mtlNum, backFace );
	if ( !pSampler && sm2) 
		pSampler = sm1->GetPixelSampler( mtlNum, backFace );
	
	return pSampler;
}

// if this function changes, please also check SupportsReShading, PreShade and PostShade
// end - ke/mjm - 03.16.00 - merge reshading code
// [attilas|24.5.2000] if this function changes, please also check EvalColorStdChannel
void MixMat::Shade(ShadeContext& sc) {
	Mtl *sm1 = mapOn[0]?sub1:NULL;
	Mtl *sm2 = mapOn[1]?sub2:NULL;
	Texmap *mp = mapOn[2]?map:NULL;

	if (gbufID) sc.SetGBufferID(gbufID);
	float mix = mp ? mp->EvalMono(sc) : u;
	if (mp && useCurve) mix = mixCurve(mix);
	if (mix<minMix) {
		if (sm1) sm1->Shade(sc);
	} else
	if (mix>maxMix) {
		if (sm2) sm2->Shade(sc);
	} else {
		if (sm1) {
			sm1->Shade(sc);
			if(sm2) {
				ShadeOutput out1 = sc.out;
				sc.ResetOutput();
				sm2->Shade(sc);
				// render elements handled by mixIn
				sc.out.MixIn(out1,mix);
				}
			}
		else {
			if (sm2) 
				sm2->Shade(sc);
			}
		}
	}

void MixMat::Update(TimeValue t, Interval& valid)
	{	

	if (Param1)
		{
		pblock->SetValue( mixmat_map1_on, 0, mapOn[0]);
		pblock->SetValue( mixmat_map2_on, 0, mapOn[1]);
		pblock->SetValue( mixmat_mask_on, 0, mapOn[2]);
		Param1 = FALSE;
		}

	ivalid = FOREVER;
	if (sub1) sub1->Update(t,valid);
	if (sub2) sub2->Update(t,valid);
	if (map) map->Update(t,valid);
	pblock->GetValue(mixmat_mix,t,u,ivalid);
	pblock->GetValue(mixmat_curvea,t,crvA,ivalid);
	pblock->GetValue(mixmat_curveb,t,crvB,ivalid);
	pblock->GetValue(mixmat_usecurve,t,useCurve,ivalid);
	pblock->GetValue(mixmat_map1_on,t,mapOn[0],ivalid);
	pblock->GetValue(mixmat_map2_on,t,mapOn[1],ivalid);
	pblock->GetValue(mixmat_mask_on,t,mapOn[2],ivalid);
	EnableStuff();
	valid &= ivalid;
	}

Interval MixMat::Validity(TimeValue t)
	{
	Interval valid = FOREVER;		
	if (sub1) valid &= sub1->Validity(t);
	if (sub2) valid &= sub2->Validity(t);
	if (map) valid &= map->Validity(t);
	pblock->GetValue(mixmat_mix,t,u,valid);
	pblock->GetValue(mixmat_curvea,t,crvA,valid);
	pblock->GetValue(mixmat_curveb,t,crvB,valid);
	return valid;
	}

Animatable* MixMat::SubAnim(int i)
	{
	switch (i) {
		case 0: return pblock;
		case 1: return sub1;
		case 2: return sub2;
		case 3: return map;
		default: return NULL;
		}
	}

TSTR MixMat::SubAnimName(int i)
	{
	switch (i) {
		case 0: return GetString(IDS_DS_PARAMETERS);
		case 1: return GetSubMtlTVName(0); //GetString(IDS_RB_MATERIALONE);
		case 2: return GetSubMtlTVName(1); // GetString(IDS_RB_MATERIALTWO);
		case 3: return GetSubTexmapTVName(0); //String(IDS_RB_MASK);
		default: return _T("");
		}
	}

RefTargetHandle MixMat::GetReference(int i)
	{
	switch (i) {
		case 0: return pblock;
		case 1: return sub1;
		case 2: return sub2;
		case 3: return map;
		default: return NULL;
		}
	}

void MixMat::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case 0: pblock = (IParamBlock2*)rtarg; break;
		case 1: sub1 = (Mtl*)rtarg; break;
		case 2: sub2 = (Mtl*)rtarg; break;
		case 3: map = (Texmap*)rtarg; break;
		}
	}

RefTargetHandle MixMat::Clone(RemapDir &remap)
	{
	MixMat *mtl = new MixMat(FALSE);
	*((MtlBase*)mtl) = *((MtlBase*)this);  // copy superclass stuff
	mtl->ReplaceReference(PB_REF,remap.CloneRef(pblock));
	if (sub1) mtl->ReplaceReference(SUB1_REF,remap.CloneRef(sub1));
	if (sub2) mtl->ReplaceReference(SUB2_REF,remap.CloneRef(sub2));
	if (map) mtl->ReplaceReference(MAP_REF,remap.CloneRef(map));
	for (int i=0; i<3; i++)
		mtl->mapOn[i] = mapOn[i];
	BaseClone(this, mtl, remap);
	return mtl;
	}

RefResult MixMat::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				mixmat_param_blk.InvalidateUI(changing_param);
				// NotifyChanged();  //DS this is redundant
				if((changing_param == mixmat_mix)
					||(changing_param == mixmat_mix)
					||(changing_param == mixmat_curvea)
					||(changing_param == mixmat_curveb)
					||(changing_param == mixmat_usecurve)
					||(changing_param == mixmat_usemat)
					||(changing_param == mixmat_map1_on)
					||(changing_param == mixmat_map2_on)
					||(changing_param == mixmat_mask_on)
					)
					mReshadeRQ = RR_NeedReshade;
				}

//			if (hTarget==pblock) ivalid.SetEmpty();

			if (hTarget != NULL) {
				switch (hTarget->SuperClassID()) {
					case MATERIAL_CLASS_ID: {
						IReshading* r = static_cast<IReshading*>(hTarget->GetInterface(IID_IReshading));
						mReshadeRQ = r == NULL ? RR_None : r->GetReshadeRequirements();
					} break;
				}
			}
			break;
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			mReshadeRQ = RR_NeedPreshade;
			NotifyChanged();
			break;

		}
	return REF_SUCCEED;
	}


//
// Note: ALL Materials and texmaps must have a Save and Load to save and load
// the MtlBase info.
#define MTL_HDR_CHUNK 0x4000
#define MAPOFF_CHUNK 0x1000
#define PARAM2_CHUNK 0x1010

IOResult MixMat::Save(ISave *isave) { 
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();
	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
/*
	for (int i=0; i<3; i++) {
		if (mapOn[i]==0) {
			isave->BeginChunk(MAPOFF_CHUNK+i);
			isave->EndChunk();
			}
		}
*/
	return IO_OK;
	}	
	  

class MixMatPostLoad : public PostLoadCallback {
	public:
		MixMat *n;
		MixMatPostLoad(MixMat *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->Param1)
				{
				n->pblock->SetValue( mixmat_map1_on, 0, n->mapOn[0]);
				n->pblock->SetValue( mixmat_map2_on, 0, n->mapOn[1]);
				n->pblock->SetValue( mixmat_mask_on, 0, n->mapOn[2]);
				}

			delete this; 

			} 
	};



IOResult MixMat::Load(ILoad *iload) { 
//	ULONG nb;
	int id;
	IOResult res;
	Param1 =TRUE;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
			case MAPOFF_CHUNK+2:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	// JBW: register old version ParamBlock to ParamBlock2 converter

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &mixmat_param_blk, this, PB_REF);
	iload->RegisterPostLoadCallback(plcb);
//	iload->RegisterPostLoadCallback(new MixMatPostLoad(this));

	return IO_OK;
	}


float MixMat::EvalDisplacement(ShadeContext& sc) {
	Mtl *sm1 = mapOn[0]?sub1:NULL;
	Mtl *sm2 = mapOn[1]?sub2:NULL;
	Texmap *mp = mapOn[2]?map:NULL;
	
	float mix = mp ? mp->EvalMono(sc) : u;
	if (mp && useCurve) mix = mixCurve(mix);
	if (mix<minMix) {
		return (sm1)?sm1->EvalDisplacement(sc):0.0f;
	} else
	if (mix>maxMix) {
		return (sm2)?sm2->EvalDisplacement(sc):0.0f;
	} else {
		if (sm1) {
			float d = sm1->EvalDisplacement(sc);
			if(sm2) {
				float d2 = sm2->EvalDisplacement(sc);
				d = (1.0f-mix)*d + mix*d2;
				}
			return d;
			}
		else {
			if (sm2) 
				return sm2->EvalDisplacement(sc);
			}
		}
	return 0.0f;
	}

Interval MixMat::DisplacementValidity(TimeValue t) {
	Mtl *sm1 = mapOn[0]?sub1:NULL;
	Mtl *sm2 = mapOn[1]?sub2:NULL;
	Texmap *mp = mapOn[2]?map:NULL;
	Interval iv;
	iv.SetInfinite();
	if (sm1) 	
		iv &= sm1->DisplacementValidity(t);		
	if (sm2) 	
		iv &= sm2->DisplacementValidity(t);		
	if (mp) { 	
		Interval ivm;
		ivm.SetInfinite();
		mp->Update(t,ivm);
		iv &= ivm;
		}
	if (useCurve) {
		pblock->GetValue(mixmat_curvea,t,crvA,iv);
		pblock->GetValue(mixmat_curveb,t,crvB,iv);
		}
	else 
		pblock->GetValue(mixmat_mix,t,u,iv);
	return iv;	
	} 

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// A mixmat is constant only if both submaps are the mask are constant
//
bool MixMat::IsOutputConst
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID				// must be ID_AM, ect
)
{
	Texmap *mp = mapOn[2]?map:NULL;
	if ( mp && mp->IsOutputMeaningful( sc ) )
		return false;

	Mtl *sm1 = mapOn[0]?sub1:NULL;
	if ( sm1 && !sm1->IsOutputConst( sc, stdID ) ) 
		return false;

	Mtl *sm2 = mapOn[1]?sub1:NULL;
	if ( sm2 && !sm2->IsOutputConst( sc, stdID ) ) 
		return false;
	
	return true;
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Evaluates the material on a single texmap channel. 
// For a mono channel, the value is copied in all 3 components of the 
// output color.
// 
bool MixMat::EvalColorStdChannel
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID,				// must be ID_AM, ect
	Color& outClr			// output var
)
{
	Mtl *sm1 = mapOn[0]?sub1:NULL;
	Mtl *sm2 = mapOn[1]?sub2:NULL;
	Texmap *mp = mapOn[2]?map:NULL;
	bool bRes = false;

	float mix;
	if ( mp && mp->IsOutputMeaningful( sc ) )
		mix = mp->EvalMono(sc);
	else
		mix = u;
	
	if ( mp && useCurve ) 
		mix = mixCurve(mix);
	
	if ( mix < minMix ) 
	{
		if (sm1) 
			bRes = sm1->EvalColorStdChannel(sc, stdID, outClr);
	} 
	else if ( mix > maxMix ) 
	{
		if (sm2) 
			bRes = sm2->EvalColorStdChannel(sc, stdID, outClr);
	} 
	else 
	{
		if (sm1) 
		{
			bRes = sm1->EvalColorStdChannel(sc, stdID, outClr);
			
			if(sm2 && bRes) 
			{
				Color c2;
				c2.Black();
				bRes = sm2->EvalColorStdChannel(sc, stdID, c2);

				// ShadeOutput::MixIn prototype and teh way it's called in MixMat::Shade
				// s.out =  (1-f)*a + f*s.out;
				// void ShadeOutput::MixIn(ShadeOutput &a, float f)
				// sc.out.MixIn(out1,mix);

				outClr = (1.0f-mix)*outClr + mix*c2;
			}
		}
		else 
		{
			if (sm2) 
				bRes = sm2->EvalColorStdChannel(sc, stdID, outClr);
		}
	}
	return bRes;
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Evaluates the material on a single texmap channel. 
//
bool MixMat::EvalMonoStdChannel
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID,				// must be ID_AM, ect
	float& outVal			// output var
)
{
	Mtl *sm1 = mapOn[0]?sub1:NULL;
	Mtl *sm2 = mapOn[1]?sub2:NULL;
	Texmap *mp = mapOn[2]?map:NULL;
	bool bRes = false;

	float mix;
	if ( mp && mp->IsOutputMeaningful( sc ) )
		mix = mp->EvalMono(sc);
	else
		mix = u;
	
	if ( mp && useCurve ) 
		mix = mixCurve(mix);
	
	if ( mix < minMix ) 
	{
		if (sm1) 
			bRes = sm1->EvalMonoStdChannel(sc, stdID, outVal);
	} 
	else if ( mix > maxMix ) 
	{
		if (sm2) 
			bRes = sm2->EvalMonoStdChannel(sc, stdID, outVal);
	} 
	else 
	{
		if (sm1) 
		{
			bRes = sm1->EvalMonoStdChannel(sc, stdID, outVal);
			
			if(sm2 && bRes) 
			{
				float val2 = 0.0f;
				bRes = sm2->EvalMonoStdChannel(sc, stdID, val2);

				// ShadeOutput::MixIn prototype and teh way it's called in MixMat::Shade
				// s.out =  (1-f)*a + f*s.out;
				// void ShadeOutput::MixIn(ShadeOutput &a, float f)
				// sc.out.MixIn(out1,mix);

				outVal = (1.0f-mix)*outVal + mix*val2;
			}
		}
		else 
		{
			if (sm2) 
				bRes = sm2->EvalMonoStdChannel(sc, stdID, outVal);
		}
	}
	return bRes;
}
