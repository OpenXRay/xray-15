/**********************************************************************
 *<
	FILE: slave.cpp

	DESCRIPTION: A slave controller that is driven by the master and the sub control

	CREATED BY: Peter Watje

	HISTORY: Oct 15, 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#include "block.h"
#include "units.h"
#include "masterblock.h"
#include "istdplug.h"


//#include "iparamm2.h"


static SlaveFloatClassDesc slaveFloatCD;
ClassDesc* GetSlaveFloatDesc() {return &slaveFloatCD;}

#ifndef NO_CONTROLLER_SLAVE_POSITION
static SlavePosClassDesc slavePosCD;
ClassDesc* GetSlavePosDesc() {return &slavePosCD;}
#endif

static SlavePoint3ClassDesc slavePoint3CD;
ClassDesc* GetSlavePoint3Desc() {return &slavePoint3CD;}

#ifndef NO_CONTROLLER_SLAVE_ROTATION
static SlaveRotationClassDesc slaveRotationCD;
ClassDesc* GetSlaveRotationDesc() {return &slaveRotationCD;}
#endif

#ifndef NO_CONTROLLER_SLAVE_SCALE
static SlaveScaleClassDesc slaveScaleCD;
ClassDesc* GetSlaveScaleDesc() {return &slaveScaleCD;}
#endif


INT_PTR CALLBACK NewLinkDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NewMasterDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//------------------------------------------------------------

SlaveControl::SlaveControl() 
	{	
	range     = Interval(GetAnimStart(),GetAnimEnd());
	master = NULL;
	scratchControl = NULL;
	sub = NULL;
	float v = 50.0f;
	masterPresent = FALSE;

	} 

SlaveControl::~SlaveControl() 
	{	
	int ct = blockID.Count();
	for (int i = 0; i < ct; i++)
		RemoveControl(0);

	if (scratchControl)
		{
		scratchControl->DeleteThis();
		}
	scratchControl = NULL;
	} 

int SlaveControl::NumSubs() 
	{
//	return 1;
	return 0;
	}


void SlaveControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
	{
//	sub->SetValue(t,val,commit,method);
	}

void SlaveControl::EnumIKParams(IKEnumCallback &callback)
	{
	if (scratchControl)
		scratchControl->EnumIKParams(callback);
	}

BOOL SlaveControl::CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags)
	{
	if (scratchControl)
		return scratchControl->CompDeriv(t,ptm,derivs,flags);
	else return FALSE;
	}

void SlaveControl::MouseCycleCompleted(TimeValue t)
	{
	if (scratchControl)
		scratchControl->MouseCycleCompleted(t);
	}

void SlaveControl::AddNewKey(TimeValue t,DWORD flags)
	{
	if (scratchControl)
		scratchControl->AddNewKey(t,flags);
	}

void SlaveControl::CloneSelectedKeys(BOOL offset)
	{
	if (scratchControl)
		scratchControl->CloneSelectedKeys(offset);
	}

void SlaveControl::DeleteKeys(DWORD flags)
	{
	if (scratchControl)
		scratchControl->DeleteKeys(flags);
	}

void SlaveControl::SelectKeys(TrackHitTab& sel, DWORD flags)
	{
	if (scratchControl)
		scratchControl->SelectKeys(sel,flags);
	}

BOOL SlaveControl::IsKeySelected(int index)
	{
	if (scratchControl)
		return scratchControl->IsKeySelected(index);
	return FALSE;
	}

void SlaveControl::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
	{
	if (scratchControl)
		scratchControl->CopyKeysFromTime(src,dst,flags);

	}

void SlaveControl::DeleteKeyAtTime(TimeValue t)
	{
	if (scratchControl)
		scratchControl->DeleteKeyAtTime(t);
	}

BOOL SlaveControl::IsKeyAtTime(TimeValue t,DWORD flags)
	{
	if (scratchControl)
		return scratchControl->IsKeyAtTime(t,flags);
	return FALSE;
	}

BOOL SlaveControl::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
	{
	if (scratchControl)
		return scratchControl->GetNextKeyTime(t,flags,nt);
	return FALSE;
	}

int SlaveControl::GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags)
	{
	if (scratchControl)
		return scratchControl->GetKeyTimes(times,range,flags);
	return 0;
	}

int SlaveControl::GetKeySelState(BitArray &sel,Interval range,DWORD flags)
	{
	if (scratchControl)
		return scratchControl->GetKeySelState(sel,range,flags);
	return 0;
	}




Animatable* SlaveControl::SubAnim(int i) 
	{
//	if (i==0)
//		return sub;
//	else 
//		{
//		DebugPrint("Subanims out of range call\n");
		return NULL;
//		}
	}


TSTR SlaveControl::SubAnimName(int i) 
	{
	return GetString(IDS_PW_SUB);
	}




BOOL SlaveControl::AssignController(Animatable *control,int subAnim) 
	{
	return FALSE;
	}


int SlaveControl::NumRefs() 
	{
	return 2;
	}

RefTargetHandle SlaveControl::GetReference(int i) 
	{
	if (i==0) return (RefTargetHandle) sub;
	else if (i==1) return (RefTargetHandle) master;
	else
		{
//		DebugPrint("get reference error occurred\n");
		return NULL;
		}
	}

void SlaveControl::SetReference(int i, RefTargetHandle rtarg) 
	{
	if (i==0) sub = (Control*) rtarg;
	else if (i==1) 
		{
		if ((rtarg == NULL) && (master))
			{
//tell the master that I am being removed
			int ct = blockID.Count();
			for (int i = 0; i < ct; i++)
				RemoveControl(0);

			}

		master = (MasterBlockControl*) rtarg;
		if (master == NULL) masterPresent = FALSE;
			else masterPresent = TRUE;
		}
	else DebugPrint("set reference error occurred\n");

	}


void SlaveControl::Copy(Control *from)
	{
	if ( from->CanCopyTrack(FOREVER,0) )
		ReplaceReference(0,from);
	superID = from->SuperClassID();
//	scratchControl->Copy(from);
//	MakeRefByID(FOREVER,0,from);
	}

/*
void SlaveControl::HoldRange()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new RangeRestore(this));
		}
	}

*/

RefResult SlaveControl::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
     	PartID& partID,  
     	RefMessage message)
	{
	switch (message) {

		case REFMSG_TARGET_DELETED:
			if (hTarget == master) {
				masterPresent = FALSE;
				}

			break;
		case REFMSG_CHANGE:
//			UpdateSlave();			
			break;


		}
	return REF_SUCCEED;
	}



class SlaveRangeRestore : public RestoreObj {
	public:
		SlaveControl *cont;
		Interval ur, rr;
		SlaveRangeRestore(SlaveControl *c) 
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
		TSTR Description() { return TSTR(_T("Slave control range")); }
	};


void SlaveControl::HoldRange()
	{
	if (theHold.Holding() && !TestAFlag(A_HELD)) {
		SetAFlag(A_HELD);
		theHold.Put(new SlaveRangeRestore(this));
		}
	}

void SlaveControl::EditTimeRange(Interval range,DWORD flags)
	{
/*
	if (!(flags&EDITRANGE_LINKTOKEYS)) {
		HoldRange();
		this->range = range;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
*/
	}

void SlaveControl::MapKeys(TimeMap *map,DWORD flags)
	{
/*
	if (flags&TRACK_MAPRANGE) {
		HoldRange();
		TimeValue t0 = map->map(range.Start());
		TimeValue t1 = map->map(range.End());

		range.Set(t0,t1);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
*/
	}
void SlaveControl::RemoveControl(int sel)

{
if (master)
	{
	if (blockID[sel] < master->Blocks.Count() )
		{
		for (int i = 0; i <master->Blocks[blockID[sel]]->externalBackPointers.Count(); i++)
			{ 
			if ( this == master->Blocks[blockID[sel]]->externalBackPointers[i])
				{
				master->Blocks[blockID[sel]]->externalBackPointers.Delete(i,1);
				i--;
				}
			}
		for (i = 0; i <master->Blocks[blockID[sel]]->backPointers.Count(); i++)
			{ 
			if ( this == master->Blocks[blockID[sel]]->backPointers[i])
				{
				master->Blocks[blockID[sel]]->backPointers[i]= NULL;
				}
			}
		}
	}
blockID.Delete(sel,1);
subID.Delete(sel,1);

}

void SlaveControl::AddControl(int blockid,int subid )
{
blockID.Append(1,&blockid,1);
subID.Append(1,&subid,1);
SlaveControl *sl = this;
master->Blocks[blockid]->externalBackPointers.Append(1,&sl,1);
if (sub == NULL)
	{
	ReplaceReference(0,master->Blocks[blockid]->controls[subid]->Clone());
	}
}

void SlaveControl::CollapseControl()
{
#define ID_TV_GETSELECTED	680

Tab<TrackViewPick> res;
res.ZeroCount();
SendMessage(trackHWND,WM_COMMAND,ID_TV_GETSELECTED,(LPARAM)&res);
if (res.Count() == 1)
	{
	if (masterPresent)
		{
		Control *mc = (Control *) master->blendControl->Clone();

		for (int ct = 0; ct < scratchControl->NumMultCurves(); ct++)
			scratchControl->DeleteMultCurve(ct);
		scratchControl->AppendMultCurve(mc);
		}
	int ct = blockID.Count();
	for (int i = 0; i < ct; i++)
		RemoveControl(0);

	res[0].client->AssignController(scratchControl->Clone(),res[0].subNum);


//	Control *mc = (Control *) res[0].client;
//	mc->AppendMultCurve(master->blendControl);



	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
//	DeleteReference(0);
//	DeleteReference(1);
//	this->DeleteThis();
	}

		// Get Interfaces
//Interface *iu = GetCOREInterface();
//Animatable *anim   = iu->GetAnim(i);
//Animatable *client = iu->GetClient(i);
//int subNum         = iu->GetSubNum(i);

//scratchControl
//list->

}


#define COUNT_CHUNK		0x01010
#define DATA_CHUNK		0x01020


IOResult SlaveControl::Save(ISave *isave)
	{		
	ULONG nb;	
//count
	int count = blockID.Count();
	isave->BeginChunk(COUNT_CHUNK);
	isave->Write(&count,sizeof(count),&nb);			
	isave->EndChunk();
//id data 
	for (int i =0; i < count; i++)
		{
		isave->BeginChunk(DATA_CHUNK);
		isave->Write(&blockID[i],sizeof(int),&nb);			
		isave->Write(&subID[i],sizeof(int),&nb);			
		isave->EndChunk();
		}
	return IO_OK;
	}

IOResult SlaveControl::Load(ILoad *iload)
	{
	int ID =  0;
	ULONG nb;
	IOResult res = IO_OK;
	int ix = 0;
	while (IO_OK==(res=iload->OpenChunk())) 
		{
		ID = iload->CurChunkID();
		if (ID ==COUNT_CHUNK)
			{
			int ct;
			iload->Read(&ct, sizeof(ct), &nb);
			blockID.SetCount(ct);
			subID.SetCount(ct);
/*
			for (int i=0; i<ct; i++) 
				{
				names[i] = NULL;
				controls[i] = NULL;
				tempControls[i] = NULL;
				}
*/

			}
		else if (ID == DATA_CHUNK)
			{
			int bID,sID;
			iload->Read(&bID, sizeof(int), &nb);
			iload->Read(&sID, sizeof(int), &nb);
			blockID[ix] = bID;
			subID[ix++] = sID;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

//rebuild all tempcontrols	
	return IO_OK;
	}


//--------------------------------------------------------------------


RefTargetHandle SlaveControl::Clone(RemapDir& remap)
	{
	SlaveControl *cont = new SlaveControl;
//	*cont = *this;
	cont->sub = NULL;
	cont->master = NULL;
	cont->scratchControl = NULL;

	cont->ReplaceReference(0,sub);
	cont->ReplaceReference(1,master);
	cont->blockID = blockID;
	cont->subID = subID;
	cont->masterPresent = masterPresent;
//	if (master)
//		master->Blocks[blockID]->externalBackPointers.Append(1,&cont,1);

	CloneControl(cont,remap);
	cont->UpdateSlave();
	BaseClone(this, cont, remap);
	return cont;
	}


void SlaveControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
}

//------------------------------------------------------------
//Slave Float Controller
//------------------------------------------------------------

RefTargetHandle SlaveFloatControl::Clone(RemapDir& remap)
	{
	SlaveFloatControl *cont = new SlaveFloatControl;
//	*cont = *this;
	cont->sub = NULL;
	cont->master = NULL;
	cont->scratchControl = NULL;

	cont->ReplaceReference(0,sub);
	cont->ReplaceReference(1,master);
	cont->blockID = blockID;
	cont->subID = subID;
	cont->masterPresent = masterPresent;
	if (master)
		{
		for (int i = 0; i < blockID.Count(); i++)
			{
			SlaveControl *c = (SlaveControl *)cont;
			master->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
			}
		}
	CloneControl(cont,remap);
	cont->UpdateSlave();
	BaseClone(this, cont, remap);
	return cont;
	}

SlaveFloatControl::SlaveFloatControl() 
	{	

	} 

void SlaveFloatControl::UpdateSlave()
{
	// CAL-06/03/02: sub could be NULL
	if (scratchControl == NULL && sub)
		scratchControl = (Control *) sub->Clone();

	if (scratchControl == NULL)
		return;

	scratchControl->DeleteKeys(TRACK_DOALL);
	float f = 0.0f;
	scratchControl->SetValue(0,&f);

	if (master)
		master->Update(scratchControl,blockID,subID);
}

void SlaveFloatControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if ( (sub == NULL) || (!masterPresent) || (blockID.Count()==0))
		{
		if (method == CTRL_ABSOLUTE)
			{
			float *v = ((float*)val);
			*v = 0.0f;
			}
		return;
		}
//copy keys into scratch control


	if (scratchControl == NULL)
		{
		UpdateSlave();
		}
	float *tv = ((float*)val);

	if (master)
		master->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);
//	cpy->DeleteThis();

	}


#ifndef NO_CONTROLLER_SLAVE_POSITION
//------------------------------------------------------------
//Slave Pos Controller
//------------------------------------------------------------

RefTargetHandle SlavePosControl::Clone(RemapDir& remap)
	{
	SlavePosControl *cont = new SlavePosControl;
//	*cont = *this;
	cont->sub = NULL;
	cont->master = NULL;
	cont->scratchControl = NULL;
	cont->ReplaceReference(0,sub);
	cont->ReplaceReference(1,master);
	cont->blockID = blockID;
	cont->subID = subID;
	cont->masterPresent = masterPresent;
	if (master)
		{
		for (int i = 0; i < blockID.Count(); i++)
			{
			SlaveControl *c = (SlaveControl *)cont;
			master->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
			}
		}
	CloneControl(cont,remap);
	cont->UpdateSlave();
	BaseClone(this, cont, remap);
	return cont;
	}

SlavePosControl::SlavePosControl() 
	{	

	} 


void SlavePosControl::UpdateSlave()
{
	// CAL-06/03/02: sub could be NULL
	if (scratchControl == NULL && sub)
		scratchControl = (Control *) sub->Clone();

	if (scratchControl == NULL)
		return;

	scratchControl->DeleteKeys(TRACK_DOALL);
	Point3 f(0.0f,0.f,0.0f);
	scratchControl->SetValue(0,&f);

	if (master)
		master->Update(scratchControl,blockID,subID);
}



void SlavePosControl::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if ( (sub == NULL) || (!masterPresent) || (blockID.Count()==0))
		{
		if (method == CTRL_ABSOLUTE)
			{
			Point3 *v = ((Point3*)val);
			*v = Point3(0.0f,0.0f,0.0f);
			}
		else
			{
			Point3 f(0.0f,0.0f,0.0f);
			Matrix3 *v = ((Matrix3*)val);
			v->PreTranslate(f);
			
			}

		return;
		}
//copy keys into scratch control



	if (scratchControl == NULL)
		{
		UpdateSlave();
		}

	if (master)
		master->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);

	}
#endif // NO_CONTROLLER_SLAVE_POSITION

//------------------------------------------------------------
//Slave Point3 Controller
//------------------------------------------------------------

RefTargetHandle SlavePoint3Control::Clone(RemapDir& remap)
	{
	SlavePoint3Control *cont = new SlavePoint3Control;
//	*cont = *this;
	cont->sub = NULL;
	cont->master = NULL;
	cont->scratchControl = NULL;

	cont->ReplaceReference(0,sub);
	cont->ReplaceReference(1,master);
	cont->blockID = blockID;
	cont->subID = subID;
	cont->masterPresent = masterPresent;
	if (master)
		{
		for (int i = 0; i < blockID.Count(); i++)
			{
			SlaveControl *c = (SlaveControl *)cont;
			master->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
			}
		}
	CloneControl(cont,remap);
	cont->UpdateSlave();
	BaseClone(this, cont, remap);
	return cont;
	}

SlavePoint3Control::SlavePoint3Control() 
	{	

	} 


void SlavePoint3Control::UpdateSlave()
{
	// CAL-06/03/02: sub could be NULL
	if (scratchControl == NULL && sub)
		scratchControl = (Control *) sub->Clone();

	if (scratchControl == NULL)
		return;

	scratchControl->DeleteKeys(TRACK_DOALL);
	Point3 f(0.0f,0.f,0.0f);
	scratchControl->SetValue(0,&f);

	if (master)
		master->Update(scratchControl,blockID,subID);
}

void SlavePoint3Control::GetValue(
		TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if ( (sub == NULL) || (!masterPresent) || (blockID.Count()==0))
		{
		Point3 *v = ((Point3*)val);
		*v = Point3(0.0f,0.0f,0.0f);
		return;
		}
//copy keys into scratch control

	if (scratchControl == NULL)
		{
		UpdateSlave();
		}

	if (master)
		master->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);
//	cpy->DeleteThis();

	}



#ifndef NO_CONTROLLER_SLAVE_ROTATION
//------------------------------------------------------------
//Slave rotation Controller
//------------------------------------------------------------

RefTargetHandle SlaveRotationControl::Clone(RemapDir& remap)
	{
	SlaveRotationControl *cont = new SlaveRotationControl;
//	*cont = *this;
	cont->sub = NULL;
	cont->master = NULL;
	cont->scratchControl = NULL;

	cont->ReplaceReference(0,sub);
	cont->ReplaceReference(1,master);
	cont->blockID = blockID;
	cont->subID = subID;
	cont->masterPresent = masterPresent;
	if (master)
		{
		for (int i = 0; i < blockID.Count(); i++)
			{
			SlaveControl *c = (SlaveControl *)cont;
			master->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
			}
		}
	CloneControl(cont,remap);
	cont->UpdateSlave();
	BaseClone(this, cont, remap);
	return cont;
	}


SlaveRotationControl::SlaveRotationControl() 
	{	

	} 


void SlaveRotationControl::UpdateSlave()
{
	// CAL-06/03/02: sub could be NULL
	if (scratchControl == NULL && sub)
		scratchControl = (Control *) sub->Clone();

	if (scratchControl == NULL)
		return;

	scratchControl->DeleteKeys(TRACK_DOALL);
	Quat f;
	f.Identity();
	scratchControl->SetValue(0,&f);

	if (master)
		master->Update(scratchControl,blockID,subID);
}

void SlaveRotationControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if ( (sub == NULL) || (!masterPresent) || (blockID.Count()==0))
		{
		Quat f;
		f.Identity();
		if (method == CTRL_ABSOLUTE)
			{
			Quat *v = ((Quat*)val);
			*v = f;
			return;
			}
		else
			{
			Matrix3 *v = ((Matrix3*)val);
			PreRotateMatrix(*v,f);
			return;				
			}

		}

//copy keys into scratch control
	if (scratchControl == NULL)
		{
		UpdateSlave();
		}

	if (master)
		master->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);
//	cpy->DeleteThis();

	}
#endif // NO_CONTROLLER_SLAVE_ROTATION

#ifndef NO_CONTROLLER_SLAVE_SCALE
//------------------------------------------------------------
//Slave Scale Controller
//------------------------------------------------------------

RefTargetHandle SlaveScaleControl::Clone(RemapDir& remap)
	{
	SlaveScaleControl *cont = new SlaveScaleControl;
//	*cont = *this;
	cont->sub = NULL;
	cont->master = NULL;
	cont->scratchControl = NULL;

	cont->ReplaceReference(0,sub);
	cont->ReplaceReference(1,master);
	cont->blockID = blockID;
	cont->subID = subID;
	cont->masterPresent = masterPresent;
	if (master)
		{
		for (int i = 0; i < blockID.Count(); i++)
			{
			SlaveControl *c = (SlaveControl *)cont;
			master->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
			}
		}
	CloneControl(cont,remap);
	cont->UpdateSlave();
	BaseClone(this, cont, remap);
	return cont;
	}

SlaveScaleControl::SlaveScaleControl() 
	{	

	} 

void SlaveScaleControl::UpdateSlave()
{
	// CAL-06/03/02: sub could be NULL
	if (scratchControl == NULL && sub)
		scratchControl = (Control *) sub->Clone();

	if (scratchControl == NULL)
		return;

	scratchControl->DeleteKeys(TRACK_DOALL);
//	Matrix3 f(1);

	Quat f;
	f.Identity();
	Point3 p(1.0f,1.0f,1.0f);
	ScaleValue s(p,f); 

	scratchControl->SetValue(0,&s);

	if (master)
		master->Update(scratchControl,blockID,subID);
}

void SlaveScaleControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
	{
	if ( (sub == NULL) || (!masterPresent) || (blockID.Count()==0))
		{
		Quat f;
		f.Identity();
		Point3 p(1.0f,1.0f,1.0f);
		if (method == CTRL_ABSOLUTE)
			{
			ScaleValue s(p,f); 
			ScaleValue *v = ((ScaleValue*)val);
			*v = s;
			return;
			}
		else
			{
			Matrix3 *mat = (Matrix3*)val;
			ScaleValue s(p,f); 
			ApplyScaling(*mat,s);
			return;				

			
			}

		}

	if (scratchControl == NULL)
		UpdateSlave();


	if (master)
		master->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);

	}
#endif // NO_CONTROLLER_SLAVE_SCALE


static INT_PTR CALLBACK SlaveDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void SlaveControl::EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		TCHAR *pname,
		HWND hParent,
		IObjParam *ip,
		DWORD flags)
	{

	if (flags & EDITTRACK_BUTTON)
		{
		trackHWND = hParent;
		SlaveDlg *dlg = new SlaveDlg(this,dim,pname,ip,hParent);
		}
	}

SlaveDlg::SlaveDlg(
		SlaveControl *cont,
		ParamDimensionBase *dim,
		TCHAR *pname,
		IObjParam *ip,
		HWND hParent)
	{
	this->cont = cont;
	this->ip   = ip;
	this->dim  = dim;
	valid = FALSE;

//	elems = cont->Elems();

	theHold.Suspend();
	MakeRefByID(FOREVER,0,cont);
	theHold.Resume();

//	hWnd = CreateDialogParam(
	int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_SLAVEPARAMS),
		        hParent,SlaveDlgProc,(LPARAM)this);	

//	TSTR title = TSTR(GetString(IDS_RB_NOISECONTROLTITLE)) + TSTR(pname);
//	SetWindowText(hWnd,title);
//	ip->RegisterTimeChangeCallback(this);
	}

SlaveDlg::~SlaveDlg()
	{
//	UnRegisterNoiseCtrlWindow(hWnd);
//	ip->UnRegisterTimeChangeCallback(this);


	theHold.Suspend();
	DeleteAllRefsFromMe();
	theHold.Resume();

	}

void SlaveDlg::Invalidate()
	{
	valid = FALSE;
//	InvalidateRect(hWnd,NULL,FALSE);	
//	InvalidateRect(GetDlgItem(hWnd,IDC_NOISE_GRAPH),NULL,FALSE);
	}

void SlaveDlg::Update()
	{
	if (!valid && hWnd) {
/*
		float strength[MAX_ELEMS];
		cont->GetStrength(ip->GetTime(),strength);

		for (int i=0; i<elems; i++) {			
//			iStrength[i]->SetValue(dim->Convert(strength[i]),FALSE);
//			CheckDlgButton(hWnd,limID[i],cont->lim[i]);
			}
		if (cont->fractal) {
		} else {
			}
*/
		valid = TRUE;
		}
	}

void SlaveDlg::SetupUI(HWND hWnd)
	{
	this->hWnd = hWnd;

/*	iRampOut = GetISpinner(GetDlgItem(hWnd,IDC_NOISE_RAMPOUTSPIN));
	iRampOut->SetLimits(0,TIME_PosInfinity,FALSE);
	iRampOut->SetScale(10.0f);
	iRampOut->LinkToEdit(GetDlgItem(hWnd,IDC_NOISE_RAMPOUT),EDITTYPE_TIME);	
*/
	
//	SetWindowLongPtr(GetDlgItem(hWnd,IDC_NOISE_GRAPH),GWLP_USERDATA,(LONG)cont);
	SetupList();


	valid = FALSE;
	Update();
	}

void SlaveDlg::SetupList()
	{
//loop through list getting names
//nuke old lis
	SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_RESETCONTENT,0,0);
	SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_SETCOUNT,0,0);
	if (cont->masterPresent)
		{
		for (int i=0; i<cont->blockID.Count(); i++) 
			{
			int id = cont->blockID[i];
			int subid = cont->subID[i];
			if (id < cont->master->Blocks.Count())
				{
				TSTR name = cont->master->Blocks[id]->SubAnimName(subid);
				SendMessage(GetDlgItem(hWnd,IDC_LIST1),
					LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
				}

			}
		}

/*
	int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
	SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_RESETCONTENT,0,0);
	for (int i=0; i<cont->Blocks.Count(); i++) {
		TSTR name = cont->SubAnimName(i+1);
		SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
		}
	if (sel!=LB_ERR) {
		SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_SETCURSEL,(WPARAM)sel,0);
	} else {
		SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_SETCURSEL,(WPARAM)-1,0);
		}
*/
	}

void SlaveDlg::SetButtonStates()
	{
	int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_GETCURSEL,0,0);
	if (sel!=LB_ERR) {
		if ((cont->blockID.Count() == 0) || (cont->subID.Count()==0))
			{
			EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
			}
		else {
			EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
			EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),TRUE);
			}
		}
	else
		{
		EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
		}


	}

BOOL MasterTrackViewFilter :: proc(Animatable *anim, Animatable *client,int subNum)

{
//make sure the parent is not a slave or 
if ( anim->ClassID() ==MASTERBLOCK_CONTROL_CLASS_ID)
	return TRUE;
return FALSE;
}


void SlaveDlg::WMCommand(int id, int notify, HWND hCtrl)
	{
	switch (id) {
		case IDC_LIST_NAME: {

			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			if (sel!=LB_ERR) {
/*
				TCHAR buf[256];
				ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_LIST_NAME));
				iName->GetText(buf,256);
				if (!cont->names[sel]) cont->names[sel] = new TSTR;
				*cont->names[sel] = buf;
				cont->NotifyDependents(FOREVER,0,REFMSG_NODE_NAMECHANGE);
				SetupList();
				ReleaseICustEdit(iName);
				EnableWindow(GetDlgItem(hWnd,IDC_SAVE),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_REPLACE),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_DELETE),TRUE);
*/

				if ((cont->blockID.Count() == 0) || (cont->subID.Count()==0))
					{
					EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
					EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
					}
				else {
					EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
					EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),TRUE);
					}

				}

			break;
			}
		case IDC_LIST1:
			if (notify==LBN_SELCHANGE) {
				SetButtonStates();				
				}

			break;
		case IDC_LINK:
			{
			if (!cont->masterPresent)
				{
				MasterTrackViewFilter filter;
				TrackViewPick res;
				BOOL MasterOK = GetCOREInterface()->TrackViewPickDlg(hWnd,&res,&filter);
				if (MasterOK && (res.anim != NULL))
					{
					cont->ReplaceReference(1,res.anim,FALSE);
					cont->propBlockID = -1;
					cont->propSubID = -1;

					int OK = DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_ADDNEWLINK),
						hWnd, NewLinkDlgProc, (LPARAM)cont);
			
					if ((OK) && (cont->propSubID != -1) && (cont->propSubID != -1))
						{
						cont->AddControl(cont->propBlockID,cont->propSubID);
						SetupList();
						}

					}
				int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
					LB_GETCURSEL,0,0);

				if ((cont->blockID.Count() == 0) || (cont->subID.Count()==0))
					{
					EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
					EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
						
					}
				else {
					if (sel!=LB_ERR)
						{
						EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
						EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),TRUE);
						}
					}

				}
			else
				{
				int OK = DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_ADDNEWLINK),
					hWnd, NewLinkDlgProc, (LPARAM)cont);
			
				if ( (OK)  && (cont->propSubID != -1) && (cont->propSubID != -1))
					{
					cont->AddControl(cont->propBlockID,cont->propSubID);
					SetupList();
					}
				}
			Change(TRUE);
			break;
			}
		case IDC_REMOVE:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			cont->RemoveControl(sel);
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_DELETESTRING,sel,0);
			SetupList();
			sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);

			if ((cont->blockID.Count() == 0) || (cont->subID.Count()==0))
				{
				EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
				}
			else {
				if (sel!=LB_ERR)
					{
					EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
					EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),TRUE);
					}
				}

			Change(TRUE);

			break;
			}
		case IDC_COLLAPSE:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			cont->CollapseControl();
			EndDialog(hWnd,1);
//			SetupList();
			break;
			}
		case IDOK:
//			DestroyWindow(hWnd);
			EndDialog(hWnd,1);
			break;
		case IDCANCEL:
			EndDialog(hWnd,0);

//			DestroyWindow(hWnd);
			break;


		}

	}


void SlaveDlg::Change(BOOL redraw)
	{
//	InvalidateRect(GetDlgItem(hWnd,IDC_NOISE_GRAPH),NULL,TRUE);
	cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
//	UpdateWindow(GetDlgItem(hWnd,IDC_NOISE_GRAPH));
	UpdateWindow(GetParent(hWnd));	
	if (redraw) ip->RedrawViews(ip->GetTime());
	}


class CheckForNonSlaveDlg : public DependentEnumProc {
	public:		
		BOOL non;
		ReferenceMaker *me;
		CheckForNonSlaveDlg(ReferenceMaker *m) {non = FALSE;me = m;}
		int proc(ReferenceMaker *rmaker) {
			if (rmaker==me) return 0;
			if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
				rmaker->ClassID()!=Class_ID(SLAVEDLG_CLASS_ID,0)) {
				non = TRUE;
				return 1;
				}
			return 0;
			}
	};
void SlaveDlg::MaybeCloseWindow()
	{
	CheckForNonSlaveDlg check(cont);
	cont->EnumDependents(&check);
	if (!check.non) {
		PostMessage(hWnd,WM_CLOSE,0,0);
		}
	}



RefResult SlaveDlg::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
     	PartID& partID,  
     	RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:
			Invalidate();			
			break;
		
		case REFMSG_REF_DELETED:
			MaybeCloseWindow();
			break;
		}
	return REF_SUCCEED;
	}








static INT_PTR CALLBACK SlaveDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	SlaveDlg *dlg = (SlaveDlg*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
			dlg = (SlaveDlg*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			dlg->SetupUI(hWnd);
			SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_SETCURSEL,0,0);
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			if (sel==-1) 
				{
				EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
				}

			break;

			}
		case WM_COMMAND:
			dlg->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);						
			break;

		case WM_PAINT:
			dlg->Update();
			return 0;			
		
		case WM_CLOSE:
			DestroyWindow(hWnd);			
			break;

		case WM_DESTROY:						
			delete dlg;
			break;
	
		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
			break;
		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else theHold.Cancel();
			break;
		case CC_COLOR_CHANGE: {
			int i = LOWORD(wParam);
			IColorSwatch *cs = (IColorSwatch*)lParam;
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
				LB_GETCURSEL,0,0);
			if (sel != -1)
				{
				if (HIWORD(wParam)) theHold.Begin();
//				dlg->cont->Blocks[sel]->color = cs->GetColor();
				if (HIWORD(wParam)) {
					theHold.Accept(GetString(IDS_DS_PARAMCHG));
					}
				}
			break;
		}

		
		default:
			return FALSE;
		}
	return TRUE;
	}





INT_PTR CALLBACK NewLinkDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	SlaveControl *slv = (SlaveControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	static Tab<int> sid,bid;

	switch (msg) {
	case WM_INITDIALOG:
		{
		sid.ZeroCount();
		bid.ZeroCount();
		slv = (SlaveControl*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

//goto master look at all sub block
		int count = slv->master->Blocks.Count();
		for (int i = 0;i < slv->master->Blocks.Count();i++)
			{
			TSTR blockName = slv->master->SubAnimName(i+1);
			for (int j = 0;j < slv->master->Blocks[i]->controls.Count();j++)
				{
				TSTR subName = slv->master->Blocks[i]->SubAnimName(j);
				TSTR finalName = blockName +" "+ subName;
//check if control is the same as ours
				if (slv->sub == NULL)
//if (((slv->sub == NULL) && (slv->superID ==slv->master->Blocks[i]->controls[j]->SuperClassID()) ) ||

					{
//					SClass_ID sID=slv->master->Blocks[i]->controls[j]->SuperClassID();
//					SClass_ID thisID=slv->SuperClassID();

					if (slv->master->Blocks[i]->controls[j]->SuperClassID() == slv->SuperClassID())  
						{
//add to list box
						sid.Append(1,&j,1);
						bid.Append(1,&i,1);
						SendMessage(GetDlgItem(hWnd,IDC_LIST1),
							LB_ADDSTRING,0,(LPARAM)(TCHAR*)finalName);

						}
					}
				else if (slv->master->Blocks[i]->controls[j]->ClassID() == slv->sub->ClassID() )  
					
					{

//add to list box
					sid.Append(1,&j,1);
					bid.Append(1,&i,1);
					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)finalName);
					}
				}
			}	

		SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_SETCURSEL,0,0);
		CenterWindow(hWnd,GetParent(hWnd));
		break;
		}
		


	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETCURSEL,0,0);
			if (sel >=0)
				{
				slv->propBlockID = bid[sel];
				slv->propSubID = sid[sel];
				slv->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				}

			EndDialog(hWnd,1);
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


/*
INT_PTR CALLBACK NewLinkDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	SlaveControl *slv = (SlaveControl*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	static Tab<int> sid,bid;

	switch (msg) {
	case WM_INITDIALOG:
		{
		sid.ZeroCount();
		bid.ZeroCount();
		slv = (SlaveControl*)lParam;
		SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

//goto master look at all sub block
		for (int i = 0;i < slv->master->Blocks.Count();i++)
			{
			TSTR blockName = slv->master->SubAnimName(i+1);
			for (int j = 0;j < slv->master->Blocks[i]->controls.Count();j++)
				{
				TSTR subName = slv->master->Blocks[i]->SubAnimName(j);
				TSTR finalName = blockName +" "+ subName;
//check if control is the same as ours
				if (slv->master->Blocks[i]->controls[j]->ClassID() == slv->sub->ClassID())
					{

//add to list box
					sid.Append(1,&j,1);
					bid.Append(1,&i,1);
					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)finalName);
					}
				}
			}	

		SendMessage(GetDlgItem(hWnd,IDC_LIST1),
			LB_SETCURSEL,0,0);
		CenterWindow(hWnd,GetParent(hWnd));
		break;
		}
		


	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
			int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETCURSEL,0,0);
			slv->propBlockID = bid[sel];
			slv->propSubID = sid[sel];
			EndDialog(hWnd,1);
			slv->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

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
*/
