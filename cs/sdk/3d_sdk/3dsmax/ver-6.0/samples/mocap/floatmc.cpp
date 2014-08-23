/**********************************************************************
 *<
	FILE: floatMC.cpp

	DESCRIPTION: Float motion capture controller

	CREATED BY: Rolf Berteig

	HISTORY: October 30, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "motion.h"
#include "mmanager.h"


static Class_ID floatmcControlClassID(FLOAT_MOTION_CLASS_ID,0); 

class FloatMC : public MCControl {
	public:
		MCDeviceBinding *bind;
		float base;
		int sampleCount;
		float *data;

		FloatMC();
		Class_ID ClassID() {return floatmcControlClassID;}  
		SClass_ID SuperClassID() {return CTRL_FLOAT_CLASS_ID;} 
		void GetClassName(TSTR& s) {s = GetString(IDS_RB_FLOATMC);}
		ParamDimension* GetParamDimension(int i) {return defaultDim;}
		RefTargetHandle Clone(RemapDir& remap);

		int NumDeviceBindings() {return 1;}
		MCDeviceBinding *GetDeviceBinding(int i) {return bind;}
		void SetDeviceBinding(int i,MCDeviceBinding *b) {bind=b;}
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

class FloatMCClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading) {return new FloatMC;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_FLOATMC);}
	SClass_ID		SuperClassID() {return CTRL_FLOAT_CLASS_ID;}
	Class_ID		ClassID() {return floatmcControlClassID;}
	const TCHAR* 	Category() {return _T("");}
	};
static FloatMCClassDesc floatmcCD;
ClassDesc* GetFloatMotionDesc() {return &floatmcCD;}


//--- FloatMC methods ----------------------------------------------

FloatMC::FloatMC()
	{
	ReplaceReference(0,CreateInterpFloat());
	bind = NULL;
	base = 0.0f;
	data = NULL;
	}

RefTargetHandle FloatMC::Clone(RemapDir& remap)
	{
	FloatMC *c = new FloatMC;
	c->MCControlClone(this,remap);
	BaseClone(this, c, remap);
	return c;
	}

void FloatMC::EditTrackParams(
		TimeValue t, ParamDimensionBase *dim, TCHAR *pname, 
		HWND hParent, IObjParam *ip, DWORD flags)
	{
	GenMCParamDlg *dlg = new GenMCParamDlg(this,IDD_MC_FLOAT);
	dlg->DoWindow(hParent, pname);
	}

void FloatMC::GetValueLive(TimeValue t,void *val, GetSetMethod method)
	{
	float pt = base;
	if (bind) pt += bind->Eval(t);
	if (method==CTRL_ABSOLUTE) {
		*((float*)val)  = pt;
	} else {
		*((float*)val) += pt;
		}
	}


void FloatMC::BeginCapture(Interval record,TimeValue sampSize)
	{
	// Set the base point to the controller value at the start time.
	cont->GetValue(record.Start(),&base,FOREVER,CTRL_ABSOLUTE);	

	// Allocate a data buffer
	sampleCount = record.Duration()/sampSize + 1;
	data = new float[sampleCount];
	for (int i=0; i<sampleCount; i++) data[i] = 0.0f;
	}

void FloatMC::Capture(Interval record,TimeValue t,int sample)
	{
	assert(sample>=0 && sample<sampleCount);	
	GetValueLive(t,&data[sample], CTRL_ABSOLUTE);	
	}
  
void FloatMC::EndCapture(Interval record,TimeValue sampSize, KeyReduceStatus *stat)
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
	base = 0.0f;
	}

void FloatMC::BeginLive(TimeValue t)
	{
	// Set the base point to the controller value at the start time.
	cont->GetValue(t,&base,FOREVER,CTRL_ABSOLUTE);	
	}
