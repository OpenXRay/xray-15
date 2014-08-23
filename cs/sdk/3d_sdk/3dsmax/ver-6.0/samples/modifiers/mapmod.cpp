/**********************************************************************
 *<
	FILE: mapmod.cpp

	DESCRIPTION:  A UVW mapping modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/21/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"
#include "iparamm.h"
#include "texutil.h"
#include "bmmlib.h"
#include "simpobj.h"
#include "simpmod.h"
#include "decomp.h"
#include "mapping.h"
#include "buildver.h"
#include "surf_api.h"
#include "MaxIcon.h"
#include "modsres.h"
#include "resourceOverride.h"


// mjm - begin - 4.21.99
#define SEL_NONE		 0
#define SEL_NURBS		(1<<0)
#define SEL_OTHERS		(1<<1)
// mjm - end

#define MAP_XYZTOUVW	6

class MapMod : public MappingMod {	
	public:
		IParamBlock *pblock;
				
		static IParamMap *pmapParam;
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;		
		static FaceAlignMode *faceAlignMode;
		static RegionFitMode *regionFitMode;
		static PickAcquire pickAcquire;
		static MapMod *editMod;

		MapMod(BOOL create);

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) {s=GetString(IDS_RB_MAPMOD);}
		virtual Class_ID ClassID() {return Class_ID(UVWMAPOSM_CLASS_ID,0);}
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);		
		TCHAR *GetObjectName() { return GetString(IDS_RB_UVWMAPPING); }
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		BOOL AssignController(Animatable *control,int subAnim);
		int SubNumToRefNum(int subNum);

		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR;}
		ChannelMask ChannelsChanged() {return TEXMAP_CHANNEL|PART_VERTCOLOR; }		
		Class_ID InputType() {return mapObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);
		Interval GetValidity(TimeValue t);

		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		int NumRefs() {return 2;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		int NumSubs() {return 2;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		
		void ActivateSubobjSel(int level, XFormModes& modes);

        UINT GetModObjTypes(); // mjm - 4.21.99

		// From MappingMod
		void EnterNormalAlign();
		void ExitNormalAlign();
		void EnterRegionFit();
		void ExitRegionFit();
		int GetMapType();
		void SetMapType(int type);
		float GetTile(TimeValue t,int which);
		void SetTile(TimeValue t,int which, float tile);
		BOOL GetFlip(int which);
		void SetFlip(int which,BOOL flip);
		void EnterAcquire();
		void ExitAcquire();
		float GetLength(TimeValue t);
		float GetWidth(TimeValue t);
		float GetHeight(TimeValue t);
		int GetAxis();
		void SetLength(TimeValue t,float v);
		void SetWidth(TimeValue t,float v);
		void SetHeight(TimeValue t,float v);
		void SetAxis(int v);
		int GetFirstParamVer() {return 3;}
		int GetPBlockVersion() {return pblock->GetVersion();}
		
		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

	};


//--- ClassDescriptor and class vars ---------------------------------

IObjParam*          MappingMod::ip        = NULL;

IParamMap*          MapMod::pmapParam     = NULL;
MoveModBoxCMode*    MapMod::moveMode      = NULL;
RotateModBoxCMode*  MapMod::rotMode       = NULL;
UScaleModBoxCMode*  MapMod::uscaleMode    = NULL;
NUScaleModBoxCMode* MapMod::nuscaleMode   = NULL;
SquashModBoxCMode*  MapMod::squashMode    = NULL;
FaceAlignMode*      MapMod::faceAlignMode = NULL;
RegionFitMode*      MapMod::regionFitMode = NULL;
PickAcquire         MapMod::pickAcquire;
MapMod*             MapMod::editMod       = NULL;

class UVWMapClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new MapMod(!loading);}
	const TCHAR *	ClassName() { return GetString(IDS_RB_UVWMAP_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(UVWMAPOSM_CLASS_ID,0); }
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFSURFACE);}
	};

static UVWMapClassDesc mapDesc;
extern ClassDesc* GetUVWMapModDesc() {return &mapDesc;}

static void DoBoxIcon(BOOL sel,float length, PolyLineProc& lp);

static GenSubObjType SOT_Apparatus(14);


//--- Parameter map/block descriptors -------------------------------

#define PB_MAPTYPE		0
#define PB_UTILE		1
#define PB_VTILE		2
#define PB_WTILE		3
#define PB_UFLIP		4
#define PB_VFLIP		5
#define PB_WFLIP		6
#define PB_CAP			7
#define PB_CHANNEL		8
#define PB_LENGTH		9
#define PB_WIDTH		10
#define PB_HEIGHT		11
#define PB_AXIS			12
#define PB_MAPCHANNEL   13
#define PB_NURBS_SRF    14

//
//
// Parameters



static int mapIDs[]  = {IDC_MAP_PLANAR,IDC_MAP_CYL,IDC_MAP_SPHERE,IDC_MAP_BALL,IDC_MAP_BOX,IDC_MAP_FACE, IDC_MAP_XYZTOUVW};
static int chanIDs[] = {IDC_MAP_TEXMAP, IDC_MAP_VERTCOL};
static int axisIDs[] = {IDC_MAP_X,IDC_MAP_Y,IDC_MAP_Z};

static ParamUIDesc descParam[] = {
	// Map type
	ParamUIDesc(PB_MAPTYPE,TYPE_RADIO,mapIDs,7),
	
	// U Tile
	ParamUIDesc (PB_UTILE, EDITTYPE_FLOAT,
	IDC_MAP_UTILE,IDC_MAP_UTILESPIN,
		-999999999.0f,999999999.0f, 0.01f),

	// V Tile
	ParamUIDesc (PB_VTILE, EDITTYPE_FLOAT,
		IDC_MAP_VTILE,IDC_MAP_VTILESPIN,
		-999999999.0f,999999999.0f, 0.01f),

	// W Tile
	ParamUIDesc (PB_WTILE, EDITTYPE_FLOAT,
		IDC_MAP_WTILE,IDC_MAP_WTILESPIN,
		-999999999.0f,999999999.0f, 0.01f),

	// U Flip
	ParamUIDesc(PB_UFLIP,TYPE_SINGLECHEKBOX,IDC_MAP_UFLIP),

	// V Flip
	ParamUIDesc(PB_VFLIP,TYPE_SINGLECHEKBOX,IDC_MAP_VFLIP),

	// W Flip
	ParamUIDesc(PB_WFLIP,TYPE_SINGLECHEKBOX,IDC_MAP_WFLIP),

	// Cap
	ParamUIDesc(PB_CAP,TYPE_SINGLECHEKBOX,IDC_MAP_CAP),

#ifndef USE_SIMPLIFIED_UVWMAP_UI
	// Channel
	ParamUIDesc(PB_CHANNEL,TYPE_RADIO,chanIDs,2),
#endif

	// Length
	ParamUIDesc (PB_LENGTH, EDITTYPE_UNIVERSE,
		IDC_MAP_LENGTH,IDC_MAP_LENGTHSPIN,
		-999999999.0f,999999999.0f, SPIN_AUTOSCALE),

	// Width
	ParamUIDesc (PB_WIDTH, EDITTYPE_UNIVERSE,
		IDC_MAP_WIDTH,IDC_MAP_WIDTHSPIN,
		-999999999.0f,999999999.0f, SPIN_AUTOSCALE),

	// Height
	ParamUIDesc (PB_HEIGHT, EDITTYPE_UNIVERSE,
		IDC_MAP_HEIGHT,IDC_MAP_HEIGHTSPIN,
		-999999999.0f, 999999999.0f, SPIN_AUTOSCALE),

	// Axis
	ParamUIDesc(PB_AXIS,TYPE_RADIO,axisIDs,3),

#ifndef USE_SIMPLIFIED_UVWMAP_UI
	// Map channel
	ParamUIDesc (PB_MAPCHANNEL, EDITTYPE_POS_INT,
		IDC_MAP_CHAN, IDC_MAP_CHAN_SPIN,
		1, MAX_MESHMAPS-1, SPIN_AUTOSCALE),
#endif
	// NURBS Texture Surface
// This ID doesn't exist - sca 2.17.2000
//	ParamUIDesc(PB_NURBS_SRF,TYPE_SINGLECHEKBOX, IDC_NURBS_TXT_SRF),

};
#ifndef USE_SIMPLIFIED_UVWMAP_UI
#define PARAMDESC_LENGH 14
#else
	#define PARAMDESC_LENGH 12
#endif

static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, PB_MAPTYPE },
	{ TYPE_FLOAT, NULL, TRUE, PB_UTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_VTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_WTILE },
	{ TYPE_INT, NULL, TRUE, PB_UFLIP },
	{ TYPE_INT, NULL, TRUE, PB_VFLIP },
	{ TYPE_INT, NULL, TRUE, PB_WFLIP }
};

static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, PB_MAPTYPE },
	{ TYPE_FLOAT, NULL, TRUE, PB_UTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_VTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_WTILE },
	{ TYPE_INT, NULL, FALSE, PB_UFLIP },
	{ TYPE_INT, NULL, FALSE, PB_VFLIP },
	{ TYPE_INT, NULL, FALSE, PB_WFLIP },
	{ TYPE_INT, NULL, FALSE, PB_CAP }
};

static ParamBlockDescID descVer2[] = {
	{ TYPE_INT, NULL, FALSE, PB_MAPTYPE },
	{ TYPE_FLOAT, NULL, TRUE, PB_UTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_VTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_WTILE },
	{ TYPE_INT, NULL, FALSE, PB_UFLIP },
	{ TYPE_INT, NULL, FALSE, PB_VFLIP },
	{ TYPE_INT, NULL, FALSE, PB_WFLIP },
	{ TYPE_INT, NULL, FALSE, PB_CAP },
	{ TYPE_INT, NULL, FALSE, PB_CHANNEL }
};

static ParamBlockDescID descVer3[] = {
	{ TYPE_INT, NULL, FALSE, PB_MAPTYPE },
	{ TYPE_FLOAT, NULL, TRUE, PB_UTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_VTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_WTILE },
	{ TYPE_INT, NULL, FALSE, PB_UFLIP },
	{ TYPE_INT, NULL, FALSE, PB_VFLIP },
	{ TYPE_INT, NULL, FALSE, PB_WFLIP },
	{ TYPE_INT, NULL, FALSE, PB_CAP },
	{ TYPE_INT, NULL, FALSE, PB_CHANNEL },
	{ TYPE_FLOAT, NULL, TRUE, PB_LENGTH },
	{ TYPE_FLOAT, NULL, TRUE, PB_WIDTH },
	{ TYPE_FLOAT, NULL, TRUE, PB_HEIGHT },
	{ TYPE_INT, NULL, FALSE, PB_AXIS },
};

static ParamBlockDescID descVer4[] = {
	{ TYPE_INT, NULL, FALSE, PB_MAPTYPE },
	{ TYPE_FLOAT, NULL, TRUE, PB_UTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_VTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_WTILE },
	{ TYPE_INT, NULL, FALSE, PB_UFLIP },
	{ TYPE_INT, NULL, FALSE, PB_VFLIP },
	{ TYPE_INT, NULL, FALSE, PB_WFLIP },
	{ TYPE_INT, NULL, FALSE, PB_CAP },
	{ TYPE_INT, NULL, FALSE, PB_CHANNEL },
	{ TYPE_FLOAT, NULL, TRUE, PB_LENGTH },
	{ TYPE_FLOAT, NULL, TRUE, PB_WIDTH },
	{ TYPE_FLOAT, NULL, TRUE, PB_HEIGHT },
	{ TYPE_INT, NULL, FALSE, PB_AXIS },
	{ TYPE_INT, NULL, FALSE, PB_MAPCHANNEL },
};

static ParamBlockDescID descVer5[] = {
	{ TYPE_INT, NULL, FALSE, PB_MAPTYPE },
	{ TYPE_FLOAT, NULL, TRUE, PB_UTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_VTILE },
	{ TYPE_FLOAT, NULL, TRUE, PB_WTILE },
	{ TYPE_INT, NULL, FALSE, PB_UFLIP },
	{ TYPE_INT, NULL, FALSE, PB_VFLIP },
	{ TYPE_INT, NULL, FALSE, PB_WFLIP },
	{ TYPE_INT, NULL, FALSE, PB_CAP },
	{ TYPE_INT, NULL, FALSE, PB_CHANNEL },
	{ TYPE_FLOAT, NULL, TRUE, PB_LENGTH },
	{ TYPE_FLOAT, NULL, TRUE, PB_WIDTH },
	{ TYPE_FLOAT, NULL, TRUE, PB_HEIGHT },
	{ TYPE_INT, NULL, FALSE, PB_AXIS },
	{ TYPE_INT, NULL, FALSE, PB_MAPCHANNEL },
	{ TYPE_INT, NULL, FALSE, PB_NURBS_SRF }		// This parameter doesn't have any UI - it should be removed or replaced with the next real parameter.
};

#define PBLOCK_LENGTH	15


// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,7,0),
	ParamVersionDesc(descVer1,8,1),	
	ParamVersionDesc(descVer2,9,2),	
	ParamVersionDesc (descVer3, 13, 3),
	ParamVersionDesc (descVer4, 14, 4),
};

#define NUM_OLDVERSIONS	5

// Current version
#define CURRENT_VERSION	5
static ParamVersionDesc curVersion(descVer5,PBLOCK_LENGTH,CURRENT_VERSION);


class ModAppEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
			int Count() { return _items.Count(); }
      Tab<ReferenceMaker*> _items;              
	};

// Finds out how many modapps are directly referencing 
// the refmaker passed in as arg.  Once a modapp is found, 
// it doesn't need to continue enumerating that branch of dependents.
int ModAppEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==MODAPP_CLASS_ID)    
			{
        _items.Append(1, &rmaker);                 
				return DEP_ENUM_SKIP;              
			}
     return DEP_ENUM_CONTINUE;              
	}



//--- MapDlgProc -------------------------------------------------------

void MappingMod::ViewportAlign() {
	ViewExp *vpt = ip->GetActiveViewport();
	if (!vpt) return;

	// Get mod contexts and nodes for this modifier
	ModContextList mcList;
	INodeTab nodeList;
	ip->GetModContexts(mcList,nodeList);

	// Viewport tm
	Matrix3 vtm;
	vpt->GetAffineTM(vtm);
	vtm = Inverse(vtm);

	// Node tm
	Matrix3 ntm = nodeList[0]->GetObjectTM(ip->GetTime());	
	ntm.NoScale();

	// MC tm
	Matrix3 mctm(1);
	if (mcList[0]->tm) mctm = *mcList[0]->tm;

	// Compute the new destination transform for tmCont
	Matrix3 destTM = vtm * Inverse(ntm) * mctm;
	destTM.PreRotateZ(PI);
	switch (GetAxis()) {
		case 0:
			destTM.PreRotateY(-HALFPI);
			break;
		case 1:
			destTM.PreRotateX(HALFPI);
			break;
		}

	// Current val of tmCont
	Matrix3 curTM(1);
	tmControl->GetValue(ip->GetTime(),&curTM,FOREVER,CTRL_RELATIVE);
	Point3 s;
	for (int i=0; i<3; i++) s[i] = Length(curTM.GetRow(i));

	// These types are aligned differently
	if (GetMapType()==MAP_CYLINDRICAL ||
		GetMapType()==MAP_SPHERICAL ||
		GetMapType()==MAP_BALL) {
		destTM.PreRotateX(-HALFPI);
		}

	// Keep position and scale the same	
	destTM.SetTrans(curTM.GetTrans());
	destTM.PreScale(s);	

	// Plug-in the new value
	SetXFormPacket pckt(destTM);
	tmControl->SetValue(ip->GetTime(), &pckt);

	nodeList.DisposeTemporary();
	ip->ReleaseViewport(vpt);
	ip->RedrawViews(ip->GetTime());
	}

static void MatrixFromNormal(Point3& normal, Matrix3& mat)
	{
	Point3 vx;
	vx.z = .0f;
	vx.x = -normal.y;
	vx.y = normal.x;	
	if ( vx.x == .0f && vx.y == .0f ) {
		vx.x = 1.0f;
		}
	mat.SetRow(0,vx);
	mat.SetRow(1,normal^vx);
	mat.SetRow(2,normal);
	mat.SetTrans(Point3(0,0,0));
	mat.NoScale();
	}

void FaceAlignMouseProc::FaceAlignMap(HWND hWnd,IPoint2 m)
	{
	ViewExp *vpt = ip->GetViewport(hWnd);
	if (!vpt) return;

	Ray ray, wray;
	float at;
	TimeValue t = ip->GetTime();	
	GeomObject *obj;
	Point3 norm, pt;
	Interval valid;

	// Get mod contexts and nodes for this modifier
	ModContextList mcList;
	INodeTab nodeList;
	ip->GetModContexts(mcList,nodeList);

	// Calculate a ray from the mouse point
	vpt->MapScreenToWorldRay(float(m.x), float(m.y),wray);

	for (int i=0; i<nodeList.Count(); i++) {
		INode *node = nodeList[i];

		// Get the object from the node
		ObjectState os = node->EvalWorldState(t);
		if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
			obj = (GeomObject*)os.obj;
		} else {
			continue;
			}

		// Back transform the ray into object space.		
		Matrix3 obtm  = node->GetObjectTM(t);
		Matrix3 iobtm = Inverse(obtm);
		ray.p   = iobtm * wray.p;
		ray.dir = VectorTransform(iobtm, wray.dir);
	
		// See if we hit the object
		if (obj->IntersectRay(t,ray,at,norm)) {
			// Calculate the hit point
			pt = ray.p + ray.dir * at;
					
			// Get the mod context tm
			Matrix3 tm(1);
			if (mcList[0]->tm) tm = tm * *mcList[0]->tm;
		
			// Transform the point and ray into mod context space
			pt = pt * tm;
			norm = Normalize(VectorTransform(tm,norm));
		
			// Construct the target transformation in mod context space
			Matrix3 destTM;
			MatrixFromNormal(norm,destTM);
			destTM.SetTrans(pt);
			destTM.PreRotateZ(PI);
			switch (mod->GetAxis())
			{
				case 0:
					destTM.PreRotateY(-HALFPI);
					break;
				case 1:
					destTM.PreRotateX(HALFPI);
					break;
			}
			// Our current transformation... gives relative TM
			Matrix3 curTM(1), relTM, id(1);
			mod->tmControl->GetValue(t,&curTM,valid,CTRL_RELATIVE);
			relTM = Inverse(curTM) * destTM;
		
			// Here's the modifications we need to make to get there
			tm.IdentityMatrix();
			tm.SetTrans(curTM.GetTrans());
			AffineParts parts;			
			decomp_affine(relTM,&parts);
			Point3 delta = destTM.GetTrans()-curTM.GetTrans();
			mod->Rotate(t,id,tm,parts.q);
			mod->Move(t,id,id,delta);
			break;
			}
		}

	nodeList.DisposeTemporary();
	ip->ReleaseViewport(vpt);
	}

int FaceAlignMouseProc::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	switch (msg)
	{
		case MOUSE_POINT:
			if (point==0) {				
				theHold.Begin();
				ip->RedrawViews(ip->GetTime(),REDRAW_BEGIN);
			} else {
				theHold.Accept(0);
				ip->RedrawViews(ip->GetTime(),REDRAW_END);
				}
			break;

		case MOUSE_MOVE: {
			theHold.Restore();
			FaceAlignMap(hWnd,m);
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);			
			break;
			}

		case MOUSE_FREEMOVE:			
			SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
			break;

	// mjm - begin - 5.5.99
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;
	// mjm - end

		case MOUSE_ABORT:
			theHold.Cancel();
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			break;
	}
	return TRUE;
}

void FaceAlignMode::EnterMode()
	{
	mod->EnterNormalAlign();	
	}

void FaceAlignMode::ExitMode()
	{
	mod->ExitNormalAlign();	
	}


void RegionFitMouseProc::RegionFitMap(HWND hWnd,IPoint2 m)
	{
	ViewExp *vpt = ip->GetViewport(hWnd);
	if (!vpt) return;

	// Get mod contexts and nodes for this modifier
	ModContextList mcList;
	INodeTab nodeList;
	ip->GetModContexts(mcList,nodeList);

	// Viewport tm
	Matrix3 vtm;
	vpt->GetAffineTM(vtm);
	vtm = Inverse(vtm);

	// Node tm
	Matrix3 ntm = nodeList[0]->GetObjectTM(ip->GetTime());	
	
	// MC tm
	Matrix3 mctm(1);
	if (mcList[0]->tm) mctm = *mcList[0]->tm;

	// Current val of tmCont.. remove any scale
	Matrix3 ctm(1);
	mod->tmControl->GetValue(ip->GetTime(),&ctm,FOREVER,CTRL_RELATIVE);
	AffineParts parts;
	decomp_affine(ctm, &parts);
	parts.q.MakeMatrix(ctm);
	ctm.Translate(parts.t);
	
	// Compute the inverse world space tm for the gizmo
	Matrix3 iwtm = Inverse(ctm * Inverse(mctm) * ntm);
	
	// Calculate a ray from the two mouse points
	Ray mray, omray;
	float at;
	Point3 p1, p2;
	vpt->MapScreenToWorldRay(float(m.x), float(m.y),mray);
	vpt->MapScreenToWorldRay(float(om.x), float(om.y),omray);
	
	// Back transform the rays into gizmo space
	mray.p    = iwtm * mray.p;
	mray.dir  = VectorTransform(iwtm, mray.dir);
	omray.p   = iwtm * omray.p;
	omray.dir = VectorTransform(iwtm, omray.dir);

	float dir, pnt, odir, opnt;
	switch (mod->GetAxis()) {
		case 0:
			dir = mray.dir.x; odir = omray.dir.x;
			pnt = mray.p.x; opnt = omray.p.x;
			break;
		case 1:
			dir = mray.dir.y; odir = omray.dir.y;
			pnt = mray.p.y; opnt = omray.p.y;
			break;
		case 2:
			dir = mray.dir.z; odir = omray.dir.z;
			pnt = mray.p.z; opnt = omray.p.z;
			break;
		}
#define EPSILON	0.001
	// Make sure we're going to hit
	if (fabs(dir)>EPSILON && fabs(odir)>EPSILON) {
	
		// Compute the point of intersection
		at = -pnt/dir;
		p1 = mray.p + at*mray.dir;
		at = -opnt/odir;
		p2 = omray.p + at*omray.dir;
		
		// Center the map in the region
		ctm.PreTranslate((p1+p2)/2.0f);

		// Compute scale factors and scale
		float sx;
		float sy;
		switch (mod->GetAxis()) {
			case 0:
				sx = (float)fabs(p1.z-p2.z);
				sy = (float)fabs(p1.y-p2.y);
				break;
			case 1:
				sx = (float)fabs(p1.x-p2.x);
				sy = (float)fabs(p1.z-p2.z);
				break;
			case 2:
				sx = (float)fabs(p1.x-p2.x);
				sy = (float)fabs(p1.y-p2.y);
				break;
			}
		
		// Scale params instead of the matrix
		TimeValue t = ip->GetTime();		
		mod->SetWidth(t,sx);
		mod->SetLength(t,sy);		
		/*
		if (sx>0.0f && sy>0.0f) {
			ctm.PreScale(Point3(sx,sy,1.0f));
			}
		*/

		// Plug-in the new value		
		SetXFormPacket pckt(ctm);
		mod->tmControl->SetValue(ip->GetTime(), &pckt);		
		}

	nodeList.DisposeTemporary();
	ip->ReleaseViewport(vpt);
	ip->RedrawViews(ip->GetTime());
	}

int RegionFitMouseProc::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	switch (msg)
	{
		case MOUSE_POINT:
			if (point==0) {				
				om = m;
				theHold.Begin();
				ip->RedrawViews(ip->GetTime(),REDRAW_BEGIN);
			} else {
				theHold.Accept(0);
				ip->RedrawViews(ip->GetTime(),REDRAW_END);
				}
			break;

		case MOUSE_MOVE: {
			theHold.Restore();
			RegionFitMap(hWnd,m);
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);			
			break;
			}

		case MOUSE_FREEMOVE:			
			SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
			break;

		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_ABORT:
			theHold.Cancel();
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			break;
	}
	return TRUE;
}

void RegionFitMode::EnterMode()
{
	mod->EnterRegionFit();
}

void RegionFitMode::ExitMode()
{
	mod->ExitRegionFit();	
}


//--- PickAcquire -------------------------------------------------------


static INT_PTR CALLBACK AcquireTypeDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	static int type = IDC_ACQUIRE_REL;

	switch (msg) {
		case WM_INITDIALOG:
			CheckRadioButton(hWnd,IDC_ACQUIRE_REL,IDC_ACQUIRE_ABS,type);
			CenterWindow(hWnd,GetParent(hWnd));
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					if (IsDlgButtonChecked(hWnd,IDC_ACQUIRE_REL)) 
						 type = IDC_ACQUIRE_REL;
					else type = IDC_ACQUIRE_ABS;
					EndDialog(hWnd,type);
					break;

				case IDCANCEL:
					EndDialog(hWnd,-1);
					break;
				}
			break;

		default:
			return FALSE;
		}
	
	return TRUE;
	}

static BOOL GetAcquireType(HWND hWnd,int &type)
	{
	type = DialogBox(
		hInstance,
		MAKEINTRESOURCE(IDD_MAP_ACQUIRE),
		hWnd,
		AcquireTypeDlgProc);
	if (type<0) return FALSE;
	else return TRUE;
	}

MappingMod *PickAcquire::FindFirstMap(ReferenceTarget *ref)
	{
	MappingMod *mod;
	if (mod = GetMappingInterface(ref)) {
		if (!mod->TestAFlag(A_MOD_DISABLED)) return mod;
		}
	
	for (int i=ref->NumRefs()-1; i>=0; i--) {
		ReferenceTarget *cref = ref->GetReference(i);
		if (cref) {
			if (mod = FindFirstMap(cref)) return mod;			
			}
		}
	return NULL;
	}

BOOL PickAcquire::Filter(INode *node)
	{
	MappingMod *amod = FindFirstMap(node->GetObjectRef());
	if (amod!=mod && amod) return TRUE;
	else return FALSE;
	}

BOOL PickAcquire::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{
	INode *node = 
		ip->PickNode(hWnd,m,this);
	return node ? TRUE : FALSE;
	}
	
class PickAcquireSourceMC : public ModContextEnumProc {
	public:
		ModContext *mc;
		PickAcquireSourceMC() {mc = NULL;}
		BOOL proc(ModContext *mc) {
			this->mc = mc;
			return FALSE;
			}
	};

BOOL PickAcquire::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	assert(node);
	MappingMod *amod = FindFirstMap(node->GetObjectRef());
	if (amod) {
		
		// Check for unsupported map types
		if (mod->ClassID()==Class_ID(DISPLACEOSM_CLASS_ID,0) &&
			(amod->GetMapType()!=MAP_PLANAR &&
			 amod->GetMapType()!=MAP_CYLINDRICAL &&
			 amod->GetMapType()!=MAP_SPHERICAL &&
			 amod->GetMapType()!=MAP_BALL)) {

			TSTR buf1 = GetString(IDS_RB_DISPLACE);
			TSTR buf2 = GetString(IDS_RB_UNSUPPORTED_MAP_TYPE);
			MessageBox(ip->GetMAXHWnd(),buf2,buf1,MB_ICONEXCLAMATION|MB_OK);
			
			return TRUE;
			}
		
		int type;
		if (!GetAcquireType(ip->GetMAXHWnd(),type)) return TRUE;

		// Get our node and mod context
		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		assert(nodes.Count());
		
		// Get the source mod's mod context
		PickAcquireSourceMC pasmc;
		amod->EnumModContexts(&pasmc);
		assert(pasmc.mc);

		// Do it!
		theHold.Begin();
		AcquireMapping(mod,mcList[0],nodes[0],amod,pasmc.mc,node,type);
		
		theHold.Accept(GetString (IDS_MM_ACQ_MAPPING));

		nodes.DisposeTemporary();
		}
	return TRUE;
	}

void PickAcquire::EnterMode(IObjParam *ip)
	{
	mod->EnterAcquire();
	}

void PickAcquire::ExitMode(IObjParam *ip)
	{
	mod->ExitAcquire();
	}

void PickAcquire::AcquireMapping(
		MappingMod *toMod, ModContext *toMC, INode *toNode,
		MappingMod *fromMod, ModContext *fromMC, INode *fromNode,
		int type)
	{
	// Build the mats
	Matrix3 fromNTM(1);
	Matrix3 toNTM(1);
	if (type==IDC_ACQUIRE_ABS) {
		fromNTM = fromNode->GetObjectTM(ip->GetTime());	
		toNTM   = toNode->GetObjectTM(ip->GetTime());	
		}
	Matrix3 fromTM  = fromMod->CompMatrix(ip->GetTime(),fromMC,&fromNTM,FALSE,type==IDC_ACQUIRE_ABS);
	Matrix3 destTM  = fromTM * Inverse(toNTM);
	if (toMC->tm) destTM = destTM * (*toMC->tm);
	
	if (type==IDC_ACQUIRE_ABS && toMod->TestAFlag(A_PLUGIN1)) {
		switch (toMod->GetMapType()) {
			case MAP_BOX:
			case MAP_PLANAR:
				destTM.PreRotateZ(-PI);
				break;
			
			case MAP_BALL:
			case MAP_SPHERICAL:
			case MAP_CYLINDRICAL:
				destTM.PreRotateZ(-HALFPI);
				break;
			}		
		}

	// Set the TM
	SetXFormPacket pckt(destTM);
	toMod->tmControl->SetValue(ip->GetTime(),&pckt);
	toMod->SetMapType(fromMod->GetMapType());
	TimeValue t = ip->GetTime();
	for (int i=0; i<3; i++) {
		toMod->SetTile(t, i, fromMod->GetTile(t,i));
		toMod->SetFlip(i, fromMod->GetFlip(i));
		}	
	
	// Copy length/width/height
	toMod->SetLength(t,fromMod->GetLength(t));
	toMod->SetWidth(t,fromMod->GetWidth(t));
	toMod->SetHeight(t,fromMod->GetHeight(t));
	if (type!=IDC_ACQUIRE_ABS) {
		toMod->SetAxis(fromMod->GetAxis());
	} else {
		toMod->SetAxis(2);
		}

	ip->RedrawViews(ip->GetTime());
	}


//--- MapDlgProc -----------------------------------------------------

class MapDlgProc : public ParamMapUserDlgProc {
	public:
		ISpinnerControl *iLengthSpin, *iWidthSpin, *iHeightSpin, *iMapChanSpin;	// mjm - 2.12.99
		MapMod *mod;
		UINT modObjTypes;
		
		MapDlgProc(MapMod *m) { mod = m; modObjTypes = SEL_NONE; }		
		void EnableMappingSubCtrls(HWND hWnd);	// mjm - 2.12.99
		void EnableChannelSubCtrls(HWND hWnd);	// mjm - 2.12.99
		void EnableNURBSButton(HWND hWnd, UINT modObjTypes);	// mjm - 4.21.99
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void DoBitmapFit(HWND hWnd);
	};

void MapDlgProc::DoBitmapFit(HWND hWnd)
	{
	BitmapInfo bi;
	TheManager->SelectFileInputEx(
		&bi, hWnd, GetString(IDS_RB_SELECTIMAGE));
	if (bi.Name()[0]) {		
		TheManager->GetImageInfo(&bi);
		mod->aspect = bi.Aspect() * float(bi.Width())/float(bi.Height());
		mod->flags |= CONTROL_ASPECT|CONTROL_HOLD;
		mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
		}
	}

// mjm - begin - 2.12.99
void MapDlgProc::EnableMappingSubCtrls(HWND hWnd)
{
	BOOL lengthState(TRUE), widthState(TRUE), heightState(TRUE), flipState(TRUE), capState(FALSE);
	int type = mod->GetMapType();
	switch (type)
	{
		case MAP_CYLINDRICAL:
			capState = TRUE;
			break;
		case MAP_XYZTOUVW:
			flipState = FALSE;
		case MAP_FACE:
			lengthState = widthState = FALSE;
			// fall through
		case MAP_PLANAR:
			heightState = FALSE;
			break;
		case MAP_SPHERICAL:
		case MAP_BALL:
		case MAP_BOX:
			break;
	}
	iLengthSpin->Enable(lengthState);
	iWidthSpin->Enable(widthState);
	iHeightSpin->Enable(heightState);
    EnableWindow(GetDlgItem(hWnd, IDC_MAP_CAP),   capState);
    EnableWindow(GetDlgItem(hWnd, IDC_MAP_UFLIP), flipState);
    EnableWindow(GetDlgItem(hWnd, IDC_MAP_VFLIP), flipState);
    EnableWindow(GetDlgItem(hWnd, IDC_MAP_WFLIP), flipState);
}

void MapDlgProc::EnableNURBSButton(HWND hWnd, UINT modObjTypes)
{
    BOOL isPlanar = IsDlgButtonChecked(hWnd, IDC_MAP_PLANAR);
// This ID doesn't exist - sca 2.17.2000
//	EnableWindow(GetDlgItem(hWnd, IDC_NURBS_TXT_SRF), isPlanar && !(modObjTypes & SEL_OTHERS));

//watje this should be false otherwise the controls will always be disabled 3/7/00
    BOOL useTexSurf = FALSE;//IsDlgButtonChecked(hWnd, IDC_NURBS_TXT_SRF);

    EnableWindow(GetDlgItem(hWnd, IDC_MAP_SPHERE),   !useTexSurf);
    EnableWindow(GetDlgItem(hWnd, IDC_MAP_BALL),     !useTexSurf);
    EnableWindow(GetDlgItem(hWnd, IDC_MAP_CYL),      !useTexSurf);
    EnableWindow(GetDlgItem(hWnd, IDC_MAP_BOX),      !useTexSurf);
    EnableWindow(GetDlgItem(hWnd, IDC_MAP_FACE),     !useTexSurf);
}

void MapDlgProc::EnableChannelSubCtrls(HWND hWnd)
{
#ifndef USE_SIMPLIFIED_UVWMAP_UI
	BOOL mapChanState = ( IsDlgButtonChecked(hWnd, IDC_MAP_TEXMAP) == BST_CHECKED );
	iMapChanSpin->Enable(mapChanState);
#endif
}
// mjm - end

BOOL MapDlgProc::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:

// mjm - begin - 2.12.99
			iLengthSpin = GetISpinner( GetDlgItem(hWnd, IDC_MAP_LENGTHSPIN) );
			iWidthSpin = GetISpinner( GetDlgItem(hWnd, IDC_MAP_WIDTHSPIN) );
			iHeightSpin = GetISpinner( GetDlgItem(hWnd, IDC_MAP_HEIGHTSPIN) );
#ifndef USE_SIMPLIFIED_UVWMAP_UI
			iMapChanSpin = GetISpinner( GetDlgItem(hWnd, IDC_MAP_CHAN_SPIN) );
// mjm - end
			SendMessage(GetDlgItem(hWnd,IDC_MAP_FITREGION),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SendMessage(GetDlgItem(hWnd,IDC_MAP_FITREGION),CC_COMMAND,CC_CMD_HILITE_COLOR,GREEN_WASH);
			SendMessage(GetDlgItem(hWnd,IDC_MAP_ACQUIRE),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SendMessage(GetDlgItem(hWnd,IDC_MAP_ACQUIRE),CC_COMMAND,CC_CMD_HILITE_COLOR,GREEN_WASH);
#endif // USE_SIMPLIFIED_UVWMAP_UI
			SendMessage(GetDlgItem(hWnd,IDC_MAP_NORMALALIGN),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SendMessage(GetDlgItem(hWnd,IDC_MAP_NORMALALIGN),CC_COMMAND,CC_CMD_HILITE_COLOR,GREEN_WASH);

// mjm - begin - 4.21.99
			// if an instance of this modifier is applied to both nurbs and non-nurbs objects,
			// then neither XYZtoYVW nor the Use Texture Surface options will be available
			// (kind of a least common denominator type thing)
			modObjTypes = mod->GetModObjTypes();
            EnableWindow(GetDlgItem(hWnd, IDC_MAP_XYZTOUVW), !(modObjTypes & SEL_NURBS));
            EnableNURBSButton(hWnd, modObjTypes);
// mjm - end
			EnableMappingSubCtrls(hWnd); // mjm - 2.12.99
			EnableChannelSubCtrls(hWnd); // mjm - 2.12.99
			break;

// mjm - begin - 2.12.99
		case WM_DESTROY:
			ReleaseISpinner(iLengthSpin);
			ReleaseISpinner(iWidthSpin);
			ReleaseISpinner(iHeightSpin);
#ifndef USE_SIMPLIFIED_UVWMAP_UI
			ReleaseISpinner(iMapChanSpin);
#endif
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {

				case IDC_MAP_SPHERE:
				case IDC_MAP_BALL:
					mod->flags |= CONTROL_UNIFORM|CONTROL_HOLD;
					mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
					// fall through

				case IDC_MAP_PLANAR:
				case IDC_MAP_CYL:
				case IDC_MAP_BOX:
				case IDC_MAP_FACE:
				case IDC_MAP_XYZTOUVW:
//				case IDC_NURBS_TXT_SRF:	// This ID doesn't exist - sca 2.17.2000

					// moved handling to NotifyRefChanged() to support changes via MaxScript // mjm - 6.1.99
//					EnableMappingSubCtrls(hWnd);
//					EnableNURBSButton(hWnd, modObjTypes); // mjm - 4.21.99
					break;

				case IDC_MAP_TEXMAP:
				case IDC_MAP_VERTCOL:
					// moved handling to NotifyRefChanged() to support changes via MaxScript // mjm - 6.1.99
//					EnableChannelSubCtrls(hWnd);
					break;
// mjm - end

				case IDC_MAP_FIT:
					mod->flags |= CONTROL_FIT|CONTROL_HOLD;
					mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
					break;

				case IDC_MAP_CENTER:
					mod->flags |= CONTROL_CENTER|CONTROL_HOLD;
					mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
					break;

				case IDC_MAP_BITMAPFIT:
					DoBitmapFit(hWnd);
					break;
				
				case IDC_MAP_ACQUIRE:
					mod->ip->SetPickMode(&mod->pickAcquire);					
					break;

				case IDC_MAP_NORMALALIGN:
					if (mod->ip->GetCommandMode()->ID()==CID_FACEALIGNMAP) {
						mod->ip->SetStdCommandMode(CID_OBJMOVE);
					} else {
						mod->ip->SetCommandMode(mod->faceAlignMode);
						}
					break;

				case IDC_MAP_RESET:
					theHold.Begin();
					mod->ReplaceReference(TM_REF,NULL);
					mod->flags |= CONTROL_FIT|CONTROL_CENTER|CONTROL_INIT|CONTROL_RESET;					
					mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
					mod->ip->RedrawViews(mod->ip->GetTime());
					theHold.Accept(0);
					break;

				case IDC_MAP_VIEWALIGN:
					theHold.Begin();
					mod->ViewportAlign();
					theHold.Accept(0);
					break;

				case IDC_MAP_FITREGION:
					if (mod->ip->GetCommandMode()->ID()==CID_REGIONFIT) {
						mod->ip->SetStdCommandMode(CID_OBJMOVE);
					} else {
						mod->ip->SetCommandMode(mod->regionFitMode);
						}
					break;
				}
			break;
		}
	return FALSE;
	}



//--- MapMod methods ---------------------------------------------------


MapMod::MapMod(BOOL create)
	{
	SetAFlag(A_PLUGIN1);
	if (create) flags = CONTROL_CENTER|CONTROL_FIT|CONTROL_INIT;
	else flags = 0;

	MakeRefByID(FOREVER, PBLOCK_REF, 
		CreateParameterBlock(descVer5, PBLOCK_LENGTH, CURRENT_VERSION));	
	tmControl = NULL;
	mLocalSetValue = true; // mjm - 6.7.99
	pblock->SetValue(PB_UTILE,0,1.0f);
	pblock->SetValue(PB_VTILE,0,1.0f);
	pblock->SetValue(PB_WTILE,0,1.0f);
	pblock->SetValue(PB_WIDTH,0,1.0f);
	pblock->SetValue(PB_LENGTH,0,1.0f);
	pblock->SetValue(PB_HEIGHT,0,1.0f);
	pblock->SetValue(PB_AXIS,0,2);
#ifndef USE_SIMPLIFIED_UVWMAP_UI
	pblock->SetValue (PB_MAPCHANNEL, 0, 1);
#endif
	mLocalSetValue = false; // mjm - 6.7.99
}

#define NEWMAP_CHUNKID	0x0100

// Following is necessary to make sure mapchannel doesn't come in as zero;
// this happens when loading up an old scene -- the pblock with the defaults
// is overwritten.
class SetChannelToOne : public PostLoadCallback {
public:
	MapMod *mm;
	SetChannelToOne (MapMod *mapMod) { mm = mapMod; }
	void proc (ILoad *iload) {
#ifndef USE_SIMPLIFIED_UVWMAP_UI
		if (mm && mm->pblock) {
			int mapChan;
			mm->pblock->GetValue (PB_MAPCHANNEL, 0, mapChan, FOREVER);
			if (!mapChan) mm->pblock->SetValue (PB_MAPCHANNEL, 0, 1);
		}
#endif
		delete this;
	}
};

IOResult MapMod::Load(ILoad *iload) {
	Modifier::Load(iload);

	ClearAFlag(A_PLUGIN1);

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {	
			case NEWMAP_CHUNKID:
				SetAFlag(A_PLUGIN1);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	iload->RegisterPostLoadCallback(
		new FixDimsPLCB(this));

	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,PBLOCK_REF));
	iload->RegisterPostLoadCallback (new SetChannelToOne(this));
		
	return IO_OK;
	}

IOResult MapMod::Save(ISave *isave)
	{
	Modifier::Save(isave);
	if (TestAFlag(A_PLUGIN1)) {
		isave->BeginChunk(NEWMAP_CHUNKID);
		isave->EndChunk();
		}
 	return IO_OK;
	}

void MapMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
{
	this->ip = ip;
	editMod  = this;

	// Add our sub object type
	// TSTR type1( GetString(IDS_RB_APPARATUS));
	// const TCHAR *ptype[] = {type1};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 1);

	// Create sub object editing modes.
	moveMode      = new MoveModBoxCMode(this,ip);
	rotMode       = new RotateModBoxCMode(this,ip);
	uscaleMode    = new UScaleModBoxCMode(this,ip);
	nuscaleMode   = new NUScaleModBoxCMode(this,ip);
	squashMode    = new SquashModBoxCMode(this,ip);	
	faceAlignMode = new FaceAlignMode(this,ip);
	regionFitMode = new RegionFitMode(this,ip);

	pickAcquire.mod = this;
	pickAcquire.ip  = ip;

	pmapParam = CreateCPParamMap (descParam,PARAMDESC_LENGH,
		pblock, ip, hInstance, MAKEINTRESOURCE(IDD_UVWMAPPARAM),
		GetString(IDS_RB_PARAMETERS), 0);		 	
	pmapParam->SetUserDlgProc(new MapDlgProc(this));

 	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
}

void MapMod::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
	{	
	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
 	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->ClearPickMode();
	pickAcquire.mod = NULL;
	pickAcquire.ip  = NULL;	

	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);	
	ip->DeleteMode(faceAlignMode);
	ip->DeleteMode(regionFitMode);
	delete moveMode; moveMode = NULL;
	delete rotMode; rotMode = NULL;
	delete uscaleMode; uscaleMode = NULL;
	delete nuscaleMode; nuscaleMode = NULL;
	delete squashMode; squashMode = NULL;
	delete faceAlignMode; faceAlignMode = NULL;
	delete regionFitMode; regionFitMode = NULL;

	DestroyCPParamMap(pmapParam);
	pmapParam = NULL; // mjm - 6.6.99
	}

void MapMod::EnterNormalAlign()
	{	
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_MAP_NORMALALIGN),CC_COMMAND,CC_CMD_SET_STATE,1);
	}

void MapMod::ExitNormalAlign()
	{	
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_MAP_NORMALALIGN),CC_COMMAND,CC_CMD_SET_STATE,0);
	}

void MapMod::EnterRegionFit()
	{
#ifndef USE_SIMPLIFIED_UVWMAP_UI
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_MAP_FITREGION),CC_COMMAND,CC_CMD_SET_STATE,1);
#endif
	}

void MapMod::ExitRegionFit()
	{
#ifndef USE_SIMPLIFIED_UVWMAP_UI
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_MAP_FITREGION),CC_COMMAND,CC_CMD_SET_STATE,0);
#endif
	}

void MapMod::EnterAcquire()
	{
#ifndef USE_SIMPLIFIED_UVWMAP_UI
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_MAP_ACQUIRE),CC_COMMAND,CC_CMD_SET_STATE,1);
#endif
	}

void MapMod::ExitAcquire()
	{
#ifndef USE_SIMPLIFIED_UVWMAP_UI
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_MAP_ACQUIRE),CC_COMMAND,CC_CMD_SET_STATE,0);
#endif
	}

float MapMod::GetLength(TimeValue t)
	{
	float f;
	pblock->GetValue(PB_LENGTH,t,f,FOREVER);
	return f;
	}

float MapMod::GetWidth(TimeValue t)
	{
	float f;
	pblock->GetValue(PB_WIDTH,t,f,FOREVER);
	return f;
	}

float MapMod::GetHeight(TimeValue t)
	{
	float f;
	pblock->GetValue(PB_HEIGHT,t,f,FOREVER);
	return f;
	}

int MapMod::GetAxis()
	{
	int a;
	pblock->GetValue(PB_AXIS,0,a,FOREVER);
	return a;
	}

void MapMod::SetLength(TimeValue t,float v)
	{
	pblock->SetValue(PB_LENGTH,t,v);
	}

void MapMod::SetWidth(TimeValue t,float v)
	{
	pblock->SetValue(PB_WIDTH,t,v);
	}

void MapMod::SetHeight(TimeValue t,float v)
	{
	pblock->SetValue(PB_HEIGHT,t,v);
	}

void MapMod::SetAxis(int v)
	{
	pblock->SetValue(PB_AXIS,0,v);
	}

int MapMod::GetMapType()
	{
	int type;
	pblock->GetValue(PB_MAPTYPE,0,type,FOREVER);
	return type;
	}

void MapMod::SetMapType(int type)
	{
	pblock->SetValue(PB_MAPTYPE,0,type);
	if (pmapParam) pmapParam->Invalidate();
	}

float MapMod::GetTile(TimeValue t,int which)
	{
	float tile;
	pblock->GetValue(PB_UTILE+which,t,tile,FOREVER);
	return tile;
	}

void MapMod::SetTile(TimeValue t,int which, float tile)
	{
	pblock->SetValue(PB_UTILE+which,t,tile);
	if (pmapParam) pmapParam->Invalidate();
	}

BOOL MapMod::GetFlip(int which)
	{
	int flip;
	pblock->GetValue(PB_UFLIP+which,0,flip,FOREVER);
	return flip;
	}

void MapMod::SetFlip(int which,BOOL flip)
	{
	pblock->SetValue(PB_UFLIP+which,0,flip);
	if (pmapParam) pmapParam->Invalidate();
	}

Interval MapMod::LocalValidity(TimeValue t)
	{	
  // aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
  return GetValidity(t);
	}

//aszabo|feb.06.02 - When LocalValidity is called by ModifyObject,
// it returns NEVER and thus the object channels are marked non valid
// As a result, the mod stack enters and infinite evaluation of the modifier
// ModifyObject now calls GetValidity and CORE calls LocalValidity to
// allow for building a cache on the input of this modifier when it's 
// being edited 
Interval MapMod::GetValidity(TimeValue t)
{
	int i;
	float f;
	Interval valid = FOREVER;
	pblock->GetValue(PB_MAPTYPE,t,i,valid);		
	pblock->GetValue(PB_UTILE,t,f,valid);
	pblock->GetValue(PB_VTILE,t,f,valid);
	pblock->GetValue(PB_WTILE,t,f,valid);
	pblock->GetValue(PB_UFLIP,t,i,valid);
	pblock->GetValue(PB_VFLIP,t,i,valid);
	pblock->GetValue(PB_WFLIP,t,i,valid);
	pblock->GetValue(PB_LENGTH,t,f,valid);
	pblock->GetValue(PB_WIDTH,t,f,valid);
	pblock->GetValue(PB_HEIGHT,t,f,valid);
	pblock->GetValue(PB_AXIS,t,i,valid);
	if (tmControl) {
		Matrix3 tm(1);
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
		}
	return valid;
}

// mjm - begin - 4.21.99
class ModObjEnumProc : public DependentEnumProc
{
private:
	INodeTab m_nodes;

public:
	const INodeTab& GetNodes() { return m_nodes; }
	virtual int proc(ReferenceMaker *rmaker);
};

int ModObjEnumProc::proc(ReferenceMaker *rmaker)
{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)
		m_nodes.Append(1, (INode **)&rmaker);
	return 0;
}

UINT MapMod::GetModObjTypes()
{
	ModObjEnumProc dep;
	EnumDependents(&dep);

	UINT type = SEL_NONE;
	for (int i = 0; i < dep.GetNodes().Count(); i++)
	{
        ObjectState os = dep.GetNodes()[i]->EvalWorldState(ip->GetTime());
        if (os.obj->ClassID() == EDITABLE_SURF_CLASS_ID)
            type |= SEL_NURBS;
		else
            type |= SEL_OTHERS;
    }

    return type;
}
// mjm - end

RefTargetHandle MapMod::Clone(RemapDir& remap) 
	{
	MapMod* newmod = new MapMod(FALSE);	
	if (TestAFlag(A_PLUGIN1)) 
		 newmod->SetAFlag(A_PLUGIN1);
	else newmod->ClearAFlag(A_PLUGIN1); 
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));	
	if (tmControl) {
		newmod->ReplaceReference(TM_REF,tmControl->Clone(remap));
		}
	BaseClone(this, newmod, remap);
	return newmod;
	}

BOOL MapMod::AssignController(Animatable *control,int subAnim)
	{
	if (subAnim==TM_REF) {
		ReplaceReference(TM_REF,(ReferenceTarget*)control);
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		return TRUE;
	} else {
		return FALSE;
		}
	}

int MapMod::SubNumToRefNum(int subNum)
	{
	if (subNum==TM_REF)
		 return TM_REF;
	else return -1;
	}

RefTargetHandle MapMod::GetReference(int i)
	{
	switch (i) {
		case TM_REF: 		return tmControl;
		case PBLOCK_REF:	return pblock;
		default: 			return NULL;
		}
	}

void MapMod::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case TM_REF: 		tmControl = (Control*)rtarg; break;
		case PBLOCK_REF:	pblock = (IParamBlock*)rtarg; break;
		}
	}

Animatable* MapMod::SubAnim(int i)
	{
	switch (i) {
		case TM_REF: 		return tmControl;
		case PBLOCK_REF:	return pblock;
		default: 			return NULL;   
		}
	}

TSTR MapMod::SubAnimName(int i)
	{
	switch (i) {
		case TM_REF: 		return GetString(IDS_RB_GIZMO);
		case PBLOCK_REF:	return GetString(IDS_RB_PARAMETERS);
		default: 			return TSTR(_T(""));
		}
	}

RefResult MapMod::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
{
	switch (message)
	{
		case REFMSG_CHANGE:
		{
// mjm - begin - 6.1.99 - accomodating maxscript
			if (hTarget == pblock) // mjm - 6.6.99 - duh ...
			{
				int pid = pblock->LastNotifyParamNum();
				if (editMod==this && pmapParam)
				{
					// if ui is up, adjust related ui controls
					// moved here from dlgproc to accomodate maxscript
					switch ( pid )
					{
						MapDlgProc *proc;
#ifndef USE_SIMPLIFIED_UVWMAP_UI
						case PB_CHANNEL:
							if ( proc = (MapDlgProc*)pmapParam->GetUserDlgProc() )
								proc->EnableChannelSubCtrls( pmapParam->GetHWnd() );
							break;
#endif
						case PB_MAPTYPE:
						case PB_NURBS_SRF:
							if ( proc = (MapDlgProc*)pmapParam->GetUserDlgProc() )
							{
								proc->EnableMappingSubCtrls( pmapParam->GetHWnd() );
								proc->EnableNURBSButton( pmapParam->GetHWnd(), GetModObjTypes() );
							}
							break;
					}
					pmapParam->Invalidate();
				}
				else if (
					( flags == (CONTROL_CENTER|CONTROL_FIT|CONTROL_INIT) ) &&
					( pid == PB_LENGTH || pid == PB_WIDTH || pid == PB_HEIGHT ) &&
					( !mLocalSetValue ) // mjm - 6.7.99
					)
				{
					// temp fix for maxscript ... overrides default fitting of gizmo on first instance of modifier evaluation
					// flags are set to CONTROL_CENTER|CONTROL_FIT|CONTROL_INIT in constructor and evaluated in InitControl(). If CONTROL_FIT is set
					// the gizmo is sized to the bounding box and will overwrite values l,w,h previously set through maxscript. This code sees if
					// a l,w, or h has been explicitly set outside of modify panel. If so, it disables default bounding box fitting.
					flags &= ~CONTROL_FIT;
				}
			}
			break;
// mjm - end
		}

		case REFMSG_GET_PARAM_DIM:
		{
			GetParamDim *gpd = (GetParamDim*)partID;			
			switch (gpd->index)
			{
				case PB_LENGTH:
				case PB_WIDTH:
				case PB_HEIGHT:
					gpd->dim = stdWorldDim;
					break;
				default:
					gpd->dim = defaultDim;
					break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME:
		{
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index)
			{				
				case PB_UTILE:
					gpn->name = GetString(IDS_RB_UTILE);
					break;
				case PB_VTILE:
					gpn->name = GetString(IDS_RB_VTILE);
					break;
				case PB_WTILE:
					gpn->name = GetString(IDS_RB_WTILE);
					break;
				case PB_LENGTH:
					gpn->name = GetString(IDS_RB_LENGTH);
					break;
				case PB_WIDTH:
					gpn->name = GetString(IDS_RB_WIDTH);
					break;
				case PB_HEIGHT:
					gpn->name = GetString(IDS_RB_HEIGHT);
					break;
				default:
					gpn->name = TSTR(_T(""));
					break;
			}
			return REF_STOP;
		}
	}
	return REF_SUCCEED;
}
	

void MapMod::ActivateSubobjSel(int level, XFormModes& modes )
{
	switch (level)
	{
		case 1: // Modifier box
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,NULL);
			break;
	}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
}


int MapMod::NumSubObjTypes() 
{ 
	return 1;
}

ISubObjType *MapMod::GetSubObjType(int i) 
{	
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Apparatus.SetName(GetString(IDS_RB_APPARATUS));
	}

	switch(i)
	{
	case 0:
		return &SOT_Apparatus;
	}

	return NULL;
}


// --- Modify Object ---------------------------------------------------


void MapMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	// If it's not a mappable object then we can't help
	Object *obj = os->obj;
	if (!obj->IsMappable()) return;

	// Get pblock values	
	int type, uflip, vflip, wflip, cap, channel, mapChannel;
	float utile, vtile, wtile;
    //int nurbsTxtSrf;
	pblock->GetValue(PB_MAPTYPE,t,type,FOREVER);
	pblock->GetValue(PB_UTILE,t,utile,FOREVER);
	pblock->GetValue(PB_VTILE,t,vtile,FOREVER);
	pblock->GetValue(PB_WTILE,t,wtile,FOREVER);
	pblock->GetValue(PB_UFLIP,t,uflip,FOREVER);
	pblock->GetValue(PB_VFLIP,t,vflip,FOREVER);
	pblock->GetValue(PB_WFLIP,t,wflip,FOREVER);
	pblock->GetValue(PB_CAP,t,cap,FOREVER);
#ifndef USE_SIMPLIFIED_UVWMAP_UI
	pblock->GetValue(PB_CHANNEL,t,channel,FOREVER);
	pblock->GetValue (PB_MAPCHANNEL, t, mapChannel, FOREVER);
	//pblock->GetValue (PB_NURBS_SRF, t, nurbsTxtSrf, FOREVER);
	
	mapChannel = channel ? 0 : mapChannel;

	// If object can't handle this map channel, we're done:
	if (obj->NumMapChannels() <= mapChannel) return;
#else
	channel = 1;
	mapChannel = 1;
#endif	// USE_SIMPLIFIED_UVWMAP_UI

	// Prepare the controller and set up mats
	if (!tmControl || (flags&CONTROL_OP) || (flags&CONTROL_INITPARAMS)) 
		InitControl(mc,obj,type,t);
	Matrix3 tm;	
	tm = Inverse(CompMatrix(t,&mc,NULL));

	if (type !=MAP_XYZTOUVW){  //UVW to XYZ
        obj->ApplyUVWMap(type,utile,vtile,wtile,uflip,vflip,wflip,cap,tm, mapChannel);
        
	} else
		{
//copies th xyz pos int the uvw channel

		
//Looks like NURBS don't support the W in UVW space so it is no good to copy the data over sense it would be meaningless
		if (os->obj->ClassID() == EDITABLE_SURF_CLASS_ID) 
			{
/*			NURBSSet getSet;
			BOOL okay = GetNURBSSet(os->obj, t, getSet, TRUE);
			if (mapChannel<2) mapChannel = 1-mapChannel;
			if (okay) {
				int ncount = getSet->GetNumObjects();

				NURBSObject *nObj;
				nObj = getSet.GetNURBSObject(0), ip->GetTime());
				SetTextureUVs()
				if (nObj->GetKind() == kNURBSSurface) {
					if (nObj->IsSelected() || ())
					// It's a blend, adjust the tension
					NURBSSurface *Surf = (NURBSSurface *)nObj;
	
					Surf->SetTextureUVs(t, 0, Point2 pt, mapChannel);
					}		
				}
*/
			}
#ifndef NO_PATCHES
		if (os->obj->IsSubClassOf(patchObjectClassID)) {


			PatchObject *patOb = (PatchObject *)os->obj;
			PatchMesh &mesh  = patOb->patch;
		
			int ct = 0;
			//if (mapChannel<2) mapChannel = 1-mapChannel;

			int fct = mesh.getNumPatches();
			int vct = mesh.getNumVerts();

			int handleCount=0, interiorCount=0;

//create TVface list
//create TVVert list
			Tab<int> vectorMappingTable;
			vectorMappingTable.SetCount(mesh.numVecs);
			for (int i=0; i<mesh.numVecs; i++) 
				vectorMappingTable[i] = -1;
			BitArray curvedEdges;
			curvedEdges.SetSize(mesh.numEdges);
			curvedEdges.ClearAll();
			int vecCount = vct;

			if (mesh.selLevel != PATCH_PATCH)
				{

//need to compute the number of texture handles and interiors also
				for (int i =0; i < fct; i++)
					{
					Patch &p = mesh.patches[i];
					if (!(mesh.patches[i].flags & PATCH_LINEARMAPPING))
						{
						if (!(mesh.patches[i].flags & PATCH_AUTO))
							{
							interiorCount += mesh.patches[i].type;
//mark which edges are curved
							for (int j=0;j < mesh.patches[i].type; j++)
								{
								int interior;
								interior = p.interior[j];
								vectorMappingTable[interior] = vecCount++;
								}	
							}
						}
					}

				for (i=0; i<fct; i++) 
					{
					Patch &p = mesh.patches[i];
					if (!(p.flags & PATCH_LINEARMAPPING))
						{
						int pcount = 3;
						if (p.type == PATCH_QUAD)
							pcount = 4;
				
//mark which edges are curved
						for (int j=0;j < pcount; j++)
							{
							int edgeID = p.edge[j];
							int vecA, vecB;
							if (!curvedEdges[edgeID])
								{
								vecA = mesh.edges[p.edge[j]].vec12;
								vecB = mesh.edges[p.edge[j]].vec21;
								vectorMappingTable[vecA] = vecCount++;
								vectorMappingTable[vecB] = vecCount++;
								handleCount += 2;
								}
							curvedEdges.Set(edgeID);
							}
						}
					}

				vct += handleCount+interiorCount;

				mesh.setNumMapVerts (mapChannel,vct);
				mesh.setNumMapPatches (mapChannel,fct);
				ct = vct;

				for (i = 0; i < mesh.getNumPatches(); i++)
			
					{
					int a,b,c,d;
					a  = mesh.patches[i].v[0];
					b  = mesh.patches[i].v[1];
					c  = mesh.patches[i].v[2];
					d  = mesh.patches[i].v[3];
					mesh.getMapPatch(mapChannel,i).setTVerts(a,b,c,d);
//need to add interior and handles if need be
					if (!(mesh.patches[i].flags & PATCH_LINEARMAPPING))
						{
						int v[8];
						for (int j = 0; j < mesh.patches[i].type*2; j++)
							{
							int vid = mesh.patches[i].vec[j];
							v[j] = vectorMappingTable[vid] ;

							PatchVec pv = mesh.getVec(vid);
							Point3 uvw = pv.p;
							uvw.x *= utile;
							uvw.y *= vtile;
							uvw.z *= wtile;
							if (uflip) uvw.x *= -1.0f;
							if (vflip) uvw.y *= -1.0f;
							if (wflip) uvw.z *= -1.0f;
							mesh.setMapVert(mapChannel,v[j],uvw);

							}
						mesh.getMapPatch(mapChannel,i).setTHandles(v,mesh.patches[i].type*2);

						if (!(mesh.patches[i].flags & PATCH_AUTO))
							{
							for (j = 0; j < mesh.patches[i].type; j++)
								{
								int vid = mesh.patches[i].interior[j];
								v[j] = vectorMappingTable[vid] ;
								PatchVec pv = mesh.getVec(vid);
								Point3 uvw = pv.p;
								uvw.x *= utile;
								uvw.y *= vtile;
								uvw.z *= wtile;
								if (uflip) uvw.x *= -1.0f;
								if (vflip) uvw.y *= -1.0f;
								if (wflip) uvw.z *= -1.0f;
								mesh.setMapVert(mapChannel,v[j],uvw);

								}
							mesh.getMapPatch(mapChannel,i).setTInteriors(v,mesh.patches[i].type);
							}
						}

					}
//look for texture verts if not there create them
				for (i=0; i<mesh.getNumVerts(); i++) {

// get point
					PatchVert pv = mesh.getVert(i);
					Point3 uvw = pv.p;
					uvw.x *= utile;
					uvw.y *= vtile;
					uvw.z *= wtile;
					if (uflip) uvw.x *= -1.0f;
					if (vflip) uvw.y *= -1.0f;
					if (wflip) uvw.z *= -1.0f;
					mesh.setMapVert(mapChannel,i,uvw);
					}
				}
			else
//else copy to just selected patches
				{
				vct = mesh.getNumMapVerts (mapChannel);
				int offset = vct;
				Tab<Point3> Points;
//				Tab<int> VIndex;
				Tab<TVPatch> Faces;
				for (int i = 0; i < mesh.getNumPatches(); i++)
					{
					if (mesh.patchSel[i] == 1)
						{
						TVPatch tv;
						int pcount = 3;
						if (mesh.patches[i].type == PATCH_QUAD) pcount =4;
						for (int k = 0; k < pcount; k++)
							{
							int found = -1;
							Point3 index = mesh.verts[mesh.patches[i].v[k]].p;
//loop through vindex see if they already exist
							for (int j = 0; j < Points.Count(); j++)
								{
								if (index == Points[j])
									{
									found = j;
									j = Points.Count();
									}
								}
							if (found == -1)
								{
								Points.Append(1,&index,1);
								tv.tv[k] = Points.Count()-1; 
								Points.Append(1,&index,1);
								}
							else
								{
								tv.tv[k] = found; 
								}
//now do handles
							if (!(mesh.patches[i].flags & PATCH_LINEARMAPPING))
								{
								found = -1;
								index = mesh.vecs[mesh.patches[i].vec[k*2]].p;
//loop through vindex see if they already exist
								for (int j = 0; j < Points.Count(); j++)
									{
									if (index == Points[j])
										{
										found = j;
										j = Points.Count();
										}
									}
								if (found == -1)
									{
									Points.Append(1,&index,1);
									tv.handles[k*2] = Points.Count()-1; 
									Points.Append(1,&index,1);
									}
								else
									{
									tv.handles[k*2] = found; 
									}

								found = -1;
								index = mesh.vecs[mesh.patches[i].vec[k*2+1]].p;
//loop through vindex see if they already exist
								for (j = 0; j < Points.Count(); j++)
									{
									if (index == Points[j])
										{
										found = j;
										j = Points.Count();
										}
									}
								if (found == -1)
									{
									Points.Append(1,&index,1);
									tv.handles[k*2+1] = Points.Count()-1; 
									Points.Append(1,&index,1);
									}
								else
									{
									tv.handles[k*2+1] = found; 
									}

//now do interiors
								if (!(mesh.patches[i].flags & PATCH_AUTO))
									{

									found = -1;
									index = mesh.vecs[mesh.patches[i].interior[k]].p;
//loop through vindex see if they already exist
									for (j = 0; j < Points.Count(); j++)
										{
										if (index == Points[j])
											{
											found = j;
											j = Points.Count();
											}
										}
									if (found == -1)
										{
										Points.Append(1,&index,1);
										tv.interiors[k] = Points.Count()-1; 
										Points.Append(1,&index,1);
										}
									else
										{
										tv.interiors[k] = found; 
										}

									}

								}

							}
//append face
						Faces.Append(1,&tv,1);
						}

					}

				if ((vct == 0) && (Points.Count() == 0)) {
					// This happens when applying UVW Map to selected patches, when no
					// previous map had been applied and no patches are selected.
					// Need to have at least one "dummy" mapping coordinate for all patches to use.
					UVVert a(0,0,0);
					if (mapChannel == 0) a = UVVert(1,1,1);
					Points.Append (1, &a);
					}
				mesh.setNumMapVerts (mapChannel,vct+Points.Count());
				if (mesh.tvPatches[mapChannel] == NULL)
					mesh.setNumMapPatches (mapChannel,fct);

				ct = vct;

				int f = 0;
				TVPatch *tvf = mesh.tvPatches[mapChannel];
				for (i = 0; i < mesh.getNumPatches(); i++)
			
					{
					if (mesh.patchSel[i] == 1)
						{
						Faces[f].tv[0] += vct;
						Faces[f].tv[1] += vct;
						Faces[f].tv[2] += vct;
						Faces[f].tv[3] += vct;
						if (!(mesh.patches[i].flags & PATCH_LINEARMAPPING))
							{
							for (int m =0; m < mesh.patches[i].type; m++)
								{
								Faces[f].handles[m*2] += vct;
								Faces[f].handles[m*2+1] += vct;
								if (!(mesh.patches[i].flags & PATCH_AUTO))
									Faces[f].interiors[m] += vct;

								}
							
							}
						tvf[i] = Faces[f++];
						}
					}
//look for texture verts if not there create them
				PatchTVert *tvp = mesh.tVerts[mapChannel];

				for (i=0; i<Points.Count(); i++) {
// get point
					Point3 uvw = Points[i];
					uvw.x *= utile;
					uvw.y *= vtile;
					uvw.z *= wtile;
					if (uflip) uvw.x *= -1.0f;
					if (vflip) uvw.y *= -1.0f;
					if (wflip) uvw.z *= -1.0f;

					tvp[i+vct].p = uvw;
					}
				}

			}
		else
#endif // NO_PATCHES

		if (os->obj->IsSubClassOf(triObjectClassID)) {
			TriObject *tobj = (TriObject*)os->obj;
			Mesh &mesh = tobj->GetMesh();
		
			int ct = 0;


			if (!mesh.mapSupport(mapChannel)) {
		// allocate texture verts. Setup tv faces into a parallel
		// topology as the regular faces
				TVFace *tvFace2;
//				int numTVerts2;
				if (mapChannel >= mesh.getNumMaps ()) mesh.setNumMaps (mapChannel+1, TRUE);
				mesh.setMapSupport (mapChannel, TRUE);
				mesh.setNumMapVerts (mapChannel, mesh.getNumVerts ());
				tvFace2 = mesh.mapFaces(mapChannel);
				for (int i=0; i<mesh.getNumFaces(); i++) {
					tvFace2[i].setTVerts(mesh.faces[i].getAllVerts());
					}
				} 

			if (mapChannel == 0)
			    ct = mesh.numCVerts;
			else ct = mesh.getNumMapVerts (mapChannel);
				//ct = mesh.numTVerts;


			int fct = mesh.getNumFaces();
			int vct = mesh.getNumVerts();


//else copy to all mesh
			if (mesh.selLevel != MESH_FACE)
				{
				mesh.setNumMapVerts(mapChannel, vct);
//					mesh.setNumMapFaces(fct);

				ct = vct;
				TVFace *tvFace = NULL;
				tvFace = mesh.mapFaces(mapChannel);
				for (int i = 0; i < mesh.getNumFaces(); i++)
					{
					tvFace[i].setTVerts(mesh.faces[i].getAllVerts());
					}
//look for texture verts if not there create them
				UVVert *tVerts = mesh.mapVerts(mapChannel);
				for (i=0; i<ct; i++) {
// get point
					Point3 uvw = mesh.getVert(i);
					uvw.x *= utile;
					uvw.y *= vtile;
					uvw.z *= wtile;
					if (uflip) uvw.x *= -1.0f;
					if (vflip) uvw.y *= -1.0f;
					if (wflip) uvw.z *= -1.0f;
					tVerts[i] = uvw;
					}

				}
//else copy to just selected faces
			else
				{
				vct = mesh.getNumMapVerts (mapChannel);
				int offset = vct;
				Tab<Point3> Points;
//				Tab<int> VIndex;
				Tab<TVFace> Faces;
				for (int i = 0; i < mesh.getNumFaces(); i++)
					{
					if (mesh.faceSel[i] == 1)
						{
						TVFace tv;
						for (int k = 0; k < 3; k++)
							{
							int found = -1;
//loop through vindex see if they already exist
							Point3 index;
							index = mesh.getVert(mesh.faces[i].v[k]);
							for (int j = 0; j < Points.Count(); j++)
								{
								
								if (index == Points[j])
									{
									found = j;
									j = Points.Count();
									}
								}
							if (found == -1)
								{
								Points.Append(1,&index,1);
								tv.t[k] = Points.Count()-1; 

								}
							else
								{
								tv.t[k] = found; 
								}
							}
//append face
						Faces.Append(1,&tv,1);
						}

					}
				mesh.setNumMapVerts (mapChannel, vct+Points.Count(), TRUE);

				int f = 0;
				TVFace *tvFace = NULL;
				tvFace = mesh.mapFaces(mapChannel);
				for (i = 0; i < mesh.getNumFaces(); i++)
			
					{
					if (mesh.faceSel[i] == 1)
						{
						Faces[f].t[0] += vct;
						Faces[f].t[1] += vct;
						Faces[f].t[2] += vct;
						tvFace[i] = Faces[f];
						f++;
						}
DebugPrint("Face %d id %d %d %d\n",i,tvFace[i].t[0],tvFace[i].t[1],tvFace[i].t[2]);
					}
//look for texture verts if not there create them
				UVVert *tVerts = mesh.mapVerts(mapChannel);
				for (i=0; i<Points.Count(); i++) {
// get point
					Point3 uvw = Points[i];
					uvw.x *= utile;
					uvw.y *= vtile;
					uvw.z *= wtile;
					if (uflip) uvw.x *= -1.0f;
					if (vflip) uvw.y *= -1.0f;
					if (wflip) uvw.z *= -1.0f;

					tVerts[i+vct] = uvw;
					}
for (i =0; i < mesh.getNumMapVerts(mapChannel); i++)
{
DebugPrint("TVert %d p %f %f %f\n",i,tVerts[i].x,tVerts[i].y,tVerts[i].z);
}
				}

			}
			else if (os->obj->IsSubClassOf(polyObjectClassID)) {
			// aszabo | Nov.29.00 | 260223
			// Fixing this in a manner that keeps the mess consistent.
			// SteveA will refactor this.
			PolyObject *polyObj = (PolyObject*)os->obj;
			MNMesh &mnMesh = polyObj->GetMesh();
		
			// The following code is a modified version of the one in
			// MNMesh::ApplyMapper (UVWMapper & mp, BOOL channel, BOOL useSel)
			
			// Set up the mapchannel if it does not exist
			DbgAssert((mapChannel >= -NUM_HIDDENMAPS) && (mapChannel < MAX_MESHMAPS));
			bool hasMap;
			if (mapChannel >= mnMesh.MNum()) {
				mnMesh.SetMapNum (mapChannel+1);
				hasMap = false;
			} else {
				hasMap = !mnMesh.M(mapChannel)->GetFlag (MN_DEAD);
			}

			MNMap *mc = mnMesh.M(mapChannel);
			int i, j;

			int newCT=0;

			if (!hasMap || mnMesh.selLevel != MNM_SL_FACE) {
				// The map channel is new and the mapping should apply to the whole object
				// allocate texture verts. Setup tv faces into a parallel
				// topology as the regular faces
				mc->ClearFlag (MN_DEAD);
				mc->setNumVerts( mnMesh.VNum() );
				mc->setNumFaces(mnMesh.FNum() );
				for (i=0; i<mnMesh.FNum(); i++) {
					if (mnMesh.f[i].GetFlag (MN_DEAD)) 
						continue;
					mc->f[i] = mnMesh.f[i];
				}
			} else {
				const unsigned int UNDEFINED = 0xffffffff;
				// We've got some preexisting map, and we're trying to apply a new map to
				// selected faces.
				Tab<DWORD> vmap;
				vmap.SetCount(mnMesh.VNum());
				for (i=0; i<vmap.Count(); i++) 
					vmap[i] = UNDEFINED;

				// Selected faces are assigned a VC face topology
				// that matches the face topology instead of inheriting 
				// whatever VC topology happened to be there.
				for (i=0; i<mnMesh.FNum(); i++) {
					MNMapFace & mmf = mc->f[i];
					if (!mnMesh.f[i].GetFlag (MN_SEL)) 
						continue;
					for (j=0; j<mnMesh.f[i].deg; j++) {
						if (vmap[mnMesh.f[i].vtx[j]] == UNDEFINED) 
							vmap[mnMesh.f[i].vtx[j]] = mc->VNum() + newCT++;
						mmf.tv[j] = vmap[mnMesh.f[i].vtx[j]];
					}
				}
				mc->setNumVerts( mc->VNum() + newCT );
			}

			if (!mnMesh.VNum()) 
				return;

			// The following part from MNMesh::ApplyMapper has been greatly 
			// simplified since normMatters = false; and wrappable = false;
			// for XYZ to UVW mapping type

			// Now map the faces
			UVVert uvw(0.0f,0.0f,0.0f);
			for (i=0; i<mnMesh.FNum(); i++) {
				if (mnMesh.f[i].GetFlag (MN_DEAD)) 
					continue;
				MNMapFace & mmf = mc->f[i];
				if ((mnMesh.selLevel == MNM_SL_FACE) && !mnMesh.f[i].GetFlag (MN_SEL)) 
					continue;
				for (j=0; j<mnMesh.f[i].deg; j++) {
					int vj = mnMesh.f[i].vtx[j];
					int tvj = mc->f[i].tv[j];

					mc->v[tvj] = mnMesh.P(vj);
					mc->v[tvj].x *= utile;
					mc->v[tvj].y *= vtile;
					mc->v[tvj].z *= wtile;
					if (uflip) mc->v[tvj].x *= -1.0f;
					if (vflip) mc->v[tvj].y *= -1.0f;
					if (wflip) mc->v[tvj].z *= -1.0f;
				}
			}

			// See if we can compact the tVert array.
			mc->CollapseDeadVerts (mnMesh.F(0));
		}
		}

	// The tex mapping depends on the geom and topo so make sure the validity interval reflects this.
	Interval iv = GetValidity(t);
	iv = iv & os->obj->ChannelValidity(t,GEOM_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	os->obj->UpdateValidity(TEXMAP_CHAN_NUM,iv);	
}


static int lStart[12] = {0,1,3,2,4,5,7,6,0,1,2,3};
static int lEnd[12]   = {1,3,2,0,5,7,6,4,4,5,6,7};

static void DoBoxIcon(BOOL sel,float length, PolyLineProc& lp)
	{
	Point3 pt[3];
	
	length *= 0.5f;
	Box3 box;
	box.pmin = Point3(-length,-length,-length);
	box.pmax = Point3( length, length, length);

	if (sel) //lp.SetLineColor(1.0f,1.0f,0.0f);
		 lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
	else //lp.SetLineColor(0.85f,0.5f,0.0f);		
		 lp.SetLineColor(GetUIColor(COLOR_GIZMOS));

	for (int i=0; i<12; i++) {
		pt[0] = box[lStart[i]];
		pt[1] = box[lEnd[i]];
		lp.proc(pt,2);
		}
	}





//--- MappingMod methods -----------------------------------------------

void* MappingMod::GetInterface(ULONG id)
	{	
	if (id==I_MAPPINGINTERFACE) return this;
	else return Modifier::GetInterface(id);
	}


void MappingMod::InitControl(
		ModContext &mc,Object *obj,int type,TimeValue t)
	{
	Box3 box;
	Matrix3 tm;
	mLocalSetValue = true; // mjm - 6.7.99

	if (tmControl==NULL) {
		MakeRefByID(FOREVER,0,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		}		
	
	if (flags&CONTROL_INIT) {
		SuspendAnimate();
		AnimateOff();		
		// Rotate the seem to the back
		SetXFormPacket pckt(Quat(RotateZMatrix(PI)),TRUE);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		ResumeAnimate();
		}

	if (flags&CONTROL_INITPARAMS) {
		SetLength(0,2.0f);
		SetWidth(0,2.0f);
		if (type==MAP_BALL ||
			type==MAP_SPHERICAL ||
			type==MAP_BOX) {			
			SetHeight(0,2.0f);
		} else {
			SetHeight(0,1.0f);
			}
		SetAxis(2);
		}

	if (!(flags&CONTROL_OP)) {
		flags = 0;
		return;
		}

	if (flags&CONTROL_HOLD) 
		{
		theHold.Begin();
		}


	Matrix3 oldtm = tm = Inverse(CompMatrix(t,&mc,NULL,FALSE));	
	AffineParts parts;
	decomp_affine(tm, &parts);
	parts.q.MakeMatrix(tm);
	tm.Translate(parts.t);
	
	Matrix3 mctm(1);
	if (mc.tm) mctm = *mc.tm;
	tm.Scale(Point3(
		Length(mctm.GetRow(0)),
		Length(mctm.GetRow(1)),
		Length(mctm.GetRow(2))
		));

	Point3 s;
	s.x = GetWidth(t);
	s.y = GetLength(t);
	s.z = GetHeight(t);	

	ModAppEnumProc dep;              
	EnumDependents(&dep);

	if (dep.Count() == 1)
		obj->GetDeformBBox(t,box,&tm,TRUE);
	else
		{
		box = *(mc.box);
		if (box.IsEmpty()) box.MakeCube(Point3(0,0,0),10.0f);	// SCA 990621: fixes problem with empty selection sets.
		if (mc.tm) { // RB 10/5/99: Need to handle NULL mc.tm.
			box = box * (*(mc.tm)) * tm;
		} else {
			box = box * tm;
			}
		}

	box.Scale(1.001f);  // prevent wrap-around on sides of boxes

	if (flags&CONTROL_ASPECT &&
		(type==MAP_PLANAR ||
		 type==MAP_BOX ||
		 type==MAP_CYLINDRICAL)) {

		if (type==MAP_PLANAR || type==MAP_BOX) {			
			float w = aspect*s.y;
			s.x *= w / s.x;			
			s.z *= s.y / s.z;				
		} else
		if (type==MAP_CYLINDRICAL) {
			float w = (s.x+s.y)*PI;			
			s.z *= w/(aspect*s.z);
			}
		}

	if (flags&CONTROL_UNIFORM) {
		float av = (s.x + s.y + s.z)/3.0f;
		s.x = s.y = s.z = av;
		}

	if (flags&CONTROL_FIT) {
		Point3 w = box.Width();

#ifdef USE_SIMPLIFIED_UVWMAP_UI
		// VizR wants to auto-tile maps based on
		// known 1m x 1m x 1m units
		if( flags&(CONTROL_INIT|CONTROL_RESET) )
			w.x = w.y = w.z = 1.0f;
#endif

		if (box.IsEmpty()) w = Point3(10.0f,10.0f,10.0f);

		if (type==MAP_PLANAR) {
			s.x = w.x==0.0f ? 1.0f : w.x;
			s.y = w.y==0.0f ? 1.0f : w.y;
			s.z = w.z==0.0f ? 1.0f : w.z;
		} else
		if (type==MAP_BOX) {
			s.x = w.x==0.0f ? 1.0f : w.x;
			s.y = w.y==0.0f ? 1.0f : w.y;
			s.z = w.z==0.0f ? 1.0f : w.z;
		} else
		if (type==MAP_SPHERICAL || type==MAP_BALL) {
			float max = w.x;
			if (w.y>max) max = w.y;
			if (w.z>max) max = w.z;
			if (max==0.0f) max = 1.0f;
			s.x = s.y = s.z = max;
		} else {
			if (w.x>w.y) s.x = s.y = w.x;
			else s.x = s.y = w.y;
			s.z = w.z;
			if (s.x==0.0f) s.x = 1.0f;
			if (s.y==0.0f) s.y = 1.0f;
			if (s.z==0.0f) s.z = 1.0f;			
			}
		}

	if (flags&(CONTROL_CENTER|CONTROL_FIT)) {		
//		Matrix3 tm(1);
//		Interval valid;
//		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);		
//		SetXFormPacket pckt(mc.box->Center()-(mc.box->Center()*tm));
//			tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		
//get current center
//get vector
		Point3 cent;
		Point3 Zero(0.0f,0.0f,0.0f);
//if the translation is zero only assigned to one object thus use the obj bbox
		if (dep.Count() == 1)

			{
			Box3 sbox;		
			obj->GetDeformBBox(t,sbox,&oldtm,TRUE);
		
			cent = sbox.Center();
			cent = VectorTransform(Inverse(oldtm),cent);
			cent = VectorTransform(mctm,cent);
			}

		else 
			{
			if (flags&CONTROL_INIT) 
				{
				cent.x = 0.0f;
				cent.y = 0.0f;
				cent.z = 0.0f;

				}
			else
				{
				Matrix3 tm(1);
				Interval valid;
				tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);		

				cent = -tm.GetTrans();

				}
			}
		SetXFormPacket pckt(
			cent, //sbox.Center(),
			Matrix3(1),
			Matrix3(1));
		
		SuspendAnimate();
		AnimateOff();
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		ResumeAnimate();
		}

	if (flags&(CONTROL_FIT|CONTROL_ASPECT|CONTROL_INITPARAMS|CONTROL_UNIFORM)) {
		SuspendAnimate();
		AnimateOff();

		// Clear out any scale in the transform
		tm = Matrix3(1);	
		tmControl->GetValue(t,&tm,FOREVER,CTRL_RELATIVE);	
		decomp_affine(tm, &parts);
		parts.q.MakeMatrix(tm);
		tm.Translate(parts.t);	
		SetXFormPacket pckt(tm);		
		tmControl->SetValue(t,&pckt,TRUE,CTRL_ABSOLUTE);

		// Set the new dimensions
		SetLength(t,s.y);
		SetWidth(t,s.x);
		SetHeight(t,s.z);

		ResumeAnimate();
		}

	if (flags&CONTROL_HOLD) 
		{
		if (flags&CONTROL_FIT)
			theHold.Accept(GetString(IDS_PW_UNDO_FIT));	
		else theHold.Accept(GetString(IDS_RB_CENTER));	
			


		}


	mLocalSetValue = false; // mjm - 6.7.99
	flags = 0;
	}


#if 0
void MappingMod::InitControl(ModContext &mc,Object *obj,int type,TimeValue t)
	{
	Box3 box;
	Matrix3 tm;

	if (tmControl==NULL) {
		MakeRefByID(FOREVER,0,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		}		

	if (flags&CONTROL_HOLD) theHold.Begin();
	if (flags&CONTROL_INIT) {
		SuspendAnimate();
		AnimateOff();		
		// Rotate the seem to the back
		SetXFormPacket pckt(Quat(RotateZMatrix(PI)),TRUE);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}

	tm = Inverse(CompMatrix(t,&mc,NULL));
	obj->GetDeformBBox(t,box,&tm,TRUE);
	box.Scale(1.001f);  // prevent wrap-around on sides of boxes

	if (flags&CONTROL_ASPECT &&
		(type==MAP_PLANAR ||
		 type==MAP_BOX ||
		 type==MAP_CYLINDRICAL)) {
		Matrix3 tm(1), id(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
		Point3 s;

		if (type==MAP_PLANAR || type==MAP_BOX) {			
			float w = aspect*Length(tm.GetRow(1));
			s.x = w / Length(tm.GetRow(0));
			s.y = 1.0f;
			s.z = Length(tm.GetRow(1)) / Length(tm.GetRow(2));	
			/*
			float a = (Length(tm.GetRow(0))+Length(tm.GetRow(1))+Length(tm.GetRow(2)))/3.0f;
			s.x = aspect*a / Length(tm.GetRow(0));
			s.y = a / Length(tm.GetRow(1));
			s.z = a / Length(tm.GetRow(2));
			*/
		} else
		if (type==MAP_CYLINDRICAL) {			
			// Old way where we sized the radius
			/*
			float lz = Length(tm.GetRow(2));
			float rad = lz*aspect/(TWOPI);
			s.x = rad / Length(tm.GetRow(0));
			s.y = rad / Length(tm.GetRow(1));
			s.z = 1.0f;
			*/
			// Now we size the height
			float w = (Length(tm.GetRow(0))+Length(tm.GetRow(1)))*PI;
			s.x = 1.0f;
			s.y = 1.0f;
			s.z = w/(aspect*Length(tm.GetRow(2)));
			}
		
		SetXFormPacket pckt(s,TRUE,id,tm);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}

	if (flags&CONTROL_UNIFORM) {
		Matrix3 tm(1), id(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
		float av = 0.0f;
		Point3 s;
		av += Length(tm.GetRow(0));
		av += Length(tm.GetRow(1));
		av += Length(tm.GetRow(2));
		av /= 3.0f;
		s.x = av/Length(tm.GetRow(0));
		s.y = av/Length(tm.GetRow(1));
		s.z = av/Length(tm.GetRow(2));

		SetXFormPacket pckt(s,TRUE,id,tm);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}

	if (flags&CONTROL_FIT) {
		
		// Get the box of the object in mod context space but unscaled
		Matrix3 mat = tm;
		mat.NoScale();
		obj->GetDeformBBox(t,box,&mat,TRUE);
		box.Scale(1.001f);  // prevent wrap-around on sides of boxes

		Point3 s, w  = box.Width();
		if (box.IsEmpty()) w = Point3(10.0f,10.0f,10.0f);
		Matrix3 tm(1), id(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);

		if (type==MAP_PLANAR) {
			s.x = w.x==0.0f ? 1.0f : w.x/2.0f;
			s.y = w.y==0.0f ? 1.0f : w.y/2.0f;
			s.z = w.z==0.0f ? 1.0f : w.z;
		} else
		if (type==MAP_BOX) {
			s.x = w.x==0.0f ? 1.0f : w.x/2.0f;
			s.y = w.y==0.0f ? 1.0f : w.y/2.0f;
			s.z = w.z==0.0f ? 1.0f : w.z/2.0f;
		} else
		if (type==MAP_SPHERICAL || type==MAP_BALL) {
			float max = w.x;
			if (w.y>max) max = w.y;
			if (w.z>max) max = w.z;
			if (max==0.0f) max = 1.0f;
			s.x = s.y = s.z = max/2.0f;
		} else {
			if (w.x>w.y) s.x = s.y = w.x/2.0f;
			else s.x = s.y = w.y/2.0f;
			s.z = w.z;
			if (s.x==0.0f) s.x = 1.0f;
			if (s.y==0.0f) s.y = 1.0f;
			if (s.z==0.0f) s.z = 1.0f;			
			}
		
		s.x /= Length(tm.GetRow(0));
		s.y /= Length(tm.GetRow(1));
		s.z /= Length(tm.GetRow(2));
		
		SetXFormPacket pckt(s,TRUE,id,tm);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		
		// redo the box so our center op works
		tm = Inverse(CompMatrix(t,&mc,NULL));
		obj->GetDeformBBox(t,box,&tm,TRUE);
		box.Scale(1.001f);  // prevent wrap-around on sides of boxes
		}		

	if (flags&(CONTROL_CENTER|CONTROL_FIT)) {
		Matrix3 tm(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);		
		SetXFormPacket pckt(VectorTransform(tm,box.Center()));		
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}

	if (flags&CONTROL_HOLD) theHold.Accept(0);	
	if (flags&CONTROL_INIT) {
		ResumeAnimate();
		}

	flags = 0;
	}
#endif


// --- Gizmo transformations ------------------------------------------

void MappingMod::Move(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) 
	{	
#ifdef DESIGN_VER
	t=0;
#endif
	assert(tmControl);	
	SetXFormPacket pckt(val,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
	}


void MappingMod::Rotate(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Quat& val, BOOL localOrigin) 
	{
#ifdef DESIGN_VER
	t=0;
#endif
	assert(tmControl);
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	}

void MappingMod::Scale(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) 
	{
#ifdef DESIGN_VER
	t=0;
#endif
	assert(tmControl);
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	}


Matrix3 MappingMod::CompMatrix(
		TimeValue t,ModContext *mc, Matrix3 *ntm,BOOL applySize, BOOL applyAxis)
	{
	Matrix3 tm(1);
	Interval valid;
	
	int type = GetMapType();

	if (tmControl) {
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
		}
	
	// Rotate icon	
	if (applyAxis && TestAFlag(A_PLUGIN1)) {
		switch (type) {
			case MAP_BOX:
			case MAP_PLANAR:
				tm.PreRotateZ(PI);
				break;
			
			case MAP_BALL:
			case MAP_SPHERICAL:
			case MAP_CYLINDRICAL:
				tm.PreRotateZ(HALFPI);
				break;
			}
		}
	
	if (applyAxis) {
		switch (GetAxis()) {
			case 0:
				tm.PreRotateY(-HALFPI);
				break;
			case 1:			
				tm.PreRotateX(HALFPI);
				break;
			}
		}

	if (applySize) {
		Point3 s;
		s.x = GetWidth(t);
		s.y = GetLength(t);
		s.z = GetHeight(t);
		switch (type) {
			case MAP_CYLINDRICAL:			
			case MAP_PLANAR:
				s.x *= 0.5f;
				s.y *= 0.5f;
				break;
			
			case MAP_BALL:
			case MAP_SPHERICAL:
			case MAP_BOX:
				s *= 0.5f;				
				break;
			}
		tm.PreScale(s);
		}

	if (mc && mc->tm) {
		tm = tm * Inverse(*mc->tm);
		}
	if (ntm) {
		tm = tm * *ntm;
		}
	return tm;
	}

void MappingMod::DoIcon(PolyLineProc& lp,BOOL sel)
	{
	int type = GetMapType();	
	switch (type) {
		case MAP_BOX: DoBoxIcon(sel,2.0f,lp); break;
		case MAP_PLANAR: DoPlanarMapIcon(sel,2.0f,2.0f,lp); break;
		case MAP_BALL:
		case MAP_SPHERICAL: DoSphericalMapIcon(sel,1.0f,lp); break;
		case MAP_CYLINDRICAL: DoCylindricalMapIcon(sel,1.0f,1.0f,lp); break;
		}
	}
 
int MappingMod::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) 
	{	
	int savedLimits;	
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 modmat, ntm = inode->GetObjectTM(t);
	DrawLineProc lp(gw);
		
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	
	if (ip && ip->GetSubObjectLevel() == 1) {		
		modmat = CompMatrix(t,mc,&ntm);
		gw->setTransform(modmat);
		DoIcon(lp,FALSE);
		}
	
	gw->setRndLimits(savedLimits);	
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
		}
	return 0;
	}

int MappingMod::Display(
		TimeValue t, INode* inode, ViewExp *vpt, int flags, 
		ModContext *mc) 
	{	
	GraphicsWindow *gw = vpt->getGW();
	#ifdef DESIGN_VER
		TimeValue rt = GetCOREInterface()->GetTime();
		Matrix3 modmat, ntm = inode->GetObjectTM(rt);
	#else
		Matrix3 modmat, ntm = inode->GetObjectTM(t);
	#endif

	DrawLineProc lp(gw);

	modmat = CompMatrix(t,mc,&ntm);	
	gw->setTransform(modmat);	
	DoIcon(lp, ip&&ip->GetSubObjectLevel()==1);

	return 0;	
	}

void MappingMod::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, Box3& box, 
		ModContext *mc) 
	{
#ifdef DESIGN_VER
		TimeValue rt = GetCOREInterface()->GetTime();
		Matrix3 modmat, ntm = inode->GetObjTMBeforeWSM(rt);
#else
	Matrix3 modmat, ntm = inode->GetObjTMBeforeWSM(t);	
#endif
	modmat = CompMatrix(t,mc,&ntm);		
	BoxLineProc bproc(&modmat);
	DoIcon(bproc,FALSE);
	box = bproc.Box();	
	}

void MappingMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{	
	Matrix3 modmat, ntm = node->GetObjectTM(t);	
	modmat = CompMatrix(t,mc,&ntm);
	cb->Center(modmat.GetTrans(),0);	
	}

void MappingMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Matrix3 ntm = node->GetObjectTM(t), modmat;
	modmat = CompMatrix(t,mc,&ntm);
	cb->TM(modmat,0);
	}

void FixDimsPLCB::proc(ILoad *iload)
	{
	if (mod->GetPBlockVersion()<mod->GetFirstParamVer()) {
		mod->flags |= CONTROL_INITPARAMS;
		}
	delete this;
	}
