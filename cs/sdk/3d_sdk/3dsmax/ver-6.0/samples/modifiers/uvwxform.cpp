/**********************************************************************
 *<
	FILE: uvwxform.cpp

	DESCRIPTION:  Transforms UVW coords

	CREATED BY: Rolf Berteig

	HISTORY: 10/26/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "resourceOverride.h"
#include "iparamm.h"
#include <vector>
#include <algorithm>
#include <functional>

#ifndef NO_MODIFIER_UVW_XFORM 	// russom - 10/11/01

class UVWXFormMod : public Modifier {	
public:		
	IParamBlock *pblock;
	
	static IObjParam *ip;
	static IParamMap *pmapParam;
	static UVWXFormMod *editMod;

	UVWXFormMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_RB_UVWXFORMMOD);}
	//SS 11/12/2002: In order to be able to add parameters to this class without
	// causing a crash when loaded in max 5.x, the class id needs to change.
	virtual Class_ID ClassID() { return Class_ID(UVW_XFORM2_CLASS_ID,0);}		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() {return GetString(IDS_RB_UVWXFORMMOD);}
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);

	IOResult Load(ILoad *iload);

	// From modifier
	ChannelMask ChannelsUsed()  {return PART_TEXMAP|PART_VERTCOLOR|PART_TOPO|PART_GEOM;} // mjm - 10.5.99 - added PART_GEOM
	ChannelMask ChannelsChanged() {return PART_TEXMAP|PART_VERTCOLOR;}
	Class_ID InputType() {return mapObjectClassID;}
	void triObjectModify(TriObject *tobj, TimeValue t);
	void polyObjectModify(PolyObject *polyOb, TimeValue t);
	void patchObjectModify(PatchObject *patchOb, TimeValue t);
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	IParamArray *GetParamBlock() {return pblock;}
	int GetParamBlockIndex(int id) {return id;}

	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i) {return pblock;}
	void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock*)rtarg;}

	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) {return pblock;}
	TSTR SubAnimName(int i) {return _T("");}

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message);

private:
	struct Params {
    Point3 tile;
    Point3 offset;
    BOOL uflip, vflip, wflip;
		int channel;
    int mapChannel;
    float wangle;
    int rotcenter;
		BOOL entireObject;
	};
	void GetParams(Params&, TimeValue, Interval&);
  static void CreateTransformMatrix(Matrix3&, const Params&);

  static void ApplyTransformOnMesh(Mesh&, const Matrix3&, int mapChannel);
  static void ApplyTransformOnMNMesh(MNMesh&, const Matrix3&, int mapChannel);
  static void ApplyTransformOnPatchMesh(PatchMesh&, const Matrix3&, int mapChannel);

	static void ApplyTransformOnMeshSelectedFaces(Mesh&, const Matrix3&, int mapChannel);
  static void ApplyTransformOnMNMeshSelectedFaces(MNMesh&, const Matrix3&, int mapChannel);
  static void ApplyTransformOnPatchMeshSelectedPatches(PatchMesh&, const Matrix3&, int mapChannel);
};


//--- ClassDescriptor and class vars ---------------------------------

IParamMap       *UVWXFormMod::pmapParam = NULL;
IObjParam       *UVWXFormMod::ip        = NULL;
UVWXFormMod     *UVWXFormMod::editMod   = NULL;


class UVWXFormClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new UVWXFormMod; }
	//SS 2/5/2003: In order to prevent maxscript from trying to expose both
	// of these classes, return an empty string for the class name.
	const TCHAR *	ClassName() { return _T(""); } //GetString(IDS_RB_UVWXFORMMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(UVW_XFORM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
};
//SS 11/12/2002: Need nearly-duplicate class desc to support changed class id;
// note that the old class desc is now private.
class UVWXForm2ClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new UVWXFormMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_UVWXFORMMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(UVW_XFORM2_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
};

static UVWXFormClassDesc uvwxformDesc;
ClassDesc* GetUVWXFormModDesc() {return &uvwxformDesc;}
static UVWXForm2ClassDesc uvwxform2Desc;
ClassDesc* GetUVWXFormMod2Desc() {return &uvwxform2Desc;}

//--- Parameter map/block descriptors -------------------------------

#define PB_UTILE	0
#define PB_VTILE	1
#define PB_WTILE	2
#define PB_UOFFSET	3
#define PB_VOFFSET	4
#define PB_WOFFSET	5
#define PB_UFLIP	6
#define PB_VFLIP	7
#define PB_WFLIP	8
#define PB_CHANNEL	9
#define PB_MAPCHANNEL	10
#define PB_WANGLE		11 //SS 9/20/2002: added rotation
#define PB_ROTCENTER	12
//#define PB_USE_SOSEL	13 //SS 11/12/2002: added param to apply only to SO sel (not yet impl)
#define PB_ENTIREOBJECT	13 //alexc - july.22.2003

//
//
// Parameters

static int chanIDs[] = {IDC_MAP_TEXMAP, IDC_MAP_VERTCOL};

static ParamUIDesc descParam[] = {
	// U Tile
	ParamUIDesc (PB_UTILE, EDITTYPE_FLOAT,
		IDC_MAP_UTILE,IDC_MAP_UTILESPIN,
		-BIGFLOAT,BIGFLOAT, SPIN_AUTOSCALE),

	// V Tile
	ParamUIDesc (PB_VTILE, EDITTYPE_FLOAT,
		IDC_MAP_VTILE,IDC_MAP_VTILESPIN,
		-BIGFLOAT,BIGFLOAT, SPIN_AUTOSCALE),

	// W Tile
	ParamUIDesc (PB_WTILE, EDITTYPE_FLOAT,
		IDC_MAP_WTILE,IDC_MAP_WTILESPIN,
		-BIGFLOAT,BIGFLOAT, SPIN_AUTOSCALE),

	// U Offset
	ParamUIDesc (PB_UOFFSET, EDITTYPE_UNIVERSE,// EDITTYPE_FLOAT,
		IDC_MAP_UOFFSET,IDC_MAP_UOFFSETSPIN,
		-BIGFLOAT,BIGFLOAT, SPIN_AUTOSCALE),

	// V Offset
	ParamUIDesc (PB_VOFFSET, EDITTYPE_UNIVERSE,// EDITTYPE_FLOAT,
		IDC_MAP_VOFFSET,IDC_MAP_VOFFSETSPIN,
		-BIGFLOAT,BIGFLOAT, SPIN_AUTOSCALE),

	// W Offset
	ParamUIDesc (PB_WOFFSET, EDITTYPE_UNIVERSE,// EDITTYPE_FLOAT,
		IDC_MAP_WOFFSET,IDC_MAP_WOFFSETSPIN,
		-BIGFLOAT,BIGFLOAT, SPIN_AUTOSCALE),

	// U Flip
	ParamUIDesc(PB_UFLIP,TYPE_SINGLECHEKBOX,IDC_MAP_UFLIP),

	// V Flip
	ParamUIDesc(PB_VFLIP,TYPE_SINGLECHEKBOX,IDC_MAP_VFLIP),

	// W Flip
	ParamUIDesc(PB_WFLIP,TYPE_SINGLECHEKBOX,IDC_MAP_WFLIP),

#ifndef RENDER_VER // removed map channel controls for VIZ Render
	// Channel
	ParamUIDesc(PB_CHANNEL,TYPE_RADIO,chanIDs,2),

	// MapChannel
	ParamUIDesc (PB_MAPCHANNEL, EDITTYPE_POS_INT,
		IDC_MAP_CHAN, IDC_MAP_CHAN_SPIN,
		1, MAX_MESHMAPS-1, SPIN_AUTOSCALE),
#endif

	// W Angle
	ParamUIDesc (PB_WANGLE, EDITTYPE_FLOAT,		//SS 9/20/2002: added rotation
		IDC_MAP_WANGLE, IDC_MAP_WANGLESPIN,
		-720.0f, 720.0f, 1.0f, stdAngleDim),

	// Center of rotation
	ParamUIDesc(PB_ROTCENTER, TYPE_SINGLECHEKBOX, IDC_MAP_ROTCENTER),

	// Apply to whole object
	ParamUIDesc(PB_ENTIREOBJECT, TYPE_SINGLECHEKBOX, IDC_MAP_ENTIREOBJECT), //alexc - july.22.2003
};
#ifndef RENDER_VER // removed map channel controls for VIZ Render
 #define PARAMDESC_LENGTH	14
#else
 #define PARAMDESC_LENGTH	12
#endif

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE,  PB_UTILE },		// U Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_VTILE },		// V Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_WTILE },		// W Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_UOFFSET },		// U Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_VOFFSET },		// V Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_WOFFSET },		// W Offset
	{ TYPE_INT,   NULL, FALSE, PB_UFLIP },		// U Flip
	{ TYPE_INT,   NULL, FALSE, PB_VFLIP },		// V Flip
	{ TYPE_INT,   NULL, FALSE, PB_WFLIP },		// W Flip	
	};

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE,  PB_UTILE },		// U Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_VTILE },		// V Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_WTILE },		// W Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_UOFFSET },		// U Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_VOFFSET },		// V Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_WOFFSET },		// W Offset
	{ TYPE_INT,   NULL, FALSE, PB_UFLIP },		// U Flip
	{ TYPE_INT,   NULL, FALSE, PB_VFLIP },		// V Flip
	{ TYPE_INT,   NULL, FALSE, PB_WFLIP },		// W Flip	
	{ TYPE_INT,   NULL, FALSE, PB_CHANNEL },		// channel
};

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE,  PB_UTILE },		// U Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_VTILE },		// V Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_WTILE },		// W Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_UOFFSET },		// U Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_VOFFSET },		// V Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_WOFFSET },		// W Offset
	{ TYPE_INT,   NULL, FALSE, PB_UFLIP },		// U Flip
	{ TYPE_INT,   NULL, FALSE, PB_VFLIP },		// V Flip
	{ TYPE_INT,   NULL, FALSE, PB_WFLIP },		// W Flip	
	{ TYPE_INT,   NULL, FALSE, PB_CHANNEL },		// channel
	{ TYPE_INT,   NULL, FALSE, PB_MAPCHANNEL },
};

static ParamBlockDescID descVer3[] = {			//SS 9/20/2002: added rotation
	{ TYPE_FLOAT, NULL, TRUE,  PB_UTILE },		// U Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_VTILE },		// V Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_WTILE },		// W Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_UOFFSET },	// U Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_VOFFSET },	// V Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_WOFFSET },	// W Offset
	{ TYPE_INT,   NULL, FALSE, PB_UFLIP },		// U Flip
	{ TYPE_INT,   NULL, FALSE, PB_VFLIP },		// V Flip
	{ TYPE_INT,   NULL, FALSE, PB_WFLIP },		// W Flip
	{ TYPE_INT,   NULL, FALSE, PB_CHANNEL },	// channel
	{ TYPE_INT,   NULL, FALSE, PB_MAPCHANNEL },
	{ TYPE_FLOAT, NULL, TRUE,  PB_WANGLE },		// W Angle
	{ TYPE_INT,   NULL, FALSE, PB_ROTCENTER },	// rotation center
	{ TYPE_INT,   NULL, FALSE, PB_ENTIREOBJECT }, // use entire object (was there but not used in this version)
};

static ParamBlockDescID descVer4[] = {		  // alexc - july.23.2003 - added "use entire object" check box
	{ TYPE_FLOAT, NULL, TRUE,  PB_UTILE },		// U Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_VTILE },		// V Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_WTILE },		// W Tile
	{ TYPE_FLOAT, NULL, TRUE,  PB_UOFFSET },	// U Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_VOFFSET },	// V Offset
	{ TYPE_FLOAT, NULL, TRUE,  PB_WOFFSET },	// W Offset
	{ TYPE_INT,   NULL, FALSE, PB_UFLIP },		// U Flip
	{ TYPE_INT,   NULL, FALSE, PB_VFLIP },		// V Flip
	{ TYPE_INT,   NULL, FALSE, PB_WFLIP },		// W Flip
	{ TYPE_INT,   NULL, FALSE, PB_CHANNEL },	// channel
	{ TYPE_INT,   NULL, FALSE, PB_MAPCHANNEL },
	{ TYPE_FLOAT, NULL, TRUE,  PB_WANGLE },		// W Angle
	{ TYPE_INT,   NULL, FALSE, PB_ROTCENTER },	// rotation center
	{ TYPE_INT,   NULL, FALSE, PB_ENTIREOBJECT }, // use entire object
};
#define PBLOCK_LENGTH	14

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,9,0),
	ParamVersionDesc(descVer1,10,1),
	ParamVersionDesc(descVer2,11,2),
	ParamVersionDesc(descVer3,14,3),
};
#define NUM_OLDVERSIONS	4

// Current version
#define CURRENT_VERSION	4
static ParamVersionDesc curVersion(descVer4, PBLOCK_LENGTH, CURRENT_VERSION);


//--- UVWXForm mod methods -------------------------------

UVWXFormMod::UVWXFormMod() {
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer3, PBLOCK_LENGTH, CURRENT_VERSION));

	pblock->SetValue(PB_UTILE,0,1.0f);
	pblock->SetValue(PB_VTILE,0,1.0f);
	pblock->SetValue(PB_WTILE,0,1.0f);
	pblock->SetValue(PB_MAPCHANNEL, 0, 1);
}

// Following is necessary to make sure mapchannel doesn't come in as zero;
// this happens when loading up an old scene -- the pblock with the defaults
// is overwritten.
class XSetChannelToOne : public PostLoadCallback {
public:
	UVWXFormMod *xm;
	XSetChannelToOne (UVWXFormMod *xMod) { xm = xMod; }
	void proc (ILoad *iload);
};

void XSetChannelToOne::proc (ILoad *iload) {
	if (xm && xm->pblock) {
		int mapChan;
		xm->pblock->GetValue (PB_MAPCHANNEL, 0, mapChan, FOREVER);
		if (!mapChan) xm->pblock->SetValue (PB_MAPCHANNEL, 0, 1);
	}
	delete this;
}

class UVWXFormPostLoadCallback : public PostLoadCallback {
	public:
		UVWXFormMod* xm;
		UVWXFormPostLoadCallback(UVWXFormMod* m) : xm(m) {}
		void proc(ILoad* iload) {
			DWORD oldVer = xm->pblock->GetVersion();
			(new ParamBlockPLCB(versions, NUM_OLDVERSIONS, &curVersion, xm, 0))
				->proc(iload); // proc contains "delete this"
			if (oldVer < 3) {
				xm->pblock->SetValue(PB_ENTIREOBJECT, 0, TRUE);
			}
			else if (oldVer < 4) {
				xm->pblock->SetValue(PB_ENTIREOBJECT, 0, FALSE);
			}
			delete this;
		}
	};

IOResult UVWXFormMod::Load(ILoad *iload) {
	Modifier::Load(iload);
	iload->RegisterPostLoadCallback(new UVWXFormPostLoadCallback(this));
	iload->RegisterPostLoadCallback(new XSetChannelToOne (this));
	return IO_OK;
}

void UVWXFormMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	this->ip = ip;
	editMod  = this;
	
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);	
	SetAFlag(A_MOD_BEING_EDITED);

	pmapParam = CreateCPParamMap (descParam,PARAMDESC_LENGTH,
		pblock, ip, hInstance, MAKEINTRESOURCE(IDD_UVWXFORMPARAM),
		GetString(IDS_RB_PARAMETERS), 0);	
}

void UVWXFormMod::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next) {
	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	
	DestroyCPParamMap(pmapParam);
}

RefTargetHandle UVWXFormMod::Clone(RemapDir& remap) {
	UVWXFormMod *mod = new UVWXFormMod();
	mod->ReplaceReference(0,pblock->Clone(remap));	
	BaseClone(this, mod, remap);
	return mod;
}

void UVWXFormMod::GetParams(Params& params, TimeValue t, Interval& iv)
{
	pblock->GetValue(PB_UTILE,				t, params.tile.x, iv);
	pblock->GetValue(PB_VTILE,				t, params.tile.y, iv);
	pblock->GetValue(PB_WTILE,				t, params.tile.z, iv);
	pblock->GetValue(PB_UOFFSET,			t, params.offset.x, iv);
	pblock->GetValue(PB_VOFFSET,			t, params.offset.y, iv);
	pblock->GetValue(PB_WOFFSET,			t, params.offset.z, iv);	
	pblock->GetValue(PB_UFLIP,				t, params.uflip, iv);
	pblock->GetValue(PB_VFLIP,				t, params.vflip, iv);
	pblock->GetValue(PB_WFLIP,				t, params.wflip, iv);
	pblock->GetValue(PB_CHANNEL,			t, params.channel,			iv);
	pblock->GetValue(PB_MAPCHANNEL,		t, params.mapChannel,		iv);
	pblock->GetValue(PB_WANGLE,				t, params.wangle,				iv);
	pblock->GetValue(PB_ROTCENTER,		t, params.rotcenter,		iv);
	pblock->GetValue(PB_ENTIREOBJECT,	t, params.entireObject, iv);

	//SS 11/5/2002: Tiling is only useful when not 0; let's fix it here.
	if (params.tile.x == 0.0f) params.tile.x = 1.0f;
	if (params.tile.y == 0.0f) params.tile.y = 1.0f;
	if (params.tile.z == 0.0f) params.tile.z = 1.0f;
}

void UVWXFormMod::CreateTransformMatrix(Matrix3& tm, const Params& params)
{
	tm.IdentityMatrix();

	// apply rotation first; we do this because we want scaling to be rotated; 
	// apply an offset if we're rotating about the center of W-space
	if (params.rotcenter)
		tm.Translate(Point3(-0.5f, -0.5f, -0.5f));
	tm.RotateZ(params.wangle);
	if (params.rotcenter)
		tm.Translate(Point3(0.5f, 0.5f, 0.5f));

	// apply flip, make sure translation is scaled too
	tm.Scale(Point3(params.uflip ? -1.0f : 1.0f,
                  params.vflip ? -1.0f : 1.0f,
                  params.wflip ? -1.0f : 1.0f), TRUE);
	tm.Translate(Point3(params.uflip ? 1.0f : 0.0f,
                      params.vflip ? 1.0f : 0.0f,
                      params.wflip ? 1.0f : 0.0f));

	// apply tiling; by putting this after rotation, we don't skew the texture;
	// make sure translation is scaled too
	tm.Scale(params.tile, TRUE);

	// apply offset
	tm.Translate(params.offset);
}

namespace
{
	bool HasSelectedFaces(Mesh& mesh)	{
		return mesh.FaceSel().NumberSet() > 0;
	}

	bool HasSelectedFaces(MNMesh& mesh)	{
		BitArray selectedFaces;
		mesh.getFaceSel(selectedFaces);
		return selectedFaces.NumberSet() > 0;
	}

	bool HasSelectedPatches(PatchMesh& mesh) {
		return mesh.PatchSel().NumberSet() > 0;
	}
}

void UVWXFormMod::triObjectModify(TriObject *tobj, TimeValue t) 
{
	Interval iv = FOREVER;
  Params params;
  GetParams(params, t, iv);
	
	if (params.channel) {
		params.mapChannel = 0;
		iv &= tobj->ChannelValidity(t, VERT_COLOR_CHAN_NUM);
	} else {
		iv &= tobj->ChannelValidity(t, TEXMAP_CHAN_NUM);
	}

	Mesh& mesh = tobj->GetMesh();

	// check if the channel is available in this mesh
	if (!mesh.mapSupport(params.mapChannel))
    return;

	Matrix3 tm;
  CreateTransformMatrix(tm, params);

	if (!params.entireObject && HasSelectedFaces(mesh))
		ApplyTransformOnMeshSelectedFaces(mesh, tm, params.mapChannel);
	else
		ApplyTransformOnMesh(mesh, tm, params.mapChannel);

	if (params.channel) tobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	else tobj->UpdateValidity(TEXMAP_CHAN_NUM, iv);
}

void UVWXFormMod::polyObjectModify(PolyObject *polyOb, TimeValue t) 
{
	Interval iv = FOREVER;
  Params params;
  GetParams(params, t, iv);

	if (params.channel) {
		params.mapChannel = 0;
		iv &= polyOb->ChannelValidity(t, VERT_COLOR_CHAN_NUM);
	} else {
		iv &= polyOb->ChannelValidity(t, TEXMAP_CHAN_NUM);
	}

	MNMesh& mesh = polyOb->GetMesh();

	// check if the channel is available in this mesh
	if ((params.mapChannel < -NUM_HIDDENMAPS) ||
			(params.mapChannel >= mesh.numm))
		return;

	Matrix3 tm;
	CreateTransformMatrix(tm, params);

	if (!params.entireObject && HasSelectedFaces(mesh))
		ApplyTransformOnMNMeshSelectedFaces(mesh, tm, params.mapChannel);
	else
		ApplyTransformOnMNMesh(mesh, tm, params.mapChannel);

	if (params.channel) polyOb->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	else polyOb->UpdateValidity(TEXMAP_CHAN_NUM,iv);
}

void UVWXFormMod::patchObjectModify(PatchObject *patchOb, TimeValue t) 
{
	Interval iv = FOREVER;
  Params params;
  GetParams(params, t, iv);

	if (params.channel) {
		params.mapChannel = 0;
		iv &= patchOb->ChannelValidity(t, VERT_COLOR_CHAN_NUM);
	} else {
		iv &= patchOb->ChannelValidity(t, TEXMAP_CHAN_NUM);
	}

	PatchMesh& mesh = patchOb->GetPatchMesh(t);//MNMesh& mesh = polyOb->GetMesh();

	// check if the channel is available in this mesh
	if (!mesh.getMapSupport(params.mapChannel))
    return;

	Matrix3 tm;
	CreateTransformMatrix(tm, params);

	if (!params.entireObject && HasSelectedPatches(mesh))
		ApplyTransformOnPatchMeshSelectedPatches(mesh, tm, params.mapChannel);
	else
		ApplyTransformOnPatchMesh(mesh, tm, params.mapChannel);

	if (params.channel) patchOb->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	else patchOb->UpdateValidity(TEXMAP_CHAN_NUM,iv);
}

void UVWXFormMod::ApplyTransformOnMesh(Mesh& mesh, const Matrix3& tm, int mapChannel)
{
	// get the vertices in this texture map
	int ct = mesh.getNumMapVerts(mapChannel);
	UVVert* mv = mesh.mapVerts(mapChannel);
	DbgAssert(mv);

	// apply the transforms to the texture vertices
	for (int i = 0; i < ct; ++i) {
		Point3& uvw = mv[i];
		uvw = uvw * tm;
	}
}

void UVWXFormMod::ApplyTransformOnMNMesh(MNMesh& mesh, const Matrix3& tm, int mapChannel)
{
	// get the vertices in this texture map
	MNMap* mnMap = mesh.M(mapChannel);
	int ct = mnMap->VNum();
	UVVert* mv = mnMap->v;
	DbgAssert(mv);

	// apply the transforms to the texture vertices
	for (int i = 0; i < ct; ++i) {
		Point3& uvw = mv[i];
		uvw = uvw * tm;
	}
}

void UVWXFormMod::ApplyTransformOnPatchMesh(PatchMesh& mesh, const Matrix3& tm, int mapChannel)
{
	// get the vertices in this texture map
	int ct = mesh.getNumMapVerts(mapChannel);
	PatchTVert* mv = mesh.mapVerts(mapChannel);
	DbgAssert(mv);

	// apply the transforms to the texture vertices
	for (int i = 0; i < ct; ++i) {
		Point3& uvw = mv[i];
		uvw = uvw * tm;
	}
}

namespace
{
	class TvMap	{
		struct TvMapItem {
			int nonSelCount;	// use count by unselected faces
			int selCount;			// use count by selected faces
			int mapIndex;			// map selected vertex to this index. (-1 = no mapping required)

			TvMapItem() : nonSelCount(0), selCount(0), mapIndex(-1) {}
		};

		static bool OnSelectionBorder(const TvMapItem& item) {
			return (item.selCount > 0) && (item.nonSelCount > 0);
		}

		std::vector<TvMapItem> mItems;

	public:
		TvMap(int size) : mItems(size) {}

		void Set(int index, bool selected) {
			if (index != -1) {
			DbgAssert(index < mItems.size());
				if (selected)
					++mItems[index].selCount;
				else
					++mItems[index].nonSelCount;
			}
		}

		int NewTVertCount() const {
			return mItems.size() +
						 std::count_if(mItems.begin(), mItems.end(), OnSelectionBorder);
		}

		template <typename TVerts>
		void MapTVerts(TVerts& mapVerts) {
			int newVertIndex = mItems.size();
			for (int i = 0; i < mItems.size(); ++i) {
				if (OnSelectionBorder(mItems[i])) {
					mItems[i].mapIndex = newVertIndex;    // save off mapping	for MapTVertIndex
					mapVerts[newVertIndex] = mapVerts[i]; // copy the vert
					++newVertIndex;
				}
			}
		}

		template <typename Index /* DWORD or int */> 
		void MapTVertIndex(Index& index) const {
			if (index != -1) {
				DbgAssert(index < mItems.size());
				int newIndex = mItems[index].mapIndex;
				if (newIndex != -1)
					index = newIndex;
			}
		}

		template <typename TVerts>
		void ApplyTransform(TVerts& mapVerts, const Matrix3& tm) const {
			for (int i = 0; i < mItems.size(); ++i) {
				if (mItems[i].selCount > 0) {
					Point3& uvw = mapVerts[i];
					uvw = uvw * tm;
				}
			}
		}
	};
}

void UVWXFormMod::ApplyTransformOnMeshSelectedFaces(Mesh& mesh, const Matrix3& tm, int mapChannel)
{
	// (1) Figure out which verts need to be duplicated and mapped
	TvMap tVertMap(mesh.getNumMapVerts(mapChannel));
	const BitArray& selectedFaces = mesh.FaceSel();

	// Determine selected and unselected texture vertices
	int faceCount = mesh.getNumFaces();
	TVFace* tFaces = mesh.mapFaces(mapChannel);
	for (int i = 0; i < faceCount; ++i) {
		bool selected = selectedFaces[i] != 0;
		TVFace& tFace = tFaces[i];
		for (int j = 0; j < 3; ++j) {
			tVertMap.Set(tFace.t[j], selected);
		}
	}

	// (2) Add new texture verts for those that are shared by selected and non-selected faces 

	// Create the new texture verts and map to them
	int newVertCount = tVertMap.NewTVertCount();
	mesh.setNumMapVerts(mapChannel, newVertCount, TRUE /* keep the old ones */);
	UVVert* mv = mesh.mapVerts(mapChannel);
	tVertMap.MapTVerts(mv);

	// map the verts on the un-selected faces to the newly created verts
	tFaces = mesh.mapFaces(mapChannel);
	for (int i = 0; i < faceCount; ++i) {
		if (!selectedFaces[i]) {
			TVFace& tFace = tFaces[i];
			for (int j = 0; j < 3; ++j) {
				tVertMap.MapTVertIndex(tFace.t[j]);
			}
		}
	}

  // (3) Apply the transformation
	tVertMap.ApplyTransform(mv, tm);
}

void UVWXFormMod::ApplyTransformOnMNMeshSelectedFaces(MNMesh& mesh, const Matrix3& tm, int mapChannel)
{
	// (1) Figure out which verts need to be duplicated and mapped
	MNMap* mnMap = mesh.M(mapChannel);
	TvMap tVertMap(mnMap->VNum());
	BitArray selectedFaces;
	mesh.getFaceSel(selectedFaces);

	// Determine selected and unselected texture vertices
	int faceCount = mnMap->numf;
	MNMapFace* tFaces = mnMap->f;
	for (int i = 0; i < faceCount; ++i) {
		bool selected = selectedFaces[i] != 0;
		MNMapFace& tFace = tFaces[i];
		for (int j = 0; j < tFace.deg; ++j) {
			tVertMap.Set(tFace.tv[j], selected);
		}
	}

	// (2) Add new texture verts for those that are shared by selected and non-selected faces 

	// Create the new texture verts and map to them
	int newVertCount = tVertMap.NewTVertCount();
	mnMap->setNumVerts(newVertCount);
	UVVert* mv = mnMap->v;
	tVertMap.MapTVerts(mv);

	// map the verts on the un-selected faces to the newly created verts
	tFaces = mnMap->f;
	for (int i = 0; i < faceCount; ++i) {
		if (!selectedFaces[i]) {
			MNMapFace& tFace = tFaces[i];
			for (int j = 0; j < tFace.deg; ++j) {
				tVertMap.MapTVertIndex(tFace.tv[j]);
			}
		}
	}

  // (3) Apply the transformation
	tVertMap.ApplyTransform(mv, tm);
}

void UVWXFormMod::ApplyTransformOnPatchMeshSelectedPatches(PatchMesh& mesh, const Matrix3& tm, int mapChannel)
{
	// (1) Figure out which verts need to be duplicated and mapped
	TvMap tVertMap(mesh.getNumMapVerts(mapChannel));
	const BitArray& selectedPatches = mesh.PatchSel();

	// Determine selected and unselected texture vertices
	int patchCount = mesh.getNumPatches();
	TVPatch* tPatches = mesh.mapPatches(mapChannel);
	Patch* patches = mesh.patches;
	for (int i = 0; i < patchCount; ++i) {
		bool selected = selectedPatches[i] != 0;
		TVPatch& tPatch = tPatches[i];
		Patch& patch = patches[i];
		const int vcount = patch.type;
		for (int j = 0; j < vcount; ++j) {
			tVertMap.Set(tPatch.tv[j], selected);
			if (!(patch.flags & PATCH_LINEARMAPPING)) {
				tVertMap.Set(tPatch.handles[2 * j], selected);
				tVertMap.Set(tPatch.handles[2 * j + 1], selected);
				if (!(patch.flags & PATCH_AUTO)) {
					tVertMap.Set(tPatch.interiors[j], selected);
				}
			}
		}
	}

	// (2) Add new texture verts for those that are shared by selected and non-selected patches 

	// Create the new texture verts and map to them
	int newVertCount = tVertMap.NewTVertCount();
	mesh.setNumMapVerts(mapChannel, newVertCount, TRUE /* keep the old ones */);
	PatchTVert* mv = mesh.mapVerts(mapChannel);
	tVertMap.MapTVerts(mv);

	// map the verts on the un-selected patches to the newly created verts
	tPatches = mesh.mapPatches(mapChannel);
	for (int i = 0; i < patchCount; ++i) {
		if (!selectedPatches[i]) {
			TVPatch& tPatch = tPatches[i];
			Patch& patch = patches[i];
			const int vcount = patch.type;
			for (int tvIndex = 0; tvIndex < patch.type; ++tvIndex) {
				tVertMap.MapTVertIndex(tPatch.tv[tvIndex]);
				if (!(patch.flags & PATCH_LINEARMAPPING)) {
					tVertMap.MapTVertIndex(tPatch.handles[2 * tvIndex]);
					tVertMap.MapTVertIndex(tPatch.handles[2 * tvIndex + 1]);
					if (!(patch.flags & PATCH_AUTO)) {
						tVertMap.MapTVertIndex(tPatch.interiors[tvIndex]);
					}
				}
			}
		}
	}

  // (3) Apply the transformation
	tVertMap.ApplyTransform(mv, tm);
}

void UVWXFormMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	// Handle any triobject types
	if (os->obj->IsSubClassOf(triObjectClassID)) {
		TriObject *tobj = (TriObject*)os->obj;
		triObjectModify(tobj,t);
	}
	// Handle any PolyObject types
	else if (os->obj->IsSubClassOf(polyObjectClassID)) {
		PolyObject *polyOb = (PolyObject*)os->obj;
		polyObjectModify(polyOb,t);
	}
	// Handle any PatchObject types
	else if (os->obj->IsSubClassOf(patchObjectClassID)) {
		PatchObject *patchOb = (PatchObject*)os->obj;
		patchObjectModify(patchOb,t);
	}
	// all others should try to convert to a triobject
	else if (os->obj->CanConvertToType(triObjectClassID)) {
		TriObject  *triOb = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
		// Now stuff this into the pipeline!
		os->obj = triOb;
		triObjectModify(triOb,t);
	}
	// No action is taken if the object is not supported or can not be converted to triobject
}

Interval UVWXFormMod::LocalValidity(TimeValue t)
{
	// aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	Interval iv = FOREVER;
	float f;
	pblock->GetValue(PB_UTILE,t,f,iv);
	pblock->GetValue(PB_VTILE,t,f,iv);
	pblock->GetValue(PB_WTILE,t,f,iv);
	pblock->GetValue(PB_UOFFSET,t,f,iv);
	pblock->GetValue(PB_VOFFSET,t,f,iv);
	pblock->GetValue(PB_WOFFSET,t,f,iv);
	pblock->GetValue(PB_WANGLE,t,f,iv);
	return iv;
}

RefResult UVWXFormMod::NotifyRefChanged( 
		Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if (editMod==this && pmapParam) pmapParam->Invalidate();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_UTILE:
				case PB_VTILE:
				case PB_WTILE: gpd->dim = stdNormalizedDim; break;
				case PB_UOFFSET:
				case PB_VOFFSET:
				case PB_WOFFSET: gpd->dim = stdWorldDim; break;
				case PB_WANGLE: gpd->dim = stdAngleDim; break;
			}
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_UTILE:	 gpn->name = GetString(IDS_RB_UTILE); break;
				case PB_VTILE:	 gpn->name = GetString(IDS_RB_VTILE); break;
				case PB_WTILE:	 gpn->name = GetString(IDS_RB_WTILE); break;
				case PB_UOFFSET: gpn->name = GetString(IDS_RB_UOFFSET); break;
				case PB_VOFFSET: gpn->name = GetString(IDS_RB_VOFFSET); break;
				case PB_WOFFSET: gpn->name = GetString(IDS_RB_WOFFSET); break;
				case PB_UFLIP: gpn->name = GetString(IDS_UVWX_UFLIP); break;
				case PB_VFLIP: gpn->name = GetString(IDS_UVWX_VFLIP); break;
				case PB_WFLIP: gpn->name = GetString(IDS_UVWX_WFLIP); break;
				case PB_CHANNEL: gpn->name = GetString(IDS_UVWX_CHANNEL); break;
				case PB_MAPCHANNEL: gpn->name = GetString(IDS_UVWX_MAPCHANNEL); break; // LAM - 3/7/03 - defect 164105
				case PB_WANGLE:	 gpn->name = GetString(IDS_UVWX_WANGLE); break;
				case PB_ROTCENTER:	 gpn->name = GetString(IDS_UVWX_ROTCENTER); break;
			}
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}

#endif // NO_MODIFIER_UVW_XFORM
