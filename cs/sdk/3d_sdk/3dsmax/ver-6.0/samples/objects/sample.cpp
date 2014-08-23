/**********************************************************************
 *<
	FILE: sample.cpp

	DESCRIPTION:  Sample implementation

	HISTORY: created November 11 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"
#include <props.h>
#include "dummy.h"
#include "Simpobj.h"
#include "notify.h"

#ifndef NO_OBJECT_RINGARRAY	// russom - 10/12/01

extern HINSTANCE hInstance;

#define RINGARRAY_CLASS_ID 0x9120
#define SLAVE_CONTROL_CLASS_ID 0x9100

// Parameter block indices
#define PB_RAD	0
#define PB_CYC	1
#define PB_AMP	2
#define PB_PHS	3

//----------------------------------------------------------------------

class RingMaster: public ReferenceTarget {
	public:
	// Object parameters

		IParamBlock *pblock;
		Tab<INode*> nodeTab;
		int numNodes;
		RingMaster();
		~RingMaster();

		INode* GetSlaveNode(int i) { return nodeTab[i]; }
		void SetSlaveNode(int i, INode * node);
		void SetNum(TimeValue t, int n, BOOL notify=TRUE); 
		void SetRad(TimeValue t, float r); 
		void SetCyc(TimeValue t, float r); 
		void SetAmp(TimeValue t, float r); 
		void SetPhs(TimeValue t, float r); 
		int GetNum(TimeValue t, Interval& valid = Interval(0,0) ); 	
		float GetRad(TimeValue t, Interval& valid = Interval(0,0) ); 	
		float GetCyc(TimeValue t, Interval& valid = Interval(0,0) ); 	
		float GetAmp(TimeValue t, Interval& valid = Interval(0,0) ); 	
		float GetPhs(TimeValue t, Interval& valid = Interval(0,0) ); 	

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method, int id);
		void UpdateUI(TimeValue t);

		// Class vars
		static HWND hMasterParams;
		static IObjParam *iObjParams;
		static int dlgNum;
		static float dlgRadius;
		static float dlgAmplitude;
		static float dlgCycles;
		static float dlgPhase;
		static ISpinnerControl *numSpin;
		static ISpinnerControl *radSpin;
		static ISpinnerControl *ampSpin;
		static ISpinnerControl *cycSpin;
		static ISpinnerControl *phsSpin;

//		static void NotifyPreDeleteNode(void* param, NotifyInfo*);

		// From Animatable

		int NumSubs()  { return 1; }
		Animatable* SubAnim(int i) { return pblock; }
		TSTR SubAnimName(int i) { return GetString(IDS_DS_RINGARRAYPAR);}		

		Class_ID ClassID() { return Class_ID(RINGARRAY_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return SYSTEM_CLASS_ID; }  
		void GetClassName(TSTR& s) { s = GetString(IDS_DB_RING_ARRAY_CLASS); }
		void DeleteThis() { delete this; }		
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );
		
		void GetSystemNodes(INodeTab &nodes, SysNodeContext ctxt);

		// From Reference Target
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		int NumRefs() { return 1;	};
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage);
	};

Control* GetNewSlaveControl(RingMaster *master, int i);

//------------------------------------------------------

class RingMasterClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new RingMaster(); }
	const TCHAR *	ClassName() { return GetString(IDS_DB_RING_ARRAY_CLASS); }
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	SClass_ID		SuperClassID() { return SYSTEM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(RINGARRAY_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};
static RingMasterClassDesc mcDesc;

ClassDesc* GetRingMasterDesc() { return &mcDesc; }
//------------------------------------------------------

HWND RingMaster::hMasterParams = NULL;
IObjParam *RingMaster::iObjParams;

int RingMaster::dlgNum = 	4;
float RingMaster::dlgRadius = 	100.0f;
float RingMaster::dlgAmplitude = 20.0f;
float RingMaster::dlgCycles = 	3.0f;
float RingMaster::dlgPhase = 	1.0f;

ISpinnerControl *RingMaster::numSpin;
ISpinnerControl *RingMaster::radSpin;
ISpinnerControl *RingMaster::ampSpin;
ISpinnerControl *RingMaster::cycSpin;
ISpinnerControl *RingMaster::phsSpin;


RingMaster::RingMaster() {
	ParamBlockDesc desc[] = {
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE }
		};

	MakeRefByID( FOREVER, 0, CreateParameterBlock( desc, 4 ) );	
//	RegisterNotification(NotifyPreDeleteNode, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	SetRad( TimeValue(0), dlgRadius );
	SetCyc( TimeValue(0), dlgCycles );
	SetAmp( TimeValue(0), dlgAmplitude );
	SetPhs( TimeValue(0), dlgPhase );
	numNodes = dlgNum;
	}

RefTargetHandle RingMaster::Clone(RemapDir& remap) {
	int i;
    RingMaster* newm = new RingMaster();	
	newm->ReplaceReference(0,pblock->Clone(remap));
	newm->numNodes = numNodes;
	newm->nodeTab.SetCount(numNodes);
	for (i=0; i<numNodes; i++) newm->nodeTab[i] = NULL;			
	for (i=0; i<numNodes; i++) {
		remap.PatchPointer((RefTargetHandle*)&newm->nodeTab[i],(RefTargetHandle)nodeTab[i]);
		}
	BaseClone(this, newm, remap);
	return(newm);
	}

RingMaster::~RingMaster() {
//	UnRegisterNotification(NotifyPreDeleteNode, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	}

void RingMaster::UpdateUI(TimeValue t)
	{
	if ( hMasterParams ) {
		radSpin->SetValue( GetRad(t), FALSE );
		ampSpin->SetValue( GetAmp(t), FALSE );
		cycSpin->SetValue( GetCyc(t), FALSE );
		phsSpin->SetValue( GetPhs(t), FALSE );
		numSpin->SetValue( GetNum(t), FALSE );
		radSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_RAD,t));
		ampSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_AMP,t));
		cycSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_CYC,t));
		phsSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_PHS,t));
		}
	}

void RingMaster::SetSlaveNode(int i, INode * node) {
	if (i>=nodeTab.Count()) {
		int nold = nodeTab.Count();
		nodeTab.SetCount(i+1);
		if (i+1>numNodes) 
			numNodes = i+1;
		for (int j = nold; j<i+1; j++) nodeTab[j] = NULL;
		}		
	nodeTab[i] = node;
	}


void RingMaster::GetSystemNodes(INodeTab &nodes, SysNodeContext ctxt)
	{

	switch (ctxt) {
		case kSNCClone:
		case kSNCFileMerge:
		case kSNCFileSave:
		case kSNCDelete:

		{
			for (int i=0; i<nodeTab.Count(); i++) {
				nodes.Append(1,&nodeTab[i]);
			}
		}
		break;
	default:
		break;
	}


}

RefTargetHandle RingMaster::GetReference(int i)  { 
	if (i==0) return pblock;
	return NULL;
	}


void RingMaster::SetReference(int i, RefTargetHandle rtarg) {
	if (i==0)
		pblock = (IParamBlock *)rtarg; 
	}		

#define TWO_PI 6.283185307f

// This is the crux of the controller: it takes an input (parent) matrix, modifies
// it accordingly.  
void RingMaster::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method,
 int id) {
	float radius,amplitude,cycles, phase;
	radius = GetRad(t,valid);
	amplitude = GetAmp(t,valid);
	cycles = GetCyc(t,valid);
	phase = GetPhs(t,valid);
	Matrix3 tmat, *mat = (Matrix3*)val;
	tmat.IdentityMatrix();
	float ang = float(id)*TWO_PI/(float)numNodes;
	tmat.Translate(Point3(radius,0.0f,amplitude*(float)cos(cycles*ang + TWO_PI*phase)));
	tmat.RotateZ(ang);
	
	(*mat) = (method==CTRL_RELATIVE) ? tmat*(*mat) : tmat;

	// Make sure spinners track when animating and in Motion Panel
	UpdateUI(t);
	}

RefResult RingMaster::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
     PartID& partID, RefMessage message ) 
    {
	switch (message) {
		case REFMSG_GET_PARAM_DIM: { 
			// the ParamBlock needs info to display in the tree view
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_RAD:	gpd->dim = stdWorldDim; break;	  
				case PB_CYC:	gpd->dim = stdWorldDim; break;	  
				case PB_AMP:	gpd->dim = stdWorldDim; break;	  
				case PB_PHS:	gpd->dim = stdWorldDim; break;	  
					break;									
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			// the ParamBlock needs info to display in the tree view
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_RAD: gpn->name = GetString(IDS_DS_RADIUS);	break;
				case PB_CYC: gpn->name = GetString(IDS_DS_CYCLES);	break;
				case PB_AMP: gpn->name = GetString(IDS_DS_AMPLITUDE);	break;
				case PB_PHS: gpn->name = GetString(IDS_DS_PHASE);	break;
				}
			return REF_STOP; 
			}
		case REFMSG_TARGET_DELETED:{
			for (int i=0; i<nodeTab.Count(); i++) {
				if (hTarget==nodeTab[i]) {
					nodeTab[i] = NULL;				
					break;
					}					
				}
			}
			return REF_STOP;
		}
	return(REF_SUCCEED);
	}

/*
class SlaveNodeDeletedRestore: public RestoreObj {
	RingMaster *rng;
	int index;
	INode* slaveNode;
	bool inRange;
	public:
		SlaveNodeDeletedRestore::SlaveNodeDeletedRestore(RingMaster *ringm, int n); 
		~SlaveNodeDeletedRestore() {}
		void Restore(int isUndo);
		void Redo();
		TSTR Description() { return TSTR("SlaveNodeDeletedRestore"); }
	};

SlaveNodeDeletedRestore::SlaveNodeDeletedRestore(RingMaster *ringm, int n) { 
	rng = ringm; 
	index = n;
	inRange = ((n < rng->nodeTab.Count()) && (n >= 0));
	if (inRange)
		slaveNode = rng->nodeTab[n];
	}

void SlaveNodeDeletedRestore::Restore(int isUndo) {
	if (inRange) {
		int oldnum = rng->nodeTab.Count();
		rng->nodeTab.SetCount(oldnum+1);
		for(int i=oldnum; i>index; i--)
			rng->nodeTab[i] = rng->nodeTab[i-1];
		rng->nodeTab[index] = slaveNode;
		rng->numNodes = oldnum+1;
		if (rng->hMasterParams) 
		    RingMaster::numSpin->SetValue(rng->numNodes, FALSE );
		rng->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		}
	}

void SlaveNodeDeletedRestore::Redo() {	
	if (inRange) {
		slaveNode = rng->nodeTab[index];
		int oldnum = rng->nodeTab.Count();
		rng->nodeTab.Delete(index,1);
		rng->numNodes = oldnum-1;
		if (rng->hMasterParams) 
		    RingMaster::numSpin->SetValue(rng->numNodes, FALSE );
		rng->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		}
	}
*/

class RingNumRestore: public RestoreObj {
	RingMaster *rng;
	int num;
	int redNum;
	BOOL attached;
	INodeTab svNodes;
	public:
		RingNumRestore::RingNumRestore(RingMaster *ringm, int n); 
		~RingNumRestore() {}
		void Restore(int isUndo);
		void Redo();
		TSTR Description() { return TSTR("RingNumRestore"); }
	};

RingNumRestore::RingNumRestore(RingMaster *ringm, int n) { 
	rng = ringm; 
	num = rng->numNodes; 
	//DebugPrint(" RingNumRestore, num = %d, n = %d  \n",num,n);
	if (rng->nodeTab.Count()==rng->numNodes) {
		attached = TRUE;
		if (n<rng->numNodes) {
			for (int i=n; i<rng->numNodes; i++)
				svNodes.Append(1,&rng->nodeTab[i]);
			}
		}
	else attached = FALSE;
	}

void RingNumRestore::Restore(int isUndo) {
	redNum = rng->nodeTab.Count();
	//DebugPrint("Restore, num= %d \n",num);
	if (attached) {
		if (num>redNum) {
			// add on nodes
			for (int i=0; i<num-redNum; i++)
				rng->nodeTab.Append(1,&svNodes[i]);
			}
		else {
			for (int i=num; i<redNum; i++)
				svNodes.Append(1,&rng->nodeTab[i]);
			rng->nodeTab.SetCount(num);
			}	
		}
	rng->numNodes = num;
	if (rng->hMasterParams) 
		RingMaster::numSpin->SetValue(rng->numNodes, FALSE );
	rng->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

void RingNumRestore::Redo() {	
	if (attached) {
		rng->nodeTab.SetCount(redNum);
		if (redNum>rng->numNodes) {
			for (int i=num; i<redNum; i++)
				rng->nodeTab[i] = svNodes[i-num];
			}			
		}
	rng->numNodes = redNum;
	if (rng->hMasterParams) 
	    RingMaster::numSpin->SetValue(rng->numNodes, FALSE );
	rng->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

//--------------------------------------------------
/*
void RingMaster::NotifyPreDeleteNode(void* parm, NotifyInfo* arg){

	RingMaster* rMast = (RingMaster*)parm;
	if(rMast == NULL) return;

	int new_node_number;
	if(arg != NULL){
		INode* deleted_node = (INode*)arg->callParam;
		int old_node_number = rMast->nodeTab.Count();
		for(int i=0; i <old_node_number; i++ ){
			if (deleted_node == rMast->nodeTab[i]){
				new_node_number = old_node_number - 1;
				if (theHold.Holding()){
 					theHold.Put(new SlaveNodeDeletedRestore(rMast, i));
				}
				rMast->numNodes = new_node_number;
				rMast->nodeTab.Delete(i,1);
				if (rMast->hMasterParams)
					RingMaster::numSpin->SetValue(rMast->numNodes, FALSE);
				rMast->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
				break;
			}
			
		}
		if(rMast->iObjParams != NULL){
			rMast->iObjParams->RedrawViews(GetCOREInterface()->GetTime(), REDRAW_INTERACTIVE, rMast);
		}

	}
}

*/

void RingMaster::SetNum(TimeValue t, int n, BOOL notify) { 
	if (n==numNodes) return;
	if (n<1) return;
	//DebugPrint("SetNum %d \n",n);
	int oldNumNodes = numNodes;
	if (theHold.Holding())
		theHold.Put(new RingNumRestore(this, n));
	if (nodeTab.Count()>0) {
		if (n<numNodes) {
			// remove nodes;
			for (int i=numNodes-1; i>=n; i--) {
				INode* node = nodeTab[i];
				if (node) {
					if (node->Selected()) {
						// Dont want to delete selected nodes
						n = i+1;
						numSpin->SetValue( n, TRUE );
						break;
						}
						node->Delete(t,TRUE);
					nodeTab[i] = NULL;
					}
				numNodes = i;
				}
			nodeTab.SetCount(n);
			}
		else {
			nodeTab.SetCount(n);
			for (int i=numNodes; i<n; i++)
				nodeTab[i] = NULL;
			// add nodes;
			if (nodeTab[0]==NULL) {
				n++;
				numSpin->SetValue( n, TRUE );
				}
			else for (int i=numNodes; i<n; i++) {
				Object * obj = nodeTab[0]->GetObjectRef();
				assert(obj);
				INode *newNode = iObjParams->CreateObjectNode(obj);
				Control* slave = GetNewSlaveControl(this,i);
				newNode->SetTMController(slave);
				newNode->FlagForeground(t,FALSE);
				INode *par = nodeTab[0]->GetParentNode();
				assert(par);
			    par->AttachChild(newNode);
				SetSlaveNode(i,newNode);
				}
			}
		}
	numNodes = n;
	if (notify)
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);

	}

int RingMaster::GetNum(TimeValue t, Interval& valid ) { 	
	return numNodes;
	}


//--------------------------------------------------
void RingMaster::SetRad(TimeValue t, float r) { 
	pblock->SetValue( PB_RAD, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float RingMaster::GetRad(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_RAD, t, f, valid );
	return f;
	}

//--------------------------------------------------
void RingMaster::SetCyc(TimeValue t, float r) { 
	pblock->SetValue( PB_CYC, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float RingMaster::GetCyc(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_CYC, t, f, valid );
	return f;
	}

//--------------------------------------------------
void RingMaster::SetAmp(TimeValue t, float r) { 
	pblock->SetValue( PB_AMP, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float RingMaster::GetAmp(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_AMP, t, f, valid );
	return f;
	}

//--------------------------------------------------
void RingMaster::SetPhs(TimeValue t, float r) { 
	pblock->SetValue( PB_PHS, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float RingMaster::GetPhs(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_PHS, t, f, valid );
	return f;
	}

//--------------------------------------------------


INT_PTR CALLBACK MasterParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
	RingMaster *mc = (RingMaster *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	if ( !mc && message != WM_INITDIALOG ) return FALSE;
	
	assert(mc->iObjParams);
	switch ( message ) {
		case WM_INITDIALOG:
			mc = (RingMaster *)lParam;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)mc );
			SetDlgFont( hDlg, mc->iObjParams->GetAppHFont() );
			
			mc->radSpin  = GetISpinner(GetDlgItem(hDlg,IDC_RADSPINNER));
			mc->cycSpin  = GetISpinner(GetDlgItem(hDlg,IDC_CYCSPINNER));
			mc->ampSpin  = GetISpinner(GetDlgItem(hDlg,IDC_AMPSPINNER));
			mc->phsSpin  = GetISpinner(GetDlgItem(hDlg,IDC_PHSSPINNER));
			mc->numSpin  = GetISpinner(GetDlgItem(hDlg,IDC_NUMSPINNER));

			mc->radSpin->SetLimits( 0.0f, 500.0f, FALSE );
			mc->cycSpin->SetLimits( 0.0f, 10.0f, FALSE );
			mc->ampSpin->SetLimits( 0.0f, 500.0f, FALSE );
			mc->phsSpin->SetLimits( -1000.0f, 1000.0f, FALSE );
			mc->numSpin->SetLimits( 1, 200, FALSE );

			mc->radSpin->SetScale(float(0.1) );
			mc->ampSpin->SetScale(float(0.1) );
			mc->phsSpin->SetScale(float(0.1) );
			mc->numSpin->SetScale(float(0.1) );

			mc->radSpin->SetValue( mc->GetRad(mc->iObjParams->GetTime()), FALSE );
			mc->cycSpin->SetValue( mc->GetCyc(mc->iObjParams->GetTime()), FALSE );
			mc->ampSpin->SetValue( mc->GetAmp(mc->iObjParams->GetTime()), FALSE );
			mc->phsSpin->SetValue( mc->GetPhs(mc->iObjParams->GetTime()), FALSE );
			mc->numSpin->SetValue( mc->GetNum(mc->iObjParams->GetTime()), FALSE );

			mc->radSpin->LinkToEdit( GetDlgItem(hDlg,IDC_RADIUS), EDITTYPE_POS_UNIVERSE );			
			mc->cycSpin->LinkToEdit( GetDlgItem(hDlg,IDC_CYCLES), EDITTYPE_FLOAT );			
			mc->ampSpin->LinkToEdit( GetDlgItem(hDlg,IDC_AMPLITUDE), EDITTYPE_FLOAT );			
			mc->phsSpin->LinkToEdit( GetDlgItem(hDlg,IDC_PHASE), EDITTYPE_FLOAT );			
			mc->numSpin->LinkToEdit( GetDlgItem(hDlg,IDC_NUMNODES), EDITTYPE_INT );			
			
			return FALSE;	// DB 2/27

		case WM_DESTROY:
			ReleaseISpinner( mc->radSpin );
			ReleaseISpinner( mc->cycSpin );
			ReleaseISpinner( mc->ampSpin );
			ReleaseISpinner( mc->phsSpin );
			ReleaseISpinner( mc->numSpin );
			mc->radSpin = NULL;
			mc->cycSpin = NULL;
			mc->ampSpin = NULL;
			mc->phsSpin = NULL;
			mc->numSpin = NULL;
			return FALSE;

		case CC_SPINNER_CHANGE:	{
			if (!theHold.Holding()) theHold.Begin();
			TimeValue t = mc->iObjParams->GetTime();
			switch ( LOWORD(wParam) ) {
				case IDC_RADSPINNER: 
					mc->SetRad(t,  mc->radSpin->GetFVal() );  
					mc->radSpin->SetKeyBrackets(mc->pblock->KeyFrameAtTime(PB_RAD,t));
					break;
				case IDC_CYCSPINNER: 
					mc->SetCyc(t,  mc->cycSpin->GetFVal() );  
					mc->cycSpin->SetKeyBrackets(mc->pblock->KeyFrameAtTime(PB_CYC,t));
					break;
				case IDC_AMPSPINNER: 
					mc->SetAmp(t,  mc->ampSpin->GetFVal() );  
					mc->ampSpin->SetKeyBrackets(mc->pblock->KeyFrameAtTime(PB_AMP,t));
					break;
				case IDC_PHSSPINNER: 
					mc->SetPhs(t,  mc->phsSpin->GetFVal() );  
					mc->phsSpin->SetKeyBrackets(mc->pblock->KeyFrameAtTime(PB_PHS,t));
					break;
				case IDC_NUMSPINNER: 
					mc->SetNum(t,  mc->numSpin->GetIVal() );  
					break;
				}
			assert(mc->iObjParams);
			mc->iObjParams->RedrawViews(t, REDRAW_INTERACTIVE, mc);
			return TRUE;

			}
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || message==WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else 
				theHold.Cancel();
			mc->iObjParams->RedrawViews(mc->iObjParams->GetTime(), REDRAW_END, mc);
			return TRUE;

		case WM_MOUSEACTIVATE:
			mc->iObjParams->RealizeParamPanel();
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			mc->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:			
			return FALSE;

		default:
			return FALSE;
		}
	}



void RingMaster::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	iObjParams = ip;
	
	if ( !hMasterParams ) {
		hMasterParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_SAMPLEPARAM),
				MasterParamDialogProc,
				GetString(IDS_RB_PARAMETERS), 
				(LPARAM)this );		
		ip->RegisterDlgWnd(hMasterParams);
		
	} else {
		SetWindowLongPtr( hMasterParams, GWLP_USERDATA, (LONG_PTR)this );		

		// Init the dialog to our values.
		radSpin->SetValue(GetRad(ip->GetTime()),FALSE);
		cycSpin->SetValue(GetCyc(ip->GetTime()),FALSE);
		ampSpin->SetValue(GetAmp(ip->GetTime()),FALSE);
		phsSpin->SetValue(GetPhs(ip->GetTime()),FALSE);
		numSpin->SetValue(GetNum(ip->GetTime()),FALSE);
		}
	}
		
void RingMaster::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	if (hMasterParams==NULL) 
		return;
	dlgRadius   = radSpin->GetFVal();
	dlgAmplitude   = ampSpin->GetFVal();
	dlgCycles   = cycSpin->GetFVal();
	dlgPhase   = phsSpin->GetFVal();
	dlgNum   = numSpin->GetIVal();
	
	if ( flags&END_EDIT_REMOVEUI ) {		
		ip->UnRegisterDlgWnd(hMasterParams);
		ip->DeleteRollupPage(hMasterParams);
		hMasterParams = NULL;
		}
	else {		
		SetWindowLongPtr( hMasterParams, GWLP_USERDATA, 0 );
		}
	
	iObjParams = NULL;
	}

#define NUMNODES_CHUNK 0x100
#define NODE_ID_CHUNK 0x110

// IO
IOResult RingMaster::Save(ISave *isave) {
	ULONG nb;
	isave->BeginChunk(NUMNODES_CHUNK);
	isave->Write(&numNodes,sizeof(numNodes), &nb);
	isave->EndChunk();
	if (numNodes>0) {
		isave->BeginChunk(NODE_ID_CHUNK);
		for (int i=0; i<numNodes; i++) {
			ULONG id = isave->GetRefID(nodeTab[i]);
			isave->Write(&id,sizeof(ULONG), &nb);
			}
		isave->EndChunk();
		}
	return IO_OK;
	}

IOResult RingMaster::Load(ILoad *iload) {
	ULONG nb;
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case NUMNODES_CHUNK: {
				res = iload->Read(&numNodes,sizeof(numNodes), &nb);
				nodeTab.SetCount(numNodes);
				for (int i=0; i<numNodes; i++) nodeTab[i] = NULL;
				}
				break;
			case NODE_ID_CHUNK:
				for (int i=0; i<numNodes; i++) {
					ULONG id;
					iload->Read(&id,sizeof(ULONG), &nb);
					if (id!=0xffffffff)
						iload->RecordBackpatch(id,(void**)&nodeTab[i]);
					}
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

//-------------------------------------------------------------
class SlaveControl : public Control {
	public:		
		RingMaster *master;
		ULONG id;

		SlaveControl(BOOL loading=FALSE) { master = NULL; id = 0; }
		SlaveControl(const SlaveControl& ctrl);
		SlaveControl(const RingMaster* m, int i);
		void SetID( ULONG i) { id = i;}
		virtual ~SlaveControl() {}	
		SlaveControl& operator=(const SlaveControl& ctrl);

		// From Control
		void Copy(Control *from) {}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		virtual BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		BOOL IsReplaceable() {return FALSE;}
		BOOL CanCopyAnim() {return FALSE;}


		// From Animatable
		void* GetInterface(ULONG id);
		int NumSubs()  { return master->NumSubs(); }
		Animatable* SubAnim(int i) { return master->SubAnim(i); }
		TSTR SubAnimName(int i) { return master->SubAnimName(i); }
		Class_ID ClassID() { return Class_ID(SLAVE_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }  
		void GetClassName(TSTR& s) { s = GetString(IDS_DB_SLAVECONTROL_CLASS); }
		void DeleteThis() { delete this; }		
		int IsKeyable(){ return 0;}
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev) { assert(master); master->BeginEditParams(ip,flags,prev); } 
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) { assert(master); master->EndEditParams(ip,flags,next); } 
		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From ReferenceTarget
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		int NumRefs() { return 1; };	
		RefTargetHandle GetReference(int i)  { assert(i==0); return master; }
		void SetReference(int i, RefTargetHandle rtarg) { assert(i==0); master = (RingMaster *)rtarg; }		
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
	};



class SlaveControlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new SlaveControl(); }
	const TCHAR *	ClassName() { return GetString(IDS_DB_SLAVE_CONTROL); }
	SClass_ID		SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SLAVE_CONTROL_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};

static SlaveControlClassDesc slvDesc;

ClassDesc* GetSlaveControlDesc() { return &slvDesc; }

Control* GetNewSlaveControl(RingMaster *master, int i) {
	return new SlaveControl(master,i);
	}

SlaveControl::SlaveControl(const SlaveControl& ctrl) {
	master = ctrl.master;
	id = ctrl.id;
	}

SlaveControl::SlaveControl(const RingMaster* m, int i) {
	id = i;
    MakeRefByID( FOREVER, 0, (ReferenceTarget *)m);
	}

RefTargetHandle SlaveControl::Clone(RemapDir& remap) {
	SlaveControl *sl = new SlaveControl;
	sl->id = id;
	sl->ReplaceReference(0, remap.CloneRef(master));
	BaseClone(this, sl, remap);
	return sl;
	}

SlaveControl& SlaveControl::operator=(const SlaveControl& ctrl) {
	master = ctrl.master;
	id = ctrl.id;
	return (*this);
	}

void SlaveControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method) {
	assert(master);
	master->GetValue(t,val,valid,method,id);	
	}


void SlaveControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method) { }

void* SlaveControl::GetInterface(ULONG id) {
	if (id==I_MASTER) 
		return (void *)master;
	else 
		return NULL;
	}

// IO
#define SLAVE_ID_CHUNK 0x200
IOResult SlaveControl::Save(ISave *isave) {
	ULONG nb;
	isave->BeginChunk(SLAVE_ID_CHUNK);
	isave->Write(&id,sizeof(id), &nb);
	isave->EndChunk();
	return IO_OK;
	}

IOResult SlaveControl::Load(ILoad *iload) {
	ULONG nb;
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case SLAVE_ID_CHUNK:
				res = iload->Read(&id,sizeof(id), &nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

//----------------------------------------------------------------------


class RingMasterCreationManager : public MouseCallBack, ReferenceMaker {
	public:
		CreateMouseCallBack *createCB;	
		INode *node0;
		RingMaster *theMaster;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		IPoint2 pt0;
		Point3 center;
		BOOL attachedToNode;
		int lastPutCount;

		void CreateNewMaster();
			
		int ignoreSelectionChange;

		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);

		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		RingMasterCreationManager()
			{
			ignoreSelectionChange = FALSE;
			}
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	};

#define CID_BONECREATE	CID_USER + 1

class RingMasterCreateMode : public CommandMode {
		RingMasterCreationManager proc;
	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		int Class() { return CREATE_COMMAND; }
		int ID() { return CID_BONECREATE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints = 100000; return &proc; }
		ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
// mjm - begin - 3.2.99
		void EnterMode() { SetCursor( LoadCursor( hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR) ) ); }
		void ExitMode() { SetCursor( LoadCursor(NULL, IDC_ARROW) ); }
// mjm - end
		BOOL IsSticky() { return FALSE; }
	};

static RingMasterCreateMode theRingMasterCreateMode;

//RingMasterCreationManager::RingMasterCreationManager( IObjCreate *ioc, ClassDesc *desc )
void RingMasterCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc )
	{
	createInterface = ioc;
	cDesc           = desc;
	createCB        = NULL;
	node0			= NULL;
	theMaster 		= NULL;
	attachedToNode = FALSE;
	CreateNewMaster();
	}

void RingMasterCreationManager::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case 0: node0 = (INode *)rtarg; break;
		default: assert(0); 
		}
	}

RefTargetHandle RingMasterCreationManager::GetReference(int i) { 
	switch(i) {
		case 0: return (RefTargetHandle)node0;
		default: assert(0); 
		}
	return NULL;
	}

//RingMasterCreationManager::~RingMasterCreationManager
void RingMasterCreationManager::End()
	{
	if (theMaster) {
		theMaster->EndEditParams( (IObjParam*)createInterface, 
	                    	          TRUE/*destroy*/, NULL );
		if ( !attachedToNode ) {
			delete theMaster;
			theMaster = NULL;
			// DS 8/21/97: If something has been put on the undo stack since this object was created, we have to flush the undo stack.
			if (theHold.GetGlobalPutCount()!=lastPutCount) {
				GetSystemSetting(SYSSET_CLEAR_UNDO);
				}
		} else if ( node0 ) {
			 // Get rid of the references.
			DeleteAllRefsFromMe();
			}
		theMaster = NULL; //JH 9/15/97
		}	
	}

RefResult RingMasterCreationManager::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
	{
	switch (message) {
		case REFMSG_TARGET_SELECTIONCHANGE:
		 	if ( ignoreSelectionChange ) {
				break;
				}
		 	if (theMaster) {
				// this will set node0 ==NULL;
				DeleteAllRefsFromMe();
				goto endEdit;
				}
			else
				return REF_SUCCEED;  //JH 9.15.97 
			// fall through

		case REFMSG_TARGET_DELETED:
			if (theMaster) {
				endEdit:
				theMaster->EndEditParams( (IObjParam*)createInterface, FALSE/*destroy*/,NULL );
				theMaster = NULL;
				node0 = NULL;
				CreateNewMaster();	
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

void RingMasterCreationManager::CreateNewMaster()
	{
	theMaster = new RingMaster();
	
	// Start the edit params process
	theMaster->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE,NULL );
	lastPutCount = theHold.GetGlobalPutCount();
	}

#define DUMSZ 20.0f
#define BOXSZ 20.0f

static BOOL needToss;

int RingMasterCreationManager::proc( 
				HWND hwnd,
				int msg,
				int point,
				int flag,
				IPoint2 m )
	{	
	int res;
	INode *newNode,*dummyNode;	
	float r;
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );


	switch ( msg ) {
		case MOUSE_POINT:
				{
				if (point==0) {
					pt0 = m;	

					assert(theMaster);

					mat.IdentityMatrix();
					if ( createInterface->SetActiveViewport(hwnd) ) {
						return FALSE;
						}
					if (createInterface->IsCPEdgeOnInView()) { 
						return FALSE;
						}
					if ( attachedToNode ) {
				   		// send this one on its way
				   		theMaster->EndEditParams( (IObjParam*)createInterface,0,NULL );
						
						// Get rid of the references.
						DeleteAllRefsFromMe();

						// new object
						CreateNewMaster();   // creates theMaster
						}

					needToss = theHold.GetGlobalPutCount()!=lastPutCount;

				   	theHold.Begin();	 // begin hold for undo
					mat.IdentityMatrix();
					center = vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
					mat.SetTrans(center);

					// Create a dummy object & node
					DummyObject *dumObj = (DummyObject *)createInterface->
						CreateInstance(HELPER_CLASS_ID,Class_ID(DUMMY_CLASS_ID,0)); 			
					assert(dumObj);					
					dummyNode = createInterface->CreateObjectNode(dumObj);
					dumObj->SetBox(Box3(Point3(-DUMSZ,-DUMSZ,-DUMSZ),Point3(DUMSZ,DUMSZ,DUMSZ)));

					// make a box object
					GenBoxObject *ob = (GenBoxObject *)createInterface->
						CreateInstance(GEOMOBJECT_CLASS_ID,Class_ID(BOXOBJ_CLASS_ID,0));
					ob->SetParams(BOXSZ,BOXSZ,BOXSZ,1,1,1,FALSE); 

					// Make a bunch of nodes, hook the box object to and a
					// slave controller of the master control to each
					for (int i=0; i<theMaster->numNodes; i++) {
						newNode = createInterface->CreateObjectNode(ob);
						SlaveControl* slave = new SlaveControl(theMaster,i);
						newNode->SetTMController(slave);
						dummyNode->AttachChild(newNode);
						theMaster->SetSlaveNode(i,newNode);
						}

					// select the dummy node.
					attachedToNode = TRUE;

					// Reference the node so we'll get notifications.
				    MakeRefByID( FOREVER, 0, theMaster->GetSlaveNode(0) );
					theMaster->SetRad(TimeValue(0),0.0f);
					mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
					createInterface->SetNodeTMRelConstPlane(dummyNode, mat);
					res = TRUE;
					}
				else {
					// select a node so if go into modify branch, see params 
					ignoreSelectionChange = TRUE;
				   	createInterface->SelectNode( theMaster->GetSlaveNode(0) );
					ignoreSelectionChange = FALSE;
					theHold.Accept(IDS_DS_CREATE);
					res = FALSE;
					}
 				createInterface->RedrawViews(createInterface->GetTime(),REDRAW_NORMAL,theMaster);  
				}
				break;
		case MOUSE_MOVE:
			if (node0) {
				r = (float)fabs(vpx->SnapLength(vpx->GetCPDisp(center,Point3(0,1,0),pt0,m)));
				theMaster->SetRad(0,r);
				theMaster->radSpin->SetValue(r, FALSE );
				createInterface->RedrawViews(createInterface->GetTime(),REDRAW_NORMAL,theMaster);
				}
			res = TRUE;
			break;

// mjm - 3.2.99 - begin
		case MOUSE_FREEMOVE:
			SetCursor( LoadCursor( hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR) ) );
			break;
// mjm - end

		case MOUSE_PROPCLICK:
			// right click while between creations
			createInterface->RemoveMode(NULL);
			break;

		case MOUSE_ABORT:
			assert(theMaster);
			theMaster->EndEditParams( (IObjParam*)createInterface, 0,NULL );
			theHold.Cancel();  // undo the changes
			// DS 8/21/97: If something has been put on the undo stack since this object was created, we have to flush the undo stack.
			if (needToss) 
				GetSystemSetting(SYSSET_CLEAR_UNDO);
			DeleteAllRefsFromMe();
			CreateNewMaster();	
			createInterface->RedrawViews(createInterface->GetTime(),REDRAW_END,theMaster); 
			attachedToNode = FALSE;
			res = FALSE;						
			break;
		}
	
	createInterface->ReleaseViewport(vpx); 
	return res;
	}

int RingMasterClassDesc::BeginCreate(Interface *i)
	{
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	
	theRingMasterCreateMode.Begin( iob, this );
	iob->PushCommandMode( &theRingMasterCreateMode );
	
	return TRUE;
	}

int RingMasterClassDesc::EndCreate(Interface *i)
	{
	ResumeSetKeyMode();
	theRingMasterCreateMode.End();
	i->RemoveMode( &theRingMasterCreateMode );
	return TRUE;
	}


#endif // NO_OBJECT_RINGARRAY
