//***************************************************************************
//* Audio Amplitude Rotation Controller for 3D Studio MAX.
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*

#include "auctrl.h"

#ifndef NO_CONTROLLER_AUDIO_ROTATION

#include "aup3base.h"
#include "aup3dlg.h"

class AudioRotationControl : public AudioP3Control {
public:
	Class_ID ClassID() { return Class_ID(AUDIO_ROTATION_CONTROL_CLASS_ID1, AUDIO_ROTATION_CONTROL_CLASS_ID2); }
	SClass_ID SuperClassID() { return CTRL_ROTATION_CLASS_ID; } 
	void GetClassName(TSTR& s) {s = AUDIO_ROTATION_CONTROL_CNAME;}

	AudioRotationControl();

	void Copy(Control *from);
	RefTargetHandle Clone(RemapDir& remap);
	void RefDeleted();

	void *CreateTempValue() {return new Quat;}
	void DeleteTempValue(void *val) {delete (Quat *)val;}
	void ApplyValue(void *val, void *delta) {PreRotateMatrix( *((Matrix3*)val), *((Quat*)delta) );}
	void MultiplyValue(void *val, float m) {*((Quat*)val) *= m;}

	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method);
};

// Class description
class AudioRotationClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AudioRotationControl(); }
	const TCHAR *	ClassName() { return AUDIO_ROTATION_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_ROTATION_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(AUDIO_ROTATION_CONTROL_CLASS_ID1,AUDIO_ROTATION_CONTROL_CLASS_ID2); }
	const TCHAR* 	Category() { return _T("");  }
};

static AudioRotationClassDesc rotationAudioCD;

ClassDesc* GetAudioRotationDesc()
{
	return &rotationAudioCD;
}

AudioRotationControl::AudioRotationControl() 
{
	type = AUDIO_ROTATION_CONTROL_CLASS_ID1;
	basePoint.x = 0.0f; basePoint.y = 0.0f; basePoint.z = 0.0f;
	targetPoint.x = 0.0f; targetPoint.y = 0.0f; targetPoint.z = 0.0f;
} 

void AudioRotationControl::Copy(Control* from)
{
	Quat fval;
	float ang[3];


	if (from->ClassID() == ClassID()) {
		basePoint = ((AudioRotationControl*)from)->basePoint;
		targetPoint = ((AudioRotationControl*)from)->targetPoint;
	}
	else {
		from->GetValue(0, &fval, Interval(0,0));
		QuatToEuler(fval, ang);
		basePoint.x = ang[0];
		basePoint.y = ang[1];
		basePoint.z = ang[2];

		targetPoint.x = ang[0];
		targetPoint.y = ang[1];
		targetPoint.z = ang[2];
	}
}

RefTargetHandle AudioRotationControl::Clone(RemapDir& remap)
{
	// make a new controller and give it our param values.
	AudioRotationControl *cont = new AudioRotationControl;
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
void AudioRotationControl::RefDeleted()
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

// Get the value at a specific instance
void AudioRotationControl::GetValueLocalTime(TimeValue t, void *val, Interval &valid,
	GetSetMethod method)
{
	float ang[3];
	float samp;

	valid.SetInstant(t); // This controller is always changing.

	// Calculate the angle based on base, target and sample
	samp = SampleAtTime(t - range.Start(), 0, FALSE);
	ang[0] = basePoint.x + (targetPoint.x - basePoint.x) * samp;
	ang[1] = basePoint.y + (targetPoint.y - basePoint.y) * samp;
	ang[2] = basePoint.z + (targetPoint.z - basePoint.z) * samp;

	EulerToQuat(ang,*((Quat*)val));
}

void AudioRotationControl::SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method)
{
}

#endif // NO_CONTROLLER_AUDIO_ROTATION

