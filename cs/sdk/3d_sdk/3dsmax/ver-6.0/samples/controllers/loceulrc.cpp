/**********************************************************************
 *<
    FILE: loceulrc.cpp

    DESCRIPTION: A Local Euler angle rotation controller

    CREATED BY: Pete Samson

    HISTORY: modified from eulrctrl.cpp

 *> Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "ctrl.h"
#include "interpik.h"
#include "euler.h"

#define HYBRID_LINEAR   1

#define LOCAL_EULER_CONTROL_CNAME     GetString(IDS_PRS_LOCALEULERXYZ)

#define EULER_X_REF     0
#define EULER_Y_REF     1
#define EULER_Z_REF     2

#define THRESHHOLD		1.0f

class LocalEulerDlg;

static DWORD subColor[] = {PAINTCURVE_XCOLOR, PAINTCURVE_YCOLOR, PAINTCURVE_ZCOLOR};

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//    WARNING - a copy of this class description is in maxscrpt\mxsagni\lam_ctrl.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class LocalEulerRotation : public Control {
    public:
        Control *rotX;
        Control *rotY;
        Control *rotZ;
        int order;
        Quat curval;
        Interval ivalid;
		Tab<TimeValue> keyTimes;

        static LocalEulerDlg *dlg;
        static IObjParam *ip;
        static ULONG beginFlags;
        static LocalEulerRotation *editControl; // The one being edited.

        LocalEulerRotation(const LocalEulerRotation &ctrl);
        LocalEulerRotation(BOOL loading=FALSE);
        ~LocalEulerRotation();
        void Update(TimeValue t);
        DWORD GetDefaultInTan() {return HYBRID_LINEAR;}
        DWORD GetDefaultOutTan() {return HYBRID_LINEAR;}

        // Animatable methods
        Class_ID ClassID() { return Class_ID(LOCAL_EULER_CONTROL_CLASS_ID,0); }  
        SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; }         
        
        void GetClassName(TSTR& s);
        void DeleteThis() { delete this; }        
        int IsKeyable() { return 1; }     

        int NumSubs()  { return 3; }
        Animatable* SubAnim(int i);
        TSTR SubAnimName(int i);
        int SubNumToRefNum(int subNum) { return subNum; }

        DWORD GetSubAnimCurveColor(int subNum) { return subColor[subNum]; }

        ParamDimension* GetParamDimension(int i) { return stdAngleDim; }
        BOOL AssignController(Animatable *control,int subAnim);
        void AddNewKey(TimeValue t,DWORD flags);
        int NumKeys();
        TimeValue GetKeyTime(int index);
		void SelectKeyByIndex(int i,BOOL sel);
		BOOL IsKeySelected(int i);
        void CopyKeysFromTime(TimeValue src, TimeValue dst, DWORD flags);
        BOOL IsKeyAtTime(TimeValue t, DWORD flags);
        BOOL GetNextKeyTime(TimeValue t, DWORD flags, TimeValue &nt);
        void DeleteKeyAtTime(TimeValue t);
		void DeleteKeys(DWORD flags);
		void DeleteKeyByIndex(int index);

        void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
        void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

        int SetProperty(ULONG id, void *data);
        void *GetProperty(ULONG id);

        // Reference methods
        int NumRefs() { return 3; };    
        RefTargetHandle GetReference(int i);
        void SetReference(int i, RefTargetHandle rtarg);
        RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
        void RescaleWorldUnits(float f) {}

        IOResult Save(ISave *isave);
        IOResult Load(ILoad *iload);

        // Control methods
        Control *GetXController() { return rotX; }
        Control *GetYController() { return rotY; }
        Control *GetZController() { return rotZ; }
        void Copy(Control *from);
        RefTargetHandle Clone(RemapDir& remap);
        BOOL IsLeaf() { return FALSE; }
        void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);    
        void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
        bool GetLocalTMComponents(TimeValue, TMComponentsArg&, Matrix3Indirect& parentTM);
        void CommitValue(TimeValue t);
        void RestoreValue(TimeValue t);
        void EnumIKParams(IKEnumCallback &callback);
        BOOL CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs, DWORD flags);
        float IncIKParam(TimeValue t, int index, float delta);
        void ClearIKParam(Interval iv, int index);
        void EnableORTs(BOOL enable);
        void MirrorIKConstraints(int axis, int which);       
        BOOL CanCopyIKParams(int which);
        IKClipObject *CopyIKParams(int which);
        BOOL CanPasteIKParams(IKClipObject *co, int which);
        void PasteIKParams(IKClipObject *co, int which);		
        void ChangeOrdering(int newOrder);

		// RB 10/27/2000: Implemented to support HI IK
		void InitIKJoints2(InitJointData2 *posData,InitJointData2 *rotData);
		BOOL GetIKJoints2(InitJointData2 *posData,InitJointData2 *rotData);
    };

LocalEulerDlg *LocalEulerRotation::dlg = NULL;
IObjParam *LocalEulerRotation::ip = NULL;
ULONG LocalEulerRotation::beginFlags = 0;
LocalEulerRotation *LocalEulerRotation::editControl = NULL;

class JointParamsLocalEuler : public JointParams {
    public:             
		// RB 10/27/2000: Added to support HI IK
		float preferredAngle[3];
		BOOL pfInit;

        JointParamsLocalEuler() : JointParams((DWORD)JNT_ROT,3) {flags |= JNT_LIMITEXACT;}
        void SpinnerChange(InterpCtrlUI *ui,WORD id,ISpinnerControl *spin,BOOL interactive);
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
    };

static INT_PTR CALLBACK LocalEulerParamDialogProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

static const int editButs[] = {IDC_EULER_X,IDC_EULER_Y,IDC_EULER_Z};

static int eulerIDs[] = {
    IDS_RB_EULERTYPE0,IDS_RB_EULERTYPE1,IDS_RB_EULERTYPE2,
    IDS_RB_EULERTYPE3,IDS_RB_EULERTYPE4,IDS_RB_EULERTYPE5,
    IDS_RB_EULERTYPE6,IDS_RB_EULERTYPE7,IDS_RB_EULERTYPE8};

typedef int EAOrdering[3];
static EAOrdering orderings[] = {
    {0,1,2},
    {0,2,1},
    {1,2,0},
    {1,0,2},
    {2,0,1},
    {2,1,0},
    {0,1,0},
    {1,2,1},
    {2,0,2},
    };

static int xyzIDs[] = {IDS_RB_X,IDS_RB_Y,IDS_RB_Z};
static int xyzRotIDs[] =
    {IDS_PRS_LOCALXROTATION,IDS_PRS_LOCALYROTATION,IDS_PRS_LOCALZROTATION};
static int xyzAxisIDs[] = {IDS_RB_XAXIS,IDS_RB_YAXIS,IDS_RB_ZAXIS};

#define EDIT_X  0
#define EDIT_Y  1
#define EDIT_Z  2

#define EULER_BEGIN     1
#define EULER_MIDDLE    2
#define EULER_END       3

class LocalEulerDlg {
    public:
        LocalEulerRotation *cont;
        HWND hWnd;
        IObjParam *ip;
        ICustButton *iEdit[3];
        static int cur;
        
        LocalEulerDlg(LocalEulerRotation *cont,IObjParam *ip);
        ~LocalEulerDlg();

        void Init();
        void SetButtonText();
        void EndingEdit(LocalEulerRotation *next);
        void BeginingEdit(LocalEulerRotation *cont, IObjParam *ip,
                          LocalEulerRotation *prev);
        void SetCur(int c,int code=EULER_MIDDLE);
        void WMCommand(int id, int notify, HWND hCtrl);
    };

int LocalEulerDlg::cur = EDIT_X;

LocalEulerDlg::LocalEulerDlg(LocalEulerRotation *cont,IObjParam *ip)
    {
    this->ip   = ip;
    this->cont = cont;
    for (int i=0; i<3; i++) {
        iEdit[i] = NULL;
        }
    
    hWnd = ip->AddRollupPage( 
        hInstance,
        MAKEINTRESOURCE(IDD_EULER_PARAMS),
        LocalEulerParamDialogProc,
        GetString(IDS_RB_EULERPARAMS), 
        (LPARAM)this);
    ip->RegisterDlgWnd(hWnd);   
    
    SetCur(cur,EULER_BEGIN);    
    UpdateWindow(hWnd);
    }

LocalEulerDlg::~LocalEulerDlg()
    {
    SetCur(cur,EULER_END);
    for (int i=0; i<3; i++) {
        ReleaseICustButton(iEdit[i]);       
        }
    ip->UnRegisterDlgWnd(hWnd);
    ip->DeleteRollupPage(hWnd);
    hWnd = NULL;
    }

void LocalEulerDlg::EndingEdit(LocalEulerRotation *next)
    {
    switch (cur) {
        case EDIT_X:
            cont->rotX->EndEditParams(ip,0,next->rotX);
            break;
        case EDIT_Y:
            cont->rotY->EndEditParams(ip,0,next->rotY);
            break;
        case EDIT_Z:
            cont->rotZ->EndEditParams(ip,0,next->rotZ);
            break;
        }
    cont = NULL;
    ip   = NULL;
    }

void LocalEulerDlg::BeginingEdit(LocalEulerRotation *cont, IObjParam *ip,
                                 LocalEulerRotation *prev)
    {
    this->ip   = ip;
    this->cont = cont;
    switch (cur) {
        case EDIT_X:
            cont->rotX->BeginEditParams(ip, BEGIN_EDIT_MOTION, prev->rotX);
            break;
        case EDIT_Y:
            cont->rotY->BeginEditParams(ip, BEGIN_EDIT_MOTION, prev->rotY);
            break;
        case EDIT_Z:
            cont->rotZ->BeginEditParams(ip, BEGIN_EDIT_MOTION, prev->rotZ);
            break;
        }   
    UpdateWindow(hWnd);
    }

void LocalEulerDlg::SetButtonText()
    {
    for (int i=0; i<3; i++) {
        iEdit[i]->SetText(GetString(
            xyzIDs[orderings[cont->order][i]]));
        }
    }

void LocalEulerDlg::Init()
    {   
    for (int i=0; i<3; i++) {
        iEdit[i] = GetICustButton(GetDlgItem(hWnd,editButs[i]));        
        iEdit[i]->SetType(CBT_CHECK);       
        }
    iEdit[cur]->SetCheck(TRUE); 
    SetButtonText();

    SendDlgItemMessage(hWnd, IDC_EULER_ORDER, CB_RESETCONTENT, 0, 0);
    for (i=0; i<9; i++) {
        SendDlgItemMessage(hWnd,IDC_EULER_ORDER, CB_ADDSTRING, 0,
            (LPARAM)GetString(eulerIDs[i]));
        }
    SendDlgItemMessage(hWnd, IDC_EULER_ORDER, CB_SETCURSEL, cont->order, 0);
    }

void LocalEulerDlg::SetCur(int c,int code)
    {
    if (c==cur && code==EULER_MIDDLE) return;
    Control *prev = NULL, *next = NULL;

    if (code!=EULER_END) {
        switch (c) {
            case EDIT_X:
                next = cont->rotX;
                break;
            case EDIT_Y:
                next = cont->rotY;
                break;
            case EDIT_Z:
                next = cont->rotZ;
                break;
            }
        }

    if (code!=EULER_BEGIN) {
        switch (cur) {
            case EDIT_X:
                cont->rotX->EndEditParams(ip,END_EDIT_REMOVEUI,next);
                prev = cont->rotX;
                break;
            case EDIT_Y:
                cont->rotY->EndEditParams(ip,END_EDIT_REMOVEUI,next);
                prev = cont->rotY;
                break;
            case EDIT_Z:
                cont->rotZ->EndEditParams(ip,END_EDIT_REMOVEUI,next);
                prev = cont->rotZ;
                break;
            }
        }

    cur = c;

    if (code!=EULER_END) {
        switch (cur) {
            case EDIT_X:
                cont->rotX->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev);
                break;
            case EDIT_Y:
                cont->rotY->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev);
                break;
            case EDIT_Z:
                cont->rotZ->BeginEditParams(ip,BEGIN_EDIT_MOTION,prev);
                break;
            }
        }
    }

void LocalEulerDlg::WMCommand(int id, int notify, HWND hCtrl)
    {
    switch (id) {
        case IDC_EULER_X:
            SetCur(0);
            iEdit[0]->SetCheck(TRUE);
            iEdit[1]->SetCheck(FALSE);
            iEdit[2]->SetCheck(FALSE);
            break;
        case IDC_EULER_Y:
            SetCur(1);
            iEdit[0]->SetCheck(FALSE);
            iEdit[1]->SetCheck(TRUE);
            iEdit[2]->SetCheck(FALSE);
            break;
        case IDC_EULER_Z:
            SetCur(2);
            iEdit[0]->SetCheck(FALSE);
            iEdit[1]->SetCheck(FALSE);
            iEdit[2]->SetCheck(TRUE);
            break;

        case IDC_EULER_ORDER:
            if (notify==CBN_SELCHANGE) {
                int res = SendDlgItemMessage(hWnd, IDC_EULER_ORDER,
                                             CB_GETCURSEL, 0, 0);
                if (res!=CB_ERR) {
                    cont->ChangeOrdering(res);
                    SetButtonText();
                    }
                }
            break;          
        }
    }

static INT_PTR CALLBACK LocalEulerParamDialogProc(HWND hDlg, UINT message,
                                               WPARAM wParam, LPARAM lParam)
    {
    LocalEulerDlg *dlg = (LocalEulerDlg*)GetWindowLongPtr(hDlg,GWLP_USERDATA);

    switch (message) {
        case WM_INITDIALOG:
            dlg = (LocalEulerDlg*)lParam;            
            SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
            dlg->hWnd = hDlg;
            dlg->Init();
            break;
        
        case WM_COMMAND:
            dlg->WMCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
            dlg->ip->RollupMouseMessage(hDlg, message, wParam, lParam);
            break;
                
        default:
            return FALSE;
        }
    return TRUE;
    }


//********************************************************
// LOCAL EULER CONTROL
//********************************************************
static Class_ID localEulerControlClassID(LOCAL_EULER_CONTROL_CLASS_ID,0);
 
class LocalEulerClassDesc : public ClassDesc {
    public:
    int             IsPublic() { return 0; }
    void *          Create(BOOL loading) { return new LocalEulerRotation(loading); }
    const TCHAR *   ClassName() { return LOCAL_EULER_CONTROL_CNAME; }
    SClass_ID       SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
    Class_ID        ClassID() { return localEulerControlClassID; }
    const TCHAR*    Category() { return _T("");  }
    };
static LocalEulerClassDesc eulerCD;
ClassDesc* GetLocalEulerCtrlDesc() {return &eulerCD;}

LocalEulerRotation::LocalEulerRotation(const LocalEulerRotation &ctrl)
    {
    order = EULERTYPE_XYZ;
    rotX = NULL;
    rotY = NULL;
    rotZ = NULL;

    if (ctrl.rotX) {
        ReplaceReference(EULER_X_REF, ctrl.rotX);
    } else {
        ReplaceReference(EULER_X_REF, NewDefaultFloatController());
        }
    if (ctrl.rotY) {
        ReplaceReference(EULER_Y_REF, ctrl.rotY);
    } else {
        ReplaceReference(EULER_Y_REF, NewDefaultFloatController());
        }
    if (ctrl.rotZ) {
        ReplaceReference(EULER_Z_REF, ctrl.rotZ);
    } else {
        ReplaceReference(EULER_Z_REF, NewDefaultFloatController());
        }
    curval = ctrl.curval;
    ivalid = ctrl.ivalid;
    }

LocalEulerRotation::LocalEulerRotation(BOOL loading) 
    {
    order = EULERTYPE_XYZ;
    rotX = NULL;
    rotY = NULL;
    rotZ = NULL;
    if (!loading) {
        ReplaceReference(EULER_X_REF, NewDefaultFloatController());
        ReplaceReference(EULER_Y_REF, NewDefaultFloatController());
        ReplaceReference(EULER_Z_REF, NewDefaultFloatController());
        ivalid = FOREVER;
        curval.Identity();
    } else {
        ivalid.SetEmpty();
        }   
    }

RefTargetHandle LocalEulerRotation::Clone(RemapDir& remap) 
    {
    LocalEulerRotation *euler = new LocalEulerRotation(TRUE); 
    euler->ReplaceReference(EULER_X_REF, remap.CloneRef(rotX));
    euler->ReplaceReference(EULER_Y_REF, remap.CloneRef(rotY));
    euler->ReplaceReference(EULER_Z_REF, remap.CloneRef(rotZ));
    euler->order = order;

    JointParams *jp = (JointParams*)GetProperty(PROPID_JOINTPARAMS);
    if (jp) {
        JointParams *jp2 = new JointParams(*jp);
        euler->SetProperty(PROPID_JOINTPARAMS, jp2);
        }
	BaseClone(this, euler, remap);
    return euler;
    }



LocalEulerRotation::~LocalEulerRotation()
    {
    DeleteAllRefsFromMe();
    }

void LocalEulerRotation::GetClassName(TSTR& s)
    {       
    TSTR format(GetString(IDS_PRS_LOCALEULERNAME));
    s.printf(format, GetString(eulerIDs[order]));
    }
// This copy method will sample the from controller and smooth out all flips
// Nikolai 1-15-99
void LocalEulerRotation::Copy(Control *from)
	{
	if (from->ClassID()==ClassID()) {
		LocalEulerRotation *ctrl = (LocalEulerRotation*)from;
		ReplaceReference(EULER_X_REF,ctrl->rotX);
		ReplaceReference(EULER_Y_REF,ctrl->rotY);
		ReplaceReference(EULER_Z_REF,ctrl->rotZ);
		curval = ctrl->curval;
		ivalid = ctrl->ivalid;
		order  = ctrl->order;
	} else {		
		Quat qPrev;
		Quat qCurr;
		Interval iv;
		int num;		
		if ((num=from->NumKeys())!=NOT_KEYFRAMEABLE && num>0) {		
			SuspendAnimate();
			AnimateOn();
			Interval anim;

			anim.SetStart(from->GetKeyTime(0));

			float eaCurr[3];
			float eaPrev[3];
			float EulerAng[3] = {0,0,0};

			from->GetValue(anim.Start(),&qPrev,iv);

			Matrix3 tm;
			qPrev.MakeMatrix(tm);
			MatrixToEuler(tm,EulerAng, order);
				
			rotX->SetValue(anim.Start(),&EulerAng[0],TRUE, CTRL_ABSOLUTE);
			rotY->SetValue(anim.Start(),&EulerAng[1],TRUE, CTRL_ABSOLUTE);
			rotZ->SetValue(anim.Start(),&EulerAng[2],TRUE, CTRL_ABSOLUTE);
			
			if(num>1)
			{
				float dEuler[3],f;	
				Matrix3 tmPrev, tmCurr;

				anim.SetEnd(from->GetKeyTime(num-1));
				
				// Here we sample over the time range, to detect flips
				for(TimeValue time = anim.Start()+1; time <= anim.End() ; time++  )
				{
					from->GetValue(time,&qCurr,iv);
					
					qPrev.MakeMatrix(tmPrev);
					qCurr.MakeMatrix(tmCurr);

					// The Euler/Quat ratio is the relation of the angle difference in Euler space to 
					// the angle difference in Quat space. If this ration is bigger than PI the rotation 
					// between the two time steps contains a flip

					f = GetEulerMatAngleRatio(tmPrev,tmCurr,eaPrev,eaCurr,order);	
										
					if(  f > PI)
					{
						// We found a flip here
						for(int j=0 ; j < 3 ; j++)
						{				
							// find the sign flip :
							if(fabs((eaCurr[j]-eaPrev[j])) < 2*PI-THRESHHOLD )
								dEuler[j] = eaCurr[j]-eaPrev[j];
							else
								// unflip the flip
								dEuler[j] = (2*PI - (float) (fabs(eaCurr[j]) + fabs(eaPrev[j]))) * (eaPrev[j] > 0 ? 1 : -1);
							
							EulerAng[j] += dEuler[j];
						}
					}
					else
					{
						// Add up the angle difference
						for(int j=0 ; j < 3 ; j++)
						{
							dEuler[j] = eaCurr[j]-eaPrev[j];
							EulerAng[j] += dEuler[j];
						}
					}
					if(from->IsKeyAtTime(time,KEYAT_ROTATION))
					{
						// Create the keys
						rotX->SetValue(time,&EulerAng[0],TRUE, CTRL_ABSOLUTE);
						rotY->SetValue(time,&EulerAng[1],TRUE, CTRL_ABSOLUTE);
						rotZ->SetValue(time,&EulerAng[2],TRUE, CTRL_ABSOLUTE);
					}
					qPrev = qCurr;
				}
			}
			// RB 2/10/99: A key at frame 0 may have been created
			if (num>0 && from->GetKeyTime(0)!=0) {
				rotX->DeleteKeyAtTime(0);
				rotY->DeleteKeyAtTime(0);
				rotZ->DeleteKeyAtTime(0);
			}
			ResumeAnimate();
		} else {
			from->GetValue(0,&qCurr,ivalid);
			SetValue(0,&qCurr,TRUE,CTRL_ABSOLUTE);
			}
		}
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}
  
/*
// This is the old way
void LocalEulerRotation::Copy(Control *from)
    {
    if (from->ClassID() == ClassID()) {
        LocalEulerRotation *ctrl = (LocalEulerRotation*)from;
        ReplaceReference(EULER_X_REF, ctrl->rotX);
        ReplaceReference(EULER_Y_REF, ctrl->rotY);
        ReplaceReference(EULER_Z_REF, ctrl->rotZ);
        curval = ctrl->curval;
        ivalid = ctrl->ivalid;
        order  = ctrl->order;
    } else {        
        Quat v;
        Interval iv;
        int num;        
        if (num=from->NumKeys()) {
            SuspendAnimate();
            AnimateOn();
            for (int i=0; i<num; i++) {
                TimeValue t = from->GetKeyTime(i);
                from->GetValue(t,&v,iv);
                SetValue(t,&v,TRUE,CTRL_ABSOLUTE);  
                }
            ResumeAnimate();
        } else {
            from->GetValue(0,&v,ivalid);
            SetValue(0,&v,TRUE,CTRL_ABSOLUTE);
            }
        }
    NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
    }
*/

// added 020819  --prs.
void LocalEulerRotation::SelectKeyByIndex(int i, BOOL sel)
	{
	int nk = NumKeys();				// sets up keyTimes table
	if (i < nk) {
		TimeValue tv = GetKeyTime(i);
		if (rotX) {
			int xkey = rotX->GetKeyIndex(tv);
			if (xkey >= 0)
				rotX->SelectKeyByIndex(xkey, sel);
			}
		if (rotY) {
			int ykey = rotY->GetKeyIndex(tv);
			if (ykey >= 0)
				rotY->SelectKeyByIndex(ykey, sel);
			}
		if (rotZ) {
			int zkey = rotZ->GetKeyIndex(tv);
			if (zkey >= 0)
				rotZ->SelectKeyByIndex(zkey, sel);
			}
		}
	}

// added 020819  --prs.
BOOL LocalEulerRotation::IsKeySelected(int i)
	{
	if (i < keyTimes.Count()) {
		TimeValue tv = GetKeyTime(i);
		if (rotX) {
			int xkey = rotX->GetKeyIndex(tv);
			if (xkey >= 0 && rotX->IsKeySelected(xkey))
				return TRUE;
			}
		if (rotY) {
			int ykey = rotY->GetKeyIndex(tv);
			if (ykey >= 0 && rotY->IsKeySelected(ykey))
				return TRUE;
			}
		if (rotZ) {
			int zkey = rotZ->GetKeyIndex(tv);
			if (zkey >= 0 && rotZ->IsKeySelected(zkey))
				return TRUE;
			}
		}
	return FALSE;
	}

void LocalEulerRotation::Update(TimeValue t)
    {
    if (!ivalid.InInterval(t)) {
        ivalid = FOREVER;
        Point3 ang(0,0,0);
        if (rotX) rotX->GetValue(t,&ang.x,ivalid);
        if (rotY) rotY->GetValue(t,&ang.y,ivalid);
        if (rotZ) rotZ->GetValue(t,&ang.z,ivalid);
        
        // This could be optimized.
        //Matrix3 tm(1);
        //tm.RotateX(x);
        //tm.RotateY(y);
        //tm.RotateZ(z);
        //curval = Quat(tm);        
        Matrix3 tm(1);
        for (int i = 2; i >= 0; i--) {
            switch (orderings[order][i]) {
                case 0: tm.RotateX(ang[i]); break;
                case 1: tm.RotateY(ang[i]); break;
                case 2: tm.RotateZ(ang[i]); break;
                }
            }
        curval = Quat(tm);
        //EulerToQuat(ang, curval, order);
        }
    }

void LocalEulerRotation::ChangeOrdering(int newOrder)
    {
    order = newOrder;
    ivalid.SetEmpty();
    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
    NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
    ip->RedrawViews(ip->GetTime());
    }

void LocalEulerRotation::SetValue(TimeValue t, void *val, int commit,
                                  GetSetMethod method)
    {
    Quat v;
    Update(t);
    if (method==CTRL_RELATIVE) {
        v = curval * Quat(*((AngAxis*)val));
    } else {
        v = *((Quat*)val);
        }
        
    float ang[3];
    //QuatToEuler(v,ang);
    Matrix3 tm;
    v.MakeMatrix(tm);
    MatrixToEuler(tm,ang, order | EULERTYPE_RF); // rotate the coordinate frame
    
    // RB: this gives the incorrect sign sometimes...
    //QuatToEuler(v, ang, order);       

    if (rotX) rotX->SetValue(t,&ang[0]);
    if (rotY) rotY->SetValue(t,&ang[1]);
    if (rotZ) rotZ->SetValue(t,&ang[2]);
    ivalid.SetEmpty();
    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
    }

void LocalEulerRotation::GetValue(TimeValue t, void *val, Interval &valid,
                                  GetSetMethod method)
    {       
    Update(t);
    valid &= ivalid;             
    if (method==CTRL_RELATIVE) {
        Matrix3 *mat = (Matrix3*)val;       
        PreRotateMatrix(*mat,curval);       
    } else {
        *((Quat*)val) = curval;
        }
    }

bool LocalEulerRotation::GetLocalTMComponents(
    TimeValue    t,
    TMComponentsArg& cmpts,
    Matrix3Indirect&)
//
// This rotation controller does not need parent matrix
//
{
  // Short circuit
  if (cmpts.rotation == NULL) return true;
  assert(cmpts.rotValidity);

  if (rotX) rotX->GetValue(t, (void*)cmpts.rotation, *cmpts.rotValidity);
  if (rotY) rotY->GetValue(t, (void*)(cmpts.rotation+1), *cmpts.rotValidity);
  if (rotZ) rotZ->GetValue(t, (void*)(cmpts.rotation+2), *cmpts.rotValidity);
  cmpts.rotRep = (TMComponentsArg::RotationRep)order;
  return true;
}

void LocalEulerRotation::CommitValue(TimeValue t)
    {
    if (rotX) rotX->CommitValue(t);
    if (rotY) rotY->CommitValue(t);
    if (rotZ) rotZ->CommitValue(t);
    }

void LocalEulerRotation::RestoreValue(TimeValue t)
    {
    if (rotX) rotX->RestoreValue(t);
    if (rotY) rotY->RestoreValue(t);
    if (rotZ) rotZ->RestoreValue(t);
    }

RefTargetHandle LocalEulerRotation::GetReference(int i)
    {
    switch (i) {
        case EULER_X_REF: return rotX;
        case EULER_Y_REF: return rotY;
        case EULER_Z_REF: return rotZ;
        default: return NULL;
        }
    }

void LocalEulerRotation::SetReference(int i, RefTargetHandle rtarg)
    {
    switch (i) {
        case EULER_X_REF: rotX = (Control*)rtarg; break;
        case EULER_Y_REF: rotY = (Control*)rtarg; break;
        case EULER_Z_REF: rotZ = (Control*)rtarg; break;
        }
    }

Animatable* LocalEulerRotation::SubAnim(int i)
    {
    return GetReference(i);
    }

TSTR LocalEulerRotation::SubAnimName(int i)
    {   
    switch (i) {
        case EULER_X_REF: return GetString(xyzRotIDs[orderings[order][0]]);
        case EULER_Y_REF: return GetString(xyzRotIDs[orderings[order][1]]);
        case EULER_Z_REF: return GetString(xyzRotIDs[orderings[order][2]]);
        default: return _T("");
        }
    }

RefResult LocalEulerRotation::NotifyRefChanged(
        Interval iv, 
        RefTargetHandle hTarg, 
        PartID& partID, 
        RefMessage msg) 
    {
    switch (msg) {
        case REFMSG_CHANGE:
            ivalid.SetEmpty();
            break;
        case REFMSG_TARGET_DELETED:
            if (rotX == hTarg) rotX = NULL;
            if (rotY == hTarg) rotY = NULL;
            if (rotZ == hTarg) rotZ = NULL; 
            break;
        case REFMSG_GET_CONTROL_DIM: {
            ParamDimension **dim = (ParamDimension **)partID;
            assert(dim);
            *dim = stdAngleDim;
            }
        }
    return REF_SUCCEED;
    }

BOOL LocalEulerRotation::AssignController(Animatable *control,int subAnim)
    {   
    switch (subAnim) {
        case EULER_X_REF:
            ReplaceReference(EULER_X_REF,(RefTargetHandle)control);
            break;
        case EULER_Y_REF:
            ReplaceReference(EULER_Y_REF,(RefTargetHandle)control);
            break;
        case EULER_Z_REF:
            ReplaceReference(EULER_Z_REF,(RefTargetHandle)control);
            break;
        }
    NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE,TREE_VIEW_CLASS_ID,FALSE);
    NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);   
    return TRUE;
    }

void LocalEulerRotation::AddNewKey(TimeValue t, DWORD flags)
    {
    if (rotX) rotX->AddNewKey(t,flags);
    if (rotY) rotY->AddNewKey(t,flags);
    if (rotZ) rotZ->AddNewKey(t,flags);
    }

int LocalEulerRotation::NumKeys()
    {
#if 1		// 020819  --prs.
	int xnum = rotX->NumKeys();
	int ynum = rotY->NumKeys();
	int znum = rotZ->NumKeys();
	keyTimes.ZeroCount();
	for (int i = 0, j = 0, k = 0;
		 i < xnum || j < ynum || k < znum; ) {
		TimeValue k1 = i < xnum ? rotX->GetKeyTime(i) : TIME_PosInfinity;
		TimeValue k2 = j < ynum ? rotY->GetKeyTime(j) : TIME_PosInfinity;
		TimeValue k3 = k < znum ? rotZ->GetKeyTime(k) : TIME_PosInfinity;
		TimeValue kmin = k1 < k2 ? (k3 < k1 ? k3 : k1) : (k3 < k2 ? k3 : k2);
		keyTimes.Append(1, &kmin, 10);
		if (k1 == kmin) ++i;
		if (k2 == kmin) ++j;
		if (k3 == kmin) ++k;
	}
	return keyTimes.Count();
#else
    int num = 0;
    if (rotX) num += rotX->NumKeys(); 
    if (rotY) num += rotY->NumKeys();
    if (rotZ) num += rotZ->NumKeys();
    return num;
#endif
    }

TimeValue LocalEulerRotation::GetKeyTime(int index)
    {
#if 1	// 020819  --prs.
	return keyTimes[index];
#else
    int onum, num = 0;
    if (rotX) num += rotX->NumKeys(); 
    if (index < num) return rotX->GetKeyTime(index);
    onum = num;
    if (rotY) num += rotY->NumKeys(); 
    if (index < num) return rotY->GetKeyTime(index-onum);
    onum = num;
    if (rotZ) num += rotZ->NumKeys(); 
    if (index < num) return rotZ->GetKeyTime(index-onum);
    return 0;
#endif
    }

void LocalEulerRotation::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
    {
    if (rotX) rotX->CopyKeysFromTime(src,dst,flags);
    if (rotY) rotY->CopyKeysFromTime(src,dst,flags);
    if (rotZ) rotZ->CopyKeysFromTime(src,dst,flags);
    }

BOOL LocalEulerRotation::IsKeyAtTime(TimeValue t, DWORD flags)
    {
    if (rotX && rotX->IsKeyAtTime(t,flags)) return TRUE;
    if (rotY && rotY->IsKeyAtTime(t,flags)) return TRUE;
    if (rotZ && rotZ->IsKeyAtTime(t,flags)) return TRUE;
    return FALSE;
    }

void LocalEulerRotation::DeleteKeyAtTime(TimeValue t)
    {
    if (rotX) rotX->DeleteKeyAtTime(t);
    if (rotY) rotY->DeleteKeyAtTime(t);
    if (rotZ) rotZ->DeleteKeyAtTime(t);
    }

// added 020823  --prs.
void LocalEulerRotation::DeleteKeys(DWORD flags)
	{
	if (rotX) rotX->DeleteKeys(flags);
	if (rotY) rotY->DeleteKeys(flags);
	if (rotZ) rotZ->DeleteKeys(flags);
	}

// added 020813  --prs.
void LocalEulerRotation::DeleteKeyByIndex(int index)
	{
	int nk = NumKeys();	// sets up keyTimes table

	if (index < nk)
		DeleteKeyAtTime(keyTimes[index]);
	}

BOOL LocalEulerRotation::GetNextKeyTime(TimeValue t, DWORD flags, TimeValue &nt)
    {
    TimeValue at,tnear = 0;
    BOOL tnearInit = FALSE;
    
    if (rotX && rotX->GetNextKeyTime(t,flags,at)) {
        if (!tnearInit) {
            tnear = at;
            tnearInit = TRUE;
        } else 
        if (ABS(at-t) < ABS(tnear-t)) tnear = at;
        }

    if (rotY && rotY->GetNextKeyTime(t,flags,at)) {
        if (!tnearInit) {
            tnear = at;
            tnearInit = TRUE;
        } else 
        if (ABS(at-t) < ABS(tnear-t)) tnear = at;
        }

    if (rotZ && rotZ->GetNextKeyTime(t,flags,at)) {
        if (!tnearInit) {
            tnear = at;
            tnearInit = TRUE;
        } else 
        if (ABS(at-t) < ABS(tnear-t)) tnear = at;
        }
    
    if (tnearInit) {
        nt = tnear;
        return TRUE;
    } else {
        return FALSE;
        }
    }
        

void LocalEulerRotation::BeginEditParams(IObjParam *ip, ULONG flags,
                                         Animatable *prev )
    {
    if (flags & BEGIN_EDIT_HIERARCHY) {
        JointParamsLocalEuler *jp = (JointParamsLocalEuler*)GetProperty(PROPID_JOINTPARAMS);
        InterpCtrlUI *ui;   

        if (!jp) {
            jp = new JointParamsLocalEuler();
            SetProperty(PROPID_JOINTPARAMS,jp);
            }

        if (prev &&
            prev->ClassID()==ClassID() && 
            (ui = (InterpCtrlUI*)prev->GetProperty(PROPID_INTERPUI))) {
            JointParams *prevjp = (JointParams*)prev->GetProperty(PROPID_JOINTPARAMS);
            prevjp->EndDialog(ui);
            ui->cont = this;
            ui->ip   = ip;
            prev->SetProperty(PROPID_INTERPUI,NULL);
            JointDlgData *jd = (JointDlgData*)GetWindowLongPtr(ui->hParams,
                                                            GWLP_USERDATA);
            jd->jp = jp;
            jp->InitDialog(ui);
        } else {
            ui = new InterpCtrlUI(NULL, ip, this);
            DWORD f=0;
            if (jp && !jp->RollupOpen()) f = APPENDROLL_CLOSED; 

            ui->hParams = ip->AddRollupPage(hInstance, 
                                            MAKEINTRESOURCE(IDD_STDJOINTPARAMS),
                                            JointParamDlgProc,
                                            GetString(IDS_RB_ROTJOINTPARAMS), 
                                            (LPARAM)new JointDlgData(ui,jp),f); 
            }
    
        SetDlgItemText(ui->hParams,IDC_XAXIS_LABEL,
            GetString(xyzAxisIDs[orderings[order][0]]));
        SetDlgItemText(ui->hParams,IDC_YAXIS_LABEL,
            GetString(xyzAxisIDs[orderings[order][1]]));
        SetDlgItemText(ui->hParams,IDC_ZAXIS_LABEL,
            GetString(xyzAxisIDs[orderings[order][2]]));

        SetProperty(PROPID_INTERPUI, ui);
        editControl = this;
        beginFlags = flags;
    } else 
    if (flags & BEGIN_EDIT_MOTION) {
        this->ip = ip;

        if (dlg) {
            dlg->BeginingEdit(this,ip,(LocalEulerRotation*)prev);
            dlg->Init();
        } else {
            dlg = new LocalEulerDlg(this,ip);    
            }
        }
    }

void LocalEulerRotation::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
    {   
    LocalEulerRotation *cont=NULL;
    if (next && next->ClassID()==ClassID()) {
        cont = (LocalEulerRotation*)next;
        }

    if (dlg) {
        if (cont) {
            dlg->EndingEdit(cont);
        } else {
            delete dlg;
            dlg = NULL;
            }
    } else {
        if (cont) return;
        
        editControl = NULL;
        beginFlags = 0;

        int index = aprops.FindProperty(PROPID_INTERPUI);
        if (index>=0) {
            InterpCtrlUI *ui = (InterpCtrlUI*)aprops[index];
            if (ui->hParams) {
                ip->UnRegisterDlgWnd(ui->hParams);
                ip->DeleteRollupPage(ui->hParams);          
                }
            index = aprops.FindProperty(PROPID_INTERPUI);
            if (index>=0) {
                delete aprops[index];
                aprops.Delete(index,1);
                }
            }
        }
    }

int LocalEulerRotation::SetProperty(ULONG id, void *data)
    {
    if (id==PROPID_JOINTPARAMS) {       
        if (!data) {
            int index = aprops.FindProperty(id);
            if (index>=0) {
                aprops.Delete(index,1);
                }
        } else {
            JointParamsLocalEuler *jp = (JointParamsLocalEuler*)GetProperty(id);
            if (jp) {
                *jp = *((JointParamsLocalEuler*)data);
                delete (JointParamsLocalEuler*)data;
            } else {
                aprops.Append(1,(AnimProperty**)&data);
                }                   
            }
        return 1;
    } else
    if (id==PROPID_INTERPUI) {      
        if (!data) {
            int index = aprops.FindProperty(id);
            if (index>=0) {             
                aprops.Delete(index,1);
                }
        } else {
            InterpCtrlUI *ui = (InterpCtrlUI*)GetProperty(id);
            if (ui) {
                *ui = *((InterpCtrlUI*)data);
            } else {
                aprops.Append(1,(AnimProperty**)&data);
                }                   
            }
        return 1;
    } else {
        return Animatable::SetProperty(id,data);
        }
    }

void* LocalEulerRotation::GetProperty(ULONG id)
    {
    if (id==PROPID_INTERPUI || id==PROPID_JOINTPARAMS) {
        int index = aprops.FindProperty(id);
        if (index>=0) {
            return aprops[index];
        } else {
            return NULL;
            }
    } else {
        return Animatable::GetProperty(id);
        }
    }


#define JOINTPARAMEULER_CHUNK   0x1002
#define ORDER_CHUNK             0x1003

IOResult LocalEulerRotation::Save(ISave *isave)
    {   
    ULONG nb;
    JointParamsLocalEuler *jp = (JointParamsLocalEuler*)GetProperty(PROPID_JOINTPARAMS);
    if (jp) {
        isave->BeginChunk(JOINTPARAMEULER_CHUNK);
        jp->Save(isave);
        isave->EndChunk();
        }

    isave->BeginChunk(ORDER_CHUNK);
    isave->Write(&order,sizeof(order),&nb);
    isave->EndChunk();

    return IO_OK;
    }

IOResult LocalEulerRotation::Load(ILoad *iload)
    {
    ULONG nb;
    IOResult res = IO_OK;
    while (IO_OK==(res=iload->OpenChunk())) {
        switch (iload->CurChunkID()) {
            case ORDER_CHUNK:
                res=iload->Read(&order,sizeof(order),&nb);
                break;

            case JOINTPARAMEULER_CHUNK: {
                JointParamsLocalEuler *jp = new JointParamsLocalEuler;
                jp->Load(iload);
                jp->flags |= JNT_LIMITEXACT;
                SetProperty(PROPID_JOINTPARAMS,jp);
                break;
                }
            }       
        iload->CloseChunk();
        if (res!=IO_OK)  return res;
        }
    return IO_OK;
    }

#define JP_PREFANGLE_CHUNK		0x0090

IOResult JointParamsLocalEuler::Save(ISave *isave)
	{
	ULONG nb;

	if (pfInit) {
		// Only save preferred angle if it has been initialized
		isave->BeginChunk(JP_PREFANGLE_CHUNK);
		isave->Write(preferredAngle,sizeof(float)*3,&nb);
		isave->EndChunk();
		}

	// Default saving
	return JointParams::Save(isave);
	}

IOResult JointParamsLocalEuler::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res;

	// Read the perferred angle chunk if it's present.
	if (iload->PeekNextChunkID()==JP_PREFANGLE_CHUNK) {
		res = iload->OpenChunk();
		res = iload->Read(preferredAngle, sizeof(float)*3, &nb);
		res = iload->CloseChunk();
		if (res!=IO_OK)  return res;
	} else {
		// Note that the preferred angle fields have not been initialized.
		pfInit = FALSE;
		}

	// Default loading
	return JointParams::Load(iload);
	}


void LocalEulerRotation::EnumIKParams(IKEnumCallback &callback)
    {
    JointParamsLocalEuler *jp = (JointParamsLocalEuler*)GetProperty(PROPID_JOINTPARAMS);
    for (int i=2; i>=0; i--) {
        if (!jp || jp->Active(i)) {
            callback.proc(this,i);
            }
        }
    }

BOOL LocalEulerRotation::CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs,
                                   DWORD flags)
    {
    JointParamsLocalEuler *jp = (JointParamsLocalEuler*)GetProperty(PROPID_JOINTPARAMS);
    Quat q;
    Interval valid;
    Point3 a(0,0,0);

    if (rotX) rotX->GetValue(t,&a[0],valid);
    if (rotY) rotY->GetValue(t,&a[1],valid);
    if (rotZ) rotZ->GetValue(t,&a[2],valid);

    for (int i=2; i>=0; i--) {
        if (!jp || jp->Active(i)) {
            for (int j=0; j<derivs.NumEndEffectors(); j++) {
                Point3 r = derivs.EndEffectorPos(j) - ptm.GetTrans();   
        
                Point3 axis = ptm.GetRow(orderings[order][i]);
                if (!(ptm.GetIdentFlags()&SCL_IDENT)) {
                    axis = Normalize(axis);
                    if (ptm.Parity()) axis = -axis;
                    }

                if (flags&POSITION_DERIV) {
                    derivs.DP(CrossProd(axis,r),j);
                    }
                if (flags&ROTATION_DERIV) {
                    derivs.DR(axis,j);
                    }
                }
            derivs.NextDOF();           
            }
        switch (orderings[order][i]) {
            case 0: ptm.PreRotateX(a[i]); break;
            case 1: ptm.PreRotateY(a[i]); break;
            case 2: ptm.PreRotateZ(a[i]); break;
            }
        }   
    return TRUE;
    }

#define MAX_IKROT   DegToRad(4.0f)
#define SGN(a)  (a<0?-1:1)

float LocalEulerRotation::IncIKParam(TimeValue t, int index, float delta)
    {
    JointParamsLocalEuler *jp = (JointParamsLocalEuler*)GetProperty(PROPID_JOINTPARAMS);
    if ((float)fabs(delta)>MAX_IKROT) delta = MAX_IKROT * SGN(delta);
    
    if (jp) {
        float v=0.0f;       
        if (jp->Limited(index) || jp->Spring(index)) {
            Interval valid;
            switch (index) {
                case 0: if (rotX) rotX->GetValue(t,&v,valid); break;
                case 1: if (rotY) rotY->GetValue(t,&v,valid); break;
                case 2: if (rotZ) rotZ->GetValue(t,&v,valid); break;
                }
            }
        delta = jp->ConstrainInc(index,v,delta);
        }
    switch (index) {
        case 0: if (rotX) rotX->SetValue(t,&delta,FALSE,CTRL_RELATIVE); break;
        case 1: if (rotY) rotY->SetValue(t,&delta,FALSE,CTRL_RELATIVE); break;
        case 2: if (rotZ) rotZ->SetValue(t,&delta,FALSE,CTRL_RELATIVE); break;
        }   
    return delta;   
    }

void LocalEulerRotation::ClearIKParam(Interval iv,int index) 
    {
    switch (index) {
        case 0: if (rotX) rotX->DeleteTime(iv,TIME_INCRIGHT|TIME_NOSLIDE); break;
        case 1: if (rotY) rotY->DeleteTime(iv,TIME_INCRIGHT|TIME_NOSLIDE); break;
        case 2: if (rotZ) rotZ->DeleteTime(iv,TIME_INCRIGHT|TIME_NOSLIDE); break;
        }
    }

void LocalEulerRotation::MirrorIKConstraints(int axis,int which)
    {
    JointParamsLocalEuler *jp = (JointParamsLocalEuler*)GetProperty(PROPID_JOINTPARAMS);
    if (jp) jp->MirrorConstraints(axis);
    }

void LocalEulerRotation::EnableORTs(BOOL enable)
    {
    if (rotX) rotX->EnableORTs(enable);
    if (rotY) rotY->EnableORTs(enable);
    if (rotZ) rotZ->EnableORTs(enable);
    }

BOOL LocalEulerRotation::CanCopyIKParams(int which)
    {
    return ::CanCopyIKParams(this,which);
    }

IKClipObject *LocalEulerRotation::CopyIKParams(int which)
    {
    return ::CopyIKParams(this,which);
    }

BOOL LocalEulerRotation::CanPasteIKParams(IKClipObject *co,int which)
    {
    return ::CanPasteIKParams(this,co,which);
    }

void LocalEulerRotation::PasteIKParams(IKClipObject *co,int which)
    {
    ::PasteIKParams(this,co,which);
    }

void JointParamsLocalEuler::SpinnerChange(
        InterpCtrlUI *ui,WORD id,ISpinnerControl *spin,BOOL interactive)
    {
    LocalEulerRotation *c = (LocalEulerRotation*)ui->cont;
    Point3 a(0,0,0);
    BOOL set = FALSE;
    Interval valid;

    if (c->rotX) c->rotX->GetValue(ui->ip->GetTime(),&a[0],valid);
    if (c->rotY) c->rotY->GetValue(ui->ip->GetTime(),&a[1],valid);
    if (c->rotZ) c->rotZ->GetValue(ui->ip->GetTime(),&a[2],valid);

    switch (id) {
        case IDC_XFROMSPIN:
            a[0] = min[0] = DegToRad(spin->GetFVal()); 
            set = TRUE;
            break;
        case IDC_XTOSPIN:
            a[0] = max[0] = DegToRad(spin->GetFVal());
            set = TRUE;
            break;
        case IDC_XSPRINGSPIN:
            a[0] = spring[0] = DegToRad(spin->GetFVal());
            set = TRUE;
            break;
        
        case IDC_YFROMSPIN:
            a[1] = min[1] = DegToRad(spin->GetFVal()); 
            set = TRUE;
            break;
        case IDC_YTOSPIN:
            a[1] = max[1] = DegToRad(spin->GetFVal());
            set = TRUE;
            break;
        case IDC_YSPRINGSPIN:
            a[0] = spring[1] = DegToRad(spin->GetFVal());
            set = TRUE;
            break;
        
        case IDC_ZFROMSPIN:
            a[2] = min[2] = DegToRad(spin->GetFVal()); 
            set = TRUE;
            break;
        case IDC_ZTOSPIN:
            a[2] = max[2] = DegToRad(spin->GetFVal());
            set = TRUE;
            break;
        case IDC_ZSPRINGSPIN:
            a[2] = spring[0] = DegToRad(spin->GetFVal());
            set = TRUE;
            break;
        
        case IDC_XDAMPINGSPIN:
            damping[0] = spin->GetFVal(); break;        
        case IDC_YDAMPINGSPIN:
            damping[1] = spin->GetFVal(); break;        
        case IDC_ZDAMPINGSPIN:
            damping[2] = spin->GetFVal(); break;

        case IDC_XSPRINGTENSSPIN:
            stens[0] = spin->GetFVal()/SPRINGTENS_UI; break;
        case IDC_YSPRINGTENSSPIN:
            stens[1] = spin->GetFVal()/SPRINGTENS_UI; break;
        case IDC_ZSPRINGTENSSPIN:
            stens[2] = spin->GetFVal()/SPRINGTENS_UI; break;
        }
    
    if (set && interactive) {               
        if (c->rotX) c->rotX->SetValue(ui->ip->GetTime(),&a[0],TRUE,CTRL_ABSOLUTE);
        if (c->rotY) c->rotY->SetValue(ui->ip->GetTime(),&a[1],TRUE,CTRL_ABSOLUTE);
        if (c->rotZ) c->rotZ->SetValue(ui->ip->GetTime(),&a[2],TRUE,CTRL_ABSOLUTE);
        ui->ip->RedrawViews(ui->ip->GetTime(),REDRAW_INTERACTIVE);
        }
    }



void LocalEulerRotation::InitIKJoints2(InitJointData2 *posData,InitJointData2 *rotData)
	{
	JointParamsLocalEuler *jp   = (JointParamsLocalEuler*)GetProperty(PROPID_JOINTPARAMS);
	InitJointData3* data3 = DowncastToJointData3(rotData);
	/*
	// Must be 0
	if (rotData->flags) return;
	*/
	if (rotData->flags && data3 == NULL) return;

	if (!jp) {
		jp = new JointParamsLocalEuler();
		SetProperty(PROPID_JOINTPARAMS,jp);
		}

	jp->flags &= ~(
		JNT_XACTIVE|JNT_YACTIVE|JNT_ZACTIVE|
		JNT_XLIMITED|JNT_YLIMITED|JNT_ZLIMITED|
		JNT_XEASE|JNT_YEASE|JNT_ZEASE);

	for (int i=0; i<3; i++) {
		jp->min[i]            = rotData->min[i];
		jp->max[i]            = rotData->max[i];
		jp->damping[i]        = rotData->damping[i];
		jp->preferredAngle[i] = rotData->preferredAngle[i];

		if (rotData->active[i]) jp->flags |= JNT_XACTIVE<<i;
		if (rotData->limit[i])  jp->flags |= JNT_XLIMITED<<i;
		if (rotData->ease[i])   jp->flags |= JNT_XEASE<<i;

		jp->pfInit = TRUE;
		}

	if (data3 != NULL) {
	  jp->SetSpring(0, data3->springOn[0] ? TRUE : FALSE);
	  jp->SetSpring(1, data3->springOn[1] ? TRUE : FALSE);
	  jp->SetSpring(2, data3->springOn[2] ? TRUE : FALSE);
	  jp->spring[0] = data3->spring[0];
	  jp->spring[1] = data3->spring[1];
	  jp->spring[2] = data3->spring[2];
	  jp->stens[0] = data3->springTension[0];
	  jp->stens[1] = data3->springTension[1];
	  jp->stens[2] = data3->springTension[2];
	  }
	}

BOOL LocalEulerRotation::GetIKJoints2(InitJointData2 *posData,InitJointData2 *rotData)
	{
	BOOL del = FALSE;
	JointParamsLocalEuler *jp   = (JointParamsLocalEuler*)GetProperty(PROPID_JOINTPARAMS);

	if (!jp) {
		jp = new JointParamsLocalEuler();
		del = TRUE;
		}

	// Init preferred angle to current controller value
	Quat qt(0.0f,0.0f,0.0f,1.0f);
	Point3 rot;
	if (!jp->pfInit) {
		GetValue(GetCOREInterface()->GetTime(), &qt, FOREVER, CTRL_ABSOLUTE);
		QuatToEuler(qt, rot, EULERTYPE_XYZ);		
		}

	for (int i=0; i<3; i++) {
		rotData->min[i]     = jp->min[i];
		rotData->max[i]     = jp->max[i];
		rotData->damping[i] = jp->damping[i];		
		rotData->active[i]  =  jp->flags & (JNT_XACTIVE<<i);
		rotData->limit[i]   =  jp->flags & (JNT_XLIMITED<<i);
		rotData->ease[i]    =  jp->flags & (JNT_XEASE<<i);

		if (jp->pfInit) {
			rotData->preferredAngle[i] = jp->preferredAngle[i];
		} else {
			rotData->preferredAngle[i] = rot[i];
			}
		}

	InitJointData3* data3 = DowncastToJointData3(rotData);
	if (data3 != NULL) {
	  data3->springOn[0] = jp->Spring(0) ? true : false;
	  data3->springOn[1] = jp->Spring(1) ? true : false;
	  data3->springOn[2] = jp->Spring(2) ? true : false;
	  data3->spring.Set(jp->spring[0], jp->spring[1], jp->spring[2]);
	  data3->springTension.Set(jp->stens[0], jp->stens[1], jp->stens[2]);
	}

	if (del) delete jp;
	return TRUE;
	}
