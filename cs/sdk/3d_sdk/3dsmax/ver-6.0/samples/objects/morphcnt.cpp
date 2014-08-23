/**********************************************************************
 *<
	FILE: morphcnt.cpp

	DESCRIPTION:  Morph controllers

	CREATED BY: Rolf Berteig

	HISTORY: created 21 August 1995

 *>     Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "prim.h"
#include "tcbgraph.h"

#ifndef NO_CONTROLLER_MORPH	// russom - 10/15/01

//---------------------------------------------------------------

class MorphKeyWindow {
	public:
		HWND hWnd;
		HWND hParent;
		Control *cont;
		MorphKeyWindow() {assert(0);}
		MorphKeyWindow(HWND hWnd,HWND hParent,Control *cont)
			{this->hWnd=hWnd; this->hParent=hParent; this->cont=cont;}
	};
static Tab<MorphKeyWindow> morphKeyWindows;

static void RegisterMorphKeyWindow(HWND hWnd, HWND hParent, Control *cont)
	{
	MorphKeyWindow rec(hWnd,hParent,cont);
	morphKeyWindows.Append(1,&rec);
	}

static void UnRegisterMorphKeyWindow(HWND hWnd)
	{	
	for (int i=0; i<morphKeyWindows.Count(); i++) {
		if (hWnd==morphKeyWindows[i].hWnd) {
			morphKeyWindows.Delete(i,1);
			return;
			}
		}	
	}

static HWND FindOpenMorphKeyWindow(HWND hParent,Control *cont)
	{	
	for (int i=0; i<morphKeyWindows.Count(); i++) {
		if (hParent == morphKeyWindows[i].hParent &&
			cont    == morphKeyWindows[i].cont) {
			return morphKeyWindows[i].hWnd;
			}
		}
	return NULL;
	}

//---------------------------------------------------------------

// WARNING - this class description also exists in maxscrpt\maxkeys.cpp
class MorphTarget {
	public:
		Object *obj;
		TSTR *name;
		Matrix3 tm;
		int refCount;
		MorphTarget(Object *o,TSTR &n,Matrix3 &m) {obj=0;name=new TSTR(n);refCount=1;tm = m;}
		MorphTarget() {obj=NULL;refCount=0;name=NULL;tm=Matrix3(1);}
	};

// WARNING - this class description also exists in maxscrpt\maxkeys.cpp
class MorphTargList : public Tab<MorphTarget> {
	public:
		int AddTarg(Object *obj,TSTR &name,Matrix3 &tm,ReferenceMaker *maker,int offset);
		BOOL DeleteTarg(int i,ReferenceMaker *maker);
		void ForceDeleteTarg(int i,int offset,ReferenceMaker *maker);
		void SetSize(int size);
		~MorphTargList();
	};

class AddMorphTargRestore : public RestoreObj {
	public:         
		ReferenceTarget *mcont;
		MorphTargList *list;
		int index;
		BOOL inc;
		MorphTarget targ;               

		AddMorphTargRestore(ReferenceTarget *mc,MorphTargList *l,int i,BOOL inc) {
			mcont = mc;
			list  = l;
			index = i;
			this->inc = inc;
			if (!inc) {
				targ.obj      = (*list)[index].obj;
				targ.name     = new TSTR(*(*list)[index].name);
				targ.tm       = (*list)[index].tm;
				targ.refCount = (*list)[index].refCount;
				}
			}
		~AddMorphTargRestore() {
			delete targ.name;
			}

		void Restore(int isUndo) {
			if (inc) {
				(*list)[index].refCount--;
			} else {
				delete (*list)[index].name;
				list->Delete(index,1);
				}
			mcont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			mcont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		void Redo() {
			if (inc) {
				(*list)[index].refCount++;
			} else {                                
				list->Insert(index,1,&targ);
				(*list)[index].name = new TSTR(*(*list)[index].name);
				}
			mcont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			mcont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}               
		TSTR Description() { return TSTR(_T("Morp Targ")); }
	};


class DeleteMorphTargRestore : public RestoreObj {
	public:         
		ReferenceTarget *mcont;
		MorphTargList *list;
		int index;		
		MorphTarget targ;               

		DeleteMorphTargRestore(ReferenceTarget *mc,MorphTargList *l,int i) {
			mcont = mc;
			list  = l;
			index = i;			
			targ.obj      = (*list)[index].obj;
			targ.name     = new TSTR(*(*list)[index].name);
			targ.tm       = (*list)[index].tm;
			targ.refCount = (*list)[index].refCount;			
			}
		DeleteMorphTargRestore() {
			delete targ.name;
			}

		void Restore(int isUndo) {
			
			list->Insert(index,1,&targ);
			(*list)[index].name = new TSTR(*(*list)[index].name);			
			
			mcont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			mcont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}
		void Redo() {
			
			delete (*list)[index].name;
			list->Delete(index,1);
			
			mcont->NotifyDependents(FOREVER,0,REFMSG_CHANGE);
			mcont->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
			}               
		TSTR Description() { return TSTR(_T("Morp Targ")); }
	};


MorphTargList::~MorphTargList()
	{
	for (int i=0; i<Count(); i++) {
		delete (*this)[i].name;
		}
	}

int MorphTargList::AddTarg(
		Object *obj,TSTR &name,Matrix3 &tm,
		ReferenceMaker *maker,int offset)
	{
	for (int i=0; i<Count(); i++) {
		if (obj==(*this)[i].obj) {
			(*this)[i].refCount++;
			if (theHold.Holding()) {
				theHold.Put(new AddMorphTargRestore((ReferenceTarget*)maker,this,i,TRUE));
				}
			return i;
			}
		}
	MorphTarget targ(obj,name,tm);
	Append(1,&targ);
	if (theHold.Holding()) {
		theHold.Put(new AddMorphTargRestore((ReferenceTarget*)maker,this,i,FALSE));
		}
	maker->MakeRefByID(FOREVER,i+offset,obj);       
	return i;
	}

BOOL MorphTargList::DeleteTarg(int i,ReferenceMaker *maker)
	{
#if 0
	(*this)[i].refCount--;
	if ((*this)[i].refCount<=0) {
		maker->DeleteReference(i);
		delete (*this)[i].name;
		Delete(i,1);
		return TRUE;
	} else {
		return FALSE;
		}                               
#endif
	return FALSE;
	}

void MorphTargList::ForceDeleteTarg(int i,int offset,ReferenceMaker *maker)
	{	
	DeleteMorphTargRestore *rest = NULL;
	if (theHold.Holding()) {
		rest = 
			new DeleteMorphTargRestore((ReferenceTarget*)maker,this,i);
		}
	maker->DeleteReference(i+offset);
	delete (*this)[i].name;
	Delete(i,1);
	if (theHold.Holding()) {
		theHold.Put(rest);
		}
	}

void MorphTargList::SetSize(int size)
	{
	Resize(size);   
	if (Count() < size) {
		for (int i=Count(); i<size; i++) {
			MorphTarget targ;
			Append(1,&targ);
			}
		}
	}


// Key flags
#define KEY_SELECTED    (1<<0) 
#define KEY_FLAGGED     (1<<1)
#define MULTS_VALID     (1<<2)

// Track flags
#define TRACK_INVALID   (1<<0)
#define RANGE_UNLOCKED  (1<<1)


// WARNING - this class description also exists in maxscrpt\maxkeys.cpp
class MorphKey {
	public:         
		DWORD flags;
		TimeValue time; 

		void SetFlag(DWORD mask) {flags|=(mask);}
		void ClearFlag(DWORD mask) {flags &= ~(mask);}
		BOOL TestFlag(DWORD mask) {return(flags&(mask)?1:0);}
	};

static int __cdecl CompareMorphKeys(const MorphKey *k1, const MorphKey *k2)
	{
	if (k1->time<k2->time) return -1;
	if (k1->time>k2->time) return 1;
	return 0;
	}

/////////////////////////////////////////////////////////////////////
//
// A simple morph controller...

// WARNING - this class description also exists in maxscrpt\maxkeys.cpp
template <class KT>
class GenMorphCont : public MorphControl {
	public:
		MorphTargList targs;
		KT keys;
		ObjectState ob;
		Interval obValid;
		int version, flaggedTarg;
		BOOL lockCache;

		GenMorphCont();
		~GenMorphCont();
		virtual void Update(TimeValue t,TimeValue realT)=0;
		virtual void HoldTrack()=0;
		void FreeCache();
		int FindKey(TimeValue t);
		void GetInterpVal(TimeValue t,int &n0, int &n1, float &u);
		void Invalidate();
		TimeValue ProcessORT(TimeValue t);
		
		// Animatable methods
		SClass_ID SuperClassID() {return CTRL_MORPH_CLASS_ID;}
		int NumKeys() {return keys.Count();}
		TimeValue GetKeyTime(int index) {return keys[index].time;}
		int GetTrackVSpace( int lineHeight ) {return 1;}
		BOOL IsAnimated() {return keys.Count()>=1;}

		void MapKeys(TimeMap *map,DWORD flags );
		void DeleteKeys( DWORD flags );
		void CloneSelectedKeys(BOOL offset);
		void DeleteTime( Interval iv, DWORD flags );
		void ReverseTime( Interval iv, DWORD flags );
		void ScaleTime( Interval iv, float s);
		void InsertTime( TimeValue ins, TimeValue amount );
		void AddNewKey(TimeValue t,DWORD flags);
		int GetSelKeyCoords(TimeValue &t, float &val,DWORD flags);
		void SetSelKeyCoords(TimeValue t, float val,DWORD flags);               
		BOOL CanCopyTrack(Interval iv,DWORD flags) {return 0;} // {return 1;}
		BOOL CanPasteTrack(TrackClipObject *cobj,Interval iv,DWORD flags) {return FALSE;} //{return (cobj && (cobj->ClassID()==ClassID()));}
		TrackClipObject *CopyTrack(Interval iv,DWORD flags);
		void PasteTrack(TrackClipObject *cobj,Interval iv,DWORD flags);
		Interval GetTimeRange(DWORD flags);
		void EditTimeRange(Interval range,DWORD flags);
		int HitTestTrack(TrackHitTab& hits,Rect& rcHit,Rect& rcTrack,float zoom,int scroll,DWORD flags );
		int PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,Rect& rcPaint,float zoom,int scroll,DWORD flags );
		BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
		void SelectKeys(TrackHitTab& sel, DWORD flags);
		void SelectKeyByIndex(int i,BOOL sel);
		BOOL IsKeySelected(int i);
		int NumSelKeys();
		void FlagKey(TrackHitRecord hit);
		int GetFlagKeyIndex();          
		int TrackParamsType() {return TRACKPARAMS_KEY;}
		BOOL SupportTimeOperations() {return TRUE;} // RB 3/26/99: This wasn't implemented (def. returned FALSE)

		// Control methods
		void Copy(Control *from) {}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		BOOL IsLeaf() {return TRUE;}            
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		void MouseCycleCompleted(TimeValue t);
		int GetFlaggedTarget() {return flaggedTarg;}

		// Morph control methods
		int NumMorphTargs() {return targs.Count();}
		Object *GetMorphTarg(int i) {return targs[i].obj;}
		void GetMorphTargName(int i,TSTR &name) {name = *targs[i].name;}
		void SetMorphTargName(int i,TSTR name) {*targs[i].name = name;NotifyDependents(FOREVER,0,REFMSG_NODE_NAMECHANGE);}
		Matrix3 GetMorphTargTM(int i) {return targs[i].tm;}
		BOOL ValidTarget(TimeValue t,Object *obj);
		void DeleteMorphTarg(int i);
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);

		void DeleteThis() {delete this;}

		int NumSubs();
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		
		// Ref methods
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		void MergeAdditionalChannels(Object *from, int branchID);
	};

template <class KT>
class Ver0PostLoad : public PostLoadCallback {
	public:
		GenMorphCont<KT> *cont;
		Ver0PostLoad(GenMorphCont<KT> *c) {cont=c;}
		void proc(ILoad *iload);
	};


// Current version
#define MORPHCONT_VERSION       1


//------------------------------------------------------------------------
//
//  Cubic Spline Morph Controller
//

#define CUBICMORPHCONT_CLASS_ID 0x09923023


class CubicMorphKey : public MorphKey {
	public:         
		int targ;
		float tens, cont, bias;
		float k, l, m, n;
		CubicMorphKey(TimeValue t,int targ) {
			time  = t;
			flags = 0;
			this->targ = targ;
			tens = cont = 0.0f;
			bias = k = l = m = n = 0.0f;
			}
		CubicMorphKey() {
			flags = 0;
			time  = 0;
			targ  = 0;
			tens = cont = 0.0f;
			bias = k = l = m = n = 0.0f;
			}
		
		void Invalidate();
		void ComputeMults();
	};

class CubicMorphKeyTab : public Tab<CubicMorphKey> {
	public:
		DWORD flags;
		Interval range;

		CubicMorphKeyTab() {
			flags = 0;
			}

		void SetFlag(DWORD mask) {flags|=(mask);}
		void ClearFlag(DWORD mask) {flags &= ~(mask);}
		BOOL TestFlag(DWORD mask) {return(flags&(mask)?1:0);}

		void AddNewKey(TimeValue t,int targ,BOOL sel=FALSE) {
			CubicMorphKey key(t,targ);
			if (sel) key.SetFlag(KEY_SELECTED);
			Append(1,&key,5);
			Invalidate();
			}
		
		void Clone(int i) {
			CubicMorphKey key = (*this)[i];
			Append(1,&key,5);
			}
		
		void Invalidate() {
			SetFlag(TRACK_INVALID);
			}

		void CheckForDups() {
			// Check for keys that landed on top of each other
			for (int i=0; i<Count()-1; i++) {
				if ((*this)[i].time==(*this)[i+1].time) {
					if ((*this)[i].TestFlag(KEY_SELECTED)) {
						Delete(i+1,1);
					} else {
						Delete(i,1);						
						}
					i--;
					}
				}                       
			}

		void Update();
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};


class CubicMorphCont : public GenMorphCont<CubicMorphKeyTab> {
	public:                         
		void Update(TimeValue t,TimeValue realT);
		void HoldTrack();
		void CalcFirstCoef(float *v,float *c);
		void CalcLastCoef(float *v,float *c);
		void CalcMiddleCoef(float *v,int *knum,float *c);

		// Animatable methods
		Class_ID ClassID() {return Class_ID(CUBICMORPHCONT_CLASS_ID,0);}  
		void GetClassName(TSTR& s) {s =GetString(IDS_RB_CUBICMORPHCONTROL_CLASS);}
		void EditTrackParams(TimeValue t,ParamDimensionBase *dim,TCHAR *pname,HWND hParent,IObjParam *ip,DWORD flags);

		// Control methods
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		void DeleteMorphTarg(int i);

		RefTargetHandle Clone(RemapDir& remap);
		void Copy(Control *from);
	};

class CubicMorphContClassDesc:public ClassDesc {
	public:
	int                     IsPublic() {return 1;}
	void *                  Create(BOOL loading = FALSE) {return new CubicMorphCont;}
	const TCHAR *   ClassName() { return GetString(IDS_RB_CUBICMORPHCONTROL); }
	SClass_ID               SuperClassID() { return CTRL_MORPH_CLASS_ID; }
	Class_ID                ClassID() { return Class_ID(CUBICMORPHCONT_CLASS_ID,0); }
	const TCHAR*    Category() { return _T("Morph");  }
	};

static CubicMorphContClassDesc cubicMorphContDesc;

ClassDesc* GetCubicMorphContDesc() { return &cubicMorphContDesc; }


class CubicMorphRest : public RestoreObj {
	public:
		CubicMorphKeyTab undo, redo;
		CubicMorphCont *cont;

		CubicMorphRest(CubicMorphCont *c) { 
			cont = c;
			undo = c->keys;
			}
		~CubicMorphRest() {}
		void Restore(int isUndo) {
			if (isUndo) {
				if (redo.Count()!=cont->keys.Count()) {
					redo = cont->keys;
					}
				}
			cont->keys = undo;
			cont->Invalidate();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo() {
			cont->keys = redo;
			cont->Invalidate();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}               
		void EndHold() { 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Morph Restore")); }
	};



//------------------------------------------------------------------------
//
//  Barycentric Morph Controller
//

#define BARYMORPHCONT_CLASS_ID 0x09923022

// WARNING - this class description also exists in maxscrpt\maxkeys.cpp
class TargetWeight {
	public:
		int targ;
		float percent;
		TargetWeight() {targ=0;percent=0.0f;}
		TargetWeight(int t,float p) {targ=t;percent=p;}
	};

// WARNING - this class description also exists in maxscrpt\maxkeys.cpp
class BaryMorphKey : public MorphKey {
	public:         		
		float tens, cont, bias;
		float k, l, m, n;
		Tab<TargetWeight> *wts;

		BaryMorphKey(TimeValue t,int targ) {
			time  = t;
			flags = 0;			
			tens = cont = 0.0f;			
			bias = k = l = m = n = 0.0f;
			wts = new Tab<TargetWeight>;
			TargetWeight wt(targ,1.0f);
			WTS().Append(1,&wt);
			}
		BaryMorphKey() {
			flags = 0;
			time  = 0;			
			tens = cont = 0.0f;
			bias = k = l = m = n = 0.0f;
			wts = new Tab<TargetWeight>;
			}
		
		BaryMorphKey& operator=(BaryMorphKey &src) {
			MorphKey::operator=(src);
			tens = src.tens; cont = src.cont; bias = src.bias;
			k = src.k; l = src.l; m = src.m; n = src.n;
			wts = new Tab<TargetWeight>;
			*wts = *src.wts;
			return *this;
			}
			
		Tab<TargetWeight> &WTS() {return *wts;}
		void Invalidate();
		void ComputeMults();
		void TargetDeleted(int t);
		float GetWeight(int t);
		void SetWeight(int t,float w);
	};

// WARNING - this class description also exists in maxscrpt\maxkeys.cpp
class BaryMorphKeyTab : public Tab<BaryMorphKey> {
	public:
		DWORD flags;
		Interval range;

		BaryMorphKeyTab() {
			flags = 0;
			}

		void SetFlag(DWORD mask) {flags|=(mask);}
		void ClearFlag(DWORD mask) {flags &= ~(mask);}
		BOOL TestFlag(DWORD mask) {return(flags&(mask)?1:0);}

		void AddNewKey(TimeValue t,int targ,BOOL sel=FALSE) {
			BaryMorphKey key(t,targ);
			if (sel) key.SetFlag(KEY_SELECTED);
			Append(1,&key,5);
			Invalidate();
			Update(); // these need to be kept current because a range query could happen before the next call to update (tmb) 
			}
		
		void Clone(int i) {
			BaryMorphKey key;
			key = (*this)[i];
			Append(1,&key,5);
			}
		
		void Invalidate() {
			SetFlag(TRACK_INVALID);
			}

		void CheckForDups() {
			// Check for keys that landed on top of each other
			for (int i=0; i<Count()-1; i++) {
				if ((*this)[i].time==(*this)[i+1].time) {
					if ((*this)[i].TestFlag(KEY_SELECTED)) {
						delete (*this)[i+1].wts;
						Delete(i+1,1);
					} else {
						delete (*this)[i].wts;
						Delete(i,1);						
						}
					i--;
					}
				}                       
			}

		void Update();
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		BaryMorphKeyTab& operator=(BaryMorphKeyTab &src) {
			Tab<BaryMorphKey>::operator=(src);
			for (int i=0; i<Count(); i++) {
				(*this)[i].wts = new Tab<TargetWeight>(*(*this)[i].wts);
				}
			range = src.range;
			flags = src.flags;
			return *this;
			}
	};


// WARNING - this class description also exists in maxscrpt\maxkeys.cpp
class BaryMorphCont : public GenMorphCont<BaryMorphKeyTab> {
	public:                         
		void Update(TimeValue t,TimeValue realT);
		void HoldTrack();
		void CalcFirstCoef(float *v,float *c);
		void CalcLastCoef(float *v,float *c);
		void CalcMiddleCoef(float *v,int *knum,float *c);

		// Animatable methods
		Class_ID ClassID() {return Class_ID(BARYMORPHCONT_CLASS_ID,0);}  
		void GetClassName(TSTR& s) {s =GetString(IDS_RB_BARYMORPHCONTROL_CLASS);}
		void EditTrackParams(TimeValue t,ParamDimensionBase *dim,TCHAR *pname,HWND hParent,IObjParam *ip,DWORD flags);
		void AddNewKey(TimeValue t,DWORD flags);

		// Control methods
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		void DeleteMorphTarg(int i);

		RefTargetHandle Clone(RemapDir& remap);
		void Copy(Control *from);
		
	};

class BaryMorphContClassDesc:public ClassDesc {
	public:
	int             IsPublic() {return 1;}
	void *          Create(BOOL loading = FALSE) {return new BaryMorphCont;}
	const TCHAR *   ClassName() {return GetString(IDS_RB_BARYMORPHCONTROL);}
	SClass_ID       SuperClassID() {return CTRL_MORPH_CLASS_ID;}
	Class_ID        ClassID() {return Class_ID(BARYMORPHCONT_CLASS_ID,0);}
	const TCHAR*    Category() {return _T("Morph");}
	void			ResetClassParams(BOOL fileReset=FALSE);
	};

static BaryMorphContClassDesc baryMorphContDesc;
ClassDesc* GetBaryMorphContDesc() { return &baryMorphContDesc; }

class BaryMorphRest : public RestoreObj {
	public:
		BaryMorphKeyTab undo, redo;
		BaryMorphCont *cont;

		BaryMorphRest(BaryMorphCont *c) { 
			cont = c;
			undo = c->keys;
			}
		~BaryMorphRest() {}
		void Restore(int isUndo) {
			if (isUndo) {
				if (redo.Count()!=cont->keys.Count()) {
					redo = cont->keys;
					}
				}
			cont->keys = undo;
			cont->Invalidate();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo() {
			cont->keys = redo;
			cont->Invalidate();
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}               
		void EndHold() { 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T("Morph Restore")); }
	};


//-----------------------------------------------------------------------

Control *newDefaultMorphControl() 
	{
	if (GetDefaultController(CTRL_MORPH_CLASS_ID)==GetCubicMorphContDesc()) {
		return new CubicMorphCont;		
	} else {
		return new BaryMorphCont;
		}
	}

template <class KT>
GenMorphCont<KT>::GenMorphCont()
	{
	lockCache = FALSE;
	obValid.SetEmpty();     
	version = MORPHCONT_VERSION;
	flaggedTarg = -1;
	}

template <class KT>
GenMorphCont<KT>::~GenMorphCont()
	{
	DeleteAllRefsFromMe();
	}


class FindMorphObjDEP : public DependentEnumProc {
	public:
		Object *morphObj;

		FindMorphObjDEP() {morphObj=NULL;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker->SuperClassID()==GEOMOBJECT_CLASS_ID &&
				rmaker->ClassID()==Class_ID(MORPHOBJ_CLASS_ID,0)) {
				morphObj = (Object*)rmaker;
				return DEP_ENUM_HALT;
				}
			return DEP_ENUM_CONTINUE;
			}
	};

template <class KT>
void GenMorphCont<KT>::MergeAdditionalChannels(
		Object *from, int branchID)
	{
	// RB 10/15/2000: Need to find the morph object referencing us and call 
	//  MergeAdditionalChannels() on it.
	FindMorphObjDEP dep;
	EnumDependents(&dep);
	if (dep.morphObj) {
		dep.morphObj->MergeAdditionalChannels(from, branchID);
		}
	}

template <class KT>
BOOL GenMorphCont<KT>::HasUVW()
	{	
	for (int i=0; i<targs.Count(); i++) {
		if (!targs[i].obj->HasUVW()) return FALSE;
		}
	return TRUE;
	}

template <class KT>
void GenMorphCont<KT>::SetGenUVW(BOOL sw)
	{
	for (int i=0; i<targs.Count(); i++) {
		targs[i].obj->SetGenUVW(sw);
		}
	}

template <class KT>
void GenMorphCont<KT>::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{       
	int index;      
	SetMorphTargetPacket *pckt = (SetMorphTargetPacket*)val;

	index = targs.AddTarg(
		pckt->obj,pckt->name,pckt->tm,this,Control::NumRefs()); 
	if (t!=0 || index!=0 || keys.Count() || pckt->forceCreate) {
		HoldTrack();
		int n = keys.Count();
		for (int i = 0; i < n; i++ ) {
			keys[i].ClearFlag(KEY_SELECTED);
			}
		if (!keys.Count() && t!=0) {
			// Make a key at frame 0
			keys.AddNewKey(0,0);
			}
		keys.AddNewKey(t,index,TRUE);
		keys.CheckForDups();
		}
	obValid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}

template <class KT>
void GenMorphCont<KT>::MouseCycleCompleted(TimeValue t)
	{
	keys.CheckForDups();
	}

template <class KT>
BOOL GenMorphCont<KT>::ValidTarget(TimeValue t,Object *obj)
	{
	// Make sure it doesn't depend on us
	obj->BeginDependencyTest();
	NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (obj->EndDependencyTest()) return FALSE;

	Update(t,t);
	ObjectState os = obj->Eval(t);
	if (os.obj->IsParticleSystem()) {
		return FALSE;
	} else if (os.obj->IsDeformable()) {
		if (ob.obj && ob.obj->NumPoints()==os.obj->NumPoints()) return TRUE;
		else if (!targs.Count()) return TRUE;
	} else {
		if (os.obj->CanConvertToType(defObjectClassID)) {
			if (!targs.Count()) return TRUE;
			Object *obj = os.obj->ConvertToType(t,defObjectClassID);
			assert(obj);
			BOOL res = ob.obj->NumPoints()==obj->NumPoints();
			if (obj!=os.obj) obj->DeleteThis();
			return res;
			}
		}
	return FALSE;
	}

template <class KT>
void GenMorphCont<KT>::DeleteMorphTarg(int i)
	{	
	targs.ForceDeleteTarg(i,Control::NumRefs(),this);
	obValid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}

template <class KT>
void GenMorphCont<KT>::FreeCache()
	{
	if (ob.obj && !lockCache) {
		ob.obj->UnlockChannels(GEOM_CHANNEL);
		ob.obj->UnlockObject(); 
		ob.DeleteObj();
		}
	}

template <class KT>
int GenMorphCont<KT>::FindKey(TimeValue t)
	{
	for (int i=0; i<keys.Count(); i++) {
		if (keys[i].time >= t) {
			if (keys[i].time==t && i==0) {
				return i;
			} else {
				return i-1;
				}
			}
		}
	return keys.Count()-1;
	}

template <class KT>
void GenMorphCont<KT>::GetInterpVal(TimeValue t,int &n0, int &n1, float &u)
	{
	int i = FindKey(t);
	if (i<0) {
		n0 = n1 = -1;
		u  = 0.0f;
		return;
		}
	if (i==keys.Count()-1) {
		n0 = n1 = i;
		u  = 0.0f;
		return;
		}
	n0 = i;
	n1 = i+1;
	u = float(t-keys[i].time) / float(keys[i+1].time - keys[i].time);
	}


template <class KT>
RefResult GenMorphCont<KT>::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message) 
	{
	switch (message) {
		case REFMSG_SELECT_BRANCH: {
			for (int i=0; i<targs.Count(); i++) {
				if (targs[i].obj==hTarget) {
					flaggedTarg = i;
					break;
					}
				}
			break;
			}

		case REFMSG_OBJECT_CACHE_DUMPED:
		case REFMSG_CHANGE:
			obValid.SetEmpty();
			FreeCache();
			break;
		}
	return REF_SUCCEED;
	}

template <class KT>
void GenMorphCont<KT>::Invalidate()
	{
	obValid.SetEmpty();
	}


template <class KT>
int GenMorphCont<KT>::NumSubs() 
	{
	return Control::NumSubs();
	}

template <class KT>
Animatable* GenMorphCont<KT>::SubAnim(int i) 
	{
	return Control::SubAnim(i);
	}

template <class KT>
TSTR GenMorphCont<KT>::SubAnimName(int i) 
	{
	return Control::SubAnimName(i);
	}


template <class KT>
int GenMorphCont<KT>::NumRefs() 
	{
	if (version) {
		return Control::NumRefs() + targs.Count();
	} else {
		return targs.Count();
		}
	}


template <class KT>
RefTargetHandle GenMorphCont<KT>::GetReference(int i) 
	{
	if (version) {
		if (i<Control::NumRefs()) {
			return Control::GetReference(i);
		} else {
			i -= Control::NumRefs();
			if (i<targs.Count()) {
				return targs[i].obj;
			} else {
				return NULL;
				}       
			}
	} else {
		if (i<targs.Count()) {
			return targs[i].obj;
		} else {
			return NULL;
			}       
		} 
	}

template <class KT>
void GenMorphCont<KT>::SetReference(int i, RefTargetHandle rtarg) 
	{
	if (version) {
		if (i<Control::NumRefs()) {
			Control::SetReference(i,rtarg);
		} else {                        
			targs[i-Control::NumRefs()].obj = (Object*)rtarg;
			}
	} else {                
		targs[i].obj = (Object*)rtarg;                  
		}
	}

template <class KT>
void Ver0PostLoad<KT>::proc(ILoad *iload)
	{
	cont->version = MORPHCONT_VERSION;
	delete this;
	}

#define TARGCOUNT_CHUNK                 0x01001
#define MORPHKEYS_CHUNK                 0x01002
#define MORPH_INORT_CHUNK               0x01003
#define MORPH_OUTORT_CHUNK              0x01004
#define MORPH_TARGNAME_CHUNK    0x01005
#define MORPH_VERSION_CHUNK             0x01006
#define MORPH_TARGTM_CHUNK              0x01007

template <class KT>
IOResult GenMorphCont<KT>::Save(ISave *isave)
	{
	ULONG nb;
	int ort;
	int ct = targs.Count();
	int ver = MORPHCONT_VERSION;

	isave->BeginChunk(MORPH_VERSION_CHUNK);
	isave->Write(&ver,sizeof(ver),&nb);
	isave->EndChunk();

	isave->BeginChunk(TARGCOUNT_CHUNK);
	isave->Write(&ct,sizeof(ct),&nb);
	isave->EndChunk();

	for (int i=0; i<targs.Count(); i++) {
		isave->BeginChunk(MORPH_TARGNAME_CHUNK);
		isave->WriteWString((TCHAR*)(*targs[i].name));
		isave->EndChunk();

		isave->BeginChunk(MORPH_TARGTM_CHUNK);
		targs[i].tm.Save(isave);
		isave->EndChunk();
		}

	ort = GetORT(ORT_BEFORE);
	isave->BeginChunk(MORPH_INORT_CHUNK);
	isave->Write(&ort,sizeof(ort),&nb);                     
	isave->EndChunk();

	ort = GetORT(ORT_AFTER);
	isave->BeginChunk(MORPH_OUTORT_CHUNK);
	isave->Write(&ort,sizeof(ort),&nb);                     
	isave->EndChunk();

	isave->BeginChunk(MORPHKEYS_CHUNK);
	keys.Save(isave);
	isave->EndChunk();

	return IO_OK;
	}

template <class KT>
IOResult GenMorphCont<KT>::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res;
	int ort, ct, ni=0, ti=0;        

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case MORPH_VERSION_CHUNK:
				res=iload->Read(&version,sizeof(version),&nb);
				break;

			case TARGCOUNT_CHUNK:
				res=iload->Read(&ct,sizeof(ct),&nb);
				targs.SetSize(ct);
				break;

			case MORPH_TARGNAME_CHUNK: {
				TCHAR *buf;
				res=iload->ReadWStringChunk(&buf);
				targs[ni++].name = new TSTR(buf);
				break;
				}
			
			case MORPH_TARGTM_CHUNK:
				res=targs[ti++].tm.Load(iload);
				break;                          

			case MORPH_INORT_CHUNK:
				res=iload->Read(&ort,sizeof(ort),&nb);
				SetORT(ort,ORT_BEFORE);
				break;

			case MORPH_OUTORT_CHUNK:
				res=iload->Read(&ort,sizeof(ort),&nb);
				SetORT(ort,ORT_AFTER);
				break;

			case MORPHKEYS_CHUNK:
				res=keys.Load(iload);
				break;
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	keys.Invalidate();
	Invalidate();
	
	// Fix up old versions
	switch (version) {
		case 0:
			iload->RegisterPostLoadCallback(new Ver0PostLoad<KT>(this));
			iload->SetObsolete();
			break;
		}

	return IO_OK;
	}

//
// Track view key stuff
//

template <class KT>
void GenMorphCont<KT>::MapKeys(TimeMap *map,DWORD flags )
	{
	int n = keys.Count();
	BOOL changed = FALSE;
	if (!n) return;
	HoldTrack();

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
		for (int i = start; i!=end; i+=inc) {
			if (keys[i].TestFlag(KEY_SELECTED)) {                   
				prev = keys[i].time;
				keys[i].time = map->map(keys[i].time);
				delta = keys[i].time - prev;
				changed = TRUE;
			} else if (slide) {
				keys[i].time += delta;
				}
			}
		}
	if (flags&TRACK_MAPRANGE && keys.TestFlag(RANGE_UNLOCKED)) {
		TimeValue t0 = map->map(keys.range.Start());
		TimeValue t1 = map->map(keys.range.End());
		keys.range.Set(t0,t1);
		}

	if (changed) {
		keys.Invalidate();
		Invalidate();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

template <class KT>
void GenMorphCont<KT>::DeleteKeys(DWORD flags)
	{
	int n = keys.Count();           
	HoldTrack();
	
	for ( int i = n-1; i >= 0; i-- ) {
		if (flags&TRACK_DOALL || keys[i].TestFlag(KEY_SELECTED)) {
			keys.Delete(i,1);                       
			}
		}
	keys.Invalidate();
	Invalidate();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

template <class KT>
void GenMorphCont<KT>::CloneSelectedKeys(BOOL offset)
	{
	int n = keys.Count();                   
	HoldTrack();
	BOOL changed = FALSE;

	for (int i = 0; i < n; i++) {
		if (keys[i].TestFlag(KEY_SELECTED)) {                   
			keys.Clone(i);
			keys[i].ClearFlag(KEY_SELECTED);
			changed = TRUE;
			}
		}
	if (changed) {          
		keys.Invalidate();
		}
	}

template <class KT>
void GenMorphCont<KT>::DeleteTime( Interval iv, DWORD flags )
	{
	Interval test = TestInterval(iv,flags);
	int n = keys.Count();   
	int d = iv.Duration()-1;
	if (d<0) d = 0;
	HoldTrack();

	for (int i = n-1; i >= 0; i--) {
		if (test.InInterval(keys[i].time) ) {
			keys.Delete(i,1);                       
		} else 
		if (!(flags&TIME_NOSLIDE)) {
			if (keys[i].time > test.End()) {
				keys[i].time -= d;
				}
			}
		}                   
	keys.Invalidate();
	Invalidate();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

template <class KT>
void GenMorphCont<KT>::ReverseTime( Interval iv, DWORD flags )
	{
	Interval test = TestInterval(iv,flags);
	int n = keys.Count();
	HoldTrack();

	for (int i = 0; i < n; i++) {
		if (test.InInterval(keys[i].time)) {
			TimeValue delta = keys[i].time - iv.Start();
			keys[i].time = iv.End()-delta;                  
			}
		}
	
	keys.Invalidate();
	Invalidate();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

template <class KT>
void GenMorphCont<KT>::ScaleTime( Interval iv, float s)
	{
	int n = keys.Count();
	TimeValue delta = int(s*float(iv.End()-iv.Start())) + iv.Start()-iv.End();
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
	
	keys.Invalidate();
	Invalidate();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

template <class KT>
void GenMorphCont<KT>::InsertTime( TimeValue ins, TimeValue amount )
	{
	int n = keys.Count();           
	HoldTrack();

	for (int i = 0; i < n; i++) {
		if ( keys[i].time >= ins ) {
			keys[i].time += amount;
			}               
		}
	
	keys.Invalidate();
	Invalidate();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

template <class KT>
void GenMorphCont<KT>::AddNewKey(TimeValue t,DWORD flags)
	{

	}


template <class KT>
int GenMorphCont<KT>::GetSelKeyCoords(TimeValue &t, float &val,DWORD flags)
	{
	TimeValue atime;
	int n = keys.Count();
	BOOL tfound = FALSE;
	for (int i=0; i<n; i++) {
		if (keys[i].TestFlag(KEY_SELECTED)) {
			if (tfound) {
				if (keys[i].time!=atime) {
					return KEYS_MULTISELECTED;
					}
			} else {
				tfound = TRUE;
				atime = keys[i].time;
				}
			}
		}	
	if (tfound) {
		t = atime;
		return KEYS_COMMONTIME;
	} else {
		return KEYS_NONESELECTED;
		}
	}

template <class KT>
void GenMorphCont<KT>::SetSelKeyCoords(TimeValue t, float val,DWORD flags)
	{
	BOOL changed = FALSE;
	if (flags&KEYCOORDS_VALUEONLY) return;
	int n = keys.Count();
	for (int i=0; i<n; i++) {
		if (keys[i].TestFlag(KEY_SELECTED)) {
			keys[i].time = t;
			changed = TRUE;
			}
		}
	if (changed) {
		keys.Invalidate();
		Invalidate();
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}

template <class KT>
TrackClipObject *GenMorphCont<KT>::CopyTrack(Interval iv,DWORD flags)
	{
	return NULL;
	}

template <class KT>
void GenMorphCont<KT>::PasteTrack(
		TrackClipObject *cobj,Interval iv,DWORD flags)
	{

	}

template <class KT>
Interval GenMorphCont<KT>::GetTimeRange(DWORD flags)
	{
	keys.Update();
	return keys.range;
	}

template <class KT>
void GenMorphCont<KT>::EditTimeRange(Interval range,DWORD flags)
	{
	HoldTrack();
	if (flags&EDITRANGE_LINKTOKEYS) {
		keys.ClearFlag(RANGE_UNLOCKED);         
	} else {
		keys.range = range;
		keys.SetFlag(RANGE_UNLOCKED);
		}       
	Invalidate();
	keys.Invalidate();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

#define HSIZE 4
#define VSIZE 5
inline void PaintKey(HDC hdc, int x, int y) {
	Ellipse(hdc,x-HSIZE,y-VSIZE,x+HSIZE,y+VSIZE);
	}

template <class KT>
int GenMorphCont<KT>::HitTestTrack(
		TrackHitTab& hits,Rect& rcHit,Rect& rcTrack,
		float zoom,int scroll,DWORD flags )
	{
	int left  = ScreenToTime(rcTrack.left,zoom,scroll) - HSIZE;
	int right = ScreenToTime(rcTrack.right,zoom,scroll) + HSIZE;
	int n = keys.Count();
	int y = (rcTrack.top+rcTrack.bottom)/2; 
		
	for ( int i = 0; i < n; i++ ) {
		if (flags&HITTRACK_SELONLY && 
			!keys[i].TestFlag(KEY_SELECTED)) continue;
		if (flags&HITTRACK_UNSELONLY && 
			keys[i].TestFlag(KEY_SELECTED)) continue;

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

template <class KT>
int GenMorphCont<KT>::PaintTrack(
		ParamDimensionBase *dim,
		HDC hdc,Rect& rcTrack,Rect& rcPaint,
		float zoom,int scroll,DWORD flags )
	{
	int left  = ScreenToTime(rcPaint.left-2*HSIZE,zoom,scroll);
	int right = ScreenToTime(rcPaint.right+2*HSIZE,zoom,scroll);
	int n = keys.Count();
	int y = (rcTrack.top+rcTrack.bottom)/2;
	HBRUSH selBrush = CreateSolidBrush(RGB(255,255,255));
	HBRUSH unselBrush = (HBRUSH)GetStockObject(GRAY_BRUSH); 

	SelectObject(hdc,GetStockObject(BLACK_PEN));    

	for ( int i = 0; i < n; i++ ) {
		if (keys[i].time > right) {
			break;
			}
		if (keys[i].time > left) {
			int x = TimeToScreen(keys[i].time,zoom,scroll);
			if ( (flags&PAINTTRACK_SHOWSEL) &&
			     keys[i].TestFlag(KEY_SELECTED)) {
				SelectObject(hdc,selBrush);
			} else {
				SelectObject(hdc,unselBrush);
				}
			PaintKey(hdc,x,y);
			}               
		}
	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	DeleteObject(selBrush);
	
	return TRACK_DONE;
	}

template <class KT>
void GenMorphCont<KT>::SelectKeyByIndex(int i,BOOL sel)
	{
	HoldTrack();
	if (sel) keys[i].SetFlag(KEY_SELECTED);
	else     keys[i].ClearFlag(KEY_SELECTED);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

template <class KT>
BOOL GenMorphCont<KT>::IsKeySelected(int i)
	{
	return keys[i].TestFlag(KEY_SELECTED);
	}

template <class KT>
BOOL GenMorphCont<KT>::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
	{
	if (!keys.Count()) return FALSE;
	if (flags&NEXTKEY_RIGHT) {		
		for (int i=0; i<keys.Count(); i++) {
			if (keys[i].time > t ) {
				nt = keys[i].time;
				return TRUE;
				}
			}
		nt = keys[0].time;
		return TRUE;
	} else {
		for (int i=keys.Count()-1; i>=0; i--) {
			if (keys[i].time < t ) {
				nt = keys[i].time;
				return TRUE;
				}
			}
		nt = keys[keys.Count()-1].time;
		return TRUE;
		}
	}

template <class KT>
void GenMorphCont<KT>::SelectKeys(TrackHitTab& sel, DWORD flags)
	{
	HoldTrack();    

	if (flags&SELKEYS_CLEARKEYS) {
		int n = keys.Count();
		for (int i = 0; i < n; i++ ) {
			keys[i].ClearFlag(KEY_SELECTED);
			}
		}
	
	if (flags&SELKEYS_DESELECT) {
		for (int i = 0; i < sel.Count(); i++) {
			keys[sel[i].hit].ClearFlag(KEY_SELECTED);
			}               
		}       
	if (flags&SELKEYS_SELECT) {                     
		for (int i = 0; i < sel.Count(); i++ ) {
			keys[sel[i].hit].SetFlag(KEY_SELECTED);
			}
		}       
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

template <class KT>
int GenMorphCont<KT>::NumSelKeys()
	{
	int n = keys.Count();
	int c = 0;
	for ( int i = 0; i < n; i++ ) {
		if (keys[i].TestFlag(KEY_SELECTED)) {
			c++;
			}
		}
	return c;
	}

template <class KT>
void GenMorphCont<KT>::FlagKey(TrackHitRecord hit)
	{
	int n = keys.Count();
	for ( int i = 0; i < n; i++ ) {
		keys[i].ClearFlag(KEY_FLAGGED);
		}
	assert(hit.hit>=0&&hit.hit<(DWORD)n);
	keys[hit.hit].SetFlag(KEY_FLAGGED);
	}

template <class KT>
int GenMorphCont<KT>::GetFlagKeyIndex()
	{
	int n = keys.Count();
	for ( int i = 0; i < n; i++ ) {
		if (keys[i].TestFlag(KEY_FLAGGED)) {
			return i;
			}
		}
	return -1;
	}

template <class KT>
TimeValue GenMorphCont<KT>::ProcessORT(TimeValue t)
	{
	int ort;
	
	if (t <= keys.range.Start()) {
		ort = GetORT(ORT_BEFORE);
	} else {
		ort = GetORT(ORT_AFTER);
		}
	
	if (ort==ORT_CONSTANT || keys.range.Empty() || keys.range.InInterval(t)) {      
		return t;
	} else {                
		switch (ort) {
			case ORT_IDENTITY:
			case ORT_LINEAR:
			case ORT_CONSTANT:                      
				if (t<keys.range.Start()) {
					return keys.range.Start();                                      
				} else {
					return keys.range.End();
					}
				break;

			case ORT_RELATIVE_REPEAT:
			case ORT_LOOP:
			case ORT_CYCLE:                                         
				return CycleTime(keys.range,t);                         

			case ORT_OSCILLATE: {
				int cycles = NumCycles(keys.range,t);
				TimeValue tp = CycleTime(keys.range,t);                         
				if (cycles&1) {
					tp = keys.range.End()-(tp-keys.range.Start());
					} 
				return tp;                              
				}                       
			}
		}
	
	return t;
	}

//------------------------------------------------------------------

static void ComputeHermiteBasis(float u, float *v) 
	{
	float u2,u3,a;
	
	u2 = u*u;
	u3 = u2*u;
	a  = 2.0f*u3 - 3.0f*u2;
	v[0] = 1.0f + a;
	v[1] = -a;
	v[2] = u - 2.0f*u2 + u3;
	v[3] = -u2 + u3;
	}

void CubicMorphCont::CalcFirstCoef(float *v,float *c)
	{
	float G, H, J;

	H = .5f*(1.0f-keys[0].tens)*v[2];
	J = 3.0f*H;
	G = v[3] - H;
	c[0] = v[0] - J - G * keys[1].k;
	c[1] = v[1] + J + G*(keys[1].k-keys[1].l);
	c[2] = G*keys[1].l;
	}

void CubicMorphCont::CalcLastCoef(float *v,float *c)
	{
	int nkeys = keys.Count();
	float G, H, J;

	H = .5f*(1.0f-keys[nkeys-1].tens)*v[3];
	J = 3.0f*H;
	G = v[2]-H;
	c[0] = -G*keys[nkeys-2].m;
	c[1] = v[0] - J + G*(keys[nkeys-2].m-keys[nkeys-2].n);
	c[2] = v[1] + J + G*keys[nkeys-2].n;
	}

void CubicMorphCont::CalcMiddleCoef(float *v,int *knum,float *c)
	{
	c[0] = -v[2]*keys[knum[1]].m;
	c[1] = v[0] + v[2]*(keys[knum[1]].m-keys[knum[1]].n) - v[3]*keys[knum[2]].k;
	c[2] = v[1] + v[2]*keys[knum[1]].n + v[3]*(keys[knum[2]].k-keys[knum[2]].l);
	c[3] = v[3]*keys[knum[2]].l;
	}

void CubicMorphCont::HoldTrack()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new CubicMorphRest(this));
		}
	}

void CubicMorphCont::Update(TimeValue t,TimeValue realT)
	{
	// RB 7/18/2000: obValid used to key off of 't' rather than 'realT'. I think
	// it should work off of realT (note that realT is used to update the
	// validity interval at the end of this function).
	//
	if (obValid.InInterval(realT)) return;      
	obValid.SetInstant(realT);
	
	// Whenever we dump the cache we need to make sure anyone
	// downstream knows about it.
	FreeCache();    
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);       

	if (!targs.Count()) {

		// Build a standin object.              
		ob = ObjectState(CreateNewTriObject());
		ob.obj->LockChannels(ALL_CHANNELS);
		ob.obj->LockObject();
		obValid = FOREVER;
		return;         
		}

	lockCache = TRUE;

	Object *obj[4] = {NULL,NULL,NULL,NULL};
	Matrix3 tms[4] = {Matrix3(1),Matrix3(1),Matrix3(1),Matrix3(1)};
	float v[4] = {0.0f,0.0f,0.0f,0.0f}, c[4] = {0.0f,0.0f,0.0f,0.0f};
	int n0, n1, knum[4], nKeys = keys.Count(), nobs;
	float u;
	
	keys.Update();  
	GetInterpVal(t,n0,n1,u);
	
	ob = targs[0].obj->Eval(t);
	MergeAdditionalChannels(ob.obj,0);

	if (nKeys==1 || n0<0 || n1<0) {
		//ob = targs[0].obj->Eval(t);
		nobs = 1;
		c[0]   = 1.0f;
		obj[0] = ob.obj;
		tms[0] = targs[0].tm;
	} else  
	if (n0==n1 || u==0.0f) {
		ObjectState os = targs[keys[n0].targ].obj->Eval(t);
		MergeAdditionalChannels(os.obj,keys[n0].targ);

		//ob = targs[keys[n0].targ].obj->Eval(t);
		nobs = 1;
		//obj[0] = ob.obj;
		c[0]   = 1.0f;
		obj[0] = os.obj;
		tms[0] = targs[keys[n0].targ].tm;
	} else 
	if (nKeys==2) {
		//ob = targs[keys[n0].targ].obj->Eval(t);
		ObjectState os1 = targs[keys[n0].targ].obj->Eval(t);
		ObjectState os2 = targs[keys[n1].targ].obj->Eval(t);
		MergeAdditionalChannels(os1.obj,keys[n0].targ);
		MergeAdditionalChannels(os2.obj,keys[n1].targ);

		//obj[0] = ob.obj;
		obj[0] = os1.obj;
		obj[1] = os2.obj;
		c[0]   = 1.0f-u;
		c[1]   = u;
		nobs   = 2;
		tms[0] = targs[keys[n0].targ].tm;
		tms[1] = targs[keys[n1].targ].tm;
	} else {
		//ob = targs[keys[n0].targ].obj->Eval(t);

		ComputeHermiteBasis(u,v);

		if (FALSE/*cycle*/) {
			
		} else {
			if (n0==0) {                            
				ObjectState os0 = targs[keys[n0].targ].obj->Eval(t);
				ObjectState os1 = targs[keys[n1].targ].obj->Eval(t);            
				ObjectState os2 = targs[keys[n1+1].targ].obj->Eval(t);
				MergeAdditionalChannels(os0.obj,keys[n0].targ);
				MergeAdditionalChannels(os1.obj,keys[n1].targ);
				MergeAdditionalChannels(os2.obj,keys[n1+1].targ);

				//obj[0] = ob.obj;
				obj[0] = os0.obj;
				obj[1] = os1.obj;
				obj[2] = os2.obj;
				CalcFirstCoef(v,c);
				tms[0] = targs[keys[n0].targ].tm;
				tms[1] = targs[keys[n1].targ].tm;
				tms[2] = targs[keys[n1+1].targ].tm;
				nobs = 3;
			} else
			if (n1==nKeys-1) {                                                              
				ObjectState os0 = targs[keys[n0-1].targ].obj->Eval(t);          
				ObjectState os1 = targs[keys[n0].targ].obj->Eval(t);
				ObjectState os2 = targs[keys[n1].targ].obj->Eval(t);
				MergeAdditionalChannels(os0.obj,keys[n0-1].targ);
				MergeAdditionalChannels(os1.obj,keys[n0].targ);
				MergeAdditionalChannels(os2.obj,keys[n1].targ);

				obj[0] = os0.obj;
				//obj[1] = ob.obj;
				obj[1] = os1.obj;
				obj[2] = os2.obj;
				CalcLastCoef(v,c);
				tms[0] = targs[keys[n0-1].targ].tm;
				tms[1] = targs[keys[n0].targ].tm;
				tms[2] = targs[keys[n1].targ].tm;
				nobs = 3;
			} else {                                
				ObjectState os0 = targs[keys[n0-1].targ].obj->Eval(t);
				ObjectState os1 = targs[keys[n0].targ].obj->Eval(t);
				ObjectState os2 = targs[keys[n1].targ].obj->Eval(t);                                            
				ObjectState os3 = targs[keys[n1+1].targ].obj->Eval(t);
				MergeAdditionalChannels(os0.obj,keys[n0-1].targ);
				MergeAdditionalChannels(os1.obj,keys[n0].targ);
				MergeAdditionalChannels(os2.obj,keys[n1].targ);
				MergeAdditionalChannels(os3.obj,keys[n1+1].targ);

				obj[0]  = os0.obj;
				//obj[1]  = ob.obj;
				obj[1]  = os1.obj;
				obj[2]  = os2.obj;
				obj[3]  = os3.obj;
				knum[0] = n0-1;
				knum[1] = n0;
				knum[2] = n1;
				knum[3] = n1+1;
				CalcMiddleCoef(v,knum,c);
				tms[0] = targs[keys[n0-1].targ].tm;
				tms[1] = targs[keys[n0].targ].tm;
				tms[2] = targs[keys[n1].targ].tm;
				tms[3] = targs[keys[n1+1].targ].tm;
				nobs = 4;
				}
			}
		}
			
	// Make sure all objects are deformable
	BOOL del[4] = {FALSE,FALSE,FALSE,FALSE};        
	for (int i=0; i<nobs; i++) {            
		if (!obj[i]->IsDeformable()) {
			Object *o = obj[i]->ConvertToType(t,defObjectClassID);
			assert(o);           
			
			// RB:6/20/96: If ConvertToType() actually returned something
			// new then it should be unlocked.
			if (o!=obj[i]) {
				
				// RB 11/21/96: This looked wrong!
				//ob.obj->UnlockObject();
				o->UnlockObject();
				}

			if (obj[i]==ob.obj) {
				// This is the one we're gonna hang on to, don't delete it.
				ob.obj = o;
				obj[i] = o;
			} else {
				if (o!=obj[i]) {
					// ConvertToType() created a new object. Make sure we delete it.
					del[i] = TRUE;
					obj[i] = o;
					}
				}
			}
		}

	// Make sure the main object is deformable
	if (!ob.obj->IsDeformable()) {
		Object *oldObj = ob.obj;
		ob.obj = ob.obj->ConvertToType(t,defObjectClassID);
		assert(ob.obj);
		
		// RB:6/20/96: If ConvertToType() actually returned something
		// new then it should be unlocked.
		if (oldObj!=ob.obj) {
			ob.obj->UnlockObject();
			}
		}

	// Prepare the geom channel to be modified
	if (ob.obj->IsObjectLocked()) {
		ob.obj = ob.obj->MakeShallowCopy(OBJ_CHANNELS);
		
		// RB 6/20/96 These next two lines didn't used to be inside this
		// if statement, however we only want to lock the object channels
		// if we made a shallow copy.
		ob.obj->LockChannels(OBJ_CHANNELS);
		ob.obj->UnlockObject();
		}	
	ob.obj->ReadyChannelsForMod(GEOM_CHANNEL);              
	
	// Blend the objects
	if (nobs>0) {
		// Find the object with the least number of points.
		int pts = ob.obj->NumPoints();
		for (int i=0; i<nobs; i++) {
			if (obj[i]->NumPoints()<pts) pts = obj[i]->NumPoints();
			}
		for (i=0; i<pts; i++) {
			Point3 p = (obj[0]->GetPoint(i)*tms[0]) * c[0];
			for (int j=1; j<nobs; j++) {
				p += (obj[j]->GetPoint(i)*tms[j]) * c[j];
				}
			ob.obj->SetPoint(i,p);
			if (ob.obj->HasWeights()) {
				double w = obj[0]->GetWeight(i) * c[0];
				for (int j=1; j<nobs; j++) {
					w += (obj[j]->GetWeight(i)) * c[j];
					}
				ob.obj->SetWeight(i, w);
				}
			}
		ob.obj->PointsWereChanged();
	} else {
		// Just transform the object
		for (int i=0; i<ob.obj->NumPoints(); i++) {
			ob.obj->SetPoint(i,ob.obj->GetPoint(i)*tms[0]);
			}
		}
	
	// Delete any objects that were created from ConvertToType()    
	for (i=0; i<nobs; i++) {
		if (del[i]) obj[i]->DeleteThis();
		}

	// Lock it since I'm going to hold on to it and return it.
	ob.obj->LockChannels(GEOM_CHANNEL);
	ob.obj->UpdateValidity(GEOM_CHAN_NUM,Interval(realT,realT));
	ob.obj->LockObject();
	lockCache = FALSE;
	}

void CubicMorphCont::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	TimeValue rt = t;
	Interval evalid;
	t = ApplyEase(t,evalid);
	t = ProcessORT(t);

	// Limit to in range
	if (NumKeys() > 1)
		{
		if (t<keys.range.Start()) 
			t = keys.range.Start();
		if (t>keys.range.End()) 
			t = keys.range.End();	
		}

	Update(t,rt);
	ObjectState *os = (ObjectState*)val;
	*os = ob;               
	}

void CubicMorphCont::DeleteMorphTarg(int i)
	{
	HoldTrack();
	for (int j=keys.Count()-1; j>=0; j--) {
		if (keys[j].targ==i) {
			keys.Delete(j,1);
		} else
		if (keys[j].targ>i) {
			keys[j].targ--;
			}
		}
	FreeCache();    
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GenMorphCont<CubicMorphKeyTab>::DeleteMorphTarg(i);
	}


//------------------------------------------------------------------

#define MORPHKEY_TABLE_CHUNK    0x02001
#define MORPHKEY_FLAGS_CHUNK    0x02002
#define MORPHKEY_RANGE_CHUNK    0x02003

	
void CubicMorphKey::Invalidate()
	{
	ClearFlag(MULTS_VALID); 
	}

void CubicMorphKey::ComputeMults()
	{
	if (!TestFlag(MULTS_VALID)) {
		float tm,cm,cp,bm,bp,tmcm,tmcp;
		tm = 0.5f*(1.0f - tens);
		cm = 1.0f - cont;       
		cp = 2.0f - cm;
		bm = 1.0f - bias;
		bp = 2.0f - bm;      
		tmcm = tm*cm;   tmcp = tm*cp;   
		k = tmcm*bp;    l = tmcp*bm;
		m = tmcp*bp;    n = tmcm*bm;
		SetFlag(MULTS_VALID);
		}
	}

void CubicMorphKeyTab::Update() 
	{
	if (TestFlag(TRACK_INVALID)) {
		ClearFlag(TRACK_INVALID);
		
		// Sort the array by time and shrink
		Sort((CompareFnc)CompareMorphKeys);
		CheckForDups(); 
		Shrink();
		
		// Update the range
		if (!TestFlag(RANGE_UNLOCKED)) {
			if (Count()) {
				range.Set((*this)[0].time,(*this)[Count()-1].time);
			} else {
				range.SetEmpty();
				}
			}
		
		// Compute the multipliers
		for (int i=0; i<Count(); i++) {
			(*this)[i].ComputeMults();
			}
		}
	}

IOResult CubicMorphKeyTab::Save(ISave *isave)
	{
	ULONG nb;

	if (Count()>0) {
		isave->BeginChunk(MORPHKEY_TABLE_CHUNK);
		isave->Write(Addr(0), sizeof(CubicMorphKey)*Count(), &nb);
		isave->EndChunk();
		}
	
	isave->BeginChunk(MORPHKEY_FLAGS_CHUNK);
	isave->Write(&flags, sizeof(flags), &nb);
	isave->EndChunk();

	isave->BeginChunk(MORPHKEY_RANGE_CHUNK);
	isave->Write(&range, sizeof(range), &nb);
	isave->EndChunk();
	
	return IO_OK;
	}

IOResult CubicMorphKeyTab::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;

	Resize(0);
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case MORPHKEY_TABLE_CHUNK: {
				int nkeys = (iload->CurChunkLength())/sizeof(CubicMorphKey);
				SetCount(nkeys);
				if (nkeys>0) 
					res = iload->Read(Addr(0),nkeys*sizeof(CubicMorphKey),&nb);
				break;
				}

			case MORPHKEY_FLAGS_CHUNK:
				res=iload->Read(&flags,sizeof(flags),&nb);
				break;

			case MORPHKEY_RANGE_CHUNK:
				res=iload->Read(&range,sizeof(range),&nb);
				break;
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	Invalidate();
	return IO_OK;
	}

//---------------------------------------------------------------------------
//
// UI stuff --- keyinfo
//
//

static HIMAGELIST hKeyInfoImages = NULL;
static BOOL tcbRegistered=FALSE;

static void LoadResources()
	{
	static BOOL loaded=FALSE;
	if (loaded) return;
	loaded = TRUE;  
	HBITMAP hBitmap, hMask;	
	
	hKeyInfoImages = ImageList_Create(16, 15, TRUE, 6, 0);
	hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_HYBRIDKEYBUTTONS));
	hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_MASK_HYBRIDKEYBUTTONS));
	ImageList_Add(hKeyInfoImages,hBitmap,hMask);
	DeleteObject(hBitmap);  
	DeleteObject(hMask);            
	}

class DeleteMorphKeyInfoResources {
	public:
		~DeleteMorphKeyInfoResources() {
			ImageList_Destroy(hKeyInfoImages);
			}
	};
static DeleteMorphKeyInfoResources      theDelete;

static INT_PTR CALLBACK CubicMorphKeyInfoWndProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define CUBICMORPH_DLG_CLASSID	0xf980e27a

class CubicMorphKeyInfo : public ReferenceMaker {
	public:
		CubicMorphCont *cont;
		IObjParam *ip;
		HWND hWnd, hName;
		ISpinnerControl *iTime, *iT, *iC, *iB;
		ICustButton *iPrevKey, *iNextKey;
		ICustStatus *iKeyNum;
		BOOL valid;

		CubicMorphKeyInfo(CubicMorphCont *c,IObjParam *i,HWND hParent);
		~CubicMorphKeyInfo();
		void Init(HWND hWnd);
		void Invalidate();
		void InvalidateGraph();
		void Update();          

		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}
		Class_ID ClassID() {return Class_ID(CUBICMORPH_DLG_CLASSID,0);}
		void MaybeCloseWindow();

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		 PartID& partID,  RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(CubicMorphCont*)rtarg;}

		void SelectPrevKey();
		void SelectNextKey();
		void ApplyTimeChange();
		void ApplyTensChange();
		void ApplyContChange();
		void ApplyBiasChange();

		void WMCommand(int id, int notify, HWND hCtrl);
		void SpinnerStart(int id);
		void SpinnerChange(int id);
		void SpinnerEnd(int id,BOOL cancel);
	};

CubicMorphKeyInfo::CubicMorphKeyInfo(
		CubicMorphCont *c,IObjParam *i,HWND hParent)
	{	
	if (!tcbRegistered) {
		InitTCBGraph(hInstance);
		tcbRegistered = TRUE;
		}

	cont = NULL;
	ip = i;
	MakeRefByID(FOREVER,0,c);

	LoadResources();
	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_CUBICMORPH_KEYINFO),
		hParent,
		CubicMorphKeyInfoWndProc,
		(LPARAM)this);  
	RegisterMorphKeyWindow(hWnd,hParent,c);
	}

CubicMorphKeyInfo::~CubicMorphKeyInfo()
	{
	UnRegisterMorphKeyWindow(hWnd);
	ReleaseISpinner(iTime);
	ReleaseISpinner(iT);
	ReleaseISpinner(iC);
	ReleaseISpinner(iB);
	ReleaseICustButton(iPrevKey);
	ReleaseICustButton(iNextKey);
	ReleaseICustStatus(iKeyNum);
	DeleteAllRefsFromMe();
	}

class CheckForNonCubicMorphDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonCubicMorphDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(CUBICMORPH_DLG_CLASSID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};
void CubicMorphKeyInfo::MaybeCloseWindow()
	{
	CheckForNonCubicMorphDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}

RefResult CubicMorphKeyInfo::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,  
		RefMessage message)
	{
	switch (message) {
		case REFMSG_NODE_NAMECHANGE:
		case REFMSG_CHANGE:
			Invalidate();
			break;

		case REFMSG_REF_DELETED:
			MaybeCloseWindow();
			break;
		}
	return REF_SUCCEED;
	}

void CubicMorphKeyInfo::SelectPrevKey()
	{
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			int j = ((i-1)+cont->keys.Count())%cont->keys.Count();
			cont->keys[i].ClearFlag(KEY_SELECTED);
			cont->keys[j].SetFlag(KEY_SELECTED);                    
			break;
			}
		}
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void CubicMorphKeyInfo::SelectNextKey()
	{
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			int j = (i+1)%cont->keys.Count();
			cont->keys[i].ClearFlag(KEY_SELECTED);
			cont->keys[j].SetFlag(KEY_SELECTED);                    
			break;
			}
		}
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void CubicMorphKeyInfo::ApplyTimeChange()
	{
	int t = iTime->GetIVal();
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].time = t;
			}
		}
	cont->Invalidate();
	cont->keys.Invalidate();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	UpdateWindow(GetParent(hWnd));
	}

void CubicMorphKeyInfo::ApplyTensChange()
	{
	float t = iT->GetFVal()/25.0f - 1.0f;
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].tens = t;
			cont->keys[i].Invalidate();
			}
		}
	cont->Invalidate();
	cont->keys.Invalidate();
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void CubicMorphKeyInfo::ApplyContChange()
	{
	float c = iC->GetFVal()/25.0f - 1.0f;
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].cont = c;
			cont->keys[i].Invalidate();
			}
		}
	cont->Invalidate();
	cont->keys.Invalidate();
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void CubicMorphKeyInfo::ApplyBiasChange()
	{
	float b = iB->GetFVal()/25.0f - 1.0f;
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].bias = b;
			cont->keys[i].Invalidate();
			}
		}
	cont->Invalidate();
	cont->keys.Invalidate();
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void CubicMorphKeyInfo::Init(HWND hWnd)
	{
	this->hWnd = hWnd;
	
	iT = GetISpinner(GetDlgItem(hWnd,IDC_TCB_TSPIN));
	iT->SetLimits(0.0f,50.0f,FALSE);
	iT->SetScale(0.1f);
	iT->LinkToEdit(GetDlgItem(hWnd,IDC_TCB_T),EDITTYPE_FLOAT);

	iC = GetISpinner(GetDlgItem(hWnd,IDC_TCB_CSPIN));
	iC->SetLimits(0.0f,50.0f,FALSE);
	iC->SetScale(0.1f);
	iC->LinkToEdit(GetDlgItem(hWnd,IDC_TCB_C),EDITTYPE_FLOAT);

	iB = GetISpinner(GetDlgItem(hWnd,IDC_TCB_BSPIN));
	iB->SetLimits(0.0f,50.0f,FALSE);
	iB->SetScale(0.1f);
	iB->LinkToEdit(GetDlgItem(hWnd,IDC_TCB_B),EDITTYPE_FLOAT);

	iTime = GetISpinner(GetDlgItem(hWnd,IDC_KEYTIMESPIN));
	iTime->SetLimits(TIME_NegInfinity,TIME_PosInfinity,FALSE);
	iTime->SetScale(10.0f);
	iTime->LinkToEdit(GetDlgItem(hWnd,IDC_KEYTIME),EDITTYPE_TIME);

	iPrevKey = GetICustButton(GetDlgItem(hWnd,IDC_PREVKEY));
	iNextKey = GetICustButton(GetDlgItem(hWnd,IDC_NEXTKEY));
	iPrevKey->SetImage(hKeyInfoImages,3,3,8,8,16,15);
	iNextKey->SetImage(hKeyInfoImages,4,4,9,9,16,15);
	iKeyNum  = GetICustStatus(GetDlgItem(hWnd,IDC_KEYNUM));

	hName = GetDlgItem(hWnd,IDC_TARGNAME);

	Update();
	}

void CubicMorphKeyInfo::Invalidate()
	{
	valid = FALSE;	
	InvalidateRect(hWnd,NULL,FALSE);
	InvalidateGraph();
	}

void CubicMorphKeyInfo::InvalidateGraph()
	{
	HWND hGraph = GetDlgItem(hWnd,IDC_TCB_GRAPH);
	InvalidateRect(hGraph,NULL,FALSE);
	}

void CubicMorphKeyInfo::Update()
	{
	valid = TRUE;

	TimeValue time;
	float T, C, B;
	BOOL timeInit=FALSE,timeValid=FALSE;
	BOOL tValid, tInit, bValid, bInit, cValid, cInit;
	int numSel = 0, sel = -1;
	tValid = tInit = bValid = bInit = cValid = cInit = FALSE;

	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			sel = i;
			numSel++;

			if (timeInit) {
				if (time != cont->keys[i].time) {
					timeValid = FALSE;
					}
			} else {
				timeInit  = TRUE;
				timeValid = TRUE;
				time      = cont->keys[i].time;
				}

			if (tInit) {
				if (T != cont->keys[i].tens) {
					tValid = FALSE;
					}
			} else {
				tInit  = TRUE;
				tValid = TRUE;
				T      = cont->keys[i].tens;
				}
			
			if (cInit) {
				if (C != cont->keys[i].cont) {
					cValid = FALSE;
					}
			} else {
				cInit  = TRUE;
				cValid = TRUE;
				C      = cont->keys[i].cont;
				}

			if (bInit) {
				if (B != cont->keys[i].bias) {
					bValid = FALSE;
					}
			} else {
				bInit  = TRUE;
				bValid = TRUE;
				B      = cont->keys[i].bias;
				}
			}
		}
	
	if (timeValid) {
		iTime->SetValue(time,FALSE);
		iTime->Enable();
		iTime->SetIndeterminate(FALSE);
	} else {
		iTime->SetIndeterminate();
		iTime->Disable();
		}

	if (tValid) {
		iT->SetValue((T+1.0f)*25.0f,FALSE);             
		iT->SetIndeterminate(FALSE);
	} else {
		iT->SetIndeterminate();         
		}
	if (cValid) {
		iC->SetValue((C+1.0f)*25.0f,FALSE);             
		iC->SetIndeterminate(FALSE);
	} else {
		iC->SetIndeterminate();         
		}
	if (bValid) {
		iB->SetValue((B+1.0f)*25.0f,FALSE);             
		iB->SetIndeterminate(FALSE);
	} else {
		iB->SetIndeterminate();         
		}

	if (tValid && cValid && bValid) {
		TCBGraphParams gp;
		gp.tens     = T;
		gp.cont     = C;
		gp.bias     = B;
		gp.easeFrom = 0.0f;
		gp.easeTo   = 0.0f;
		HWND hGraph = GetDlgItem(hWnd,IDC_TCB_GRAPH);
		EnableWindow(hGraph,TRUE);
		SendMessage(hGraph,WM_SETTCBGRAPHPARAMS,0,(LPARAM)&gp);
		UpdateWindow(hGraph);
	} else {
		HWND hGraph = GetDlgItem(hWnd,IDC_TCB_GRAPH);
		EnableWindow(hGraph,FALSE);
		}

	if (numSel==1) {
		TSTR buf;
		buf.printf(_T("%d"),sel+1);
		iKeyNum->SetText(buf);
		iPrevKey->Enable();
		iNextKey->Enable();
		//buf.printf(GetString(IDS_RB_TARGETNUMBER),cont->keys[sel].targ);           
		SetWindowText(hName,*cont->targs[cont->keys[sel].targ].name);
	} else {
		iKeyNum->SetText(_T(""));
		iPrevKey->Disable();
		iNextKey->Disable();
		SetWindowText(hName,NULL);              
		}
	}

void CubicMorphKeyInfo::WMCommand(int id, int notify, HWND hCtrl)
	{
	switch (id) {
		case IDC_PREVKEY:
			SelectPrevKey();
			break;
		case IDC_NEXTKEY:
			SelectNextKey();
			break;
		
		case IDCANCEL:                  
		case IDOK:
			DestroyWindow(hWnd);
			break;
		}
	}

void CubicMorphKeyInfo::SpinnerStart(int id)
	{
	theHold.Begin();
	cont->HoldTrack();
	}

void CubicMorphKeyInfo::SpinnerChange(int id)
	{
	switch (id) {
		case IDC_KEYTIMESPIN:
			ApplyTimeChange(); break;

		case IDC_TCB_TSPIN:
			ApplyTensChange(); break;
		case IDC_TCB_CSPIN:
			ApplyContChange(); break;
		case IDC_TCB_BSPIN:
			ApplyBiasChange(); break;
		}
	}

void CubicMorphKeyInfo::SpinnerEnd(int id,BOOL cancel)
	{
	if (cancel) {
		theHold.Cancel();
	} else {
		theHold.Accept(GetString(IDS_RB_EDITKEYINFO));
		}
	}



static INT_PTR CALLBACK CubicMorphKeyInfoWndProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	CubicMorphKeyInfo *k = (CubicMorphKeyInfo*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			k = (CubicMorphKeyInfo*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			k->Init(hWnd);
			return FALSE;   // DB 2/27

		case WM_PAINT:
			if (!k->valid) {
				k->Update();
				}
			return 0;
		
		case CC_SPINNER_BUTTONDOWN:
			k->SpinnerStart(LOWORD(wParam));
			break;

		case CC_SPINNER_CHANGE:
			k->SpinnerChange(LOWORD(wParam));
			break;

		case CC_SPINNER_BUTTONUP:
			k->SpinnerEnd(LOWORD(wParam),!HIWORD(wParam));
			break;

		case WM_COMMAND:
			k->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);                                               
			break;
		
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			delete k;
			break;

		default:
			return 0;
		}
	return 1;
	}

void CubicMorphCont::EditTrackParams(
		TimeValue t,ParamDimensionBase *dim,TCHAR *pname,HWND hParent,
		IObjParam *ip,DWORD flags)
	{
	HWND hCur = FindOpenMorphKeyWindow(hParent,this);
	if (hCur) {
		SetForegroundWindow(hCur);
		return;
		}
	new CubicMorphKeyInfo(this,ip,hParent);
	}

RefTargetHandle CubicMorphCont::Clone(RemapDir& remap)
	{
	CubicMorphCont *cont = new CubicMorphCont;
	cont->keys = keys;
	cont->obValid.SetEmpty();
	cont->version = version;
	for (int i=0; i<targs.Count(); i++) {
		cont->targs.AddTarg(
			targs[i].obj,
			*(targs[i].name),
			targs[i].tm,
			cont,
			cont->Control::NumRefs());              
		}
	BaseClone(this, cont, remap);
	return cont;
	}

void CubicMorphCont::Copy(Control *from)
	{
	if (from->ClassID()==ClassID()) {
		CubicMorphCont *cont = (CubicMorphCont*)from;
		keys = cont->keys;
		obValid.SetEmpty();
		version = cont->version;
		for (int i=0; i<cont->targs.Count(); i++) {
			targs.AddTarg(
				cont->targs[i].obj,
				*(cont->targs[i].name),
				cont->targs[i].tm,
				this,
				Control::NumRefs());              
			}
	} else if (from->ClassID()==Class_ID(BARYMORPHCONT_CLASS_ID,0)) {
		BaryMorphCont *cont = (BaryMorphCont*)from;
		for (int i=0; i<cont->targs.Count(); i++) {
			targs.AddTarg(
				cont->targs[i].obj,
				*(cont->targs[i].name),
				cont->targs[i].tm,
				this,
				Control::NumRefs());              
			}
		for (i=0; i<cont->keys.Count(); i++) {
			if (cont->keys[i].WTS().Count()) {
				int best    = cont->keys[i].WTS()[0].targ;
				float fbest = cont->keys[i].WTS()[0].percent;
				for (int j=0; j<cont->keys[i].WTS().Count(); j++) {
					if (fabs(cont->keys[i].WTS()[j].percent)>fabs(fbest)) {
						best  = cont->keys[i].WTS()[j].targ;
						fbest = cont->keys[i].WTS()[j].percent;
						}
					}
				keys.AddNewKey(cont->keys[i].time,best);
				}
			}
		}
	}




//-----------------------------------------------------------------------

	
void BaryMorphKey::Invalidate()
	{
	ClearFlag(MULTS_VALID); 
	}

void BaryMorphKey::ComputeMults()
	{
	if (!TestFlag(MULTS_VALID)) {
		float tm,cm,cp,bm,bp,tmcm,tmcp;
		tm = 0.5f*(1.0f - tens);
		cm = 1.0f - cont;       
		cp = 2.0f - cm;
		bm = 1.0f - bias;
		bp = 2.0f - bm;      
		tmcm = tm*cm;   tmcp = tm*cp;   
		k = tmcm*bp;    l = tmcp*bm;
		m = tmcp*bp;    n = tmcm*bm;
		SetFlag(MULTS_VALID);
		}
	}

void BaryMorphKey::TargetDeleted(int t)
	{
	for (int i=wts->Count()-1; i>=0; i--) {
		if (WTS()[i].targ>t) WTS()[i].targ--;
		else
		if (WTS()[i].targ==t) WTS().Delete(i,1);
		}
	}

float BaryMorphKey::GetWeight(int t)
	{
	for (int i=0; i<wts->Count(); i++) {
		if (WTS()[i].targ==t) return WTS()[i].percent;
		}
	return 0.0f;
	}

void BaryMorphKey::SetWeight(int t,float w)
	{
	for (int i=0; i<wts->Count(); i++) {
		if (WTS()[i].targ==t) {
			if (w==0.0f) WTS().Delete(i,1);
			else WTS()[i].percent = w;
			return;
			}
		}
	TargetWeight wt(t,w);
	WTS().Append(1,&wt);
	}

void BaryMorphKeyTab::Update() 
	{
	if (TestFlag(TRACK_INVALID)) {
		ClearFlag(TRACK_INVALID);
		
		// Sort the array by time and shrink
		Sort((CompareFnc)CompareMorphKeys);
		//CheckForDups();	// RB 4/21/99: Only do this on mouse cycle completed.
		Shrink();
		
		// Update the range
		if (!TestFlag(RANGE_UNLOCKED)) {
			if (Count()) {
				range.Set((*this)[0].time,(*this)[Count()-1].time);
			} else {
				range.SetEmpty();
				}
			}
		
		// Compute the multipliers
		for (int i=0; i<Count(); i++) {
			(*this)[i].ComputeMults();
			}
		}
	}

#define BARYKEYTABLE_COUNT		0x0300
#define BARYKEYTABLE_KEYDATA	0x0310
#define BARYKEYTABLE_TARG		0x0320

IOResult BaryMorphKeyTab::Save(ISave *isave)
	{
	ULONG nb;	
	
	isave->BeginChunk(MORPHKEY_FLAGS_CHUNK);
	isave->Write(&flags, sizeof(flags), &nb);
	isave->EndChunk();

	isave->BeginChunk(MORPHKEY_RANGE_CHUNK);
	isave->Write(&range, sizeof(range), &nb);
	isave->EndChunk();
	
	int ct = Count();
	isave->BeginChunk(BARYKEYTABLE_COUNT);
	isave->Write(&ct, sizeof(ct), &nb);
	isave->EndChunk();

	for (int i=0; i<ct; i++) {
		isave->BeginChunk(BARYKEYTABLE_KEYDATA);
		isave->Write(&(*this)[i],sizeof(BaryMorphKey),&nb);
		isave->EndChunk();
		
		for (int j=0; j<(*this)[i].wts->Count(); j++) {
			isave->BeginChunk(BARYKEYTABLE_TARG);
			isave->Write(&(*this)[i].WTS()[j],sizeof(TargetWeight),&nb);
			isave->EndChunk();
			}		
		}

	return IO_OK;
	}

IOResult BaryMorphKeyTab::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res = IO_OK;
	int ct, ix=-1;

	Resize(0);
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case BARYKEYTABLE_COUNT:
				res=iload->Read(&ct,sizeof(ct),&nb);
				SetCount(ct);
				break;

			case BARYKEYTABLE_KEYDATA:
				ix++;
				res=iload->Read(&(*this)[ix],sizeof(BaryMorphKey),&nb);
				(*this)[ix].wts = new Tab<TargetWeight>;
				break;

			case BARYKEYTABLE_TARG: {
				TargetWeight wt;
				res=iload->Read(&wt,sizeof(wt),&nb);
				(*this)[ix].wts->Append(1,&wt);
				break;
				}

			case MORPHKEY_FLAGS_CHUNK:
				res=iload->Read(&flags,sizeof(flags),&nb);
				break;

			case MORPHKEY_RANGE_CHUNK:
				res=iload->Read(&range,sizeof(range),&nb);
				break;
			}
		
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	Invalidate();
	return IO_OK;
	}

//-------------------------------------------------------------------

void BaryMorphCont::CalcFirstCoef(float *v,float *c)
	{
	float G, H, J;

	H = .5f*(1.0f-keys[0].tens)*v[2];
	J = 3.0f*H;
	G = v[3] - H;
	c[0] = v[0] - J - G * keys[1].k;
	c[1] = v[1] + J + G*(keys[1].k-keys[1].l);
	c[2] = G*keys[1].l;
	}

void BaryMorphCont::CalcLastCoef(float *v,float *c)
	{
	int nkeys = keys.Count();
	float G, H, J;

	H = .5f*(1.0f-keys[nkeys-1].tens)*v[3];
	J = 3.0f*H;
	G = v[2]-H;
	c[0] = -G*keys[nkeys-2].m;
	c[1] = v[0] - J + G*(keys[nkeys-2].m-keys[nkeys-2].n);
	c[2] = v[1] + J + G*keys[nkeys-2].n;
	}

void BaryMorphCont::CalcMiddleCoef(float *v,int *knum,float *c)
	{
	c[0] = -v[2]*keys[knum[1]].m;
	c[1] = v[0] + v[2]*(keys[knum[1]].m-keys[knum[1]].n) - v[3]*keys[knum[2]].k;
	c[2] = v[1] + v[2]*keys[knum[1]].n + v[3]*(keys[knum[2]].k-keys[knum[2]].l);
	c[3] = v[3]*keys[knum[2]].l;
	}

void BaryMorphCont::HoldTrack()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new BaryMorphRest(this));
		}
	}

void BaryMorphCont::Update(TimeValue t,TimeValue realT)
	{
	// RB 7/18/2000: obValid used to key off of 't' rather than 'realT'. I think
	// it should work off of realT (note that realT is used to update the
	// validity interval at the end of this function).
	//
	if (obValid.InInterval(realT)) return;      
	obValid.SetInstant(realT);
	
	// Whenever we dump the cache we need to make sure anyone
	// downstream knows about it.
	FreeCache();    
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);       

	if (!targs.Count()) {
		// Build a standin object.              
		ob = ObjectState(CreateNewTriObject());
		ob.obj->LockChannels(ALL_CHANNELS);
		ob.obj->LockObject();
		obValid = FOREVER;
		return;         
		}

	lockCache = TRUE;
	
	float v[4] = {0.0f,0.0f,0.0f,0.0f}, c[4] = {0.0f,0.0f,0.0f,0.0f};
	int n0, n1, nKeys = keys.Count();
	float u;
	int knum[4], keyct=0;
	
	keys.Update();  
	GetInterpVal(t,n0,n1,u);	

	// Compute coeffeceints	
	if (nKeys==0) keyct=0;
	else
	if (nKeys==1 || n0<0 || n1<0) {
		keyct   = 1;
		knum[0] = 0;
		c[0]    = 1.0f;
	} else 
	if (n0==n1 || u==0.0f) {
		keyct   = 1;
		knum[0] = n0;
		c[0]    = 1.0f;
	} else 
	if (nKeys==2) {
		keyct   = 2;
		knum[0] = n0;
		knum[1] = n1;
		c[0]    = 1.0f-u;
		c[1]    = u;		
	} else {
		ComputeHermiteBasis(u,v);
		if (n0==0) {
			keyct = 3;
			knum[0] = n0;
			knum[1] = n1;
			knum[2] = n1+1;
			CalcFirstCoef(v,c);
		} else
		if (n1==nKeys-1) {
			keyct = 3;
			knum[0] = n0-1;
			knum[1] = n0;
			knum[2] = n1;
			CalcLastCoef(v,c);
		} else {			
			keyct = 4;
			knum[0] = n0-1;
			knum[1] = n0;
			knum[2] = n1;
			knum[3] = n1+1;
			CalcMiddleCoef(v,knum,c);
			}
		}
	
	// Interpolate weights
	Tab<float> wts;
	wts.SetCount(targs.Count());
	for (int i=0; i<wts.Count(); i++) {
		wts[i] = 0.0f;
		for (int j=0; j<keyct; j++) {
			wts[i] += keys[knum[j]].GetWeight(i) * c[j];
			}
		}
	// Special case... if there are no keys, set the first weight to 1
	if (!nKeys) wts[0] = 1.0f;

	// Evaluate objects with non-zero weights
	// Always evaluate the 0th object because that's the object we are going to deform
	Tab<Object*> objs;
	objs.SetCount(targs.Count());
	Tab<BOOL> del;
	del.SetCount(targs.Count());
	for (i=0; i<targs.Count(); i++) {
		objs[i] = NULL;
		del[i]  = FALSE;
		if (wts[i]!=0.0f || !i) {
			// Eval the obejct			
			ObjectState os = targs[i].obj->Eval(t);
			MergeAdditionalChannels(os.obj,i);				

			objs[i] = os.obj;
			if (i==0) ob = os;

			// Make sure all objects are deformable
			if (!objs[i]->IsDeformable()) {
				// Convert to deformable
				Object *o = objs[i]->ConvertToType(t,defObjectClassID);
				
				// Unlock if it's a new object				
				if (o!=objs[i]) {
					o->UnlockObject();
					objs[i] = o;
					// Mark it as needing deletion (if it's not the first object)
					if (i) del[i] = TRUE;
					}
				}			
			}
		}
	ob.obj = objs[0]; // Make sure they're in synch	

	// Prepare the geom channel to be modified
	if (ob.obj->IsObjectLocked()) {
		ob.obj = ob.obj->MakeShallowCopy(OBJ_CHANNELS);		
		ob.obj->LockChannels(OBJ_CHANNELS);
		ob.obj->UnlockObject();
		}	
	ob.obj->ReadyChannelsForMod(GEOM_CHANNEL);              
	
	// Blend between all the targets
	int pts = ob.obj->NumPoints();
	for (i=0; i<pts; i++) {
		Point3 pt(0,0,0);
		for (int j=0; j<targs.Count(); j++) {
			if (objs[j] && i<objs[j]->NumPoints()) {
				pt += (objs[j]->GetPoint(i)*targs[j].tm) * wts[j];
				}
			}
		ob.obj->SetPoint(i,pt);
		if (ob.obj->HasWeights()) {
			double w = 0.0;
			for (int j=0; j<targs.Count(); j++) {
				if (objs[j] && i<objs[j]->NumPoints()) {
					w += objs[j]->GetWeight(i) * wts[j];
					}
				}
			ob.obj->SetWeight(i, w);
			}
		}
	ob.obj->PointsWereChanged();
		
	// Delete any objects that were created from ConvertToType()    
	for (i=0; i<targs.Count(); i++) {
		if (del[i]) objs[i]->DeleteThis();
		}

	// Lock it since I'm going to hold on to it and return it.
	ob.obj->LockChannels(GEOM_CHANNEL);
	ob.obj->UpdateValidity(GEOM_CHAN_NUM,Interval(realT,realT));
	ob.obj->LockObject();
	lockCache = FALSE;
	}

void BaryMorphCont::AddNewKey(TimeValue t,DWORD flags)
	{	
	int n0, n1, nKeys = keys.Count();
	float u;
	int knum[4], keyct=0;
	float c[4], v[4];
	
	if (!targs.Count()) return;
	GetInterpVal(t,n0,n1,u);

	// Compute coeffeceints	
	if (nKeys==0) keyct=0;
	else
	if (nKeys==1 || n0<0 || n1<0) {
		keyct   = 1;
		knum[0] = 0;
		c[0]    = 1.0f;
	} else 
	if (n0==n1 || u==0.0f) {
		keyct   = 1;
		knum[0] = n0;
		c[0]    = 1.0f;
	} else 
	if (nKeys==2) {
		keyct   = 2;
		knum[0] = n0;
		knum[1] = n1;
		c[0]    = 1.0f-u;
		c[1]    = u;		
	} else {
		ComputeHermiteBasis(u,v);
		if (n0==0) {
			keyct = 3;
			knum[0] = n0;
			knum[1] = n1;
			knum[2] = n1+1;
			CalcFirstCoef(v,c);
		} else
		if (n1==nKeys-1) {
			keyct = 3;
			knum[0] = n0-1;
			knum[1] = n0;
			knum[2] = n1;
			CalcLastCoef(v,c);
		} else {			
			keyct = 4;
			knum[0] = n0-1;
			knum[1] = n0;
			knum[2] = n1;
			knum[3] = n1+1;
			CalcMiddleCoef(v,knum,c);
			}
		}

	// Interpolate weights
	Tab<float> wts;
	wts.SetCount(targs.Count());
	for (int i=0; i<wts.Count(); i++) {
		wts[i] = 0.0f;
		for (int j=0; j<keyct; j++) {
			wts[i] += keys[knum[j]].GetWeight(i) * c[j];
			}
		}
	if (keyct==0) wts[0] = 1.0f;

	// Deselect all keys
	for (i=0; i<keys.Count(); i++) keys[i].ClearFlag(KEY_SELECTED);

	// Add the key
	HoldTrack();
	BaryMorphKey key;
	key.time = t;	
	key.SetFlag(KEY_SELECTED);
	for (i=0; i<wts.Count(); i++) {
		key.SetWeight(i,wts[i]);
		}
	key.Invalidate();
	keys.Append(1,&key,5);
	keys.Invalidate();	
	Invalidate();
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	}

void BaryMorphCont::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{       
	TimeValue rt = t;
	Interval evalid;
	t = ApplyEase(t,evalid);
	t = ProcessORT(t);

	// Limit to in range
	if (NumKeys()>1)
		{
		if (t<keys.range.Start()) 
			t = keys.range.Start();
		if (t>keys.range.End()) 
			t = keys.range.End();	
		}

	Update(t,rt);
	ObjectState *os = (ObjectState*)val;
	*os = ob;               
	}

void BaryMorphCont::DeleteMorphTarg(int i)
	{
	HoldTrack();
	for (int j=keys.Count()-1; j>=0; j--) {
		keys[j].TargetDeleted(i);
		}
	FreeCache();    
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	GenMorphCont<BaryMorphKeyTab>::DeleteMorphTarg(i);
	}

RefTargetHandle BaryMorphCont::Clone(RemapDir& remap)
	{
	BaryMorphCont *cont = new BaryMorphCont;
	cont->keys = keys;
	cont->obValid.SetEmpty();
	cont->version = version;
	for (int i=0; i<targs.Count(); i++) {
		cont->targs.AddTarg(
			targs[i].obj,
			*(targs[i].name),
			targs[i].tm,
			cont,
			cont->Control::NumRefs());              
		}
	BaseClone(this, cont, remap);
	return cont;
	}

void BaryMorphCont::Copy(Control *from)
	{
	if (from->ClassID()==ClassID()) {
		BaryMorphCont *cont = (BaryMorphCont*)from;
		keys = cont->keys;
		obValid.SetEmpty();
		version = cont->version;
		for (int i=0; i<cont->targs.Count(); i++) {
			targs.AddTarg(
				cont->targs[i].obj,
				*(cont->targs[i].name),
				cont->targs[i].tm,
				this,
				Control::NumRefs());              
			}
	} else if (from->ClassID()==Class_ID(CUBICMORPHCONT_CLASS_ID,0)) {
		CubicMorphCont *cont = (CubicMorphCont*)from;
		for (int i=0; i<cont->targs.Count(); i++) {
			targs.AddTarg(
				cont->targs[i].obj,
				*(cont->targs[i].name),
				cont->targs[i].tm,
				this,
				Control::NumRefs());              
			}
		for (i=0; i<cont->keys.Count(); i++) {
			keys.AddNewKey(cont->keys[i].time,cont->keys[i].targ);
			}
		}
	}



//---------------------------------------------------------------------------
//
// UI stuff --- keyinfo for Barycentric controller
//
//

static INT_PTR CALLBACK BaryMorphKeyInfoWndProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static BOOL constrain100 = TRUE;

void BaryMorphContClassDesc::ResetClassParams(BOOL fileReset)
	{
	constrain100 = TRUE;
	}

#define BARYMORPH_DLG_CLASSID	0x918c74ba

class BaryMorphKeyInfo : public ReferenceMaker {
	public:
		BaryMorphCont *cont;
		IObjParam *ip;
		HWND hWnd, hList;
		ISpinnerControl *iTime, *iT, *iC, *iB, *iPerc;
		ICustButton *iPrevKey, *iNextKey;
		ICustStatus *iKeyNum;
		BOOL valid;
		int lastSel;

		BaryMorphKeyInfo(BaryMorphCont *c,IObjParam *i,HWND hParent);
		~BaryMorphKeyInfo();
		void Init(HWND hWnd);
		void Invalidate();
		void InvalidateGraph();
		void Update();          

		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}
		Class_ID ClassID() {return Class_ID(BARYMORPH_DLG_CLASSID,0);}
		void MaybeCloseWindow();

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		 PartID& partID,  RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(BaryMorphCont*)rtarg;}

		void SelectPrevKey();
		void SelectNextKey();
		void ApplyTimeChange();
		void ApplyTensChange();
		void ApplyContChange();
		void ApplyBiasChange();
		void SetupPercentSpin();
		void SetPercent100();
		void ApplyPercentChange();

		void WMCommand(int id, int notify, HWND hCtrl);
		void SpinnerStart(int id);
		void SpinnerChange(int id);
		void SpinnerEnd(int id,BOOL cancel);
	};

BaryMorphKeyInfo::BaryMorphKeyInfo(
		BaryMorphCont *c,IObjParam *i,HWND hParent)
	{	
	if (!tcbRegistered) {
		InitTCBGraph(hInstance);
		tcbRegistered = TRUE;
		}

	cont = NULL;
	ip = i;
	MakeRefByID(FOREVER,0,c);
	lastSel = -1;

	LoadResources();
	hWnd = CreateDialogParam(
		hInstance,
		MAKEINTRESOURCE(IDD_BARYMORPH_KEYINFO),
		hParent,
		BaryMorphKeyInfoWndProc,
		(LPARAM)this);  
	RegisterMorphKeyWindow(hWnd,hParent,c);
	}

BaryMorphKeyInfo::~BaryMorphKeyInfo()
	{
	UnRegisterMorphKeyWindow(hWnd);
	ReleaseISpinner(iTime);
	ReleaseISpinner(iT);
	ReleaseISpinner(iC);
	ReleaseISpinner(iB);
	ReleaseISpinner(iPerc);
	ReleaseICustButton(iPrevKey);
	ReleaseICustButton(iNextKey);
	ReleaseICustStatus(iKeyNum);
	DeleteAllRefsFromMe();
	}

class CheckForNonBaryMorphDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonBaryMorphDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(BARYMORPH_DLG_CLASSID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};
void BaryMorphKeyInfo::MaybeCloseWindow()
	{
	CheckForNonBaryMorphDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}

RefResult BaryMorphKeyInfo::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,  
		RefMessage message)
	{
	switch (message) {
		case REFMSG_NODE_NAMECHANGE:
		case REFMSG_CHANGE:
			Invalidate();
			break;

		case REFMSG_REF_DELETED:
			MaybeCloseWindow();
			break;
		}
	return REF_SUCCEED;
	}

void BaryMorphKeyInfo::SelectPrevKey()
	{
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			int j = ((i-1)+cont->keys.Count())%cont->keys.Count();
			cont->keys[i].ClearFlag(KEY_SELECTED);
			cont->keys[j].SetFlag(KEY_SELECTED);                    
			break;
			}
		}
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaryMorphKeyInfo::SelectNextKey()
	{
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			int j = (i+1)%cont->keys.Count();
			cont->keys[i].ClearFlag(KEY_SELECTED);
			cont->keys[j].SetFlag(KEY_SELECTED);                    
			break;
			}
		}
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaryMorphKeyInfo::ApplyTimeChange()
	{
	int t = iTime->GetIVal();
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].time = t;
			}
		}
	cont->Invalidate();
	cont->keys.Invalidate();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	UpdateWindow(GetParent(hWnd));
	}

void BaryMorphKeyInfo::ApplyTensChange()
	{
	float t = iT->GetFVal()/25.0f - 1.0f;
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].tens = t;
			cont->keys[i].Invalidate();
			}
		}
	cont->Invalidate();
	cont->keys.Invalidate();
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaryMorphKeyInfo::ApplyContChange()
	{
	float c = iC->GetFVal()/25.0f - 1.0f;
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].cont = c;
			cont->keys[i].Invalidate();
			}
		}
	cont->Invalidate();
	cont->keys.Invalidate();
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaryMorphKeyInfo::ApplyBiasChange()
	{
	float b = iB->GetFVal()/25.0f - 1.0f;
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].bias = b;
			cont->keys[i].Invalidate();
			}
		}
	cont->Invalidate();
	cont->keys.Invalidate();
	InvalidateGraph();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void BaryMorphKeyInfo::SetPercent100()
	{
	float total = 0.0f, selVal = 0.0f;
	int sel=-1;
	int listSel = SendMessage(hList,LB_GETCURSEL,0,0);

	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			sel = i;
			break;
			}
		}
	if (sel<0 || !cont->targs.Count()) return;
		
	for (i=0; i<cont->targs.Count(); i++) {
		if (i==listSel) {
			selVal = cont->keys[sel].GetWeight(i);
		} else {
			total += cont->keys[sel].GetWeight(i);
			}
		}

	if (total==0.0f) {				
		float ct = float(cont->targs.Count());
		if (listSel>=0) ct -= 1.0f;
		if (ct>=0) {
			ct = (1.0f-selVal)/ct;
			for (i=0; i<cont->targs.Count(); i++) {
				if (i!=listSel) {
					cont->keys[sel].SetWeight(i,ct);
					}
				}
			}
	} else {
		float f = (1.0f-selVal)/total;
		for (i=0; i<cont->targs.Count(); i++) {
			if (i!=listSel) {
				cont->keys[sel].SetWeight(i,
					cont->keys[sel].GetWeight(i)*f);
				}
			}
		}	
	}

void BaryMorphKeyInfo::ApplyPercentChange()
	{
	float pct = iPerc->GetFVal()/100.0f;
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			cont->keys[i].SetWeight(lastSel,pct);
			}
		}
	if (constrain100) SetPercent100();
	cont->Invalidate();
	cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	UpdateWindow(hWnd);
	}

void BaryMorphKeyInfo::Init(HWND hWnd)
	{
	this->hWnd = hWnd;	

	iT = GetISpinner(GetDlgItem(hWnd,IDC_TCB_TSPIN));
	iT->SetLimits(0.0f,50.0f,FALSE);
	iT->SetScale(0.1f);
	iT->LinkToEdit(GetDlgItem(hWnd,IDC_TCB_T),EDITTYPE_FLOAT);

	iC = GetISpinner(GetDlgItem(hWnd,IDC_TCB_CSPIN));
	iC->SetLimits(0.0f,50.0f,FALSE);
	iC->SetScale(0.1f);
	iC->LinkToEdit(GetDlgItem(hWnd,IDC_TCB_C),EDITTYPE_FLOAT);

	iB = GetISpinner(GetDlgItem(hWnd,IDC_TCB_BSPIN));
	iB->SetLimits(0.0f,50.0f,FALSE);
	iB->SetScale(0.1f);
	iB->LinkToEdit(GetDlgItem(hWnd,IDC_TCB_B),EDITTYPE_FLOAT);

	iTime = GetISpinner(GetDlgItem(hWnd,IDC_KEYTIMESPIN));
	iTime->SetLimits(TIME_NegInfinity,TIME_PosInfinity,FALSE);
	iTime->SetScale(10.0f);
	iTime->LinkToEdit(GetDlgItem(hWnd,IDC_KEYTIME),EDITTYPE_TIME);

	iPerc = GetISpinner(GetDlgItem(hWnd,IDC_BARYMORPH_PERCENTSPIN));
	iPerc->SetLimits(-999999999.0f,999999999.0f,FALSE);
	iPerc->SetScale(0.5f);
	iPerc->LinkToEdit(GetDlgItem(hWnd,IDC_BARYMORPH_PERCENT),EDITTYPE_FLOAT);

	iPrevKey = GetICustButton(GetDlgItem(hWnd,IDC_PREVKEY));
	iNextKey = GetICustButton(GetDlgItem(hWnd,IDC_NEXTKEY));
	iPrevKey->SetImage(hKeyInfoImages,3,3,8,8,16,15);
	iNextKey->SetImage(hKeyInfoImages,4,4,9,9,16,15);
	iKeyNum  = GetICustStatus(GetDlgItem(hWnd,IDC_KEYNUM));

	hList = GetDlgItem(hWnd,IDC_BARYMORPH_TARGLIST);

	Update();
	}

void BaryMorphKeyInfo::Invalidate()
	{
	valid = FALSE;	
	InvalidateRect(hWnd,NULL,FALSE);
	InvalidateGraph();
	}

void BaryMorphKeyInfo::InvalidateGraph()
	{
	HWND hGraph = GetDlgItem(hWnd,IDC_TCB_GRAPH);
	InvalidateRect(hGraph,NULL,FALSE);
	}

void BaryMorphKeyInfo::Update()
	{
	valid = TRUE;

	TimeValue time;
	float T, C, B;
	BOOL timeInit=FALSE,timeValid=FALSE;
	BOOL tValid, tInit, bValid, bInit, cValid, cInit;
	int numSel = 0, sel = -1;
	tValid = tInit = bValid = bInit = cValid = cInit = FALSE;

	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			sel = i;
			numSel++;

			if (timeInit) {
				if (time != cont->keys[i].time) {
					timeValid = FALSE;
					}
			} else {
				timeInit  = TRUE;
				timeValid = TRUE;
				time      = cont->keys[i].time;
				}

			if (tInit) {
				if (T != cont->keys[i].tens) {
					tValid = FALSE;
					}
			} else {
				tInit  = TRUE;
				tValid = TRUE;
				T      = cont->keys[i].tens;
				}
			
			if (cInit) {
				if (C != cont->keys[i].cont) {
					cValid = FALSE;
					}
			} else {
				cInit  = TRUE;
				cValid = TRUE;
				C      = cont->keys[i].cont;
				}

			if (bInit) {
				if (B != cont->keys[i].bias) {
					bValid = FALSE;
					}
			} else {
				bInit  = TRUE;
				bValid = TRUE;
				B      = cont->keys[i].bias;
				}
			}
		}
	
	if (timeValid) {
		iTime->SetValue(time,FALSE);
		iTime->Enable();
		iTime->SetIndeterminate(FALSE);
	} else {
		iTime->SetIndeterminate();
		iTime->Disable();
		}

	if (tValid) {
		iT->SetValue((T+1.0f)*25.0f,FALSE);             
		iT->SetIndeterminate(FALSE);
	} else {
		iT->SetIndeterminate();         
		}
	if (cValid) {
		iC->SetValue((C+1.0f)*25.0f,FALSE);             
		iC->SetIndeterminate(FALSE);
	} else {
		iC->SetIndeterminate();         
		}
	if (bValid) {
		iB->SetValue((B+1.0f)*25.0f,FALSE);             
		iB->SetIndeterminate(FALSE);
	} else {
		iB->SetIndeterminate();         
		}

	if (tValid && cValid && bValid) {
		TCBGraphParams gp;
		gp.tens     = T;
		gp.cont     = C;
		gp.bias     = B;
		gp.easeFrom = 0.0f;
		gp.easeTo   = 0.0f;
		HWND hGraph = GetDlgItem(hWnd,IDC_TCB_GRAPH);
		EnableWindow(hGraph,TRUE);
		SendMessage(hGraph,WM_SETTCBGRAPHPARAMS,0,(LPARAM)&gp);
		UpdateWindow(hGraph);
	} else {
		HWND hGraph = GetDlgItem(hWnd,IDC_TCB_GRAPH);
		EnableWindow(hGraph,FALSE);
		}

	if (numSel==1) {
		TSTR buf;
		buf.printf(_T("%d"),sel+1);
		iKeyNum->SetText(buf);
		iPrevKey->Enable();
		iNextKey->Enable();
		
		SendMessage(hList,LB_RESETCONTENT,0,0);
		EnableWindow(GetDlgItem(hWnd,IDC_BARYMORPH_TARGETLEBEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_BARYMORPH_PERCENTLABEL),TRUE);
		
		if (cont->targs.Count())
			 EnableWindow(GetDlgItem(hWnd,IDC_BARYMORPH_100TOTAL),TRUE);
		else EnableWindow(GetDlgItem(hWnd,IDC_BARYMORPH_100TOTAL),FALSE);

		float total = 0.0f;
		for (int i=0; i<cont->targs.Count(); i++) {
			float pct = cont->keys[sel].GetWeight(i)*100.0f;
			total += pct;
			TSTR name;
			name.printf(_T("%.1f%%\t%s"),pct,*cont->targs[i].name);
			SendMessage(hList,LB_ADDSTRING,0,(LONG_PTR)(TCHAR*)name);
			}

		TSTR tot;
		tot.printf(_T("%.1f%%"),total);
		SetDlgItemText(hWnd,IDC_BARYMORPH_TOTAL,tot);

		SetupPercentSpin();
	} else {
		iKeyNum->SetText(_T(""));
		iPrevKey->Disable();
		iNextKey->Disable();
		
		SendMessage(hList,LB_RESETCONTENT,0,0);
		EnableWindow(GetDlgItem(hWnd,IDC_BARYMORPH_TARGETLEBEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_BARYMORPH_PERCENTLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_BARYMORPH_100TOTAL),FALSE);
		SetDlgItemText(hWnd,IDC_BARYMORPH_TOTAL,_T(""));
		iPerc->SetValue(0.0f,FALSE);
		iPerc->Disable();
		}

	CheckDlgButton(hWnd,IDC_BARYMORPH_100TOTAL,constrain100);
	}

void BaryMorphKeyInfo::SetupPercentSpin()
	{
	int sel=-1;
	for (int i=0; i<cont->keys.Count(); i++) {
		if (cont->keys[i].TestFlag(KEY_SELECTED)) {
			if (sel>=0) {
				sel = -1;
				break;
				}
			sel = i;			
			}
		}

	if (lastSel>=0 && sel>=0) {
		float pct = cont->keys[sel].GetWeight(lastSel)*100.0f;
		iPerc->SetValue(pct,FALSE);
		iPerc->Enable();
		SendMessage(hList,LB_SETCURSEL,lastSel,0);
	} else {
		iPerc->SetValue(0.0f,FALSE);
		iPerc->Disable();
		}
	}

void BaryMorphKeyInfo::WMCommand(int id, int notify, HWND hCtrl)
	{
	switch (id) {
		case IDC_BARYMORPH_TARGLIST:
			if (notify==LBN_SELCHANGE) {
				lastSel = SendMessage(hList,LB_GETCURSEL,0,0);
				SetupPercentSpin();
				}
			break;

		case IDC_BARYMORPH_100TOTAL:
			constrain100 = IsDlgButtonChecked(hWnd,IDC_BARYMORPH_100TOTAL);
			if (constrain100) {
				theHold.Begin();
				cont->HoldTrack();
				SetPercent100();
				theHold.Accept(GetString(IDS_RB_EDITKEYINFO));
				cont->Invalidate();
				cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				ip->RedrawViews(ip->GetTime());
				}
			break;

		case IDC_PREVKEY:
			SelectPrevKey();
			break;
		case IDC_NEXTKEY:
			SelectNextKey();
			break;
		
		case IDCANCEL:                  
		case IDOK:
			DestroyWindow(hWnd);
			break;
		}
	}

void BaryMorphKeyInfo::SpinnerStart(int id)
	{
	theHold.Begin();
	cont->HoldTrack();
	}

void BaryMorphKeyInfo::SpinnerChange(int id)
	{
	switch (id) {
		case IDC_KEYTIMESPIN:
			ApplyTimeChange(); break;

		case IDC_TCB_TSPIN:
			ApplyTensChange(); break;
		case IDC_TCB_CSPIN:
			ApplyContChange(); break;
		case IDC_TCB_BSPIN:
			ApplyBiasChange(); break;

		case IDC_BARYMORPH_PERCENTSPIN:
			ApplyPercentChange(); break;
		}
	ip->RedrawViews(ip->GetTime());
	UpdateWindow(hWnd);
	}

void BaryMorphKeyInfo::SpinnerEnd(int id,BOOL cancel)
	{
	if (cancel) {
		theHold.Cancel();
	} else {
		if (id==IDC_KEYTIMESPIN) {
			// RB 4/21/99: Do this on mouse up.
			cont->keys.CheckForDups();
			}
		theHold.Accept(GetString(IDS_RB_EDITKEYINFO));
		}
	ip->RedrawViews(ip->GetTime());
	}



static INT_PTR CALLBACK BaryMorphKeyInfoWndProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BaryMorphKeyInfo *k = (BaryMorphKeyInfo*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			k = (BaryMorphKeyInfo*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			k->Init(hWnd);
			return FALSE;   // DB 2/27

		case WM_PAINT:
			if (!k->valid) {
				k->Update();
				}
			return 0;
		
		case CC_SPINNER_BUTTONDOWN:
			k->SpinnerStart(LOWORD(wParam));
			break;

		case CC_SPINNER_CHANGE:
			k->SpinnerChange(LOWORD(wParam));
			break;

		case CC_SPINNER_BUTTONUP:
			k->SpinnerEnd(LOWORD(wParam),!HIWORD(wParam));
			break;

		case WM_COMMAND:
			k->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);                                               
			break;
		
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			delete k;
			break;

		default:
			return 0;
		}
	return 1;
	}


void BaryMorphCont::EditTrackParams(
		TimeValue t,ParamDimensionBase *dim,TCHAR *pname,HWND hParent,
		IObjParam *ip,DWORD flags)
	{
	HWND hCur = FindOpenMorphKeyWindow(hParent,this);
	if (hCur) {
		SetForegroundWindow(hCur);
		return;
		}
	new BaryMorphKeyInfo(this,ip,hParent);
	}




#endif // NO_CONTROLLER_MORPH
