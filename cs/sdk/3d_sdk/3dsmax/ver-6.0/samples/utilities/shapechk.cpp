/**********************************************************************
 *<
	FILE: shapechk.cpp

	DESCRIPTION:  A utility that checks Shape objects for validity

	CREATED BY: Tom Hudson

	HISTORY: created July 29, 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"
#include "polyshp.h"
#include "shape.h"

#ifndef NO_UTILITY_SHAPECHECK	// russom - 12/04/01

#define SHAPE_CHECK_CLASS_ID		0x7cc401b4

class ShapeCheck;

class SCDisplayCallback : public ViewportDisplayCallback {
	public:
		ShapeCheck *sc;
		void SetSC(ShapeCheck *s) { sc = s; }
		void Display(TimeValue t, ViewExp *vpt, int flags);
		void GetViewportRect( TimeValue t, ViewExp *vpt, Rect *rect );
		BOOL Foreground() { return FALSE; } // return TRUE if the object changes a lot or FALSE if it doesn't change much		
	};

class CSIntersection : public IntersectionCallback3D {
	public:
		Point3Tab table;
		void Reset() { table.Delete(0, table.Count()); table.Shrink(); }
		BOOL Intersect(Point3 p, int piece); // Return FALSE to stop intersect tests
		void Display(GraphicsWindow *gw);
		void GetBBox(Box3 &box);
	};

BOOL CSIntersection::Intersect(Point3 p, int piece) {
	table.Append(1, &p);
	return TRUE;
	}

void CSIntersection::Display(GraphicsWindow *gw) {
	gw->setColor(LINE_COLOR, 1.0f, 0.0f, 0.0f);
	for(int i = 0; i < table.Count(); ++i)
		gw->marker(&table[i], BIG_BOX_MRKR);
	}

void CSIntersection::GetBBox(Box3 &box) {
	box.Init();
	for(int i = 0; i < table.Count(); ++i)
		box += table[i];
	}

class ShapeCheck : public UtilityObj, public TimeChangeCallback {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		ICustButton *iPick;
		SCDisplayCallback dcb;
		INode *theNode;
		CSIntersection csi;
		Interval objValid;

		ShapeCheck();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		void SetNode(INode *node);
		BOOL CheckShape(TimeValue t);	// Returns TRUE if self-intersects
		// Time change callmack method
		void TimeChanged(TimeValue t);
	};

static ShapeCheck theShapeCheck;

class ShapeCheckClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theShapeCheck;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_SHAPE_CHECK);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(SHAPE_CHECK_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	};

static ShapeCheckClassDesc shapeCheckDesc;
ClassDesc* GetShapeCheckDesc() {return &shapeCheckDesc;}

void SCDisplayCallback::Display(TimeValue t, ViewExp *vpt, int flags) {
	if(!sc->theNode)
		return;
	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(sc->theNode->GetObjectTM(t));	
	sc->csi.Display(gw);
	}

void SCDisplayCallback::GetViewportRect( TimeValue t, ViewExp *vpt, Rect *rect ) {
	if(!sc->theNode)
		return;
	Box3 box;
	Matrix3 identTM(1);
	sc->csi.GetBBox(box);
	// Put box in world space
	Matrix3 mat = sc->theNode->GetObjectTM(t);
	box = box * mat;
	// Get a screen bound box
	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(identTM);		
	DWORD cf;
	DWORD orCf = 0;
	DWORD andCf = 0xffff;
	IPoint3 pt;
	rect->SetEmpty();
	for ( int i = 0; i < 8; i++ ) {
		cf = gw->wTransPoint( &box[i], &pt );
		orCf |= cf;
		andCf &= cf;
		*rect += IPoint2(pt.x,pt.y);
		}
	// If out of view frustrum, bail out
	if(andCf) {
		rect->SetEmpty();
		return;
		}
	// Grow the box to allow for markers
	rect->left   -= 8; 
	rect->top    -= 8; 
	rect->right  += 8; 
	rect->bottom += 8; 
	}

class ShapeCheckPickNodeCallback : public PickNodeCallback {
	public:		
		BOOL Filter(INode *node);
	};

BOOL ShapeCheckPickNodeCallback::Filter(INode *node)
	{
	ObjectState os = node->EvalWorldState(theShapeCheck.ip->GetTime());
	if (os.obj->SuperClassID()==SHAPE_CLASS_ID) return TRUE;
	else return FALSE;
	}

static ShapeCheckPickNodeCallback thePickFilt;

class ShapeCheckPickModeCallback : public PickModeCallback {
	public:		
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		
		void EnterMode(IObjParam *ip) {theShapeCheck.iPick->SetCheck(TRUE);}
		void ExitMode(IObjParam *ip) {theShapeCheck.iPick->SetCheck(FALSE);}

		PickNodeCallback *GetFilter() {return &thePickFilt;}
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
	};

static ShapeCheckPickModeCallback thePickMode;

BOOL ShapeCheckPickModeCallback::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{
	return ip->PickNode(hWnd,m,&thePickFilt)?TRUE:FALSE;
	}

BOOL ShapeCheckPickModeCallback::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) {
		theShapeCheck.SetNode(node);
		theShapeCheck.CheckShape(ip->GetTime());
		ip->ForceCompleteRedraw(FALSE);
		}
	return TRUE;
	}


static INT_PTR CALLBACK ShapeCheckDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theShapeCheck.Init(hWnd);			
			SetDlgItemText(hWnd, IDC_CHECK_MESSAGE, GetString(IDS_NO_SHAPE_SELECTED));
			break;
		
		case WM_DESTROY:
			theShapeCheck.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theShapeCheck.csi.Reset();
					theShapeCheck.theNode = NULL;
					theShapeCheck.ip->ForceCompleteRedraw(FALSE);
					theShapeCheck.iu->CloseUtility();
					break;				
		
				case IDC_SHAPECHECK_PICK:
					theShapeCheck.ip->SetPickMode(&thePickMode); 
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE; 
	}

ShapeCheck::ShapeCheck()
	{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;	
	iPick = NULL;
	theNode = NULL;
	objValid.SetEmpty();
	dcb.SetSC(this);
	}

void ShapeCheck::BeginEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_SHAPE_CHECK_PANEL),
		ShapeCheckDlgProc,
		GetString(IDS_RB_SHAPE_CHECK),
		0);
	ip->RegisterViewportDisplayCallback(FALSE, &dcb);
	ip->RegisterTimeChangeCallback(this);
	}
	
void ShapeCheck::EndEditParams(Interface *ip,IUtil *iu) 
	{
	if(theNode) {
		SetNode(NULL);
		ip->ForceCompleteRedraw(FALSE);
		}
	ip->UnRegisterTimeChangeCallback(this);
	ip->UnRegisterViewportDisplayCallback(FALSE, &dcb);
	ip->ClearPickMode();
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void ShapeCheck::Init(HWND hWnd)
	{
	iPick = GetICustButton(GetDlgItem(hWnd,IDC_SHAPECHECK_PICK));
	iPick->SetType(CBT_CHECK);
	iPick->SetHighlightColor(GREEN_WASH);
	}

void ShapeCheck::Destroy(HWND hWnd)
	{
	ReleaseICustButton(iPick);
	iPick = NULL;
	}

void ShapeCheck::SetNode(INode *node) {
	theNode = node;
	csi.Reset();
	objValid.SetEmpty();
	}

class NullView: public View {
	public:
		Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
		NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
	};

BOOL ShapeCheck::CheckShape(TimeValue t)
	{
	BOOL intersects = FALSE;
	if(!theNode)
		return FALSE;
	if(objValid.InInterval(t))
		return FALSE;
	ObjectState os = theNode->EvalWorldState(t);
	if(os.obj->SuperClassID()==SHAPE_CLASS_ID) {
		Matrix3 tm = theNode->GetObjTMAfterWSM(t);
		
		PolyShape shape;
		((ShapeObject *)os.obj)->MakePolyShape(t, shape);
		objValid = os.Validity(t);
		BOOL si = FALSE;
		// Flush the intersection counter
		csi.Reset();
		// Check each polyline for self-intersection
		for(int i = 0; i < shape.numLines; ++i) {
			if(shape.lines[i].SelfIntersects(TRUE, &csi))
				si = TRUE;
			}
		// Now check them against each other
		for(i = 0; i < shape.numLines; ++i) {
			for(int j = i+1; j < shape.numLines; ++j) {
				if(i != j)
					if(shape.lines[i].HitsPolyLine(shape.lines[j],TRUE, &csi))
						si = TRUE;
				}
			}
		TSTR buf;
		if(si) {
			buf = GetString(IDS_SHAPE_INTERSECTS);
			ip->NotifyViewportDisplayCallbackChanged(FALSE,&dcb);
			intersects = TRUE;
			}
		else
			buf = GetString(IDS_SHAPE_OK);

		SetDlgItemText(hPanel, IDC_CHECK_MESSAGE, buf);
		}
	else
		assert(0);
	return intersects;
	}

void ShapeCheck::TimeChanged(TimeValue t) {
	if(theNode) {
		CheckShape(t);
		ip->ForceCompleteRedraw(FALSE);
		}
	}

#endif // NO_UTILITY_SHAPECHECK
