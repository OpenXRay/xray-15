/**********************************************************************
 *<
	FILE:			Wood.cpp

	DESCRIPTION:	Wood 3D Texture map.

	CREATED BY:		Suryan Stalin

	HISTORY:		Modified from Marble.cpp by adding IPAS wood stuff

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

				   
//Includes
#include "mtlhdr.h"
#include "woodres.h"
#include "iparamm2.h"
#include "stdmat.h"
#include "wooddent.h"
#include "wood.h"
#include "macrorec.h"


// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { wood_params, };  // pblock ID
// dents_params param IDs
enum 
{ 
	wood_map1,wood_map2,wood_color1,wood_color2,
	wood_map1_on, wood_map2_on, // main grad params 

	wood_thickness,wood_rnoise, wood_anoise,
	wood_coords,
//	wood_seed

};

//externs
extern		HINSTANCE			hInstance;

ParamDlg *Wood::xyzGenDlg;

//Globals
static		WoodClassDesc		woodCD;
static		int					subTexId[NSUBTEX] = { IDC_WOOD_TEX1, IDC_WOOD_TEX2 };
static		ParamBlockDescID	pbdesc[] =	{
											{	TYPE_FLOAT, NULL, TRUE,wood_thickness }, 	// size
											{	TYPE_FLOAT, NULL, TRUE,wood_rnoise }, 	// r1
											{	TYPE_FLOAT, NULL, TRUE,wood_anoise }, 	// r2
											{	TYPE_RGBA, NULL, TRUE,wood_color1 },  // col1
											{	TYPE_RGBA, NULL, TRUE,wood_color2 }   // col2
											};
static		int					nameID[] =	{	IDS_DS_WOODSIZE, 
												IDS_DS_WOODR1, 
												IDS_DS_WOODR2, 
												IDS_DS_COLOR1, 
												IDS_DS_COLOR2 
											};
static		int					colID[2] =	{ IDC_WOOD_COL1, IDC_WOOD_COL2 };
static		Class_ID			woodClassID(WOOD_CLASS_ID,0);
static		float				noiseTable[NOISE_DIM+1][NOISE_DIM+1][NOISE_DIM+1];
static      int					noiseInited = 0;
// Array of old versions
static ParamVersionDesc versions[1] = {
	ParamVersionDesc(pbdesc,5,1)	
	};
// per instance gradient block
static ParamBlockDesc2 wood_param_blk ( wood_params, _T("parameters"),  0, &woodCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 1, 
	//rollout
	IDD_WOOD, IDS_DS_WOOD_PARAMS, 0, 0, NULL, 
	// params
	wood_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP1,
		p_refno,		2,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_WOOD_TEX1,
		end,
	wood_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP2,
		p_refno,		3,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_WOOD_TEX2,
		end,
	wood_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_WOOD_COL1, 
		end,
	wood_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(1,1,1), 
		p_ui,			TYPE_COLORSWATCH, IDC_WOOD_COL2, 
		end,
	wood_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	wood_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,

	wood_thickness,	_T("thickness"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_WOODSIZE,
		p_default,		7.0,
		p_range,		0.0, 999999999.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_WOODSIZE_EDIT, IDC_WOODSIZE_SPIN, .1f,
		end,
	wood_rnoise,	_T("radialNoise"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_WOODR1,
		p_default,		1.0,
		p_range,		0.0, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_R1_EDIT, IDC_R1_SPIN, .1f,
		end,
	wood_anoise,	_T("axialNoise"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_WOODR2,
		p_default,		1.0f,
		p_range,		0.0, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,   IDC_R2_EDIT, IDC_R2_SPIN, .1f,
		end,
	wood_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_COORDINATES,
		p_refno,		0, 
		end,
//	wood_seed,	_T("seed"),   TYPE_INT,			0,	IDS_PW_SEED,
//		p_default,		65432,
//		p_range,		0, 99999999,
//		p_ui, 			TYPE_SPINNER, EDITTYPE_INT,   IDC_WOODSEED_EDIT, IDC_WOODSEED_SPIN, 1.0f,
//		end,

	end
);

//Class Implementations

//dialog stuff to get the Set Ref button
class WoodDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Wood *wood;		
		WoodDlgProc(Wood *m) {wood = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			wood = (Wood*)m;
			}
	};



BOOL WoodDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_WOOD_SWAP:
					{
					wood->SwapInputs();
					}
				}
			break;
		}
	return FALSE;
	}


//  Wood
Wood::Wood() 
	{
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	Param1 = FALSE;
	subTex[0] = subTex[1] = NULL;
	pblock = NULL;
	xyzGen = NULL;
	seed = 0;
	InitNoise();
	woodCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	vers = 0;
	}

void Wood::Init() 
	{
	if (xyzGen) xyzGen->Reset();
	else ReplaceReference( 0, GetNewDefaultXYZGen());	
	ivalid.SetEmpty();
	SetColor(0, Color(0.79f,0.69f,0.27f), TimeValue(0));
	SetColor(1, Color(0.51f,0.32f,0.05f), TimeValue(0));
	SetSize(7.0f, TimeValue(0));
	SetR1(1.0f, TimeValue(0));
	SetR2(1.0f, TimeValue(0));
	for (int i=0; i<NSUBTEX; i++) mapOn[i] = TRUE;
	}

void Wood::Reset() 
	{
	woodCD.Reset(this, TRUE); // reset pb2 params
	DeleteReference(2);
	DeleteReference(3);
	Init();
	}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Wood::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif


void Wood::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

Class_ID Wood::ClassID() 
{	
	return woodClassID; 
}

RefTargetHandle Wood::Clone(RemapDir &remap) 
{
	Wood *mnew = new Wood();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(xyzGen));
	mnew->ReplaceReference(1,remap.CloneRef(pblock));
	mnew->col[0] = col[0];
	mnew->col[1] = col[1];
	mnew->r1 = r1;
	mnew->r2 = r2;
	mnew->size = size;
//	mnew->seed = seed;
//	mnew->InitNoise();
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i+2,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

ParamDlg* Wood::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	// create the rollout dialogs
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = woodCD.CreateParamDlgs(hwMtlEdit, imp, this);
// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
//attach a dlg proc to handle the swap button 
	wood_param_blk.SetUserDlgProc(new WoodDlgProc(this));

	return masterDlg;

}

BOOL Wood::SetDlgThing(ParamDlg* dlg)
{
	// PW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if (dlg == xyzGenDlg)
		xyzGenDlg->SetThing(xyzGen);
	else 
		return FALSE;
	return TRUE;
}

void Wood::ReadSXPData(TCHAR *name, void *sxpdata) 
{
	WoodState *state = (WoodState*)sxpdata;
	if (state->version==WOOD_VERS) {
		SetColor(0, ColrFromCol24(state->col1),0);
		SetColor(1, ColrFromCol24(state->col2),0);
		SetR1(state->r1,0);
		SetR2(state->r2,0);
		SetSize(state->size,0);
		}
}

void Wood::Update(TimeValue t, Interval& valid) 
{		

/*	if (Param1)
		{
		pblock->SetValue(wood_map1_on,t,mapOn[0]);
		pblock->SetValue(wood_map2_on,t,mapOn[1]);
		pblock->SetValue(wood_seed,t,seed);
		Param1 = FALSE;
		}
*/

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		xyzGen->Update(t,ivalid);
		pblock->GetValue( wood_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( wood_color2, t, col[1], ivalid );
		col[1].ClampMinMax();
		pblock->GetValue( wood_rnoise, t, r1, ivalid );
		pblock->GetValue( wood_anoise, t, r2, ivalid );
		pblock->GetValue( wood_thickness, t, size, ivalid );
		pblock->GetValue( wood_map1_on, t, mapOn[0], ivalid );
		pblock->GetValue( wood_map2_on, t, mapOn[1], ivalid );

//		int rseed =-9876545;
//		pblock->GetValue( wood_seed, t, rseed, ivalid );
//		if (rseed != seed)
//			{
//			seed = rseed;
//			InitNoise();
//			}
//		else seed = rseed;
//
		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
}


void Wood:: SetColor(int i, Color c, TimeValue t) 
{
//    col[i] = c;
	col[i].r = c.r;
	col[i].g = c.g;
	col[i].b = c.b;
	pblock->SetValue( i==0?wood_color1:wood_color2, t, c);
}

void Wood::SwapInputs() 
{
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
	pblock->SwapControllers(wood_color1,0,wood_color2,0);
	wood_param_blk.InvalidateUI(wood_color1);
	wood_param_blk.InvalidateUI(wood_color2);
	wood_param_blk.InvalidateUI(wood_map1);
	wood_param_blk.InvalidateUI(wood_map2);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);

}

void Wood::SetR1(float f, TimeValue t) 
{ 
	r1 = f; 
	pblock->SetValue( wood_rnoise, t, f);
}

void Wood::SetR2(float f, TimeValue t) 
{ 
	r2 = f; 
	pblock->SetValue( wood_anoise, t, f);
}

void Wood::SetSize(float f, TimeValue t) 
{ 
	size = f; 
	pblock->SetValue( wood_thickness, t, f);
}

RefTargetHandle Wood::GetReference(int i) 
{
	switch(i) {
		case 0: return xyzGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
		}
}

void Wood::SetReference(int i, RefTargetHandle rtarg) 
{
	switch(i) {
		case 0: xyzGen = (XYZGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
		}
}

void Wood::SetSubTexmap(int i, Texmap *m) 
{
	ReplaceReference(i+2,m);
	if (i==0)
		{
		wood_param_blk.InvalidateUI(wood_map1);
		ivalid.SetEmpty();
		}
	else if (i==1)
		{
		wood_param_blk.InvalidateUI(wood_map2);
		ivalid.SetEmpty();
		}

}

TSTR Wood::GetSubTexmapSlotName(int i) 
{
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_COLOR1)); 
		case 1:  return TSTR(GetString(IDS_DS_COLOR2)); 
		default: return TSTR(_T(""));
		}
}
	 
Animatable* Wood::SubAnim(int i) 
{
	switch (i) {
		case 0: return xyzGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
		}
}

TSTR Wood::SubAnimName(int i) 
{
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_COORDINATES));		
		case 1: return TSTR(GetString(IDS_DS_PARAMETERS));		
		default: return GetSubTexmapTVName(i-2);
		}
}


RefResult Wood::NotifyRefChanged(	Interval changeInt, RefTargetHandle hTarget, 
									PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock ) 
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
//			if (hTarget != xyzGen  && hTarget != pblock ) 
				wood_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
#ifdef SHOW_3DMAPS_WITH_2D
				if (changing_param != -1)
					DiscardTexHandle();
#endif
				}
			if (hTarget == xyzGen ) 
				{
#ifdef SHOW_3DMAPS_WITH_2D
				DiscardTexHandle();
#endif
				// NotifyChanged();  //DS this is redundant
				}

			break;
		}
	return(REF_SUCCEED);
}

#define MAPOFF_CHUNK 0x1000
#define PARAM2_CHUNK 0x1010

IOResult Wood::Save(ISave *isave) 
{ 
//	ULONG nb;
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

//	isave->BeginChunk(WOOD_NOISE_CHUNK);
//	isave->Write(&seed,sizeof(int),&nb);			
//	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
	
	return IO_OK;
}	

class WoodPostLoad : public PostLoadCallback 
{
	public:
		Wood *chk;
		WoodPostLoad(Wood *b) {chk=b;}
		void proc(ILoad *iload) {
			if (chk->vers<1) {
				if (chk->pblock) 
					chk->pblock->RescaleParam(wood_thickness, 0, 100.0f);
//					ScaleFloatController(chk->pblock, PB_SIZE, 100.0f);
//				iload->SetObsolete();
				}
			delete this;
			}
};
	  
//watje

class Wood2PostLoadCallback:public  PostLoadCallback
{
public:
	Wood      *s;
	int Param1;
	Wood2PostLoadCallback(Wood *r, BOOL b) {s=r;Param1 = b;}
	void proc(ILoad *iload);
};

void Wood2PostLoadCallback::proc(ILoad *iload)
{
	if (Param1)
		{
		TimeValue t  = 0;
		s->pblock->SetValue(wood_map1_on,t,s->mapOn[0]);
		s->pblock->SetValue(wood_map2_on,t,s->mapOn[1]);
//		s->pblock->SetValue(wood_seed,t,s->seed);
		Param1 = FALSE;
		}

//	Interval ivalid;
//	s->pblock->GetValue( wood_seed, 0, s->seed, ivalid );
//	s->InitNoise();

	delete this;
}


IOResult Wood::Load(ILoad *iload) 
{ 
//	ULONG		nb;
	IOResult res;
	int id;
	vers = 0;
	Param1 = TRUE;

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch(id = iload->CurChunkID())  
		{
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case WOODVERS1_CHUNK:
				vers = 1;

				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;

#if 0
			case WOOD_NOISE_CHUNK:
				iload->Read(&seed,sizeof(int),&nb);			
//				InitNoise();
				break;
#endif
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;

			
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (Param1)
		{
		ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &wood_param_blk, this, 1);
		iload->RegisterPostLoadCallback(plcb);
		}
	Wood2PostLoadCallback* wood2plcb = new Wood2PostLoadCallback(this,Param1);
	iload->RegisterPostLoadCallback(wood2plcb);

	return IO_OK;
}

void Wood::InitNoise()
{
//	if(!seed)
//		seed = rand()+1;
	if (!noiseInited) {
		noiseInited = 1;
		srand(65432);
		int i, j, k, ii, jj, kk;
		for (i=0; i<=NOISE_DIM; i++)
			for (j=0; j<=NOISE_DIM; j++)
				for (k=0; k<=NOISE_DIM; k++)
				{
					noiseTable[i][j][k] = (float)(rand()&0x7FFF);
					ii = (i==NOISE_DIM)?0:i; 
					jj = (j==NOISE_DIM)?0:j; 
					kk = (k==NOISE_DIM)?0:k; 
					noiseTable[i][j][k] = noiseTable[ii][jj][kk];
				}
		}
}

float Wood::WoodNoise(float x)
{
   int ix;
   double fx, mx;
   double n0, n1;
   mx = fmod((double)x, FNOISE_DIM); 
   if (mx<0) mx += FNOISE_DIM;
   ix = (int)mx;
   fx = fmod(mx, 1.0);
   n0 = noiseTable[ix][0][0];
   n1 = noiseTable[ix+1][0][0];

   return (float)((n0+fx*(n1-n0))/32768.0);
}

/*
float noise(float x, float y, float z)
{
   int ix, iy, iz;
   float fx, fy, fz, mx, my, mz;
   float n, n00, n01, n10, n11, n0, n1;
   mx = fmod(x, FNOISE_DIM); if (mx<0) mx += FNOISE_DIM;
   my = fmod(y, FNOISE_DIM); if (my<0) my += FNOISE_DIM;
   mz = fmod(z, FNOISE_DIM); if (mz<0) mz += FNOISE_DIM;
   ix = (int)mx;
   iy = (int)my;
   iz = (int)mz;
   fx = fmod(mx, 1.0);
   fy = fmod(my, 1.0);
   fz = fmod(mz, 1.0);
   n = noise_table[ix][iy][iz];
   n00 = n + fx*(noise_table[ix+1][iy][iz]-n);
   n = noise_table[ix][iy][iz+1];
   n01 = n + fx*(noise_table[ix+1][iy][iz+1]-n);
   n = noise_table[ix][iy+1][iz];
   n10 = n + fx*(noise_table[ix+1][iy+1][iz]-n);
   n = noise_table[ix][iy+1][iz+1];
   n11 = n + fx*(noise_table[ix+1][iy+1][iz+1]-n);
   n0 = n00 + fy*(n10-n00);
   n1 = n01 + fy*(n11-n01);
   return(((float)(n0+fz*(n1-n0)))/32768.0);
}

*/

void Wood::LerpColor(RGBA *c, RGBA *a, RGBA *b, float f)
{
   int alph, ialph;
   
   if (f>1.0) f = 1.0f;
   alph = (int)(4096*f);
   ialph = 4096-alph;

   c->r = (float)(((int)(ialph*a->r + alph*b->r)*255)>>12);
   c->g = (float)(((int)(ialph*a->g + alph*b->g)*255)>>12);
   c->b = (float)(((int)(ialph*a->b + alph*b->b)*255)>>12);
   
   c->r /= 255.0f;
   c->g /= 255.0f;
   c->b /= 255.0f;

}

/* smooth step function with hermite interpolation*/
float Wood::SmoothStep(float x0, float x1, float v)
{
   if (v<=x0) return(0.0f);
   else if (v>=x1) return(1.0f);
   else {
      float u = (v-x0)/(x1-x0);
      return(u*u*(3-2*u));
   }
}

float Wood::WoodFunc(Point3 p) {
	float r;
	float px = p.x/size;
	float py = p.y/size;
	float pz = p.z/size;
	px += WoodNoise(px)*r1;
	py += WoodNoise(py)*r1;
	pz += WoodNoise(pz)*r1;
	r = (float) sqrt(py*py+pz*pz);
	r += WoodNoise(r)+r2*WoodNoise(px/4.0f);
	r = (float)fmod((double)r, 1.0); /* be periodic */
	r = SmoothStep(0.0f, 0.8f, r) - SmoothStep(0.83f, 1.0f, r);
	return(r);	
	}

static AColor black(0.0f,0.0f,0.0f,0.0f);

RGBA Wood::EvalColor(ShadeContext& sc) 
{
	Point3	p,dp;
	RGBA	c;
	if (!sc.doMaps) return black;
	if (gbufID) sc.SetGBufferID(gbufID);
	
	xyzGen->GetXYZ(sc,p,dp);
	if (size==0.0f) size=.0001f;
//	p *= FACT/size;

	float d = WoodFunc(p);
	
	if (d<=.0005f) 
		return  (mapOn[0]&&subTex[0]) ? subTex[0]->EvalColor(sc): col[0];
	else 
		if (d>=.9995) 
			return  (mapOn[1]&&subTex[1]) ? subTex[1]->EvalColor(sc): col[1];
	RGBA c0 = (mapOn[0]&&subTex[0]) ? subTex[0]->EvalColor(sc): col[0];
	RGBA c1 = (mapOn[1]&&subTex[1]) ? subTex[1]->EvalColor(sc): col[1];
	c = (1.0f-d)*c0 + d*c1;
	return c;
}

Point3 Wood::EvalNormalPerturb(ShadeContext& sc) 
{
	float del,d;
	Point3 p,dp;
	
	if (!sc.doMaps) return Point3(0,0,0);
	if (gbufID) sc.SetGBufferID(gbufID);
	xyzGen->GetXYZ(sc,p,dp);
	if (size==0.0f) size=.0001f;
//	p *= FACT/size;

	d = WoodFunc(p);
//	del = 20.0f;
	del = 0.1f;
	Point3 np;
	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);
    np.x = (WoodFunc(p+del*M[0]) - d)/del;
	np.y = (WoodFunc(p+del*M[1]) - d)/del;
	np.z = (WoodFunc(p+del*M[2]) - d)/del;
//	return np*100.0f;
	np = sc.VectorFromNoScale(np,REF_OBJECT);
	Texmap *sub0 = mapOn[0]?subTex[0]:NULL;
	Texmap *sub1 = mapOn[1]?subTex[1]:NULL;
	if (sub0||sub1) {
		// d((1-k)*a + k*b ) = dk*(b-a) + k*(db-da) + da
		float a,b;
		Point3 da,db;
		if (sub0) { 	a = sub0->EvalMono(sc); 	da = sub0->EvalNormalPerturb(sc);		}
		else {	 a = Intens(col[0]);	 da = Point3(0.0f,0.0f,0.0f);		 }
		if (sub1) { 	b = sub1->EvalMono(sc); 	db = sub1->EvalNormalPerturb(sc);	}
		else {	 b = Intens(col[1]);	 db= Point3(0.0f,0.0f,0.0f);		 }
		np = (b-a)*np + d*(db-da) + da;
		}
	else 
		np *= Intens(col[1])-Intens(col[0]);
	return np;
	}

//WoodClassDesc
Class_ID WoodClassDesc::ClassID() 
{ 
	return woodClassID; 
}
ClassDesc* GetWoodDesc()		{ return &woodCD;  }

