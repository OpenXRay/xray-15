/**********************************************************************
 *<
	FILE: PLATE.CPP

	DESCRIPTION: Thin Wall Glass Refraction.

	CREATED BY: Dan Silva

	HISTORY:  12/4/98 Updated to Param Block 2 Peter Watje

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include <bmmlib.h>
#include "render.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "buildver.h"
#ifndef NO_MAPTYPE_THINWALL // orb 01-03-2001 Removing map types

//#define DBG        // define this to display rendered bitmap

extern HINSTANCE hInstance;

#define PLATE_CLASS_ID 0xd1f5a804

static Class_ID plateClassID(PLATE_CLASS_ID,0);

// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { plate_params, };  // pblock ID
// plate_params param IDs
enum 
{ 
	plate_blur, plate_thick, plate_reframt,
	plate_apply,
	plate_nthframe,
	plate_useenviroment,
	plate_frame

};

//---------------------------------------------------

class PlateMap {	
	public:
		Bitmap  *bm;
		IPoint2 org;
		int devW;
		int nodeID;
		TimeValue mapTime;
		PlateMap *next;
		PlateMap() { next = NULL; bm = NULL; nodeID = -1; mapTime = 0; }
		~PlateMap() { FreeMap(); }
		void FreeMap() { if (bm) bm->DeleteThis(); bm = NULL; } 
		
		int AllocMap(int w, int h);
	};

int PlateMap::AllocMap(int w, int h) {
	if ( bm && w==bm->Width() && h==bm->Height())
		return 1;
	BitmapInfo bi;
	if (bm) bm->DeleteThis();
	bi.SetName(_T(""));
	bi.SetWidth(w);
	bi.SetHeight(h);
	bi.SetType(BMM_TRUE_32);
	bi.SetCustomFlag(BMM_CUSTOM_GAMMA);
	bi.SetCustomGamma(1.0f);

	bm = TheManager->Create(&bi);

//	bm->CreateChannels(BMM_CHAN_Z); 
	return 1;
	}



//--------------------------------------------------------------
// Plate: 
//--------------------------------------------------------------

class Plate: public Texmap { 
	friend class PlatePostLoad;
	float blur;
	float thick,refrAmt;
    PlateMap *maps;
	Interval ivalid;
	int rollScroll;
	public:
		BOOL Param1;
		BOOL applyBlur;
		BOOL do_nth;
		BOOL useEnvMap;
		int nth;
		IParamBlock2 *pblock;   // ref #1
		Plate();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		// methods inherited from StdPlate:
		void SetDoNth(BOOL onoff) { do_nth = onoff;}
		void SetNth(int n){ nth = n;}
		void SetApplyBlur(BOOL onoff) { applyBlur = onoff; }
		void SetBlur(float b, TimeValue t);
		void SetThick(float v, TimeValue t);
		void SetRefrAmt(float v, TimeValue t);
		BOOL GetDoNth() { return do_nth; }
		int GetNth() { return nth;}
		BOOL GetApplyBlur() { return applyBlur;}
		float GetBlur(TimeValue t) { 
			return pblock->GetFloat(plate_blur,t); 
			}


		void NotifyChanged();
	
		// Evaluate the color of map for the context.
		RGBA EvalColor(ShadeContext& sc);

		// optimized evaluation for monochrome use
		float EvalMono(ShadeContext& sc);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		BOOL HandleOwnViewPerturb() { return FALSE; }

		ULONG LocalRequirements(int subMtlNum) {	
			return MTLREQ_AUTOREFLECT;
			}

		int BuildMaps(TimeValue t, RenderMapsContext &rmc);
		int DoThisFrame(TimeValue t, BOOL fieldRender, TimeValue mapTime);
		PlateMap *FindMap(int nodeNum);
		void FreeMaps();
		int RenderBegin(TimeValue t, ULONG flags) { return 1;}
		int RenderEnd(TimeValue t) { FreeMaps(); return 1; }

		Class_ID ClassID() {	return plateClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_FLATPLATE); }  
		void DeleteThis() { delete this; }	

		int NumSubs() {return 1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 1; }
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

		bool IsLocalOutputMeaningful( ShadeContext& sc );
	};

class PlateClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Plate; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_FLATPLATE_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return plateClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_ENV;  }
// PW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("thinWallRefraction"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static PlateClassDesc plateCD;

ClassDesc* GetPlateDesc() { return &plateCD;  }
//-----------------------------------------------------------------------------
//  Plate
//-----------------------------------------------------------------------------

static ParamBlockDesc2 plate_param_blk ( plate_params, _T("parameters"),  0, &plateCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_PLATE, IDS_DS_PLATE_PARAMS, 0, 0, NULL, 
	// params
	plate_blur,	_T("blur"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_BLUR,
		p_default,		1.0,
		p_range,		0.0, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_ACUBE_BLUR_EDIT, IDC_ACUBE_BLUR_SPIN, 0.01f,
		end,
	plate_thick,	_T("thicknessOffset"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_THICKFACT,
		p_default,		0.5,
		p_range,		0.0, 10.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_PLT_THK_EDIT, IDC_PLT_THK_SPIN, 0.01f,
		end,
	plate_reframt,	_T("bumpMapEffect"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_REFRAMT,
		p_default,		1.0,
		p_range,		0.0, 5.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_PLT_AMT_EDIT, IDC_PLT_AMT_SPIN, 0.01f,
		end,
	plate_apply,	_T("applyBlur"), TYPE_BOOL,			0,				IDS_PW_APPLYBLUR,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_ACUBE_BLUR,
		end,

	plate_nthframe,	_T("nthFrame"),   TYPE_INT,			0,	IDS_PW_NTHFRAME,
		p_default,		1,
		p_range,		1, 10000,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT,  IDC_ACUBE_NTH_EDIT, IDC_ACUBE_NTH_SPIN, 1.0f,
		end,

	plate_useenviroment,	_T("useEnviroment"), TYPE_BOOL,			0,				IDS_PW_USENVIROMENT,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USE_ENVMAP,
		end,
	plate_frame, _T("frame"), TYPE_INT,				0,				IDS_PW_FRAME,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_FIRST_ONLY, IDC_EVERY_NTH,
		end,

	end
);





#define NPARAMS 3
#define PLATE_VERSION 5

// Version 1 desc
static ParamBlockDescID pbdesc1[] = {
	{ TYPE_INT, NULL, TRUE,plate_blur }, 	// blur
	{ TYPE_FLOAT, NULL, TRUE,plate_thick } 	// blurOff
	};

// Version 2 desc
static ParamBlockDescID pbdesc2[] = {
	{ TYPE_FLOAT, NULL, TRUE,plate_blur } 	// blur
	};

// Version 3 desc
static ParamBlockDescID pbdesc3[] = {
	{ TYPE_FLOAT, NULL, TRUE,plate_blur }, 	// blur
	{ TYPE_FLOAT, NULL, TRUE,plate_thick }, 	// noise amount
	{ TYPE_FLOAT, NULL, TRUE,plate_thick }, 	// noise levels
	{ TYPE_FLOAT, NULL, TRUE,plate_thick }, 	// noise size
	{ TYPE_FLOAT, NULL, TRUE,plate_thick } 	// noise phase
	};

static ParamBlockDescID pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE,plate_blur }, 	// blur
	{ TYPE_FLOAT, NULL, TRUE,plate_thick }, 	// thickness
	{ TYPE_FLOAT, NULL, TRUE,plate_reframt }, 	// refraction amount
	};

static ParamVersionDesc versions[4] = {
	ParamVersionDesc(pbdesc1,2,1),
	ParamVersionDesc(pbdesc2,1,2),
	ParamVersionDesc(pbdesc3,5,3),
	ParamVersionDesc(pbdesc,3,4)
	};

//static ParamVersionDesc curVersion(pbdesc,NPARAMS,PLATE_VERSION);

void Plate::Init() {
	ivalid.SetEmpty();
	nth = 1;
	do_nth = TRUE;
	applyBlur = TRUE;
	useEnvMap = TRUE;
	SetBlur(1.0f, TimeValue(0));
	SetThick(0.5f, TimeValue(0));
	SetRefrAmt(1.0f, TimeValue(0));
	}

void Plate::Reset() {
	plateCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	}

void Plate::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Plate::Plate() {
	Param1 = FALSE;
	pblock = NULL;
	maps = NULL;
	plateCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}

RefTargetHandle Plate::Clone(RemapDir &remap) {
	Plate *mnew = new Plate();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->do_nth = do_nth;
	mnew->applyBlur = applyBlur;
	mnew->ivalid.SetEmpty();	
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* Plate::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	IAutoMParamDlg* masterDlg = plateCD.CreateParamDlgs(hwMtlEdit, imp, this);
	return masterDlg;
	}

void Plate::Update(TimeValue t, Interval& valid) {		
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		pblock->GetValue( plate_blur, t, blur, ivalid );
		pblock->GetValue( plate_reframt, t, refrAmt, ivalid);
		pblock->GetValue( plate_thick, t, thick, ivalid);


		pblock->GetValue( plate_apply, t, applyBlur, ivalid );

		pblock->GetValue( plate_nthframe, t, nth, ivalid);
		pblock->GetValue( plate_useenviroment, t, useEnvMap, ivalid);
		pblock->GetValue( plate_frame, t, do_nth, ivalid);


		}
	valid &= ivalid;
	}

void Plate::FreeMaps() {
	PlateMap *cm,*nxtcm;
	for (cm = maps; cm!=NULL; cm = nxtcm) {
		nxtcm = cm->next;
	   	delete cm;		
		}
	maps = NULL;
	}


void Plate::SetBlur(float f, TimeValue t) { 
	blur = f; 
	pblock->SetValue( plate_blur, t, f);
	}


void Plate::SetThick(float v, TimeValue t){
	thick = v; 
	pblock->SetValue( plate_thick, t, v);
	}

void Plate::SetRefrAmt(float v, TimeValue t) {
	refrAmt = v; 
	pblock->SetValue( plate_reframt, t, v);
	}


RefTargetHandle Plate::GetReference(int i) {
	switch(i) {
		case 0:	return pblock ;
		default: return NULL;
		}
	}

void Plate::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0:	pblock = (IParamBlock2 *)rtarg; break;
		}
	}
	 
Animatable* Plate::SubAnim(int i) {
	switch (i) {
		case 0: return pblock;
		default: return NULL;
		}
	}

TSTR Plate::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));
		default: return TSTR("");		
		}
	}

static int nameID[] = { IDS_DS_BLUR, IDS_DS_THICKFACT,IDS_DS_REFRAMT };

RefResult Plate::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
				plate_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
				}


			break;
		}
	return(REF_SUCCEED);
	}


static void FlipAxis(Matrix3& tm, int k) {
	MRow* m = tm.GetAddr();
	for (int i=0; i<4; i++) m[i][k] = -m[i][k];
	}

/* build reflection matrix for plane p */
static void BuildReflMatrix(Matrix3& rm, float *p) {
	MRow* m = rm.GetAddr();
	m[0][0] = 1.0f-2.0f*p[0]*p[0];		
	m[1][1] = 1.0f-2.0f*p[1]*p[1];		
	m[2][2] = 1.0f-2.0f*p[2]*p[2];		
	m[0][1] = m[1][0] = -2.0f*p[0]*p[1];		
	m[0][2] = m[2][0] = -2.0f*p[0]*p[2];		
	m[1][2] = m[2][1] = -2.0f*p[1]*p[2];		
	m[3][0] = -2.0f*p[0]*p[3];		
	m[3][1] = -2.0f*p[1]*p[3];		
	m[3][2] = -2.0f*p[2]*p[3];		
	rm.SetNotIdent();
	}


PlateMap *Plate::FindMap(int nodeNum) {
	PlateMap *cm;
	for (cm=maps; cm!=NULL; cm = cm->next)
		if (cm->nodeID==nodeNum) return cm;
	return NULL;
	}

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

int Plate::DoThisFrame(TimeValue t, BOOL fieldRender, TimeValue mapTime) {
	if (!do_nth) return 0;  // only do it once.
	if (nth==1) return 1;   // need every one
	TimeValue del = abs(t - mapTime);
	if (fieldRender) del*=2;
	return  (del>=nth*GetTicksPerFrame())?1:0;
	}


static void TransformPlane(Matrix3& tm, Point4 plin, Point4 plout) {
	Point3 n = VectorTransform(tm, Point3(plin[0],plin[1],plin[2]));
	plout[0] = n.x;
	plout[1] = n.y;
	plout[2] = n.z;
	plout[3] = plin[3] - DotProd(tm.GetRow(3),n); 
	}

int Plate::BuildMaps(TimeValue t, RenderMapsContext &rmc) {
	SubRendParams srp;
	rmc.GetSubRendParams(srp);
  	PlateMap *pmap = FindMap(rmc.NodeRenderID());
	if (pmap&&!DoThisFrame(t,srp.fieldRender,pmap->mapTime))
		return 1;
	ViewParams vp;

	Box2 sbox;

	rmc.GetCurrentViewParams(vp);

	rmc.FindMtlScreenBox(sbox, &vp.affineTM, rmc.SubMtlIndex());

	int xmin,xmax,ymin,ymax;

	BOOL fieldRender = FALSE;
	if (srp.fieldRender) {
		sbox.top *= 2;
		sbox.bottom *= 2;
		fieldRender = TRUE;
		}

		// add a margin around object for refraction
	int ew = srp.devWidth/20;
	int eh = srp.devHeight/20;
	xmax = sbox.right+ew;
	xmin = sbox.left-ew;
	ymax = sbox.bottom+eh;
	ymin = sbox.top-eh;

	if (srp.rendType==RENDTYPE_REGION) {
		ymin = MAX(ymin,srp.ymin-eh);
		ymax = MIN(ymax,srp.ymax+eh);
		xmin = MAX(xmin,srp.xmin-ew);
		xmax = MIN(xmax,srp.xmax+ew);
		}

	srp.xmin = MAX(xmin,-ew);
	srp.xmax = MIN(xmax,srp.devWidth+ew);
	srp.ymin = MAX(ymin,-eh);
	srp.ymax = MIN(ymax,srp.devHeight+eh);

	int xorg = xmin;
	int yorg = ymin;
	int devw = srp.devWidth;
	int devh = srp.devHeight;

	// The renderer is set up to allocate A-buffer for the range
	// 0..devWidth-1,  0..devHeight-1.  If the region of the plate
	// bitmap goes out of this region, it must be extended.
	int d1 = 0, d2 = 0;
	if (srp.xmin<0) d1 = -srp.xmin;
	if (srp.xmax>=srp.devWidth) d2 = srp.xmax-srp.devWidth+1;
	int d = MAX(d1,d2);
	if (d) {
		srp.devWidth += 2*d;
		srp.xmax += d;
		srp.xmin += d;
		// must also correct fov (or zoom) so the renderer computes the
		// same xscale and yscale
		if (vp.projType==PROJ_PERSPECTIVE) {
			vp.fov = 2.0f*(float)atan((float(srp.devWidth)/float(devw))*tan(0.5*vp.fov));
			}
		else {
			vp.zoom *= (float(srp.devWidth)/float(devw));
			}
		srp.blowupCenter.x += d; // DS: 11/6/00
		}	

	d1 = d2 = 0;
	if (srp.ymin<0) d1 = -srp.ymin;
	if (srp.ymax>=srp.devHeight) d2 = srp.ymax-srp.devHeight+1;
	d = MAX(d1,d2);
	if (d) {
//		if (fieldRender)
//			d = ((d+1)>>1)<<1; // make d even, so line doubling (below) works right.
		srp.devHeight += 2*d;
		srp.ymax += d;
		srp.ymin += d;
		srp.blowupCenter.y += d;   // DS: 11/6/00
		}	


	srp.xorg = srp.xmin;
	srp.yorg = srp.ymin;

	srp.doEnvMap = useEnvMap;
	//srp.doingMirror = TRUE;  // DS: 11/6/00: no longer necessary

	int w = srp.xmax-srp.xmin;
	int h = srp.ymax-srp.ymin;


	if (w<=0||h<=0) 
		return 1;
#ifdef DBG
	w = ((w+3)/4)*4;   // For some reason this needs to be a multiple of 4 for bm->Display
#endif

  	if (pmap==NULL) {
	  	pmap = new PlateMap;
		pmap->nodeID = rmc.NodeRenderID();
		pmap->next = maps;
		maps = pmap;
		}
	
	pmap->AllocMap(w, h);
	pmap->org.x = xorg-devw/2;
	pmap->org.y = yorg-devh/2;
	pmap->devW = devw;  // TBD: This should be accessable from the SC
	pmap->mapTime = t;

	
	// Set up clipping planes to clip out stuff between camera and plate.
	Box3 b = rmc.ObjectSpaceBoundingBox();
	Matrix3 tmObToWorld = rmc.ObjectToWorldTM();
	Matrix3 obToCam = tmObToWorld*vp.affineTM;	
	int nplanes=0;
	Point4 pl[6],pc;
	Point4 clip[6];

	pl[0] = Point4( 1.0f, 0.0f, 0.0f, -b.pmax.x);
	pl[1] = Point4(-1.0f, 0.0f, 0.0f,  b.pmin.x);
	pl[2] = Point4( 0.0f, 1.0f, 0.0f, -b.pmax.y);
	pl[3] = Point4( 0.0f,-1.0f, 0.0f,  b.pmin.y);
	pl[4] = Point4( 0.0f, 0.0f, 1.0f, -b.pmax.z);
	pl[5] = Point4( 0.0f, 0.0f,-1.0f,  b.pmin.z);
	if (vp.projType==PROJ_PARALLEL) {
		for (int i=0; i<6; i++) {
			pc = TransformPlane(obToCam,pl[i]);	
			// see if camera = (0,0,0) is in front of plane
			if (pc.z>0.0f)  
				clip[nplanes++] = -pc;
			}
		}
	else {
		for (int i=0; i<6; i++) {
			pc = TransformPlane(obToCam,pl[i]);	
			// see if camera = (0,0,0) is in front of plane
			if (pc.w>0.0f)  
				clip[nplanes++] = -pc;
			}
		}

	srp.fieldRender = FALSE;

//	assert(nplanes<4);
	if (!rmc.Render(pmap->bm, vp, srp, clip, nplanes))
		return 0;

/*
	if (srp.fieldRender) {
		// Double the lines, otherwise the blur wont work right,
		// because the blank lines in between get averaged in and darken it.
		int	evenLines = srp.evenLines; 
		if(srp.ymin&1) 
			evenLines = !evenLines;

		PixelBuf l64(w);
		if (evenLines) {
			for (int i=0; i<h; i+=2) {
				BMM_Color_64 *p64=l64.Ptr();
				if (i+1<h) {
					pmap->bm->GetPixels(0,i,  w, p64); 
					pmap->bm->PutPixels(0,i+1,w, p64);				
					}
				}
			}
		else {
			for (int i=0; i<h; i+=2) {
				BMM_Color_64 *p64=l64.Ptr();
				if (i+1<h) {
					pmap->bm->GetPixels(0,i+1,w, p64); 
					pmap->bm->PutPixels(0,i  ,w, p64);				
					}
				}
			}
		}
*/

#ifdef DBG
	if (devw>200){
		pmap->bm->UnDisplay();
		TSTR buf;
		RenderGlobalContext *gc = rmc.GetGlobalContext();
		RenderInstance* inst = gc->GetRenderInstance(rmc.NodeRenderID());
		INode *node  = inst->GetINode();
		buf.printf(_T("Thinwall: %s"), node->GetName());
		pmap->bm->Display(buf, BMM_UR);
		MessageBox(NULL, _T("hi"), _T(" Plate Test"), MB_OK|MB_ICONEXCLAMATION);
		}
#endif

	if (applyBlur) {
		// I tried pyramids here, but SATs looked much better. 
		//  maybe we should give users a choice?
		pmap->bm->SetFilter(BMM_FILTER_SUM); 
//		pmap->bm->SetFilter(BMM_FILTER_PYRAMID); 
		BitmapFilter *filt = pmap->bm->Filter();
		if (filt)
			filt->MakeDirty();  // so filter gets recomputed for each frame
		}
	else 
		pmap->bm->SetFilter(BMM_FILTER_NONE); 
	return 1;
	}

inline float FMax(float a, float b) { return (a>b?a:b); }

static BMM_Color_64 black64 = {0,0,0,0};
static AColor black(0.0f,0.0f,0.0f,0.0f);
static RGBA blackrgba(0.0f,0.0f,0.0f,1.0f);


static Point3 RefractVector(ShadeContext &sc, Point3 N, Point3 V, float ior) { 
	float VN,nur,k1;
	VN = DotProd(-V,N);
	if (sc.backFace) nur = ior;
	else nur = (ior!=0.0f) ? 1.0f/ior: 1.0f;
	k1 = 1.0f-nur*nur*(1.0f-VN*VN);
	if (k1<=0.0f) {
		// Total internal reflection: 
		return FNormalize(2.0f*VN*N + V);
		}
	else 
		return (nur*VN-(float)sqrt(k1))*N + nur*V;
	}

//float GetZ(Bitmap *bm, int x, int y, int w, int h) {
//	ULONG ctype;
//	float *zb = (float*)bm->GetChannel(BMM_CHAN_Z, ctype); 
//	return zb[w*y+x];
//	}

bool Plate::IsLocalOutputMeaningful( ShadeContext& sc ) 
{ 
	PlateMap *pmap = FindMap( sc.NodeID() );
	if ( pmap != NULL && sc.globContext == NULL )
			return false;
	return true; 
}

static void whoa() {}

RGBA Plate::EvalColor(ShadeContext& sc) {
	BMM_Color_64 c;
	IPoint2 s;
	int id = sc.NodeID();
	PlateMap *pmap = FindMap(id);
	if (gbufID) sc.SetGBufferID(gbufID);
	if (pmap) {
		s = sc.ScreenCoord();
		int w = pmap->bm->Width(); 
		int h = pmap->bm->Height();

		Point3 view = sc.OrigView();
		Point3 v2 = sc.V();
		Point3 p = sc.P();
		Point3 dV,dvf;
		Point3 N0 = sc.OrigNormal();

		Point3 vf = RefractVector(sc, N0, view, sc.GetIOR()); 
		
		RenderGlobalContext *gc = sc.globContext;
		if (gc==NULL) return blackrgba;

		// total deflection due to refraction
		dV = view-v2;

		// deflection due to flat refracton (no bumps)
		dvf = view-vf;

		dV = refrAmt*(dV-dvf) + thick*dvf;

		// compute screen deflection: This is really a cheat, and the
		// scale factor is arbitrary. Infact it depends on the distance 
		// between to the point on the glass plate and  to the point being
		// seen behind it, which we don't know.
		// these should be multiplied by the factor (Zbehind-Zcur)/Zcur
		// This assumes that the factor is .1

		float dsx,dsy;
		if (gc->projType==0) {
			// perspective
			dsx = dV.x*0.1f*gc->xscale;
			dsy = dV.y*0.1f*gc->yscale;
			}
		else {
			// parallel projection
			dsx = -dV.x*gc->xscale*10.0f;
			dsy = -dV.y*gc->yscale*10.0f;
			}

		if (gc->fieldRender) dsy *= 2.0f;		
		int x = s.x - (pmap->org.x+gc->devWidth/2);
		int y = s.y - (pmap->org.y+gc->devHeight/2);

		if (applyBlur) {
			float du = 1.0f/float(w);
			float dv = 1.0f/float(h);

			float u = (float(x)+dsx)*du; 
			float v = (float(y)+dsy)*dv; 
			if (u<0.0f||u>1.0f||v<0.0f||v>1.0f) {
				if (useEnvMap) {
					return sc.EvalGlobalEnvironMap(view-dvf);
					}
				else 
					return blackrgba;
				}
			else 
				pmap->bm->GetFiltered(u,v, du*blur, dv*blur,&c);
			}
		else {
			int ix = x + int(dsx); 
			int iy = y + int(dsy); 
			if (ix<0||ix>=w||iy<0||iy>=h) {
				if (useEnvMap)
					return sc.EvalGlobalEnvironMap(view-dvf);
				else 
					return blackrgba;
				}
			else 
				pmap->bm->GetLinearPixels(ix,iy,1,&c);
			}
		return c;
		}
	else 
		return blackrgba;
	}

float Plate::EvalMono(ShadeContext& sc) {
	return Intens(EvalColor(sc));
	}

Point3 Plate::EvalNormalPerturb(ShadeContext& sc) {
	return Point3(0,0,0);
	}

#define MTL_HDR_CHUNK 0x4000
#define DONT_DO_NTH_CHUNK 0x1000
#define NTH_CHUNK 0x1001
#define DONT_APPLY_BLUR_CHUNK 0x1002
#define DONT_USE_ENV_CHUNK 0x1003
#define DO_NOISE_CHUNK 0x1004
#define NOISE_TYPE_CHUNK 0x1005
#define PARAM2_CHUNK 0x1006

IOResult Plate::Save(ISave *isave) { 
	IOResult res;
//	ULONG nb;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();

	return IO_OK;
	}
		
class PlatePostLoad : public PostLoadCallback {
	public:
		Plate *n;
		BOOL Param1;
		PlatePostLoad(Plate *ns, BOOL b) {n = ns; Param1 = b;}
		void proc(ILoad *iload) {  
			if (Param1)
				{
				n->pblock->SetValue( plate_apply, 0, n->applyBlur);
				n->pblock->SetValue( plate_nthframe, 0, n->nth);
				n->pblock->SetValue( plate_useenviroment, 0, n->useEnvMap);
				n->pblock->SetValue( plate_frame, 0, n->do_nth);


				}

			delete this; 


			} 
	};

IOResult Plate::Load(ILoad *iload) { 
	ULONG nb;
	IOResult res;
	Param1 = TRUE;
//	iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,3, &curVersion, this,0));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case DONT_DO_NTH_CHUNK:
				do_nth = FALSE;
				break;
			case DONT_APPLY_BLUR_CHUNK:
				applyBlur = FALSE;
				break;
			case DONT_USE_ENV_CHUNK:
				useEnvMap = FALSE;
				break;
			case NTH_CHUNK:
				iload->Read(&nth,sizeof(nth),&nb);			
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 4, &plate_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);

	iload->RegisterPostLoadCallback(new PlatePostLoad(this,Param1));

	return IO_OK;
	}
#endif // NO_MAPTYPE_THINWALL