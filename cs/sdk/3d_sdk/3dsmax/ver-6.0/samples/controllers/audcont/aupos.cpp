//***************************************************************************
//* Audio Amplitude Position Controller for 3D Studio MAX.
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*

#include "auctrl.h"

#ifndef NO_CONTROLLER_AUDIO_POSITION

#include "aup3base.h"
#include "aup3dlg.h"

class AudioPositionControl : public AudioP3Control {
public:
	Class_ID ClassID() { return Class_ID(AUDIO_POSITION_CONTROL_CLASS_ID1, AUDIO_POSITION_CONTROL_CLASS_ID2); }
	SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; } 
	void GetClassName(TSTR& s) {s = AUDIO_POSITION_CONTROL_CNAME;}

	AudioPositionControl();
	AudioPositionControl& operator=(const AudioPositionControl& from);

	void Copy(Control *from);
	RefTargetHandle Clone(RemapDir& remap);
	void RefDeleted();

	void *CreateTempValue() {return new Point3;}
	void DeleteTempValue(void *val) {delete (Point3*)val;}
	void ApplyValue(void *val, void *delta) {((Matrix3*)val)->PreTranslate(*((Point3*)delta));}
	void MultiplyValue(void *val, float m) {*((Point3*)val) *= m;}

	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method);
};

// Class description
class AudioPositionClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { return new AudioPositionControl(); }
	const TCHAR *	ClassName() { return AUDIO_POSITION_CONTROL_CNAME; }
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(AUDIO_POSITION_CONTROL_CLASS_ID1,AUDIO_POSITION_CONTROL_CLASS_ID2); }
	const TCHAR* 	Category() { return _T("");  }
};

static AudioPositionClassDesc positionAudioCD;

ClassDesc* GetAudioPositionDesc()
{
	return &positionAudioCD;
}

AudioPositionControl::AudioPositionControl() 
{
	type = AUDIO_POINT3_CONTROL_CLASS_ID1;
	basePoint.x = 0.0f; basePoint.y = 0.0f; basePoint.z = 0.0f;
	targetPoint.x = 0.0f; targetPoint.y = 0.0f; targetPoint.z = 0.0f;
} 

void AudioPositionControl::Copy(Control* from)
{
	Point3 fval;

	if (from->ClassID() == ClassID()) {
		basePoint = ((AudioPositionControl*)from)->basePoint;
		targetPoint = ((AudioPositionControl*)from)->targetPoint;
	}
	else {
		from->GetValue(0, &fval, Interval(0,0));
		basePoint = fval;
		targetPoint = fval;
	}
}

RefTargetHandle AudioPositionControl::Clone(RemapDir& remap)
{
	// Make a new controller and give it our param values.
	AudioPositionControl *cont = new AudioPositionControl;
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
void AudioPositionControl::RefDeleted()
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

void AudioPositionControl::GetValueLocalTime(TimeValue t, void *val, Interval &valid,
	GetSetMethod method)
{
	valid.SetInstant(t); // This controller is always changing.

	// Multiply the target-base with sample and add base
	*((Point3*)val) = basePoint + (targetPoint - basePoint) * SampleAtTime(t - range.Start(), 0, FALSE);
}

void AudioPositionControl::SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method)
{
}

#endif // NO_CONTROLLER_AUDIO_POSITION

