/**********************************************************************
 *<
	FILE: gsphere2.cpp

	DESCRIPTION:  Geodesic Sphere Object
				  Edition 2 rewritten using ParamBlock2's

	CREATED BY: Steve Anderson
				Ed 2. John Wainwright

	HISTORY: created 12/18/95

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#include "prim.h"

#ifndef NO_OBJECT_STANDARD_PRIMITIVES

#include "iparamm2.h"
#include "Simpobj.h"
#include "surf_api.h"

#define GSPHERE_CLASS_ID 0
#define PBLOCK_REF_NO	 0
ClassDesc* GetGSphereDesc();

// JBW: IParamArray has gone since the class variable UI paramters are stored in static ParamBlocks
//      all corresponding class vars have gone, including the ParamMaps since they are replaced 
//      by the new descriptors
class GSphereObject : public SimpleObject2
{
	public:	
		// Class vars
		static IObjParam *ip;
		static BOOL typeinCreate;

// JBW: minimal constructor, call MakeAutoParamBlocks() on my ClassDesc to
//      have all the declared per-instance P_AUTO_CONSTRUCT blocks made, initialized and
//      wired in.
		GSphereObject() { GetGSphereDesc()->MakeAutoParamBlocks(this); }
		
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_GEOSPHERE); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist);

		// From GeomObject
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		
		// Animatable methods		
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return Class_ID(GSPHERE_CLASS_ID, 32670); } 
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

// JBW: the Load() post-load callback insertion has gone since versioning is 
//		handled automatically by virtue of permanent parameter IDs.  These IDs
//		are defined in enums and are never retired so that old versions can be
//		automatically re-mapped to new ones
//
//      Note that this is only true in new plug-ins; old plug-ins need to 
//		continue to support version re-mapping as before for version up until
//		converting to the new descriptors
		IOResult Load(ILoad *iload);
		
// JBW: all the IParamArray methods are gone since we don't need them for the class variables

		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
// JBW: the GetParamName() and GetParamDim() function have gone	as this all 
//      is available in the descriptors. REFMSG_GET_PARAM_NAME, etc. become unnecessary as well
};

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

//--- ClassDescriptor and class vars ---------------------------------

// The class descriptor for gsphere
class GSphereClassDesc: public ClassDesc2 
{
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new GSphereObject; }
	const TCHAR *	ClassName() { return GetString(IDS_GEOSPHERE_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(GSPHERE_CLASS_ID, 32670); }
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES); }
// JBW:  the ResetClassParams() has gone since this is automatic now
//       using the default values in the descriptors

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("GeoSphere"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

static GSphereClassDesc gsphereDesc;
ClassDesc* GetGSphereDesc() { return &gsphereDesc; }

IObjParam* GSphereObject::ip = NULL;
BOOL GSphereObject::typeinCreate       = FALSE;

// JBW:  all the old static class variables have gone, most now hosted in 
//       static param blocks

// Misc stuff
#define MAX_SEGMENTS	200
#define MIN_SEGMENTS	1

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float(1.0E30)

#define DEF_SEGMENTS	4
#define DEF_RADIUS		float(0.0)
#define DEF_BASETYPE	2
#define DEF_SMOOTH		1
#define DEF_HEMI		0
#define DEF_BASEPIVOT	0
#define DEF_MAPPING		0

// JBW: Here follows the new parameter block descriptors.  There are now 3, 
//      two new STATIC ones to hold the old class var parameters, one for the main
//		per-instance parameters.  Apart from all the extra 
//      metadata you see in the definitions, one important new idea is the
//      folding of ParamMap description info into the parameter descriptor and
//      providing a semi-automatic rollout desipaly mechanism.
//      

// Parameter Block definitions

// JBW: First come the position and version independent IDs for each
//      of the blocks and the parameters in each block.  These IDs are used
//	    in subsequent Get/SetValue() parameter access, etc. and for version-independent
//      load and save

// block IDs
enum { geo_creation_type, geo_type_in, geo_params, };
// geo_creation_type param IDs
enum { geo_create_meth, };
// geo_type_in param IDs
enum { geo_ti_pos, geo_ti_radius, };
// geo_param param IDs
enum { geo_radius, geo_segs, geo_type, geo_hemi, geo_smooth, geo_basepivot, geo_mapping, };

// JBW: here are the two static block descriptors.  This form of 
//      descriptor declaration uses a static NParamBlockDesc instance whose constructor
//      uses a varargs technique to walk through all the param definitions.
//      It has the advantage of supporting optional and variable type definitions, 
//      but may generate a tad more code than a simple struct template.  I'd
//      be interested in opinions about this.

//      I'll briefly describe the first definition so you can figure the others.  Note
//      that in certain places where strings are expected, you supply a string resource ID rather than
//      a string at it does the lookup for you as needed.
//
//		line 1: block ID, internal name, local (subanim) name, flags
//																 AUTO_UI here means the rollout will
//																 be automatically created (see BeginEditParams for details)
//      line 2: since AUTO_UI was set, this line gives: 
//				dialog resource ID, rollout title, flag test, appendRollout flags
//		line 3: required info for a parameter:
//				ID, internal name, type, flags, local (subanim) name
//		lines 4-6: optional parameter declaration info.  each line starts with a tag saying what
//              kind of spec it is, in this case default value, value range, and UI info as would
//              normally be in a ParamUIDesc less range & dimension
//	    the param lines are repeated as needed for the number of parameters defined.

// class creation type block
static ParamBlockDesc2 geo_crtype_blk ( geo_creation_type, _T("GeosphereCreationType"), 0, &gsphereDesc, P_CLASS_PARAMS + P_AUTO_UI, 
	//rollout
	IDD_GSPHERE1, IDS_CREATION_METHOD, BEGIN_EDIT_CREATE, 0, NULL,
	// params
	geo_create_meth,  _T("creationMethod"), 		TYPE_INT, 		0, IDS_CREATION_METHOD, 	 
		p_default, 		1, 
		p_range, 		0, 1, 
		p_ui, 			TYPE_RADIO, 	2, IDC_CREATEDIAMETER, IDC_CREATERADIUS, 
		end, 
	end
	);

// class type-in block
static ParamBlockDesc2 geo_typein_blk ( geo_type_in, _T("GeosphereTypeIn"),  0, &gsphereDesc, P_CLASS_PARAMS + P_AUTO_UI, 
	//rollout
	IDD_GSPHERE3, IDS_KEYBOARD_ENTRY, BEGIN_EDIT_CREATE, APPENDROLL_CLOSED, NULL,
	// params
	geo_ti_pos, 			_T("typeInPos"), 		TYPE_POINT3, 		0, 	IDS_RB_POS,
		p_default, 		Point3(0,0,0), 
		p_range, 		-99999999.0, 99999999.0, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_TI_POSX, IDC_TI_POSXSPIN, IDC_TI_POSY, IDC_TI_POSYSPIN, IDC_TI_POSZ, IDC_TI_POSZSPIN, SPIN_AUTOSCALE, 
		end, 
	geo_ti_radius, 		_T("typeInRadius"), 		TYPE_FLOAT, 		0, 	IDS_RB_RADIUS, 
		p_default, 		25.0, 
		p_range, 		MIN_RADIUS, MAX_RADIUS, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_RADIUS, IDC_RADSPINNER, SPIN_AUTOSCALE, 
		end, 
	end
	);

// JBW: this descriptor defines the main per-instance parameter block.  It is flagged as AUTO_CONSTRUCT which
//      means that the CreateInstance() will automatically create one of these blocks and set it to the reference
//      number given (0 in this case, as seen at the end of the line).

// per instance geosphere block
static ParamBlockDesc2 geo_param_blk ( geo_params, _T("GeosphereParameters"),  0, &gsphereDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF_NO, 
	//rollout
	IDD_GSPHERE2, IDS_PARAMETERS, 0, 0, NULL,
	// params
	geo_hemi, 	_T("hemisphere"),		TYPE_BOOL, 		P_ANIMATABLE,				IDS_HEMI,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_HEMI, 
		end, 
	geo_segs, 	_T("segs"), 			TYPE_INT, 		P_ANIMATABLE, 	IDS_RB_SEGS, 
		p_default, 		4, 
		p_range, 		MIN_SEGMENTS, MAX_SEGMENTS, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_SEGMENTS, IDC_SEGSPINNER, 0.05f, 
		end, 
	geo_radius,  _T("radius"), 			TYPE_FLOAT, 	P_ANIMATABLE + P_RESET_DEFAULT, 	IDS_RB_RADIUS, 
		p_default, 		0.0,	
		p_ms_default,	25.0,
		p_range, 		MIN_RADIUS, MAX_RADIUS, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_RADIUS, IDC_RADSPINNER, SPIN_AUTOSCALE, 
		end, 
	geo_type, 	_T("baseType"),			TYPE_INT, 		0,				IDS_BASETYPE,
		p_default, 		2, 
		p_range, 		0, 2, 
		p_ui, 			TYPE_RADIO, 	3, IDC_TETRA, IDC_OCTA, IDC_ICOSA, 
		end, 
	geo_basepivot, 	_T("baseToPivot"), 	TYPE_BOOL, 		0,				IDS_BASEPIVOT, 
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_BASEPIVOT, 
		end, 
	geo_smooth, 	_T("smooth"), 		TYPE_BOOL, 		P_ANIMATABLE,				IDS_RB_SMOOTH,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_OBSMOOTH, 
		end, 
	geo_mapping, 	_T("mapCoords"), 	TYPE_BOOL, 		0,				IDS_MAPPING,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_MAPPING, 
		end, 
	end
	);

// JBW: pre-ParamBlock2 version info, similar to old scheme except here we
//      install ParamBlockDesc2 parameter IDs into the ParamBlockDescIDs for
//      each version so the post-load callback can do the right mapping to the new ParamBlock2
#define NUM_OLDVERSIONS	3

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, geo_radius },		
	{ TYPE_INT, NULL, TRUE,   geo_segs },
	{ TYPE_INT, NULL, FALSE,  geo_type },
	{ TYPE_INT, NULL, FALSE,  geo_smooth }
};

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, geo_radius },		
	{ TYPE_INT, NULL, TRUE,   geo_segs },
	{ TYPE_INT, NULL, FALSE,  geo_type },
	{ TYPE_INT, NULL, FALSE,  geo_hemi },
	{ TYPE_INT, NULL, FALSE,  geo_smooth }
};

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, geo_radius },
	{ TYPE_INT, NULL, TRUE,   geo_segs },
	{ TYPE_INT, NULL, FALSE,  geo_type },
	{ TYPE_INT, NULL, FALSE,  geo_hemi },
	{ TYPE_INT, NULL, FALSE,  geo_smooth },
	{ TYPE_INT, NULL, FALSE,  geo_basepivot },
	{ TYPE_INT, NULL, FALSE,  geo_mapping }
};
#define PBLOCK_LENGTH	7  

// Array of old ParamBlock Ed. 1 versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc (descVer0, 4, 0),
	ParamVersionDesc (descVer1, 5, 1),
	ParamVersionDesc (descVer2, 7, 2),
};

// JBW:  for loading old ParamBlock versions, register an Ed. 2 param block converter post-load callback
//       This one takes the ParamBlockDescID version array as before, but now takes the 
//       ParamBlockDesc2 for the block to describe the current version and will convert the loaded ParamBlock
//       into a corresponding ParamBlock2
IOResult
GSphereObject::Load(ILoad *iload) 
{	
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &geo_param_blk, this, PBLOCK_REF_NO);
	iload->RegisterPostLoadCallback(plcb);
	return IO_OK;
}

//--- TypeInDlgProc --------------------------------
class GSphereTypeInDlgProc : public ParamMap2UserDlgProc {
public:
	GSphereObject *so;

	GSphereTypeInDlgProc(GSphereObject *s) {so=s;}
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {delete this;}
};

BOOL GSphereTypeInDlgProc::DlgProc(
		TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg != WM_COMMAND) return FALSE;
	if (LOWORD(wParam) != IDC_TI_CREATE) return FALSE;
// JBW: access to STATIC paramblock data is via functions on
//      on the descriptor
	if (geo_typein_blk.GetFloat(geo_ti_radius) == 0.0) return TRUE;
	
	// We only want to set the value if the object is 
	// not in the scene.
	if (so->TestAFlag(A_OBJ_CREATING)) {
		so->pblock2->SetValue(geo_radius, 0, geo_typein_blk.GetFloat(geo_ti_radius));
		}
	so->typeinCreate = TRUE;

	Matrix3 tm(1);
	tm.SetTrans(geo_typein_blk.GetPoint3(geo_ti_pos));
	so->suspendSnap = FALSE;
	so->ip->NonMouseCreate(tm);
	// NOTE that calling NonMouseCreate will cause this
	// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
	return TRUE;	
}

//--- GSphere methods -------------------------------

// JBW: the GeoSphere constructor has gone.  The paramblock creation and wiring and
//		the intial value setting is automatic now.

// JBW: BeginEditParams() becomes much simpler with automatic UI param blocks.
//      you redirect the BeginEditParams() to the ClassDesc instance and it
//      throws up the appropriate rollouts.

void GSphereObject::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	SimpleObject::BeginEditParams(ip, flags, prev);
	this->ip = ip;

	// If this has been freshly created by type-in, set creation values:
	if (typeinCreate) {
		pblock2->SetValue (geo_radius, 0, geo_typein_blk.GetFloat(geo_ti_radius));
		typeinCreate = FALSE;
	}

	// throw up all the appropriate auto-rollouts
	gsphereDesc.BeginEditParams(ip, this, flags, prev);
	// install a callback for the type in.
	geo_typein_blk.SetUserDlgProc(new GSphereTypeInDlgProc(this));
}
		
// JBW: similarly for EndEditParams and you also don't have to snapshot
//		current parameter values as initial values for next object as
//		this is automatic for the new ParamBlock params unless disabled.

void GSphereObject::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{		
	SimpleObject::EndEditParams(ip, flags, next);
	this->ip = NULL;
	// tear down the appropriate auto-rollouts
	gsphereDesc.EndEditParams(ip, this, flags, next);
}

// CONSTRUCTING THE MESH:

// To construct a geodesic sphere, we take a tetrahedron, subdivide each face into
// segs^2 faces, and project the vertices onto the sphere of the correct radius.

// This subdivision produces 3 kinds of vertices: 4 "corner" vertices, which are the
// original tetrahedral vertices; "edge" vertices, those that lie on the tetrahedron's
// edges, and "face" vertices.  There are 6 edges with (segs-1) verts on each, and
// 4 faces with (segs-1)*(segs-2)/2 verts.

// We construct these vertices in this order: the first four are the corner vertices.
// Then we use spherical interpolation to place edge vertices along each edge.
// Finally, we use the same interpolation to produce face vertices between the edge
// vertices.


// Assumed in the following function: the vertices have the same radius, or
// distance from the origin, and they have nontrivial cross product.

void SphericalInterpolate (Mesh& amesh, int v1, int v2, int *current, int num)
	{
	int i;
	float theta, theta1, theta2, sn, cs, rad;
	Point3 a, b, c;

	if (num<2) { return; }
	a=amesh.getVert (v1);
	b=amesh.getVert (v2);
	rad = DotProd (a, a);
	if (rad==0) {
		for (i=1; i<num; i++) amesh.setVert ((*current)++, a);
		return;
	}
	cs = DotProd (a, b) / rad;
	LimitValue (cs, -1.0f, 1.0f);
	theta = (float) acos (cs);
	sn = Sin (theta);

	for (i=1; i<num; i++) {
		theta1 = (theta*i)/num;
		theta2 = (theta*(num-i))/num;
		c = (a*Sin(theta2) + b*Sin(theta1))/sn;
		amesh.setVert ((*current)++, c);
	}
}

// The order we produced the verts in makes for difficult face construction.  Thus
// we use the following functions to find the index of the vertex in a given row, 
// given column, of a given basetype face.

// (Each face is viewed as a triangular grid of vertices.  The orientation of the
// triangle is such that it "points up", so rows are "horizontal" while columns
// are diagonal: column 0 is the left edge of the triangle.)

int tetra_find_vert_index (int s, int f, int r, int c)
{						// segs, face, row, column.
	if (r==0) {
		if (f<3) { return 0; }
		else { return 1; }
	}
	
	if (c==0) {
		if (r==s) {
			if (f<3) { return f+1; }
			else { return 3; }
		}
		if (f<3) { return 4+(s-1)*f+r-1; }
		else { return 4+(s-1)*4+r-1; }
	}

	if (c==r) {
		if (r==s) {
			if (f<3) { return (f+1)%3+1; }
			else { return 2; }
		}
		else {
			if (f<3) { return 4+(s-1)*((f+1)%3)+r-1; }
			else { return 4+(s-1)*3+r-1; }
		}
	}

	if (r==s) {
		switch (f) {
		case 0: return 4+(s-1)*3 + c-1;
		case 1: return 4+(s-1)*5 + c-1;
		case 2:	return 4+(s-1)*4 + s-1-c;
		case 3:	return 4+(s-1)*5 + s-1-c;
		}
	}
	
	return 4 + (s-1)*6 + f*(s-1)*(s-2)/2 + (r-2)*(r-1)/2 + c-1;
}

int hemi_tetra_find_vert_index (int s, int f, int r, int c)
{								// segs, face, row, column.
	if (r==0) return 0;
	
	if (c==0) {
		if (r==s) return f+1;
		return 4+(s-1)*f+r-1;
	}

	if (c==r) {
		if (r==s) return (f+1)%3+1;
		return 4+(s-1)*((f+1)%3)+r-1;
	}

	if (r==s) return 4 + (s-1)*(f+3) + c-1;
	
	return 4 + (s-1)*6 + f*(s-1)*(s-2)/2 + (r-2)*(r-1)/2 + c-1;
}

int octa_find_vert_index (int s, int f, int r, int c)
{						// segs, face, row, column.

	if (r==0) { // Top corner of face
		if (f<4) return 0;
		return 5;
	}

	if (r==s) {
		if (((f<4)&&(c==0)) || ((f>3)&&(c==r))) return f%4+1;
		if (((f>3)&&(c==0)) || ((f<4)&&(c==r))) return (f+1)%4+1;
		if (f<4) return 6+(s-1)*(8+f)+c-1;
		return 6+(s-1)*(4+f)+s-1-c;
	}

	if (c==0) {  // r is between 0 and s.
		if (f<4) return 6+(s-1)*f + r-1;
		return 6+(s-1)*((f+1)%4+4) + r-1;
	}
	
	if (c==r) {
		if (f<4) return 6+(s-1)*((f+1)%4) + r-1;
		return 6+(s-1)*f + r-1;
	}

	return 6 + (s-1)*12 + f*(s-1)*(s-2)/2 + (r-1)*(r-2)/2 + c-1;
}

int hemi_octa_find_vert_index (int s, int f, int r, int c)
{								// segs, face, row, column.
	if (r==0) return 0;

	if (r==s) {	// Bottom row.
		if (c==0) return f+1;
		if (c==r) return (f+1)%4+1;
		return 5+(s-1)*(4+f)+c-1;
	}

	if (c==0) return 5+(s-1)*f + r-1;
	if (c==r) return 5+(s-1)*((f+1)%4) + r-1;

	return 5 + (s-1)*8 + f*(s-1)*(s-2)/2 + (r-1)*(r-2)/2 + c-1;
}

int icosa_find_vert_index (int s, int f, int r, int c)
{							// segs, face, row, column

	if (r==0) {	// Top corner of face
		if (f<5) { return 0; }
		if (f>14) { return 11; }
		return f-4;
	}

	if ((r==s) && (c==0)) { // Lower left corner of face
		if (f<5) { return f+1; }
		if (f<10) { return (f+4)%5 + 6; }
		if (f<15) { return (f+1)%5 + 1; }
		return (f+1)%5 + 6;
	}
	
	if ((r==s) && (c==s)) { // Lower right corner
		if (f<5) { return (f+1)%5+1; }
		if (f<10) { return f+1; }
		if (f<15) { return f-9; }
		return f-9;
	}

	if (r==s) { // Bottom edge
		if (f<5) { return 12 + (5+f)*(s-1) + c-1; }
		if (f<10) { return 12 + (20+(f+4)%5)*(s-1) + c-1; }
		if (f<15) { return 12 + (f-5)*(s-1) + s-1-c; }
		return 12 + (5+f)*(s-1) + s-1-c;
	}

	if (c==0) { // Left edge
		if (f<5) { return 12 + f*(s-1) + r-1; }
		if (f<10) { return 12 + (f%5+15)*(s-1) + r-1; }
		if (f<15) { return 12 + ((f+1)%5+15)*(s-1) + s-1-r; }
		return 12 + ((f+1)%5+25)*(s-1) + r-1;
	}

	if (c==r) { // Right edge
		if (f<5) { return 12 + ((f+1)%5)*(s-1) + r-1; }
		if (f<10) { return 12 + (f%5+10)*(s-1) + r-1; }
		if (f<15) { return 12 + (f%5+10)*(s-1) + s-1-r; }
		return 12 + (f%5+25)*(s-1) + r-1;
	}
	
	// Not an edge or corner.
	return 12 + 30*(s-1) + f*(s-1)*(s-2)/2 + (r-1)*(r-2)/2 + c-1;
}

int hemi_icosa_find_vert_index (int s, int f, int r, int c)
{								// segs, face, row, column
	int frow = f/5, f0=f%5, f1=(f+1)%5, f4=(f+4)%5;
	int grow = f/10, g0=f%10, g1=(f+1)%10, g9=(f+9)%10;
	int parity = f%2, h0=g0/2, h1=(h0+1)%5, h4=(h0+4)%5;

	if (r==0) {	// Top corner of face
		switch (grow) {
		case 0: return frow ? f0+11 : 0;
		case 1: return h0 + 1;
		case 2:	return g0 + 16;
		case 3:	return parity ? h0+11 : h0+6;
		}
	}

	if ((r==s) && (c==0)) { // Lower left corner of face
		switch (grow) {
		case 0:	return frow ? f1+1 : f0+1;
		case 1:	return parity ? h0+6 : h4+11;
		case 2:	return parity ? h1+6 : h0+11;
		case 3:	return g9+16;
		}
	}

	if ((r==s) && (c==s)) { // Lower right corner
		switch (grow) {
		case 0: return frow ? f0+1 : f1+1;
		case 1: return parity ? h0+11 : h0+6;
		case 2: return parity ? h0+11 : h0+6;
		case 3: return g0+16;
		}
	}

	if (r==s) { // Bottom edge
		switch (grow) {
		case 0:
			if (frow)	return 26 + (5+f0)*(s-1) + s-1-c;
						return 26 + (5+f0)*(s-1) + c-1;
		case 1:
			if (parity)	return 26 + (h0+25)*(s-1) + c-1;
						return 26 + (h0+30)*(s-1) + c-1;
		case 2:
			if (parity) return 26 + (h1+30)*(s-1) + s-1-c;
						return 26 + (h0+25)*(s-1) + s-1-c;
		case 3:			return 26 + (g9+55)*(s-1) + c-1;
		}
	}

	if (c==0) { // Left edge
		switch (grow) {
		case 0:
			if (frow)	return 26 + (20+f1)*(s-1) + s-1-r;
						return 26 + f0*(s-1) + r-1;
		case 1:
			if (parity)	return 26 + (h0+10)*(s-1) + r-1;
						return 26 + (h0+20)*(s-1) + r-1;
		case 2:
			if (parity)	return 26 + (38+h0*4)*(s-1) + s-1-r;
						return 26 + (36+h0*4)*(s-1) + s-1-r;
		case 3:			return 26 + (g9*2+36)*(s-1) + r-1;
		}
	}

	if (c==r) { // Right edge
		switch (grow) {
		case 0:
			if (frow)	return 26 + (15+f0)*(s-1) + s-1-r;
						return 26 +	f1*(s-1) + r-1;
		case 1:
			if (parity)	return 26 + (h0+15)*(s-1) + r-1;
						return 26 + (h0+10)*(s-1) + r-1;
		case 2:
			if (parity)	return 26 + (37+h0*4)*(s-1) + s-1-r;
						return 26 + (35+h0*4)*(s-1) + s-1-r;
		case 3:			return 26 + (g0*2+35)*(s-1) + r-1;
		}
	}
	
	// Not an edge or corner.
	return 26 + 65*(s-1) + f*(s-1)*(s-2)/2 + (r-1)*(r-2)/2 + c-1;
}

int find_vert_index (int basetype, int segs, int face, int row, int column)
{
	switch (basetype) {
	case 0: return tetra_find_vert_index (segs, face, row, column);
	case 1: return hemi_tetra_find_vert_index (segs, face, row, column);
	case 2: return octa_find_vert_index (segs, face, row, column);
	case 3: return hemi_octa_find_vert_index (segs, face, row, column);
	case 4: return icosa_find_vert_index (segs, face, row, column);
	default: return hemi_icosa_find_vert_index (segs, face, row, column);
	}
}


BOOL GSphereObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock2->GetValue(geo_mapping, 0, genUVs, v);
	return genUVs; 
	}

void GSphereObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock2->SetValue(geo_mapping, 0, sw);				
	}


// Now put it all together sensibly
#define EPSILON 1e-5f
void GSphereObject::BuildMesh(TimeValue t)
	{
	int i;
	int nf=0, nv=0;
	int segs, basetype, nverts, nfaces, nsections;
	int row, column, face, a, b, c, d;
	float radius, subrad, subz, theta, sn, cs;
	BOOL smooth, hemi, tverts, basePivot;

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	pblock2->GetValue(geo_radius, t, radius, ivalid);
	pblock2->GetValue(geo_segs, t, segs, ivalid);
	pblock2->GetValue(geo_hemi, t, hemi, ivalid);
	pblock2->GetValue(geo_smooth, t, smooth, ivalid);
	pblock2->GetValue(geo_type, t, basetype, ivalid);
	pblock2->GetValue(geo_basepivot, t, basePivot, ivalid);
	pblock2->GetValue(geo_mapping, t, tverts, ivalid);
	LimitValue(segs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);
	
	basetype = basetype*2 + hemi;

	switch (basetype) {
		case 0:  nsections = 4;  break;
		case 1:  nsections = 3;  break;
		case 2:  nsections = 8;  break;
		case 3:  nsections = 4;  break;
		case 4:  nsections = 20; break;
		default: nsections = 40; break;
	}

	if (hemi) {
		nfaces = nsections * segs*segs + ((basetype==5) ? (segs*10) : (segs*nsections));
		nverts = nsections * (segs-1)*(segs-2)/2 + 1;	// face verts, including origin.
		switch (basetype) {
		case 1:
			nverts += 6*(segs-1) + 4;
			break;
		case 3:
			nverts += 8*(segs-1) + 5;
			break;
		case 5:
			nverts += 65*(segs-1) + 26;
			break;
		}
	} else {
		nfaces = nsections * segs * segs;
		nverts = nfaces/2 + 2;
	}
	
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(smooth != 0);
	mesh.setNumTVerts (0);
	mesh.setNumTVFaces (0);

	switch (basetype) {
	case 0: // Based on tetrahedron
		// First four tetrahedral vertices 
		mesh.setVert (nv++, (float)0.0, (float)0.0, radius);
		mesh.setVert (nv++, radius*(Sqrt(8.f/9.f)), (float)0.0, -radius/((float)3.));
		mesh.setVert (nv++, -radius*(Sqrt(2.f/9.f)), radius*(Sqrt(2.f/3.f)), -radius/((float)3.));
		mesh.setVert (nv++, -radius*(Sqrt(2.f/9.f)), -radius*(Sqrt(2.f/3.f)), -radius/((float)3.));

		// Edge vertices
		SphericalInterpolate (mesh, 0, 1, &nv, segs);
		SphericalInterpolate (mesh, 0, 2, &nv, segs);
		SphericalInterpolate (mesh, 0, 3, &nv, segs);
		SphericalInterpolate (mesh, 1, 2, &nv, segs);
		SphericalInterpolate (mesh, 1, 3, &nv, segs);
		SphericalInterpolate (mesh, 2, 3, &nv, segs);

		// Face vertices
		for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+i, 4+(segs-1)+i, &nv, i+1);
		for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+(segs-1)+i, 4+2*(segs-1)+i, &nv, i+1);
		for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+2*(segs-1)+i, 4+i, &nv, i+1);
		for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+4*(segs-1)+i, 4+3*(segs-1)+i, &nv, i+1);
		break;

	case 1:	// Hemi-tetrahedron
		// Modified four tetrahedral verts.  (Yes, I'm making this up -- no, it's not really geodesic)
		mesh.setVert (nv++, 0.0f, 0.0f, radius);
		mesh.setVert (nv++, radius, 0.0f, 0.0f);
		mesh.setVert (nv++, -radius/2.0f, radius*(Sqrt(.75)), 0.0f);
		mesh.setVert (nv++, -radius/2.0f, -radius*(Sqrt(.75)), 0.0f);

		// Edge Vertices
		SphericalInterpolate (mesh, 0, 1, &nv, segs);
		SphericalInterpolate (mesh, 0, 2, &nv, segs);
		SphericalInterpolate (mesh, 0, 3, &nv, segs);
		SphericalInterpolate (mesh, 1, 2, &nv, segs);
		SphericalInterpolate (mesh, 2, 3, &nv, segs);
		SphericalInterpolate (mesh, 3, 1, &nv, segs);

		// Face vertices
		for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+i, 4+(segs-1)+i, &nv, i+1);
		for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+(segs-1)+i, 4+2*(segs-1)+i, &nv, i+1);
		for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+2*(segs-1)+i, 4+i, &nv, i+1);

		// Last point:
		mesh.setVert (nv++, 0.0f, 0.0f, 0.0f);

		break;

	case 2: //Based on the Octahedron
		// First 6 octahedral vertices
		mesh.setVert (nv++, (float)0, (float)0, radius);
		mesh.setVert (nv++, radius, (float)0, (float)0);
		mesh.setVert (nv++, (float)0, radius, (float)0);
		mesh.setVert (nv++, -radius, (float)0, (float)0);
		mesh.setVert (nv++, (float)0, -radius, (float)0);
		mesh.setVert (nv++, (float)0, (float)0, -radius);

		// Edge vertices
		for (face=0; face<4; face++) SphericalInterpolate (mesh, 0, face+1, &nv, segs);
		for (face=0; face<4; face++) SphericalInterpolate (mesh, 5, face+1, &nv, segs);
		for (face=0; face<4; face++) SphericalInterpolate (mesh, face+1, (face+1)%4+1, &nv, segs);

		// Face vertices
		for (face=0; face<4; face++) {
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 6+face*(segs-1)+i, 6+((face+1)%4)*(segs-1)+i, &nv, i+1);
			}
		}
		for (face=0; face<4; face++) {
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 6+((face+1)%4+4)*(segs-1)+i, 6+(face+4)*(segs-1)+i, &nv, i+1);
			}
		}
		break;

	case 3: // Hemioctahedron -- this one's perfect every time.
		// First 5 octahedral vertices
		mesh.setVert (nv++, (float)0, (float)0, radius);
		mesh.setVert (nv++, radius, (float)0, (float)0);
		mesh.setVert (nv++, (float)0, radius, (float)0);
		mesh.setVert (nv++, -radius, (float)0, (float)0);
		mesh.setVert (nv++, (float)0, -radius, (float)0);

		// Edge vertices
		for (face=0; face<4; face++) SphericalInterpolate (mesh, 0, face+1, &nv, segs);
		for (face=0; face<4; face++) SphericalInterpolate (mesh, face+1, (face+1)%4+1, &nv, segs);

		// Face vertices
		for (face=0; face<4; face++) {
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 5+face*(segs-1)+i, 5+((face+1)%4)*(segs-1)+i, &nv, i+1);
			}
		}

		// Last point:
		mesh.setVert (nv++, 0.0f, 0.0f, 0.0f);
		break;

	case 4:  // Based on the Icosahedron
		// First 12 icosahedral vertices
		mesh.setVert (nv++, (float)0, (float)0, radius);
		subz = Sqrt (.2f) * radius;
		subrad = 2*subz;
		for (face=0; face<5; face++) {
			theta = 2*PI*face/5;
			SinCos (theta, &sn, &cs);
			mesh.setVert (nv++, subrad*cs, subrad*sn, subz);
		}
		for (face=0; face<5; face++) {
			theta = PI*(2*face+1)/5;
			SinCos (theta, &sn, &cs);
			mesh.setVert (nv++, subrad*cs, subrad*sn, -subz);
		}
 		mesh.setVert (nv++, (float)0, (float)0, -radius);

		// Edge vertices: 6*5*(segs-1) of these.
		for (face=0; face<5; face++) SphericalInterpolate (mesh, 0, face+1, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, (face+1)%5+1, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, face+6, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, (face+4)%5+6, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+6, (face+1)%5+6, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, 11, face+6, &nv, segs);
		
		// Face vertices: 4 rows of 5 faces each.
		for (face=0; face<5; face++) {
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 12+face*(segs-1)+i, 12+((face+1)%5)*(segs-1)+i, &nv, i+1);
			}
		}	
		for (face=0; face<5; face++) {
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 12+(face+15)*(segs-1)+i, 12+(face+10)*(segs-1)+i, &nv, i+1);
			}
		}	
		for (face=0; face<5; face++) {
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 12+((face+1)%5+15)*(segs-1)+segs-2-i, 12+(face+10)*(segs-1)+segs-2-i, &nv, i+1);
			}
		}	
		for (face=0; face<5; face++) {
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 12+((face+1)%5+25)*(segs-1)+i, 12+(face+25)*(segs-1)+i, &nv, i+1);
			}
		}
		break;	

	case 5:  // HemiIcosahedron, based loosely on a segs-2 icosahedron
		// First 26 icosahedral vertices: 
		mesh.setVert (nv++, 0.0f, 0.0f, radius);
		subz = Sqrt (.2f) * radius;
		subrad = 2*subz;
		for (face=0; face<5; face++) {
			theta = 2*PI*face/5.0f;
			SinCos (theta, &sn, &cs);
			mesh.setVert (face+6, subrad*cs, subrad*sn, subz);
			SphericalInterpolate (mesh, 0, face+6, &nv, 2);
		}
		nv += 5;
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+6, (face+1)%5+6, &nv, 2);
		for (face=0; face<10; face++) {
			theta = 2*PI*face/10.0f + PI/10.0f;
			SinCos (theta, &sn, &cs);
			mesh.setVert (nv++, radius*cs, radius*sn, 0.0f);
		}

		// Edge vertices: 13*5*(segs-1) of these.
		for (face=0; face<5; face++) SphericalInterpolate (mesh, 0, face+1, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, (face+1)%5+1, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, face+6, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, face+11, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, (face+4)%5+11, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, face+6, face+11, &nv, segs);
		for (face=0; face<5; face++) SphericalInterpolate (mesh, (face+4)%5+11, face+6, &nv, segs);
		for (face=0; face<5; face++) {
			SphericalInterpolate (mesh, face+6, face*2+16, &nv, segs);
			SphericalInterpolate (mesh, face+11, face*2+16, &nv, segs);
			SphericalInterpolate (mesh, face+11, face*2+17, &nv, segs);
			SphericalInterpolate (mesh, (face+1)%5+6, face*2+17, &nv, segs);
		}
		for (face=0; face<10; face++) SphericalInterpolate (mesh, face+16, (face+1)%10+16, &nv, segs);
		
		// Face vertices: representing 40 faces, 8 groups of 5.
		for (face=0; face<5; face++) {	// Top 5:
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 26+face*(segs-1)+i, 26+((face+1)%5)*(segs-1)+i, &nv, i+1);
			}
		}	
		for (face=0; face<5; face++) {	// Stellation of above
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 26+((face+1)%5+20)*(segs-1)+segs-2-i, 26+(face+15)*(segs-1)+segs-2-i, &nv, i+1);
			}
		}	
		for (face=0; face<5; face++) {	// Rest of this level
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 26+(face+20)*(segs-1)+i, 26+(face+10)*(segs-1)+i, &nv, i+1);
			}
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 26+(face+10)*(segs-1)+i, 26+(face+15)*(segs-1)+i, &nv, i+1);
			}
		}	
		for (face=0; face<10; face++) {	// Downward-pointing faces in lowest level
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 26+(face*2+36)*(segs-1)+segs-2-i, 26+(face*2+35)*(segs-1)+segs-2-i, &nv, i+1);
			}
		}
		for (face=0; face<10; face++) {	// Upward-pointing faces in lowest level
			int f9 = (face+9)%10;
			for (i=1; i<segs-1; i++) {
				SphericalInterpolate (mesh, 26+(f9*2+36)*(segs-1)+i, 26+(face*2+35)*(segs-1)+i, &nv, i+1);
			}
		}

		mesh.setVert (nv++, 0.0f, 0.0f, 0.0f);
		break;	
	}

	// Now make faces 
	// Set all smoothing, edge flags
	for (i=0; i<nfaces; i++) mesh.faces[i].setEdgeVisFlags(1, 1, 1);
	if (!smooth) for (i=0; i<nfaces; i++) mesh.faces[i].setSmGroup (0);
	else {
		if (hemi) {
			int sp_faces = nsections*segs*segs;
			for (i=0; i<sp_faces; i++) {
				mesh.faces[i].setSmGroup (1);
				mesh.faces[i].setMatID (1);
			}
			for (; i<nfaces; i++) {
				mesh.faces[i].setSmGroup (2);
				mesh.faces[i].setMatID (0);
			}
		} else {
			for (i=0; i<nfaces; i++) {
				mesh.faces[i].setSmGroup(1);
				mesh.faces[i].setMatID (1);
			}
		}
	}

	for (face=0; face<nsections; face++) {
		for (row=0; row<segs; row++) {
			for (column=0; column<=row; column++) {
				a = find_vert_index (basetype, segs, face, row, column);
				b = find_vert_index (basetype, segs, face, row+1, column);
				c = find_vert_index (basetype, segs, face, row+1, column+1);
				mesh.faces[nf].setVerts (a, b, c);
				nf++;
				if (column<row) {
					d = find_vert_index (basetype, segs, face, row, column+1);
					mesh.faces[nf].setVerts (a, c, d);
					nf++;
				}
			}
		}
	}
	
	if (hemi) {
		int bfaces, bfoffset;
		switch (basetype) {
		case 1: bfaces=3, bfoffset=0; break;
		case 3: bfaces=4, bfoffset=0; break;
		case 5: bfaces=10, bfoffset=30; break;
		}
		for (face=0; face<bfaces; face++) {
			for (column=0; column<segs; column++) {
				a = find_vert_index (basetype, segs, face+bfoffset, segs, column);
				b = find_vert_index (basetype, segs, face+bfoffset, segs, column+1);
				c = nv-1;
				mesh.faces[nf].setVerts (b, a, c);
				nf++;
			}
		}
	}

	// Textured verts, if required: do standard spherical deal.
	Matrix3 id(1);
	if (tverts) mesh.ApplyUVWMap (MAP_SPHERICAL, 1.0f, 1.0f, 1.0f, 0, 0, 0, 0, id);

	if (basePivot && (!hemi)) for (i=0; i<nverts; i++) mesh.verts[i].z += radius;

	mesh.InvalidateTopologyCache();
}

class GSphereObjCreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;
	GSphereObject *ob;
	Point3 p0;
public:
	int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj(GSphereObject *obj) {ob = obj;}
};

int GSphereObjCreateCallBack::proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	Point3 p1, center;

	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
	}


	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
		case 0:  // only happens with MOUSE_POINT msg
			ob->suspendSnap = TRUE;				
			sp0 = m;
			p0 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
			mat.SetTrans(p0);
			break;
		case 1:
			mat.IdentityMatrix();
			if (geo_crtype_blk.GetInt(geo_create_meth)) {
				p1 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
				r = Length(p1-p0);
				mat.SetTrans(p0);
			} else {
				p1 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
				center = (p0+p1)/float(2);
				mat.SetTrans(center);
				r = Length(center-p0);
				mat.SetTrans(center);
			} 
			ob->pblock2->SetValue(geo_radius, 0, r);
			geo_param_blk.InvalidateUI();

			if (flags&MOUSE_CTRL) {
				float ang = (float)atan2(p1.y-p0.y, p1.x-p0.x);					
				mat.PreRotateZ(ob->ip->SnapAngle(ang));
			}

			if (msg==MOUSE_POINT) {
				ob->suspendSnap = FALSE;
				return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
			}
			break;					   
		}
	} else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}

	return TRUE;
}

static GSphereObjCreateCallBack gsphereCreateCB;

CreateMouseCallBack* GSphereObject::GetCreateMouseCallBack() 
	{
	gsphereCreateCB.SetObj(this);
	return(&gsphereCreateCB);
	}


BOOL GSphereObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock2->GetValue(geo_radius, t, radius, FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}

// From GeomObject
int GSphereObject::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
	{
	//SS 9/29/2000 KENNY MERGE: Looks as though the simple calculation below;
	// based solely on the radius, doesn't take into account changes to the
	// hemisphere or base pivot parameters. This seems like a general fix,
	// so it's being rolled into Magma.
	int smooth, hemi, basePivot;
	pblock2->GetValue(geo_smooth, t, smooth, FOREVER);
	pblock2->GetValue(geo_hemi, t, hemi, FOREVER);
	pblock2->GetValue(geo_basepivot, t, basePivot, FOREVER);
	if (!smooth || hemi || basePivot) {
	//if (!smooth) {
		return SimpleObject::IntersectRay(t, ray, at, norm);
		}	
	
	float r;
	float a, b, c, ac4, b2, at1, at2;
	float root;
	BOOL neg1, neg2;

	pblock2->GetValue(geo_radius, t, r, FOREVER);

	a = DotProd(ray.dir, ray.dir);
	b = DotProd(ray.dir, ray.p) * 2.0f;
	c = DotProd(ray.p, ray.p) - r*r;
	
	ac4 = 4.0f * a * c;
	b2 = b*b;

	if (ac4 > b2) return 0;

	// We want the smallest positive root
	root = Sqrt(b2-ac4);
	at1 = (-b + root) / (2.0f * a);
	at2 = (-b - root) / (2.0f * a);
	neg1 = at1<0.0f;
	neg2 = at2<0.0f;
	if (neg1 && neg2) return 0;
	else
	if (neg1 && !neg2) at = at2;
	else 
	if (!neg1 && neg2) at = at1;
	else
	if (at1<at2) at = at1;
	else at = at2;
	
	norm = Normalize(ray.p + at*ray.dir);

	return 1;
	}

void GSphereObject::InvalidateUI() 
{
	// if this was caused by a NotifyDependents from pblock2, LastNotifyParamID()
	// will contain ID to update, else it will be -1 => inval whole rollout
	geo_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}

RefTargetHandle GSphereObject::Clone(RemapDir& remap) 
{
	GSphereObject* newob = new GSphereObject();	
	newob->ReplaceReference(0, pblock2->Clone(remap));
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
}

#ifndef NO_NURBS

Object *
BuildNURBSSphere(float radius, BOOL hemi, BOOL recenter, BOOL genUVs)
{
	NURBSSet nset;

	Point3 center(0, 0, 0);
	Point3 northAxis(0, 0, 1);
	Point3 refAxis(0, -1, 0);

	float startAngleU = -PI;
	float endAngleU = PI;
	float startAngleV;
	if (hemi)
		startAngleV = 0.0f;
	else
		startAngleV = -HALFPI;
	float endAngleV = HALFPI;
	if (recenter) {
		if (hemi)
			center = Point3(0.0f, 0.0f, 0.0f);
		else
			center = Point3(0.0f, 0.0f, radius);
	}

	NURBSCVSurface *surf = new NURBSCVSurface();
	nset.AppendObject(surf);
	surf->SetGenerateUVs(genUVs);

	if (hemi) {
		surf->SetTextureUVs(0, 0, Point2(0.0f, 0.5f));
		surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
		surf->SetTextureUVs(0, 2, Point2(1.0f, 0.5f));
		surf->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));
	} else {
		surf->SetTextureUVs(0, 0, Point2(0.0f, 0.0f));
		surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
		surf->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
		surf->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));
	}

	surf->FlipNormals(TRUE);
	surf->Renderable(TRUE);
	char bname[80];
	char sname[80];
	strcpy(bname, GetString(IDS_GEOSPHERE));
	sprintf(sname, "%s%s", bname, GetString(IDS_CT_SURF));
	surf->SetName(sname);
	GenNURBSSphereSurface(radius, center, northAxis, refAxis, 
					startAngleU, endAngleU, startAngleV, endAngleV,
					FALSE, *surf);

	if (hemi) {
		// now create caps on the ends
		NURBSCapSurface *cap0 = new NURBSCapSurface();
		nset.AppendObject(cap0);
		cap0->SetGenerateUVs(genUVs);
		cap0->SetParent(0);
		cap0->SetEdge(0);
		cap0->FlipNormals(TRUE);
		cap0->Renderable(TRUE);
		char sname[80];
		sprintf(sname, "%s%s%", bname, GetString(IDS_CT_CAP));
		cap0->SetName(sname);
	}

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
}

#endif

Object* GSphereObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_NURBS
	if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float radius;
		int recenter, hemi, genUVs;
		pblock2->GetValue(geo_radius, t, radius, valid);
		pblock2->GetValue(geo_hemi, t, hemi, valid);	
		pblock2->GetValue(geo_basepivot, t, recenter, valid);
		pblock2->GetValue(geo_mapping, t, genUVs, valid);
		Object *ob = BuildNURBSSphere(radius, hemi, recenter, genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM, valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM, valid);
		ob->UnlockObject();
		return ob;
		
	} else 
#endif

        {
            return SimpleObject::ConvertToType(t, obtype);
		}
	}

int GSphereObject::CanConvertToType(Class_ID obtype)
	{
#ifndef NO_NURBS
	if (obtype==defObjectClassID ||
		obtype==triObjectClassID || obtype==EDITABLE_SURF_CLASS_ID) {
#else
	if (obtype==defObjectClassID ||
		obtype==triObjectClassID) {
#endif
		return 1;
	} else {
		return SimpleObject::CanConvertToType(obtype);
		}
	}


void GSphereObject::GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1, &id);
    nlist.Append(1, &name);
#endif
}

#endif // NO_OBJECT_STANDARD_PRIMITIVES

