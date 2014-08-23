 /**********************************************************************
 *<
	FILE: composite_material.cpp

	DESCRIPTION:  A compistes mulitple materials

	CREATED BY: Peter Watje

	HISTORY:10/4/98

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "iparamm2.h"
// begin - ke/mjm - 03.16.00 - merge reshading code
//#include "iReshade.h"
// end - ke/mjm - 03.16.00 - merge reshading code
#include "buildver.h"
#ifndef NO_MTL_COMPOSITE

extern HINSTANCE hInstance;

static Class_ID compositematClassID(0x61dc0cd7, 0x13640af6);

#define PB_REF		0
// [attilas|29.5.2000] the maximum # of mtls that can be composited
#define MAX_NUM_MTLS	10


// IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { compmat_params, };  // pblock ID

// compmat_params param IDs

enum 
{ 
	compmat_mtls,
	compmat_type, 
	compmat_map_on, 
	compmat_amount
};


class CompositeMat : public Mtl, public IReshading  {	
	public:
		IParamBlock2 *pblock2; 	// ref #0
		float currentAmount;
		int currentMtl;
		int currentType;
		Interval ivalid;
		ReshadeRequirements mReshadeRQ; // mjm - 06.02.00
		TimeValue curTime;	// time of last update, for getpixelsampler

		CompositeMat(BOOL loading);
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
		Color GetSelfIllumColor(int mtlNum=0, BOOL backFace=FALSE);
		float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
		float GetShininess(int mtlNum=0, BOOL backFace=FALSE);		
		float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
		float WireSize(int mtlNum=0, BOOL backFace=FALSE);
				
		IAutoMParamDlg* masterDlg;
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		
		Sampler*  GetPixelSampler(int mtlNum, BOOL backFace );
		void Shade(ShadeContext& sc);
		float EvalDisplacement(ShadeContext& sc); 
		Interval DisplacementValidity(TimeValue t); 
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t);
		
		Class_ID ClassID() {return compositematClassID; }
		SClass_ID SuperClassID() {return MATERIAL_CLASS_ID;}
		void GetClassName(TSTR& s) {s=GetString(IDS_PW_COMPOSITE_MATERIAL);}  

		void DeleteThis() {delete this;}	

		// Methods to access sub-materials of meta-materials
		//fix this 

		int NumSubMtls() {return pblock2->Count(compmat_mtls);}  //how to handle this ??????
		Mtl* GetSubMtl(int i) {
								Mtl *sm1 = NULL;
								Interval iv;
								pblock2->GetValue(compmat_mtls,0,sm1,iv,i);
								return sm1;
								}
		void SetSubMtl(int i, Mtl *m) {
	
								if ((i!= 0) || (m!=NULL))  //watje 3-18-99 to prevent the base material from getting nulled
									pblock2->SetValue(compmat_mtls,0,m,i);
								}
		TSTR GetSubMtlSlotName(int i);

//		BOOL IsMultiMtl() { return TRUE; }

		// Methods to access sub texture maps of material or texmap
		int NumSubTexmaps() {return 0;}
		Texmap* GetSubTexmap(int i) {return NULL;}
		void SetSubTexmap(int i, Texmap *m) {	}

		int NumSubs() {return 1;} 
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) {return subNum;}

		// direct ParamBlock access
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock
		// From ref
 		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		// IO
		IOResult Save(ISave *isave); 
		IOResult Load(ILoad *iload);

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


class CompositeMatClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return GetAppID() != kAPP_VIZR;}
	void *			Create(BOOL loading) {return new CompositeMat(loading);}
	const TCHAR *	ClassName() {return GetString(IDS_COMPOSITE_MATERIAL_OBJECT);}
	SClass_ID		SuperClassID() {return MATERIAL_CLASS_ID;}
	Class_ID 		ClassID() {return compositematClassID;}
	const TCHAR* 	Category() {return _T("");}
	const TCHAR*	InternalName() { return _T("composite"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};
static CompositeMatClassDesc compositematCD;
ClassDesc* GetCompositeMatDesc() {return &compositematCD;}


class CompositeMatPBAccessor : public PBAccessor
{
public:
void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);    // set from v
void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid);    // set from v

TSTR GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex);


};


void CompositeMatPBAccessor::Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		CompositeMat* p = (CompositeMat*)owner;
		switch (id)
		{
		case compmat_mtls:
			{
//				IReshading* r = NULL;
//				if( v.r != NULL )
//					r = static_cast<IReshading*>(v.r->GetInterface(IID_IReshading));
//				p->mReshadeRQ = (r == NULL) ? IReshading::RR_None : r->GetReshadeRequirements();
			} break;

		case compmat_amount:
			{
			int type ;
			Interval iv;
			p->pblock2->GetValue(compmat_type,t,type,iv,tabIndex);
			if (type == 2)
				{
				if (v.f > 100.0f) v.f = 100.f;
				}


			break;
			}


		}
	}


void CompositeMatPBAccessor::Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)    // set from v
	{
		CompositeMat* p = (CompositeMat*)owner;
		switch (id)
		{
		case compmat_amount:
			{
			int type ;
			Interval iv;
			p->pblock2->GetValue(compmat_type,t,type,iv,tabIndex);
			if (type == 2)
				{
				if (v.f > 100.0f) v.f = 100.f;
				}


			break;
			}


		}
	}


TSTR CompositeMatPBAccessor::GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex)
{
	CompositeMat* p = (CompositeMat*)owner;
	TSTR out;
	switch (id)
		{
		case compmat_amount:
			{
			out.printf(_T("%s %d"),GetString(IDS_PW_AMOUNT),tabIndex+1);
			break;
			}
		case compmat_mtls:
			{
			TSTR name;
			Mtl *sm1 = NULL;
			Interval iv;
			if ((p) && (p->pblock2)) p->pblock2->GetValue(compmat_mtls,0,sm1,iv,tabIndex);
			name = sm1->GetFullName();

			if (tabIndex == 0)
				{
				out.printf(_T("%s: %s"),GetString(IDS_PW_BASE),name);
				}
			else
				{
				out.printf(_T("%s %d: %s"),GetString(IDS_RB_MATERIAL2),tabIndex,name);
				}
			break;
			}
		}
	return out;

}


static CompositeMatPBAccessor compositeMat_accessor;



// per instance compmat block
static ParamBlockDesc2 compmat_param_blk ( compmat_params, _T("parameters"),  0, &compositematCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PB_REF, 
	//rollout
	IDD_COMPOSITE_MAT, IDS_PW_COMPOSITE_MATERIAL_PARAMS, 0, 0, NULL, 
	// params
	compmat_mtls,		_T("materialList"),	TYPE_MTL_TAB, MAX_NUM_MTLS,   P_SUBANIM|P_COMPUTED_NAME,	IDS_RB_MATERIAL2,	
		p_submtlno,		0,
		p_ui,			TYPE_MTLBUTTON, IDC_COMPMAT_MAT1, IDC_COMPMAT_MAT2, IDC_COMPMAT_MAT3,  IDC_COMPMAT_MAT4, 
										IDC_COMPMAT_MAT5, IDC_COMPMAT_MAT6,  IDC_COMPMAT_MAT7, 
										IDC_COMPMAT_MAT8, IDC_COMPMAT_MAT9, IDC_COMPMAT_MAT10, 
		p_accessor,		&compositeMat_accessor,
		end,

	compmat_type,		_T("mixType"),	TYPE_INT_TAB,	MAX_NUM_MTLS-1,0,	IDS_PW_TYPE,
		p_default,		0,
		end,



	compmat_map_on,	_T("mapEnables"), TYPE_BOOL_TAB, MAX_NUM_MTLS-1, 0, IDS_MAPENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1, IDC_MAPON2, IDC_MAPON3, IDC_MAPON4, IDC_MAPON5,
											IDC_MAPON6, IDC_MAPON7, IDC_MAPON8, IDC_MAPON9,
		end,

	compmat_amount,  _T("amount"),		TYPE_FLOAT_TAB, 	MAX_NUM_MTLS-1,P_ANIMATABLE|P_COMPUTED_NAME|P_TV_SHOW_ALL,	IDS_PW_AMOUNT, 
		p_default, 		100.0f,	
		p_range, 		0.00f, 200.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, 
						IDC_COMPMAP_AMOUNT1,IDC_COMPMAP_AMOUNTSPIN1,
						IDC_COMPMAP_AMOUNT2,IDC_COMPMAP_AMOUNTSPIN2,
						IDC_COMPMAP_AMOUNT3,IDC_COMPMAP_AMOUNTSPIN3,
						IDC_COMPMAP_AMOUNT4,IDC_COMPMAP_AMOUNTSPIN4,
						IDC_COMPMAP_AMOUNT5,IDC_COMPMAP_AMOUNTSPIN5,
						IDC_COMPMAP_AMOUNT6,IDC_COMPMAP_AMOUNTSPIN6,
						IDC_COMPMAP_AMOUNT7,IDC_COMPMAP_AMOUNTSPIN7,
						IDC_COMPMAP_AMOUNT8,IDC_COMPMAP_AMOUNTSPIN8,
						IDC_COMPMAP_AMOUNT9,IDC_COMPMAP_AMOUNTSPIN9,
//						IDC_COMPMAP_AMOUNT10,IDC_COMPMAP_AMOUNTSPIN10,
						0.1f, //SPIN_AUTOSCALE, 
		p_accessor,		&compositeMat_accessor,
		end, 



	end
	);

//dialog stuff to get the Set Ref button
class CompMatDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		ICustButton *iABut[MAX_NUM_MTLS-1];
		ICustButton *iSBut[MAX_NUM_MTLS-1];
		ICustButton *iMBut[MAX_NUM_MTLS-1];
		CompositeMat *mat;		
		CompMatDlgProc(CompositeMat *m);
		~CompMatDlgProc();
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void ReloadDialog();
		Class_ID ClassID() {return compositematClassID;}
		void SetThing(ReferenceTarget *m);
		ReferenceTarget* GetThing() { return (ReferenceTarget *)mat; }

	};

CompMatDlgProc::CompMatDlgProc(CompositeMat *m)
		{
		mat = m;
		for (int i=0; i<MAX_NUM_MTLS-1; i++) 
			{
			iABut[i] = NULL; 
			iSBut[i] = NULL; 
			iMBut[i] = NULL; 
			}

		}		

CompMatDlgProc::~CompMatDlgProc()
		{
		for (int i=0; i<MAX_NUM_MTLS-1; i++) 
			{
			ReleaseICustButton(iABut[i]);
			ReleaseICustButton(iSBut[i]);
			ReleaseICustButton(iMBut[i]);
			iABut[i] = NULL; 
			iSBut[i] = NULL; 
			iMBut[i] = NULL; 
			}
		

		}		

void CompMatDlgProc::SetThing(ReferenceTarget *m) {
	mat = (CompositeMat*)m;
	ReloadDialog();
	}
void CompMatDlgProc::ReloadDialog()
	{
	for (int i = 0; i< MAX_NUM_MTLS-1;i++)
		{
		Interval iv;
		int type;
		mat->pblock2->GetValue(compmat_type,0,type,iv,i);
//		type =0;
		if (type == 0)
			{
			iABut[i]->SetCheck(TRUE);
			iSBut[i]->SetCheck(FALSE);
			iMBut[i]->SetCheck(FALSE);
			}
		else if (type == 1)
			{
			iABut[i]->SetCheck(FALSE);
			iSBut[i]->SetCheck(TRUE);
			iMBut[i]->SetCheck(FALSE);
			}
		else 
			{
			iABut[i]->SetCheck(FALSE);
			iSBut[i]->SetCheck(FALSE);
			iMBut[i]->SetCheck(TRUE);
///need to limit UI button
//			m->pblock();
			}
		}

	}


BOOL CompMatDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	int a = -1;
	int s = -1;
	int m = -1;
	switch (msg) {
		case WM_INITDIALOG: {

//			mod->hParams = hWnd;
			iABut[0] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A1));
			iABut[1] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A2));
			iABut[2] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A3));
			iABut[3] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A4));
			iABut[4] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A5));
			iABut[5] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A6));
			iABut[6] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A7));
			iABut[7] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A8));
			iABut[8] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A9));
//			iABut[9] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_A10));

			iSBut[0] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S1));
			iSBut[1] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S2));
			iSBut[2] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S3));
			iSBut[3] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S4));
			iSBut[4] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S5));
			iSBut[5] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S6));
			iSBut[6] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S7));
			iSBut[7] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S8));
			iSBut[8] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S9));
//			iSBut[9] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_S10));
			
			iMBut[0] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M1));
			iMBut[1] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M2));
			iMBut[2] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M3));
			iMBut[3] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M4));
			iMBut[4] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M5));
			iMBut[5] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M6));
			iMBut[6] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M7));
			iMBut[7] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M8));
			iMBut[8] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M9));
//			iMBut[9] = GetICustButton(GetDlgItem(hWnd,IDC_COMPMAT_M10));


			for (int i = 0; i< MAX_NUM_MTLS-1;i++)
				{
				iABut[i]->SetType(CBT_CHECK);
				iABut[i]->SetHighlightColor(GREEN_WASH);

				iSBut[i]->SetType(CBT_CHECK);
				iSBut[i]->SetHighlightColor(GREEN_WASH);

				iMBut[i]->SetType(CBT_CHECK);
				iMBut[i]->SetHighlightColor(GREEN_WASH);

				Interval iv;
				int type;
				mat->pblock2->GetValue(compmat_type,0,type,iv,i);
//				type =0;
				if (type == 0)
					{
					iABut[i]->SetCheck(TRUE);
					iSBut[i]->SetCheck(FALSE);
					iMBut[i]->SetCheck(FALSE);
					}
				else if (type == 1)
					{
					iABut[i]->SetCheck(FALSE);
					iSBut[i]->SetCheck(TRUE);
					iMBut[i]->SetCheck(FALSE);

					}
				else 
					{
					iABut[i]->SetCheck(FALSE);
					iSBut[i]->SetCheck(FALSE);
					iMBut[i]->SetCheck(TRUE);
					}
				}
			break;
			}
/*
		case WM_PAINT:
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;
*/

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_COMPMAT_A1: a = 0;break;
				case IDC_COMPMAT_A2: a = 1;break;
				case IDC_COMPMAT_A3: a = 2;break;
				case IDC_COMPMAT_A4: a = 3;break;
				case IDC_COMPMAT_A5: a = 4;break;
				case IDC_COMPMAT_A6: a = 5;break;
				case IDC_COMPMAT_A7: a = 6;break;
				case IDC_COMPMAT_A8: a = 7;break;
				case IDC_COMPMAT_A9: a = 8;break;
//				case IDC_COMPMAT_A10:a = 9;break;
				case IDC_COMPMAT_S1: s = 0;	break;
				case IDC_COMPMAT_S2: s = 1;	break;
				case IDC_COMPMAT_S3: s = 2;	break;
				case IDC_COMPMAT_S4: s = 3;	break;
				case IDC_COMPMAT_S5: s = 4;	break;
				case IDC_COMPMAT_S6: s = 5;	break;
				case IDC_COMPMAT_S7: s = 6;	break;
				case IDC_COMPMAT_S8: s = 7;	break;
				case IDC_COMPMAT_S9: s = 8;	break;
//				case IDC_COMPMAT_S10:s = 9;	break;
				case IDC_COMPMAT_M1: m = 0;	break;
				case IDC_COMPMAT_M2: m = 1;	break;
				case IDC_COMPMAT_M3: m = 2;	break;
				case IDC_COMPMAT_M4: m = 3;	break;
				case IDC_COMPMAT_M5: m = 4;	break;
				case IDC_COMPMAT_M6: m = 5;	break;
				case IDC_COMPMAT_M7: m = 6;	break;
				case IDC_COMPMAT_M8: m = 7;	break;
				case IDC_COMPMAT_M9: m = 8;	break;
//				case IDC_COMPMAT_M10:m = 9;	break;
				
				}
			if (a != -1)
				{
				mat->pblock2->SetValue(compmat_type,0,0,a);
				iABut[a]->SetCheck(TRUE);
				iSBut[a]->SetCheck(FALSE);
				iMBut[a]->SetCheck(FALSE);
				}
			else if (s != -1)
				{
				mat->pblock2->SetValue(compmat_type,0,1,s);
				iABut[s]->SetCheck(FALSE);
				iSBut[s]->SetCheck(TRUE);
				iMBut[s]->SetCheck(FALSE);
				}
			else if (m != -1)
				{
				mat->pblock2->SetValue(compmat_type,0,2,m);
				iABut[m]->SetCheck(FALSE);
				iSBut[m]->SetCheck(FALSE);
				iMBut[m]->SetCheck(TRUE);
				compmat_param_blk.InvalidateUI(compmat_amount,m);


				}

			break;


		}
	return FALSE;
	}


//-------------------------------------------------------------------


CompositeMat::CompositeMat(BOOL loading) : mReshadeRQ(RR_None) // mjm - 06.02.00
{
	pblock2 = NULL;
//	dlg = NULL;
//	sub1 = sub2 = NULL;	
//	map = NULL;
	ivalid.SetEmpty();
//	for (int i=0; i<3; i++) 
//		mapOn[i] = 1;
	if (!loading)
	{
		compositematCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
		pblock2->DefineParamAlias(_T("baseMaterial"), compmat_mtls, 0);  // JBW 5/24/99, add alias for base material to support macroRecording
		Init();
	}
}

void CompositeMat::Init()
	{
	ivalid.SetEmpty();
	Mtl *sm1 = NULL;
	Interval iv;
	pblock2->GetValue(compmat_mtls,0,sm1,iv,0);
	if (sm1 == NULL) 
//		pblock2->SetValue(compmat_mtls,0,m,i);
		pblock2->SetValue(compmat_mtls,0, (Mtl*)GetStdMtl2Desc()->Create(),0);
	}

void CompositeMat::Reset()
	{
	compositematCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	CompMatDlgProc *parmDlg = (CompMatDlgProc *) compmat_param_blk.GetUserDlgProc();
	if (parmDlg) parmDlg->ReloadDialog();
//watje 3-22-99 set the name of the first material
	Mtl *m = NULL;;
	Interval iv;
	pblock2->GetValue(compmat_mtls,0,m,iv,0);
	if (m) m->SetName(GetString(IDS_PW_BASE));

	}

void* CompositeMat::GetInterface(ULONG id)
{
	if( id == IID_IReshading )
		return (IReshading*)( this );
	else if ( id == IID_IValidityToken )
		return (IValidityToken*)( this );
	else
		return Mtl::GetInterface(id);
}

Mtl *CompositeMat::UseMtl() 
	{

	Mtl *m = NULL;;
	Interval iv;
	pblock2->GetValue(compmat_mtls,0,m,iv,0);
	return m;

	}

Color CompositeMat::GetAmbient(int mtlNum, BOOL backFace) { 
	return UseMtl()?UseMtl()->GetAmbient(mtlNum,backFace):Color(0,0,0);
	}		
Color CompositeMat::GetDiffuse(int mtlNum, BOOL backFace){ 
	return UseMtl()?UseMtl()->GetDiffuse(mtlNum,backFace):Color(0,0,0);
	}				
Color CompositeMat::GetSpecular(int mtlNum, BOOL backFace){
	return UseMtl()?UseMtl()->GetSpecular(mtlNum,backFace):Color(0,0,0);
	}		
Color CompositeMat::GetSelfIllumColor(int mtlNum, BOOL backFace){
	return UseMtl()?UseMtl()->GetSelfIllumColor(mtlNum,backFace):Color(0,0,0);
	}		

float CompositeMat::GetXParency(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetXParency(mtlNum,backFace):0.0f;
	}
float CompositeMat::GetShininess(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetShininess(mtlNum,backFace):0.0f;
	}		
float CompositeMat::GetShinStr(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->GetShinStr(mtlNum,backFace):0.0f;
	}

float CompositeMat::WireSize(int mtlNum, BOOL backFace) {
	return UseMtl()?UseMtl()->WireSize(mtlNum,backFace):0.0f;
	}
		
ParamDlg* CompositeMat::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
	{
	masterDlg = compositematCD.CreateParamDlgs(hwMtlEdit, imp, this);
	compmat_param_blk.SetUserDlgProc(new CompMatDlgProc(this));
//	dlg = new CompositeMatDlg(hwMtlEdit, imp, this);
	return masterDlg;
	}

TSTR CompositeMat::GetSubMtlSlotName(int i) {
	TSTR name;
	if (i== 0)
		name.printf("%s",GetString(IDS_PW_BASE));
	else name.printf("%s %d",GetString(IDS_PW_MAT),i);
	return name;
	}


static Color black(0,0,0);

// begin - ke/mjm - 03.16.00 - merge reshading code
/***************
BOOL CompositeMat::SupportsReShading(ShadeContext& sc)
{
	int i(0);
	Mtl* submtl = NULL;
	BOOL result(TRUE), enabled(FALSE);

	// check the first (constant) submaterial
	pblock2->GetValue(compmat_mtls, sc.CurTime(), submtl, FOREVER, i);
	if (submtl)
		result &= submtl->SupportsReShading(sc);

	// check any other submaterials
	for (i=1; i<MAX_NUM_MTLS; i++)
	{
		pblock2->GetValue(compmat_map_on, sc.CurTime(), enabled, FOREVER, i-1);
		if (enabled)
		{
			pblock2->GetValue(compmat_mtls, sc.CurTime(), submtl, FOREVER, i);
			if (submtl)
				result &= submtl->SupportsReShading(sc);
		}
	}
	return result;
}
*************/

void CompositeMat::PreShade(ShadeContext& sc, IReshadeFragment* pFrag)
{
	int i(0);
	Mtl *submtl = NULL;
//	BOOL enabled(FALSE);

	char texLengths[12];

	int lengthChan = pFrag->NTextures();
	pFrag->AddIntChannel(0);
	pFrag->AddIntChannel(0);
	pFrag->AddIntChannel(0);

	int nPrevTex = 3 + lengthChan;

	// preshade any submaterials
	for (i=0; i<MAX_NUM_MTLS; i++)
	{
		pblock2->GetValue(compmat_mtls, sc.CurTime(), submtl, FOREVER, i);
		if (submtl){
			IReshading* pReshading = (IReshading*)(submtl->GetInterface(IID_IReshading));
			if( pReshading ){
				pReshading->PreShade(sc, pFrag);
				int nTex = pFrag->NTextures();
				texLengths[i] = char( nTex - nPrevTex);
				nPrevTex = nTex;
			}
		}
	}
	int* pI = (int*)&texLengths[0];
	pFrag->SetIntChannel( lengthChan++, pI[0] );
	pFrag->SetIntChannel( lengthChan++, pI[1] );
	pFrag->SetIntChannel( lengthChan, pI[2] );

}

void CompositeMat::PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* )
{
	int i(0), type(2);
	Mtl* submtl = NULL;
	BOOL enabled(TRUE);
	ShadeOutput out1, out2;
	float amount(100.0f); // , gamount(0.0f);

	char texLengths[12];

	int* pI = (int*)&texLengths[0];
	pI[0] = pFrag->GetIntChannel(nextTexIndex++);
	pI[1] = pFrag->GetIntChannel(nextTexIndex++);
	pI[2] = pFrag->GetIntChannel(nextTexIndex++);



	// postshade any submaterials
	for ( i=0; i<MAX_NUM_MTLS; i++)
	{
		if( i>0 )
			pblock2->GetValue(compmat_map_on, sc.CurTime(), enabled, FOREVER, i-1);
		else 
			enabled = TRUE;

		pblock2->GetValue(compmat_mtls, sc.CurTime(), submtl, FOREVER, i);
		if ( enabled && submtl ){
			if( i > 0 ){
				pblock2->GetValue(compmat_amount, sc.CurTime(), amount, FOREVER, i-1);
				pblock2->GetValue(compmat_type, sc.CurTime(), type, FOREVER, i-1);
			}
			amount = amount * 0.01f;
			sc.ResetOutput();
			IReshading* pReshading = (IReshading*)(submtl->GetInterface(IID_IReshading));
			if( pReshading ) 
				pReshading->PostShade(sc, pFrag, nextTexIndex);
			
			// first check for base material
			if( i > 0 ) {
				// not base material, composite the next material
				out2 = sc.out;
					
				if (type == 0) // additive
				{
					out2.t.r += 1.0f-amount;
					out2.t.g += 1.0f-amount;
					out2.t.b += 1.0f-amount;
					out2.c *= amount;
					out2.t.ClampMinMax();
					out2.ior *= amount;

					float f1 = 1.0f - (out1.t.r + out1.t.g + out1.t.b) / 3.0f; 
					float f2 = 1.0f - (out2.t.r + out2.t.g + out2.t.b) / 3.0f; 
					out1.c = out1.c * (1.0f-f2) + out2.c * f1;
					out1.t = out1.t - (1.0f-out2.t);
					out1.ior = out1.ior + out2.ior;
					out1.t.ClampMinMax();
				}
				else if (type == 1) // subtractive
				{
					out2.t.r += 1.0f-amount;
					out2.t.g += 1.0f-amount;
					out2.t.b += 1.0f-amount;
					out2.c *= amount;
					out2.t.ClampMinMax();
					out2.ior *= amount;

					float f1 = 1.0f - (out1.t.r + out1.t.g + out1.t.b) / 3.0f;
					float f2 = 1.0f - (out2.t.r + out2.t.g + out2.t.b) / 3.0f;

					out1.c = out1.c * (1.0f-f2) - out2.c * f1;
					out1.t = out1.t - (1.0f-out2.t);
					out1.ior = out1.ior + out2.ior;

					out1.t.ClampMinMax();
				}
				else //mix
				{
					out1.MixIn(out2, 1.0f-amount);
				}
			}// end, not base mtl
			else {
				// base material.
				out1 = sc.out;
				out1.ior *= amount;
			}

		}// end, if has submtl & enabled
		else {
			nextTexIndex += texLengths[i];
		}

	}// end, for each mtl

	sc.out = out1;
}

Sampler*  CompositeMat::GetPixelSampler(int mtlNum, BOOL backFace )
{
	Mtl *sm1 = NULL;
	Interval iv;

	for (int i = 0; i < MAX_NUM_MTLS; i++){
		BOOL enabled;
		if (i==0) 
			enabled = 1;
		else 
			pblock2->GetValue(compmat_map_on, curTime, enabled, iv, i-1);

		if (enabled){
			pblock2->GetValue(compmat_mtls, curTime, sm1, iv, i ); //????i or i-1
			if (sm1 != NULL){
				Sampler* pSampler = sm1->GetPixelSampler( mtlNum, backFace );
				if( pSampler ) 
					return pSampler; // return the first one found
			}
		}
	}// end, for each material
	return NULL;
}

// if this function changes, please also check SupportsReShading, PreShade and PostShade
// end - ke/mjm - 03.16.00 - merge reshading code
// [attilas|29.5.2000] if this function changes, please also check EvalColorStdChannel
void CompositeMat::Shade(ShadeContext& sc)
{
	Mtl *sm1 = NULL;
	int id =0; 
//	float gamount;
	if (gbufID){
		sc.SetGBufferID(gbufID);
		id = gbufID;
	}

	Interval iv;
	int first = 1;
	ShadeOutput out1;
	int nEles = sc.NRenderElements();
	
	for (int i = 0; i < MAX_NUM_MTLS; i++){
		BOOL enabled;
		float amount;
		if (i==0) enabled = 1;
		else pblock2->GetValue(compmat_map_on,sc.CurTime(),enabled,iv,i-1);
		if (enabled){
			pblock2->GetValue(compmat_mtls,sc.CurTime(),sm1,iv,i);
			if (sm1 != NULL){
				if (i==0) amount = 100.f;
				else pblock2->GetValue(compmat_amount,sc.CurTime(),amount,iv,i-1);
				amount = amount*0.01f;
				int type;
				if (i==0) type = 2;
				else pblock2->GetValue(compmat_type,sc.CurTime(),type,iv,i-1);

				if (first ==1){	// first material
					first = 0;
					sm1->Shade(sc); // sc.out already reset for first
					out1 = sc.out;
					if (type == 0){
						out1.t.r += 1.0f-amount;
						out1.t.g += 1.0f-amount;
						out1.t.b += 1.0f-amount;
						out1.c *= amount;
						out1.t.ClampMinMax();

						// render elements
						for( int i = 0; i < nEles; ++i )
							out1.elementVals[i] *= amount;

					}
					out1.ior *= amount;

//					gamount = 1.0f-(out1.t.r + out1.t.g + out1.t.b)/3.0f;
				
				} else {	// not first material
//					pblock2->GetValue(compmat_mtls,sc.CurTime(),sm2,iv,i);
//					sc.out.ior = s*a.ior + f*ior;
//					if (f<=0.5f) gbufId = a.gbufId;
					sc.ResetOutput();
					sm1->Shade(sc);
					ShadeOutput out2 = sc.out;

					if (type == 0){  // additive
						out2.t.r += 1.0f-amount;
						out2.t.g += 1.0f-amount;
						out2.t.b += 1.0f-amount;
						out2.c *= amount;
						out2.t.ClampMinMax();

						out2.ior *= amount;

						float f1 = 1.0f-(out1.t.r + out1.t.g + out1.t.b)/3.0f; 
						float f2 = 1.0f-(out2.t.r + out2.t.g + out2.t.b)/3.0f; 

						out1.c =  out1.c *(1.0f-f2) + out2.c * (f1); 
						out1.t =  out1.t- (1.0f-out2.t);
						out1.ior = out1.ior + out2.ior;
//						if (f2 > gamount) {
//							gamount = f2;
//							out1.gbufId = out1.gbufId;//?? ke 6.13.00
//						}
						out1.t.ClampMinMax();
			
						// render elements
						for( int i = 0; i < nEles; ++i )
							out1.elementVals[i] = out1.elementVals[i] * (1.0f-f2) + out2.elementVals[i] * f1;
				
					} else if (type == 1) { // subtractive
						//NB: as of 6.13.00 this is SAME as additive case
						out2.t.r += 1.0f-amount;
						out2.t.g += 1.0f-amount;
						out2.t.b += 1.0f-amount;
						out2.c *= amount;

						out2.t.ClampMinMax();

						out2.ior *= amount;

						float f1 = 1.0f-(out1.t.r + out1.t.g + out1.t.b)/3.0f; 
						float f2 = 1.0f-(out2.t.r + out2.t.g + out2.t.b)/3.0f; 

						out1.c =  out1.c *(1.0f-f2) - out2.c * f1; 
						out1.t =  out1.t- (1.0f-out2.t);
						out1.ior = out1.ior + out2.ior;
//						if (f2 > gamount){
//							gamount = f2;
//							out1.gbufId = out1.gbufId;	//?? ke 6.13.00
//						}
						out1.t.ClampMinMax();

						// render elements
						for( int i = 0; i < nEles; ++i )
							out1.elementVals[i] = out1.elementVals[i] * (1.0f-f2) + out2.elementVals[i] * f1;
					
					} else { //mix

						// mixIn handles render elements
						out1.MixIn(out2,1.0f-amount);
					}
				}// end, not first mtl
			} // end, sm1 not null
		}// end, enabled
	} // end, for each material

	sc.out = out1;
}

/* some old code... saved by someone
	if (sm1 != NULL) sm1->Shade(sc);
	return;
	if ((sm1) && (sm2))
		{
		sc.ResetOutput();
		sm1->Shade(sc);
		ShadeOutput out1 = sc.out;
		sc.ResetOutput();
		sm2->Shade(sc);
		ShadeOutput out2 = sc.out;
		float f1 = 1.0f-(out1.t.r + out1.t.g + out1.t.b)/3.0f; 
		float f2 = 1.0f-(out2.t.r + out2.t.g + out2.t.b)/3.0f; 
		sc.out.c =  out1.c *(1.0f-f2) + out2.c * f1; 
		sc.out.t =  out1.t- out2.t;

//		if (sc.out.t.r > 1.0f) sc.out.t.r = 1.0f;
//		if (sc.out.t.g > 1.0f) sc.out.t.g = 1.0f;
//		if (sc.out.t.b > 1.0f) sc.out.t.b = 1.0f;
//		if (sc.out.t.r < 0.0f) sc.out.t.r = 0.0f;
//		if (sc.out.t.g < 0.0f) sc.out.t.g = 0.0f;
//		if (sc.out.t.b < 0.0f) sc.out.t.b = 0.0f;

//sc.out.ior = s*a.ior + f*ior;
//if (f<=0.5f) gbufId = a.gbufId;
		
		}
*/


void CompositeMat::Update(TimeValue t, Interval& valid)
	{	
	ivalid = FOREVER;
	curTime = t;
	for (int i = 0; i < MAX_NUM_MTLS; i++)
		{
		Mtl *sub;
		float u;
		pblock2->GetValue(compmat_mtls,t,sub,valid,i);
		if (sub != NULL)
			{
			sub->Update(t,valid);
			if (i != MAX_NUM_MTLS-1)
				pblock2->GetValue(compmat_amount,t,u,valid,i);
			}
		}
	valid &= ivalid;
	}

Interval CompositeMat::Validity(TimeValue t)
	{
	Interval valid = FOREVER;		
	for (int i = 0; i < MAX_NUM_MTLS; i++)
		{
		Mtl *sub;
		float u;
		pblock2->GetValue(compmat_mtls,t,sub,valid,i);
		if (sub != NULL)
			{
			valid &= sub->Validity(t);
			if (i != MAX_NUM_MTLS-1)
				pblock2->GetValue(compmat_amount,t,u,valid,i);
			}
		}
//	pblock2->GetValue(compmat_type,t,currentType,ivalid);

	return valid;
	}

Animatable* CompositeMat::SubAnim(int i)
	{
	switch (i) {
		case 0: return pblock2;
		default: return NULL;
		}
	}

TSTR CompositeMat::SubAnimName(int i)
	{
	switch (i) {
		case 0: return GetString(IDS_DS_PARAMETERS);
		default: return _T("");
		}
	}

RefTargetHandle CompositeMat::GetReference(int i)
	{
	switch (i) {
		case 0: return pblock2;
		default: return NULL;
		}
	}

void CompositeMat::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case 0: pblock2 = (IParamBlock2*)rtarg; break;
		}
	}

RefTargetHandle CompositeMat::Clone(RemapDir &remap)
	{
	CompositeMat *mtl = new CompositeMat(FALSE);
	*((MtlBase*)mtl) = *((MtlBase*)this);  // copy superclass stuff
	mtl->ReplaceReference(PB_REF,remap.CloneRef(pblock2));
	BaseClone(this, mtl, remap);
	return mtl;
	}

RefResult CompositeMat::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message)
	{
	switch (message) {
 		case REFMSG_CHANGE:
			if (hTarget == pblock2)	{
				ivalid.SetEmpty();
//				compmat_param_blk.InvalidateUI();
				ParamID changing_param = pblock2->LastNotifyParamID();
				compmat_param_blk.InvalidateUI(changing_param);
				if( (changing_param == compmat_amount )
					||(changing_param == compmat_map_on )
					||(changing_param == compmat_type ))
				{
					mReshadeRQ = RR_NeedReshade;
				} else if( changing_param == compmat_mtls ) {
					mReshadeRQ = RR_NeedPreshade; // really want to get from mtl, but dont have it
//					NotifyChanged();
				}
			} else {
				ivalid.SetEmpty();
				NotifyChanged();
			}

			if (hTarget != NULL) {
				switch (hTarget->SuperClassID()) {
					case MATERIAL_CLASS_ID: {
						// never hit for this mtl ????
						IReshading* r = static_cast<IReshading*>(hTarget->GetInterface(IID_IReshading));
						mReshadeRQ = r == NULL ? RR_None : r->GetReshadeRequirements();
					} break;
				}
			}
			break;
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			if( hTarget ){
				switch (hTarget->SuperClassID()) {
					case MATERIAL_CLASS_ID: {
						mReshadeRQ = RR_NeedPreshade;
					} break;
				}
			}
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
#define VER_CHUNK 0x1010

IOResult CompositeMat::Save(ISave *isave) { 
	IOResult res;
	ULONG nb;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	int ver = 2;
	isave->BeginChunk(VER_CHUNK);
	isave->Write(&ver,sizeof(ver),&nb);			
	isave->EndChunk();

	return IO_OK;
	}	
	  

//watje
class CompositeMatPostLoadCallback:public  PostLoadCallback
{
public:
	CompositeMat      *s;
	CompositeMatPostLoadCallback(CompositeMat *r) {s=r;}
	void proc(ILoad *iload);
};


void CompositeMatPostLoadCallback::proc(ILoad *iload)
{
	if (s->pblock2->Count(compmat_type) == MAX_NUM_MTLS)
		s->pblock2->Delete(compmat_type, 0,1);
	if (s->pblock2->Count(compmat_map_on) == MAX_NUM_MTLS)
		s->pblock2->Delete(compmat_map_on, 0,1);
	if (s->pblock2->Count(compmat_amount) == MAX_NUM_MTLS)
		s->pblock2->Delete(compmat_amount, 0,1);

//	( checker_map1_on, 0, s->mapOn[0]);
//	s->pblock->SetValue( checker_map2_on, 0, s->mapOn[1]);

	delete this;

}
 



IOResult CompositeMat::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	IOResult res;
	int ver = 1;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case VER_CHUNK:
				iload->Read(&ver,sizeof(ver),&nb);			
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	if (ver == 1)
		{
		CompositeMatPostLoadCallback* checkerplcb = new CompositeMatPostLoadCallback(this);
		iload->RegisterPostLoadCallback(checkerplcb);

		}

	return IO_OK;
	}


float CompositeMat::EvalDisplacement(ShadeContext& sc) {

	int first = 1;
	float disp = 0.0f;
	Interval iv;
	Mtl *sm1 = NULL;
	for (int i = 0; i < MAX_NUM_MTLS; i++)
		{
		BOOL enabled;
		float amount;
		if (i==0) enabled = 1;
		else pblock2->GetValue(compmat_map_on,sc.CurTime(),enabled,iv,i-1);
		if (enabled)
			{
			pblock2->GetValue(compmat_mtls,sc.CurTime(),sm1,iv,i);

			if (sm1 != NULL) 
				{
				if (i==0) amount = 100.f;
				else pblock2->GetValue(compmat_amount,sc.CurTime(),amount,iv,i-1);

				amount = amount*0.01f;
				int type;
				if (i==0) type = 0;
				else pblock2->GetValue(compmat_type,sc.CurTime(),type,iv,i-1);
				if (first ==1)
					{
					float d2 = sm1->EvalDisplacement(sc);
					disp = d2;

					}
				else
					{
					if (type == 0)  // addative
						{
						float d2 = sm1->EvalDisplacement(sc);
						disp += d2;

						}
					else if (type == 1) // subtractive
						{
						float d2 = sm1->EvalDisplacement(sc);
						disp -= d2;

						}
					else //mix
						{
						float d2 = sm1->EvalDisplacement(sc);
						disp = (1.0f-amount)*disp + amount*d2;
						}

					}


				}
			}
		}

/*
	Mtl *sm1 = mapOn[0]?sub1:NULL;
	Mtl *sm2 = mapOn[1]?sub2:NULL;
	Texmap *mp = mapOn[2]?map:NULL;
	
	float mix = mp ? mp->EvalMono(sc) : u;
	if (mp && useCurve) mix = mixCurve(mix);
	if (mix<0.0001f) {
		return (sm1)?sm1->EvalDisplacement(sc):0.0f;
	} else
	if (mix>0.9999f) {
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
*/
	return disp;
	}

Interval CompositeMat::DisplacementValidity(TimeValue t) {
	Interval iv;
	iv.SetInfinite();
	Mtl *sm1 = NULL;

	for (int i = 0; i < MAX_NUM_MTLS; i++)
		{
		BOOL enabled;
		if (i==0) enabled = 1;
		else pblock2->GetValue(compmat_map_on,t,enabled,iv,i-1);

		if (enabled)
			{
			pblock2->GetValue(compmat_mtls,t,sm1,iv,i);
			if (sm1 != NULL) 
				{
				iv &= sm1->DisplacementValidity(t);		

				}
			}
		}

/*
	Mtl *sm1 = mapOn[0]?sub1:NULL;
	Mtl *sm2 = mapOn[1]?sub2:NULL;
	Texmap *mp = mapOn[2]?map:NULL;
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
*/
	return iv;	
	} 


//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// The submat with a higher index "cover" the ones "below".
// If the first submat taken from the end of the array has amount == 100,
// that's al it matters. Otherwise all until the second consecutive submat
// with amount == 100 matter
//
bool CompositeMat::IsOutputConst
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID				// must be ID_AM, ect
)
{
	Mtl *sm = NULL;
	int numSubMatOn = 0;
	int numConsec = 0;
	bool bIsConst = true;
	Interval iv;

	// Iterate through the submats in reverse order because that is
	// the order of their significance
	for (int i = MAX_NUM_MTLS-1; i <= 0; i--)
	{
		BOOL enabled;
		float amount;
		
		// The first one is always enabled
		if ( i == 0 ) 
			enabled = 1;
		else 
			pblock2->GetValue( compmat_map_on, sc.CurTime(), enabled, iv, i-1 );
		
		if ( enabled )
		{
			pblock2->GetValue( compmat_mtls, sc.CurTime(), sm, iv, i );
			if ( sm != NULL ) 
			{
				numSubMatOn++;

				// All of the first on is always composited
				if ( i == 0 ) 
					amount = 100.f;
				else 
					pblock2->GetValue( compmat_amount, sc.CurTime(), amount, iv, i-1 );
				
				if ( numSubMatOn == 1 && amount == 100.0f )
					return sm->IsOutputConst( sc, stdID );
				else
				{
					if ( amount == 100.0f )
						numConsec++;
					else
						numConsec = 0;
					bool b = sm->IsOutputConst( sc, stdID );
					bIsConst = (bIsConst && b );
					if ( !bIsConst )
						return bIsConst;
					else if ( numConsec == 2 )
						return bIsConst;
				}
			}
		}
	}
	return bIsConst;
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Evaluates the material on a single texmap channel. 
//
// Note: Channels are added, subtracted or mixed without taking into 
// account the opacity of the materials
// 
bool CompositeMat::EvalColorStdChannel
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID,				// must be ID_AM, ect
	Color& outClr			// output var
)
{
	Mtl *sm1 = NULL;
	int id =0; 
	
	Interval iv;
	int first = 1;
	Color c1;
	float t1;	// dummy transparency color (to mimic the logic of Shade())
	c1.Black();
	
	for (int i = 0; i < MAX_NUM_MTLS; i++){

		BOOL enabled;
		float amount;
		
		// The first one is always enabled
		if ( i == 0 ) 
			enabled = 1;
		else 
			pblock2->GetValue( compmat_map_on, sc.CurTime(), enabled, iv, i-1 );
		
		if ( enabled ){

			pblock2->GetValue( compmat_mtls, sc.CurTime(), sm1, iv, i );
			
			if ( sm1 != NULL ) {
				// All of the first on is always composited
				if ( i == 0 ) 
					amount = 100.f;
				else 
					pblock2->GetValue( compmat_amount, sc.CurTime(), amount, iv, i-1 );
				
				amount *= 0.01f;
				
				int type;
				// The first one is mixed in
				if ( i == 0 ) 
					type = 2;
				else 
					pblock2->GetValue( compmat_type, sc.CurTime(), type, iv, i-1 );

				// [attilas|29.5.2000] I don't understand why the i == 0 test
				// is not used to ensure special treatment for the base material
				if ( first == 1 ){
					first = 0;
					if ( !sm1->EvalColorStdChannel(sc, stdID, outClr) )
						return false;
					c1 = outClr;
					t1 = 0;	// we don't account for any transparency
					
					// [attilas|29.5.2000] Why is this needed?
					if (type == 0){
						t1 += 1.0f - amount;
						c1 *= amount;
						if(t1 > 1.0f)
							t1 = 1.0f;
						else if(t1 < 0.0f)
							t1 = 0.0f;
					}

				}
				else{
					outClr.Black();
					if ( !sm1->EvalColorStdChannel(sc, stdID, outClr) )
						return false;
					Color c2 = outClr;
					float t2 = 0;	// do not account for any transparency

					if ( type == 0 ){  // additive
						t2 += 1.0f - amount;
						c2 *= amount;
						if(t2 > 1.0f)
							t2 = 1.0f;
						else if(t2 < 0.0f)
							t2 = 0.0f;


						float f1 = 1.0f - t1;
						float f2 = 1.0f - t2;

						c1 = c1 * (1.0f-f2) + c2 * f1;
						t1 = t1 - (1.0f-t2);
						if(t1 > 1.0f)
							t1 = 1.0f;
						else if(t1 < 0.0f)
							t1 = 0.0f;

					}
					else if ( type == 1 ){ // subtractive

						t2 += 1.0f - amount;
						c2 *= amount;
						if(t2 > 1.0f)
							t2 = 1.0f;
						else if(t2 < 0.0f)
							t2 = 0.0f;

						float f1 = 1.0f - t1;
						float f2 = 1.0f - t2;

						c1 = c1 * (1.0f-f2) - c2 * f1;
						t1 = t1 - (1.0f-t2);
						if(t1 > 1.0f)
							t1 = 1.0f;
						else if(t1 < 0.0f)
							t1 = 0.0f;
					
					}
					else{ //mix
					
						// copies the behaviour of ShadeOutput::MixIn()
						float f = 1.0f - amount;
						if(f <= 0.0f)
							c1 = c2;
						else if(f < 1.0f){
							float s = 1.0f - f;
							c1 = s*c2 + f*c1;
							t1 = s*t2 + f*t1;
						}
					
					}
				}
			}
		}
	}
	outClr = c1;
	return true;
}

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// Evaluates the material on a single texmap channel. 
//
bool CompositeMat::EvalMonoStdChannel
( 
	ShadeContext& sc, // describes context of evaluation
	int stdID,				// must be ID_AM, ect
	float& outVal			// output var
)
{
	Mtl *sm1 = NULL;
	int id =0; 
	
	Interval iv;
	int first = 1;
	float val1 = 0.0f;
	
	for (int i = 0; i < MAX_NUM_MTLS; i++)
	{
		BOOL enabled;
		float amount;
		
		// The first one is always enabled
		if ( i == 0 ) 
			enabled = 1;
		else 
			pblock2->GetValue( compmat_map_on, sc.CurTime(), enabled, iv, i-1 );
		
		if ( enabled )
		{
			pblock2->GetValue( compmat_mtls, sc.CurTime(), sm1, iv, i );
			if ( sm1 != NULL ) 
			{
				// All of the first on is always composited
				if ( i == 0 ) 
					amount = 100.f;
				else 
					pblock2->GetValue( compmat_amount, sc.CurTime(), amount, iv, i-1 );
				
				amount = amount*0.01f;
				
				int type;
				// The first one is mixed in
				if ( i == 0 ) 
					type = 2;
				else 
					pblock2->GetValue( compmat_type, sc.CurTime(), type, iv, i-1 );

				// [attilas|29.5.2000] I don't understand why the i == 0 test
				// is not used to ensure special treatment for the base material
				if ( first == 1 )
				{
					first = 0;
					if ( !sm1->EvalMonoStdChannel(sc, stdID, outVal) )
						return false;
					val1 = outVal;

					// [attilas|29.5.2000] Why is this needed?
					if (type == 0)
						val1 *= amount;
				}
				else
				{
					outVal = 0.0f;
					if ( !sm1->EvalMonoStdChannel(sc, stdID, outVal) )
						return false;
					float val2 = outVal;

					if ( type == 0 )  // additive
					{
						val2 *= amount;
						val1 += val2; 
					}
					else if ( type == 1 ) // subtractive
					{
						val2 *= amount;
						val1 -= val2; 
					}
					else //mix
					{
						// ShadeOutput::MixIn prototype and teh way it's called in CMtl::Shade
						// s.out =  (1-f)*a + f*s.out;
						// void ShadeOutput::MixIn(ShadeOutput &a, float f)
						// out1.MixIn(out2,1.0f-amount);
						val1 = amount*val2 + (1.0f - amount)*val1;
					}
				}
			}
		}
	}
	outVal = val1;
	return true;
}
#endif // NO_MTL_COMPOSITE