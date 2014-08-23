/**********************************************************************
 *<

  	FILE: boolcntrl.cpp

	DESCRIPTION: This new Boolean Controller plug-in is a true ON/OFF 
		controller with values 1 and 0. The controller is mainly expected 
		to work with IK Enabled state although it can be assigned to any 
		animatable binary control state.

		This is a float controller.

		Note 2. The behavior of this Boolean Controller is different form 
		the default On/OFF controller (the old Boolean COntroller) in the 
		following ways: 
			- It is NOT a flip-flop controller but a true Boolean (although 
				internally it's a float controller).
			- You can put two or more consecutive ON-ON or OFF-OFF keys.
			- Adding a key doesn't change the past keyframes, only the 
				future (upto the next keyframe)
			- You can edit the function curve; the keyframes are visible and movable.
			- No disappearence of keyframes (this was the biggest complain)!

	CREATED BY: Ambarish Goswami Sept-Oct/2001
	            

	HISTORY: 

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "boolcntrl.h"
#include "istdplug.h"
#include "setkeymode.h"

#define BOOL_CONTROL_CNAME		GetString(IDS_AG_BOOL)

#define FCURVE_STEP 5

#define MAKEBOOL(fl) ( (fl)<=0.5f ? FALSE : TRUE )


//enum {	bool_on_off, };


#define DrawKeyKnot(hdc,x,y) Rectangle(hdc,x-3,y-3,x+3,y+3)


class BKey : public IBoolFloatKey {
	public:
//		float val;
//		TimeValue time;
//		DWORD flags;

		BKey() {flags=0;}
		BKey(TimeValue t) {time=t; flags=0;}

		void Setflags(DWORD mask) { flags|=(mask); }
		void Clearflags(DWORD mask) { flags &= ~(mask); }
		BOOL Testflags(DWORD mask);

		
		BOOL ElemSelected(int el) {return Testflags(el <= 2 ? (KEY_XSEL<<el) : KEY_WSEL);}
		BOOL AnyElemSelected() {return Testflags(KEY_XSEL|KEY_YSEL|KEY_ZSEL|KEY_WSEL);}
		void SelectElem(int el);
		void DeselectElem(int el);
		void SelectElemBits(DWORD el);
		void DeselectElemBits(DWORD el);
		void DeselectAllElems() {Clearflags(KEY_ALLSEL);}
		
		BOOL TimeLocked() {return Testflags(KEY_TIME_LOCK);}
		void LockTime() {Setflags(KEY_TIME_LOCK);}
		void UnlockTime() {Clearflags(KEY_TIME_LOCK);}
//		BOOL ValLocked(int j) {return Testflags(1<<(KEY_VALLOCK_SHIFT+j));}
//		void LockVal(int j) {Setflags(1<<(KEY_VALLOCK_SHIFT+j));}
//		void UnlockVal(int j) {Clearflags(1<<(KEY_VALLOCK_SHIFT+j));}
		float& operator[](int el); 
	};

BOOL BKey::Testflags(DWORD mask) { 
	BOOL whatis;
	whatis = flags&(mask)?1:0;
	return whatis; 
}

inline float &OperatorIndex(float &val,int i)
	{return val;}

inline float& BKey::operator[](int el)
	{return OperatorIndex(val,el);}

inline void BKey::SelectElem(int el) 
	{
	Setflags(el <= 2 ? (KEY_XSEL<<el) : KEY_WSEL);
	Setflags(KEY_SELECTED);
	}
inline void BKey::DeselectElem(int el) 
	{
	Clearflags(el <= 2 ? (KEY_XSEL<<el) : KEY_WSEL);
	if (!AnyElemSelected()) {
		Clearflags(KEY_SELECTED);
		}
	}

inline void BKey::SelectElemBits(DWORD el) 
	{
	Setflags(el);
	Setflags(KEY_SELECTED);
	}
inline void BKey::DeselectElemBits(DWORD el) 
	{
	Clearflags(el);
	if (!AnyElemSelected()) {
		Clearflags(KEY_SELECTED);
		}
	}


static int __cdecl CompareBKeys(const BKey *k1, const BKey *k2)
	{
	if (k1->time < k2->time) return -1;
	if (k1->time > k2->time) return 1;
	return 0;
	}

class KeyCallbackParams {
	public:
		int index,ktime,x0,y0;
		BOOL sel,onTop;
		float dval,kval;
		KeyCallbackParams() {}
		KeyCallbackParams(int index,BOOL sel,float dval,float kval,int ktime,int x0,int y0)
			{this->index=index;this->sel=sel;this->dval=dval;this->kval=kval;this->ktime=ktime;this->x0=x0;this->y0=y0;onTop=FALSE;}
	};

class SegCallbackParams {
	public:
		int x0,y0,x1,y1;
		BOOL inRange;
		SegCallbackParams() {}
		SegCallbackParams(int x0,int y0,int x1, int y1,BOOL inRange)
			{this->x0=x0;this->y0=y0;this->x1=x1;this->y1=y1;this->inRange=inRange;}
	};

class FCurveCallback {
	public:
		virtual BOOL DoSeg(int el,int x0,int y0,int x1, int y1,BOOL inRange)=0;
		virtual BOOL DoKey(int el,int index,BOOL sel,float dval,float kval,int ktime,int x0,int y0,BOOL onTop=FALSE)=0;
	};



class BoolCntrlKeyTable : public Tab<BKey> {		
	public:		
		DWORD tflags;	// track flags		
		Interval range;

		// CAL-10/22/2002: need to have a constructor and initialize data members.
		BoolCntrlKeyTable() { tflags=0; range.SetEmpty(); }
		void InterpValue(TimeValue t, float &val, Interval &valid);
		int PaintFCurves(
			Control *cont,
			ParamDimensionBase *dim,
			HDC hdc,
			Rect& rcGraph,
			Rect& rcPaint,
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags );
		int AddKey(Control *cont,TimeValue t, BKey& key);
		void ClearTFlag(DWORD mask) { tflags &= ~(mask); }
		BOOL TestTFlag(DWORD mask) { return(tflags&(mask)?1:0); }
		void SetTFlag(DWORD mask) { tflags|=(mask); }
		void DeleteKey(int index);
		void KeysChanged(BOOL timeChanged=TRUE);
		void MouseCycleCompleted(TimeValue t);
		void CheckForDups();
//		void FixupDisplayFlags(DWORD &flags);
		int FindKey(TimeValue t, int start=-1);
		int HitTestFCurves(
			Control *cont,
			ParamDimensionBase *dim,
			TrackHitTab& hits,
			Rect& rcHit,
			Rect& rcGraph,			
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags );
		int ProcessFCurves(
			BOOL hitTesting,
			FCurveCallback *callback,
			Control *cont,
			ParamDimensionBase *dim,
			Rect& rcGraph,
			float tzoom,
			int tscroll,
			float vzoom,
			int vscroll,
			DWORD flags );

		BOOL TrackNotKeyable(void) { return TestTFlag(TFLAG_NOTKEYABLE); }
};


// CAL-11/27/2002: new function to interpolate the keys
void BoolCntrlKeyTable::InterpValue(TimeValue t, float &val, Interval &valid)
{
	if (!Count()){		// no keys are present yet
		valid = FOREVER;
		return;
	}
	else if (Count() == 1){
		if (t >= (*this)[0].time) {
			val = (*this)[0].val;
			valid.SetInstant(t);
			return; 
		}
		else { //(t < keys[0].time)
			// CAL-10/22/2002: better to use the key 0 value instead of random value at curval
			//		and to be consistent with the situation when there're more than one keys.
			val = (*this)[0].val;
			valid.SetInstant(t);
			return;
		}
	}
	else { // keys.Count() > 1
		if (t >= (*this)[Count()-1].time){
			val = (*this)[Count()-1].val;
			valid.SetInstant(t);
			return; 
		}
		else if (t < (*this)[0].time){
			// CAL-10/22/2002: should keep the value in curval as well.
			val = (*this)[0].val;
			valid.SetInstant(t);
			return; 
		}
		for (int i = 1; i < Count(); i++) {
			if (t == (*this)[i].time) {
				val = (*this)[i].val;
				valid.SetInstant(t);
				return; 
			}
			else if (t < (*this)[i].time){
				val = (*this)[i-1].val;
				valid.SetInstant(t);
				return; 
			}
		}
	}
}


void BoolCntrlKeyTable::DeleteKey(int index)
	{	
	Delete(index,1);
	}


void BoolCntrlKeyTable::CheckForDups()
	{
	// Check for keys that landed on top of each other
	for (int i=0; i<Count()-1; i++) {
		if ((*this)[i].time==(*this)[i+1].time) {
			if ((*this)[i].AnyElemSelected()) {
				DeleteKey(i+1);
			} else {
				DeleteKey(i);				
				}
			i--;
			}
		}
	}


void BoolCntrlKeyTable::MouseCycleCompleted(TimeValue t)
	{
	CheckForDups();
	}

class StdHitTestFCurveCallback : public FCurveCallback {
	public:
		Control *cont;
		ParamDimensionBase *dim;
		TrackHitTab *hits;
		Rect rcGraph;
		Rect rcHit;
		float tzoom;
		int tscroll;
		float vzoom;
		int vscroll;
		DWORD flags;	
		DWORD tflags;
		DWORD hitRes;

		StdHitTestFCurveCallback(Control *cont,ParamDimensionBase *dim,TrackHitTab& hits,Rect& rcHit,
			Rect& rcGraph,float tzoom,int tscroll,float vzoom,int vscroll,DWORD flags, DWORD tflags);
		
		BOOL DoSeg(int el,int x0,int y0,int x1, int y1,BOOL inRange);
		BOOL DoKey(int el,int index,BOOL sel,float dval,float kval,int ktime,int x0,int y0,BOOL onTop=FALSE);
	};

StdHitTestFCurveCallback::StdHitTestFCurveCallback(Control *cont,ParamDimensionBase *dim,TrackHitTab& hits,Rect& rcHit,
			Rect& rcGraph,float tzoom,int tscroll,float vzoom,int vscroll,DWORD flags, DWORD tflags)
	{
	this->cont=cont;this->dim=dim;this->hits=&hits;this->rcGraph=rcGraph;
	this->rcHit=rcHit;this->tzoom=tzoom;this->tscroll=tscroll;this->tflags=tflags;
	this->vzoom=vzoom;this->vscroll=vscroll;this->flags=flags;
	hitRes = HITCURVE_NONE;
	}

static BOOL LineIntersectsBox(int x0, int y0, int x1, int y1, Rect rect)
	{
	if (x0>rect.right||x1<rect.left) return FALSE;
	if ((y0<rect.top&&y1<rect.top)||(y0>rect.bottom&&y1>rect.bottom)) return FALSE;
	
	int dx = x1-x0, dy = y1-y0;
	if (dx) {
	 	float slope = float(dy)/float(dx);
		int y = y0+int((rect.left-x0)*slope);
		if (y>=rect.top&&y<=rect.bottom) return TRUE;
		y = y0+int((rect.right-x0)*slope);
		if (y>=rect.top&&y<=rect.bottom) return TRUE;
		}
	if (dy) {
		float slope = float(dx)/float(dy);
		int x = x0+int((rect.top-y0)*slope);
		if (x>=rect.left&&x<=rect.right) return TRUE;
		x = x0+int((rect.bottom-y0)*slope);
		if (x>=rect.left&&x<=rect.right) return TRUE;
		}
	return FALSE;
	}



BOOL StdHitTestFCurveCallback::DoSeg(int el,int x0,int y0,int x1, int y1,BOOL inRange)
	{
	if (hitRes != HITCURVE_NONE) return TRUE;
	if (LineIntersectsBox(x0,y0,x1,y1,rcHit)) {		
		hitRes = HITCURVE_WHOLE;
		if (flags&HITTRACK_ABORTONHIT) return FALSE;
		}
	return TRUE;
	}

BOOL StdHitTestFCurveCallback::DoKey(
		int el,int index,BOOL sel,float dval,float kval,int ktime,int x0,int y0,BOOL onTop)
	{
	if (flags&HITTRACK_SELONLY && !sel) {
		return TRUE;
		}
	if (flags&HITTRACK_UNSELONLY && sel) {
		return TRUE;
		}
	if (rcHit.Contains(IPoint2(x0,y0))) {
		TrackHitRecord rec(index,(el <= 2 ? (KEY_XSEL<<el) : KEY_WSEL));
		hits->Append(1,&rec);
		hitRes = HITCURVE_KEY;
		if (flags&HITTRACK_ABORTONHIT) return FALSE;
		}
	return TRUE;
	}


int BoolCntrlKeyTable::HitTestFCurves(
		Control *cont,ParamDimensionBase *dim,TrackHitTab& hits,Rect& rcHit,
		Rect& rcGraph,float tzoom,int tscroll,float vzoom,int vscroll,DWORD flags )
	{	
	if ((flags&HITTRACK_SELONLY && !(tflags&CURVE_SELECTED))) {
		return HITCURVE_NONE;
		}				
		StdHitTestFCurveCallback cb(cont,dim,hits,rcHit,rcGraph,tzoom,tscroll,vzoom,vscroll,flags,tflags);	
		ProcessFCurves(TRUE,&cb,cont,dim,rcGraph,tzoom,tscroll,vzoom,vscroll,flags );
		if (!(tflags&CURVE_SELECTED) && cb.hitRes==HITCURVE_KEY) {
			cb.hitRes = HITCURVE_WHOLE;
			}
		if ((flags&HITTRACK_SELONLY) && cb.hitRes==HITCURVE_WHOLE) {
			cb.hitRes = HITCURVE_NONE;
			}
		return cb.hitRes;
	}

int BoolCntrlKeyTable::AddKey(Control *cont,TimeValue t, BKey &key)
	{
	int n = Count();
	int i = FindKey(t);
	int res;
	
	if (i>=0 && t==(*this)[i].time) {		
		(*this)[i].val = key.val;		
		res = i;
	} else {	
		res = Insert(i+1,1,&key);
		}
	if (n<2) {
		cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_BECOMING_ANIMATED);
		}
	return res;
	}


int BoolCntrlKeyTable::FindKey(TimeValue t, int start)
	{
	int n = Count();
	if (n==0) return(-1);
	if (start < 0)  start = 0;
	if (start >= n) start = n-1;
	if ((*this)[start].time < t) {
		for (int i=start; i<n; i++) {
			if ((*this)[i].time>t) {
				if (i==0) return(-1);
				else return(i-1);					
				} 		
			}
		return(n-1);
	} else {
		for (int i=start; i>=0; i--) {
			if ((*this)[i].time<=t) {
				return i;				
				} 		
			}	
		return -1;
		}	
	}


int BoolCntrlKeyTable::ProcessFCurves(
		BOOL hitTesting,
		FCurveCallback *callback,
		Control *cont,
		ParamDimensionBase *dim,
		Rect& rcGraph,
		float tzoom,
		int tscroll,
		float vzoom,
		int vscroll,
		DWORD flags )
	{	
//	FixupDisplayFlags(flags);
	int h = rcGraph.h()-1;
	int n = Count();
	int xo,x,y[1], yo[1], i, k;
	Interval valid;
	TimeValue t, dt=1, pwt, wt, kx;
	float val=0.0;
	float fy, m;
	int pkey=-1, nkey;
	BOOL init = FALSE, inRange;
	// CAL-07/01/02: change the testing to be the same as the one in KeyTable<KEYTABLE_PARAMS>::ProcessFCurves() function in core/ctrltemp.h
	BOOL drawSel  = ((flags&PAINTCURVE_FROZEN)&&!(flags&PAINTCURVE_FROZENKEYS))?FALSE:TRUE;	// ((tflags&CURVE_SELECTED)&&(flags&PAINTTRACK_SHOWSEL)) || hitTesting;
	Tab<KeyCallbackParams> keyParams[1];
	Tab<SegCallbackParams> segParams[1];
//	BOOL cvHSV = IsColorHSV(*this);

		segParams[0].Resize( (rcGraph.right-rcGraph.left)/FCURVE_STEP+1 );
		keyParams[0].Resize(n);
	
	// RB 3/29/99: Added -FCURVE_STEP to rcGraph.left below to fix 166837 (note divide by 0 a couple of lines down "/float(wt-pwt)").
	pwt = cont->ApplyEase(ScreenToTime(rcGraph.left-FCURVE_STEP,tzoom,tscroll),valid);

	// RB 3/29/99: Add 400 pixel slop so tangent handles still draw for keys off the right of the track view window.
	// Note that a better solution might be to detect the case where no ease curves are applied and
	// revert to a more straight forward drawing technique.
	for (x = rcGraph.left; x<rcGraph.right+400; x += FCURVE_STEP ) {
		t       = ScreenToTime(x,tzoom,tscroll);				
		inRange = range.InInterval(cont->ApplyEase(t,valid));		

		// Do warped keys
		if (drawSel) {
			wt      = cont->ApplyEase(t,valid);
			nkey = FindKey(wt,pkey);
			if (pkey!=nkey) {

				int dkey = nkey < pkey ? pkey : nkey;
				if (dkey >= 0) {
					// RB 5/12/99: Loop through keys in case we're stepping past more than one.					
					while (1) { 
						kx = x - FCURVE_STEP +
							int((float((*this)[dkey].time-pwt)/float(wt-pwt))*FCURVE_STEP); 
						cont->GetValue(t+dt,&val,valid);					
//						if (cvHSV) ConvertHSV(&val);
						m = cont->GetMultVal((*this)[dkey].time,valid);		

//						for (j=0;j<1;j++) {
							if (!(flags&(DISPLAY_XCURVE<<0))) continue;
							fy = dim->Convert((*this)[dkey][0]);
							fy *= m; // Multiplier curve
							y[0] = ValueToScreen(fy,rcGraph.h()-1,vzoom,vscroll);				
							
							KeyCallbackParams kcp(dkey,(*this)[dkey].ElemSelected(0),
									OperatorIndex(val,0),(*this)[dkey][0],
									(*this)[dkey].time,kx,y[0]);
							keyParams[0].Append(1,&kcp,10);
							if (x != rcGraph.left) {
								SegCallbackParams scb(xo,yo[0],kx,y[0],inRange);
								segParams[0].Append(1,&scb,10);	
								yo[0] = y[0];
								}
//							}										
						xo  = kx;

						// RB 5/12/99: If the difference between dkey and pkey is larger than 1 then we're about
						// to step past 1 or more keys. Loop through to pkey. However if we're not hit testing
						// then there's no reason to draw all the keys that are on top of eachother.
						if (!hitTesting) break;
						if (dkey<=0 || dkey==pkey) break;
						if (pkey < dkey) dkey--;
						else dkey++;
						if (dkey==pkey) break;
						}
					}
				pkey = nkey;
				}
			}				
		
		// Draw curve section
		cont->GetValue(t,&val,valid);
//		if (cvHSV) ConvertHSV(&val);
//		for (j=0;j<1;j++) {
			if (!(flags&(DISPLAY_XCURVE<<0))) {
				continue;
				}
			fy = dim->Convert(OperatorIndex(val,0));
			y[0] = ValueToScreen(fy,h,vzoom,vscroll);
			
			if (x != rcGraph.left) {
				SegCallbackParams scb(xo,yo[0],x,y[0],inRange);
				segParams[0].Append(1,&scb,10);				
				} 

			yo[0] = y[0];
//			}

		xo  = x;		
		pwt = wt;
		}
			
	if (hitTesting) {
		
		// RB 3/29/99: Hit test all keys and tangents before curve segments. This way tangents and keys have priority over curves.
//		for (j=0;j<1;j++) {
			for (i=0;i<keyParams[0].Count();i++) {
				if ( !callback->DoKey(0,keyParams[0][i].index,keyParams[0][i].sel,
					keyParams[0][i].dval,keyParams[0][i].kval,keyParams[0][i].ktime,
					keyParams[0][i].x0,keyParams[0][i].y0) ) {
					return 0;
				}
			}
//		}
//		for (j=0;j<1;j++) {
			for (i=0;i<segParams[0].Count();i++) {
				if (!callback->DoSeg(0,segParams[0][i].x0,segParams[0][i].y0,
					segParams[0][i].x1,segParams[0][i].y1,segParams[0][i].inRange) ) {
					return 0;
				}
			}
//		}

	} 
	else {
		// First mark keys that are on top of each other
		for (i=0;i<keyParams[0].Count();i++) {
//			for (j=0;j<1;j++) {
				for (k=0;k<1;k++) {
					if (0==k) continue;
					if (i>=keyParams[0].Count()) continue;
					if (i>=keyParams[k].Count()) continue;

					if (abs(keyParams[0][i].y0-keyParams[k][i].y0)<3) {
						keyParams[0][i].onTop = TRUE;
						keyParams[k][i].onTop = TRUE;
					}
				}
//			}
		}

//		for (j=0;j<1;j++) {
			for (i=0;i<segParams[0].Count();i++) {
				if (!callback->DoSeg(0,segParams[0][i].x0,segParams[0][i].y0,
					segParams[0][i].x1,segParams[0][i].y1,segParams[0][i].inRange) ) {
					return 0;
				}
			}
			for (i=0;i<keyParams[0].Count();i++) {
				if ( !callback->DoKey(0,keyParams[0][i].index,keyParams[0][i].sel,
					keyParams[0][i].dval,keyParams[0][i].kval,keyParams[0][i].ktime,
					keyParams[0][i].x0,keyParams[0][i].y0,keyParams[0][i].onTop) ) {
					return 0;
				}
			}
			
//		}
	}
	return 0;
	}



#define A_USE_SETVAL_BUFFER_RANGE	A_PLUGIN2

class SetKeyBuffer : public BoolCntrlKeyTable {
	public:
		float oldVal;		
		SetKeyBuffer() {}
		SetKeyBuffer(float v) : BoolCntrlKeyTable() {oldVal = v;}
		SetKeyBuffer(float v, BoolCntrlKeyTable &table) : BoolCntrlKeyTable(table) {
			oldVal = v;			
			}
		int GetKeyIndex(TimeValue t);
	};


class BoolCntrl : public IBoolCntrl, public ISetKey  {
	public:	
		float curval;//, startVal;
		BoolCntrlKeyTable keys;
		Interval ivalid;		
		SetKeyBuffer *setkeyBuffer;
		static BoolCntrl *editCont;
		static HWND hWnd;
//		IParamBlock2* pblock;
		SetKeyBuffer* &GetSetKeyBuffer() {return setkeyBuffer;}
		int mSetKeyFlags;

		BoolCntrl(BOOL loading=FALSE);
		~BoolCntrl();
		float &GetCurVal() {return curval;}
		void Update(TimeValue t);

		int PaintFCurves(ParamDimensionBase *dim,HDC hdc,Rect& rcGraph,Rect& rcPaint,
			float tzoom,int tscroll,float vzoom,int vscroll,DWORD flags );
		BOOL IsCurveSelected();
		
		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 1;}
		BOOL IsAnimated() {return keys.Count()?TRUE:FALSE;}
		Class_ID ClassID() {return BOOLCNTRL_CLASS_ID;} 
		SClass_ID SuperClassID() {return CTRL_FLOAT_CLASS_ID;}
		void GetClassName(TSTR& s) {s = BOOL_CONTROL_CNAME;}	
		void EditTrackParams(TimeValue t,ParamDimensionBase *dim,TCHAR *pname,HWND hParent,IObjParam *ip,DWORD flags);
		// CAL-09/11/02: add this definition so that R-Click key in Dope Sheet will invoke EditTrackParams() (433260)
		int TrackParamsType() {return TRACKPARAMS_KEY;}
		void* GetInterface(ULONG id);

//		int NumSubs()  {return 1;} //because it uses the paramblock
//		Animatable* SubAnim(int i) {return pblock;}
//		TSTR SubAnimName(int i) { return GetString(IDS_AG_BOOL_PARAMS); }
//		int SubNumToRefNum(int subNum) {if (subNum==0) return BOOL_PBLOCK_REF; else return -1;}
//		int	NumParamBlocks() { return 1; }
//		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
//		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock


		int SetProperty(ULONG id, void *data);
		void *GetProperty(ULONG id);
		void SetValidEmpty() {ivalid.SetEmpty();}

		// Set-key mode support
		void CommitSetKeyBuffer(TimeValue t);
		void RevertSetKeyBuffer();
		void RevertSetKeyBuffer(BOOL clearHeldFlag);
		BOOL SetKeyBufferPresent() {return setkeyBuffer? TRUE: FALSE;}
		void HoldSetKeyBuffer();

		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);

		// Reference methods
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		RefTargetHandle Clone(RemapDir &remap = NoRemap());	
//		RefTargetHandle GetReference(int i);
//		void SetReference(int i, RefTargetHandle rtarg);
		int HitTestFCurves(	ParamDimensionBase *dim,TrackHitTab& hits,Rect& rcHit,
			Rect& rcGraph,float tzoom,int tscroll,float vzoom,int vscroll,DWORD flags );

		// Control methods				
		void Copy(Control *from);
		BOOL IsLeaf() {return TRUE;}
		int AddKeyFromCurValue(TimeValue t);
//		void OffsetKeys(float delta); 
		void CommitValue(TimeValue t);
		void RestoreValue(TimeValue t) {}
		void SetORT(int ort,int type);
		// void EnableORTs(BOOL enable);	// CAL-11/11/2002: use the inherited function from Control
		
		void HoldTrack();
		Interval GetTimeRange(DWORD flags);
		void EditTimeRange(Interval range,DWORD flags);
		void MoveKeys(ParamDimensionBase *dim,float delta,DWORD flags);
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
		
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);

		// StdControl methods
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method=CTRL_ABSOLUTE);
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);		
		void *CreateTempValue() {return new float;}
		void DeleteTempValue(void *val) {delete (float*)val;}
		void ApplyValue(void *val, void *delta) {*((float*)val) += *((float*)delta);}
		void MultiplyValue(void *val, float m) {*((float*)val) *= m;}

//		float Get_on_off_mode();
//		void Set_on_off_mode(float onOff);

		// From IKeyControl
		int GetNumKeys() {return keys.Count();}
		void SetNumKeys(int n);
		void GetKey(int i,IKey *key);
		void SetKey(int i,IKey *key);
		int AppendKey(IKey *key);
		void SortKeys();
		DWORD &GetTrackFlags() {return keys.tflags;}
		
		//from IBoolSetKey
		void SetKeyWithoutBuffer(const TimeValue &t) { AddKeyFromCurValue(t); }
		int &SetKeyFlags(void) { return mSetKeyFlags; }
		void ResetKeyTimes(const TimeValue &src, const TimeValue &dst){};
		void SetCurrentValue(void *v) { if(!setkeyBuffer) curval = *((float*)v); }
		
	};


// CAL-11/27/2002: new function to be used by SetValueLocalTime to update the current value
void BoolCntrl::Update(TimeValue t)
	{
	if (!ivalid.InInterval(t)) {
		if (GetSetKeyMode() && setkeyBuffer) {
			setkeyBuffer->InterpValue(t, curval, ivalid);
		} else {
			keys.InterpValue(t, curval, ivalid);
			}
		}
	}


void BoolCntrl::SetORT(int ort,int type)
	{	
	if (type==ORT_BEFORE) {
		if (ort==ORT_LOOP) {
			keys.SetTFlag(TRACK_LOOPEDIN);
		} else {
			keys.ClearTFlag(TRACK_LOOPEDIN);
			}
	} else {
		if (ort==ORT_LOOP) {
			keys.SetTFlag(TRACK_LOOPEDOUT);
		} else {
			keys.ClearTFlag(TRACK_LOOPEDOUT);
			}
		}
	keys.KeysChanged(FALSE);
	Control::SetORT(ort,type);
	}


void BoolCntrl::EditTrackParams(
		TimeValue t,ParamDimensionBase *dim,TCHAR *pname,HWND hParent,IObjParam *ip,DWORD flags)
	{
	if (!NumSelKeys()) return;	

	for (int i = 0; i < keys.Count(); i++) {
		if (keys[i].ElemSelected(0)){ 
			if (keys[i].val == 1.0) 
				keys[i].val = 0.0;
			else if (keys[i].val == 0.0) 
				keys[i].val = 1.0;
		}
	}
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

	


void BoolCntrl::CopyKeysFromTime(
		TimeValue src,TimeValue dst,DWORD flags)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	int index = GetKeyIndex(src);	

	if (index>=0) {
		BKey key = keys[index];
		key.time = dst;
		keys.AddKey(this,dst,key);
	} 
	else {
		Interval iv;
		BKey k;		
		k.time = dst;
		GetValue(src,&k.val,iv);		
		keys.AddKey(this,dst,k);
		}
	
	keys.KeysChanged(FALSE);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}


BOOL BoolCntrl::IsCurveSelected() 
		{
			return keys.TestTFlag(CURVE_SELECTED);
		}

int BoolCntrl::HitTestFCurves(	
		ParamDimensionBase *dim,
		TrackHitTab& hits,
		Rect& rcHit,
		Rect& rcGraph,
		float tzoom,
		int tscroll,
		float vzoom,
		int vscroll,
		DWORD flags )
	{	
	int res;

	// Redirect GetValue() to use original key table
	SetKeyBuffer *savePtr = setkeyBuffer; 
	setkeyBuffer=NULL; 
	ivalid.SetEmpty();

	res = keys.HitTestFCurves(this,dim,hits,rcHit,rcGraph,tzoom,tscroll,vzoom,vscroll,flags);

	// Redirect back to setvalBuffer
	setkeyBuffer = savePtr;

	return res;
	}


inline int BoolCntrl::AddKeyFromCurValue(TimeValue t) 
	{
	if (!ivalid.InInterval(t)) {
		ClearAFlag(A_SET);
		keys.InterpValue(t,curval,ivalid);
		}
	BKey k;
	k.time = t;
	k.val = (curval > 0.0f) ? 1.0f : 0.0f;
	return keys.AddKey(this,t,k);
	}


void BoolCntrl::CommitValue(TimeValue t) {
	if (TestAFlag(A_SET)) {		
		ClearAFlag(A_SET);
		if (ivalid.InInterval(t)) {
			if (keys.Count()==0) {
				if (Animating()&&(t!=TimeValue(0))) {					
					BKey k;
					tmpStore.GetBytes(sizeof(float),&k.val,this);
					k.val = (k.val > 0.0f) ? 1.0f : 0.0f;
					k.time = 0;
					keys.AddKey(this,0,k);
					AddKeyFromCurValue(t);
				}
			}
			else {
				if (Animating()) 
					AddKeyFromCurValue(t);
/*				else  { // (if !Animating() then flip the curve)
					float vval;
					Interval iv;
					GetValue(t, &vval, iv);
					if( curval != vval){
						if (startVal == 0.0) 
							startVal = 1.0;
						else if (startVal == 1.0) 
							startVal = 0.0;
						for (int i = 0; i < keys.Count(); i++) {
							if (keys[i].val == 1.0) keys[i].val = 0.0;
							else if (keys[i].val == 0.0) keys[i].val = 1.0;
						}
					}
				}
*/
				else {
					// (if !Animating() then flip the curve)
					float old;
					tmpStore.GetBytes(sizeof(float),&old,this);
					if (curval != old) {
						for (int i = 0; i < keys.Count(); i++) {
							if (keys[i].val == 1.0) keys[i].val = 0.0;
							else if (keys[i].val == 0.0) keys[i].val = 1.0;
						}
					}
				}
			}

		}
//				else {
//					float old;
//					tmpStore.GetBytes(sizeof(float),&old,this);					
//					OffsetKeys(curval-old);
//					}
//	}
		tmpStore.Clear(this);
		keys.KeysChanged(FALSE);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
}


/*
class BoolCntrlPBAccessor : public PBAccessor
{ 
	public:

		void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
		{
			BoolCntrl* b = (BoolCntrl*)owner;			
			UpdateWindow(b->hWnd);

		}
};
*/


//static BoolCntrlPBAccessor bool_cntrl_accessor;


BoolCntrl *BoolCntrl::editCont = NULL;
HWND BoolCntrl::hWnd = NULL;
	
static Class_ID BoolCntrlClassID(BOOLCNTRL_CLASS_ID); 
class BoolCntrlClassDesc:public ClassDesc2 {
	public:
	BOOL HasClassParams() {return TRUE;}
	int 			IsPublic() { return 1; }	
	const TCHAR *	ClassName() { return BOOL_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID 	   	ClassID() { return BoolCntrlClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("boolcntrl"); }	// returns fixed parsable name (scripter-visible name)
	void *			Create(BOOL loading) { return new BoolCntrl(loading); }	
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
	};


static BoolCntrlClassDesc BoolCntrlDesc;
ClassDesc2* GetBoolCntrlDesc() { return &BoolCntrlDesc; }

 BoolCntrl::BoolCntrl(BOOL loading) 
	{

	ivalid.SetInfinite();
	curval =  1.0;
//	startVal = 1.0;

	// make the paramblock
//	rangeLinked  = TRUE;
//	BoolCntrlDesc.MakeAutoParamBlocks(this);
//	pblock->CallSets();
	setkeyBuffer=NULL;
	mSetKeyFlags=0;
}


BoolCntrl::~BoolCntrl()
	{
	if (setkeyBuffer) delete setkeyBuffer;
	setkeyBuffer = NULL;
	}

RefTargetHandle BoolCntrl::Clone(RemapDir &remap)
	{
	BoolCntrl *bc = new BoolCntrl(TRUE);
//	bc->ReplaceReference(BOOL_PBLOCK_REF, pblock->Clone(remap));
	bc->curval	   = curval;
//	bc->startVal   = startVal;
//	bc->range      = range;
	bc->ivalid     = ivalid;
	bc->keys       = keys;
	BaseClone(this, bc, remap);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return bc;
	}


/*
class BoolCtrlDlgProc : public ParamMap2UserDlgProc 
{
	public:

		void UpdateBoolName(BoolCntrl* b){
			IParamMap2* pmap = b->pblock->GetMap();
		}

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{	
			
			BoolCntrl * b = (BoolCntrl*)map->GetParamBlock()->GetOwner();
			UpdateBoolName(b);
			b->hWnd = hWnd;

			switch (msg) {

				case WM_INITDIALOG:  
					
					// probably don't need this at all.
			
				break;


				case WM_COMMAND: // probably don't need this at all.

					if ( LOWORD(wParam) == IDC_BOOL_ON || LOWORD(wParam) == IDC_BOOL_OFF){
						int a = 1;

					}
				
				break;


				case CC_SPINNER_CHANGE:  // probably don't need this at all.
				
				
				break;
			}
			return FALSE;
		}

		void SetParamBlock(IParamBlock2* pb) 
		{ 
			UpdateBoolName((BoolCntrl*)pb->GetOwner());
		}

		void DeleteThis() { }
};

*/

//static BoolCtrlDlgProc boolCtrlDlgProc;

/* enum { boolcntrl_params };
static ParamBlockDesc2 boolcntrl_paramblk (boolcntrl_params, _T("BoolCntrlParameters"),  0, &BoolCntrlDesc, 
										   P_AUTO_CONSTRUCT, BOOL_PBLOCK_REF, 
	//rollout
	bool_on_off, _T("on_off_mode"),	TYPE_FLOAT, 0, NULL,
		p_default, 		1.0, 
		p_range, 		0.0, 1.0, 
		end,

	end
	);

*/

class BoolCntrlRestore : public RestoreObj {
	public:
		BoolCntrlKeyTable undo;
		BoolCntrlKeyTable redo;
		float curvalUndo;
		float curvalRedo;
		// Interval urange, rrange;
		// BOOL rangeLinkedU;
		// BOOL rangeLinkedR;
		BoolCntrl *cont;

		BoolCntrlRestore(BoolCntrl *c) {
			cont   = c;
			undo   = c->keys;			
			curvalUndo = c->curval;
			// urange = c->range;
			// rangeLinkedU = c->rangeLinked;
			}
		
		void Restore(int isUndo) {
			if (isUndo) {
				redo   = cont->keys;
				curvalRedo = cont->curval;
				// rrange = cont->range;
				// rangeLinkedR = cont->rangeLinked;
				}
			cont->keys  = undo;
			cont->curval = curvalUndo;
			// cont->range = urange;
			// cont->rangeLinked = rangeLinkedU;
			cont->SetValidEmpty();
			cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			}

		void Redo() {
			cont->keys  = redo;
			cont->curval = curvalRedo;
			// cont->range = rrange;
			// cont->rangeLinked = rangeLinkedR;
			cont->SetValidEmpty();
			cont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			}

		int Size() {return 1;}
		void EndHold() {cont->ClearAFlag(A_HELD);}
		TSTR Description() { return TSTR(_T("BoolCntrlRestore")); }
	};



class BoolClipObject : public TrackClipObject {
	public:
		BoolCntrlKeyTable tab;


		void DeleteThis() {delete this;}

		int NumKeys() {return Count();}
		BOOL GetKeyVal(int i, void *val) {
			*((float*)val) = tab[i].val;
			return TRUE;
			}
		BOOL SetKeyVal(int i, void *val) {
			tab[i].val = (*((float*)val) > 0.0f) ? 1.0f : 0.0f;
			return TRUE;
			}


		BoolClipObject(Interval iv) : TrackClipObject(iv) {}
		Class_ID ClassID() {return BOOLCNTRL_CLASS_ID;}
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }

		int Count() {return tab.Count();}
		void Append(BKey &k) {tab.Append(1,&k,10);}
		BKey& operator[](int i) {return tab[i];}
	};

//-----------------------------------------------------------------------------




#define INTERVAL_CHUNK 		0x2500
#define TRACKFLAG_CHUNK		0x3002
#define RANGE_CHUNK			0x3003
#define JOINTPARAM_CHUNK	0x3004
#define BKEY_CHUNK			0x3005
#define FLOAT_CHUNK			0x3006
#define START_CHUNK			0x3007


IOResult BoolCntrl::Save(ISave *isave){
	ULONG nb;	
	Control::Save(isave);

	isave->BeginChunk(FLOAT_CHUNK);
	isave->Write(&curval,sizeof(float),&nb);			
	isave->EndChunk();

//	isave->BeginChunk(START_CHUNK);
//	isave->Write(&startVal,sizeof(float),&nb);			
//	isave->EndChunk();

	isave->BeginChunk(INTERVAL_CHUNK);
	isave->Write(&ivalid,sizeof(ivalid),&nb);			
	isave->EndChunk();
	
	isave->BeginChunk(TRACKFLAG_CHUNK);
	isave->Write(&keys.tflags,sizeof(keys.tflags),&nb);			
	isave->EndChunk();

	isave->BeginChunk(RANGE_CHUNK);
	isave->Write(&keys.range,sizeof(keys.range),&nb);			
	isave->EndChunk();


	if (keys.Count()>0) {
		isave->BeginChunk(BKEY_CHUNK);
		isave->Write(keys.Addr(0), sizeof(BKey)*keys.Count(), &nb);
		isave->EndChunk();		
		}


	return(IO_OK);
	}


IOResult BoolCntrl::Load(ILoad *iload)
{
	
	ULONG nb;
	IOResult res = IO_OK;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			
			case FLOAT_CHUNK:
				res=iload->Read(&curval, sizeof(curval), &nb);
			break;

//			case START_CHUNK:
//				res=iload->Read(&startVal, sizeof(startVal), &nb);
//			break;

			case INTERVAL_CHUNK:
				res=iload->Read(&ivalid, sizeof(ivalid), &nb);
			break;

			case TRACKFLAG_CHUNK:
				res=iload->Read(&keys.tflags, sizeof(keys.tflags), &nb);
			break;

			case RANGE_CHUNK:
				res=iload->Read(&keys.range, sizeof(keys.range), &nb);
			break;

			case BKEY_CHUNK:

				int nkeys = (iload->CurChunkLength())/sizeof(BKey);
				keys.SetCount(nkeys);				
				if (nkeys>0) {
					res = iload->Read(keys.Addr(0),nkeys*sizeof(BKey),&nb);
				}
			break;
			
		}		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	
	keys.KeysChanged(FALSE);
	return IO_OK;
}

void BoolCntrl::Copy(Control *from) {
	if (from!=NULL) {
		assert(from->SuperClassID()==SuperClassID());
		float v;
		Interval iv;			
		int num;
		keys.Resize(0);
			
		OperatorIndex(v,0) = 0.0f;				
				

		if ((num=from->NumKeys())!=NOT_KEYFRAMEABLE && num>0) {			
			SuspendAnimate();
			AnimateOn();
			for (int i=0; i<num; i++) {
				TimeValue t = from->GetKeyTime(i);
				from->GetValue(t,&v,iv);
				AddNewKey(t,ADDKEY_INTERP);
				SetValue(t,&v);	
			}
			ResumeAnimate();
		} 
		else {
			from->GetValue(0,&v,iv); 		
			SetValue(0,&v);	
		}
	}
}



void BoolCntrl::CommitSetKeyBuffer(TimeValue t)
	{
	if(keys.TrackNotKeyable()) return;
	if (setkeyBuffer) {
		curval = setkeyBuffer->oldVal;
		HoldTrack();
		keys = *setkeyBuffer;		
		delete setkeyBuffer;
		setkeyBuffer = NULL;
		ivalid.SetEmpty();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

void BoolCntrl::RevertSetKeyBuffer()
	{
	if (setkeyBuffer) {
		HoldSetKeyBuffer();
		curval = setkeyBuffer->oldVal;
		delete setkeyBuffer;
		setkeyBuffer = NULL;
		ivalid.SetEmpty();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

void BoolCntrl::RevertSetKeyBuffer(BOOL clearHeldFlag)
	{
	RevertSetKeyBuffer();
	// after tossing the set-key buffer, we are probably about to call
	// HoldTrack() which checks the 'held' flag before doing a theHold.Put().
	// We want both operations to do a put.
	ClearAFlag(A_HELD);
	}


class SetKeyBufferRest : public RestoreObj {
	public:
		BoolCntrl *cont;
		SetKeyBuffer svBufUndo;
		SetKeyBuffer svBufRedo;
		float curvalUndo, curvalRedo;
//		float startValUndo, startValRedo;
		BOOL noBufUndo, noBufRedo;

		SetKeyBufferRest(BoolCntrl *c) {
			cont = c;
			curvalUndo = cont->GetCurVal();
//			startValUndo = cont->startVal;
			if (cont->GetSetKeyBuffer()) {
				noBufUndo = FALSE;
				svBufUndo = *(cont->GetSetKeyBuffer());
			} else {
				noBufUndo = TRUE;
				}
			noBufRedo = FALSE;
			}

		void Restore(int isUndo) {
			if (isUndo) {
				if (cont->GetSetKeyBuffer()) {
					noBufRedo = FALSE;
					svBufRedo = *(cont->GetSetKeyBuffer());
				} else {
					noBufRedo = TRUE;
					}
				curvalRedo = cont->GetCurVal();
//				startValRedo = cont->startVal;
				}
			if (noBufUndo) {
				if (cont->GetSetKeyBuffer()) {
					delete cont->GetSetKeyBuffer();
					cont->GetSetKeyBuffer() = NULL;
					}
			} else {
				if (!cont->GetSetKeyBuffer()) {
					cont->GetSetKeyBuffer() = new SetKeyBuffer(cont->GetCurVal());
					}
				*(cont->GetSetKeyBuffer()) = svBufUndo;
				}
			cont->GetCurVal() = curvalUndo;
//			cont->startVal = startValUndo;
			cont->SetValidEmpty();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo() {
			if (noBufRedo) {
				if (cont->GetSetKeyBuffer()) {
					delete cont->GetSetKeyBuffer();
					cont->GetSetKeyBuffer() = NULL;
					}
			} else {
				if (!cont->GetSetKeyBuffer()) {
					cont->GetSetKeyBuffer() = new SetKeyBuffer(cont->GetCurVal());
					}
				*(cont->GetSetKeyBuffer()) = svBufRedo;
				}
			cont->GetCurVal() = curvalRedo;
//			cont->startVal = startValRedo;
			cont->SetValidEmpty();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() {cont->ClearAFlag(A_HELD);}
		TSTR Description() { return TSTR(_T("SetKeyBufferRest")); }
	};



void BoolCntrl::HoldSetKeyBuffer()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {		
		theHold.Put(new SetKeyBufferRest(this));
		SetAFlag(A_HELD);
		}
	}

void BoolCntrl::HoldTrack()
	{
	if (theHold.Holding()&&!TestAFlag(A_HELD)) {		
		theHold.Put(new BoolCntrlRestore(this));
		SetAFlag(A_HELD);
		}
	}

void BoolCntrl::EditTimeRange(Interval range,DWORD flags)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	if (flags&EDITRANGE_LINKTOKEYS) {
		keys.ClearTFlag(RANGE_UNLOCKED);
	} else {
		keys.SetTFlag(RANGE_UNLOCKED);
		keys.range = range;
		}
	keys.KeysChanged(FALSE);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Interval BoolCntrl::GetTimeRange(DWORD flags)
	{
	if (GetSetKeyMode() && setkeyBuffer && TestAFlag(A_USE_SETVAL_BUFFER_RANGE)) {
		return setkeyBuffer->range;
		}

	Interval iv;	
	int n = keys.Count();
	if (!n) return iv;
	if (flags&TIMERANGE_SELONLY) {
		for (int i = 0; i < n; i++) {
			if (keys[i].Testflags(KEY_SELECTED)) {
				if (iv.Empty()) {
					iv.SetInstant(keys[i].time);	
				} else {
					iv += keys[i].time;
					}
				}
			}
		return iv;
	} else {
		return keys.range;
		}
	}


int BoolCntrl::GetSelKeyCoords(TimeValue &t, float &val,DWORD flags) 
	{
	BOOL tfound=FALSE, vfound = FALSE, tuncommon = FALSE, vuncommon = FALSE;
	int n = keys.Count(), res;
	TimeValue atime;
	float aval;
	for (int i = 0; i < n; i++ ) {
		if (flags&KEYCOORDS_TIMEONLY) {
			if ( keys[i].Testflags(KEY_SELECTED) ) {
				if (tfound) {
					if (keys[i].time!=atime) {
						return KEYS_MULTISELECTED;
						}
				} else {
					tfound = TRUE;
					atime = keys[i].time;
					}
				}
		} else {
			for (int j=0;j<1;j++) {
				if (keys[i].ElemSelected(j)) {
					if (tfound) {
						if (keys[i].time!=atime) {
							tuncommon = TRUE;
							}						
					} else {
						tfound = TRUE;
						atime = keys[i].time;
						}
					if (vfound) {
						if (aval != keys[i][j]) {
							vuncommon = TRUE;
							} 
					} else {
						vfound = TRUE;
						aval  = keys[i][j];
						}
					}
				}
			}
		if (tuncommon && vuncommon) return KEYS_MULTISELECTED;
		}
	t   = atime;
	val = aval;
	res = 0;
	if (tfound && !tuncommon) {
		res	|= KEYS_COMMONTIME;
		}
	if (vfound && !vuncommon) {
		res	|= KEYS_COMMONVALUE;
		}
	if (!tfound && !vfound) {
		res = KEYS_NONESELECTED;
		}
	return res;
	}


int BoolCntrl::SetSelKeyCoordsExpr(
		ParamDimension *dim,
		TCHAR *timeExpr, TCHAR *valExpr, DWORD flags)
	{
	Expr texpr, vexpr;	
	float vin, vout=0.0f, tfin, tfout=0.0f;
	
	if (timeExpr) {
		texpr.defVar(SCALAR_VAR,KEYCOORDS_TIMEVAR);
		if (texpr.load(timeExpr)!=EXPR_NORMAL) return KEYCOORDS_EXPR_ERROR;		
		}
	if (valExpr) {
		vexpr.defVar(SCALAR_VAR,KEYCOORDS_VALVAR);
		if (vexpr.load(valExpr)!=EXPR_NORMAL) return KEYCOORDS_EXPR_ERROR;		
		}

	int n = keys.Count();
	if (!n) return KEYCOORDS_EXPR_OK;
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	for (int i = 0; i < n; i++ ) {
		if (!(flags&KEYCOORDS_VALUEONLY)) {
			if (keys[i].TimeLocked()) continue;
			if (keys[i].Testflags(KEY_SELECTED)) {				
				tfin = float(keys[i].time)/float(GetTicksPerFrame());
				texpr.eval(&tfout, 1, &tfin);
				keys[i].time = int(tfout*GetTicksPerFrame());
				}
			}
		if (!(flags&KEYCOORDS_TIMEONLY)) {
			if (keys[i].ElemSelected(0)) {
				vin = dim->Convert(keys[i][0]);
				vexpr.eval(&vout, 1, &vin);
				// CAL-06/28/02: restrict the value to be either 0 or 1
				keys[i][0] = (dim->UnConvert(vout) > 0.0f) ? 1.0f : 0.0f;
			}
		}
	}	
	keys.KeysChanged();
	keys.CheckForDups();
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	return KEYCOORDS_EXPR_OK;
	}

void BoolCntrl::SetSelKeyCoords(TimeValue t, float val,DWORD flags)
	{
	int n = keys.Count();
	if (!n) return;
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	for (int i = 0; i < n; i++ ) {
		if (flags&KEYCOORDS_TIMEONLY) {
			if (keys[i].TimeLocked()) continue;
			if (keys[i].Testflags(KEY_SELECTED)) {
				keys[i].time = t;
				}
		} 
		else {
			if (keys[i].ElemSelected(0)) {
				// CAL-06/28/02: restrict the value to be either 0 or 1
				keys[i][0] = (val > 0.0f) ? 1.0f : 0.0f;					
				if (!(flags&KEYCOORDS_VALUEONLY)) {
					if (keys[i].TimeLocked()) continue;
					keys[i].time = t;
				}
			}
		}
	}	
	keys.KeysChanged();
	keys.CheckForDups();
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}


void BoolCntrl::MoveKeys(ParamDimensionBase *dim,float delta,DWORD flags)
	{
	int n = keys.Count();
	if (!n) return;
	float m = 1.0f;
	Interval valid;
	BOOL changed = FALSE;
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	for (int i = 0; i < n; i++ ) {
			if (keys[i].AnyElemSelected()) {
				m = GetMultVal(keys[i].time,valid);	
			}
			if (keys[i].ElemSelected(0)) {
				keys[i][0] = dim->UnConvert(dim->Convert(keys[i][0]*m)+delta)/m;
				changed = TRUE;
				if (keys[i][0] > 0.0) {
					if (delta < 0.0){
						keys[i][0] = 0.0;
					}
					else
						keys[i][0] = 1.0;
				}
				else if (keys[i][0] <= 0.0) {
					if (delta > 0.0){
						keys[i][0] = 1.0;
					}
					else
						keys[i][0] = 0.0;
				}
			}		
	}	
	if (changed) {
		keys.KeysChanged(TRUE); // FALSE indicates that key times didn't change so sorting isn't necessary.
		ivalid.SetEmpty();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}


void BoolCntrl::MapKeys(TimeMap *map,DWORD flags)
	{
	int n = keys.Count();
	BOOL changed = FALSE;
	if (!n || ((flags&TRACK_DOSEL)&&NumSelKeys()<1)) return;
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	if (flags&TRACK_MAPRANGE && keys.TestTFlag(RANGE_UNLOCKED)) {		
		TimeValue t0 = map->map(keys.range.Start());
		TimeValue t1 = map->map(keys.range.End());
		keys.range.Set(t0,t1);
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
		keys.KeysChanged(TRUE);
		keys.CheckForDups();
		ivalid.SetEmpty();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}




int BoolCntrl::GetKeyIndex(TimeValue t)
	{
	for (int i=0; i<keys.Count(); i++) {
		if (keys[i].time==t) return i;
		if (keys[i].time>t) return -1; // RB 10/2/2000 changed from "keys[i].time<t". 
		}
	return -1;
	}

void BoolCntrl::DeleteKeyAtTime(TimeValue t)
	{
	int index = GetKeyIndex(t);
	if (index>=0) {
		RevertSetKeyBuffer(TRUE);
		HoldTrack();
		keys.DeleteKey(index);
		keys.KeysChanged(FALSE);
		ivalid.SetEmpty();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

BOOL BoolCntrl::IsKeyAtTime(TimeValue t,DWORD flags)
	{
	for (int i=0; i<keys.Count(); i++) {
		if (keys[i].time>t) return FALSE;
		if (keys[i].time==t) return TRUE;		
		}
	return FALSE;
	}

void BoolCntrl::DeleteTime(Interval iv, DWORD flags)
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
	
	keys.KeysChanged();
	keys.CheckForDups();
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolCntrl::ReverseTime(Interval iv, DWORD flags)
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

	keys.KeysChanged();
	keys.CheckForDups();
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolCntrl::ScaleTime(Interval iv, float s)
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

	keys.KeysChanged();
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolCntrl::InsertTime(TimeValue ins, TimeValue amount)
	{
	int n = keys.Count();		
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	for (int i = 0; i < n; i++) {		
		if (keys[i].time >= ins) {
			keys[i].time += amount;
			}		
		}

	keys.KeysChanged();
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolCntrl::DeleteKeys(DWORD flags)
	{
	int n = keys.Count();		
	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	
	for (int i = n-1; i >= 0; i--) {
		if (flags&TRACK_DOALL || keys[i].flags&KEY_SELECTED) {
			keys.Delete(i,1);
			}
		}

	keys.KeysChanged(FALSE);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolCntrl::DeleteKeyByIndex(int index)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	keys.DeleteKey(index);
	keys.KeysChanged(FALSE);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BoolCntrl::SelectKeyByIndex(int i,BOOL sel)
	{
	HoldTrack();
	if (sel) keys[i].flags |=  KEY_SELECTED;
	else     keys[i].flags &= ~KEY_SELECTED;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}


void BoolCntrl::SelectKeys( TrackHitTab& sel, DWORD flags )
	{
	HoldTrack();
	BOOL fcurve = flags&SELKEYS_FCURVE;

	if (flags&SELKEYS_CLEARKEYS) {
		int n = keys.Count();
		for (int i = 0; i < n; i++ ) {
			keys[i].Clearflags(KEY_ALLSEL);
			}
		}
	if ((flags&SELKEYS_CLEARCURVE)) {
		keys.ClearTFlag(CURVE_SELECTED);
		}
	if (flags&SELKEYS_DESELECT) {
		for (int i = 0; i < sel.Count(); i++ ) {
			if (fcurve) {				
				keys[sel[i].hit].DeselectElemBits(sel[i].flags);
			} else {
				keys[sel[i].hit].Clearflags(KEY_ALLSEL);
				}
			}		
		} 	
	if (flags&SELKEYS_SELECT) {	
		if (fcurve) {
			// If the curve isn't yet selected, eat the input and just
			// select the curve.
			if (!keys.TestTFlag(CURVE_SELECTED)) {				
				keys.SetTFlag(CURVE_SELECTED);
				return;
				}
			}
		for (int i = 0; i < sel.Count(); i++ ) {
			if (fcurve) {
				keys[sel[i].hit].SelectElemBits(sel[i].flags);
			} else {
				keys[sel[i].hit].Setflags(KEY_ALLSEL);
				}
			}
		}	
	// 274946 (also 171917)
	tmpStore.PutInt('JZ', &tmpStore);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	tmpStore.PutInt(0, &tmpStore);
	}


void BoolCntrl::FlagKey(TrackHitRecord hit)
	{
	int n = keys.Count();
	for (int i = 0; i < n; i++) {
		keys[i].flags &= ~KEY_FLAGGED;
		}
	assert(hit.hit>=0&&hit.hit<(DWORD)n);
	keys[hit.hit].flags |= KEY_FLAGGED;
	}

int BoolCntrl::GetFlagKeyIndex()
	{
	int n = keys.Count();
	for (int i = 0; i < n; i++) {
		if (keys[i].flags & KEY_FLAGGED) {
			return i;
			}
		}
	return -1;
	}

int BoolCntrl::NumSelKeys()
	{
	int n = keys.Count();
	int c = 0;
	for ( int i = 0; i < n; i++ ) {
		if (keys[i].Testflags(KEY_SELECTED)) {
			c++;
			}
		}
	return c;
	}

void BoolCntrl::CloneSelectedKeys(BOOL offset)
	{
	int n = keys.Count();			
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	BOOL changed = FALSE;

	for (int i = 0; i < n; i++) {
		if (keys[i].flags & KEY_SELECTED) {
			BKey key(keys[i].time);
			// CAL-09/09/02: need to copy the value as well. Also, use KEY_ALLSEL flag instead, so that the
			//		function curve editor can edit the newly created keys.
			key.val = keys[i].val;
			// key.flags |= KEY_SELECTED;
			key.Setflags(KEY_ALLSEL);
			keys.Append(1,&key,5);
			// keys[i].flags &= ~KEY_SELECTED;
			keys[i].Clearflags(KEY_ALLSEL);
			changed = TRUE;
			}
		}
	if (changed) {
		keys.Shrink();
		ivalid.SetEmpty();
		SortKeys();
		}
	}



inline void BoolCntrl::AddNewKey(
		TimeValue t,DWORD flags)
	{
	Interval iv;	
	int i;
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	BKey key(t);
	i = AddKeyFromCurValue(t);
	if(GetSetKeyMode()) return;
	
	if (flags&ADDKEY_SELECT) {
		keys[i].Setflags(KEY_ALLSEL);
		}			
	if (flags&ADDKEY_FLAGGED) {
		int n = keys.Count();
		for (int j = 0; j < n; j++) {
			keys[j].Clearflags(KEY_FLAGGED);
			}		
		keys[i].Setflags(KEY_FLAGGED);
		}

	keys.KeysChanged(FALSE);
	keys.CheckForDups();
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}




BOOL BoolCntrl::IsKeySelected(int index)
	{
	return keys[index].flags & KEY_SELECTED;
	}

TrackClipObject *BoolCntrl::CopyTrack(Interval iv, DWORD flags)
	{
	BoolClipObject *cobj = new BoolClipObject(iv);	
	Interval test = TestInterval(iv,flags);	
	for (int i = 0; i < keys.Count(); i++) {
		if (test.InInterval(keys[i].time)) {
			cobj->Append(keys[i]);
			}
		}
	cobj->tab.Shrink();
	return cobj;
	}

void BoolCntrl::PasteTrack(TrackClipObject *cobj,Interval iv, DWORD flags)
	{
	BoolClipObject *cob = (BoolClipObject*)cobj;	
	RevertSetKeyBuffer(TRUE);
	HoldTrack();		
	DeleteTime(iv,flags);	
	InsertTime(iv.Start(),cob->clip.Duration()-1);	
	for (int i = 0; i < cob->tab.Count(); i++) {
		BKey key(cob->tab[i].time);
		key.time -= cob->clip.Start() - iv.Start();
		key.val = cob->tab[i].val;
		keys.Append(1,&key);
		}
	keys.KeysChanged();
	keys.CheckForDups();
	ivalid.SetEmpty();
	// SortKeys();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

int BoolCntrl::HitTestTrack(			
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
				if (flags&HITTRACK_ABORTONHIT) return TRACK_DOSTANDARD;
				}
			}		
		}
	return TRACK_DOSTANDARD;
	}


int BoolCntrl::PaintTrack(			
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
	int y = (rcTrack.top+rcTrack.bottom)/2, lx = rcPaint.left-2;
	HBRUSH selBrush   = CreateSolidBrush(RGB(255,255,255));
	HBRUSH barBrush   = CreateSolidBrush(RGB(113,160,231)); //RGB(80,100,180)
	HBRUSH unselBrush = (HBRUSH)GetStockObject(GRAY_BRUSH);	
	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc,barBrush);
	int xi, xip1;
	
	float cval;
	GetValue(GetAnimStart(), &cval, Interval(0, 0));
	BOOL state = MAKEBOOL(cval);

	switch (n)
	{
		case 0:
		{
			if (!Animating()){
				if (state) {		
					Rectangle(hdc,lx,y-4,rcPaint.right,y+4);
				}
			}
			break;
		}
		case 1:
		{
			if (keys[0].val) {
				xi  = TimeToScreen(keys[0].time,zoom,scroll);
				Rectangle(hdc,xi,y-4,rcPaint.right,y+4);
			}
			break;
		}
		default:
		{
			if (keys[n-1].val) {
				xi  = TimeToScreen(keys[n-1].time,zoom,scroll);
				Rectangle(hdc,xi,y-4,rcPaint.right,y+4);
			}
			for (int i = n-2; i > -1; i--) {	
				if (keys[i].time > right) {
					break;
				}
				xi  = TimeToScreen(keys[i].time,zoom,scroll);
				xip1 = TimeToScreen(keys[i+1].time,zoom,scroll);
				if (keys[i].time > left) {			
					if (keys[i].val) {							
						Rectangle(hdc,xi,y-4,xip1,y+4);
					}
				}		
		
			}
			break;
		}
	}

	
	for (int i = 0; i < n; i++) {
		if (keys[i].time > right) {
			break;
			}
		if (keys[i].time > left) {			
			xi  = TimeToScreen(keys[i].time,zoom,scroll);
			if ((flags&PAINTTRACK_SHOWSEL) && (keys[i].flags&KEY_SELECTED)) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}
			Ellipse(hdc,xi-4,y-5,xi+4,y+5);
			}		
		}

	DeleteObject(selBrush);
	DeleteObject(barBrush);
	return TRACK_DOSTANDARD;
	}

// CAL-11/26/2002: override this method to use setkeyBuffer range
void BoolCntrl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if (GetSetKeyMode() && setkeyBuffer) {
		SetAFlag(A_USE_SETVAL_BUFFER_RANGE);
		StdControl::GetValue(t, val, valid, method);
		ClearAFlag(A_USE_SETVAL_BUFFER_RANGE);
	} else {
		StdControl::GetValue(t, val, valid, method);
		}
	}

// CAL-11/11/2002: remove the overriding of GetValue (use the inherited function from StdControl to
//		handle ORTs.) and replace it by GetValueLocalTime().
void BoolCntrl::GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if (!ivalid.InInterval(t)) {
		ClearAFlag(A_SET);
		if (GetSetKeyMode() && setkeyBuffer) {
			setkeyBuffer->InterpValue(t, curval, ivalid);
		} else {
			keys.InterpValue(t,curval,ivalid);
			}
		}
	// CAL-11/27/2002: this isn't necessary anymore. InterpValue will include the t time instant to ivalid
	// if (!ivalid.InInterval(t)) {	// fix to 462993, 021023  --prs.
	//	ivalid.SetInstant(t);
	//	}
	valid &= ivalid;
	(*(float*)val) = curval;
	}


int SetKeyBuffer::GetKeyIndex(TimeValue t)
	{
	for (int i=0; i<Count(); i++) {
		if ((*this)[i].time==t) return i;
		if ((*this)[i].time>t) return -1;
		}
	return -1;
	}

float GetSetValueRelVal(float *val) 
	{
	return *((float*)val);
	}

void BoolCntrlKeyTable::KeysChanged(BOOL timeChanged)
	{
	if (timeChanged) {
//		SortKeys();
		Sort((CompareFnc)CompareBKeys);		
		}
	if (!TestTFlag(RANGE_UNLOCKED)) {
		if (Count()) {
			range.Set((*this)[0].time,(*this)[Count()-1].time);
		} else {
			range.SetEmpty();
			}
		}
	}

// CAL-11/26/2002: override this method to use setkeyBuffer range
void BoolCntrl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
	{
	if (GetSetKeyMode() && setkeyBuffer) {
		SetAFlag(A_USE_SETVAL_BUFFER_RANGE);
		StdControl::SetValue(t, val, commit, method);
		ClearAFlag(A_USE_SETVAL_BUFFER_RANGE);
	} else {
		StdControl::SetValue(t, val, commit, method);
		}
	}

void BoolCntrl::SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method)
	{	

	// CAL-11/27/2002: it may not be the desired behavior to return when the value to set is equal to the current value
	// float *v = (float*)val;
	// float vval;
	// GetValueLocalTime(t, &vval, FOREVER);	// CAL-11/11/2002: get local vlaue
	// if(!GetSetKeyModeStatus()) { if( *v == vval) return; }

	// CAL-11/27/2002: call Update() to update the current value
	Update(t);
	
	if (GetSetKeyMode()) {

		// RB 11/01/2000: When set-key mode is on, store changes through SetValue()
		// in an alternate key table until the user decides to commit their changes.
		
		// Save state for undo
		HoldSetKeyBuffer();

		// If this controller hasn't been modified since set-key mode was turned on,
		// we need to make one now.
		if (!setkeyBuffer) {
			// Make a copy of the existing key table. We'll also preserve the old
			// 'curval' in case we didn't previously have any keys
			setkeyBuffer = new SetKeyBuffer(curval, keys);
			}			
					
		// Modify current val
		if (method==CTRL_RELATIVE) {
			curval += GetSetValueRelVal((float*)val);
		} else {		
			curval = *((float*)val);
			}

		// Find the key at the current time
		int ix = setkeyBuffer->FindKey(t);	

		if (ix<0 || (*setkeyBuffer)[ix].time!=t) {
			// No key at current time. Add one using current (modified) value
			BKey k;
			k.val  = (curval > 0.0f) ? 1.0f : 0.0f;
			k.time = t;
			setkeyBuffer->AddKey(this, t, k);

			// Flag table for update
			setkeyBuffer->KeysChanged(TRUE);

		} else {
			
			// Update the key's value to reflect the change
			(*setkeyBuffer)[ix].val = (curval > 0.0f) ? 1.0f : 0.0f;

			// Flag table for update
			setkeyBuffer->KeysChanged(FALSE);
			}

		ivalid.SetInstant(t);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	} else {

		// Non-set-key mode.

		if(keys.TrackNotKeyable()) return;	// CAL-10/8/2002: test if track is keyable
		if (!TestAFlag(A_SET)) {				
			HoldTrack();
			tmpStore.PutBytes(sizeof(float),&curval,this);
//			tmpStore.PutBytes(sizeof(float),&startVal,this);
			SetAFlag(A_SET);
			}

		if (method==CTRL_RELATIVE) {
			curval += GetSetValueRelVal((float*)val);
			//curval += *((T*)val);
		} 
		else {		
			curval = *((float*)val);
		}
		if (curval > 0.0) curval = 1.0;
		else if (curval <= 0.0) curval = 0.0;

#if 0
		if (!Animating()){
//			if (startVal == 0.0) 
//				startVal = 1.0;
//			else if (startVal == 1.0) 
//				startVal = 0.0;
			for (int i = 0; i < keys.Count(); i++) {
				if (keys[i].val == 1.0) 
					keys[i].val = 0.0;
				else if (keys[i].val == 0.0) 
					keys[i].val = 1.0;
			}
		}
#endif

//		pblock->SetValue(bool_on_off, GetCOREInterface()->GetTime(), curval);
		ivalid.SetInstant(t);
		if (commit) CommitValue(t);
		keys.KeysChanged(FALSE);
		if (!commit) NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}	
	}



void BoolCntrl::Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type)
	{
	Interval iv;
	GetValueLocalTime(t,val,iv);		// CAL-11/11/2002: get local value
	valid &= iv;
	}
/*
void BoolCntrl::SortKeys()
	{
	keys.Sort((CompareFnc)CompareBKeys);
	}
*/



/*
  float BoolCntrl::Get_on_off_mode(){
	float on_off;
	pblock->GetValue(bool_on_off, GetCOREInterface()->GetTime(), on_off, FOREVER);
	if (on_off == 0 || on_off == 1) return on_off;
	else return -1;
}


  void BoolCntrl::Set_on_off_mode(float on_off)
	{
	pblock->SetValue(bool_on_off, GetCOREInterface()->GetTime(), on_off);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}



RefTargetHandle BoolCntrl::GetReference(int i)
	{
		switch (i)
		{
			case BOOL_PBLOCK_REF:
				return pblock;
		}
		return NULL;
	}

void BoolCntrl::SetReference(int i, RefTargetHandle rtarg)
	{
		switch (i)
		{
			case BOOL_PBLOCK_REF:
				pblock = (IParamBlock2*)rtarg; break;
		}
	}
*/

int BoolCntrl::SetProperty(ULONG id, void *data)
	{
	if (id==PROPID_JOINTPARAMS) {		
		if (!data) {
			int index = aprops.FindProperty(id);
			if (index>=0) {
				aprops.Delete(index,1);
				}
		} else {
			JointParams *jp = (JointParams*)GetProperty(id);
			if (jp) {
				*jp = *((JointParams*)data);
				delete (JointParams*)data;
			} else {
				aprops.Append(1,(AnimProperty**)&data);
				}					
			}
		return 1;
	} else
	if (id==PROPID_INTERPUI || id==PROPID_KEYINFO) {		
		int index = aprops.FindProperty(id);
		if (!data) {			
			if (index>=0) {				
				aprops.Delete(index,1);
				}
		} else {
			if (index>=0) {	
				assert(0);
			} else {
				aprops.Append(1,(AnimProperty**)&data);
				}
			}
		return 1;	
	} else {
		return Animatable::SetProperty(id,data);
		}
	}


void* BoolCntrl::GetProperty(ULONG id)
	{
	if (id==PROPID_INTERPUI || id==PROPID_JOINTPARAMS || id==PROPID_KEYINFO) {
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





class StdDrawFCurveCallback : public FCurveCallback {
	public:
		Control *cont;
		ParamDimensionBase *dim;
		HDC hdc;
		Rect rcGraph;
		Rect rcPaint;
		float tzoom;
		int tscroll;
		float vzoom;
		int vscroll;
		DWORD flags;
		HPEN pen[4], dpen[4], bpen, otPen;
		HBRUSH brush[2];
		SIZE size;
		int dt;
		BOOL showStat;
		DWORD tflags;

		StdDrawFCurveCallback(Control *cont,ParamDimensionBase *dim,HDC hdc,Rect& rcGraph,
				Rect& rcPaint,float tzoom,int tscroll,float vzoom,int vscroll,DWORD flags, DWORD tflags);
		~StdDrawFCurveCallback();

		BOOL DoSeg(int el,int x0,int y0,int x1, int y1,BOOL inRange);
		BOOL DoKey(int el,int index,BOOL sel,float dval,float kval,int ktime,int x0,int y0,BOOL onTop=FALSE);
	};

StdDrawFCurveCallback::StdDrawFCurveCallback(Control *cont,ParamDimensionBase *dim,HDC hdc,Rect& rcGraph,
		Rect& rcPaint,float tzoom,int tscroll,float vzoom,int vscroll,DWORD flags, DWORD tflags)	
	{
	this->cont=cont;this->dim=dim;this->hdc=hdc;this->rcGraph=rcGraph;
	this->rcPaint=rcPaint;this->tzoom=tzoom;this->tscroll=tscroll;this->tflags=tflags;
	this->vzoom=vzoom;this->vscroll=vscroll;this->flags=flags;
			
		DWORD color = F_COLOR;
//		if (flags&PAINTCURVE_XCOLOR) color = X_COLOR;
//		if (flags&PAINTCURVE_YCOLOR) color = Y_COLOR;
//		if (flags&PAINTCURVE_ZCOLOR) color = Z_COLOR;

		if (flags&PAINTCURVE_FROZEN) {
			LOGBRUSH lb;
			lb.lbStyle = BS_SOLID;
			lb.lbColor = color;
			pen[0] = ExtCreatePen(PS_COSMETIC|PS_ALTERNATE,1,&lb,0,NULL);
		} else {
			pen[0] = CreatePen(PS_SOLID,2,color);
			}
		dpen[0] = CreatePen(PS_DOT,1,color);	// CAL-11/11/2002: change pen width to 1

	brush[0] = ColorMan()->GetBrush(kTrackViewUnSelectedKeys);
	brush[1] = ColorMan()->GetBrush(kTrackViewSelectedKeys);
	bpen  = CreatePen(PS_SOLID,2,ColorMan()->GetColor(kTrackViewKeyOutline));
	otPen = CreatePen(PS_SOLID,2,ColorMan()->GetColor(kTrackViewKeyOutlineOnTop));
	SetBkMode(hdc,TRANSPARENT);	

	SelectObject(hdc,GetFixedFont());
	GetTextExtentPoint(hdc,_T("1234567890"),10,&size);
	SetTextColor(hdc,ColorMan()->GetColor(kTrackViewTrackText));
	dt = int(float(size.cx)/tzoom);
	
	showStat = (tflags&CURVE_SELECTED)&&(flags&PAINTTRACK_SHOWSTATS);
	}

StdDrawFCurveCallback::~StdDrawFCurveCallback()
	{	
	SetBkMode(hdc,OPAQUE);
	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	DeleteObject(otPen);
	DeleteObject(bpen);
	for (int j=0;j<1;j++) {
		DeleteObject(pen[j]);
		DeleteObject(dpen[j]);		
		}
	}

BOOL StdDrawFCurveCallback::DoSeg(int el,int x0,int y0,int x1, int y1,BOOL inRange)
	{
	// CAL-11/11/2002: keep the old pen and restore it at the end
	HPEN oldPen = (HPEN)SelectObject(hdc,inRange?pen[el]:dpen[el]);
	MoveToEx(hdc,x0,y0,NULL);
	LineTo(hdc,x1,y1);
	SelectObject(hdc, oldPen);
	return TRUE;
	}

BOOL StdDrawFCurveCallback::DoKey(
		int el,int index,BOOL sel,float dval,float kval,int ktime,int x0,int y0,BOOL onTop)
	{
	int shift;
	SelectObject(hdc,onTop?otPen:bpen);
	SelectObject(hdc,brush[sel]);
	DrawKeyKnot(hdc,x0,y0);
	if (showStat && sel) {
		TCHAR buf[256];
		TSTR stime;
		shift = dval > kval ? 0 : -size.cy;
		TimeToString(ktime,stime);
		_stprintf(buf,_T("%s, %.3f"),(char *)stime,dim->Convert(kval));
		TextOut(hdc,x0+4,y0+shift,buf,_tcslen(buf));
		}
	return TRUE;
	}



int BoolCntrlKeyTable::PaintFCurves(		
		Control *cont,ParamDimensionBase *dim,HDC hdc,
		Rect& rcGraph,Rect& rcPaint,float tzoom,int tscroll,
		float vzoom,int vscroll,DWORD flags )		
	{
	
		StdDrawFCurveCallback cb(cont,dim,hdc,rcGraph,rcPaint,tzoom,tscroll,vzoom,vscroll,flags,tflags);
		ProcessFCurves(FALSE,&cb,cont,dim,rcGraph,tzoom,tscroll,vzoom,vscroll,flags );
	
	return 0;
	}


int BoolCntrl::PaintFCurves(
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
	// Redirect GetValue() to use original key table
	SetKeyBuffer *savePtr = setkeyBuffer; 
	setkeyBuffer=NULL; 
	ivalid.SetEmpty();

	keys.PaintFCurves(this,dim,hdc,rcGraph,rcPaint,tzoom,tscroll,vzoom,vscroll,flags );

	// Redirect back to setvalBuffer
	setkeyBuffer = savePtr;

	if (setkeyBuffer) {		
		flags |= PAINTCURVE_FROZEN;		
		Interval saveRange = keys.range;
		keys.range.SetEmpty();
		StdControl::PaintFCurves(dim,hdc,rcGraph,rcPaint,tzoom,tscroll,vzoom,vscroll,flags);
		keys.range = saveRange;
		}
	return 0;
	}

//--------- From IKeyControl ----------------------------

void BoolCntrl::SetNumKeys(int n)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	keys.SetCount(n);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}


void BoolCntrl::SortKeys()
	{
	keys.KeysChanged(TRUE);	
	ivalid.SetEmpty();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}


void BoolCntrl::GetKey(int i,IKey *key)
	{
	key->time  = keys[i].time;
	((BKey*)key)->val  = keys[i].val;
//	key->flags = keys[i].kflag;
	}



void BoolCntrl::SetKey(int i,IKey *key)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();

	keys[i].time  = key->time;
	// CAL-06/28/02: restrict the value to be either 0 or 1
	keys[i].val = (((BKey*)key)->val > 0.0f) ? 1.0f : 0.0f;

	keys.KeysChanged(FALSE);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}


int BoolCntrl::AppendKey(IKey *key)
	{
	RevertSetKeyBuffer(TRUE);
	HoldTrack();
	BKey k;
	k.time  = key->time;
	k.val  = (((BKey*)key)->val > 0.0f) ? 1.0f : 0.0f;
	int res = keys.Append(1,&k);
	keys.KeysChanged(FALSE);
	ivalid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	return res;
	}


void* BoolCntrl::GetInterface(ULONG id)
	{
	switch (id) {
		case I_KEYCONTROL:
		return (IKeyControl*)this;
		case I_SETKEYCONTROL:
		        return (ISetKey*)this;
		}
	return Control::GetInterface(id);
	}
	
