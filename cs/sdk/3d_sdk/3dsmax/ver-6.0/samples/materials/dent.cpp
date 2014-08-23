/**********************************************************************
 *<
	FILE:			Dent.cpp

	DESCRIPTION:	Dent 3D Texture map.

	CREATED BY:		Suryan Stalin, on 4th April 1996

	HISTORY:		Modified from Marble.cpp by adding IPAS dent stuff
	*>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


//Includes
#include "buildver.h"
#include "mtlhdr.h"
#include "woodres.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "wooddent.h"
#include "dent.h"
#include "macrorec.h"
#ifndef NO_MAPTYPE_DENT // orb 01-03-2001 Removing map types

// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { dents_params, };  // pblock ID
// dents_params param IDs
enum 
{ 
	dents_map1,dents_map2,dents_color1,dents_color2,
	dents_map1_on, dents_map2_on, // main grad params 

	dents_size,dents_strength, dents_iterations,
	dents_coords,
//	dents_seed

};

//Externs
extern		HINSTANCE			hInstance;

ParamDlg *Dent::xyzGenDlg;

//Globals
static		DentClassDesc		dentCD;
static		int					subTexId[NSUBTEX] = { IDC_DENT_TEX1, IDC_DENT_TEX2 };
static		ParamBlockDescID	pbdesc[] =	{
											{	TYPE_FLOAT, NULL, TRUE,dents_size }, 	// size
											{	TYPE_FLOAT, NULL, TRUE,dents_strength }, 	// km
											{	TYPE_INT, NULL, TRUE,dents_iterations }, 	// nits
											{	TYPE_RGBA, NULL, TRUE,dents_color1 },  // col1
											{	TYPE_RGBA, NULL, TRUE,dents_color2 }   // col2
											};
static		int					nameID[] =	{	IDS_DS_DENTSIZE, 
												IDS_DS_DENT_KM, 
												IDS_DS_DENT_NITS, 
												IDS_DS_COLOR1, 
												IDS_DS_COLOR2 
											};
static		int					colID[2] =	{ IDC_DENT_COL1, IDC_DENT_COL2 };
static		Class_ID			dentClassID(DENT_CLASS_ID,0);
static		float				noiseTable[NOISE_DIM+1][NOISE_DIM+1][NOISE_DIM+1];
static 		int   	noiseInited = 0;

// Array of old versions
static ParamVersionDesc versions[1] = {
	ParamVersionDesc(pbdesc,5,1)	
	};


// per instance gradient block
static ParamBlockDesc2 dents_param_blk ( dents_params, _T("parameters"),  0, &dentCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 1, 
	//rollout
	IDD_DENT, IDS_DS_DENT_PARAMS, 0, 0, NULL, 
	// params
	dents_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP1,
		p_refno,		2,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_DENT_TEX1,
		end,
	dents_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_PW_MAP2,
		p_refno,		3,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_DENT_TEX2,
		end,
	dents_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_DENT_COL1, 
		end,
	dents_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(1,1,1), 
		p_ui,			TYPE_COLORSWATCH, IDC_DENT_COL2, 
		end,
	dents_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	dents_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,

	dents_size,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_DENTSIZE,
		p_default,		200.0,
		p_range,		0.0, 999999999.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_DENTSIZE_EDIT, IDC_DENTSIZE_SPIN, 1.0f,
		end,
	dents_strength,	_T("strength"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_DENT_KM,
		p_default,		20.0,
		p_range,		0.0, 999999999.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_KM_EDIT, IDC_KM_SPIN, .1f,
		end,
	dents_iterations,	_T("iterations"),   TYPE_INT,			P_ANIMATABLE,	IDS_DS_DENT_NITS,
		p_default,		2,
		p_range,		0, 10,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT,   IDC_NITS_EDIT, IDC_NITS_SPIN, 1.0f,
		end,
	dents_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_COORDINATES,
		p_refno,		0, 
		end,
//	dents_seed,	_T("seed"),   TYPE_INT,			0,	IDS_PW_SEED,
//		p_default,		65432,
//		p_range,		0, 999999999,
//		p_ui, 			TYPE_SPINNER, EDITTYPE_INT,  IDC_DENTSEED_EDIT, IDC_DENTSEED_SPIN, 1.0f,
//		end,

	end
);



//Class Implementations

//dialog stuff to get the Set Ref button
class DentsDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Dent *dent;		
		DentsDlgProc(Dent *m) {dent = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}

		void SetThing(ReferenceTarget *m) {
			dent = (Dent*)m;
			}

	};



BOOL DentsDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_DENT_SWAP:
					{
					dent->SwapInputs();
					}
				}
			break;
		}
	return FALSE;
	}


//  Dent
void Dent::Init() 	{
	if (xyzGen) xyzGen->Reset();
	else ReplaceReference( 0, GetNewDefaultXYZGen());	
	ivalid.SetEmpty();
	SetColor(0, Color(0.0f,0.0f,0.0f), TimeValue(0));
	SetColor(1, Color(1.0f,1.0f,1.0f), TimeValue(0));
	SetSize(200.0f, TimeValue(0));
	SetKm(20.0f, TimeValue(0));
	SetNits(2, TimeValue(0));
	mapOn[0] = mapOn[1] = 1;
	}

void Dent::Reset() 	{
	dentCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(2);
	DeleteReference(3);
	Init();
	}

Dent::Dent() {
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	Param1 = FALSE;
	subTex[0] = subTex[1] = NULL;
	pblock = NULL;
	xyzGen = NULL;
	//seed = 0;
	InitNoise();
	dentCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	vers = 0;
	}


void Dent::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

Class_ID Dent::ClassID() 
{	
	return dentClassID; 
}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Dent::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif


RefTargetHandle Dent::Clone(RemapDir &remap) 
{
	Dent *mnew = new Dent();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(xyzGen));
	mnew->ReplaceReference(1,remap.CloneRef(pblock));
	mnew->col[0] = col[0];
	mnew->col[1] = col[1];
	mnew->km = km;
	mnew->nits = nits;
	mnew->size = size;
	mnew->InitNoise();
	//mnew->seed = seed;
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

ParamDlg* Dent::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	// create the rollout dialogs
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = dentCD.CreateParamDlgs(hwMtlEdit, imp, this);
// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
//attach a dlg proc to handle the swap button 
	dents_param_blk.SetUserDlgProc(new DentsDlgProc(this));

	return masterDlg;

}

BOOL Dent::SetDlgThing(ParamDlg* dlg)
{
	// PW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if (dlg == xyzGenDlg)
		xyzGenDlg->SetThing(xyzGen);
	else 
		return FALSE;
	return TRUE;
}


void Dent::ReadSXPData(TCHAR *name, void *sxpdata) 
{
	DentState *state = (DentState*)sxpdata;
	if (state->version==DENT_VERS) {
		SetColor(0, ColrFromCol24(state->col1),0);
		SetColor(1, ColrFromCol24(state->col2),0);
		SetKm(state->km,0);
		SetNits(state->nits,0);
		SetSize(state->size,0);
		}
}

void Dent::Update(TimeValue t, Interval& valid) 
{		


/*	if (Param1)  //hack to fix 2.x files 
		{
		pblock->SetValue(dents_map1_on,t,mapOn[0]);
		pblock->SetValue(dents_map2_on,t,mapOn[1]);

		pblock->SetValue(dents_seed,t,seed);
		Param1 = FALSE;
		}
*/

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		xyzGen->Update(t,ivalid);
		pblock->GetValue(dents_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( dents_color2, t, col[1], ivalid );
		col[1].ClampMinMax();
		pblock->GetValue( dents_strength, t, km, ivalid );
		pblock->GetValue( dents_iterations, t, nits, ivalid );
		pblock->GetValue( dents_size, t, size, ivalid );
		pblock->GetValue( dents_map1_on, t, mapOn[0], ivalid );
		pblock->GetValue( dents_map2_on, t, mapOn[1], ivalid );

//		int rseed =-9876545;
//		pblock->GetValue( dents_seed, t, rseed, ivalid );
//		if (rseed != seed)
//			{
//			seed = rseed;
//			InitNoise();
//			}
//		else seed = rseed;

		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
}


void Dent:: SetColor(int i, Color c, TimeValue t) 
{
//    col[i] = c;
	col[i].r = c.r;
	col[i].g = c.g;
	col[i].b = c.b;
	pblock->SetValue( i==0?dents_color1:dents_color2, t, c);
}

void Dent::SwapInputs() 
{
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
	pblock->SwapControllers(dents_color1,0,dents_color2,0);
	dents_param_blk.InvalidateUI(dents_color1);
	dents_param_blk.InvalidateUI(dents_color2);
	dents_param_blk.InvalidateUI(dents_map1);
	dents_param_blk.InvalidateUI(dents_map2);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRec->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);
}

void Dent::SetKm(float f, TimeValue t) 
{ 
	km = f; 
	pblock->SetValue( dents_strength, t, f);
}

void Dent::SetNits(int f, TimeValue t) 
{ 
	nits = f; 
	pblock->SetValue( dents_iterations, t, f);
}

void Dent::SetSize(float f, TimeValue t) 
{ 
	size = f; 
	pblock->SetValue( dents_size, t, f);
}

RefTargetHandle Dent::GetReference(int i) 
{
	switch(i) {
		case 0: return xyzGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
		}
}

void Dent::SetReference(int i, RefTargetHandle rtarg) 
{
	switch(i) {
		case 0: xyzGen = (XYZGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
		}
}

void Dent::SetSubTexmap(int i, Texmap *m) 
{
	ReplaceReference(i+2,m);
	if (i==0)
		{
		dents_param_blk.InvalidateUI(dents_map1);
		ivalid.SetEmpty();
		}
	else if (i==1)
		{
		dents_param_blk.InvalidateUI(dents_map2);
		ivalid.SetEmpty();
		}


}

TSTR Dent::GetSubTexmapSlotName(int i) 
{
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_COLOR1)); 
		case 1:  return TSTR(GetString(IDS_DS_COLOR2)); 
		default: return TSTR(_T(""));
		}
}
	 
Animatable* Dent::SubAnim(int i) 
{
	switch (i) {
		case 0: return xyzGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
		}
}

TSTR Dent::SubAnimName(int i) 
{
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_COORDINATES));		
		case 1: return TSTR(GetString(IDS_DS_PARAMETERS));		
		default: return GetSubTexmapTVName(i-2);
		}
}


RefResult Dent::NotifyRefChanged(	Interval changeInt, RefTargetHandle hTarget, 
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
//				if (hTarget != xyzGen  && hTarget != pblock ) 
				dents_param_blk.InvalidateUI(changing_param);
#ifdef SHOW_3DMAPS_WITH_2D
				if (changing_param != -1)
					DiscardTexHandle();
#endif
			// notify our dependents that we've changed
//				NotifyChanged();
				}
			else if (hTarget == xyzGen ) 
				{
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
#ifdef SHOW_3DMAPS_WITH_2D
				DiscardTexHandle();
#endif
				}

			break;
		}
	return(REF_SUCCEED);
}

#define MAPOFF_CHUNK 0x1000
#define PARAM2_CHUNK 0x1010

IOResult Dent::Save(ISave *isave) 
{ 
//	ULONG nb;
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

//	isave->BeginChunk(DENT_NOISE_CHUNK);
//	isave->Write(&seed,sizeof(int),&nb);			
//	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
	
	return IO_OK;
}	

//watje
class Dents2PostLoadCallback:public  PostLoadCallback
{
public:
	Dent      *s;
	int Param1;
	Dents2PostLoadCallback(Dent *r, BOOL b) {s=r;Param1 = b;}
	void proc(ILoad *iload);
};

void Dents2PostLoadCallback::proc(ILoad *iload)
{
	if (Param1)
		{
		TimeValue t  = 0;
		s->pblock->SetValue(dents_map1_on,t,s->mapOn[0]);
		s->pblock->SetValue(dents_map2_on,t,s->mapOn[1]);
//		s->pblock->SetValue(dents_seed,t,s->seed);
		Param1 = FALSE;

		}
	Interval ivalid;
//	s->pblock->GetValue( dents_seed, 0, s->seed, ivalid );
//	s->InitNoise();

	delete this;
}

class DentPostLoad : public PostLoadCallback 
{
	public:
		Dent *chk;
		DentPostLoad(Dent *b) {chk=b;}
		void proc(ILoad *iload) {
			if (chk->vers<1) {
				if (chk->pblock) 
					{
//					ScaleFloatController(chk->pblock, PB_SIZE, 100.0f);
					chk->pblock->RescaleParam(dents_size, 0, 100.0f);
					}
//				iload->SetObsolete();
				}
			delete this;
			}
};


IOResult Dent::Load(ILoad *iload) 
{ 
//	ULONG nb;
	IOResult res;
	int id;
	vers = 0;
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case DENTVERS1_CHUNK:
				vers = 1;
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;

#if 0
			case DENT_NOISE_CHUNK:
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
		ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &dents_param_blk, this, 1);
		iload->RegisterPostLoadCallback(plcb);
		}
	Dents2PostLoadCallback* dents2plcb = new Dents2PostLoadCallback(this,Param1);
	iload->RegisterPostLoadCallback(dents2plcb);

	return IO_OK;
}

void Dent::InitNoise()
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


float Dent::DentNoise(float x, float y, float z)
{
#if 0
	Point3 p(x,y,z);
	return 0.5f*(noise3(p)+1.0f);
#else
   int ix, iy, iz;
   double fx, fy, fz, mx, my, mz;
   double n, n00, n01, n10, n11, n0, n1;
   mx = fmod((double)x, FNOISE_DIM); if (mx<0) mx += FNOISE_DIM;
   my = fmod((double)y, FNOISE_DIM); if (my<0) my += FNOISE_DIM;
   mz = fmod((double)z, FNOISE_DIM); if (mz<0) mz += FNOISE_DIM;
   ix = (int)mx;
   iy = (int)my;
   iz = (int)mz;
   fx = fmod(mx, 1.0);
   fy = fmod(my, 1.0);
   fz = fmod(mz, 1.0);
   n = noiseTable[ix][iy][iz];
   n00 = n + fx*(noiseTable[ix+1][iy][iz]-n);
   n = noiseTable[ix][iy][iz+1];
   n01 = n + fx*(noiseTable[ix+1][iy][iz+1]-n);
   n = noiseTable[ix][iy+1][iz];
   n10 = n + fx*(noiseTable[ix+1][iy+1][iz]-n);
   n = noiseTable[ix][iy+1][iz+1];
   n11 = n + fx*(noiseTable[ix+1][iy+1][iz+1]-n);
   n0 = n00 + fy*(n10-n00);
   n1 = n01 + fy*(n11-n01);
   return (float)(((n0+fz*(n1-n0)))/32768.0);
#endif   
}


void Dent::LerpColor(RGBA *c, RGBA *a, RGBA *b, float f)
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

// Smooth step function with hermite interpolation
float Dent::SmoothStep(float x0, float x1, float v)
{
   if (v<=x0) return(0.0f);
   else if (v>=x1) return(1.0f);
   else {
      float u = (v-x0)/(x1-x0);
      return(u*u*(3-2*u));
   }
}

float Dent::DentFunc(Point3 p) 
{
	double s, mag;
	int i;
   
	s = 1.0;
	mag = 0.0;
	for (i=0; i<nits; i++)
	{
		mag += fabs(.5-DentNoise(p.x, p.y, p.z))/s;
		s *= 2.0;
		p.x*=2.0f;
		p.y*=2.0f;
		p.z*=2.0f;
	}
	return((float)(mag*mag*mag*km));
	
}

static AColor black(0.0f,0.0f,0.0f,0.0f);

RGBA Dent::EvalColor(ShadeContext& sc) 
{
	Point3	p,dp;
	RGBA	c;
	if (!sc.doMaps) return black;
	if (gbufID) sc.SetGBufferID(gbufID);
	
	xyzGen->GetXYZ(sc,p,dp);
	if (size==0.0f) size=.0001f;
	p *= DENTSIZE/size;
	float  d = DentFunc(p);
	
	if (d<=.0005) 
		return  mapOn[0]&&subTex[0] ? subTex[0]->EvalColor(sc): col[0];
	else 
		if (d>=.9995) 
			return  mapOn[1]&&subTex[1] ? subTex[1]->EvalColor(sc): col[1];
	RGBA c0 = mapOn[0]&&subTex[0] ? subTex[0]->EvalColor(sc): col[0];
	RGBA c1 = mapOn[1]&&subTex[1] ? subTex[1]->EvalColor(sc): col[1];
//	LerpColor(&c, &c0, &c1, d);
	return (1.0f-d)*c0 + d*c1;
}

Point3 Dent::EvalNormalPerturb(ShadeContext& sc) 
{
	float del,d;
	Point3 p,dp;
	
	if (!sc.doMaps) return Point3(0,0,0);

	if (gbufID) sc.SetGBufferID(gbufID);
	xyzGen->GetXYZ(sc,p,dp);
	if (size==0.0f) size=.0001f;
	p *= DENTSIZE/size;

	d = DentFunc(p);

	del = 0.1f;
	Point3 np;
	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);
    np.x = (DentFunc(p+del*M[0]) - d)/del;
	np.y = (DentFunc(p+del*M[1]) - d)/del;
	np.z = (DentFunc(p+del*M[2]) - d)/del;
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

//DentClassDesc
Class_ID DentClassDesc::ClassID() 
{ 
	return dentClassID; 
}
/*
// C Implementations
static BOOL CALLBACK  PanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	DentDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (DentDlg*)lParam;
		theDlg->hPanel = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (DentDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	theDlg->isActive = 1;
	int	res = theDlg->PanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
}
*/
ClassDesc* GetDentDesc()		{ return &dentCD;  }
#endif // NO_MAPTYPE_DENT
