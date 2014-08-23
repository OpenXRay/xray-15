/**********************************************************************
 *<
	FILE: OUTPUT.CPP

	DESCRIPTION: OUTPUT Composite.

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "iparamm2.h"
#include "buildver.h"
#ifndef NO_MAPTYPE_OUTPUT // orb 01-03-2001 Removing map types

extern HINSTANCE hInstance;



#define NSUBTEX 1    // number of texture map slots

static Class_ID outputClassID(OUTPUT_CLASS_ID,0);

static int subTexId[NSUBTEX] = { IDC_OUT_MAP};
enum { output_params };  // pblock ID
// output_params param IDs
enum 
{ 
	output_map1,
	output_map1_on, // main grad params 
	output_output
};


//--------------------------------------------------------------
// Output: A Composite texture map
//--------------------------------------------------------------
class Output: public Texmap { 
	Texmap* subTex[NSUBTEX];  // ref 0
	TextureOutput *texout; // ref 1

	Interval ivalid;
	BOOL rollScroll;
	public:
		BOOL Param1;
		static ParamDlg* texoutDlg;
		IParamBlock2 *pblock;   // ref #2	
		BOOL mapOn[NSUBTEX];

		Output();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void NotifyChanged();

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NSUBTEX; }
		Texmap* GetSubTexmap(int i) { return subTex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID() {	return outputClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_OUTPUT); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return 3; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 3; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		// From Texmap
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }
	};

class OutputClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Output; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_OUTPUT_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return outputClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_COLMOD;  }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("output"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static OutputClassDesc maskCD;

ClassDesc* GetOutputDesc() { return &maskCD;  }

//-----------------------------------------------------------------------------
//  Output
//-----------------------------------------------------------------------------

ParamDlg* Output::texoutDlg;


static ParamBlockDesc2 output_param_blk ( output_params, _T("parameters"),  0, &maskCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 2, 
	//rollout
	IDD_OUTPUT, IDS_DS_OUTPUTPARAMS, 0, 0, NULL, 
	// params
	output_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		0,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_OUT_MAP,
		end,
	output_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	output_output,		_T("output"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_OUTPUT,
		p_refno,		1, 
		end,

	end
);

void Output::Init() {
	ivalid.SetEmpty();
	if (texout) texout->Reset();
	else ReplaceReference( 1, GetNewDefaultTextureOutput());	
	texout->SetRollupOpen(1);
	mapOn[0] = 1;
	}

void Output::Reset() {
	DeleteReference(0);	// get rid of map
	Init();
	maskCD.Reset(this, TRUE);	// reset all pb2's
	}

void Output::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Output::Output() {
	for (int i=0; i<NSUBTEX; i++) subTex[i] = NULL;
	texout   = NULL;
	maskCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}


static AColor white(1.0f,1.0f,1.0f,1.0f);

AColor Output::EvalColor(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	return texout->Filter((subTex[0]&&mapOn[0])? subTex[0]->EvalColor(sc): white);
	}

float Output::EvalMono(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	return texout->Filter((subTex[0]&&mapOn[0])? subTex[0]->EvalMono(sc): 1.0f);
	}

Point3 Output::EvalNormalPerturb(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	return texout->Filter((subTex[0]&&mapOn[0])? subTex[0]->EvalNormalPerturb(sc): Point3(0,0,0));
	}

RefTargetHandle Output::Clone(RemapDir &remap) {
	Output *mnew = new Output();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(1,remap.CloneRef(texout));
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* Output::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	// create the rollout dialogs
	IAutoMParamDlg* masterDlg = maskCD.CreateParamDlgs(hwMtlEdit, imp, this);
	texoutDlg = texout->CreateParamDlg(hwMtlEdit, imp);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(texoutDlg);
	return masterDlg;

	}

BOOL Output::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((texoutDlg!= NULL) && (dlg == texoutDlg))
		texoutDlg->SetThing(texout);
	else 
		return FALSE;
	return TRUE;
}


void Output::Update(TimeValue t, Interval& valid) {		

	if (Param1)
		{
		pblock->SetValue( output_map1_on, 0, mapOn[0]);
		Param1 = FALSE;
		}

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		texout->Update(t,ivalid);
		pblock->GetValue( output_map1_on, t, mapOn[0], ivalid);

		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}

RefTargetHandle Output::GetReference(int i) {
	if (i==0) return subTex[0];
	else if (i==1) return texout;
	else  return pblock;

	}

void Output::SetReference(int i, RefTargetHandle rtarg) {
	if (i==0) subTex[0] = (Texmap *)rtarg; 
	else if (i==1) texout = (TextureOutput *)rtarg;
	else pblock = (IParamBlock2 *)rtarg;
	}

void Output::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i,m);
	if (i==0)
		{
		output_param_blk.InvalidateUI(output_map1);
		ivalid.SetEmpty();
		}	

	}

TSTR Output::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_MAP)); 
		default: assert(0); return TSTR(_T(""));
		}
	}
	 
Animatable* Output::SubAnim(int i) {
	switch(i) {
		case 0: return subTex[0];
		case 1: return texout;
		case 2: return pblock;
		default: assert(0); return NULL;
		}
	}

TSTR Output::SubAnimName(int i) {
	switch (i) {
		case 0: return GetSubTexmapTVName(0);
		case 1: return TSTR(GetString(IDS_DS_OUTPUT));
		case 2: TSTR(GetString(IDS_DS_PARAMETERS));		
		default: assert(0); return _T("");
		}
	}

RefResult Output::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
//				if (hTarget != pblock && hTarget != texout ) 
					output_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
				}
			else if (hTarget == texout ) 
				{
				// NotifyChanged();  //DS this is redundant
				}


			break;

		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define MAPOFF_CHUNK 0x1000
#define PARAM2_CHUNK 0x1010

IOResult Output::Save(ISave *isave) { 
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();


	return IO_OK;
	}	
	  
class OutputPostLoad : public PostLoadCallback {
	public:
		Output *n;
		OutputPostLoad(Output *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->Param1)
				{
				n->pblock->SetValue( output_map1_on, 0, n->mapOn[0]);
				}
			delete this; 


			} 
	};


IOResult Output::Load(ILoad *iload) { 
//	ULONG nb;
	IOResult res;
	int id;
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
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

//	iload->RegisterPostLoadCallback(new OutputPostLoad(this));

	return IO_OK;
	}


#endif // NO_MAPTYPE_OUTPUT