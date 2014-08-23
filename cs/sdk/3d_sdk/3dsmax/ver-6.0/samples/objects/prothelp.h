/**********************************************************************
 *<
	FILE: prothelp.h

	DESCRIPTION:  Defines an Angle-Measuring Helper Class

	CREATED BY: Don Brittain

	HISTORY: created 12 June 1997

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __PROTHELP__H__ 

#define __PROTHELP__H__

class ProtHelpObject: public HelperObject 
{
	friend class ProtHelpObjCreateCallBack;
	friend INT_PTR CALLBACK ProtHelpParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	friend void resetProtParams();
public:

	// Class vars
	static Mesh mesh;
	static short meshBuilt;
	static HWND hProtHelpParams;
	static IObjParam *iObjParams;
	BOOL created;

	// Snap suspension flag (TRUE during creation only)
	BOOL suspendSnap;

	// Object parameters		
	short editting;
	double lastAngle;
	INode *refNode[2];

	Interval ivalid;

	//  inherited virtual methods for Reference-management
	RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message );
	void BuildMesh();
	void UpdateUI(TimeValue t);
	void GetMat(TimeValue t, INode* inod, ViewExp *vpt, Matrix3& mat);
	BOOL GetTargetPoint(int which, TimeValue t, Point3* q);
	int DrawLines(TimeValue t, INode *inode, GraphicsWindow *gw, int drawing );

	ProtHelpObject();
	~ProtHelpObject();

	//  inherited virtual methods:

	// From BaseObject
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	CreateMouseCallBack* GetCreateMouseCallBack();
	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	TCHAR *GetObjectName() { return GetString(IDS_DB_PROTRACTOR); }

	// From Object
	ObjectState Eval(TimeValue time);
	void InitNodeName(TSTR& s) { s = GetString(IDS_DB_PROTRACTOR); }
	Interval ObjectValidity(TimeValue time);
	void Invalidate();
	int DoOwnSelectHilite() { return 1; }

	// From GeomObject
	int IntersectRay(TimeValue t, Ray& r, float& at);
	void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
	void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
	void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );

	BOOL HasViewDependentBoundingBox() { return true; }

	// Animatable methods
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return Class_ID(PROTHELP_CLASS_ID,0); }  
	void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_DB_PROTHELPER_CLASS)); }

	TSTR SubAnimName(int i) { return TSTR(GetString(IDS_RB_PARAMETERS));}

	// From ref
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	int NumRefs();
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};				


#endif
