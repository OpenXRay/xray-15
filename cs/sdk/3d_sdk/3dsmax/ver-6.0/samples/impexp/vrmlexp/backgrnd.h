/**********************************************************************
 *<
    FILE: backgrnd.h
 
    DESCRIPTION:  Defines a VRML 2.0 Background helper object
 
    CREATED BY: Scott Morrison
 
    HISTORY: created 29 Aug. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __Background__H__
 
#define __Background__H__
 
#define Background_CLASS_ID1 0xAC5D3442
#define Background_CLASS_ID2 0xFBBDBAD

#define BackgroundClassID Class_ID(Background_CLASS_ID1, Background_CLASS_ID2)

extern ClassDesc* GetBackgroundDesc();

class BackgroundCreateCallBack;

class BackgroundObject: public HelperObject {			   
    friend class BackgroundCreateCallBack;
    friend class BackgroundObjPick;
    friend BOOL CALLBACK RollupDialogProc( HWND hDlg, UINT message,
                                           WPARAM wParam, LPARAM lParam );
    friend void BuildObjectList(BackgroundObject *ob);

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

    HWND imageDlg;
    IParamBlock *pblock;
    static IParamMap *skyParam;
    static IParamMap *groundParam;
    TSTR back, bottom, front, left, right, top;

    BackgroundObject();
    ~BackgroundObject();


    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    // From BaseObject
    void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
    int HitTest(TimeValue t, INode* inode, int type, int crossing,
                int flags, IPoint2 *p, ViewExp *vpt);
    int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    TCHAR *GetObjectName() { return _T(GetString(IDS_BACKGROUND)); }


    // From Object
           ObjectState Eval(TimeValue time);
    void InitNodeName(TSTR& s) { s = _T(GetString(IDS_BACKGROUND)); }
    Interval ObjectValidity();
    Interval ObjectValidity(TimeValue time);
    int DoOwnSelectHilite() { return 1; }

    void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
    void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

    // Animatable methods
    void DeleteThis() { delete this; }
    Class_ID ClassID() { return Class_ID(Background_CLASS_ID1,
                                         Background_CLASS_ID2);}  
    void GetClassName(TSTR& s) { s = TSTR(_T(GetString(IDS_BACKGROUND_CLASS))); }
    int IsKeyable(){ return 1;}
    LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
                                       WPARAM wParam,   LPARAM lParam )
    { return 0; }


    int NumRefs() {return 1;}
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);

    // IO
    IOResult Save(ISave *isave);
    IOResult Load(ILoad *iload);
};				

#define PB_SKY_NUM_COLORS      0
#define PB_SKY_COLOR1          1
#define PB_SKY_COLOR2          2
#define PB_SKY_COLOR2_ANGLE    3
#define PB_SKY_COLOR3          4
#define PB_SKY_COLOR3_ANGLE    5
#define PB_GROUND_NUM_COLORS   6
#define PB_GROUND_COLOR1       7
#define PB_GROUND_COLOR2       8
#define PB_GROUND_COLOR2_ANGLE 9
#define PB_GROUND_COLOR3       10
#define PB_GROUND_COLOR3_ANGLE 11
#define PB_BG_SIZE             12
#define PB_BG_LENGTH           13

#endif

