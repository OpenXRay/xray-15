/**********************************************************************
 *<
    FILE: touch.h
 
    DESCRIPTION:  Defines a VRML 2.0 TouchSensor helper
 
    CREATED BY: Scott Morrison
 
    HISTORY: created 4 Sept. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __TouchSensor__H__
 
#define __TouchSensor__H__
 
#define TouchSensor_CLASS_ID1 0x73fa3442
#define TouchSensor_CLASS_ID2 0xF002BDAD

#define TouchSensorClassID Class_ID(TouchSensor_CLASS_ID1, TouchSensor_CLASS_ID2)

extern ClassDesc* GetTouchSensorDesc();

class TouchSensorCreateCallBack;
class TouchSensorObjPick;

class TouchSensorObj {
  public:
    INode *node;
    TSTR listStr;
    void ResetStr(void) {
        if (node)
            listStr.printf("%s", node->GetName());
        else listStr.printf("%s", _T("NO_NAME"));
    }
    TouchSensorObj(INode *n = NULL) {
        node = n;
        ResetStr();
    }
};

class TouchSensorObject: public HelperObject {			   
    friend class TouchSensorCreateCallBack;
    friend class TouchSensorObjPick;
    friend BOOL CALLBACK RollupDialogProc( HWND hDlg, UINT message,
                                           WPARAM wParam, LPARAM lParam );
    friend void BuildObjectList(TouchSensorObject *ob);

  public:

    // Class vars
    static HWND hRollup;
    static int dlgPrevSel;
    BOOL needsScript;  // Do we need to generate a script node?

    RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
                                PartID& partID, RefMessage message );
    float radius;
    static IObjParam *iObjParams;

    Mesh mesh;
    void BuildMesh(TimeValue t);

    Tab<TouchSensorObj*> objects;
    CommandMode *previousMode;

    static ICustButton *TouchSensorPickButton;
    static ICustButton *ParentPickButton;

    IParamBlock *pblock;
    static IParamMap *pmapParam;

    INode* triggerObject;

    TouchSensorObject();
    ~TouchSensorObject();


    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    // From BaseObject
           void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
    int HitTest(TimeValue t, INode* inode, int type, int crossing,
                int flags, IPoint2 *p, ViewExp *vpt);
    int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    TCHAR *GetObjectName() { return _T(GetString(IDS_TOUCH_SENSOR)); }


    Tab<TouchSensorObj*> GetObjects() { return objects; }

    // From Object
           ObjectState Eval(TimeValue time);
    void InitNodeName(TSTR& s) { s = _T(GetString(IDS_TOUCH_SENSOR)); }
    Interval ObjectValidity();
    Interval ObjectValidity(TimeValue time);
    int DoOwnSelectHilite() { return 1; }

    void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
    void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

    // Animatable methods
           void DeleteThis() { delete this; }
    Class_ID ClassID() { return Class_ID(TouchSensor_CLASS_ID1,
                                         TouchSensor_CLASS_ID2);}  
    void GetClassName(TSTR& s) { s = TSTR(_T(GetString(IDS_TOUCH_SENSOR_CLASS))); }
    int IsKeyable(){ return 1;}
    LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
                                       WPARAM wParam,   LPARAM lParam )
    { return 0; }


    int NumRefs() {return objects.Count() + 2;}
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);
//    IOResult Load(ILoad *iload) ;

};				

#define PB_TS_SIZE          0
#define PB_TS_ENABLED       1
#define PB_TS_NUMOBJS       2
#define PB_TS_LENGTH        3

#endif

