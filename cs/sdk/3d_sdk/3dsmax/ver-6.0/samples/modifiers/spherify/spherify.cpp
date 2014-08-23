/*===========================================================================*\
 |    File: spherify.cpp
 |
 | Purpose: Project the verticies of an object onto the surface of a sphere.
 |			Note this modifier does not support sub-object selection.
 |
 | History: Mark Meier, Begun 09/25/95, Last change 09/26/95.
 |          MM, 12/19/96, Brought it into the modern era...
\*===========================================================================*/
#include "max.h"			// Main MAX include file
#include "iparamm.h"		// Parameter Map include file
#include "spherify.h"		// Resource editor include file

/*===========================================================================*\
 | Misc Defines
\*===========================================================================*/
// The unique ClassID
#define SPHERIFY_CLASS_ID	Class_ID(0xDE17A34E, 0x8A41E2A0)

// This is the name that will appear in the Modifier stack
#define INIT_MOD_NAME			GetString(IDS_SPHERIFY)

// Name used for debugging
#define CLASSNAME				GetString(IDS_SPHERIFYMOD)

// This is the name on the creation button
#define SPHERIFY_CLASSNAME		GetString(IDS_SPHERIFY)

// This is the category the button goes into
#define CATEGORY_NAME			GetString(IDS_MAXADDITIONAL)

// The name of the parameter block sub-anim
#define PARAMETERS_NAME			GetString(IDS_PARAMETERS)

// The text string for track view for the percentage parameter
#define PERCENT_PARAM_NAME		GetString(IDS_PERCENT)

// Parameter block indicies
#define PB_PERCENT 0

// The DLL instance handle
HINSTANCE hInstance;

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

/*===========================================================================*\
 | Class Definitions:
\*===========================================================================*/
class SpherifyMod : public Modifier {
  public:
	SpherifyMod();

	// Class vars
	static Interface *ip;
	static ICustEdit *percentEdit;
	static ISpinnerControl *percentSpin;	
	static IParamMap *pmap;

	// The parameter block for the percentage parameter
	IParamBlock *pblock;

	// ---- Methods from Animatable ----
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) { s= CLASSNAME; }  
	virtual Class_ID ClassID() { return SPHERIFY_CLASS_ID;}
	SClass_ID SuperClassID() { return OSM_CLASS_ID; }
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags, Animatable *next);
	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i) { return pblock; }
	TSTR SubAnimName(int i) { return PARAMETERS_NAME;}		

	// ---- Methods from ReferenceMaker ----
	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget,
	   PartID& partID, RefMessage message );
	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return pblock; }
	void SetReference(int i, RefTargetHandle rtarg) {
		pblock = (IParamBlock*)rtarg;
	}

	// ---- Methods from ReferenceTarget ----
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// ---- Inherited virtual methods from BaseObject ----
	// This is the name that appears in the history list (modifier stack).
	TCHAR *GetObjectName() { return INIT_MOD_NAME; }
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 
 	BOOL ChangeTopology() {return FALSE;}

	// ---- Methods from Modifier ----
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO;}
	ChannelMask ChannelsChanged() {return PART_GEOM;}
	Class_ID InputType() { return defObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, 
		ObjectState *os, INode *node);

	// ---- Local methods ----
	float GetPercent(TimeValue t);
	Deformer& GetDeformer(TimeValue t, ModContext &mc, Matrix3 mat, 
		Matrix3 invmat);
	Interval SpherifyValidity(TimeValue t);
};

// This class is our deformer.  It provides some variables to store 
// data associated with the deformation, and a single method, Map, 
// which is called to deform each point.
class SpherifyDeformer : public Deformer {
  public:
	float cx, cy, cz, xsize, ysize, zsize, size, percent;
	Matrix3 tm, invtm;
	Box3 bbox;

	SpherifyDeformer();
	SpherifyDeformer(TimeValue t, ModContext &mc, Matrix3 mat,
		Matrix3 invmat, float per);

	// This is the method called by the Deform method one point at a time.
	Point3 Map(int i, Point3 p);
};

/*===========================================================================*\
 | Parameter Map and Parameter Block User Interface Stuff
\*===========================================================================*/
static ParamUIDesc descParam[] = {
	ParamUIDesc(PB_PERCENT, EDITTYPE_FLOAT, IDC_PERCENT, IDC_PERCENT_SPIN,
		0.0f, 100.0f, 1.0f),
};
#define DESC_LENGTH 1

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 }
};
#define PBLOCK_LENGTH		1
#define CURRENT_VERSION		0

/*===========================================================================*\
 | User Interface stuff...
\*===========================================================================*/
void SpherifyMod::BeginEditParams(IObjParam *ip, 
	ULONG flags, Animatable *prev) {
	this->ip = ip;
	// Create and add the Alter rollup to the command panel...
	pmap = CreateCPParamMap(descParam, DESC_LENGTH,
		pblock, ip, hInstance, MAKEINTRESOURCE(IDD_SPHERIFY), 
		GetString(IDS_PARAMETERS), 0);
}

void SpherifyMod::EndEditParams( IObjParam *ip, ULONG flags, 
	Animatable *next) {
	// Delete the parameter map
	if (pmap) { DestroyCPParamMap(pmap); }
	this->ip = NULL;
	this->pmap = NULL;
}

SpherifyMod::SpherifyMod() {
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));	
	pblock->SetValue(PB_PERCENT, 0, 100.0f);
}

// Initialize the class variables.
Interface *SpherifyMod::ip = NULL;
ICustEdit *SpherifyMod::percentEdit;
ISpinnerControl *SpherifyMod::percentSpin = NULL;
IParamMap *SpherifyMod::pmap = NULL;

/*===========================================================================*\
 | Modification stuff...
\*===========================================================================*/
float sign(float x) { return (x < 0.0f ? -1.0f : 1.0f); }

// This method, called by the system, modifies the items...
void SpherifyMod::ModifyObject(TimeValue t, ModContext &mc, 
	ObjectState *os, INode *node) {	

	Matrix3 mat, invmat;

	// Get the deformation space tm from the ModContext.  We use this to
	// transform our points in and out of deformation space.
	if (mc.tm)
		mat = *mc.tm;		
	else
		mat.IdentityMatrix();	
	
	// Compute the tm's inverse
	invmat = Inverse(mat);
 
	// Call the Deform method of the object passing it our deformer.
	// The Deform method of the object calls the Map method of the 
	// deformer we pass one point at a time.
	os->obj->Deform(&GetDeformer(t, mc, mat, invmat), TRUE);

	// This informs the system that the object may need to be re-evaluated
	// if the user moves to a new time.  We pass the channel we have 
	// modified, and the interval of our modification.
	os->obj->UpdateValidity(GEOM_CHAN_NUM, SpherifyValidity(t));
}

Interval SpherifyMod::SpherifyValidity(TimeValue t) {
	float p;
	Interval valid = FOREVER;
	pblock->GetValue(PB_PERCENT, t, p, valid);
	return valid;
}

RefTargetHandle SpherifyMod::Clone(RemapDir& remap) {
	SpherifyMod* newmod = new SpherifyMod();	
	newmod->ReplaceReference(0, pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}

RefResult SpherifyMod::NotifyRefChanged(Interval changeInt,
	RefTargetHandle hTarget, PartID& partID, RefMessage message) {

	switch (message) {
		case REFMSG_CHANGE:
			if (pmap && pmap->GetParamBlock()==pblock) {
				pmap->Invalidate();
				}
			break;

		case REFMSG_GET_PARAM_DIM: { 
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_PERCENT:
					gpd->dim = defaultDim;
					break;
			};
			return REF_STOP; 
		}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_PERCENT:
					gpn->name = PERCENT_PARAM_NAME;
					break;
			};
			return REF_STOP; 
		}
	}
	return REF_SUCCEED;
}

float SpherifyMod::GetPercent(TimeValue t) {
	float f;
	Interval valid;
	pblock->GetValue(PB_PERCENT, t, f, valid);
	return f;
}


SpherifyDeformer::SpherifyDeformer() {
	tm.IdentityMatrix();
	invtm = Inverse(tm);
};

// This method returns our deformer object
Deformer& SpherifyMod::GetDeformer(TimeValue t, ModContext &mc, 
	Matrix3 mat, Matrix3 invmat) {
	static SpherifyDeformer deformer;
	deformer = SpherifyDeformer(t, mc, mat, invmat, GetPercent(t));
	return deformer;
};

// This constructor stores some data we get from the 
// ModContext which we use during the deformation.
SpherifyDeformer::SpherifyDeformer(TimeValue t, ModContext &mc, 
	Matrix3 mat, Matrix3 invmat, float per) {

	// Save the tm and inverse tm
	tm = mat; invtm = invmat;

	// Save the bounding box
	assert(mc.box);
	bbox = *mc.box;

	// Compute the size and center
	xsize = bbox.pmax.x - bbox.pmin.x;
	ysize = bbox.pmax.y - bbox.pmin.y;
	zsize = bbox.pmax.z - bbox.pmin.z;
	size=(xsize>ysize) ? xsize:ysize;
	size=(zsize>size) ? zsize:size;
	size /= 2.0f;
	cx = bbox.Center().x;
	cy = bbox.Center().y;
	cz = bbox.Center().z;

	// Get the percentage to spherify at this time
	percent = per/100.0f;
};

// This is the method which deforms a single point.
Point3 SpherifyDeformer::Map(int i, Point3 p) {
	float x, y, z;
	float xw,yw,zw,vdist,mfac;
	float dx, dy, dz;
	
	// Multiply by the ModContext tm
	p = p*tm;

	// Spherify the point
	x = p.x; y = p.y; z = p.z;
	xw= x-cx; yw= y-cy; zw= z-cz;
	if(xw==0.0 && yw==0.0 && zw==0.0)
		xw=yw=zw=1.0f;
	vdist=(float) sqrt(xw*xw+yw*yw+zw*zw);
	mfac=size/vdist;
	dx = xw+sign(xw)*((float) (fabs(xw*mfac)-fabs(xw))*percent);
	dy = yw+sign(yw)*((float) (fabs(yw*mfac)-fabs(yw))*percent);
	dz = zw+sign(zw)*((float) (fabs(zw*mfac)-fabs(zw))*percent);
	x=dx+cx; y=dy+cy; z=dz+cz;

	// Mutiliply the point by the inverse of the ModContext tm and 
	// return the point.
	p.x = x; p.y = y; p.z = z;
	p = p*invtm;
	return p;
}

/*===========================================================================*\
 | Class Descriptor
\*===========================================================================*/
class SpherifyClassDesc : public ClassDesc {
  public:
	int				IsPublic() { return 1; }
	void			*Create(BOOL loading=FALSE) { return new SpherifyMod(); }
	const TCHAR		*ClassName() { return SPHERIFY_CLASSNAME; }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return SPHERIFY_CLASS_ID; }
	const TCHAR		*Category() { return CATEGORY_NAME;  }
};
static SpherifyClassDesc SpherifyCD;

/*===========================================================================*\
 | DLL/Lib Functions
\*===========================================================================*/
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
	hInstance = hinstDLL;

	if (! controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	
	return(TRUE);
}

__declspec( dllexport ) int LibNumberClasses() { 
	return 1; 
}

__declspec( dllexport ) ClassDesc *LibClassDesc(int i) { 
	return &SpherifyCD; 
}

__declspec( dllexport ) const TCHAR *LibDescription() {
	return GetString(IDS_SPHERIFYTITLE); 
}

__declspec( dllexport ) ULONG LibVersion() { 
	return VERSION_3DSMAX; 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

