/*****************************************************************************
 *<
	FILE: force.cpp

	DESCRIPTION: Point Force icon for Dynamics / Force field for Particles

	CREATED BY: Eric Peterson (from Audrey's PBOMB.CPP)

	HISTORY: 6/97

    Modified: 9/18/01 Bayboro: removing dependency to EDP

 *>	Copyright (c) 1997, All Rights Reserved, assigned to, and for Yost Group Inc.
 *****************************************************************************/
#include "dynwarps.h"
#include "dynw.h"
#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"
// #include "pod.h" // Bayboro 9/18/01

static Class_ID FORCE_CLASS_ID(0x3d4d3ca5, 0x3b22358f);
static Class_ID FORCEMOD_CLASS_ID(0x44975ae9, 0x65f53e3a);

class ForceMod;

class ForceObject : public SimpleWSMObject // , IOperatorInterface  // Bayboro 9/18/01
{	
	public:									
		ForceObject();		
		~ForceObject();		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		ForceField *GetForceField(INode *node);
		BOOL SupportsDynamics() {return TRUE;}

		// From Animatable		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void MapKeys(TimeMap *map,DWORD flags);
				
		// from object		
		CreateMouseCallBack* GetCreateMouseCallBack();		
		
		// From SimpleWSMObject		
		void InvalidateUI();		

		// From Animatable		
		void DeleteThis() {delete this;}		
		Class_ID ClassID() {return FORCE_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_FORCE);}

		// From BaseObject
		IParamArray *GetParamBlock() {return pblock;}
		int GetParamBlockIndex(int id) {return id;}
						
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);
		
		// From SimpleWSMObject				
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void BuildMesh(TimeValue t);

		int DialogID() {return IDD_SW_DYNFORCE;}
		ParamUIDesc *UIDesc();
		int UIDescLength();
		TSTR UIStrName();
//		int NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index); // Bayboro 9/18/01
//		void* GetInterface(ULONG id); // Bayboro 9/18/01
		ForceField *ff;
		void SetUpModifier(TimeValue t,INode *node);
		ForceMod *mf;
};

IObjParam *ForceObject::ip        = NULL;
IParamMap *ForceObject::pmapParam = NULL;
HWND       ForceObject::hSot      = NULL;

class ForceClassDesc:public ClassDesc
{	public:
		int 			IsPublic() {return 1;}
		void *			Create(BOOL loading = FALSE) { return new ForceObject;}
		const TCHAR *	ClassName() {return GetString(IDS_AP_FORCE_CLASS);}
		SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
		Class_ID		ClassID() {return FORCE_CLASS_ID;}
		const TCHAR* 	Category() {return GetString(IDS_EP_SW_FORCES);}
};

static ForceClassDesc ForceDesc;
ClassDesc* GetForceObjDesc() {return &ForceDesc;}

class PtForceField : public ForceField
{	public:
		ForceObject *obj;
		TimeValue dtsq,dt;
		INode *node;
		int count;
		Matrix3 tm,invtm;
		Interval tmValid;
		Point3 force;
		Interval fValid;
		Point3 Force(TimeValue t,const Point3 &pos, const Point3 &vel, int index);
};

class ForceMod : public SimpleWSMMod
{	public:				
		PtForceField force;
		ForceMod() {}
		ForceMod(INode *node,ForceObject *obj);		
		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_AP_FORCEMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return FORCEMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_FORCEBINDING);}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
};

class ForceModClassDesc:public ClassDesc
{	public:
		int 			IsPublic() { return 0; }
		void *			Create(BOOL loading = FALSE) {return new ForceMod;}
		const TCHAR *	ClassName() { return GetString(IDS_AP_FORCEMOD);}
		SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
		Class_ID		ClassID() {return FORCEMOD_CLASS_ID;}
		const TCHAR* 	Category() {return _T("");}
};

static ForceModClassDesc ForceModDesc;

ClassDesc* GetForceModDesc() {return &ForceModDesc;}

class ForceModData : public LocalModData
{	public:
		LocalModData *Clone ();
};

LocalModData *ForceModData::Clone ()
{	ForceModData *clone;
	clone = new ForceModData ();
	return(clone);
}

//--- ForceObject Parameter map/block descriptors ------------------
#define PB_ONTIME			0
#define PB_OFFTIME			1
#define PB_STRENGTH			2
#define PB_UNITS			3
#define PB_FEEDBACKON		4 
#define PB_REVERSIBLE		5
#define PB_TARGETVEL		6
#define PB_CONTROLGAIN		7
#define PB_ENABLESINES		8
#define PB_TIMEPER1			9
#define PB_AMP1				10
#define PB_PHASPER1			11
#define PB_TIMEPER2			12
#define PB_AMP2				13
#define PB_PHASPER2			14
#define PB_RANGEON			15
#define PB_RANGEVAL			16
#define PB_ICONSIZE			17

static int UnitsIDs[] = {IDC_AP_FINNEWTONS,IDC_AP_FINLBS};

static ParamUIDesc descParamForce[] = {

	// Force Time On
	ParamUIDesc(
		PB_ONTIME,
		EDITTYPE_TIME,
		IDC_AP_FONTIME,IDC_AP_FONTIMESPIN,
		-999999999.0f,999999999.0f,
		10.0f),
	
	// Force Time Off
	ParamUIDesc(
		PB_OFFTIME,
		EDITTYPE_TIME,
		IDC_AP_FOFFTIME,IDC_AP_FOFFTIMESPIN,
		-999999999.0f,999999999.0f,
		10.0f),
	
	// Strength
	ParamUIDesc(
		PB_STRENGTH,
		EDITTYPE_FLOAT,
		IDC_AP_FVALUE,IDC_AP_FVALUESPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Force Units
	ParamUIDesc(PB_UNITS,TYPE_RADIO,UnitsIDs,2),

	// Feedback
	ParamUIDesc(PB_FEEDBACKON,TYPE_SINGLECHEKBOX,IDC_AP_FEEDBACKON),

	// Reversible
	ParamUIDesc(PB_REVERSIBLE,TYPE_SINGLECHEKBOX,IDC_AP_REVERSIBLE),
	
	// Target Speed
	ParamUIDesc(
		PB_TARGETVEL,
		EDITTYPE_UNIVERSE,
		IDC_AP_CONTROLPT,IDC_AP_CONTROLPTSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Gain
	ParamUIDesc(
		PB_CONTROLGAIN,
		EDITTYPE_FLOAT,
		IDC_AP_GAIN,IDC_AP_GAINSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Reversible
	ParamUIDesc(PB_ENABLESINES,TYPE_SINGLECHEKBOX,IDC_AP_VARIATIONON),
	
	// Period 1
	ParamUIDesc(
		PB_TIMEPER1,
		EDITTYPE_TIME,
		IDC_AP_AMPVART1,IDC_AP_AMPVART1SPIN,
		0.0f,999999999.0f,
		10.0f),

	// Amp 1
	ParamUIDesc(
		PB_AMP1,
		EDITTYPE_FLOAT,
		IDC_AP_AMPAMP1,IDC_AP_AMPAMP1SPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Phase 1
	ParamUIDesc(
		PB_PHASPER1,
		EDITTYPE_FLOAT,
		IDC_AP_AMPPHAS1,IDC_AP_AMPPHAS1SPIN,
		0.0f,360.0f,
		0.5f),

	// Period 2
	ParamUIDesc(
		PB_TIMEPER2,
		EDITTYPE_TIME,
		IDC_AP_AMPVART2,IDC_AP_AMPVART2SPIN,
		0.0f,999999999.0f,
		10.0f),

	// Amp 2
	ParamUIDesc(
		PB_AMP2,
		EDITTYPE_FLOAT,
		IDC_AP_AMPAMP2,IDC_AP_AMPAMP2SPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Phase 2
	ParamUIDesc(
		PB_PHASPER2,
		EDITTYPE_FLOAT,
		IDC_AP_AMPPHAS2,IDC_AP_AMPPHAS2SPIN,
		0.0f,360.0f,
		0.5f),

	// Range on
	ParamUIDesc(PB_RANGEON,TYPE_SINGLECHEKBOX,IDC_AP_RANGE),
	
	// Range limit
	ParamUIDesc(
		PB_RANGEVAL,
		EDITTYPE_UNIVERSE,
		IDC_AP_RANGEVAL,IDC_AP_RANGEVALSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Icon Size
	ParamUIDesc(
		PB_ICONSIZE,
		EDITTYPE_UNIVERSE,
		IDC_AP_DYNF_ICONSIZE,IDC_AP_DYNF_ICONSIZESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	};

#define FORCEPARAMDESC_LENGTH	18

ParamBlockDescID descForceVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	// PB_ONTIME
	{ TYPE_INT, NULL, FALSE, 1 },	// PB_OFFTIME
	{ TYPE_FLOAT, NULL, TRUE, 2 },	// PB_STRENGTH
	{ TYPE_INT, NULL, FALSE, 3 },	// PB_UNITS
	{ TYPE_INT, NULL, FALSE, 4 },	// PB_FEEDBACKON
	{ TYPE_INT, NULL, FALSE, 5 },	// PB_REVERSIBLE
	{ TYPE_FLOAT, NULL, TRUE, 6 },	// PB_TARGETVEL
	{ TYPE_FLOAT, NULL, TRUE, 7 },  // PB_CONTROLGAIN
	{ TYPE_INT, NULL, FALSE, 8 },   // PB_ENABLESINES
	{ TYPE_INT, NULL, TRUE, 9 },    // PB_TIMEPER1
	{ TYPE_FLOAT, NULL, TRUE, 10 }, // PB_AMP1
	{ TYPE_FLOAT, NULL, TRUE, 11 }, // PB_PHASPER1
	{ TYPE_INT, NULL, TRUE, 12 },   // PB_TIMEPER2
	{ TYPE_FLOAT, NULL, TRUE, 13 }, // PB_AMP2
	{ TYPE_FLOAT, NULL, TRUE, 14 }, // PB_PHASPER2
	{ TYPE_INT, NULL, FALSE, 15 },  // PB_RangeOn
	{ TYPE_FLOAT, NULL, TRUE, 16 }, // PB_Range
	{ TYPE_FLOAT, NULL, FALSE, 17}  // PB_ICONSIZE
	};

#define PBLOCK_LENGTH	18

#define CURRENT_VERSION	0


//--- Deflect object methods -----------------------------------------

ForceObject::ForceObject()
{	int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	MakeRefByID(FOREVER, 0,CreateParameterBlock(descForceVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);	
	ff = NULL;
	mf = NULL;
	pblock->SetValue(PB_ONTIME,0,0);
	pblock->SetValue(PB_OFFTIME,0,FToTick*30);
	pblock->SetValue(PB_STRENGTH,0,1.0f);
	pblock->SetValue(PB_UNITS,0,0);
	pblock->SetValue(PB_FEEDBACKON,0,0);
	pblock->SetValue(PB_REVERSIBLE,0,0);
	pblock->SetValue(PB_CONTROLGAIN,0,50.0f);
	pblock->SetValue(PB_TARGETVEL,0,100.0f);
	pblock->SetValue(PB_ENABLESINES,0,0);
	pblock->SetValue(PB_TIMEPER1,0,100);
	pblock->SetValue(PB_AMP1,0,100.0f);
	pblock->SetValue(PB_PHASPER1,0,0.0f);
	pblock->SetValue(PB_TIMEPER2,0,100);
	pblock->SetValue(PB_AMP2,0,100.0f);
	pblock->SetValue(PB_RANGEON,0,0);
	pblock->SetValue(PB_RANGEVAL,0,1000.0f);
	pblock->SetValue(PB_PHASPER2,0,0.0f);
}

class ForceDlgProc : public ParamMapUserDlgProc
{	public:
		ForceObject *po;
		HWND hwnd;
		ForceDlgProc(ForceObject *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void Update(TimeValue t);
		void DoFeedback();
		void DoEnableVar();
		void DoRange();
		void DeleteThis() {delete this;}
};
void ForceDlgProc::DoFeedback()
{ int feedbackon;
  po->pblock->GetValue(PB_FEEDBACKON,0,feedbackon,FOREVER);
  if (feedbackon)
  {	SpinnerOn(hwnd,IDC_AP_CONTROLPTSPIN,IDC_AP_CONTROLPT);
	SpinnerOn(hwnd,IDC_AP_GAINSPIN,IDC_AP_GAIN);
  }
  else
  {	SpinnerOff(hwnd,IDC_AP_CONTROLPTSPIN,IDC_AP_CONTROLPT);
	SpinnerOff(hwnd,IDC_AP_GAINSPIN,IDC_AP_GAIN);
  }
  EnableWindow(GetDlgItem(hwnd,IDC_AP_REVERSIBLE),feedbackon);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_CONTROLPT_TXT),feedbackon);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_GAIN_TXT),feedbackon);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_GAIN_PCNT),feedbackon);
}
void ForceDlgProc::DoEnableVar()
{ int enablevar;
  po->pblock->GetValue(PB_ENABLESINES,0,enablevar,FOREVER);
  if (enablevar)
  {	SpinnerOn(hwnd,IDC_AP_AMPVART1SPIN,IDC_AP_AMPVART1);
 	SpinnerOn(hwnd,IDC_AP_AMPAMP1SPIN,IDC_AP_AMPAMP1);
	SpinnerOn(hwnd,IDC_AP_AMPPHAS1SPIN,IDC_AP_AMPPHAS1);
	SpinnerOn(hwnd,IDC_AP_AMPVART2SPIN,IDC_AP_AMPVART2);
	SpinnerOn(hwnd,IDC_AP_AMPAMP2SPIN,IDC_AP_AMPAMP2);
	SpinnerOn(hwnd,IDC_AP_AMPPHAS2SPIN,IDC_AP_AMPPHAS2);
  }
  else
  {	SpinnerOff(hwnd,IDC_AP_AMPVART1SPIN,IDC_AP_AMPVART1);
 	SpinnerOff(hwnd,IDC_AP_AMPAMP1SPIN,IDC_AP_AMPAMP1);
 	SpinnerOff(hwnd,IDC_AP_AMPPHAS1SPIN,IDC_AP_AMPPHAS1);
 	SpinnerOff(hwnd,IDC_AP_AMPVART2SPIN,IDC_AP_AMPVART2);
	SpinnerOff(hwnd,IDC_AP_AMPAMP2SPIN,IDC_AP_AMPAMP2);
	SpinnerOff(hwnd,IDC_AP_AMPPHAS2SPIN,IDC_AP_AMPPHAS2);
  }
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPVART1_TXT),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPAMP1_TXT),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPAMP1_PCNT),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPPHAS1_TXT),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPPHAS1_DEG),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPVART2_TXT),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPAMP2_TXT),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPAMP2_PCNT),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPPHAS2_TXT),enablevar);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_AMPPHAS2_DEG),enablevar);
}
void ForceDlgProc::DoRange()
{ int enablerng;
  po->pblock->GetValue(PB_RANGEON,0,enablerng,FOREVER);
  if (enablerng) 
	SpinnerOn(hwnd,IDC_AP_RANGEVALSPIN,IDC_AP_RANGEVAL);
  else
	SpinnerOff(hwnd,IDC_AP_RANGEVALSPIN,IDC_AP_RANGEVAL);
  EnableWindow(GetDlgItem(hwnd,IDC_AP_RANGEVAL_TXT),enablerng);
}

void ForceDlgProc::Update(TimeValue t)
{ DoFeedback();
  DoEnableVar();
  DoRange();
}

BOOL ForceDlgProc::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{	switch (msg) 
	{	case WM_INITDIALOG: 
		{ hwnd=hWnd;
		  Update(t);
		  break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{	case IDC_AP_FEEDBACKON:
					DoFeedback();
					return TRUE;
				case IDC_AP_VARIATIONON:
					DoEnableVar();
					return TRUE;
				case IDC_AP_RANGE:
					DoRange();
					return TRUE;
				default: return TRUE;
			}
	}
	return FALSE;
}

void ForceObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	if (pmapParam)
	{	// Left over
		pmapParam->SetParamBlock(pblock);
	}
	else
	{	hSot = ip->AddRollupPage(hInstance,MAKEINTRESOURCE(IDD_SW_DESC),DefaultSOTProc,GetString(IDS_AP_TOP),(LPARAM)ip,APPENDROLL_CLOSED);
		// Gotta make a new one.
		pmapParam = CreateCPParamMap(descParamForce,FORCEPARAMDESC_LENGTH,pblock,ip,hInstance,MAKEINTRESOURCE(IDD_SW_DYNFORCE),GetString(IDS_AP_PARAMETERS),0);
	}
	if (pmapParam) pmapParam->SetUserDlgProc(new ForceDlgProc(this));
}

void ForceObject::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;
	if (flags&END_EDIT_REMOVEUI )
	{	DestroyCPParamMap(pmapParam);
		ip->DeleteRollupPage(hSot);
		pmapParam = NULL;		
	}	
}

ForceField *ForceObject::GetForceField(INode *node)
{	PtForceField *pb = new PtForceField;	
	pb->obj  = this;
	pb->node = node;
	pb->tmValid.SetEmpty();
	pb->fValid.SetEmpty();
	pb->dt=GetTicksPerFrame();
	pb->dtsq=pb->dt*pb->dt;
	return pb;
}

void ForceObject::MapKeys(TimeMap *map,DWORD flags)
{	TimeValue TempTime;
// mapped values
	pblock->GetValue(PB_ONTIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_ONTIME,0,TempTime);
	pblock->GetValue(PB_OFFTIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_OFFTIME,0,TempTime);
// scaled values
	pblock->GetValue(PB_TIMEPER1,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_TIMEPER1,0,TempTime);
	pblock->GetValue(PB_TIMEPER2,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_TIMEPER2,0,TempTime);
}

#define NUM_SEGS 12
#define NUM_SEGS2 24

void ForceObject::BuildMesh(TimeValue t)
{	ivalid = FOREVER;
	float l1,l2,l3,l4,r1,r2,r3,length;
	pblock->GetValue(PB_ICONSIZE,t,length,ivalid);
	int dorange,dofrange=0,norvs,norfs;
	pblock->GetValue(PB_RANGEON,t,dorange,ivalid);
	if (dorange){ dorange=73;dofrange=72;}
	l1=0.0f;
	l2=length/2.0f;
	l3=0.1f*length;
	l4=length;
	r1=length/6.0f;
	r2=r1/2.0f;
	r3=(r1+r2)/2.0f;
	float u,cosu,sinu,r1cosu,r2cosu,r3cosu,r1sinu,r2sinu,r3sinu;
	mesh.setNumVerts((norvs=76)+dorange);
	mesh.setNumFaces((norfs=84)+dofrange);
	int i,i2,i3,i4,i5,i6,i7;
	i2=NUM_SEGS;
	i3=2*NUM_SEGS;
	i4=3*NUM_SEGS;
	i5=4*NUM_SEGS;
	i6=5*NUM_SEGS;
	i7=6*NUM_SEGS;
    for (i=0;i<NUM_SEGS;i++)
    {	u=float(i)/float(NUM_SEGS)*TWOPI;
		cosu=(float)cos(u);sinu=(float)sin(u);
		r1cosu=r1*cosu;r1sinu=r1*sinu;
		r2cosu=r2*cosu;r2sinu=r2*sinu;
		r3cosu=r3*cosu;r3sinu=r3*sinu;
		mesh.setVert(i,   Point3(r1cosu,r1sinu,-l4));
		mesh.setVert(i+i2,Point3(r1cosu,r1sinu,-l2));
		mesh.setVert(i+i3,Point3(r2cosu,r2sinu,-l2));
		mesh.setVert(i+i4,Point3(r2cosu,r2sinu,-l3));
		mesh.setVert(i+i5,Point3(r3cosu,r3sinu,-l3));
		mesh.setVert(i+i6,Point3(r3cosu,r3sinu,-l1));
	}
    mesh.setVert(i7  ,Point3(0.0f,0.0f,-l4));
    mesh.setVert(i7+1,Point3(0.0f,0.0f,-l2));
    mesh.setVert(i7+2,Point3(0.0f,0.0f,-l3));
    mesh.setVert(i7+3,Point3(0.0f,0.0f,-l1));
    for (i=0;i<NUM_SEGS;i++) 
    {	int v,v2,v3,v4,v5,v6;
		if (i>(NUM_SEGS-2))
		{	v=0;
			v2=i2;
			v3=i3;
			v4=i4;
			v5=i5;
			v6=i6;
		}
		else
		{	v=i+1;
			v2=v+i2;
			v3=v+i3;
			v4=v+i4;
			v5=v+i5;
			v6=v+i6;
		}
		mesh.faces[i].setEdgeVisFlags(1,0,0);
	    mesh.faces[i].setSmGroup(0);
	    mesh.faces[i].setVerts(i,v,i7);
		int ii2=i+i2;
		mesh.faces[ii2].setEdgeVisFlags(1,0,0);
	    mesh.faces[ii2].setSmGroup(0);
	    mesh.faces[ii2].setVerts(ii2,v2,i7+1);
		int ii3=i+i3;
		mesh.faces[ii3].setEdgeVisFlags(1,0,0);
	    mesh.faces[ii3].setSmGroup(0);
	    mesh.faces[ii3].setVerts(ii3,v3,i7+1);
		int ii4=i+i4;
		mesh.faces[ii4].setEdgeVisFlags(1,0,0);
	    mesh.faces[ii4].setSmGroup(0);
	    mesh.faces[ii4].setVerts(ii4,v4,i7+2);
		int ii5=i+i5;
		mesh.faces[ii5].setEdgeVisFlags(1,0,0);
	    mesh.faces[ii5].setSmGroup(0);
	    mesh.faces[ii5].setVerts(ii5,v5,i7+2);
		int ii6=i+i6;
		mesh.faces[ii6].setEdgeVisFlags(1,0,0);
	    mesh.faces[ii6].setSmGroup(0);
	    mesh.faces[ii6].setVerts(ii6,v6,i7+3);
    }
	mesh.faces[72].setEdgeVisFlags(1,0,0);
    mesh.faces[72].setSmGroup(2);
    mesh.faces[72].setVerts(0,12,6);
	mesh.faces[73].setEdgeVisFlags(0,0,1);
    mesh.faces[73].setSmGroup(2);
    mesh.faces[73].setVerts(6,12,18);
	mesh.faces[74].setEdgeVisFlags(1,0,0);
    mesh.faces[74].setSmGroup(2);
    mesh.faces[74].setVerts(3,15,9);
	mesh.faces[75].setEdgeVisFlags(0,0,1);
    mesh.faces[75].setSmGroup(2);
    mesh.faces[75].setVerts(9,15,21);
	mesh.faces[76].setEdgeVisFlags(1,0,0);
    mesh.faces[76].setSmGroup(2);
    mesh.faces[76].setVerts(24,36,30);
	mesh.faces[77].setEdgeVisFlags(0,0,1);
    mesh.faces[77].setSmGroup(2);
    mesh.faces[77].setVerts(30,36,42);
	mesh.faces[78].setEdgeVisFlags(1,0,0);
    mesh.faces[78].setSmGroup(2);
    mesh.faces[78].setVerts(27,39,33);
	mesh.faces[79].setEdgeVisFlags(0,0,1);
    mesh.faces[79].setSmGroup(2);
    mesh.faces[79].setVerts(33,39,45);
	mesh.faces[80].setEdgeVisFlags(1,0,0);
    mesh.faces[80].setSmGroup(2);
    mesh.faces[80].setVerts(48,60,54);
	mesh.faces[81].setEdgeVisFlags(0,0,1);
    mesh.faces[81].setSmGroup(2);
    mesh.faces[81].setVerts(54,60,66);
	mesh.faces[82].setEdgeVisFlags(1,0,0);
    mesh.faces[82].setSmGroup(2);
    mesh.faces[82].setVerts(51,63,57);
	mesh.faces[83].setEdgeVisFlags(0,0,1);
    mesh.faces[83].setSmGroup(2);
    mesh.faces[83].setVerts(57,63,69);
	if (dorange)
	{   int newv;
		pblock->GetValue(PB_RANGEVAL,t,length,ivalid);
		int i;
		for (i=0; i<NUM_SEGS2; i++)
	    { u = float(i)/float(NUM_SEGS2) * TWOPI;
		  mesh.setVert(i+norvs, Point3((float)cos(u) * length, (float)sin(u) * length, 0.0f));
	    }
		newv=NUM_SEGS2+norvs;
	    for (i=0; i<NUM_SEGS2; i++) 
	    { u = float(i)/float(NUM_SEGS2) * TWOPI;
		  mesh.setVert(i+newv, Point3(0.0f, (float)cos(u) * length, (float)sin(u) * length));
	    } newv+=NUM_SEGS2;
	    for (i=0; i<NUM_SEGS2; i++)
	    { u = float(i)/float(NUM_SEGS2) * TWOPI;
		  mesh.setVert(i+newv, Point3((float)cos(u) * length, 0.0f, (float)sin(u) * length));
	    }	
		newv+=NUM_SEGS2;
	    mesh.setVert(newv, Point3(0.0f, 0.0f, 0.0f));
		int vi=norvs;
	    for (i=norfs; i<norfs+dofrange; i++) 
	    { int i1 = vi+1;
	      if ((i1-norvs)%NUM_SEGS2==0) i1 -= NUM_SEGS2;
	      mesh.faces[i].setEdgeVisFlags(1,0,0);
	      mesh.faces[i].setSmGroup(0);
	      mesh.faces[i].setVerts(vi,i1,newv);
		  vi++;
		}
	}
	mesh.InvalidateGeomCache();
}

class ForceObjCreateCallback : public CreateMouseCallBack
{	public:
		ForceObject *ob;	
		Point3 p0,p1;
		IPoint2 sp0;
		int proc(ViewExp *vpt,int msg,int point,int flags,IPoint2 m,Matrix3& mat);
};

int ForceObjCreateCallback::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
{
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{ vpt->SnapPreview(m,m,NULL, snapdim);
	}
#endif
	
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE)
	 {	switch(point)
		{	case 0:								
				// if hidden by category, re-display particles and objects
				GetCOREInterface()->SetHideByCategoryFlags(GetCOREInterface()->GetHideByCategoryFlags() & ~(HIDE_OBJECTS|HIDE_PARTICLES));
				sp0 = m;
				p0  = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
				break;
			case 1:
				p1  = vpt->SnapPoint(m,m,NULL,snapdim);
				float x=Length(p1-p0);
				ob->pblock->SetValue(PB_ICONSIZE,0,x);
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT)
				{	if (Length(m-sp0)<3) return CREATE_ABORT;
					else return CREATE_STOP;
				}
				break;
		}
	}
	else
	{	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;
	}
	return TRUE;
}

static ForceObjCreateCallback ForceCreateCB;

CreateMouseCallBack* ForceObject::GetCreateMouseCallBack()
{	ForceCreateCB.ob = this;
	return &ForceCreateCB;
}

void ForceObject::InvalidateUI() 
{	if (pmapParam) pmapParam->Invalidate();}

Modifier *ForceObject::CreateWSMMod(INode *node)
{	return new ForceMod(node,this);}

RefTargetHandle ForceObject::Clone(RemapDir& remap) 
{	ForceObject* newob = new ForceObject();
	newob->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
}

ParamDimension *ForceObject::GetParameterDim(int pbIndex) 
{	switch (pbIndex)
	{	case PB_ONTIME:		return stdTimeDim;
		case PB_OFFTIME:	return stdTimeDim;
		case PB_TIMEPER1:	return stdTimeDim;
		case PB_TIMEPER2:	return stdTimeDim;
		case PB_PHASPER1:	return stdAngleDim;
		case PB_PHASPER2:	return stdAngleDim;
		default: return defaultDim;
	}
}

TSTR ForceObject::GetParameterName(int pbIndex) 
{	switch (pbIndex)
	{	case PB_ONTIME:		return GetString(IDS_AP_ONTIME);
		case PB_OFFTIME:	return GetString(IDS_AP_OFFTIME);
		case PB_STRENGTH:	return GetString(IDS_AP_STRENGTH);
		case PB_UNITS:		return GetString(IDS_AP_UNITS);
		case PB_FEEDBACKON:	return GetString(IDS_AP_FEEDBACKON);
		case PB_REVERSIBLE:	return GetString(IDS_AP_REVERSIBLE);
		case PB_TARGETVEL:	return GetString(IDS_AP_TARGETVEL);
		case PB_CONTROLGAIN:return GetString(IDS_AP_CONTROLGAIN);
		case PB_ENABLESINES:return GetString(IDS_AP_ENABLESINES);
		case PB_TIMEPER1:	return GetString(IDS_AP_TIMEPER1);
		case PB_AMP1:		return GetString(IDS_AP_AMP1);
		case PB_PHASPER1:	return GetString(IDS_AP_PHASPER1);
		case PB_TIMEPER2:	return GetString(IDS_AP_TIMEPER2);
		case PB_AMP2:		return GetString(IDS_AP_AMP2);
		case PB_PHASPER2:	return GetString(IDS_AP_PHASPER2);
		case PB_RANGEON:	return GetString(IDS_AP_RANGEON);
		case PB_RANGEVAL:	return GetString(IDS_AP_RANGEVAL);
		case PB_ICONSIZE:	return GetString(IDS_AP_ICONSIZE);
		default: 			return TSTR(_T(""));
	}
}

ParamUIDesc *ForceObject::UIDesc()
{	return descParamForce;}

int ForceObject::UIDescLength()
{	return FORCEPARAMDESC_LENGTH;}

TSTR ForceObject::UIStrName()
{   return GetString(IDS_AP_FORCEPARAM);}


ForceMod::ForceMod(INode *node,ForceObject *obj)
{	//MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);
	pblock = NULL;
	obRef = NULL;
	}

Interval ForceMod::GetValidity(TimeValue t) 
{	if (nodeRef)
	{	Interval valid=FOREVER;
		Matrix3 tm;
		float f;
		TimeValue tt;
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_STRENGTH,t,f,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_TARGETVEL,t,f,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_CONTROLGAIN,t,f,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_TIMEPER1,t,tt,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_TIMEPER2,t,tt,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_PHASPER1,t,f,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_PHASPER2,t,f,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_AMP1,t,f,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_AMP2,t,f,valid);
		((ForceObject*)GetWSMObject(t))->pblock->GetValue(PB_RANGEVAL,t,f,valid);
		tm=nodeRef->GetObjectTM(t,&valid);
		return valid;
	}
	else
	{	return FOREVER;
	}
}

class ForceDeformer : public Deformer
{	public:		
	Point3 Map(int i, Point3 p) {return p;}
};
static ForceDeformer gdeformer;

Deformer& ForceMod::GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{	return gdeformer;}

RefTargetHandle ForceMod::Clone(RemapDir& remap) 
{	ForceMod *newob = new ForceMod(nodeRef,(ForceObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
}

void ForceMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj)
	{	force.obj=(ForceObject*)GetWSMObject(t);
		force.node=nodeRef;
		force.tmValid.SetEmpty();
		force.fValid.SetEmpty();
		force.dt=GetTicksPerFrame();
		force.dtsq=force.dt*force.dt;
		obj->ApplyForceField(&force);
	}
}

Point3 PtForceField::Force(TimeValue t,const Point3 &pos, const Point3 &vel,int index)
{	Point3 ApplyAt,OutForce,zdir;
	fValid= FOREVER;		
	if (!tmValid.InInterval(t)) 
	{	tmValid=FOREVER;
		tm=node->GetObjectTM(t,&tmValid);
		invtm=Inverse(tm);
	}
	ApplyAt=tm.GetTrans();
	fValid&=tmValid;
	TimeValue t1,t2;
	obj->pblock->GetValue(PB_ONTIME,t,t1,fValid);
	obj->pblock->GetValue(PB_OFFTIME,t,t2,fValid);
	if ((t>=t1)&&(t<=t2))
	{	float BaseF;
		zdir=tm.GetRow(2);
		obj->pblock->GetValue(PB_STRENGTH,t,BaseF,fValid); //assume N
		int tpf=GetTicksPerFrame();
		//int tpf2=tpf*tpf;
		int tps=TIME_TICKSPERSEC;
		int tps2=tps*tps;
		//int FToTick=(int)((float)tps/(float)GetFrameRate());
		BaseF/=(float)tps2;//convert to kg-m/t2
		BaseF*=100.0f; //convert to kg-cm/t2
		int UnitsVal;
		obj->pblock->GetValue(PB_UNITS,t,UnitsVal,fValid);
		if (UnitsVal!=0) BaseF*=4.4591f; //lbf is this much larger than N=kg-m/s2
		TimeValue tage=t-t1;
		TimeValue Per1,Per2;
		obj->pblock->GetValue(PB_TIMEPER1,t,Per1,fValid);
		obj->pblock->GetValue(PB_TIMEPER2,t,Per2,fValid);
		float scalefactor=1.0f;
		int sintoggle;
		obj->pblock->GetValue(PB_ENABLESINES,t,sintoggle,fValid);
		if (sintoggle)
		{	if (Per1>0)
			{	float phase1,amp1;
				float relage1=(float)tage/(float)Per1;
				obj->pblock->GetValue(PB_PHASPER1,t,phase1,fValid);
				obj->pblock->GetValue(PB_AMP1,t,amp1,fValid);
				scalefactor+=0.01f*amp1*(float)sin(TWOPI*relage1+phase1);
			}
			if (Per2>0)
			{	float phase2,amp2;
				float relage2=(float)tage/(float)Per2;
				obj->pblock->GetValue(PB_PHASPER2,t,phase2,fValid);
				obj->pblock->GetValue(PB_AMP2,t,amp2,fValid);
				scalefactor+=0.01f*amp2*(float)sin(TWOPI*relage2+phase2);
			}
		}
		int feedback;
		obj->pblock->GetValue(PB_FEEDBACKON,t,feedback,fValid);
		float scalefactorg=1.0f;
		if (feedback)
		{	float targetspeed,loopgain;
			obj->pblock->GetValue(PB_TARGETVEL,t,targetspeed,fValid);
			targetspeed/=(float)tpf;
			obj->pblock->GetValue(PB_CONTROLGAIN,t,loopgain,fValid);
			float diffspeed=targetspeed-DotProd(vel,zdir);
			//scalefactorg*=diffspeed*loopgain/(float)tpf;
			scalefactorg*=diffspeed*loopgain/100.0f;
			int revon;
			obj->pblock->GetValue(PB_REVERSIBLE,t,revon,fValid);
			if ((!revon)&&(scalefactorg<0.0f)) scalefactorg=0.0f;
		}
		OutForce=zdir*BaseF*scalefactor*scalefactorg;
		int rangeon;
		obj->pblock->GetValue(PB_RANGEON,t,rangeon,fValid);
		if (rangeon)
		{	float maxrange;
			obj->pblock->GetValue(PB_RANGEVAL,t,maxrange,fValid);
			float distance=Length(ApplyAt-pos);
			if (distance<maxrange)
			{	if (maxrange>FLOAT_EPSILON) OutForce*=(1.0f-distance/maxrange);
				else OutForce=Zero;
			}
			else OutForce=Zero;
		}
	}
	else OutForce=Zero;
	return OutForce*6.25e-03f;
}

#define DONTCARE	2
#define NORMALOP	-1

/* // Bayboro 9/18/01
int ForceObject::NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index)
{	
	if (!mf)
		mf = (ForceMod *)CreateWSMMod(node);
	SetUpModifier(t,node);
	Point3 findforce = mf->force.Force(t,part->position,part->velocity,index);
	part->velocity += 10.0f*findforce * dt;
	return (NORMALOP);
}
*/  // Bayboro 9/18/01

ForceObject::~ForceObject()
{	
	DeleteAllRefsFromMe();
	if (ff)
		delete ff;	
	if (mf)
		delete mf;	
}

/*  // Bayboro 9/18/01
void* ForceObject::GetInterface(ULONG id) 
{
	switch (id)
	{ 
		case I_NEWPARTOPERATOR: return (IOperatorInterface*)this;
	}
	return Object::GetInterface(id);
}
*/  // Bayboro 9/18/01

void ForceObject::SetUpModifier(TimeValue t,INode *node)
{
	mf->force.obj  = (ForceObject*)(mf->GetWSMObject(t));
	mf->force.node = mf->nodeRef;
	mf->force.tmValid.SetEmpty();
	mf->force.fValid.SetEmpty();
	mf->force.dt = GetTicksPerFrame();
	mf->force.dtsq = mf->force.dt * mf->force.dt;
}





