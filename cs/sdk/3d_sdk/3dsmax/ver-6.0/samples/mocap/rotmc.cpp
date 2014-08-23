/**********************************************************************
 *<
	FILE: rotMC.cpp

	DESCRIPTION: Rotation motion capture controller

	CREATED BY: Rolf Berteig

	HISTORY: October 30, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
#include "mmanager.h"


static Class_ID rotmcControlClassID(ROT_MOTION_CLASS_ID,0); 

class RotationMC : public MCControl {
	public:
		MCDeviceBinding *bind[3];
		Point3 base;
		int sampleCount;
		Point3 *data;

		RotationMC();
		Class_ID ClassID() {return rotmcControlClassID;}  
		SClass_ID SuperClassID() {return CTRL_ROTATION_CLASS_ID;} 
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_ROTMC);}
		ParamDimension* GetParamDimension(int i) {return stdAngleDim;}
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

class RotMCClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new RotationMC;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_ROTMC);}
	SClass_ID		SuperClassID() {return CTRL_ROTATION_CLASS_ID;}
	Class_ID		ClassID() {return rotmcControlClassID;}
	const TCHAR* 	Category() {return _T("");}
	};
static RotMCClassDesc rotmcCD;
ClassDesc* GetRotMotionDesc() {return &rotmcCD;}


//--- RotationMC methods ----------------------------------------------

RotationMC::RotationMC()
	{
	ReplaceReference(0,CreateInterpRotation());
	bind[0] = bind[1] = bind[2] = NULL;
	base = Point3(0,0,0);
	data = NULL;
	}

RefTargetHandle RotationMC::Clone(RemapDir& remap)
	{
	RotationMC *c = new RotationMC;
	c->MCControlClone(this,remap);
	BaseClone(this, c, remap);
	return c;
	}

void RotationMC::EditTrackParams(
		TimeValue t, ParamDimensionBase *dim, TCHAR *pname, 
		HWND hParent, IObjParam *ip, DWORD flags)
	{
	GenMCParamDlg *dlg = new GenMCParamDlg(this,IDD_MC_ROTATION);
	dlg->DoWindow(hParent, pname);
	}

void RotationMC::GetValueLive(TimeValue t,void *val, GetSetMethod method)
	{
	Point3 pt = base;
	for (int i=0; i<3; i++) if (bind[i]) pt[i] += DegToRad(bind[i]->Eval(t));
	Quat q;
	EulerToQuat(pt,q);
	if (method==CTRL_ABSOLUTE) {		
		*((Quat*)val) = q;
	} else {
		Matrix3 *tm = (Matrix3*)val;		
		PreRotateMatrix(*tm,q);		
		}
	}
									  
void RotationMC::BeginCapture(Interval record,TimeValue sampSize)
	{
	// Set the base point to the controller value at the start time.
	Quat q;
	cont->GetValue(record.Start(),&q,FOREVER,CTRL_ABSOLUTE);	
	QuatToEuler(q,base);

	// Allocate a data buffer
	sampleCount = record.Duration()/sampSize + 1;
	data = new Point3[sampleCount];
	for (int i=0; i<sampleCount; i++) data[i] = Point3(0,0,0);
	}

void RotationMC::Capture(Interval record,TimeValue t,int sample)
	{
	assert(sample>=0 && sample<sampleCount);	
	Point3 pt = base;
	for (int i=0; i<3; i++) if (bind[i]) pt[i] += DegToRad(bind[i]->Eval(t));
	data[sample] = pt;	
	}
  
void RotationMC::EndCapture(Interval record,TimeValue sampSize, KeyReduceStatus *stat) 
	{		
	// Clear any keys out of the record interval
	cont->DeleteTime(record,TIME_INCLEFT|TIME_INCRIGHT|TIME_NOSLIDE);

	// Make keys out of the data
	SuspendAnimate();
	AnimateOn();	
	for (int i=0; i<sampleCount; i++) {
		TimeValue t = record.Start() + i * sampSize;
		Quat q;				
		EulerToQuat(data[i],q);
		cont->SetValue(t,&q,1,CTRL_ABSOLUTE);
		if (i%UPDATE_RATE==0) if (stat->Progress(i)!=KEYREDUCE_CONTINUE) goto abort;
		}	

abort:
	ResumeAnimate();
	delete[] data;
	base = Point3(0,0,0);
	}

void RotationMC::BeginLive(TimeValue t)
	{
	// Set the base point to the controller value at the start time.
	Quat q;
	cont->GetValue(t,&q,FOREVER,CTRL_ABSOLUTE);	
	QuatToEuler(q,base);	
	}


