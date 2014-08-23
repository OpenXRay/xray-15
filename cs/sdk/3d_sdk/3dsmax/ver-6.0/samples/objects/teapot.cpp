/*****************************************************************************
 *<
	FILE: teapot.cpp

	DESCRIPTION:  Teapot object, Revised implementation

	CREATED BY: Charles Thaeler

        BASED ON : Sphere_C

	HISTORY: created 12/4/95

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/
#include "prim.h"
#include "iparamm.h"
#include "simpobj.h"
#include "surf_api.h"
#include "tea_util.h"

#ifndef NO_OBJECT_TEAPOT

// The teapot object class definition.  It is derived from SimpleObject and
// IParamArray.  SimpleObject is the class to derive objects from which have 
// geometry are renderable, and represent themselves using a mesh.  
// IParamArray is used as part of the Parameter Map scheme used to manage
// the user interface parameters.
class TeapotObject : public SimpleObject, public IParamArray {
	public:			
		// Class vars
		// There is only one set of these variables shared by all instances
		// of this class.  This is OK because there is only one Teapot
		// being edited at a time.
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static int dlgSegments;
		static int dlgCreateMeth;
		static int dlgSmooth;
        static int crtTeapart; // this is being kept for revisioning purposes
		static int dlgBody;
		static int dlgHandle;
		static int dlgSpout;
		static int dlgLid;
		static int dlgUVs;
		static Point3 crtPos;		
		static float crtRadius;
		// This is the Interface pointer into MAX.  It is used to call
		// functions implemented in MAX itself.  
		static IObjParam *ip;

		TeapotObject();

		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
			
		// This method allows the plug-in to provide MAX with a procedure
		// to manage user input from the mouse during creation of the 
		// Teapot object.
		CreateMouseCallBack* GetCreateMouseCallBack();
		// This method is called when the user may edit the Teapots
		// parameters.
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		// Called when the user is done editing the Teapots parameters.
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		// This is the name that appears in the history list (modifier stack).
		TCHAR *GetObjectName() { return GetString(IDS_RB_TEAPOT); }
		
		// From GeomObject 
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		
		// Animatable methods
		// Deletes the instance from memory.		
		void DeleteThis() {delete this;}
		// This returns the unique ClassID of the Teapot procedural object.
		Class_ID ClassID() { 
			return Class_ID(TEAPOT_CLASS_ID1,TEAPOT_CLASS_ID2); } 
		
		// From ReferenceTarget
		// Called by MAX when the teapot is being loaded from disk.
		IOResult Load(ILoad *iload);
				
		// From IParamArray
		// These methods provide MAX with access to the class variables
		// used by the parameter map.
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		// From SimpleObject
		// This method builds the mesh representation at the specified time.
		void BuildMesh(TimeValue t);
		// This method returns a flag to indicate if it is OK to 
		// display the object at the time passed.
		BOOL OKtoDisplay(TimeValue t);
		// This method informs the system that the user interface
		// controls need to be updated to reflect the current time.
		void InvalidateUI();
		// Returns the 'dimension' for the parameter whose index is
		// passed.  This is the type and order of magnitude of the 
		// parameter.
		ParamDimension *GetParameterDim(int pbIndex);
		// Returns the name of the parameter whose index is passed.
		TSTR GetParameterName(int pbIndex);
	};


// Misc stuff
#define MAX_SEGMENTS	64
#define MIN_SEGMENTS	1

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float(1.0E30)

#define MIN_SMOOTH		0
#define MAX_SMOOTH		2

#define DEF_SEGMENTS	4
#define DEF_RADIUS		float(0.0)

#define SMOOTH_ON		1
#define SMOOTH_OFF		0

#define POT_BODY		1
#define POT_HANDLE		1
#define POT_SPOUT		1
#define POT_LID			1
#define POT_UVS			1
#define OLD_POT_BOTH    0
#define OLD_POT_BODY    1
#define OLD_POT_LID     2
#define FIXED_OLD_VER   -1


//--- Parameter map/block descriptors -------------------------------
// The parameter map descriptors define the properties of a parameter
// such as the type (spinner, radio button, check box, etc.), which
// resource ID they refer to, and which index into the virtual array
// they use.

// Parameter block indices
#define PB_RADIUS		0
#define PB_SEGS			1
#define PB_SMOOTH		2
#define PB_TEAPART      3
#define PB_TEA_BODY		4
#define PB_TEA_HANDLE	5
#define PB_TEA_SPOUT	6
#define PB_TEA_LID		7
#define PB_GENUVS		8

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS		2

//
//
//	Creation method

static int createMethIDs[] = {IDC_CREATEDIAMETER,IDC_CREATERADIUS};

static ParamUIDesc descCreate[] = {
	// Diameter/radius
	ParamUIDesc(PB_CREATEMETHOD,TYPE_RADIO,createMethIDs,2)
	};
#define CREATEDESC_LENGH 1

//
//
// Type in

static ParamUIDesc descTypeIn[] = {
	
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_TI_POSX,IDC_TI_POSXSPIN,
		IDC_TI_POSY,IDC_TI_POSYSPIN,
		IDC_TI_POSZ,IDC_TI_POSZSPIN,
		-99999999.0f,99999999.0f,
		SPIN_AUTOSCALE),
	
	// Radius
	ParamUIDesc(
		PB_TI_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE)	
	};
#define TYPEINDESC_LENGH 2

//
//
// Parameters
static ParamUIDesc descParam[] = {
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	
	
	// Segments
	ParamUIDesc(
		PB_SEGS,
		EDITTYPE_INT,
		IDC_SEGMENTS,IDC_SEGSPINNER,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Smooth
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_OBSMOOTH),

    // Parts
 	ParamUIDesc(PB_TEAPART,     TYPE_SINGLECHEKBOX,IDC_TEAPART), // Obsolete but needed for versioning
 	ParamUIDesc(PB_TEA_BODY,	TYPE_SINGLECHEKBOX,IDC_TEA_BODY),
	ParamUIDesc(PB_TEA_HANDLE,	TYPE_SINGLECHEKBOX,IDC_TEA_HANDLE),
	ParamUIDesc(PB_TEA_SPOUT,	TYPE_SINGLECHEKBOX,IDC_TEA_SPOUT),
	ParamUIDesc(PB_TEA_LID,		TYPE_SINGLECHEKBOX,IDC_TEA_LID),

	// Generate Texture Mapping
	ParamUIDesc(PB_GENUVS,		TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),

	};
#define PARAMDESC_LENGH 9


// The parameter block descriptor defines the type of value represented
// by the parameter (int, float, Color...) and if it is animated or not.

// This class requires these values to be initialized:
// { - ParameterType, 
//   - Not Used, must be set to NULL, 
//   - Flag which indicates if the parameter is animatable,
//   - ID of the parameter used to match a corresponding ID in the 
//     other version of the parameter
//  }

// This one is the oldest version.
static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 } };

// This is the older version.
static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, TRUE, 3 } };

// This is the current version.
static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_BOOL, NULL, TRUE, 2 },
	{ TYPE_INT, NULL, FALSE, 3 },
	{ TYPE_BOOL, NULL, TRUE, 4 },
	{ TYPE_BOOL, NULL, TRUE, 5 },
	{ TYPE_BOOL, NULL, TRUE, 6 },
	{ TYPE_BOOL, NULL, TRUE, 7 },
	{ TYPE_BOOL, NULL, FALSE, 8 } };
#define PBLOCK_LENGTH	9

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,1),
	ParamVersionDesc(descVer1,4,2)
	};
#define NUM_OLDVERSIONS	2

// Current version
#define CURRENT_VERSION	3
static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TypeInDlgProc --------------------------------

class TeapotTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		TeapotObject *so;

		TeapotTypeInDlgProc(TeapotObject *s) {so=s;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,
			WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

// This is the method called when the user clicks on the Create button
// in the Keyboard Entry rollup.  It was registered as the dialog proc
// for this button by the SetUserDlgProc() method called from 
// BeginEditParams().
BOOL TeapotTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,
			WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (so->crtRadius==0.0) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (so->TestAFlag(A_OBJ_CREATING)) {
						so->pblock->SetValue(PB_RADIUS,0,so->crtRadius);
						}

					Matrix3 tm(1);
					tm.SetTrans(so->crtPos);
					so->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}


//--- Teapot methods -------------------------------

// Constructor
TeapotObject::TeapotObject()
{
	// Create the parameter block and make a reference to it.
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer2, PBLOCK_LENGTH, 
		CURRENT_VERSION));
	assert(pblock);

	// Initialize the default values.
	pblock->SetValue(PB_RADIUS,0,crtRadius);
	pblock->SetValue(PB_SMOOTH,0,dlgSmooth);
	pblock->SetValue(PB_SEGS,0,dlgSegments);	
    pblock->SetValue(PB_TEAPART,	0,  crtTeapart);
    pblock->SetValue(PB_TEA_BODY,	0,  dlgBody);
    pblock->SetValue(PB_TEA_HANDLE,	0,  dlgHandle);
    pblock->SetValue(PB_TEA_SPOUT,	0,  dlgSpout);
    pblock->SetValue(PB_TEA_LID,	0,  dlgLid);
    pblock->SetValue(PB_GENUVS,		0,  dlgUVs);
}

// Called by MAX when the Teapot object is loaded from disk.
IOResult TeapotObject::Load(ILoad *iload) 
{	
	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the MAX file 
	// being loaded.
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
}

// This method is called by the system when the user needs 
// to edit the objects parameters in the command panel.  
void TeapotObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	// We subclass off SimpleObject so we must call its
	// BeginEditParams() method first.
	SimpleObject::BeginEditParams(ip,flags,prev);
	// Save the interface pointer.
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last Teapot ceated
		pmapCreate->SetParamBlock(this);
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			// Here we create each new rollup page in the command panel
			// using our descriptors.
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_TEAPOTPARAM1),
				GetString(IDS_RB_CREATIONMETHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_TEAPOTPARAM3),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_TEAPOTPARAM2),
			GetString(IDS_RB_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		// This handles processing the Create button in the 
		// Keyboard Entry rollup page.
		pmapTypeIn->SetUserDlgProc(new TeapotTypeInDlgProc(this));
		}
	}
		
// This is called by the system to terminate the editing of the
// parameters in the command panel.  
void TeapotObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{		
	SimpleObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {
		// Remove the rollup pages from the command panel.
		if (pmapCreate) DestroyCPParamMap(pmapCreate);
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
		pmapCreate = NULL;
		}

	// Save these values in class variables so the next object 
	// created will inherit them.
	pblock->GetValue(PB_SEGS,		ip->GetTime(),dlgSegments,	FOREVER);
	pblock->GetValue(PB_SMOOTH,		ip->GetTime(),dlgSmooth,	FOREVER);	
    pblock->GetValue(PB_TEA_BODY,	ip->GetTime(),dlgBody,		FOREVER);
    pblock->GetValue(PB_TEA_HANDLE,	ip->GetTime(),dlgHandle,	FOREVER);
    pblock->GetValue(PB_TEA_SPOUT,	ip->GetTime(),dlgSpout,		FOREVER);
    pblock->GetValue(PB_TEA_LID,	ip->GetTime(),dlgLid,		FOREVER);
    pblock->GetValue(PB_GENUVS,		ip->GetTime(),dlgUVs,		FOREVER);
	}


static void blend_vector(Teapoint d0,Teapoint d1,Teapoint d2,Teapoint d3,
						 float t, Teapoint *result);

static int
GetShare(TeaShare shares[], int patch, int edge, int vert)
{
int *verts;
 
    switch (edge) {
    case 0:
        verts = shares[patch].left;
        break;
    case 1:
        verts = shares[patch].top;
        break;
    case 2:
        verts = shares[patch].right;
        break;
    case 3:
        verts = shares[patch].bottom;
        break;
    }
    return verts[vert];
}

static void
display_curve(int * patch_array, TeaShare shares[],
			  Teapoint d0, Teapoint d1, Teapoint d2, Teapoint d3, int steps,
			  Mesh& amesh, int *nvert, int patch, int row)
{
float    t,		/* t varies on 0.0 -> 1.0 */
         step;
int      i;
Teapoint temp;
TeaEdges *edge = &edges[patch];

    step = (float)1.0 / steps;
    t = step;

	if (row == 0) {
		switch (edge->first) {
		case GenAll:				// Generate all
		case GenSingularityBegin:	// in row 0 all these are the same
			/* the first point IS d0 so we don't need to calculate it */
			amesh.setVert(*nvert, d0.x, d0.y, d0.z);
			patch_array[row * (steps + 1)] = *nvert;
			(*nvert)++;

			/* no interior vertices are shared */
			for (i = 1; i < steps; i++) {
				blend_vector(d0, d1, d2, d3, t, &temp);
				amesh.setVert(*nvert, temp.x, temp.y, temp.z);
				patch_array[row * (steps + 1) + i] = *nvert;
				(*nvert)++;
				t += step;
			}

			/* the last vertex IS d3 so we don't need to calculate it */
			amesh.setVert(*nvert, d3.x, d3.y, d3.z);
			patch_array[row * (steps + 1) + steps] = *nvert;
			(*nvert)++;
			break;
		case ShareBegin:
			patch_array[row * (steps + 1)] = GetShare(shares, edge->patch0, edge->edge0, steps);

			/* no interior vertices are shared */
			for (i = 1; i < steps; i++) {
				blend_vector(d0, d1, d2, d3, t, &temp);
				amesh.setVert(*nvert, temp.x, temp.y, temp.z);
				patch_array[row * (steps + 1) + i] = *nvert;
				(*nvert)++;
				t += step;
			}

			/* the last vertex IS d3 so we don't need to calculate it */
			amesh.setVert(*nvert, d3.x, d3.y, d3.z);
			patch_array[row * (steps + 1) + steps] = *nvert;
			(*nvert)++;
			break;
		case ShareAll:  // This should never happen with the current data
			for (i = 0; i <= steps; i++)
				patch_array[row * (steps + 1) + i] = GetShare(shares, edge->patch0, edge->edge0, i);
			break;
		default:
			assert(0);
			break;
		}
	} else {
		EdgeType et;
		if (row < steps)
			et = edge->center;
		else
			et = edge->last;
		switch (et) {
		case GenAll:				// Generate all
			/* the first point IS d0 so we don't need to calculate it */
			amesh.setVert(*nvert, d0.x, d0.y, d0.z);
			patch_array[row * (steps + 1)] = *nvert;
			(*nvert)++;

			/* no interior vertices are shared */
			for (i = 1; i < steps; i++) {
				blend_vector(d0, d1, d2, d3, t, &temp);
				amesh.setVert(*nvert, temp.x, temp.y, temp.z);
				patch_array[row * (steps + 1) + i] = *nvert;
				(*nvert)++;
				t += step;
			}

			/* the last vertex IS d3 so we don't need to calculate it */
			amesh.setVert(*nvert, d3.x, d3.y, d3.z);
			patch_array[row * (steps + 1) + steps] = *nvert;
			(*nvert)++;
			break;

		case GenSingularityBegin:	// generate all vertices except the first which
									// is a singularity from the first row
			patch_array[row * (steps + 1)] = patch_array[0];

			/* no interior vertices are shared */
			for (i = 1; i < steps; i++) {
				blend_vector(d0, d1, d2, d3, t, &temp);
				amesh.setVert(*nvert, temp.x, temp.y, temp.z);
				patch_array[row * (steps + 1) + i] = *nvert;
				(*nvert)++;
				t += step;
			}

			/* the last vertex IS d3 so we don't need to calculate it */
			amesh.setVert(*nvert, d3.x, d3.y, d3.z);
			patch_array[row * (steps + 1) + steps] = *nvert;
			(*nvert)++;
			break;
		case ShareAll:			/* this should only happen as the last row */
			for (i = 0; i <= steps; i++)
				patch_array[row * (steps + 1) + i] = GetShare(shares, edge->patch2, edge->edge2, i);
			break;
		case ShareBegin:
			patch_array[row * (steps + 1)] = GetShare(shares, edge->patch3, edge->edge3, row);

			/* no interior vertices are shared */
			for (i = 1; i < steps; i++) {
				blend_vector(d0, d1, d2, d3, t, &temp);
				amesh.setVert(*nvert, temp.x, temp.y, temp.z);
				patch_array[row * (steps + 1) + i] = *nvert;
				(*nvert)++;
				t += step;
			}

			/* the last vertex IS d3 so we don't need to calculate it */
			amesh.setVert(*nvert, d3.x, d3.y, d3.z);
			patch_array[row * (steps + 1) + steps] = *nvert;
			(*nvert)++;
			break;
		case ShareBeginSingularityEnd:
			patch_array[row * (steps + 1)] = GetShare(shares, edge->patch3, edge->edge3, row);

			/* no interior vertices are shared */
			for (i = 1; i < steps; i++) {
				blend_vector(d0, d1, d2, d3, t, &temp);
				amesh.setVert(*nvert, temp.x, temp.y, temp.z);
				patch_array[row * (steps + 1) + i] = *nvert;
				(*nvert)++;
				t += step;
			}
	
			patch_array[row * (steps + 1) + steps] = patch_array[steps];
			break;
		default:
			assert(0);
			break;
		}
	}
}


static void
blend_vector(Teapoint d0,Teapoint d1,Teapoint d2,Teapoint d3, float t,
 Teapoint *result)
{
    result->x = d0.x * (1.0f - t)*(1.0f - t)*(1.0f - t) +
                d1.x * 3.0f * t * (1.0f - t)*(1.0f - t) +
                d2.x * 3.0f * t*t          *(1.0f - t) +
                d3.x *        t*t*t;
    result->y = d0.y * (1.0f - t)*(1.0f - t)*(1.0f - t) +
                d1.y * 3.0f * t * (1.0f - t)*(1.0f - t) +
                d2.y * 3.0f * t*t           *(1.0f - t) +
                d3.y *        t*t*t;
    result->z = d0.z * (1.0f - t)*(1.0f - t)*(1.0f - t) +
                d1.z * 3.0f * t * (1.0f - t)*(1.0f - t) +
                d2.z * 3.0f * t*t           *(1.0f - t) +
                d3.z *        t*t*t;
}


static void
display_patch(TeaShare shares[], int patch, int steps, Mesh& amesh, int *nvert, int *nface,
			int smooth, int genUVs, int *uv_vert, int *uv_face)
{
	float    t,step;
	Teapoint    d0,d1,d2,d3;
	int start_vert, *patch_array;

	patch_array = (int *)malloc(sizeof(int) * (steps + 1) * (steps + 1));

    step = 1.0f / steps;
    t = 0.0f;
	start_vert = *nvert;

	for (int i = 0; i <= steps; i++) {
        blend_vector(verts[patches[patch][0][0]],
                     verts[patches[patch][0][1]],
                     verts[patches[patch][0][2]],
                     verts[patches[patch][0][3]],t,&d0);    
        blend_vector(verts[patches[patch][1][0]],
                     verts[patches[patch][1][1]],
                     verts[patches[patch][1][2]],
                     verts[patches[patch][1][3]],t,&d1);    
        blend_vector(verts[patches[patch][2][0]],
                     verts[patches[patch][2][1]],
                     verts[patches[patch][2][2]],
                     verts[patches[patch][2][3]],t,&d2);    
        blend_vector(verts[patches[patch][3][0]],
                     verts[patches[patch][3][1]],
                     verts[patches[patch][3][2]],
                     verts[patches[patch][3][3]],t,&d3);    
        display_curve(patch_array, shares, d0,d1,d2,d3,steps, amesh, nvert, patch, i);

        t += step;
    }

	/* now that we have generated all the vertices we can save the edges */
	for (i = 0; i <= steps; i++) {
		shares[patch].left[i] = patch_array[i];					/* "left" edge */
        shares[patch].top[i] = patch_array[i*(steps+1)+steps];	/* "top" edge */
		shares[patch].right[i] = patch_array[steps*(steps+1)+i];	/* "right" edge */
        shares[patch].bottom[i] = patch_array[i*(steps+1)];		/* "bottom" edge */
	}

	/* now it's time to add the faces */
	for (int x = 0; x < steps; x++) {
		for (int y = 0; y < steps; y++) {
			int va, vb, vc, vd;

			va = patch_array[x * (steps + 1) + y];
			vb = patch_array[(x+1) * (steps + 1) + y];

			vd = patch_array[x * (steps + 1) + (y+1)];
			vc = patch_array[(x+1) * (steps + 1) + (y+1)];

#ifdef NO_DEGENERATE_POLYS
            if (va != vb && va != vc && vc != vb) {
#endif                
                amesh.faces[*nface].setEdgeVisFlags(1, 1, 0);
                amesh.faces[*nface].setVerts(va, vb, vc);
                amesh.faces[*nface].setSmGroup(smooth);
                (*nface)++;
#ifdef NO_DEGENERATE_POLYS
			}
#endif

#ifdef NO_DEGENERATE_POLYS
            if (vc != vd && vc != va && va != vd) {
#endif
                amesh.faces[*nface].setEdgeVisFlags(1, 1, 0);
                amesh.faces[*nface].setVerts(vc, vd, va);
                amesh.faces[*nface].setSmGroup(smooth);
                (*nface)++;
#ifdef NO_DEGENERATE_POLYS
            }
#endif
		}
	}


	// Now the UVs
	if (genUVs) {
		int base_vert = *uv_vert;
		float u, v,
				bU = edges[patch].bU, eU = edges[patch].eU,
				bV = edges[patch].bV, eV = edges[patch].eV,
				dU = (eU - bU)/float(steps),
				dV = (eV - bV)/float(steps);
		u = bU;
		for (int x = 0; x <= steps; x++) {
			v = bV;
			for (int y = 0; y <= steps; y++) {
				amesh.setTVert((*uv_vert)++, u, v, 0.0f);
				v += dV;
			}
			u += dU;
		}
		int na, nb;
		for(int ix=0; ix<steps; ix++) {
			na = base_vert + (ix * (steps + 1));
			nb = base_vert + (ix + 1) * (steps + 1);

			for (int jx=0; jx<steps; jx++) {
                int vva = patch_array[ix * (steps + 1) + jx];
                int vvb = patch_array[(ix+1) * (steps + 1) + jx];
                
                int vvd = patch_array[ix * (steps + 1) + (jx+1)];
                int vvc = patch_array[(ix+1) * (steps + 1) + (jx+1)];
                
#ifdef NO_DEGENERATE_POLYS
                if (vva != vvb && vva != vvc && vvc != vvb)
#endif
                    amesh.tvFace[(*uv_face)++].setTVerts(na,nb,nb+1);
#ifdef NO_DEGENERATE_POLYS
                if (vvc != vvd && vvc != vva && vva != vvd)
#endif
                    amesh.tvFace[(*uv_face)++].setTVerts(nb+1,na+1,na);
				na++;
				nb++;
			}
		}
	}

	free(patch_array);
}


BOOL TeapotObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void TeapotObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}



// Builds the mesh representation for the Teapot based on the
// state of it's parameters at the time requested.
void TeapotObject::BuildMesh(TimeValue t)
{

	int segs, smooth,
		body, handle, spout, lid, genUVs,
		cverts = 0, nvert = 0,
		cfaces = 0, nface = 0,
		uv_verts = 0, nuv_vert = 0,
		uv_faces = 0, nuv_face = 0,
		index, i, oldpart;
	float radius;

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	pblock->GetValue(PB_RADIUS,		t, radius, ivalid);
	pblock->GetValue(PB_SEGS,		t, segs, ivalid);
	pblock->GetValue(PB_SMOOTH,		t, smooth, ivalid);
	pblock->GetValue(PB_TEA_BODY,	t, body, ivalid);
	pblock->GetValue(PB_TEA_HANDLE,	t, handle, ivalid);
	pblock->GetValue(PB_TEA_SPOUT,	t, spout, ivalid);
	pblock->GetValue(PB_TEA_LID,	t, lid, ivalid);
	pblock->GetValue(PB_GENUVS,		t, genUVs, ivalid);
    pblock->GetValue(PB_TEAPART,    t, oldpart, ivalid);
    switch (oldpart) {
    case OLD_POT_BOTH:
        body = handle = spout = lid = 1;
        pblock->SetValue(PB_TEAPART,	t,  FIXED_OLD_VER);
        pblock->SetValue(PB_TEA_BODY,	t,  body);
        pblock->SetValue(PB_TEA_HANDLE,	t,  handle);
        pblock->SetValue(PB_TEA_SPOUT,	t,  spout);
        pblock->SetValue(PB_TEA_LID,	t,  lid);
        break;
    case OLD_POT_BODY:
        body = handle = spout = 1;
        lid = 0;
        pblock->SetValue(PB_TEAPART,	t,  FIXED_OLD_VER);
        pblock->SetValue(PB_TEA_BODY,	t,  body);
        pblock->SetValue(PB_TEA_HANDLE,	t,  handle);
        pblock->SetValue(PB_TEA_SPOUT,	t,  spout);
        pblock->SetValue(PB_TEA_LID,	t,  lid);
        break;
    case OLD_POT_LID:
        body = handle = spout = 0;
        lid = 1;
        pblock->SetValue(PB_TEAPART,	t,  FIXED_OLD_VER);
        pblock->SetValue(PB_TEA_BODY,	t,  body);
        pblock->SetValue(PB_TEA_HANDLE,	t,  handle);
        pblock->SetValue(PB_TEA_SPOUT,	t,  spout);
        pblock->SetValue(PB_TEA_LID,	t,  lid);
        break;
    case FIXED_OLD_VER:
        break;
    }

	LimitValue(segs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(smooth, MIN_SMOOTH, MAX_SMOOTH);
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);
	LimitValue(body, 0, 1);
	LimitValue(handle, 0, 1);
	LimitValue(spout, 0, 1);
	LimitValue(lid, 0, 1);
	LimitValue(genUVs, 0, 1);
	
//	int *patch_array = (int *)malloc(sizeof(int) * (segs + 1) * (segs + 1));

	TeaShare shares[PATCH_COUNT]; /* make an array to hold sharing data */
	for (index = 0; index < PATCH_COUNT; index++) {
        shares[index].left =   (int *)malloc(sizeof(int) * (segs + 1));
        shares[index].top =    (int *)malloc(sizeof(int) * (segs + 1));
        shares[index].right =  (int *)malloc(sizeof(int) * (segs + 1));
        shares[index].bottom = (int *)malloc(sizeof(int) * (segs + 1));
	}

	if (body) {
		cverts += (segs - 1) * (segs - 1) * 16 +
					segs * 16 + (segs - 1) * 16 + 1;
#ifdef NO_DEGENERATE_POLYS
		cfaces += segs * segs * BODY_PATCHES * 2 - 4 * segs;
#else
		cfaces += segs * segs * BODY_PATCHES * 2;
#endif
		if (genUVs) {
			uv_verts += (segs + 1) * (segs + 1) * BODY_PATCHES;
#ifdef NO_DEGENERATE_POLYS
			uv_faces += segs * segs * BODY_PATCHES * 2 - 4 * segs;
#else
			uv_faces += segs * segs * BODY_PATCHES * 2;
#endif
		}
	}
	if (handle) {
		cverts += (segs + 1) * segs * 2 + segs * segs * 2;
		cfaces += segs * segs * HANDLE_PATCHES * 2;
			if (genUVs) {
			uv_verts += (segs + 1) * (segs + 1) * HANDLE_PATCHES;
			uv_faces += segs * segs * HANDLE_PATCHES * 2;
		}
	}
	if (spout) {
		cverts += (segs + 1) * segs * 2 + segs * segs * 2;
		cfaces += segs * segs * SPOUT_PATCHES * 2;
			if (genUVs) {
			uv_verts += (segs + 1) * (segs + 1) * SPOUT_PATCHES;
			uv_faces += segs * segs * SPOUT_PATCHES * 2;
		}
	}
	if (lid) {
		cverts += (segs - 1) * (segs - 1) * 8 +
					segs * 8 + (segs - 1) * 8 + 1;
#ifdef NO_DEGENERATE_POLYS
		cfaces += segs * segs * LID_PATCHES * 2 - 4 * segs;
#else
		cfaces += segs * segs * LID_PATCHES * 2;
#endif
			if (genUVs) {
			uv_verts += (segs + 1) * (segs + 1) * LID_PATCHES;
#ifdef NO_DEGENERATE_POLYS
			uv_faces += segs * segs * LID_PATCHES * 2 - 4 * segs;
#else
			uv_faces += segs * segs * LID_PATCHES * 2;
#endif
		}
	}

	mesh.setNumVerts(cverts);
	mesh.setNumFaces(cfaces);
	mesh.setSmoothFlags(smooth != 0);
	mesh.setNumTVerts(uv_verts);
	mesh.setNumTVFaces(uv_faces);

  	if (body) {
		for (index = FIRST_BODY_PATCH, i = 0; i < BODY_PATCHES; i++, index++)
			display_patch(shares, index, segs, mesh, &nvert, &nface, smooth, genUVs, &nuv_vert, &nuv_face);
	}
	if (handle) {
		for (index = FIRST_HANDLE_PATCH, i = 0; i < HANDLE_PATCHES; i++, index++)
			display_patch(shares, index, segs, mesh, &nvert, &nface, smooth, genUVs, &nuv_vert, &nuv_face);
	}
	if (spout) {
		for (index = FIRST_SPOUT_PATCH, i = 0; i < SPOUT_PATCHES; i++, index++)
			display_patch(shares, index, segs, mesh, &nvert, &nface, smooth, genUVs, &nuv_vert, &nuv_face);
	}
	if (lid) {
		for (index = FIRST_LID_PATCH, i = 0; i < LID_PATCHES; i++, index++)
			display_patch(shares, index, segs, mesh, &nvert, &nface, smooth, genUVs, &nuv_vert, &nuv_face);
	}

	/* scale about 0,0,0 by radius / 2 (the teapot is generated 2x) */
	for (int c = 0; c < mesh.getNumVerts(); c++) {
#ifdef CENTER_ZERO
		mesh.verts[c].z -= 1.2;
#endif
		mesh.verts[c].x *= (radius / (float)2.0);
		mesh.verts[c].y *= (radius / (float)2.0);
		mesh.verts[c].z *= (radius / (float)2.0);
	}

	/* Clean up */
	for (index = 0; index < PATCH_COUNT; index++) {
		free(shares[index].left);
		free(shares[index].top);
		free(shares[index].right);
		free(shares[index].bottom);
	}

	mesh.InvalidateTopologyCache();
}

// Declare a class derived from CreateMouseCallBack to handle
// the user input during the creation phase of the Teapot.
class TeapotObjCreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;
	TeapotObject *ob;
	Point3 p0;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, 
			Matrix3& mat);
		void SetObj(TeapotObject *obj) {ob = obj;}
	};

// This is the method that actually handles the user input
// during the Teapot creation.
int TeapotObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, 
	int flags, IPoint2 m, Matrix3& mat ) {

	float r;
	Point3 p1,center;

	#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _3D_CREATE
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
			vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
	}
	#endif

		if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:  // only happens with MOUSE_POINT msg				
				sp0 = m;
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p0);
				break;
			case 1:
				mat.IdentityMatrix();
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				if (ob->dlgCreateMeth) {
					r = Length(p1-p0);
					mat.SetTrans(p0);
					}
				else {
					center = (p0+p1)/float(2);
					mat.SetTrans(center);
					r = Length(center-p0);
					} 
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p1.y-p0.y,p1.x-p0.x);					
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) {										
					return (Length(m-sp0)<3 || Length(p1-p0)<0.1f)?CREATE_ABORT:CREATE_STOP;
					}
				break;					   
			}
		}
	else
	if (msg == MOUSE_ABORT) {		
		return CREATE_ABORT;
		}

	return TRUE;
	}

// A single instance of the callback object.
static TeapotObjCreateCallBack teapotCreateCB;

// This method allows MAX to access and call our proc method to 
// handle the user input.
CreateMouseCallBack* TeapotObject::GetCreateMouseCallBack() 
	{
	teapotCreateCB.SetObj(this);
	return(&teapotCreateCB);
	}


// Return TRUE if it is OK to display the mesh at the time requested,
// return FALSE otherwise.
BOOL TeapotObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}

// From ParamArray
// These methods allow the parameter map to access our class variables.
// Based on the virtual array index passed in we set or get the 
// variable requested.
BOOL TeapotObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL TeapotObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		}	
	return TRUE;
	}

BOOL TeapotObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL TeapotObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL TeapotObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v = crtRadius; break;
		}
	return TRUE;
	}

BOOL TeapotObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


// From GeomObject
int TeapotObject::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
	{
	int smooth;
	pblock->GetValue(PB_SMOOTH,t,smooth,FOREVER);
	if (!smooth) {
		return SimpleObject::IntersectRay(t,ray,at,norm);
		}	
	
	BuildMesh(t);

	return mesh.IntersectRay(ray, at, norm);
	}

// This method is called when the user interface controls need to be
// updated to reflect new values because of the user moving the time
// slider.  Here we simply call a method of the parameter map to 
// handle this for us.
void TeapotObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

// This method returns the dimension of the parameter requested.
// This dimension describes the type and magnitude of the value
// stored by the parameter.
ParamDimension *TeapotObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return stdWorldDim;			
		case PB_SEGS:
			return stdSegmentsDim;			
		case PB_SMOOTH:
		case PB_TEA_BODY:
		case PB_TEA_HANDLE:
		case PB_TEA_SPOUT:
		case PB_TEA_LID:
		case PB_GENUVS:
			return stdNormalizedDim;			
		default:
			return defaultDim;
		}
	}

// This method returns the name of the parameter requested.
TSTR TeapotObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return GetString(IDS_RB_RADIUS);
		case PB_SEGS:
			return GetString(IDS_RB_SEGMENTS);
		case PB_SMOOTH:
			return GetString(IDS_RB_SMOOTH);
		case PB_TEA_BODY:
			return GetString(IDS_RB_BODY);
		case PB_TEA_HANDLE:
			return GetString(IDS_RB_HANDLE);
		case PB_TEA_SPOUT:
			return GetString(IDS_RB_SPOUT);
		case PB_TEA_LID:
			return GetString(IDS_RB_LID);
		case PB_GENUVS:
			return GetString(IDS_RB_GENTEXCOORDS);
		default:
			return TSTR(_T(""));
		}
	}

// Make a copy of the Teapot object parameter block.
RefTargetHandle TeapotObject::Clone(RemapDir& remap) 
	{
	TeapotObject* newob = new TeapotObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}


static int
isCorner(int x, int y)
{
    if ((x == 0 && y == 0) ||
        (x == 3 && y == 3) ||
        (x == 0 && y == 3) ||
        (x == 3 && y == 0))
        return TRUE;
    return FALSE;
}

static void
setVV(PatchMesh &patch, int patch_array[4][4], int p, int x, int y, float radius,
      int *ix, int *jx)
{
 	Point3 pnt;
 		
    pnt.x = verts[patches[p][x][y]].x * radius;
    pnt.y = verts[patches[p][x][y]].y * radius;
    pnt.z = verts[patches[p][x][y]].z * radius;
    if (isCorner(x, y)) {
        patch_array[x][y] = *ix; // save it
        patch.setVert((*ix)++, pnt);
    } else {
        patch_array[x][y] = *jx; // save it
        patch.setVec((*jx)++, pnt);
    }
}

static void
BuildTeapotPart(int base_patch, int num_patchs, PatchMesh &patch, int *in_ix, int *in_jx,
        int *in_patch, float radius, TeaShare shares[], int genUVs)
{
    int ix = *in_ix,
        jx = *in_jx,
        bpatch = *in_patch;

    for (int i = 0; i < num_patchs; i++) {
        int x, y;
        int p = base_patch + i;
	 	int bvert = ix, bvec = jx;
    	int patch_array[4][4];
		if (genUVs) {
			int tv = (bpatch+i) * 4;
			patch.setTVert(tv+0, UVVert(edges[p].eU, edges[p].bV, 0.0f));
			patch.setTVert(tv+1, UVVert(edges[p].eU, edges[p].eV, 0.0f));
			patch.setTVert(tv+2, UVVert(edges[p].bU, edges[p].eV, 0.0f));
			patch.setTVert(tv+3, UVVert(edges[p].bU, edges[p].bV, 0.0f));
			patch.getTVPatch(bpatch+i).setTVerts(tv+3,tv+0,tv+1,tv+2);
		}

        for (x = 0; x < 4; x++) {
            for (y = 0; y < 4; y++) {
                switch((y == 0 ? edges[p].first : (y == 3 ? edges[p].last : edges[p].center))) {
                case GenAll:
                    setVV(patch, patch_array, p, x, y, radius, &ix, &jx);
                    break;
                case GenSingularityBegin:
                    if (x == 0 && y >= 1) { // the first point is common (must gen. 1st tangent)
                        if (y == 3) { // corner
							patch_array[x][y] = patch_array[0][0];
						} else { // tangent
							if (y == 1) {
#if 0
								if (edges[p].first == ShareAll)
									patch_array[x][y] = GetShare(shares, p-1, 3, y);
								else
#endif
									setVV(patch, patch_array, p, x, y, radius, &ix, &jx);
							} else
								patch_array[x][y] = patch_array[0][1];
						}
                    } else {
                        setVV(patch, patch_array, p, x, y, radius, &ix, &jx);
					}
                    break;
                case ShareBeginSingularityEnd:
                    if (x == 0) {
                        patch_array[x][y] = GetShare(shares, edges[p].patch3, edges[p].edge3, y);
                    } else if (x == 3 && y > 1) {
                        if (y == 3)
							patch_array[x][y] = patch_array[3][0]; // corner
						else
							patch_array[x][y] = patch_array[3][1]; // tangent
                    } else {
                        setVV(patch, patch_array, p, x, y, radius, &ix, &jx);
					}
                    break;
                case ShareBegin:
                    if (x == 0) {
                        patch_array[x][y] = GetShare(shares, edges[p].patch3, edges[p].edge3, y);
                    } else {
                        setVV(patch, patch_array, p, x, y, radius, &ix, &jx);
                    }
                    break;
                case ShareAll: // this seems to work look CAREFULLY before changing
                    if (y == 0)
                        patch_array[x][y] = GetShare(shares, edges[p].patch0, edges[p].edge0, x);
                    else
                        patch_array[x][y] = GetShare(shares, edges[p].patch2, edges[p].edge2, x);
                    break;
                default:
                    assert(0); // we should never get here
                }
            }
        }

		/* now that we have generated all the vertices we can save the edges */
		for (int j = 0; j <  4; j++) {
			shares[p].left[j] =   patch_array[j][0];
			shares[p].top[j] =    patch_array[3][j];
			shares[p].right[j] =  patch_array[j][3];
			shares[p].bottom[j] = patch_array[0][j];
		}

		// Build the patches
		patch.patches[bpatch + i].SetType(PATCH_QUAD);
		patch.patches[bpatch + i].setVerts(patch_array[0][0],
                                           patch_array[0][3],
                                           patch_array[3][3],
                                           patch_array[3][0]);
		patch.patches[bpatch + i].setVecs(patch_array[0][1], patch_array[0][2],
                                          patch_array[1][3], patch_array[2][3],
                                          patch_array[3][2], patch_array[3][1],
                                          patch_array[2][0], patch_array[1][0]);
		patch.patches[bpatch + i].setInteriors(patch_array[1][1],
                                               patch_array[1][2],
                                               patch_array[2][2],
                                               patch_array[2][1]);
		patch.patches[bpatch + i].smGroup = 1;
		patch.patches[bpatch + i].SetAuto(FALSE);
    }

    *in_ix = ix;
    *in_jx = jx;
    *in_patch += num_patchs;
}
 
void BuildTeapotPatch(PatchMesh &patch, float radius,
		            int body, int handle, int spout, int lid, int genUVs)
{
	int nverts = 0;
	int nvecs = 0;
	int npatches = 0;
	int ntvpatches = 0;
	int ntvverts = 0;
    int bpatch = 0;
	int ix = 0;
    int jx = 0;

	if (body) {
        nverts += 17;
        nvecs += 132;
		ntvverts += 64;
        npatches += BODY_PATCHES;
		ntvpatches += BODY_PATCHES;
	}
	if (handle) {
        nverts += 6;
        nvecs += 36;
		ntvverts += 16;
        npatches += HANDLE_PATCHES;
		ntvpatches += HANDLE_PATCHES;
	}
	if (spout) {
        nverts += 6;
        nvecs += 36;
		ntvverts += 16;
        npatches += SPOUT_PATCHES;
		ntvpatches += SPOUT_PATCHES;
	}
	if (lid) {
        nverts += 9;
        nvecs += 68;
		ntvverts += 32;
        npatches += LID_PATCHES;
		ntvpatches += LID_PATCHES;
	}

	patch.setNumVerts(nverts);
	patch.setNumVecs(nvecs);
	patch.setNumPatches(npatches);
	patch.setNumTVerts(genUVs ? ntvverts : 0);
	patch.setNumTVPatches(genUVs ? ntvpatches : 0);

	TeaShare shares[PATCH_COUNT]; /* make an array to hold sharing data */
    for (int c = 0; c < PATCH_COUNT; c++) {
        shares[c].left =   (int *)malloc(4 * sizeof(int));
        shares[c].top =    (int *)malloc(4 * sizeof(int));
        shares[c].right =  (int *)malloc(4 * sizeof(int));
        shares[c].bottom = (int *)malloc(4 * sizeof(int));
    }

	if (body)
        BuildTeapotPart(FIRST_BODY_PATCH, BODY_PATCHES, patch, &ix, &jx,
			&bpatch, radius/2.0f, shares, genUVs);
 	if (handle)
        BuildTeapotPart(FIRST_HANDLE_PATCH, HANDLE_PATCHES, patch, &ix, &jx,
			&bpatch, radius/2.0f, shares, genUVs);
	if (spout)
        BuildTeapotPart(FIRST_SPOUT_PATCH, SPOUT_PATCHES, patch, &ix, &jx,
			&bpatch, radius/2.0f, shares, genUVs);
	if (lid)
        BuildTeapotPart(FIRST_LID_PATCH, LID_PATCHES, patch, &ix, &jx,
			&bpatch, radius/2.0f, shares, genUVs);

	for (c = 0; c < PATCH_COUNT; c++) {
		free(shares[c].left);
		free(shares[c].top);
		free(shares[c].right);
		free(shares[c].bottom);
	}

    assert(patch.buildLinkages());
	patch.InvalidateGeomCache();
}


Point3 Tbody[] = {	Point3(0.0,    0.0, 0.0),
					Point3(0.4000, 0.0, 0.0),
					Point3(0.6600, 0.0, 0.0155),
					Point3(0.7620, 0.0, 0.0544),
					Point3(0.7445, 0.0, 0.0832),
					Point3(0.8067, 0.0, 0.1225),
					Point3(1.0186, 0.0, 0.3000),
					Point3(1.0153, 0.0, 0.6665),
					Point3(0.7986, 0.0, 1.120),
					Point3(0.7564, 0.0, 1.210),
					Point3(0.7253, 0.0, 1.265),
					Point3(0.6788, 0.0, 1.264),
					Point3(0.7056, 0.0, 1.200)};

Point3 Tlid[] = {	Point3(0.0,    0.0, 1.575),
					Point3(0.1882, 0.0, 1.575),
					Point3(0.1907, 0.0, 1.506),
					Point3(0.1443, 0.0, 1.473),
					Point3(0.0756, 0.0, 1.390),
					Point3(0.0723, 0.0, 1.336),
					Point3(0.3287, 0.0, 1.283),
					Point3(0.6531, 0.0, 1.243),
					Point3(0.6500, 0.0, 1.200)};

#ifndef NO_NURBS

void
BuildNURBSBody(NURBSSet& nset, float radius, int genUVs)
{
	Point3 scale(radius, radius, radius);

	double knots[] = {	0.0, 0.0, 0.0, 0.0,
						0.1, 0.2, 0.3, 0.4,
						0.5, 0.6, 0.7, 0.8, 0.9,
						1.0, 1.0, 1.0, 1.0};

	NURBSCVCurve *c = new NURBSCVCurve();
	c->SetOrder(4);
	c->SetNumKnots(17);
	c->SetNumCVs(13);
	for (int k = 0; k < 17; k++)
		c->SetKnot(k, knots[k]);
	for (int i = 0; i < 13; i++) {
		NURBSControlVertex ncv;
		ncv.SetPosition(0, Tbody[i] * ScaleMatrix(scale));
		ncv.SetWeight(0, 1.0f);
		c->SetCV(i, ncv);
	}
	c->SetNSet(&nset);
	int p1 = nset.AppendObject(c);

	NURBSLatheSurface *s = new NURBSLatheSurface();

	s->SetParent(p1);
	Matrix3 mat = TransMatrix(Point3(0, 0, 0));
	s->SetAxis(0, mat);
    s->SetName(GetString(IDS_RB_TEAPOT));
    s->FlipNormals(FALSE);
	s->SetGenerateUVs(genUVs);
	s->Renderable(TRUE);
	s->SetTextureUVs(0, 0, Point2(0.0, 0.0));
	s->SetTextureUVs(0, 1, Point2(0.0, 2.0));
	s->SetTextureUVs(0, 2, Point2(4.0, 0.0));
	s->SetTextureUVs(0, 3, Point2(4.0, 2.0));
	s->SetNSet(&nset);
	nset.AppendObject(s);
}


void
BuildNURBSLid(NURBSSet& nset, float radius, int genUVs)
{
	Point3 scale(radius, radius, radius);
	double knots[] = {	0.0, 0.0, 0.0, 0.0,
						1.0/6.0, 1.0/3.0, 0.5,
						2.0/3.0, 5.0/6.0,
						1.0, 1.0, 1.0, 1.0};
	NURBSCVCurve *c = new NURBSCVCurve();
	c->SetOrder(4);
	c->SetNumKnots(13);
	c->SetNumCVs(9);
	for (int k = 0; k < 13; k++)
		c->SetKnot(k, knots[k]);
	for (int i = 0; i < 9; i++) {
		NURBSControlVertex ncv;
		ncv.SetPosition(0, Tlid[i] * ScaleMatrix(scale));
		ncv.SetWeight(0, 1.0f);
		c->SetCV(i, ncv);
	}
	c->SetNSet(&nset);
	int p1 = nset.AppendObject(c);

	NURBSLatheSurface *s = new NURBSLatheSurface();
	Matrix3 mat = TransMatrix(Point3(0, 0, 0));
	s->SetAxis(0, mat);
	s->SetParent(p1);
    s->SetName(GetString(IDS_RB_TEAPOT));
    s->FlipNormals(TRUE);
	s->SetGenerateUVs(genUVs);
	s->Renderable(TRUE);
	s->SetTextureUVs(0, 0, Point2(0.0, 2.0));
	s->SetTextureUVs(0, 1, Point2(0.0, 0.0));
	s->SetTextureUVs(0, 2, Point2(2.0, 2.0));
	s->SetTextureUVs(0, 3, Point2(2.0, 0.0));
	nset.AppendObject(s);
}

#define F(s1, s2, s1r, s1c, s2r, s2c) \
	fuse.mSurf1 = (s1); \
	fuse.mSurf2 = (s2); \
	fuse.mRow1 = (s1r); \
	fuse.mCol1 = (s1c); \
	fuse.mRow2 = (s2r); \
	fuse.mCol2 = (s2c); \
	nset.mSurfFuse.Append(1, &fuse);

void
BuildNURBSPart(NURBSSet& nset, int firstPatch, int numPatches, int *bpatch, float radius, int genUVs)
{
	Point3 scale(radius, radius, radius);

	for (int face = 0; face < numPatches; face++) {
		NURBSCVSurface *s = new NURBSCVSurface();
		nset.AppendObject(s);
		int bp = *bpatch + face;

		s->SetUOrder(4);
		s->SetVOrder(4);
		s->SetNumUKnots(8);
		s->SetNumVKnots(8);
		for (int i = 0; i < 4; i++) {
			s->SetUKnot(i, 0.0);
			s->SetUKnot(i+4, 1.0);
			s->SetVKnot(i, 0.0);
			s->SetVKnot(i+4, 1.0);
		}
		s->SetNumCVs(4, 4);
		s->FlipNormals(TRUE);

		int pnum = firstPatch + face;
		for (int r = 0; r < 4; r++) {
			for (int c = 0; c < 4; c++) {
				Teapoint *tp = &verts[patches[pnum][r][c]];
				NURBSControlVertex ncv;
				ncv.SetPosition(0, Point3(tp->x, tp->y, tp->z) * ScaleMatrix(scale));
				ncv.SetWeight(0, 1.0f);
				s->SetCV(r, c, ncv);
			}
		}

		s->SetGenerateUVs(genUVs);

		s->SetTextureUVs(0, 0, Point2(edges[pnum].bU, edges[pnum].bV));
		s->SetTextureUVs(0, 1, Point2(edges[pnum].bU, edges[pnum].eV));
		s->SetTextureUVs(0, 2, Point2(edges[pnum].eU, edges[pnum].bV));
		s->SetTextureUVs(0, 3, Point2(edges[pnum].eU, edges[pnum].eV));
	}

	// Now for fusing
	NURBSFuseSurfaceCV fuse;
	int s = *bpatch;
	// fuse the seams
	for (int u = 0; u < ((NURBSCVSurface*)nset.GetNURBSObject(s))->GetNumUCVs(); u++) {
		F(s,   s+1, u, 0, u, ((NURBSCVSurface*)nset.GetNURBSObject(s+1))->GetNumVCVs()-1);
		F(s+1, s,   u, 0, u, ((NURBSCVSurface*)nset.GetNURBSObject(s  ))->GetNumVCVs()-1);
		F(s+2, s+3, u, 0, u, ((NURBSCVSurface*)nset.GetNURBSObject(s+3))->GetNumVCVs()-1);
		F(s+3, s+2, u, 0, u, ((NURBSCVSurface*)nset.GetNURBSObject(s+2))->GetNumVCVs()-1);
	}

	// fuse the middles
	for (int v = 1; v < ((NURBSCVSurface*)nset.GetNURBSObject(s))->GetNumVCVs(); v++) {
		F(s,   s+2, ((NURBSCVSurface*)nset.GetNURBSObject(s  ))->GetNumUCVs()-1, v, 0, v);
		F(s+1, s+3, ((NURBSCVSurface*)nset.GetNURBSObject(s+1))->GetNumUCVs()-1, v, 0, v);
	}


	*bpatch += numPatches;
}

Object* BuildNURBSTeapot(float radius, int body, int handle, int spout, int lid, int genUVs)
{
	int numpat = 0;

	if (body)
		numpat += 1;
	if (handle)
		numpat += HANDLE_PATCHES;
	if (spout)
		numpat += SPOUT_PATCHES;
	if (lid)
		numpat += 1;

	NURBSSet nset;
	
	int bpatch = 0;

 	if (handle)
        BuildNURBSPart(nset, FIRST_HANDLE_PATCH, HANDLE_PATCHES, &bpatch, radius/2.0f, genUVs);
	if (spout)
        BuildNURBSPart(nset, FIRST_SPOUT_PATCH, SPOUT_PATCHES, &bpatch, radius/2.0f, genUVs);
	if (body)
        BuildNURBSBody(nset, radius, genUVs);
	if (lid)
        BuildNURBSLid(nset, radius, genUVs);

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
}

#endif

Object* TeapotObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
#ifndef NO_PATCHES
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float radius;
		int body, handle, spout, lid, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_TEA_BODY,t,body,valid);	
		pblock->GetValue(PB_TEA_HANDLE,t,handle,valid);	
		pblock->GetValue(PB_TEA_SPOUT,t,spout,valid);	
		pblock->GetValue(PB_TEA_LID,t,lid,valid);	
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);	
		PatchObject *ob = new PatchObject();
		BuildTeapotPatch(ob->patch,radius, body, handle, spout, lid, genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->SetChannelValidity(TEXMAP_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	} 
#endif
#ifndef NO_NURBS
    if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float radius;
		int body, handle, spout, lid, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_TEA_BODY,t,body,valid);	
		pblock->GetValue(PB_TEA_HANDLE,t,handle,valid);	
		pblock->GetValue(PB_TEA_SPOUT,t,spout,valid);	
		pblock->GetValue(PB_TEA_LID,t,lid,valid);	
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);	
		Object *ob = BuildNURBSTeapot(radius, body, handle, spout, lid, genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->SetChannelValidity(TEXMAP_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	}
#endif
    return SimpleObject::ConvertToType(t,obtype);
	}

int TeapotObject::CanConvertToType(Class_ID obtype)
	{
#ifndef NO_PATCHES
	if(obtype == patchObjectClassID) return 1;
#endif
#ifndef NO_NURBS
	if(obtype == EDITABLE_SURF_CLASS_ID) return 1;
#endif
	return SimpleObject::CanConvertToType(obtype);
	}

void TeapotObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
#ifndef NO_NURBS
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}

// See the Advanced Topics section on DLL Functions and Class Descriptors
// for more information.
/*===========================================================================*\
 | The Class Descriptor
\*===========================================================================*/

class TeapotClassDesc:public ClassDesc {
	public:
	// The IsPublic() method should return TRUE if the plug-in can be picked
	// and assigned by the user. Some plug-ins may be used privately by other
	// plug-ins implemented in the same DLL and should not appear in lists for
	// user to choose from, so these plug-ins would return FALSE.
	int 			IsPublic() { return 1; }
	// This is the method that actually creates a new instance of
	// a plug-in class.  By having the system call this method,
	// the plug-in may use any memory manager it wishes to 
	// allocate its objects.  The system calls the correspoding 
	// DeleteThis() method of the plug-in to free the memory.  Our 
	// implementations use 'new' and 'delete'.
	void *			Create(BOOL loading = FALSE) { return new TeapotObject; }
	// This is used for debugging purposes to give the class a 
	// displayable name.  It is also the name that appears on the button
	// in the MAX user interface.
	const TCHAR *	ClassName() { return GetString(IDS_RB_TEAPOT_CLASS); }
	// The system calls this method at startup to determine the type of object
	// this is.  In our case, we're a geometric object so we return 
	// GEOMOBJECT_CLASS_ID.  The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	// The system calls this method to retrieve the unique
	// class id for this object.
	Class_ID		ClassID() { 
		return Class_ID(TEAPOT_CLASS_ID1,TEAPOT_CLASS_ID2); }
	// The category is selected
	// in the bottom most drop down list in the create branch.
	// If this is set to be an exiting category (i.e. "Primatives", ...) then
	// the plug-in will appear in that category. If the category doesn't
	// yet exists then it is created.  We use the new How To category for
	// all the example plug-ins in the How To sections.
	const TCHAR* 	Category() { return GetString(IDS_RB_PRIMITIVES); }
	
	void ResetClassParams(BOOL fileReset);
	};

// Declare a static instance of the class descriptor.
static TeapotClassDesc teapotDesc;
// This function returns the address of the descriptor.  We call it from 
// the LibClassDesc() function, which is called by the system when loading
// the DLLs at startup.
ClassDesc* GetTeapotDesc() { return &teapotDesc; }

// Initialize the class vars
int TeapotObject::dlgSegments       = DEF_SEGMENTS;
int TeapotObject::dlgCreateMeth     = 1; // create_radius
int TeapotObject::dlgSmooth         = SMOOTH_ON;
IParamMap *TeapotObject::pmapCreate = NULL;
IParamMap *TeapotObject::pmapParam  = NULL;
IParamMap *TeapotObject::pmapTypeIn = NULL;
IObjParam *TeapotObject::ip         = NULL;
Point3 TeapotObject::crtPos         = Point3(0,0,0);
float TeapotObject::crtRadius       = 0.0f;
int TeapotObject::crtTeapart        = FIXED_OLD_VER;
int TeapotObject::dlgBody			= POT_BODY;
int TeapotObject::dlgHandle			= POT_HANDLE;
int TeapotObject::dlgSpout			= POT_SPOUT;
int TeapotObject::dlgLid			= POT_LID;
int TeapotObject::dlgUVs			= POT_UVS;

void TeapotClassDesc::ResetClassParams(BOOL fileReset)
	{
	TeapotObject::dlgSegments       = DEF_SEGMENTS;
	TeapotObject::dlgCreateMeth     = 1; // create_radius
	TeapotObject::dlgSmooth         = SMOOTH_ON;
	TeapotObject::crtPos            = Point3(0,0,0);
	TeapotObject::crtRadius         = 0.0f;
	TeapotObject::crtTeapart        = FIXED_OLD_VER;
	TeapotObject::dlgBody			= POT_BODY;
	TeapotObject::dlgHandle			= POT_HANDLE;
	TeapotObject::dlgSpout			= POT_SPOUT;
	TeapotObject::dlgLid			= POT_LID;
	TeapotObject::dlgUVs			= POT_UVS;
	}

#endif // NO_OBJECT_TEAPOT