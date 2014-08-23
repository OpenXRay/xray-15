/**********************************************************************
 *<
	FILE: twist.cpp

	DESCRIPTION:  Twist OSM

	CREATED BY: Dan Silva & Rolf Berteig
	MODIFIED BY: Keith Trummel

	HISTORY: created 30 Jauary, 1995
	         Keith modified bend.cpp to create this on 13 March, 1995
		 updated for new API on 24 April, 1995
		 updated and added authorization code 11 May, 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#include "mods.h"

#ifndef NO_MODIFIER_TWIST // JP Morel - June 28th 2002

#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include <stdio.h>

#ifdef CHECK_AUTHORIZE
#define TwistMod		TwistMod2
#define TwistDeformer	TwistDeformer2
#define AuthInfo		AuthInfo2
#define TwistClassDesc	TwistClassDesc2
#endif

//------------------------------------------------------
// Functions used by the rest of the library
//------------------------------------------------------
static int authorized = 0;

static int 
IsAuthorized()
{
    return authorized;
}

static void
Authorize()
{
    authorized = 1;
}

static void
UnAuthorize()
{
    authorized = 0;
}
//////////////////////////////////////////////////////////


// in mods.cpp
extern HINSTANCE hInstance;

#define ABS(x)   (((x) >= 0) ? (x) : -(x))

#define BIGFLOAT	float(999999)



class TwistMod : public SimpleMod {
		static HWND hAuthWnd;
		static int inAuthorizationMode;
		static IParamMap *pmapParam;

	public:
		TwistMod();
		
		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_TWISTMOD); }  
		
	#ifdef CHECK_AUTHORIZE
		virtual Class_ID ClassID() { return Class_ID(TWISTOSM_CLASS_ID,1);}
	#else
		virtual Class_ID ClassID() { return Class_ID(TWISTOSM_CLASS_ID,0);}
	#endif
		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
	
	#ifdef CHECK_AUTHORIZE
		TCHAR *GetObjectName() { return GetString(IDS_RB_TWIST2); }
	#else
		TCHAR *GetObjectName() { return GetString(IDS_RB_TWIST); }
	#endif
				
		IOResult Load(ILoad *iload);

		// From simple mod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		BOOL GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis);
		void InvalidateUI() {if (pmapParam) pmapParam->Invalidate();}
	};

class TwistDeformer: public Deformer {
	public:
		Matrix3 tm,invtm;
		Box3 bbox;
		TimeValue time;
		float theAngle;
		float height;
		float angleOverHeight, from, to, bias;
		int doRegion, doBias;
		TwistDeformer();
		TwistDeformer(
			TimeValue t, ModContext &mc,
			float angle, int naxis, float bias, 
			float from, float to, int doRegion,
			Matrix3& modmat, Matrix3& modinv);
		void SetAxis(Matrix3 &tmAxis);
		void CalcHeight(int axis, float angle);
		Point3 Map(int i, Point3 p); 
	};


#define TWISTWSM_CLASSID	Class_ID(TWISTOSM_CLASS_ID,2)

class TwistWSM : public SimpleOSMToWSMObject {
	public:
		TwistWSM() {}
		TwistWSM(TwistMod *m) : SimpleOSMToWSMObject(m) {}
		void DeleteThis() { delete this; }
		SClass_ID SuperClassID() {return WSM_OBJECT_CLASS_ID;}
		Class_ID ClassID() {return TWISTWSM_CLASSID;} 
		TCHAR *GetObjectName() {return GetString(IDS_RB_TWIST);}
		RefTargetHandle Clone(RemapDir& remap)
			{TwistWSM *newobj = new TwistWSM((TwistMod*)mod->Clone(remap));
		newobj->SimpleOSMToWSMClone(this,remap);
		BaseClone(this, newobj, remap);
		return newobj;}
	};

//--- ClassDescriptor and class vars ---------------------------------


HWND TwistMod::hAuthWnd           = NULL;
int TwistMod::inAuthorizationMode = 0;
IParamMap *TwistMod::pmapParam     = NULL;

static INT_PTR CALLBACK AuthorizationDialogProc( HWND hDlg, UINT message, 
				       WPARAM wParam, LPARAM lParam );
static INT_PTR CALLBACK AuthRollupProc( HWND hDlg, UINT message, 
			      WPARAM wParam, LPARAM lParam );

class AuthInfo {
public:
    TwistMod    *twistMod;
    IObjParam   *iobjParam;
    int         create;
};


class TwistClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TwistMod; }	
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
#ifdef CHECK_AUTHORIZE	
	const TCHAR *	ClassName() { return GetString(IDS_RB_TWIST2_CLASS); }
	Class_ID		ClassID() { return Class_ID(TWISTOSM_CLASS_ID,1); }
#else
	const TCHAR *	ClassName() { return GetString(IDS_RB_TWIST_CLASS); }
	Class_ID		ClassID() { return Class_ID(TWISTOSM_CLASS_ID,0); }
#endif
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static TwistClassDesc twistDesc;
#ifndef CHECK_AUTHORIZE	
extern ClassDesc* GetTwistModDesc() { return &twistDesc; }
#else
extern ClassDesc* GetTwistModDesc2() { return &twistDesc; }
#endif


class TwistWSMClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) 
		{if (loading) return new TwistWSM; else return new TwistWSM(new TwistMod);}
	const TCHAR *	ClassName() { return GetString(IDS_RB_TWIST_CLASS); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return TWISTWSM_CLASSID; }
	const TCHAR* 	Category() {return GetSpaceWarpCatString(SPACEWARP_CAT_MODBASED);}
	};

static TwistWSMClassDesc twistWSMDesc;
extern ClassDesc* GetTwistWSMDesc() { return &twistWSMDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_ANGLE	0
#define PB_BIAS		1
#define PB_AXIS		2
#define PB_DOREGION	3
#define PB_FROM		4
#define PB_TO		5

//
//
// Parameters

static int axisIDs[] = {IDC_X,IDC_Y,IDC_Z};

static ParamUIDesc descParam[] = {
	// Angle
	ParamUIDesc(
		PB_ANGLE,
		EDITTYPE_FLOAT,
		IDC_ANGLE,IDC_ANGLESPINNER,
		-BIGFLOAT,BIGFLOAT,
		0.5f),
	
	// Bias
	ParamUIDesc(
		PB_BIAS,
		EDITTYPE_FLOAT,
		IDC_TWIST_BIAS,IDC_TWIST_BIASSPIN,
		-100.0f,100.0f,
		0.5f),
	
	// Axis
	ParamUIDesc(PB_AXIS,TYPE_RADIO,axisIDs,3),

	// Affect region
	ParamUIDesc(PB_DOREGION,TYPE_SINGLECHEKBOX,IDC_TWIST_AFFECTREGION),

	// From
	ParamUIDesc(
		PB_FROM,
		EDITTYPE_UNIVERSE,
		IDC_TWIST_FROM,IDC_TWIST_FROMSPIN,
		-BIGFLOAT,BIGFLOAT,
		SPIN_AUTOSCALE),

	// To
	ParamUIDesc(
		PB_TO,
		EDITTYPE_UNIVERSE,
		IDC_TWIST_TO,IDC_TWIST_TOSPIN,
		-BIGFLOAT,BIGFLOAT,		
		SPIN_AUTOSCALE),	
	};
#define PARAMDESC_LENGH 6

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_INT, NULL, FALSE, 1 },
	{ TYPE_INT, NULL, FALSE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 } };

#define PBLOCK_LENGTH	6

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,2,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- TwistDlgProc -------------------------------



class TwistDlgProc : public ParamMapUserDlgProc {
	public:
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {}
	};
static TwistDlgProc theTwistProc;

#ifndef CHECK_AUTHORIZE	
BOOL TwistDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_TWIST_FROMSPIN: {
					float from, to;
					map->GetParamBlock()->GetValue(PB_FROM,t,from,FOREVER);
					map->GetParamBlock()->GetValue(PB_TO,t,to,FOREVER);
					if (from>to) {
						map->GetParamBlock()->SetValue(PB_TO,t,from);
						map->Invalidate();
						}
					break;
					}
				
				case IDC_TWIST_TOSPIN: {
					float from, to;
					map->GetParamBlock()->GetValue(PB_FROM,t,from,FOREVER);
					map->GetParamBlock()->GetValue(PB_TO,t,to,FOREVER);
					if (from>to) {
						map->GetParamBlock()->SetValue(PB_FROM,t,to);
						map->Invalidate();
						}
					break;
					}
				}
			break;
		}
	return FALSE;
	}
#endif

//--- Twist methods -------------------------------


TwistMod::TwistMod() : SimpleMod()
	{	
	MakeRefByID(FOREVER, SIMPMOD_PBLOCKREF, 
		CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue( PB_AXIS, TimeValue(0), 2/*Z*/ );
	}

IOResult TwistMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,SIMPMOD_PBLOCKREF));
	return IO_OK;
	}


void TwistMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
        static AuthInfo authInfo;

#ifdef CHECK_AUTHORIZE        
   	if (!IsAuthorized()) {
	    authInfo.twistMod = this;
	    authInfo.iobjParam = ip;
	    authInfo.create = flags&BEGIN_EDIT_CREATE;
	    hAuthWnd = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_AUTHMSG),
			AuthRollupProc,
			GetString(IDS_RB_TWISTPLUGIN),
			(LPARAM)&authInfo );	    
	    inAuthorizationMode = 1;
	    return;
		}
#endif

	inAuthorizationMode = 0;

	SimpleMod::BeginEditParams(ip,flags,prev);
		
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_TWISTPARAM),
		GetString(IDS_RB_PARAMETERS),
		0);	
	pmapParam->SetUserDlgProc(&theTwistProc);
	}

		
void TwistMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	if (inAuthorizationMode) {
	    if (flags&END_EDIT_REMOVEUI) {
			if (hAuthWnd) {		    	
		    	ip->DeleteRollupPage(hAuthWnd);
		    	hAuthWnd = NULL;
				}		
	    	}
	    return;
		}

	SimpleMod::EndEditParams(ip,flags,next);
	DestroyCPParamMap(pmapParam);
	}

Interval TwistMod::GetValidity(TimeValue t)
	{
	float f;
	Interval valid = FOREVER;
	pblock->GetValue(PB_ANGLE,t,f,valid);	
	pblock->GetValue(PB_BIAS,t,f,valid);
	pblock->GetValue(PB_FROM,t,f,valid);
	pblock->GetValue(PB_TO,t,f,valid);
	return valid;
	}

BOOL TwistMod::GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis)
	{
	int limit;
	pblock->GetValue(PB_DOREGION,t,limit,FOREVER);
	pblock->GetValue(PB_FROM,t,zmin,FOREVER);
	pblock->GetValue(PB_TO,t,zmax,FOREVER);
	pblock->GetValue(PB_AXIS,t,axis,FOREVER);
	return limit?TRUE:FALSE;
	}

RefTargetHandle TwistMod::Clone(RemapDir& remap) 
	{	
	TwistMod* newmod = new TwistMod();
	newmod->ReplaceReference(SIMPMOD_PBLOCKREF,pblock->Clone());
	newmod->SimpleModClone(this);
	BaseClone(this, newmod, remap);
	return(newmod);
	}



TwistDeformer::TwistDeformer() 
	{ 
	tm.IdentityMatrix();
	time = 0;	
	}

TwistDeformer::TwistDeformer(
		TimeValue t, ModContext &mc,
		float angle, int naxis, float bias,
		float from, float to, int doRegion,
		Matrix3& modmat, Matrix3& modinv) 
	{	
	this->doRegion = doRegion;
	this->from = from;
	this->to   = to;
	if (bias!=0.0f) {
		this->bias = 1.0f-(bias+100.0f)/200.0f;
		if (this->bias < 0.00001f) this->bias = 0.00001f;
		if (this->bias > 0.99999f) this->bias = 0.99999f;
		this->bias = float(log(this->bias)/log(0.5));
		doBias = TRUE;
	} else {
		this->bias = 1.0f;
		doBias = FALSE;
		}
	
	Matrix3 mat;
	Interval valid;	
	time   = t;	

	tm = modmat;
	invtm = modinv;
	mat.IdentityMatrix();
	
	switch ( naxis ) {
		case 0: mat.RotateY( -HALFPI );	 break; //X
		case 1: mat.RotateX( HALFPI );  break; //Y
		case 2: break;  //Z
		}
	SetAxis( mat );	
	assert (mc.box);
	bbox = *mc.box;
	CalcHeight(naxis,DegToRad(angle));
	}

void TwistDeformer::SetAxis(Matrix3 &tmAxis)
	{
	Matrix3 itm = Inverse(tmAxis);
	tm    = tm*tmAxis;
	invtm =	itm*invtm;
	}

void TwistDeformer::CalcHeight(int axis, float angle)
	{
	switch ( axis ) {
		case 0:
			height = bbox.pmax.x - bbox.pmin.x;
			break;
		case 1:
			height = bbox.pmax.y - bbox.pmin.y;
			break;
		case 2:
			height = bbox.pmax.z - bbox.pmin.z;
			break;
		}
	if (height==0.0f) {
		theAngle = 0.0f;
		angleOverHeight = 0.0f;
	} else {
		theAngle = angle;
		angleOverHeight = angle / height;
		}
	}

Point3 TwistDeformer::Map(int i, Point3 p)
	{
	float x, y, z, cosine, sine, a;
	if (theAngle==0.0f) return p;
	p = p * tm;

	x = p.x;
	y = p.y;
	
	if (doRegion) {
		if (p.z<from) {
			z = from;
		} else 
		if (p.z>to) {
			z = to;
		} else {
			z = p.z;
			}
	} else {	
		z = p.z;
		}	
	
	if (doBias) {
		float u = z/height;
		a = theAngle * (float)pow(fabs(u), bias);
		if (u<0.0) a = -a;
	} else {
		a = z * angleOverHeight;
		}
	cosine = float(cos(a));
	sine = float(sin(a));
	p.x =  cosine*x + sine*y;
	p.y = -sine*x + cosine*y;

	p = p * invtm;
	return p;
	}


Deformer& TwistMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	float angle, from, to, bias;
	int axis;	
	int doRegion;
	static TwistDeformer deformer;
	pblock->GetValue(PB_ANGLE,t,angle,FOREVER);	
	pblock->GetValue(PB_AXIS,t,axis,FOREVER);
	pblock->GetValue(PB_FROM,t,from,FOREVER);
	pblock->GetValue(PB_TO,t,to,FOREVER);
	pblock->GetValue(PB_BIAS,t,bias,FOREVER);
	pblock->GetValue(PB_DOREGION,t,doRegion,FOREVER);
	deformer = TwistDeformer(t,mc,angle,axis,bias,from,to,doRegion,mat,invmat);
	return deformer;
	}

ParamDimension *TwistMod::GetParameterDim(int pbIndex)
	{
	switch (pbIndex) {
		case PB_ANGLE: 	return defaultDim; 		
		case PB_FROM:	return stdWorldDim;
		case PB_TO:		return stdWorldDim;
		case PB_BIAS:	return defaultDim;
		default:		return defaultDim;
		}
	}

TSTR TwistMod::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_ANGLE:	return GetString(IDS_RB_ANGLE);
		case PB_FROM:	return GetString(IDS_RB_FROM);
		case PB_TO:		return GetString(IDS_RB_TO);
		case PB_BIAS:	return GetString(IDS_RB_BIAS);
		default:		return TSTR(_T(""));
		}
	}



//--- Authorization Code -------------------------------------------


static void OutputLockID (HWND hWnd, unsigned int lockID)
{
    char string[10];

    PAINTSTRUCT ps;
    HDC hDC;
    InvalidateRect (hWnd, NULL, TRUE);
    hDC = BeginPaint (hWnd, &ps);
    SetBkMode (hDC, TRANSPARENT);
    sprintf (string, "%0x", lockID);
    TextOut (hDC, 0, 0, string, strlen(string));
    EndPaint (hWnd, &ps);
}


static INT_PTR CALLBACK AuthRollupProc( HWND hDlg, UINT message, 
			      WPARAM wParam, LPARAM lParam )
{
    AuthInfo *authInfo = (AuthInfo *) GetWindowLongPtr (hDlg, GWLP_USERDATA);

    switch (message) {
    case WM_CREATE:
	break;

    case WM_INITDIALOG:
	authInfo = (AuthInfo *) lParam;
	SetWindowLongPtr (hDlg, GWLP_USERDATA, (LONG_PTR) authInfo);
	return TRUE;

    case WM_COMMAND:
	switch (wParam) {
	case IDC_AUTHORIZE:
	    DialogBox (hInstance, MAKEINTRESOURCE(IDD_AUTHORIZE),
		       hDlg, AuthorizationDialogProc);
	    if (IsAuthorized()) {
		authInfo->twistMod->EndEditParams (authInfo->iobjParam, BEGIN_EDIT_CREATE,NULL);
		authInfo->twistMod->BeginEditParams (authInfo->iobjParam, 
						     authInfo->create?BEGIN_EDIT_CREATE:0,NULL);
	    }
	    return TRUE;
	case IDCANCEL:
	    EndDialog (hDlg, 0);
	    return TRUE;
	}
    }
    return FALSE;
}

static INT_PTR CALLBACK AuthorizationDialogProc( HWND hDlg, UINT message, 
				       WPARAM wParam, LPARAM lParam )
{
#define AUTH_LENGTH 20
    static HWND hCtrlWin;
    static char authString[AUTH_LENGTH];
    static unsigned int lockID;
    unsigned int authNum;

    switch (message) {
    case WM_INITDIALOG:
	hCtrlWin = GetDlgItem (hDlg, IDC_LOCKID);
	SetDlgItemText (hDlg, IDC_EDIT2, authString);
	SetFocus (GetDlgItem (hDlg, IDC_EDIT2));
	lockID = HardwareLockID ();
	return FALSE;

    case WM_COMMAND:
	switch (wParam) {
	case IDC_AUTHORIZE:
	    GetDlgItemText (hDlg, IDC_EDIT2, authString, AUTH_LENGTH);
	    sscanf (authString, "%x", &authNum);
	    if (authNum != lockID) {
//		char buffer[50];
//		sprintf (buffer, "Incorrect authorization string %s %0x",
//			 authString, authNum);
		
		TSTR buf1 = GetString(IDS_RB_INCORRECTSTRING);
		TSTR buf2 = GetString(IDS_RB_AUTHORIZATION);
		MessageBox (NULL, buf1, buf2,MB_ICONINFORMATION);
	    }
	    else
		Authorize();
	    EndDialog (hDlg, 0);
	    return TRUE;
	case IDCANCEL:
	    EndDialog (hDlg, 0);
	    return TRUE;
	}

    case WM_PAINT:
	OutputLockID (hCtrlWin, lockID);
	break;

    }
    return FALSE;
}

#endif // NO_MODIFIER_TWIST 