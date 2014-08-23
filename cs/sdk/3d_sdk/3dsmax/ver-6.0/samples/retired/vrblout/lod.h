/**********************************************************************
 *<
    FILE: lod.h
 
    DESCRIPTION:  Defines a Mr. Blue Helper Class
 
    CREATED BY: Charles Thaeler
 
    HISTORY: created 29 Feb. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __LOD__H__
 
#define __LOD__H__
 
#define LOD_CLASS_ID1 0x538c1715
#define LOD_CLASS_ID2 0x0

extern ClassDesc* GetLODDesc();

class LODCreateCallBack;
class LODObjPick;

class LODObj {
public:
	INode *node;
	TSTR listStr;
	float dist;
	void ResetStr(void) {
		if (node)
			listStr.printf("%s   %g", node->GetName(), dist);
		else listStr.printf("%s   %g", _T("NO_NAME"), dist);
	}
	void SetDist(float d) { dist = d; ResetStr(); }
	LODObj(INode *n = NULL, float d = 0.0f) {
		node = n;
		dist = d;
		ResetStr();
	}
};

class LODObject: public HelperObject {			   
	friend class LODCreateCallBack;
	friend class LODObjPick;
	friend INT_PTR CALLBACK RollupDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	friend void BuildObjectList(LODObject *ob);

	// Class vars
    static HWND hRollup;
	static ISpinnerControl *sizeSpin;
	static ISpinnerControl *distSpin;
	static ICustButton *lodPickButton;
	static int dlgPrevSel;


	RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message );
	float radius;
	float dlgCurSelDist;
	static IObjParam *iObjParams;

	Mesh mesh;
	void BuildMesh(void);
	void GetDistPoints(float radius, Point3 *x, Point3 *y, Point3 *z);
	void DrawDistSphere(TimeValue t, INode* inode, GraphicsWindow *gw);

	Tab<LODObj*> lodObjects;
	CommandMode *previousMode;

public:
	LODObject();
	~LODObject();


        RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From BaseObject
	void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
//	void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	CreateMouseCallBack* GetCreateMouseCallBack();
	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	TCHAR *GetObjectName() { return _T("LOD"); }


	void SetSize(float r);
	float GetSize(void) { return radius; }
	void SetCurDist(float d);
	float GetCurDist(void) { return dlgCurSelDist; }

	Tab<LODObj*> GetLODObjects() { return lodObjects; }

	// From Object
	ObjectState Eval(TimeValue time);
	void InitNodeName(TSTR& s) { s = _T("LOD"); }
	Interval ObjectValidity();
	Interval ObjectValidity(TimeValue time);
	int DoOwnSelectHilite() { return 1; }

	void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
	void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

	// Animatable methods
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return Class_ID(LOD_CLASS_ID1,
                                         LOD_CLASS_ID2);}  
	void GetClassName(TSTR& s) { s = TSTR(_T("LOD")); }
	int IsKeyable(){ return 1;}
	LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
            WPARAM wParam,   LPARAM lParam ){return(0);}


	int NumRefs() {return lodObjects.Count();}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};				


#endif

