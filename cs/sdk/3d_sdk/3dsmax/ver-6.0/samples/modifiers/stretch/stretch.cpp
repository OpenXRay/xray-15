/**********************************************************************
 *
 *  FILE: stretch.cpp
 *
 *  DESCRIPTION:   DLL implementation of stretch modifier
 *
 *  CREATED BY: Keith Trummel (modified bend.cpp)
 *
 *  (C) Copyright 1995-1996 by Autodesk, Inc.
 *
 *  This program is copyrighted by Autodesk, Inc. and is licensed to you under
 *  the following conditions.  You may not distribute or publish the source
 *  code of this program in any form.  You may incorporate this code in object
 *  form in derivative works provided such derivative works are (i.) are de-
 *  signed and intended to work solely with Autodesk, Inc. products, and (ii.)
 *  contain Autodesk's copyright notice "(C) Copyright 1994 by Autodesk, Inc."
 *
 *  AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.  AUTODESK SPE-
 *  CIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
 *  A PARTICULAR USE.  AUTODESK, INC.  DOES NOT WARRANT THAT THE OPERATION OF
 *  THE PROGRAM WILL BE UNINTERRUPTED OR ERROR FREE.
 *
 **********************************************************************/
//
// RB 1/14/97 -- Added WSM version
//

#include "max.h"
#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include "stretch.h"
#include "resource.h"

#define MAX_PATH_LENGTH 257
#define MAX_STRING_LENGTH  256

#define BIGFLOAT        float(999999)

#define PB_STRETCH      0
#define PB_AMPLIFY      1
#define PB_AXIS         2
#define PB_DOREGION     3
#define PB_FROM         4
#define PB_TO           5

HINSTANCE hInstance;
HINSTANCE hResource;
int controlsInit = FALSE;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
    hInstance = hinstDLL;
    hResource = hinstDLL;

    if ( !controlsInit ) {
        controlsInit = TRUE;
                
        // initialize Chicago controls
        InitCommonControls();
        
        InitCustomControls (hResource);
    }

    switch(fdwReason) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return(TRUE);
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() 
{ 
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_LIB_DESCRIPTION, stringBuf, 
                    MAX_STRING_LENGTH);
        #if 0
            static TCHAR tmpBuf[25];
            float vernum = 1.0f;
            char filename[MAX_PATH_LENGTH];
            DWORD size, dummy;

        	GetModuleFileName(hResource, filename, MAX_PATH);
        	size = GetFileVersionInfoSize(filename, &dummy);
        	if (size) {
        	    char *buf = (char *)malloc(size);
        	    GetFileVersionInfo(filename, NULL, size, buf);
        	    VS_FIXEDFILEINFO *qbuf;
        	    UINT len;
        	    if (VerQueryValue(buf, "\\", (void **)&qbuf, &len)) {
            		// got the version information
            		DWORD ms = qbuf->dwProductVersionMS;
            		// DWORD ls = qbuf->dwProductVersionLS;
            		vernum = HIWORD(ms) + (LOWORD(ms) / 100.0f);
        	    }
        	    free(buf);
        	}
        	sprintf (tmpBuf, "%.2f", vernum);
        	_tcscat (stringBuf, _T(tmpBuf));
        #endif
	
        loaded = 1;
    }

    return stringBuf;
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{ 
    return 2; 
}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) 
{
    switch(i) {
    case 0: return GetStretchModDesc();
	case 1: return GetStretchWSMDesc();
    default: return 0;
    }
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() 
{ 
    return VERSION_3DSMAX; 
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

class StretchMod: public SimpleMod {
    friend BOOL CALLBACK StretchParamDialogProc (HWND hDlg, UINT message, 
                                                 WPARAM wParam, LPARAM lParam);

public:
    static IParamMap *pmapParam;
    
    StretchMod();

    //  inherited virtual methods:

    // From Animatable
    void DeleteThis() { delete this; }
    void GetClassName(TSTR& s);
    virtual Class_ID ClassID() {return Class_ID(STRETCHOSM_CLASS_ID,12332321);}
    void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
    RefTargetHandle Clone(RemapDir& remap = NoRemap());
    TCHAR *GetObjectName ();

    // From simple mod
    Deformer& GetDeformer(TimeValue t, ModContext &mc, Matrix3& mat,
                          Matrix3& invmat);
    Interval GetValidity(TimeValue t);
    ParamDimension *GetParameterDim(int pbIndex);
    TSTR GetParameterName (int pbIndex);
    BOOL GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis);
    void InvalidateUI() {if (pmapParam) pmapParam->Invalidate();}
};


class StretchDeformer: public Deformer {
        public:
                Matrix3 tm,invtm;
                Box3 bbox;
                TimeValue time;
                float from, to;
                float amount;
                float amplifier;
                int theAxis;
                float heightMin;
                float heightMax;
                int doRegion;
                StretchDeformer();
                StretchDeformer(TimeValue t, ModContext &mc,float stretch, 
                                float amplify, int naxis, float from, 
                                float to, int doRegion, Matrix3& modmat, 
                                Matrix3& modinv);
                void SetAxis(Matrix3 &tmAxis);
                void CalcBulge(int axis, float stretch, float amplify);
                Point3 Map(int i, Point3 p); 
        };

#define STRETCHWSM_CLASSID	Class_ID(STRETCHOSM_CLASS_ID,0x98fe71a2)

class StretchWSM : public SimpleOSMToWSMObject {
	public:
		StretchWSM() {}
		StretchWSM(StretchMod *m) : SimpleOSMToWSMObject(m) {}
		void DeleteThis() { delete this; }
		SClass_ID SuperClassID() {return WSM_OBJECT_CLASS_ID;}
		Class_ID ClassID() {return STRETCHWSM_CLASSID;} 
		TCHAR *GetObjectName();
		RefTargetHandle Clone(RemapDir& remap)
			{StretchWSM *newobj = new StretchWSM((StretchMod*)mod->Clone(remap));
		newobj->SimpleOSMToWSMClone(this,remap);
		BaseClone(this, newobj, remap);
		return newobj;}
	};


//--- ClassDescriptor and class vars ---------------------------------

IParamMap *StretchMod::pmapParam = NULL;

class StretchClassDesc:public ClassDesc 
{
public:
    int                 IsPublic() { return 1; }
    void *              Create(BOOL loading = FALSE) { return new StretchMod; }
    const TCHAR *       ClassName();
    SClass_ID           SuperClassID() { return OSM_CLASS_ID; }
    Class_ID            ClassID() { return Class_ID(STRETCHOSM_CLASS_ID,12332321); }
    const TCHAR*        Category();
};

static StretchClassDesc stretchDesc;
extern ClassDesc* GetStretchModDesc() { return &stretchDesc; }

class StretchWSMClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) 
		{if (loading) return new StretchWSM; else return new StretchWSM(new StretchMod);}
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return STRETCHWSM_CLASSID; }
	const TCHAR* 	Category() {return GetSpaceWarpCatString(SPACEWARP_CAT_MODBASED);}
	};

static StretchWSMClassDesc stretchWSMDesc;
extern ClassDesc* GetStretchWSMDesc() { return &stretchWSMDesc; }

//
//
// Parameters

static int axisIDs[] = {IDC_X,IDC_Y,IDC_Z};

static ParamUIDesc descParam[] = {
        // Stretch
        ParamUIDesc(
                PB_STRETCH,
                EDITTYPE_FLOAT,
                IDC_STRETCH,IDC_STRETCHSPINNER,
                -BIGFLOAT,BIGFLOAT,
                0.1f),

        // Amplify
        ParamUIDesc(
                PB_AMPLIFY,
                EDITTYPE_FLOAT,
                IDC_AMPLIFY,IDC_AMPLIFYSPINNER,
                -BIGFLOAT,BIGFLOAT,
                0.1f),

        // Axis
        ParamUIDesc(PB_AXIS,TYPE_RADIO,axisIDs,3),

        // Affect region
        ParamUIDesc(PB_DOREGION,TYPE_SINGLECHEKBOX,IDC_STRETCH_AFFECTREGION),

        // From
        ParamUIDesc(
                PB_FROM,
                EDITTYPE_UNIVERSE,
                IDC_STRETCH_FROM,IDC_STRETCH_FROMSPIN,
                -BIGFLOAT,0.0f,
                SPIN_AUTOSCALE),

        // To
        ParamUIDesc(
                PB_TO,
                EDITTYPE_UNIVERSE,
                IDC_STRETCH_TO,IDC_STRETCH_TOSPIN,
                0.0f,BIGFLOAT,          
                SPIN_AUTOSCALE),        
        };
#define PARAMDESC_LENGH 6


static ParamBlockDescID descVer[] = {
        { TYPE_FLOAT, NULL, TRUE, 0 },
        { TYPE_FLOAT, NULL, TRUE, 1 },  
        { TYPE_INT, NULL, FALSE, 2 },
        { TYPE_INT, NULL, FALSE, 3 },
        { TYPE_FLOAT, NULL, TRUE, 4 },
        { TYPE_FLOAT, NULL, TRUE, 5 } };

#define PBLOCK_LENGTH   6

class StretchDlgProc : public ParamMapUserDlgProc 
{
public:
    BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,
                 LPARAM lParam);
    void DeleteThis() {}
};
static StretchDlgProc theStretchProc;

BOOL StretchDlgProc::DlgProc(TimeValue t, IParamMap *map,
                             HWND hWnd, UINT msg, WPARAM wParam, 
                             LPARAM lParam )
{
    switch (msg) {
    case CC_SPINNER_CHANGE:
        switch (LOWORD(wParam)) {
        case IDC_STRETCH_FROMSPIN: {
            float from, to;
            map->GetParamBlock()->GetValue(PB_FROM,t,from,FOREVER);
            map->GetParamBlock()->GetValue(PB_TO,t,to,FOREVER);
            if (from>to) {
                map->GetParamBlock()->SetValue(PB_TO,t,from);
                map->Invalidate();
            }
            break;
        }
            
        case IDC_STRETCH_TOSPIN: {
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


const TCHAR * StretchClassDesc::ClassName() 
{ 
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_CLASS_NAME, stringBuf, MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

const TCHAR * StretchWSMClassDesc::ClassName() 
{ 
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_CLASS_NAME, stringBuf, MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

const TCHAR * StretchClassDesc::Category() 
{ 
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_CATEGORY, stringBuf, MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

StretchMod::StretchMod() : SimpleMod()
{
    MakeRefByID(FOREVER, SIMPMOD_PBLOCKREF, 
                CreateParameterBlock(descVer, PBLOCK_LENGTH, 0));
        
    pblock->SetValue(PB_AXIS, TimeValue(0), 2/*Z*/);
}

void StretchMod::GetClassName (TSTR& s) 
{ 
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_CLASS_NAME, stringBuf, MAX_STRING_LENGTH);
        loaded = 1;
    }
    s = stringBuf;
}  

void StretchMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
    SimpleMod::BeginEditParams(ip,flags,prev);
        
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];
    if (! loaded) {
        LoadString (hResource, IDS_PARAMETERS, stringBuf, 
                    MAX_STRING_LENGTH);
        loaded = 1;
    }

    pmapParam = 
        CreateCPParamMap (descParam, PARAMDESC_LENGH,
                          pblock, ip, hResource,
                          MAKEINTRESOURCE(IDD_STRETCHPARAM),
                          stringBuf, 0);        
    pmapParam->SetUserDlgProc(&theStretchProc);
}

                

void StretchMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
    SimpleMod::EndEditParams(ip,flags,next);
    DestroyCPParamMap(pmapParam);
}


RefTargetHandle StretchMod::Clone(RemapDir& remap) {
        StretchMod* newmod = new StretchMod();
        newmod->ReplaceReference(SIMPMOD_PBLOCKREF, pblock->Clone(remap));
        newmod->SimpleModClone(this);
		BaseClone(this, newmod, remap);
        return(newmod);
        }

TCHAR * StretchMod::GetObjectName()
{ 
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_OBJECT_NAME, stringBuf, MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}

TCHAR * StretchWSM::GetObjectName()
{ 
    static int loaded = 0;
    static TCHAR stringBuf[MAX_STRING_LENGTH];

    if (! loaded) {
        LoadString (hResource, IDS_OBJECT_NAME, stringBuf, MAX_STRING_LENGTH);
        loaded = 1;
    }
    return stringBuf;
}


Deformer& StretchMod::GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,
                                  Matrix3& invmat)
{
    float angle, dir, from, to;
    int axis;   
    int doRegion;
    pblock->GetValue(PB_STRETCH,t,angle,FOREVER);
    pblock->GetValue(PB_AMPLIFY,t,dir,FOREVER);
    pblock->GetValue(PB_AXIS,t,axis,FOREVER);
    pblock->GetValue(PB_FROM,t,from,FOREVER);
    pblock->GetValue(PB_TO,t,to,FOREVER);
    pblock->GetValue(PB_DOREGION,t,doRegion,FOREVER);
    static StretchDeformer deformer;
    deformer = StretchDeformer(t,mc,angle,dir,axis,from,to,doRegion,
                               mat,invmat);
    return deformer;
}
Interval StretchMod::GetValidity(TimeValue t)
{
    float f;
    Interval valid = FOREVER;
    pblock->GetValue(PB_STRETCH, t, f, valid);
    pblock->GetValue(PB_AMPLIFY, t, f, valid);
    pblock->GetValue(PB_AXIS, t, f, valid);
    pblock->GetValue(PB_FROM, t, f, valid);
    pblock->GetValue(PB_TO, t, f, valid);
    return valid;
}




ParamDimension *
StretchMod::GetParameterDim(int pbIndex)
{
    switch (pbIndex) {
    case PB_STRETCH:    return defaultDim;
    case PB_AMPLIFY:    return defaultDim;
    case PB_FROM:       return stdWorldDim;
    case PB_TO:         return stdWorldDim;
    default:            return defaultDim;
    }
}

TSTR StretchMod::GetParameterName(int pbIndex)
{
    static TCHAR buf[1024];
    int bufLen = 1024;

    switch (pbIndex) {
    case PB_STRETCH:
        LoadString (hResource, IDS_STRETCH, buf, bufLen);
        return TSTR (buf);
    case PB_AMPLIFY:
        LoadString (hResource, IDS_AMPLIFY, buf, bufLen);
        return TSTR (buf);
    case PB_AXIS:
        LoadString (hResource, IDS_AXIS, buf, bufLen);
        return TSTR (buf);
    case PB_FROM:
        LoadString (hResource, IDS_FROM, buf, bufLen);
        return TSTR (buf);
    case PB_TO:
        LoadString (hResource, IDS_TO, buf, bufLen);
        return TSTR (buf);
    default:
        return TSTR(_T(""));
    }
}

BOOL StretchMod::GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis)
{
    int limit;
    pblock->GetValue(PB_DOREGION,t,limit,FOREVER);
    pblock->GetValue(PB_FROM,t,zmin,FOREVER);
    pblock->GetValue(PB_TO,t,zmax,FOREVER);
    pblock->GetValue(PB_AXIS,t,axis,FOREVER);
    return limit?TRUE:FALSE;
}

StretchDeformer::StretchDeformer() 
{ 
    tm.IdentityMatrix();
    time = 0;   
}

StretchDeformer::StretchDeformer(TimeValue t, ModContext &mc,
                                 float stretch, float amplify, int naxis, 
                                 float from, float to, int doRegion,
                                 Matrix3& modmat, Matrix3& modinv) 
{
    Matrix3 mat;
    Interval valid;     
//    if (from==to) doRegion = FALSE;
    this->doRegion = doRegion;
    this->from = from;
    this->to   = to;
    time   = t; 

    tm = modmat;
    invtm = modinv;
    mat.IdentityMatrix();
        
    switch ( naxis ) {
    case 0: mat.RotateY( -HALFPI );      break; //X
    case 1: mat.RotateX( HALFPI );  break; //Y
    case 2: break;  //Z
    }
    SetAxis( mat );     
    assert (mc.box);
    bbox = *mc.box;
    CalcBulge(naxis, stretch, amplify);
} 

void StretchDeformer::SetAxis(Matrix3 &tmAxis)
{
    Matrix3 itm = Inverse(tmAxis);
    tm    = tm*tmAxis;
    invtm =     itm*invtm;
}

// This calculates some values that are then stored to reduce computations
void StretchDeformer::CalcBulge(int axis, float stretch, float amplify)
{
    theAxis = axis;
    amount = stretch;
    amplifier = (amplify >= 0) ? amplify + 1 : 1.0F / (-amplify + 1.0F);
    if (! doRegion) {
        switch ( axis ) {
        case 0:
            heightMin = bbox.pmin.x;
            heightMax = bbox.pmax.x;
            break;
        case 1:
            heightMin = bbox.pmin.y;
            heightMax = bbox.pmax.y;
            break;
        case 2:
            heightMin = bbox.pmin.z;
            heightMax = bbox.pmax.z;
            break;
        }
    } else {
        heightMin = from;
        heightMax = to;
    }
}

// This mapper does the following.  
// It scales the Z value by 1 + amount, if amount is greater than 0 and by
// -1 / (amount - 1) if amount is less than 0.
// In the x and y it scales by a quadratic in the height.
// The quadratic is determined by the conditions that its value is 1 at
// both the minimum and maximum height and halfway in between is the maximum
// bulge (inward or outward depending upon whether stretch or squash)
Point3 StretchDeformer::Map(int i, Point3 p)
{
    float fraction, normHeight;
    float xyScale, zScale, a, b, c;
    
    if (amount == 0 || (heightMax - heightMin == 0))
        return p;

	if ( (doRegion) && (to-from == 0.0f ) )
		return p;
    
    p = p * tm;
    
    if (doRegion && p.z > to)
        normHeight = (to - heightMin) / (heightMax - heightMin);
    else if (doRegion && p.z < from)
        normHeight = (from - heightMin) / (heightMax - heightMin);
    else
        normHeight = (p.z - heightMin) / (heightMax - heightMin);

    if (amount < 0) {   // Squash
        xyScale = (amplifier * -amount + 1.0F);
        zScale = (-1.0F / (amount - 1.0F));
    } else {           // Stretch
        xyScale = 1.0F / (amplifier * amount + 1.0F);
        zScale = amount + 1.0F;
    }

    // a, b, and c are the coefficients of the quadratic function f(x)
    // such that f(0) = 1, f(1) = 1, and f(0.5) = xyScale
    a = 4.0F * (1.0F - xyScale);
    b = -4.0F * (1.0F - xyScale);
    c = 1.0F;
    fraction = (((a * normHeight) + b) * normHeight) + c;
    p.x *= fraction;
    p.y *= fraction;

    if (doRegion && p.z < from)
	p.z += (zScale - 1.0F) * from;
    else if (doRegion && p.z <= to)
	p.z *= zScale;
    else if (doRegion && p.z > to)
	p.z += (zScale - 1.0F) * to;
    else
	p.z *= zScale;

    p = p * invtm;
    return p;
}

