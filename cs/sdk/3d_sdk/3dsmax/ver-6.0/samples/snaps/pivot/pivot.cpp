/*****************************************************************************
 *<
	FILE: pivot.cpp

	DESCRIPTION:  Snaps to pivots

	CREATED BY: John Hutchinson		

	HISTORY: created 12/12/96

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/
// The main MAX include file.
#include "max.h"

//master include file for osnaps
#include "osnapapi.h"

#include "data.h"
#include "resource.h"

#define SUBCOUNT 2
#define NOSUB -1
#define PIV_SUB 0
#define BBOX_SUB 1

typedef Tab<Point3> Point3Tab;

// This is the DLL instance handle passed in when the plug-in is 
// loaded at startup.
HINSTANCE hInstance;

TCHAR *GetString(int id);

#define PIVOT_OSNAP_CLASS_ID Class_ID(0x24960fd5, 0x343a4913)

class PivotSnap : public Osnap {

private:
	OsnapMarker markerdata[SUBCOUNT];
	TSTR name[SUBCOUNT];
	HBITMAP tools;
	HBITMAP masks;

public:

	PivotSnap();//constructor
	virtual ~PivotSnap();

	virtual int numsubs(){return SUBCOUNT;} //the number of subsnaps this guy has
	virtual TSTR *snapname(int index); // the snap’s name to be displayed in the UI
	virtual boolean ValidInput(SClass_ID scid, Class_ID cid);// supports anything 
	Class_ID ClassID() { return PIVOT_OSNAP_CLASS_ID; }

	virtual OsnapMarker *GetMarker(int index){return &(markerdata[index]);} 
	virtual HBITMAP getTools(){return tools;} 
	virtual HBITMAP getMasks(){return masks;} 
	virtual WORD HiliteMode(){return HILITE_BOX;}
	virtual WORD AccelKey(int index); //virtual key codes
	virtual void Snap(Object* pobj, IPoint2 *p, TimeValue t);
};



TSTR *PivotSnap::snapname(int index){
	return &name[index];
}


WORD PivotSnap::AccelKey(int index){
	switch (index){
	case PIV_SUB:
		return 0x4F;
		break;
	case BBOX_SUB:
		return 0x42;
		break;
	default:
		return 0;
	}
}


PivotSnap::PivotSnap()
{
	tools = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ICONS));
	masks = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MASK));
	name[PIV_SUB]= TSTR(GetString(IDS_PIVOT));
	name[BBOX_SUB]= TSTR(GetString(IDS_BBOX));
	markerdata[0]=OsnapMarker(5,mark0verts,mark0es);
	markerdata[1]=OsnapMarker(13,mark1verts,mark1es);
};

PivotSnap::~PivotSnap()
{
	DeleteObject(tools);
	DeleteObject(masks);
}

boolean PivotSnap::ValidInput(SClass_ID scid, Class_ID cid){

	//JH 6/17/99
	//The implementation of the snap manager assumes that any nodes which record hits
	//will still be around when the scenetraversal ends. With the introduction of autogrid
	//this assumption is clearly wrong since the temporary grid object comes and goes.
	//The short term fix is to avoid snapping to these grid objects.
	if(theman->GetNode() == GetCOREInterface()->GetActiveGrid())
		return FALSE;


	boolean c_ok = FALSE, sc_ok = FALSE;
	sc_ok |= (scid == GEOMOBJECT_CLASS_ID)? TRUE : FALSE;
	sc_ok |= (scid == SHAPE_CLASS_ID)? TRUE : FALSE;
	sc_ok |= (scid == CAMERA_CLASS_ID)? TRUE : FALSE;
	sc_ok |= (scid == LIGHT_CLASS_ID)? TRUE : FALSE;
	sc_ok |= (scid == HELPER_CLASS_ID)? TRUE : FALSE;
//	c_ok |= (cid == Class_ID(0xe44f10b3,0))? TRUE : FALSE; 
	return sc_ok;
}

static BOOL EssentiallyEmpty(Box3 &box)
{
	Point3 p2(0.01f,0.01f,0.01f);
	Point3 p1(-0.01f,-0.01f,-0.01f);
	if(box.pmin == p1 && box.pmax == p2)
		return TRUE;
	return FALSE;
}

void PivotSnap::Snap(Object* pobj, IPoint2 *p, TimeValue t)
{	
	// This snap computes the bounding box points of a node as 
	// well as the pivot point

	//local copy of the cursor position
	Point2 fp = Point2((float)p->x, (float)p->y);

	//In this snap mode we actually need to get a pointer to the node so that we 
	// can check for WSM's and compute the pivot point
	INode *inode = theman->GetNode();
	Matrix3 atm(1); //This will hold the nodes tm before WSMs

	//See if this guys has any spacewarps applied
	BOOL wsm = (BOOL) inode->GetProperty(PROPID_HAS_WSM);

	//If it does then we'll need to get a meaningful tm as follows
	if(wsm)
		atm = inode->GetObjTMBeforeWSM(t);

	//get the node's bounding box
	Box3 box;
	box.Init();
	pobj->GetDeformBBox(t, box, NULL );

	if(EssentiallyEmpty(box))
		pobj->GetLocalBoundBox(t, inode, theman->GetVpt() , box);

		//We need a hitmesh which shows the bounding box of the node
	//This automatic variable gets passed to the hitmesh copy constructor
	// in every case
	HitMesh thehitmesh, *phitmesh;
	thehitmesh.setNumVerts(8);
	for(int jj = 0;jj<8;++jj)
		thehitmesh.setVert(jj,box[jj]);


	BOOL got_one= FALSE;

	//Compute all the hit point candidates
	if(	GetActive(PIV_SUB))
	{
		got_one = FALSE;
		Point3 *pivpt;

		//JH 10/02/01
		//DID 296059
		Matrix3 tm(1);
		Point3 pos = inode->GetObjOffsetPos();
		tm.PreTranslate(pos);
		Quat quat = inode->GetObjOffsetRot();
		PreRotateMatrix(tm, quat);
		ScaleValue scale = inode->GetObjOffsetScale();
		ApplyScaling(tm, scale);
		Matrix3 InvTm = Inverse(tm);



		//JH 10/02/01
		//atm contains the identity normally, or the node TM before spacewarps, when space warps are applied
		//We're computing a point relative to the node TM, so in the former case the inverse of
		//the object offset pos is what we want. In the latter (when the node TM is identtity, we must add
		//in the node TM before WSM.
		pivpt = new Point3(atm.GetTrans() + InvTm.GetTrans());

		//Make a hitmesh
		phitmesh = new HitMesh(thehitmesh);

		//now register a hit with the osnap manager
		theman->RecordHit(new OsnapHit(*pivpt, this, PIV_SUB, phitmesh));
	}

	if(	GetActive(BBOX_SUB))
	{

		//set up our highlight mesh
		for(int ii = 0;ii<8;++ii)
		{
			phitmesh = new HitMesh(thehitmesh);

			theman->RecordHit(new OsnapHit(box[ii], this, BBOX_SUB, phitmesh));
		}
	}


};


//static PivotSnap thePivotSnap;

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
	void *			Create(BOOL loading = FALSE) {return new PivotSnap();}
//	void *			Create(OsnapManager *pman) { return new PivotSnap(pman); }
	// This is used for debugging purposes to give the class a 
	// displayable name.  It is also the name that appears on the button
	// in the MAX user interface.
	const TCHAR *	ClassName() { return _T("PivotSnap"); }
	// The system calls this method at startup to determine the type of object
	// this is.  In our case, we're a geometric object so we return 
	// GEOMOBJECT_CLASS_ID.  The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return OSNAP_CLASS_ID; }
	// The system calls this method to retrieve the unique
	// class id for this object.
	Class_ID		ClassID() { 
		return PIVOT_OSNAP_CLASS_ID; }
	// The category is selected
	// in the bottom most drop down list in the create branch.
	// If this is set to be an exiting category (i.e. "Primatives", ...) then
	// the plug-in will appear in that category. If the category doesn't
	// yet exists then it is created.  We use the new How To category for
	// all the example plug-ins in the How To sections.
	const TCHAR* 	Category() { return _T(""); }
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

