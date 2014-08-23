/**********************************************************************
 *<
	FILE: bomb.cpp

	DESCRIPTION:  A bomb space warp

	CREATED BY: Rolf Berteig

	HISTORY: created 27 July, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"


static int waitPostLoad = 0;

// in mods.cpp
extern HINSTANCE hInstance;

#define PB_STRENGTH		0
#define PB_GRAVITY		1
#define PB_CHAOS		2
#define PB_DETONATION	3
#define PB_SPIN			4
#define PB_FALLOFF		5
#define PB_FALLOFFON	6
#define PB_MINFRAG		7
#define PB_MAXFRAG		8
#define PB_SEED			9


#define OB_REF 		0
#define NODE_REF 	1

inline float Rand1() {return float(rand())/float(RAND_MAX);}

static INT_PTR CALLBACK BombParamProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);	
static void DrawFalloffSphere(float radius, PolyLineProc& lp);

class BombObject: public WSMObject {
	public:	
		IParamBlock *pblock;		
		Mesh mesh;
		
		static HWND hParam, hSot;
		static IObjParam *ip;
		static ISpinnerControl *strengthSpin;	
		static ISpinnerControl *gravSpin;
		static ISpinnerControl *detSpin;
		static ISpinnerControl *chaosSpin;
		static ISpinnerControl *spinSpin;
		static ISpinnerControl *falloffSpin;
		static ISpinnerControl *minFragSpin;
		static ISpinnerControl *maxFragSpin;
		static ISpinnerControl *seedSpin;

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		void UpdateUI(TimeValue t);				
		void BuildMesh(TimeValue t,Mesh &mesh);

	public:
		BombObject();
		~BombObject();

		void SetStrength(TimeValue t, float f);
		void SetGravity(TimeValue t, float f);
		void SetChaos(TimeValue t, float f);
		void SetDetonation(TimeValue t,TimeValue det);
		void SetSpin(TimeValue t, float f);
		void SetFalloff(TimeValue t, float f);
		void SetFalloffOn(TimeValue t,int onOff);
		void SetMinFrag(TimeValue t,int m);
		void SetMaxFrag(TimeValue t,int m);
		void SetSeed(TimeValue t,int m);

		float GetStrength(TimeValue t, Interval& valid = Interval(0,0));
		float GetGravity(TimeValue t, Interval& valid = Interval(0,0));
		float GetChaos(TimeValue t, Interval& valid = Interval(0,0));
		TimeValue GetDetonation(TimeValue t, Interval& valid = Interval(0,0));	
		float GetSpin(TimeValue t, Interval& valid = Interval(0,0));
		float GetFalloff(TimeValue t, Interval& valid = Interval(0,0));
		int GetFalloffOn(TimeValue t, Interval& valid = Interval(0,0));
		int GetMinFrag(TimeValue t, Interval& valid = Interval(0,0));
		int GetMaxFrag(TimeValue t, Interval& valid = Interval(0,0));
		int GetSeed(TimeValue t, Interval& valid = Interval(0,0));

		//  inherited virtual methods:

		// From Animatable
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_BOMBOBJECT_CLASS); }  
		Class_ID ClassID() { return Class_ID(BOMB_OBJECT_CLASS_ID,0); }  		
		void DeleteThis() {delete this;}
		void MapKeys(TimeMap *map,DWORD flags);

		// From ref
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}		
		IOResult Load(ILoad *iload);

		// From BaseObject
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
		TCHAR *GetObjectName() { return GetString(IDS_RB_BOMB);}
		
		// from object
		int IsRenderable() { return FALSE; }
		int DoOwnSelectHilite() { return TRUE; }
		ObjectHandle ApplyTransform(Matrix3& matrix) { return this; }
		Interval ObjectValidity(TimeValue time);
		ObjectState Eval(TimeValue time);
		void InitNodeName(TSTR& s) {s = GetString(IDS_RB_BOMB);}

		// From WSMObject
		Modifier *CreateWSMMod(INode *node);

		
		// from Animatable
		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i) { return pblock; }
		TSTR SubAnimName(int i) { return TSTR(GetString(IDS_RB_PARAMETERS));}
		void EnableControls();

		void RescaleWorldUnits(float f);
	};

HWND BombObject::hParam             		= NULL;
HWND BombObject::hSot             		    = NULL;
IObjParam *BombObject::ip       			= NULL;
ISpinnerControl* BombObject::strengthSpin   = NULL;
ISpinnerControl* BombObject::gravSpin   	= NULL;
ISpinnerControl* BombObject::detSpin  		= NULL;
ISpinnerControl* BombObject::chaosSpin  	= NULL;
ISpinnerControl* BombObject::spinSpin  		= NULL;
ISpinnerControl* BombObject::falloffSpin  	= NULL;
ISpinnerControl* BombObject::minFragSpin  	= NULL;
ISpinnerControl* BombObject::maxFragSpin  	= NULL;
ISpinnerControl* BombObject::seedSpin   	= NULL;

class BombClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new BombObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_BOMB_CLASS); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(BOMB_OBJECT_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetSpaceWarpCatString(SPACEWARP_CAT_GEOMDEF);  }
	};

static BombClassDesc bombDesc;
ClassDesc* GetBombObjDesc() { return &bombDesc; }


class BombMod : public Modifier {
	
	private:
		BombObject  *obRef;
		INode       *nodeRef;		

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );	

	public:
		BombMod();
		BombMod(INode *node,BombObject *obj);
		~BombMod();

		Interval LocalValidity(TimeValue t);
		ChannelMask ChannelsUsed()  { return PART_GEOM|PART_TOPO; }
		ChannelMask ChannelsChanged() { return PART_GEOM|PART_TOPO; }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os,INode *node);
		Class_ID InputType() { return Class_ID(TRIOBJ_CLASS_ID,0); }

		//  inherited virtual methods:

		// From Animatable
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_BOMBMOD); }  
		SClass_ID SuperClassID() { return WSM_CLASS_ID; }
		Class_ID ClassID() { return Class_ID(BOMB_CLASS_ID,0); } 
		void DeleteThis() {delete this;}
		
		// From ref
		int NumRefs() { return 2; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// From BaseObject
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_RB_BOMBBINDING); }
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BombObject *GetWSMObject(TimeValue t);
	};

class BombModClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new BombMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_BOMB_CLASS); }
	SClass_ID		SuperClassID() { return WSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(BOMB_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};

static BombModClassDesc bombModDesc;
ClassDesc* GetBombModDesc() { return &bombModDesc; }


static INT_PTR CALLBACK BombParamProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
	BombObject *bo = (BombObject *)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (!bo && message != WM_INITDIALOG) return FALSE;
	
	switch ( message ) {
		case WM_INITDIALOG:
			bo = (BombObject *)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)bo);			
			
			bo->strengthSpin = GetISpinner(GetDlgItem(hWnd,IDC_STRENGTHSPIN));			
			bo->strengthSpin->SetLimits(-9999999, 9999999, FALSE);
			bo->strengthSpin->SetAutoScale();
			bo->strengthSpin->SetValue(bo->GetStrength(bo->ip->GetTime()),FALSE);
			bo->strengthSpin->LinkToEdit(GetDlgItem(hWnd,IDC_STRENGTH),EDITTYPE_FLOAT);
			
			bo->gravSpin = GetISpinner(GetDlgItem(hWnd,IDC_GRAVITYSPIN));			
			bo->gravSpin->SetLimits( -9999999, 9999999, FALSE);
			bo->gravSpin->SetScale(0.01f);
			bo->gravSpin->SetValue(bo->GetGravity(bo->ip->GetTime()),FALSE);
			bo->gravSpin->LinkToEdit(GetDlgItem(hWnd,IDC_GRAVITY),EDITTYPE_FLOAT);
			
			bo->detSpin = GetISpinner(GetDlgItem(hWnd,IDC_DETONATIONSPIN));
			bo->detSpin->SetLimits(-9999999,9999999,FALSE);
			bo->detSpin->SetScale(10.0f);
			bo->detSpin->SetValue(bo->GetDetonation(bo->ip->GetTime()), FALSE);
			bo->detSpin->LinkToEdit(GetDlgItem(hWnd,IDC_DETONATION),EDITTYPE_TIME);
			
			bo->chaosSpin = GetISpinner(GetDlgItem(hWnd,IDC_BOMB_CHAOSSPIN));
			bo->chaosSpin->SetLimits(0,10,FALSE);
			bo->chaosSpin->SetScale(0.01f);
			bo->chaosSpin->SetValue(bo->GetChaos(bo->ip->GetTime()), FALSE);
			bo->chaosSpin->LinkToEdit(GetDlgItem(hWnd,IDC_BOMB_CHAOS),EDITTYPE_FLOAT);

			bo->spinSpin = GetISpinner(GetDlgItem(hWnd,IDC_SPINSPIN));
			bo->spinSpin->SetLimits(0.0f,9999999.0f,FALSE);
			bo->spinSpin->SetScale(0.01f);			
			bo->spinSpin->LinkToEdit(GetDlgItem(hWnd,IDC_SPIN),EDITTYPE_FLOAT);
			bo->spinSpin->SetValue(bo->GetSpin(bo->ip->GetTime()), FALSE);

			bo->falloffSpin = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFFSPIN));
			bo->falloffSpin->SetLimits(0.0f,9999999.0f,FALSE);
			bo->falloffSpin->SetAutoScale();
			bo->falloffSpin->LinkToEdit(GetDlgItem(hWnd,IDC_FALLOFF),EDITTYPE_UNIVERSE);
			bo->falloffSpin->SetValue(bo->GetFalloff(bo->ip->GetTime()), FALSE);

			bo->minFragSpin = GetISpinner(GetDlgItem(hWnd,IDC_MINFRAGSPIN));
			bo->minFragSpin->SetLimits(1,9999999,FALSE);
			bo->minFragSpin->SetScale(0.1f);
			bo->minFragSpin->LinkToEdit(GetDlgItem(hWnd,IDC_MINFRAG),EDITTYPE_INT);
			bo->minFragSpin->SetValue(bo->GetMinFrag(bo->ip->GetTime()), FALSE);
			
			bo->maxFragSpin = GetISpinner(GetDlgItem(hWnd,IDC_MAXFRAGSPIN));
			bo->maxFragSpin->SetLimits(1,9999999,FALSE);
			bo->maxFragSpin->SetScale(0.1f);
			bo->maxFragSpin->LinkToEdit(GetDlgItem(hWnd,IDC_MAXFRAG),EDITTYPE_INT);
			bo->maxFragSpin->SetValue(bo->GetMaxFrag(bo->ip->GetTime()), FALSE);

			bo->seedSpin = GetISpinner(GetDlgItem(hWnd,IDC_SEEDSPIN));
			bo->seedSpin->SetLimits(0,9999999,FALSE);
			bo->seedSpin->SetScale(0.1f);
			bo->seedSpin->LinkToEdit(GetDlgItem(hWnd,IDC_SEED),EDITTYPE_INT);
			bo->seedSpin->SetValue(bo->GetSeed(bo->ip->GetTime()), FALSE);

			CheckDlgButton(hWnd,IDC_FALLOFF_ON,bo->GetFalloffOn(bo->ip->GetTime()));
			bo->EnableControls();
			return TRUE;

		case WM_DESTROY:
			ReleaseISpinner(bo->strengthSpin);
			ReleaseISpinner(bo->gravSpin);
			ReleaseISpinner(bo->detSpin);			
			ReleaseISpinner(bo->chaosSpin);
			ReleaseISpinner(bo->spinSpin);
			ReleaseISpinner(bo->falloffSpin);
			ReleaseISpinner(bo->minFragSpin);
			ReleaseISpinner(bo->maxFragSpin);
			ReleaseISpinner(bo->seedSpin);
			return FALSE;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;

		case CC_SPINNER_CHANGE:
			if (!theHold.Holding()) theHold.Begin();

			switch (LOWORD(wParam) ) {
				case IDC_STRENGTHSPIN:
					bo->SetStrength(bo->ip->GetTime(),bo->strengthSpin->GetFVal());						
					break;
				case IDC_GRAVITYSPIN:
					bo->SetGravity(bo->ip->GetTime(),bo->gravSpin->GetFVal());
					break;
				case IDC_DETONATIONSPIN:
					bo->SetDetonation(bo->ip->GetTime(),bo->detSpin->GetIVal());
					break;
				case IDC_BOMB_CHAOSSPIN:
					bo->SetChaos(bo->ip->GetTime(),bo->chaosSpin->GetFVal());
					break;

				case IDC_SPINSPIN:
					bo->SetSpin(bo->ip->GetTime(),bo->spinSpin->GetFVal());
					break;

				case IDC_FALLOFFSPIN:
					bo->SetFalloff(bo->ip->GetTime(),bo->falloffSpin->GetFVal());
					break;

				case IDC_MINFRAGSPIN:
					bo->SetMinFrag(bo->ip->GetTime(),bo->minFragSpin->GetIVal());
					break;

				case IDC_MAXFRAGSPIN:
					bo->SetMaxFrag(bo->ip->GetTime(),bo->maxFragSpin->GetIVal());
					break;

				case IDC_SEEDSPIN:
					bo->SetSeed(bo->ip->GetTime(),bo->seedSpin->GetIVal());
					break;

				}
			bo->ip->RedrawViews(bo->ip->GetTime(),REDRAW_INTERACTIVE);
			return TRUE;
		
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (!HIWORD(wParam) && message!=WM_CUSTEDIT_ENTER) {
				theHold.Cancel();
			} else {
				theHold.Accept(GetString (IDS_PARAM_CHANGE));				
				}
			bo->ip->RedrawViews(bo->ip->GetTime(),REDRAW_END);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_FALLOFF_ON:
					bo->SetFalloffOn(bo->ip->GetTime(),IsDlgButtonChecked(hWnd,IDC_FALLOFF_ON));
					bo->ip->RedrawViews(bo->ip->GetTime(),REDRAW_END);
					bo->EnableControls();
					break;
				}
			return TRUE;
		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			bo->ip->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;

		default:
			return FALSE;
		}
	}


Modifier *BombObject::CreateWSMMod(INode *node)
	{
	return new BombMod(node,this);
	}


void BombObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	this->ip = ip;
	
	if (!hParam) {
		hSot = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_BOMB_SOT),
				DefaultSOTProc,
				GetString(IDS_RB_SOT), 
				(LPARAM)ip,APPENDROLL_CLOSED);

		hParam = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_BOMBPARAMS),
				BombParamProc,
				GetString(IDS_RB_BOMBPARAMS), 
				(LPARAM)this );
	} else {
		SetWindowLongPtr(hParam, GWLP_USERDATA, (LONG_PTR)this);		

		// Init the dialog to our values.
		strengthSpin->SetValue(GetStrength(ip->GetTime()),FALSE);
		gravSpin->SetValue(GetGravity(ip->GetTime()),FALSE );
		chaosSpin->SetValue(GetChaos(ip->GetTime()),FALSE );
		detSpin->SetValue(GetDetonation(ip->GetTime()),FALSE );
		spinSpin->SetValue(GetSpin(ip->GetTime()),FALSE);
		falloffSpin->SetValue(GetFalloff(ip->GetTime()),FALSE);
		minFragSpin->SetValue(GetMinFrag(ip->GetTime()),FALSE);
		maxFragSpin->SetValue(GetMaxFrag(ip->GetTime()),FALSE);
		seedSpin->SetValue(GetSeed(ip->GetTime()),FALSE);
		CheckDlgButton(hParam,IDC_FALLOFF_ON,GetFalloffOn(ip->GetTime()));
		}

	if (GetFalloffOn(ip->GetTime())) {
		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime());
		}
	}

void BombObject::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
	{
	if (flags&END_EDIT_REMOVEUI) {		
		ip->DeleteRollupPage(hParam);
		ip->DeleteRollupPage(hSot);
		hParam = NULL;
		hSot   = NULL;
	} else {		
		SetWindowLongPtr(hParam, GWLP_USERDATA, 0);		
		}
	
	if (GetFalloffOn(ip->GetTime())) {
		NotifyDependents(FOREVER,0,REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime());
		}
	this->ip = NULL;
	}


// The current version of bomb
#define BOMB_VERSION	2

// parameters for version 0
static ParamBlockDescID descV0[] = {
		{ TYPE_FLOAT, NULL, TRUE, 1 },
		{ TYPE_FLOAT, NULL, TRUE, 2 },
		{ TYPE_INT, NULL, FALSE, 3 }};

// parameters for version 1
static ParamBlockDescID descV1[] = {
		{ TYPE_FLOAT, NULL, TRUE, 1 },
		{ TYPE_FLOAT, NULL, TRUE, 2 },
		{ TYPE_FLOAT, NULL, TRUE, 4 },
		{ TYPE_INT, NULL, FALSE, 3 }};

// parameters for version 2
static ParamBlockDescID descV2[] = {
		{ TYPE_FLOAT, NULL, TRUE, 1 },
		{ TYPE_FLOAT, NULL, TRUE, 2 },
		{ TYPE_FLOAT, NULL, TRUE, 4 },
		{ TYPE_INT, NULL, FALSE, 3 },
		{ TYPE_FLOAT, NULL, TRUE, 5 }, // spin
		{ TYPE_FLOAT, NULL, TRUE, 6 }, // falloff
		{ TYPE_INT, NULL, FALSE, 7 },  // falloff on
		{ TYPE_INT, NULL, FALSE, 8 },  // min frag
		{ TYPE_INT, NULL, FALSE, 9 },  // max frag
		{ TYPE_INT, NULL, FALSE, 10 },  // seed
	};

		
BombObject::BombObject()
	{
	MakeRefByID(FOREVER,0,CreateParameterBlock(descV2,10,BOMB_VERSION));	
	pblock->SetValue(PB_STRENGTH, TimeValue(0), 1.0f);
	pblock->SetValue(PB_GRAVITY, TimeValue(0), 1.0f);
	pblock->SetValue(PB_DETONATION, TimeValue(0), 800); 
	pblock->SetValue(PB_MINFRAG, TimeValue(0), 1); 
	pblock->SetValue(PB_MAXFRAG, TimeValue(0), 1); 
	pblock->SetValue(PB_FALLOFF, TimeValue(0), 100.0f); 
	BuildMesh(TimeValue(0),mesh);
	mesh.EnableEdgeList(1);
	}


class BombPostLoad : public PostLoadCallback {
	public:
		BombObject *bo;
		BombPostLoad(BombObject *b) {bo=b;}
		void proc(ILoad *iload) {			
			if (bo->pblock->GetVersion()!=BOMB_VERSION) {
				switch (bo->pblock->GetVersion()) {
					case 0:
						bo->ReplaceReference(0,
							UpdateParameterBlock(
								descV0, 3, bo->pblock,
								descV1, 4, BOMB_VERSION));
						iload->SetObsolete();
						break;

					case 1:
						bo->ReplaceReference(0,
							UpdateParameterBlock(
								descV1, 4, bo->pblock,
								descV2, 10, BOMB_VERSION));
						iload->SetObsolete();
						break;

					default:
						assert(0);
						break;
					}
				}
			waitPostLoad--;
			delete this;
			}
	};

IOResult BombObject::Load(ILoad *iload) 
	{	
	waitPostLoad++;
	iload->RegisterPostLoadCallback(new BombPostLoad(this));
	return IO_OK;
	}



BombObject::~BombObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	}


void BombObject::RescaleWorldUnits(float f)
	{
	if (TestAFlag(A_WORK1))
		return;
	SetAFlag(A_WORK1);
	WSMObject::RescaleWorldUnits(f);
	pblock->RescaleParam(PB_STRENGTH,f*f);	
	pblock->RescaleParam(PB_GRAVITY,f);	

	}


void BombObject::EnableControls()
	{
	if (falloffSpin)
		{
		BOOL fallOffOn;
		pblock->GetValue(PB_FALLOFFON, 0, fallOffOn, FOREVER);
		falloffSpin->Enable(fallOffOn);
		}
	}

void BombObject::SetStrength(TimeValue t,float f)
	{
	pblock->SetValue(PB_STRENGTH, t, f);		
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void BombObject::SetGravity(TimeValue t, float f)
	{
	pblock->SetValue(PB_GRAVITY, t, f);		
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void BombObject::SetChaos(TimeValue t, float f)
	{
	pblock->SetValue(PB_CHAOS, t, f);		
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void BombObject::SetDetonation(TimeValue t,TimeValue det)
	{
	pblock->SetValue(PB_DETONATION, t, det);		
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void BombObject::SetSpin(TimeValue t, float f)
	{
	pblock->SetValue(PB_SPIN, t, f);	
	}

void BombObject::SetFalloff(TimeValue t, float f)
	{
	pblock->SetValue(PB_FALLOFF, t, f);
	}

void BombObject::SetFalloffOn(TimeValue t,int onOff)
	{
	pblock->SetValue(PB_FALLOFFON, t, onOff);
	}

void BombObject::SetMinFrag(TimeValue t,int m)
	{
	pblock->SetValue(PB_MINFRAG, t, m);
	}

void BombObject::SetMaxFrag(TimeValue t,int m)
	{
	pblock->SetValue(PB_MAXFRAG, t, m);
	}

void BombObject::SetSeed(TimeValue t,int m)
	{
	pblock->SetValue(PB_SEED, t, m);
	}

float BombObject::GetStrength(TimeValue t, Interval& valid)
	{
	float f;
	pblock->GetValue(PB_STRENGTH, t, f, valid);
	return f;
	}

float BombObject::GetGravity(TimeValue t, Interval& valid)
	{
	float f;
	pblock->GetValue(PB_GRAVITY, t, f, valid);
	return f;
	}

float BombObject::GetChaos(TimeValue t, Interval& valid)
	{
	float f;
	pblock->GetValue(PB_CHAOS, t, f, valid);
	return f;
	}

TimeValue BombObject::GetDetonation(TimeValue t, Interval& valid)
	{
	int det;
	pblock->GetValue(PB_DETONATION, t, det, valid);
	return TimeValue(det);
	}

float BombObject::GetSpin(TimeValue t, Interval& valid)
	{
	float f;
	pblock->GetValue(PB_SPIN, t, f, valid);
	return f;
	}

float BombObject::GetFalloff(TimeValue t, Interval& valid)
	{
	float f;
	pblock->GetValue(PB_FALLOFF, t, f, valid);
	return f;
	}

int BombObject::GetFalloffOn(TimeValue t, Interval& valid)
	{
	int i;
	pblock->GetValue(PB_FALLOFFON, t, i, valid);
	return i;
	}

int BombObject::GetMinFrag(TimeValue t, Interval& valid)
	{
	int i;
	pblock->GetValue(PB_MINFRAG, t, i, valid);
	return i;
	}

int BombObject::GetMaxFrag(TimeValue t, Interval& valid)
	{
	int i;
	pblock->GetValue(PB_MAXFRAG, t, i, valid);
	return i;
	}

int BombObject::GetSeed(TimeValue t, Interval& valid)
	{
	int i;
	pblock->GetValue(PB_SEED, t, i, valid);
	return i;
	}

void BombObject::MapKeys(TimeMap *map,DWORD flags)
	{
	TimeValue det = GetDetonation(0);
	det = map->map(det);
	SetDetonation(0,det);
	}

static void MakeTri(Face *f, int a,  int b , int c) {
	f[0].setVerts(a,b,c);
	f[0].setSmGroup(0);
	f[0].setEdgeVisFlags(1,1,1);	
	}


class BombMtl: public Material {
	public:
	BombMtl();
	};
static BombMtl swMtl;

#define BOMB_R	float(.7)
#define BOMB_G	float(0)
#define BOMB_B	float(0)

BombMtl::BombMtl():Material() {
	Kd[0] = BOMB_R;
	Kd[1] = BOMB_G;
	Kd[2] = BOMB_B;
	Ks[0] = BOMB_R;
	Ks[1] = BOMB_G;
	Ks[2] = BOMB_B;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME|GW_BACKCULL;
	selfIllum = (float)1.0;
	}


void BombObject::BuildMesh(TimeValue t,Mesh &mesh)
	{		
	float s = 10.0f;
	mesh.setNumVerts(4);
	mesh.setNumFaces(4);
	mesh.setVert(0,s*Point3(0.0f,1.0f,0.0f));
	mesh.setVert(1,s*Point3(-float(cos(DegToRad(30.0f))),-float(sin(DegToRad(30.0f))),0.0f));
	mesh.setVert(2,s*Point3(float(cos(DegToRad(30.0f))),-float(sin(DegToRad(30.0f))),0.0f));
	mesh.setVert(3,s*Point3(0.0f,0.0f,1.0f));
	MakeTri(&(mesh.faces[0]),2,1,0);
	MakeTri(&(mesh.faces[1]),3,0,1);
	MakeTri(&(mesh.faces[2]),3,1,2);
	MakeTri(&(mesh.faces[3]),3,2,0);
	mesh.InvalidateGeomCache();	
	}


int BombObject::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
	{		
	Point2 pt( (float)p[0].x, (float)p[0].y );
	HitRegion hitRegion;
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = &swMtl; 
	Matrix3 mat = inode->GetObjectTM(t);	
	gw->setTransform(mat);

	MakeHitRegion(hitRegion, type, crossing, 4, p);

	return mesh.select(gw, mtl, &hitRegion, flags & HIT_ABORTONHIT);
	}

void BombObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) {
	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();	   	
	gw->setTransform(tm);
	mesh.snap(gw, snap, p, tm);
	}

int BombObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
	{
	GraphicsWindow *gw = vpt->getGW();
	Material *mtl = &swMtl;
	Matrix3 mat = inode->GetObjectTM(t);
 	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|/*GW_BACKCULL|*/ (rlim&GW_Z_BUFFER?GW_Z_BUFFER:0) );//removed BC 2/16/99 DB

	gw->setTransform(mat);
	if (inode->Selected()) 
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!inode->IsFrozen())
		//gw->setColor( LINE_COLOR, swMtl.Kd[0], swMtl.Kd[1], swMtl.Kd[2]);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SPACE_WARPS));
	mesh.render(gw, mtl, NULL, COMP_ALL);
		
	if (hParam && GetWindowLongPtr(hParam,GWLP_USERDATA)==(LONG_PTR)this &&
		GetFalloffOn(t)) {
		DrawLineProc lp(gw);
		DrawFalloffSphere(GetFalloff(t),lp);
		}

	gw->setRndLimits(rlim);
	return(0);
	}

class BombObjCreateCallBack: public CreateMouseCallBack {	
	BombObject *ob;	
	public:
		int proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		void SetObj(BombObject *obj) { ob = obj; }
	};

int BombObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 pt;

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:  // only happens with MOUSE_POINT msg
				pt = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE); //vpt->GetPointOnCP(m);
				mat.SetTrans(pt);
				break;
			case 1:								
				pt = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE); //vpt->GetPointOnCP(m);
				mat.SetTrans(pt);
				if (msg==MOUSE_POINT) 
					return CREATE_STOP;
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT)
		return CREATE_ABORT;
	else
	if (msg == MOUSE_FREEMOVE) {
		vpt->SnapPreview(m,m);
		}

	return TRUE;
	}

static BombObjCreateCallBack bombCreateCB;


CreateMouseCallBack* BombObject::GetCreateMouseCallBack()
	{
	bombCreateCB.SetObj(this);
	return &bombCreateCB;
	}


RefTargetHandle BombObject::Clone(RemapDir& remap) {
	BombObject* newob = new BombObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newob, remap);
	return(newob);
	}

void BombObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
	{
	Box3 meshBox;
	Matrix3 mat = inode->GetObjectTM(t);
	box.Init();
	if (hParam && GetWindowLongPtr(hParam,GWLP_USERDATA)==(LONG_PTR)this &&
		GetFalloffOn(t)) {
		BoxLineProc bproc(&mat);
		DrawFalloffSphere(GetFalloff(t),bproc);
		box = bproc.Box();
		}
	GetLocalBoundBox(t,inode,vpt,meshBox);	
	for(int i = 0; i < 8; i++)
		box += mat * meshBox[i];
	}

void BombObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
	{		
	box = mesh.getBoundingBox();
	}

void BombObject::UpdateUI(TimeValue t)
	{
	if (hParam && !waitPostLoad) {
		if (this == (BombObject *)GetWindowLongPtr(hParam,GWLP_USERDATA)) {
			strengthSpin->SetValue(GetStrength(t),FALSE);
			gravSpin->SetValue(GetGravity(t),FALSE);
			chaosSpin->SetValue(GetChaos(t),FALSE);			
			falloffSpin->SetValue(GetFalloff(t),FALSE);
			spinSpin->SetValue(GetSpin(t),FALSE);

			if (pblock->KeyFrameAtTime(PB_STRENGTH,t))
				 strengthSpin->SetKeyBrackets(TRUE);
			else strengthSpin->SetKeyBrackets(FALSE);

			if (pblock->KeyFrameAtTime(PB_GRAVITY,t))
				 gravSpin->SetKeyBrackets(TRUE);
			else gravSpin->SetKeyBrackets(FALSE);

			if (pblock->KeyFrameAtTime(PB_CHAOS,t))
				 chaosSpin->SetKeyBrackets(TRUE);
			else chaosSpin->SetKeyBrackets(FALSE);

			if (pblock->KeyFrameAtTime(PB_SPIN,t))
				 spinSpin->SetKeyBrackets(TRUE);
			else spinSpin->SetKeyBrackets(FALSE);

			if (pblock->KeyFrameAtTime(PB_FALLOFF,t))
				 falloffSpin->SetKeyBrackets(TRUE);
			else falloffSpin->SetKeyBrackets(FALSE);
			}
		}
	}

ObjectState BombObject::Eval(TimeValue time)	
	{	
	return ObjectState(this);
	}

Interval BombObject::ObjectValidity(TimeValue time) 
	{	
	Interval ivalid = FOREVER;
	if (!waitPostLoad) {
		GetStrength(time,ivalid);
		GetGravity(time,ivalid);	
		GetFalloff(time,ivalid);
		GetChaos(time,ivalid);
		GetSpin(time,ivalid);
		UpdateUI(time);
		}
	return ivalid;	
	}


RefResult BombObject::NotifyRefChanged( 
		Interval changeInt,
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message )
	{					
	switch (message) {				
		case REFMSG_CHANGE:
			if (ip) UpdateUI(ip->GetTime());
			break;

		case REFMSG_GET_PARAM_DIM: { 
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_STRENGTH:
					gpd->dim = stdNormalizedDim;
					break;
				case PB_GRAVITY:
					gpd->dim = stdNormalizedDim;
					break;	
				case PB_CHAOS:
					gpd->dim = stdNormalizedDim;
					break;	
				case PB_DETONATION:
					gpd->dim = stdTimeDim;
					break;
				case PB_SPIN:
					gpd->dim = defaultDim;
					break;
				case PB_FALLOFF:
					gpd->dim = stdWorldDim;
					break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_STRENGTH:
					gpn->name = GetString(IDS_RB_STRENGTH2);
					break;
				case PB_GRAVITY:
					gpn->name = GetString(IDS_RB_GRAVITY);
					break;	
				case PB_CHAOS:
					gpn->name = GetString(IDS_RB_CHAOS);
					break;	
				case PB_DETONATION:
					gpn->name = GetString(IDS_RB_DETONATION);
					break;
				case PB_SPIN:
					gpn->name = GetString(IDS_RB_SPIN);
					break;
				case PB_FALLOFF:
					gpn->name = GetString(IDS_RB_FALLOFF);
					break;
				}
			return REF_STOP; 
			}
		}
	return REF_SUCCEED;
	}




/*----------------------------------------------------------------*/
// Bomb modifier


void BombMod::BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev )
	{	
	// aszabo|feb.05.02
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	}

void BombMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{	
	// aszabo|feb.05.02
	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	}


RefTargetHandle BombMod::GetReference(int i) {
	switch(i) {
		case OB_REF: return (RefTargetHandle)obRef;
		case NODE_REF: return (RefTargetHandle)nodeRef;		
		default: return NULL;
		}
	}

void BombMod::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case OB_REF: obRef = (BombObject *)rtarg; return;
		case NODE_REF: nodeRef = (INode *)rtarg;  return;		
		}
	}

BombMod::BombMod()
	{
	obRef   = NULL;
	nodeRef = NULL;	
	}

BombMod::BombMod(INode *node,BombObject *obj)
	{	
	MakeRefByID(FOREVER,OB_REF,obj);
	MakeRefByID(FOREVER,NODE_REF,node);	
	}

BombMod::~BombMod()
	{
	DeleteAllRefsFromMe();	
	}

BombObject *BombMod::GetWSMObject(TimeValue t)
	{
	if (nodeRef) {
		ObjectState os = nodeRef->EvalWorldState(t);
		assert(os.obj && os.obj->SuperClassID()==WSM_OBJECT_CLASS_ID);
		return (BombObject*)os.obj;		
	} else {
		return NULL;
		}
	}

Interval BombMod::LocalValidity(TimeValue t)
	{
	BombObject *obj = GetWSMObject(t);

	if (obj && nodeRef && !waitPostLoad) {
		// Force a cache of input if being edited.
		if (TestAFlag(A_MOD_BEING_EDITED))
			return NEVER;  
		Interval valid = FOREVER;
		Matrix3 tm;		
		obj->GetStrength(t,valid);
		obj->GetGravity(t,valid);
		obj->GetDetonation(t,valid);		
		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	} else {
		return FOREVER;
		}
	}


#define GRAVITY_CONSTANT	(-0.0001f * (1200.0f*1200.0f)/(float(TIME_TICKSPERSEC)*float(TIME_TICKSPERSEC)))
#define STRENGTH_CONSTANT	(5.0f * (1200.0f/float(TIME_TICKSPERSEC)))

#define CHAOS		float(0.5)
#define CHAOSBASE	(float(1)-CHAOS/2)

#define UNDEFINED	0xffffffff

void BombMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{	
	BombObject *bobj = GetWSMObject(t);

	if (bobj && nodeRef) {
		assert(os->obj->IsSubClassOf(triObjectClassID));
		TriObject *triOb = (TriObject *)os->obj;
		Interval valid = FOREVER;

		if (os->GetTM()) {
			Matrix3 tm = *(os->GetTM());
			for (int i=0; i<triOb->GetMesh().getNumVerts(); i++) {
				triOb->GetMesh().verts[i] = triOb->GetMesh().verts[i] * tm;
				}			
			os->obj->UpdateValidity(GEOM_CHAN_NUM,os->tmValid());
			os->SetTM(NULL,FOREVER);
			}
		
		if (waitPostLoad) {
			valid.SetInstant(t);
			triOb->UpdateValidity(GEOM_CHAN_NUM,valid);
			triOb->UpdateValidity(TOPO_CHAN_NUM,valid);
			return;
			}		

		Matrix3 tm;		
		TimeValue det  = bobj->GetDetonation(t,valid);	
		float strength = bobj->GetStrength(t,valid) * STRENGTH_CONSTANT;
		float grav     = bobj->GetGravity(t,valid);		
		float chaos    = bobj->GetChaos(t,valid);
		float chaosBase = (float(1)-chaos/2);
		float rotSpeed = bobj->GetSpin(t,valid) * TWOPI/float(TIME_TICKSPERSEC);
		int minClust = bobj->GetMinFrag(t,valid), maxClust = bobj->GetMaxFrag(t,valid);
		if (minClust<1) minClust = 1;
		if (maxClust<1) maxClust = 1;
		int clustVar = maxClust-minClust+1;
		float falloff = bobj->GetFalloff(t,valid);
		int useFalloff = bobj->GetFalloffOn(t,valid);
		int seed = bobj->GetSeed(t,valid);		

		//tm = nodeRef->GetNodeTM(t,&valid);		
		tm = nodeRef->GetObjectTM(t,&valid);
		
		if (t<det) {
			valid.Set(TIME_NegInfinity,det-1);
			triOb->UpdateValidity(GEOM_CHAN_NUM,valid);
			triOb->UpdateValidity(TOPO_CHAN_NUM,valid);
			triOb->PointsWereChanged();		
			return;		
			}

		float dt = float(t-det);
		valid.SetInstant(t);

		int n0=0,n1=1,n2=2,nv;
		Point3 *verts, v, p0, p1, g(0.0f,0.0f,grav*GRAVITY_CONSTANT);
		Face *f = triOb->GetMesh().faces;
		float dot;		

		Mesh &mesh = triOb->GetMesh();

		// First, segment the faces.
		srand((unsigned int)seed);		
		Tab<DWORD> vclust;
		Tab<DWORD> vmap;
		Tab<Point3> nverts;
		vclust.SetCount(mesh.getNumVerts());
		vmap.SetCount(mesh.getNumVerts());		
		int numClust = 0;

		if (minClust==1 && maxClust==1) {
			// Simple case... 1 face per cluster
			nv = triOb->GetMesh().getNumFaces() * 3;
			verts = new Point3[nv];
			vclust.SetCount(nv);
			for (int i=0; i<nv; i++) {
				vclust[i] = i/3;
				}
			int j=0;
			for (i=0; i<mesh.getNumFaces(); i++) {
				verts[j]   = mesh.verts[mesh.faces[i].v[0]];
				verts[j+1] = mesh.verts[mesh.faces[i].v[1]];
				verts[j+2] = mesh.verts[mesh.faces[i].v[2]];
				mesh.faces[i].v[0] = j;
				mesh.faces[i].v[1] = j+1;
				mesh.faces[i].v[2] = j+2;
				j += 3;
				}
			numClust = triOb->GetMesh().getNumFaces();
		} else {
			// More complex case... clusters are randomely sized
			for (int i=0; i<mesh.getNumVerts(); i++) {
				vclust[i] = UNDEFINED;
				vmap[i] = i;
				}
			int cnum = 0;
			for (i=0; i<mesh.getNumFaces(); ) {
				int csize = int(Rand1()*float(clustVar)+float(minClust));
				if (i+csize>mesh.getNumFaces()) {
					csize = mesh.getNumFaces()-i;					
					}
				
				// Make sure each face in the cluster has at least 2 verts in common with another face.
				BOOL verified = FALSE;
				while (!verified) {
					verified = TRUE;
					if (csize<2) break;

					for (int j=0; j<csize; j++) {
						BOOL match = FALSE;
						for (int k=0; k<csize; k++) {
							if (k==j) continue;
							int common = 0;
							for (int i1=0; i1<3; i1++) {
								for (int i2=0; i2<3; i2++) {
									if (mesh.faces[i+j].v[i1]==
										mesh.faces[i+k].v[i2]) common++;
									}
								}
							if (common>=2) {
								match = TRUE;
								break;
								}
							}
						if (!match) {
							csize = j;
							verified = FALSE; // Have to check again
							break;
							}
						}
					}
				if (csize==0) csize = 1;

				// Clear the vert map
				for (int j=0; j<mesh.getNumVerts(); j++) vmap[j] = UNDEFINED;			

				// Go through the cluster and remap verts.
				for (j=0;j<csize; j++) {
					for (int k=0; k<3; k++) {
						if (vclust[mesh.faces[i+j].v[k]]==UNDEFINED) {
							vclust[mesh.faces[i+j].v[k]] = cnum;
						} else
						if (vclust[mesh.faces[i+j].v[k]]!=(DWORD)cnum) {
							if (vmap[mesh.faces[i+j].v[k]]==UNDEFINED) {
								vclust.Append(1,(DWORD*)&cnum,50);
								nverts.Append(1,&mesh.verts[mesh.faces[i+j].v[k]],50);
								mesh.faces[i+j].v[k] =
									vmap[mesh.faces[i+j].v[k]] = 
										mesh.getNumVerts()+nverts.Count()-1;
							} else {
								mesh.faces[i+j].v[k] =
									vmap[mesh.faces[i+j].v[k]];
								}
							}
						}
					}
				
				cnum++;
				numClust++;
				i += csize;
				}

			nv = mesh.getNumVerts() + nverts.Count();
			verts = new Point3[nv];
			for (i=0; i<mesh.getNumVerts(); i++) {
				verts[i] = mesh.verts[i];
				}
			for (   ; i<mesh.getNumVerts()+nverts.Count(); i++) {
				verts[i] = nverts[i-mesh.getNumVerts()];
				}
			}

		// Find the center of all clusters
		Tab<Point3> clustCent;
		Tab<DWORD> clustCounts;
		clustCent.SetCount(numClust);
		clustCounts.SetCount(numClust);
		for (int i=0; i<numClust; i++) {
			clustCent[i]   = Point3(0,0,0);
			clustCounts[i] = 0;
			}
		for (i=0; i<nv; i++) {
			if (vclust[i]==UNDEFINED) continue;
			clustCent[vclust[i]] += verts[i];
			clustCounts[vclust[i]]++;
			}

		// Build transformations for all clusters
		Tab<Matrix3> mats;
		mats.SetCount(numClust);
		srand((unsigned int)seed);
		for (i=0; i<numClust; i++) {
			if (clustCounts[i]) clustCent[i] /= float(clustCounts[i]);

			v   = clustCent[i] - tm.GetTrans();
			float u = 1.0f;
			if (useFalloff) {
				u = 1.0f - Length(v)/falloff;
				if (u<0.0f) u = 0.0f;
				}
			dot = DotProd(v,v);
			if (dot==0.0f) dot = 0.000001f;
			v  = v / dot * strength * u;
			v.x *= chaosBase + chaos * Rand1();
			v.y *= chaosBase + chaos * Rand1();
			v.z *= chaosBase + chaos * Rand1();
			p1 = v*dt + 0.5f*g*dt*dt; // projectile motion

			// Set rotation
			Point3 axis;
			axis.x = -1.0f + 2.0f*Rand1();
			axis.y = -1.0f + 2.0f*Rand1();
			axis.z = -1.0f + 2.0f*Rand1();
			axis = Normalize(axis);
			float angle = dt*rotSpeed*(chaosBase + chaos * Rand1())*u;
			Quat q = QFromAngAxis(angle, axis);
			q.MakeMatrix(mats[i]);
			
			mats[i].PreTranslate(-clustCent[i]);
			mats[i].Translate(clustCent[i]+p1);			
			}

		// Now transform the clusters
		for (i=0; i<nv; i++) {
			if (vclust[i]==UNDEFINED) continue;
			verts[i] = verts[i] * mats[vclust[i]];
			}
			
		/*
		// Seed the random number generator with num verts.
		srand((unsigned int)nv);		
		
		for (int i=0; i<triOb->mesh.getNumFaces(); i++,f++) {			
			p0  = verts[n0] = triOb->mesh.verts[f->v[0]];
			p0 += verts[n1] = triOb->mesh.verts[f->v[1]];
			p0 += verts[n2] = triOb->mesh.verts[f->v[2]];
			p0 /= 3.0f;
			v   = p0 - tm.GetTrans();
			dot = DotProd(v,v);
			if (dot==0.0f) dot = 0.000001f;
			v  = v / dot * strength;
			v.x *= chaosBase + chaos * Rand1();
			v.y *= chaosBase + chaos * Rand1();
			v.z *= chaosBase + chaos * Rand1();
			p1 = v*dt + 0.5f*g*dt*dt; // projectile motion

			verts[n0] += p1;
			verts[n1] += p1;
			verts[n2] += p1;

			f->setVerts(n0,n1,n2);
			f->setEdgeVisFlags(1,1,1);
			n0 += 3;
			n1 += 3;
			n2 += 3;
			}
		*/
		
		triOb->UpdateValidity(GEOM_CHAN_NUM,valid);
		triOb->UpdateValidity(TOPO_CHAN_NUM,valid);
		triOb->PointsWereChanged();		

		delete[] triOb->GetMesh().verts;
		triOb->GetMesh().verts    = verts;
		triOb->GetMesh().numVerts = nv;
		triOb->GetMesh().vertSel.SetSize(nv,TRUE);
		triOb->GetMesh().InvalidateTopologyCache ();
		}		 
	}

RefTargetHandle BombMod::Clone(RemapDir& remap) 
	{
	BombMod *newob = new BombMod(nodeRef,obRef);		
	BaseClone(this, newob, remap);
	return newob;
	}

RefResult BombMod::NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message ) 
	{ 
	switch ( message ) {
		case REFMSG_TARGET_DELETED:			
			// THis means the Bomb node is being deleted. As a result,
			// we must delete ourselves. 
			DeleteMe();  // also deletes all refs and 
						 // sends REFMSG_TARGET_DELETED to all Dependents			
			return REF_STOP; 		
		};

	return REF_SUCCEED; 
	}	


#define NUM_SEGS	32

static void DrawFalloffSphere(float radius, PolyLineProc& lp)
	{
	float u;
	Point3 pt[3];
	
	lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));	
	
	// XY
	pt[0] = Point3(radius,0.0f,0.0f);
	for (int i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].x = (float)cos(u) * radius;
		pt[1].y = (float)sin(u) * radius;
		pt[1].z = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
		}

	// YZ	
	pt[0] = Point3(0.0f,radius,0.0f);
	for (i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].y = (float)cos(u) * radius;
		pt[1].z = (float)sin(u) * radius;
		pt[1].x = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
		}
	
	// ZX	
	pt[0] = Point3(0.0f,0.0f,radius);
	for (i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].z = (float)cos(u) * radius;
		pt[1].x = (float)sin(u) * radius;
		pt[1].y = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
		}
	}
