/**********************************************************************
 *<
	FILE: followu.cpp

	DESCRIPTION: A utility that generates path follow

	CREATED BY: Rolf Berteig

	HISTORY: created 1/27/97

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "ctrl.h"
#include "utilapi.h"

#ifndef NO_UTILITY_FOLLOWBANK	// russom - 12/04/01

#define FOLLOW_CLASS_ID			Class_ID(0x4557a2c4,0x00186d34)
#define FOLLOW_CNAME			GetString(IDS_RB_FOLLOWUTIL)

class FollowUtil : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;		

		BOOL bank, flip;
		float bankAmount, tracking;
		TimeValue start, end;
		int samples;

		FollowUtil();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}
		void SelectionSetChanged(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		void SetStates(HWND hWnd);

		Matrix3 CalcRefFrame(TimeValue t,INode *node);
		Point3 GetPosition(INode *node,TimeValue t);
		void DoFollow();		
	};
static FollowUtil theFollowUtil;

//--- Class Descriptor ------------------------------------------

class FollowUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theFollowUtil;}
	const TCHAR *	ClassName() {return FOLLOW_CNAME;}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return FOLLOW_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	void			ResetClassParams(BOOL fileReset);
	};

void FollowUtilClassDesc::ResetClassParams(BOOL fileReset) 
	{
	if (fileReset) {
		theFollowUtil = FollowUtil();
		}
	}

static FollowUtilClassDesc followUtilDesc;
ClassDesc* GetFollowUtilDesc() {return &followUtilDesc;}

#define BANKSCALE 100.0f
#define FromBankUI(a) ((a)*BANKSCALE)
#define ToBankUI(a)	  ((a)/BANKSCALE)

#define TRACKSCALE 0.04f
#define FromTrackUI(a) ((a)*TRACKSCALE)
#define ToTrackUI(a)   ((a)/TRACKSCALE)


///--- Panel Proc ------------------------------------------------

static INT_PTR CALLBACK FollowUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theFollowUtil.Init(hWnd);
			break;

		case WM_DESTROY:
			theFollowUtil.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_PATH_BANK:
					theFollowUtil.bank = IsDlgButtonChecked(hWnd,IDC_PATH_BANK);
					theFollowUtil.SetStates(hWnd);
					break;
				
				case IDC_PATH_ALLOWFLIP:
					theFollowUtil.flip = IsDlgButtonChecked(hWnd,IDC_PATH_ALLOWFLIP);
					break;

				case IDC_FOLLOW_APPLY:
					theFollowUtil.DoFollow();
					break;

				case IDOK:
					theFollowUtil.iu->CloseUtility();
					break;
				}
			break;

		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl*)lParam;
			switch (LOWORD(wParam)) {
				case IDC_FOLLOW_BANKSPIN:
					theFollowUtil.bankAmount = FromBankUI(spin->GetFVal());
					break;

				case IDC_FOLLOW_TRACKSPIN:
					theFollowUtil.tracking = FromTrackUI(spin->GetFVal());
					break;

				case IDC_FOLLOW_STARTSPIN:
					theFollowUtil.start = spin->GetIVal();
					break;

				case IDC_FOLLOW_ENDSPIN:
					theFollowUtil.end = spin->GetIVal();
					break;

				case IDC_FOLLOW_SAMPSPIN:
					theFollowUtil.samples = spin->GetIVal();
					break;
				}
			break;
			}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theFollowUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}	

// --- FollowUtil methods -----------------------------------------

FollowUtil::FollowUtil()
	{
	iu = NULL;
	ip = NULL;
    hPanel = NULL;
	bank = FALSE;
	flip = FALSE;
	bankAmount = FromBankUI(0.5f);
	tracking = FromTrackUI(0.5f);
	start = 0;
	end = 16000;
	samples = 25;
	}

void FollowUtil::BeginEditParams(Interface *ip,IUtil *iu)
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_FOLLOW_PANEL),
		FollowUtilDlgProc,
		GetString(IDS_RB_FOLLOWUTIL),
		0);
	}

void FollowUtil::EndEditParams(Interface *ip,IUtil *iu)
	{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}


void FollowUtil::SelectionSetChanged(Interface *ip,IUtil *iu)
	{
	SetStates(hPanel);
	if (ip->GetSelNodeCount()==1) {
		SetDlgItemText(hPanel,IDC_SEL_NAME,ip->GetSelNode(0)->GetName());
	} else if (ip->GetSelNodeCount()) {
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_RB_MULTISEL));
	} else {
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_RB_NONESEL));
		}	
	}

void FollowUtil::Init(HWND hWnd)
	{
	hPanel = hWnd;

	CheckDlgButton(hWnd,IDC_PATH_BANK,bank);
	CheckDlgButton(hWnd,IDC_PATH_ALLOWFLIP,flip);

	ISpinnerControl * spin;
	spin = GetISpinner(GetDlgItem(hWnd,IDC_FOLLOW_STARTSPIN));
	spin->SetLimits(TIME_NegInfinity,TIME_PosInfinity,FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_FOLLOW_START),EDITTYPE_TIME);
	spin->SetValue(start,FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_FOLLOW_ENDSPIN));
	spin->SetLimits(TIME_NegInfinity,TIME_PosInfinity,FALSE);
	spin->SetScale(10.0f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_FOLLOW_END),EDITTYPE_TIME);
	spin->SetValue(end,FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_FOLLOW_SAMPSPIN));
	spin->SetLimits(1,999999999,FALSE);
	spin->SetScale(0.1f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_FOLLOW_SAMP),EDITTYPE_INT);
	spin->SetValue(samples,FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_FOLLOW_BANKSPIN));
	spin->SetLimits(-999999999,999999999,FALSE);
	spin->SetScale(0.01f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_FOLLOW_BANK),EDITTYPE_FLOAT);
	spin->SetValue(ToBankUI(bankAmount),FALSE);
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_FOLLOW_TRACKSPIN));
	spin->SetLimits(0.01f,10.0f,FALSE);
	spin->SetScale(0.01f);
	spin->LinkToEdit(GetDlgItem(hWnd,IDC_FOLLOW_TRACK),EDITTYPE_FLOAT);
	spin->SetValue(ToTrackUI(tracking),FALSE);
	ReleaseISpinner(spin);

	SelectionSetChanged(ip,iu);	
	}

void FollowUtil::Destroy(HWND hWnd)
	{
	hPanel = NULL;
	}

void FollowUtil::SetStates(HWND hWnd)
	{
	if (ip->GetSelNodeCount()) {
		EnableWindow(GetDlgItem(hPanel,IDC_FOLLOW_APPLY),TRUE);
	} else {
		EnableWindow(GetDlgItem(hPanel,IDC_FOLLOW_APPLY),FALSE);
		}

	if (bank) {
		EnableWindow(GetDlgItem(hWnd,IDC_FOLLOW_BANKLABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_FOLLOW_BANKSPIN),TRUE);		
		EnableWindow(GetDlgItem(hWnd,IDC_FOLLOW_TRACKLABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_FOLLOW_TRACKSPIN),TRUE);
	} else {
		EnableWindow(GetDlgItem(hWnd,IDC_FOLLOW_BANKLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_FOLLOW_BANKSPIN),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_FOLLOW_TRACKLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_FOLLOW_TRACKSPIN),FALSE);
		}
	}


Point3 FollowUtil::GetPosition(INode *node,TimeValue t)
	{
	Matrix3 tm = node->GetNodeTM(t);
	return tm.GetTrans();
	}

#define DT			1
#define NUMSTEPS	5

Matrix3 FollowUtil::CalcRefFrame(TimeValue t,INode *node)
	{
	Interval valid;
	Matrix3 tm(1);	
	TimeValue s = start, e = end;
	if (s==e) return tm;
	if (s>e) {
		int temp = e;
		e = s;
		s = temp;
		}
	float u = float(t-s)/float(e);	
	int length = e-s;

	Point3 pt0, pt1;
	pt0 = GetPosition(node,t-DT);
	pt1 = GetPosition(node,t+DT);
	
	Point3 pathNorm(0,0,0);
	if (flip) {
		// Compute the normal to the plane of the path by sampling points on the path
#define NUM_SAMPLES 20
		Point3 v[NUM_SAMPLES], cent(0,0,0);
		for (int i=0; i<NUM_SAMPLES; i++) {
			TimeValue tt = int((float(i)/float(NUM_SAMPLES))*float(length)+s);
			v[i] = GetPosition(node,tt);
			cent += v[i];
			}		
		cent /= float(NUM_SAMPLES);
		for (i=1; i<NUM_SAMPLES; i++) {
			pathNorm += Normalize((v[i]-cent)^(v[i-1]-cent));
			}
		pathNorm = Normalize(pathNorm);
		}

	// X
	tm.SetRow(0,Normalize(pt1-pt0));

	if (flip) {
		// Choose Z in the plane of the path		
		// Z
		tm.SetRow(2,Normalize(tm.GetRow(0)^pathNorm));

		// Y
		tm.SetRow(1,tm.GetRow(2)^tm.GetRow(0));
	} else {
		// Choose Y in the world XY plane
		// Y
		tm.SetRow(1,Normalize(Point3(0,0,1)^tm.GetRow(0)));

		// Z
		tm.SetRow(2,tm.GetRow(0)^tm.GetRow(1));
		}

	
	if (bank) {
		// Average over NUMSTEPS samples
		Point3 pt0, pt1, pt2, v0, v1;
		float cv = 0.0f;
		u -= float(NUMSTEPS/2+1)*tracking;
						
		if (u+(NUMSTEPS+2)*tracking > 1.0f) u = 1.0f - (NUMSTEPS+2)*tracking;
		if (u<0.0f) u=0.0f;			

		pt1 = GetPosition(node,int(u*length)+s);
		u += tracking;		
		pt2 = GetPosition(node,int(u*length)+s);
		u += tracking;		
		for (int i=0; i<NUMSTEPS; i++) {			
			pt0 = pt1;
			pt1 = pt2;
			if (u>1.0f) {				
				break;
			} else {				
				pt2 = GetPosition(node,int(u*length)+s);
				}
			v0 = Normalize(pt2-pt1);
			v1 = Normalize(pt1-pt0);			
			v0.z = v1.z = 0.0f; // remove Z component.
			cv += (v0^v1).z * bankAmount / Length(pt1-pt0);
			u  += tracking;			
			}	
		if (i) tm.PreRotateX(cv/float(i));
		}

	return tm;
	}

void FollowUtil::DoFollow()
	{
	theHold.Begin();
	SuspendAnimate();
	AnimateOn();
	for (int j=0; j<ip->GetSelNodeCount(); j++) {
		INode *node = ip->GetSelNode(j);
		Control *cont=node->GetTMController()->GetRotationController();		
		if (cont && cont->IsKeyable()) {
			cont->DeleteTime(Interval(start,end),TIME_INCLEFT|TIME_INCRIGHT|TIME_NOSLIDE);
			for (int i=0; i<=samples; i++) {
				TimeValue t = int((float(i)/float(samples))*(end-start)) + start;
				Matrix3 tm = CalcRefFrame(t,node);
				Quat q(tm);
				cont->SetValue(t,&q,1,CTRL_ABSOLUTE);
				}
			}
		}
	ResumeAnimate();
	theHold.Accept(GetString(IDS_RB_APPLYFOLLOW));
	ip->RedrawViews(ip->GetTime());
	}

#endif // NO_UTILITY_FOLLOWBANK
