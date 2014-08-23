/**********************************************************************
 *<
	FILE: dispmod.cpp

	DESCRIPTION:  Displacement mapping modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/21/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"

#ifndef NO_MODIFIER_DISPLACE // JP Morel - July 23th 2002

#include "iparamm.h"
#include "bmmlib.h"
#include "texutil.h"
#include "simpobj.h"
#include "simpmod.h"
#include "mapping.h"
#include "sctex.h"
#include "MaxIcon.h"
#include "modsres.h"
#include "resourceOverride.h"

#include "macrorec.h"


#define IDC_SET_MAPNAME 0x9999
#define IDC_SET_IMAGENAME 0x999A

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//       WARNING - a copy of this class description is in maxscrpt\maxmods.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class DispMod : public MappingMod {	
	public:
		IParamBlock *pblock;		
		Bitmap *thebm;
		BitmapInfo bi;
		Texmap *tmap;

		static IParamMap *pmapParam;
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;		
		static FaceAlignMode *faceAlignMode;
		static RegionFitMode *regionFitMode;
		static PickAcquire pickAcquire;
		static DispMod *editMod;

		DispMod(BOOL create);
		~DispMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_DISPMOD); }  
		virtual Class_ID ClassID() { return Class_ID(DISPLACEOSM_CLASS_ID,0);}
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);		
		TCHAR *GetObjectName() { return GetString(IDS_RB_DISPLACE); }
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		BOOL AssignController(Animatable *control,int subAnim);
		int SubNumToRefNum(int subNum);

		ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE|TEXMAP_CHANNEL|VERTCOLOR_CHANNEL;}
		ChannelMask ChannelsChanged() {return PART_GEOM|TEXMAP_CHANNEL|VERTCOLOR_CHANNEL; }
		Class_ID InputType() {return defObjectClassID;}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t);
		Interval GetValidity(TimeValue t);

		int NumRefs() {return 3;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) {
			if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/21/03
			bi.EnumAuxFiles(nameEnum,flags);
			MappingMod::EnumAuxFiles( nameEnum, flags ); // LAM - 4/21/03
		}

		int NumSubs() {return 3;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);

		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		
		void FreeBitmap();
		void LoadBitmap();
		void SetImageTime(TimeValue t);
	
		void ActivateSubobjSel(int level, XFormModes& modes);
		
		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		// From mapping mod
		void EnterNormalAlign();
		void ExitNormalAlign();
		void EnterRegionFit();
		void ExitRegionFit();
		int GetMapType();
		void SetMapType(int type);
		int GetMapChannel ();
		void SetMapChannel (int chan);
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
		int GetFirstParamVer() {return 5;}
		int GetPBlockVersion() {return pblock->GetVersion();}

	};


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//       WARNING - a copy of this class description is in maxscrpt\maxmods.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class DisplaceObject : public SimpleWSMObject {	
	public:		
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static HWND hSot;
		Bitmap *thebm;
		BitmapInfo bi;
		Texmap *tmap;
		BOOL initParams;

		DisplaceObject();		
		~DisplaceObject();

		void FreeBitmap();
		void LoadBitmap();
		void SetImageTime(TimeValue t);

		// From Animatable		
		void DeleteThis() {delete this;}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() {return GetString(IDS_RB_DISPLACE);}
		Class_ID ClassID() {return Class_ID(DISPLACE_OBJECT_CLASS_ID,0);}		
		void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) {
			if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/21/03
			bi.EnumAuxFiles(nameEnum,flags);
			SimpleWSMObject::EnumAuxFiles( nameEnum, flags ); // LAM - 4/21/03
		}
		int NumSubs() {return 2;}  
		Animatable* SubAnim(int i) {return GetReference(i);}
		TSTR SubAnimName(int i) 
			{if (i==0) return SimpleWSMObject::SubAnimName(i); else return _T("Displacement Map");}
		
		// from object		
		int DoOwnSelectHilite() {return TRUE;}
		CreateMouseCallBack* GetCreateMouseCallBack();		

		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);
		int NumRefs() {return 2;}
		RefTargetHandle GetReference(int i) { if (i==0) return pblock; else return tmap;}
		void SetReference(int i, RefTargetHandle rtarg);			

		// From WSMObject
		Modifier *CreateWSMMod(INode *node);		
		ForceField *GetForceField(INode *node);

		void BuildMesh(TimeValue);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);		
		void DisableControls();
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	};

class DisplaceField : public ForceField {
	public:
		DisplaceObject *dobj;		
		Bitmap *thebm;
		INode *node;
		Matrix3 tm, invtm;
		Interval tmValid;
		Point3 Force(TimeValue t,const Point3 &pos, const Point3 &vel,int index);
		void DeleteThis() {delete this;} // RB 5/12/99
	};

class DisplaceWSMod : public SimpleWSMMod {
	public:
		DisplaceField force;

		DisplaceWSMod() {}
		DisplaceWSMod(INode *node,DisplaceObject *obj);		

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		// From Animatable
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_DISPLACEMOD);}
		SClass_ID SuperClassID() { return WSM_CLASS_ID; }		
		void DeleteThis() {delete this;}		
		Class_ID ClassID() { return Class_ID(DISPLACE_WSM_CLASS_ID,0); } 		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() {return GetString(IDS_RB_DISPLACEBINDING);}
		
		// From SimpleWSMMod		
		Interval GetValidity(TimeValue t);		
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	};


static GenSubObjType SOT_Apparatus(14);


//--- ClassDescriptor and class vars ---------------------------------

IParamMap*          DispMod::pmapParam     = NULL;
MoveModBoxCMode*    DispMod::moveMode      = NULL;
RotateModBoxCMode*  DispMod::rotMode       = NULL;
UScaleModBoxCMode*  DispMod::uscaleMode    = NULL;
NUScaleModBoxCMode* DispMod::nuscaleMode   = NULL;
SquashModBoxCMode*  DispMod::squashMode    = NULL;
FaceAlignMode*      DispMod::faceAlignMode = NULL;
RegionFitMode*      DispMod::regionFitMode = NULL;
PickAcquire         DispMod::pickAcquire;
DispMod*            DispMod::editMod       = NULL;

class DispClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new DispMod(!loading); }
	const TCHAR *	ClassName() { return GetString(IDS_RB_DISPLACE_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(DISPLACEOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static DispClassDesc dispDesc;
extern ClassDesc* GetDispModDesc() {return &dispDesc;}


IParamMap *DisplaceObject::pmapParam = NULL;
IObjParam *DisplaceObject::ip        = NULL;
HWND       DisplaceObject::hSot      = NULL;

class DispObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new DisplaceObject;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_DISPLACE_CLASS);}
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() {return Class_ID(DISPLACE_OBJECT_CLASS_ID,0);}
	const TCHAR* 	Category() {return GetSpaceWarpCatString(SPACEWARP_CAT_GEOMDEF);}
	};

static DispObjClassDesc dispObjDesc;
extern ClassDesc* GetDispObjDesc() {return &dispObjDesc;}


class DispWSModeClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) {return new DisplaceWSMod;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_DISPLACE_CLASS);}
	SClass_ID		SuperClassID() { return WSM_CLASS_ID; }
	Class_ID		ClassID() {return Class_ID(DISPLACE_WSM_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};

static DispWSModeClassDesc dispWSModDesc;
extern ClassDesc* GetDispWSModDesc() {return &dispWSModDesc;}

//--- Parameter map/block descriptors -------------------------------

#define PB_MAPTYPE		0
#define PB_UTILE		1
#define PB_VTILE		2
#define PB_WTILE		3
#define PB_BLUR			4
#define PB_USEMAP		5
#define PB_APPLYMAP		6
#define PB_STRENGTH		7
#define PB_DECAY		8
#define PB_CENTERLUM	9
#define PB_UFLIP		10
#define PB_VFLIP		11
#define PB_WFLIP		12
#define PB_CENTERL		13
#define PB_CAP			14
#define PB_LENGTH		15
#define PB_WIDTH		16
#define PB_HEIGHT		17
#define PB_AXIS			18
#define PB_CHANNEL  19
#define PB_MAPCHANNEL 20

//
//
// Parameters

static int mapIDs[] = {IDC_DISP_PLANAR,IDC_DISP_CYL,IDC_DISP_SPHERE,IDC_DISP_BALL};
static int axisIDs[] = {IDC_DISP_X,IDC_DISP_Y,IDC_DISP_Z};
static int chanIDs[] = { IDC_DISP_TEXMAP, IDC_DISP_VERTCOL };

static ParamUIDesc descParam[] = {
	// Map type
	ParamUIDesc(PB_MAPTYPE,TYPE_RADIO,mapIDs,4),
	
	// U Tile
	ParamUIDesc(
		PB_UTILE,
		EDITTYPE_FLOAT,
		IDC_DISP_UTILE,IDC_DISP_UTILESPIN,
		-999999999.0f,999999999.0f,
		0.01f),

	// V Tile
	ParamUIDesc(
		PB_VTILE,
		EDITTYPE_FLOAT,
		IDC_DISP_VTILE,IDC_DISP_VTILESPIN,
		-999999999.0f,999999999.0f,
		0.01f),

	// W Tile
	ParamUIDesc(
		PB_WTILE,
		EDITTYPE_FLOAT,
		IDC_DISP_WTILE,IDC_DISP_WTILESPIN,
		-999999999.0f,999999999.0f,
		0.01f),

	// Blur
	ParamUIDesc(
		PB_BLUR,
		EDITTYPE_FLOAT,
		IDC_DISP_BLUR,IDC_DISP_BLURSPIN,
		0.0f,10.0f,
		0.01f),	

	// Strength
	ParamUIDesc(
		PB_STRENGTH,
		EDITTYPE_UNIVERSE,
		IDC_DISP_STRENGTH,IDC_DISP_STRENGTHSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),
	
	// Decay
	ParamUIDesc(
		PB_DECAY,
		EDITTYPE_FLOAT,
		IDC_DISP_DECAY,IDC_DISP_DECAYSPIN,
		-999999999.0f,999999999.0f,
		0.005f),
	
	// Center lum
	ParamUIDesc(PB_CENTERLUM,TYPE_SINGLECHEKBOX,IDC_DISP_CENTERLUM),

	// U Flip
	ParamUIDesc(PB_UFLIP,TYPE_SINGLECHEKBOX,IDC_DISP_UFLIP),

	// V Flip
	ParamUIDesc(PB_VFLIP,TYPE_SINGLECHEKBOX,IDC_DISP_VFLIP),

	// W Flip
	ParamUIDesc(PB_WFLIP,TYPE_SINGLECHEKBOX,IDC_DISP_WFLIP),	

	// Center
	ParamUIDesc(
		PB_CENTERL,
		EDITTYPE_FLOAT,
		IDC_DISP_CENTERL,IDC_DISP_CENTERLSPIN,
		0.0f,1.0f,
		0.005f),	

	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_DISP_LENGTH,IDC_DISP_LENGTHSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_DISP_WIDTH,IDC_DISP_WIDTHSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),

	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_DISP_HEIGHT,IDC_DISP_HEIGHTSPIN,
		-999999999.0f,999999999.0f,
		SPIN_AUTOSCALE),
	
	// Cap
	ParamUIDesc(PB_CAP,TYPE_SINGLECHEKBOX,IDC_DISP_CAP),

	// Axis
	ParamUIDesc(PB_AXIS,TYPE_RADIO,axisIDs,3),

	// Use existing mapping
	ParamUIDesc(PB_USEMAP,TYPE_SINGLECHEKBOX,IDC_DISP_USEMAP),

	// Apply mapping
	ParamUIDesc(PB_APPLYMAP,TYPE_SINGLECHEKBOX,IDC_DISP_APPLYMAP),	
	
	// Texture map or vertex color channel?
	ParamUIDesc (PB_CHANNEL, TYPE_RADIO, chanIDs, 2),

	// Which map channel?
	ParamUIDesc (PB_MAPCHANNEL, EDITTYPE_POS_INT,
		IDC_DISP_CHAN, IDC_DISP_CHAN_SPIN, 1, MAX_MESHMAPS-1, SPIN_AUTOSCALE),
	};
#define PARAMDESC_LENGH 21
#define PARAMDESC_LENGH_SW 16

static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },	
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },	
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 }
};

static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },	
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },	
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_INT, NULL, FALSE, 6 }};

static ParamBlockDescID descVer2[] = {
	{ TYPE_INT, NULL, FALSE, 0 },		// map type
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// utile
	{ TYPE_FLOAT, NULL, TRUE, 2 },		// vtile
	{ TYPE_FLOAT, NULL, TRUE, 9 },		// wtile
	{ TYPE_FLOAT, NULL, TRUE, 7 },		// blur
	{ TYPE_INT, NULL, FALSE, 3 },		// use map
	{ TYPE_INT, NULL, FALSE, 4 },		// apply map
	{ TYPE_FLOAT, NULL, TRUE, 5 },		// strength
	{ TYPE_FLOAT, NULL, TRUE, 8 },		// decay
	{ TYPE_INT, NULL, FALSE, 6 },		// center lum
	{ TYPE_INT, NULL, FALSE, 10 },		// U flip
	{ TYPE_INT, NULL, FALSE, 11 },		// V flip
	{ TYPE_INT, NULL, FALSE, 12 },		// W flip	
	};		

static ParamBlockDescID descVer3[] = {
	{ TYPE_INT, NULL, FALSE, 0 },		// map type
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// utile
	{ TYPE_FLOAT, NULL, TRUE, 2 },		// vtile
	{ TYPE_FLOAT, NULL, TRUE, 9 },		// wtile
	{ TYPE_FLOAT, NULL, TRUE, 7 },		// blur
	{ TYPE_INT, NULL, FALSE, 3 },		// use map
	{ TYPE_INT, NULL, FALSE, 4 },		// apply map
	{ TYPE_FLOAT, NULL, TRUE, 5 },		// strength
	{ TYPE_FLOAT, NULL, TRUE, 8 },		// decay
	{ TYPE_INT, NULL, FALSE, 6 },		// center lum
	{ TYPE_INT, NULL, FALSE, 10 },		// U flip
	{ TYPE_INT, NULL, FALSE, 11 },		// V flip
	{ TYPE_INT, NULL, FALSE, 12 },		// W flip	
	{ TYPE_FLOAT, NULL, TRUE, 13 },		// center L
	};		

static ParamBlockDescID descVer4[] = {
	{ TYPE_INT, NULL, FALSE, 0 },		// map type
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// utile
	{ TYPE_FLOAT, NULL, TRUE, 2 },		// vtile
	{ TYPE_FLOAT, NULL, TRUE, 9 },		// wtile
	{ TYPE_FLOAT, NULL, TRUE, 7 },		// blur
	{ TYPE_INT, NULL, FALSE, 3 },		// use map
	{ TYPE_INT, NULL, FALSE, 4 },		// apply map
	{ TYPE_FLOAT, NULL, TRUE, 5 },		// strength
	{ TYPE_FLOAT, NULL, TRUE, 8 },		// decay
	{ TYPE_INT, NULL, FALSE, 6 },		// center lum
	{ TYPE_INT, NULL, FALSE, 10 },		// U flip
	{ TYPE_INT, NULL, FALSE, 11 },		// V flip
	{ TYPE_INT, NULL, FALSE, 12 },		// W flip	
	{ TYPE_FLOAT, NULL, TRUE, 13 },		// center L
	{ TYPE_INT, NULL, FALSE, 14 },		// cap
	};		

static ParamBlockDescID descVer5[] = {
	{ TYPE_INT, NULL, FALSE, 0 },		// map type
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// utile
	{ TYPE_FLOAT, NULL, TRUE, 2 },		// vtile
	{ TYPE_FLOAT, NULL, TRUE, 9 },		// wtile
	{ TYPE_FLOAT, NULL, TRUE, 7 },		// blur
	{ TYPE_INT, NULL, FALSE, 3 },		// use map
	{ TYPE_INT, NULL, FALSE, 4 },		// apply map
	{ TYPE_FLOAT, NULL, TRUE, 5 },		// strength
	{ TYPE_FLOAT, NULL, TRUE, 8 },		// decay
	{ TYPE_INT, NULL, FALSE, 6 },		// center lum
	{ TYPE_INT, NULL, FALSE, 10 },		// U flip
	{ TYPE_INT, NULL, FALSE, 11 },		// V flip
	{ TYPE_INT, NULL, FALSE, 12 },		// W flip	
	{ TYPE_FLOAT, NULL, TRUE, 13 },		// center L
	{ TYPE_INT, NULL, FALSE, 14 },		// cap
	{ TYPE_FLOAT, NULL, TRUE, 15 },		// length
	{ TYPE_FLOAT, NULL, TRUE, 16 },		// width
	{ TYPE_FLOAT, NULL, TRUE, 17 },		// height
	{ TYPE_INT, NULL, FALSE, 18 },		// axis
	};		

static ParamBlockDescID descVer6[] = {
	{ TYPE_INT, NULL, FALSE, 0 },		// map type
	{ TYPE_FLOAT, NULL, TRUE, 1 },		// utile
	{ TYPE_FLOAT, NULL, TRUE, 2 },		// vtile
	{ TYPE_FLOAT, NULL, TRUE, 9 },		// wtile
	{ TYPE_FLOAT, NULL, TRUE, 7 },		// blur
	{ TYPE_INT, NULL, FALSE, 3 },		// use map
	{ TYPE_INT, NULL, FALSE, 4 },		// apply map
	{ TYPE_FLOAT, NULL, TRUE, 5 },		// strength
	{ TYPE_FLOAT, NULL, TRUE, 8 },		// decay
	{ TYPE_INT, NULL, FALSE, 6 },		// center lum
	{ TYPE_INT, NULL, FALSE, 10 },		// U flip
	{ TYPE_INT, NULL, FALSE, 11 },		// V flip
	{ TYPE_INT, NULL, FALSE, 12 },		// W flip	
	{ TYPE_FLOAT, NULL, TRUE, 13 },		// center L
	{ TYPE_INT, NULL, FALSE, 14 },		// cap
	{ TYPE_FLOAT, NULL, TRUE, 15 },		// length
	{ TYPE_FLOAT, NULL, TRUE, 16 },		// width
	{ TYPE_FLOAT, NULL, TRUE, 17 },		// height
	{ TYPE_INT, NULL, FALSE, 18 },		// axis
	{ TYPE_INT, NULL, FALSE, PB_CHANNEL },
	{ TYPE_INT, NULL, FALSE, PB_MAPCHANNEL },
	};		

#define PBLOCK_LENGTH	21


// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,7,0),
	ParamVersionDesc(descVer1,9,1),
	ParamVersionDesc(descVer2,13,2),
	ParamVersionDesc(descVer3,14,3),
	ParamVersionDesc(descVer4,15,4),
	ParamVersionDesc(descVer5,19,5),
	};
#define NUM_OLDVERSIONS	6

// Current version
#define CURRENT_VERSION	6
static ParamVersionDesc curVersion(descVer6,PBLOCK_LENGTH,CURRENT_VERSION);



class BitmapRestore : public RestoreObj {
public:		
	DispMod *mpMod;
	BitmapInfo biRedo,biUndo;

    BitmapRestore(DispMod *pMod);

    void Restore(int isUndo);
    void Redo();
};

BitmapRestore::BitmapRestore(DispMod *pMod)
{
	mpMod = pMod;
	biUndo = pMod->bi;

}

void BitmapRestore::Restore(int isUndo)
{
	if (isUndo) {
		biRedo = mpMod->bi;

	}
	mpMod->bi = biUndo;
	mpMod->FreeBitmap();
	mpMod->LoadBitmap();
	mpMod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	if (mpMod->pmapParam) 
		SendMessage(mpMod->pmapParam->GetHWnd(),WM_COMMAND,IDC_SET_IMAGENAME,1);

}

void BitmapRestore::Redo()
{
	mpMod->bi = biRedo;
	mpMod->FreeBitmap();
	mpMod->LoadBitmap();
	mpMod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	if (mpMod->pmapParam) 
		SendMessage(mpMod->pmapParam->GetHWnd(),WM_COMMAND,IDC_SET_IMAGENAME,1);

}


class BitmapObjRestore : public RestoreObj {
public:		
	DisplaceObject *mpMod;
	BitmapInfo biRedo,biUndo;

    BitmapObjRestore(DisplaceObject *pMod);

    void Restore(int isUndo);
    void Redo();
};

BitmapObjRestore::BitmapObjRestore(DisplaceObject *pMod)
{
	mpMod = pMod;
	biUndo = pMod->bi;

}

void BitmapObjRestore::Restore(int isUndo)
{
	if (isUndo) {
		biRedo = mpMod->bi;

	}
	mpMod->bi = biUndo;
	mpMod->FreeBitmap();
	mpMod->LoadBitmap();
	mpMod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	if (mpMod->pmapParam) 
		SendMessage(mpMod->pmapParam->GetHWnd(),WM_COMMAND,IDC_SET_IMAGENAME,1);

}

void BitmapObjRestore::Redo()
{
	mpMod->bi = biRedo;
	mpMod->FreeBitmap();
	mpMod->LoadBitmap();
	mpMod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	if (mpMod->pmapParam) 
		SendMessage(mpMod->pmapParam->GetHWnd(),WM_COMMAND,IDC_SET_IMAGENAME,1);

}


//--- DispDlgProc -------------------------------------------------------

class DispDlgProc : public ParamMapUserDlgProc, DADMgr {
	public:
		HWND hWnd;
		DispMod *mod;
		DisplaceObject *obj;
		DispDlgProc(DispMod *m) {mod = m;obj = NULL;}
		DispDlgProc(DisplaceObject *o) {mod = NULL;obj = o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DoBitmapFit(HWND hWnd);
		void SetImageName(HWND hWnd);
		void PickImage(HWND hWnd);
		void RemoveImage(HWND hWnd);
		void SetMapName(HWND hWnd);
		void PickMap(HWND hWnd);
		void RemoveMap(HWND hWnd);
		void DeleteThis() {delete this;}
		void RedrawViews() { 
			Interface *ip = obj?obj->ip:mod->ip;
			ip->RedrawViews(ip->GetTime(),REDRAW_NORMAL);
			}
		// From DADMgr
		SClass_ID GetDragType(HWND hwnd, POINT p) {
			if (hwnd == GetDlgItem(hWnd,IDC_DISP_PICKMAP)) 
				return TEXMAP_CLASS_ID;
			if (hwnd == GetDlgItem(hWnd,IDC_DISP_PICKIMAGE)) 
				return BITMAPDAD_CLASS_ID;
			else return NULL;
			}		
		BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) {
			if (hfrom==hto) return FALSE;
			if (hto == GetDlgItem(hWnd,IDC_DISP_PICKMAP)) 
				return type==TEXMAP_CLASS_ID;
			if (hto == GetDlgItem(hWnd,IDC_DISP_PICKIMAGE)) 
        //aszabo|Nov.15.01 - Drop method does not handle dropping NULL ref targets, so don't allow dropping them
				return (type==BITMAPDAD_CLASS_ID && dropThis != NULL);
			return NULL;
			}
		int SlotOwner() {return OWNER_SCENE;}
		ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type) {
			if (hwnd == GetDlgItem(hWnd,IDC_DISP_PICKMAP)) {
				return obj? obj->tmap: mod->tmap;
				}
			if (hwnd == GetDlgItem(hWnd,IDC_DISP_PICKIMAGE)) {
				DADBitmapCarrier *bmc = GetDADBitmapCarrier();
				BitmapInfo *pbi = obj? &obj->bi: &mod->bi;
				TSTR nm = pbi->Name();
				bmc->SetName(nm);
				return bmc;
				}
			return NULL;
			}
		void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type) {
			if (hwnd == GetDlgItem(hWnd,IDC_DISP_PICKMAP)) {



				if (!theHold.Holding())
					theHold.Begin();
				if (mod) {
					mod->ReplaceReference(2,dropThis);
					theHold.Accept(GetString(IDS_PW_PICKMAP));
					mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
					mod->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
				} else {
					obj->ReplaceReference(1,dropThis);
					theHold.Accept(GetString(IDS_PW_PICKMAP));
					obj->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
					obj->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
					}
				RedrawViews();
				SetMapName(hWnd);
				}
			else if (hwnd == GetDlgItem(hWnd,IDC_DISP_PICKIMAGE)) {
				assert(dropThis != NULL);
				assert(dropThis->SuperClassID()==BITMAPDAD_CLASS_ID);
				DADBitmapCarrier *bmc = (DADBitmapCarrier*)dropThis;
				if (mod) {
					mod->bi.SetName(bmc->GetName());	
					mod->FreeBitmap();
					mod->LoadBitmap();
					mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
				} else {
					assert(obj);
					obj->bi.SetName(bmc->GetName());	
					obj->FreeBitmap();
					obj->LoadBitmap();
					obj->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
					}
				RedrawViews();
				SetImageName(hWnd);
				}
			}
		BOOL AutoTooltip() { return TRUE; }

		void DisableControls();

	};


static void GetBMName(BitmapInfo& bi, TSTR &fname) { 
	TSTR fullName;
	if (bi.Name()[0]==0)
		fullName = bi.Device();
	else 
		fullName =  bi.Name();
	SplitPathFile(fullName,NULL,&fname);
	}


void DispDlgProc::DisableControls()
	{
//enable tex map remove button
	BOOL bitmapRemove;
	BOOL texmapRemove;
	TSTR fname;
	if (mod) GetBMName(mod->bi, fname);
	if (obj) GetBMName(obj->bi, fname);
	
	if (fname.length()>0) 
		bitmapRemove = TRUE;
	else bitmapRemove = FALSE;

	if ((mod&&mod->tmap) || (obj&&obj->tmap)) 
		texmapRemove = TRUE;
	else texmapRemove = FALSE;


	EnableWindow(GetDlgItem(hWnd,IDC_DISP_REMOVEIMAGE),bitmapRemove);

	EnableWindow(GetDlgItem(hWnd,IDC_DISP_REMOVEMAP),texmapRemove);

//get mod
	int type;
	if (mod) mod->pblock->GetValue(PB_MAPTYPE,0,type,FOREVER);
	else if (obj) obj->pblock->GetValue(PB_MAPTYPE,0,type,FOREVER);

	BOOL cent;
	if (mod) mod->pblock->GetValue(PB_CENTERLUM,0,cent,FOREVER);
	else if (obj) obj->pblock->GetValue(PB_CENTERLUM,0,cent,FOREVER);

	BOOL w,h,l;
	w = TRUE;
	h = TRUE;
	l = TRUE;
	if (type == MAP_PLANAR) h = FALSE;
	if ( (type == MAP_SPHERICAL) || (type == MAP_BALL))
		{
		}

	BOOL existing = FALSE;
	if (mod) mod->pblock->GetValue(PB_USEMAP,0,existing,FOREVER);
	if (existing)
		{
		l = FALSE;
		w = FALSE;
		h = FALSE;

		}
	existing = !existing;
	EnableWindow(GetDlgItem(hWnd,IDC_DISP_PLANAR),existing);
	EnableWindow(GetDlgItem(hWnd,IDC_DISP_CYL),existing);
	EnableWindow(GetDlgItem(hWnd,IDC_DISP_SPHERE),existing);
	EnableWindow(GetDlgItem(hWnd,IDC_DISP_BALL),existing);
	if (mod) EnableWindow(GetDlgItem(hWnd,IDC_DISP_CAP),existing);


	ISpinnerControl *spin;

	spin = GetISpinner (GetDlgItem (hWnd, IDC_DISP_LENGTHSPIN));
	spin->Enable (l);
	ReleaseISpinner (spin);

	spin = GetISpinner (GetDlgItem (hWnd, IDC_DISP_WIDTHSPIN));
	spin->Enable (w);
	ReleaseISpinner (spin);

	spin = GetISpinner (GetDlgItem (hWnd, IDC_DISP_HEIGHTSPIN));
	spin->Enable (h);
	ReleaseISpinner (spin);

	spin = GetISpinner (GetDlgItem (hWnd, IDC_DISP_HEIGHTSPIN));
	spin->Enable (h);
	ReleaseISpinner (spin);

	spin = GetISpinner (GetDlgItem (hWnd, IDC_DISP_CENTERLSPIN));
	spin->Enable (cent);
	ReleaseISpinner (spin);

	}

void DispDlgProc::SetImageName(HWND hWnd)
	{ 	
	/*
	if ((mod&&mod->thebm) || (obj&&obj->thebm)) {
		TSTR fname;
		if (mod) 
			GetBMName(mod->bi, fname); 
		else 
			GetBMName(obj->bi, fname); 
		SetDlgItemText(hWnd, IDC_DISP_PICKIMAGE,fname);
	} else {
		SetDlgItemText(hWnd, IDC_DISP_PICKIMAGE, GetString(IDS_RB_NONE));
		}
	*/
	// RB 3/2/98: Display the name even if the bitmap can't be found.
	TSTR fname;
	if (mod) GetBMName(mod->bi, fname);
	if (obj) GetBMName(obj->bi, fname);
	
	if (fname.length()>0) 
		 SetDlgItemText(hWnd, IDC_DISP_PICKIMAGE,fname);
	else SetDlgItemText(hWnd, IDC_DISP_PICKIMAGE, GetString(IDS_RB_NONE));
	InvalidateRect(GetDlgItem(hWnd,IDC_DISP_PICKIMAGE),NULL,TRUE);
	
	// RB 3/16/99: This is the right way to set text for a custom button.
	// there seems to be a problem refreshing the button when SetDlgItemText() is used.
	/*
	ICustButton *but = GetICustButton(GetDlgItem(hWnd, IDC_DISP_PICKIMAGE));
	if (but) {
		if (fname.length()>0) 
			 but->SetText(fname);
		else but->SetText(GetString(IDS_RB_NONE));
		ReleaseICustButton(but);
		}
		*/
	}

void DispDlgProc::PickImage(HWND hWnd)
	{	
	

	BitmapInfo *bi = mod ? &mod->bi : &obj->bi;
	if(TheManager->SelectFileInputEx(
		bi, GetCOREInterface()->GetMAXHWnd(), GetString(IDS_RB_SELECTDISPIMAGE)))
		{
		if (mod) {
			mod->FreeBitmap();
			mod->LoadBitmap();
			mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
		} else {
			obj->FreeBitmap();
			obj->LoadBitmap();
			obj->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
			}
		SetImageName(hWnd);
		}
	

	}

void DispDlgProc::RemoveImage(HWND hWnd)
	{
	
	if (mod) {
		mod->bi.SetName(_T(""));
		mod->bi.SetDevice(_T(""));
		mod->FreeBitmap();
		mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
	} else {
		obj->bi.SetName(_T(""));
		obj->bi.SetDevice(_T(""));
		obj->FreeBitmap();
		obj->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
		}
	SetImageName(hWnd);
	
	}

void DispDlgProc::SetMapName(HWND hWnd)
	{
	if ((mod&&mod->tmap) || (obj&&obj->tmap)) {
		TSTR fname;
		if (mod)  fname = mod->tmap->GetFullName();
		else      fname = obj->tmap->GetFullName();		
		SetDlgItemText(hWnd, IDC_DISP_PICKMAP,fname);
		EnableWindow(GetDlgItem(hWnd,IDC_DISP_REMOVEMAP),TRUE);
		InvalidateRect(GetDlgItem(hWnd,IDC_DISP_PICKMAP),NULL,TRUE);
	} else {
		SetDlgItemText(hWnd, IDC_DISP_PICKMAP, GetString(IDS_RB_NONE));
		EnableWindow(GetDlgItem(hWnd,IDC_DISP_REMOVEMAP),FALSE);
		InvalidateRect(GetDlgItem(hWnd,IDC_DISP_PICKMAP),NULL,TRUE);
		}
	}

void DispDlgProc::PickMap(HWND hWnd)
	{
	
	BOOL newMat=FALSE, cancel=FALSE;
	Interface *ip = mod?mod->ip:obj->ip;
	MtlBase *mtl = ip->DoMaterialBrowseDlg(
		GetCOREInterface()->GetMAXHWnd(),
		BROWSE_MAPSONLY|BROWSE_INCNONE|BROWSE_INSTANCEONLY,
		newMat,cancel);
	if (cancel) {
		if (newMat) mtl->DeleteThis();
		return;
		}
	if (mod) {
		mod->ReplaceReference(2,mtl);
		mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
		mod->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	} else {
		obj->ReplaceReference(1,mtl);
		obj->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
		obj->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
	SetMapName(hWnd);
	

	}

void DispDlgProc::RemoveMap(HWND hWnd)
	{
	
     // Declare this as references since we want to manipulate the
     // orignal material library and not a copy.

	MtlBaseLib &mlib1 = GetCOREInterface()->GetMaterialLibrary();
 
	int index = -1;
	if ( (mod) && (mod->tmap) )
		mlib1.FindMtlByName(mod->tmap->GetName());
	else if ( (obj) && (obj->tmap)) mlib1.FindMtlByName(obj->tmap->GetName());

	if (index == -1) 
	   {
		if ( (mod) && (mod->tmap) )
			mlib1.Add(mod->tmap);
		else if ( (obj) && (obj->tmap)) mlib1.Add(obj->tmap);
		}

	if (mod) {
		theHold.Begin();
		mod->DeleteReference(2);
		theHold.Accept(GetString(IDS_PW_REMOVEMAP));
		mod->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
		mod->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	} else {
		theHold.Begin();
		obj->DeleteReference(1);
		theHold.Accept(GetString(IDS_PW_REMOVEMAP));
		obj->NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
		obj->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
	SetMapName(hWnd);
	
	}

void DispDlgProc::DoBitmapFit(HWND hWnd)
	{
	BitmapInfo bi;
	if (TheManager->SelectFileInputEx(
		&bi, GetCOREInterface()->GetMAXHWnd(), GetString(IDS_RB_SELECTIMAGE)))
		{		
		TheManager->GetImageInfo(&bi);
		mod->aspect = bi.Aspect() * float(bi.Width())/float(bi.Height());
		mod->flags |= CONTROL_ASPECT|CONTROL_HOLD;
		mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
		}
	}

BOOL DispDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			this->hWnd = hWnd;
			SendMessage(GetDlgItem(hWnd,IDC_DISP_FITREGION),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SendMessage(GetDlgItem(hWnd,IDC_DISP_FITREGION),CC_COMMAND,CC_CMD_HILITE_COLOR,GREEN_WASH);
			SendMessage(GetDlgItem(hWnd,IDC_DISP_NORMALALIGN),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SendMessage(GetDlgItem(hWnd,IDC_DISP_NORMALALIGN),CC_COMMAND,CC_CMD_HILITE_COLOR,GREEN_WASH);
			SendMessage(GetDlgItem(hWnd,IDC_DISP_ACQUIRE),CC_COMMAND,CC_CMD_SET_TYPE,CBT_CHECK);
			SendMessage(GetDlgItem(hWnd,IDC_DISP_ACQUIRE),CC_COMMAND,CC_CMD_HILITE_COLOR,GREEN_WASH);
			SetImageName(hWnd);
			SetMapName(hWnd);
			ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_DISP_PICKMAP));
			but->SetDADMgr(this);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_DISP_PICKIMAGE));
			but->SetDADMgr(this);
			ReleaseICustButton(but);
			DisableControls();
			break;
			}
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_DISP_CENTERLUM:
				case IDC_DISP_PLANAR:
				case IDC_DISP_CYL:
				case IDC_DISP_USEMAP:
					DisableControls();
					break;
				case IDC_DISP_SPHERE:
				case IDC_DISP_BALL:
					DisableControls();
					if (!mod) break;
					mod->flags |= CONTROL_UNIFORM|CONTROL_HOLD;
					mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
					break;

				case IDC_DISP_FIT:
					if (!mod) break;
					mod->flags |= CONTROL_FIT|CONTROL_HOLD;
					mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
					break;

				case IDC_DISP_CENTER:
					if (!mod) break;
					mod->flags |= CONTROL_CENTER|CONTROL_HOLD;
					mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
					break;

				case IDC_DISP_BITMAPFIT:
					if (!mod) break;
					DoBitmapFit(hWnd);
					break;

				case IDC_DISP_ACQUIRE:
					mod->ip->SetPickMode(&mod->pickAcquire);					
					break;

				case IDC_DISP_NORMALALIGN:
					if (!mod) break;
					if (mod->ip->GetCommandMode()->ID()==CID_FACEALIGNMAP) {
						mod->ip->SetStdCommandMode(CID_OBJMOVE);
					} else {
						mod->ip->SetCommandMode(mod->faceAlignMode);
						}
					break;

				case IDC_DISP_RESET:
					if (!mod) break;
					theHold.Begin();
					mod->ReplaceReference(TM_REF,NULL);
					mod->flags |= CONTROL_FIT|CONTROL_CENTER|CONTROL_INIT;
					theHold.Accept(0);
					mod->NotifyDependents(FOREVER,PART_TEXMAP,REFMSG_CHANGE);
					break;
				
				case IDC_SET_MAPNAME:
					SetMapName(hWnd);
					break;

				case IDC_SET_IMAGENAME:
					SetImageName(hWnd);
					break;

				case IDC_DISP_PICKIMAGE:
					theHold.Begin();
					if (mod) theHold.Put(new BitmapRestore(mod));
					else if (obj) theHold.Put(new BitmapObjRestore(obj));
					PickImage(hWnd);
					theHold.Accept(GetString(IDS_PW_PICKMAP));
					DisableControls();
					break;
				
				case IDC_DISP_REMOVEIMAGE:
					theHold.Begin();
					if (mod) theHold.Put(new BitmapRestore(mod));
					else if (obj) theHold.Put(new BitmapObjRestore(obj));
					RemoveImage(hWnd);
					theHold.Accept(GetString(IDS_PW_REMOVEMAP));
					DisableControls();

					break;

				case IDC_DISP_PICKMAP:
					macroRecorder->Disable();
					theHold.Begin();
					PickMap(hWnd);
					theHold.Accept(GetString(IDS_PW_PICKMAP));
					DisableControls();
					macroRecorder->Enable();
					break;
				
				case IDC_DISP_REMOVEMAP:
					
					RemoveMap(hWnd);
					
					DisableControls();
					break;

				case IDC_DISP_VIEWALIGN:
					theHold.Begin();
					mod->ViewportAlign();
					theHold.Accept(0);
					break;

				case IDC_DISP_FITREGION:
					if (!mod) break;
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

//--- Displace methods -------------------------------

DispMod::DispMod(BOOL create)
	{	
	tmap = NULL;
	SetAFlag(A_PLUGIN1);
	if (create) flags = CONTROL_CENTER|CONTROL_FIT|CONTROL_INIT;
	else flags = 0;

	MakeRefByID(FOREVER, PBLOCK_REF, 
		CreateParameterBlock(descVer6, PBLOCK_LENGTH, CURRENT_VERSION));	
	
	thebm = NULL;
	pblock->SetValue(PB_UTILE,0,1.0f);
	pblock->SetValue(PB_VTILE,0,1.0f);
	pblock->SetValue(PB_WTILE,0,1.0f);
	pblock->SetValue(PB_CENTERL,0,0.5f);
	pblock->SetValue(PB_LENGTH,0,1.0f);
	pblock->SetValue(PB_WIDTH,0,1.0f);
	pblock->SetValue(PB_HEIGHT,0,1.0f);
	pblock->SetValue(PB_AXIS,0,2);
	pblock->SetValue (PB_MAPCHANNEL, 0, 1);
	tmControl = NULL;
	}

DispMod::~DispMod()
	{
	FreeBitmap();
	}

void DispMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
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

	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_DISPLACEPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);		
 	pmapParam->SetUserDlgProc(new DispDlgProc(this));

 	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
	}
		
void DispMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
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
	}

void DispMod::EnterNormalAlign()
	{
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_DISP_NORMALALIGN),CC_COMMAND,CC_CMD_SET_STATE,1);
	}

void DispMod::ExitNormalAlign()
	{
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_DISP_NORMALALIGN),CC_COMMAND,CC_CMD_SET_STATE,0);
	}

void DispMod::EnterRegionFit()
	{
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_DISP_FITREGION),CC_COMMAND,CC_CMD_SET_STATE,1);
	}

void DispMod::ExitRegionFit()
	{
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_DISP_FITREGION),CC_COMMAND,CC_CMD_SET_STATE,0);
	}

void DispMod::EnterAcquire()
	{
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_DISP_ACQUIRE),CC_COMMAND,CC_CMD_SET_STATE,1);
	}

void DispMod::ExitAcquire()
	{
	SendMessage(GetDlgItem(pmapParam->GetHWnd(),IDC_DISP_ACQUIRE),CC_COMMAND,CC_CMD_SET_STATE,0);
	}

int DispMod::GetMapType()
	{
	int type;
	pblock->GetValue(PB_MAPTYPE,0,type,FOREVER);
	return type;
	}

void DispMod::SetMapType(int type)
	{
	switch (type) {
		case MAP_PLANAR:
		case MAP_CYLINDRICAL:
		case MAP_SPHERICAL:
		case MAP_BALL:
			pblock->SetValue(PB_MAPTYPE,0,type);
			if (pmapParam) pmapParam->Invalidate();
			break;
		}
	}

int DispMod::GetMapChannel ()
	{
	int chan;
	pblock->GetValue(PB_CHANNEL,0,chan,FOREVER);
	if (chan) return 0;
	pblock->GetValue (PB_MAPCHANNEL, 0, chan, FOREVER);
	return chan;
	}

void DispMod::SetMapChannel (int chan)
	{
	if (!chan) {
		pblock->SetValue (PB_CHANNEL, 0, 1);
	} else {
		pblock->SetValue (PB_CHANNEL, 0, 0);
		pblock->SetValue (PB_MAPCHANNEL, 0, chan);
	}
}

float DispMod::GetTile(TimeValue t,int which)
	{
	float tile;
	pblock->GetValue(PB_UTILE+which,t,tile,FOREVER);
	return tile;
	}

void DispMod::SetTile(TimeValue t,int which, float tile)
	{
	pblock->SetValue(PB_UTILE+which,t,tile);
	if (pmapParam) pmapParam->Invalidate();
	}

BOOL DispMod::GetFlip(int which)
	{
	int flip;
	pblock->GetValue(PB_UFLIP+which,0,flip,FOREVER);
	return flip;
	}

void DispMod::SetFlip(int which,BOOL flip)
	{
	pblock->SetValue(PB_UFLIP+which,0,flip);
	if (pmapParam) pmapParam->Invalidate();
	}

float DispMod::GetLength(TimeValue t)
	{
	float f;
	pblock->GetValue(PB_LENGTH,t,f,FOREVER);
	return f;
	}

float DispMod::GetWidth(TimeValue t)
	{
	float f;
	pblock->GetValue(PB_WIDTH,t,f,FOREVER);
	return f;
	}

float DispMod::GetHeight(TimeValue t)
	{
	float f;
	pblock->GetValue(PB_HEIGHT,t,f,FOREVER);
	return f;
	}

int DispMod::GetAxis()
	{
	int a;
	pblock->GetValue(PB_AXIS,0,a,FOREVER);
	return a;
	}

void DispMod::SetLength(TimeValue t,float v)
	{
	pblock->SetValue(PB_LENGTH,t,v);
	}

void DispMod::SetWidth(TimeValue t,float v)
	{
	pblock->SetValue(PB_WIDTH,t,v);
	}

void DispMod::SetHeight(TimeValue t,float v)
	{
	pblock->SetValue(PB_HEIGHT,t,v);
	}

void DispMod::SetAxis(int v)
	{
	pblock->SetValue(PB_AXIS,0,v);
	}

Interval DispMod::LocalValidity(TimeValue t)
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
Interval DispMod::GetValidity(TimeValue t)
{
	float f;		
	Interval valid = FOREVER;
	pblock->GetValue(PB_UTILE,t,f,valid);
	pblock->GetValue(PB_VTILE,t,f,valid);
	pblock->GetValue(PB_WTILE,t,f,valid);
	pblock->GetValue(PB_STRENGTH,t,f,valid);	
	pblock->GetValue(PB_DECAY,t,f,valid);
	pblock->GetValue(PB_BLUR,t,f,valid);
	pblock->GetValue(PB_CENTERL,t,f,valid);
	pblock->GetValue(PB_LENGTH,t,f,valid);
	pblock->GetValue(PB_WIDTH,t,f,valid);
	pblock->GetValue(PB_HEIGHT,t,f,valid);	
	if (thebm) {
		if (bi.FirstFrame()!=bi.LastFrame()) {
			valid.SetInstant(t);
			return valid;
			}
		}
	if (tmap) {
		valid &= tmap->Validity(t);
		}
	if (tmControl) {
		Matrix3 tm(1);
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
		}
	return valid;
}

RefTargetHandle DispMod::Clone(RemapDir& remap) 
	{
	DispMod* newmod = new DispMod(FALSE);	
	if (TestAFlag(A_PLUGIN1)) 
		 newmod->SetAFlag(A_PLUGIN1);
	else newmod->ClearAFlag(A_PLUGIN1);
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));	
	if (tmap) newmod->ReplaceReference(2,tmap);
	if (tmControl) {
		newmod->MakeRefByID(FOREVER,TM_REF,tmControl->Clone(remap));
		}
	newmod->bi = bi;
	if (bi.Name()[0]!=0||bi.Device()[0]!=0) {
		newmod->LoadBitmap();
		}
	newmod->aspect = aspect;
	newmod->flags  = flags;
	BaseClone(this, newmod, remap);
	return newmod;
	}

static void SetTMapTime(TimeValue t,Texmap *tmap)
	{
	if (!tmap) return;
	tmap->LoadMapFiles(t);
	tmap->Update(t,FOREVER);
	for (int i=0; i<tmap->NumSubTexmaps(); i++) {
		SetTMapTime(t,tmap->GetSubTexmap(i));
		}
	}

void DispMod::SetImageTime(TimeValue t)
	{
	if (thebm) {
		int length = bi.LastFrame()-bi.FirstFrame()+1;
		int frame = (t/GetTicksPerFrame())%length;
		if (frame<0) frame += length;
		frame += bi.FirstFrame();
		bi.SetCurrentFrame(frame);
		thebm->GoTo(&bi);
		}
	if (tmap) {
		SetTMapTime(t,tmap);
		}
	}

void DispMod::LoadBitmap() 
	{

	BOOL silentMode = TheManager->SilentMode();
	BOOL netRendering = GetCOREInterface()->GetQuietMode();
	if (netRendering) 
		 TheManager->SetSilentMode(TRUE);
	if (bi.Name()[0]==0&&bi.Device()[0]==0) 
		{
		TheManager->SetSilentMode(silentMode);
		return;
		}

	BMMRES errorReturn;
	thebm = TheManager->Load(&bi,&errorReturn);
	if ((errorReturn != BMMRES_SUCCESS) && (netRendering))
		{
		if (errorReturn==BMMRES_FILENOTFOUND)
			GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,_T("%s: %s"),GetString(IDS_PW_MAPMISSING),bi.Name());
		else GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,_T("%s: %s"),GetString(IDS_PW_MAPERROR),bi.Name());
		}	
	TheManager->SetSilentMode(silentMode);
	}

void DispMod::FreeBitmap()
	{
	if (thebm) {
		thebm->DeleteThis();
		thebm = NULL;
		}
	}

BOOL DispMod::AssignController(Animatable *control,int subAnim)
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

int DispMod::SubNumToRefNum(int subNum)
	{
	switch (subNum) {
		case TM_REF: return TM_REF;
		case 2:      return 2;
		default:     return -1;
		}
	}

RefTargetHandle DispMod::GetReference(int i)
	{
	switch (i) {
		case TM_REF: 		return tmControl;
		case PBLOCK_REF:	return pblock;
		case 2:             return tmap;
		default: 			return NULL;
		}
	}

void DispMod::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case TM_REF: 		tmControl = (Control*)rtarg; break;
		case PBLOCK_REF:	pblock = (IParamBlock*)rtarg; break;
		case 2:             
			tmap = (Texmap*)rtarg; 
			if (editMod==this && pmapParam) {
				((DispDlgProc*)pmapParam->GetUserDlgProc())->SetMapName(
					pmapParam->GetHWnd());
				}
			break;
		}
	}

Animatable* DispMod::SubAnim(int i)
	{
	switch (i) {
		case TM_REF: 		return tmControl;
		case PBLOCK_REF:	return pblock;
		case 2:             return tmap;
		default: 			return NULL;   
		}
	}

TSTR DispMod::SubAnimName(int i)
	{
	switch (i) {
		case TM_REF: 		return GetString(IDS_RB_GIZMO);
		case PBLOCK_REF:	return GetString(IDS_RB_PARAMETERS);
		case 2:             return _T("Displacement Map");
		default: 			return TSTR(_T(""));
		}
	}

RefResult DispMod::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			if (editMod==this && pmapParam) 
				{
				pmapParam->Invalidate();
				((DispDlgProc*)pmapParam->GetUserDlgProc())->DisableControls();
				}
			if (tmap == hTarget)
				{
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}

			break;

		case REFMSG_NODE_NAMECHANGE:
			if ((ip) && (pmapParam) )
				SendMessage(pmapParam->GetHWnd(),WM_COMMAND,IDC_SET_MAPNAME,1);
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_LENGTH:
				case PB_WIDTH:
				case PB_HEIGHT:
				case PB_STRENGTH:	gpd->dim = stdWorldDim; break;
				default:			gpd->dim = defaultDim; break;
				}			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_UTILE:		gpn->name = GetString(IDS_RB_UTILE); break;
				case PB_VTILE:		gpn->name = GetString(IDS_RB_VTILE); break;
				case PB_WTILE:		gpn->name = GetString(IDS_RB_WTILE); break;
				case PB_STRENGTH:	gpn->name = GetString(IDS_RB_STRENGTH2); break;
				case PB_BLUR:		gpn->name = GetString(IDS_RB_BLUR); break;
				case PB_DECAY:		gpn->name = GetString(IDS_RB_DECAY); break;
				case PB_CENTERL:	gpn->name = GetString(IDS_RB_CENTERPOINT); break;
				case PB_LENGTH:		gpn->name = GetString(IDS_RB_LENGTH); break;
				case PB_WIDTH:		gpn->name = GetString(IDS_RB_WIDTH); break;
				case PB_HEIGHT:		gpn->name = GetString(IDS_RB_HEIGHT); break;
				case PB_CHANNEL: gpn->name = GetString (IDS_UVWX_CHANNEL); break;
				case PB_MAPCHANNEL: gpn->name = GetString (IDS_UVWX_MAPCHANNEL); break;
				default:			gpn->name = TSTR(_T("")); break;
				}
			return REF_STOP; 
			}
		}
	return REF_SUCCEED;
	}


void DispMod::ActivateSubobjSel(int level, XFormModes& modes )
	{	
	switch (level) {
		case 1: // Modifier box
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,NULL);
			break;		
		}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}


// Following is necessary to make sure mapchannel doesn't come in as zero;
// this happens when loading up an old scene -- the pblock with the defaults
// is overwritten.
class DispSetChannelToOne : public PostLoadCallback {
public:
	DispMod *mm;
	DispSetChannelToOne (DispMod *mapMod) { mm = mapMod; }
	void proc (ILoad *iload) {
		if (mm && mm->pblock) {
			int mapChan;
			mm->pblock->GetValue (PB_MAPCHANNEL, 0, mapChan, FOREVER);
			if (!mapChan) mm->pblock->SetValue (PB_MAPCHANNEL, 0, 1);
		}
		delete this;
	}
};

#define MAPNAME_CHUNK	0x0100
#define BMIO_CHUNK	0x0200
#define NEWMAP_CHUNKID	0x0300

IOResult DispMod::Load(ILoad *iload)
	{	
	IOResult res = IO_OK;
	Modifier::Load(iload);

	iload->RegisterPostLoadCallback(
		new FixDimsPLCB(this));

	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,PBLOCK_REF));

	iload->RegisterPostLoadCallback (new DispSetChannelToOne(this));

	ClearAFlag(A_PLUGIN1);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case MAPNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				bi.SetName(buf);
				FreeBitmap();
				LoadBitmap();
				break;
				}
			case BMIO_CHUNK: 
				res = bi.Load(iload);
				FreeBitmap();
				LoadBitmap();
				break;

			case NEWMAP_CHUNKID:
				SetAFlag(A_PLUGIN1);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}	
	
	return IO_OK;
	}

IOResult DispMod::Save(ISave *isave)
	{
	Modifier::Save(isave);
	isave->BeginChunk(BMIO_CHUNK);
	bi.Save(isave);
	isave->EndChunk();
	if (TestAFlag(A_PLUGIN1)) {
		isave->BeginChunk(NEWMAP_CHUNKID);
		isave->EndChunk();
		}
	return IO_OK;
	}

int DispMod::NumSubObjTypes() 
{ 
	return 1;
}

ISubObjType *DispMod::GetSubObjType(int i) 
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


class DispDeformer: public Deformer {
	public:
		TimeValue curTime;
		Matrix3 tm,invtm;
		Bitmap *thebm;
		Texmap *tmap;
		int type, uflip, vflip, cap;
		float utile, vtile;
		float offset, mult, blur, decay;		
		Tab<Point3> *existing; // RB 3/26/99: Changed to Point3 from Point2
		Tab<Point3> *normals;		
		
		DispDeformer() {}
		DispDeformer(
			Matrix3 &tm, Matrix3 &invtm, 
			Bitmap *bm, Texmap *tmap, int type,
			float strength, float utile, float vtile,
			int uflip, int vflip,
			float blur, float decay,
			int centerLum, float center, int cap,
			Tab<Point3> *existing, // RB 3/26/99: Changed to Point3 from Point2
			Tab<Point3> *normals,
			TimeValue t);
		Point3 Map(int i, Point3 p); 
		Point3 MapForce(Point3 p); 
		Point2 MapPoint(Point3 p,Point3 &v,float &dist,Point3 norm);
		float GetPixel(Point3 pt,float u, float v, float w); // RB 3/26/99: Added w
		BOOL InRange(Point2 &uv);
	};

DispDeformer::DispDeformer(
		Matrix3 &tm, Matrix3 &invtm, 
		Bitmap *bm, Texmap *tmap, int type,
		float strength, float utile, float vtile,
		int uflip, int vflip,
		float blur, float decay,
		int centerLum, float center, int cap,
		Tab<Point3> *existing, // RB 3/26/99: Changed to Point3 from Point2
		Tab<Point3> *normals,
		TimeValue t)
	{
	this->tm       = tm;
	this->invtm    = invtm;
	thebm          = bm;
	this->type     = type;
	this->utile    = utile;
	this->vtile    = vtile;
	this->uflip    = uflip;
	this->vflip    = vflip;
	this->blur     = blur;
	this->decay    = decay;
	this->existing = existing;
	this->normals  = normals;
	this->tmap     = tmap;	
	this->cap      = cap;
	curTime = t;

	mult = strength;
	if (centerLum) {
		offset = -center; //-0.5f;
	} else {
		offset = 0.0f;
		}

	}

static int LargestComponent(Point3 &p)
	{
	int l = 0;
	if (fabs(p.y)>fabs(p[l])) l = 1;
	if (fabs(p.z)>fabs(p[l])) l = 2;
	return l;
	}

Point2 DispDeformer::MapPoint(Point3 p,Point3 &v, float &dist,Point3 norm)
	{
	switch (type) {		
		case MAP_PLANAR:
			v    = Point3(0,0,1);
			dist = (float)fabs(p.z);
			return Point2((p.x+1.0f)/2.0f,(p.y+1.0f)/2.0f);
		
		case MAP_BALL:
		case MAP_SPHERICAL: {
			Point2 uv;
			dist = Length(p);
			if (dist==0.0f) dist = 1.0f;
			v = p/dist;
			if (!thebm && !tmap) return Point2(0.5f,0.5f);
			float a1 = (float)atan2(v.x,v.y);
			float a2 = (float)asin(v.z);
			
			if (type==MAP_BALL) {
				float r = 0.5f-(a2+HALFPI)/TWOPI;
				uv.x = -(0.5f + r * (float)cos(a1+HALFPI));
				uv.y = 0.5f + r * (float)sin(a1+HALFPI);				
			} else {
				//uv.x = a1/TWOPI;
				uv.x = -(a1+HALFPI)/TWOPI;
				uv.y = (a2+HALFPI)/PI;
				}
			return uv;
			}				

		case MAP_CYLINDRICAL: {			
			Point2 uv;
			dist = Length(Point3(p.x, p.y, 0.0f));
			if (dist==0.0f) dist = 1.0f;
			v = Point3(p.x/dist, p.y/dist, 0.0f);
			if (!thebm && !tmap) return Point2(0.5f,p.z+0.5f);
			if (cap && (LargestComponent(norm)==2)) {
				if (norm.z<0) v = Point3(0,0,-1);
				else          v = Point3(0,0,1);
				// Do a planar mapping on the cap.				
				return Point2((p.x+1.0f)/2.0f,(p.y+1.0f)/2.0f);
			} else {
				float a1 = (float)atan2(v.x,v.y);
				//uv.x = a1/TWOPI;
				uv.x = -(a1+HALFPI)/TWOPI;
				uv.y = p.z+0.5f;
				return uv;
				}			
			}
		}
	//assert(0);
	return Point2(0.0f,0.0f);
	}

float DispDeformer::GetPixel(Point3 pt,float u, float v, float w)
	{
	if (!thebm && !tmap) return 1.0f;
	float f = 0.0f;
	if (thebm) {
		BMM_Color_64 c;
		if (blur==0.0f) {
			int w = thebm->Width()-1;
			int h = thebm->Height()-1;
			int x = (int)(frac(u)*(float)w);
			int y = h - (int)(frac(v)*(float)h);
			thebm->GetLinearPixels(x,y,1,&c);
		} else {		
			float wb = (blur*10.0f+1.0f)/float(thebm->Width());
			float hb = (blur*10.0f+1.0f)/float(thebm->Height());
			thebm->GetFiltered(frac(u),1.0f-frac(v),wb,hb,&c);
			}
		f += float(c.r)/float(0xFFFF) * 0.299f;
		f += float(c.g)/float(0xFFFF) * 0.587f;
		f += float(c.b)/float(0xFFFF) * 0.114f;		
		} 
	if (tmap) {
		SCTex shadeContext;
		shadeContext.scale = 1.0f;
		shadeContext.duvw  = Point3(blur/10.0f+0.001f,blur/10.0f+0.001f,blur/10.0f+0.001f/*0.0f*/);
		shadeContext.scrPos.x = int(u);
		shadeContext.scrPos.y = int(v);		
		shadeContext.uvw.x = u;
		shadeContext.uvw.y = v;
		shadeContext.uvw.z = w; //0.0f; RB 3/26/99: Don't want to loose w since 3D textures need it.
		shadeContext.pt = pt;
		shadeContext.dpt = Point3(blur/10.0f+0.001f,blur/10.0f+0.001f,blur/10.0f+0.001f);
		shadeContext.curTime = curTime;
		AColor c;
		c  = tmap->EvalColor(shadeContext);
		f += c.r * 0.299f;
		f += c.g * 0.587f;
		f += c.b * 0.114f;		
		}
	return f;
	}

BOOL DispDeformer::InRange(Point2 &uv)
	{
	if (existing) return TRUE;

	switch (type) {
		case MAP_PLANAR:
			if (uv.x<0.0f || uv.x>1.0f || uv.y<0.0f || uv.y>1.0f)
				 return FALSE;
			else return TRUE;
		
		case MAP_BALL:
		case MAP_SPHERICAL:
			return TRUE;

		case MAP_CYLINDRICAL:
			if (uv.y<0.0f || uv.y>1.0f) return FALSE;
			else return TRUE;
		}
	
	return FALSE;
	}

Point3 DispDeformer::Map(int i, Point3 p)
	{
	Point3 v, op=p;
	Point3 uv; // RB 3/26/99: Changed to Point3 from Point2
	float s, dist, m;

	p = p * tm;	
	if (existing) {
		uv   = existing->operator[](i);
		dist = Length(p);
		v    = normals->operator[](i);
	} else {		
		//uv = MapPoint(p,v,dist,normals?normals->operator[](i):Point3(1,0,0));
		Point2 uv2 = MapPoint(p,v,dist,normals?normals->operator[](i):Point3(1,0,0));
		uv = Point3(uv2.x, uv2.y, 0.0f);
		}
	if (!InRange(Point2(uv.x, uv.y))) 
		return p * invtm;
	if (decay!=0.0f) m = mult * (float)exp(-decay*dist);
	else m = mult;
	if (uflip) uv.x = 1.0f-uv.x;
	if (vflip) uv.y = 1.0f-uv.y;
	s = GetPixel(op, uv.x * utile, uv.y * vtile, uv.z); // RB 3/26/99: Added z component to support 3D textures. Do we need a z tile param?
	s = (s+offset)*m;	
	p = p * invtm;
	if (!existing) {
		v = Normalize(VectorTransform(invtm,v));
		}

	p += s*v;
	return p;
	}

// This is an adjustment to forces to make the independent of time scale.
// They were previously dependent on the old 1200 ticks per second constant.
// Note that the constants are being squared because we are dealing with acceleration not velocity.
static float forceScaleFactor = float(1200*1200)/float(TIME_TICKSPERSEC*TIME_TICKSPERSEC);

Point3 DispDeformer::MapForce(Point3 p)
	{
	Point3 v, op=p;
	Point2 uv;
	float s, dist, m;

	p = p * tm;	
	uv = MapPoint(p,v,dist,Point3(1,0,0));
	if (!InRange(uv)) return Point3(0,0,0);
	if (decay!=0.0f) m = mult * (float)exp(-decay*dist);
	else m = mult;
	if (m>9999999.0f) m = 9999999.0f;
	if (m<-9999999.0f) m = -9999999.0f;
	s = GetPixel(op, uv.x * utile, uv.y * vtile, 0.0f);
	s = (s+offset)*m;	
	v = Normalize(VectorTransform(invtm,v));
	return s*v * 0.00001f * forceScaleFactor;	
	}



void DispMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{	
	int type, centerLum, uflip, vflip, wflip, apply, useExisting, cap;
	float strength, utile, vtile, wtile, blur, decay, center;

	pblock->GetValue(PB_MAPTYPE,t,type,FOREVER);
	pblock->GetValue(PB_CENTERLUM,t,centerLum,FOREVER);
	pblock->GetValue(PB_UTILE,t,utile,FOREVER);
	pblock->GetValue(PB_VTILE,t,vtile,FOREVER);
	pblock->GetValue(PB_WTILE,t,wtile,FOREVER);
	pblock->GetValue(PB_UFLIP,t,uflip,FOREVER);
	pblock->GetValue(PB_VFLIP,t,vflip,FOREVER);
	pblock->GetValue(PB_WFLIP,t,wflip,FOREVER);
	pblock->GetValue(PB_STRENGTH,t,strength,FOREVER);
	pblock->GetValue(PB_BLUR,t,blur,FOREVER);
	pblock->GetValue(PB_DECAY,t,decay,FOREVER);	
	pblock->GetValue(PB_APPLYMAP,t,apply,FOREVER);	
	pblock->GetValue(PB_USEMAP,t,useExisting,FOREVER);	
	pblock->GetValue(PB_CENTERL,t,center,FOREVER);	
	pblock->GetValue(PB_CAP,t,cap,FOREVER);
	int chan = GetMapChannel ();

	// Set the image to the correct frame
	SetImageTime(t);

	// Prepare the controller and set up mats
	if (!tmControl || (flags&CONTROL_OP) || (flags&CONTROL_INITPARAMS)) 
		InitControl(mc,os->obj,type,t);	

	Interval valid = GetValidity(t);
	Matrix3 tm, invtm;	
	invtm = CompMatrix(t,&mc,NULL);
	tm    = Inverse(invtm);		

	if (thebm) {
		if (blur==0.0f) {
			thebm->SetFilter(BMM_FILTER_NONE);
		} else {
			thebm->SetFilter(BMM_FILTER_SUM);
			}
		}

	// If we're going to use existing mapping coords, we'll need to
	// build a table that corrisponds witht he points of this object.
	Tab<Point3> existing; // RB 3/26/99: Changed to Point3 from Point2
	Tab<Point3> normals;

	if ((cap||useExisting) && os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *obj = (TriObject*)os->obj;
		Mesh &mesh = obj->GetMesh();
		TVFace *tf = mesh.mapFaces (chan);
		UVVert *tv = mesh.mapVerts (chan);		
		if (tf || cap) { 
			// RB 2/12/99: Added "&& useExisting" to "if (tf)" conditions because we may be
			// in this block of code ONLY because cap is on (so we need normals) but we don't
			// want to confuse things so that we and up using existing (which is what was happening).

			if (tf && useExisting) existing.SetCount (mesh.getNumVerts());
			normals.SetCount (mesh.getNumVerts());			
			for (int i=0; i<mesh.getNumVerts(); i++) {
				if (tf && useExisting) existing[i] = Point3(0.0f,0.0f,0.0f);  // RB 3/26/99: Changed to Point3 from Point2
				normals[i]  = Point3(0,0,0);				
			}

			for (i=0; i<mesh.getNumFaces(); i++) {
				Point3 v0 = mesh.verts[mesh.faces[i].v[1]] - mesh.verts[mesh.faces[i].v[0]];
				Point3 v1 = mesh.verts[mesh.faces[i].v[2]] - mesh.verts[mesh.faces[i].v[1]];
				Point3 n  = v0^v1;

				for (int j=0; j<3; j++) {
					if (tf && useExisting) {
						// RB 3/26/99: Changed to Point3 from Point2
						//Point3 pt = tv[tf[i].t[j]];
						//existing[mesh.faces[i].v[j]] = Point2(pt.x,pt.y);
						existing[mesh.faces[i].v[j]] = tv[tf[i].t[j]];
					}
					normals[mesh.faces[i].v[j]] += n;					
				}
			}
			
			for (i=0; i<mesh.getNumVerts(); i++) {
				normals[i] = Normalize(normals[i]);
				}
			}
		}
#ifndef NO_PATCHES
	else if ((cap||useExisting) && os->obj->IsSubClassOf(patchObjectClassID)) 
		{
// is whole mesh
		PatchObject *pobj = (PatchObject*)os->obj;
	// Apply our mapping
		PatchMesh &mesh = pobj->patch;

		PatchTVert *tv = NULL;
		TVPatch *tf = NULL;
		if (mesh.getMapSupport(chan))
			{

			tv = mesh.tVerts[chan];
			tf = mesh.tvPatches[chan];

			if (tf || cap) 
				{
				
			// RB 2/12/99: Added "&& useExisting" to "if (tf)" conditions because we may be
			// in this block of code ONLY because cap is on (so we need normals) but we don't
			// want to confuse things so that we and up using existing (which is what was happening).

				if (tf && useExisting) existing.SetCount (mesh.getNumVerts()+mesh.getNumVecs());
				normals.SetCount (mesh.getNumVerts()+mesh.getNumVecs());			
				for (int i=0; i<mesh.getNumVerts()+mesh.getNumVecs(); i++) 
					{
					if (tf && useExisting) existing[i] = Point3(0.0f,0.0f,0.0f);  // RB 3/26/99: Changed to Point3 from Point2
					normals[i]  = Point3(0,0,0);				
					}

				for (i=0; i<mesh.getNumPatches(); i++) 
					{
					int count = mesh.patches[i].type;
/*
					Point3 n ;

					if (count == 4)
						{
						Point3 n1,n2;
						Point3 v0 = mesh.verts[mesh.patches[i].v[0]].p - mesh.verts[mesh.patches[i].v[1]].p;
						Point3 v1 = mesh.verts[mesh.patches[i].v[3]].p - mesh.verts[mesh.patches[i].v[0]].p;
						n1  = v0^v1;
						v0 = mesh.verts[mesh.patches[i].v[2]].p - mesh.verts[mesh.patches[i].v[1]].p;
						v1 = mesh.verts[mesh.patches[i].v[3]].p - mesh.verts[mesh.patches[i].v[2]].p;
						n2  = v0^v1;
						n = Normalize(n1 + n1)*-1.0f;
						}
					else
						{
						Point3 v0 = mesh.verts[mesh.patches[i].v[1]].p - mesh.verts[mesh.patches[i].v[0]].p;
						Point3 v1 = mesh.verts[mesh.patches[i].v[2]].p - mesh.verts[mesh.patches[i].v[1]].p;
						n  = (v0^v1)*-1.0f;
						}
*/

					for (int j=0; j<count; j++) 
						{
						if (tf && useExisting) 
							{
						// RB 3/26/99: Changed to Point3 from Point2
						//Point3 pt = tv[tf[i].t[j]];
						//existing[mesh.faces[i].v[j]] = Point2(pt.x,pt.y);
							existing[mesh.patches[i].v[j]] = tv[tf[i].tv[j]];

							existing[mesh.getNumVerts()+mesh.patches[i].vec[j*2]] = tv[tf[i].tv[j]];
							if (j==0)
								existing[mesh.getNumVerts()+mesh.patches[i].vec[count*2-1]] = tv[tf[i].tv[j]];
							else existing[mesh.getNumVerts()+mesh.patches[i].vec[j*2-1]] = tv[tf[i].tv[j]];

							existing[mesh.getNumVerts()+mesh.patches[i].interior[j]] = tv[tf[i].tv[j]];

							if (!(mesh.patches[j].flags & PATCH_LINEARMAPPING))
								{
								int index = tf[i].handles[0];
								if (index != -1)
									{
									Point3 tp = tv[tf[i].handles[j*2]];
									existing[mesh.getNumVerts()+mesh.patches[i].vec[j*2]] = tv[tf[i].handles[j*2]];
									if (j==0)
										existing[mesh.getNumVerts()+mesh.patches[i].vec[count*2-1]] = tv[tf[i].handles[count*2-1]];
									else existing[mesh.getNumVerts()+mesh.patches[i].vec[j*2-1]] = tv[tf[i].handles[j*2-1]];
									}

								}
							if (!(mesh.patches[j].flags & PATCH_AUTO))
								{
								int index = tf[i].interiors[0];
								if (index != -1)
									existing[mesh.getNumVerts()+mesh.patches[i].interior[j]] = tv[tf[i].interiors[j]];

								}

							}

						Point3 v0 = mesh.vecs[mesh.patches[i].vec[j*2]].p - mesh.verts[mesh.patches[i].v[j]].p ;
						Point3 v1 ;
						if (j==0)
							v1 = mesh.vecs[mesh.patches[i].vec[count*2-1]].p - mesh.verts[mesh.patches[i].v[j]].p;
						else v1 = mesh.vecs[mesh.patches[i].vec[j*2-1]].p - mesh.verts[mesh.patches[i].v[j]].p;

						Point3 n = v0^v1;

						normals[mesh.patches[i].v[j]] += n;					
						normals[mesh.getNumVerts()+mesh.patches[i].interior[j]] += n;					
						normals[mesh.getNumVerts()+mesh.patches[i].vec[j*2]] += n;					
						if (j==0)
							normals[mesh.getNumVerts()+mesh.patches[i].vec[count*2-1]] += n;					
						else normals[mesh.getNumVerts()+mesh.patches[i].vec[j*2-1]] += n;					

						}
					}
				
				
				for (i=0; i<mesh.getNumVerts()+mesh.getNumVecs(); i++) 
					{
					normals[i] = Normalize(normals[i]);
					}
				}
			}
			
		}
#endif // NO_PATCHES
	else if ((cap||useExisting) && os->obj->IsSubClassOf(polyObjectClassID)) {
		PolyObject *tobj = (PolyObject*)os->obj;
				// Apply our mapping
		MNMesh &mesh = tobj->GetMesh();

		int numMaps = mesh.MNum ();
		MNMapFace *tf=NULL;
		Point3 *tv = NULL;
		if (chan >= numMaps) 
			{
			}
		else
			{
			tf = mesh.M(chan)->f;
			tv = mesh.M(chan)->v;
			

			if (tf || cap) 
				{ 
			// RB 2/12/99: Added "&& useExisting" to "if (tf)" conditions because we may be
			// in this block of code ONLY because cap is on (so we need normals) but we don't
			// want to confuse things so that we and up using existing (which is what was happening).

				if (tf && useExisting) existing.SetCount (mesh.numv);
				normals.SetCount (mesh.numv);			
				for (int i=0; i<mesh.numv; i++) 
					{
					if (tf && useExisting) existing[i] = Point3(0.0f,0.0f,0.0f);  // RB 3/26/99: Changed to Point3 from Point2
					normals[i]  = Point3(0,0,0);				
					}
				
//				for (i=0; i<mesh.numf; i++) 
//					normals[i] = mesh.GetVertexNormal (i);

				for (i=0; i<mesh.numf; i++) 
					{
					int degree = mesh.f[i].deg;
					Point3 norm = mesh.GetFaceNormal (i, TRUE);
					for (int j=0; j<degree; j++) 
						{
						if (tf && useExisting) 
							{
							// RB 3/26/99: Changed to Point3 from Point2
						//Point3 pt = tv[tf[i].t[j]];
						//existing[mesh.faces[i].v[j]] = Point2(pt.x,pt.y);
							existing[mesh.f[i].vtx[j]] = tv[tf[i].tv[j]];
							}
						normals[mesh.f[i].vtx[j]] += norm;					

						}
					}
			
				for (i=0; i<mesh.numv; i++) 
					{
					normals[i] = Normalize(normals[i]);
					}
				}
			}
		}


	// Apply works on all objects now.
	if (apply) {
		os->obj->ApplyUVWMap (type, utile, vtile, wtile, uflip, vflip, wflip, FALSE, tm, chan);
		}
	/*
	// Apply the mapping if it's a tri-object and we want to apply
	if (apply && os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *obj = (TriObject*)os->obj;
		Mesh &mesh = obj->mesh;
		mesh.ApplyUVWMap(type,utile,vtile,wtile,
			uflip,vflip,wflip,FALSE,tm);
		}
	*/
	
	DispDeformer deformer (tm,invtm,thebm,tmap,type, strength,utile,vtile, uflip, vflip,
		blur,decay,centerLum, center, cap, existing.Count()?&existing:NULL,
		normals.Count()?&normals:NULL,t);

	os->obj->Deform(&deformer, TRUE);
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
	}


//--- Displace Object ------------------------------------------------------

DisplaceObject::DisplaceObject()
	{
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer6, PBLOCK_LENGTH, CURRENT_VERSION));	
	
	thebm = NULL;
	pblock->SetValue(PB_UTILE,0,1.0f);
	pblock->SetValue(PB_VTILE,0,1.0f);
	pblock->SetValue(PB_WTILE,0,1.0f);
	pblock->SetValue(PB_CENTERL,0,0.5f);
	pblock->SetValue(PB_LENGTH,0,1.0f);
	pblock->SetValue(PB_WIDTH,0,1.0f);
	pblock->SetValue(PB_HEIGHT,0,1.0f);	
	tmap = NULL;
	initParams = FALSE;
	}

DisplaceObject::~DisplaceObject()
	{
	FreeBitmap();
	}

void DisplaceObject::SetReference(int i, RefTargetHandle rtarg) 
	{
	if (i==0) 
		pblock = (IParamBlock*)rtarg; 
	else {
		tmap = (Texmap*)rtarg;
		if (pmapParam && pmapParam->GetParamBlock()==pblock) {
			((DispDlgProc*)pmapParam->GetUserDlgProc())->SetMapName(
				pmapParam->GetHWnd());
			}
		}
	}

void DisplaceObject::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	
	if (pmapParam) {				
		pmapParam->SetParamBlock(pblock);
	} else {
		hSot = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_DISPLACE_SOT),
				DefaultSOTProc,
				GetString(IDS_RB_SOT), 
				(LPARAM)ip,APPENDROLL_CLOSED);

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH_SW,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_DISPLACEPARAM_WS),
			GetString(IDS_RB_PARAMETERS),
			0);		
		}
 	pmapParam->SetUserDlgProc(new DispDlgProc(this)); 	
	}

void DisplaceObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{
	SimpleWSMObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {				
		DestroyCPParamMap(pmapParam);
		ip->DeleteRollupPage(hSot);
		pmapParam = NULL;		
		}	
	}

RefTargetHandle DisplaceObject::Clone(RemapDir& remap)
	{
	DisplaceObject* newob = new DisplaceObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	if (tmap) newob->ReplaceReference(1,tmap);
	newob->bi = bi;
	newob->LoadBitmap();
	BaseClone(this, newob, remap);
	return newob;
	}

void DisplaceObject::FreeBitmap()
	{
	if (thebm) {
		thebm->DeleteThis();
		thebm = NULL;
		}
	}

void DisplaceObject::LoadBitmap() 
	{
	BOOL silentMode = TheManager->SilentMode();
	BOOL netRendering = GetCOREInterface()->GetQuietMode();
	if (netRendering) 
		 TheManager->SetSilentMode(TRUE);
	if (bi.Name()[0]==0&&bi.Device()[0]==0)
		{
		TheManager->SetSilentMode(silentMode);
		return;	
		}

	BMMRES errorReturn;
	thebm = TheManager->Load(&bi,&errorReturn);

	if ((errorReturn != BMMRES_SUCCESS) && (netRendering))
		{
		if (errorReturn==BMMRES_FILENOTFOUND)
			GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,_T("%s: %s"),GetString(IDS_PW_MAPMISSING),bi.Name());
		else GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,_T("%s: %s"),GetString(IDS_PW_MAPERROR),bi.Name());
		}	
	TheManager->SetSilentMode(silentMode);

	}

void DisplaceObject::SetImageTime(TimeValue t)
	{
	if (thebm) {
		int length = bi.LastFrame()-bi.FirstFrame()+1;
		int frame = (t/GetTicksPerFrame())%length;
		if (frame<0) frame += length;
		frame += bi.FirstFrame();
		bi.SetCurrentFrame(frame);
		thebm->GoTo(&bi);
		}
	if (tmap) {
		//tmap->LoadMapFiles(t);
		//tmap->Update(t,FOREVER);
		SetTMapTime(t,tmap);
		}
	}

class FixDimsWSPLCB1 : public PostLoadCallback {
	public:
		DisplaceObject *obj;
		FixDimsWSPLCB1(DisplaceObject *o) {obj=o;}
		void proc(ILoad *iload);
	};

void FixDimsWSPLCB1::proc(ILoad *iload)
	{
	if (obj->pblock->GetVersion()<5) {
		obj->initParams = TRUE;
		}
	delete this;
	}

class FixDimsWSPLCB2 : public PostLoadCallback {
	public:
		DisplaceObject *obj;
		FixDimsWSPLCB2(DisplaceObject *o) {obj=o;}
		void proc(ILoad *iload);
	};

void FixDimsWSPLCB2::proc(ILoad *iload)
	{
	if (obj->initParams) {
		obj->pblock->SetValue(PB_LENGTH,0,1.0f);
		obj->pblock->SetValue(PB_WIDTH,0,1.0f);
		obj->pblock->SetValue(PB_HEIGHT,0,1.0f);
		obj->initParams = FALSE;
		}
	delete this;
	}

IOResult DisplaceObject::Load(ILoad *iload)
	{
	IOResult res = IO_OK;
	
	iload->RegisterPostLoadCallback(
		new FixDimsWSPLCB1(this));

	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));

	iload->RegisterPostLoadCallback(
		new FixDimsWSPLCB2(this));

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case MAPNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				bi.SetName(buf);
				FreeBitmap();
				LoadBitmap();
				break;
				}
			case BMIO_CHUNK: 
				res = bi.Load(iload);
				FreeBitmap();
				LoadBitmap();
				break;			
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}	
	
	return IO_OK;
	}

IOResult DisplaceObject::Save(ISave *isave)
	{
	isave->BeginChunk(BMIO_CHUNK);
	bi.Save(isave);
	isave->EndChunk();
	return IO_OK;
	}

Modifier *DisplaceObject::CreateWSMMod(INode *node)
	{
	return new DisplaceWSMod(node,this);
	}

ForceField *DisplaceObject::GetForceField(INode *node)
	{
	DisplaceField *df = new DisplaceField;
	df->dobj   = this;
	df->thebm  = thebm;
	df->node   = node;
	df->tmValid.SetEmpty();
	return df;
	}

void DisplaceObject::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *DisplaceObject::GetParameterDim(int pbIndex)
	{
	switch (pbIndex) {
		case PB_STRENGTH:
		case PB_LENGTH:
		case PB_WIDTH:
		case PB_HEIGHT:
			return stdWorldDim;
		default:
			return defaultDim;
		}	
	}

TSTR DisplaceObject::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_UTILE: 		return GetString(IDS_RB_UTILE);
		case PB_VTILE: 		return GetString(IDS_RB_VTILE);
		case PB_WTILE: 		return GetString(IDS_RB_WTILE);
		case PB_BLUR:		return GetString(IDS_RB_BLUR);
		case PB_STRENGTH:	return GetString(IDS_RB_STRENGTH2);
		case PB_DECAY:		return GetString(IDS_RB_DECAY);
		case PB_CENTERL:	return GetString(IDS_RB_CENTERPOINT);
		case PB_LENGTH:		return GetString(IDS_RB_LENGTH);
		case PB_WIDTH:		return GetString(IDS_RB_WIDTH);
		case PB_HEIGHT:		return GetString(IDS_RB_HEIGHT);
		default: 			return TSTR(_T(""));
		}
	}

void DisplaceObject::BuildMesh(TimeValue t)
	{
	int type;
	float u;
#define NUM_SEGS	16
	ivalid = FOREVER;
	pblock->GetValue(PB_MAPTYPE,0,type,ivalid);	
	if (type==MAP_PLANAR) {
		mesh.setNumVerts(4);
		mesh.setNumFaces(2);
		mesh.setVert(0,-1.0f,-1.0f,0.0f);
		mesh.setVert(1, 1.0f,-1.0f,0.0f);
		mesh.setVert(2, 1.0f, 1.0f,0.0f);
		mesh.setVert(3,-1.0f, 1.0f,0.0f);
		mesh.faces[0].setEdgeVisFlags(1,0,1);
		mesh.faces[0].setSmGroup(1);
		mesh.faces[0].setVerts(0,1,3);
		mesh.faces[1].setEdgeVisFlags(1,1,0);
		mesh.faces[1].setSmGroup(1);
		mesh.faces[1].setVerts(1,2,3);
	} else 
	if (type==MAP_SPHERICAL||type==MAP_BALL) {		
		mesh.setNumVerts(3*NUM_SEGS+1);
		mesh.setNumFaces(3*NUM_SEGS);

		for (int i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i, Point3((float)cos(u), (float)sin(u), 0.0f));
			}
		for (i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i+NUM_SEGS, Point3(0.0f, (float)cos(u), (float)sin(u)));
			}
		for (i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i+2*NUM_SEGS, Point3((float)cos(u), 0.0f, (float)sin(u)));
			}		
		mesh.setVert(3*NUM_SEGS, Point3(0.0f, 0.0f, 0.0f));
		
		for (i=0; i<3*NUM_SEGS; i++) {
			int i1 = i+1;
			if (i1%NUM_SEGS==0) i1 -= NUM_SEGS;
			mesh.faces[i].setEdgeVisFlags(1,0,0);
			mesh.faces[i].setSmGroup(1);
			mesh.faces[i].setVerts(i,i1,3*NUM_SEGS);
			}
	} else
	if (type==MAP_CYLINDRICAL) {
		mesh.setNumVerts(2*NUM_SEGS);
		mesh.setNumFaces(2*NUM_SEGS);

		for (int i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i, Point3((float)cos(u), (float)sin(u), -0.5f));
			}
		for (i=0; i<NUM_SEGS; i++) {
			u = float(i)/float(NUM_SEGS) * TWOPI;
			mesh.setVert(i+NUM_SEGS, Point3((float)cos(u), (float)sin(u), 0.5f));
			}
		
		for (i=0; i<NUM_SEGS; i++) {
			int i1 = i+1;
			if (i1%NUM_SEGS==0) i1 -= NUM_SEGS;
			mesh.faces[i*2].setEdgeVisFlags(1,0,1);
			mesh.faces[i*2].setSmGroup(1);
			mesh.faces[i*2].setVerts(i,i1,i+NUM_SEGS);

			mesh.faces[i*2+1].setEdgeVisFlags(1,1,0);
			mesh.faces[i*2+1].setSmGroup(1);
			mesh.faces[i*2+1].setVerts(i1,i1+NUM_SEGS,i+NUM_SEGS);
			}
		}	

	float l, w, h;
	pblock->GetValue(PB_LENGTH,t,l,ivalid);
	pblock->GetValue(PB_WIDTH,t,w,ivalid);
	pblock->GetValue(PB_HEIGHT,t,h,ivalid);
	for (int i=0; i<mesh.getNumVerts(); i++) {
		mesh.verts[i].x *= w;
		mesh.verts[i].y *= l;
		mesh.verts[i].z *= h;
 		}

	mesh.InvalidateGeomCache();
	}

class DisplaceObjCreateCallBack: public CreateMouseCallBack {	
	public:
		DisplaceObject *ob;		
		Point3 p0, p1;
		IPoint2 sp0, sp1;	
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);		
	};

int DisplaceObjCreateCallBack::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat) 
	{
	Point3 d;
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:  // only happens with MOUSE_POINT msg
//				sp0 = m;
//				p0  = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE); //vpt->GetPointOnCP(m);
//				mat.SetTrans(p0);
				sp0 = m;
				
				ob->pblock->SetValue(PB_LENGTH,0,0.0f);
				ob->pblock->SetValue(PB_WIDTH,0,0.0f);

//				ob->suspendSnap = TRUE;								

				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1 = p0 + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p0+p1));
				break;
			case 1:	
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				p1.z = p0.z +(float).01; 
				if (flags&MOUSE_CTRL) 
					mat.SetTrans(p0);
				 else 
 					mat.SetTrans(float(.5)*(p0+p1));

				d = (p1-p0) *0.5f;
				

				if (flags&MOUSE_CTRL) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
					}

				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_HEIGHT,0,float(fabs(d.x)));

//				ob->pmapParam->Invalidate();										

				if (msg==MOUSE_POINT && 
						(Length(sp1-sp0)<3 || Length(d)<0.1f)) {
					return CREATE_ABORT;
					}

				if (msg==MOUSE_POINT) {
					return CREATE_STOP;
					}

/*				
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE); // vpt->GetPointOnCP(m);
				d  = Length(p1-p0)/2.0f;				
				mat.IdentityMatrix();
				//mat.Scale(Point3(d,d,d));
				ob->pblock->SetValue(PB_LENGTH,0,d);
				ob->pblock->SetValue(PB_WIDTH,0,d);
				ob->pblock->SetValue(PB_HEIGHT,0,d);
				mat.SetTrans(p0);
				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<4) return CREATE_ABORT;
					else return CREATE_STOP;
					}

*/
				break;
			}
		} 
	else
	if (msg == MOUSE_ABORT) {
		return CREATE_ABORT;
		} 
	else
	if (msg == MOUSE_FREEMOVE) {
		vpt->SnapPreview(m,m);
		}

	return TRUE;
	}

static DisplaceObjCreateCallBack displaceCreateCB;

CreateMouseCallBack* DisplaceObject::GetCreateMouseCallBack()
	{
	displaceCreateCB.ob = this;
	return &displaceCreateCB;
	}



//--- Displace World space modifier ---------------------------------------


DisplaceWSMod::DisplaceWSMod(INode *node,DisplaceObject *obj)
	{	
	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);	
	pblock = NULL;	
	obRef = NULL;
	}

RefTargetHandle DisplaceWSMod::Clone(RemapDir& remap)
	{
	DisplaceWSMod *newob = new DisplaceWSMod(nodeRef,(DisplaceObject*)obRef);	
	newob->SimpleWSMModClone(this);
	BaseClone(this, newob, remap);
	return newob;
	}

Interval DisplaceWSMod::GetValidity(TimeValue t)
	{
	if (nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;		
		DisplaceObject *dobj = (DisplaceObject*)GetWSMObject(t);
		dobj->pblock->GetValue(PB_UTILE,t,f,valid);
		dobj->pblock->GetValue(PB_VTILE,t,f,valid);
		dobj->pblock->GetValue(PB_STRENGTH,t,f,valid);	
		dobj->pblock->GetValue(PB_DECAY,t,f,valid);
		dobj->pblock->GetValue(PB_BLUR,t,f,valid);
		dobj->pblock->GetValue(PB_CENTERL,t,f,valid);
		dobj->pblock->GetValue(PB_LENGTH,t,f,valid);
		dobj->pblock->GetValue(PB_WIDTH,t,f,valid);
		dobj->pblock->GetValue(PB_HEIGHT,t,f,valid);
		tm = nodeRef->GetObjectTM(t,&valid);
		if (dobj->thebm) {
			if (dobj->bi.FirstFrame()!=dobj->bi.LastFrame()) {
				valid.SetInstant(t);				
				}
			}
		if (dobj->tmap) {
			valid &= dobj->tmap->Validity(t);
			}
		return valid;
	} else {
		return FOREVER;
		}
	}


static DispDeformer deformer;

Deformer& DisplaceWSMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{		
	int type, centerLum, uflip, vflip, cap;
	float strength, utile, vtile, blur, decay, center;
	Matrix3 tm, invtm;
	Point3 s;

	DisplaceObject *dobj = (DisplaceObject*)GetWSMObject(t);
	dobj->pblock->GetValue(PB_MAPTYPE,t,type,FOREVER);
	dobj->pblock->GetValue(PB_CENTERLUM,t,centerLum,FOREVER);
	dobj->pblock->GetValue(PB_UTILE,t,utile,FOREVER);
	dobj->pblock->GetValue(PB_VTILE,t,vtile,FOREVER);
	dobj->pblock->GetValue(PB_UFLIP,t,uflip,FOREVER);
	dobj->pblock->GetValue(PB_VFLIP,t,vflip,FOREVER);
	dobj->pblock->GetValue(PB_STRENGTH,t,strength,FOREVER);
	dobj->pblock->GetValue(PB_BLUR,t,blur,FOREVER);
	dobj->pblock->GetValue(PB_DECAY,t,decay,FOREVER);	
	dobj->pblock->GetValue(PB_CENTERL,t,center,FOREVER);
	dobj->pblock->GetValue(PB_CAP,t,cap,FOREVER);
	dobj->pblock->GetValue(PB_LENGTH,t,s.y,FOREVER);
	dobj->pblock->GetValue(PB_WIDTH,t,s.x,FOREVER);
	dobj->pblock->GetValue(PB_HEIGHT,t,s.z,FOREVER);
	
	//invtm = nodeRef->GetNodeTM(t);
	invtm = nodeRef->GetObjectTM(t);
	invtm.PreScale(s);
	tm    = Inverse(invtm);

	if (dobj->thebm) {
		if (blur==0.0f) {
			dobj->thebm->SetFilter(BMM_FILTER_NONE);
		} else {
			dobj->thebm->SetFilter(BMM_FILTER_SUM);
			}
		}

	dobj->SetImageTime(t);

	deformer = DispDeformer (tm,invtm,dobj->thebm, dobj->tmap,type,
		strength,utile,vtile, uflip, vflip, blur,decay,centerLum,center,cap,NULL,NULL,t);
 	return deformer;
 	}

Point3 DisplaceField::Force(TimeValue t,const Point3 &pos, const Point3 &vel,int index)
	{
	int type, centerLum, uflip, vflip, cap;
	float strength, utile, vtile, blur, decay, center;

	if (!tmValid.InInterval(t)) {		
		tmValid = FOREVER;
		Point3 s;
		dobj->pblock->GetValue(PB_LENGTH,t,s.y,tmValid);
		dobj->pblock->GetValue(PB_WIDTH,t,s.x,tmValid);
		dobj->pblock->GetValue(PB_HEIGHT,t,s.z,tmValid);		
		invtm = node->GetObjectTM(t,&tmValid);
		invtm.PreScale(s);
		tm    =	Inverse(invtm);
		}
	
	dobj->pblock->GetValue(PB_MAPTYPE,t,type,FOREVER);
	dobj->pblock->GetValue(PB_CENTERLUM,t,centerLum,FOREVER);
	dobj->pblock->GetValue(PB_UTILE,t,utile,FOREVER);
	dobj->pblock->GetValue(PB_VTILE,t,vtile,FOREVER);
	dobj->pblock->GetValue(PB_UFLIP,t,uflip,FOREVER);
	dobj->pblock->GetValue(PB_VFLIP,t,vflip,FOREVER);
	dobj->pblock->GetValue(PB_STRENGTH,t,strength,FOREVER);
	dobj->pblock->GetValue(PB_BLUR,t,blur,FOREVER);
	dobj->pblock->GetValue(PB_DECAY,t,decay,FOREVER);	
	dobj->pblock->GetValue(PB_CENTERL,t,center,FOREVER);
	dobj->pblock->GetValue(PB_CAP,t,cap,FOREVER);

	dobj->SetImageTime(t);

	DispDeformer deformer(
		tm,invtm,thebm,dobj->tmap,type,
		strength,utile,vtile,
		uflip, vflip,
		blur,decay,centerLum,center,cap,NULL,NULL,t);
	
	return deformer.MapForce(pos);
	}

void DisplaceWSMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	ParticleObject *obj = GetParticleInterface(os->obj);
	if (obj) {
		DisplaceObject *dobj = (DisplaceObject*)GetWSMObject(t);
		force.dobj   = dobj;
		force.thebm  = dobj->thebm;
		force.node   = nodeRef;
		force.tmValid.SetEmpty();
		obj->ApplyForceField(&force);
	} else {
		SimpleWSMMod::ModifyObject(t,mc,os,node);
		}
	}


RefResult DisplaceObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			if (hTarget==pblock && pmapParam) 
				{
				((DispDlgProc*)pmapParam->GetUserDlgProc())->DisableControls();

				}
			if (tmap == hTarget)
				{
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}

			break;
		}
	return SimpleWSMObject::NotifyRefChanged(changeInt,hTarget, partID, message);\
	}

#endif // NO_MODIFIER_DISPLACE