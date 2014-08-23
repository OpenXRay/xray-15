#include "mods.h"
#include "JointMorphGizmo.h"


class PostDeleteNodePointListRestore : public RestoreObj 
	{
public:	
	GizmoJointMorphClass *mod;
	PostDeleteNodePointListRestore(GizmoJointMorphClass *mod)
		{
		this->mod = mod;
		}
	void Restore(int isUndo)
		{
		}
	void Redo()
		{
		mod->FilloutListBox();
		mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

		}
	void EndHold() 
		{

		}
	TSTR Description() {return TSTR(_T("Morph Restore"));}
	};

class DeleteNodePointListRestore : public RestoreObj {
public:	

	GizmoJointMorphClass *mod;

//	int uindex;
	int *unodeMapListData;
	Point3 *unodePointListData;

	Tab<Matrix3> utmList;
	Tab<Matrix3> rtmList;
	
	Tab<int*> unodeMapList;
	Tab<Point3*> unodePointList;
	Tab<int*> rnodeMapList;
	Tab<Point3*> rnodePointList;

	DeleteNodePointListRestore(GizmoJointMorphClass *mod, int index)
		{
		this->mod = mod;
		unodeMapList = mod->nodeMapList;
		unodePointList = mod->nodePointList;
		utmList = mod->tmList;

		unodeMapListData = mod->nodeMapList[index];
		unodePointListData = mod->nodePointList[index];

		}
	~DeleteNodePointListRestore () 
		{
		delete [] unodeMapListData;
		delete [] unodePointListData;

		}
	void Restore(int isUndo)
		{
		if (isUndo)
			{
			int ct = mod->nodeMapList.Count();

//			if (ct)
				rnodeMapList = mod->nodeMapList;
			ct = mod->nodePointList.Count();
//			if (ct) 
				rnodePointList = mod->nodePointList;
			rtmList = mod->tmList;
			}

		mod->nodeMapList = unodeMapList; 
		mod->nodePointList = unodePointList;

		mod->tmList = utmList;

		mod->FilloutListBox();
		mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

		}
	void Redo()
		{


		mod->nodeMapList = rnodeMapList; 
		mod->nodePointList = rnodePointList;

		mod->tmList = rtmList;

//		mod->FilloutListBox();
//		mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

		}
	void EndHold() 
		{

		}
	TSTR Description() {return TSTR(_T("Morph Restore"));}
};




static PickControlNode thePickMode;

BOOL PickControlNode::Filter(INode *node)
	{
	node->BeginDependencyTest();
	mod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) {		
		return FALSE;
	} else {
		return TRUE;
		}
	}

BOOL PickControlNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickControlNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) 
		{
		mod->AddMorphNode(node);
		mod->FilloutListBox();
		}
	return TRUE;
	}

void PickControlNode::EnterMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(mod->hGizmoParams,IDC_ADDNODE));
	if (iBut) 
		{
		iBut->SetType(CBT_CHECK);
		iBut->SetHighlightColor(GREEN_WASH);
		iBut->SetCheck(TRUE);
		}
	ReleaseICustButton(iBut);
	}

void PickControlNode::ExitMode(IObjParam *ip)
	{
	ICustButton *iBut = GetICustButton(GetDlgItem(mod->hGizmoParams,IDC_ADDNODE));
	if (iBut) 
		{
		iBut->SetType(CBT_CHECK);
		iBut->SetHighlightColor(GREEN_WASH);
		iBut->SetCheck(FALSE);
		}
	ReleaseICustButton(iBut);
	}


void GizmoParamsMapDlgProc::FilloutText()
{
TSTR name;
//get parent id
int parent;
giz->pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,parent, FOREVER);
TCHAR *temp = giz->bonesMod->GetBoneName(parent);
name.printf("%s: %s",GetString(IDS_PW_GIZMOPARAM_PARENT),temp);
//set the text of the parent
SetWindowText(GetDlgItem(giz->hGizmoParams,IDC_PARENT_TEXT),
					name.data());
//get child id
int child;
giz->pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_child_id,0,child, FOREVER);
//set the text of the child
temp = giz->bonesMod->GetBoneName(child);
name.printf("%s: %s",GetString(IDS_PW_GIZMOPARAM_CHILD),temp);
SetWindowText(GetDlgItem(giz->hGizmoParams,IDC_CHILD_TEXT),
					name.data());

}


void GizmoJointMorphClass::FilloutListBox()
{
int sel;

sel = SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
			LB_GETCURSEL  ,(WPARAM) 0,(LPARAM) 0);
SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
			LB_RESETCONTENT,0,0);

for (int i=0; i < pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_names); i++)
	{
	TCHAR *name;

	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_names,0,name, FOREVER,i);
	SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
				LB_ADDSTRING,0,(LPARAM) name);

	}



int iret = SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
			LB_SETCURSEL,sel,0);

		
}


void GizmoJointMorphClass::SetMorphName(int fsel)
	{
	if ( (fsel>=0) &&
   	     (fsel < pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_names)) 
		)
		{
		currentSelectedMorph = fsel;
		TCHAR *n;
		pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_names,0,n,FOREVER,currentSelectedMorph);
		pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_name,0,n,currentSelectedMorph);

		}

	}

BOOL GizmoParamsMapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
			
	{


	switch (msg) {
		case WM_INITDIALOG:
			{

			giz->hGizmoParams = hWnd;
			FilloutText();
			giz->FilloutListBox();
			giz->currentSelectedMorph = -1;
			giz->UpdateMorphList();
			giz->SetMorphName(giz->currentSelectedMorph);


			break;
			}
		case WM_CC_CHANGE_CURVEPT:
		case WM_CC_CHANGE_CURVETANGENT:
		case WM_CC_DEL_CURVEPT:
			giz->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			break;
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{
				case IDC_ADDSTACK:
					{
					giz->AddFromStack();
					giz->FilloutListBox();
					break;
					}
				case IDC_ADDNODE:
					{
					thePickMode.mod  = giz;					
					giz->ip->SetPickMode(&thePickMode);
					break;
					}

				case IDC_DELETE:
					{
					int sel;

					sel = SendMessage(GetDlgItem(giz->hGizmoParams,IDC_LIST1),
						LB_GETCURSEL  ,(WPARAM) 0,(LPARAM) 0);

					giz->DeleteMorph(sel);
					giz->FilloutListBox();
					giz->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

					break;
					}

				case IDC_LIST1:
					{
					if (HIWORD(wParam)==LBN_SELCHANGE) 
						{
						int fsel;
						fsel = SendMessage(
							GetDlgItem(hWnd,IDC_LIST1),
							LB_GETCURSEL,0,0);	
						giz->SetMorphName(fsel);


						}
					}

				}
			break;
			}



		}
	return FALSE;
	}








class GizmoJointMorphClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new GizmoJointMorphClass; }
	const TCHAR *	ClassName() { return GetString(IDS_PW_GIZMOJOINT); }
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
											   
	Class_ID		ClassID() { return GIZMOJOINTMORPH_CLASSID; }
	const TCHAR* 	Category() { return GetString(IDS_PW_GIZMOCATEGORY);}

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("gizmoJointMorph"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle



	};

static GizmoJointMorphClassDesc gizmoJointMorphDesc;
extern ClassDesc* GetGizmoJointMorphDesc() {return &gizmoJointMorphDesc;}

class GizmoJointMorphPBAccessor : public PBAccessor
{ 
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		GizmoJointMorphClass* p = (GizmoJointMorphClass*)owner;

		switch (id)
		{
			case skin_gizmoparam_name:

					if (p->bonesMod)
						p->bonesMod->UpdateGizmoList();
				
				break;
			case skin_gizmoparam_jointmorph_name:
				if ( (p->currentSelectedMorph>=0) &&
					 (p->currentSelectedMorph < p->pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_names)) 
					)
					{
					p->pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_names,0,v.s,p->currentSelectedMorph);
					p->UpdateMorphList();
	
					}

				break;
				

		}
	}
};

static GizmoJointMorphPBAccessor gizmoJoint_accessor;


// per instance gizmo block
static ParamBlockDesc2 skin_gizmojointmorphparam_blk ( skin_gizmojointmorphparam, _T("jointmorphgizmos"),  0, &gizmoJointMorphDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_BONESDEFGIZMOSJOINT, IDS_PW_GIZMOPARAM, 0,0, NULL,
	// params
	skin_gizmoparam_name, 	_T("name"),		TYPE_STRING, 	0,  IDS_PW_GIZMOPARAM_NAME,
		p_ui,  TYPE_EDITBOX,  IDC_NAME,
		p_accessor,		&gizmoJoint_accessor,
		end, 



	skin_gizmoparam_jointmorph_parent_id,  _T("parent"),	TYPE_INT, 	0, 	IDS_PW_GIZMOPARAM_PARENT, 
		end, 
	skin_gizmoparam_jointmorph_child_id,  _T("child"),	TYPE_INT, 	0, 	IDS_PW_GIZMOPARAM_CHILD, 
		end, 

	skin_gizmoparam_jointmorph_initial_angle,  _T("initial_angle"),	TYPE_FLOAT,	0, 	IDS_PW_GIZMOPARAM_INITIALANGLE, 
		p_default, 		180.f	,
		end, 


	skin_gizmoparam_jointmorph_enable,  _T("enable"),	TYPE_BOOL,	0, 	IDS_PW_ENABLE, 
		p_default, 		TRUE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_ENABLE,
		end, 

	skin_gizmoparam_jointmorph_name, 	_T("currentJointName"),		TYPE_STRING, 	0,  IDS_PW_GIZMOPARAM_NAME,
		p_ui,  TYPE_EDITBOX,  IDC_NAME2,
		p_accessor,		&gizmoJoint_accessor,
		end, 


	skin_gizmoparam_jointmorph_angles,  _T("angles"),	TYPE_FLOAT_TAB, 	0,P_VARIABLE_SIZE, 	IDS_PW_GIZMOPARAM_ANGLES, 
		end, 


	skin_gizmoparam_jointmorph_count,  _T("count"),	TYPE_INT, 	0, 	IDS_PW_GIZMOPARAM_COUNT, 
		end, 

	skin_gizmoparam_jointmorph_morphpos,  _T("morphPoints"),	TYPE_POINT3_TAB, 	0,P_VARIABLE_SIZE, 	IDS_PW_GIZMOPARAM_MPOINTS, 
		end, 

	skin_gizmoparam_jointmorph_nodes,  _T("nodes"),	TYPE_INODE_TAB, 	0,P_VARIABLE_SIZE, 	IDS_PW_GIZMOPARAM_NODES, 
		end, 


	skin_gizmoparam_jointmorph_names, 	_T("names"),		TYPE_STRING_TAB, 	0,P_VARIABLE_SIZE,  IDS_PW_GIZMOPARAM_NAMES,
		end, 

	skin_gizmoparam_jointmorph_maptable, 	_T("mapTable"),		TYPE_INT_TAB, 	0, 0, IDS_PW_GIZMOPARAM_MAPTABLE,
		end, 

	skin_gizmoparam_jointmorph_vecs,  _T("vecs"),	TYPE_POINT3_TAB, 	0,P_VARIABLE_SIZE, 	IDS_PW_GIZMOPARAM_VECS, 
		end, 

	skin_gizmoparam_jointmorph_ease,  _T("ease"),	TYPE_FLOAT, 	0, 	IDS_PW_GIZMOPARAM_EASE, 
		p_default, 		1.0f,
		end, 

	end
	);

GizmoJointMorphClass::GizmoJointMorphClass()
{
dontPropogate = FALSE;
addFromStack=FALSE;
addNode = FALSE;
pblock_gizmo_data = NULL;
//pPointList = NULL;
//nPointList = NULL;
GetGizmoJointMorphDesc()->MakeAutoParamBlocks(this);
removeDoubleTransform.IdentityMatrix();
putbackDoubleTransform.IdentityMatrix();
}
BOOL GizmoJointMorphClass::IsVolumeBased()
{

	return FALSE;
}

BOOL GizmoJointMorphClass::IsEnabled()
{
	BOOL enabled;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_enable,0,enabled,FOREVER);
	return enabled;

}

BOOL GizmoJointMorphClass::IsInVolume(Point3 p, Matrix3 tm)
{
	return TRUE;
}

void GizmoJointMorphClass::SetInitialName()
{

if ((pblock_gizmo_data) && (bonesMod))
	{
	int parent;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,parent,FOREVER);
	TSTR name;
	name.printf("%s - %s",GetString(IDS_PW_GIZMOJOINT),bonesMod->GetBoneFlat(parent)->GetName());
	pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,name.data());
	}
else pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,GetString(IDS_PW_GIZMOJOINT));
}

TCHAR *GizmoJointMorphClass::GetName()
{
TCHAR *name;
pblock_gizmo_data->GetValue(skin_gizmoparam_name,0,name,FOREVER);
return name;
}
void GizmoJointMorphClass::SetName(TCHAR *name)
{
pblock_gizmo_data->SetValue(skin_gizmoparam_name,0,name);

}



void GizmoJointMorphClass::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{

	gizmoJointMorphDesc.BeginEditParams(ip, this, flags, prev);
	skin_gizmojointmorphparam_blk.SetUserDlgProc(new GizmoParamsMapDlgProc(this));
	// install a callback for the type in.
	this->ip = ip;

	}

void GizmoJointMorphClass::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{

	gizmoJointMorphDesc.EndEditParams(ip, this, flags, next);
	ip = NULL;
	}

void GizmoJointMorphClass::Enable(BOOL enable)
{
pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_enable,0,enable);
}


BOOL GizmoJointMorphClass::InitialCreation(int count, Point3 *pt, int modCount, int *mapTable)  
{
	if (count == 0) 
		{
		MessageBox(GetCOREInterface()->GetMAXHWnd(),GetString(IDS_PW_VERTEXERROR),NULL,MB_OK);
		return FALSE;
		}
	if (modCount > 1) 
		{
		MessageBox(GetCOREInterface()->GetMAXHWnd(),GetString(IDS_PW_INSTANCEERROR),NULL,MB_OK);
		return FALSE;
		}
	int parent = -1, child = -1;
//get selected bone
	child = bonesMod->GetSelectedBone();
	INode *childNode = bonesMod->GetBoneFlat(child);
	INode *parentNode;
	parentNode = childNode->GetParentNode();
//figure out the parnet
//loop through bones looking for any that have the matching parent
	for (int i = 0; i < bonesMod->GetNumBonesFlat(); i++)
		{
		if (bonesMod->GetBoneFlat(i))
			{
			if (bonesMod->GetBoneFlat(i) == parentNode)
				{
				parent = i;
				i = bonesMod->GetNumBonesFlat();
				}
			}
		}

	
	pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_parent_id,0,parent);
	pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_child_id,0,child);

	if ((parent < 0) || (child < 0)) 
		{
		MessageBox(GetCOREInterface()->GetMAXHWnd(),GetString(IDS_PW_PARENTERROR),NULL,MB_OK);
		return FALSE;
		}

	TimeValue t = GetCOREInterface()->GetTime();
	float initialAngle = GetCurrentAngle(t);
	Point3 *initialVec = new Point3;
	*initialVec = GetCurrentVec(t);
	pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_initial_angle,0,initialAngle);

	Matrix3 parentTm;
	parentTm = bonesMod->GetBoneFlat(parent)->GetObjectTM(t);


//get initial points
	Matrix3 toParentSpace = Inverse(parentTm);
	pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_count,0,count);

//	pblock_gizmo_data->SetCount(skin_gizmoparam_jointmorph_initialpos,count);
	pblock_gizmo_data->SetCount(skin_gizmoparam_jointmorph_morphpos,count);

	for (i = 0; i < count; i++)
		{
//put points in parent bone space
		Point3 p = pt[i] * toParentSpace;
		Point3 zero(0.0f,0.0f,0.0f); 
//get set morph points
//		pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_initialpos,0,p);
		pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_morphpos,0,zero);
		}

//get set name
	TSTR name;
	name.printf("Base Morph %3.2f",initialAngle);
	TCHAR *tname = (TCHAR *) name.data();
	pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_names,1,&tname);
	pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_angles,1,&initialAngle);
	pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_vecs,1,&initialVec);

	INode *n = NULL;
	pblock_gizmo_data->SetCount(skin_gizmoparam_jointmorph_nodes,1);
	pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_nodes,0,n,0);

	int *mapList = NULL;
	nodeMapList.Append(1,&mapList);
	Point3 *pointList = NULL;
	nodePointList.Append(1,&pointList);


	for (i =0; i < count ; i++)
		{
		pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_maptable,1,&mapTable[i]);
		}
	delete initialVec;

	initialPoint = GetEndPoint(t);
	tmList.SetCount(1);
	tmList[0] = GetCurrentTm(t); 

	return TRUE;
		
}

float GizmoJointMorphClass::GetCurrentAngle(TimeValue t)
{
	Matrix3 parentTm, childTm;

	int parent,child;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,parent,FOREVER);
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_child_id,0,child,FOREVER);

	if ((parent < 0) || (child < 0)) return FALSE;

	parentTm = bonesMod->GetBoneFlat(parent)->GetObjectTM(t);
	childTm = bonesMod->GetBoneFlat(child)->GetObjectTM(t);


//get initial angle
	float initialAngle;
	Point3 l1,l2,l3,l4;
	bonesMod->GetEndPoints(parent, l1, l2);
	bonesMod->GetEndPoints(child, l3, l4);

//compute the initial angle between the joints
	l1 = l1 * parentTm;
	l2 = l2 * parentTm;
	l3 = l3 * childTm;
	l4 = l4 * childTm;
	initialAngle = fabs(acos(DotProd(Normalize(l1-l2),Normalize(l4-l3))) * 180.f/PI);
	return initialAngle;
}

Point3 GizmoJointMorphClass::GetCurrentVec(TimeValue t)
{
	Matrix3 parentTm, childTm;

	int parent,child;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,parent,FOREVER);
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_child_id,0,child,FOREVER);

	if ((parent < 0) || (child < 0)) return FALSE;

	parentTm = bonesMod->GetBoneFlat(parent)->GetObjectTM(t);
	childTm = bonesMod->GetBoneFlat(child)->GetObjectTM(t);


//get initial angle
//	float initialAngle;
	Point3 l1,l2,l3,l4;
	bonesMod->GetEndPoints(parent, l1, l2);
	bonesMod->GetEndPoints(child, l3, l4);

//compute the initial angle between the joints
	l3 = l3 * childTm * Inverse(parentTm);
	l4 = l4 * childTm * Inverse(parentTm);
	Point3 vec = Normalize(l4-l3);
	return  vec;
}


Matrix3 GizmoJointMorphClass::GetCurrentTm(TimeValue t)
{
	Matrix3  childTm;

	int child;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_child_id,0,child,FOREVER);
	childTm = bonesMod->GetBoneFlat(child)->GetObjectTM(t);

	Matrix3  parentTm;
	int parent;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,parent,FOREVER);
	parentTm = bonesMod->GetBoneFlat(parent)->GetObjectTM(t);

//compute the initial angle between the joints
	return  childTm * Inverse(parentTm);
}

Point3 GizmoJointMorphClass::GetEndPoint(TimeValue t)
{
	int child;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_child_id,0,child,FOREVER);

	if ((child < 0)) return FALSE;

//get initial angle
	Point3 l1,l2,l3,l4;
	bonesMod->GetEndPoints(child, l3, l4);
	return l3;
}


void GizmoJointMorphClass::AddMorphNode(INode *node)
{
addNode = TRUE;

TimeValue t = GetCOREInterface()->GetTime();
float angle  = GetCurrentAngle(t);
Point3 *vec=new Point3;
*vec  = GetCurrentVec(t);

pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_count,0,pCount,FOREVER);


Matrix3 ctm = GetCurrentTm(t); 

BOOL found = FALSE;
int deleteAngle = -1;
for (int i = 0; i < pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_angles); i++)
	{
	Interval iv;

	if (ctm == tmList[i])
		{
		found = TRUE;
		deleteAngle = i;
		}

	}


tempPointList.SetCount(pCount);
tempMapListFromStack.SetCount(pCount);

for (i=0; i < pCount; i++)
	{
	int mapID;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_maptable,0,mapID,FOREVER,i);
	tempMapListFromStack[i] = mapID;
	tempPointList[i] = Point3(0.0f,0.0f,0.0f);
	}


//do any enval if the number of morph points equals number of source points we can bail
//fire off a modify which grab initial pos
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//get a node by sorting through the enums
MyEnumProc dep;              
EnumDependents(&dep);
if (dep.Nodes.Count()>0)
	{
	INode *targetNode = dep.Nodes[0];
//evaluate it
	ObjectState tos = targetNode->EvalWorldState(t);
	ObjectState mos = node->EvalWorldState(t);
//if the 2 objects have the same number of points you don't need the map table
	if (tos.obj->NumPoints() != mos.obj->NumPoints())
		{
		int *pList= NULL;

		if (found)
			{
			pList = nodeMapList[deleteAngle];
			if (pList)
				delete [] pList;

			}
		pList = new int[pCount];

//now build remap list by comparing positions.
		for (i = 0; i < pCount; i++)
			{	
			Point3 a = tempPointList[i];
			for (int j = 0; j < mos.obj->NumPoints(); j++)
				{
				Point3 b = mos.obj->GetPoint(j);
				if (Length(a-b) <0.0001f)
					{
					pList[i] = j;
					}
				}
			}

		if (found)
			nodeMapList[deleteAngle] = pList;
		else nodeMapList.Append(1,&pList);

		}
	else 
		{
		int *pList = NULL;
		if (found)
			{
			pList = nodeMapList[deleteAngle];
			delete [] pList;
			nodeMapList[deleteAngle] = NULL;
			}
		else
			{
			nodeMapList.Append(1,&pList);
			}

		}

	Point3 *ppList = NULL;
	if (found)
		{
		ppList = nodePointList[deleteAngle];
		}
	else ppList = NULL;
	if (ppList) delete [] ppList;
	ppList = new Point3[pCount];
	for (i = 0; i < pCount; i++)
		{
		ppList[i] = tempPointList[i];
		}

	if (!found)
		{
		TSTR name;
		name.printf("%s angle %3.2f",node->GetName(),angle);
		TCHAR *tname = (TCHAR *) name.data();
		pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_names,1,&tname);
		pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_angles,1,&angle);
		pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_vecs,1,&vec);
		pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_nodes,1,&node);
		tmList.Append(1,&ctm);
		nodePointList.Append(1,&ppList);
		Point3 *zero = new Point3(0.0f,0.0f,0.0f);
		for (i = 0; i < pCount; i++)
			{
			pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_morphpos,1,&zero);
			}
		delete zero;

		}
	else
		{
		nodePointList[deleteAngle] = ppList;
		pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_nodes,0,node,deleteAngle*pCount);
		Point3 zero(0.0f,0.0f,0.0f);
		for (i = 0; i < pCount; i++)
			{
			pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_morphpos,0,zero,i+deleteAngle*pCount);

			}
		}
	}
delete vec;
addNode = FALSE;


}

void GizmoJointMorphClass::DeleteMorph(int index)
{
if ((index >= 0) && (index < pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_names)))
	{
	if (!theHold.Holding())
		theHold.Begin();

	theHold.Put(new DeleteNodePointListRestore(this,index));

	pblock_gizmo_data->Delete(skin_gizmoparam_jointmorph_nodes,index,1);
	pblock_gizmo_data->Delete(skin_gizmoparam_jointmorph_angles,index,1);
	pblock_gizmo_data->Delete(skin_gizmoparam_jointmorph_vecs,index,1);
	pblock_gizmo_data->Delete(skin_gizmoparam_jointmorph_morphpos,pCount*index,pCount);
	pblock_gizmo_data->Delete(skin_gizmoparam_jointmorph_names,index,1);

	theHold.Put(new PostDeleteNodePointListRestore(this));


/*	if (nodeMapList[index])
		{
		int *nl = nodeMapList[index];
		delete [] nl;
		}
*/
	nodeMapList.Delete(index,1);
/*	if (nodePointList[index])
		{
		Point3 *nl = nodePointList[index];
		delete [] nl;
		}
*/
	nodePointList.Delete(index,1);
	tmList.Delete(index,1);

	theHold.Accept(GetString(IDS_PW_DELETE_MORPH));

	}

}

void GizmoJointMorphClass::AddFromStack()
{
addFromStack = TRUE;
TimeValue t = GetCOREInterface()->GetTime();
float angle  = GetCurrentAngle(t);
Point3 *vec  = new Point3;
*vec = GetCurrentVec(t);
Matrix3 ctm = GetCurrentTm(t);

pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_count,0,pCount,FOREVER);
tempPointList.SetCount(pCount);
tempMapListFromStack.SetCount(pCount);

for (int i=0; i < pCount; i++)
	{
	int mapID;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_maptable,0,mapID,FOREVER,i);
	tempMapListFromStack[i] = mapID;
	tempPointList[i] = Point3(0.0f,0.0f,0.0f);
	}



BOOL found = FALSE;
int deleteAngle = -1;
for (i = 0; i < pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_angles); i++)
	{
	Interval iv;
	if (ctm == tmList[i])
		{
		found = TRUE;
		deleteAngle = i;
		}


	}




addFromStack = TRUE;
//fire off a modify which grab initial pos
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//get a node by sorting through the enums
MyEnumProc dep;              
EnumDependents(&dep);
if (dep.Nodes.Count()>0)
	{
	INode *node = dep.Nodes[0];
//evaluate it
	ObjectState nos = node->EvalWorldState(t);
	Matrix3 wtm = node->GetObjectTM(t);
	Interval valid;

	int parent;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,parent,FOREVER);
	
	Matrix3 tm = bonesMod->GetBoneFlat(parent)->GetObjectTM(t);
	Matrix3 itm= Inverse(tm);
//loop through its points looking for matches
	Point3 *offset = new Point3;


	Interval iv;
	for (i=0; i < pCount; i++)
		{
//get points off the stack and produce an offset
		int mapID;
		mapID = tempMapListFromStack[i];
		Point3 p = nos.obj->GetPoint(mapID);
		Point3 initialP;
		initialP = tempPointList[i];
		*offset = p - initialP;
		*offset = VectorTransform(wtm,*offset);
		*offset = VectorTransform(itm,*offset);
		if (!found)
			pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_morphpos,1,&offset);
		else 
			{
			Point3 a;
			pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_morphpos,0,*offset,i+deleteAngle*pCount);
			}

		}
	delete offset;

//assign a name
	if (!found)
		{
		TSTR name;
		name.printf("Stack Morph angle %3.2f",angle);
		TCHAR *tname = (TCHAR *) name.data();
		pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_names,1,&tname);
		pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_angles,1,&angle);
		pblock_gizmo_data->Append(skin_gizmoparam_jointmorph_vecs,1,&vec);
		INode *node = NULL;
		int nodeCount = pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_nodes);
		pblock_gizmo_data->SetCount(skin_gizmoparam_jointmorph_nodes,nodeCount+1);
		pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_nodes,0,node,nodeCount);
 		tmList.Append(1,&ctm);

		int *plist = NULL;
		nodeMapList.Append(1,&plist);
		Point3 *pplist = NULL;
		nodePointList.Append(1,&pplist);
		}

	}
delete vec;
addFromStack = FALSE;
tempMapListFromStack.ZeroCount();
}

void GizmoJointMorphClass::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)
{

}

int GizmoJointMorphClass::Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm) 
{

	if (noMorphs) return 1;

	int ct = initialPos.Count();
	for (int i = 0; i < ct; i++)
		{
		Point3 pList[3];
		pList[0] = 	initialPos[i] * removeDoubleTransform;
		pList[1] = 	finalPos[i] * removeDoubleTransform;
		gw->marker(&pList[0],SM_DOT_MRKR);
		gw->marker(&pList[1],SM_DOT_MRKR);
		gw->polyline(2, pList, NULL, NULL, 0);
		}



return 1;
}


void GizmoJointMorphClass::PostDeformSetup(TimeValue t)
{
}

class SortClass
	{
public:
	int id;
	float angle;
	};

static int sort( const void *elem1, const void *elem2 ) {
	SortClass *a = (SortClass *)elem1;
	SortClass *b = (SortClass *)elem2;
	if ( a->angle == b->angle)
		return 0;
	if ( a->angle < b->angle)
		return -1;
	else
		return 1;

}

void GizmoJointMorphClass::EvalNode(INode *node,int knot, TimeValue t)
{
if (!node) return;
ObjectState os = node->EvalWorldState(t);

MyEnumProc dep;              
EnumDependents(&dep);
INode *bnode ;
if (dep.Nodes.Count()>0)
	{
	bnode = dep.Nodes[0];
	}
else return;

Matrix3 wtm = bnode->GetObjectTM(t);
Interval valid;

int parent;
pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,parent,FOREVER);

Matrix3 tm = bonesMod->GetBoneFlat(parent)->GetObjectTM(t);
Matrix3 itm= Inverse(tm);

Matrix3 concat = wtm * itm;

int *mapList = nodeMapList[knot];
Point3 *pointList = nodePointList[knot];
for (int i =0; i < pCount; i++)
	{
	int id = 0;
	Point3 a(0.0f,0.0f,0.0f);
	Point3 b(0.0f,0.0f,0.0f);
	b = pointList[i];
	
	if ( mapList == NULL)
		{
		pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_maptable,0,id,FOREVER,i);
		}
	else
		{
		id = mapList[i];
		}

	if ((id < os.obj->NumPoints()) && (id >=0))
		a = os.obj->GetPoint(id);
	a = a * concat;
	b = b * concat;
	a = a-b;


	dontPropogate = TRUE;
	theHold.Suspend();
	pblock_gizmo_data->SetValue(skin_gizmoparam_jointmorph_morphpos,0,a,i+knot*pCount);
	theHold.Resume();
	dontPropogate = FALSE;

	}

}



void GizmoJointMorphClass::PreDeformSetup(TimeValue t)
{


//bend the lattice
//points based on weights in local space
//child tm
int parentId, childId;
Interval iv;
pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,parentId,FOREVER);
pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_child_id,0,childId,FOREVER);

if ((childId<0) || (parentId<0)) 
	{
	val = FALSE;
	return;
	}

INode *pNode = bonesMod->GetBoneFlat(parentId);
INode *cNode = bonesMod->GetBoneFlat(childId);
if (pNode && cNode)
	val= TRUE;
else
	{
	val = FALSE;
	return;
	}

float jointmorph = 0.0f;
BOOL useGraph = FALSE;




Matrix3 toWorld;
Matrix3 parentTm;
Matrix3 childTm;
int id;
pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_parent_id,0,id,FOREVER);

toWorld = bonesMod->GetBoneFlat(id)->GetObjectTM(t);
gizmoTm = toWorld;
igizmoTm = Inverse(toWorld);
parentTm = toWorld;

childTm = bonesMod->GetBoneFlat(childId)->GetObjectTM(t);


float currentAngle, initialAngle;

pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_initial_angle,0,initialAngle,FOREVER);

currentAngle = GetCurrentAngle(t);
Point3 vec = GetCurrentVec(t);

per = 0.f;
float ang = currentAngle;

currentBendAngle = ang;

per = 0.0f;
int numberAngles;

numberAngles = pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_names);

Tab<SortClass> sortList;
sortList.SetCount(numberAngles);
for (int i=0; i < numberAngles; i++)
	{
	float angle;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_angles,0,angle, FOREVER,i);
	sortList[i].id = i;
	sortList[i].angle = angle; 
	}

if (numberAngles <= 0) 
	{
	noMorphs = TRUE;
	return;
	}
else noMorphs = FALSE;

sortList.Sort(sort);
//find angle
int aid = -2;
for (i=0; i < numberAngles; i++)
	{
	if ( sortList[i].angle > currentAngle )
		{
		aid = i-1;
		i = numberAngles;
		}
	}
if (aid == -1)
	{
	pKnot = sortList[0].id;
	nKnot = sortList[0].id;
	per = 0.0f;

	}
else if (aid == -2)
	{
	pKnot = sortList[numberAngles-1].id;
	nKnot = sortList[numberAngles-1].id;
	per = 0.0f;

	}
else
	{
	pKnot = sortList[aid].id;
	nKnot = sortList[aid+1].id;
	float pAngle,nAngle;
	pAngle = sortList[aid].angle;
	nAngle = sortList[aid+1].angle;
	per = (currentAngle-pAngle)/(nAngle-pAngle);
	
	}
if (ip)
	{
	TSTR angleText;	
	angleText.printf(GetString(IDS_CURRENT_ANGLE),currentBendAngle);
//set the text of the parent
	SetWindowText(GetDlgItem(hGizmoParams,IDC_ANGLE_TEXT),
					angleText.data());

	}



overallPer = 1.0f;
	{
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_count,0,pCount,FOREVER);

	int max = -1;
	for (i=0; i < pCount; i++)
		{
		int mapID;
		pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_maptable,0,mapID,FOREVER,i);
		if (mapID > max) max = mapID;
		}
	tempMapList.SetCount(max+1);
	for (i=0; i < max; i++)
		tempMapList[i] = 0;

	for (i=0; i < pCount; i++)
		{
		int mapID;
		pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_maptable,0,mapID,FOREVER,i);
		tempMapList[mapID] = i;
		}


	}
//now check if any of the targets are nodes if so our point list needs to be updated
//pPointList = new Point3[pCount];
//nPointList = new Point3[pCount];
for (i=0; i < numberAngles; i++)
	{
	INode *node;
	pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_nodes,0,node,FOREVER,i);
	if (node)
		EvalNode(node,i,t);
	}

tempDists.SetCount(numberAngles);
tempPer.SetCount(numberAngles);


numberGizmos = numberAngles;
Matrix3 ctm = GetCurrentTm(t);

Point3 cx,cy,cz;
Point3 ox,oy,oz;
cx = initialPoint;
cx.x+=1.0f;
cy = initialPoint;
cx.y+=1.0f;
cz = initialPoint;
cz.z +=1.0f;
ox = cx;
oy = cy;
oz = cz;

cx = cx * ctm;
cy = cy * ctm;
cz = cz * ctm;

float ease;

pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_ease,0,ease,FOREVER);

for (i=0; i < numberAngles; i++)
	{
   	tempPer[i] = 0.0f;
	tempDists[i] = 0.0f;

	Point3 x,y,z;
	x = ox * tmList[i];
	y = oy * tmList[i];
	z = oz * tmList[i];
	float dist = Length(cx-x)+Length(cy-y)+Length(cz-z);
	tempDists[i] = pow(dist,ease);
	}


float largest = -1.0f;
for (i=0; i < numberAngles; i++)
	{
	if (tempDists[i]  > largest)
		largest = tempDists[i];
	}
int nuke = -1;
for (i=0; i < numberAngles; i++)
	{
	if (tempDists[i] < 0.001f)
		{
		nuke = i;
		i = numberAngles;
		}
	else
		{
		tempPer[i] = largest/tempDists[i];
		}
	}
if (nuke != -1)
	{
	for (i=0; i < numberAngles; i++)
		{
		tempPer[i] = 0.0f;
		}
	tempPer[nuke] = 1.0f;

	}
else
	{
	float sum = 0.0f;
	for (i=0; i < numberAngles; i++)
		{
		sum += tempPer[i];
		}
	for (i=0; i < numberAngles; i++)
		{
		float ang = tempPer[i];
		tempPer[i] = tempPer[i]/sum;
		}


  }
if (initialPos.Count()!=pCount)
	initialPos.SetCount(pCount);
if (finalPos.Count()!=pCount)
	finalPos.SetCount(pCount);

}



Point3 GizmoJointMorphClass::DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm)

{

//new
//p = p * removeDoubleTransform;
//if we adding from the stack temporarily turn off the deformation so we can record the points

if ((addFromStack) || (addNode))
	{
	int id = -1;
	for (int i=0; i < pCount; i++)
		{
		int mapID;
		mapID = tempMapListFromStack[i];
		if (index == mapID)
			{
			id = i;
			i = pCount;
			}
		}
	if (id != -1)
		{
		p = p * removeDoubleTransform;
		tempPointList[id] = p;
		p = p * putbackDoubleTransform;
		}

	}
if (!noMorphs)
	{
	

//deform the points
//move into bone space
	int id = tempMapList[index];
	Point3 a,b;

//new

	initialPos[id]=p ;

//to world
	p = p * tm;
//to bonespace
	p = p * igizmoTm;

	for (int i = 0; i < numberGizmos; i++)
		{
		pblock_gizmo_data->GetValue(skin_gizmoparam_jointmorph_morphpos,0,a, FOREVER,i*pCount+id);

		float per = tempPer[i];

		p += a * tempPer[i];
		}
	p = p * gizmoTm * Inverse(tm);


	finalPos[id] = p ;

	
	}



return p;
}





IOResult GizmoJointMorphClass::Save(ISave *isave)
	{


	ULONG nb;

	isave->BeginChunk(GIZMOTM_CHUNK);
	gizmoTm.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(GIZMOPOINTCOUNT_CHUNK);
	isave->Write(&pCount,sizeof(pCount),&nb);
	isave->EndChunk();

	int ct = pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_names);

	isave->BeginChunk(GIZMOMORPHCOUNT_CHUNK);
	isave->Write(&ct,sizeof(ct),&nb);
	isave->EndChunk();

	isave->BeginChunk(GIZMOMORPHINITIALPOINT_CHUNK);
	isave->Write(&initialPoint,sizeof(initialPoint),&nb);
	isave->EndChunk();

	for (int i=0; i < ct; i++)
		{
		isave->BeginChunk(GIZMOMORPHTMS_CHUNK);
		tmList[i].Save(isave);
		isave->EndChunk();
		}

	for (i=0; i < ct; i++)
		{
		if (nodeMapList[i])
			{
			int *map = nodeMapList[i];
			isave->BeginChunk(GIZMOMORPHMAPS_CHUNK);
			isave->Write(&i,sizeof(i),&nb);
			isave->Write(map,sizeof(int)*pCount,&nb);
			isave->EndChunk();
			}

		if (nodePointList[i])
			{
			Point3 *map = nodePointList[i];
			isave->BeginChunk(GIZMOMORPHPOINTS_CHUNK);
			isave->Write(&i,sizeof(i),&nb);
			isave->Write(map,sizeof(Point3)*pCount,&nb);
			isave->EndChunk();
			}

		}

	return IO_OK;
	}

IOResult GizmoJointMorphClass::Load(ILoad *iload)
	{

	
	IOResult res = IO_OK;
	
	int ct;
	ULONG nb;
	int i;
	int tmct = 0;
	while (IO_OK==(res=iload->OpenChunk())) {
		int id = iload->CurChunkID();
		switch(id)  {

			case GIZMOTM_CHUNK: 
				{
				gizmoTm.Load(iload);
				break;
				}
			case GIZMOPOINTCOUNT_CHUNK: 
				{
				iload->Read(&pCount,sizeof(pCount),&nb);
				break;
				}

			case GIZMOMORPHCOUNT_CHUNK: 
				{
				iload->Read(&ct,sizeof(ct),&nb);
				nodeMapList.SetCount(ct);
				nodePointList.SetCount(ct);
				tmList.SetCount(ct);
				for (i=0; i < ct; i++)
					{
					nodeMapList[i] = NULL;
					nodePointList[i] = NULL;
					}
				break;
				}
			case GIZMOMORPHINITIALPOINT_CHUNK: 
				{
				iload->Read(&initialPoint,sizeof(initialPoint),&nb);
				break;
				}
			case GIZMOMORPHTMS_CHUNK: 
				{
//				for (i=0; i < ct; i++)
				tmList[tmct].Load(iload);
				tmct++;
				break;
				}
			case GIZMOMORPHMAPS_CHUNK: 
				{
				int *n = new int[pCount];
				int c;
				iload->Read(&c,sizeof(c),&nb);
				iload->Read(n,sizeof(int)*pCount,&nb);
				nodeMapList[c] = n;
				}
			case GIZMOMORPHPOINTS_CHUNK: 
				{
				Point3 *n = new Point3[pCount];
				int c;
				iload->Read(&c,sizeof(c),&nb);
				iload->Read(n,sizeof(Point3)*pCount,&nb);
				nodePointList[c] = n;
				}


		


			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}


static HIMAGELIST hToolsImages = NULL;
void GizmoJointMorphClass::LoadResources()
	{

	if (!hToolsImages) {
		HBITMAP hTBitmap, hTMask;	
		hToolsImages = ImageList_Create(16, 15, TRUE, 16, 0);
		hTBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GRAPHBITMAP));
		hTMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GRAPHBITMAPMASK));
		ImageList_Add(hToolsImages,hTBitmap,hTMask);
		DeleteObject(hTBitmap);
		DeleteObject(hTMask);
		}

	}

void *GizmoJointMorphClass::GetInterface(ULONG id)
{
	if(id == I_RESMAKER_INTERFACE)
		return (void *) (ResourceMakerCallback *) this;
	else if(id == I_GIZMO)
		return (void *) (GizmoClass *) this;
	else if(id == I_GIZMO2)
		return (void *) (IGizmoClass2 *) this;
	else
		return (void *) NULL;
}


BOOL GizmoJointMorphClass::GetToolTip(int iButton, TSTR &ToolTip,ICurveCtl *pCCtl)
{
	switch(iButton)
	{
		case 0:ToolTip = _T("Bulge");break;
		default:
			ToolTip = _T(" ");break;
	}
	return TRUE;
}
BOOL GizmoJointMorphClass::SetCustomImageList(HIMAGELIST &hCTools,ICurveCtl *pCCtl)
{

	LoadResources();
	hCTools = hToolsImages;
	return TRUE;
}

Interval GizmoJointMorphClass::LocalValidity(TimeValue t)
{
	return FOREVER;

}


void GizmoJointMorphClass::UpdateMorphList()
{

int sel;

sel = SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
			LB_GETCURSEL  ,(WPARAM) 0,(LPARAM) 0);


SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
			LB_RESETCONTENT,0,0);

for (int i =0; i < pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_names); i++) 
	{

	
	TCHAR *name = pblock_gizmo_data->GetStr(skin_gizmoparam_jointmorph_names,0,i);
	SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
				LB_ADDSTRING,0,(LPARAM) name);
	}
int iret = SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
			LB_SETCURSEL,sel,0);
if (iret == LB_ERR)
	{
	int sel = pblock_gizmo_data->Count(skin_gizmoparam_jointmorph_names);
	int tret = SendMessage(GetDlgItem(hGizmoParams,IDC_LIST1),
			LB_SETCURSEL,sel,0L);
	if (tret != LB_ERR)
		currentSelectedMorph = sel;
	else currentSelectedMorph = -1;
	}

}



RefResult GizmoJointMorphClass::NotifyRefChanged(
		Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message)
	{

	if (dontPropogate) return REF_STOP;

	switch (message) {
		case REFMSG_CHANGE:
			if ( (hTarget == pblock_gizmo_data))
				{
				ParamID changing_param = pblock_gizmo_data->LastNotifyParamID();
				skin_gizmojointmorphparam_blk.InvalidateUI(changing_param);
				}
			break;
		}

	return REF_SUCCEED;
	}

IGizmoBuffer *GizmoJointMorphClass::CopyToBuffer()
{

//new a class specific version
GizmoJointMorphBuffer *buffer = new GizmoJointMorphBuffer();
buffer->cid = ClassID();
buffer->mod = this;

return (IGizmoBuffer *) buffer;

}

void GizmoJointMorphClass::PasteFromBuffer(IGizmoBuffer *buf)
{
//copy buffer to param block and curve graph
GizmoJointMorphBuffer *buffer = (GizmoJointMorphBuffer *) buf;

if (buffer->mod==this) return;

ReplaceReference(0,buffer->mod->pblock_gizmo_data->Clone());
initialPoint = buffer->mod->initialPoint;
tmList = buffer->mod->tmList;

//Tab<int*> nodeMapList;
int ct = nodeMapList.Count();
for (int i = 0; i < ct; i++)
	{
	if (nodeMapList[i])
		delete [] nodeMapList[i];
	if (nodePointList[i])
		delete [] nodePointList[i];
	}
ct = buffer->mod->nodeMapList.Count();
nodeMapList.SetCount(ct);
nodePointList.SetCount(ct);
pCount = buffer->mod->pCount;
for (i = 0; i < ct; i++)
	{
	nodeMapList[i] = NULL;
	nodePointList[i] = NULL;
	if (buffer->mod->nodeMapList[i])
		{
		int *in = new int[pCount];
		int *from = buffer->mod->nodeMapList[i];
		for (int j=0; j < pCount; j++)
			in[j] = from[j];
		nodeMapList[i] = in;
		}
	if (buffer->mod->nodePointList[i])
		{
		Point3 *in = new Point3[pCount];
		Point3 *from = buffer->mod->nodePointList[i];
		for (int j=0; j < pCount; j++)
			in[j] = from[j];
		nodePointList[i] = in;
		}

	}
}


//new
void GizmoJointMorphClass::SetBackTransFormMatrices(Matrix3 removeDoubleTransform, Matrix3 putbackDoubleTransform)
{
	this->removeDoubleTransform = removeDoubleTransform;
	this->putbackDoubleTransform = putbackDoubleTransform;
}
