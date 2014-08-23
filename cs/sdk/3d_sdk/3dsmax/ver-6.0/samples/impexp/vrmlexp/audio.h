/**********************************************************************
 *<
    FILE: audio.g
 
    DESCRIPTION:  Defines an AuioClip VRML 2.0 helper object
 
    CREATED BY: Scott Morrison
 
    HISTORY: created 29 Aug. 1996
 
 *> Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
 
#ifndef __AudioClip__H__
 
#define __AudioClip__H__
 
#define AudioClip_CLASS_ID1 0xA73D3442
#define AudioClip_CLASS_ID2 0xFB15DBAD

#define AudioClipClassID Class_ID(AudioClip_CLASS_ID1, AudioClip_CLASS_ID2)

extern ClassDesc* GetAudioClipDesc();

class AudioClipObject: public HelperObject {			   
    friend class AudioClipCreateCallBack;
    friend class AudioClipObjPick;
    friend BOOL CALLBACK RollupDialogProc( HWND hDlg, UINT message,
                                           WPARAM wParam, LPARAM lParam );
  public:


    RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
                                PartID& partID, RefMessage message );
    static IObjParam *iObjParams;
    TSTR desc, url;

    Mesh mesh;
    void BuildMesh(TimeValue t);

    HWND imageDlg;
    IParamBlock *pblock;
    static IParamMap *pmapParam;

    BOOL written;

    AudioClipObject();
    ~AudioClipObject();


    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    // From BaseObject
    void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
    int HitTest(TimeValue t, INode* inode, int type, int crossing,
                int flags, IPoint2 *p, ViewExp *vpt);
    int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    TCHAR *GetObjectName() { return _T(GetString(IDS_AUDIO_CLIP)); }


    // From Object
           ObjectState Eval(TimeValue time);
    void InitNodeName(TSTR& s) { s = _T(GetString(IDS_AUDIO_CLIP)); }
    Interval ObjectValidity();
    Interval ObjectValidity(TimeValue time);
    int DoOwnSelectHilite() { return 1; }

    void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
    void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

    // Animatable methods
    void DeleteThis() { delete this; }
    Class_ID ClassID() { return Class_ID(AudioClip_CLASS_ID1,
                                         AudioClip_CLASS_ID2);}  
    void GetClassName(TSTR& s) { s = TSTR(_T(GetString(IDS_AUDIO_CLIP_CLASS))); }
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

#define PB_AC_LOOP      0
#define PB_AC_PITCH     1
#define PB_AC_SIZE      2
#define PB_AC_START     3
#define PB_AC_LENGTH    4

#endif

