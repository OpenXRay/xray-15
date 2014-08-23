//***************************************************************************
//* Audio Amplitude Controller for 3D Studio MAX.
//*	Dialog class for all "3 point" controllers
//* 
//* Code & design by Christer Janson
//* Autodesk European Developer Support - Neuchatel Switzerland
//* December 1995
//*
static INT_PTR CALLBACK AudioP3DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class AudioP3Dlg : public ReferenceMaker {
	public:
		AudioP3Control *cont;
		ParamDimensionBase *dim;
		IObjParam *ip;
		HWND hWnd;
		BOOL valid;

		ISpinnerControl *iSamples;
		ISpinnerControl *iBaseX;
		ISpinnerControl *iBaseY;
		ISpinnerControl *iBaseZ;
		ISpinnerControl *iTargetX;
		ISpinnerControl *iTargetY;
		ISpinnerControl *iTargetZ;
		ISpinnerControl *iThreshold;

		AudioP3Dlg(AudioP3Control *cont, ParamDimensionBase *dim, IObjParam *ip, HWND hParent);
		~AudioP3Dlg();

		void Invalidate();
		void Update();
		void SetupUI(HWND hWnd);
		void Change();
		void WMCommand(int id, int notify, HWND hCtrl);
		void SpinnerChange(int id,BOOL drag);
		void SpinnerStart(int id);
		void SpinnerEnd(int id,BOOL cancel);
		void EnableChannelUI(BOOL state);

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return cont;}
		void SetReference(int i, RefTargetHandle rtarg) {cont=(AudioP3Control*)rtarg;}

		void SetActive();
		void ShowAbout(HWND hWnd);
};
