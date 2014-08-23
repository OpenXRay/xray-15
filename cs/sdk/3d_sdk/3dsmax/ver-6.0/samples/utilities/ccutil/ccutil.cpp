/**********************************************************************
 *<
	FILE: CCUtil.cpp

	DESCRIPTION:	Test Utility to show the use of CurveControl

	CREATED BY: Nikolai Sander, Kinetix Development

	HISTORY: Created 10/15/98

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "max.h"
#include "utilapi.h"
#include "icurvctl.h"
#include "resource.h"

#define CCUTIL_CLASS_ID	Class_ID(0x36d873ab, 0x6d2c806e)

static HIMAGELIST hTools = NULL;
static void LoadCurveControlResources();
//static void RegisterCurveCtlWindow();
LRESULT CALLBACK CurveCtlWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

extern TCHAR *GetString(int id);

static char buf[11];

#define CURVECTLWINDOW				_T("CurveCtlWindow")

class DummyRefMaker : public ReferenceMaker
{
	void DeleteThis() {}
	virtual void* GetInterface(ULONG id);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message){return REF_DONTCARE;}
};

class CCUtil : public UtilityObj, public ResourceMakerCallback {

	friend class CCUtilClassDesc;
	
public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		float xMin, xMax;
		float xZoom, yZoom;
		int xScroll, yScroll;
		
		//Constructor/Destructor
		CCUtil();
		~CCUtil();

		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		int  GetCurveNum();
		void ReadPtVals(Point2 &in, Point2 &out, Point2 &pos, int &flags, int &index);
		void ShowCurveControl();
		void ChangeCurveNumbers(BOOL inc);
		void SetColor(BOOL Active);
		void SetXRange();
		void SetZoom();
		void GetZoom();
		void SetScroll();
		void GetScroll();
		void SetDisplay();
		void GetCurveValues(BOOL Active);
		void SetAnimated();
		void SetFlags();
		void SetCommand();
		void GetPtInfo();
		void SetPt(BOOL insert);
		void DelPt();
		void GetPlacement();
		void SetPlacement();
		void SetSwatchColor(ICurve *pCurve,int index);

		
		BOOL SetCustomImageList(HIMAGELIST &hCTools,ICurveCtl *pCCtl);
		BOOL GetToolTip(int iButton, TSTR &ToolTip,ICurveCtl *pCCtl);
		void ResetCallback(int curvenum, ICurveCtl *pCCtl);
		void NewCurveCreatedCallback(int curvenum,ICurveCtl *pCCtl);
		

	private:
		ICurveCtl *mpCCtl;
		ISpinnerControl *iXMin;
		ISpinnerControl *iXMax;
		ISpinnerControl *iXZoom;
		ISpinnerControl *iYZoom;
		ISpinnerControl *iXScroll;
		ISpinnerControl *iYScroll;
		IColorSwatch    *iColor;
};


static CCUtil theCCUtil;

class CCUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return &theCCUtil;}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return CCUTIL_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static CCUtilClassDesc CCUtilDesc;
ClassDesc* GetCCUtilDesc() {return &CCUtilDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void CCUtilClassDesc::ResetClassParams (BOOL fileReset) 
{
	if(theCCUtil.mpCCtl)
	{
		theCCUtil.mpCCtl->SetActive(FALSE);
		theCCUtil.mpCCtl->DeleteThis();
		theCCUtil.mpCCtl = NULL;
	}

}

static DummyRefMaker theDummyRefMaker;

void *DummyRefMaker::GetInterface(ULONG id)
{
	if(id == I_RESMAKER_INTERFACE)
		return (void *) (ResourceMakerCallback *) &theCCUtil;
	else
		return (void *) NULL;
}


static INT_PTR CALLBACK CCUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	if(msg == WM_CC_CHANGE_CURVEPT)
	{
		ICurve *pCurve = (ICurve *) lParam;
		int index = LOWORD(wParam);
		theCCUtil.SetSwatchColor(pCurve,index);
	}

	if(msg == WM_CC_CHANGE_CURVETANGENT)
	{
		ICurve *pCurve = (ICurve *) lParam;
		int index = LOWORD(wParam);
		BOOL in = HIWORD(wParam)&IN_CURVETANGENT_CHANGED;
		BOOL out = HIWORD(wParam)&OUT_CURVETANGENT_CHANGED;
		theCCUtil.SetSwatchColor(pCurve,index);
	}

	if (((msg==CC_SPINNER_BUTTONUP) && HIWORD(wParam)) ||
		((msg==CC_SPINNER_CHANGE) ))// && (!HIWORD(wParam)))) 
	{
		ISpinnerControl *spin;
		spin = (ISpinnerControl *) lParam;
		
		switch (LOWORD(wParam)) 
		{
		case IDC_XMIN_SPIN:
			if ((msg == CC_SPINNER_CHANGE))// || (!HIWORD(wParam))) 
			{
				theCCUtil.xMin = spin->GetFVal();
				if(theCCUtil.xMin >= theCCUtil.xMax)
				{
					theCCUtil.xMin = theCCUtil.xMax-1.0f;
					spin->SetValue(theCCUtil.xMin,FALSE);
				}
				theCCUtil.SetXRange();
			}
			break;
		case IDC_XMAX_SPIN:
			if ((msg == CC_SPINNER_CHANGE))// || (!HIWORD(wParam))) 
			{
				theCCUtil.xMax = spin->GetFVal();
				if(theCCUtil.xMin >= theCCUtil.xMax)
				{
					theCCUtil.xMax = theCCUtil.xMin+1.0f;
					spin->SetValue(theCCUtil.xMax,FALSE);
				}
				theCCUtil.SetXRange();
			}
			break;
		case IDC_XZOOM_SPIN:
			if ((msg == CC_SPINNER_CHANGE))// || (!HIWORD(wParam))) 
			{
				theCCUtil.xZoom = spin->GetFVal();
				theCCUtil.SetZoom();
			}
			break;
		case IDC_YZOOM_SPIN:
			if ((msg == CC_SPINNER_CHANGE))// || (!HIWORD(wParam))) 
			{
				theCCUtil.yZoom = spin->GetFVal();
				theCCUtil.SetZoom();
			}

			break;
		case IDC_XSCROLL_SPIN:
			if ((msg == CC_SPINNER_CHANGE))// || (!HIWORD(wParam))) 
			{
				theCCUtil.xScroll = spin->GetIVal();
				theCCUtil.SetScroll();
			}
			break;
		case IDC_YSCROLL_SPIN:
			if ((msg == CC_SPINNER_CHANGE))// || (!HIWORD(wParam))) 
			{
				theCCUtil.yScroll = spin->GetIVal();
				theCCUtil.SetScroll();
			}

			break;
		}
	}		
	switch (msg) {
		case WM_INITDIALOG:
			theCCUtil.Init(hWnd);
			break;

		case WM_DESTROY:
			theCCUtil.Destroy(hWnd);
			break;

		case WM_COMMAND:
				switch (LOWORD(wParam)) 
				{
				case IDC_CC :
					theCCUtil.ShowCurveControl();
					break;
				case IDC_PLUS :
					theCCUtil.ChangeCurveNumbers(TRUE);
					break;
				case IDC_MINUS :
					theCCUtil.ChangeCurveNumbers(FALSE);
					break;
				case IDC_SETCOLOR :
					theCCUtil.SetColor(TRUE);
					break;
				case IDC_SETDCOLOR :
					theCCUtil.SetColor(FALSE);
					break;
				case IDC_GETZOOM :
					theCCUtil.GetZoom();
					break;
				case IDC_GETSCROLL :
					theCCUtil.GetScroll();
					break;
				case IDC_SETDISP :
					theCCUtil.SetDisplay();
					break;
				case IDC_GETVAL :
					theCCUtil.GetCurveValues(TRUE);
					break;
				case IDC_GETDISABLED:
					theCCUtil.GetCurveValues(FALSE);
					break;
				case IDC_SETANIMATED :
					theCCUtil.SetAnimated();
					break;
				case IDC_CMD_SET :
					theCCUtil.SetCommand();
					break;
				case IDC_PT_GET:
					theCCUtil.GetPtInfo();
					break;
				case IDC_PT_SET:
					theCCUtil.SetPt(FALSE);
					break;
				case IDC_PT_INSERT:
					theCCUtil.SetPt(TRUE);
					break;
				case IDC_PT_DELETE:
					theCCUtil.DelPt();
					break;
				case IDC_GET_PLACEMENT:
					theCCUtil.GetPlacement();
					break;
				case IDC_SET_PLACEMENT:
					theCCUtil.SetPlacement();
					break;
				case IDC_DRAWBG:
				case IDC_DRAWGRID:
				case IDC_DRAWUTOOLBAR:
				case IDC_SHOWRESET:
				case IDC_DRAWLTOOLBAR:
				case IDC_DRAWSCROLLBARS:
				case IDC_AUTOSCROLL:
				case IDC_DRAWRULER:
				case IDC_ASPOPUP:
				case IDC_CONSTRAIN_Y:	
				case IDC_HIDE_DISABLED_CURVES:
				case IDC_RCMENU_MOVE_XY:
				case IDC_RCMENU_MOVE_X:
				case IDC_RCMENU_MOVE_Y:
				case IDC_RCMENU_SCALE:
				case IDC_RCMENU_INSERT_CORNER:
				case IDC_RCMENU_INSERT_BEZIER:
				case IDC_RCMENU_DELETE:
					theCCUtil.SetFlags();
					break;
				case IDC_SC_CURVENUMBER:
				case IDC_SC_R:
				case IDC_SC_G:
				case IDC_SC_B:
				case IDC_DISPEDIT:
				case IDC_CMD_EDIT:
				case IDC_PT_INDEX:
				case IDC_PT_POS_X:
				case IDC_PT_POS_Y:
				case IDC_PT_IN_X:
				case IDC_PT_IN_Y:
				case IDC_PT_OUT_X:
				case IDC_PT_OUT_Y:
				case IDC_SIZE_X:
				case IDC_SIZE_Y:
				case IDC_POS_X:
				case IDC_POS_Y:
					if(HIWORD(wParam) == EN_SETFOCUS)
						DisableAccelerators();
					// Otherwise enable them again
					else if(HIWORD(wParam) == EN_KILLFOCUS) {
						EnableAccelerators();
					}
					break;					
				}
			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theCCUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}



//--- CCUtil -------------------------------------------------------
CCUtil::CCUtil()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
	mpCCtl = NULL;
	iXMin = NULL;
	iXMax = NULL;
	xMin = 0;
	xMax = 1;
	xScroll = -24;
	yScroll = -40;
	xZoom = 492.727f;
	yZoom = 344.643f;
}

CCUtil::~CCUtil()
{

}

void CCUtil::BeginEditParams(Interface *ip,IUtil *iu) 
{
	//RegisterCurveCtlWindow();
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		CCUtilDlgProc,
		GetString(IDS_PARAMS),
		0);
	
	iXMin = SetupIntSpinner (hPanel, IDC_XMIN_SPIN, IDC_XMIN, -10000, 10000, (int) xMin);
	iXMax = SetupIntSpinner (hPanel, IDC_XMAX_SPIN, IDC_XMAX, -10000, 10000, (int) xMax);

	iXZoom = SetupFloatSpinner(hPanel, IDC_XZOOM_SPIN, IDC_XZOOM, 0.00001f, 10000.0f, xZoom );
	iYZoom = SetupFloatSpinner(hPanel, IDC_YZOOM_SPIN, IDC_YZOOM, 0.00001f, 10000.0f, yZoom );
	iXScroll = SetupIntSpinner(hPanel, IDC_XSCROLL_SPIN, IDC_XSCROLL, -10000, 10000, xScroll );
	iYScroll = SetupIntSpinner(hPanel, IDC_YSCROLL_SPIN, IDC_YSCROLL, -10000, 10000, yScroll );
	iColor = GetIColorSwatch(GetDlgItem(hPanel, IDC_COLOR), 	RGB(255,255,255), _T("Curve Color"));
	

}
	
void CCUtil::EndEditParams(Interface *ip,IUtil *iu) 
{

	ReleaseISpinner (iXMin);
	ReleaseISpinner (iXMax);
	ReleaseISpinner (iXZoom);
	ReleaseISpinner (iYZoom);
	ReleaseISpinner (iXScroll);
	ReleaseISpinner (iYScroll);
	ReleaseIColorSwatch (iColor);

	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void CCUtil::Init(HWND hPanel)
{
	SetCheckBox(hPanel, IDC_DRAWBG,1);
	SetCheckBox(hPanel, IDC_DRAWGRID,1);
	SetCheckBox(hPanel, IDC_DRAWUTOOLBAR,1);
	SetCheckBox(hPanel, IDC_SHOWRESET,1);
	SetCheckBox(hPanel, IDC_DRAWLTOOLBAR,1);
	SetCheckBox(hPanel, IDC_DRAWSCROLLBARS,1);
	SetCheckBox(hPanel, IDC_AUTOSCROLL,1);
	SetCheckBox(hPanel, IDC_DRAWRULER,1);
	SetCheckBox(hPanel, IDC_ASPOPUP,1);
	SetCheckBox(hPanel, IDC_CONSTRAIN_Y,1);
	SetCheckBox(hPanel, IDC_HIDE_DISABLED_CURVES,1);
	
	SetCheckBox(hPanel, IDC_RCMENU_MOVE_XY,1);
	SetCheckBox(hPanel, IDC_RCMENU_MOVE_X,1);
	SetCheckBox(hPanel, IDC_RCMENU_MOVE_Y,1);
	SetCheckBox(hPanel, IDC_RCMENU_SCALE,1);
	SetCheckBox(hPanel, IDC_RCMENU_INSERT_CORNER,1);
	SetCheckBox(hPanel, IDC_RCMENU_INSERT_BEZIER,1);
	SetCheckBox(hPanel, IDC_RCMENU_DELETE,1);
	
}

void CCUtil::Destroy(HWND hWnd)
{

}

void CCUtil::ShowCurveControl()
{

	ICurve *pCurve = NULL;
	if(!mpCCtl)
	{
		mpCCtl = (ICurveCtl *) CreateInstance(REF_MAKER_CLASS_ID,CURVE_CONTROL_CLASS_ID);
		
		if(!mpCCtl)
			return;
	}

	mpCCtl->SetXRange(xMin,xMax);
	mpCCtl->SetYRange(0.0f,1.0f);

	mpCCtl->RegisterResourceMaker(&theDummyRefMaker);
	mpCCtl->SetNumCurves(4);
	
	LoadCurveControlResources();
	SetZoom();
	SetScroll();
	mpCCtl->SetTitle("Test Control Curve");
	
	BitArray ba;
	ba.SetSize(6);
	ba.Set(0);//ba.Set(0);ba.Set(0);ba.Set(3);
	mpCCtl->SetDisplayMode(ba);
	

	SetFlags();
	
	mpCCtl->SetCustomParentWnd(GetDlgItem(hPanel, IDC_CURVE));
	
	mpCCtl->SetMessageSink(hPanel);
	
	if(mpCCtl->IsActive())
		mpCCtl->SetActive(FALSE);
	else
		mpCCtl->SetActive(TRUE);
}



void CCUtil::ChangeCurveNumbers(BOOL inc)
{	
	int i = mpCCtl->GetNumCurves();
	
	if(inc)
	{
		mpCCtl->SetNumCurves(i+1);
	}
	else
		if(i > 0)
			mpCCtl->SetNumCurves(i-1);
}

void CCUtil::SetColor(BOOL Active)
{
	int r,g,b, style, width;

	GetDlgItemText(hPanel, IDC_SC_R, buf, 10); r = (int)atoi(buf);
	GetDlgItemText(hPanel, IDC_SC_G, buf, 10); g = (int)atoi(buf);
	GetDlgItemText(hPanel, IDC_SC_B, buf, 10); b = (int)atoi(buf);
	GetDlgItemText(hPanel, IDC_STYLE, buf, 10); style = (int)atoi(buf);
	GetDlgItemText(hPanel, IDC_WIDTH, buf, 10); width = (int)atoi(buf);


	ICurve *pCurve = mpCCtl->GetControlCurve(GetCurveNum());

	if(pCurve)
		if(Active)
			pCurve->SetPenProperty(RGB(r,g,b),width, style);
		else
			pCurve->SetDisabledPenProperty(RGB(r,g,b),width,style);

}

void CCUtil::SetXRange()
{
	mpCCtl->SetXRange(xMin, xMax);
}

void CCUtil::SetZoom()
{
	mpCCtl->SetZoomValues(xZoom,yZoom);
}

void CCUtil::GetZoom()
{
	mpCCtl->GetZoomValues(&xZoom,&yZoom);
	iXZoom->SetValue(xZoom,FALSE);
	iYZoom->SetValue(yZoom,FALSE);

}

void CCUtil::SetScroll()
{
	mpCCtl->SetScrollValues(xScroll,yScroll);
}

void CCUtil::GetScroll()
{
	mpCCtl->GetScrollValues(&xScroll,&yScroll);
	iXScroll->SetValue(xScroll,FALSE);
	iYScroll->SetValue(yScroll,FALSE);

}

void CCUtil::SetDisplay()
{

	long lDisp;

	GetDlgItemText(hPanel, IDC_DISPEDIT, buf, 10); lDisp = atol(buf);
	BitArray Disp(sizeof(long)*8);
	
	for(int i= 0 ; i < Disp.GetSize(); i++)
	{
		if( (lDisp>>i & 1)	)
			Disp.Set(i);
	}
	
	mpCCtl->SetDisplayMode(Disp);
}

int CCUtil::GetCurveNum()
{
	GetDlgItemText(hPanel, IDC_SC_CURVENUMBER, buf, 10); 
	return ((int)atoi(buf));
}


void CCUtil::GetCurveValues(BOOL Active)
{
	ICurve *pCurve = mpCCtl->GetControlCurve(GetCurveNum());

	if(pCurve)
	{
		int r,g,b;
		int width, style;
		COLORREF color;

		if(Active)
			pCurve->GetPenProperty(color,width,style);
		else
			pCurve->GetDisabledPenProperty(color,width,style);

		BOOL isAnimated = pCurve->GetCanBeAnimated();
		
		r = GetRValue(color);
		g = GetGValue(color);
		b = GetBValue(color);

		sprintf(buf,"%d",r);
		SetDlgItemText(hPanel, IDC_SC_R, buf); 
		sprintf(buf,"%d",g);
		SetDlgItemText(hPanel, IDC_SC_G, buf); 
		sprintf(buf,"%d",b);
		SetDlgItemText(hPanel, IDC_SC_B, buf); 
		
		SetCheckBox(hPanel, IDC_SETANIMATED, isAnimated);
	}

}

void CCUtil::SetAnimated()
{
	ICurve *pCurve = mpCCtl->GetControlCurve(GetCurveNum());

	if(pCurve)
		pCurve->SetCanBeAnimated(GetCheckBox(hPanel, IDC_SETANIMATED));

}

void CCUtil::SetFlags()
{
	DWORD flags = CC_NONE;
	
	flags |= GetCheckBox(hPanel, IDC_DRAWBG) ? CC_DRAWBG : 0;
	flags |= GetCheckBox(hPanel, IDC_DRAWGRID) ? CC_DRAWGRID : 0;
	flags |= GetCheckBox(hPanel, IDC_DRAWUTOOLBAR) ? CC_DRAWUTOOLBAR : 0;
	flags |= GetCheckBox(hPanel, IDC_DRAWLTOOLBAR) ? CC_DRAWLTOOLBAR : 0;
	flags |= GetCheckBox(hPanel, IDC_SHOWRESET) ? CC_SHOWRESET : 0;
	flags |= GetCheckBox(hPanel, IDC_DRAWSCROLLBARS) ? CC_DRAWSCROLLBARS : 0;
	flags |= GetCheckBox(hPanel, IDC_AUTOSCROLL) ? CC_AUTOSCROLL : 0;
	flags |= GetCheckBox(hPanel, IDC_DRAWRULER) ? CC_DRAWRULER : 0;
	flags |= GetCheckBox(hPanel, IDC_ASPOPUP) ? CC_ASPOPUP : 0;
	flags |= GetCheckBox(hPanel, IDC_CONSTRAIN_Y) ? CC_CONSTRAIN_Y : 0;
	flags |= GetCheckBox(hPanel, IDC_HIDE_DISABLED_CURVES) ? CC_HIDE_DISABLED_CURVES : 0;
	
	
	flags |= GetCheckBox(hPanel, IDC_RCMENU_MOVE_XY) ? CC_RCMENU_MOVE_XY : 0;
	flags |= GetCheckBox(hPanel, IDC_RCMENU_MOVE_X) ? CC_RCMENU_MOVE_X : 0;
	flags |= GetCheckBox(hPanel, IDC_RCMENU_MOVE_Y) ? CC_RCMENU_MOVE_Y : 0;
	flags |= GetCheckBox(hPanel, IDC_RCMENU_SCALE) ? CC_RCMENU_SCALE : 0;
	flags |= GetCheckBox(hPanel, IDC_RCMENU_INSERT_CORNER) ? CC_RCMENU_INSERT_CORNER : 0;
	flags |= GetCheckBox(hPanel, IDC_RCMENU_INSERT_BEZIER) ? CC_RCMENU_INSERT_BEZIER : 0;
	flags |= GetCheckBox(hPanel, IDC_RCMENU_DELETE) ? CC_RCMENU_DELETE : 0;

	if(mpCCtl) mpCCtl->SetCCFlags(flags);

}

void CCUtil::SetCommand()
{
	
	GetDlgItemText(hPanel, IDC_CMD_EDIT, buf, 10); 
	int id = (int)atoi(buf);
	if(mpCCtl) mpCCtl->SetCommandMode(id);
}

void CCUtil::GetPtInfo()
{
	ICurve *pCurve = mpCCtl->GetControlCurve(GetCurveNum());
	
	int idx=0;

	if(pCurve)
	{
		sprintf(buf,"%d",pCurve->GetNumPts());
		SetDlgItemText(hPanel, IDC_PT_NUM, buf); 
		
		GetDlgItemText(hPanel, IDC_PT_INDEX, buf, 10); 
		idx = (int)atoi(buf);

		if(idx >= 0 && idx < pCurve->GetNumPts())
		{
			CurvePoint pt = pCurve->GetPoint(ip->GetTime(),idx);
			sprintf(buf,"%.7f",pt.p.x);
			SetDlgItemText(hPanel, IDC_PT_POS_X, buf); 
			sprintf(buf,"%.7f",pt.p.y);
			SetDlgItemText(hPanel, IDC_PT_POS_Y, buf); 

			sprintf(buf,"%.7f",pt.in.x);
			SetDlgItemText(hPanel, IDC_PT_IN_X, buf); 
			sprintf(buf,"%.7f",pt.in.y);
			SetDlgItemText(hPanel, IDC_PT_IN_Y, buf); 

			sprintf(buf,"%.7f",pt.out.x);
			SetDlgItemText(hPanel, IDC_PT_OUT_X, buf); 
			sprintf(buf,"%.7f",pt.out.y);
			SetDlgItemText(hPanel, IDC_PT_OUT_Y, buf);
			
			SetCheckBox(hPanel, IDC_CORNER, pt.flags & CURVEP_CORNER );	
			SetCheckBox(hPanel, IDC_BEZIER, pt.flags & CURVEP_BEZIER );
			SetCheckBox(hPanel, IDC_LOCKED_X, pt.flags & CURVEP_LOCKED_X);
			SetCheckBox(hPanel, IDC_LOCKED_Y, pt.flags & CURVEP_LOCKED_Y);
			

		}
	}

}

void CCUtil::ReadPtVals(Point2 &in, Point2 &out, Point2 &pos, int &flags, int &index)
{

	GetDlgItemText(hPanel, IDC_PT_POS_X, buf, 10); 
	pos.x = (float)atof(buf);
	GetDlgItemText(hPanel, IDC_PT_POS_Y, buf, 10); 
	pos.y = (float)atof(buf);

	GetDlgItemText(hPanel, IDC_PT_IN_X, buf, 10); 
	in.x = (float)atof(buf);
	GetDlgItemText(hPanel, IDC_PT_IN_Y, buf, 10); 
	in.y = (float)atof(buf);

	GetDlgItemText(hPanel, IDC_PT_OUT_X, buf, 10); 
	out.x = (float)atof(buf);
	GetDlgItemText(hPanel, IDC_PT_OUT_Y, buf, 10); 
	out.y = (float)atof(buf);

	GetDlgItemText(hPanel, IDC_PT_INDEX, buf, 10); 
	index = (int)atof(buf);
			
	flags |= GetCheckBox(hPanel, IDC_CORNER) ? CURVEP_CORNER : 0;
	flags |= GetCheckBox(hPanel, IDC_BEZIER) ? CURVEP_BEZIER : 0;
	flags |= GetCheckBox(hPanel, IDC_LOCKED_X) ? CURVEP_LOCKED_X : 0;
	flags |= GetCheckBox(hPanel, IDC_LOCKED_Y) ? CURVEP_LOCKED_Y : 0;	
}

void CCUtil::SetPt(BOOL insert)
{
	Point2 in, out, pos;
	int idx, flags = 0;
	ReadPtVals(in,out,pos,flags,idx);
	ICurve *pCurve = NULL;
	
	if(mpCCtl)
		pCurve = mpCCtl->GetControlCurve(GetCurveNum());

	if(pCurve)
	{
		if(idx >= 0 && idx < pCurve->GetNumPts())
		{
			CurvePoint pt;

			pt.p = pos;
			pt.in = in;
			pt.out = out;
			pt.flags = flags;
			
			if(insert)
				pCurve->Insert(idx,pt);
			else
				pCurve->SetPoint(ip->GetTime(),idx,&pt);

			mpCCtl->Redraw();
		}
	}
}

void CCUtil::DelPt()
{
	ICurve *pCurve = NULL;

	if(mpCCtl)
		pCurve = mpCCtl->GetControlCurve(GetCurveNum());
	
	if(pCurve)
	{
		GetDlgItemText(hPanel, IDC_PT_INDEX, buf, 10); 
		int index = (int)atof(buf);
		if(index >= 0 && index < pCurve->GetNumPts() )
		{
			pCurve->Delete(index);
			mpCCtl->Redraw();
		}
	}
}

void CCUtil::GetPlacement()
{
	
	if(mpCCtl)
	{
		HWND hCurve = mpCCtl->GetHWND();
		if(hCurve)
		{
			RECT rect;
			GetWindowRect(hCurve,&rect);
		
			sprintf(buf,"%d",rect.top);
			SetDlgItemText(hPanel, IDC_POS_X, buf); 
	
			sprintf(buf,"%d",rect.left);
			SetDlgItemText(hPanel, IDC_POS_Y, buf); 

			sprintf(buf,"%d",rect.bottom-rect.top);
			SetDlgItemText(hPanel, IDC_SIZE_Y, buf); 

			sprintf(buf,"%d",rect.right-rect.left);
			SetDlgItemText(hPanel, IDC_SIZE_X, buf); 
			
		}
	}
}

void CCUtil::SetPlacement()
{
	if(mpCCtl && mpCCtl->GetCCFlags() & CC_ASPOPUP)
	{
		HWND hCurve = mpCCtl->GetHWND();
		if(hCurve)
		{
			int x,y,w,h;
			GetDlgItemText(hPanel,IDC_POS_X,buf,80);
			x = atoi(buf);
			GetDlgItemText(hPanel,IDC_POS_Y,buf,80);
			y = atoi(buf);
			GetDlgItemText(hPanel,IDC_SIZE_X,buf,80);
			w = atoi(buf);
			GetDlgItemText(hPanel,IDC_SIZE_Y,buf,80);
			h = atoi(buf);

			SetWindowPos(hCurve, HWND_TOP,x,y,w,h,SWP_SHOWWINDOW);
		}
	}
}
void CCUtil::SetSwatchColor(ICurve *pCurve,int index)
{
	Interval valid;
	if(mpCCtl && mpCCtl->GetNumCurves() >= 3 )
	{
		int col[3];
		CurvePoint cp = pCurve->GetPoint(ip->GetTime(), index);
		for(int i = 0 ; i < 3; i ++ )
		{
		col[i] = int (mpCCtl->GetControlCurve(i)->GetValue(ip->GetTime(),cp.p.x,valid,TRUE)*255);
		}
		iColor->SetColor(RGB(col[0],col[1],col[2]));
	}
}

//***************************************************************************
//**
//** ResourceMakerCallback implementation
//**
//***************************************************************************

BOOL CCUtil::SetCustomImageList(HIMAGELIST &hCTools,ICurveCtl *pCCtl)
{
	LoadCurveControlResources();
	hCTools = hTools;
	return TRUE;
}

BOOL CCUtil::GetToolTip(int iButton, TSTR &ToolTip,ICurveCtl *pCCtl)
{
	switch(iButton)
	{
		case 0:ToolTip = _T("Red Curve On/Off Toggle");break;
		case 1:ToolTip = _T("Green Curve On/Off Toggle");break;
		case 2:ToolTip = _T("Blue Curve On/Off Toggle");break;
		default:
			ToolTip = _T("Visibility On/Off Toggle");break;
	}
	return TRUE;
}

void CCUtil::ResetCallback(int curvenum,ICurveCtl *pCCtl)
{
	ICurve *pCurve = NULL;

	pCurve = mpCCtl->GetControlCurve(curvenum);
	if(pCurve)
	{
		pCurve->SetNumPts(2);
		NewCurveCreatedCallback(curvenum, pCCtl);
	}

}
void CCUtil::NewCurveCreatedCallback(int curvenum,ICurveCtl *pCCtl)
{
	float df = curvenum/10.0f;

	ICurve *pCurve = NULL;
	
	if(mpCCtl)
		pCurve = mpCCtl->GetControlCurve(curvenum);

	if(pCurve)
	{
		CurvePoint pt = pCurve->GetPoint(ip->GetTime(),0);
		pt.p.y = df;			
		pCurve->SetPoint(ip->GetTime(),0,&pt);
		
		pt = pCurve->GetPoint(ip->GetTime(),1);
		pt.p.y = 0.1f+df;			
		pCurve->SetPoint(ip->GetTime(),1,&pt);
		pCurve->SetLookupTableSize(100);

		switch(curvenum)
		{
		case 0:
			pCurve->SetPenProperty( RGB(255,0,0) );
			pCurve->SetDisabledPenProperty( RGB(255,180,180) );
			break;
		case 1:
			pCurve->SetPenProperty( RGB(0,128,0) );
			pCurve->SetDisabledPenProperty( RGB(190,200,190) );
			break;
		case 2:
			pCurve->SetPenProperty( RGB(0,0,255));
			pCurve->SetDisabledPenProperty( RGB(200,200,255));
			break;
		default:
			pCurve->SetPenProperty( RGB(0,0,0));
			pCurve->SetDisabledPenProperty( RGB(128,128,128));
		}
		
	}			
}

static void LoadCurveControlResources()
{

	HBITMAP hBitmap, hMask;

	hTools = ImageList_Create(16, 15, TRUE, 29, 0);
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DISPLAYRGB));
	hMask   = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MASK_DISPLAYRGB));
	ImageList_Add(hTools,hBitmap,hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);

}



