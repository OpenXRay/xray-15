/**********************************************************************
 *<
    FILE: mr_blue.h
 
    DESCRIPTION:  Defines a Mr. Blue Helper Class
 
    CREATED BY: Charles Thaeler
 
    BASED ON: tapehelp.h
 
    HISTORY: created 13 Feb. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __MR_BLUE__H__
 
#define __MR_BLUE__H__
 
#define MR_BLUE_CLASS_ID1 0xa187ac4
#define MR_BLUE_CLASS_ID2 0x0

typedef enum VRBL_Action {
    MrBlueMessage,
    HyperLinkJump,
    SetViewpoint,
    Animate
};

typedef enum VRBL_TriggerType {
    MouseClick,
    DistProximity,
    BoundingBox,
    LineOfSight
};

typedef enum VRBL_LOS_Type {
    CanSee,
    CanSeeAndSeen
};

typedef enum AnimToggle {
    Start,
    Stop
};

class MrBlueAnimObj {
  public:
    INode *node;
    TSTR listStr;
    void ResetStr(void) {
        if (node)
            listStr.printf("%s", node->GetName());
        else listStr.printf("%s ", _T("NO_NAME"));
    }
    MrBlueAnimObj(INode *no = NULL ) {
        node = no;
        ResetStr();
    }
};

#define MIN_LOS_DIST	0.0f
#define MAX_LOS_DIST	99999.0f

#define MIN_PROX_DIST	0.0001f
#define MAX_PROX_DIST	99999.0f

#define MIN_LOS_VPT_ANG 0.0f
#define MAX_LOS_VPT_ANG 90.0f

#define MIN_LOS_OBJ_ANG 0.0f
#define MAX_LOS_OBJ_ANG 180.0f

extern ClassDesc* GetMrBlueDesc();

class MrBlueCreateCallBack;
class AnimObjPick;

class MrBlueObject: public HelperObject {			   
    friend class MrBlueObjCreateCallBack;
    friend class AnimObjPick;
    friend INT_PTR CALLBACK MrBlueActionDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
    friend INT_PTR CALLBACK MrBlueTriggerDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
    friend INT_PTR CALLBACK MrBlueHLParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
    friend INT_PTR CALLBACK MrBlueVPTParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
    friend INT_PTR CALLBACK MrBlueAnimParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
    
    // Class vars
           static Mesh mesh;
    static short meshBuilt;
    static HWND hMrBlueActions;
    static HWND hMrBlueTriggers;
    static HWND hMrBlueParamsHL;
    static HWND hMrBlueParamsVPT;
    static HWND hMrBlueParamsAnim;
    static IObjParam *iObjParams;
    
    static ISpinnerControl *proxDistSpin;
    static ISpinnerControl *bboxXSpin;
    static ISpinnerControl *bboxYSpin;
    static ISpinnerControl *bboxZSpin;
    static ISpinnerControl *losVptAngSpin;
    static ISpinnerControl *losObjAngSpin;
    static ICustButton *animPickButton;
    
    
    // Object parameters		
           short enable;
    
    VRBL_Action action;
    short mouseEnabled;
    short proxDistEnabled;
    short bboxEnabled;
    short losEnabled;
    float proxDist;
    float bboxX;
    float bboxY;
    float bboxZ;
    VRBL_LOS_Type losType;
    float losVptAngle;
    float losObjAngle;
    MrBlueAnimObj curAnimObj;
    TSTR hlURL;
    TSTR hlCamera;
    TSTR hlDesc;
    INode *vptCamera;
    TSTR vptDesc;
    
    Tab<MrBlueAnimObj*> animObjects;
    CommandMode *previousMode;
    AnimToggle animState;
    Interval ivalid;
    
    //  inherited virtual methods for Reference-management
                                                   RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
                                                                               PartID& partID, RefMessage message );
    void BuildMesh();
    void UpdateUI(TimeValue t);
    void GetMat(TimeValue t, INode* inod, ViewExp *vpt, Matrix3& mat);
    void GetLinePoints(TimeValue t, Point3* q, float len);
    
    float GetTDist(Matrix3 tm, Point3 pt);
    void GetProxDistPoints(float radius, Point3 *x, Point3 *y, Point3 *z);
    void GetLosVptConePoints(float dist, Point3 *v);
    void GetLosObjConePoints(float dist, Point3 *v);
    void DrawLine(TimeValue t, INode* inode, GraphicsWindow *gw);
    void DrawProxDist(TimeValue t, INode* inode, GraphicsWindow *gw);
    void DrawBBox(TimeValue t, INode* inode, GraphicsWindow *gw);
    void DrawLosObjCone(TimeValue t, INode* inode, GraphicsWindow *gw);
    void DrawLosVptCone(TimeValue t, INode* inode, GraphicsWindow *gw);
    
    VRBL_Action GetDlgAction();
    void SetRollUps(IObjParam *ip);
    
  public:
    MrBlueObject();
    ~MrBlueObject();
    
    static VRBL_Action dlgAction;
    static BOOL dlgMouseEnabled;
    static BOOL dlgProxDistEnabled;
    static BOOL dlgBBoxEnabled;
    static BOOL dlgLosEnabled;
    static float dlgProxDist;
    static float dlgBBoxX;
    static float dlgBBoxY;
    static float dlgBBoxZ;
    static VRBL_LOS_Type dlgLosType;
    static float dlgLosVptAngle;
    static float dlgLosObjAngle;
    static int dlgAnimPrevSel;
    static AnimToggle dlgAnimState;
    
    
    void Enable(int enab) { enable = enab; }
    
    void SetAction(VRBL_Action act);
    VRBL_Action GetAction() { return action; }
    
    void SetMouseEnabled(int onOff);
    int GetMouseEnabled(void)	{ return mouseEnabled; }
    
    void SetProxDistEnabled(int onOff);
    int GetProxDistEnabled(void)	{ return proxDistEnabled; }
    void SetProxDist(float dist);
    float GetProxDist(void) { return proxDist; }
    void SetBBoxEnabled(int onOff);
    int GetBBoxEnabled(void)	{ return bboxEnabled; }
    void SetBBoxX(float dist);
    float GetBBoxX(void) { return bboxX; }
    void SetBBoxY(float dist);
    float GetBBoxY(void) { return bboxY; }
    void SetBBoxZ(float dist);
    float GetBBoxZ(void) { return bboxZ; }
    
    void SetLosEnabled(int onOff);
    int GetLosEnabled(void)	{ return losEnabled; }
    void SetLosType(VRBL_LOS_Type type);
    VRBL_LOS_Type GetLosType(void)	{ return losType; }
    void SetLosVptAngle(float ang);
    float GetLosVptAngle(void) { return losVptAngle; }
    void SetLosObjAngle(float ang);
    float GetLosObjAngle(void) { return losObjAngle; }
    void SetAnimState(AnimToggle state);
    
    
    // For VRBL out's access
    TSTR GetURL(void) { return hlURL;}
    TSTR GetCamera(void) { return hlCamera; }
    TSTR GetDesc(void) { return hlDesc; }
    INode *GetVptCamera(void) { return vptCamera; }
    TSTR GetVptDesc(void) { return vptDesc; }
    AnimToggle GetAnimState(void) { return animState; }
    Tab<MrBlueAnimObj*> *GetAnimObjects(void) { return &animObjects; }
    
    
    //  inherited virtual methods:
    
    // From BaseObject
    int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
    void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
    int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    TCHAR *GetObjectName() { return _T("VRML"); }
    
    // From Object
    ObjectState Eval(TimeValue time);
    void InitNodeName(TSTR& s) { s = _T("VRML"); }
    Interval ObjectValidity();
    Interval ObjectValidity(TimeValue time);
    int DoOwnSelectHilite() { return 1; }
    
    // From GeomObject
    int IntersectRay(TimeValue t, Ray& r, float& at);
    void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
    void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
    void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
    
    // Animatable methods
    void DeleteThis() { delete this; }
    Class_ID ClassID() { return Class_ID(MR_BLUE_CLASS_ID1,
                                         MR_BLUE_CLASS_ID2);}  
    void GetClassName(TSTR& s) { s = TSTR(_T("VRML")); }
    int IsKeyable(){ return 1;}
    LRESULT CALLBACK TrackViewWinProc(HWND hwnd, UINT message, WPARAM wParam,
                                      LPARAM lParam ) { return 0;}
    // From ref
    RefTargetHandle Clone(RemapDir& remap = NoRemap());
    int NumRefs() {return (animObjects.Count() + 1);}
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);
    
    // IO
    IOResult Save(ISave *isave);
    IOResult Load(ILoad *iload);
};				


#endif
