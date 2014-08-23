/*****************************************************************************
 *<
	FILE: pin.cpp

	DESCRIPTION: Pin Constraint for use With Dynamics

	CREATED BY: Eric Peterson

	HISTORY: 10/98

 *>	Copyright (c) 1998, All Rights Reserved, assigned to, and for Yost Group Inc.
 *****************************************************************************/
#include "dynwarps.h"
#include "dynw.h"
#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"

static Class_ID PIN_CLASS_ID(0x41d14a7d, 0x793d56e4);
static Class_ID PINMOD_CLASS_ID(0x1f756c96, 0x26180a50);

class PinObject : public SimpleWSMObject {	
	public:									
		PinObject();		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		ForceField *GetForceField(INode *node); //ok
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
		Class_ID ClassID() {return PIN_CLASS_ID;}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_PIN);}

		// From BaseObject
		IParamArray *GetParamBlock() {return pblock;}
		int GetParamBlockIndex(int id) {return id;}
						
		// From WSMObject
		Modifier *CreateWSMMod(INode *node);
		
		// From SimpleWSMObject				
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void BuildMesh(TimeValue t);

		int DialogID() {return IDD_SW_DYNPIN;}
		ParamUIDesc *UIDesc();
		int UIDescLength();
		TSTR UIStrName();
	};

IObjParam *PinObject::ip        = NULL;
IParamMap *PinObject::pmapParam = NULL;
HWND       PinObject::hSot      = NULL;

class PinClassDesc:public ClassDesc
{	public:
		int 			IsPublic() {return 1;}
		void *			Create(BOOL loading = FALSE) { return new PinObject;}
		const TCHAR *	ClassName() {return GetString(IDS_AP_PIN_CLASS);}
		SClass_ID		SuperClassID() {return WSM_OBJECT_CLASS_ID; }
		Class_ID		ClassID() {return PIN_CLASS_ID;}
		const TCHAR* 	Category() {return GetString(SPACEWARPS_FOR_DYNAMICS);}
};

static PinClassDesc PinDesc;
ClassDesc* GetPinObjDesc() {return &PinDesc;}

class PinMod;

class PinField : public ForceField //ok
{	public:
		PinObject *obj;
		TimeValue dtsq,dt;
		INode *node;
		int count;
		Matrix3 tm,invtm;
		Interval tmValid;
		Point3 force;
		Interval fValid;
		Point3 Force(TimeValue t,const Point3 &pos, const Point3 &vel, int index); //ok
};

class PinMod : public SimpleWSMMod
{	public:				
		PinField force; //ok
		PinMod() {}
		PinMod(INode *node,PinObject *obj);		
		// From Animatable
		void GetClassName(TSTR& s) {s= GetString(IDS_AP_PINMOD);}
		SClass_ID SuperClassID() {return WSM_CLASS_ID;}
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return PINMOD_CLASS_ID;}
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_AP_PINBINDING);}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
};

class PinModClassDesc:public ClassDesc
{	public:
		int 			IsPublic() { return 0; }
		void *			Create(BOOL loading = FALSE) {return new PinMod;}
		const TCHAR *	ClassName() { return GetString(IDS_AP_PINMOD);}
		SClass_ID		SuperClassID() {return WSM_CLASS_ID;}
		Class_ID		ClassID() {return PINMOD_CLASS_ID;}
		const TCHAR* 	Category() {return _T("");}
};

static PinModClassDesc PinModDesc;

ClassDesc* GetPinModDesc() {return &PinModDesc;}

class PinModData : public LocalModData
{	public:
		LocalModData *Clone ();
};

LocalModData *PinModData::Clone ()
{	PinModData *clone;
	clone = new PinModData ();
	return(clone);
}

//--- PinObject Parameter map/block descriptors ------------------
#define PB_ONTIME			0
#define PB_OFFTIME			1
#define PB_PINTYPE 			2
#define PB_ICONSIZE			3

static int ConstraintIDs[] = {IDC_AP_CONSTRAINTSURF,IDC_AP_CONSTRAINTAXLE,IDC_AP_CONSTRAINTPIN};

static ParamUIDesc descParamPin[] = {

	// Pin Time On
	ParamUIDesc(
		PB_ONTIME,
		EDITTYPE_TIME,
		IDC_AP_FONTIME,IDC_AP_FONTIMESPIN,
		-999999999.0f,999999999.0f,
		10.0f),
	
	// Pin Time Off
	ParamUIDesc(
		PB_OFFTIME,
		EDITTYPE_TIME,
		IDC_AP_FOFFTIME,IDC_AP_FOFFTIMESPIN,
		-999999999.0f,999999999.0f,
		10.0f),
	
	// Pin Type
	ParamUIDesc(PB_PINTYPE,TYPE_RADIO,ConstraintIDs,3),

	// Icon Size
	ParamUIDesc(
		PB_ICONSIZE,
		EDITTYPE_UNIVERSE,
		IDC_AP_DYNF_ICONSIZE,IDC_AP_DYNF_ICONSIZESPIN,
		0.0f, 9999999.0f,
		SPIN_AUTOSCALE),
	};

#define PINPARAMDESC_LENGTH	4

ParamBlockDescID descPinVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	// PB_ONTIME
	{ TYPE_INT, NULL, FALSE, 1 },	// PB_OFFTIME
	{ TYPE_INT, NULL, FALSE, 1 },	// PB_PINTYPE
	{ TYPE_FLOAT, NULL, TRUE, 17}  // PB_ICONSIZE
};

#define PBLOCK_LENGTH	4

#define CURRENT_VERSION	0


//--- Deflect object methods -----------------------------------------

PinObject::PinObject()
{	int FToTick=(int)((float)TIME_TICKSPERSEC/(float)GetFrameRate());
	MakeRefByID(FOREVER, 0,CreateParameterBlock(descPinVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);	
	pblock->SetValue(PB_ONTIME,0,0);
	pblock->SetValue(PB_OFFTIME,0,FToTick*100);
	pblock->SetValue(PB_PINTYPE,0,0);
}

void PinObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	if (pmapParam)
	{	// Left over
		pmapParam->SetParamBlock(pblock);
	}
	else
	{	hSot = ip->AddRollupPage(hInstance,MAKEINTRESOURCE(IDD_SW_DESC_DYNONLY),DefaultSOTProc,GetString(IDS_AP_TOP),(LPARAM)ip,APPENDROLL_CLOSED);
		// Gotta make a new one.
		pmapParam = CreateCPParamMap(descParamPin,PINPARAMDESC_LENGTH,pblock,ip,hInstance,MAKEINTRESOURCE(IDD_SW_DYNPIN),GetString(IDS_AP_PARAMETERS),0);
	}
//	if (pmapParam) pmapParam->SetUserDlgProc(new PinDlgProc(this));
}

void PinObject::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;
	if (flags&END_EDIT_REMOVEUI )
	{	DestroyCPParamMap(pmapParam);
		ip->DeleteRollupPage(hSot);
		pmapParam = NULL;		
	}	
}

ForceField *PinObject::GetForceField(INode *node)
{	PinField *pb = new PinField;	
	pb->obj  = this;
	pb->node = node;
	pb->tmValid.SetEmpty();
	pb->fValid.SetEmpty();
	pb->dt=GetTicksPerFrame();
	pb->dtsq=pb->dt*pb->dt;
	return pb;
}

void PinObject::MapKeys(TimeMap *map,DWORD flags)
{	TimeValue TempTime;
// mapped values
	pblock->GetValue(PB_ONTIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_ONTIME,0,TempTime);
	pblock->GetValue(PB_OFFTIME,0,TempTime,FOREVER);
	TempTime=map->map(TempTime);
	pblock->SetValue(PB_OFFTIME,0,TempTime);
}

#define NUM_SEGS 16

#define PLANECONSTRAINT 0
#define AXLECONSTRAINT	1
#define PINCONSTRAINT	2

void PinObject::BuildMesh(TimeValue t)
{	ivalid = FOREVER;
	int kindofpin;
	pblock->GetValue(PB_PINTYPE,t,kindofpin,ivalid);
	float iconsize;
	pblock->GetValue(PB_ICONSIZE,t,iconsize,ivalid);

	if (kindofpin == PLANECONSTRAINT)
	{	float rad, u, rcosu, rsinu, halfside;
		int iloop, loopoffset1, loopoffset2, loopoffset3;
		int v1, v2, v3;
		loopoffset1 = NUM_SEGS;
		loopoffset2 = loopoffset1 + NUM_SEGS;
		loopoffset3 = loopoffset2 + NUM_SEGS;
		rad = 0.4f*iconsize;
		halfside = 0.5f * iconsize;
		mesh.setNumVerts(3*NUM_SEGS+1+8);
		mesh.setNumFaces(3*NUM_SEGS+4);
		for (iloop = 0; iloop<NUM_SEGS; iloop++)
		{	u = (float(iloop)/float(NUM_SEGS))*TWOPI;
			rcosu = rad*(float)cos(u);
			rsinu = rad*(float)sin(u);
			mesh.setVert(iloop,              Point3(rcosu,rsinu, 0.0f));
			mesh.setVert(iloop + loopoffset1,Point3(0.0f ,rcosu,rsinu));
			mesh.setVert(iloop + loopoffset2,Point3(rcosu, 0.0f,rsinu));
		}
		mesh.setVert(loopoffset3, Zero);
		mesh.setVert(loopoffset3+1,Point3( halfside, halfside, rad));
		mesh.setVert(loopoffset3+2,Point3(-halfside, halfside, rad));
		mesh.setVert(loopoffset3+3,Point3(-halfside,-halfside, rad));
		mesh.setVert(loopoffset3+4,Point3( halfside,-halfside, rad));
		mesh.setVert(loopoffset3+5,Point3( halfside, halfside,-rad));
		mesh.setVert(loopoffset3+6,Point3(-halfside, halfside,-rad));
		mesh.setVert(loopoffset3+7,Point3(-halfside,-halfside,-rad));
		mesh.setVert(loopoffset3+8,Point3( halfside,-halfside,-rad));

		v3 = loopoffset3;
		for (iloop = 0; iloop<NUM_SEGS; iloop++)
		{	v1 = iloop;
			v2 = (iloop>(NUM_SEGS-2)?iloop-NUM_SEGS+1:iloop+1);
			mesh.faces[iloop].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop].setSmGroup(0);
			mesh.faces[iloop].setVerts(v1,v2,v3);

			mesh.faces[iloop+loopoffset1].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop+loopoffset1].setSmGroup(0);
			mesh.faces[iloop+loopoffset1].setVerts(v1+loopoffset1,v2+loopoffset1,v3);

			mesh.faces[iloop+loopoffset2].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop+loopoffset2].setSmGroup(0);
			mesh.faces[iloop+loopoffset2].setVerts(v1+loopoffset2,v2+loopoffset2,v3);
		}
		mesh.faces[loopoffset3  ].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3  ].setSmGroup(0);
		mesh.faces[loopoffset3  ].setVerts(loopoffset3+1,loopoffset3+2,loopoffset3+3);
		mesh.faces[loopoffset3+1].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+1].setSmGroup(0);
		mesh.faces[loopoffset3+1].setVerts(loopoffset3+3,loopoffset3+4,loopoffset3+1);

		mesh.faces[loopoffset3+2].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+2].setSmGroup(0);
		mesh.faces[loopoffset3+2].setVerts(loopoffset3+5,loopoffset3+6,loopoffset3+7);
		mesh.faces[loopoffset3+3].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+3].setSmGroup(0);
		mesh.faces[loopoffset3+3].setVerts(loopoffset3+7,loopoffset3+8,loopoffset3+5);
	}
	else if (kindofpin == AXLECONSTRAINT)
	{	float rad, u, rcosu, rsinu, halfside;
		int iloop, loopoffset1, loopoffset2, loopoffset3;
		int v1, v2, v3;
		loopoffset1 = NUM_SEGS;
		loopoffset2 = loopoffset1 + NUM_SEGS;
		loopoffset3 = loopoffset2 + NUM_SEGS;
		rad = 0.4f*iconsize;
		halfside = 0.5f * iconsize;
		mesh.setNumVerts(3*NUM_SEGS+1+16);
		mesh.setNumFaces(3*NUM_SEGS+8);
		for (iloop = 0; iloop<NUM_SEGS; iloop++)
		{	u = (float(iloop)/float(NUM_SEGS))*TWOPI;
			rcosu = rad*(float)cos(u);
			rsinu = rad*(float)sin(u);
			mesh.setVert(iloop,              Point3(rcosu,rsinu, 0.0f));
			mesh.setVert(iloop + loopoffset1,Point3(0.0f ,rcosu,rsinu));
			mesh.setVert(iloop + loopoffset2,Point3(rcosu, 0.0f,rsinu));
		}
		mesh.setVert(loopoffset3, Zero);

		mesh.setVert(loopoffset3+1,Point3( halfside, halfside, rad));
		mesh.setVert(loopoffset3+2,Point3(-halfside, halfside, rad));
		mesh.setVert(loopoffset3+3,Point3(-halfside,-halfside, rad));
		mesh.setVert(loopoffset3+4,Point3( halfside,-halfside, rad));
		mesh.setVert(loopoffset3+5,Point3( halfside, halfside,-rad));
		mesh.setVert(loopoffset3+6,Point3(-halfside, halfside,-rad));
		mesh.setVert(loopoffset3+7,Point3(-halfside,-halfside,-rad));
		mesh.setVert(loopoffset3+8,Point3( halfside,-halfside,-rad));

		mesh.setVert(loopoffset3+9 ,Point3( halfside, rad, halfside));
		mesh.setVert(loopoffset3+10,Point3(-halfside, rad, halfside));
		mesh.setVert(loopoffset3+11,Point3(-halfside, rad,-halfside));
		mesh.setVert(loopoffset3+12,Point3( halfside, rad,-halfside));
		mesh.setVert(loopoffset3+13,Point3( halfside,-rad, halfside));
		mesh.setVert(loopoffset3+14,Point3(-halfside,-rad, halfside));
		mesh.setVert(loopoffset3+15,Point3(-halfside,-rad,-halfside));
		mesh.setVert(loopoffset3+16,Point3( halfside,-rad,-halfside));

		v3 = loopoffset3;
		for (iloop = 0; iloop<NUM_SEGS; iloop++)
		{	v1 = iloop;
			v2 = (iloop>(NUM_SEGS-2)?iloop-NUM_SEGS+1:iloop+1);
			mesh.faces[iloop].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop].setSmGroup(0);
			mesh.faces[iloop].setVerts(v1,v2,v3);

			mesh.faces[iloop+loopoffset1].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop+loopoffset1].setSmGroup(0);
			mesh.faces[iloop+loopoffset1].setVerts(v1+loopoffset1,v2+loopoffset1,v3);

			mesh.faces[iloop+loopoffset2].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop+loopoffset2].setSmGroup(0);
			mesh.faces[iloop+loopoffset2].setVerts(v1+loopoffset2,v2+loopoffset2,v3);
		}
		mesh.faces[loopoffset3  ].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3  ].setSmGroup(0);
		mesh.faces[loopoffset3  ].setVerts(loopoffset3+1,loopoffset3+2,loopoffset3+3);
		mesh.faces[loopoffset3+1].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+1].setSmGroup(0);
		mesh.faces[loopoffset3+1].setVerts(loopoffset3+3,loopoffset3+4,loopoffset3+1);

		mesh.faces[loopoffset3+2].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+2].setSmGroup(0);
		mesh.faces[loopoffset3+2].setVerts(loopoffset3+5,loopoffset3+6,loopoffset3+7);
		mesh.faces[loopoffset3+3].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+3].setSmGroup(0);
		mesh.faces[loopoffset3+3].setVerts(loopoffset3+7,loopoffset3+8,loopoffset3+5);

		mesh.faces[loopoffset3+4].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+4].setSmGroup(0);
		mesh.faces[loopoffset3+4].setVerts(loopoffset3+ 9,loopoffset3+10,loopoffset3+11);
		mesh.faces[loopoffset3+5].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+5].setSmGroup(0);
		mesh.faces[loopoffset3+5].setVerts(loopoffset3+11,loopoffset3+12,loopoffset3+ 9);

		mesh.faces[loopoffset3+6].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+6].setSmGroup(0);
		mesh.faces[loopoffset3+6].setVerts(loopoffset3+13,loopoffset3+14,loopoffset3+15);
		mesh.faces[loopoffset3+7].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+7].setSmGroup(0);
		mesh.faces[loopoffset3+7].setVerts(loopoffset3+15,loopoffset3+16,loopoffset3+13);

	}
	else if (kindofpin == PINCONSTRAINT)
	{	float rad, u, rcosu, rsinu, halfside;
		int iloop, loopoffset1, loopoffset2, loopoffset3;
		int v1, v2, v3;
		loopoffset1 = NUM_SEGS;
		loopoffset2 = loopoffset1 + NUM_SEGS;
		loopoffset3 = loopoffset2 + NUM_SEGS;
		rad = 0.4f*iconsize;
		halfside = 0.5f * iconsize;
		mesh.setNumVerts(3*NUM_SEGS+1+24);
		mesh.setNumFaces(3*NUM_SEGS+12);
		for (iloop = 0; iloop<NUM_SEGS; iloop++)
		{	u = (float(iloop)/float(NUM_SEGS))*TWOPI;
			rcosu = rad*(float)cos(u);
			rsinu = rad*(float)sin(u);
			mesh.setVert(iloop,              Point3(rcosu,rsinu, 0.0f));
			mesh.setVert(iloop + loopoffset1,Point3(0.0f ,rcosu,rsinu));
			mesh.setVert(iloop + loopoffset2,Point3(rcosu, 0.0f,rsinu));
		}
		mesh.setVert(loopoffset3, Zero);

		mesh.setVert(loopoffset3+1,Point3( halfside, halfside, rad));
		mesh.setVert(loopoffset3+2,Point3(-halfside, halfside, rad));
		mesh.setVert(loopoffset3+3,Point3(-halfside,-halfside, rad));
		mesh.setVert(loopoffset3+4,Point3( halfside,-halfside, rad));
		mesh.setVert(loopoffset3+5,Point3( halfside, halfside,-rad));
		mesh.setVert(loopoffset3+6,Point3(-halfside, halfside,-rad));
		mesh.setVert(loopoffset3+7,Point3(-halfside,-halfside,-rad));
		mesh.setVert(loopoffset3+8,Point3( halfside,-halfside,-rad));

		mesh.setVert(loopoffset3+9 ,Point3( halfside, rad, halfside));
		mesh.setVert(loopoffset3+10,Point3(-halfside, rad, halfside));
		mesh.setVert(loopoffset3+11,Point3(-halfside, rad,-halfside));
		mesh.setVert(loopoffset3+12,Point3( halfside, rad,-halfside));
		mesh.setVert(loopoffset3+13,Point3( halfside,-rad, halfside));
		mesh.setVert(loopoffset3+14,Point3(-halfside,-rad, halfside));
		mesh.setVert(loopoffset3+15,Point3(-halfside,-rad,-halfside));
		mesh.setVert(loopoffset3+16,Point3( halfside,-rad,-halfside));

		mesh.setVert(loopoffset3+17,Point3( rad, halfside, halfside));
		mesh.setVert(loopoffset3+18,Point3( rad,-halfside, halfside));
		mesh.setVert(loopoffset3+19,Point3( rad,-halfside,-halfside));
		mesh.setVert(loopoffset3+20,Point3( rad, halfside,-halfside));
		mesh.setVert(loopoffset3+21,Point3(-rad, halfside, halfside));
		mesh.setVert(loopoffset3+22,Point3(-rad,-halfside, halfside));
		mesh.setVert(loopoffset3+23,Point3(-rad,-halfside,-halfside));
		mesh.setVert(loopoffset3+24,Point3(-rad, halfside,-halfside));

		v3 = loopoffset3;
		for (iloop = 0; iloop<NUM_SEGS; iloop++)
		{	v1 = iloop;
			v2 = (iloop>(NUM_SEGS-2)?iloop-NUM_SEGS+1:iloop+1);
			mesh.faces[iloop].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop].setSmGroup(0);
			mesh.faces[iloop].setVerts(v1,v2,v3);

			mesh.faces[iloop+loopoffset1].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop+loopoffset1].setSmGroup(0);
			mesh.faces[iloop+loopoffset1].setVerts(v1+loopoffset1,v2+loopoffset1,v3);

			mesh.faces[iloop+loopoffset2].setEdgeVisFlags(1,0,0);
			mesh.faces[iloop+loopoffset2].setSmGroup(0);
			mesh.faces[iloop+loopoffset2].setVerts(v1+loopoffset2,v2+loopoffset2,v3);
		}
		mesh.faces[loopoffset3  ].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3  ].setSmGroup(0);
		mesh.faces[loopoffset3  ].setVerts(loopoffset3+1,loopoffset3+2,loopoffset3+3);
		mesh.faces[loopoffset3+1].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+1].setSmGroup(0);
		mesh.faces[loopoffset3+1].setVerts(loopoffset3+3,loopoffset3+4,loopoffset3+1);

		mesh.faces[loopoffset3+2].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+2].setSmGroup(0);
		mesh.faces[loopoffset3+2].setVerts(loopoffset3+5,loopoffset3+6,loopoffset3+7);
		mesh.faces[loopoffset3+3].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+3].setSmGroup(0);
		mesh.faces[loopoffset3+3].setVerts(loopoffset3+7,loopoffset3+8,loopoffset3+5);

		mesh.faces[loopoffset3+4].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+4].setSmGroup(0);
		mesh.faces[loopoffset3+4].setVerts(loopoffset3+ 9,loopoffset3+10,loopoffset3+11);
		mesh.faces[loopoffset3+5].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+5].setSmGroup(0);
		mesh.faces[loopoffset3+5].setVerts(loopoffset3+11,loopoffset3+12,loopoffset3+ 9);

		mesh.faces[loopoffset3+6].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+6].setSmGroup(0);
		mesh.faces[loopoffset3+6].setVerts(loopoffset3+13,loopoffset3+14,loopoffset3+15);
		mesh.faces[loopoffset3+7].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+7].setSmGroup(0);
		mesh.faces[loopoffset3+7].setVerts(loopoffset3+15,loopoffset3+16,loopoffset3+13);

		mesh.faces[loopoffset3+8].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+8].setSmGroup(0);
		mesh.faces[loopoffset3+8].setVerts(loopoffset3+17,loopoffset3+18,loopoffset3+19);
		mesh.faces[loopoffset3+9].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+9].setSmGroup(0);
		mesh.faces[loopoffset3+9].setVerts(loopoffset3+19,loopoffset3+20,loopoffset3+17);

		mesh.faces[loopoffset3+10].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+10].setSmGroup(0);
		mesh.faces[loopoffset3+10].setVerts(loopoffset3+21,loopoffset3+22,loopoffset3+23);
		mesh.faces[loopoffset3+11].setEdgeVisFlags(1,1,0);
		mesh.faces[loopoffset3+11].setSmGroup(0);
		mesh.faces[loopoffset3+11].setVerts(loopoffset3+23,loopoffset3+24,loopoffset3+21);

	}
	else
	{	mesh.setNumVerts(0);
		mesh.setNumFaces(0);
	}
	mesh.InvalidateGeomCache();
}

class PinObjCreateCallback : public CreateMouseCallBack
{	public:
		PinObject *ob;	
		Point3 p0,p1;
		IPoint2 sp0;
		int proc(ViewExp *vpt,int msg,int point,int flags,IPoint2 m,Matrix3& mat);
};

int PinObjCreateCallback::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
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

static PinObjCreateCallback PinCreateCB;

CreateMouseCallBack* PinObject::GetCreateMouseCallBack()
{	PinCreateCB.ob = this;
	return &PinCreateCB;
}

void PinObject::InvalidateUI() 
{	if (pmapParam) pmapParam->Invalidate();}

Modifier *PinObject::CreateWSMMod(INode *node)
{	return new PinMod(node,this);}

RefTargetHandle PinObject::Clone(RemapDir& remap) 
{	PinObject* newob = new PinObject();
	newob->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newob, remap);
	return newob;
}

ParamDimension *PinObject::GetParameterDim(int pbIndex) 
{	switch (pbIndex)
	{	case PB_ONTIME:		return stdTimeDim;
		case PB_OFFTIME:	return stdTimeDim;
		case PB_ICONSIZE:	return stdWorldDim;
		default: return defaultDim;
	}
}

TSTR PinObject::GetParameterName(int pbIndex) 
{	switch (pbIndex)
	{	case PB_ONTIME:		return GetString(IDS_AP_ONTIME);
		case PB_OFFTIME:	return GetString(IDS_AP_OFFTIME);
		case PB_PINTYPE:	return GetString(IDS_AP_PINTYPE);
		case PB_ICONSIZE:	return GetString(IDS_AP_ICONSIZE);
		default: 			return TSTR(_T(""));
	}
}

ParamUIDesc *PinObject::UIDesc()
{	return descParamPin;}

int PinObject::UIDescLength()
{	return PINPARAMDESC_LENGTH;}

TSTR PinObject::UIStrName()
{   return GetString(IDS_AP_PINPARAM);}


PinMod::PinMod(INode *node,PinObject *obj)
{	//MakeRefByID(FOREVER,SIMPWSMMOD_OBREF,obj);
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);
	pblock = NULL;
	obRef = NULL;
	}

Interval PinMod::GetValidity(TimeValue t) 
{	if (nodeRef)
	{	Interval valid=FOREVER;
		Matrix3 tm;
		tm=nodeRef->GetObjectTM(t,&valid);
		return valid;
	}
	else
	{	return FOREVER;
	}
}

class PinDeformer : public Deformer
{	public:		
	Point3 Map(int i, Point3 p) {return p;}
};
static PinDeformer gdeformer;

Deformer& PinMod::GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{	return gdeformer;}

RefTargetHandle PinMod::Clone(RemapDir& remap) 
{	PinMod *newob = new PinMod(nodeRef,(PinObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
}

void PinMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj)
	{	force.obj=(PinObject*)GetWSMObject(t);
		force.node=nodeRef;
		force.tmValid.SetEmpty();
		force.fValid.SetEmpty();
		force.dt=GetTicksPerFrame();
		force.dtsq=force.dt*force.dt;
		obj->ApplyForceField(&force); //ok
	}
}

Point3 PinField::Force(TimeValue t,const Point3 &pos, const Point3 &vel,int index) //ok
{	fValid= FOREVER;		
	if (!tmValid.InInterval(t)) 
	{	tmValid=FOREVER;
		tm=node->GetObjectTM(t,&tmValid);
		invtm=Inverse(tm);
	}
	fValid&=tmValid;
	TimeValue t1,t2;
	obj->pblock->GetValue(PB_ONTIME,t,t1,fValid);
	obj->pblock->GetValue(PB_OFFTIME,t,t2,fValid);
	Point3 OutPin;
	if ((t>=t1)&&(t<=t2))
		OutPin = Zero;
	else 
		OutPin = Zero;
	return OutPin*6.25e-03f;
}

