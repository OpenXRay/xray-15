/*****************************************************************************
 *<
	FILE: Shape.cpp

	DESCRIPTION:  Implementation of an Osnap for ShapeObjects
					Provides perpendicular and tangent snapping

	CREATED BY: John Hutchinson		

	HISTORY: created 12/12/96

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/
// The main MAX include file.
#include "max.h"
//some other classes 
#include "polyshp.h"
#include "splshape.h"
#include "tab.h"

#include <assert.h>
#include <float.h>

//master include file for osnaps
#include "osnapapi.h"

#include "resource.h"
#include "data.h"


#define SUBCOUNT 2
#define NOSUB -1
#define PERP_SUB 0
#define TAN_SUB 1

#define TAN_TOLERANCE 0.001

//flags for extended perpendiculars
#define NEG	0
#define POS 1
#define BOTH 2

// This is the DLL instance handle passed in when the plug-in is 
// loaded at startup.
HINSTANCE hInstance;


//some prototypes
Point3 *PerpToSeg(const Point3 k, const Point3 l, const Point3 m, const BOOL extend = FALSE, const int extdir = -1);

Point3 *TanToSeg(const Point3 k, const Point3 l, const Point3 m);

TCHAR *GetString(int id);

#define SHAPE1_SNAP_CLASS_ID Class_ID(0x1f120de7, 0x30bc716f)

class ShapeSnap : public Osnap {
private:
	boolean active[SUBCOUNT];
	TSTR name[SUBCOUNT];
	OsnapMarker markerdata[SUBCOUNT];
	HBITMAP tools;
	HBITMAP masks;
	float paramval;

public:

	ShapeSnap();//constructor
	virtual ~ShapeSnap();

	virtual int numsubs(){return SUBCOUNT;} //the number of subsnaps this guy has
	virtual TSTR *snapname(int index); // the snap’s name to be displayed in the UI
	virtual boolean ValidInput(SClass_ID scid, Class_ID cid);// the type of object that it recognizes SPHERE_CLASS_ID 
	Class_ID ClassID() { return SHAPE1_SNAP_CLASS_ID; }

	virtual OsnapMarker *GetMarker(int index){return &(markerdata[index]);} // single object might contain subsnaps
	virtual HBITMAP getTools(){return tools;} // single object might contain subsnaps
	virtual HBITMAP getMasks(){return masks;} // single object might contain subsnaps
	virtual WORD AccelKey(int index);
	virtual void Snap(Object* pobj, IPoint2 *p, TimeValue t);

	void PointsOnPolyLine_Tan(PolyLine& pline,  const Point3& relpoint);
	void PointsOnPolyShape_Tan(const PolyShape& pshape,  const Point3& relpoint);
	void PointsOnPolyLine_Perp(PolyLine& pline, const Point3& relpoint);
	void PointsOnPolyShape_Perp(const PolyShape& pshape, const Point3& relpoint);

};


TSTR *ShapeSnap::snapname(int index){
	return &(name[index]);
}



ShapeSnap::ShapeSnap(){ //constructor
	active[PERP_SUB] = FALSE;
	active[TAN_SUB] = FALSE;

	name[PERP_SUB] = TSTR( GetString(IDS_PERP));
	name[TAN_SUB] = TSTR( GetString(IDS_TAN));

	tools = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ICONS));
	masks = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MASK));

	markerdata[0]=OsnapMarker(8,mark0verts,mark0es);
	markerdata[1]=OsnapMarker(11,mark1verts,mark1es);

	paramval = 0.5f;

};

ShapeSnap::~ShapeSnap()
{
	DeleteObject(tools);
	DeleteObject(masks);
}


boolean ShapeSnap::ValidInput(SClass_ID scid, Class_ID cid){
	boolean c_ok = FALSE, sc_ok = FALSE;
	sc_ok |= (scid == SHAPE_CLASS_ID)? TRUE : FALSE;
//	c_ok |= (cid == Class_ID(SPLINESHAPE_CLASS_ID,0))? TRUE : FALSE; 
//	c_ok |= (cid == Class_ID(PATCHOBJ_CLASS_ID,0))? TRUE : FALSE; 
	return sc_ok;
}


WORD ShapeSnap::AccelKey(int index){
	switch (index){
/*	case ARC_SUB:
		return 0x41;
		break;
	*/
	case PERP_SUB:
		return 0x50;
		break;
	case TAN_SUB:
		return 0x54;
		break;
	default:
		return 0;
	}
}

Point3 *PerpToSeg(const Point3 k, const Point3 l, const Point3 m, const BOOL extend, const int extflag)
{
	Point3 mk(m-k), lk(l-k);
	if(Length(mk^lk) < FLT_EPSILON)
		return NULL; //colinear TODO optimize
	float s = DotProd(mk,lk)/DotProd(lk,lk);
	
	//	if (s<0.0f || s>1.0f)
	if ((s<0.0f || s>1.0f) && !extend)
		return NULL;
	if(extflag == POS && s<0.0f)
		return NULL;
	if(extflag == NEG && s>1.0f)
		return NULL;

	// we're ok so return the computed point
	return new Point3(k + (s * lk));
}

Point3 *TanToSeg(const Point3 k, const Point3 l, const Point3 m)
{
	Point3 mk(m-k), lk(l-k);
//	if(Length(mk^lk) < TAN_TOLERANCE)
	float costheta = (float)fabs(DotProd(mk,lk)/sqrt(DotProd(lk,lk) * DotProd(mk,mk)));
	if (1.0f - costheta < TAN_TOLERANCE)
		return new Point3(k);
	else 
		return NULL;
}


void ShapeSnap::PointsOnPolyLine_Perp(PolyLine& pline, const Point3& relpoint)
{
	BOOL open = !pline.IsClosed();
	Point3 *ppoint;
	PolyPt *p1 = pline.pts;
	PolyPt *p2 = p1 + 1;
	int segs = pline.Verts()-1;
//watje 3-7-99 fix to handle hidden spline segs
	BOOL vis = TRUE;
	for (int i=0; i<segs; ++i, p1++, p2++)
	{
		BOOL extend = open && (segs==1 || i==0 || i==segs-1)?TRUE:FALSE;
		int look = -1;
		
		// if we need to look for an extended perpendicular,
		// figure out if we should look beyond the end of the last segment
		// or before the beginning of the first
		// or possibly look both ways in the case of a single segment poly
		if(extend){
			if(segs == 1)
				look = BOTH;
			else if(i==0)
				look = NEG;
			else if(i==segs-1)
				look = POS;
		}

		PolyPt ppt = pline.pts[i];
		if (ppt.flags & POLYPT_VISEDGE)
			vis = TRUE;
		else if (ppt.flags & POLYPT_INVIS_EDGE)
			vis = FALSE;
		if (vis)
			if(ppoint = PerpToSeg(p1->p,p2->p,relpoint,extend,look))
				theman->RecordHit(new OsnapHit(*ppoint, this, PERP_SUB, NULL));
//			AddCandidate(ppoint);
	}
	
	//if this guy is closed, check the closing segment
	if (!open)
		if (vis)
			if(ppoint = PerpToSeg(pline.pts->p,p1->p,relpoint))
				theman->RecordHit(new OsnapHit(*ppoint, this, PERP_SUB, NULL));
}

void ShapeSnap::PointsOnPolyLine_Tan(PolyLine& pline, const Point3& relpoint)
{
	Point3 *ppoint;
	PolyPt *p1 = pline.pts;
	PolyPt *p2 = p1 + 1;
	int segs = pline.Verts()-1;
//watje 3-7-99 fix to handle hidden spline segs
	BOOL vis = TRUE;

	for (int i=0; i<segs; ++i, p1++, p2++)
		{
		PolyPt ppt = pline.pts[i];
		if (ppt.flags & POLYPT_VISEDGE)
			vis = TRUE;
		else if (ppt.flags & POLYPT_INVIS_EDGE)
			vis = FALSE;
		if (vis)
			if(ppoint = TanToSeg(p1->p,p2->p,relpoint))
				theman->RecordHit(new OsnapHit(*ppoint, this, TAN_SUB, NULL));
		}
	
	if (pline.IsClosed())
		if (vis)
			if(ppoint = TanToSeg(pline.pts->p,p1->p,relpoint))
				theman->RecordHit(new OsnapHit(*ppoint, this, TAN_SUB, NULL));
}

void ShapeSnap::PointsOnPolyShape_Perp(const PolyShape& pshape, const Point3& relpoint)
{
	for(int i=0;i<pshape.numLines;++i)
		PointsOnPolyLine_Perp(pshape.lines[i], relpoint);
}

void ShapeSnap::PointsOnPolyShape_Tan(const PolyShape& pshape, const Point3& relpoint)
{
	for(int i=0;i<pshape.numLines;++i)
		PointsOnPolyLine_Tan(pshape.lines[i], relpoint);
}


void ShapeSnap::Snap(Object* pobj, IPoint2 *p, TimeValue t)
{	
	if(!theman->OKForRelativeSnap())
		return;
	Point3 relpoint(theman->GetRefPoint());//the last point the user clicked.
	int flags = 0;
	int savedLimits;

// relpoint should really be returned in the active node's space
	Matrix3 tm = theman->GetNode()->GetObjectTM(t);

//transform the relative point into the node's coordinate system
	relpoint = Inverse(tm) * relpoint;

//save the point that was passed in 
	Point2 fp = Point2((float)p->x, (float)p->y);
	IPoint2 ifp = IPoint2((int)p->x, (int)p->y);

//Check if we're even hitting the shape, if not return
	HitRegion hr;
	hr.type = POINT_RGN;
	hr.epsilon = theman->GetSnapStrength();
	hr.pt.x = p->x;
	hr.pt.y = p->y;

	ViewExp *vpt = theman->GetVpt();
	GraphicsWindow *gw = vpt->getGW();

	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->setHitRegion(&hr);
	gw->clearHitCode();

	if(0 == pobj->HitTest(t, theman->GetNode(), HITTYPE_POINT, TRUE, flags & HIT_ABORTONHIT, &ifp, vpt)) 
		return;

	//Now convert this into a splineshape 
//	SplineShape *splobj;
//	if(pobj->CanConvertToType(splineShapeClassID))
//		splobj = (SplineShape *)pobj->ConvertToType(t, splineShapeClassID);
//	else return;
//	assert(splobj);
	Point3Tab thepoints;

	if(	GetActive(PERP_SUB))
	{
		//Now compute the points on the splines and do the snapping	
		PolyShape pshp;
		((ShapeObject *)pobj)->MakePolyShape(t, pshp, -1, FALSE);
//		splobj->GetShape().MakePolyShape(pshp, -1, FALSE);
		PointsOnPolyShape_Perp(pshp, relpoint); 
	}

	if(	GetActive(TAN_SUB)) //Tangent snapping
	{
		//Now compute the points on the splines and do the snapping	
		PolyShape pshp;
		((ShapeObject *)pobj)->MakePolyShape(t, pshp, PSHAPE_BUILTIN_STEPS, FALSE);
//		splobj->GetShape().MakePolyShape(pshp, PSHAPE_BUILTIN_STEPS, FALSE);
		PointsOnPolyShape_Tan(pshp, relpoint); 
	}
//	if(splobj != pobj)
//		splobj->DeleteThis();
}



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

class ShapeClassDesc:public ClassDesc {
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
	void *			Create(BOOL loading = FALSE) {return new ShapeSnap();}
//	void *			Create(OsnapManager *pman) { return new ShapeSnap(pman); }
	// This is used for debugging purposes to give the class a 
	// displayable name.  It is also the name that appears on the button
	// in the MAX user interface.
	const TCHAR *	ClassName() { return _T("ShapeSnap"); }
	// The system calls this method at startup to determine the type of object
	// this is.  In our case, we're a geometric object so we return 
	// GEOMOBJECT_CLASS_ID.  The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return OSNAP_CLASS_ID; }
	// The system calls this method to retrieve the unique
	// class id for this object.
	Class_ID		ClassID() { 
		return SHAPE1_SNAP_CLASS_ID; }
	// The category is selected
	// in the bottom most drop down list in the create branch.
	// If this is set to be an exiting category (i.e. "Primatives", ...) then
	// the plug-in will appear in that category. If the category doesn't
	// yet exists then it is created.  We use the new How To category for
	// all the example plug-ins in the How To sections.
	const TCHAR* 	Category() { return _T(""); }
	};

// Declare a static instance of the class descriptor.
static ShapeClassDesc sampDesc;
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

