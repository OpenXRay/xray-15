/**********************************************************************
 *<
	FILE: mmanager.cpp

	DESCRIPTION: Motion capture manager

	CREATED BY: Rolf Berteig

	HISTORY: October 30, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
#include "mmanager.h"
#include "notify.h"
#include "midiman.h"

//--- A command mode for capturing ----------------------------------

class CaptureCMode : 
			public CommandMode, 
			public ChangeForegroundCallback,
			public MouseCallBack {
	public:
		BOOL valid, recording;		
		CaptureCMode() {valid=FALSE; recording=FALSE;}

		// Command mode
		int Class() {return ANIMATION_COMMAND;}
		int ID() {return 0x8f623bb4;}
		MouseCallBack *MouseProc(int *numPoints) {*numPoints=1;return this;}
		ChangeForegroundCallback *ChangeFGProc() {return this;}
		BOOL ChangeFG(CommandMode *oldMode) {if (oldMode!=this) return TRUE; else return FALSE;}
		void EnterMode() {}
		void ExitMode() {}
		
		// Mouse proc
		int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m) {return TRUE;}

		// FG proc
		BOOL IsValid() {return valid;}
		void Invalidate() {valid=FALSE;}
		void Validate() {valid=TRUE;}
		void callback(TimeValue t,IScene *scene) {
			
			// RB 10/27/2000: We always want to flag animated - even in test mode.
			//if (recording) scene->FlagFGAnimated(t);
			scene->FlagFGAnimated(t);

			for (int i=0; i<theMM.cont.Count(); i++) {
				if (theMM.cont[i]->GetRecordState()) {
					theMM.cont[i]->FlagDependents(t);
					}
				}
			}
	};
CaptureCMode theCaptureMode;



//--- MCControl Base Class ------------------------------------------

MCControl::MCControl()
	{
	flags  = 0;
	cont   = NULL;
	selSet = -1;
	theMM.Register(this);	
	}

MCControl::~MCControl()
	{
	theMM.UnRegister(this);
	}

TSTR MCControl::SubAnimName(int i) 
	{
	if (trackName.length()) {
		TSTR buf = GetString(IDS_RB_DATA);
		buf = buf + TSTR(_T(" (")) + trackName + TSTR(_T(")"));
		return buf;
	} else {
		return GetString(IDS_RB_DATA);
		}
	}
	
RefTargetHandle MCControl::GetReference(int i) 
	{
	switch (i) {
		case 0:  return cont;
		default: return GetDeviceBinding(i-1);
		}
	}

void MCControl::SetReference(int i, RefTargetHandle rtarg) 
	{
	switch (i) {
		case 0:  cont = (Control*)rtarg; break;
		default: SetDeviceBinding(i-1,(MCDeviceBinding*)rtarg);
		}
	}

BOOL MCControl::AssignController(Animatable *control,int subAnim)
	{
	ReplaceReference(0,(RefTargetHandle)control);
	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE,TREE_VIEW_CLASS_ID,FALSE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
	return TRUE;
	}

void MCControl::Copy(Control *from)
	{
	if (from->IsKeyable()) {
		ReplaceReference(0,from);
		}
	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE,TREE_VIEW_CLASS_ID,FALSE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);	
	}

void MCControl::AddNewKey(TimeValue t,DWORD flags)
	{
	cont->AddNewKey(t,flags);
	}

int MCControl::NumKeys()
	{
	return cont->NumKeys();
	}

TimeValue MCControl::GetKeyTime(int index)
	{
	return cont->GetKeyTime(index);
	}

void MCControl::CopyKeysFromTime(
		TimeValue src,TimeValue dst,DWORD flags)
	{
	cont->CopyKeysFromTime(src,dst,flags);
	}

BOOL MCControl::IsKeyAtTime(TimeValue t,DWORD flags)
	{
	return cont->IsKeyAtTime(t,flags);
	}

BOOL MCControl::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
	{
	return cont->GetNextKeyTime(t,flags,nt);
	}

void MCControl::DeleteKeyAtTime(TimeValue t)
	{
	cont->DeleteKeyAtTime(t);
	}

int MCControl::GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags)
	{
	return cont->GetKeyTimes(times,range,flags);
	}

int MCControl::GetKeySelState(BitArray &sel,Interval range,DWORD flags)
	{
	return cont->GetKeySelState(sel,range,flags);
	}

void MCControl::SelectKeys(TrackHitTab& sel, DWORD flags)
	{
	cont->SelectKeys(sel, flags);
	}

void MCControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if (IsLiveOn() || IsRecordOn()) {
		GetValueLive(t,val,method);
		valid.SetInstant(t);
	} else {
		cont->GetValue(t,val,valid,method);
		}
	}

void MCControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	cont->SetValue(t,val,commit,method);
	}

void MCControl::CommitValue(TimeValue t)
	{
	cont->CommitValue(t);
	}

void MCControl::RestoreValue(TimeValue t)
	{
	cont->RestoreValue(t);
	}

void MCControl::EnumIKParams(IKEnumCallback &callback)
	{
	cont->EnumIKParams(callback);
	}

BOOL MCControl::CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags)
	{
	return cont->CompDeriv(t,ptm,derivs,flags);
	}

float MCControl::IncIKParam(TimeValue t,int index,float delta)
	{
	return cont->IncIKParam(t,index,delta);
	}

void MCControl::ClearIKParam(Interval iv,int index)
	{
	cont->ClearIKParam(iv,index);
	}

void MCControl::EnableORTs(BOOL enable)
	{
	cont->EnableORTs(enable);
	}


void MCControl::BeginEditParams( 
		IObjParam *ip, ULONG flags,Animatable *prev)
	{}

void MCControl::EndEditParams( 
		IObjParam *ip, ULONG flags,Animatable *next)
	{}

void MCControl::MCControlClone(MCControl *src,RemapDir& remap)
	{
	ReplaceReference(0,remap.CloneRef(src->cont));
	for (int i=0; i<src->NumDeviceBindings(); i++) {
		MCDeviceBinding *b = src->GetDeviceBinding(i);
		if (b) {
			ReplaceReference(i+1,remap.CloneRef(b));
			}
		}
	}


void MCControl::LiveOn()    
	{
	flags |=  MCC_LIVE_ON;
	for (int i=0; i<NumDeviceBindings(); i++) {
		if (GetDeviceBinding(i)) GetDeviceBinding(i)->BeginActivate();
		}
	}

void MCControl::RecordOn(BOOL reset)  
	{
	flags |=  MCC_RECORD_ON;
	for (int i=0; i<NumDeviceBindings(); i++) {
		if (GetDeviceBinding(i)) GetDeviceBinding(i)->BeginActivate(reset);
		}
	}


void MCControl::RecordOff() 
	{
	flags &= ~MCC_RECORD_ON;
	for (int i=0; i<NumDeviceBindings(); i++) {
		if (GetDeviceBinding(i)) GetDeviceBinding(i)->EndActivate();
		}
	}

void MCControl::LiveOff()   
	{
	flags &= ~MCC_LIVE_ON;
	for (int i=0; i<NumDeviceBindings(); i++) {
		if (GetDeviceBinding(i)) GetDeviceBinding(i)->EndActivate();
		}
	}


#define FLAGS_CHUNK_ID				0x0100
#define SELSETINDEX_CHUNK_ID		0x0120
#define TRACKNAME_CHUNK_ID			0x0130

IOResult MCControl::Save(ISave *isave)
	{
	ULONG nb;	

	isave->BeginChunk(FLAGS_CHUNK_ID);
	isave->Write(&flags,sizeof(flags),&nb);
	isave->EndChunk();

	isave->BeginChunk(SELSETINDEX_CHUNK_ID);
	isave->Write(&selSet,sizeof(selSet),&nb);
	isave->EndChunk();

	isave->BeginChunk(TRACKNAME_CHUNK_ID);
	isave->WriteWString(trackName);
	isave->EndChunk();

	return IO_OK;
	}

IOResult MCControl::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case FLAGS_CHUNK_ID:
				res=iload->Read(&flags,sizeof(flags),&nb);
				break;

			case SELSETINDEX_CHUNK_ID:
				res=iload->Read(&selSet,sizeof(selSet),&nb);
				break;

			case TRACKNAME_CHUNK_ID: {
				TCHAR *ptr = NULL;
				res=iload->ReadWStringChunk(&ptr);
				trackName = TSTR(ptr);
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
		
	return IO_OK;
	}


class InSceneDepProc : public DependentEnumProc {
	public:
		int ct;
		MCControl *me;
		InSceneDepProc(MCControl *m) {ct=0;me=m;}
		int proc(ReferenceMaker *rmaker) {	  
			if (rmaker!=me && rmaker->SuperClassID()!=0) {
				ct++;
				}
			return 0;
			}
	};

BOOL MCControl::ControllerInScene()
	{
	InSceneDepProc proc(this);
	EnumDependents(&proc);
	return proc.ct>0;
	}

void MCControl::RefDeleted()
	{
	if (!ControllerInScene()) theMM.UnRegister(this);
	}

RefResult MCControl::AutoDelete()
	{
	RefDeleted();
	return ReferenceTarget::AutoDelete();
	}

void MCControl::RefAdded(RefMakerHandle rm)
	{
	if (ControllerInScene() &&
		!theMM.IsRegistered(this)) 
		 theMM.Register(this);
	}

void MCControl::RefDeletedUndoRedo()
	{
	if (!ControllerInScene()) theMM.UnRegister(this);
	}

void MCControl::RefAddedUndoRedo(RefMakerHandle rm)
	{
	if (ControllerInScene() &&
		!theMM.IsRegistered(this)) 
		 theMM.Register(this);
	}

void MCControl::Accumulate(TimeValue t)
	{
	for (int i=0; i<NumDeviceBindings(); i++) {
		if (GetDeviceBinding(i)) GetDeviceBinding(i)->Accumulate(t);
		}
	}


//--- MotionManager Descriptor ----------------------------------------------------

MotionManager theMM;

class MotionManClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theMM;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_MOTIONMAN);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(MOTION_MANAGER_CLASS_ID,0);}
	const TCHAR* 	Category() {return _T("");}
	BOOL			NeedsToSave() {return TRUE;}
	IOResult 		Save(ISave *isave) {return theMM.Save(isave);}
	IOResult 		Load(ILoad *iload) {return theMM.Load(iload);}
	};

static MotionManClassDesc motionManDesc;
ClassDesc* GetMotionManDesc() {return &motionManDesc;}


//--- MotionManager ----------------------------------------------------

static INT_PTR CALLBACK MotionManDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void SubClassNamedSel(HWND hNameSel);

static DWORD MIDIInProc(
		HMIDIIN hMidiIn,
		UINT wMsg, DWORD dwInstance,
		DWORD dwParam1, DWORD dwParam2);

static StartStopParams ssPresets[] = {
	StartStopParams(STARTSTOP_MEDIACONTROLSTATION,IDS_RB_MEDIACONTROLSTATION,15,21,22,23)
	};
#define STARTSTOP_PRESETS	1

int StartStopParams::GetChannel() {
	if (presetType==STARTSTOP_CUSTOM) return channel;
	else return ssPresets[presetType-1].channel;
	}
int StartStopParams::GetStop() {
	if (presetType==STARTSTOP_CUSTOM) return stop;
	else return ssPresets[presetType-1].stop;
	}
int StartStopParams::GetPlay() {
	if (presetType==STARTSTOP_CUSTOM) return play;
	else return ssPresets[presetType-1].play;
	}
int StartStopParams::GetRecord() {
	if (presetType==STARTSTOP_CUSTOM) return record;
	else return ssPresets[presetType-1].record;
	}


static void NotifyReset(void *param, NotifyInfo *info)
	{	
	if (!info || info->intcode!=NOTIFY_SYSTEM_POST_RESET) return;
	for (int i=0; i<theMM.selSets.Count(); i++) {
		delete theMM.selSets[i];
		}
	theMM.selSets.Resize(0);
	}

MotionManager::MotionManager()
	{
	RegisterNotification(NotifyReset, NULL, NOTIFY_SYSTEM_POST_RESET);

	hTrackIcons = NULL;
	hButtonIcons = NULL;
	InitializeCriticalSection(&csect);	

	listBuilt = FALSE;
	live      = FALSE;
	recording = FALSE;

	pre = 0;
	in  = 0;
	out = 16000;

	reduce        = FALSE;
	doubleSample  = FALSE;
	reduceThresh  = 1.0f;
	livePreRoll   = FALSE;
	playWhileTest = FALSE;
		
	ssParams = StartStopParams(STARTSTOP_MEDIACONTROLSTATION,IDS_RB_MEDIACONTROLSTATION,15,21,22,23);
	startStopEnabled = FALSE;
	recDown          = FALSE;
	playDown         = FALSE;

	playSpeed = 1;
	}

MotionManager::~MotionManager()
	{
	UnRegisterNotification(NotifyReset, NULL);
	DeleteCriticalSection(&csect);
	ImageList_Destroy(hTrackIcons);
	for (int i=0; i<selSets.Count(); i++) {
		delete selSets[i];
		}
	}

void MotionManager::SetupDeviceList()
	{
	if (!listBuilt) {
		listBuilt = TRUE;
		SubClassList *sublist = GetCOREInterface()->GetDllDir().ClassDir().
			GetClassList(MOT_CAP_DEV_CLASS_ID);
		for (int i=0; i<sublist->Count(ACC_ALL); i++) {
			if (!(*sublist)[i].IsPublic()) continue;
			MCInputDevice *d = (MCInputDevice*)(*sublist)[i].CD()->Create();
			device.Append(1,&d);
			}
		}
	}

void MotionManager::InitUI()
	{
	if (!hTrackIcons) {
		hTrackIcons = ImageList_Create(10, 10, TRUE, 2, 0);
		HBITMAP hBitmap, hMask;
		hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_MM_ICONS));
		hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_MM_ICONSMASK));
		ImageList_Add(hTrackIcons,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);

		hButtonIcons = ImageList_Create(16, 16, TRUE, 1, 0);		
		hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BUTTONS));
		hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BUTTONS_MASK));
		ImageList_Add(hButtonIcons,hBitmap,hMask);
		DeleteObject(hBitmap);
		DeleteObject(hMask);
		}
	}

BOOL MotionManager::IsRegistered(MCControl *c)
	{
	for (int i=0; i<cont.Count(); i++) {
		if (cont[i]==c) return TRUE;
		}
	return FALSE;
	}

void MotionManager::Register(MCControl *c)
	{
	cont.Append(1,&c);
	if (hPanel) SetupList();
	}

void MotionManager::UnRegister(MCControl *c)
	{
	for (int i=0; i<cont.Count(); i++) {
		if (cont[i]==c) {
			cont.Delete(i,1);
			break;
			}
		}
	if (hPanel) SetupList();
	}


class FindSubName : public DependentEnumProc {
	public:		
		Animatable *anim;
		TSTR name;
		FindSubName(Animatable *a) {anim=a;}
		int proc(ReferenceMaker *rmaker) {
			for (int i=0; i<rmaker->NumSubs(); i++) {
				if (rmaker->SubAnim(i)==anim) {
					name = rmaker->SubAnimName(i);
					return 1;
					}
				}
			return 0;
			}
	};

class FindParentName : public DependentEnumProc {
	public:				
		TSTR name;	
		ReferenceMaker *me;
		FindParentName(ReferenceMaker *r) {me=r;}
		int proc(ReferenceMaker *rmaker) {
			switch (rmaker->SuperClassID()) {
				case MATERIAL_CLASS_ID:
				case TEXMAP_CLASS_ID:
				case SOUNDOBJ_CLASS_ID:
				case ATMOSPHERIC_CLASS_ID:
					rmaker->GetClassName(name);
					return 1;

				default:
					if (rmaker!=me) {
						FindParentName fpn(rmaker);
						rmaker->EnumDependents(&fpn);
						if (fpn.name.Length()) {
							name = fpn.name;
							return 1;
							}
						}
					break;

				}
			return 0;
			}
	};

TSTR MotionManager::ControlName(MCControl *c)
	{
	if (c->trackName.length()) {
		return c->trackName;
		}

	TSTR parent, name;
	FindSubName fsn(c);
	
	// Try to find some names
	c->NotifyDependents(FOREVER,(PartID)&parent,REFMSG_GET_NODE_NAME);
	c->EnumDependents(&fsn);
	
	// Not within a node... look for a material or something
	if (!parent.Length()) {
		FindParentName fpn(c);
		c->EnumDependents(&fpn);
		parent = fpn.name;
		}

	if (parent.Length() && fsn.name.Length()) {
		name = parent + TSTR(_T("\\")) + fsn.name;
	} else if (parent.Length()) {
		name = parent;
	} else if (fsn.name.Length()) {
		name = fsn.name;
	} else {
		c->GetClassName(name);
		}

	return name;
	}

TSTR MotionManager::ControlName(int index)
	{
	if (index<0 || index>=cont.Count()) return _T("");
	else return ControlName(cont[index]);
	}

void MotionManager::ToggleRecordMode(int index)
	{
	if (index>=0 && index<cont.Count()) {
		cont[index]->SetRecordState(!cont[index]->GetRecordState());
		}
	}


void MotionManager::BeginEditParams(
		Interface *ip,IUtil *iu)
	{	
	ssParams.channel    = GetMotionCaptureINISetting(MCAP_INI_CHANNEL);
	ssParams.presetType = GetMotionCaptureINISetting(MCAP_INI_PRESET);
	ssParams.stop       = GetMotionCaptureINISetting(MCAP_INI_STOP);
	ssParams.play       = GetMotionCaptureINISetting(MCAP_INI_PLAY);
	ssParams.record     = GetMotionCaptureINISetting(MCAP_INI_RECORD);
	startStopEnabled    = GetMotionCaptureINISetting(MCAP_INI_SSENABLE);

	this->ip = ip;
	this->iu = iu;
	SetupDeviceList();
	for (int i=0; i<device.Count(); i++) {
		device[i]->UtilityStarted(this);
		}
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_MANAGER_PARAMS),
		MotionManDlgProc,
		GetString(IDS_RB_MOTIONMAN),
		0);		

	if (startStopEnabled) {
		MIDIMan_Open(MIDIInProc,0,10);
		MIDIMan_Start();
		}
	}
	
void MotionManager::EndEditParams(
		Interface *ip,IUtil *iu)
	{
	if (startStopEnabled) {
		MIDIMan_Stop();
		MIDIMan_Close(MIDIInProc,0);
		}

	for (int i=0; i<device.Count(); i++) {
		device[i]->UtilityStopped(this);
		}
	this->ip = NULL;
	this->iu = NULL;	
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}


void MotionManager::SetupList()
	{
	TSTR max;

	SendDlgItemMessage(hPanel,IDC_MM_TRACKS,LB_RESETCONTENT,0,0);

	for (int i=0; i<cont.Count(); i++) {
		TSTR name = ControlName(cont[i]);
		SendDlgItemMessage(
			hPanel,IDC_MM_TRACKS,LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
		if (name.Length()>max.Length()) max = name;
		}
		
	HWND hCont = GetDlgItem(hPanel,IDC_MM_TRACKS);
	HDC hdc = GetDC(hCont);
	SIZE size;
	GetTextExtentPoint(hdc,max,max.Length(),&size);
	SendDlgItemMessage(hPanel,IDC_MM_TRACKS,LB_SETHORIZONTALEXTENT,size.cx+12,0);
	ReleaseDC(hCont,hdc);			
	}

void MotionManager::Init(HWND hWnd)
	{
	InitUI();
	hPanel = hWnd;
		
	ISpinnerControl *spin = 
		GetISpinner(GetDlgItem(hPanel,IDC_MM_PREROLLSPIN));
	spin->SetLimits(TIME_NegInfinity, TIME_PosInfinity, FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hPanel,IDC_MM_PREROLL), EDITTYPE_TIME);
	spin->SetValue(pre,FALSE);
	ReleaseISpinner(spin);
	
	spin = GetISpinner(GetDlgItem(hPanel,IDC_MM_INSPIN));
	spin->SetLimits(TIME_NegInfinity, TIME_PosInfinity, FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hPanel,IDC_MM_IN), EDITTYPE_TIME);
	spin->SetValue(in,FALSE);
	ReleaseISpinner(spin);
	
	spin = GetISpinner(GetDlgItem(hPanel,IDC_MM_OUTSPIN));
	spin->SetLimits(TIME_NegInfinity, TIME_PosInfinity, FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hPanel,IDC_MM_OUT), EDITTYPE_TIME);
	spin->SetValue(out,FALSE);
	ReleaseISpinner(spin);

	ICustButton *but = GetICustButton(GetDlgItem(hPanel,IDC_MM_START));
	but->SetType(CBT_CHECK);
	but->SetHighlightColor(RED_WASH);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hPanel,IDC_MM_LIVE));
	but->SetType(CBT_CHECK);
	but->SetHighlightColor(RED_WASH);
	ReleaseICustButton(but);	
	
	but = GetICustButton(GetDlgItem(hPanel,IDC_MM_DELSET));
	but->SetImage(hButtonIcons,0,0,0,0,16,16);
	ReleaseICustButton(but);		

	CheckDlgButton(hPanel,IDC_SAMPLES1,!doubleSample);
	CheckDlgButton(hPanel,IDC_SAMPLES2, doubleSample);
	CheckDlgButton(hPanel,IDC_REDUCE_KEYS, reduce);
	CheckDlgButton(hPanel,IDC_MM_LIVEPREROLL, livePreRoll);
	CheckDlgButton(hPanel,IDC_MM_PLAYTEST, playWhileTest);
	CheckDlgButton(hPanel,IDC_MM_STARTSTOPENABLE,startStopEnabled);

	spin = GetISpinner(GetDlgItem(hPanel,IDC_REDUCE_THRESHSPIN));
	spin->SetLimits(0.0f, float(999999), FALSE);
	spin->SetScale(0.1f);
	spin->LinkToEdit(GetDlgItem(hPanel,IDC_REDUCE_THRESH), EDITTYPE_FLOAT);
	spin->SetValue(reduceThresh,FALSE);
	ReleaseISpinner(spin);
	
	SubClassNamedSel(GetDlgItem(hPanel,IDC_MM_SETS));
	for (int i=0; i<selSets.Count(); i++) {
		SendMessage(GetDlgItem(hPanel,IDC_MM_SETS), CB_ADDSTRING, 0, 
			(LPARAM)(TCHAR*)*(selSets[i]));
		}

	SetupList();
	}

void MotionManager::Destroy(HWND hWnd)
	{
	// Save settings
	ISpinnerControl *spin = 
		GetISpinner(GetDlgItem(hPanel,IDC_MM_PREROLLSPIN));	
	pre   = spin->GetIVal();
	ReleaseISpinner(spin);
	spin  = GetISpinner(GetDlgItem(hPanel,IDC_MM_INSPIN));	
	in    = spin->GetIVal();
	ReleaseISpinner(spin);
	spin  = GetISpinner(GetDlgItem(hPanel,IDC_MM_OUTSPIN));	
	out   = spin->GetIVal();
	ReleaseISpinner(spin);	
	}

void MotionManager::DrawTrack(
		HDC hdc,int index,BOOL sel,RECT rect)
	{
	if (index>=0) {
		TSTR name = ControlName(index);	
		TextOut(hdc,rect.left+10,rect.top,name,name.Length());
		ImageList_Draw(
			hTrackIcons,
			cont[index]->GetRecordState(),
			hdc, rect.left, rect.top+2, ILD_NORMAL);
		}
	}

void MotionManager::CheckMessages()
	{
	MSG msg;

	// Make sure any mouse messages go to us.
	while (PeekMessage(&msg,NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE)) {
		BOOL trap = TRUE;

		// Don't trap left clicks to record controlls.
		if (GetParent(msg.hwnd)==hPanel && msg.message!=WM_RBUTTONDOWN) {
			int id = GetWindowLongPtr(msg.hwnd,GWL_ID);
			if (id==IDC_MM_START || id==IDC_MM_STOP || id==IDC_MM_LIVE) {
				trap = FALSE;
				}
			}
		
		if (msg.message==WM_RBUTTONDOWN) {
			Stop();
			return;
			}

		//if (trap) msg.hwnd = hPanel;
		if (!trap) {
			ip->TranslateAndDispatchMAXMessage(msg);
			}
		}
	while (PeekMessage(&msg,hPanel,0,0,PM_REMOVE)) {			
		ip->TranslateAndDispatchMAXMessage(msg);
		}
	UpdateWindow(hPanel);
	}

void MotionManager::TrackParams(HWND hWnd,int index)
	{
	if (index>=0 && index<cont.Count()) {
		cont[index]->EditTrackParams(
			0, NULL, ControlName(index), hWnd, (IObjParam*)ip, 0);
		}
	}

void MotionManager::MidiNote(int channel, int note)
	{

	}

void MotionManager::NewSelSet()
	{
	int set = selSets.Count();
	TCHAR buf[256];
	GetDlgItemText(hPanel,IDC_MM_SETS,buf,256);
	TSTR *name = new TSTR(buf);
	selSets.Append(1,&name);

	for (int i=0; i<cont.Count(); i++) {
		if (cont[i]->GetRecordState()) {
			cont[i]->selSet = set;
			}
		}
	}

void MotionManager::DeleteSelSet()
	{
	LRESULT res;	
 	HWND hNameSel = GetDlgItem(hPanel,IDC_MM_SETS);
	TCHAR buf[256];
	GetDlgItemText(hPanel,IDC_MM_SETS,buf,256);
	if (CB_ERR != 
		(res = SendMessage(hNameSel, CB_FINDSTRINGEXACT, 0, (LPARAM)buf))) {		
		SendMessage(hNameSel, CB_DELETESTRING, res, 0);
		ClearSelSetName();
		
		selSets.Delete(res,1);

		// Offset set indices
		for (int i=0; i<cont.Count(); i++) {
			if (cont[i]->selSet==res) cont[i]->selSet = -1;
			if (cont[i]->selSet>res) cont[i]->selSet--;
			}
		}	
	}

void MotionManager::ClearSelSetName()
	{				
	SetDlgItemText(hPanel,IDC_MM_SETS,NULL);
	}

void MotionManager::ActivateSelSet(int i)
	{
	for (int j=0; j<cont.Count(); j++) {
		if (cont[j]->selSet == i) {
			cont[j]->SetRecordState(TRUE);
		} else {
			cont[j]->SetRecordState(FALSE);
			}
		}
	InvalidateRect(GetDlgItem(hPanel,IDC_MM_TRACKS),NULL,TRUE);
	}


static WNDPROC NamedSelWndProc = NULL;

static LRESULT CALLBACK NamedSelSubWndProc(
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	switch (message) {
		case WM_SETFOCUS:  
			DisableAccelerators(); 
			if (theMM.ip) theMM.ip->UnRegisterDlgWnd(theMM.hPanel);
			break;		
		case WM_KILLFOCUS: 
			EnableAccelerators();  
			if (theMM.ip) theMM.ip->RegisterDlgWnd(theMM.hPanel);
			break;
					
		case WM_CHAR:
			if (wParam==13) {
				TCHAR buf[256];
				HWND hCombo = GetParent(hWnd);						
				LRESULT res;
				GetWindowText(hWnd,buf,256);				
				if (CB_ERR != 
					(res = SendMessage(hCombo, CB_FINDSTRINGEXACT, 0, (LPARAM)buf))) {
					// String is already in the list.					
					SendMessage(hCombo, CB_SETCURSEL, res, 0);
					SendMessage(GetParent(hCombo), WM_COMMAND, 
						MAKEWPARAM(GetWindowLongPtr(hCombo,GWL_ID),CBN_SELCHANGE),
						(LPARAM)hCombo );
				} else {
					SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
					SendMessage(hWnd, EM_SETSEL, 0, (WPARAM)((INT)-1));
					theMM.NewSelSet();
					}
				return 0;
				}
			break;
		}
	return CallWindowProc(NamedSelWndProc,hWnd,message,wParam,lParam);
	}

static BOOL CALLBACK EnumChildren(HWND hwnd, LPARAM lParam)
	{	
	NamedSelWndProc = (WNDPROC)SetWindowLongPtr(hwnd,GWLP_WNDPROC,(LONG_PTR)NamedSelSubWndProc );
	return FALSE;
	}

static void SubClassNamedSel(HWND hNameSel)
	{
	EnumChildWindows(hNameSel,EnumChildren,0);
	}


static INT_PTR CALLBACK MotionManDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theMM.Init(hWnd);	 		
			break;
				
		case WM_DESTROY:
			theMM.Destroy(hWnd);
			break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theMM.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;
		
		case WM_RBUTTONDOWN:
			theMM.Stop();
			break;

		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl *)lParam;			
			switch (LOWORD(wParam)) {
				case IDC_MM_PREROLLSPIN:    theMM.pre = spin->GetIVal(); break;
				case IDC_MM_INSPIN:		    theMM.in = spin->GetIVal(); break;
				case IDC_MM_OUTSPIN:        theMM.out = spin->GetIVal(); break;
				case IDC_REDUCE_THRESHSPIN: theMM.reduceThresh = spin->GetFVal(); break;
				}
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_MM_PLAYTEST:
					theMM.playWhileTest = IsDlgButtonChecked(hWnd,IDC_MM_PLAYTEST);
					break;

				case IDC_MM_LIVEPREROLL:
					theMM.livePreRoll = IsDlgButtonChecked(hWnd,IDC_MM_LIVEPREROLL);
					break;

				case IDC_REDUCE_KEYS:
					theMM.reduce = IsDlgButtonChecked(hWnd,IDC_REDUCE_KEYS);
					break;

				case IDC_MM_START:
					theMM.Start();
					break;

				case IDC_MM_STOP:
					theMM.Stop();
					break;

				case IDC_MM_LIVE:
					theMM.Live();
					break;

				case IDC_MM_TRACKS:
					if (HIWORD(wParam)==LBN_SELCHANGE) {
						int index = SendDlgItemMessage(
							hWnd, IDC_MM_TRACKS, LB_GETCURSEL, 0, 0);
						if (index!=LB_ERR) {
							theMM.ToggleRecordMode(index);
							InvalidateRect(GetDlgItem(hWnd,IDC_MM_TRACKS),NULL,TRUE);
							}
						theMM.ClearSelSetName();
					} else if (HIWORD(wParam)==LBN_DBLCLK) {
						int index = SendDlgItemMessage(
							hWnd, IDC_MM_TRACKS, LB_GETCURSEL, 0, 0);
						if (index!=LB_ERR) {
							theMM.ToggleRecordMode(index);
							InvalidateRect(GetDlgItem(hWnd,IDC_MM_TRACKS),NULL,TRUE);
							theMM.TrackParams(hWnd,index);
							}
						}
					break;

				case IDC_MM_ALL:
				case IDC_MM_INVERT:
				case IDC_MM_NONE: {
					for (int i=0; i<theMM.cont.Count(); i++) {
						if (LOWORD(wParam)==IDC_MM_ALL)	   theMM.cont[i]->SetRecordState(TRUE);
						if (LOWORD(wParam)==IDC_MM_INVERT) theMM.ToggleRecordMode(i);
						if (LOWORD(wParam)==IDC_MM_NONE)   theMM.cont[i]->SetRecordState(FALSE);
						}
					InvalidateRect(GetDlgItem(hWnd,IDC_MM_TRACKS),NULL,TRUE);
					theMM.ClearSelSetName();
					break;
					}

				case IDC_MM_SETS:
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						int index = SendDlgItemMessage(
							hWnd, IDC_MM_SETS, CB_GETCURSEL, 0, 0);
						if (index!=LB_ERR) {
							theMM.ActivateSelSet(index);
							}
						}
					break;

				case IDC_MM_DELSET:
					theMM.DeleteSelSet();
					break;

				case IDC_MM_STARTSTOPENABLE:
					theMM.SetStartStopEnable(IsDlgButtonChecked(hWnd,IDC_MM_STARTSTOPENABLE));
					break;

				case IDC_MM_AUTOSTART:
					theMM.SetupStartStop();
					break;
				}
			break;

		case WM_MEASUREITEM: {
			MEASUREITEMSTRUCT *mi = (LPMEASUREITEMSTRUCT)lParam;
			TSTR name = theMM.ControlName(mi->itemID);						
			HWND hCont = GetDlgItem(hWnd,IDC_MM_TRACKS);
			HDC hdc = GetDC(hCont);
			SIZE size;
			GetTextExtentPoint(hdc,name,name.Length(),&size);
			mi->itemWidth  = size.cx + 12;
			mi->itemHeight = size.cy - 2;
			ReleaseDC(hCont,hdc);			
			break;
			}

		case WM_DRAWITEM: {
			DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT*)lParam;
			theMM.DrawTrack(di->hDC,di->itemID,
				di->itemState,di->rcItem);
			break;
			}

		default:
			return FALSE;
		}
	return TRUE; 
	}



static INT_PTR CALLBACK SelectBindingDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	static int *index=NULL;

	switch (msg) {
		case WM_INITDIALOG: {
			index = (int*)lParam;
			SendDlgItemMessage(hWnd,IDC_DEVICE_LIST,
				LB_ADDSTRING,0,(LPARAM)GetString(IDS_RB_NONE));			
			for (int i=0; i<theMM.device.Count(); i++) {
				TSTR name = theMM.device[i]->DeviceName();
				SendDlgItemMessage(hWnd,IDC_DEVICE_LIST,
					LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
				}
			break;
			}
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_DEVICE_LIST:
					if (HIWORD(wParam)!=LBN_DBLCLK) break;
					// fallthrough

				case IDOK:
					*index = SendDlgItemMessage(
							hWnd, IDC_DEVICE_LIST, LB_GETCURSEL, 0, 0);
					EndDialog(hWnd,1);
					break;
					
				case IDCANCEL:
					EndDialog(hWnd,0);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}


MCDeviceBinding *MotionManager::SelectBinding(HWND hWnd, BOOL &cancel)
	{
	int index;
	SetupDeviceList();
	if (DialogBoxParam(
			hInstance,
			MAKEINTRESOURCE(IDD_SELBINDING),
			hWnd,
			SelectBindingDlgProc,
			(LPARAM)&index)) {

		if (index>=0 && index-1<device.Count()) {			
			cancel = FALSE;
			if (index==0) return NULL;
			return device[index-1]->CreateBinding();
			}
		}
	cancel = TRUE;
	return NULL;	
	}

static void SetupParams(HWND hWnd)
	{
	ISpinnerControl *spin;			
	
	spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_CHANNELSPIN));	
	spin->SetValue(theMM.ssParams.GetChannel()+1,FALSE);
	if (theMM.ssParams.presetType!=STARTSTOP_CUSTOM) 
		 spin->Disable();
	else spin->Enable();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_STOPSPIN));	
	spin->SetValue(theMM.ssParams.GetStop(),FALSE);
	if (theMM.ssParams.presetType!=STARTSTOP_CUSTOM) 
		 spin->Disable();
	else spin->Enable();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_PLAYSPIN));	
	spin->SetValue(theMM.ssParams.GetPlay(),FALSE);
	if (theMM.ssParams.presetType!=STARTSTOP_CUSTOM) 
		 spin->Disable();
	else spin->Enable();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_RECORDSPIN));	
	spin->SetValue(theMM.ssParams.GetRecord(),FALSE);
	if (theMM.ssParams.presetType!=STARTSTOP_CUSTOM) 
		 spin->Disable();
	else spin->Enable();
	ReleaseISpinner(spin);
	}

static INT_PTR CALLBACK StartStopDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	static int oldType;

	switch (msg) {
		case WM_INITDIALOG: {
			CenterWindow(hWnd,GetParent(hWnd));
			
			oldType = theMM.ssParams.presetType;
			SendDlgItemMessage(hWnd,IDC_MIDI_PRESETS,CB_ADDSTRING,
				0, (LPARAM)GetString(IDS_RB_CUSTOM));
			for (int i=0; i<STARTSTOP_PRESETS; i++) {
				SendDlgItemMessage(hWnd,IDC_MIDI_PRESETS,CB_ADDSTRING,
					0, (LPARAM)GetString(ssPresets[i].devID));
				}
			SendDlgItemMessage(hWnd,IDC_MIDI_PRESETS,CB_SETCURSEL,
				theMM.ssParams.presetType,0);

			ISpinnerControl *spin;
			
			spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_CHANNELSPIN));
			spin->SetLimits(1,16,FALSE);
			spin->SetScale(0.05f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_CHANNEL),EDITTYPE_INT);			
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_STOPSPIN));
			spin->SetLimits(0,127,FALSE);
			spin->SetScale(0.1f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_STOP),EDITTYPE_INT);			
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_PLAYSPIN));
			spin->SetLimits(0,127,FALSE);
			spin->SetScale(0.1f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_PLAY),EDITTYPE_INT);			
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_RECORDSPIN));
			spin->SetLimits(0,127,FALSE);
			spin->SetScale(0.1f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_MIDI_RECORD),EDITTYPE_INT);			
			ReleaseISpinner(spin);

			SetupParams(hWnd);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_MIDI_PRESETS:
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						int res = SendDlgItemMessage(hWnd,IDC_MIDI_PRESETS,
							CB_GETCURSEL,0,0);
						if (res!=CB_ERR) {
							theMM.ssParams.presetType = res;
							SetupParams(hWnd);
							}
						}
					break;

				case IDOK:
					if (theMM.ssParams.presetType==STARTSTOP_CUSTOM) {
						ISpinnerControl *spin;

						spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_CHANNELSPIN));
						theMM.ssParams.channel = spin->GetIVal()-1;
						ReleaseISpinner(spin);

						spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_STOPSPIN));
						theMM.ssParams.stop = spin->GetIVal();
						ReleaseISpinner(spin);

						spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_PLAYSPIN));
						theMM.ssParams.play = spin->GetIVal();
						ReleaseISpinner(spin);

						spin = GetISpinner(GetDlgItem(hWnd,IDC_MIDI_RECORDSPIN));
						theMM.ssParams.record = spin->GetIVal();
						ReleaseISpinner(spin);
						}

					EndDialog(hWnd,1);
					break;					

				case IDCANCEL:
					theMM.ssParams.presetType = oldType;
					EndDialog(hWnd,0);
					break;
				}
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}

void MotionManager::SetupStartStop()
	{
	if (DialogBox(
			hInstance,
			MAKEINTRESOURCE(IDD_MANAGER_STARTSTOP),
			hPanel,
			StartStopDlgProc)) {
		
		SetMotionCaptureINISetting(MCAP_INI_CHANNEL,ssParams.channel );
		SetMotionCaptureINISetting(MCAP_INI_PRESET,ssParams.presetType );
		SetMotionCaptureINISetting(MCAP_INI_STOP,ssParams.stop );
		SetMotionCaptureINISetting(MCAP_INI_PLAY,ssParams.play );
		SetMotionCaptureINISetting(MCAP_INI_RECORD,ssParams.record );
		}
	}

static DWORD MIDIInProc(
		HMIDIIN hMidiIn,
		UINT wMsg, DWORD dwInstance,
		DWORD dwParam1, DWORD dwParam2)
	{
	switch (wMsg) {
		case MIM_DATA: {
			int ch = MIDI_CHANNEL(dwParam1);
			int nn = MIDI_NOTENUMBER(dwParam1);			
			int event = MIDI_EVENT(dwParam1);
			int vel = MIDI_VELOCITY(dwParam1);
					
			if (ch!=theMM.ssParams.GetChannel()) break;
			if (event!=MIDI_CONTROLCHANGE) break;

			if ((theMM.live || theMM.recording) &&
				(nn==theMM.ssParams.GetStop())) {
				PostMessage(theMM.hPanel,WM_COMMAND,IDC_MM_STOP,0);
				return MIDIPROC_PROCESSED;
				}

			if (nn==theMM.ssParams.GetPlay()) {
				if (vel==0) {
					theMM.playDown = FALSE;
					if (theMM.recDown) {
						PostMessage(theMM.hPanel,WM_COMMAND,IDC_MM_START,0);
					} else {
						break;
						}
				} else {
					theMM.playDown = TRUE;
					}
				return MIDIPROC_PROCESSED;
				}

			if (nn==theMM.ssParams.GetRecord()) {
				if (vel==0) {
					theMM.recDown = FALSE;
					if (theMM.playDown) {
						PostMessage(theMM.hPanel,WM_COMMAND,IDC_MM_START,0);
					} else {
						if (!theMM.recording)
							PostMessage(theMM.hPanel,WM_COMMAND,IDC_MM_LIVE,0);
						}
				} else {
					theMM.recDown = TRUE;
					}
				return MIDIPROC_PROCESSED;
				}
			}
		}
	
	if (theMM.live || theMM.recording) return MIDIPROC_PROCESSED;
	return MIDIPROC_NOTPROCESSED;
	}

void MotionManager::SetStartStopEnable(BOOL onOff)
	{
	if (startStopEnabled==onOff) return;
	if (onOff) {
		MIDIMan_Open(MIDIInProc,0,10);
		MIDIMan_Start();
	} else {
		MIDIMan_Stop();
		MIDIMan_Close(MIDIInProc,0);
		}
	startStopEnabled = onOff;
	SetMotionCaptureINISetting(MCAP_INI_SSENABLE,startStopEnabled);
	}

void MotionManager::Start() 
	{
	if (ip->IsAnimPlaying()) {
		ip->EndAnimPlayback();
		}

	// Check to make sure there is something to record.
	BOOL any=FALSE;
	for (int i=0; i<cont.Count(); i++) {
		if (cont[i]->GetRecordState()) {
			any = TRUE;
			break;
			}
		}
	if (!any) {
		TSTR buf1 = GetString(IDS_RB_NOTRACKSON);
		TSTR buf2 = GetString(IDS_RB_MOTIONCAPTURE);
		MessageBox(hPanel,buf1,buf2,MB_OK|MB_ICONEXCLAMATION);
		Stop();
		return;
		}

	playSpeed = ip->GetPlaybackSpeed();
	
	// Grab settings from UI
	ISpinnerControl *spin = 
		GetISpinner(GetDlgItem(hPanel,IDC_MM_PREROLLSPIN));	
	pre   = spin->GetIVal();
	ReleaseISpinner(spin);
	spin  = GetISpinner(GetDlgItem(hPanel,IDC_MM_INSPIN));	
	in    = spin->GetIVal();
	ReleaseISpinner(spin);
	spin  = GetISpinner(GetDlgItem(hPanel,IDC_MM_OUTSPIN));	
	out   = spin->GetIVal();
	ReleaseISpinner(spin);	
	spin  = GetISpinner(GetDlgItem(hPanel,IDC_REDUCE_THRESHSPIN));	
	reduceThresh = spin->GetFVal();
	ReleaseISpinner(spin);
	reduce       = IsDlgButtonChecked(hPanel,IDC_REDUCE_KEYS);
	doubleSample = IsDlgButtonChecked(hPanel,IDC_SAMPLES2);

	// Make sure the current range fits.
	if (pre>in) pre=in;
	if (in>/*=*/out) {
		TSTR buf1 = GetString(IDS_RB_NOTIME);
		TSTR buf2 = GetString(IDS_RB_MOTIONCAPTURE);
		MessageBox(hPanel,buf1,buf2,MB_OK|MB_ICONEXCLAMATION);
		Stop();
		return;
		}
	Interval anim    = ip->GetAnimRange();
	Interval animNew = anim;	
	if (animNew.Start()>pre) {
		animNew.SetStart(pre);		
		}
	if (animNew.End()<out) {
		animNew.SetEnd(out);		
		}
	if (!(animNew==anim)) ip->SetAnimRange(animNew);

	// Do it!
	SetCapture(hPanel);
	DoCapture(
		pre, in, out, GetTicksPerFrame()/(doubleSample?2:1));	
	ReleaseCapture();

	// Put the range back
	if (!(animNew==anim)) ip->SetAnimRange(anim);
	}


#define TimeValToMS(t) ((t*1000)/TIME_TICKSPERSEC)
#define MSToTimeVal(ms) ((ms*TIME_TICKSPERSEC)/1000)

static void CALLBACK PlayCallback(
		UINT idEvent,UINT reserved,DWORD_PTR user,
		DWORD_PTR reserved1,DWORD_PTR reserved2)
	{
	theMM.Increment();
	}

void MotionManager::Stop()
	{
	stop = TRUE;
	
	ICustButton *but = GetICustButton(GetDlgItem(hPanel,IDC_MM_LIVE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);	

	but = GetICustButton(GetDlgItem(hPanel,IDC_MM_START));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);	
	}

void MotionManager::Live()
	{
	if (ip->IsAnimPlaying()) {
		ip->EndAnimPlayback();
		}

	SHORT res=0;		

	// Don't re-enter
	if (live || recording) {
		Stop();
		return;
		}
	live = TRUE;

	SetCapture(hPanel);
	playSpeed = ip->GetPlaybackSpeed();

	// Turn on the controllers which are in record mode.
	for (int i=0; i<cont.Count(); i++) {
		if (cont[i]->GetRecordState()) {			
			cont[i]->BeginLive(ip->GetTime());
			cont[i]->LiveOn();
			}
		}
	
	// Push capture mode
	theCaptureMode.recording = FALSE;
	ip->PushCommandMode(&theCaptureMode);

	// Clear the key state
	GetAsyncKeyState(VK_ESCAPE);
	stop = FALSE;

	// Play soundtrack while recordin' -- Harry D, 23/12/98
	SoundObj *snd;
	snd = ip->GetSoundObject();
	snd->Stop();

	// Set the clock period to 1 millisecond and start timer
	calls     = 0;
	startTime = GetAnimStart();
	timeBeginPeriod(1);	
	TimeValue t = 0;
	UINT playTimer = 
		timeSetEvent(1,0,PlayCallback,(DWORD_PTR)this,TIME_PERIODIC);	

	snd->Scrub(pre, GetAnimEnd()); // start sound here // mjm - 9.10.99


	// Loop until the user hits escape
	while (!stop && !(res&1)) {
		if (playWhileTest) {
			t = GetTime();
			if (t>GetAnimEnd()) {
				t = startTime;
				calls = 0;
				snd->Stop();
				snd->Scrub(pre, GetAnimEnd()); // start sound here // mjm - 9.10.99

				}
			}

		// Process some messages
		CheckMessages();

		// Invalidate controllers
		for (int i=0; i<cont.Count(); i++) {
			cont[i]->NotifyDependents(FOREVER,0,REFMSG_CHANGE);			
			}
		if (playWhileTest) {
			ip->SetTime(t,FALSE);
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		} else {
			ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
			}

		res = GetAsyncKeyState(VK_ESCAPE);
		}

	snd->Stop();

	// Shut off timer
	timeKillEvent(playTimer);
	timeEndPeriod(1);

	// Turn off the live button
	ICustButton *but = GetICustButton(GetDlgItem(hPanel,IDC_MM_LIVE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);	

	// Turn off the controllers which are in record mode.
	for (i=0; i<cont.Count(); i++) {
		if (cont[i]->GetRecordState()) {			
			cont[i]->LiveOff();
			cont[i]->EndLive(ip->GetTime());
			}
		}

	// Pop off animate mode.
	ip->PopCommandMode();
	ip->DeleteMode(&theCaptureMode);

	// Invalidate controllers and redraw one more time to put everything back.
	for (i=0; i<cont.Count(); i++) {
		cont[i]->NotifyDependents(FOREVER,0,REFMSG_CHANGE);		
		}
	ip->RedrawViews(ip->GetTime(),REDRAW_END);

	ReleaseCapture();

	live = FALSE;
	}


void MotionManager::Increment()
	{	
	EnterCriticalSection(&theMM.csect);	
	
	// Increment the counter.
	calls++;	

	for (int i=0; i<device.Count(); i++) {
		device[i]->Cycle(calls);
		}

	// Accumulate 50 times per second
	if (calls%20==0) {
		TimeValue t = GetTime();
		for (int i=0; i<cont.Count(); i++) {
			if (cont[i]->GetRecordState()) {
				cont[i]->Accumulate(t);
				}
			}
		}

	if (recording) {
		TimeValue t = GetTime();
		if (!capture && record.InInterval(t)) {
			capture = TRUE;
			// Turn on the controllers which are in record mode.
			for (int i=0; i<cont.Count(); i++) {
				if (cont[i]->GetRecordState()) {				
					if (livePreRoll) {
						cont[i]->LiveOff();
						cont[i]->EndLive(t);
						}
					cont[i]->RecordOn(!livePreRoll);
					}
				}
			}
		
		// Tell controllers to capture a sample
		if (t>=nextSample && t>record.Start() && samples<totalSamples) {
			for (int i=0; i<cont.Count(); i++) {
				if (cont[i]->GetRecordState()) {
					cont[i]->Capture(record,t,samples);
					}
				}
			samples++;
			nextSample += sampleRate;
			}
		}

	LeaveCriticalSection(&theMM.csect);	
	}

TimeValue MotionManager::GetTime()
	{
	TimeValue res;
	EnterCriticalSection(&theMM.csect);	
	TimeValue dt = MSToTimeVal(calls);
	switch (playSpeed) {
		case -4: dt /= 4; break;
		case -2: dt /= 2; break;
		case  2: dt *= 2; break;
		case  4: dt *= 4; break;
		}
	res = startTime + dt;		
	LeaveCriticalSection(&theMM.csect);	
	return res;
	}


static ULONG _stdcall Function(void *foo) {return 0;}

class CaptureStatus : public KeyReduceStatus {
	public:		
		int pp;
		int total;
		Interface *ip;
		TCHAR *title;
		BOOL canceled;

		CaptureStatus(Interface *i,TCHAR *t) {ip=i;title=t;pp=0;canceled=FALSE;}
		void Init(int total) {
			this->total = total;
			ip->ProgressStart(title, TRUE, Function, 0);
			}
		void Done() {
			ip->ProgressEnd();
			}
		int Progress(int p) {
			int pct = total?((p+pp)*100)/total:100;
			ip->ProgressUpdate(pct);
			if (ip->GetCancel()) {
				ip->SetCancel(FALSE);
				canceled = TRUE;
				return KEYREDUCE_STOP;
			} else {
				return KEYREDUCE_CONTINUE;
				}
			}
	};


void MotionManager::DoCapture(
		TimeValue pre, TimeValue start, TimeValue end, TimeValue samp)
	{
	TimeValue t;
	SHORT res=0;	
	int numCont = 0;

	// Don't re-enter
	if (recording || live) {
		Stop();
		return;
		}
	recording = TRUE;
	capture   = FALSE;		

	theHold.SuperBegin();
	theHold.Begin();

	// Set the clock period to 1 millisecond
	timeBeginPeriod(1);
	

	// Play soundtrack while recordin' -- Harry D, 23/12/98
	SoundObj *snd;
	snd = ip->GetSoundObject();
	snd->Stop();
	
	// Setup everything and redraw at the start
	startTime    = pre;
	record       = Interval(start,end);
	sampleRate   = samp;
	calls        = 0;	
	nextSample   = start;
	samples      = 0;	
	totalSamples = record.Duration()/sampleRate + 1;
	theCaptureMode.recording = TRUE;
	ip->PushCommandMode(&theCaptureMode);
	ip->SetTime(startTime);

	// Turn on the controllers which are in record mode.
	for (int i=0; i<cont.Count(); i++) {
		if (cont[i]->GetRecordState()) {
			numCont++;
			cont[i]->BeginCapture(record,samp);
			if (livePreRoll) {
				cont[i]->BeginLive(start);
				cont[i]->LiveOn();
				}
			}
		}

	// Clear the key state
	GetAsyncKeyState(VK_ESCAPE);
	stop = FALSE;

	// Set a time event.
	UINT playTimer = 
		timeSetEvent(1,0,PlayCallback,(DWORD_PTR)this,TIME_PERIODIC);	
	
//	snd->Scrub(start,end); // start sound here
	snd->Scrub(pre, end); // start sound here // mjm - 9.10.99

	// Loop through the capture range
	while ((t=GetTime())<=end && !stop && !(res&1)) {
		
		// Process some messages
		CheckMessages();

		// While recording, invalidate controllers
		if (record.InInterval(t) || livePreRoll) {
			for (int i=0; i<cont.Count(); i++) {
				cont[i]->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
				}
			}

		// Step to the next frame and redraw
		ip->SetTime(t,FALSE);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);		
		
		// Check for escape key
		res = GetAsyncKeyState(VK_ESCAPE);
		}

	snd->Stop();
	
	timeKillEvent(playTimer);
	timeEndPeriod(1);

	// Setup progress bar.
	CaptureStatus stat(ip,_T("Creating Keys"));
	stat.Init(totalSamples*numCont);

	// Turn off the controllers which are in record mode.	
	for (i=0; i<cont.Count(); i++) {
		if (cont[i]->GetRecordState()) {
			if (!stat.canceled) 
				cont[i]->EndCapture(record,samp,&stat);
			cont[i]->RecordOff();			
			stat.pp += totalSamples;			
			}
		}
	theHold.Accept(GetString(IDS_RB_MOTIONCAPTURE));
	stat.Done();	
	stat.pp = 0;
	
	// Reduce keys
	if (!stat.canceled && reduce) {
		for (i=0; i<cont.Count(); i++) {
			if (cont[i]->GetRecordState()) {						
				
				// Set up title in progress bar
				TSTR title = ControlName(cont[i]);
				title = TSTR(_T("Reducing Keys: ")) + title;
				stat.title = title;

				// Reduce the keys
				float thresh = reduceThresh;
				switch (cont[i]->cont->SuperClassID()) {
					case CTRL_ROTATION_CLASS_ID:
						thresh = DegToRad(thresh); break;
					case CTRL_SCALE_CLASS_ID:
						thresh = thresh/100.0f; break;
					}
				int res = ApplyKeyReduction(
					cont[i]->cont, record, thresh, samp, &stat);
				
				// Close status
				stat.Done();

				if (res!=KEYREDUCE_CONTINUE) break;
				}
			}
		}	
	theHold.SuperAccept(GetString(IDS_RB_MOTIONCAPTURE));

	// Bump off the command mode.
	ip->PopCommandMode();
	ip->DeleteMode(&theCaptureMode);
	ip->SetTime(start,REDRAW_END);
	
	recording = FALSE;

	Stop();
	}


#define DOUBLESAMP_CHUNK_ID		0x0100
#define LIVEPRE_CHUNK_ID		0x0110
#define REDUCE_CHUNK_ID			0x0120
#define REDUCETHRESH_CHUNK_ID	0x0130
#define PRE_CHUNK_ID			0x0140
#define IN_CHUNK_ID				0x0150
#define OUT_CHUNK_ID			0x0160
#define SELSET_CHUNK_ID			0x0170
#define PLAYTEST_CHUNK_ID		0x0180

IOResult MotionManager::Save(ISave *isave)
	{
	ULONG nb;	

	isave->BeginChunk(DOUBLESAMP_CHUNK_ID);
	isave->Write(&doubleSample,sizeof(doubleSample),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(LIVEPRE_CHUNK_ID);
	isave->Write(&livePreRoll,sizeof(livePreRoll),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(PLAYTEST_CHUNK_ID);
	isave->Write(&playWhileTest,sizeof(playWhileTest),&nb);
	isave->EndChunk();	

	isave->BeginChunk(REDUCE_CHUNK_ID);
	isave->Write(&reduce,sizeof(reduce),&nb);
	isave->EndChunk();

	isave->BeginChunk(REDUCETHRESH_CHUNK_ID);
	isave->Write(&reduceThresh,sizeof(reduceThresh),&nb);
	isave->EndChunk();

	isave->BeginChunk(PRE_CHUNK_ID);
	isave->Write(&pre,sizeof(pre),&nb);
	isave->EndChunk();

	isave->BeginChunk(IN_CHUNK_ID);
	isave->Write(&in,sizeof(in),&nb);
	isave->EndChunk();

	isave->BeginChunk(OUT_CHUNK_ID);
	isave->Write(&out,sizeof(out),&nb);
	isave->EndChunk();

	for (int i=0; i<selSets.Count(); i++) {
		isave->BeginChunk(SELSET_CHUNK_ID);
		isave->WriteWString(*(selSets[i]));
		isave->EndChunk();
		}

	return IO_OK;
	}

IOResult MotionManager::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	if (iu) iu->CloseUtility();

	for (int i=0; i<selSets.Count(); i++) {
		delete selSets[i];
		}
	selSets.Resize(0);

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case SELSET_CHUNK_ID: {
				TCHAR *buf;
				iload->ReadWStringChunk(&buf);
				TSTR *name = new TSTR(buf);
				selSets.Append(1,&name);
				break;
				}

			case DOUBLESAMP_CHUNK_ID:
				res=iload->Read(&doubleSample,sizeof(doubleSample),&nb);
				break;
			
			case LIVEPRE_CHUNK_ID:
				res=iload->Read(&livePreRoll,sizeof(livePreRoll),&nb);
				break;

			case PLAYTEST_CHUNK_ID:
				res=iload->Read(&playWhileTest,sizeof(playWhileTest),&nb);
				break;

			case REDUCE_CHUNK_ID:
				res=iload->Read(&reduce,sizeof(reduce),&nb);
				break;

			case REDUCETHRESH_CHUNK_ID:
				res=iload->Read(&reduceThresh,sizeof(reduceThresh),&nb);
				break;

			case PRE_CHUNK_ID:
				res=iload->Read(&pre,sizeof(pre),&nb);
				break;

			case IN_CHUNK_ID:
				res=iload->Read(&in,sizeof(in),&nb);
				break;

			case OUT_CHUNK_ID:
				res=iload->Read(&out,sizeof(out),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
		
	return IO_OK;
	}



//--- General UI for MCControl parameters-----------------------------

static INT_PTR CALLBACK MCContParamDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class MCParamWindow {
	public:
		HWND hWnd;
		HWND hParent;
		Control *cont;
		MCParamWindow() {assert(0);}
		MCParamWindow(HWND hWnd,HWND hParent,Control *cont)
			{this->hWnd=hWnd; this->hParent=hParent; this->cont=cont;}
	};
static Tab<MCParamWindow> mcParamWindows;

static void RegisterMCParamWindow(HWND hWnd, HWND hParent, Control *cont)
	{
	MCParamWindow rec(hWnd,hParent,cont);
	mcParamWindows.Append(1,&rec);
	}

static void UnRegisterMCParamWindow(HWND hWnd)
	{	
	for (int i=0; i<mcParamWindows.Count(); i++) {
		if (hWnd==mcParamWindows[i].hWnd) {
			mcParamWindows.Delete(i,1);
			return;
			}
		}	
	}

static HWND FindOpenMCParamWindow(HWND hParent,Control *cont)
	{	
	for (int i=0; i<mcParamWindows.Count(); i++) {
		if (hParent == mcParamWindows[i].hParent &&
			cont    == mcParamWindows[i].cont) {
			return mcParamWindows[i].hWnd;
			}
		}
	return NULL;
	}

MCParamDlg::MCParamDlg(MCControl *c)
	{
	blockInvalidate = FALSE;
	valid   = FALSE;
	cont    = NULL;	
	hWnd    = NULL;
	binding = NULL;
	ReplaceReference(0,c);
	for (int i=0; i<cont->NumDeviceBindings(); i++) {
		if (cont->GetDeviceBinding(i)) {
			binding = cont->GetDeviceBinding(i);
			break;
			}
		}
	}

void MCParamDlg::DoWindow(HWND hParent, TCHAR *pname)
	{
	HWND hCur = FindOpenMCParamWindow(hParent,cont);
	if (hCur) {
		SetForegroundWindow(hCur);
		DeleteAllRefsFromMe();
		delete this;
		return;
		}

	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_MC_PARAMS),
		hParent,
		MCContParamDlgProc,
		(LPARAM)this);
	
	if (pname && TSTR(pname).Length()) {
		TSTR buf = TSTR(GetString(IDS_RB_MOTIONCAPTURE)) 
			+ TSTR(_T("\\")) + TSTR(pname);
		SetWindowText(hWnd,buf);
		}
	RegisterMCParamWindow(hWnd,hParent,cont);
	}

class CheckForNonMCParamDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonMCParamDlg(ReferenceMaker *m) {non=FALSE;me=m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(MCPARAMDLG_CLASS_ID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}	
	};
void MCParamDlg::MaybeCloseWindow()
	{
	CheckForNonMCParamDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}

RefResult MCParamDlg::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
     	PartID& partID,  
     	RefMessage message)
	{
	switch (message) {
		case REFMSG_NEW_BINDING:
			UpdateBinding(partID);
			break;

		case REFMSG_NODE_NAMECHANGE:
		case REFMSG_CHANGE:
			if (!blockInvalidate) Invalidate();			
			break;
		
		case REFMSG_REF_DELETED:
			MaybeCloseWindow();
			break;
		}
	return REF_SUCCEED;
	}

void MCParamDlg::Invalidate()
	{
	valid = FALSE;	
	InvalidateRect(GetHPanel(),NULL,FALSE);	
	}

void MCParamDlg::Update()
	{		
	if (!valid) {
		if (binding) binding->UpdateRollup(iRoll);
		valid = TRUE;
		}
	}

void MCParamDlg::SetupUI(HWND hWnd)
	{
	this->hWnd = hWnd;
	iRoll = GetIRollup(GetDlgItem(hWnd,IDC_MAIN_ROLLUP));
	
	// have the controller add it's rollup
	AddRollup();

	// Have the binding add it's rollup
	if (binding) binding->AddRollup(this);	

	// Display all the rollups
	iRoll->Show();
	}

void MCParamDlg::Close()
	{
	UnRegisterMCParamWindow(hWnd);
	DeleteAllRefsFromMe();
	SetWindowLongPtr(hWnd,GWLP_USERDATA,0);
	delete this;
	}

void MCParamDlg::SetBinding(MCDeviceBinding *b)
	{
	binding = b;
	if (iRoll->GetNumPanels()>1) {
		iRoll->DeleteRollup(1, iRoll->GetNumPanels()-1);
		}
	if (binding) {
		binding->AddRollup(this);
		iRoll->Show();
		}
	Invalidate();
	}

static INT_PTR CALLBACK MCContParamDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	static int dialogWidth;
	MCParamDlg *dlg = (MCParamDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!dlg && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
		case WM_INITDIALOG: {
			dlg = (MCParamDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			dlg->SetupUI(hWnd);
			Rect rect;
			GetWindowRect(hWnd,&rect);
			dialogWidth = rect.w()-1;
			break;
			}
		
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			dlg->Close();
			break;

		case WM_PAINT:
			dlg->Update();
			return FALSE;		

		case WM_WINDOWPOSCHANGING: {
			WINDOWPOS *wp = (WINDOWPOS*)lParam;			
			wp->cx = dialogWidth;
			if (wp->cy<20) wp->cy = 20;
			break;
			}

		case WM_SIZE: {
			Rect rect;
			GetClientRectP(GetDlgItem(hWnd,IDC_MAIN_ROLLUP),&rect);
			int height = HIWORD(lParam)-rect.top-8;
			if (height<10) height = 10;
			SetWindowPos(
				GetDlgItem(hWnd,IDC_MAIN_ROLLUP),
				NULL,
				0, 0,
				rect.w()-1, height,
				SWP_NOMOVE|SWP_NOZORDER);
			break;
			}		

		default:
			return FALSE;
		}
	return TRUE;
	}



//--- GenMCParamDlg ----------------------------------------------

static INT_PTR CALLBACK GenMCDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static int bindingButtons[]     = {IDC_XBINDING,     IDC_YBINDING,     IDC_ZBINDING};
static int bindingEditButtons[] = {IDC_EDIT_XBINDING,IDC_EDIT_YBINDING,IDC_EDIT_ZBINDING};

GenMCParamDlg::GenMCParamDlg(MCControl *c, int dlgID) : 
		MCParamDlg(c)
	{	
	editBinding = -1;
	for (int i=0; i<c->NumDeviceBindings(); i++) {
		if (c->GetDeviceBinding(i)) {
			editBinding = i;
			break;
			}
		}
	hPanel = NULL;
	this->dlgID = dlgID;
	}

void GenMCParamDlg::Update()
	{
	if (!valid) {
		MCParamDlg::Update();
		SetupButtons();		
		}
	}

void GenMCParamDlg::AddRollup()
	{
	int strID = 0;
	switch (dlgID) {
		case IDD_MC_FLOAT:    strID = IDS_RB_FLOATMC;   break;
		case IDD_MC_POSITION: strID = IDS_RB_POSMC;		break;
		case IDD_MC_POINT3:   strID = IDS_RB_POINT3MC;	break;
		case IDD_MC_ROTATION: strID = IDS_RB_ROTMC;		break;
		case IDD_MC_SCALE:    strID = IDS_RB_SCALEMC;   break;		
		}
	iRoll->AppendRollup(
			hInstance, 
			MAKEINTRESOURCE(dlgID), 
			GenMCDlgProc, 
			GetString(strID),
			(LPARAM)this);	
	}

void GenMCParamDlg::SetupButtons()
	{
	for (int i=0; i<cont->NumDeviceBindings(); i++) {
		if (cont->GetDeviceBinding(i)) {
			SetDlgItemText(hPanel,bindingButtons[i],
				cont->GetDeviceBinding(i)->BindingName());
		} else {
			SetDlgItemText(hPanel,bindingButtons[i],
				GetString(IDS_RB_NONE));
			}
		
		ICustButton *but;
		but = GetICustButton(GetDlgItem(hPanel,bindingEditButtons[i]));
		if (but) {
			but->SetType(CBT_CHECK);
			if (!cont->GetDeviceBinding(i)) {
				but->SetCheck(FALSE);
				but->Disable();
			} else if (editBinding==i) {
				but->SetCheck(TRUE);
				but->Enable();
			} else {
				but->SetCheck(FALSE);
				but->Enable();
				}
			ReleaseICustButton(but);
			}
		}	
	
	ICustEdit *edit = GetICustEdit(GetDlgItem(hPanel,IDC_TRACKNAME));
	edit->SetText(cont->trackName);
	ReleaseICustEdit(edit);
	}

void GenMCParamDlg::UpdateBinding(int which)
	{
	if (editBinding==which) {
		SetEditBinding(which);
		}
	}

void GenMCParamDlg::SetEditBinding(int which)
	{	
	editBinding = which;
	SetupButtons();
	switch (which) {
		case 0:  
		case 1:  
		case 2:  SetBinding(cont->GetDeviceBinding(which)); break;
		default: SetBinding(NULL); break;
		}
	}

void GenMCParamDlg::AssignBinding(int which)
	{
	BOOL cancel;	
	MCDeviceBinding *b = theMM.SelectBinding(hPanel,cancel);
	if (!cancel) {
		theHold.Begin();
		cont->ReplaceReference(which+1,b);
		theHold.Accept(GetString(IDS_RB_PICKBINDING));
		cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		cont->NotifyDependents(FOREVER,which,REFMSG_NEW_BINDING);
		if (b) SetEditBinding(which);
		else if (which==editBinding) SetEditBinding(-1);
		}
	}

static INT_PTR CALLBACK GenMCDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	GenMCParamDlg *dlg = (GenMCParamDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!dlg && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
		case WM_INITDIALOG:
			dlg = (GenMCParamDlg*)lParam;
			dlg->hPanel = hWnd;
			dlg->SetupButtons();
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);			
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TRACKNAME: {
					ICustEdit *edit = GetICustEdit(GetDlgItem(hWnd,IDC_TRACKNAME));
					TCHAR buf[256];
					edit->GetText(buf,256);
					dlg->cont->trackName = buf;
					dlg->blockInvalidate = TRUE;
					dlg->cont->NotifyDependents(FOREVER,0,REFMSG_NODE_NAMECHANGE);
					dlg->blockInvalidate = FALSE;
					ReleaseICustEdit(edit);
					if (theMM.hPanel) theMM.SetupList();
					break;
					}

				case IDC_EDIT_XBINDING: dlg->SetEditBinding(0); break;
				case IDC_EDIT_YBINDING: dlg->SetEditBinding(1); break;
				case IDC_EDIT_ZBINDING: dlg->SetEditBinding(2); break;

				case IDC_XBINDING: dlg->AssignBinding(0); break;
				case IDC_YBINDING: dlg->AssignBinding(1); break;
				case IDC_ZBINDING: dlg->AssignBinding(2); break;
				}
		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:   			
   			dlg->iRoll->DlgMouseMessage(hWnd,msg,wParam,lParam);
			break; 

		case WM_PAINT:
			dlg->Update();
			return FALSE;

		default:
			return FALSE;
		}
	return TRUE;
	}


