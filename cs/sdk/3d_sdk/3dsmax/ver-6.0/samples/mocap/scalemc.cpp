/**********************************************************************
 *<
	FILE: scaleMC.cpp

	DESCRIPTION: Scale motion capture controller

	CREATED BY: Rolf Berteig

	HISTORY: October 30, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
#include "mmanager.h"


static Class_ID scalemcControlClassID(SCALE_MOTION_CLASS_ID,0); 

class ScaleMC : public MCControl {
	public:
		MCDeviceBinding *bind[3];
		Point3 base;
		int sampleCount;
		Point3 *data;

		ScaleMC();
		Class_ID ClassID() {return scalemcControlClassID;}  
		SClass_ID SuperClassID() {return CTRL_SCALE_CLASS_ID;} 
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_SCALEMC);}
		ParamDimension* GetParamDimension(int i) {return stdPercentDim;}
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

class ScaleMCClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new ScaleMC;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_SCALEMC);}
	SClass_ID		SuperClassID() {return CTRL_SCALE_CLASS_ID;}
	Class_ID		ClassID() {return scalemcControlClassID;}
	const TCHAR* 	Category() {return _T("");}
	};
static ScaleMCClassDesc scalemcCD;
ClassDesc* GetScaleMotionDesc() {return &scalemcCD;}


//--- ScaleMC methods ----------------------------------------------

ScaleMC::ScaleMC()
	{
	ReplaceReference(0,CreateInterpScale());
	bind[0] = bind[1] = bind[2] = NULL;
	base = Point3(1,1,1);
	data = NULL;
	}

RefTargetHandle ScaleMC::Clone(RemapDir& remap)
	{
	ScaleMC *c = new ScaleMC;
	c->MCControlClone(this,remap);
	BaseClone(this, c, remap);
	return c;
	}

void ScaleMC::EditTrackParams(
		TimeValue t, ParamDimensionBase *dim, TCHAR *pname, 
		HWND hParent, IObjParam *ip, DWORD flags)
	{
	GenMCParamDlg *dlg = new GenMCParamDlg(this,IDD_MC_SCALE);
	dlg->DoWindow(hParent, pname);
	}

void ScaleMC::GetValueLive(TimeValue t,void *val, GetSetMethod method)
	{
	Point3 pt = base;
	for (int i=0; i<3; i++) {
		if (bind[i]) pt[i] += bind[i]->Eval(t)/100.0f;		
		}
	if (method==CTRL_ABSOLUTE) {
		*((ScaleValue*)val) = ScaleValue(pt);
	} else {
		Matrix3 *tm = (Matrix3*)val;
		tm->PreScale(pt);
		}
	}

void ScaleMC::BeginCapture(Interval record,TimeValue sampSize)
	{
	// Set the base point to the controller value at the start time.
	ScaleValue s;
	cont->GetValue(record.Start(),&s,FOREVER,CTRL_ABSOLUTE);	
	base = s.s;

	// Allocate a data buffer
	sampleCount = record.Duration()/sampSize + 1;
	data = new Point3[sampleCount];
	for (int i=0; i<sampleCount; i++) data[i] = Point3(1,1,1);
	}

void ScaleMC::Capture(Interval record,TimeValue t,int sample)
	{
	assert(sample>=0 && sample<sampleCount);	
	ScaleValue s;
	GetValueLive(t,&s, CTRL_ABSOLUTE);	
	data[sample] = s.s; 
	}
  
void ScaleMC::EndCapture(Interval record,TimeValue sampSize, KeyReduceStatus *stat) 
	{		
	// Clear any keys out of the record interval
	cont->DeleteTime(record,TIME_INCLEFT|TIME_INCRIGHT|TIME_NOSLIDE);

	// Make keys out of the data
	SuspendAnimate();
	AnimateOn();	
	for (int i=0; i<sampleCount; i++) {
		TimeValue t = record.Start() + i * sampSize;
		ScaleValue s(data[i]);
		cont->SetValue(t,&s,1,CTRL_ABSOLUTE);
		if (i%UPDATE_RATE==0) if (stat->Progress(i)!=KEYREDUCE_CONTINUE) goto abort;
		}	

abort:
	ResumeAnimate();
	delete[] data;
	base = Point3(1,1,1);
	}

void ScaleMC::BeginLive(TimeValue t)
	{
	// Set the base point to the controller value at the start time.
	ScaleValue s;
	cont->GetValue(t,&s,FOREVER,CTRL_ABSOLUTE);
	base = s.s;
	}

