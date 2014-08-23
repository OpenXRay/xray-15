/**********************************************************************
 *<
    FILE: sound.h
 
    DESCRIPTION:  Defines a VRML 2.0 Sound node helper
 
    CREATED BY: Scott Morrison
 
    HISTORY: created 29 Feb. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __Sound__H__
 
#define __Sound__H__
 
#define Sound_CLASS_ID1 0xAC92442
#define Sound_CLASS_ID2 0xF83deAD

#define SoundClassID Class_ID(Sound_CLASS_ID1, Sound_CLASS_ID2)

extern ClassDesc* GetSoundDesc();

class SoundCreateCallBack;
class SoundObjPick;

class SoundObject: public HelperObject {			   
    friend class SoundCreateCallBack;
    friend class SoundObjPick;
    friend BOOL CALLBACK RollupDialogProc( HWND hDlg, UINT message,
                                           WPARAM wParam, LPARAM lParam );
    friend void BuildObjectList(SoundObject *ob);

  public:

    // Class vars
    static HWND hRollup;
    static int dlgPrevSel;
    static ISpinnerControl *minBackSpin;
    static ISpinnerControl *maxBackSpin;
    static ISpinnerControl *minFrontSpin;
    static ISpinnerControl *maxFrontSpin;

    BOOL needsScript;  // Do we need to generate a script node?

    RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
                                PartID& partID, RefMessage message );
    static IObjParam *iObjParams;

    INode *audioClip;

    Mesh mesh;
    void BuildMesh(TimeValue t);

    CommandMode *previousMode;

    static ICustButton *SoundPickButton;
    IParamBlock *pblock;
    static IParamMap *pmapParam;

    SoundObject();
    ~SoundObject();
    void DrawEllipsoids(TimeValue t, INode* node, GraphicsWindow* gw);

    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    // From BaseObject
           void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
    int HitTest(TimeValue t, INode* inode, int type, int crossing,
                int flags, IPoint2 *p, ViewExp *vpt);
    int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    TCHAR *GetObjectName() { return _T(GetString(IDS_SOUND)); }


    // From Object
           ObjectState Eval(TimeValue time);
    void InitNodeName(TSTR& s) { s = _T(GetString(IDS_SOUND)); }
    Interval ObjectValidity();
    Interval ObjectValidity(TimeValue time);
    int DoOwnSelectHilite() { return 1; }

    void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
    void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

    // Specific to sound object
	float GetMinBack(TimeValue t, Interval& valid = Interval(0,0));
	float GetMaxBack(TimeValue t, Interval& valid = Interval(0,0));
	float GetMinFront(TimeValue t, Interval& valid = Interval(0,0));
	float GetMaxFront(TimeValue t, Interval& valid = Interval(0,0));
    void SetMinBack(TimeValue t, float f);
    void SetMaxBack(TimeValue t, float f);
    void SetMinFront(TimeValue t, float f);
    void SetMaxFront(TimeValue t, float f);
    
    // Animatable methods
           void DeleteThis() { delete this; }
    Class_ID ClassID() { return Class_ID(Sound_CLASS_ID1,
                                         Sound_CLASS_ID2);}  
    void GetClassName(TSTR& s) { s = TSTR(_T(GetString(IDS_SOUND_CLASS))); }
    int IsKeyable(){ return 1;}
    LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
                                       WPARAM wParam,   LPARAM lParam )
    { return 0; }


    int NumRefs() {return 2;}
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);

};				

#define PB_SND_SIZE       0
#define PB_SND_INTENSITY  1
#define PB_SND_MAX_BACK   2
#define PB_SND_MIN_BACK   3
#define PB_SND_MAX_FRONT  4
#define PB_SND_MIN_FRONT  5
#define PB_SND_PRIORITY   6
#define PB_SND_SPATIALIZE 7
#define PB_SND_LENGTH     8

#endif

