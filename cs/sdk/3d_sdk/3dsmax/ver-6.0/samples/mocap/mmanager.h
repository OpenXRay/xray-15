/**********************************************************************
 *<
	FILE: mmanager.h

	DESCRIPTION: Motion capture manager

	CREATED BY: Rolf Berteig

	HISTORY: October 30, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#ifndef  _MMANAGER_H_
#define	 _MMANAGER_H_

#include "keyreduc.h"
#include "mcapdev.h"


// Flags for MCControl
#define MCC_RECORD_ACTIVE	(1<<0)
#define MCC_RECORD_ON		(1<<1)
#define MCC_LIVE_ON			(1<<2)

#define UPDATE_RATE	10

// Base class for motion capture controllers.
class MCControl : public IMCControl {
	public:		
		Control *cont; // The keyframe controller that will store the data		
		DWORD flags;
		int selSet;
		TSTR trackName;

		MCControl();
		~MCControl();

		void DeleteThis() {delete this;}
		int IsKeyable() {return 1;}	

		int NumSubs()  {return 1;}
		Animatable* SubAnim(int i) {return cont;}
		TSTR SubAnimName(int i);
		
		int NumRefs() {return 1 + NumDeviceBindings();}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		int SubNumToRefNum(int subNum) {return subNum;}

		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage)
			{return REF_SUCCEED;}
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		void RefDeleted();
		void RefAdded(RefMakerHandle rm);
		void RefDeletedUndoRedo();
		void RefAddedUndoRedo(RefMakerHandle rm);
		RefResult AutoDelete();

		void Copy(Control *from);
		BOOL AssignController(Animatable *control,int subAnim);

		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);		
		
		// These are just passed on to the sub controller.		
		void AddNewKey(TimeValue t,DWORD flags);
		int NumKeys();
		TimeValue GetKeyTime(int index);
		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);
		BOOL IsKeyAtTime(TimeValue t,DWORD flags);
		BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
		void DeleteKeyAtTime(TimeValue t);		
		int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags); // RB 3/7/99: Added
		int GetKeySelState(BitArray &sel,Interval range,DWORD flags); // RB 3/7/99: Added
		void SelectKeys(TrackHitTab& sel, DWORD flags); // RB 3/7/99: Added
		BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);	
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t);
		void EnumIKParams(IKEnumCallback &callback);
		BOOL CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags);
		float IncIKParam(TimeValue t,int index,float delta);
		void ClearIKParam(Interval iv,int index);
		void EnableORTs(BOOL enable);		

		int TrackParamsType() {return TRACKPARAMS_WHOLE;}

		BOOL ControllerInScene();

		// Local methods
		void MCControlClone(MCControl *src,RemapDir& remap);
		void Accumulate(TimeValue t);
		virtual BOOL GetRecordState() {return flags&MCC_RECORD_ACTIVE ? TRUE : FALSE;}
		virtual void SetRecordState(BOOL onOff) {if (onOff) flags|=MCC_RECORD_ACTIVE; else flags &= ~MCC_RECORD_ACTIVE;}
		virtual void LiveOn();
		virtual void LiveOff();
		virtual void RecordOn(BOOL reset=TRUE);
		virtual void RecordOff();
		virtual BOOL IsLiveOn()  {return flags&MCC_LIVE_ON;}
		virtual BOOL IsRecordOn(){return flags&MCC_RECORD_ON;}
		virtual void BeginCapture(Interval record,TimeValue sampSize) {}
		virtual void EndCapture(Interval record,TimeValue sampSize, KeyReduceStatus *stat) {}
		virtual void BeginLive(TimeValue t) {}
		virtual void EndLive(TimeValue t) {}
		virtual void Capture(Interval record,TimeValue t,int sample) {}		
	};

#define STARTSTOP_CUSTOM				0
#define STARTSTOP_MEDIACONTROLSTATION	1

class StartStopParams {
	public:
		int presetType, devID;
		int channel, stop, play, record;
		StartStopParams(int t, int d, int c, int s, int p, int r)
			{presetType=t; devID=d, channel=c; stop=s; play=p; record=r;}
		StartStopParams() {}
		int GetChannel();
		int GetStop();
		int GetPlay();
		int GetRecord();
	};

class MotionManager : public UtilityObj, public IMCapManager {
	public:
		// Controllers and devices that we're managing
		Tab<MCControl*> cont;
		Tab<MCInputDevice*> device;
		BOOL listBuilt;

		// Parameters
		BOOL doubleSample, reduce, livePreRoll, playWhileTest;
		float reduceThresh;
		TimeValue pre, in, out;
		Tab<TSTR*> selSets;

		// Record/Live data
		CRITICAL_SECTION csect;
		UINT calls;
		BOOL stop, live, recording, capture;
		TimeValue startTime, sampleRate, nextSample;
		int samples, totalSamples, playSpeed;
		Interval record;

		// UI stuff
		Interface *ip;		
		IUtil *iu;
		HWND hPanel;
		HIMAGELIST hTrackIcons, hButtonIcons;
		
		// Start/Stop params
		StartStopParams ssParams;
		BOOL startStopEnabled;
		BOOL recDown, playDown;

		MotionManager();
		~MotionManager();
		void InitUI();
		void DeleteThis() {}

		void SetupDeviceList();

		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		
		void Init(HWND hWnd);
		void SetupList();
		void Destroy(HWND hWnd);
		void DrawTrack(HDC hdc,int index,BOOL sel,RECT rect);
		TSTR ControlName(MCControl *c);
		TSTR ControlName(int index);
		void Start();
		void Stop();
		void Live();
		void CheckMessages();
		void SetupStartStop();
		void SetStartStopEnable(BOOL onOff);

		void NewSelSet();
		void DeleteSelSet();
		void ActivateSelSet(int i);
		void ClearSelSetName();

		void TrackParams(HWND hWnd,int index);

		MCDeviceBinding *SelectBinding(HWND hWnd, BOOL &cancel);		
		void ToggleRecordMode(int index);

		void Register(MCControl *c);
		void UnRegister(MCControl *c);		
		BOOL IsRegistered(MCControl *c);

		void DoCapture(TimeValue pre, TimeValue start, TimeValue end, TimeValue samp);		
		void Increment();

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From IMCapManager
		void MidiNote(int channel, int note);
		TimeValue GetTime();
	};

extern MotionManager theMM;


//--- Devices ----------------------------------------------

//extern MCInputDevice *GetMouseDevice();
//extern MCInputDevice *GetMidiDevice();
//extern MCInputDevice *GetJoyDevice();



//--- General UI for Motion Capture controllers ------------------------

#define MCPARAMDLG_CLASS_ID 0x11fa4b2a

class MCParamDlg : public IMCParamDlg {
	public:
		MCControl *cont;		
		HWND hWnd;
		BOOL valid, blockInvalidate;		

		MCParamDlg(MCControl *c);
		void DoWindow(HWND hParent, TCHAR *pname);

		Class_ID ClassID() {return Class_ID(MCPARAMDLG_CLASS_ID,0);}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}		
		
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(MCControl*)rtarg;}

		void MaybeCloseWindow();
		void Invalidate();
		virtual void Update();
		void SetupUI(HWND hWnd);
		void Close();

		void SetBinding(MCDeviceBinding *b);

		virtual void AddRollup()=0;
		virtual void UpdateBinding(int which)=0;
		virtual HWND GetHPanel()=0;


	};

class GenMCParamDlg : public MCParamDlg {
	public:
		int editBinding;
		HWND hPanel;
		int dlgID;		

		GenMCParamDlg(MCControl *c,int dlgID);
		void Update();
		void AddRollup();
		void SetupButtons();
		void SetEditBinding(int which);
		void AssignBinding(int which);
		void UpdateBinding(int which);
		HWND GetHPanel() {return hPanel;}
	};

#define REFMSG_NEW_BINDING	(REFMSG_USER+0x0381)

#endif

 