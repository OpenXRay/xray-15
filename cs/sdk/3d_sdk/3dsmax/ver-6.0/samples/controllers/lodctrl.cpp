/**********************************************************************
 *<
	FILE: lodctrl.cpp

	DESCRIPTION: A level of detail controller

	CREATED BY: Rolf Berteig

	HISTORY: 4/12/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#include "ctrl.h"
#include "units.h"
#include "utilapi.h"
#ifdef DESIGN_VER
#include <ILayer.h>
#endif

#define LOD_CONTROL_CLASS_ID	Class_ID(0xbbe961a8,0xa0ee7b7f)
#define LOD_CONTROL_CNAME		GetString(IDS_RB_LODCONTROL)

#define LOD_UTILITY_CLASS_ID	Class_ID(0x100d37ef,0x1aa0ab84)
#define LOD_UTILITY_CNAME		GetString(IDS_RB_LODUTILITU)

class LODCtrl : public StdControl {
	public:
		float min, max, bmin, bmax;
		WORD grpID;
		int order;
		BOOL viewport, highest;

		LODCtrl();

		// Animatable methods		
		void DeleteThis() {delete this;}		
		int IsKeyable() {return 0;}		
		Class_ID ClassID() {return LOD_CONTROL_CLASS_ID;} 
		SClass_ID SuperClassID() {return CTRL_FLOAT_CLASS_ID;}
		void GetClassName(TSTR& s) {s = LOD_CONTROL_CNAME;}
		BOOL CanCopyAnim() {return FALSE;}
		BOOL CanMakeUnique() {return FALSE;}				

		// Reference methods
		RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		RefTargetHandle Clone(RemapDir &remap = NoRemap());

		// Control methods				
		void Copy(Control *from) {}
		BOOL IsLeaf() {return TRUE;}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		BOOL IsReplaceable() {return FALSE;}
		BOOL CanInstanceController() {return FALSE;}

		// StdControl methods
		void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);			
		void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}
		void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type)
			{*((float*)val) = 1.0f;}
		void *CreateTempValue() {return new float;}
		void DeleteTempValue(void *val) {delete (float*)val;}
		void ApplyValue(void *val, void *delta) {*((float*)val) += *((float*)delta);}
		void MultiplyValue(void *val, float m) {*((float*)val) *= m;}

		float EvalVisibility(TimeValue t,View &view,Box3 pbox,Interval &valid);
		BOOL VisibleInViewports();
	};



class GroupIDMapEntry {
	public:
		WORD srcID, dstID;
		GroupIDMapEntry(WORD s, WORD d) {srcID=s; dstID=d;}
	};

class LODUtil : 
			public UtilityObj, 
			public PickModeCallback, 
			public PickNodeCallback {
	public:
		IUtil *iu;
		Interface *ip;		
		HWND hWnd;
		INodeTab nodes;
		int disp;
		Tab<GroupIDMapEntry> clonedGroupIDMap;

		// From UtilityObj		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}
		void SelectionSetChanged(Interface *ip,IUtil *iu);		

		// From PickModeCallback
		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		BOOL RightClick(IObjParam *ip,ViewExp *vpt) {return TRUE;}
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);
		PickNodeCallback *GetFilter() {return this;}
		BOOL AllowMultiSelect() {return TRUE;}
		BOOL Filter(INode *node);

		// Local methods
		LODUtil();
		void FindGroupMembers(INode *root,WORD id,INodeTab &nodes);
		WORD FindUniqueGroupID();
		WORD FindClonedGroupID(WORD gid);
		void ClearClonedGroupMap();
		INode *FindSelGroupHead();
		int GetSelID();
		void SetStates();
		void SetupList();
		void ListSelChanged();
		void CreateNewSet();
		void ResetSet();
		void ResetOutput();
		void SetViewportObj(BOOL onOff);
		void DeleteFromSet();
		void SetMin(float m);
		void SetMax(float m);
		void SetBlendRegions();
		float ConvertToPercent(float size);
		float ConvertFromPercent(float perc);
		void SetDisplayMode(int mode);
		void InitDlg();
	};
static LODUtil theLODUtil;

class LODClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 0;}
	void *			Create(BOOL loading) {return new LODCtrl();}
	const TCHAR *	ClassName() {return LOD_CONTROL_CNAME;}
	SClass_ID		SuperClassID() {return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() {return LOD_CONTROL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};
static LODClassDesc lodCD;
ClassDesc* GetLODControlDesc() {return &lodCD;}

class LODUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theLODUtil;}
	const TCHAR *	ClassName() {return LOD_UTILITY_CNAME;}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return LOD_UTILITY_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static LODUtilClassDesc lodUtilDesc;
ClassDesc* GetLODUtilDesc() {return &lodUtilDesc;}


//--- LODCtrl --------------------------------------------------------

LODCtrl::LODCtrl()
	{
	bmin = min = 0.0f;
	bmax = max = 0.0f;
	grpID = 0;
	order = 0;
	viewport = FALSE;
	highest  = FALSE;
	}

#define MIN_CHUNK		0x0100
#define MAX_CHUNK		0x0110
#define BMIN_CHUNK		0x0112
#define BMAX_CHUNK		0x0115
#define GROUPID_CHUNK	0x0120
#define ORDER_CHUNK		0x0130
#define VIEWPORT_CHUNK	0x0140
#define HIGHEST_CHUNK	0x0150

IOResult LODCtrl::Save(ISave *isave)
	{
	ULONG nb;

	isave->BeginChunk(MIN_CHUNK);
	isave->Write(&min,sizeof(min),&nb);
	isave->EndChunk();

	isave->BeginChunk(MAX_CHUNK);
	isave->Write(&max,sizeof(max),&nb);
	isave->EndChunk();

	isave->BeginChunk(BMIN_CHUNK);
	isave->Write(&bmin,sizeof(bmin),&nb);
	isave->EndChunk();

	isave->BeginChunk(BMAX_CHUNK);
	isave->Write(&bmax,sizeof(bmax),&nb);
	isave->EndChunk();

	isave->BeginChunk(GROUPID_CHUNK);
	isave->Write(&grpID,sizeof(grpID),&nb);
	isave->EndChunk();

	isave->BeginChunk(ORDER_CHUNK);
	isave->Write(&order,sizeof(order),&nb);
	isave->EndChunk();

	isave->BeginChunk(VIEWPORT_CHUNK);
	isave->Write(&viewport,sizeof(viewport),&nb);
	isave->EndChunk();

	isave->BeginChunk(HIGHEST_CHUNK);
	isave->Write(&highest,sizeof(highest),&nb);
	isave->EndChunk();

	return IO_OK;
	}

IOResult LODCtrl::Load(ILoad *iload)
	{
	ULONG nb;
	IOResult res;
	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {	
			case MIN_CHUNK:
				iload->Read(&min,sizeof(min),&nb);
				break;

			case MAX_CHUNK:
				iload->Read(&max,sizeof(max),&nb);
				break;

			case BMIN_CHUNK:
				iload->Read(&bmin,sizeof(bmin),&nb);
				break;

			case BMAX_CHUNK:
				iload->Read(&bmax,sizeof(bmax),&nb);
				break;

			case GROUPID_CHUNK:
				iload->Read(&grpID,sizeof(grpID),&nb);
				break;

			case ORDER_CHUNK:
				iload->Read(&order,sizeof(order),&nb);
				break;

			case VIEWPORT_CHUNK:
				iload->Read(&viewport,sizeof(viewport),&nb);
				break;

			case HIGHEST_CHUNK:
				iload->Read(&highest,sizeof(highest),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	return IO_OK;
	}

void LODCtrl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	*((float*)val) = 1.0f;
	theLODUtil.ClearClonedGroupMap();
	}

RefTargetHandle LODCtrl::Clone(RemapDir &remap)
	{
	LODCtrl *cont  = new LODCtrl;
	cont->min      = min;
	cont->max      = max; 
	cont->bmin     = bmin;
	cont->bmax     = bmax;	
	cont->order    = order;
	cont->viewport = viewport;
	cont->highest  = highest;
	cont->grpID    = theLODUtil.FindClonedGroupID(grpID);

#if 0
	Control *cont = NewDefaultFloatController();
	float val = 1.0f;
	cont->SetValue(0,&val);
#endif
	BaseClone(this, cont, remap);
	return cont;
	}

float LODCtrl::EvalVisibility(
		TimeValue t,View &view,Box3 pbox,Interval &valid)
	{	
	float xmin, xmax, ymin, ymax;	
	for (int i=0; i<8; i++) {
		Point2 pt = view.ViewToScreen(pbox[i]*view.worldToView);
		if (!i) {
			xmin = xmax = pt.x;
			ymin = ymax = pt.y;
		} else {
			if (pt.x<xmin) xmin = pt.x;
			if (pt.x>xmax) xmax = pt.x;
			if (pt.y<ymin) ymin = pt.y;
			if (pt.y>ymax) ymax = pt.y;
			}
		}
	float w = xmax-xmin;
	float h = ymax-ymin;
	float size = (float)sqrt((w*w) + (h*h));
	if (highest && size>=min) return 1.0f;
	if (size>=min && size<=max) return 1.0f;	
	if (size<min && size>bmin) {
		return 1.0f - (min-size)/(min-bmin);
		}
	if (size>max && size<bmax) {
		return 1.0f - (size-max)/(bmax-max);
		}
	return 0.0f;
	}

BOOL LODCtrl::VisibleInViewports()
	{
	return viewport;
	}

//--- LODUtil --------------------------------------------------------

static INT_PTR CALLBACK LODUtilDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define NODESEL_LODANDNONLOD	-4
#define NODESEL_LODMIX			-3
#define NODESEL_NONLOD			-2
#define NODESEL_NONE			-1


LODUtil::LODUtil()
	{
	iu   = NULL;
	ip   = NULL;
	hWnd = NULL;
	disp = IDC_LOD_DISPPERCENT;
	}

void LODUtil::BeginEditParams(Interface *ip,IUtil *iu)
	{
	this->iu = iu;
	this->ip = ip;
	hWnd = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_LOD_UTILPARAMS),
		LODUtilDlgProc,
		GetString(IDS_RB_LODUTILITU),
		0);
	}

void LODUtil::EndEditParams(Interface *ip,IUtil *iu)
	{
	ip->ClearPickMode();
	ip->DeleteRollupPage(hWnd);	
	this->iu = NULL;
	this->ip = NULL;	
	}

void LODUtil::SelectionSetChanged(Interface *ip,IUtil *iu)
	{
	SetStates();
	}


static int __cdecl CompareNodes(const void *elem1, const void *elem2)
	{
	INode *node1 = *((INode**)elem1);
	INode *node2 = *((INode**)elem2);
	LODCtrl *cont1 = (LODCtrl*)node1->GetVisController();
	LODCtrl *cont2 = (LODCtrl*)node2->GetVisController();
	if (!cont1 || !cont2) return 0;
	return cont1->order-cont2->order;
	}

#define BLEND_PERCENT	0.2f

void LODUtil::InitDlg()
	{
	ISpinnerControl *iW = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWWIDTHSPIN));
	ISpinnerControl *iH = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWHEIGHTSPIN));

	iW->SetLimits(1.0f,99999999.0f,FALSE);
	iW->SetScale(0.1f);
	iW->LinkToEdit(GetDlgItem(hWnd,IDC_LOD_VIEWWIDTH),EDITTYPE_INT);	
	iW->SetValue(ip->GetRendWidth(),FALSE);
	iH->SetLimits(1.0f,99999999.0f,FALSE);
	iH->SetScale(0.1f);
	iH->LinkToEdit(GetDlgItem(hWnd,IDC_LOD_VIEWHEIGHT),EDITTYPE_INT);	
	iH->SetValue(ip->GetRendHeight(),FALSE);

	ReleaseISpinner(iW);
	ReleaseISpinner(iH);
	}

void LODUtil::ResetOutput()
	{
	ISpinnerControl *iW = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWWIDTHSPIN));
	ISpinnerControl *iH = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWHEIGHTSPIN));
	iW->SetValue(ip->GetRendWidth(),FALSE);
	iH->SetValue(ip->GetRendHeight(),FALSE);
	ReleaseISpinner(iW);
	ReleaseISpinner(iH);
	SetupList();
	ListSelChanged();
	}

void LODUtil::SetStates()
	{	
	int id = GetSelID();

	ISpinnerControl *iMin = GetISpinner(GetDlgItem(hWnd,IDC_LOD_MINSPIN));
	ISpinnerControl *iMax = GetISpinner(GetDlgItem(hWnd,IDC_LOD_MAXSPIN));	

	iMin->SetLimits(0.0f,99999999.0f,FALSE);
	iMin->SetScale(0.1f);
	iMin->LinkToEdit(GetDlgItem(hWnd,IDC_LOD_MIN),EDITTYPE_FLOAT);	
	iMax->SetLimits(0.0f,99999999.0f,FALSE);
	iMax->SetScale(0.1f);
	iMax->LinkToEdit(GetDlgItem(hWnd,IDC_LOD_MAX),EDITTYPE_FLOAT);	
	
	ReleaseISpinner(iMin);
	ReleaseISpinner(iMax);	

	SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_RESETCONTENT,0,0);

	nodes.SetCount(0);
	if (id>=0) {		
		LODCtrl *cont;
		LODCtrl *cprev=NULL;		
		FindGroupMembers(ip->GetRootNode(),(WORD)id,nodes);
		nodes.Sort(CompareNodes);
		for (int i=0; i<nodes.Count(); i++) {
			cont = (LODCtrl*)nodes[i]->GetVisController();
			if (!cont) {
				return;
				}
			
			if (cprev) {
				cont->min   = cprev->max;
				cont->order = cprev->order+1;
				if (cont->max<cont->min) cont->max = cont->min;
				cont->bmin = cont->min - (cprev->max-cprev->min)*BLEND_PERCENT;
			} else {
				cont->min   = 0;
				cont->bmin  = 0;
				cont->order	= 0;
				}
			if (i<nodes.Count()-1) {
				LODCtrl*cnext = (LODCtrl*)nodes[i+1]->GetVisController();
				if (!cnext) return;
				cont->bmax = cont->max + (cnext->max-cnext->min)*BLEND_PERCENT;
				cont->highest = FALSE;
			} else {
				cont->bmax = cont->max + (cont->max-cont->min)*BLEND_PERCENT;
				cont->highest = TRUE;
				}
			cprev = cont;
			}
		SetupList();
		ICustButton *but;
		but = GetICustButton(GetDlgItem(hWnd,IDC_LOD_ADDTOSET));
		but->SetType(CBT_CHECK);
		but->SetHighlightColor(GREEN_WASH);
		but->Enable();
		ReleaseICustButton(but);		
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_RESET),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_DISPPIXELS),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_DISPPERCENT),TRUE);
		CheckDlgButton(hWnd,disp,TRUE);
		but = GetICustButton(GetDlgItem(hWnd,IDC_LOD_CREATENEWSET));
		but->Disable();
		ReleaseICustButton(but);
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_DISPPIXELS),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_DISPPERCENT),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_RESET),FALSE);
		ICustButton *but;
		but = GetICustButton(GetDlgItem(hWnd,IDC_LOD_ADDTOSET));
		but->Disable();
		but->SetCheck(FALSE);
		ip->ClearPickMode();
		ReleaseICustButton(but);		
		but = GetICustButton(GetDlgItem(hWnd,IDC_LOD_CREATENEWSET));
		if (id==NODESEL_NONLOD) {
			but->Enable();
		} else {
			but->Disable();
			}
		ReleaseICustButton(but);
		}
	ListSelChanged();
	}

float LODUtil::ConvertToPercent(float size)
	{
	ISpinnerControl *iW = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWWIDTHSPIN));
	ISpinnerControl *iH = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWHEIGHTSPIN));
	int w = iW->GetIVal();
	int h = iH->GetIVal();
	ReleaseISpinner(iW);
	ReleaseISpinner(iH);
	float screen = (float)sqrt(float(w*w)+float(h*h));
	return size/screen * 100.0f;
	}

float LODUtil::ConvertFromPercent(float perc)
	{
	ISpinnerControl *iW = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWWIDTHSPIN));
	ISpinnerControl *iH = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWHEIGHTSPIN));
	int w = iW->GetIVal();
	int h = iH->GetIVal();
	ReleaseISpinner(iW);
	ReleaseISpinner(iH);
	float screen = (float)sqrt(float(w*w)+float(h*h));
	return screen*perc/100.0f;
	}

void LODUtil::SetupList()
	{
	LODCtrl *cont;	
	int sel = SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_RESETCONTENT,0,0);
	for (int i=0; i<nodes.Count(); i++) {
		cont = (LODCtrl*)nodes[i]->GetVisController();
		if (!cont) continue;
		TSTR buf;
		float val = cont->max;
		if (disp==IDC_LOD_DISPPERCENT) {
			val = ConvertToPercent(val);
			}
		buf.printf(_T("%.1f\t%s"),val,nodes[i]->GetName());
		SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_ADDSTRING,0,
			(LPARAM)(TCHAR*)buf);		
		}
	if (sel!=LB_ERR) {
		SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_SETCURSEL,sel,0);
		}
	}

void LODUtil::SetDisplayMode(int mode)
	{
	disp = mode;
	SetupList();
	ListSelChanged();
	}

void LODUtil::ListSelChanged()
	{
	ICustButton *but;

	int sel = SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_GETCURSEL,0,0);
	ISpinnerControl *iMin = GetISpinner(GetDlgItem(hWnd,IDC_LOD_MINSPIN));
	ISpinnerControl *iMax = GetISpinner(GetDlgItem(hWnd,IDC_LOD_MAXSPIN));	
	
	if (sel==LB_ERR) {
		iMin->Disable();
		iMin->SetValue(0.0f,FALSE);
		iMax->Disable();
		iMax->SetValue(0.0f,FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_MINLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_MAXLABEL),FALSE);		
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_VIEWPORTOBJ),FALSE);
		but = GetICustButton(GetDlgItem(hWnd,IDC_LOD_DELETEFROMSET));
		but->Disable();
		ReleaseICustButton(but);
	} else {
		iMin->Enable();		
		iMax->Enable();		
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_MINLABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_MAXLABEL),TRUE);		
		EnableWindow(GetDlgItem(hWnd,IDC_LOD_VIEWPORTOBJ),TRUE);
		but = GetICustButton(GetDlgItem(hWnd,IDC_LOD_DELETEFROMSET));
		but->Enable();
		ReleaseICustButton(but);

		LODCtrl *cont = (LODCtrl*)nodes[sel]->GetVisController();
		if (cont) {
		
			if (disp==IDC_LOD_DISPPERCENT) {
				iMin->SetValue(ConvertToPercent(cont->min),FALSE);
				iMax->SetValue(ConvertToPercent(cont->max),FALSE);
			} else {
				iMin->SetValue(cont->min,FALSE);
				iMax->SetValue(cont->max,FALSE);
				}
			CheckDlgButton(hWnd,IDC_LOD_VIEWPORTOBJ,cont->viewport);
			}
		}

	ReleaseISpinner(iMin);
	ReleaseISpinner(iMax);
	}

void LODUtil::FindGroupMembers(INode *root,WORD id,INodeTab &nodes)
	{
	for (int i=0; i<root->NumberOfChildren(); i++) {
		INode *node = root->GetChildNode(i);		
		Control *cont = node->GetVisController();
		if (cont && cont->ClassID()==LOD_CONTROL_CLASS_ID) {
			LODCtrl *lcont = (LODCtrl*)cont;
			if (lcont->grpID==id) {
				nodes.Append(1,&node,10);
				}
			}			
		FindGroupMembers(node,id,nodes);
		}	
	}

static BOOL IsAncestorSelected(INode *node)
	{
	if (node->GetParentNode()) {		
		if (node->GetParentNode()->Selected()) return TRUE;
		return IsAncestorSelected(node->GetParentNode());
	} else {
		return FALSE;
		}
	}

int LODUtil::GetSelID()
	{
	int id = -1;
	BOOL gotLOD=FALSE, gotNonLOD=FALSE; 

	if (!ip->GetSelNodeCount()) return NODESEL_NONE;

	INode *grpHead = FindSelGroupHead();

	for (int i=0; i<ip->GetSelNodeCount(); i++) {
		if (ip->GetSelNode(i)==grpHead) continue;
		if (IsAncestorSelected(ip->GetSelNode(i)->GetParentNode())) continue;

		Control *cont = ip->GetSelNode(i)->GetVisController();
		if (cont && cont->ClassID()==LOD_CONTROL_CLASS_ID) {
			LODCtrl *lcont = (LODCtrl*)cont;
			gotLOD = TRUE;
			if (id==-1) id = lcont->grpID;
			else if (id!=lcont->grpID) id = -2;				
		} else {
			gotNonLOD = TRUE;
			}
		}

	//if (gotNonLOD && gotLOD)  return NODESEL_LODANDNONLOD;
	if (gotLOD && id<0)       return NODESEL_LODMIX;
	if (gotNonLOD && !gotLOD) return NODESEL_NONLOD;

	return id;
	}

class NodeSize {
	public:
	INode *node; int faces;
	NodeSize() {node=NULL;faces=0;}
	NodeSize(INode *n,TimeValue t);
	void CountFaces(INode *node,TimeValue t);
	};

class NullView: public View {
	public:
		Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
		NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
	};

NodeSize::NodeSize(INode *n,TimeValue t)
	{
	node  = n;
	faces = 0;
	CountFaces(node,t);
	}

void NodeSize::CountFaces(INode *node,TimeValue t)
	{
	ObjectState os = node->EvalWorldState(t,TRUE);
	BOOL needDelete = FALSE;
	NullView view;
	if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
		GeomObject *obj = (GeomObject*)os.obj;
		Mesh *mesh = obj->GetRenderMesh(t, node, view, needDelete);
		if (mesh) faces += mesh->getNumFaces();
		if (needDelete) delete mesh;		
		}
	for (int i=0; i<node->NumberOfChildren(); i++) {
		CountFaces(node->GetChildNode(i),t);
		}
	}

static int __cdecl CompareNodeSize(const void *elem1, const void *elem2)
	{
	NodeSize *ns1 = (NodeSize*)elem1;
	NodeSize *ns2 = (NodeSize*)elem2;
	return ns1->faces - ns2->faces;	
	}

static void CheckIDs(INode *root,BitArray &used)
	{	
	Control *cont = (Control*)root->GetVisController();	
	if (cont && cont->ClassID()==LOD_CONTROL_CLASS_ID) {
		LODCtrl *lcont = (LODCtrl*)cont;
		used.Set(lcont->grpID);
		}
	for (int i=0; i<root->NumberOfChildren(); i++) {
		CheckIDs(root->GetChildNode(i),used);		
		}
	}

void LODUtil::ClearClonedGroupMap()
	{
	clonedGroupIDMap.SetCount(0);
	}

WORD LODUtil::FindClonedGroupID(WORD gid)
	{
	for (int i=0; i<clonedGroupIDMap.Count(); i++) {
		if (clonedGroupIDMap[i].srcID == gid) return clonedGroupIDMap[i].dstID;
		}
	
	// First of this group to be cloned
	WORD newID = FindUniqueGroupID();
	GroupIDMapEntry entry(gid, newID);
	clonedGroupIDMap.Append(1, &entry);
	return newID;
	}

WORD LODUtil::FindUniqueGroupID()
	{
	BitArray used;
	used.SetSize(65536);
	// CAL-09/01/03: ip may be NULL if not in utility panel. use GetCOREInterface(). (Defect #474315)
	CheckIDs(GetCOREInterface()->GetRootNode(),used);
	for (int i=0; i<used.GetSize(); i++) {
		if (!used[i]) return (WORD)i;
		}	
	return 65535;
	}

INode *LODUtil::FindSelGroupHead()
	{
	INode *grpHead = NULL;
	for (int i=0; i<ip->GetSelNodeCount(); i++) {
		if (IsAncestorSelected(ip->GetSelNode(i)->GetParentNode())) continue;
		INode *node = ip->GetSelNode(i);
		if (node->IsGroupHead()) {
			BOOL res = TRUE;
			for (int j=0; j<ip->GetSelNodeCount(); j++) {
				if (i==j) continue;
				if (IsAncestorSelected(ip->GetSelNode(j)->GetParentNode())) continue;
				INode *cnode = ip->GetSelNode(j);
				if (cnode->GetParentNode()!=node) {
					res = FALSE;
					break;
					}
				}
			if (res) {
				grpHead = node;
				break;
				}
			}
		}
	return grpHead;
	}

static void SetInheritVis(INode *node)
	{
#ifdef DESIGN_VER
	if (node->GetReference(NODE_LAYER_REF))
		((ILayer *)node->GetReference(NODE_LAYER_REF))->SetRenderByLayer(FALSE, node);
#endif
	node->SetInheritVisibility(TRUE);
	for (int i=0; i<node->NumberOfChildren(); i++) {	
		SetInheritVis(node->GetChildNode(i));
		}
	}

void LODUtil::CreateNewSet()
	{
	// See if one of the selected objects is the group head of the
	// rest of the selected objects
	INode *grpHead = FindSelGroupHead();	

	if (!grpHead) {
		TSTR buf2(GetString(IDS_RB_LODUTILITU));
		TSTR buf1(GetString(IDS_RB_LODSAMEPARENT));
		MessageBox(hWnd,buf1,buf2,MB_ICONSTOP);
		return;
		}	

	// Turn on visibility inheritance
	SetInheritVis(grpHead);

	// Build a sorted list of nodes along with their sizes
	Tab<NodeSize> nodesizes;
	for (int i=0; i<ip->GetSelNodeCount(); i++) {
		if (ip->GetSelNode(i)==grpHead) continue;
		if (IsAncestorSelected(ip->GetSelNode(i)->GetParentNode())) continue;

		NodeSize ns(ip->GetSelNode(i),ip->GetTime());
		nodesizes.Append(1,&ns,10);
		}	
	nodesizes.Sort(CompareNodeSize);
	
	// Assign LOD controllers	
	WORD grpID = FindUniqueGroupID();
	if (grpID==65535) return;

	int ct = nodesizes.Count();
	for (i=0; i<ct; i++) {
		LODCtrl *cont = new LODCtrl;		
		cont->grpID    = grpID;
		cont->order    = i;
		cont->viewport = i==0;
		nodesizes[i].node->SetVisController(cont);
		}

	SetStates();
	ResetSet();
	ip->RedrawViews(ip->GetTime());
	}

void LODUtil::ResetSet()
	{
	// Compute the target screen size
	ISpinnerControl *iW = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWWIDTHSPIN));
	ISpinnerControl *iH = GetISpinner(GetDlgItem(hWnd,IDC_LOD_VIEWHEIGHTSPIN));
	int w = iW->GetIVal();
	int h = iH->GetIVal();
	ReleaseISpinner(iW);
	ReleaseISpinner(iH);
	float size = (float)sqrt(float(w*w)+float(h*h));

	Tab<NodeSize> nodesizes;
	for (int i=0; i<nodes.Count(); i++) {		
		NodeSize ns(nodes[i],ip->GetTime());
		nodesizes.Append(1,&ns,10);
		}
	nodesizes.Sort(CompareNodeSize);
	for (i=0; i<nodesizes.Count(); i++) {
		nodes[i] = nodesizes[i].node;
		}

	float maxFaces = float(nodesizes[nodesizes.Count()-1].faces);	
	maxFaces = float(sqrt(maxFaces));

	// Reset LOD controllers		
	int ct = nodes.Count();	
	float prev = 0.0f;
	for (i=0; i<ct; i++) {		
		LODCtrl *cont  = (LODCtrl*)nodes[i]->GetVisController();
		if (!cont) continue;		
		cont->order = i;
		cont->min   = prev;
		cont->max   = (float)sqrt(float(nodesizes[i].faces))/maxFaces * size;
		prev = cont->max;		
		}
	SetBlendRegions();
	SetStates();
	}

void LODUtil::SetBlendRegions()
	{
	int ct = nodes.Count();	
	for (int i=0; i<ct; i++) {		
		LODCtrl *cont  = (LODCtrl*)nodes[i]->GetVisController();
		LODCtrl *cprev  = NULL;
		LODCtrl *cnext = NULL;
		if (!cont) continue;
		
		if (i) cprev = (LODCtrl*)nodes[i-1]->GetVisController();
		if (i<ct-1) cnext = (LODCtrl*)nodes[i+1]->GetVisController();

		if (cprev) {
			cont->bmin = cont->min - (cprev->max-cprev->min)*BLEND_PERCENT;
		} else {
			cont->bmin = 0.0f;
			}
		if (cnext) {
			cont->bmax = cont->max + (cnext->max-cnext->min)*BLEND_PERCENT;
		} else {
			cont->bmax = cont->max + (cont->max-cont->min)*BLEND_PERCENT;
			}		
		}
	}

void LODUtil::SetViewportObj(BOOL onOff)
	{
	int sel = SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_GETCURSEL,0,0);
	if (sel==LB_ERR) return;
	LODCtrl *cont = (LODCtrl*)nodes[sel]->GetVisController();
	if (!cont) return;

	for (int i=0; i<nodes.Count(); i++) {		
		LODCtrl *cnt = (LODCtrl*)nodes[i]->GetVisController();
		if (!cnt) continue;
		cnt->viewport = FALSE;
		cnt->NotifyDependents(FOREVER,PART_HIDESTATE,REFMSG_CHANGE);
		cnt->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		}
	cont->viewport = TRUE;
	cont->NotifyDependents(FOREVER,PART_HIDESTATE,REFMSG_CHANGE);

	ListSelChanged();
	ip->RedrawViews(ip->GetTime());
	}

void LODUtil::DeleteFromSet()
	{
	int sel = SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_GETCURSEL,0,0);
	if (sel==LB_ERR) return;
	
	// If this was the viewport object, pick another viewport object
	LODCtrl *cont = (LODCtrl*)nodes[sel]->GetVisController();
	if (cont && cont->viewport) {
		int next = sel+1;
		if (next>=nodes.Count()) next = 0;		
		LODCtrl *cnt = (LODCtrl*)nodes[next]->GetVisController();
		if (cnt) {
			cnt->viewport = TRUE;
			cnt->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
			cnt->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);		
			}
		}
	nodes[sel]->SetVisController(NULL);	
	SetStates();
	ip->RedrawViews(ip->GetTime());
	}

void LODUtil::SetMin(float m)
	{
	int sel = SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_GETCURSEL,0,0);
	if (sel==LB_ERR) return;
	LODCtrl *cont = (LODCtrl*)nodes[sel]->GetVisController();
	if (!cont) return;
	
	if (disp==IDC_LOD_DISPPERCENT) 
		m = ConvertFromPercent(m);
	cont->min = m;

	// Propogate down
	float min = m;
	for (int i=sel-1; i>=0; i--) {
		LODCtrl *cnt = (LODCtrl*)nodes[i]->GetVisController();
		if (!cnt) continue;
		cnt->max = min;
		if (cnt->min > cnt->max) cnt->min = cnt->max;
		min = cnt->min;
		}

	// Propogate up
	if (cont->max < cont->min) {
		cont->max = cont->min;
		float max = cont->min;
		for (int i=sel+1; i<nodes.Count(); i++) {
			LODCtrl *cnt = (LODCtrl*)nodes[i]->GetVisController();
			if (!cnt) continue;
			cnt->min = max;
			if (cnt->max < cnt->min) cnt->max = cnt->min;
			max = cnt->max;
			}
		}

	SetBlendRegions();
	SetupList();
	ListSelChanged();
	}

void LODUtil::SetMax(float m)
	{
	int sel = SendDlgItemMessage(hWnd,IDC_LOD_LIST,LB_GETCURSEL,0,0);
	if (sel==LB_ERR) return;
	LODCtrl *cont = (LODCtrl*)nodes[sel]->GetVisController();
	if (!cont) return;
	
	if (disp==IDC_LOD_DISPPERCENT) 
		m = ConvertFromPercent(m);
	cont->max = m;

	// Propogate up		
	float max = m;
	for (int i=sel+1; i<nodes.Count(); i++) {
		LODCtrl *cnt = (LODCtrl*)nodes[i]->GetVisController();
		if (!cnt) continue;
		cnt->min = max;
		if (cnt->max < cnt->min) cnt->max = cnt->min;
		max = cnt->max;
		}

	// Propogate down
	if (cont->min > cont->max) {
		cont->min = cont->max;
		float min = cont->min;
		for (int i=sel-1; i>=0; i--) {
			LODCtrl *cnt = (LODCtrl*)nodes[i]->GetVisController();
			if (!cnt) continue;
			cnt->max = min;
			if (cnt->min > cnt->max) cnt->min = cnt->max;
			min = cnt->min;
			}
		}	
	
	SetBlendRegions();
	SetupList();
	ListSelChanged();
	}

BOOL LODUtil::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
	}

BOOL LODUtil::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) {
		// Compute all the sizes for existing set members
		Tab<NodeSize> nodesizes;
		for (int i=0; i<nodes.Count(); i++) {		
			NodeSize ns(nodes[i],ip->GetTime());
			nodesizes.Append(1,&ns,10);
			}

		// Compute the size for this node
		NodeSize ns(node,ip->GetTime());

		// Make a new LOD controller
		LODCtrl *cnt   = (LODCtrl*)nodes[0]->GetVisController();
		if (!cnt) return FALSE;
		LODCtrl *cont  = new LODCtrl;		
		cont->grpID    = cnt->grpID;		
		cont->viewport = FALSE;		

		// Find its location in the list
		BOOL found = FALSE;
		for (i=0; i<nodesizes.Count(); i++) {
			if (ns.faces<nodesizes[i].faces) {
				cont->order = i;
				found = TRUE;
				}
			if (found) {
				LODCtrl *cnt = (LODCtrl*)nodes[i]->GetVisController();
				if (!cnt) return FALSE;
				cnt->order++;
				}
			}
		if (!found) {
			cont->order = i;
			}
		
		// Assign the new controller
		node->SetVisController(cont);
		
		// Redraw
		ip->RedrawViews(ip->GetTime());
		SetStates();
		}
	return FALSE;
	}

void LODUtil::EnterMode(IObjParam *ip)
	{
	ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_LOD_ADDTOSET));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	}

void LODUtil::ExitMode(IObjParam *ip)
	{
	ICustButton *but = GetICustButton(GetDlgItem(hWnd,IDC_LOD_ADDTOSET));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	}

BOOL LODUtil::Filter(INode *node)
	{
	for (int i=0; i<nodes.Count(); i++) {
		if (nodes[i]==node) return FALSE;		
		}
	INode *head = FindSelGroupHead();
	if (node->GetParentNode()!=head) return FALSE;
	return TRUE;
	}

static INT_PTR CALLBACK LODUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:			
			theLODUtil.hWnd = hWnd;
			theLODUtil.InitDlg();
			theLODUtil.SetStates();
			break;

		case WM_DESTROY:			
			break;
		
		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl*)lParam;
			switch (LOWORD(wParam)) {
				case IDC_LOD_VIEWWIDTHSPIN:
				case IDC_LOD_VIEWHEIGHTSPIN:
					theLODUtil.SetDisplayMode(theLODUtil.disp);
					break;

				case IDC_LOD_MINSPIN:
					theLODUtil.SetMin(spin->GetFVal());
					break;
				case IDC_LOD_MAXSPIN:
					theLODUtil.SetMax(spin->GetFVal());
					break;
				}
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_LOD_DISPPERCENT:
				case IDC_LOD_DISPPIXELS:
					theLODUtil.SetDisplayMode(LOWORD(wParam));
					break;

				case IDC_LOD_LIST:
					if (HIWORD(wParam)==LBN_SELCHANGE) {
						theLODUtil.ListSelChanged();
					} else 
					if (HIWORD(wParam)==LBN_DBLCLK) {
						theLODUtil.SetViewportObj(IsDlgButtonChecked(hWnd,IDC_LOD_VIEWPORTOBJ));
						}
					break;

				case IDC_LOD_CREATENEWSET:
					theLODUtil.CreateNewSet();
					break;

				case IDC_LOD_DELETEFROMSET:
					theLODUtil.DeleteFromSet();
					break;

				case IDC_LOD_ADDTOSET:
					theLODUtil.ip->SetPickMode(&theLODUtil);
					break;

				case IDC_LOD_VIEWPORTOBJ:
					theLODUtil.SetViewportObj(IsDlgButtonChecked(hWnd,IDC_LOD_VIEWPORTOBJ));
					break;

				case IDC_LOD_RESET:
					theLODUtil.ResetSet();
					break;

				case IDC_LOD_RESETOUTPUT:
					theLODUtil.ResetOutput();
					break;

				case IDOK:
					theLODUtil.iu->CloseUtility();
					break;							
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theLODUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}	

