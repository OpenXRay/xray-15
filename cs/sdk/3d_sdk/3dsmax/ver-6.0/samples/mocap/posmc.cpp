/**********************************************************************
 *<
	FILE: posMC.cpp

	DESCRIPTION: Position motion capture controller

	CREATED BY: Rolf Berteig

	HISTORY: October 30, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
#include "mmanager.h"


static Class_ID posmcControlClassID(POS_MOTION_CLASS_ID,0); 

class PositionMC : public MCControl {
	public:
		MCDeviceBinding *bind[3];
		Point3 base;
		int sampleCount;
		Point3 *data;

		PositionMC();
		Class_ID ClassID() {return posmcControlClassID;}  
		SClass_ID SuperClassID() {return CTRL_POSITION_CLASS_ID;} 
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_POSMC);}
		ParamDimension* GetParamDimension(int i) {return stdWorldDim;}
		RefTargetHandle Clone(RemapDir& remap);

		int NumDeviceBindings() {return 3;}
		MCDeviceBinding *GetDeviceBinding(int i) {return bind[i];}
		void SetDeviceBinding(int i,MCDeviceBinding *b) {bind[i]=b;}
		void GetValueLive(TimeValue t,void *val, GetSetMethod method);
		void BeginCapture(Interval record,TimeValue sampSize);
		void EndCapture(Interval record,TimeValue sampSize, KeyReduceStatus *stat);
		void Capture(Interval record,TimeValue t,int sample);
		void BeginLive(TimeValue t);

		void EditTrackParams(
			TimeValue t, ParamDimensionBase *dim, TCHAR *pname, 
			HWND hParent, IObjParam *ip, DWORD flags);
	};


//--- Class Descriptor -----------------------------------------------

class PosMCClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new PositionMC;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_POSMC);}
	SClass_ID		SuperClassID() {return CTRL_POSITION_CLASS_ID;}
	Class_ID		ClassID() {return posmcControlClassID;}
	const TCHAR* 	Category() {return _T("");}
	};
static PosMCClassDesc posmcCD;
ClassDesc* GetPosMotionDesc() {return &posmcCD;}


//--- PositionMC methods ----------------------------------------------

PositionMC::PositionMC()
	{
	ReplaceReference(0,CreateInterpPosition());
	bind[0] = bind[1] = bind[2] = NULL;
	base = Point3(0,0,0);
	data = NULL;
	}

RefTargetHandle PositionMC::Clone(RemapDir& remap)
	{
	PositionMC *c = new PositionMC;
	c->MCControlClone(this,remap);
	BaseClone(this, c, remap);
	return c;
	}

void PositionMC::EditTrackParams(
		TimeValue t, ParamDimensionBase *dim, TCHAR *pname, 
		HWND hParent, IObjParam *ip, DWORD flags)
	{
	GenMCParamDlg *dlg = new GenMCParamDlg(this,IDD_MC_POSITION);
	dlg->DoWindow(hParent, pname);
	}

void PositionMC::GetValueLive(TimeValue t,void *val, GetSetMethod method)
	{
	Point3 pt = base;
	for (int i=0; i<3; i++) if (bind[i]) pt[i] += bind[i]->Eval(t);
	if (method==CTRL_ABSOLUTE) {
		*((Point3*)val) = pt;
	} else {
		Matrix3 *tm = (Matrix3*)val;
		tm->PreTranslate(pt);
		}
	}

void PositionMC::BeginCapture(Interval record,TimeValue sampSize)
	{
	// Set the base point to the controller value at the start time.
	cont->GetValue(record.Start(),&base,FOREVER,CTRL_ABSOLUTE);	

	// Allocate a data buffer
	sampleCount = record.Duration()/sampSize + 1;
	data = new Point3[sampleCount];
	for (int i=0; i<sampleCount; i++) data[i] = Point3(0,0,0);
	}

void PositionMC::Capture(Interval record,TimeValue t,int sample)
	{
	assert(sample>=0 && sample<sampleCount);	
	GetValueLive(t,&data[sample], CTRL_ABSOLUTE);	
	}
  
void PositionMC::EndCapture(Interval record,TimeValue sampSize, KeyReduceStatus *stat) 
	{		
	// Clear any keys out of the record interval
	cont->DeleteTime(record,TIME_INCLEFT|TIME_INCRIGHT|TIME_NOSLIDE);

	// Make keys out of the data
	SuspendAnimate();
	AnimateOn();	
	for (int i=0; i<sampleCount; i++) {
		TimeValue t = record.Start() + i * sampSize;
		cont->SetValue(t,&data[i],1,CTRL_ABSOLUTE);
		if (i%UPDATE_RATE==0) if (stat->Progress(i)!=KEYREDUCE_CONTINUE) goto abort;
		}	

abort:
	ResumeAnimate();
	delete[] data;
	base = Point3(0,0,0);
	}

void PositionMC::BeginLive(TimeValue t)
	{
	// Set the base point to the controller value at the start time.
	cont->GetValue(t,&base,FOREVER,CTRL_ABSOLUTE);	
	}


