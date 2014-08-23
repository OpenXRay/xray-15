//***************************************************************************
//* Audio Amplitude Float Controller for 3D Studio MAX.
//*	Base class for all "3 point" controllers
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*
class AudioP3Dlg;

class AudioP3Control : public AudioBaseControl {
public:
	Point3 basePoint;
	Point3 targetPoint;

	AudioP3Dlg* pDlg;

	AudioP3Control();
	~AudioP3Control();

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// Reference methods
	RefResult NotifyRefChanged(Interval, RefTargetHandle, PartID&, RefMessage) {return REF_SUCCEED;}

	void EditTrackParams(TimeValue t, ParamDimensionBase *dim, TCHAR *pname,
		HWND hParent, IObjParam *ip, DWORD flags);
};
