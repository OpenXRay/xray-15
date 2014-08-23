//
//	app/scene.cpp
//  app/tvpickmulti.cpp -  done
//  app/jagapi.cpp - done
//  app/jagimp.cpp - done
//  app/treevw.h - done
//  core/listctrl.cpp - done
//  core/coremain.h - done
//  maxsdk/include/maxapi.h - done
//  maxsdk/include/plugapi.h - done
//  resmgr/ktx.rc - done

#ifndef __MB__H
#define __MB__H

#include "iparamm2.h"
#include "Simpobj.h"

//#define MASTERBLOCK_SUPER_CLASS_ID	0x64c95999

#define PICKMULTI_FLAG_ANIMATED	 1
#define PICKMULTI_FLAG_VISTRACKS 2

#define BLOCK_CONTROL_CNAME	GetString(IDS_PW_BLOCK)
#define BLOCK_CONTROL_CLASS_ID	Class_ID(0x64c959cf, 0x47df4589)

#define SLAVE_CONTROL_CNAME	GetString(IDS_PW_SLAVE)
#define SLAVE_CONTROL_CLASS_ID	Class_ID(0x64c959cf, 0x47df4590)

#define SLAVEFLOAT_CONTROL_CNAME	GetString(IDS_PW_SLAVEFLOAT)
#define SLAVEFLOAT_CONTROL_CLASS_ID	Class_ID(0x64c959cf, 0x47df4501)

#define SLAVEPOS_CONTROL_CNAME	GetString(IDS_PW_SLAVEPOS)
#define SLAVEPOS_CONTROL_CLASS_ID	Class_ID(0x64c959cf, 0x47df4512)

#define SLAVEROTATION_CONTROL_CNAME	GetString(IDS_PW_SLAVEROTATION)
#define SLAVEROTATION_CONTROL_CLASS_ID	Class_ID(0x64c959cf, 0x47df4523)

#define SLAVESCALE_CONTROL_CNAME	GetString(IDS_PW_SLAVESCALE)
#define SLAVESCALE_CONTROL_CLASS_ID	Class_ID(0x64c959cf, 0x47df4534)


#define SLAVEPOINT3_CONTROL_CNAME	GetString(IDS_PW_SLAVEPOINT3)
#define SLAVEPOINT3_CONTROL_CLASS_ID	Class_ID(0x64c959cf, 0x47df4535)



#define MASTERBLOCK_CONTROL_CNAME	GetString(IDS_PW_MASTERBLOCK)
#define MASTERBLOCK_CONTROL_CLASS_ID	Class_ID(0x64c959cf, 0x47df4578)
#define MASTERBLOCKDLG_CLASS_ID	0xaab659c4
#define SLAVEDLG_CLASS_ID	0xaab659c5

#define CONTROLCONTAINER_CNAME	GetString(IDS_PW_CONTROLCONTAINER)
#define CONTROLCONTAINER_CLASS_ID	Class_ID(0x64c959cf, 0x47df423)


#define BLOCK_PBLOCK_REF 0


#define MASTER_PBLOCK_REF 0


class NameList : public Tab<TSTR*> {
	public:
		void Free() {
			for (int i=0; i<Count(); i++) {
				delete (*this)[i];
				(*this)[i] = NULL;
				}
			}
		void Duplicate() {
			for (int i=0; i<Count(); i++) {
				if ((*this)[i]) (*this)[i] = new TSTR(*(*this)[i]);
				}
			}
	};


class BlockDataClass
{
Tab<Control> c;
//name
//Control *c;
};

class ControlContainerObject ;
class SlaveControl;
//need a container controller

class BlockControl : public Control {
	public:		
//		IParamBlock2* pblock2;
		Interval range;
		BlockControl();
		~BlockControl();

		Color color;
		HWND trackHWND;
//watje 4-30-99
		BOOL suspendNotifies;

		Tab<Control*> controls;
		Tab<Control*> tempControls;
		Tab<SlaveControl*> backPointers;

		Tab<SlaveControl*> externalBackPointers;

		void NotifySlaves();

		
		void RebuildTempControl();
		void AddKeyToTempControl(TimeValue t,  TimeValue scale, BOOL isRelative = TRUE);
//		void AddKeyToSub(Control *sub,int whichSub, TimeValue t,  TimeValue scale, BOOL isRelative = TRUE);
		void AddKeyToSub(Control *sub,int whichSub, TimeValue t,  TimeValue scale, Interval mrange, BOOL isRelative = TRUE);
		
		NameList names;
		TimeValue start,end;
		TimeValue l;
		int propStart;
//		int index;

		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 0;}		
		BOOL IsAnimated() {return TRUE;}

// JBW: direct ParamBlock access is added
//		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
//		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
//		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

		Interval GetTimeRange(DWORD flags)
			{
			Interval iv(0,end - start);
			return iv;
			}

		int NumSubs();  //number of sub tracks in pblock2
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		BOOL AssignController(Animatable *control,int subAnim);
//		int SubNumToRefNum(int subNum);

		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		BOOL CanCopyAnim() {return FALSE;}
		BOOL CanMakeUnique(){return FALSE;}
		BOOL IsReplaceable() { return FALSE;}


		BOOL CanApplyEaseMultCurves() { return FALSE;}

		// Reference methods
//		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
		RefResult NotifyRefChanged(
				Interval changeInt, 
				RefTargetHandle hTarget, 
     			PartID& partID,  
     			RefMessage message);


		// Control methods				
		void Copy(Control *from) {}
		BOOL IsLeaf() {return FALSE;}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}		

		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);

		int TrackParamsType() {return TRACKPARAMS_NONE;}

		
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		Class_ID ClassID() { return BLOCK_CONTROL_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = BLOCK_CONTROL_CNAME;}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);

		void GetValue(TimeValue t, void *val, Interval &valid, int whichSub, GetSetMethod method=CTRL_ABSOLUTE);


		int DeleteControl(int i);
		int AddControl( HWND hWnd);
		Control* BuildSlave(TrackViewPick res,Control* list, BOOL createdList);
		Control* BuildListControl(TrackViewPick res, BOOL &createdList);
		int AddBlockName(ReferenceTarget *anim,ReferenceTarget *client, int subNum, NameList &names);


	};


class BlockControlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { return new BlockControl(); }
	const TCHAR *	ClassName() { return BLOCK_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return BLOCK_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }

	};


#define KEY_FLAGGED     (1<<1)

class BlockKeysClass
{
public:
TimeValue start,end;
int block_id;
BOOL startSelected;
BOOL endSelected;
BOOL relative;
int startFlag,endFlag;
float blend;

};


class MasterBlockControl : public Control {
	public:		
		//float strength[MAX_ELEMS];

//list of all block controls 
//		IParamBlock2* pblock2;
//		Control *cont; // make strength animatable.
		IObjParam *iop;

		void UpdateRange();

		HWND trackHWND;
		Tab<BlockControl*> Blocks;
		Control *blendControl;
		Tab<BlockKeysClass> BlockKeys;
		BOOL relativeHit;
		BOOL startRestoreState, endRestoreState;

		Interval range;
		NameList names;
		BOOL isCurveSelected;
		BOOL rangeUnlocked;

		int CurrentSelectedTrack;
		int AddDialogSelect;
		TimeValue propStart, propEnd;
		BOOL propRelative;
		Color propColor;
		Tab<TrackViewPick> propTargetList;
		TSTR propBlockName;
		NameList propNames;
		int propNamePos;

		ControlContainerObject *propContainer;

		Control* BuildSlave(TrackViewPick res,Control* list, BOOL createdList);
		Control* BuildListControl(TrackViewPick res, BOOL &createdList);
		int ReplaceBlock(HWND hWnd,int whichBlock);
		int AddBlock(HWND hWnd);
		int AddSelected(HWND hWnd);
		int AppendBlock(BlockControl *b, int i, TrackViewPick res,int where=-1);
		int AppendBlock(BlockControl *b,int i, TrackViewPick res, Control *bdata,int where=-1);

//watje 4-24-99
		int AppendBlockNoSlave(BlockControl *b,int i, TSTR *name, Control *bdata);

		int AddBlockName(ReferenceTarget *anim,ReferenceTarget *client, int subNum, NameList &names);
		int AttachAdd(HWND hWnd);
		int AttachAddNullAt(HWND hListWnd, int where);
		int AttachDeleteAt(HWND hListWnd, int where);
		int BuildNewBlock();

		int SaveBlock(int whichBlock);
		int DeleteBlock(int whichBlock);
		int LoadBlock();

		MasterBlockControl();
		

		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 1;}		
		BOOL IsAnimated() {return TRUE;}

		// Paint myself in TrackView
		int PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,Rect& rcPaint,float zoom,int scroll,DWORD flags);
		// Get a little more room in TrackView to paint the curve
		int GetTrackVSpace( int i ) {return 3;}
		int PaintFCurves(ParamDimensionBase *dim,HDC hdc,Rect& rcGraph,	Rect& rcPaint,float tzoom,int tscroll,float vzoom,int vscroll,DWORD flags );



// JBW: direct ParamBlock access is added
//		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
//		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
//		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock
		int SubNumToRefNum(int subNum) { if (subNum == 0)
											return 0;
											else return -1;
										}
		BOOL CanCopyAnim() {return FALSE;}
		BOOL AssignController(Animatable *control,int subAnim);
		BOOL CanApplyEaseMultCurves() { return FALSE;}



		int NumSubs();  //nmumber of blocks
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
//		BOOL AssignController(Animatable *control,int subAnim);
//		int SubNumToRefNum(int subNum);
//int SubNumToRefNum(int subNum) {if (subNum==0) return PATHPOS_PBLOCK_REF; else return -1;}
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefResult NotifyRefChanged(
				Interval changeInt, 
				RefTargetHandle hTarget, 
     			PartID& partID,  
     			RefMessage message);


		BOOL CanMakeUnique() {return FALSE;}

		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
	
		int HitTestTrack(TrackHitTab& hits,
				Rect& rcHit,Rect& rcTrack,
				float zoom,int scroll,DWORD flags);

		int HitTestFCurves(ParamDimensionBase *dim,TrackHitTab& hits,
				Rect& rcHit, Rect& rcGraph,
				float tzoom, int tscroll,
				float vzoom,int vscroll, DWORD flags);

		
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}

		void SelectCurve(BOOL sel);
		BOOL IsCurveSelected();

		// Reference methods
//		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// Control methods				
		void Copy(Control *from) {}
		BOOL IsLeaf() {return TRUE;}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}		
		void HoldTrack();
		
		void UpdateControl(int i);
		// From INoiseControl

		void HoldRange();
		Interval GetTimeRange(DWORD flags) {
				UpdateRange(); 
				return range;}
		void EditTimeRange(Interval range,DWORD flags);
		void MapKeys(TimeMap *map,DWORD flags );

		void GetStrength(TimeValue t,float *strength) {
//			cont->GetValue(t,strength,FOREVER);
			}


		Class_ID ClassID() { return MASTERBLOCK_CONTROL_CLASS_ID; }  
		SClass_ID SuperClassID() { return MASTERBLOCK_SUPER_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = MASTERBLOCK_CONTROL_CNAME;}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);

		void GetValue2(TimeValue t, void *val, Interval &valid, int whichBlock, int whichSub, GetSetMethod method=CTRL_ABSOLUTE);
		void GetValue3(Control *sub, TimeValue t, void *val, Interval &valid, Tab<int> whichBlock, Tab<int> whichSub, Interval localIV, GetSetMethod method=CTRL_ABSOLUTE);

		int NumKeys();
		void CloneSelectedKeys(BOOL offset);
		void DeleteKeys(DWORD flags);
		BOOL IsKeySelected(int index);
		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);
		int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags);
		int GetKeySelState(BitArray &sel,Interval range,DWORD flags);
		BOOL IsKeyAtTime(TimeValue t,DWORD flags);


		void AddKey(TimeValue t, int whichBlock);
		void AddNewKey(TimeValue t,DWORD flags);

		void MoveKeys(ParamDimensionBase *dim,float delta,DWORD flags);
		void SelectKeys(TrackHitTab& sel, DWORD flags);
		void SetSelKeyCoords(TimeValue t, float val,DWORD flags);
		int GetSelKeyCoords(TimeValue &t, float &val,	DWORD flags);
		int NumSelKeys();
		void FlagKey(TrackHitRecord hit);
		int GetFlagKeyIndex();
		TimeValue GetKeyTime(int index) ;
		void SelectKeyByIndex(int i,BOOL sel);


		void MatchNode(Tab<BOOL> selSet, HWND hParent);
		int AttachAddMoveUp(int i);
		int AttachAddMoveDown(int i);
		void AttachAddToList(int where, TrackViewPick res);
		TSTR* GetBlockName(ReferenceTarget *anim,ReferenceTarget *client, int subNum);
//		RecurseSubs(TSTR matchString, Animatable* anim, TrackViewPick& r);
		int RecurseSubs(TSTR matchString, Class_ID pid, Animatable* anim, TrackViewPick& r);
		BOOL IsReplaceable() { return FALSE;}

		void Update(Control *sub, Tab<int> whichBlock, Tab<int> whichSub);


 //		void AddKey();

		// StdControl methods
  };


class MasterBlockClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new MasterBlockControl(); }
	const TCHAR *	ClassName() { return MASTERBLOCK_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return MASTERBLOCK_SUPER_CLASS_ID; }
	Class_ID		ClassID() { return MASTERBLOCK_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.
	const TCHAR*	InternalName() { return _T("MasterBlock"); }			// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	};





class SlaveControl : public Control {
	public:		

		HWND trackHWND;

		SClass_ID superID;
		int propBlockID,propSubID;
		MasterBlockControl *master;
		BOOL masterPresent;
		Control *sub;
		Control *scratchControl;

		Tab<int> blockID;
		Tab<int> subID;


		Interval range;

		SlaveControl();
		~SlaveControl();
		

		// Animatable methods		
//		int IsKeyable() {return sub->IsKeyable();}	
		int IsKeyable() {return FALSE;}	
		
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		void EnumIKParams(IKEnumCallback &callback);
		BOOL CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags);
		void MouseCycleCompleted(TimeValue t);

		BOOL CanApplyEaseMultCurves() { return FALSE;}

		void EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags);
	
		int TrackParamsType() {return TRACKPARAMS_WHOLE;}


		BOOL IsAnimated() {return TRUE;}

// JBW: direct ParamBlock access is added
//		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
//		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
//		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock


		int NumSubs();  //nmumber of blocks
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		void DeleteThis() {delete this;}		

		void AddNewKey(TimeValue t,DWORD flags);
		void CloneSelectedKeys(BOOL offset);
		void DeleteKeys(DWORD flags);
		void SelectKeys(TrackHitTab& sel, DWORD flags);
		BOOL IsKeySelected(int index);

//		int TrackParamsType() {return TRACKPARAMS_NONE;}


		void HoldRange();
		BOOL AssignController(Animatable *control,int subAnim);

	
		void CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags);
		void DeleteKeyAtTime(TimeValue t);
		BOOL IsKeyAtTime(TimeValue t,DWORD flags);
		BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
		int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags);
		int GetKeySelState(BitArray &sel,Interval range,DWORD flags);

//		int SubNumToRefNum(int subNum);
//int SubNumToRefNum(int subNum) {if (subNum==0) return PATHPOS_PBLOCK_REF; else return -1;}
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);


		// Reference methods
//		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
		RefResult NotifyRefChanged(
				Interval changeInt, 
				RefTargetHandle hTarget, 
     			PartID& partID,  
     			RefMessage message);

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// Control methods				
		void Copy(Control *from);// {}
		BOOL IsLeaf() {return TRUE;}
		void CommitValue(TimeValue t) {sub->CommitValue(t);}
		void RestoreValue(TimeValue t) {sub->RestoreValue(t);}		
		BOOL IsReplaceable() {return TRUE;}
		
		// From INoiseControl

//		void HoldRange();
		Interval GetTimeRange(DWORD flags) {
//			if (masterPresent)
//				return master->range;
//			else
//				{
				Interval iv;
				iv.SetEmpty();
				return iv;
//				}
			}
		void EditTimeRange(Interval range,DWORD flags);
		void MapKeys(TimeMap *map,DWORD flags );



		Class_ID ClassID() { return SLAVE_CONTROL_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = SLAVE_CONTROL_CNAME;}

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);


		void RemoveControl(int sel);
		void AddControl(int blockid,int subid );
		void CollapseControl();

		virtual void UpdateSlave() {}

		// StdControl methods
/*
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}		
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);		
		void *CreateTempValue() {return new float;}
		void DeleteTempValue(void *val) {delete (float*)val;}
		void ApplyValue(void *val, void *delta) {*((float*)val) += *((float*)delta);}
		void MultiplyValue(void *val, float m) {*((float*)val) *= m;}
*/
	};

/*
class SlaveClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SlaveControl(); }
	const TCHAR *	ClassName() { return SLAVE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return SLAVE_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.
	const TCHAR*	InternalName() { return _T("SlaveBlock"); }			// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	};
*/

class SlaveFloatControl : public SlaveControl {
	public:		
	
		SlaveFloatControl();
		

		// Animatable methods		
//void DeleteThis() {delete this;}		

//		void SetValue(TimeValue t, void *val, int commit, GetSetMethod method);
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		Class_ID ClassID() { return SLAVEFLOAT_CONTROL_CLASS_ID; }  
//		SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = SLAVEFLOAT_CONTROL_CNAME;}
		void UpdateSlave();
		// Control methods
		RefTargetHandle Clone(RemapDir& remap);


// Reference methods
//RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
	};

class SlaveFloatClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SlaveFloatControl(); }
	const TCHAR *	ClassName() { return SLAVEFLOAT_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return SLAVEFLOAT_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.
	const TCHAR*	InternalName() { return _T("SlaveFloat"); }			// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	};

#ifndef NO_CONTROLLER_SLAVE_POSITION
class SlavePosControl : public SlaveControl {
	public:		

		SlavePosControl();
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		Class_ID ClassID() { return SLAVEPOS_CONTROL_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = SLAVEPOS_CONTROL_CNAME;}
		void UpdateSlave(); 
		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
	};

class SlavePosClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SlavePosControl(); }
	const TCHAR *	ClassName() { return SLAVEPOS_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() { return SLAVEPOS_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.
	const TCHAR*	InternalName() { return _T("SlavePos"); }			// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	};
#endif // NO_CONTROLLER_SLAVE_POSITION


class SlavePoint3Control : public SlaveControl {
	public:		

		SlavePoint3Control();
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);
		Class_ID ClassID() { return SLAVEPOINT3_CONTROL_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_POINT3_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = SLAVEPOINT3_CONTROL_CNAME;}
		void UpdateSlave();
		// Control methods
		RefTargetHandle Clone(RemapDir& remap);
	};


class SlavePoint3ClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SlavePoint3Control(); }
	const TCHAR *	ClassName() { return SLAVEPOINT3_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POINT3_CLASS_ID; }
	Class_ID		ClassID() { return SLAVEPOINT3_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.
	const TCHAR*	InternalName() { return _T("SlavePoint3"); }			// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	};


#ifndef NO_CONTROLLER_SLAVE_ROTATION
class SlaveRotationControl : public SlaveControl {
	public:		

		SlaveRotationControl();
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);

		Class_ID ClassID() { return SLAVEROTATION_CONTROL_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = SLAVEROTATION_CONTROL_CNAME;}
		void UpdateSlave();

		// Control methods
		RefTargetHandle Clone(RemapDir& remap);

	};




class SlaveRotationClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SlaveRotationControl(); }
	const TCHAR *	ClassName() { return SLAVEROTATION_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return SLAVEROTATION_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.
	const TCHAR*	InternalName() { return _T("SlaveRotation"); }			// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	};
#endif // NO_CONTROLLER_SLAVE_ROTATION


#ifndef NO_CONTROLLER_SLAVE_SCALE
class SlaveScaleControl : public SlaveControl {
	public:		

		SlaveScaleControl();
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method);

		Class_ID ClassID() { return SLAVESCALE_CONTROL_CLASS_ID; }  
		SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; } 
		void GetClassName(TSTR& s) {s = SLAVESCALE_CONTROL_CNAME;}
		void UpdateSlave();
		// Control methods
		RefTargetHandle Clone(RemapDir& remap);


	};

class SlaveScaleClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new SlaveScaleControl(); }
	const TCHAR *	ClassName() { return SLAVESCALE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_SCALE_CLASS_ID; }
	Class_ID		ClassID() { return SLAVESCALE_CONTROL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");  }
// JBW: new descriptor data accessors added.
	const TCHAR*	InternalName() { return _T("SlaveScale"); }			// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	};
#endif // NO_CONTROLLER_SLAVE_SCALE


class RangeRestore : public RestoreObj {
	public:
		MasterBlockControl *cont;
		Interval ur, rr;
		RangeRestore(MasterBlockControl *c) 
			{
			cont = c;
			ur   = cont->range;
			}   		
		void Restore(int isUndo) 
			{
			rr = cont->range;
			cont->range = ur;

			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			cont->range = rr;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_RANGE))); }
	};


class MasterBlockRest : public RestoreObj {
	public:
		Tab<BlockKeysClass> undo, redo;
		Interval ur, rr;
		
		MasterBlockControl *cont;

		MasterBlockRest(MasterBlockControl *c) { 
			cont = c;
			undo = c->BlockKeys;
			ur   = cont->range;

			}
		~MasterBlockRest() {}
		void Restore(int isUndo) {
			if (isUndo) {
				if (redo.Count()!=cont->BlockKeys.Count()) {
					redo = cont->BlockKeys;
					rr = cont->range;
					}
				}
			cont->BlockKeys = undo;
			cont->range = ur;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo() {
			cont->BlockKeys = redo;
			cont->range  = rr;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}               
		void EndHold() { 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_RESTORE))); }
	};


class MasterBlockAddKey : public RestoreObj {
	public:
		Tab<BlockKeysClass> undo, redo;
		MasterBlockControl *cont;
		Interval ur, rr;

		MasterBlockAddKey(MasterBlockControl *c) { 
			cont = c;
			undo = c->BlockKeys;
			ur = c->range;
			}
		~MasterBlockAddKey() {}
		void Restore(int isUndo) {
			if (isUndo) {
				if (redo.Count()!=cont->BlockKeys.Count()) {
					redo = cont->BlockKeys;
					}
				rr = cont->range;
				}
			cont->BlockKeys = undo;
			cont->range = ur;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			for (int i = 0; i < cont->Blocks.Count();i++)
				{
				cont->Blocks[i]->NotifySlaves();
				}

			}
		void Redo() {
			cont->BlockKeys = redo;
			cont->range = rr;

			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			for (int i = 0; i < cont->Blocks.Count();i++)
				{
				cont->Blocks[i]->NotifySlaves();
				}

			}               
		void EndHold() { 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_ADDKEY))); }
	};

class MasterBlockDeleteKey : public RestoreObj {
	public:
		Tab<BlockKeysClass> undo, redo;
		MasterBlockControl *cont;
		Interval ur, rr;

		MasterBlockDeleteKey(MasterBlockControl *c) { 
			cont = c;
			undo = c->BlockKeys;
			ur = c->range;
			}
		~MasterBlockDeleteKey() {}
		void Restore(int isUndo) {
			if (isUndo) {
				if (redo.Count()!=cont->BlockKeys.Count()) {
					redo = cont->BlockKeys;
					}
				rr = cont->range;
				}
			cont->BlockKeys = undo;
			cont->range = ur;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			for (int i = 0; i < cont->Blocks.Count();i++)
				{
				cont->Blocks[i]->NotifySlaves();
				}

			}
		void Redo() {
			cont->BlockKeys = redo;
			cont->range = rr;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			for (int i = 0; i < cont->Blocks.Count();i++)
				{
				cont->Blocks[i]->NotifySlaves();
				}

			}               
		void EndHold() { 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_DELETEKEY))); }
	};



class MasterBlockAdd : public RestoreObj {
	public:
		Tab<BlockControl*> undo, redo;
		MasterBlockControl *cont;

		MasterBlockAdd(MasterBlockControl *c) { 
			cont = c;
			undo = c->Blocks;
			}
		~MasterBlockAdd() {}
		void Restore(int isUndo) {
			if (isUndo) {
					redo = cont->Blocks;
					}
			cont->Blocks = undo;
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo() {
			cont->Blocks = redo;

			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}               
		void EndHold() { 
			cont->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_ADDBLOCK))); }
	};


class MasterBlockDlg 
{

//: public ReferenceMaker, public TimeChangeCallback {
	public:
		MasterBlockControl *cont;	
		ParamDimensionBase *dim;
		IObjParam *ip;
		HWND hWnd;
		BOOL valid;
		int elems;

		MasterBlockDlg(
			MasterBlockControl *cont,
			ParamDimensionBase *dim,
			TCHAR *pname,
			IObjParam *ip,
			HWND hParent);
		~MasterBlockDlg();

//		Class_ID ClassID() {return Class_ID(MASTERBLOCKDLG_CLASS_ID,0);}
//		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void MaybeCloseWindow();

//		void TimeChanged(TimeValue t) {Invalidate();}

		void Invalidate();
		void SetupList();
		void SetButtonStates();
		void EnableButtons();

		void Update();
		void SetupUI(HWND hWnd);
		void Change(BOOL redraw=FALSE);
		void WMCommand(int id, int notify, HWND hCtrl);

//		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
//	         PartID& partID,  RefMessage message);
//		int NumRefs() {return 1;}
//		RefTargetHandle GetReference(int i) {return cont;}
//		void SetReference(int i, RefTargetHandle rtarg) {cont=(MasterBlockControl*)rtarg;}
	};

class SlaveDlg : public ReferenceMaker{//, public TimeChangeCallback {
	public:
		SlaveControl *cont;	
		ParamDimensionBase *dim;
		IObjParam *ip;
		HWND hWnd;
		BOOL valid;
		int elems;

		SlaveDlg(
			SlaveControl *cont,
			ParamDimensionBase *dim,
			TCHAR *pname,
			IObjParam *ip,
			HWND hParent);
		~SlaveDlg();

		Class_ID ClassID() {return Class_ID(MASTERBLOCKDLG_CLASS_ID,0);}
		SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

		void MaybeCloseWindow();

//		void TimeChanged(TimeValue t) {Invalidate();}

		void Invalidate();
		void SetupList();
		void SetButtonStates();

		void Update();
		void SetupUI(HWND hWnd);
		void Change(BOOL redraw=FALSE);
		void WMCommand(int id, int notify, HWND hCtrl);

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(SlaveControl*)rtarg;}
	};




class MasterBlockTrackViewFilter :public TrackViewFilter

{
BOOL proc(Animatable *anim, Animatable *client,int subNum);
};

class MasterBlockTrackViewFilterAdd :public TrackViewFilter

{
BOOL proc(Animatable *anim, Animatable *client,int subNum);
};


class MasterTrackViewFilter :public TrackViewFilter

{
BOOL proc(Animatable *anim, Animatable *client,int subNum);
};

class MasterMatchNodeViewFilter :public TrackViewFilter

{
BOOL proc(Animatable *anim, Animatable *client,int subNum);
};


//dummy object to save the data



// JBW: IParamArray has gone since the class variable UI paramters are stored in static ParamBlocks
//      all corresponding class vars have gone, including the ParamMaps since they are replaced 
//      by the new descriptors

// block IDs

enum { container_params };
// geo_param param IDs
enum { container_refs, container_color, container_start, container_end,container_names,container_blockname};

class ControlContainerObject : public SimpleObject2
{
	public:	
		// Class vars
		static IObjParam *ip;

//		IParamBlock2 *pblock2;


// JBW: minimal constructor, call MakeAutoParamBlocks() on my ClassDesc to
//      have all the declared per-instance P_AUTO_CONSTRUCT blocks made, initialized and
//      wired in.
		ControlContainerObject();
//		{ 
//				GetGSphereDesc()->MakeAutoParamBlocks(this); 
//				}
		
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return CONTROLCONTAINER_CNAME; }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist);

		
		// Animatable methods		
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return CONTROLCONTAINER_CLASS_ID; } 
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { 
			return (pblock2->ID() == id) ? pblock2 : NULL; 
			} // return id'd ParamBlock

// JBW: the Load() post-load callback insertion has gone since versioning is 
//		handled automatically by virtue of permanent parameter IDs.  These IDs
//		are defined in enums and are never retired so that old versions can be
//		automatically re-mapped to new ones
//
//      Note that this is only true in new plug-ins; old plug-ins need to 
//		continue to support version re-mapping as before for version up until
//		converting to the new descriptors
//IOResult Load(ILoad *iload);
		
// JBW: all the IParamArray methods are gone since we don't need them for the class variables

		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
// JBW: the GetParamName() and GetParamDim() function have gone	as this all 
//      is available in the descriptors. REFMSG_GET_PARAM_NAME, etc. become unnecessary as well
};

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;


// The class descriptor for gsphere
class ControlContainerClassDesc: public ClassDesc2 
{
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new ControlContainerObject; }
	const TCHAR *	ClassName() { return CONTROLCONTAINER_CNAME; }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return CONTROLCONTAINER_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_PW_HOLDER); }
// JBW:  the ResetClassParams() has gone since this is automatic now
//       using the default values in the descriptors

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("controlContainer"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	  TSTR name;
	};


#endif
