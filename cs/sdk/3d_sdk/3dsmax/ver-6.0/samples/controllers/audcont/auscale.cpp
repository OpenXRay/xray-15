//***************************************************************************
//* Audio Amplitude Scale Controller for 3D Studio MAX.
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*

#include "auctrl.h"

#ifndef NO_CONTROLLER_AUDIO_SCALE

#include "aup3base.h"
#include "aup3dlg.h"

class AudioScaleControl : public AudioP3Control {
public:
	Class_ID ClassID() { return Class_ID(AUDIO_SCALE_CONTROL_CLASS_ID1, AUDIO_SCALE_CONTROL_CLASS_ID2); }
	SClass_ID SuperClassID() { return CTRL_SCALE_CLASS_ID; } 
	void GetClassName(TSTR& s) {s = AUDIO_SCALE_CONTROL_CNAME;}

	AudioScaleControl();

	void Copy(Control *from);
	RefTargetHandle Clone(RemapDir& remap);
	void RefDeleted();

	void *CreateTempValue() {return new ScaleValue;}
	void DeleteTempValue(void *val) {delete (ScaleValue*)val;}
	void ApplyValue(void *val, void *delta) {ApplyScaling( *((Matrix3*)val), *((ScaleValue*)delta) );}
	void MultiplyValue(void *val, float m) {*((ScaleValue*)val) *= m;}

	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method);
};

// Class description
class AudioScaleClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AudioScaleControl(); }
	const TCHAR *	ClassName() { return AUDIO_SCALE_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_SCALE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(AUDIO_SCALE_CONTROL_CLASS_ID1,AUDIO_SCALE_CONTROL_CLASS_ID2); }
	const TCHAR* 	Category() { return _T("");  }
};

static AudioScaleClassDesc scaleAudioCD;

ClassDesc* GetAudioScaleDesc()
{
	return &scaleAudioCD;
}


AudioScaleControl::AudioScaleControl() 
{
	type = AUDIO_SCALE_CONTROL_CLASS_ID1;
	basePoint.x = 1.0f; basePoint.y = 1.0f; basePoint.z = 1.0f;
	targetPoint.x = 1.0f; targetPoint.y = 1.0f; targetPoint.z = 1.0f;
} 

void AudioScaleControl::Copy(Control* from)
{
	ScaleValue fval;

	if (from->ClassID() == ClassID()) {
		basePoint = ((AudioScaleControl*)from)->basePoint;
		targetPoint = ((AudioScaleControl*)from)->targetPoint;
	}
	else {
		from->GetValue(0, &fval, Interval(0,0));
		basePoint.x = fval.s.x;
		basePoint.y = fval.s.y;
		basePoint.z = fval.s.z;

		targetPoint.x = fval.s.x;
		targetPoint.y = fval.s.y;
		targetPoint.z = fval.s.z;
	}
}

RefTargetHandle AudioScaleControl::Clone(RemapDir& remap)
{
	// make a new controller and give it our param values.
	AudioScaleControl *cont = new AudioScaleControl;
	// *cont = *this;
	cont->type = type;
	cont->range = range;
	cont->channel = channel;
	cont->absolute = absolute;
	cont->numsamples = numsamples;
	cont->enableRuntime = enableRuntime;
	cont->szFilename = szFilename;
	cont->quickdraw = quickdraw;
	cont->basePoint = basePoint;
	cont->targetPoint = targetPoint;
	BaseClone(this, cont, remap);
	return cont;
}

// When the last reference to a controller is
// deleted we need to close the realtime recording device and 
// its parameter dialog needs to be closed
void AudioScaleControl::RefDeleted()
{
	int c=0;
	RefListItem  *ptr = GetRefList().first;
	while (ptr) {
		if (ptr->maker!=NULL) {
			if (ptr->maker->SuperClassID()) c++;
		}
		ptr = ptr->next;
	}	
	if (!c) {
		// Stop the real-time recording is the object is deleted.
		if (rtwave->IsRecording())
			rtwave->StopRecording();

		if (pDlg != NULL)
			DestroyWindow(pDlg->hWnd);
	}
}

// Return a value at at specific instant.
void AudioScaleControl::GetValueLocalTime(TimeValue t, void *val, Interval &valid,
	GetSetMethod method)
{
	float samp;

	valid.SetInstant(t); // This controller is always changing.

	// Get a sample from the wave
	samp = SampleAtTime(t - range.Start(), 0, FALSE);

	// Create a scale value based on the sample and the base/targets
	// for X, Y and Z values
	*((ScaleValue*)val) = ScaleValue(
		Point3(
			basePoint.x + (targetPoint.x - basePoint.x) * samp,
			basePoint.y + (targetPoint.y - basePoint.y) * samp,
			basePoint.z + (targetPoint.z - basePoint.z) * samp),
			Quat(0.0,0.0,0.0,1.0));
}

void AudioScaleControl::SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method)
{
}

#endif // NO_CONTROLLER_AUDIO_SCALE

