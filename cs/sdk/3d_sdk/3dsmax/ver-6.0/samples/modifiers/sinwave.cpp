/**********************************************************************
 *<
	FILE: sinwave.cpp

	DESCRIPTION:  Simple WSM

	CREATED BY: Rolf Berteig

	HISTORY: created 9 February, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"

#ifndef NO_MODIFIER_RIPPLE // JP Morel - June 28th 2002

#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"

// in mods.cpp
extern HINSTANCE hInstance;


#define DEF_FLEX		float(1.0)


// This is the function we're usin'
float WaveFunc( float radius, TimeValue t, float amp, 
				float waveLen, float phase, float decay )
	{
	if (waveLen == float(0)) {
		waveLen = float(0.0000001);
		}
	return float( amp * sin( TWOPI * ( radius/waveLen + phase ) ) 
		* exp(-decay * (float)fabs(radius) ) );
	}

class WaveObject : public SimpleWSMObject {	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
					
		WaveObject();		

		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
				
		// from object		
		int DoOwnSelectHilite() {return TRUE;}
		CreateMouseCallBack* GetCreateMouseCallBack();
		//int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);		

		// From SimpleObject		
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		// WaveObject stuff		
		virtual int DialogID()=0;
		virtual int Circles()=0;
	};

class SinWaveObject : public WaveObject {	
	public:				
		SinWaveObject();

		TCHAR *GetObjectName() {return GetString(IDS_RB_RIPPLE);}		
		Class_ID ClassID() {return Class_ID(SINEWAVE_OBJECT_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);

		// From WSMObject
		Modifier *CreateWSMMod(INode *node);
		
		// From SimpleObject
		void MakeCircle(TimeValue t, Mesh &mesh, int startVert, int& face, float radius,float a1,float a2,float w,float s, float d, int numCircleSegs);
		void BuildMesh(TimeValue t);

		// WaveObject stuff
		int DialogID() {return IDD_SINWAVEPARAM1;}
		int Circles() {return 10;}
	};

class LinWaveObject : public WaveObject {	
	public:		
		LinWaveObject();

		TCHAR *GetObjectName() { return GetString(IDS_RB_WAVE); }
		Class_ID ClassID() { return Class_ID(LINWAVE_OBJECT_CLASS_ID,0);}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);
		
		// From SimpleObject		
		void BuildMesh(TimeValue t);

		// WaveObject stuff
		int DialogID() {return IDD_LINWAVEPARAM1;}
		int Circles() {return 4;}
	};

//--- ClassDescriptor and class vars ---------------------------------


IObjParam *SinWaveObject::ip        = NULL;
IParamMap *SinWaveObject::pmapParam = NULL;
HWND       SinWaveObject::hSot      = NULL;

class SinWaveClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SinWaveObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_RIPPLE_CLASS); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SINEWAVE_OBJECT_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetSpaceWarpCatString(SPACEWARP_CAT_GEOMDEF);}
	};

static SinWaveClassDesc swDesc;
ClassDesc* GetSinWaveObjDesc() { return &swDesc; }

class LinWaveClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new LinWaveObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_WAVE_CLASS); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(LINWAVE_OBJECT_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetSpaceWarpCatString(SPACEWARP_CAT_GEOMDEF);}
	};

static LinWaveClassDesc lwDesc;
ClassDesc* GetLinWaveObjDesc() { return &lwDesc; }


//--- SineWaveMod -----------------------------------------------------

class WaveMod : public SimpleWSMMod {
	public:		
		static IParamMap *pmapParam;

		WaveMod() {}
		WaveMod(INode *node,WaveObject *obj);		

		// From Animatable
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_WAVEMOD); }
		SClass_ID SuperClassID() { return WSM_CLASS_ID; }		
		void DeleteThis() {delete this;}
		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void InvalidateUI() {if (pmapParam) pmapParam->Invalidate();}
	};

class SinWaveMod : public WaveMod {
	public:		
		
		SinWaveMod() {}
		SinWaveMod(INode *node,SinWaveObject *obj) : WaveMod(node,obj) {}

		// From Animatable		
		SClass_ID SuperClassID() { return WSM_CLASS_ID; }
		Class_ID ClassID() { return Class_ID(SINEWAVE_CLASS_ID,0); } 		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_RIPPLE_BINDING);}

		// From SimpleWSMMod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
	};

class LinWaveMod : public WaveMod {
	public:		
		
		LinWaveMod() {}
		LinWaveMod(INode *node,LinWaveObject *obj) : WaveMod(node,obj) {}

		// From Animatable		
		SClass_ID SuperClassID() { return WSM_CLASS_ID; }
		Class_ID ClassID() { return Class_ID(LINWAVE_CLASS_ID,0); } 		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_WAVEBINDING);}

		// From SimpleWSMMod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
	};

//--- ClassDescriptor and class vars ---------------------------------

class SinWaveModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new SinWaveMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_RIPPLE_CLASS); }
	SClass_ID		SuperClassID() { return WSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SINEWAVE_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");}
	};

static SinWaveModClassDesc swModDesc;
ClassDesc* GetSinWaveModDesc() { return &swModDesc; }

class LinWaveModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new LinWaveMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_WAVE_CLASS); }
	SClass_ID		SuperClassID() { return WSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(LINWAVE_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");}
	};

static LinWaveModClassDesc lwModDesc;
ClassDesc* GetLinWaveModDesc() { return &lwModDesc; }

IParamMap *WaveMod::pmapParam = NULL;


//--- Object Space Modifier versions -----------------------------------------

class WaveOMod : public SimpleMod {
	public:		
		static IParamMap *pmapParam;

		WaveOMod();

		// From Animatable
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_WAVEMOD); }		
		void DeleteThis() {delete this;}
		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);		
		
		// From SimpleMod		
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void InvalidateUI() {if (pmapParam) pmapParam->Invalidate();}

		virtual int DialogID()=0;
	};

class SinWaveOMod : public WaveOMod {
	public:
		// From Animatable		
		Class_ID ClassID() { return Class_ID(SINEWAVE_OMOD_CLASS_ID,0); } 		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_RIPPLE);}

		// From SimpleMod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
		int DialogID() {return IDD_SINWAVEOMODPARAM;}
	};


class LinWaveOMod : public WaveOMod {
	public:		
		// From Animatable		
		Class_ID ClassID() { return Class_ID(LINWAVE_OMOD_CLASS_ID,0); } 
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_WAVE);}

		// From SimpleMod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
		int DialogID() {return IDD_LINWAVEOMODPARAM;}
	};

//--- ClassDescriptor and class vars ---------------------------------

IParamMap *WaveOMod::pmapParam = NULL;


class SinWaveOModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SinWaveOMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_RIPPLE_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SINEWAVE_OMOD_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static SinWaveOModClassDesc swOModDesc;
ClassDesc* GetSinWaveOModDesc() { return &swOModDesc; }

class LinWaveOModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new LinWaveOMod;}
	const TCHAR *	ClassName() { return GetString(IDS_RB_WAVE_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(LINWAVE_OMOD_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static LinWaveOModClassDesc lwOModDesc;
ClassDesc* GetLinWaveOModDesc() { return &lwOModDesc; }



//--- SineWaveObject Parameter map/block descriptors ------------------

#define PB_AMPLITUDE	0
#define PB_AMPLITUDE2	1
#define PB_WAVELEN		2
#define PB_PHASE		3
#define PB_DECAY		4

#define PB_CIRCLES		5
#define PB_SEGMENTS		6
#define PB_DIVISIONS	7

//
//
// Parameters

static ParamUIDesc descParamObj[] = {
	
	// Amplitude 1
	ParamUIDesc(
		PB_AMPLITUDE,
		EDITTYPE_UNIVERSE,
		IDC_AMPLITUDE,IDC_AMPSPINNER,
		-9999999.0f, 9999999.0f,
		SPIN_AUTOSCALE),	

	// Amplitude 2
	ParamUIDesc(
		PB_AMPLITUDE2,
		EDITTYPE_UNIVERSE,
		IDC_AMPLITUDE2,IDC_AMPSPINNER2,
		-9999999.0f, 9999999.0f,
		SPIN_AUTOSCALE),	

	// Wave Length
	ParamUIDesc(
		PB_WAVELEN,
		EDITTYPE_UNIVERSE,
		IDC_WAVELEN,IDC_WAVELENSPINNER,
		-9999999.0f, 9999999.0f,
		SPIN_AUTOSCALE),	

	// Phase
	ParamUIDesc(
		PB_PHASE,
		EDITTYPE_FLOAT,
		IDC_PHASE,IDC_PHASESPINNER,
		-9999999.0f, 9999999.0f,
		0.1f),
		
	// Decay
	ParamUIDesc(
		PB_DECAY,
		EDITTYPE_FLOAT,
		IDC_DECAY,IDC_DECAYSPINNER,
		0.0f, 9999999.0f,
		0.001f),
		
	// Circles
	ParamUIDesc(
		PB_CIRCLES,
		EDITTYPE_INT,
		IDC_CIRCLES,IDC_CIRCLESSPINNER,
		3.0f, 200.0f,
		0.1f),

	// Segments
	ParamUIDesc(
		PB_SEGMENTS,
		EDITTYPE_INT,
		IDC_SEGS,IDC_SEGSPINNER,
		3.0f, 200.0f,
		0.1f),
	
	// Divisions
	ParamUIDesc(
		PB_DIVISIONS,
		EDITTYPE_INT,
		IDC_DIVISIONS,IDC_DIVSPINNER,
		1.0f, 200.0f,
		0.1f)
	};
#define OBJPARAMDESC_LENGH 8


ParamBlockDescID descObjVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 } };

ParamBlockDescID descObjVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 } };

#define OBJPBLOCK_LENGTH	8


// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descObjVer0,7,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_OBJVERSION	1
static ParamVersionDesc curVersion(descObjVer1,OBJPBLOCK_LENGTH,CURRENT_OBJVERSION);



//--- SineWaveObject methods ------------------------------------------


WaveObject::WaveObject()
	{
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descObjVer1, OBJPBLOCK_LENGTH, CURRENT_OBJVERSION));
	assert(pblock);	
	}

void WaveObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapParam) {
		
		// Left over from last SinWave ceated
		pmapParam->SetParamBlock(pblock);
	} else {
		hSot = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_SINWAVE_SOT),
				DefaultSOTProc,
				GetString(IDS_RB_SOT), 
				(LPARAM)ip,APPENDROLL_CLOSED);

		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			descParamObj,OBJPARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(DialogID()),
			GetString(IDS_RB_PARAMETERS),
			0);		
		}
	}

void WaveObject::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
	{		
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {				
		DestroyCPParamMap(pmapParam);
		ip->DeleteRollupPage(hSot);
		pmapParam = NULL;		
		}	
	}



#if 0
class SinWaveMtl: public Material {
	public:
	SinWaveMtl();
	};
static SinWaveMtl swMtl;

#define SINEWAVE_R	float(0)
#define SINEWAVE_G	float(.5)
#define SINEWAVE_B	float(.5)

SinWaveMtl::SinWaveMtl():Material() {
	Kd[0] = SINEWAVE_R;
	Kd[1] = SINEWAVE_G;
	Kd[2] = SINEWAVE_B;
	Ks[0] = SINEWAVE_R;
	Ks[1] = SINEWAVE_G;
	Ks[2] = SINEWAVE_B;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME|GW_BACKCULL;
	selfIllum = (float)1.0;
	}
 
int WaveObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
	{
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = &swMtl;	
 	DWORD rlim = gw->getRndLimits();
	
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|/*GW_BACKCULL|*/ (rlim&GW_Z_BUFFER?GW_Z_BUFFER:0) );	//removed BC 2/16/99 DB
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		gw->setColor( LINE_COLOR, swMtl.Kd[0], swMtl.Kd[1], swMtl.Kd[2]);
	
	SimpleWSMObject::Display(t,inode,vpt,flags);
			
	gw->setRndLimits(rlim);
	return(0);
	}
#endif

class SinWaveObjCreateCallBack: public CreateMouseCallBack {	
	WaveObject *ob;	
	Point3 p0, p1;
	IPoint2 sp0, sp1;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		void SetObj(WaveObject *obj) { ob = obj; }
	};

int SinWaveObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float w;	

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:  // only happens with MOUSE_POINT msg
				sp0    = m;
				p0     = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE); //vpt->GetPointOnCP(m);
				mat.SetTrans(p0);
				break;
			case 1:								
				sp1 = m;		
				p1  = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE); //vpt->GetPointOnCP(m);
				w   = float(4)*Length(p0-p1)/ob->Circles();
				ob->pblock->SetValue(PB_WAVELEN,0,w);
				ob->pmapParam->Invalidate();
				break;
			case 2:	
				if ( Length(sp1-sp0)<3 ) return CREATE_ABORT;			
				w = vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m);				
				ob->pblock->SetValue(PB_AMPLITUDE,0,w);
				ob->pblock->SetValue(PB_AMPLITUDE2,0,w);
				ob->pmapParam->Invalidate();				
				if (msg==MOUSE_POINT) 
					return CREATE_STOP;
				break;					   
			}
		}
	else
	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;
	else
	if (msg == MOUSE_FREEMOVE) {
		vpt->SnapPreview(m,m);
		}
	return TRUE;
	}

static SinWaveObjCreateCallBack sinwaveCreateCB;


CreateMouseCallBack* WaveObject::GetCreateMouseCallBack()
	{
	sinwaveCreateCB.SetObj(this);
	return &sinwaveCreateCB;
	}

void WaveObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *WaveObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_AMPLITUDE:	return stdWorldDim;		
		case PB_AMPLITUDE2:	return stdWorldDim;	
		case PB_WAVELEN: 	return stdWorldDim;		
		case PB_PHASE:		return stdNormalizedDim;		
		case PB_DECAY:		return stdNormalizedDim;			
		default:			return defaultDim;
		}
	}

TSTR WaveObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_AMPLITUDE:	return GetString(IDS_RB_AMPLITUDE1);
		case PB_AMPLITUDE2:	return GetString(IDS_RB_AMPLITUDE2);
		case PB_WAVELEN:	return GetString(IDS_RB_WAVELEN);
		case PB_PHASE:		return GetString(IDS_RB_PHASE);
		case PB_DECAY:		return GetString(IDS_RB_DECAY);
		default:			return TSTR(_T(""));
		}
	}



//--- Circular sine wave ---------------------------------------------

SinWaveObject::SinWaveObject()
	{
	pblock->SetValue(PB_CIRCLES, TimeValue(0), 10);
	pblock->SetValue(PB_SEGMENTS, TimeValue(0), 16);
	pblock->SetValue(PB_DIVISIONS, TimeValue(0), 4);
	}

IOResult SinWaveObject::Load(ILoad *iload)
	{
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}

Modifier *SinWaveObject::CreateWSMMod(INode *node)
	{
	return new SinWaveMod(node,this);
	}

RefTargetHandle SinWaveObject::Clone(RemapDir& remap) 
	{
	SinWaveObject* newob = new SinWaveObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newob, remap);
	return(newob);
	}

static void MakeQuad(Face *f, int a,  int b , int c , int d, int sg, int dv = 0) {
	f[0].setVerts( a+dv, b+dv, c+dv);
	f[0].setSmGroup(sg);
	f[0].setEdgeVisFlags(1,1,0);
	f[1].setVerts( c+dv, d+dv, a+dv);
	f[1].setSmGroup(sg);
	f[1].setEdgeVisFlags(1,1,0);
	}

void SinWaveObject::MakeCircle(TimeValue t, Mesh &mesh, 
		int startVert, int& face, float radius,
		float a1,float a2,float w,float s, float d, int numCircleSegs)
	{
	float x, y, z, u, u2;			

	for ( int i = startVert; i < startVert+numCircleSegs; i++ ) {
		u = float(i-startVert) / float(numCircleSegs);
		x = radius * (float)cos( u * TWOPI );
		y = radius * (float)sin( u * TWOPI );		
		
		u2 = (u > 0.5f) ? (u-0.5f) : u;
		u2 = (u2 > 0.25f) ? (0.5f-u2) : u2;
		u2 = u2 * 4.0f;
		u2 = u2*u2;
		z = WaveFunc(radius, t, a1*(1.0f-u2) + a2*u2, w, s, d);

		if ( startVert == 0 ) {
			mesh.setVert( i+1, Point3(x, y, z) );
		} else {
			mesh.setVert( numCircleSegs + i, Point3(x, y, z) );
		
			if ( i < startVert+numCircleSegs-1 ) {
				MakeQuad(&(mesh.faces[face]), 
					i+numCircleSegs,i+numCircleSegs+1,i+1,i,1);
				face += 2;
			} else {
				MakeQuad(&(mesh.faces[face]), 
					i+numCircleSegs,startVert+numCircleSegs,startVert,i,1);
				face += 2;
				}
			}
		}
	}

void SinWaveObject::BuildMesh(TimeValue t)
	{		
	int startVert = 1, face = 0, nverts, nfaces, numCircles, numCircleSegs, divs;
	float radius = float(0);
	float dr;
	float a, a2, w, s, d;	

	ivalid = FOREVER;
	pblock->GetValue(PB_AMPLITUDE,t,a,ivalid);
	pblock->GetValue(PB_AMPLITUDE2,t,a2,ivalid);
	pblock->GetValue(PB_WAVELEN,t,w,ivalid);
	pblock->GetValue(PB_PHASE,t,s,ivalid);
	pblock->GetValue(PB_DECAY,t,d,ivalid);
	pblock->GetValue(PB_SEGMENTS,t,numCircleSegs,ivalid);
	pblock->GetValue(PB_CIRCLES,t,numCircles,ivalid);
	pblock->GetValue(PB_DIVISIONS,t,divs,ivalid);
	LimitValue(d,0.0f,float(1.0E30));

	dr     = w/float(divs);
	nverts = numCircles * numCircleSegs + 1;
	nfaces = (numCircles-1) * numCircleSegs * 2;
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);

	mesh.setVert( 0, Point3(0,0,0) );
	MakeCircle(t,mesh,0,face,radius, a, a2, w, s, d, numCircleSegs);

	for ( int i = 1; i < numCircles; i++ ) {
		MakeCircle(t,mesh,startVert,face,radius, a, a2, w, s, d, numCircleSegs);
		startVert += numCircleSegs;
		radius += dr;
		}
	
	mesh.InvalidateGeomCache();
	}


//--- Linear sine wave ---------------------------------------------

LinWaveObject::LinWaveObject()
	{
	pblock->SetValue(PB_CIRCLES, TimeValue(0), 4);
	pblock->SetValue(PB_SEGMENTS, TimeValue(0), 20);
	pblock->SetValue(PB_DIVISIONS, TimeValue(0), 10);
	}

Modifier *LinWaveObject::CreateWSMMod(INode *node)
	{
	return new LinWaveMod(node,this);
	}

RefTargetHandle LinWaveObject::Clone(RemapDir& remap) 
	{
	LinWaveObject* newob = new LinWaveObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newob, remap);
	return(newob);
	}


void LinWaveObject::BuildMesh(TimeValue t)
	{		
	int startVert = 1, face = 0, nverts, nfaces, numSides, numSegs, divs;
	float radius = float(0);
	float dx, dy, starty, startx;
	float a, a2, w, s, d, x, y, z, u;	
	int nv=0, nf=0, ix, den;

	ivalid = FOREVER;
	pblock->GetValue(PB_AMPLITUDE,t,a,ivalid);
	pblock->GetValue(PB_AMPLITUDE2,t,a2,ivalid);
	pblock->GetValue(PB_WAVELEN,t,w,ivalid);
	pblock->GetValue(PB_PHASE,t,s,ivalid);
	pblock->GetValue(PB_DECAY,t,d,ivalid);
	pblock->GetValue(PB_SEGMENTS,t,numSegs,ivalid);
	pblock->GetValue(PB_CIRCLES,t,numSides,ivalid);
	pblock->GetValue(PB_DIVISIONS,t,divs,ivalid);	
	LimitValue(d,0.0f,float(1.0E30));
	
	dy     = w/float(divs);
	dx     = dy * 4;
	starty = -float(numSegs)/2.0f * dy;
	startx = -float(numSides)/2.0f * dx;
	nverts = (numSides+1) * (numSegs+1);
	nfaces = (numSides) * numSegs * 2;
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);

	
	for (int i=0; i<=numSides; i++) {
		x   = startx + dx * float(i);		
		den = (int)(dx*numSides*0.5f);
		u   = (float)fabs(x/(den?den:0.00001f));		
		u   = u*u;
		//u = smoothstep(0.0f,1.0f,u);
		for (int j=0; j<=numSegs; j++) {
			y = starty + float(j) * dy;
			z = WaveFunc(y, t, a*(1.0f-u)+a2*u, w, s, d);
			mesh.setVert(nv++,Point3(x, y, z));			
			}
		}
		
	for (i=0; i<numSides; i++) {
		ix = i * (numSegs+1);
		for (int j=0; j<numSegs; j++) {
			MakeQuad(&(mesh.faces[nf]), 
				ix+numSegs+1+j, ix+numSegs+2+j, ix+1+j, ix+j, 1);				
			nf += 2;
			}
		}
	
	assert(nv==mesh.numVerts);
	assert(nf==mesh.numFaces);
	mesh.InvalidateGeomCache();
	}



/*----------------------------------------------------------------*/
// Sine Wave modifier


//--- Parameter map/block descriptors -----------------------------

#define PB_FLEX			0

//
//
// Parameters

static ParamUIDesc descParamMod[] = {
	// Angle
	ParamUIDesc(
		PB_FLEX,
		EDITTYPE_FLOAT,
		IDC_FLEX,IDC_FLEXSPINNER,
		-10.0f,10.0f,
		0.01f)
	};
#define MODPARAMDESC_LENGH 1

ParamBlockDescID descModVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 } };

#define MODPBLOCK_LENGTH	1

#if 0
// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);
#endif

#define CURRENT_MODVERSION	0


//--- SinWaveDeformer  --------------------------------------

class SinWaveDeformer : public Deformer {
	public:
		float amp, amp2, wave, phase, decay, flex;
		TimeValue time;
		Matrix3 tm, itm;		
		Point3 Map(int i, Point3 p); 
	};

Point3 SinWaveDeformer::Map(int i, Point3 p)
	{
	Point3 pt = p * tm;
	float r, oldZ, u, a, len;
		
	if (amp!=amp2) {
		len  = Length(pt);
		if (len==0.0f) {
			a = amp;
		} else {
			u = (float)acos(pt.x/len)/PI;
	 		u = (u > 0.5) ? (1.0f-u) : u;
			u *= 2.0f;
	 		//u = u*u;
	 		u = smoothstep(0.0f,1.0f,u);
	 		a = amp*(1.0f-u) + amp2*u;
			}
	} else {
		a = amp;
		}	
	
	oldZ = pt.z;
	pt.z = float(0);	
	r    = Length(pt);
	pt.z = oldZ + flex * WaveFunc(r, time, a, wave, phase, decay);
	return pt * itm;
	}

class LinWaveDeformer : public Deformer {
	public:
		float amp, amp2, wave, phase, decay, flex, dist;
		TimeValue time;
		Matrix3 tm, itm;		
		Point3 Map(int i, Point3 p); 
	};

Point3 LinWaveDeformer::Map(int i, Point3 p)
	{
	if (dist == 0.0f) return p;
	Point3 pt = p * tm;	
	float u = (float)fabs(2.0f*pt.x/dist);
	u = u*u;
	pt.z += flex * WaveFunc(pt.y, time, amp*(1.0f-u)+amp2*u, wave, phase, decay);
	return pt * itm;
	}

//--- WaveMod methods -----------------------------------------


WaveMod::WaveMod(INode *node,WaveObject *obj)
	{	
	//MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	MakeRefByID(FOREVER, SIMPWSMMOD_PBLOCKREF, 
		CreateParameterBlock(descModVer0, MODPBLOCK_LENGTH, CURRENT_MODVERSION));
	obRef = NULL;
	pblock->SetValue(PB_FLEX, TimeValue(0), DEF_FLEX);
	}

void WaveMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleWSMMod::BeginEditParams(ip,flags,prev);
		
	pmapParam = CreateCPParamMap(
		descParamMod,MODPARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_SINWAVEMODPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);		
	}
		
void WaveMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	SimpleWSMMod::EndEditParams(ip,flags,next);
	DestroyCPParamMap(pmapParam);
	}

Interval WaveMod::GetValidity(TimeValue t) 
	{
	if (nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;
		pblock->GetValue(PB_FLEX,t,f,valid);
		SinWaveObject *obj = (SinWaveObject*)GetWSMObject(t);
		obj->pblock->GetValue(PB_AMPLITUDE,t,f,valid);
		obj->pblock->GetValue(PB_AMPLITUDE2,t,f,valid);
		obj->pblock->GetValue(PB_WAVELEN,t,f,valid);
		obj->pblock->GetValue(PB_PHASE,t,f,valid);
		obj->pblock->GetValue(PB_DECAY,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	} else {
		return FOREVER;
		}
	}

ParamDimension *WaveMod::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_FLEX: 	return stdNormalizedDim;
		default:		return defaultDim;
		}	
	}

TSTR WaveMod::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_FLEX: 	return GetString(IDS_RB_FLEXIBILITY);
		default:		return GetString(IDS_RB_PARAMETERS);
		}
	}


// --- SinWaveMod -------

Deformer& SinWaveMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	Interval valid;
	static SinWaveDeformer sd;
	pblock->GetValue(PB_FLEX,t,sd.flex,FOREVER);
	SinWaveObject *obj = (SinWaveObject*)GetWSMObject(t);
	obj->pblock->GetValue(PB_AMPLITUDE,t,sd.amp,FOREVER);
	obj->pblock->GetValue(PB_AMPLITUDE2,t,sd.amp2,FOREVER);
	obj->pblock->GetValue(PB_WAVELEN,t,sd.wave,FOREVER);
	obj->pblock->GetValue(PB_PHASE,t,sd.phase,FOREVER);
	obj->pblock->GetValue(PB_DECAY,t,sd.decay,FOREVER);
	LimitValue(sd.decay,0.0f,float(1.0E30));

	sd.time = t;
	//sd.itm  = nodeRef->GetNodeTM(t,&valid);
	sd.itm  = nodeRef->GetObjectTM(t,&valid);
	sd.tm   = Inverse( sd.itm );
	return sd;
	}

RefTargetHandle SinWaveMod::Clone(RemapDir& remap) 
	{
	SinWaveMod *newob = new SinWaveMod(nodeRef,(SinWaveObject*)obRef);
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}

// --- LinWaveMod -------

Deformer& LinWaveMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	Interval valid;
	static LinWaveDeformer ld;
	pblock->GetValue(PB_FLEX,t,ld.flex,FOREVER);
	LinWaveObject *obj = (LinWaveObject*)GetWSMObject(t);
	obj->pblock->GetValue(PB_AMPLITUDE,t,ld.amp,FOREVER);
	obj->pblock->GetValue(PB_AMPLITUDE2,t,ld.amp2,FOREVER);
	obj->pblock->GetValue(PB_WAVELEN,t,ld.wave,FOREVER);
	obj->pblock->GetValue(PB_PHASE,t,ld.phase,FOREVER);
	obj->pblock->GetValue(PB_DECAY,t,ld.decay,FOREVER);
	LimitValue(ld.decay,0.0f,float(1.0E30));

	ld.time = t;
	//ld.itm  = nodeRef->GetNodeTM(t,&valid);
	ld.itm  = nodeRef->GetObjectTM(t,&valid);
	ld.tm   = Inverse(ld.itm);
	//ld.dist = mc.box ? mc.box->Width().x : (4.0f*ld.wave);
	//ld.dist = (2.0f*ld.wave); // Use wave length for the WSM version
		
	int numSides, divs;
	obj->pblock->GetValue(PB_CIRCLES,t,numSides,FOREVER);
	obj->pblock->GetValue(PB_DIVISIONS,t,divs,FOREVER);	
	ld.dist = (ld.wave/float(divs)) * 4.0f * float(numSides);

	if (ld.dist == 0.0f) ld.dist = 1.0f;
	return ld;
	}

RefTargetHandle LinWaveMod::Clone(RemapDir& remap) 
	{
	LinWaveMod *newob = new LinWaveMod(nodeRef,(LinWaveObject*)obRef);
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}



//--- Parameter map/block descriptors -----------------------------

//
//
// Parameters

static ParamUIDesc descParamOMod[] = {
	
	// Amplitude 1
	ParamUIDesc(
		PB_AMPLITUDE,
		EDITTYPE_UNIVERSE,
		IDC_AMPLITUDE,IDC_AMPSPINNER,
		-9999999.0f, 9999999.0f,
		SPIN_AUTOSCALE),	

	// Amplitude 2
	ParamUIDesc(
		PB_AMPLITUDE2,
		EDITTYPE_UNIVERSE,
		IDC_AMPLITUDE2,IDC_AMPSPINNER2,
		-9999999.0f, 9999999.0f,
		SPIN_AUTOSCALE),	

	// Wave Length
	ParamUIDesc(
		PB_WAVELEN,
		EDITTYPE_UNIVERSE,
		IDC_WAVELEN,IDC_WAVELENSPINNER,
		-9999999.0f, 9999999.0f,
		SPIN_AUTOSCALE),	

	// Phase
	ParamUIDesc(
		PB_PHASE,
		EDITTYPE_FLOAT,
		IDC_PHASE,IDC_PHASESPINNER,
		-9999999.0f, 9999999.0f,
		0.1f),
		
	// Decay
	ParamUIDesc(
		PB_DECAY,
		EDITTYPE_FLOAT,
		IDC_DECAY,IDC_DECAYSPINNER,
		0.0f, 9999999.0f,
		0.001f),
	};
#define OMODPARAMDESC_LENGH 5

ParamBlockDescID descOModVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 } };
#define OMODPBLOCK_LENGTH	5

#define CURRENT_OMODVERSION	0


//--- WaveOMod methods -----------------------------------------


WaveOMod::WaveOMod()
	{		
	MakeRefByID(FOREVER, SIMPMOD_PBLOCKREF, 
		CreateParameterBlock(descOModVer0, OMODPBLOCK_LENGTH, CURRENT_OMODVERSION));
	
	pblock->SetValue(PB_WAVELEN,0,50.0f);
	}


void WaveOMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleMod::BeginEditParams(ip,flags,prev);
		
	pmapParam = CreateCPParamMap(
		descParamOMod,OMODPARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(DialogID()),
		GetString(IDS_RB_PARAMETERS),
		0);		
	}
		
void WaveOMod::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
	{
	SimpleMod::EndEditParams(ip,flags,next);
	DestroyCPParamMap(pmapParam);
	}

Interval WaveOMod::GetValidity(TimeValue t) 
	{	
	Interval valid = FOREVER;	
	float f;	
	pblock->GetValue(PB_AMPLITUDE,t,f,valid);
	pblock->GetValue(PB_AMPLITUDE2,t,f,valid);
	pblock->GetValue(PB_WAVELEN,t,f,valid);
	pblock->GetValue(PB_PHASE,t,f,valid);
	pblock->GetValue(PB_DECAY,t,f,valid);	
	return valid;	
	}

ParamDimension *WaveOMod::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_AMPLITUDE:	return stdWorldDim;			
		case PB_AMPLITUDE2:	return stdWorldDim;	
		case PB_WAVELEN: 	return stdWorldDim;		
		case PB_PHASE:		return stdNormalizedDim;		
		case PB_DECAY:		return stdNormalizedDim;			
		default:			return defaultDim;
		}
	}

TSTR WaveOMod::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_AMPLITUDE:	return GetString(IDS_RB_AMPLITUDE1);
		case PB_AMPLITUDE2:	return GetString(IDS_RB_AMPLITUDE2);
		case PB_WAVELEN:	return GetString(IDS_RB_WAVELEN);
		case PB_PHASE:		return GetString(IDS_RB_PHASE);
		case PB_DECAY:		return GetString(IDS_RB_DECAY);
		default:			return TSTR(_T(""));
		}
	}


//--- SinWaveOMod methods -----------------------------------------

Deformer& SinWaveOMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	Interval valid;
	static SinWaveDeformer sd;	
	pblock->GetValue(PB_AMPLITUDE,t,sd.amp,FOREVER);
	pblock->GetValue(PB_AMPLITUDE2,t,sd.amp2,FOREVER);
	pblock->GetValue(PB_WAVELEN,t,sd.wave,FOREVER);
	pblock->GetValue(PB_PHASE,t,sd.phase,FOREVER);
	pblock->GetValue(PB_DECAY,t,sd.decay,FOREVER);
	LimitValue(sd.decay,0.0f,float(1.0E30));
	sd.time = t;
	sd.itm  = invmat;
	sd.tm   = mat;
	sd.flex = 1.0f;
	return sd;
	}

RefTargetHandle SinWaveOMod::Clone(RemapDir& remap) 
	{
	SinWaveOMod *newob = new SinWaveOMod();
	newob->ReplaceReference(SIMPMOD_PBLOCKREF,pblock->Clone(remap));
	newob->SimpleModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}


//--- LinWaveOMod methods -----------------------------------------

Deformer& LinWaveOMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	Interval valid;
	static LinWaveDeformer ld;	
	pblock->GetValue(PB_AMPLITUDE,t,ld.amp,FOREVER);
	pblock->GetValue(PB_AMPLITUDE2,t,ld.amp2,FOREVER);
	pblock->GetValue(PB_WAVELEN,t,ld.wave,FOREVER);
	pblock->GetValue(PB_PHASE,t,ld.phase,FOREVER);
	pblock->GetValue(PB_DECAY,t,ld.decay,FOREVER);
	LimitValue(ld.decay,0.0f,float(1.0E30));
	ld.time = t;
	ld.itm  = invmat;
	ld.tm   = mat;
	ld.flex = 1.0f;
	ld.dist = mc.box ? mc.box->Width().x : (4.0f*ld.wave);
	return ld;
	}

RefTargetHandle LinWaveOMod::Clone(RemapDir& remap) 
	{
	LinWaveOMod *newob = new LinWaveOMod();
	newob->ReplaceReference(SIMPMOD_PBLOCKREF,pblock->Clone(remap));
	newob->SimpleModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}

#endif // NO_MODIFIER_RIPPLE

