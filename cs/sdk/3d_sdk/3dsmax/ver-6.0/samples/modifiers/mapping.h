/**********************************************************************
 *<
	FILE: mapping.h

	DESCRIPTION:  

	CREATED BY: Rolf Berteig

	HISTORY: 10/21/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __MAPPING__
#define __MAPPING__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//     WARNING - a copy of this class description is in maxscrpt\maxmods.cpp
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
class MappingMod : public Modifier {
	public:
		DWORD flags;
		float aspect;
		Control *tmControl;
		
		static IObjParam *ip;
		
		MappingMod() : mLocalSetValue(false), flags(0), aspect(1), tmControl(NULL) { } // mjm - 6.7.99
		void* GetInterface(ULONG id);
		void InitControl(ModContext &mc,Object *obj,int type,TimeValue t);
		void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
		void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE);
		void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
		Matrix3 CompMatrix(TimeValue t,ModContext *mc, Matrix3 *ntm,BOOL applySize=TRUE, BOOL applyAxis=TRUE);
		void DoIcon(PolyLineProc& lp,BOOL sel);
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void ViewportAlign();

		virtual void EnterNormalAlign()=0;
		virtual void ExitNormalAlign()=0;
		virtual void EnterRegionFit()=0;
		virtual void ExitRegionFit()=0;
		virtual int GetMapType()=0;
		virtual void SetMapType(int type)=0;
		virtual float GetTile(TimeValue t,int which)=0;
		virtual void SetTile(TimeValue t,int which, float tile)=0;
		virtual BOOL GetFlip(int which)=0;
		virtual void SetFlip(int which,BOOL flip)=0;
		
		virtual float GetLength(TimeValue t)=0;
		virtual float GetWidth(TimeValue t)=0;
		virtual float GetHeight(TimeValue t)=0;
		virtual int GetAxis()=0;
		virtual void SetLength(TimeValue t,float v)=0;
		virtual void SetWidth(TimeValue t,float v)=0;
		virtual void SetHeight(TimeValue t,float v)=0;
		virtual void SetAxis(int v)=0;
		virtual int GetFirstParamVer()=0;
		virtual int GetPBlockVersion()=0;

		virtual void EnterAcquire()=0;
		virtual void ExitAcquire()=0;
		
	protected:
		bool mLocalSetValue; // mjm - 6.7.99
	};

#define I_MAPPINGINTERFACE	0x9836d7f1
#define GetMappingInterface(anim) ((MappingMod*)anim->GetInterface(I_MAPPINGINTERFACE))

#define TM_REF		0
#define PBLOCK_REF	1

// Flags
#define CONTROL_FIT			(1<<0)
#define CONTROL_CENTER		(1<<1)
#define CONTROL_ASPECT		(1<<2)
#define CONTROL_UNIFORM		(1<<3)
#define CONTROL_HOLD		(1<<4)
#define CONTROL_INIT		(1<<5)
#define CONTROL_RESET		(1<<6)	// used by #ifdef USE_SIMPLIFIED_UVWMAP_UI
#define CONTROL_OP			(CONTROL_FIT|CONTROL_CENTER|CONTROL_ASPECT|CONTROL_UNIFORM)
#define CONTROL_INITPARAMS	(1<<10)



#define CID_FACEALIGNMAP 	0x4f298c7a
#define CID_REGIONFIT 		0x4f298c7b

class FaceAlignMouseProc : public MouseCallBack {
	public:
		MappingMod *mod;
		IObjParam *ip;
		FaceAlignMouseProc(MappingMod *m,IObjParam *i) {mod=m;ip=i;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		void FaceAlignMap(HWND hWnd,IPoint2 m);
	};

class FaceAlignMode : public CommandMode {
	public:
		ChangeFGObject fgProc;
		FaceAlignMouseProc proc;
		IObjParam *ip;
		MappingMod *mod;

		FaceAlignMode(MappingMod *m,IObjParam *i) 
			: fgProc(m), proc(m,i) {ip=i;mod=m;}

		int Class() {return MOVE_COMMAND;}		
		int ID() {return CID_FACEALIGNMAP;}
		MouseCallBack *MouseProc(int *numPoints) {*numPoints=2;return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
		BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc() != &fgProc;}
		void EnterMode();
		void ExitMode();
	};

class RegionFitMouseProc : public MouseCallBack {
	public:
		MappingMod *mod;
		IObjParam *ip;
		IPoint2 om;
		RegionFitMouseProc(MappingMod *m,IObjParam *i) {mod=m;ip=i;}
		int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
		void RegionFitMap(HWND hWnd,IPoint2 m);
	};

class RegionFitMode : public CommandMode {
	public:
		ChangeFGObject fgProc;
		RegionFitMouseProc proc;
		IObjParam *ip;
		MappingMod *mod;

		RegionFitMode(MappingMod *m,IObjParam *i) 
			: fgProc(m), proc(m,i) {ip=i;mod=m;}

		int Class() {return MOVE_COMMAND;}		
		int ID() {return CID_REGIONFIT;}
		MouseCallBack *MouseProc(int *numPoints) {*numPoints=2;return &proc;}
		ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
		BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc() != &fgProc;}
		void EnterMode();
		void ExitMode();
	};

class PickAcquire : 
			public PickModeCallback,
			public PickNodeCallback {
	public:		
		MappingMod *mod;
		IObjParam *ip;

		PickAcquire() {mod = NULL; ip = NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(IObjParam *ip,ViewExp *vpt);		
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);		
		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}

		BOOL Filter(INode *node);

		MappingMod *FindFirstMap(ReferenceTarget *ref);
		void AcquireMapping(
			MappingMod *toMod, ModContext *toMC, INode *toNode,
			MappingMod *fromMod, ModContext *fromMC, INode *fromNode,
			int type);
	};

class FixDimsPLCB : public PostLoadCallback {
	public:
		MappingMod *mod;
		FixDimsPLCB(MappingMod *m) {mod=m;}
		void proc(ILoad *iload);
		int Priority() { return 0; } // mjm - 3.29.99 - update for R3
	};


#endif //__MAPPING__
