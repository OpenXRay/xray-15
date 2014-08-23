/*===========================================================================*\
  How To Create an Object Modifier Plug-In

  FILE: BendMod.cpp

  DESCRIPTION:  Bend OSM

  CREATED BY: Dan Silva & Rolf Berteig

  HISTORY: created 30 Jauary, 1995

  Paramblock2 Support Added, April 2000  Neil Hazzard (Developer Consulting	Group)
 

  Copyright (c) 1994, All Rights Reserved.
\*===========================================================================*/
#include "max.h"
#include "resource.h"

// ParamBlock2 Support uses iparamm2.h


#include "simpmod.h"
#include "simpobj.h"
#include "iparamm2.h"

#define BIGFLOAT	float(999999)

// The unique Class_ID of this modifier.  It is specified as 
// two 32-bit quantities.
//
// Use the old ClassIds to allow loading of old files
//

// #define BEND_CID Class_ID(0x21066be6, 0x462d11cd) // old id for testing
#define BEND_CID Class_ID(0x4c497006, 0x62615491)
// #define BEND_CID Class_ID(BENDOSM_CLASS_ID,0) // official ID in MAX

// This is the Class_ID for the world space modifier version of Bend

// #define BENDWSM_CID Class_ID(0x73cc4a3d, 0x3a304fb5)	  // old id for testing
#define BENDWSM_CID Class_ID(0x28420e3d, 0x5f264e8a)
// #define BENDWSM_CID Class_ID(BENDOSM_CLASS_ID,1) // official ID in MAX

// This is the DLL instance handle passed in when the plug-in is 
// loaded at startup.
HINSTANCE hInstance;

// This function returns a pointer to a string in the string table of
// the resource library.  Note that this function maintains the buffer
// and that only one string is loaded at a time.  Therefore if you intend
// to use this string, you must copy to another buffer since it will 
// be overwritten on the next GetString() call.

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////		
//		The original BendMod was Subclassed off SimpleMod.  For Paramblock2 support SimpleMod2 is now used.
//		SimpleMod2 has one public  member, pblock2, which is an instance of the Paramblock2 used by the class
//		
//		Major Changes:
//
//		THere is no need for the GetParamName() and GetParamDim() as this is all available in the descriptors
//	
//		IParamArray is no longer needed since the class variable UI parameters are stored in static ParamBlocks
//      all corresponding class varibales have gone, including the ParamMaps since they are replaced 
//      by the new descriptors
//	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BendMod : public SimpleMod2 {	
	public:

		BendMod();
		
		// --- Interhited virtual methods of Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_BENDMOD); }  
		virtual Class_ID ClassID() { return BEND_CID;}
		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);

		// Add Direct Paramblock2 Support
		int	NumParamBlocks() { return 1; }	
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }


		// --- Interhited virtual methods of ReferenceMaker
		IOResult Load(ILoad *iload);

		// --- Interhited virtual methods of ReferenceTarget
		RefTargetHandle Clone(RemapDir& remap = NoRemap());

		// --- Interhited virtual methods of BaseObject
		TCHAR *GetObjectName() { return GetString(IDS_RB_BEND2);}

		// --- Interhited virtual methods of SimpleMod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
		Interval GetValidity(TimeValue t);

		BOOL GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis);
		void InvalidateUI(); 
	};

// This the the class whose Map method is called to deforms each point.
class BendDeformer: public Deformer {
	public:
		Matrix3 tm,invtm, tmAbove, tmBelow;
		Box3 bbox;
		TimeValue time;
		float r, from, to;
		int doRegion;
		BendDeformer();
		BendDeformer(
			TimeValue t, ModContext &mc,
			float angle, float dir, int naxis, 
			float from, float to, int doRegion, 
			Matrix3& modmat, Matrix3& modinv);
		void SetAxis(Matrix3 &tmAxis);
		void CalcR(int axis, float angle);
		// This is the deformers callback method which gets
		// called for each point. 
		Point3 Map(int i, Point3 p); 
	};

// This is the class to create the world space modifier version of Bend.
class BendWSM : public SimpleOSMToWSMObject {
	public:
		BendWSM() {}
		BendWSM(BendMod *m) : SimpleOSMToWSMObject(m) {}

		// --- Interhited virtual methods of Animatable
		void DeleteThis() { delete this; }
		SClass_ID SuperClassID() {return WSM_OBJECT_CLASS_ID;}
		Class_ID ClassID() {return BENDWSM_CID;} 

		// --- Interhited virtual methods of ReferenceTarget
		RefTargetHandle Clone(RemapDir& remap)
			{return (new BendWSM((BendMod*)mod->Clone(remap)))->SimpleOSMToWSMClone(this,remap);}

		// --- Interhited virtual methods of BaseObject
		TCHAR *GetObjectName() {return GetString(IDS_RB_BEND2);}
	};

// See the Advanced Topics section on DLL Functions and Class Descriptors
// for more information.
/*===========================================================================*\
 | The Class Descriptor
\*===========================================================================*/

// The classDesc is now derived from CLassDesc2

class BendClassDesc:public ClassDesc2 {
	public:
	// The IsPublic() method should return TRUE if the plug-in can be picked
	// and assigned by the user. Some plug-ins may be used privately by other
	// plug-ins implemented in the same DLL and should not appear in lists for
	// user to choose from -- these plug-ins would return FALSE.
	int 			IsPublic() { return 1; }
	// This is the method that actually creates a new instance of
	// a plug-in class.  The system calls the correspoding 
	// DeleteThis() method of the plug-in to free the memory.  Our 
	// implementations use 'new' and 'delete'.
	void *			Create(BOOL loading = FALSE) { return new BendMod; }
	// This is the name that appears on the button
	// in the MAX user interface.
	const TCHAR *	ClassName() { return GetString(IDS_RB_BEND_CLASS); }
	// The system calls this method at startup to determine the type of object
	// this is.  In our case, we're a geometric object so we return 
	// GEOMOBJECT_CLASS_ID.  The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	// The system calls this method to retrieve the unique
	// class id for this item.
	Class_ID		ClassID() { return BEND_CID; }
	// The category is selected
	// in the bottom most drop down list in the create branch.
	// If this is set to be an exiting category (i.e. "Primatives", ...) then
	// the plug-in will appear in that category. If the category doesn't
	// yet exists then it is created.  We use the new How To category for
	// all the example plug-ins in the How To sections.
	const TCHAR* 	Category() { return GetString(IDS_RB_HOWTO);}

	// The following are used by MAX Script and the Schematic View
	const TCHAR*	InternalName()	{return _T("BendMod");}
	HINSTANCE	HInstance() {return hInstance;};
	};

static BendClassDesc bendDesc;
ClassDesc* GetBendModDesc() { return &bendDesc; }

class BendWSMClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) 
		{if (loading) return new BendWSM; else return new BendWSM(new BendMod);}
	const TCHAR *	ClassName() { return GetString(IDS_RB_BEND_CLASS); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return BENDWSM_CID; }
	const TCHAR* 	Category() {return GetString(IDS_MODBASED);}

	const TCHAR*	InternalName()	{return _T("BendModWSM");}
	HINSTANCE	HInstance() {return hInstance;}

	};

static BendWSMClassDesc bendWSMDesc;
ClassDesc* GetBendWSMDesc() { return &bendWSMDesc; }



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// We will need the following so that we can load in the original IDs and then get converted to the new system
// The new IDs have been inserted for automatic load conversion.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum { bend_params,};

// CAL-11/10/00: the order here must be the same as the order in the older version PB
enum {	bend_angle,
		bend_dir,
		bend_axis,
		bend_fromto,
		bend_from,
		bend_to,
};


// The old ParamBlockDescID has been updated to use the IDs created in the enum

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, bend_angle },
	{ TYPE_FLOAT, NULL, TRUE, bend_dir },
	{ TYPE_INT, NULL, FALSE, bend_axis } };

// The current version
static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, bend_angle },
	{ TYPE_FLOAT, NULL, TRUE, bend_dir },	
	{ TYPE_INT, NULL, FALSE, bend_axis },
	{ TYPE_INT, NULL, FALSE, bend_fromto },
	{ TYPE_FLOAT, NULL, TRUE, bend_from },
	{ TYPE_FLOAT, NULL, TRUE, bend_to } };

#define PBLOCK_LENGTH	6
#define NUM_OLDVERSIONS	2

// CAL-11/10/00: need to include all versions.
// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0),
	ParamVersionDesc(descVer1,PBLOCK_LENGTH,1)
	};


// CAL-11/10/00: These are not used anymore.
// Current version
// #define CURRENT_VERSION	1
// static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);



//--- BendDlgProc -------------------------------

////////////////////////////////////////////////////////////////////////////////////////
//
// For ParamMap2 Create an instance of a class derived from ParamMap2UserDlgProc.  This 
// is used to provide special processing of controls in the rollup page.-- NH 
// The original use of this has been replaced by the use of PBAccessor
//
////////////////////////////////////////////////////////////////////////////////////////

class BendDlgProc : public ParamMap2UserDlgProc {
	public:
		BOOL DlgProc(TimeValue t,IParamMap2* map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {}
	};

static BendDlgProc theBendProc;


BOOL BendDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	return FALSE;
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PBaccessor allows for a call back mechanism on GetValue / SetValue calls.  It allows you to check the value
// being passed by PB2Value and to manipulate it as necessary.
//
// Implement the check here for from/to crossover.  THis was originaly implemented in the Dialog Prodcedure - NH
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class bendPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BendMod* u = (BendMod*)owner;
		IParamMap2* pmap = u->pblock2->GetMap();
		float from, to;

		switch(id)
		{
			case bend_from:
				u->pblock2->GetValue(bend_to,t,to,FOREVER);
				from = v.f;
				if (from >to) {
					u->pblock2->SetValue(bend_to,t,from);
				}
				break;

			case bend_to:
				u->pblock2->GetValue(bend_from,t,from,FOREVER);
				to = v.f;
				if (from>to) {
					u->pblock2->SetValue(bend_from,t,to);
				}
				break;
				

		}
	}

};


static bendPBAccessor   bendPBAccessor;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Lets create the new Parameter Maps
//
// 		The paramblock will be created with P_AUTO_UI and P_AUTO_CONTSRUCT. This means that the system will take
//		care of creating the ParmMaps, references and the UI.  As the P_AUTO_UI is used additional information
//		on the dialog resource is given,

static ParamBlockDesc2 bend_param_blk ( bend_params, _T("Bend Parameters"),  0, &bendDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, SIMPMOD_PBLOCKREF, 
	//rollout
	IDD_BENDPARAM, IDS_RB_PARAMETERS, 0, 0, NULL, 
	// params

	bend_angle,	_T("BendAngle"),	TYPE_FLOAT,	P_RESET_DEFAULT|P_ANIMATABLE,	IDS_ANGLE,
		p_default,		0.0f,
		p_range, 		-BIGFLOAT, BIGFLOAT, 
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ANGLE, IDC_ANGLESPINNER, 0.5f,
		end,

	bend_dir,	_T("BendDir"),	TYPE_FLOAT,	P_RESET_DEFAULT|P_ANIMATABLE,	IDS_DIR,
		p_default,		0.0f,
		p_range, 		-BIGFLOAT, BIGFLOAT, 
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_DIR, IDC_DIRSPINNER, 0.5f,
		end,

	bend_axis, 		_T("BendAxis"), TYPE_INT, P_RESET_DEFAULT,	IDS_AXIS, 	
		p_default, 		2, 
		p_ui, 			TYPE_RADIO, 3,IDC_X,IDC_Y,IDC_Z, 
		p_vals,			0,1,2,
		end, 
		
	bend_fromto, 		_T("FromTo"), TYPE_BOOL, P_RESET_DEFAULT,	IDS_FROMTO, 	
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_BEND_AFFECTREGION, 
		end, 

	bend_from,	_T("BendFrom"),	TYPE_FLOAT,	P_RESET_DEFAULT|P_ANIMATABLE,	IDS_FROM,
		p_default,		0.0f,
		p_range, 		-BIGFLOAT, 0.0f, 
		p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_BEND_FROM, IDC_BEND_FROMSPIN, SPIN_AUTOSCALE,
		p_accessor,		&bendPBAccessor,
		end,

	bend_to,	_T("BendTo"),	TYPE_FLOAT,	P_RESET_DEFAULT|P_ANIMATABLE,	IDS_TO,
		p_default,		0.0f,
		p_range, 		0.0f, BIGFLOAT, 
		p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_BEND_TO, IDC_BEND_TOSPIN, SPIN_AUTOSCALE,
		p_accessor,		&bendPBAccessor,
		end,

	end
	);





//--- Bend methods -------------------------------


BendMod::BendMod() 
	{	

//  The following is removed as it is all handled by the calls to MakeAutoParamBlock() - NH 

//	MakeRefByID(FOREVER, SIMPMOD_PBLOCKREF, 
//		CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	
//	pblock->SetValue(PB_AXIS, TimeValue(0), 2);


	bendDesc.MakeAutoParamBlocks(this);
	assert(pblock2);
	}

// --- Interhited virtual methods of Animatable
// This method is called by the system when the user needs 
// to edit the modifiers parameters in the command panel.  
void BendMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	// Call the SimpleMod method first to take care of its processing.
	this->ip = ip ;
	SimpleMod::BeginEditParams(ip,flags,prev);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams - NH
	bendDesc.BeginEditParams(ip,this,flags,prev);
		
	}
		
// This is called by the system to terminate the editing of the
// parameters in the command panel.  
void BendMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	// We must call the SimpleMod method first to take care of its 
	// processing.
	SimpleMod::EndEditParams(ip,flags,next);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams - NH
	bendDesc.EndEditParams(ip,this,flags,next);


	}

// --- Interhited virtual methods of ReferenceMaker
// Called by MAX when the modifier is loaded from disk.

IOResult BendMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the MAX file 
	// being loaded.
	
	// CAL-11/10/00: old versions PB will be converted by the new ParamBlock2PLCB
	// iload->RegisterPostLoadCallback(
	//	new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,SIMPMOD_PBLOCKREF));


	// this callback allows old Paramblocks to be loaded in and automatically converted to 
	// the ParamBlock2 mechanism - NH

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &bend_param_blk, this, SIMPMOD_PBLOCKREF);
	iload->RegisterPostLoadCallback(plcb);


	return IO_OK;
	}

// --- Interhited virtual methods of ReferenceTarget
// This method makes a new BendMod and copies the state of this modifier
// to the new modifier.
// This method is called when the user makes a copy of
// an object in the scene or when they press the make unique
// button in the modfier stack rollup page.
RefTargetHandle BendMod::Clone(RemapDir& remap) {	
	BendMod* newmod = new BendMod();	
	newmod->ReplaceReference(SIMPMOD_PBLOCKREF,pblock2->Clone(remap));
	newmod->SimpleModClone(this);
	BaseClone(this, newmod, remap);
	return(newmod);
	}

// --- Interhited virtual methods of SimpleMod
// This returns the vality interval of the modifier. The SimpleMod
// class calls this method and combines the result with the 
// validity interval of the controllers controlling the gizmo and center
// to form the total validity interval.


// This has been modified to use the pblock2 instead of the old pblock - NH


Interval BendMod::GetValidity(TimeValue t)
	{
	float f;	
	// Start our interval at forever...
	Interval valid = FOREVER;
	// Intersect each parameters interval to narrow it down.
	pblock2->GetValue(bend_angle,t,f,valid);
	pblock2->GetValue(bend_dir,t,f,valid);	
	pblock2->GetValue(bend_from,t,f,valid);
	pblock2->GetValue(bend_to,t,f,valid);
	// Return the final validity interval.
	return valid;
	}

void BendMod::InvalidateUI()
{
	bend_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}

// This has been modified to use the pblock2 instead of the old pblock - NH

BOOL BendMod::GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis)
	{
	int limit;
	pblock2->GetValue(bend_fromto,t,limit,FOREVER);
	pblock2->GetValue(bend_from,t,zmin,FOREVER);
	pblock2->GetValue(bend_to,t,zmax,FOREVER);
	pblock2->GetValue(bend_axis,t,axis,FOREVER);
	return limit?TRUE:FALSE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// The following Reference management code has been updated to handle the Paramblock2 pointer.
//
//////////////////////////////////////////////////////////////////////////////////////////////


RefTargetHandle SimpleMod2::GetReference(int i) 
	{ 
	switch (i) {
		case 0: return tmControl;
		case 1: return posControl;
		case 2: return pblock2;
		default: return NULL;
		}
	}

void SimpleMod2::SetReference(int i, RefTargetHandle rtarg) 
	{ 
	switch (i) {
		case 0: tmControl = (Control*)rtarg; break;
		case 1: posControl = (Control*)rtarg; break;
		case 2: pblock2 = (IParamBlock2*)rtarg; break;
		}
	}

Animatable* SimpleMod2::SubAnim(int i) 
	{ 
	switch (i) {
		case 0: return posControl;
		case 1: return tmControl;
		case 2: return pblock2;			
		default: return NULL;
		}
	}


BendDeformer::BendDeformer() 
	{ 
	tm.IdentityMatrix();
	time = 0;	
	}

void BendDeformer::SetAxis(Matrix3 &tmAxis)
	{
	Matrix3 itm = Inverse(tmAxis);
	tm    = tm*tmAxis;
	invtm =	itm*invtm;
	}

void BendDeformer::CalcR(int axis, float angle)
	{
	float len = float(0);
	if (!doRegion) {
		switch (axis) {
			case 0:  len = bbox.pmax.x - bbox.pmin.x; break;
			case 1:	 len = bbox.pmax.y - bbox.pmin.y; break;
			case 2:  len = bbox.pmax.z - bbox.pmin.z; break;
			}
	} else {
		len = to-from;
		}

	// Skip the singularity
	if (fabs(angle) <0.0001) {
		r = float(0);
	} else {
		r = len/angle;
		}	
	}

// This is the deformers callback method which actually gets
// called for each selected point. It deforms the point passed
// and returns it.
Point3 BendDeformer::Map(int i, Point3 p)
	{
	float x, y, c, s, yr;
	if (r==0 && !doRegion) return p;
	// The point is first transformed by the tm.  This is typical
	// for all modifiers. See the Advanced Topics section on the 
	// Geometry Pipeline for more details.
	p = p * tm;
	if (doRegion) {
		if (p.z<from) {
			return tmBelow * p * invtm;			
		} else 
		if (p.z>to) {
			return tmAbove * p * invtm;
			}
		}	
	if (r==0) return p * invtm;
	x = p.x;
	y = p.z;
	yr = y/r;
	c = float(cos(PI-yr));
	s = float(sin(PI-yr));
	p.x = r*c + r - x*c;
	p.z = r*s - x*s;
	// The point is finally transformed by the inverse of the tm.
	p = p * invtm;
	return p;
	}

BendDeformer::BendDeformer(
		TimeValue t, ModContext &mc,
		float angle, float dir, int naxis, 
		float from, float to, int doRegion,
		Matrix3& modmat, Matrix3& modinv) 
	{	
	this->doRegion = doRegion;
	this->from = from;
	this->to   = to;
	Matrix3 mat;
	Interval valid;	
	time   = t;	

	tm = modmat;
	invtm = modinv;
	mat.IdentityMatrix();
	
	switch (naxis) {
		case 0: mat.RotateY( -HALFPI );	 break; //X
		case 1: mat.RotateX( HALFPI );  break; //Y
		case 2: break;  //Z
		}
	mat.RotateZ(DegToRad(dir));	
	SetAxis(mat);	
	assert (mc.box);
	bbox = *mc.box;
	CalcR(naxis,DegToRad(angle));
	
	// Turn this off for a sec.
	this->doRegion = FALSE;
		
	float len  = to-from;
	float rat1, rat2;
	if (len==0.0f) {
		rat1 = rat2 = 1.0f;
	} else {
		rat1 = to/len;
		rat2 = from/len;
		}
	Point3 pt;
	tmAbove.IdentityMatrix();
	tmAbove.Translate(Point3(0.0f,0.0f,-to));
	tmAbove.RotateY(DegToRad(angle * rat1));
	tmAbove.Translate(Point3(0.0f,0.0f,to));
	pt = Point3(0.0f,0.0f,to);
	tmAbove.Translate((Map(0,pt*invtm)*tm)-pt);

	tmBelow.IdentityMatrix();
	tmBelow.Translate(Point3(0.0f,0.0f,-from));
	tmBelow.RotateY(DegToRad(angle * rat2));	
	tmBelow.Translate(Point3(0.0f,0.0f,from));
	pt = Point3(0.0f,0.0f,from);
	tmBelow.Translate((Map(0,pt*invtm)*tm)-pt);	
	
	this->doRegion = doRegion;

	} 

// Provides a reference to our callback object to handle the deformation.
// This has been modified to use the pblock2 instead of the old pblock - NH

Deformer& BendMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	float angle, dir, from, to;
	int axis;	
	int doRegion;
	pblock2->GetValue(bend_angle,t,angle,FOREVER);
	pblock2->GetValue(bend_dir,t,dir,FOREVER);
	pblock2->GetValue(bend_axis,t,axis,FOREVER);
	pblock2->GetValue(bend_from,t,from,FOREVER);
	pblock2->GetValue(bend_to,t,to,FOREVER);
	pblock2->GetValue(bend_fromto,t,doRegion,FOREVER);
	static BendDeformer deformer;
	deformer = BendDeformer(t,mc,angle,dir,axis,from,to,doRegion,mat,invmat);
	return deformer;
	}



// The following five functions are used by every plug-in DLL.
/*===========================================================================*\
 | The DLL and Library Functions
\*===========================================================================*/
// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;

	if (! controlsInit) {
		controlsInit = TRUE;
		
		// Initialize MAX's custom controls
		InitCustomControls(hInstance);
		
		// Initialize Win95 controls
		InitCommonControls();
	}
	
	return(TRUE);
	}

// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int LibNumberClasses() {return 2;}

// This function return the ith class descriptor
__declspec( dllexport ) ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case 0: return GetBendModDesc();		
		case 1: return GetBendWSMDesc();		
		default: return 0;
		}
	}

// This function returns a string that describes the DLL.  This string appears in 
// the File / Summary Info / Plug-In Info dialog box.
__declspec( dllexport ) const TCHAR *LibDescription() { 
	return GetString(IDS_LIB_DESC); 
	}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }







