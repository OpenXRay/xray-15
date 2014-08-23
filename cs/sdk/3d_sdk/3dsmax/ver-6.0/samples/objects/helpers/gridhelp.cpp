/**********************************************************************
 *<
	FILE: gridhelp.cpp

	DESCRIPTION:  A grid helper implementation

	CREATED BY: Tom Hudson (based on Dan Silva's Object implementations)

	HISTORY: created 31 January 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "helpers.h"
#include "gridhelp.h"

// Parameter block indices
#define PB_LENGTH		0
#define PB_WIDTH		1
#define PB_GRID			2

#define PBLOCK_LENGTH 3

// xavier robitaille | 03.02.07 | fixed values for metric units
#ifndef METRIC_UNITS_FIXED_VALUES
#define GRIDHELP_DFLT_SPACING	10.0f
#else
#define GRIDHELP_DFLT_SPACING	 1.0f
#endif

//------------------------------------------------------

class GridHelpObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new GridHelpObject; }
	const TCHAR *	ClassName() { return GetString(IDS_DB_GRID_CLASS); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(GRIDHELP_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};

static GridHelpObjClassDesc gridHelpObjDesc;

ClassDesc* GetGridHelpDesc() { return &gridHelpObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for grid class.
GridHelpObject *GridHelpObject::editOb = NULL;
IParamMap *GridHelpObject::pmapParam = NULL;
IObjParam *GridHelpObject::iObjParams = NULL;
int GridHelpObject::dlgGridColor = GRID_COLOR_GRAY;

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),	
	
	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	
	
	// Grid
	ParamUIDesc(
		PB_GRID,
		EDITTYPE_UNIVERSE,
		IDC_GRID,IDC_GRIDSPINNER,
		BMIN_GRID,BMAX_GRID,
		SPIN_AUTOSCALE),	
	
	};
#define PARAMDESC_LENGTH 3

int MaxCoord(Point3 p)
{
	int best = 0;
	if(fabs(p.x)<fabs(p.y))
		best = 1;
	switch(best)
	{
	case 0:
		if(fabs(p.x)<fabs(p.z))
			best = 2;
		break;
	case 1:
		if(fabs(p.y)<fabs(p.z))
			best = 2;
		break;
	}
	return best;
}

int MostOrthogonalPlane( Matrix3& tmConst, ViewExp *vpt)
{
	int plane =  GRID_PLANE_TOP;

	Matrix3 tm,tmv, itmv;
	vpt->GetAffineTM(tm );  
	tmv = tmConst*tm;   // CP to view transform.
	itmv = Inverse(tmv);
	Point3 viewz = itmv.GetRow(2);
	switch(MaxCoord(viewz))
	{
		bool dir;
	case 0:
		dir = viewz.x>0.0f;
		plane = dir?GRID_PLANE_LEFT:GRID_PLANE_RIGHT;
		break;
	case 1:
		dir = viewz.y>0.0f;
		plane = dir?GRID_PLANE_BACK:GRID_PLANE_FRONT;
		break;
	case 2:
		dir = viewz.z<0.0f;
		plane = dir?GRID_PLANE_BOTTOM:GRID_PLANE_TOP;
		break;
	}

	return plane;
}

void GridHelpObject::FixConstructionTM(Matrix3 &tm, ViewExp *vpt)
{
	int plane = GRID_PLANE_TOP;

	if(vpt->GetGridType() < 0)
	{//JH 10/03/98 extending this to fix the construction plane to the most orthogonal plane
		//when the view is orthographic
#ifdef DESIGN_VER //JH 2/19/99 Removing this for shiva
		if( !(vpt->IsPerspView() || vpt->GetViewCamera() || (vpt->GetViewType() == VIEW_ISO_USER)))
		{
			plane = MostOrthogonalPlane(tm, vpt);
		}
		else
#endif
			plane = constPlane; //Original behavior for all non-grid views
	}
	else
		plane = vpt->GetGridType();

	if(plane == GRID_PLANE_BOTTOM)
		tm.PreRotateX(3.1415926f);	
	else if(plane == GRID_PLANE_RIGHT)
		tm.PreRotateY(3.1415926f/2.0f);
	else if(plane == GRID_PLANE_LEFT)
		tm.PreRotateY(-3.1415926f/2.0f);
	else if(plane == GRID_PLANE_FRONT)
		tm.PreRotateX(3.1415926f/2.0f);
	else if(plane == GRID_PLANE_BACK)
		tm.PreRotateX(-3.1415926f/2.0f);
}

class GridDlgProc : public ParamMapUserDlgProc {
	public:
		GridHelpObject *go;
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void SetObject(GridHelpObject *ob) { go = ob; }
		void DeleteThis() {}
	};

BOOL GridDlgProc::DlgProc( TimeValue t, IParamMap *map, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	switch ( message ) {
		case WM_INITDIALOG:
			CheckRadioButton(hDlg, IDC_GRID_GRAY_COLOR, IDC_GRID_HOME_INTENSITY, IDC_GRID_GRAY_COLOR+go->GetColor());
			CheckRadioButton(hDlg, IDC_GRID_XY_PLANE, IDC_GRID_ZX_PLANE, IDC_GRID_XY_PLANE+go->GetConstructionPlane() % 3);
			return FALSE;			

		case WM_COMMAND:			
			switch( LOWORD(wParam) ) {
				case IDC_GRID_GRAY_COLOR:
				case IDC_GRID_OBJECT_COLOR:
				case IDC_GRID_HOME_COLOR:
				case IDC_GRID_HOME_INTENSITY:
					go->SetColor(LOWORD(wParam) - IDC_GRID_GRAY_COLOR);
					go->iObjParams->RedrawViews(t,REDRAW_END);
					break;
				case IDC_GRID_XY_PLANE:
				case IDC_GRID_YZ_PLANE:
				case IDC_GRID_ZX_PLANE:
					go->SetConstructionPlane(LOWORD(wParam) - IDC_GRID_XY_PLANE);
					go->iObjParams->RedrawViews(t,REDRAW_END);
					break;
				}
			return FALSE;
		default:
			return FALSE;
		}
	}

GridDlgProc theGridProc;

void GridHelpObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
	{
	editOb = this;
	iObjParams = ip;
	if (pmapParam) {
		// Left over from last one created
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_GRIDPARAM),
			GetString(IDS_DB_PARAMETERS),
			0);
		}
	theGridProc.SetObject(this);
	pmapParam->SetUserDlgProc(&theGridProc);
	}
		
void GridHelpObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	editOb = NULL;
	if(IsDlgButtonChecked(pmapParam->GetHWnd(), IDC_GRID_HOME_INTENSITY ))
		dlgGridColor = GRID_COLOR_HOME_INT;
	else if(IsDlgButtonChecked(pmapParam->GetHWnd(), IDC_GRID_HOME_COLOR ))
		dlgGridColor = GRID_COLOR_HOME;
	else if(IsDlgButtonChecked(pmapParam->GetHWnd(), IDC_GRID_OBJECT_COLOR ))
		dlgGridColor = GRID_COLOR_OBJECT;
	else
		dlgGridColor = GRID_COLOR_GRAY;

	if (flags&END_EDIT_REMOVEUI ) {
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		}
	iObjParams = NULL;
	theGridProc.SetObject(NULL);
	}

void GridHelpObject::UpdateMesh(TimeValue t) {
	if ( ivalid.InInterval(t) )
		return;
	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float length, width;
	pblock->GetValue(PB_LENGTH, t, length, ivalid);
	pblock->GetValue(PB_WIDTH, t, width, ivalid);
	}

void GridHelpObject::UpdateUI(TimeValue t)
	{
	if ( editOb==this ) {
		CheckRadioButton(pmapParam->GetHWnd(), IDC_GRID_GRAY_COLOR, IDC_GRID_HOME_INTENSITY,
				IDC_GRID_GRAY_COLOR + GetColor());
		CheckRadioButton(pmapParam->GetHWnd(), IDC_GRID_XY_PLANE, IDC_GRID_ZX_PLANE,
				IDC_GRID_XY_PLANE + GetConstructionPlane() % 3);
		}
	}

ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_INT, NULL, TRUE, 9 },
	{ TYPE_INT, NULL, TRUE,10  },
	{ TYPE_FLOAT, NULL, TRUE,11 } };

ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 } };

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,12,0)	
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);

GridHelpObject::GridHelpObject() : ConstObject() 
	{
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue(PB_LENGTH,0,BDEF_DIM);
	pblock->SetValue(PB_WIDTH,0,BDEF_DIM);
	pblock->SetValue(PB_GRID,0,GRIDHELP_DFLT_SPACING);

	InvalidateGrid();
	myTM.IdentityMatrix();

	gridColor = dlgGridColor;
	constPlane = GRID_PLANE_TOP;
	}

GridHelpObject::~GridHelpObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	}


void GridHelpObject::SetGrid( TimeValue t, float len )
	{
	pblock->SetValue( PB_GRID, t, len );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void GridHelpObject::SetColor( int c )
	{
	gridColor = c % GRID_MAX_COLORS;
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void GridHelpObject::SetConstructionPlane( int p, BOOL notify )
	{
	constPlane = p;
	if(notify)
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float GridHelpObject::GetGrid( TimeValue t, Interval& valid )
	{
	float f;
	pblock->GetValue( PB_GRID, t, f, valid );
	return f;
	}

class GridHelpObjCreateCallBack: public CreateMouseCallBack {
	GridHelpObject *ob;
	Point3 p0,p1;
	IPoint2 sp1, sp0;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(GridHelpObject *obj) { ob = obj; }
	};

int GridHelpObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 d;

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
			case 0:
				sp0 = m;
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				p1 = p0 + Point3(.01,.01,0.0);
				mat.SetTrans(float(.5)*(p0+p1));				
				break;
			case 1:
				sp1 = m;
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif

				//JH 7/22/03 Fix for DID 474833
				//When snapping a grid taking the midpoint of the inputs is strange
				//instead, use the z value of p0
				//mat.SetTrans(float(.5)*(p0+p1));
				mat.SetTrans(float(.5)*(p0+Point3(p1.x, p1.y, p0.z)));
				d = p1-p0;
				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)) );
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)) );
				ob->pmapParam->Invalidate();
				if (msg==MOUSE_POINT) {
					if (Length(sp1-sp0) < 4) return CREATE_ABORT;
					else return CREATE_STOP;					
					}
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;

	return TRUE;
	}

static GridHelpObjCreateCallBack gridHelpCreateCB;

CreateMouseCallBack* GridHelpObject::GetCreateMouseCallBack() {
	gridHelpCreateCB.SetObj(this);
	return(&gridHelpCreateCB);
	}

	
void GridHelpObject::GetBBox(TimeValue t,  Matrix3& tm, Box3& box) {	
	float length, width;
	Point2 vert[2];
		
	pblock->GetValue(PB_LENGTH, t, length, FOREVER);
	pblock->GetValue(PB_WIDTH, t, width, FOREVER);

	vert[0].x = -width/float(2);
	vert[0].y = -length/float(2);
	vert[1].x = width/float(2);
	vert[1].y = length/float(2);

	box.Init();	
	box += tm * Point3( vert[0].x, vert[0].y, (float)0 );
	box += tm * Point3( vert[1].x, vert[0].y, (float)0 );
	box += tm * Point3( vert[0].x, vert[1].y, (float)0 );
	box += tm * Point3( vert[1].x, vert[1].y, (float)0 );
	box += tm * Point3( vert[0].x, vert[0].y, (float)0.1 );
	box += tm * Point3( vert[1].x, vert[0].y, (float)0.1 );
	box += tm * Point3( vert[0].x, vert[1].y, (float)0.1 );
	box += tm * Point3( vert[1].x, vert[1].y, (float)0.1 );
	}

void GridHelpObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box ) {
	Matrix3 tm = myTM;
	FixConstructionTM(tm, vpt);
	GetBBox(t,tm,box);
	}

void GridHelpObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
	{
	Matrix3 tm = myTM * (inode->GetObjectTM(t));
	FixConstructionTM(tm, vpt);
	GetBBox(t,tm,box);
	}

// Get the transform for this view
void GridHelpObject::GetConstructionTM( TimeValue t, INode* inode, ViewExp *vpt, Matrix3 &tm ) {
	tm = inode->GetObjectTM(t);
	FixConstructionTM(tm, vpt);
	}

// Get snap values
Point3 GridHelpObject::GetSnaps( TimeValue t ) {	
	float snap = GetGrid(t);
	return Point3(snap,snap,snap);
	}

void GridHelpObject::SetSnaps(TimeValue t, Point3 p)
{
	SetGrid(t, p.x);
}

Point3 GridHelpObject::GetExtents(TimeValue t)
{
	float x, y;
	pblock->GetValue(PB_LENGTH, t, x, FOREVER);
	pblock->GetValue(PB_WIDTH, t, y, FOREVER);
	return Point3(x, y, 0.0f);	
}
		
void GridHelpObject::SetExtents(TimeValue t, Point3 p)
{
	pblock->SetValue(PB_LENGTH, t, p.x);
	pblock->SetValue(PB_WIDTH, t, p.y);
}


int GridHelpObject::Select(TimeValue t, INode *inode, GraphicsWindow *gw, Material *mtl, HitRegion *hr, int abortOnHit ) {
	DWORD	savedLimits;
	Matrix3 tm;
	float width, w2, height, h2;
	pblock->GetValue(PB_LENGTH, t, height, FOREVER);
	pblock->GetValue(PB_WIDTH, t, width, FOREVER);
	if ( width==0 || height==0 )
		return 0;

	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->setHitRegion(hr);
	gw->clearHitCode();
	gw->setMaterial(*mtl);

	w2 = width / (float)2;
	h2 = height / (float)2;
		
	Point3 pt[3];

	if(!inode->IsActiveGrid()) {

		pt[0] = Point3( -w2, -h2,(float)0.0);
		pt[1] = Point3( -w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );

		if((hr->type != POINT_RGN) && !hr->crossing) {	// window select needs *every* face to be enclosed
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
				
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( w2, -h2,(float)0.0);
		pt[1] = Point3( w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( -w2, -h2,(float)0.0);
		pt[1] = Point3( w2, -h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( -w2, h2,(float)0.0);
		pt[1] = Point3( w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( -w2, (float)0, (float)0.0);
		pt[1] = Point3( w2, (float)0, (float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}

		pt[0] = Point3( (float)0, -h2,(float)0.0);
		pt[1] = Point3( (float)0, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
		if((hr->type != POINT_RGN) && !hr->crossing) {
			if(gw->checkHitCode())
				gw->clearHitCode();
			else
				return FALSE;
			}
			
		if ( abortOnHit ) {
			if(gw->checkHitCode()) {
				gw->setRndLimits(savedLimits);
				return TRUE;
				}
			}
		}
	else {
		float grid = GetGrid(t);
		int xSteps = (int)floor(w2 / grid);
		int ySteps = (int)floor(h2 / grid);
		float minX = (float)-xSteps * grid;
		float maxX = (float)xSteps * grid;
		float minY = (float)-ySteps * grid;
		float maxY = (float)ySteps * grid;
		float x,y;
		int ix;

		// Adjust steps for whole range
		xSteps *= 2;
		ySteps *= 2;

		// First, the vertical lines
		pt[0].y = minY;
		pt[0].z = (float)0;
		pt[1].y = maxY;
		pt[1].z = (float)0;

		for(ix=0,x=minX; ix<=xSteps; x+=grid,++ix) {
			pt[0].x = pt[1].x = x;
			gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
			if((hr->type != POINT_RGN) && !hr->crossing) {
				if(gw->checkHitCode())
					gw->clearHitCode();
				else
					return FALSE;
				}
			
			if ( abortOnHit ) {
				if(gw->checkHitCode()) {
					gw->setRndLimits(savedLimits);
					return TRUE;
					}
				}
   			}

		// Now, the horizontal lines
		pt[0].x = minX;
		pt[0].z = (float)0;
		pt[1].x = maxX;
		pt[1].z = (float)0;

		for(ix=0,y=minY; ix<=ySteps; y+=grid,++ix) {
			pt[0].y = pt[1].y = y;
			gw->polyline( 2, pt, NULL, NULL, FALSE, NULL );
		
			if((hr->type != POINT_RGN) && !hr->crossing) {
				if(gw->checkHitCode())
					gw->clearHitCode();
				else
					return FALSE;
				}
			
			if ( abortOnHit ) {
				if(gw->checkHitCode()) {
					gw->setRndLimits(savedLimits);
					return TRUE;
					}
				}
			}
		}

	if((hr->type != POINT_RGN) && !hr->crossing)
		return TRUE;
	return gw->checkHitCode();	
	}

// From BaseObject
int GridHelpObject::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) {
	Matrix3 tm;	
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();

   	tm = myTM * (inode->GetObjectTM(t));
	FixConstructionTM(tm, vpt);
	UpdateMesh(t);
	gw->setTransform(tm);

	MakeHitRegion(hitRegion, type, crossing, 4, p);
	return Select(t, inode, gw, mtl, &hitRegion, flags & HIT_ABORTONHIT );
	}

static float GridCoord(float v, float g) {
	float r = (float)(int((fabs(v)+0.5f*g)/g))*g;	
	return v<0.0f ? -r : r;
	}			

void GridHelpObject::Snap(TimeValue t, INode* inode, SnapInfo *info, IPoint2 *p, ViewExp *vpt) {
	Matrix3 invPlane = Inverse(info->plane);

	// If this isn't the current grid object, forget it!
	if(!inode->IsActiveGrid())
		return;

	Matrix3 tm = inode->GetObjectTM(t);
	FixConstructionTM(tm, vpt);
	GraphicsWindow *gw = vpt->getGW();	

	UpdateMesh(t);
	gw->setTransform(tm);

	Point2 fp = Point2((float)p->x, (float)p->y);

	// Don't bother snapping unless the grid intersection priority is at least as important as what we have so far
	if(info->gIntPriority > 0 && info->gIntPriority <= info->priority) {
		// Find where it lies on the plane
		Point3 local = vpt->GetPointOnCP(*p);
		// Get the grid size
		float grid = GetGrid(t);
		// Snap it to the grid
		Point3 snapped = Point3(GridCoord(local.x,grid),GridCoord(local.y,grid),0.0f);
		// If constrained to the plane, make sure this point is in it!
		if(info->snapType == SNAP_2D || info->flags & SNAP_IN_PLANE) {
			Point3 test = snapped * tm * invPlane;
			if(fabs(test.z) > 0.0001)	// Is it in the plane (within reason)?
				goto testLines;
			}
		// Now find its screen location...
		Point2 screen2;
		IPoint3 pt3;
		gw->wTransPoint(&snapped,&pt3);
		screen2.x = (float)pt3.x;
		screen2.y = (float)pt3.y;
		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);
		if(len <= info->strength) {
			// Is this priority better than the best so far?
			if(info->gIntPriority < info->priority) {
				info->priority = info->gIntPriority;
				info->bestWorld = snapped * tm;
				info->bestScreen = screen2;
				info->bestDist = len;
				}
			else
			if(len < info->bestDist) {
				info->priority = info->gIntPriority;
				info->bestWorld = snapped * tm;
				info->bestScreen = screen2;
				info->bestDist = len;
				}
			}
		}
	// Don't bother snapping unless the grid line priority is at least as important as what we have so far
	testLines:
	if(info->gLinePriority > 0 && info->gLinePriority <= info->priority) {
		// Find where it lies on the plane
		Point3 local = vpt->GetPointOnCP(*p);
		// Get the grid size
		float grid = GetGrid(t);
		// Snap it to the grid axes
		float xSnap = GridCoord(local.x,grid);
		float ySnap = GridCoord(local.y,grid);
		float xDist = (float)fabs(xSnap - local.x);
		float yDist = (float)fabs(ySnap - local.y);
		Point3 snapped;
		// Which one is closer?
		if(xDist < yDist)
			snapped = Point3(xSnap,local.y,0.0f);
		else
			snapped = Point3(local.x,ySnap,0.0f);
		// If constrained to the plane, make sure this point is in it!
		if(info->snapType == SNAP_2D || info->flags & SNAP_IN_PLANE) {
			Point3 test = snapped * tm * invPlane;
			if(fabs(test.z) > 0.0001)	// Is it in the plane (within reason)?
				return;
			}
		// Now find its screen location...
		Point2 screen2;
		IPoint3 pt3;
		gw->wTransPoint(&snapped,&pt3);
		screen2.x = (float)pt3.x;
		screen2.y = (float)pt3.y;
		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);
		if(len <= info->strength) {
			// Is this priority better than the best so far?
			if(info->gLinePriority < info->priority) {
				info->priority = info->gLinePriority;
				info->bestWorld = snapped * tm;
				info->bestScreen = screen2;
				info->bestDist = len;
				}
			else
			if(len < info->bestDist) {
				info->priority = info->gLinePriority;
				info->bestWorld = snapped * tm;
				info->bestScreen = screen2;
				info->bestDist = len;
				}
			}
		}		
	}

int GridHelpObject::IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm)
	{
	if (r.dir.z==0.0f) return FALSE;

	at = -r.p.z/r.dir.z;
	norm = Point3(0,0,1);
	return TRUE;
	}


// This (viewport intensity) should be a globally accessible variable!
#define VPT_INTENS ((float)0.62)

// SECSTART is the fraction of the viewport intensity where the secondary lines start
#define SECSTART ((float)0.75)
static int dotted_es[2] = {GW_EDGE_INVIS, GW_EDGE_INVIS};

int GridHelpObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) {

	Matrix3 tm;
	float width, w2, height, h2;
	pblock->GetValue(PB_LENGTH, t, height, FOREVER);
	pblock->GetValue(PB_WIDTH, t, width, FOREVER);
	if ( width==0 || height==0 )
		return 0;

	w2 = width / (float)2;
	h2 = height / (float)2;
		
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = gw->getMaterial();
	bool gw_dot_support = (gw->getRndLimits() & (GW_POLY_EDGES | GW_WIREFRAME))?true:false;
	int *es = (IsTransient() && gw_dot_support)?dotted_es:NULL;

   	tm = myTM * (inode->GetObjectTM(t));
	FixConstructionTM(tm, vpt);
	UpdateMesh(t);		
	gw->setTransform(tm);

	Point3 pt[3];

	float grid = GetGrid(t);
	int xSteps = (int)floor(w2 / grid);
	int ySteps = (int)floor(h2 / grid);

#if defined(GAME_VER) // SF 06.19.01: No 'bad grid' in GMAX
	// russom 07/24/01
	// increased badGrid threshold for gmax
	// BOOL badGrid = FALSE;
	BOOL badGrid = (xSteps > 5000 || ySteps > 5000) ? TRUE : FALSE;
#else
	BOOL badGrid = (xSteps > 200 || ySteps > 200) ? TRUE : FALSE;
#endif

	if(!inode->IsActiveGrid() || badGrid) {

#if defined(GAME_VER)
		// SF 06.19.01: Use wire color for in-active grid objects in GMAX
		gw->setColor(LINE_COLOR, Color(inode->GetWireColor()));
#else
		//The line color only gets set correctly when the display is in wireframe mode. Objects that always display as lines, must set the color prior to draw.
		//The color is picked up from the current material color.
		gw->setColor(LINE_COLOR, mtl->Kd[0], mtl->Kd[1], mtl->Kd[2]);
#endif

		pt[0] = Point3( -w2, -h2,(float)0.0);
		pt[1] = Point3( -w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, es );

		pt[0] = Point3( w2, -h2,(float)0.0);
		pt[1] = Point3( w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, es );

		pt[0] = Point3( -w2, -h2,(float)0.0);
		pt[1] = Point3( w2, -h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, es );

		pt[0] = Point3( -w2, h2,(float)0.0);
		pt[1] = Point3( w2, h2,(float)0.0);
		gw->polyline( 2, pt, NULL, NULL, FALSE, es );

		if(badGrid) {
			pt[0] = Point3( -w2, -h2, (float)0.0);
			pt[1] = Point3( w2, h2, (float)0.0);
			gw->polyline( 2, pt, NULL, NULL, FALSE, es );

			pt[0] = Point3( w2, -h2,(float)0.0);
			pt[1] = Point3( -w2, h2,(float)0.0);
			gw->polyline( 2, pt, NULL, NULL, FALSE, es );
			}
		else {
			pt[0] = Point3( -w2, (float)0, (float)0.0);
			pt[1] = Point3( w2, (float)0, (float)0.0);
			gw->polyline( 2, pt, NULL, NULL, FALSE, es );

			pt[0] = Point3( (float)0, -h2,(float)0.0);
			pt[1] = Point3( (float)0, h2,(float)0.0);
			gw->polyline( 2, pt, NULL, NULL, FALSE, es );
			}
		}
	else {	// Active grid representation
		float minX = (float)-xSteps * grid;
		float maxX = (float)xSteps * grid;
		float minY = (float)-ySteps * grid;
		float maxY = (float)ySteps * grid;
		float x,y;
		int ix;
		int selected = inode->Selected();
		float priBrite = VPT_INTENS * SECSTART;

		// Adjust steps for whole range
		xSteps *= 2;
		ySteps *= 2;

		// First, the vertical lines
		pt[0].y = minY;
		pt[0].z = (float)0;
		pt[1].y = maxY;
		pt[1].z = (float)0;

		Point3 dspClr1, dspClr2;
		DWORD rgb;
		switch(gridColor) {
		case GRID_COLOR_GRAY:
			dspClr2 = Point3(0,0,0);
			dspClr1 = Point3(priBrite, priBrite, priBrite);
			break;
		case GRID_COLOR_OBJECT:
			rgb = inode->GetWireColor();
			dspClr2 = Point3(GetRValue(rgb)/255.0f, GetGValue(rgb)/255.0f, GetBValue(rgb)/255.0f);
			dspClr1 = (Point3(1,1,1) + dspClr2) / 2.0f;
			break;
		case GRID_COLOR_HOME:
			dspClr2 = GetUIColor(COLOR_GRID);
			dspClr1 = (Point3(1,1,1) + dspClr2) / 2.0f;
			break;
		case GRID_COLOR_HOME_INT:
			dspClr1 = GetUIColor(COLOR_GRID_INTENS);
			if(dspClr1.x < 0.0f) {	// means "invert"
				dspClr1 = Point3(1,1,1) + dspClr1;
				dspClr2 = Point3(0.8f,0.8f,0.8f);
			}
			else
				dspClr2 = Point3(0,0,0);
			break;
		}

// SF 06.19.01: GMAX draws frozen grid objects with regular grid colors
#if defined(DESIGN_VER) || defined(GAME_VER)
		if(!selected)
#else
		if(!selected && !inode->IsFrozen() && !inode->Dependent())
#endif
//			gw->setColor( LINE_COLOR, priBrite, priBrite, priBrite );
			gw->setColor( LINE_COLOR, dspClr1 );

		for(ix=0,x=minX; ix<=xSteps; x+=grid,++ix) {
#if defined(GAME_VER) // dont overdraw center line
			if(x == 0.0f) continue;
#endif
			pt[0].x = pt[1].x = x;
			gw->polyline( 2, pt, NULL, NULL, FALSE, es );
   			}

		// Draw origin line if not selected
#if defined(DESIGN_VER) || defined(GAME_VER)
		if(!selected) {
#else
		if(!selected && !inode->IsFrozen() && !inode->Dependent()) {
#endif
//			gw->setColor( LINE_COLOR, (float)0, (float)0, (float)0 );
			gw->setColor( LINE_COLOR, dspClr2 );
			pt[0].x = pt[1].x = 0.0f;
			gw->polyline( 2, pt, NULL, NULL, FALSE, es );
			}

		// Now, the horizontal lines
		pt[0].x = minX;
		pt[0].z = 0.0f;
		pt[1].x = maxX;
		pt[1].z = 0.0f;

#if defined(GAME_VER)
		if(!selected)
#else
		if(!selected && !inode->IsFrozen() && !inode->Dependent())
#endif
			gw->setColor( LINE_COLOR, dspClr1 );

		for(ix=0,y=minY; ix<=ySteps; y+=grid,++ix) {
#if defined(GAME_VER) // dont overdraw center line
			if(y == 0.0f) continue;
#endif
			pt[0].y = pt[1].y = y;
			gw->polyline( 2, pt, NULL, NULL, FALSE, es );
			}

		// Draw origin line if not selected
#if defined(GAME_VER)
		if(!selected) {
#else
		if(!selected && !inode->IsFrozen() && !inode->Dependent()) {
#endif
			gw->setColor( LINE_COLOR, dspClr2 );
			pt[0].y = pt[1].y = 0.0f;
			gw->polyline( 2, pt, NULL, NULL, FALSE, es );
			}

		// Inform the viewport about the smallest grid scale
		vpt->SetGridSize(grid);
		}

	return(0);
	}

// From Object
ObjectHandle GridHelpObject::ApplyTransform(Matrix3& matrix){
	// RB	
	myTM = myTM * matrix;

	return(ObjectHandle(this));
	}

// From ConstObject

ObjectHandle GridHelpObject::CreateTriObjRep(TimeValue t) {
	return NULL;
	}

//
// Reference Managment:
//

// This is only called if the object MAKES references to other things.
RefResult GridHelpObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
     PartID& partID, RefMessage message ) 
    {
	switch (message) {
		case REFMSG_CHANGE:
			InvalidateGrid();
			if (editOb==this) InvalidateUI();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

ObjectState GridHelpObject::Eval(TimeValue time){
	return ObjectState(this);
	}

Interval GridHelpObject::ObjectValidity(TimeValue time) {
	UpdateMesh(time);
	UpdateUI(time);
	return ivalid;	
	}


int GridHelpObject::CanConvertToType(Class_ID obtype) {
	return 0;
	}

Object* GridHelpObject::ConvertToType(TimeValue t, Class_ID obtype) {
	return NULL;
	}

RefTargetHandle GridHelpObject::Clone(RemapDir& remap) {
	GridHelpObject* newob = new GridHelpObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->InvalidateGrid();
	newob->myTM = myTM;
	newob->gridColor = gridColor;
	newob->constPlane = constPlane;
	BaseClone(this, newob, remap);
	return(newob);
	}


#define TM_CHUNK	0x2100
#define COLOR_CHUNK	0x2110
#define PLANE_CHUNK	0x2120

// IO
IOResult GridHelpObject::Save(ISave *isave) {
	ULONG nb;
	isave->BeginChunk(TM_CHUNK);
	isave->Write(&myTM,sizeof(Matrix3), &nb);
	isave->EndChunk();

	isave->BeginChunk(COLOR_CHUNK);
	isave->Write(&gridColor, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(PLANE_CHUNK);
	isave->Write(&constPlane, sizeof(int), &nb);
	isave->EndChunk();
	return IO_OK;
	}

IOResult  GridHelpObject::Load(ILoad *iload) {
	ULONG nb;
	IOResult res;
	gridColor = GRID_COLOR_GRAY;
	constPlane = GRID_PLANE_TOP;
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case TM_CHUNK:
				res = iload->Read(&myTM,sizeof(Matrix3), &nb);
				break;
			case COLOR_CHUNK:
				res = iload->Read(&gridColor, sizeof(int), &nb);
				break;
			case PLANE_CHUNK:
				res = iload->Read(&constPlane, sizeof(int), &nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

ParamDimension *GridHelpObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH:
		case PB_WIDTH:
		case PB_GRID:
			return stdWorldDim;			
		default:
			return defaultDim;
		}
	}

TSTR GridHelpObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH:
			return TSTR(GetString(IDS_TH_LENGTH));
		case PB_WIDTH:
			return TSTR(GetString(IDS_TH_WIDTH));
		case PB_GRID:
			return TSTR(GetString(IDS_DB_GRID));
		default:
			return TSTR(_T(""));
		}
	}

Animatable* GridHelpObject::SubAnim(int i) {
	return pblock;
	}

TSTR GridHelpObject::SubAnimName(int i) {
	return TSTR(GetString(IDS_DB_PARAMETERS));
	}

