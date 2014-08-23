/**********************************************************************
 *<
	FILE: gridhelp.h

	DESCRIPTION:  Defines a Grid Helper Object Class

	CREATED BY: Tom Hudson

	HISTORY: created 31 January 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __GRIDHELP__H__ 

#define __GRIDHELP__H__

#include "iparamm.h"

#ifndef MAX_FLOAT
#define MAX_FLOAT ((float)1.0e20)
#endif

class GridHelpObjCreateCallBack;
class VIZGridHelpObjCreateCallback;

#define BMIN_LENGTH		float(0)
#define BMAX_LENGTH		MAX_FLOAT
#define BMIN_WIDTH		float(0)
#define BMAX_WIDTH		MAX_FLOAT
#define BMIN_GRID		float(0)
#define BMAX_GRID		MAX_FLOAT

#define BDEF_DIM		float(0)

#define GRID_COLOR_GRAY		0
#define GRID_COLOR_OBJECT	1
#define GRID_COLOR_HOME		2
#define GRID_COLOR_HOME_INT	3
#define GRID_MAX_COLORS		4


class GridHelpObject: public ConstObject, public IParamArray {			   
	public:
		// Object parameters		
		IParamBlock *pblock;
		int gridColor;
		int constPlane;

		// Offset
		Matrix3	myTM;

		Interval ivalid;

		// Class vars
		static GridHelpObject *editOb;
		static IParamMap *pmapParam;
		static IObjParam *iObjParams;
		static int dlgGridColor;
	
		// From BaseObject
		TCHAR* GetObjectName() { return GetString(IDS_DB_GRID_OBJECT); }
		
	//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );
		void BuildMesh(TimeValue t,Mesh& amesh);
		void UpdateMesh(TimeValue t);
		void UpdateUI(TimeValue t);
		void GetBBox(TimeValue t, Matrix3 &tm, Box3& box);
		int Select(TimeValue t, INode *inode, GraphicsWindow *gw, Material *ma, HitRegion *hr, int abortOnHit );
		void FixConstructionTM(Matrix3 &tm, ViewExp *vpt);

		GridHelpObject();
		~GridHelpObject();

		void SetGrid( TimeValue t,float grid );
		float GetGrid( TimeValue t, Interval& valid = Interval(0,0) );

		void SetColor(int c);
		int GetColor(void)	{ return gridColor; }
		
		void InvalidateGrid() { ivalid.SetEmpty(); }

		//  inherited virtual methods:

		// From ConstObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// From Object
		ObjectState Eval(TimeValue time);
		void InitNodeName(TSTR& s) { s = TSTR(GetString(IDS_DB_GRID_NODENAME)); }
		int UsesWireColor() { return 1; }
		ObjectHandle ApplyTransform(Matrix3& matrix);
		Interval ObjectValidity(TimeValue t);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		int IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm);

		// From HelperObject
		ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

		// From ConstObject
		void GetConstructionTM( TimeValue t, INode* inode, ViewExp *vpt, Matrix3 &tm );	// Get the transform for this view
		void SetConstructionPlane(int p, BOOL notify=TRUE);
		int  GetConstructionPlane(void)			{ return constPlane; }
		Point3 GetSnaps( TimeValue t );		// Get snaps
		void SetSnaps(TimeValue t, Point3 p);
		Point3 GetExtents(TimeValue t);
		void SetExtents(TimeValue t, Point3 p);

		// Animatable methods
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID(GRIDHELP_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_DB_GRIDHELPER)); }
		int IsKeyable(){ return 1;}
		LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
	            WPARAM wParam,   LPARAM lParam ){return(0);}

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}

 		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);		

		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		void InvalidateUI() { if (pmapParam) pmapParam->Invalidate(); }
	};				

// Here's an alternative construction grid to be used by VIZ
class VIZGridHelpObject: public ConstObject, public IParamArray {
	//factory method
	static VIZGridHelpObject* CreateImplicitGridObject();
private:
	bool m_implicit;
	public:
		// Object parameters		
		IParamBlock *pblock;
		int gridColor;
		int constPlane;

		// Offset
		Matrix3	myTM;

		Interval ivalid;

		// Class vars
		static VIZGridHelpObject *editOb;
		static IParamMap *pmapParam;
		static IObjParam *iObjParams;
		static int dlgGridColor;
		
		// From BaseObject
		TCHAR* GetObjectName() { return GetString(IDS_DB_GRID_OBJECT); }

		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );
		void BuildMesh(TimeValue t,Mesh& amesh);
		void UpdateMesh(TimeValue t);
		void UpdateUI(TimeValue t);
		void GetBBox(TimeValue t, Matrix3 &tm, Box3& box);
		int Select(TimeValue t, INode *inode, GraphicsWindow *gw, Material *ma, HitRegion *hr, int abortOnHit );
		void FixConstructionTM(Matrix3 &tm, ViewExp *vpt);

		VIZGridHelpObject();
		~VIZGridHelpObject();

		void SetGrid( TimeValue t,float grid );
		float GetGrid( TimeValue t, Interval& valid = Interval(0,0) );

		void SetColor(int c);
		int GetColor(void)	{ return gridColor; }
		
		void InvalidateGrid() { ivalid.SetEmpty(); }

		//  inherited virtual methods:

		// From ConstObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// From Object
		ObjectState Eval(TimeValue time);
		void InitNodeName(TSTR& s) { s = TSTR(GetString(IDS_DB_GRID_NODENAME)); }
		int UsesWireColor() { return 1; }
		ObjectHandle ApplyTransform(Matrix3& matrix);
		Interval ObjectValidity(TimeValue t);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		int IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm);

		// From HelperObject
		ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

		// From ConstObject
		void GetConstructionTM( TimeValue t, INode* inode, ViewExp *vpt, Matrix3 &tm );	// Get the transform for this view
		void SetConstructionPlane(int p, BOOL notify=TRUE);
		int  GetConstructionPlane(void)			{ return constPlane; }
		Point3 GetSnaps( TimeValue t );		// Get snaps
		void SetSnaps(TimeValue t, Point3 p);
		Point3 GetExtents(TimeValue t);
		void SetExtents(TimeValue t, Point3 p);

		// Animatable methods
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID(GRIDHELP_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_DB_GRIDHELPER)); }
		int IsKeyable(){ return 1;}
		LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
	            WPARAM wParam,   LPARAM lParam ){return(0);}

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}

 		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);		

		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		void InvalidateUI() { if (pmapParam) pmapParam->Invalidate(); }


	};				

#endif // __GRIDHELP__H__
