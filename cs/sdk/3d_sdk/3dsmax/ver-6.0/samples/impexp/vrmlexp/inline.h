/**********************************************************************
 *<
    FILE: vrml_ins.h
 
    DESCRIPTION:  Defines a VRML Insert Helper Class
 
    CREATED BY: Charles Thaeler
 
    HISTORY: created 6 Feb. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __VRML_INS__H__
 
#define __VRML_INS__H__
 
#define VRML_INS_CLASS_ID1 0xACAD4567
#define VRML_INS_CLASS_ID2 0x0

extern ClassDesc* GetVRMLInsDesc();

class VRMLInsCreateCallBack;

class VRMLInsObject: public HelperObject {			   
    friend class VRMLInsCreateCallBack;
    friend BOOL CALLBACK VRMLInsRollupDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

    // Class vars
    static HWND hRollup;
    static ISpinnerControl *sizeSpin;


    RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
                                PartID& partID, RefMessage message );
    float radius;
    static IObjParam *iObjParams;

    Mesh mesh;
    void MakeQuad(int *f, int a, int b, int c, int d, int vab, int vbc, int vcd, int vda);
    void BuildMesh(void);
    TSTR insURL;
    BOOL useSize;

public:
    VRMLInsObject();
    ~VRMLInsObject();


    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    // From BaseObject
    void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
    int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
    //	void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
    int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    TCHAR *GetObjectName() { return _T(GetString(IDS_INLINE)); }


    void SetSize(float r);
    float GetSize(void) { return radius; }
    void SetUseSize(int v) { useSize = v; }
    BOOL GetUseSize(void) { return useSize; }

    TSTR& GetUrl(void) { return insURL; }

    // From Object
    ObjectState Eval(TimeValue time);
    void InitNodeName(TSTR& s) { s = _T(GetString(IDS_INLINE)); }
    Interval ObjectValidity();
    Interval ObjectValidity(TimeValue time);
    int DoOwnSelectHilite() { return 1; }

    void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
    void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

    // Animatable methods
    void DeleteThis() { delete this; }
    Class_ID ClassID() { return Class_ID(VRML_INS_CLASS_ID1,
                                         VRML_INS_CLASS_ID2);}  
    void GetClassName(TSTR& s) { s = TSTR(_T(GetString(IDS_INLINE_CLASS))); }
    int IsKeyable(){ return 1;}
    LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
                                       WPARAM wParam,   LPARAM lParam ){return(0);}

    // IO
    IOResult Save(ISave *isave);
    IOResult Load(ILoad *iload);
};				


#endif

