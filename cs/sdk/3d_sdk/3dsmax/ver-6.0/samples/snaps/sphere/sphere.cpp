/*****************************************************************************
 *<
	FILE: sphere.cpp

	DESCRIPTION:  An example object snap
				  This plugin will respond to spheres and supports two subsnaps:
				  quadrant and center. Center being the (0,0,0) point in object space. 
				  The quadrants are the points (+-r,0,0),(0,+-r,0),(0,0,+-r) in object space.
				  Note that the snap points are computed in object space and that the plugin
				  need not worry about the node tm. 

	CREATED BY: John Hutchinson		

	HISTORY: created 6/17/97

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/
// The main MAX include file.
#include "max.h"

//master include file for osnaps
#include "osnapapi.h"

//#include "data.h"
#include "resource.h"

//include the header file which defines the class to which we're gonna snap
#include "simpobj.h"

//An osnap can support multiple subsnaps. The following constants are for discriminating
//between them. In this case we'll have two. 
#define SUBCOUNT 2
#define NOSUB -1
#define CENTER_SUB 0
#define QUAD_SUB 1

//typedef Tab<Point3> Point3Tab;

// This is the DLL instance handle passed in when the plug-in is 
// loaded at startup.
HINSTANCE hInstance;

TCHAR *GetString(int id);

#define SPHERE_OSNAP_CLASS_ID Class_ID(0x12b51e31, 0x232101d2)

class SphereSnap : public Osnap {

private:
//	OsnapMarker markerdata[SUBCOUNT];
	TSTR name[SUBCOUNT]; //A local array of our subsnap names 
	HBITMAP tools; //a handle to the bitmaps
	HBITMAP masks; // a handle to the masks

public:

	SphereSnap();//constructor
	virtual ~SphereSnap();

	virtual int numsubs(){return SUBCOUNT;} //the number of subsnaps this guy has
	virtual TSTR *snapname(int index); // the snap’s name to be displayed in the UI
	virtual TCHAR* Category(){return _T("How To");}
	Class_ID ClassID() { return SPHERE_OSNAP_CLASS_ID; }

	virtual boolean ValidInput(SClass_ID scid, Class_ID cid);// supports anything 

	virtual OsnapMarker *GetMarker(int index){return NULL;} 
	virtual HBITMAP getTools(){return tools;} 
	virtual HBITMAP getMasks(){return masks;} 
	virtual WORD AccelKey(int index); //virtual key codes
	virtual void Snap(Object* pobj, IPoint2 *p, TimeValue t);
};



TSTR *SphereSnap::snapname(int index){
	return &name[index];
}


WORD SphereSnap::AccelKey(int index){
	switch (index){
	case CENTER_SUB:
		return 0x4F;
		break;
	case QUAD_SUB:
		return 0x42;
		break;
	default:
		return 0;
	}
}


SphereSnap::SphereSnap()
{
	tools = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ICONS));
	masks = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MASK));
	name[CENTER_SUB]= TSTR(GetString(IDS_CENTER));
	name[QUAD_SUB]= TSTR(GetString(IDS_QUAD));
};

SphereSnap::~SphereSnap()
{
	DeleteObject(tools);
	DeleteObject(masks);
}

boolean SphereSnap::ValidInput(SClass_ID scid, Class_ID cid){
	boolean c_ok = FALSE, sc_ok = FALSE;
	sc_ok |= (scid == GEOMOBJECT_CLASS_ID)? TRUE : FALSE;
	c_ok |= (cid == Class_ID(SPHERE_CLASS_ID,0))? TRUE : FALSE; 
	return sc_ok && c_ok;
}


void SphereSnap::Snap(Object* pobj, IPoint2 *p, TimeValue t)
{	
	// This snap computes the center of spheres as well as it's quadrants 

	//local copy of the cursor position
	Point2 fp = Point2((float)p->x, (float)p->y);



	BOOL got_one= FALSE;

	//Check if the center snap is turned on.
	//If so compute the center
	if(	GetActive(CENTER_SUB))
	{
		//The center in object space is just the object space origin
		Point3 centerpt(0.0f, 0.0f, 0.0f);

		//now register a hit with the osnap manager
		if(CheckPotentialHit(&centerpt,0,fp))
			//CheckPotentialHit returns TRUE if the point is within snapstrength
			//of the cursor position
			theman->RecordHit(new OsnapHit(centerpt, this, CENTER_SUB, NULL));
	}


	//check if the quad sub is turned on, if so compute the points
	if(	GetActive(QUAD_SUB))
	{
		Point3 thepoints[6];//we compute six quadrants
	
		// get some parametric data from the sphere
		// Notice that the cast is safe because we would not be in this
		// method if the node had not satisfied our ValidInput method. 
		float radius;
		((GenSphere *)pobj)->pblock->GetValue(0,t,radius,FOREVER);

		// set up the points to which we snap, in this case six
		// notice that these are in object space
		thepoints[0] = Point3(0.0f,0.0f,radius);
		thepoints[1] = Point3(0.0f,radius,0.0f);
		thepoints[2] = Point3(radius,0.0f,0.0f);
		thepoints[3] = Point3(0.0f,0.0f,-radius);
		thepoints[4] = Point3(0.0f,-radius,0.0f);
		thepoints[5] = Point3(-radius,0.0f,0.0f);

		for(int ii = 0;ii<6;++ii)
		{
			if(CheckPotentialHit(thepoints,ii,fp))
				theman->RecordHit(new OsnapHit(thepoints[ii], this, QUAD_SUB, NULL));
		}
	}


};




TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}


/*===========================================================================*\
 | The Class Descriptor
\*===========================================================================*/

class OsnapClassDesc:public ClassDesc {
	public:
	// The IsPublic() method should return TRUE if the plug-in can be picked
	// and assigned by the user. Some plug-ins may be used privately by other
	// plug-ins implemented in the same DLL and should not appear in lists for
	// user to choose from, so these plug-ins would return FALSE.
	int 			IsPublic() { return 0; }
	// This is the method that actually creates a new instance of
	// a plug-in class.  By having the system call this method,
	// the plug-in may use any memory manager it wishes to 
	// allocate its objects.  The system calls the correspoding 
	// DeleteThis() method of the plug-in to free the memory.  Our 
	// implementations use 'new' and 'delete'.
	void *			Create(BOOL loading = FALSE) {return new SphereSnap();}
//	void *			Create(OsnapManager *pman) { return new SphereSnap(pman); }
	// This is used for debugging purposes to give the class a 
	// displayable name.  It is also the name that appears on the button
	// in the MAX user interface.
	const TCHAR *	ClassName() { return _T("SphereSnap"); }
	// The system calls this method at startup to determine the type of object
	// this is.  In our case, we're a geometric object so we return 
	// GEOMOBJECT_CLASS_ID.  The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return OSNAP_CLASS_ID; }
	// The system calls this method to retrieve the unique
	// class id for this object.
	Class_ID		ClassID() { 
		return SPHERE_OSNAP_CLASS_ID; }
	// The category is selected
	// in the bottom most drop down list in the create branch.
	// If this is set to be an exiting category (i.e. "Primatives", ...) then
	// the plug-in will appear in that category. If the category doesn't
	// yet exists then it is created.  We use the new How To category for
	// all the example plug-ins in the How To sections.
	const TCHAR* 	Category() { return _T("How To"); }
	};

// Declare a static instance of the class descriptor.
static OsnapClassDesc sampDesc;
// This function returns the address of the descriptor.  We call it from 
// the LibClassDesc() function, which is called by the system when loading
// the DLLs at startup.
ClassDesc* GetSampDesc() { return &sampDesc; }

/*===========================================================================*\
 | The DLL Functions
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
__declspec( dllexport ) int LibNumberClasses() {return 1;}

// This function return the ith class descriptor
__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetSampDesc();		
		default: return 0;
		}
	}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG
LibVersion() {  return VERSION_3DSMAX; }

