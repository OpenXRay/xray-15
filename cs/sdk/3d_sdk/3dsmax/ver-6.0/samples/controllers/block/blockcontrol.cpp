#include "block.h"
#include "units.h"
#include "masterblock.h"
#include "istdplug.h"

#include "iparamm2.h"

static BlockControlClassDesc blockControlCD;
ClassDesc* GetBlockControlDesc() {return &blockControlCD;}

/*


enum { block_params };
// path_params param IDs
enum { block_slaves };

// per instance path controller block
static ParamBlockDesc2 block_paramblk (block_params, _T("MasterParameters"),  0, &blockCD, P_AUTO_CONSTRUCT + P_AUTO_UI, BLOCK_PBLOCK_REF, 
	//rollout
	0, 0, 0, 0, NULL,
	// params
		block_slaves,	_T("Slaves"),		TYPE_REFTARG_TAB,10, P_ANIMATABLE, 	IDS_PW_SLAVE, 
		end, 
	end
	);

*/

INT_PTR CALLBACK BlockPropDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TrackPropDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//------------------------------------------------------------

BlockControl::BlockControl() 
	{	
	range     = Interval(GetAnimStart(),GetAnimEnd());
//	blockCD.MakeAutoParamBlocks(this);
//watje 4-30-99
	suspendNotifies = FALSE;

	} 
BlockControl::~BlockControl() 
	{	
	for (int i = 0; i < tempControls.Count(); i++)
		delete tempControls[i];
	} 


int BlockControl::NumSubs() 
	{
	return controls.Count();
	}

Animatable* BlockControl::SubAnim(int i) 
	{
	if (i < controls.Count()) return controls[i];
	return NULL;
	}

TSTR BlockControl::SubAnimName(int i) 
	{
	TSTR name;
	if (i < names.Count())
		{
		if (names[i] && names[i]->length()) 
			{
			name = *names[i];
			}
		} 
	else if (controls[i]) 
			{
			controls[i]->GetClassName(name);
			} 

	return name;
	}


BOOL BlockControl::AssignController(Animatable *control,int subAnim) 
	{
	
	MessageBox(  GetCOREInterface()->GetMAXHWnd(),          // handle of owner window
					 (LPCTSTR) GetString(IDS_PW_ERROR_MSG),     // address of text in message box
					(LPCTSTR) NULL,  // address of title of message box
						MB_OK | MB_ICONWARNING | MB_APPLMODAL );         // style of message box);
//	ReplaceReference(Control::NumRefs(),(Control*)control);
	return FALSE;
	}
//MessageBox(A, B, "Error !", MB_OK | MB_ICONWARNING | MB_APPLMODAL)
/*
int BlockControl::SubNumToRefNum(int subNum) 
	{	
	return Control::NumRefs();
	}
*/
int BlockControl::NumRefs() 
	{
//DebugPrint("Block Num ref %d\n",controls.Count());

	return controls.Count();
	}

RefTargetHandle BlockControl::GetReference(int i) 
	{

	if (i < controls.Count()) 
		{
//		DebugPrint("Getting block control ref %d\n",i);
		return controls[i];
		}
	return NULL;
//	return cont;
	}

void BlockControl::SetReference(int i, RefTargetHandle rtarg) 
	{
//	DebugPrint("Setting block control ref %d\n",i);

	if (i==controls.Count() && rtarg) {
		controls.Resize(controls.Count()+1);
		}
	if (i==controls.Count() && !rtarg) {
		return;
		}

	controls[i] = (Control*)rtarg;

	}

void BlockControl::NotifySlaves()

{
//watje 4-30-99
if (!suspendNotifies)
	{

	int i, count;
	count = backPointers.Count();

	for (i=0; i<count; i++) 
		{
		if ( (backPointers[i]) && (backPointers[i]->master))
			{
			backPointers[i]->UpdateSlave();
			backPointers[i]->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		}
	count = externalBackPointers.Count();
	for (i=0; i<count; i++) 
		{
		if (externalBackPointers[i])
			{
			externalBackPointers[i]->UpdateSlave();
			externalBackPointers[i]->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		}
	}

			
}

RefResult BlockControl::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
     	PartID& partID,  
     	RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
//			Invalidate();	
			NotifySlaves();
			break;
		
		}
	return REF_SUCCEED;
	}




//extern float noise1(float arg);



RefTargetHandle BlockControl::Clone(RemapDir& remap)
	{
	// make a new noise controller and give it our param values.
//	BlockControl *cont = new BlockControl;
//	*cont = *this;
	// Clone the strength controller
//	cont->ReplaceReference(Control::NumRefs(),remap.CloneRef(cont->cont));
//	CloneControl(cont,remap);
	BaseClone(this, NULL, remap);
	return NULL;
	}
/*
void BlockControl::GetValueLocalTime(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	}

void BlockControl::Extrapolate(
		Interval range,TimeValue t,void *val,Interval &valid,int type)
	{
	}

*/

void BlockControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
//DebugPrint("Error occured this getValue should never be called on a block control\n");
//	float *v = (float*)val;

//	if (method==CTRL_ABSOLUTE) {
//		*v = 0.0f;
//		}

//	sub->GetValue(t,val,valid,CTRL_RELATIVE);
	}

void BlockControl::GetValue(
		TimeValue t, void *val, Interval &valid, int whichSub, GetSetMethod method)
	{
//DebugPrint("Error occured this getValue should never be called on a block control\n");
	float *v = (float*)val;

//	if (method==CTRL_ABSOLUTE) {
//		*v = 0.0f;
//		}

	controls[whichSub]->GetValue(t,val,valid,method);
	}

void BlockControl::SetValue(
		TimeValue t, void *val, int commit, GetSetMethod method)
	{

//this should never get called
//DebugPrint("Error occured SetValue should never be called on a block control\n");
/*
	if (method==CTRL_ABSOLUTE) {
		float v = *((float*)val);
		float before = 0.0f, after = 0.0f;
		Interval valid;
		sub->GetValue(t,&before,valid,CTRL_RELATIVE);
		sub->GetValue(t,&after,valid,CTRL_RELATIVE);
		v = -before + v + -after;
		sub->SetValue(t,&v,commit,method);
	} else {
		sub->SetValue(t,val,commit,method);
		}
*/
	}


void BlockControl::RebuildTempControl()
{


for (int i =0; i < controls.Count();i++)
//Nuke all keys
	{
	if (tempControls[i] == NULL)
		tempControls[i] = (Control *) controls[i]->Clone();
/*
		Blocks[whichBlock]->tempControls[whichSub]->DeleteKeys(TRACK_DOALL);

		if (Blocks[whichBlock]->tempControls[whichSub]->SuperClassID() == CTRL_FLOAT_CLASS_ID)
			{
			float f = 0.0f;
			Blocks[whichBlock]->tempControls[whichSub]->SetValue(0,&f);
			}
		else if (Blocks[whichBlock]->tempControls[whichSub]->SuperClassID() == CTRL_POSITION_CLASS_ID)
			{
			Point3 f(0.0f,0.0f,0.0f);
			Blocks[whichBlock]->tempControls[whichSub]->SetValue(0,&f);
			}
*/
	tempControls[i]->DeleteKeys(TRACK_DOALL);
	float f = 0.0f;
	Point3 p(0.0f,0.0f,0.0f);
	if (tempControls[i]->SuperClassID() == CTRL_FLOAT_CLASS_ID)
		tempControls[i]->SetValue(0,&f);
	else if ( (tempControls[i]->SuperClassID() == CTRL_POSITION_CLASS_ID) || (tempControls[i]->SuperClassID() == CTRL_POINT3_CLASS_ID))
		tempControls[i]->SetValue(0,&p);

	}
}
void BlockControl::AddKeyToTempControl(TimeValue t,  TimeValue scale, BOOL isRelative)
{
//DebugPrint("key data\n");
for (int i =0; i < controls.Count();i++)
//Nuke all keys
	{
	
//copy track in 
//need to add relative controls
	Interval iv(start,end);
	TrackClipObject *cpy = controls[i]->CopyTrack(iv, TIME_INCLEFT|TIME_INCRIGHT);
//	int sz = sizeof(&cpy);
	iv.Set(t,t + (end-start));
	DWORD flags = TIME_INCLEFT|TIME_INCRIGHT;
	if (isRelative)
		{
		flags |= PASTE_RELATIVE;
//look at last key	and get offset
		}

	tempControls[i]->PasteTrack(cpy, iv, flags);
//now loop through and add relative value

//now need to scale those keys
	float s = 1.0f;
	if ((end-start) != 0)
		 s =  (float)scale/(float)(end-start);

	tempControls[i]->ScaleTime(iv, s);

//	int numKeys = tempControls[i]->NumKeys();
	cpy->DeleteThis();
//			}
//		tempControls[i]->DeleteKeys(TRACK_DOALL);
	}

}


void BlockControl::AddKeyToSub(Control *sub, int whichSub, TimeValue t,  TimeValue scale, Interval mrange,BOOL isRelative)
{
//DebugPrint("key data\n");
int i = whichSub;
//for (int i =0; i < controls.Count();i++)
//Nuke all keys
	{
	
//copy track in 
//need to add relative controls
	Interval iv(start,end);
	TrackClipObject *cpy = controls[i]->CopyTrack(iv, TIME_INCLEFT|TIME_INCRIGHT);
//	int sz = sizeof(&cpy);
//now need to scale those keys
	float s = 1.0f;
	if ((end-start) != 0)
		 s =  (float)scale/(float)(end-start);
   	iv.Set(t,t + (end-start));
	DWORD flags = TIME_INCLEFT|TIME_INCRIGHT;
	if (isRelative)
		{
		flags |= PASTE_RELATIVE;
//look at last key	and get offset
		}

	Interval scaleIV = iv;
	iv = iv & mrange;
	sub->PasteTrack(cpy, iv, flags);
//now loop through and add relative value


	sub->ScaleTime(scaleIV, s);

//	int numKeys = tempControls[i]->NumKeys();
	cpy->DeleteThis();
//			}
//		tempControls[i]->DeleteKeys(TRACK_DOALL);
	}

}

/*
void BlockControl::AddKeyToSub(Control *sub,int whichSub, BOOL isRelative)
{
DebugPrint("key data\n");
int i = whichSub;
//for (int i =0; i < controls.Count();i++)
//Nuke all keys
//	{
	
//copy track in 
//need to add relative controls
	Interval iv= tempControls[i]->GetTimeRange(TIMERANGE_ALL);



	TrackClipObject *cpy = tempControls[i]->CopyTrack(iv, TIME_INCLEFT|TIME_INCRIGHT);
//	int sz = sizeof(&cpy);
//	iv.Set(t,t + (end-start));
	DWORD flags = TIME_INCLEFT|TIME_INCRIGHT;
	if (isRelative)
		{
		flags |= PASTE_RELATIVE;
//look at last key	and get offset
		}

	sub->PasteTrack(cpy, iv, flags);
//now loop through and add relative value

//now need to scale those keys
//	float s = 1.0f;
//	if ((end-start) != 0)
//		 s =  (float)scale/(float)(end-start);

//	sub->ScaleTime(iv, s);

	cpy->DeleteThis();
//	}

}
*/
#define BLOCKCOUNT_CHUNK		0x01010
#define NAME_CHUNK				0x01020
#define NONAME_CHUNK			0x01030
#define COLOR_CHUNK				0x01040
#define START_CHUNK				0x01050
#define END_CHUNK				0x01060
#define BACKPOINTERS_CHUNK		0x01070
#define EXBACKPOINTERS_COUNT_CHUNK	0x01080
#define EXBACKPOINTERS_CHUNK	0x01090



IOResult BlockControl::Save(ISave *isave)
	{		
	ULONG nb;	
//count
	int count = controls.Count();
	isave->BeginChunk(BLOCKCOUNT_CHUNK);
	isave->Write(&count,sizeof(int),&nb);			
	isave->EndChunk();
//names

	for (int i=0; i<count; i++) {
		if (names[i]) {
			isave->BeginChunk(NAME_CHUNK);
			isave->WriteWString(*names[i]);
			isave->EndChunk();
		} else {
			isave->BeginChunk(NONAME_CHUNK);
			isave->EndChunk();
			}
		}
//color
	Color c = color;
	isave->BeginChunk(COLOR_CHUNK);
	isave->Write(&c,sizeof(c),&nb);			
	isave->EndChunk();

//start
	TimeValue s = start;
	isave->BeginChunk(START_CHUNK);
	isave->Write(&s,sizeof(s),&nb);			
	isave->EndChunk();

//end
	TimeValue e = end;
	isave->BeginChunk(END_CHUNK);
	isave->Write(&e,sizeof(e),&nb);			
	isave->EndChunk();

//back pointers
	isave->BeginChunk(BACKPOINTERS_CHUNK);
	for (i=0; i<count; i++) 
		{
		ULONG id = isave->GetRefID(backPointers[i]);
		isave->Write(&id,sizeof(ULONG), &nb);
		}
	isave->EndChunk();

//external back pointers
	count = externalBackPointers.Count();
	isave->BeginChunk(EXBACKPOINTERS_COUNT_CHUNK);
	isave->Write(&count,sizeof(int),&nb);			
	isave->EndChunk();

//external back pointers
	isave->BeginChunk(EXBACKPOINTERS_CHUNK);
	for (i=0; i<count; i++) 
		{
		ULONG id = isave->GetRefID(externalBackPointers[i]);
		isave->Write(&id,sizeof(ULONG), &nb);
		}
	isave->EndChunk();
	
	return IO_OK;
	}

IOResult BlockControl::Load(ILoad *iload)
	{
	int ID =  0;
	ULONG nb;
	IOResult res = IO_OK;
	int ix = 0;

	while (IO_OK==(res=iload->OpenChunk())) 
		{
		ID = iload->CurChunkID();
		if (ID ==BLOCKCOUNT_CHUNK)
			{
			int ct;
			iload->Read(&ct, sizeof(ct), &nb);
			controls.SetCount(ct);
			tempControls.SetCount(ct);
			names.SetCount(ct);
			backPointers.SetCount(ct);
			for (int i=0; i<ct; i++) 
				{
				names[i] = NULL;
				controls[i] = NULL;
				tempControls[i] = NULL;
				backPointers[i] = NULL;
				}

			}
		else if (ID == NAME_CHUNK)
			{
			TCHAR *buf;
			iload->ReadWStringChunk(&buf);
			names[ix++] = new TSTR(buf);
			}
		else if (ID == NONAME_CHUNK)
			{
			ix++;
			}
		else if (ID ==COLOR_CHUNK)
			{
			Color c;
			iload->Read(&c, sizeof(Color), &nb);
			color = c;
			}
		else if (ID ==START_CHUNK)
			{
			TimeValue s;
			iload->Read(&s, sizeof(s), &nb);
			start = s;
			}
		else if (ID ==END_CHUNK)
			{
			TimeValue e;
			iload->Read(&e, sizeof(e), &nb);
			end = e;
			}
		else if (ID ==BACKPOINTERS_CHUNK)
			{
			for (int i=0; i<backPointers.Count(); i++) 
				{
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					iload->RecordBackpatch(id,(void**)&backPointers[i]);
				}
			}
		else if (ID ==EXBACKPOINTERS_COUNT_CHUNK)
			{
			int ct;
			iload->Read(&ct, sizeof(ct), &nb);
			externalBackPointers.SetCount(ct);
			for (int i=0; i<ct; i++) 
				{
				externalBackPointers[i] = NULL;
				}

			}
		else if (ID ==EXBACKPOINTERS_CHUNK)
			{
			for (int i=0; i<externalBackPointers.Count(); i++) 
				{
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					iload->RecordBackpatch(id,(void**)&externalBackPointers[i]);
				}
			}


		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

//rebuild all tempcontrols	
	return IO_OK;
	}



void BlockControl::EditTrackParams(
			TimeValue t,
			ParamDimensionBase *dim,
			TCHAR *pname,
			HWND hParent,
			IObjParam *ip,
			DWORD flags)
{
trackHWND = hParent;

int OK = DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_ADDNEWTRACK),
				hParent, BlockPropDlgProc, (LPARAM)this);

}

int BlockControl::AddBlockName(ReferenceTarget *anim,ReferenceTarget *client, int subNum, NameList &names)

{

MyEnumProc dep;              
anim->EnumDependents(&dep);
TSTR nodeName = TSTR( dep.Nodes[0]->GetName());
TSTR np = TSTR(client->SubAnimName(subNum));
TSTR Slash("/");
nodeName += Slash;
nodeName += np;
		
TSTR *st = new TSTR(nodeName);
names.Append(1,&st,1);
return 1;
}

Control* BlockControl::BuildListControl(TrackViewPick res, BOOL &createdList)
{
Control *list=NULL;
createdList = FALSE;
if ((res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) && (res.client->ClassID() != Class_ID(FLOATLIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_FLOAT_CLASS_ID,
	Class_ID(FLOATLIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) && (res.client->ClassID() == Class_ID(FLOATLIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control *)res.client;
	}
else if ((res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) && (res.client->ClassID() != Class_ID(POSLIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_POSITION_CLASS_ID,
	Class_ID(POSLIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) && (res.client->ClassID() == Class_ID(POSLIST_CONTROL_CLASS_ID,0)))
	{
	list =  (Control *)res.client;
	}
else if ((res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) && (res.client->ClassID() != Class_ID(ROTLIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_ROTATION_CLASS_ID,
	Class_ID(ROTLIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) && (res.client->ClassID() == Class_ID(ROTLIST_CONTROL_CLASS_ID,0)))
	{
	list =  (Control *)res.client;
	}
else if ((res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) && (res.client->ClassID() != Class_ID(SCALELIST_CONTROL_CLASS_ID,0)))
	{
	list = (Control*)GetCOREInterface()->CreateInstance(
	CTRL_SCALE_CLASS_ID,
	Class_ID(SCALELIST_CONTROL_CLASS_ID,0));
	createdList = TRUE;
	}
else if ((res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) && (res.client->ClassID() == Class_ID(SCALELIST_CONTROL_CLASS_ID,0)))
	{
	list =  (Control *)res.client;
	}
return list;
}

Control* BlockControl::BuildSlave(TrackViewPick res,Control* list, BOOL createdList)
{
int count = list->NumSubs()-2;
Control *slave = NULL;
for (int i = 0; i < count; i++)
	{
	if (res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVEFLOAT_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	else if (res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVEPOS_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	else if (res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVEROTATION_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	else if (res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) 
		{
		slave = (Control*)list->SubAnim(i);
		if (slave->ClassID() == SLAVESCALE_CONTROL_CLASS_ID)
			{
//			list->AssignController(res.anim,count);
			return slave;
			}
		}
	}
BOOL isRotation = FALSE;
if (res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID) 
	slave = (Control*)new SlaveFloatControl;
#ifndef NO_CONTROLLER_SLAVE_POSITION
else if (res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID) 
	slave = (Control*)new SlavePosControl;
#endif
#ifndef NO_CONTROLLER_SLAVE_ROTATION
else if (res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID) 
	{
	slave = (Control*)new SlaveRotationControl;
	isRotation = TRUE;
	}
#endif
#ifndef NO_CONTROLLER_SLAVE_SCALE
else if (res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID) 
	slave = (Control*)new SlaveScaleControl;
#endif
if (createdList)
	{
//	if (isRotation)
//		{
//		list->AssignController(slave,count);
//		list->AssignController(res.anim->Clone(),count+1);
//		}	
//	else
		{
		list->AssignController(res.anim->Clone(),count);
		list->AssignController(slave,count+1);
		}	
	}
else
	{
	list->AssignController(slave,count);
	}
return slave;
}


int BlockControl::AddControl(HWND hWnd)
{
//pop up track view selector
Interface *ip = GetCOREInterface();


TrackViewPick res;
MasterBlockTrackViewFilter filter;
if (ip->TrackViewPickDlg(hWnd, &res,&filter ))
	{
//pop frame selector
	if (res.anim != NULL)
		{
		int OK = DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_TRACKPROP),
			hWnd, TrackPropDlgProc, (LPARAM)this);
		if (OK)
			{
			Control *list;
			BOOL createdList = FALSE;
//check for list control if not add
			list = BuildListControl(res,createdList);
//check if list has a slave control
			Control *slaveControl;
			slaveControl = BuildSlave(res,list,createdList);


			int i = controls.Count();
			Control *ctemp = NULL;
			controls.Append(1,&ctemp,1);
			tempControls.Append(1,&ctemp,1);
//			backPointers.Append(1,&ctemp,1);
			ReplaceReference(i,res.anim->Clone());

//copy relvant keys
			propStart = propStart * GetTicksPerFrame();
			Interval iv(propStart,propStart + (end-start));
			TrackClipObject *cpy = controls[i]->CopyTrack(iv, TIME_INCLEFT|TIME_INCRIGHT);
//nuke all keys 
			controls[i]->DeleteKeys(TRACK_DOALL);
//paste back relevant keys
			iv.Set(0,end-start);
			controls[i]->PasteTrack(cpy, iv, TIME_INCLEFT|TIME_INCRIGHT);


			tempControls[i] = (Control *) res.anim->Clone();
			SlaveControl *sl = (SlaveControl *) slaveControl;
			backPointers.Append(1,&sl,1);

			AddBlockName(res.anim,res.client,res.subNum,names);
//add slaves controls to the selected tracks and put the original as a sub anim of the slaves
//set slave	to have reference to master
			slaveControl->ReplaceReference(1,this);
//copy selected track into slave sub
			slaveControl->ReplaceReference(0,(Control*)res.anim->Clone());
			int bc;

			#define ID_TV_GETFIRSTSELECTED	680
			Tab<TrackViewPick> r;
			SendMessage(trackHWND,WM_COMMAND,ID_TV_GETFIRSTSELECTED,(LPARAM)&r);
			bc = r[0].subNum-1;

			if (res.anim->SuperClassID() == CTRL_FLOAT_CLASS_ID)
				{
				float f = 0.0f;
				tempControls[i]->DeleteKeys(TRACK_DOALL);
				tempControls[i]->SetValue(0,&f);
				SlaveFloatControl *slave = (SlaveFloatControl *) slaveControl;

				slave->scratchControl = (Control *) res.anim->Clone();
//now replace track with slave
//	int bc = Blocks.Count()-1;
				slave->blockID.Append(1,&bc,1);
				slave->subID.Append(1,&i,1);
				}
#ifndef NO_CONTROLLER_SLAVE_POSITION
			else if (res.anim->SuperClassID() == CTRL_POSITION_CLASS_ID)
				{
				Point3 f(0.0f,0.0f,0.0f);
				tempControls[i]->DeleteKeys(TRACK_DOALL);
				tempControls[i]->SetValue(0,&f);
				SlavePosControl *slave = (SlavePosControl *) slaveControl;

				slave->scratchControl = (Control *) res.anim->Clone();
//now replace track with slave
//	int bc = Blocks.Count()-1;
				slave->blockID.Append(1,&bc,1);
				slave->subID.Append(1,&i,1);
				}
#endif
#ifndef NO_CONTROLLER_SLAVE_ROTATION
			else if (res.anim->SuperClassID() == CTRL_ROTATION_CLASS_ID)
				{
				Quat f;
				f.Identity();
				tempControls[i]->DeleteKeys(TRACK_DOALL);
				tempControls[i]->SetValue(0,&f);
				SlaveRotationControl *slave = (SlaveRotationControl *) slaveControl;

				slave->scratchControl = (Control *) res.anim->Clone();
//now replace track with slave
				slave->blockID.Append(1,&bc,1);
				slave->subID.Append(1,&i,1);

				}
#endif
#ifndef NO_CONTROLLER_SLAVE_SCALE
			else if (res.anim->SuperClassID() == CTRL_SCALE_CLASS_ID)
				{
				Matrix3 f(1);
//		f.Identity();
				tempControls[i]->DeleteKeys(TRACK_DOALL);
				tempControls[i]->SetValue(0,&f);
				SlaveScaleControl *slave = (SlaveScaleControl *) slaveControl;

				slave->scratchControl = (Control *) res.anim->Clone();
//now replace track with slave
//	int bc = Blocks.Count()-1;
				slave->blockID.Append(1,&bc,1);
				slave->subID.Append(1,&i,1);
				}
#endif
			if (createdList)
				res.client->AssignController(list,res.subNum);
//res.client->AssignController(slave,res.subNum);

			return 1;
			
				
			}
		}

	}
return 1;

}

int BlockControl::DeleteControl(int Index)

{
//names.Delete(1,whichBlock);
if ((Index < 0) || (Index >= controls.Count())) return 0;
//notify all back pointer that there block is about to be deleted

DeleteReference(Index);
controls.Delete(Index,1);
tempControls.Delete(Index,1);
names.Delete(Index,1);

for (int i = 0;i <backPointers.Count();i++)
	{
//lock at the bockid table
//	int subCount = Blocks[whichBlock]->backPointers[i]->blockID.Count();
	for (int j=0; j< backPointers[i]->blockID.Count(); j++)
		{
		if (backPointers[i]->subID[j] == Index)
			{
			backPointers[i]->blockID.Delete(j,1);
			backPointers[i]->subID.Delete(j,1);
			j--;
			}
		else if (backPointers[i]->subID[j] > Index)
			{
			backPointers[i]->subID[j] -= 1;
			}
		}

	}

backPointers.Delete(Index,1);

for (i = 0;i < externalBackPointers.Count();i++)
	{
	for (int j=0; j< externalBackPointers[i]->blockID.Count(); j++)
		{
		if (externalBackPointers[i]->subID[j] == Index)
			{
			externalBackPointers[i]->blockID.Delete(j,1);
			externalBackPointers[i]->subID.Delete(j,1);
			j--;
			}
		else if (externalBackPointers[i]->subID[j] > Index)
			{
			externalBackPointers[i]->subID[j] -= 1;
			}
		}

	}
NotifyDependents(FOREVER,0,REFMSG_CHANGE);
NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);

return 1;
}



INT_PTR CALLBACK BlockPropDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	BlockControl *blk = (BlockControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);


	switch (msg) {
	case WM_INITDIALOG:
		{
		blk = (BlockControl*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);


//load up list box with sub anims names
		for (int i = 0; i < blk->NumSubs(); i++)
			{
			TSTR finalName = blk->SubAnimName(i);
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_ADDSTRING,0,(LPARAM)(TCHAR*)finalName);
			}
		SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_SETCURSEL,0,0);
		CenterWindow(hWnd,GetParent(hWnd));
		break;
		}
		


	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADD:
			{
			blk->AddControl( hWnd);
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_SETCOUNT,0,0);
			for (int i = 0; i < blk->NumSubs(); i++)
				{
				TSTR finalName = blk->SubAnimName(i);
				SendMessage(GetDlgItem(hWnd,IDC_LIST1),
					LB_ADDSTRING,0,(LPARAM)(TCHAR*)finalName);
				}
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_SETCURSEL,0,0);
			break;
			}
		case IDC_REMOVE:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETCURSEL,0,0);
			if (sel >=0)
				{
				blk->DeleteControl(sel);
				SendMessage(GetDlgItem(hWnd,IDC_LIST1),
					LB_DELETESTRING,sel,0);

				}
			break;
			}
		case IDOK:
			{
			EndDialog(hWnd,1);
			break;
			}
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}




INT_PTR CALLBACK TrackPropDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	BlockControl *blk = (BlockControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	ISpinnerControl *spin;
	static TSTR zero = FormatUniverseValue(0.0f);
	Rect rect;

	switch (msg) {
	case WM_INITDIALOG:
		{
		blk = (BlockControl*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

		Interval range = GetCOREInterface()->GetAnimRange();

		
		spin = GetISpinner(GetDlgItem(hWnd,IDC_STARTSPIN));
		spin->SetLimits(-999999.0f,9999999.0f, FALSE);
		spin->SetAutoScale();
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_START), EDITTYPE_INT);
		spin->SetValue(range.Start()/GetTicksPerFrame(),FALSE);
		ReleaseISpinner(spin);

		blk->propStart = range.Start()/GetTicksPerFrame();
		CenterWindow(hWnd,GetParent(hWnd));
		break;
		}
		
	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam;
		switch (LOWORD(wParam)) {
		case IDC_STARTSPIN: blk->propStart = spin->GetIVal(); break;
		}
		break;



	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
			EndDialog(hWnd,1);
			blk->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

			break;
			}
		case IDCANCEL:
			EndDialog(hWnd,0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
