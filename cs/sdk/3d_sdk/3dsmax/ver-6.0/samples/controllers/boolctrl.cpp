/**********************************************************************
 *<
	FILE: boolctrl.cpp

	DESCRIPTION: A boolean controller

	CREATED BY: Rolf Berteig

	HISTORY: 11/18/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "ctrl.h"
#include "units.h"
#include "exprlib.h"

// RB 10/2/2000: This enables a change where on/off keys are slid from the
// right in place of adding a new key to prevent flipping the state of 
// downstream animation.
// The rule is: to change the on/off state at time 't', if a key exists at a time >t,
// then slide that key to time 't' otherwise make a new key.
#define BOOLCTRL_NO_FLIPPING

#define BOOL_CONTROL_CLASS_ID	Class_ID(0x984b8d27,0x938f3e43)
#define BOOL_CONTROL_CNAME		GetString(IDS_RB_BOOLCONTROL)

#define KEY_SELECTED	(1<<0)
#define KEY_FLAGGED		(1<<1)

#define MAKEBOOL(fl) ( (fl)<=0.0f ? FALSE : TRUE )

#ifdef BOOLCTRL_NO_FLIPPING
#define ADDKEY_NOSLIDE	(1<<31)
#endif


class BKey {
	public:
		TimeValue time;
		DWORD flags;

		BKey() {flags=0;}
		BKey(TimeValue t) {time=t; flags=0;}
	};


static int __cdecl CompareBKeys(const BKey *k1, const BKey *k2)
	{
	if (k1->time < k2->time) return -1;
	if (k1->time > k2->time) return 1;
	return 0;
	}

class SetKeyBuffer : public Tab<BKey> {
	public:
		BOOL oldOnOffVal;
		BOOL startOnOff;

		SetKeyBuffer() : Tab<BKey>() {}
		SetKeyBuffer(BOOL onOff, BOOL start) : Tab<BKey>() {oldOnOffVal=onOff; startOnOff=start;}
		SetKeyBuffer(BOOL onOff, BOOL start, Tab<BKey> &table) : Tab<BKey>(table) {
			oldOnOffVal = onOff;
			startOnOff  = start;
			}
		int GetKeyIndex(TimeValue t);
		SetKeyBuffer &operator=(SetKeyBuffer &from) {
			Tab<BKey>::operator=(from);
			oldOnOffVal = from.oldOnOffVal;
			startOnOff  = from.startOnOff;
			return *this;
			}
	};

class BoolControl : public StdControl {
	public:				
		Tab<BKey> keys;
		Interval range, valid;
		BOOL onOff, startOnOff, rangeLinked;		
		SetKeyBuffer *setkeyBuffer;

		BoolControl();
		~BoolControl();
				
		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 1;}
		BOOL IsAnimated() {return keys.Count()?TRUE:FALSE;}
		Class_ID ClassID() {return BOOL_CONTROL_CLASS_ID;} 
		SClass_ID SuperClassID() {return CTRL_FLOAT_CLASS_ID;}
		void GetClassName(TSTR& s) {s = BOOL_CONTROL_CNAME;}

		// Set-key mode support
		void CommitSetKeyBuffer(TimeValue t);
		void RevertSetKeyBuffer();
		void RevertSetKeyBuffer(BOOL clearHeldFlag);
		BOOL SetKeyBufferPresent() {return setkeyBuffer? TRUE: FALSE;}
		void HoldSetKeyBuffer();

		// Reference methods
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		RefTargetHandle Clone(RemapDir &remap = NoRemap());

		// Control methods				
		void Copy(Control *from);
		BOOL IsLeaf() {return TRUE;}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		
		void HoldTrack();
		Interval GetTimeRange(DWORD flags);
		void EditTimeRange(Interval range,DWORD flags);
		void MapKeys(TimeMap *map,DWORD flags );
		
		int NumKeys() {return keys.Count();}
		TimeValue GetKeyTime(int index) {return keys[index].time;}
		int GetKeyIndex(TimeValue t);		
		void DeleteKeyAtTime(TimeValue t);
		BOOL IsKeyAtTime(TimeValue t,DWORD flags);		
		void DeleteTime(Interval iv, DWORD flags);
		void ReverseTime(Interval iv, DWORD flags);
		void ScaleTime(Interval iv, float s);
		void InsertTime(TimeValue ins, TimeValue amount);
		BOOL SupportTimeOperations() {return TRUE;}
		void DeleteKeys(DWORD flags);
		void DeleteKeyByIndex(int index);
		void SelectKeys(TrackHitTab& sel, DWORD flags);
		void SelectKeyByIndex(int i,BOOL sel);
		void FlagKey(TrackHitRecord hit);
		int GetFlagKeyIndex();
		int NumSelKeys();
		void CloneSelectedKeys(BOOL offset=FALSE);
		void AddNewKey(TimeValue t,DWORD flags);		
		BOOL IsKeySelected(int index);
		BOOL CanCopyTrack(Interval iv, DWORD flags) {return TRUE;}
		BOOL CanPasteTrack(TrackClipObject *cobj,Interval iv, DWORD flags) {return cobj->ClassID()==ClassID();}
		TrackClipObject *CopyTrack(Interval iv, DWORD flags);
		void PasteTrack(TrackClipObject *cobj,Interval iv, DWORD flags);
		int GetSelKeyCoords(TimeValue &t, float &val,DWORD flags);
		void SetSelKeyCoords(TimeValue t, float val,DWORD flags);
		int SetSelKeyCoordsExpr(ParamDimension *dim,TCHAR *timeExpr, TCHAR *valExpr, DWORD flags);

		int HitTestTrack(			
			TrackHitTab& hits,
			Rect& rcHit,
			Rect& rcTrack,			
			float zoom,
			int scroll,
			DWORD flags);
		int PaintTrack(
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcTrack,
			Rect& rcPaint,
			float zoom,
			int scroll,
			DWORD flags);
		

		// StdControl methods
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE) {GetValue(t,val,valid,method);}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method);
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);		
		void *CreateTempValue() {return new float;}
		void DeleteTempValue(void *val) {delete (float*)val;}
		void ApplyValue(void *val, void *delta) {*((float*)val) += *((float*)delta);}
		void MultiplyValue(void *val, float m) {*((float*)val) *= m;}

		void SortKeys();
		
	};

class BoolClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new BoolControl();}
	const TCHAR *	ClassName() {return BOOL_CONTROL_CNAME;}
	SClass_ID		SuperClassID() {return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() {return BOOL_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};
static BoolClassDesc boolCD;
ClassDesc* GetBoolControlDesc() {return &boolCD;}



class BoolControlRestore : public RestoreObj {
	public:
		Tab<BKey> undo;
		Tab<BKey> redo;
		Interval urange, rrange;
		BOOL onOffU, startOnOffU, rangeLinkedU;
		BOOL onOffR, startOnOffR, rangeLinkedR;
		BoolControl *cont;

		BoolControlRestore(BoolControl *c) {
			cont   = c;
			undo   = c->keys;			
			urange = c->range;
			onOffU  = c->onOff;
			startOnOffU  = c->startOnOff;
			rangeLinkedU = c->rangeLinked;
			}
		
		void Restore(int isUndo) {
			if (isUndo) {
				redo   = cont->keys;
				rrange = cont->range;
				onOffR  = cont->onOff;
				startOnOffR  = cont->startOnOff;
				rangeLinkedR = cont->rangeLinked;
				}
			cont->keys  = undo;
			cont->range = urange;
			cont->onOff       = onOffU;
			cont->startOnOff  = startOnOffU;
			cont->rangeLinked = rangeLinkedU;
			cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			}

		void Redo() {
			cont->keys  = redo;
			cont->range = rrange;
			cont->onOff       = onOffR;
			cont->startOnOff  = startOnOffR;
			cont->rangeLinked = rangeLinkedR;
			cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			}

		int Size() {return 1;}
		void EndHold() {cont->ClearAFlag(A_HELD);}
	};


class BoolClipObject : public TrackClipObject {
	public:
		Tab<BKey> tab;

		Class_ID ClassID() {return BOOL_CONTROL_CLASS_ID;}
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
		void DeleteThis() {delete this;}

		BoolClipObject(Interval iv) : TrackClipObject(iv) {}
	};



//-----------------------------------------------------------------------------

BoolControl::BoolControl() 
	{
	startOnOff   = TRUE; 
	rangeLinked  = TRUE;
	setkeyBuffer = NULL;
	}

BoolControl::~BoolControl()
	{
	if (setkeyBuffer) delete setkeyBuffer;
	setkeyBuffer = NULL;
	}

RefTargetHandle BoolControl::Clone(RemapDir &remap)
	{
	BoolControl *bc = new BoolControl;
	bc->range      = range;
	bc->valid      = valid;
	bc->keys       = keys;
	bc->onOff      = onOff;
	bc->startOnOff = startOnOff;
	CloneControl(bc,remap);
	BaseClone(this, bc, remap);
	return bc;
	}

#define KEYTIME_CHUNKID		0x0100
#define KEYFLAGS_CHUNKID	0x0110
#define NUMKEYS_CHUNKID		0x0130
#define INITSTATE_CHUNKID	0x0140

IOResult BoolControl::Save(ISave *isave)
	{
	ULONG nb;
	int ct = keys.Count();
	
	// Save the number of keys
	isave->BeginChunk(NUMKEYS_CHUNKID);
	isave->Write(&ct,sizeof(ct),&nb);
	isave->EndChunk();

	// Write each key
	for (int i=0; i<ct; i++) {
		isave->BeginChunk(KEYTIME_CHUNKID);
		isave->Write(&keys[i].time,sizeof(keys[i].time),&nb);
		isave->EndChunk();

		isave->BeginChunk(KEYFLAGS_CHUNKID);
		isave->Write(&keys[i].flags,sizeof(keys[i].flags),&nb);
		isave->EndChunk();		
		}

	// RB 5/3/99: This wasn't getting saved but it should be...
	isave->BeginChunk(INITSTATE_CHUNKID);
	isave->Write(&startOnOff,sizeof(startOnOff),&nb);
	isave->EndChunk();	
	
	return IO_OK;
	}

IOResult BoolControl::Load(ILoad *iload)
	{
	ULONG nb;
	int ct = 0;	
	IOResult res;

	if (IO_OK!=(res=iload->OpenChunk())) return res;
	assert(iload->CurChunkID()==NUMKEYS_CHUNKID);
	iload->Read(&ct,sizeof(ct),&nb);
	iload->CloseChunk();
	
	keys.SetCount(ct);

	for (int i=0; i<ct; i++) {
		iload->OpenChunk();
		assert(iload->CurChunkID()==KEYTIME_CHUNKID);
		iload->Read(&keys[i].time,sizeof(keys[i].time),&nb);
		iload->CloseChunk();
		
		iload->OpenChunk();
		assert(iload->CurChunkID()==KEYFLAGS_CHUNKID);
		iload->Read(&keys[i].flags,sizeof(keys[i].flags),&nb);
		iload->CloseChunk();		
		}

	// RB 5/3/99: Looks like this was originally setup to assume the chunks would be
	// in a fixed order. Which was OK then, but now that I have to add a chunk this
	// has to be more general. I'm going to leave the above code alone since those
	// chunks should always be present. We'll start looking for more chunks after...

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {	
			case INITSTATE_CHUNKID:
				iload->Read(&startOnOff,sizeof(startOnOff),&nb);
				break;
			}

		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	return IO_OK;
	}

void BoolControl::Copy(Control *from)
	{	
	if (from) {
		if (from->SuperClassID()!=CTRL_FLOAT_CLASS_ID) return;
		TimeValue t;
		float s;
		Interval iv;
		TimeValue tpf = GetTicksPerFrame();
		Interval v = from->GetTimeRange(TIMERANGE_ALL);

		BOOL state = 1;
		TimeValue astart = GetAnimStart();
		if (v.Start()>astart) {
			from->GetValue(astart, &s, iv);
			if (s<=0.0f) {
				state = 0;

#ifdef BOOLCTRL_NO_FLIPPING
				AddNewKey(astart, ADDKEY_NOSLIDE);
#else
				AddNewKey(astart, 0);
#endif
				}
			}

		for (t = v.Start(); t<=v.End(); t+=tpf) {
			from->GetValue(t, &s, iv);
			if (state) {
				if (s<=0.0f) {
					state = 0;
#ifdef BOOLCTRL_NO_FLIPPING
					AddNewKey(t, ADDKEY_NOSLIDE);					
#else
					AddNewKey(t, 0);
#endif
					}
				}
			else {
				if (s>0.0f) {
					state = 1;
#ifdef BOOLCTRL_NO_FLIPPING
					AddNewKey(t, ADDKEY_NOSLIDE);					
#else
					AddNewKey(t, 0);
#endif
					}
				}
			}
		}
	}


void BoolControl::CommitSetKeyBuffer(TimeValue t)
	{
	if (setkeyBuffer) {
		onOff      = setkeyBuffer->oldOnOffVal;
		HoldTrack();
		keys       = *setkeyBuffer;		
		startOnOff = setkeyBuffer->startOnOff;
		delete setkeyBuffer;
		setkeyBuffer = NULL;
		valid.SetEmpty();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

void BoolControl::RevertSetKeyBuffer()
	{
	if (setkeyBuffer) {
		HoldSetKeyBuffer();
		onOff = setkeyBuffer->oldOnOffVal;
		delete setkeyBuffer;
		setkeyBuffer = NULL;
		valid.SetEmpty();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

void BoolControl::RevertSetKeyBuffer(BOOL clearHeldFlag)
	{
	RevertSetKeyBuffer();
	// after tossing the set-key buffer, we are probably about to call
	// HoldTrack() which checks the 'held' flag before doing a theHold.Put().
	// We want both operations to do a put.
	ClearAFlag(A_HELD);
	}


class SetKeyBufferRest : public RestoreObj {
	public:
		BoolControl *cont;
		SetKeyBuffer svBufUndo;
		SetKeyBuffer svBufRedo;
		BOOL curvalUndo, curvalRedo;
		BOOL noBufUndo, noBufRedo;

		SetKeyBufferRest(BoolControl *c) {
			cont = c;
			curvalUndo = cont->onOff;
			if (cont->setkeyBuffer) {
				noBufUndo = FALSE;
				svBufUndo = *(cont->setkeyBuffer);
			} else {
				noBufUndo = TRUE;
				}
			noBufRedo = FALSE;
			}

		void Restore(int isUndo) {
			if (isUndo) {
				if (cont->setkeyBuffer) {
					noBufRedo = FALSE;
					svBufRedo = *(cont->setkeyBuffer);
				} else {
					noBufRedo = TRUE;
					}
				curvalRedo = cont->onOff;
				}
			if (noBufUndo) {
				if (cont->setkeyBuffer) {
					delete cont->setkeyBuffer;
					cont->setkeyBuffer = NULL;
					}
			} else {
				if (!cont->setkeyBuffer) {
					cont->setkeyBuffer = new SetKeyBuffer(cont->onOff, cont->startOnOff);
					}
				*(cont->setkeyBuffer) = svBufUndo;
				}
			cont->onOff = curvalUndo;
			cont->valid.SetEmpty();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo() {
			if (noBufRedo) {
				if (cont->setkeyBuffer) {
					delete cont->setkeyBuffer;
					cont->setkeyBuffer = NULL;
					}
			} else {
				if (!cont->setkeyBuffer) {
					cont->setkeyBuffer = new SetKeyBuffer(cont->onOff, cont->startOnOff);
					}
				*(cont->setkeyBuffer) = svBufRedo;
				}
			cont->onOff = curvalRedo;
			cont->valid.SetEmpty();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() {cont->ClearAFlag(A_HELD);}
		TSTR Description() { return TSTR(_T("SetKeyBufferRest")); }
	};

void BoolControl::HoldSetKeyBuffer()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {		
		theHold.Put(new SetKeyBufferRest(this));
		SetAFlag(A_HELD);
		}
	}

void BoolControl::HoldTrack()
	{
	if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
		theHold.Put(new BoolControlRestore(this));
		SetAFlag(A_HELD);
		}
	}

void BoolControl::EditTimeRange(Interval range,DWORD flags)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	if (flags&EDITRANGE_LINKTOKEYS && keys.Count()) {
		this->range.Set(keys[0].time,keys[keys.Count()-1].time);
		rangeLinked = TRUE;
	} else {		
		rangeLinked = FALSE;
		this->range = range;		
		}
	valid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Interval BoolControl::GetTimeRange(DWORD flags)
	{
	if (rangeLinked && keys.Count()) 
		 return Interval(keys[0].time,keys[keys.Count()-1].time);
	else return range;
	}


int BoolControl::GetSelKeyCoords(TimeValue &t, float &val,DWORD flags) 
	{
	BOOL tfound=FALSE, tuncommon = FALSE;
	int n = keys.Count(), res;
	TimeValue atime;	
	
	for (int i = 0; i < n; i++ ) {		
		if (keys[i].flags & KEY_SELECTED) {
			if (tfound) {
				if (keys[i].time!=atime) {
					return KEYS_MULTISELECTED;
					}
			} else {
				tfound = TRUE;
				atime = keys[i].time;
				}
			}		
		if (tuncommon) return KEYS_MULTISELECTED;
		}
	t   = atime;	
	res = 0;
	if (tfound && !tuncommon) {
		res	|= KEYS_COMMONTIME;
		}	
	if (!tfound) {
		res = KEYS_NONESELECTED;
		}
	return res;
	}

int BoolControl::SetSelKeyCoordsExpr(
		ParamDimension *dim,
		TCHAR *timeExpr, TCHAR *valExpr, DWORD flags)
	{
	Expr texpr;
	float tfin, tfout=0.0f;

	if (timeExpr) {
		texpr.defVar(SCALAR_VAR,KEYCOORDS_TIMEVAR);
		if (texpr.load(timeExpr)!=EXPR_NORMAL) return KEYCOORDS_EXPR_ERROR;		
		}

	int n = keys.Count();
	if (!n) return KEYCOORDS_EXPR_OK;
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	for (int i=0; i<n; i++) {
		if (!(flags&KEYCOORDS_VALUEONLY)) {			
			if (keys[i].flags & KEY_SELECTED) {
				tfin = float(keys[i].time)/float(GetTicksPerFrame());
				texpr.eval(&tfout, 1, &tfin);
				keys[i].time = int(tfout*GetTicksPerFrame());
				}
			}
		}
	valid.SetEmpty();
	SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	return KEYCOORDS_EXPR_OK;
	}

void BoolControl::SetSelKeyCoords(TimeValue t, float val,DWORD flags)
	{
	int n = keys.Count();
	if (!n) return;
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	for (int i = 0; i < n; i++ ) {		
		if (!(flags&KEYCOORDS_VALUEONLY)) {			
			if (keys[i].flags & KEY_SELECTED) {
				keys[i].time = t;
				}		
			}
		}	
	valid.SetEmpty();
	SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::MapKeys(TimeMap *map,DWORD flags)
	{
	int n = keys.Count();
	BOOL changed = FALSE;
	if (!n) return;
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	if (flags&TRACK_MAPRANGE) {		
		TimeValue t0 = map->map(range.Start());
		TimeValue t1 = map->map(range.End());
		range.Set(t0,t1);
		changed = TRUE;
		}	

	if (flags&TRACK_DOALL) {
		for (int i=0; i<n; i++) {			
			keys[i].time = map->map(keys[i].time);
			changed = TRUE;
			}
	} else 
	if (flags&TRACK_DOSEL) {
		BOOL slide = flags&TRACK_SLIDEUNSEL;
		TimeValue delta = 0, prev;
		int start, end, inc;
		if (flags&TRACK_RIGHTTOLEFT) {
			start = n-1;
			end = -1;
			inc = -1;
		} else {
			start = 0;
			end = n;
			inc = 1;
			} 
		for (int i=start; i!=end; i+=inc) {			
			if (keys[i].flags & KEY_SELECTED) {
				prev = keys[i].time;
				keys[i].time = map->map(keys[i].time);
				delta = keys[i].time - prev;
				changed = TRUE;
			} else if (slide) {
				keys[i].time += delta;
				}
			}
		}
	
	if (changed) {
		valid.SetEmpty();
		SortKeys();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}


int BoolControl::GetKeyIndex(TimeValue t)
	{
	for (int i=0; i<keys.Count(); i++) {
		if (keys[i].time==t) return i;
		if (keys[i].time>t) return -1; // RB 10/2/2000 changed from "keys[i].time<t". 
		}
	return -1;
	}

void BoolControl::DeleteKeyAtTime(TimeValue t)
	{
	int index = GetKeyIndex(t);
	if (index>=0) {
		
		RevertSetKeyBuffer(TRUE);
		HoldTrack();
		keys.Delete(index,1);
		valid.SetEmpty();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

BOOL BoolControl::IsKeyAtTime(TimeValue t,DWORD flags)
	{
	for (int i=0; i<keys.Count(); i++) {
		if (keys[i].time>t) return FALSE;
		if (keys[i].time==t) return TRUE;		
		}
	return FALSE;
	}

void BoolControl::DeleteTime(Interval iv, DWORD flags)
	{
	Interval test = TestInterval(iv,flags);
	int n = keys.Count();	
	int d = iv.Duration()-1;
	if (d<0) d = 0;
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	for (int i = n-1; i >= 0; i--) {
		if (test.InInterval(keys[i].time)) {
			keys.Delete(i,1);
		} else 
		if (!(flags&TIME_NOSLIDE)) {			
			if (keys[i].time > test.End()) {
				keys[i].time -= d;
				}
			}
		}		    
	
	valid.SetEmpty();
	SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::ReverseTime(Interval iv, DWORD flags)
	{
	Interval test = TestInterval(iv,flags);
	int n = keys.Count();
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	for (int i = 0; i < n; i++) {		
		if (test.InInterval(keys[i].time)) {
			TimeValue delta = keys[i].time - iv.Start();
			keys[i].time = iv.End()-delta;			
			}
		}
	valid.SetEmpty();
	SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::ScaleTime(Interval iv, float s)
	{
	int n = keys.Count();
	TimeValue delta = int(s*float(iv.End()-iv.Start())) + iv.Start()-iv.End();
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	for (int i = 0; i < n; i++) {		
		if (iv.InInterval(keys[i].time)) {
			keys[i].time = 
				int(s*float(keys[i].time - iv.Start())) + iv.Start();
		} else 
		if (keys[i].time > iv.End()) {
			keys[i].time += delta;
			}
		}
	valid.SetEmpty();
	SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::InsertTime(TimeValue ins, TimeValue amount)
	{
	int n = keys.Count();		
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	for (int i = 0; i < n; i++) {		
		if (keys[i].time >= ins) {
			keys[i].time += amount;
			}		
		}
	valid.SetEmpty();
	SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::DeleteKeys(DWORD flags)
	{
	int n = keys.Count();		
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	
	for (int i = n-1; i >= 0; i--) {
		if (flags&TRACK_DOALL || keys[i].flags&KEY_SELECTED) {
			keys.Delete(i,1);
			}
		}	
	valid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::DeleteKeyByIndex(int index)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	keys.Delete(index,1);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::SelectKeyByIndex(int i,BOOL sel)
	{
	HoldTrack();
	if (sel) keys[i].flags |=  KEY_SELECTED;
	else     keys[i].flags &= ~KEY_SELECTED;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::SelectKeys(TrackHitTab& sel, DWORD flags)
	{
	HoldTrack();
	
	if (flags&SELKEYS_CLEARKEYS) {
		int n = keys.Count();
		for (int i = 0; i < n; i++ ) {
			keys[i].flags &= ~KEY_SELECTED;
			}
		}
	
	if (flags&SELKEYS_DESELECT) {
		for (int i = 0; i < sel.Count(); i++ ) {			
			keys[sel[i].hit].flags &= ~KEY_SELECTED;
			}		
		} 	
	if (flags&SELKEYS_SELECT) {			
		for (int i = 0; i < sel.Count(); i++ ) {
			keys[sel[i].hit].flags |= KEY_SELECTED;
			}
		}	
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::FlagKey(TrackHitRecord hit)
	{
	int n = keys.Count();
	for (int i = 0; i < n; i++) {
		keys[i].flags &= ~KEY_FLAGGED;
		}
	assert(hit.hit>=0&&hit.hit<(DWORD)n);
	keys[hit.hit].flags |= KEY_FLAGGED;
	}

int BoolControl::GetFlagKeyIndex()
	{
	int n = keys.Count();
	for (int i = 0; i < n; i++) {
		if (keys[i].flags & KEY_FLAGGED) {
			return i;
			}
		}
	return -1;
	}

int BoolControl::NumSelKeys()
	{
	int n = keys.Count();
	int c = 0;
	for ( int i = 0; i < n; i++ ) {
		if (keys[i].flags & KEY_SELECTED) {
			c++;
			}
		}
	return c;
	}

void BoolControl::CloneSelectedKeys(BOOL offset)
	{
	int n = keys.Count();			
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	BOOL changed = FALSE;

	for (int i = 0; i < n; i++) {
		if (keys[i].flags & KEY_SELECTED) {
			BKey key(keys[i].time);
			key.flags |= KEY_SELECTED;
			keys.Append(1,&key,5);
			keys[i].flags &= ~KEY_SELECTED;
			changed = TRUE;
			}
		}
	if (changed) {
		keys.Shrink();
		valid.SetEmpty();
		SortKeys();
		}
	}

void BoolControl::AddNewKey(TimeValue t,DWORD flags)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	BKey key(t);
	if (flags&ADDKEY_SELECT) {
		key.flags |= KEY_SELECTED;
		}
	if (flags&ADDKEY_FLAGGED) {
		int n = keys.Count();
		for (int i = 0; i < n; i++) {
			keys[i].flags &= ~KEY_FLAGGED;
			}		
		key.flags |= KEY_FLAGGED;
		}
#ifdef BOOLCTRL_NO_FLIPPING
	if (!(flags&ADDKEY_NOSLIDE)) {
		// Slide key from the right (if present) instead of adding a new key.
		int n = keys.Count();
		for (int i = 0; i < n; i++) {
			if (keys[i].time == t) {
				if (flags&ADDKEY_FLAGGED) {
					keys[i].flags |= KEY_FLAGGED;
					}
				if (flags&ADDKEY_SELECT) {
					keys[i].flags |= KEY_SELECTED;
					}
				// Don't add a key on top of another key
				break;
				}

			if (keys[i].time > t) {
				keys[i].time = t;
				if (flags&ADDKEY_FLAGGED) {
					keys[i].flags |= KEY_FLAGGED;
					}
				if (flags&ADDKEY_SELECT) {
					keys[i].flags |= KEY_SELECTED;
					}
				break;
				}
			}
		if (i==n) keys.Append(1,&key);
	} else {
		keys.Append(1,&key);
		}
#else
	keys.Append(1,&key);
#endif

	valid.SetEmpty();
	SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);	
	}

BOOL BoolControl::IsKeySelected(int index)
	{
	return keys[index].flags & KEY_SELECTED;
	}

TrackClipObject *BoolControl::CopyTrack(Interval iv, DWORD flags)
	{	
	BoolClipObject *cobj = new BoolClipObject(iv);	
	Interval test = TestInterval(iv,flags);	
	for (int i = 0; i < keys.Count(); i++) {
		if (test.InInterval(keys[i].time)) {
			BKey nk(keys[i].time);
			cobj->tab.Append(1,&nk,10);
			}
		}
	cobj->tab.Shrink();
	return cobj;
	}

void BoolControl::PasteTrack(TrackClipObject *cobj,Interval iv, DWORD flags)
	{
	BoolClipObject *cob = (BoolClipObject*)cobj;	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();		
	DeleteTime(iv,flags);	
	InsertTime(iv.Start(),cob->clip.Duration()-1);	
	for (int i = 0; i < cob->tab.Count(); i++) {
		BKey key(cob->tab[i].time);
		key.time -= cob->clip.Start() - iv.Start();
		keys.Append(1,&key);		
		}	
	valid.SetEmpty();
	SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

int BoolControl::HitTestTrack(			
		TrackHitTab& hits,
		Rect& rcHit,
		Rect& rcTrack,			
		float zoom,
		int scroll,
		DWORD flags)
	{
	int left  = ScreenToTime(rcTrack.left,zoom,scroll) - 4;
	int right = ScreenToTime(rcTrack.right,zoom,scroll) + 4;
	int n = keys.Count();
	int y = (rcTrack.top+rcTrack.bottom)/2;	
		
	for ( int i = 0; i < n; i++ ) {
		if (flags&HITTRACK_SELONLY && 
			!(keys[i].flags & KEY_SELECTED)) continue;
		if (flags&HITTRACK_UNSELONLY && 
			(keys[i].flags & KEY_SELECTED)) continue;

		if (keys[i].time > right) {
			break;
			}
		if (keys[i].time > left) {
			int x = TimeToScreen(keys[i].time,zoom,scroll);
			if (rcHit.Contains(IPoint2(x,y))) {
				TrackHitRecord rec(i,0);
				hits.Append(1,&rec);
				if (flags&HITTRACK_ABORTONHIT) return TRACK_DONE;
				}
			}		
		}
	return TRACK_DONE;
	}

int BoolControl::PaintTrack(			
		ParamDimensionBase *dim,
		HDC hdc,
		Rect& rcTrack,
		Rect& rcPaint,
		float zoom,
		int scroll,
		DWORD flags)
	{
	int left  = ScreenToTime(rcPaint.left-8,zoom,scroll);
	int right = ScreenToTime(rcPaint.right+8,zoom,scroll);
	int n = keys.Count();
	int y = (rcTrack.top+rcTrack.bottom)/2, lx, x = rcPaint.left-2;
	HBRUSH selBrush   = CreateSolidBrush(RGB(255,255,255));
	HBRUSH barBrush   = CreateSolidBrush(RGB(113,160,231)); //RGB(80,100,180)
	HBRUSH unselBrush = (HBRUSH)GetStockObject(GRAY_BRUSH);	
	BOOL state = startOnOff;
	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc,barBrush);
	
	for (int i = 0; i < n; i++) {		
		lx = x;
		x  = TimeToScreen(keys[i].time,zoom,scroll);
		if (keys[i].time > left) {			
			if (state) {							
				Rectangle(hdc,lx,y-4,x,y+4);
				}
			}		
		state = !state;
		if (keys[i].time > right) {
			break;
			}
		}
	if (state) {		
		Rectangle(hdc,x,y-4,rcPaint.right,y+4);
		}
	
	for (i = 0; i < n; i++) {
		if (keys[i].time > right) {
			break;
			}
		if (keys[i].time > left) {			
			x  = TimeToScreen(keys[i].time,zoom,scroll);
			if ((flags&PAINTTRACK_SHOWSEL) && (keys[i].flags&KEY_SELECTED)) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}
			Ellipse(hdc,x-4,y-5,x+4,y+5);
			}		
		}

	DeleteObject(selBrush);
	DeleteObject(barBrush);
	return TRACK_DONE;
	}


void BoolControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	Interval wvalid = FOREVER;	
	float *v = (float*)val;
			
	t = ApplyEase(t,wvalid);	
	
 	// We may re-direct to the setkey buffer
	BOOL sOnOff = startOnOff;
	Tab<BKey> *keyTab = &keys;
	if (GetSetKeyMode() && setkeyBuffer) {
		keyTab = setkeyBuffer;
		sOnOff = setkeyBuffer->startOnOff;
		}

	if (!this->valid.InInterval(t)) {
		onOff = sOnOff;
		for (int i=0; i<keyTab->Count(); i++) {
			if ((*keyTab)[i].time>t) break;
			onOff = !onOff;
			}
		if (keyTab->Count()) {
			if (t < (*keyTab)[0].time) {
				this->valid.Set(TIME_NegInfinity,(*keyTab)[0].time-1);
			} else 
			if (t > (*keyTab)[keyTab->Count()-1].time) {
				this->valid.Set((*keyTab)[keyTab->Count()-1].time,TIME_PosInfinity);
			} else this->valid.SetInstant(t);
		} else {
			this->valid = FOREVER;
			}
		}
	valid &= this->valid;
	*v = onOff ? 1.0f : -1.0f;
	}


int SetKeyBuffer::GetKeyIndex(TimeValue t)
	{
	for (int i=0; i<Count(); i++) {
		if ((*this)[i].time==t) return i;
		if ((*this)[i].time>t) return -1;
		}
	return -1;
	}

void BoolControl::SetValueLocalTime(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{
	float *v = (float*)val;
	float tv;

	GetValue(t,&tv,FOREVER);
	if (MAKEBOOL(tv) == MAKEBOOL(*v)) {
		return;
		}
	
	if (GetSetKeyMode()) {
		// Set key mode is on

		// Save state for undo
		HoldSetKeyBuffer();

		// If this controller hasn't been modified since set-key mode was turned on,
		// we need to make one now.
		if (!setkeyBuffer) {
			// Make a copy of the existing key table. We'll also preserve the old
			// 'onOff' in case we didn't previously have any keys
			setkeyBuffer = new SetKeyBuffer(onOff, startOnOff, keys);
			}			

		int index = setkeyBuffer->GetKeyIndex(t);
		if (index>=0) {
			setkeyBuffer->Delete(index,1);
			// Delete a second key to avoid flipping
			if (setkeyBuffer->Count() > index) setkeyBuffer->Delete(index,1);
			setkeyBuffer->Shrink();
		} else {

			// Slide key from right (if present) instead of adding a new key
			int n = setkeyBuffer->Count();
			for (int i = 0; i < n; i++) {
				if ((*setkeyBuffer)[i].time > t) {
					(*setkeyBuffer)[i].time = t;
					break;
					}
				}
			if (i==n) {
				BKey k(t);
				setkeyBuffer->Append(1,&k);
				setkeyBuffer->Sort((CompareFnc)CompareBKeys);
				}
			}

	} else {
		// Non-set-key mode

		HoldTrack();
		if (Animating()) {		
			int index = GetKeyIndex(t);
			if (index>=0) {
				keys.Delete(index,1);
	#ifdef BOOLCTRL_NO_FLIPPING
				// Delete a second key to avoid flipping
				if (keys.Count() > index) keys.Delete(index,1);
	#endif
				keys.Shrink();
			} else {

	#ifdef BOOLCTRL_NO_FLIPPING
				// Slide key from right (if present) instead of adding a new key
				int n = keys.Count();
				for (int i = 0; i < n; i++) {
					if (keys[i].time > t) {
						keys[i].time = t;
						break;
						}
					}
				if (i==n) {
					BKey k(t);
					keys.Append(1,&k);
					SortKeys();
					}
	#else
				BKey k(t);
				keys.Append(1,&k);
				SortKeys();
	#endif
				}
		} else {
			if (keys.Count()) {			
				startOnOff = !startOnOff;
			} else {
				onOff = startOnOff = MAKEBOOL(*v);			
				}
			}
		}
	
	valid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolControl::Extrapolate(
		Interval range,TimeValue t,void *val,Interval &valid,int type)
	{	
	GetValue(t,val,valid);	
	}

void BoolControl::SortKeys()
	{
	keys.Sort((CompareFnc)CompareBKeys);
	}

