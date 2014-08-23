/**********************************************************************
 *<
	FILE: tapehelp.h

	DESCRIPTION:  Defines a Measuring Tape Helper Class

	CREATED BY: Don Brittain

	HISTORY: created 8 October 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __TAPEHELP__H__ 

#define __TAPEHELP__H__

#include <float.h>
#define MIN_TAPE_LEN	0.0f
#define MAX_TAPE_LEN	999999.0f
// mjm - 1.19.99 - changed to address a complaint that the ui clamp on length values was too low.
// how high is high enough? -- any higher begin to risk float overflow in code areas such as scene rendering
//#define MAX_TAPE_LEN	99999.0f


class TapeHelpCreateCallBack;

class TapeHelpObject: public HelperObject 
{
	friend class TapeHelpObjCreateCallBack;
	friend INT_PTR CALLBACK TapeHelpParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	friend void resetTapeParams();
	
	// Class vars
	static Mesh mesh;
	static short meshBuilt;
	static HWND hTapeHelpParams;
	static IObjParam *iObjParams;
	static ISpinnerControl *lengthSpin;
	static float dlgLength;
	static short dlgSpecLen;

	// Object parameters		
	IParamBlock *pblock;
	short enable;
	short editting;
	short specLenState;
	float lastDist;
	Point3 dirPt;

	Interval ivalid;

	//  inherited virtual methods for Reference-management
	RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message );
	void BuildMesh();
	void UpdateUI(TimeValue t);
	void GetMat(TimeValue t, INode* inod, ViewExp *vpt, Matrix3& mat);
	void GetLinePoints(TimeValue t, Point3* q, float len);
	int DrawLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing );

public:
	TapeHelpObject();
	~TapeHelpObject();

	void SetLength( TimeValue t,float len );
	float GetLength( TimeValue t, Interval& valid = Interval(0,0) );
	void SetSpecLen(int onOff);
	int GetSpecLen(void)	{ return specLenState; }
	void Enable(int enab) { enable = enab; }

	//  inherited virtual methods:

	// From BaseObject
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	CreateMouseCallBack* GetCreateMouseCallBack();
	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	TCHAR *GetObjectName() { return GetString(IDS_DB_TAPE); }

	// From Object
	ObjectState Eval(TimeValue time);
	void InitNodeName(TSTR& s) { s = GetString(IDS_DB_TAPE); }
	Interval ObjectValidity();
	Interval ObjectValidity(TimeValue time);
	int DoOwnSelectHilite() { return 1; }

	// From GeomObject
	int IntersectRay(TimeValue t, Ray& r, float& at);
	void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
	void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
	void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );

	BOOL HasViewDependentBoundingBox() { return true; }

	// Animatable methods
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return Class_ID(TAPEHELP_CLASS_ID,0); }  
	void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_DB_TAPEHELPER_CLASS)); }

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i) { return pblock; }
	TSTR SubAnimName(int i) { return TSTR(GetString(IDS_RB_PARAMETERS));}

	// From ref
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i) {return pblock;}
	void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
};				


#endif
