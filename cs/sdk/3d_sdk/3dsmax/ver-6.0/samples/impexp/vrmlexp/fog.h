/**********************************************************************
 *<
    FILE: fog.h
 
    DESCRIPTION:  VRML 2.0 Fog helper
 
    CREATED BY: Scott Morrison
 
    HISTORY: created 28 Aug. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __Fog__H__
 
#define __Fog__H__
 
#define Fog_CLASS_ID1 0xACAD3442
#define Fog_CLASS_ID2 0xF42DBAD

#define FogClassID Class_ID(Fog_CLASS_ID1, Fog_CLASS_ID2)

extern ClassDesc* GetFogDesc();

class FogCreateCallBack;

class FogObject: public HelperObject {			   
    friend class FogCreateCallBack;
    friend class FogObjPick;
    friend BOOL CALLBACK RollupDialogProc( HWND hDlg, UINT message,
                                           WPARAM wParam, LPARAM lParam );
    friend void BuildObjectList(FogObject *ob);

  public:

    // Class vars
    static HWND hRollup;
    static int dlgPrevSel;


    RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
                                PartID& partID, RefMessage message );
    float radius;
    static IObjParam *iObjParams;

    Mesh mesh;
    void BuildMesh(TimeValue t);

    IParamBlock *pblock;
    static IParamMap *pmapParam;

    FogObject();
    ~FogObject();


    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    // From BaseObject
    void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
    int HitTest(TimeValue t, INode* inode, int type, int crossing,
                int flags, IPoint2 *p, ViewExp *vpt);
    int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    TCHAR *GetObjectName() { return _T(GetString(IDS_FOG)); }


    // From Object
           ObjectState Eval(TimeValue time);
    void InitNodeName(TSTR& s) { s = _T(GetString(IDS_FOG)); }
    Interval ObjectValidity();
    Interval ObjectValidity(TimeValue time);
    int DoOwnSelectHilite() { return 1; }

    void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
    void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

    // Animatable methods
    void DeleteThis() { delete this; }
    Class_ID ClassID() { return Class_ID(Fog_CLASS_ID1,
                                         Fog_CLASS_ID2);}  
    void GetClassName(TSTR& s) { s = TSTR(_T(GetString(IDS_FOG_CLASS))); }
    int IsKeyable(){ return 1;}
    LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
                                       WPARAM wParam,   LPARAM lParam )
    { return 0; }


    int NumRefs() {return 1;}
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);

};				

#define PB_TYPE       0
#define PB_COLOR      1
#define PB_VIS_RANGE  2
#define PB_FOG_SIZE   3
#define PB_FOG_LENGTH 4

#endif

