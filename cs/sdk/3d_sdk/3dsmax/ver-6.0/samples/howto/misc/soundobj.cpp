/**********************************************************************
 *<
	FILE:			soundobj.cpp

	DESCRIPTION:	Sound plug-in object base class

	CREATED BY:		Rolf Berteig

	HISTORY:		created 2 July 1995
					CCJ 8.28.00 - Integrated some neat changes & fixes
					from Matt @ Boomer Labs - Thanks!


 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "core.h"
#include "object.h"
#include "undolib.h"
#include "control.h"
#include "Maxapi.h"
#include "decomp.h"
#include "coremain.h"
#include "soundobj.h"
#include <mmsystem.h>
#include <vfw.h>
#include "audio.h"
#include "sound.h"
#include "custcont.h"
#include "istdplug.h"
#include "notetrck.h"

#ifdef WIN95STUFF

// CORE.DLL instance handle in coremain.cpp
extern HINSTANCE hInstance;

#define TimeValToMS(t) ((t*1000)/TIME_TICKSPERSEC)
#define MSToTimeVal(ms) ((ms*TIME_TICKSPERSEC)/1000)


#define FILTER_RES	1200

class DefSoundObj;

class WaveAnimChild : public ReferenceTarget {
	friend class SoundObjRestore;
	public:
		DefSoundObj *snd;
		void Init(DefSoundObj *snd);

		// Animatable methods
		void DeleteThis() {}
		virtual SClass_ID SuperClassID() {return SClass_ID(SOUNDOBJ_CLASS_ID);}
		Class_ID ClassID() {return Class_ID(DEF_SOUNDOBJ_CLASS_ID,1);}
		void GetClassName(TSTR& s) {s=GetResString(IDS_RB_SOUNDOBJECT);}
		
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}
		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
		int PaintTrack(			
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcTrack,
			Rect& rcPaint,
			float zoom,
			int scroll,
			DWORD flags );
		int GetTrackVSpace( int lineHeight ) {return 8;}
		int PaintFCurves(			
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcGraph,
			Rect& rcPaint,
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags );
		BOOL IsAnimated() {return TRUE;}

		// Reference methods
		RefResult AutoDelete() {return REF_SUCCEED;}
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message) {return REF_SUCCEED;}
	};

class MetronomeAnimChild : public ReferenceTarget {
	public:
		DefSoundObj *snd;		
		void Init(DefSoundObj *snd);
		
		// Animatable methods
		void DeleteThis() {}
		virtual SClass_ID SuperClassID() {return SClass_ID(SOUNDOBJ_CLASS_ID);}
		Class_ID ClassID() {return Class_ID(DEF_SOUNDOBJ_CLASS_ID,2);}
		void GetClassName(TSTR& s) {s=GetResString(IDS_RB_SOUNDOBJECT);}
		
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}
		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
		int PaintTrack(			
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcTrack,
			Rect& rcPaint,
			float zoom,
			int scroll,
			DWORD flags );
		int GetTrackVSpace( int lineHeight ) {return 1;}
		BOOL IsAnimated() {return TRUE;}

		// Reference methods
		RefResult AutoDelete() {return REF_SUCCEED;}
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message) {return REF_SUCCEED;}
	};


// Isn't it ironic for a sound object to be def? :)
class DefSoundObj : 
		public SoundObj,
		public IWaveSound,
		public IWavePaint {

	friend static void CALLBACK PlayCallback(
		UINT idEvent,UINT reserved,DWORD_PTR user,DWORD_PTR reserved1,DWORD_PTR reserved2);
	friend static void CALLBACK BeepCallback(
		UINT idEvent,UINT reserved,DWORD_PTR user,DWORD_PTR reserved1,DWORD_PTR reserved2);
	friend static INT_PTR CALLBACK SoundParamsWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		// WIN64 Cleanup: Shuler
	friend class SoundObjRestore;

	private:
		HWND hWndDum, hParams;
		BOOL playing, waveActive, metroActive;
		UINT playTimer,period,beepTimer,beepPeriod;
		DWORD calls, bpMinute, bpMeasure, beepCalls, beepEnd, beepStart, beepWrap;
		TimeValue t0, t1, st;
		PAVISTREAM pavi;
		TSTR sndFile;
		FilteredWave *wave;
		WaveAnimChild waveAnim;
		MetronomeAnimChild metroAnim;
		Interval range;
		IObjParam *ip;

		void SetupDummyWindow();

	public:
		DefSoundObj();
		~DefSoundObj();

		BOOL Play(TimeValue tStart,TimeValue t0,TimeValue t1,TimeValue frameStep);
		void Scrub(TimeValue t0,TimeValue t1);
		TimeValue Stop();
		TimeValue GetTime();
		BOOL Playing() {return playing;}
		void SaveSound(PAVIFILE pfile,TimeValue t0,TimeValue t1);
		void SetMute(BOOL mute) {waveActive = !mute;}
		BOOL IsMute() {return !waveActive;}
		void Hold();
		void SetSoundFile(PAVISTREAM stream,TSTR &file);

		// From IWaveSound
		TSTR GetSoundFileName() {return sndFile;}
		BOOL SetSoundFileName(TSTR name);
		void SetStartTime(TimeValue t);
		TimeValue GetStartTime();
		TimeValue GetEndTime();

		// From IWavePaint
		void PaintWave(HDC hdc, RECT* rect, Interval i);

		// Animatable methods
		void* GetInterface(ULONG id);
		void DeleteThis() {delete this;}
		Class_ID ClassID() {return Class_ID(DEF_SOUNDOBJ_CLASS_ID,0);}
		void GetClassName(TSTR& s) {s=GetResString(IDS_RB_DEFSOUND);}
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}
		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
		int Paint(			
			HDC hdc,
			Rect& rcTrack,
			Rect& rcPaint,
			float zoom,
			int scroll,
			DWORD flags,
			int which );
		int GetTrackVSpace( int lineHeight ) {return 1;}
		Interval GetTimeRange(DWORD flags) {return range;}
		void MapKeys(TimeMap *map,DWORD flags);
		BOOL IsAnimated() {return TRUE;}

		int NumSubs()  {return wave?2:1; }
		Animatable* SubAnim(int i) {if (i==0) return &metroAnim; else return &waveAnim;}
		TSTR SubAnimName(int i); // CCJ 8.28.00 expanded below. // {if (i==0) return GetResString(IDS_RB_METRONOME); else return GetResString(IDS_RB_WAVEFORM);}		

		// Reference methods
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message) {return REF_SUCCEED;}
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

void WaveAnimChild::Init(DefSoundObj *snd) {this->snd=snd;}
int WaveAnimChild::PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,Rect& rcPaint,float zoom,int scroll,DWORD flags )
	{return snd->Paint(hdc,rcTrack,rcPaint,zoom,scroll,flags,1);}
void WaveAnimChild::EditTrackParams(TimeValue t,ParamDimensionBase *dim,TCHAR *pname,HWND hParent,IObjParam *ip,DWORD flags)
	{snd->EditTrackParams(t,dim,pname,hParent,ip,flags);}

void MetronomeAnimChild::Init(DefSoundObj *snd) {this->snd=snd;}
int MetronomeAnimChild::PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,Rect& rcPaint,float zoom,int scroll,DWORD flags )
	{return snd->Paint(hdc,rcTrack,rcPaint,zoom,scroll,flags,0);}
void MetronomeAnimChild::EditTrackParams(TimeValue t,ParamDimensionBase *dim,TCHAR *pname,HWND hParent,IObjParam *ip,DWORD flags)
	{snd->EditTrackParams(t,dim,pname,hParent,ip,flags);}


class DefSoundObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1;}
	void *			Create(BOOL loading) {return new DefSoundObj;}
	const TCHAR *	ClassName() { return GetResString(IDS_RB_DEFSOUND); }
	SClass_ID		SuperClassID() { return SClass_ID(SOUNDOBJ_CLASS_ID); }
	Class_ID 		ClassID() { return Class_ID(DEF_SOUNDOBJ_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};
static DefSoundObjClassDesc sndObjDesc;
ClassDesc *GetSoundObjDescriptor() {return &sndObjDesc;}

class SoundObjRestore : public RestoreObj {	
	public:
		DefSoundObj *snd;
		Interval undo,redo;
		SoundObjRestore(DefSoundObj *snd) {this->snd=snd;undo=snd->range;}   		
		void Restore(int isUndo) 
			{
			redo = snd->range;
			snd->range = undo;
			snd->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			snd->waveAnim.NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			}
		void Redo() 
			{
			snd->range = redo;
			snd->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			snd->waveAnim.NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			}
		void EndHold() {snd->ClearAFlag(A_HELD);}
	};


static void CALLBACK PlayCallback(
		UINT idEvent,UINT reserved,DWORD_PTR user,DWORD_PTR reserved1,DWORD_PTR reserved2)
	{
	DefSoundObj *dobj = (DefSoundObj*)user;
	// Should probably have a semaphore for this
	dobj->calls++;
	}


#define WM_BEEPME	WM_USER+10
#define BEAT1_FREQ	1000
#define BEAT1_DUR	10
#define BEATN_FREQ	2000
#define BEATN_DUR	10

static void CALLBACK BeepCallback(
		UINT idEvent,UINT reserved,DWORD_PTR user,DWORD_PTR reserved1,DWORD_PTR reserved2)
	{
		// WIN64 Cleanup: Shuler
	DefSoundObj *dobj = (DefSoundObj*)user;	
	TimeValue t = dobj->GetTime();
	
	DWORD c = ++dobj->beepCalls;
	if (dobj->beepCalls >= dobj->beepEnd) {
		dobj->beepCalls = dobj->beepStart-1;
		dobj->beepTimer = 
			timeSetEvent(dobj->beepWrap,1,BeepCallback,(DWORD_PTR)dobj,TIME_ONESHOT);
	} else {
		dobj->beepTimer = 
			timeSetEvent(dobj->beepPeriod,1,BeepCallback,(DWORD_PTR)dobj,TIME_ONESHOT);
		}	

	if( (c%dobj->bpMeasure)==0 ) {		
		Beep(BEAT1_FREQ,BEAT1_DUR);
	} else {
		Beep(BEATN_FREQ,BEATN_DUR);
		}	
	}

DefSoundObj::DefSoundObj() 
	{
	hWndDum     = NULL;
	hParams     = NULL;
	playing     = FALSE;
	bpMinute    = 60;
	bpMeasure   = 4;
	pavi        = NULL;	
	wave        = NULL;
	waveActive  = FALSE;
	metroActive = FALSE;
	playTimer   = 0;
	beepTimer   = 0;
	SetupDummyWindow();
	waveAnim.Init(this);
	metroAnim.Init(this);
	range.SetEmpty();
	t1 = t0 = 0;
	ip = NULL;

	hWndDum = CreateWindow(
		_T("SOUNDDUMMYWINDOW"), NULL,
		WS_POPUP,
		0,0,0,0, 
		NULL,
		NULL,
		hInstance,
		NULL );
	}

DefSoundObj::~DefSoundObj()
	{
	if (Playing()) Stop();
	if (pavi) {
#ifndef INTERIM_64_BIT	// CCJ
		AVIStreamRelease(pavi);
#endif	// INTERIM_64_BIT
		pavi = NULL;
		}
	if (wave) delete wave;
	if (hWndDum) DestroyWindow(hWndDum);	
	if (hParams) DestroyWindow(hParams);
	}

BOOL DefSoundObj::Play(TimeValue tStart,TimeValue t0,TimeValue t1,TimeValue frameStep)
	{
#ifndef INTERIM_64_BIT	// CCJ
	if (hParams) {
		TSTR buf1 = GetResString(IDS_RB_CANTPLAYSOUND);
		TSTR buf2 = GetResString(IDS_RB_SOUND);
		MessageBox(NULL,buf1,buf2,MB_OK|MB_ICONSTOP|MB_TASKMODAL);
		return FALSE;
		}		
#endif	// INTERIM_64_BIT
	LONG mst  = TimeValToMS(tStart);
	LONG mt0  = TimeValToMS(t0);
	LONG mt1  = TimeValToMS(t1);		

	calls     = 0;
	this->t0  = t0;
	this->t1  = t1;
	st        = tStart;
	period    = (1000*frameStep)/(TIME_TICKSPERSEC*4);	
	if (!period) period = 1; 	
	
	timeBeginPeriod(period);
	
	if (pavi && waveActive) {
		LONG shift = TimeValToMS(range.Start());

		float rs,re;
		rs = float(range.Start()-t0)/(t1-t0);
		re = float(range.End()-t0)/(t1-t0);

#ifndef INTERIM_64_BIT	// CCJ
		aviaudioPlay(hWndDum,pavi,mst-shift,mt0-shift,mt1-shift,FALSE,TRUE,rs,re);
#endif	// INTERIM_64_BIT
		}

#ifndef INTERIM_64_BIT	// CCJ
	if (metroActive) {			
		beepPeriod = 60000 / bpMinute;		
		beepCalls  = mst/beepPeriod;
		beepEnd    = mt1/beepPeriod;
		beepStart  = mt0/beepPeriod;
		int mod    = (mt0%beepPeriod);
		beepWrap   = (mod?(beepPeriod - mod):0) + (mt1%beepPeriod);
		if (beepWrap==0) beepWrap = beepPeriod;
		LONG d     = beepPeriod - (mst%beepPeriod);
		beepTimer  = timeSetEvent(d,1,BeepCallback,(DWORD_PTR)this,TIME_ONESHOT);
		}
#endif	// INTERIM_64_BIT

	playTimer = timeSetEvent(period,1,PlayCallback,(DWORD_PTR)this,TIME_PERIODIC);		
	playing = TRUE;
	return TRUE;
	}

void DefSoundObj::Scrub(TimeValue t0,TimeValue t1)
	{
#ifndef INTERIM_64_BIT	// CCJ
	if (pavi && waveActive) {
		// CAL-06/23/03: check backward scrubbing, i.e. (t1 > t0), here, because we remove the checking of
		// backward scrubbing that used to be in the callers. (ECO 1053)
		if (( ((t0 >= range.Start()) && (t0<=range.End())) ||
		      ((t1 >= range.Start()) && (t1<=range.End())) ) &&
			(t1 > t0))
			{
			LONG shift = TimeValToMS(range.Start());
//watje
			float rs,re;
			rs = float(range.Start()-t0)/(t1-t0);
			re = float(range.End()-t0)/(t1-t0);

			aviaudioPlay(hWndDum,pavi,
				TimeValToMS(t0)-shift,
				TimeValToMS(t0)-shift,
				TimeValToMS(t1)-shift,FALSE,FALSE,rs,re);
			}
		}
#endif	// INTERIM_64_BIT
	}

TimeValue DefSoundObj::Stop()
	{
#ifndef INTERIM_64_BIT	// CCJ
	if (pavi && waveActive) {
		aviaudioStop();		
		}

	if (metroActive) {
		if (beepTimer) {
			timeKillEvent(beepTimer);
			beepTimer = 0;
			}		
		}
#endif	// INTERIM_64_BIT

	timeKillEvent(playTimer);
	timeEndPeriod(period);
	playTimer = 0;
	playing   = FALSE;
	return GetTime();
	}

void DefSoundObj::SaveSound(PAVIFILE pfile,TimeValue t0,TimeValue t1)
	{
#ifndef INTERIM_64_BIT	// CCJ
	AVISTREAMINFO sInfo;
	PAVISTREAM stream;
	HRESULT	hr;
	LPVOID	lpFormat;
	LONG	cbFormat;
	void *lpBuffer;
	LONG start, end, read;
	LONG plBytes, plSamples, sampPerSec;
	LONG sTotal, sSaved;
	LONG slBegin, slCurrent, slEnd, slWaveBegin, slWaveEnd;

	if (!pavi) return;
	if (!waveActive) return;

	// Adjust for time shift.
	t0 -= range.Start();
	t1 -= range.Start();

	AVIStreamInfo(pavi,&sInfo,sizeof(sInfo));
	hr = AVIFileCreateStream(pfile,&stream,&sInfo);
    if (hr != AVIERR_OK) return;		
	
	AVIStreamFormatSize(pavi,0,&cbFormat);
    lpFormat = GlobalAllocPtr(GHND,cbFormat);
    if (!lpFormat) {
		AVIStreamRelease(stream);
		return;
		}
    AVIStreamReadFormat(pavi,0,lpFormat,&cbFormat);
	AVIStreamSetFormat(stream,0,lpFormat,cbFormat);
	sampPerSec = ((LPWAVEFORMAT)lpFormat)->nSamplesPerSec;
	GlobalFreePtr(lpFormat);

	#define BUFFER_SIZE	1024

	if (!(lpBuffer = malloc(BUFFER_SIZE))) {
		AVIStreamRelease(stream);
		return;
		}
	
	LONG aviStart = AVIStreamStartTime(pavi);
    LONG aviEnd   = AVIStreamEndTime(pavi);
	LONG aviLen   = aviEnd-aviStart;
	start = TimeValToMS(t0);
	end   = TimeValToMS(t1);
	
	if (start < aviStart) {
		LONG d = (aviStart-start)/aviLen + 1;
		start += d * aviLen;
		end   += d * aviLen;		
		}
	if (start > aviEnd) {
		LONG d = (start-aviEnd)/aviLen + 1;
		start -= d * aviLen;
		end   -= d * aviLen;		
		}
	
	// CCJ - 8.28.00
	// Thanks to Mathew Kaustinen at Boomer Labs for bringing to our attention
	// that the intermediate result in this calculation can easily overflow.
	// sTotal = ((end-start)*sampPerSec)/1000;
	sTotal = MulDiv(end-start, sampPerSec, 1000);

	// I kluged this in because there are wav files with dwSampleSize set to 1024
	// instead of some reasonable value like 2 or 4.  Heuristically, dividing by
	// this number seems to get us what we need.
	// It is safe to say that I don't know what is really going on - DB 2/17/96
	if(sInfo.dwSampleSize > 16)
		sTotal /= sInfo.dwSampleSize;
	sSaved = 0;

	slBegin     = AVIStreamTimeToSample(pavi,start%aviLen);
	slCurrent   = slBegin;
	slEnd       = AVIStreamTimeToSample(pavi,end%aviLen);
	slWaveBegin = AVIStreamStart(pavi);
	slWaveEnd   = AVIStreamEnd(pavi);
	
	while (sSaved < sTotal) {
		
		if (slCurrent >= slWaveEnd) {
	    	slCurrent = slWaveBegin;			
			}

		read = slWaveEnd - slCurrent;

		if (read + sSaved > sTotal) {
			read = sTotal - sSaved;
			}

		if (read > BUFFER_SIZE / (int)sInfo.dwSampleSize) {
			read = BUFFER_SIZE / (int)sInfo.dwSampleSize;
			}

		AVIStreamRead(pavi,slCurrent,read,lpBuffer,BUFFER_SIZE,&plBytes,&plSamples);
		AVIStreamWrite(stream,sSaved,read,lpBuffer,BUFFER_SIZE,AVIIF_KEYFRAME,&plSamples,&plBytes);
		
		slCurrent += read;
		sSaved += read;
		}

	free(lpBuffer);
	AVIStreamRelease(stream);
#endif	// INTERIM_64_BIT
	}


TimeValue DefSoundObj::GetTime()
	{
	int c = calls;
	TimeValue len = (c*period*TIME_TICKSPERSEC)/1000;
	TimeValue t = st + len;
	if (t > t1 && t1!=t0) {
		return (t-t0)%(t1-t0) + t0;
	} else {
		return t;
		}	
	}

void DefSoundObj::Hold()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		theHold.Put(new SoundObjRestore(this));
		SetAFlag(A_HELD);
		}
	}

void DefSoundObj::MapKeys(TimeMap *map,DWORD flags)
	{
	if (wave && !playing) {
		Hold();
		TimeValue st = map->map(range.Start());
		TimeValue et = map->map(range.End());
		range.Set(st,et);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		waveAnim.NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}
	}

void DefSoundObj::SetStartTime(TimeValue t)
	{	
	if (Playing()) Stop();
	Hold();
	range.Set(t,t + range.End()-range.Start());	
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	waveAnim.NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

TimeValue DefSoundObj::GetStartTime()
	{
	return range.Start();
	}

TimeValue DefSoundObj::GetEndTime()
	{
	return range.End();
	}


#define WAVE_FCURVE_HEIGHT	4

int WaveAnimChild::PaintFCurves(			
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcGraph,
			Rect& rcPaint,
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags )
	{
	int org = (rcGraph.top+rcGraph.bottom)/2; //ValueToScreen(0.0f,rcPaint.h()-1,vzoom,vscroll);
	Rect rp = rcPaint;
	Rect rg = rcGraph;

	rg.top    = org - rcGraph.h()/WAVE_FCURVE_HEIGHT;
	rg.bottom = org + rcGraph.h()/WAVE_FCURVE_HEIGHT;
	rp.top    = rg.top;
	rp.bottom = rg.bottom;
	
	snd->Paint(hdc,rg,rp,tzoom,tscroll,flags,1);
	return 0;
	}


int DefSoundObj::Paint(			
		HDC hdc,
		Rect& rcTrack,
		Rect& rcPaint,
		float zoom,
		int scroll,
		DWORD flags,
		int which )
	{
	TimeValue t,tp,p = MSToTimeVal(60000/bpMinute);
	TimeValue tl = ScreenToTime(rcPaint.left,zoom,scroll);
	TimeValue tr = ScreenToTime(rcPaint.right,zoom,scroll) + p;
	
	if (which==0) {
		tl -= tl%p + p;		
		SelectObject(hdc,GetStockObject(BLACK_PEN));	
		SelectObject(hdc,GetStockObject(BLACK_BRUSH));	
		for (t=tl; t<=tr; t+=p) {
			int x = TimeToScreen(t,zoom,scroll);
			if ((t/p)%bpMeasure==0) {			
				Rectangle(hdc,x-3,rcTrack.top+3,x+4,rcTrack.top+7);			
				}		
			Rectangle(hdc,x-1,rcTrack.top+1,x+2,rcTrack.top+9);				
			}
	} else {
		HPEN cpen = CreatePen(PS_SOLID,0,RGB(100,100,100));
		HPEN pen[2][2];
		pen[0][0] = CreatePen(PS_SOLID,0,RGB(0,0,100));
		pen[0][1] = CreatePen(PS_SOLID,0,RGB(0,0,200));
		pen[1][0] = CreatePen(PS_SOLID,0,RGB(100,0,0));
		pen[1][1] = CreatePen(PS_SOLID,0,RGB(200,0,0));

		for (int i=0; i<wave->Channels(); i++) {
			int xl = rcPaint.left;
			int xr = rcPaint.right;
			DWORD val;
			int y, dy, max = wave->Max(i);
			if (max==0) max=1;

			dy = (rcTrack.bottom - rcTrack.top)/
				 (wave->Channels()*2);
			y  = rcTrack.top + dy * (i*2+1);			

			SelectObject(hdc,cpen);
			MoveToEx(hdc,xl,y,NULL);
			LineTo(hdc,xr+1,y);			

			SelectObject(hdc,pen[i]);

			tp = ScreenToTime(xl-1,zoom,scroll) ;
			for (; xl<=xr; xl++) {
				t   = ScreenToTime(xl,zoom,scroll) - range.Start();
				val	= wave->Sample(tp,t,i);
				tp  = t;
				TimeValue t2 = ScreenToTime(xl,zoom,scroll);
				if (t2>=range.Start()&&t2<=range.End()) {
					SelectObject(hdc,pen[i][1]);
				} else {
					val = 0.0f;
					SelectObject(hdc,pen[i][0]);
					}
				MoveToEx(hdc, xl, y - (dy*val)/max, NULL);
				LineTo(hdc, xl, y + (dy*val)/max);
				}
			}

		SelectObject(hdc,GetStockObject(BLACK_PEN));
		DeleteObject(cpen);
		DeleteObject(pen[0][0]);
		DeleteObject(pen[1][0]);
		DeleteObject(pen[0][1]);
		DeleteObject(pen[1][1]);
		}

	return TRACK_DONE;
	}

// CCJ 8.28.00
// Thanks to Matt @ Boomer Labs for giving us this code that 
// provides the name/status of the files in the trackview.
TSTR DefSoundObj::SubAnimName(int i)
{
	if (i==0)
		if (metroActive)
			return GetResString(IDS_RB_METRONOME_ACTIVE);
		else
			return GetResString(IDS_RB_METRONOME_INACTIVE);
	else {
		if (!_tcslen(sndFile))
			return GetResString(IDS_RB_WAVEFORM_NONE);

		TSTR	shortname;

		SplitPathFile(sndFile ,NULL, &shortname);
		TSTR str1 = GetResString(IDS_RB_WAVEFORM_TITLE);
		str1 += shortname;
		if (!waveActive) {
			str1 += _T(" ");
			str1 += GetResString(IDS_RB_WAVEFORM_INACTIVE);
			}

		return str1;
	}
}

#define BPMIMUTE_CHUNK		0x05000
#define BPMEASURE_CHUNK		0x05001
#define SOUNDFILE_CHUNK		0x05002
#define WAVEACTIVE_CHUNK	0x05003
#define METROACTIVE_CHUNK	0x05004
#define WAVESTART_CHUNK		0x05005
#define WAVENOTES_CHUNK		0x05007
#define METRONOTES_CHUNK	0x05008

IOResult DefSoundObj::Save(ISave *isave)
	{
	ULONG nb;
	TimeValue st = range.Start();

	isave->BeginChunk(BPMIMUTE_CHUNK);
	isave->Write(&bpMinute,sizeof(bpMinute),&nb);			
	isave->EndChunk();
	
	isave->BeginChunk(BPMEASURE_CHUNK);
	isave->Write(&bpMeasure,sizeof(bpMeasure),&nb);			
	isave->EndChunk();

	isave->BeginChunk(WAVEACTIVE_CHUNK);
	isave->Write(&waveActive,sizeof(waveActive),&nb);			
	isave->EndChunk();

	isave->BeginChunk(METROACTIVE_CHUNK);
	isave->Write(&metroActive,sizeof(metroActive),&nb);			
	isave->EndChunk();
	 
	isave->BeginChunk(SOUNDFILE_CHUNK);
	isave->WriteWString((TCHAR*)sndFile);
	isave->EndChunk();
	
	// NOTE: I want this to be loaded after the sound file chunk.
	isave->BeginChunk(WAVESTART_CHUNK);
	isave->Write(&st,sizeof(st),&nb);			
	isave->EndChunk();
	
	for (int i=0; i<waveAnim.NumNoteTracks(); i++) {
		isave->BeginChunk(WAVENOTES_CHUNK);
		NoteTrack *nt = waveAnim.GetNoteTrack(i);
		nt->Save(isave);
		isave->EndChunk();
		}
	for (    i=0; i<metroAnim.NumNoteTracks(); i++) {
		isave->BeginChunk(METRONOTES_CHUNK);
		NoteTrack *nt = waveAnim.GetNoteTrack(i);
		nt->Save(isave);
		isave->EndChunk();
		}

	return IO_OK;
	}

IOResult DefSoundObj::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res;

	if (Playing()) Stop();
	if (pavi) {
#ifndef INTERIM_64_BIT	// CCJ
		AVIStreamRelease(pavi);
#endif	// INTERIM_64_BIT
		pavi = NULL;
		}
	if (wave) delete wave;
	wave = NULL;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case WAVENOTES_CHUNK: {
				NoteTrack *nt = NewDefaultNoteTrack();
				nt->Load(iload);
				waveAnim.AddNoteTrack(nt);
				break;
				}

			case METRONOTES_CHUNK: {
				NoteTrack *nt = NewDefaultNoteTrack();
				nt->Load(iload);
				metroAnim.AddNoteTrack(nt);
				break;
				}

			case BPMIMUTE_CHUNK:
				res=iload->Read(&bpMinute,sizeof(bpMinute),&nb);
				break;

			case BPMEASURE_CHUNK:
				res=iload->Read(&bpMeasure,sizeof(bpMeasure),&nb);
				break;

			case WAVEACTIVE_CHUNK:
				res=iload->Read(&waveActive,sizeof(waveActive),&nb);
				break;

			case METROACTIVE_CHUNK:
				res=iload->Read(&metroActive,sizeof(metroActive),&nb);
				break;

			case SOUNDFILE_CHUNK: {
				wchar_t *buf = NULL;
				res=iload->ReadWStringChunk(&buf);
				sndFile = buf;				

#ifndef INTERIM_64_BIT	// CCJ
				PAVISTREAM stream = GetAudioStream(sndFile,iload->GetDir(APP_SOUND_DIR));
				if (stream) {
					BOOL lineChange = FALSE;					
					pavi    = stream;					
					wave = new FilteredWave(stream,FILTER_RES);
					range.Set(wave->Start(),wave->End());
					NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					NotifyDependents(FOREVER,PART_ALL,REFMSG_OBREF_CHANGE,TREE_VIEW_CLASS_ID);
				} else {
					if (sndFile.Length()) {
						TSTR buf1 = GetResString(IDS_RB_CANTOPENSOUND);
						TSTR buf2 = GetResString(IDS_RB_SOUND);
						GetCOREInterface()->Log()->LogEntry(SYSLOG_WARN,TRUE,buf2,buf1);
						}
					sndFile = (TCHAR*)NULL;	
					}
#endif	// INTERIM_64_BIT

				break;
				}

			case WAVESTART_CHUNK: {
				TimeValue st;
				res=iload->Read(&st,sizeof(st),&nb);
				// If the wave file couldn't be loaded then don't set the range.
				if (wave) {
					range.Set(st,range.End()+st-range.Start());
					}
				break;
				}
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	
	return IO_OK;
	}




void DefSoundObj::SetSoundFile(PAVISTREAM stream,TSTR &file)
	{	
#ifndef INTERIM_64_BIT	// CCJ
	if (pavi) AVIStreamRelease(pavi);
	pavi    = stream;
	sndFile = file;
	if (wave) delete wave;	
	wave = new FilteredWave(stream,FILTER_RES);
	range.Set(wave->Start(),wave->End());
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_OBREF_CHANGE,TREE_VIEW_CLASS_ID);
	SetSaveRequiredFlag(TRUE);
#endif	// INTERIM_64_BIT
	}

BOOL DefSoundObj::SetSoundFileName(TSTR name)
	{
#ifndef INTERIM_64_BIT	// CCJ
	PAVISTREAM stream = 
		GetAudioStream(name,
		GetCOREInterface()->GetDir(APP_SOUND_DIR));
	if (stream) {
		waveActive = TRUE;
		SetSoundFile(stream,name);
		return TRUE;
	} else {
		return FALSE;
		}
#else // INTERIM_64_BIT
	return FALSE;
#endif	// INTERIM_64_BIT
	}

void* DefSoundObj::GetInterface(ULONG id)
	{
	if (id==I_WAVESOUND) return (IWaveSound*)this;
	if (id==I_WAVEPAINT) return (IWavePaint*)this;

	else return SoundObj::GetInterface(id);
	}



static TSTR lastSoundDir;
static BOOL lastSoundDirInit = FALSE;

TSTR &GetLastSoundDir(IObjParam *ip)
	{
	if (!lastSoundDirInit) {
		lastSoundDirInit = TRUE;
		lastSoundDir     = ip->GetDir(APP_SOUND_DIR);
		}
	return lastSoundDir;
	}


static INT_PTR CALLBACK SoundParamsWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
		// WIN64 Cleanup: Shuler
	static BOOL picked, removed;	
	static TSTR file;
	static DWORD bpMinute, bpMeasure;
	// >>**BL - These parms are required for updating TV
	static BOOL	bwaveActive, bmetroActive;
	// <<**BL

	DefSoundObj *snd = (DefSoundObj*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
			// WIN64 Cleanup: Shuler

	switch (msg) {
		case WM_INITDIALOG: {			
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			// WIN64 Cleanup: Shuler
			snd = (DefSoundObj*)lParam;
			
			snd->ip->RegisterDlgWnd(hWnd);

			picked  = FALSE;
			removed = FALSE;

			ISpinnerControl *spin; 
			
			spin = GetISpinner(GetDlgItem(hWnd,IDC_BPMINUTESPIN));
			spin->SetLimits(1,60000,FALSE);
			spin->SetScale(0.1f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_BPMINUTE),EDITTYPE_INT);
			spin->SetValue((int)snd->bpMinute,FALSE);
			ReleaseISpinner(spin);
			bpMinute = snd->bpMinute;

			spin = GetISpinner(GetDlgItem(hWnd,IDC_BPMEASURESPIN));
			spin->SetLimits(2,10000,FALSE);
			spin->SetScale(0.1f);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_BPMEASURE),EDITTYPE_INT);
			spin->SetValue((int)snd->bpMeasure,FALSE);
			ReleaseISpinner(spin);
			bpMeasure = snd->bpMeasure;

			if (snd->sndFile.Length()) {
				SetWindowText(GetDlgItem(hWnd,IDC_SOUNDNAME),snd->sndFile);
				EnableWindow(GetDlgItem(hWnd,IDC_REMOVESOUND),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_RELOADSOUND),TRUE);
			} else {
				SetWindowText(GetDlgItem(hWnd,IDC_SOUNDNAME),GetResString(IDS_RB_NONE));
				EnableWindow(GetDlgItem(hWnd,IDC_REMOVESOUND),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_RELOADSOUND),FALSE);
				}
			
			if (snd->waveActive) {
				CheckDlgButton(hWnd,IDC_SOUNDACTIVE,TRUE);
			} else {
				CheckDlgButton(hWnd,IDC_SOUNDACTIVE,FALSE);
				}

			if (snd->metroActive) {
				CheckDlgButton(hWnd,IDC_METROACTIVE,TRUE);
			} else {
				CheckDlgButton(hWnd,IDC_METROACTIVE,FALSE);
				}

			break;
			}

		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl*)lParam; 
			switch (LOWORD(wParam)) {
				case IDC_BPMINUTESPIN:
					snd->bpMinute = (int)spin->GetIVal();
					SetSaveRequiredFlag(TRUE);
					break;

				case IDC_BPMEASURESPIN:
					snd->bpMeasure = (int)spin->GetIVal();
					SetSaveRequiredFlag(TRUE);
					break;
				}
			snd->metroAnim.NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CHOOSESOUND:
#ifndef INTERIM_64_BIT	// CCJ
					if (GetSoundFileName(hWnd,file,GetLastSoundDir(snd->ip))) {
						picked  = TRUE;
						removed = FALSE;
						CheckDlgButton(hWnd,IDC_SOUNDACTIVE,TRUE);
						SetWindowText(GetDlgItem(hWnd,IDC_SOUNDNAME),file);
						EnableWindow(GetDlgItem(hWnd,IDC_REMOVESOUND),TRUE);
						}
#endif	// INTERIM_64_BIT
					break;

				case IDC_REMOVESOUND:
					removed = TRUE;	 
					picked  = FALSE;
					file    = (TCHAR*)NULL;
					SetWindowText(GetDlgItem(hWnd,IDC_SOUNDNAME),GetResString(IDS_RB_NONE));
					EnableWindow(GetDlgItem(hWnd,IDC_REMOVESOUND),FALSE);
					break;
							
				case IDC_RELOADSOUND: {
#ifndef INTERIM_64_BIT	// CCJ
					PAVISTREAM stream = 
						GetAudioStream(snd->sndFile,snd->ip->GetDir(APP_SOUND_DIR));
					if (stream) {
						snd->SetSoundFile(stream,file);
					} else {
						MessageBox(hWnd,
							GetResString(IDS_RB_CANTOPENSOUND),
							GetResString(IDS_RB_SOUND),
							MB_OK|MB_ICONSTOP|MB_TASKMODAL);
						}
#endif	// INTERIM_64_BIT
					break;
					}

				case IDOK:
#ifndef INTERIM_64_BIT	// CCJ
					if (picked) {
						PAVISTREAM stream = 
							GetAudioStream(file,snd->ip->GetDir(APP_SOUND_DIR));
						if (stream) {
							snd->SetSoundFile(stream,file);
						} else {
							TSTR buf1 = GetResString(IDS_RB_CANTOPENSOUND);
							TSTR buf2 = GetResString(IDS_RB_SOUND);
							MessageBox(hWnd,buf1,buf2,MB_OK|MB_ICONSTOP|MB_TASKMODAL);
							}
					} else
					if (removed) {
						if (snd->pavi) AVIStreamRelease(snd->pavi);
						snd->pavi    = NULL;
						snd->sndFile = (TCHAR*)NULL;
						if (snd->wave) delete snd->wave;
						snd->wave = NULL;
						snd->range.SetEmpty();
						snd->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
						// **>>BL - handled below
						//snd->NotifyDependents(FOREVER,PART_ALL,REFMSG_OBREF_CHANGE,TREE_VIEW_CLASS_ID);
						// **<<BL
						SetSaveRequiredFlag(TRUE);
						}
#endif	// INTERIM_64_BIT
					// **>>BL - If the Activity Status has changed, refresh the trackview
					bwaveActive = snd->waveActive;
					snd->waveActive  = IsDlgButtonChecked(hWnd,IDC_SOUNDACTIVE);
					bmetroActive = snd->metroActive;
					snd->metroActive = IsDlgButtonChecked(hWnd,IDC_METROACTIVE);
					if ((removed) || (bwaveActive != snd->waveActive) || (bmetroActive != snd->metroActive))
						snd->NotifyDependents(FOREVER,PART_ALL,REFMSG_OBREF_CHANGE,TREE_VIEW_CLASS_ID);
					// **<<BL

					DestroyWindow(hWnd);
					break;

				case IDCANCEL:
					snd->bpMinute  = bpMinute;
					snd->bpMeasure = bpMeasure;
					snd->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					snd->metroAnim.NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					DestroyWindow(hWnd);
					break;
				}
			break;

		case WM_DESTROY:
			snd->ip->UnRegisterDlgWnd(hWnd);
			snd->hParams = NULL;
			break;

		default:
			return 0;			
		}
	return 1;
	}

void DefSoundObj::EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags)
	{
	// CCJ - 8.28.00
	// Thanks to Matt @ Boomer Labs for giving us this code that 
	// turns off the animation playback instead of bringing up a dialog box
	// when trying to edit sound parameters while the animation is playing.

	//	static BOOL block = FALSE;
	//	if (block) return;

	if (Playing()) {
		// Stop Playback before editting this entry
		ip->EndAnimPlayback();
/*
		block = TRUE;
		ip->Stop();
		TSTR buf1 = GetResString(IDS_RB_CANTEDITSOUND);
		TSTR buf2 = GetResString(IDS_RB_SOUND);
		MessageBox(NULL,buf1,buf2,MB_OK|MB_ICONSTOP|MB_TASKMODAL);
		block = FALSE;
		return;
*/
		}
	this->ip = ip;

	if (!hParams) {
		hParams = CreateDialogParam(
			getResMgr().getHInst(RES_ID_RB),
			MAKEINTRESOURCE(IDD_SOUNDPARAMS),
			hParent,
			SoundParamsWndProc,
			(LPARAM)this);
	} else {
		SetActiveWindow(hParams);
		}
	}


static LONG_PTR CALLBACK SoundDummyWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
		// WIN64 Cleanup: Shuler
	switch (msg) {
		case WM_BEEPME:
			Beep(wParam,lParam);
			break;

		case MM_WOM_OPEN:
		case MM_WOM_DONE:
		case MM_WOM_CLOSE:
#ifndef INTERIM_64_BIT	// CCJ
			aviaudioMessage(hWnd,msg,wParam,lParam);
#endif	// INTERIM_64_BIT
			break;

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}
	return 0;
	}

void DefSoundObj::SetupDummyWindow()
	{
	static BOOL setup = FALSE;
	if (setup) return;
	setup = TRUE;	

#ifndef INTERIM_64_BIT	// CCJ
	AVIFileInit();
#endif	// INTERIM_64_BIT

	WNDCLASS  wc;

	wc.style         = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.lpfnWndProc   = SoundDummyWndProc;
    wc.lpszClassName = _T("SOUNDDUMMYWINDOW");

	RegisterClass(&wc);	
	}

void DefSoundObj::PaintWave(HDC hdc, RECT* rect, Interval i)
	{
	Rect r;
	r.top = rect->top;
	r.left = rect->left;
	r.bottom = rect->bottom;
	r.right = rect->right;

	if (wave) {
		float zoom = (float)(rect->right-rect->left)/(float)i.Duration();
		int scroll = zoom*i.Start();
		Paint(hdc,r,r,zoom,scroll,0,1);
		}
	}

#endif // WIN95STUFF

/*----------------------------------------------------------------------*/

SoundObj *NewDefaultSoundObj()
	{
	#ifdef WIN95STUFF
	return new DefSoundObj();
	#else
	return NULL;
	#endif
	}


