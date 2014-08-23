/*===========================================================================*\
	FILE: sunlight.cpp

	DESCRIPTION: Sunlight system plugin.

	HISTORY: Created Oct.15 by John Hutchinson
			Derived from the ringarray

	Copyright (c) 1996, All Rights Reserved.
 \*==========================================================================*/
/*===========================================================================*\
 | Include Files:
\*===========================================================================*/
#include <max.h>
#include <iparamb2.h>
#include <istdplug.h>
#include "sunlight.h"
#include "natLight.h"
#include "suntypes.h"
#include "autovis.h"
#include "sunclass.h"
#include "decomp.h"
#include "matrix3.h"
#include "compass.h"
#include "iparamb.h"
#include "macrorec.h"
#include "toneop.h"
#include "maxscrpt.h"
#include "maxclses.h"
#include "sunlightOverride.h"
#include <notify.h>

#ifdef _DEBUG
	#include <stdio.h>
#endif
#ifdef GLOBAL_COORD_SYS
#include <ICoordSys.h>
#include <ICoordSys_i.c>
#endif
// The DLL instance handle
HINSTANCE hInstance;

/*===========================================================================*\
 | Definitions:
\*===========================================================================*/
#define LIGHT_TM	0
#define LIGHT_MULT	1
#define LIGHT_INTENSE 2
#define LIGHT_SKY	3

#define REF_PARAM	0
#define REF_POINT	1
#define REF_LOOKAT	2
#define REF_MULTIPLIER 3
#define NUM_REF 4

// Parameter block indices
//#define PB_RAD	0
//#define PB_LAT	1
//#define PB_LONG	2
//#define PB_DATE	4
//#define PB_TIME 3
//#define PB_ZONE 5
//#define PB_DST 6

#define PB_RAD	4
#define PB_LAT	2
#define PB_LONG	3
#define PB_DATE	1
#define PB_TIME 0
#define PB_ZONE 5
#define PB_DST	6
#define PB_MANUAL 7
#define PB_SKY	8

//default data
#ifdef APL_DBCS		// --hide. for Japanese version
#define SF_LAT 35.7500f
#define SF_LONG -139.5833f
static char *defcityname = "Tokyo Japan";
#define SF_ZONE 9
#else
#define SF_LAT 37.618f
#define SF_LONG 122.373f
static char *defcityname ="San Francisco, CA";
#define SF_ZONE -8
#endif	// APL_DBCS
#define MINYEAR 1583
#define MAXYEAR 3000
#define MINRADIUS 0.0f
#define MAXRADIUS 100000000.0f
#define SUN_RGB Point3(0.88235294f, 0.88235294f, 0.88235294f)  // 225

// functions from GEOLOC.CPP
extern BOOL doLocationDialog(HWND hParent, IObjParam* ip, float* latitude,
							 float* longitude, char* cityName);

static int holdLastCity;
static char holdLastName[64];

// externals from GEOLOC.CPP
extern int lastCity;
extern char* lastCityName;

static int getTimeZone(float longi);
static float stdang_to_compass(float stdang);
float compass_to_stdang(float compass);
static float getZRot(TimeValue t, INode* node);

static bool inCreate = false;

class CityRestore : public RestoreObj {
public:
	enum {
		kBufSize = 64	// should be the same is SunMaster::city
	};

	CityRestore(SunMaster* master, const char* name, int undoCity)
		: _master(master), _undoIndex(undoCity), _redoIndex(lastCity)
	{
		strncpy(_redoCity, master->GetCity(), kBufSize);
		strncpy(_undoCity, name != NULL ? name : "", kBufSize);
	}

	virtual void Restore(int isUndo) {
		_master->SetCity(_undoCity);
		holdLastCity = lastCity = _undoIndex;
		strcpy(holdLastName, _undoCity);
		_master->UpdateUI(GetCOREInterface()->GetTime());
	}

	virtual void Redo() {
		_master->SetCity(_redoCity);
		holdLastCity = lastCity = _redoIndex;
		strcpy(holdLastName, _undoCity);
		_master->UpdateUI(GetCOREInterface()->GetTime());
	}

	virtual int Size() { return sizeof(*this); }
	virtual TSTR Description() { return "Undo Sunlight City Name"; }

	SunMaster*	_master;
	int			_undoIndex;
	int			_redoIndex;
	char		_undoCity[kBufSize];
	char		_redoCity[kBufSize];
};

/*===========================================================================*\
 | Sun Master Methods:
\*===========================================================================*/

// This method returns a new instance of the slave controller.
Control* GetNewSlaveControl(SunMaster *master, int i);

// Initialize the class variables...
//HWND SunMaster::hMasterParams = NULL;
IObjParam *SunMaster::iObjParams;


ISpinnerControl *SunMaster::radSpin;
ISpinnerControl *SunMaster::latSpin;
ISpinnerControl *SunMaster::longSpin;
ISpinnerControl *SunMaster::hourSpin;
ISpinnerControl *SunMaster::minSpin;
ISpinnerControl *SunMaster::secSpin;
ISpinnerControl *SunMaster::yearSpin;
ISpinnerControl *SunMaster::monthSpin;
ISpinnerControl *SunMaster::daySpin;
ISpinnerControl *SunMaster::northSpin;
ISpinnerControl *SunMaster::zoneSpin;
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
ISliderControl  *SunMaster::skySlider;
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
ICustStatus	*SunMaster::altEdit;
ICustStatus	*SunMaster::azEdit;
ICustStatus	*SunMaster::cityDisplay;
// no longer static 
// SYSTEMTIME SunMaster::theTime;


void SunMaster::GetClassName(TSTR& s) {
	s = GetString(daylightSystem ? IDS_DAY_CLASS : IDS_SUN_CLASS);
}
Animatable* SunMaster::SubAnim(int i)
{
	switch (i) {
	case 0:
		return pblock;
	case 1:
		return theLookat;
	case 2:
		return theMultiplier;
	}
	return NULL;
}

TSTR SunMaster::SubAnimName(int i)
{
	switch (i) {
	case 0:
		return GetString(IDS_DB_PARAMETERS);
	case 1:
		return GetString(IDS_LOOKAT);
	case 2:
		return GetString(IDS_MULTIPLIER);
	}
	return TSTR();
}

#define SUNMASTER_VERSION_SUNLIGHT 1
#define SUNMASTER_VERSION_DAYLIGHT 3

/* here's what it looked like in VIZ
	ParamBlockDesc desc[] = {
		{ TYPE_FLOAT, NULL, FALSE },//radius
		{ TYPE_FLOAT, NULL, FALSE },//lat
		{ TYPE_FLOAT, NULL, FALSE },//long
		{ TYPE_FLOAT, NULL, TRUE }, //date
		{ TYPE_INT, NULL, TRUE },//time
		{ TYPE_INT, NULL, FALSE },//zone
		{ TYPE_BOOL, NULL, FALSE },//dst
		};

*/

static ParamBlockDescID desc[] = {
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_DATE}, //date
		{ TYPE_INT, NULL, TRUE , PB_TIME},//time
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		};

	//Paramneters for MAX 2.0 version
/*
static ParamBlockDescID desc1[] = {
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		};*/
static ParamBlockDescID desc1[] = {
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		};

static ParamBlockDescID desc2Daylight[] = {
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		{ TYPE_BOOL, NULL, FALSE, PB_MANUAL},//manual override
		};

static ParamBlockDescID desc3Daylight[] = {
		{ TYPE_INT, NULL, TRUE, PB_TIME },//time
		{ TYPE_FLOAT, NULL, TRUE , PB_DATE}, //date
		{ TYPE_FLOAT, NULL, TRUE, PB_LAT },//lat
		{ TYPE_FLOAT, NULL, TRUE, PB_LONG },//long
		{ TYPE_FLOAT, NULL, TRUE, PB_RAD },//radius
		{ TYPE_INT, NULL, FALSE, PB_ZONE },//zone
		{ TYPE_BOOL, NULL, FALSE, PB_DST},//dst
		{ TYPE_BOOL, NULL, FALSE, PB_MANUAL},//manual override
		{ TYPE_FLOAT, NULL, TRUE, PB_SKY },//skyCondition
		};

// Constructor.
SunMaster::SunMaster(bool daylight) {

	thePoint = NULL;
	theObject = NULL;
	theLookat = NULL;
	theMultiplier = NULL;
	ignore_point_msgs = FALSE;
	hMasterParams = NULL;
	daylightSystem = daylight;



	// Create a parameter block and make a reference to it.
	if (daylightSystem)
		MakeRefByID( FOREVER, REF_PARAM, CreateParameterBlock( desc3Daylight, 9, SUNMASTER_VERSION_DAYLIGHT ) );
	else
		MakeRefByID( FOREVER, REF_PARAM, CreateParameterBlock( desc1, 7, SUNMASTER_VERSION_SUNLIGHT ) );

	//Make the controllers linear
	
	Control *c = (Control *) CreateInstance(CTRL_FLOAT_CLASS_ID,Class_ID(LININTERP_FLOAT_CLASS_ID,0)); 
	pblock->SetController(PB_DATE, c, TRUE);
	c = (Control *) CreateInstance(CTRL_FLOAT_CLASS_ID,Class_ID(LININTERP_FLOAT_CLASS_ID,0)); 
	pblock->SetController(PB_TIME, c, TRUE);
	
	// Set the initial values at time 0.
	GetLocalTime(&theTime);
	tvalid = NEVER;

	//TIME_ZONE_INFORMATION tzi;
	//DWORD result = GetTimeZoneInformation(&tzi);
	
	int zone = SF_ZONE;

	// Don't use the time zone of the computer but use the one of the default city.
	//if (result == TIME_ZONE_ID_DAYLIGHT || result ==TIME_ZONE_ID_STANDARD)
	//	if(tzi.Bias != 0) zone=-tzi.Bias/60;

	SetLat( TimeValue(0), SF_LAT);
	SetLong( TimeValue(0), SF_LONG );
	SetCity(defcityname);
	SetNorth(TimeValue(0), 0.0f);
	SetZone(TimeValue(0), zone);

	// Since we don't use the time zone of the computer anymore then default the daylight saving time to false.
	//SetDst(TimeValue(0), result == TIME_ZONE_ID_DAYLIGHT ? TRUE : FALSE);
	SetDst(TimeValue(0), FALSE);

	SetManual(0, false);
	refset=FALSE;
    mbOldAnimation = FALSE;
	timeref=0.0f;
	dateref=0;

	// Get time notification so we can update the UI
	GetCOREInterface()->RegisterTimeChangeCallback(this);

// Since the users typically render an image per solstice/equinox, create
// the daylight system to a time that approximatly represents the summer solstice
// of the current year (i.e. June 12th of the current year, 12:00).
#ifdef DAYLIGHT_DEFAULT_TIME_SUMMER_SOLSTICE
	theTime.wMonth = 6;
	theTime.wDay = 21;

	theTime.wHour = 12;
	theTime.wMinute = 0;
	theTime.wSecond = 0;
#endif // DAYLIGHT_DEFAULT_TIME_SUMMER_SOLSTICE

	// since we know that theTime is valid we can do this
	interpJulianStruct jd = fusetime(theTime);
	SetDate( TimeValue(0),  jd );
	SetTime( TimeValue(0),  jd );
	// now that the parmeter block is up to date we can calculate
	// the dependent vars
	calculate(TimeValue(0),FOREVER);

	}

static void NodesCloned(void* param, NotifyInfo*)
{
	static_cast<SunMaster*>(param)->RefThePointClone();
	UnRegisterNotification(NodesCloned, param, NOTIFY_POST_NODES_CLONED);
}

SunMaster::~SunMaster()
{
	// Just in case this gets deleted after Clone is called
	// but before the NOTIFY_POST_NODES_CLONED notification is sent.
	UnRegisterNotification(NodesCloned, this, NOTIFY_POST_NODES_CLONED);
	// Stop time notifications
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
}

void SunMaster::RefThePointClone()
{
	if (thePoint != NULL) {
		INode* p = thePoint;
		thePoint = NULL;
		ReplaceReference(REF_POINT, p);
	}
}

// This method is called to return a copy of the ring master.
RefTargetHandle SunMaster::Clone(RemapDir& remap) {
    SunMaster* newm = new SunMaster(daylightSystem);	
	newm->ReplaceReference(REF_PARAM,pblock->Clone(remap));
	newm->ReplaceReference(REF_POINT,NULL);
	newm->ReplaceReference(REF_LOOKAT,remap.CloneRef(theLookat));
	newm->ReplaceReference(REF_MULTIPLIER,remap.CloneRef(theMultiplier));
	newm->thePoint = NULL;
	newm->theObject = NULL;
	newm->dateref = dateref;
	remap.PatchPointer((RefTargetHandle*)&newm->thePoint,(RefTargetHandle)thePoint);
	if (newm->thePoint == NULL)
		RegisterNotification(NodesCloned, newm, NOTIFY_POST_NODES_CLONED);
	else
		RefThePointClone();
	remap.PatchPointer((RefTargetHandle*)&newm->theObject,(RefTargetHandle)theObject);

	BaseClone(this, newm, remap);
	return(newm);
	}

// This method is called to update the UI parameters to reflect the
// correct values at the time passed.  Note that FALSE is passed as
// the notify parameter.  This ensure that notification message are
// not sent when the values are updated.
void SunMaster::UpdateUI(TimeValue t)
	{
	if ( hMasterParams ) {

		bool dateTime = !GetManual(t);

		EnableWindow(GetDlgItem(hMasterParams,IDC_MANUAL), !inCreate);
		CheckDlgButton(hMasterParams, IDC_MANUAL, !dateTime);

		radSpin->Enable(dateTime);
		latSpin->Enable(dateTime);
		longSpin->Enable(dateTime);
		hourSpin->Enable(dateTime);
		minSpin->Enable(dateTime);
		secSpin->Enable(dateTime);
		monthSpin->Enable(dateTime);
		daySpin->Enable(dateTime);
		yearSpin->Enable(dateTime);
		northSpin->Enable(dateTime);
		zoneSpin->Enable(dateTime);
		if (daylightSystem)
		{
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			skySlider->Enable(dateTime);
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
		}

		EnableWindow(GetDlgItem(hMasterParams,IDC_DST), dateTime);
		EnableWindow(GetDlgItem(hMasterParams,IDC_GETLOC), dateTime);

		radSpin->SetValue( GetRad(t), FALSE );
		latSpin->SetValue( GetLat(t), FALSE );
		longSpin->SetValue( GetLong(t), FALSE );
		if (daylightSystem)
		{
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			skySlider->SetValue( GetSkyCondition(t), FALSE);
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
		}

		radSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_RAD,t));
		latSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_LAT,t));
		longSpin->SetKeyBrackets(pblock->KeyFrameAtTime(PB_LONG,t));
		if (daylightSystem)
		{
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			skySlider->SetKeyBrackets(pblock->KeyFrameAtTime(PB_SKY,t));
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
		}

		BOOL timekey = pblock->KeyFrameAtTime(PB_TIME,t);
		BOOL datekey = pblock->KeyFrameAtTime(PB_DATE,t);

		hourSpin->SetValue( GetHour(t), FALSE );
		minSpin->SetValue( GetMin(t), FALSE );
		secSpin->SetValue( GetSec(t),FALSE);
		monthSpin->SetValue( GetMon(t), FALSE );
		daySpin->SetValue( GetDay(t), FALSE );
		yearSpin->SetValue( GetYr(t), FALSE );

		hourSpin->SetKeyBrackets(timekey);
		minSpin->SetKeyBrackets(timekey);
		secSpin->SetKeyBrackets(timekey);
		monthSpin->SetKeyBrackets(datekey);
		daySpin->SetKeyBrackets(datekey);
		yearSpin->SetKeyBrackets(datekey);

		northSpin->SetValue( GetNorth(t), FALSE);
		zoneSpin->SetValue( GetZone(t), FALSE);
		CheckDlgButton(hMasterParams,IDC_DST, GetDst(t));
		char buf[3];
		if(azEdit) azEdit->SetText(itoa((int)rtd(az),buf,10));
		if(altEdit) altEdit->SetText(itoa((int)rtd(alt),buf,10));
		if(cityDisplay) cityDisplay->SetText(city);
		}
	}


// The master controller of a system plug-in should implement this 
// method to give MAX a list of nodes that are part of the system.   
// The master controller should fill in the given table with the 
// INode pointers of the nodes that are part of the system. This 
// will ensure that operations like cloning and deleting affect 
// the whole system.  MAX will use GetInterface() in the 
// tmController of each selected node to retrieve the master 
// controller and then call GetSystemNodes() on the master 
// controller to get the list of nodes.
void SunMaster::GetSystemNodes(INodeTab &nodes, SysNodeContext c)
	{
			if(thePoint)nodes.Append(1,&thePoint);
			if(theObject)nodes.Append(1,&theObject);

			if (daylightSystem)
				NatLightAssembly::AppendAssemblyNodes(theObject, nodes, c);
	}

int SunMaster::NumRefs()
{
	return NUM_REF;
};

// This methods returns the ith reference - there are two: the 
// parameter block and the light (helper, actually).
RefTargetHandle SunMaster::GetReference(int i)  { 
	switch (i) {
	case REF_PARAM:
		return pblock;
	case REF_POINT:
		return thePoint;
	case REF_LOOKAT:
		return theLookat;
	case REF_MULTIPLIER:
		return theMultiplier;
		}
	return NULL;
	}

// This methods sets the ith reference - there are two.
void SunMaster::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case REF_PARAM:
		pblock = static_cast<IParamBlock*>(rtarg);
		break;
	case REF_POINT:
		thePoint = static_cast<INode*>(rtarg);
		break;
	case REF_LOOKAT:
		theLookat = static_cast<Control*>(rtarg);
		break;
	case REF_MULTIPLIER:
		theMultiplier = static_cast<Control*>(rtarg);
		break;
		}
	}		

BOOL SunMaster::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
	{
	TimeValue at,tnear = 0;
	BOOL tnearInit = FALSE;
	Control *ct, *cd;
	ct = cd = NULL;
	ct = pblock->GetController(PB_TIME);
	cd = pblock->GetController(PB_DATE);

	
	if (cd && cd->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
		if (ABS(at-t) < ABS(tnear-t)) tnear = at;
		}

	if (ct && ct->GetNextKeyTime(t,flags,at)) {
		if (!tnearInit) {
			tnear = at;
			tnearInit = TRUE;
		} else 
		if (ABS(at-t) < ABS(tnear-t)) tnear = at;
		}

	
	if (tnearInit) {
		nt = tnear;
		return TRUE;
	} else {
		return FALSE;
		}
	}

// This function converts a position on the unit sphere,
// given in azimuth altitude, to xyz coordinates
Point3 az_to_xyz(float az, float alt){
	double x,y,z;
	x = cos(alt)*sin(az);
	y = cos(alt)*cos(az);
	z =	sin(alt);
	Point3 xyzp(x,y,z);
	return Normalize(xyzp);
}


// ======= This method is the crux of the system plug-in ==========
// This method gets called by each slave controller and based on the slaves
// ID, it is free to do whatever it wants. In the current system there is only 
// the slave controller of the light.
void SunMaster::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method,
 int id) {
	Matrix3 tmat;
	Matrix3 *mat;
	tmat.IdentityMatrix();
	Point3 unitdir;
	float* mult;

	switch(id){
		case LIGHT_TM:
			if (theLookat != NULL && GetManual(t)) {
				theLookat->GetValue(t, val, valid, method);
			} else {
				float radius = GetRad(t,valid);

				// calculate the controllers dependent variables: the azimuth and altitude
				calculate(t,valid);

				// Calculate the translation and rotation of the node 
				//CAUTION slop data types
				mat = (Matrix3*)val;
				tmat.RotateX(PI/2.0f - float(alt));
				tmat.RotateZ(PI - float(az));
				unitdir = az_to_xyz(float(az), float(alt));
				tmat.SetTrans(radius*unitdir);
				if (theLookat != NULL) {
					SuspendAnimate();
					AnimateOff();
					SetXFormPacket xform(tmat);
					theLookat->SetValue(t, &xform, 0);
					ResumeAnimate();
				}

				Matrix3 tm = *mat;
				(*mat) = (method==CTRL_RELATIVE) ? tmat*(*mat) : tmat;

				if (theLookat != NULL) {
					// Lookat controllers setup some of their state in
					// the GetValue method, so it needs to be called.
					Interval v;
					theLookat->GetValue(t, &tm, v, method);
				}

				// Make sure spinners track when animating and in Motion Panel
				// the limits on the day spinner may be wrong
				if ( hMasterParams ) {
					int year, month, day ,leap, modays;
					month = GetMon(t);
					year = GetYr(t);
					day = GetDay(t);
					leap = isleap(year);
					if (month == 12) modays = 31;
					else modays = mdays[leap][month]-mdays[leap][month-1];
					daySpin->SetLimits(1,modays,FALSE);
				}

//				UpdateUI(t);
			}
			break;

		case LIGHT_INTENSE:
		case LIGHT_MULT:
			mult=(float*)val;
			if (theMultiplier != NULL && GetManual(t)) {
				theMultiplier->GetValue(t, val, valid, method);
			} else {
				// calculate the controllers dependent variables: the azimuth and altitude
				calculate(t,valid);
				*mult = calcIntensity(t, valid);
				if (theMultiplier != NULL) {
					SuspendAnimate();
					AnimateOff();
					theMultiplier->SetValue(t, mult, 0);
					ResumeAnimate();
				}
			}
			if (id == LIGHT_MULT) {
				float factor = 1500.0f;
				ToneOperatorInterface* pI = static_cast<ToneOperatorInterface*>(
					GetCOREInterface(TONE_OPERATOR_INTERFACE));
				if (pI != NULL) {
					ToneOperator* p = pI->GetToneOperator();
					if (p != NULL) {
						factor = p->GetPhysicalUnit(t, valid);
					}
				}
				*mult /= factor;
			}
			break;

		case LIGHT_SKY:
			mult=(float*)val;
			*mult = GetSkyCondition(t, valid);
			break;
	}
}

static inline int getJulian(int mon, int day)
{
	static int days[] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
	};

	return days[mon - 1] + day;
}

float SunMaster::calcIntensity(TimeValue t, Interval& valid)
{
	const float Esc = 127500;	// Solar Illumination Constant in lx
	const float kPi = 3.14159265358979323846f;
	const float k2Pi365 = (2.0f * kPi) / 365.0f;
	float cloudy;

	cloudy = GetSkyCondition(t, valid);

	if (cloudy > 0.75f || alt <= 0.0f) {
		return 0.0f;
	}

	int julian = getJulian(GetMon(t, valid), GetDay(t, valid));

	float Ext = Esc * (1.0f + .034f * cosf(k2Pi365 * float(julian - 2)));
	float opticalAirMass = 1.0f / sin(alt);
	if (cloudy < 0.35f)
		opticalAirMass *= -0.21f;
	else
		opticalAirMass *= -0.80f;

	return Ext * exp(opticalAirMass);
}

void SunMaster::SetValue(TimeValue t, void *val, int commit, GetSetMethod method, int id)
{
	switch (id) {
	case LIGHT_TM:
		// Always set the lookat controller. To keep
		// the lookat and time somewhat in sync.
		if (GetManual(t) && theLookat != NULL)
			theLookat->SetValue(t, val, commit, method);
		break;

	case LIGHT_INTENSE:
		// Always set the lookat controller. To keep
		// the lookat and time somewhat in sync.
		if (GetManual(t) && theMultiplier != NULL)
			theMultiplier->SetValue(t, val, commit, method);
		break;

	case LIGHT_MULT:
		// Always set the lookat controller. To keep
		// the lookat and time somewhat in sync.
		if (GetManual(t) && theMultiplier != NULL) {
			float factor = 1500.0f;
			ToneOperatorInterface* pI = static_cast<ToneOperatorInterface*>(
				GetCOREInterface(TONE_OPERATOR_INTERFACE));
			if (pI != NULL) {
				ToneOperator* p = pI->GetToneOperator();
				if (p != NULL) {
					factor = p->GetPhysicalUnit(t);
				}
			}
			factor *= *static_cast<float*>(val);
			theMultiplier->SetValue(t, &factor, commit, method);
		}
		break;

	case LIGHT_SKY:
		SetSkyCondition(t, *static_cast<float*>(val));
		break;
	}
}


class TimeDimension : public ParamDimension {
	public:
		DimType DimensionType() {return DIM_CUSTOM;}
		// Enforce range limits. Out-of-range values are reset to valid limits.
		float Convert(float value)
		{
			// Convert seconds to hours.
			if (value < 0.0f)
				return 0.0f;
			else if (value >= SECS_PER_DAY)
				value = SECS_PER_DAY - 1;
			return value/3600.0f;
		}
		float UnConvert(float value)
		{
			// Convert hours to seconds.
			if (value < 0.0f)
				return 0.0f;
			else if (value >= 24.0f)
				return SECS_PER_DAY - 1;
			return value*3600.0f;
		}
	};
static TimeDimension theTimeDim;


// This is the method that recieves change notification messages
RefResult SunMaster::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
     PartID& partID, RefMessage message ) 
    {
	switch (message) {
		case REFMSG_GET_PARAM_DIM: { 
			// The ParamBlock needs info to display in the track view.
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_RAD: gpd->dim = stdWorldDim;break;
				case PB_TIME: gpd->dim = &theTimeDim;break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_DATE: gpn->name = GetString(IDS_SOL_DATE);break;
				case PB_TIME: gpn->name = GetString(IDS_SOL_TIME);break;
				case PB_RAD: gpn->name = GetString(IDS_RAD);break;
				case PB_LAT: gpn->name = GetString(IDS_LAT);break;
				case PB_LONG: gpn->name = GetString(IDS_LONG);break;
				case PB_SKY: gpn->name = GetString(IDS_SKY_COVER);break;
//				case PB_ZONE: gpn->name = GetString(IDS_ZONE);break;
				}
			return REF_STOP; 
			}

		case REFMSG_TARGET_DELETED:
			if (hTarget==thePoint) {
				thePoint = NULL;
				break;
				}
			return REF_STOP;

		case REFMSG_CHANGE:
			if ( hTarget==thePoint && !ignore_point_msgs ) {
				if (hMasterParams) 
					UpdateUI(iObjParams->GetTime());
				break;
				}
			else if (hTarget == pblock){
				if (hMasterParams) 
					UpdateUI(iObjParams->GetTime());
				break;
				}
			else if (hTarget == theLookat && GetManual(GetCOREInterface()->GetTime()))
				break;
			else if (hTarget == theMultiplier && GetManual(GetCOREInterface()->GetTime()))
				break;
			return REF_STOP;
		}
	return(REF_SUCCEED);
	}

// Sets the city name.
void SunMaster::SetCity(const char* name)
{
	const char* nm = name;
	if (nm != NULL)
	{
		if (*nm == '+')
			++nm;
		strcpy(city, nm);
	}
	else
	{
		*city = '\0';
		lastCity = -1;
	}
}

// The following methods set and get the radius value in the parameter block.
void SunMaster::SetRad(TimeValue t, float r) { 
	pblock->SetValue( PB_RAD, t, r );
	}

float SunMaster::GetRad(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_RAD, t, f, valid );
	return f;
	}

// The following methods set and get the lat/long in the parameter block.
void SunMaster::SetLat(TimeValue t, float r) { 
	pblock->SetValue( PB_LAT, t, r );
	}

float SunMaster::GetLat(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_LAT, t, f, valid );
	return f;
	}

void SunMaster::SetLong(TimeValue t, float r) { 
	pblock->SetValue( PB_LONG, t, r );
	}

float SunMaster::GetLong(TimeValue t, Interval& valid ) { 	
	float f;
	pblock->GetValue( PB_LONG, t, f, valid );
	return f;
	}

// Other pblock access methods

void SunMaster::SetZone(TimeValue t, int h) { 
	pblock->SetValue( PB_ZONE, t, h );
	}

int SunMaster::GetZone(TimeValue t, Interval& valid ) { 	
	int i;
	pblock->GetValue( PB_ZONE, t, i, valid );
	return i;
	}

void SunMaster::SetDst(TimeValue t, BOOL h) { 
	pblock->SetValue( PB_DST, t, h );
	}

BOOL  SunMaster::GetDst(TimeValue t, Interval& valid ) { 	
	BOOL b;
	pblock->GetValue( PB_DST, t, b, valid );
	return b;
	}

void SunMaster::SetManual(TimeValue t, BOOL h) { 
	if (daylightSystem) {
		pblock->SetValue( PB_MANUAL, t, h );
		}
	}

BOOL  SunMaster::GetManual(TimeValue t, Interval& valid ) {
	if (daylightSystem && pblock != NULL) {
		BOOL b;
		pblock->GetValue( PB_MANUAL, t, b, valid );
		return b;
		}
	return false;
	}

void SunMaster::SetSkyCondition(TimeValue t, float sky)
{
	if (daylightSystem)
		pblock->SetValue( PB_SKY, t, sky );
}

float SunMaster::GetSkyCondition(TimeValue t, Interval& valid)
{
	if (daylightSystem) {
		float f;
		pblock->GetValue( PB_SKY, t, f, valid );
		return f;
	}
	return 1.0f;
}

void SunMaster::SetNorth(TimeValue t, float r) { 
	if (thePoint) align_north(t,r);
	}

float SunMaster::GetNorth(TimeValue t, Interval& valid ) { 	
	return getZRot(t, thePoint);
	}


// This method actually places a representation of the current Julian date
// into the parameter block where it is interpolated. Because pblocks
// can only handle floats, two values are interpolated separately.
// The number of days goes into PB_DATE and is interpolated as a float.
//
// Furthermore because the number of JulianDays can be such a large number,
// prior to sticking it in the pblock we save a local epoch and measure date
// changes relative to this benchmark
//
void SunMaster::SetDate(TimeValue t, const interpJulianStruct& jd) { 
	double t1;

	// if were not animating we save a local epoch and measure relative to it
//	if(!Animating()){
	if(!refset){
		dateref=(long)jd.days;
		refset = TRUE;
		t1=0.0;
	}
	else {
		tvalid.SetEmpty();
		t1 = jd.days - dateref;
	}

	pblock->SetValue( PB_DATE, t, (float) t1 ); //caution possible data loss
	}

// This method puts the time of day into the pblock.
// The number of seconds in the current day goes into PB_TIME and
// is interpolated as an int.
void SunMaster::SetTime(TimeValue t, const interpJulianStruct& jd) { 
	pblock->SetValue( PB_TIME, t, jd.subday );
	}

// GetTime pulls the interpolated parameter block values out and passes them through
// fracturetime which adds the local epoch back in and explodes the value into a gregorian
// representation in the static var theTime.

// We implement a validity mechanism for the local time representation

void SunMaster::GetTime(TimeValue t, Interval& valid ) {
	if(0){
//	if(tvalid.InInterval(t)){
		valid &= tvalid;
		return;
	}
	else{
		tvalid = FOREVER;
		interpJulianStruct jd;
		float date;
		int time;
		pblock->GetValue( PB_DATE, t, date, tvalid );
		pblock->GetValue( PB_TIME, t, time, tvalid );
		valid &= tvalid;
		if (mbOldAnimation) {
			jd.days=date;
			jd.subday=time;
		}
		else {
			jd.days=floor(date + 0.5);
			jd.subday=time % (24 * 60 * 60);	// Truncate time to a day
			if (jd.subday < 0)					// No negative times.
				jd.subday += 24 * 60 * 60;
		}
		jd.epoch=dateref;
		fracturetime(jd,theTime);
	}
	}

//////////////////////////////////////////////////////////////
// The Pseudo pblock methods for getting time and setting time
//////////////////////////////////////////////////////////////

void SunMaster::SetHour(TimeValue t, int h) { 
	theTime.wHour = h;
	SetTime(t,fusetime(theTime));
	}

int SunMaster::GetHour(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wHour;
	}

void SunMaster::SetMin(TimeValue t, int h) { 
	theTime.wMinute = h;
	SetTime(t,fusetime(theTime));
	}

int SunMaster::GetMin(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wMinute;
	}

void SunMaster::SetSec(TimeValue t, int h) { 
	theTime.wSecond = h;
	SetTime(t,fusetime(theTime));
	}

int SunMaster::GetSec(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wSecond;
	}

void SunMaster::SetMon(TimeValue t, int h) { 
	theTime.wMonth = h;
	SetDate(t,fusetime(theTime));
	}

int SunMaster::GetMon(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wMonth;
	}

void SunMaster::SetDay(TimeValue t, int h) { 
	theTime.wDay = h;
	SetDate(t,fusetime(theTime));
	}

int SunMaster::GetDay(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wDay;
	}

void SunMaster::SetYr(TimeValue t, int h) { 
	theTime.wYear = h;
	SetDate(t,fusetime(theTime));
	}

int SunMaster::GetYr(TimeValue t, Interval& valid ) { 	
	GetTime(t,valid);//throw away return, we want side-effect
	return theTime.wYear;
	}

/////////////////////////////////////////////////////////////
//Methods for getting the private, dependent variables
///////////////////////////////////////////////////////////////

Point2 SunMaster::GetAzAlt(TimeValue t, Interval& valid ) {
	Point2 result;
	GetTime(t,valid);
	calculate(t,FOREVER);
	result.x = (float) az;
	result.y = (float) alt;
	return result;
}

// calculate the dependent variables at the given time
void SunMaster::calculate(TimeValue t, Interval& valid){
	double latitude,longitude;
	long hour,min,sec,month,day,year,zone;
	BOOL dst;

	// Retrieve the values of the UI parameters at the time passed in.
	zone = GetZone(t,valid);
	dst = GetDst(t,valid);
	latitude = GetLat(t,valid);
	longitude = GetLong(t,valid);
	hour = GetHour(t,valid);
	min = GetMin(t,valid);
	sec = GetSec(t,valid);
	month = GetMon(t,valid);
	day = GetDay(t,valid);
	year = GetYr(t,valid);

	double st;
	long zonedst = zone;
	if(dst) zonedst++;
	sunLocator(dtr(latitude), dtr(longitude), month, day,
		year, hour - zonedst, min,sec, 0,
		   &alt, &az, &st);

	

#ifdef _DEBUG
	FILE *stream;
	stream=fopen("round.log","a");
	float date;
	pblock->GetValue( PB_DATE, t, date, valid );
	float time;
	pblock->GetValue( PB_TIME, t, time, valid );
	fprintf(stream, "Yr: %d\tMon: %d\tDay: %d\tHr: %d\tMin: %d\tSec: %d\n", year, month, day, hour, min, sec );
/*	printf( "UYr: %d\tUMon: %d\tUDay: %d\tUHr: %d\tUMin: %d\tUSec: %d\n", hourSpin->GetIVal(),\
		monthSpin->GetIVal(), daySpin->GetIVal(), hourSpin->GetIVal(),\
		minSpin->GetIVal(), secSpin->GetIVal() );*/
	fprintf(stream, "Date: %f\tOffset: %d\tTime: %f\tOffset: %f\n", date, dateref, time, timeref );
  	fprintf(stream, "Lat: %f\tLong: %f\tZone: %ld\n\n", latitude, longitude, zone );
	fprintf(stream, "Az: %f\tAlt: %f\tSt: %f\n\n\n", az, alt, st );
	fclose(stream);
#endif

}


//method to rotate the helper object from the UI of the master controller
void SunMaster::align_north(TimeValue now, float north){
	assert(thePoint);

	// animate off and suspend msg processing
	ignore_point_msgs = TRUE;
	SuspendAnimate();
	AnimateOff();

	AffineParts lparts;
	float ang[3];
	INode *parent;
	Matrix3 nodeTM, parentTM, localTM, newTM;
  	parent = thePoint->GetParentNode();
	nodeTM = thePoint->GetNodeTM(now);
	parentTM = parent->GetNodeTM(now);
	localTM = nodeTM * Inverse(parentTM);
	decomp_affine(localTM, &lparts);
	QuatToEuler(lparts.q, ang);

	int turns = (int) (ang[2]/FTWO_PI);
	float subturn = ang[2] - turns;//we only affect the scrap
	ang[2]= turns + fdtr(compass_to_stdang(north));
	
	// build a new matrix
	EulerToQuat(ang, lparts.q);
	lparts.q.MakeMatrix(newTM);
	newTM.SetTrans(lparts.t);

	// animate back on and msg processing back to normal
	thePoint->SetNodeTM(now, newTM);
	ResumeAnimate();
	ignore_point_msgs = FALSE;
}

float stdang_to_compass(float stdang){
	float rel =  - stdang;
	if (rel >=0.0f) 
		return rel;
	else 
		return FTWO_PI - (float)fabs(rel);
}

float compass_to_stdang(float compass){
		return  - compass;
}

// Returns the local Zrotation of the node less any winding number
// measured relative to the local z axis 
// positive values are clockwise

float getZRot(TimeValue t,INode *node){

	AffineParts lparts;
	float ang[3];
	INode *parent;
	Matrix3 nodeTM, parentTM, localTM, newTM;
	if (node ){
		nodeTM = node->GetNodeTM(t);
		parent = node->GetParentNode();
		if(parent){
			parentTM = parent->GetNodeTM(t);
			localTM = nodeTM * Inverse(parentTM);
			decomp_affine(localTM, &lparts);
			QuatToEuler(lparts.q, ang);
			float test1 = (ang[2]/FTWO_PI);
			int test2 = (int) test1;
			float temp = (float) test1-test2;
			return frtd(stdang_to_compass(temp*FTWO_PI));
		} else
			return 0.0f;
	}  else
		return 0.0f;
}




// Get the APPROXIMATE time zone from the longitude.
int getTimeZone(float longi)
{
    int tz;
    if (longi >= 0)
        tz = -(int)((longi + 7.5) / 15);
    else
        tz = (int)((-longi + 7.5) / 15);

    return tz;
}



// The Dialog Proc

INT_PTR CALLBACK MasterParamDialogProc( HWND hDlg, UINT message, 
	WPARAM wParam, LPARAM lParam )
	{
	char buf[3];
	SunMaster *mc = (SunMaster *)GetWindowLongPtr( hDlg, GWLP_USERDATA );
	if ( !mc && message != WM_INITDIALOG ) return FALSE;
	TimeValue now = mc->iObjParams->GetTime();

	assert(mc->iObjParams);
	switch ( message ) {
		int year, month,day,leap,modays;// locals for handling date changes
		case WM_INITDIALOG:
			mc = (SunMaster *)lParam;
			SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR)mc );
			SetDlgFont( hDlg, mc->iObjParams->GetAppHFont() );

			// set up the date locals based on the current UI
			year = mc->GetYr(now);
			month = mc->GetMon(now);
			day = mc->GetDay(now);
			leap = isleap(year);
			if (month == 12) modays = 31;
			else modays = mdays[leap][month]-mdays[leap][month-1];

			// reset these copies
			holdLastCity = -1;
			strncpy(holdLastName, mc->GetCity(), sizeof(holdLastName));
			
			mc->radSpin  = GetISpinner(GetDlgItem(hDlg,IDC_RADSPINNER));
			mc->latSpin  = GetISpinner(GetDlgItem(hDlg,IDC_LATSPINNER));
			mc->longSpin  = GetISpinner(GetDlgItem(hDlg,IDC_LONGSPINNER));
			mc->yearSpin  = GetISpinner(GetDlgItem(hDlg,IDC_YEARSPINNER));
			mc->monthSpin  = GetISpinner(GetDlgItem(hDlg,IDC_MONTHSPINNER));
			mc->daySpin  = GetISpinner(GetDlgItem(hDlg,IDC_DAYSPINNER));
			mc->secSpin  = GetISpinner(GetDlgItem(hDlg,IDC_SECSPINNER));
			mc->hourSpin  = GetISpinner(GetDlgItem(hDlg,IDC_HOURSPINNER));
			mc->minSpin  = GetISpinner(GetDlgItem(hDlg,IDC_MINSPINNER));
			mc->northSpin  = GetISpinner(GetDlgItem(hDlg,IDC_NORTHSPINNER));
			mc->zoneSpin  = GetISpinner(GetDlgItem(hDlg,IDC_ZONESPINNER));
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			mc->skySlider = mc->daylightSystem ? GetISlider(GetDlgItem(hDlg,IDC_SKY_COVER_SLIDER)) : NULL;
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			mc->azEdit  = GetICustStatus(GetDlgItem(hDlg,IDC_AZ));
			mc->altEdit  = GetICustStatus(GetDlgItem(hDlg,IDC_ALT));
			mc->cityDisplay  = GetICustStatus(GetDlgItem(hDlg,IDC_CITY));

			mc->radSpin->SetLimits( MINRADIUS,MAXRADIUS, FALSE );
			mc->latSpin->SetLimits( -90.0f, 90.0f, FALSE );
			mc->longSpin->SetLimits( -180.0f, 180.0f, FALSE );
			mc->yearSpin->SetLimits(MINYEAR,MAXYEAR, FALSE );
			mc->zoneSpin->SetLimits(-12,12, FALSE );
			mc->monthSpin->SetLimits(1,12, FALSE );
			mc->daySpin->SetLimits( 1, modays, FALSE );
			mc->hourSpin->SetLimits(0,23, FALSE );
			mc->minSpin->SetLimits( 0, 59, FALSE );
			mc->secSpin->SetLimits( 0, 59, FALSE );
			mc->northSpin->SetLimits(0.0f, 359.99f, FALSE );

			mc->radSpin->SetAutoScale( TRUE );
			mc->latSpin->SetScale(float(0.1) );
			mc->longSpin->SetScale(float(0.1) );
			mc->yearSpin->SetScale(float(1.0) );
			mc->monthSpin->SetScale(float(1.0) );
			mc->daySpin->SetScale(float(1.0) );
			mc->hourSpin->SetScale(float(1.0) );
			mc->minSpin->SetScale(float(1.0) );
			mc->secSpin->SetScale(float(1.0) );
			mc->northSpin->SetScale(float(1.0) );
			mc->zoneSpin->SetScale(float(1.0) );

			SetupFloatSlider(hDlg, IDC_SKY_COVER_SLIDER, 0, 0, 1,
				mc->GetSkyCondition(now), 2);

			mc->latSpin->SetValue( mc->GetLat(now), FALSE );
			mc->longSpin->SetValue( mc->GetLong(now), FALSE );
			mc->radSpin->SetValue( mc->GetRad(now), FALSE );
			mc->hourSpin->SetValue( mc->GetHour(now), FALSE );
			mc->minSpin->SetValue( mc->GetMin(now), FALSE );
			mc->secSpin->SetValue( mc->GetSec(now), FALSE );
			mc->monthSpin->SetValue( month, FALSE );
			mc->daySpin->SetValue( mc->GetDay(now), FALSE );
			mc->yearSpin->SetValue( year , FALSE );
			mc->northSpin->SetValue( mc->GetNorth(now), FALSE );
			mc->zoneSpin->SetValue( mc->GetZone(now), FALSE );
			CheckDlgButton(hDlg,IDC_DST, mc->GetDst(now));
			mc->azEdit->SetText(itoa((int)rtd(mc->Getaz()),buf,10));
			mc->altEdit->SetText(itoa((int)rtd(mc->Getalt()),buf,10));
			mc->cityDisplay->SetText(mc->GetCity());

			mc->radSpin->LinkToEdit( GetDlgItem(hDlg,IDC_RADIUS), EDITTYPE_POS_UNIVERSE );			
			mc->latSpin->LinkToEdit( GetDlgItem(hDlg,IDC_LAT), EDITTYPE_FLOAT );			
			mc->longSpin->LinkToEdit( GetDlgItem(hDlg,IDC_LONG), EDITTYPE_FLOAT );			
			mc->yearSpin->LinkToEdit( GetDlgItem(hDlg,IDC_YEAR), EDITTYPE_INT );			
			mc->monthSpin->LinkToEdit( GetDlgItem(hDlg,IDC_MONTH), EDITTYPE_INT );			
			mc->daySpin->LinkToEdit( GetDlgItem(hDlg,IDC_DAY), EDITTYPE_INT );			
			mc->hourSpin->LinkToEdit( GetDlgItem(hDlg,IDC_HOUR), EDITTYPE_INT );			
			mc->minSpin->LinkToEdit( GetDlgItem(hDlg,IDC_MIN), EDITTYPE_INT );			
			mc->secSpin->LinkToEdit( GetDlgItem(hDlg,IDC_SEC), EDITTYPE_INT );			
			mc->northSpin->LinkToEdit( GetDlgItem(hDlg,IDC_NORTH), EDITTYPE_FLOAT );			
 			mc->zoneSpin->LinkToEdit( GetDlgItem(hDlg,IDC_ZONE), EDITTYPE_INT );			

			mc->iObjParams->RedrawViews(now, REDRAW_INTERACTIVE, mc);

			return FALSE;	// DB 2/27

		case WM_DESTROY:
			ReleaseISpinner( mc->radSpin );
			ReleaseISpinner( mc->latSpin );
			ReleaseISpinner( mc->longSpin );
			ReleaseISpinner( mc->daySpin );
			ReleaseISpinner( mc->monthSpin );
			ReleaseISpinner( mc->yearSpin );
			ReleaseISpinner( mc->hourSpin );
			ReleaseISpinner( mc->minSpin );
			ReleaseISpinner( mc->secSpin );
			ReleaseISpinner( mc->northSpin );
			ReleaseISpinner( mc->zoneSpin );
			ReleaseICustStatus( mc->azEdit);
			ReleaseICustStatus( mc->altEdit);
			ReleaseICustStatus( mc->cityDisplay);
			mc->radSpin  = NULL;
			mc->latSpin  = NULL;
			mc->longSpin  = NULL;
			mc->yearSpin  = NULL;
			mc->monthSpin  = NULL;
			mc->daySpin  = NULL;
			mc->hourSpin  = NULL;
			mc->minSpin  = NULL;
			mc->secSpin  = NULL;
			mc->azEdit  = NULL;
			mc->altEdit  =NULL;
			mc->cityDisplay  = NULL;
			mc->northSpin  =NULL;
			mc->zoneSpin  =NULL;


			return FALSE;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;


		case CC_SPINNER_CHANGE:
			if (!theHold.Holding())
				theHold.Begin();
			year = mc->GetYr(now);
			month = mc->GetMon(now);
			day = mc->GetDay(now);
			leap = isleap(year);
			if (month == 12) modays = 31;
			else modays = mdays[leap][month]-mdays[leap][month-1];


			switch ( LOWORD(wParam) ) {
				case IDC_RADSPINNER: mc->SetRad(now,  mc->radSpin->GetFVal() );  break;
				case IDC_LATSPINNER:
					mc->SetLat(now,  mc->latSpin->GetFVal() );
					mc->SetCity(NULL);
					break;
				case IDC_LONGSPINNER:
					mc->SetLong(now,  mc->longSpin->GetFVal() );
					mc->SetCity(NULL);
					break;
				case IDC_HOURSPINNER: mc->SetHour(now,  mc->hourSpin->GetIVal() );  break;
				case IDC_MINSPINNER: mc->SetMin(now,  mc->minSpin->GetIVal() );  break;
				case IDC_SECSPINNER: mc->SetSec(now,  mc->secSpin->GetIVal() );  break;
				case IDC_MONTHSPINNER:
					month =  mc->monthSpin->GetIVal();
					if (month == 12) modays = 31;
					else modays = mdays[leap][month]-mdays[leap][month-1];
					if (day > modays){
						day=modays;
						mc->SetDay(now,  day );}
					mc->daySpin->SetLimits(1,modays,FALSE);
					mc->SetMon(now,  month );
					break;
				case IDC_DAYSPINNER: mc->SetDay(now,  mc->daySpin->GetIVal() );  break;
				case IDC_YEARSPINNER: 
					year =  mc->yearSpin->GetIVal();
					leap = isleap(year);
					if (month == 12) modays = 31;
					else modays = mdays[leap][month]-mdays[leap][month-1];
					if (day > modays){
						day=modays;
						mc->SetDay(now,  day );}
					mc->daySpin->SetLimits(1,modays,FALSE);
					mc->SetYr(now,  year );
					break;
				case IDC_NORTHSPINNER:mc->SetNorth(now,  mc->northSpin->GetFVal() );  break;
				case IDC_ZONESPINNER: mc->SetZone(now,  mc->zoneSpin->GetIVal() );  break;
				}
			//Notify dependents: they will call getvalue which will update UI
			//else : calculate and Update UI directly
			if (mc->theObject) mc->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
			else {
				mc->calculate(now,FOREVER);
				mc->UpdateUI(now);
			}

			assert(mc->iObjParams);
			mc->iObjParams->RedrawViews(now, REDRAW_INTERACTIVE, mc);
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (!HIWORD(wParam) && message != WM_CUSTEDIT_ENTER)
			{
				theHold.Cancel();
				switch (LOWORD(wParam))
				{
					case IDC_LATSPINNER:
					case IDC_LONGSPINNER:
						// reset if spinner operation is cancelled
						lastCity = holdLastCity;
						mc->SetCity(holdLastName);
						mc->calculate(now,FOREVER);
						mc->UpdateUI(now);
						break;
				}
			} else {
				if ((holdLastCity != -1 || *holdLastName != '\0') && theHold.Holding())
					theHold.Put(new CityRestore(mc, holdLastName, holdLastCity));
				strncpy(holdLastName, mc->GetCity(), sizeof(holdLastName));
				holdLastCity = -1;
//				mc->iObjParams->RedrawViews(now, REDRAW_END, mc);
				theHold.Accept(GetString(IDS_UNDO_PARAM));
			}
			return TRUE;

		case CC_SLIDER_CHANGE:
			if (!theHold.Holding())
				theHold.Begin();
#ifndef NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			switch ( LOWORD(wParam) ) {
				case IDC_SKY_COVER_SLIDER:
					if (mc->daylightSystem)
						mc->SetSkyCondition(now, mc->skySlider->GetFVal());
					break;
			}
#endif // NO_DAYLIGHT_SKY_COVERAGE_SLIDER
			//Notify dependents: they will call getvalue which will update UI
			//else : calculate and Update UI directly
			if (mc->theObject) mc->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
			else {
				mc->UpdateUI(now);
			}

			assert(mc->iObjParams);
			mc->iObjParams->RedrawViews(now, REDRAW_INTERACTIVE, mc);
			return TRUE;

		case CC_SLIDER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;

		case CC_SLIDER_BUTTONUP:
			if (! HIWORD(wParam)) {
				theHold.Cancel();
			} else {
				theHold.Accept(GetString(IDS_UNDO_PARAM));
			}
			return TRUE;

		case WM_MOUSEACTIVATE:
			mc->iObjParams->RealizeParamPanel();
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			mc->iObjParams->RollupMouseMessage(hDlg,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:
				switch(LOWORD(wParam)) { // Switch on ID
				default:
					return FALSE;
				case IDC_MANUAL:
					theHold.Begin();
					mc->SetManual(now,IsDlgButtonChecked(mc->hMasterParams,IDC_MANUAL));
					theHold.Accept(GetString(IDS_UNDO_PARAM));
					break;
				// The user clicked the daylight savings checkbox.
				case IDC_DST:
					theHold.Begin();
					mc->SetDst(now,IsDlgButtonChecked(mc->hMasterParams,IDC_DST));
					theHold.Accept(GetString(IDS_UNDO_PARAM));
					break;
				case IDC_GETLOC: 
					float lat = mc->GetLat(now);
					float longi = mc->GetLong(now);
					char city[256] = "";
					if (doLocationDialog(hDlg, mc->iObjParams, &lat, &longi, city)) {
						int tz = getTimeZone(longi);
						theHold.Begin();
						mc->SetCity(city);		// Set this first so updates triggered by
						mc->SetLat(now,lat);	// setting other values will update the UI.
						mc->SetLong(now,longi);
						mc->SetZone(now,tz);
						theHold.Put(new CityRestore(mc, holdLastName, holdLastCity));
						theHold.Accept(GetString(IDS_UNDO_PARAM));
						// set these copies
						holdLastCity = lastCity;
						strcpy(holdLastName, city);
					}
					break;
				}
			if (mc->theObject) mc->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
			else {
				mc->calculate(now,FOREVER);
				mc->UpdateUI(now);
			}
			mc->iObjParams->RedrawViews(now, REDRAW_INTERACTIVE, mc);
			return TRUE;

		default:
			return FALSE;
		}
	}



// This method is called when the sun masters parameters may be edited
// in the motion branch of the command panel.  
void SunMaster::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	// Save the interface pointer passed in.  This pointer is only valid
	// between BeginEditParams() and EndEditParams().
	iObjParams = ip;
	inCreate = (flags & BEGIN_EDIT_CREATE) != 0;

	if ( !hMasterParams  ) {
		// Add the rollup page to the command panel. This method sets the
		// dialog proc used to manage the user interaction with the dialog
		// controls.


		hMasterParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(daylightSystem ? IDD_DAYPARAM : IDD_SUNPARAM),
				MasterParamDialogProc,
				GetString(IDS_SUN_DLG_NAM), 
				(LPARAM)this
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
				, 0,
				(ROLLUP_CAT_STANDARD + 2 * ROLLUP_CAT_CUSTATTRIB) / 3
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
				);
		
	} else {
		SetWindowLongPtr( hMasterParams, GWLP_USERDATA, (LONG_PTR)this );		
/*
		// Init the dialog to our values.
		radSpin->SetValue(GetRad(ip->GetTime()),FALSE);
		latSpin->SetValue(GetLat(ip->GetTime()),FALSE);
		longSpin->SetValue(GetLong(ip->GetTime()),FALSE);
		hourSpin->SetValue(GetHour(ip->GetTime()),FALSE);
		minSpin->SetValue(GetMin(ip->GetTime()),FALSE);
		secSpin->SetValue(GetSec(ip->GetTime()),FALSE);
		monthSpin->SetValue(GetMon(ip->GetTime()),FALSE);
		daySpin->SetValue(GetDay(ip->GetTime()),FALSE);
		yearSpin->SetValue(GetYr(ip->GetTime()),FALSE);
		BOOL temp = GetDst(ip->GetTime());
		CheckDlgButton(hMasterParams,IDC_DST,temp );
		*/
		}
	//JH 11/20/00
	//FIx for 136634 Commented out the manual updates to the UI and just call update Ui in both cases.
	UpdateUI(ip->GetTime());
	
	}
		
// This method is called when the user is finished editing the sun masters
// parameters in the command panel.		
void SunMaster::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next ){

	if (hMasterParams==NULL) 
		return;

	// Check if the rollup page should be removed.
/*	if ( flags&END_EDIT_REMOVEUI ){ 		

		BOOL keepit = FALSE;
		if(next){
			Class_ID ncid = next->ClassID();
			keepit = (Class_ID(SLAVE_CONTROL_CID1,SLAVE_CONTROL_CID2) == ncid)?TRUE:FALSE;
		}
		if (keepit){
	//		SetWindowLongPtr( hMasterParams, GWLP_USERDATA, 0 );
			SlaveControl* sc = (SlaveControl *)next;
			SetWindowLongPtr( hMasterParams, GWLP_USERDATA, (LONG)(sc->master) );
			SunMaster* nextsun =(SunMaster*) sc->master;
			nextsun->hMasterParams = hMasterParams;
		}else if(hMasterParams)
			ip->DeleteRollupPage(hMasterParams);
	}*/

	if ( flags&END_EDIT_REMOVEUI ) {		
		ip->UnRegisterDlgWnd(hMasterParams);
		ip->DeleteRollupPage(hMasterParams);
		hMasterParams = NULL;
		}
	else {		
		SetWindowLongPtr( hMasterParams, GWLP_USERDATA, 0 );
		}


	hMasterParams = NULL;
	iObjParams = NULL;
}

#define CITY_CHUNK		0x120
#define NODE_ID_CHUNK 0x110
#define EPOCH_CHUNK 0x100
#define NEW_ANIM_CHUNK 0xf0

IOResult SunMaster::Save(ISave *isave) {
	ULONG nb;
	IOResult res;
	//Fix for 137204 If there's a city name save it
	isave->BeginChunk(CITY_CHUNK);
		res = isave->WriteCString(city);
		assert(res == IO_OK);
	isave->EndChunk();

	isave->BeginChunk(NODE_ID_CHUNK);
		ULONG id;
		id = isave->GetRefID(theObject);
		isave->Write(&id,sizeof(ULONG), &nb);
	isave->EndChunk();
	isave->BeginChunk(EPOCH_CHUNK);
		isave->Write(&dateref,sizeof(ULONG), &nb);
	isave->EndChunk();

    if (!mbOldAnimation) {
	    isave->BeginChunk(NEW_ANIM_CHUNK);
	    isave->EndChunk();
    }

	return IO_OK;
	}

IParamBlock *SpecialUpdateParameterBlock(
		ParamBlockDescID *pdescOld, int oldCount, IParamBlock *oldPB,
		ParamBlockDescID *pdescNew, int newCount, DWORD newVersion)
	{
	IParamBlock *opb = oldPB;
	IParamBlock *npb = CreateParameterBlock(pdescNew,newCount,newVersion);

	theHold.Suspend();

	for (int i=0; i<oldCount; i++) {
//		int index = FindParam(pdescOld,oldCount,pdescNew[i].id);
		//since the old decsriptor didn't have any ID's we'll just remap them
		// in the naive way, i.e. in the natural order.
		int index = i;
		Control *c = opb->GetController(index);
		if (index>=0) {			
			BOOL oldC, newC;
			oldC = c==NULL;
			newC = TRUE ;//npb->params[i].flags & CONSTANT;

			if (newC) {				
				npb->SetController(i, NULL);	
			} else {
				assert(0); // this should never happen 'cause we just made this parameter block!
				}

			if (oldC) {
//				npb->SetValue(i, TimeValue(0), opb->GetValue(index));
				Interval ivalid = FOREVER;
				Point3 pt;
				int k;
				float f;

				switch (opb->GetParameterType(index)) {
					case TYPE_RGBA:
					case TYPE_POINT3:
						opb->GetValue(index, TimeValue(0), pt, ivalid);
						npb->SetValue(i, TimeValue(0), pt);
						break;

					case TYPE_BOOL:
					case TYPE_INT:
						opb->GetValue(index, TimeValue(0), k, ivalid);
						npb->SetValue(i, TimeValue(0), k);
						break;

					case TYPE_FLOAT:
						opb->GetValue(index, TimeValue(0), f, ivalid);
						npb->SetValue(i, TimeValue(0), f);
						break;

					default:
						assert(0);
						break;
					}
					
			} else {
				npb->SetController(i,c); 
				c->MakeReference(FOREVER,npb);
				}
//			npb->params[i].flags = opb->params[index].flags;			
			}
		}
	
	theHold.Resume();
	return npb;
	}


class SunMasterPostLoad : public PostLoadCallback {
public:
    SunMaster *bo;
    SunMasterPostLoad(SunMaster *b) {bo=b;}
	void replaceSunlightWithDaylight();
    void proc(ILoad *iload) {
		int sunMasterVersion = bo->daylightSystem
			? SUNMASTER_VERSION_DAYLIGHT : SUNMASTER_VERSION_SUNLIGHT;
		int nParams = bo->daylightSystem ? 9 : 7;
		ParamBlockDescID* pDesc = bo->daylightSystem ? desc3Daylight : desc1;
        if (bo->pblock->GetVersion() != sunMasterVersion) {
            switch (bo->pblock->GetVersion()) {
            case 0:
                bo->ReplaceReference(REF_PARAM,
                                     UpdateParameterBlock(
                                         desc, 7, bo->pblock,
                                         pDesc, nParams, sunMasterVersion));
				bo->SetZone(TimeValue(0),-1 * bo->GetZone(TimeValue(0)));
                iload->SetObsolete();
                break;

            case 1:
                bo->ReplaceReference(REF_PARAM,
                                     UpdateParameterBlock(
                                         desc1, 7, bo->pblock,
                                         pDesc, nParams, sunMasterVersion));
                iload->SetObsolete();
                break;

            case 2:
                bo->ReplaceReference(REF_PARAM,
                                     UpdateParameterBlock(
                                         desc2Daylight, 8, bo->pblock,
                                         desc3Daylight, 9, SUNMASTER_VERSION_DAYLIGHT));
				bo->daylightSystem = true;
                iload->SetObsolete();
                break;

            case 3:
				bo->daylightSystem = true;
                iload->SetObsolete();
                break;

            default:
                assert(0);
                break;
            }
        }
//        waitPostLoad--;
        delete this;
    }
};


IOResult SunMaster::Load(ILoad *iload) {
	ULONG nb;
	TCHAR	*cp;
	IOResult res;

    mbOldAnimation = TRUE;
	iload->RegisterPostLoadCallback(new SunMasterPostLoad(this));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CITY_CHUNK:
					res = iload->ReadCStringChunk(&cp);
					if (res == IO_OK)
						SetCity(cp);
				break;
			case NODE_ID_CHUNK:
					ULONG id;
					iload->Read(&id,sizeof(ULONG), &nb);
					if (id!=0xffffffff)
						iload->RecordBackpatch(id,(void**)&theObject);
				break;
			case EPOCH_CHUNK:
					iload->Read(&dateref,sizeof(LONG), &nb);
					if(dateref != 0)
						refset = TRUE;
				break;
			case NEW_ANIM_CHUNK:
					mbOldAnimation = FALSE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	tvalid = NEVER;

	return IO_OK;
	}

void SunMaster::TimeChanged(TimeValue t)
{
	UpdateUI(t);
}

/*===========================================================================*\
 | Sun Slave Controller  Methods:
\*===========================================================================*/


// This method returns a new instance of the slave controller.
Control* GetNewSlaveControl(SunMaster *master, int i) {
	return new SlaveControl(master,i);
	}

// Constructor.  
SlaveControl::SlaveControl(const SlaveControl& ctrl) {
	daylightSystem = ctrl.daylightSystem;
	master = ctrl.master;
	id = ctrl.id;
	}

void SlaveControl::GetClassName(TSTR& s)
{
   s = GetString(id == LIGHT_TM ? IDS_SLAVE_POS_CLASS : IDS_SLAVE_FLOAT_CLASS);
}

// This constructor creates a reference from the slave controller to 
// the sun master object.
SlaveControl::SlaveControl(const SunMaster* m, int i) {
	daylightSystem = m->daylightSystem;
	id = i;
    MakeRefByID( FOREVER, 0, (ReferenceTarget *)m);
	}

SClass_ID SlaveControl::SuperClassID()
{
	switch (id) {
	case LIGHT_TM:
		return CTRL_POSITION_CLASS_ID;
	}
	return CTRL_FLOAT_CLASS_ID;
}

// This method is called to create a copy of the slave controller.
RefTargetHandle SlaveControl::Clone(RemapDir& remap) {
	SlaveControl *sl = new SlaveControl(daylightSystem);
	sl->id = id;
	sl->ReplaceReference(0, remap.CloneRef(master));
	BaseClone(this, sl, remap);
	return sl;
	}

SlaveControl& SlaveControl::operator=(const SlaveControl& ctrl) {
	daylightSystem = ctrl.daylightSystem;
	master = ctrl.master;
	id = ctrl.id;
	return (*this);
	}

// ========= This method is used to retrieve the value of the 
// controller at the specified time. =========

// This is a important aspect of the system plug-in - this method 
// calls the master object to get the value.

void SlaveControl::GetValue(TimeValue t, void *val, Interval &valid, 
	GetSetMethod method) {
	// Ensure the ring master exists.
	assert(master);
	master->GetValue(t,val,valid,method,id);	
	}

void SlaveControl::SetValue(TimeValue t, void *val, int commit, 
	GetSetMethod method)
{
	master->SetValue(t, val, commit, method, id);
	}

void* SlaveControl::GetInterface(ULONG id) {
	if (id==I_MASTER) 
		return (void *)master;
	else 
		return NULL;
	}

const int kSlaveIDChunk = 0x4444;
IOResult SlaveControl::Save(ISave *isave)
{
	isave->BeginChunk(kSlaveIDChunk);
	ULONG nb;
	IOResult res = isave->Write(&id, sizeof(id), &nb);
	if (res == IO_OK && nb != sizeof(id))
		res = IO_ERROR;
	isave->EndChunk();
	return res;
}

class SlaveControlPostLoad : public PostLoadCallback {
public:
	SlaveControlPostLoad(SlaveControl* slave) : mpSlave(slave) {}

	void proc(ILoad* iload) {
		if (mpSlave->master != NULL) {
			if (mpSlave->daylightSystem != mpSlave->master->daylightSystem) {
				mpSlave->daylightSystem = mpSlave->master->daylightSystem;
				iload->SetObsolete();
			}
		}
		delete this;
	}

private:
	SlaveControl*	mpSlave;
};

IOResult SlaveControl::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;

	iload->RegisterPostLoadCallback(new SlaveControlPostLoad(this));

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
		case kSlaveIDChunk:
			res = iload->Read(&id, sizeof(id), &nb);
			if (res != IO_OK && nb != sizeof(id))
				res = IO_ERROR;
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	return IO_OK;
}

// These are the API methods
void SlaveControl::GetSunTime( TimeValue t, SYSTEMTIME&	sunt ){
	master->GetTime(t,FOREVER);
	int hours = master->GetZone(t,FOREVER);
	if(master->GetDst(t,FOREVER)) hours--;

	
	sunt.wYear = master->theTime.wYear;
	sunt.wMonth = master->theTime.wMonth;
	sunt.wDay = master->theTime.wDay;

	sunt.wHour = (master->theTime.wHour + hours)%24;
	sunt.wMinute = master->theTime.wMinute;
	sunt.wSecond = master->theTime.wSecond;
}


 void SlaveControl::GetSunLoc(TimeValue t, Point2& origin, Point2& orient){

//	master.GetTime(t, FOREVER);
//	master.Calculate(t,FOREVER);
	
	origin.x = master->GetLat(t, FOREVER);
	origin.y = master->GetLong(t, FOREVER);
	Point2 temp =master->GetAzAlt(t,FOREVER); 
	orient.x = temp.x;
	orient.y = temp.y;
}


/*===========================================================================*\
 | SunMasterCreationManager Class:
\*===========================================================================*/
// This is the class used to manage the creation process of the ring array
// in the 3D viewports.
class SunMasterCreationManager : public MouseCallBack, ReferenceMaker {
	private:
		CreateMouseCallBack *createCB;	
		INode *node0;
		// This holds a pointer to the SunMaster object.  This is used
		// to call methods on the sun  master such as BeginEditParams() and
		// EndEditParams() which bracket the editing of the UI parameters.
		SunMaster *theMaster;
		// This holds the interface pointer for calling methods provided
		// by MAX.
		IObjCreate *createInterface;

		ClassDesc *cDesc;
		// This holds the nodes TM relative to the CP
		Matrix3 mat;
		// This holds the initial mouse point the user clicked when
		// creating the ring array.
		IPoint2 pt0;
		// This holds the center point of the ring array
		Point3 center;
		// This flag indicates the dummy nodes have been hooked up to
		// the master node and the entire system has been created.
		BOOL attachedToNode;
		bool daylightSystem;

		// This method is called to create a new SunMaster object (theMaster)
		// and begin the editing of the systems user interface parameters.
		void CreateNewMaster(HWND rollup);
			
		// This flag is used to catch the reference message that is sent
		// when the system plug-in itself selects a node in the scene.
		// When the user does this, the plug-in recieves a reference message
		// that it needs to respond to.
		int ignoreSelectionChange;

		// --- Inherited virtual methods of ReferenceMaker ---
		// This returns the number of references this item has.
		// It has a single reference to the first created node in the scene.
		int NumRefs() { return 1; }
		// This methods retrieves the ith reference.
		RefTargetHandle GetReference(int i);
		// This methods stores the ith reference.
		void SetReference(int i, RefTargetHandle rtarg);

		// This method recieves the change notification messages sent
		// when the the reference target changes.
	    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);

	public:
		// This method is called just before the creation command mode is
		// pushed on the command stack.
		void Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight );
		// This method is called after the creation command mode is finished 
		// and is about to be popped from the command stack.
		void End();
		
		// Constructor.
		SunMasterCreationManager()
			{
			ignoreSelectionChange = FALSE;
			}
		// --- Inherited virtual methods from MouseCallBack
		// This is the method that handles the user / mouse interaction
		// when the system plug-in is being created in the viewports.
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		bool IsActive() const { return theMaster != NULL; }
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
	};

/*===========================================================================*\
 | SunMasterCreateMode Class:
\*===========================================================================*/
#define CID_RINGCREATE	CID_USER + 0x509C2DF4
// This is the command mode that manages the overall process when 
// the system is created.  
// See the Advanced Topics section on Command Modes and Mouse Procs for 
// more details on these methods.
class SunMasterCreateMode : public CommandMode {
	// This instance of SunMasterCreationMangaer handles the user/mouse
	// interaction as the sun array is created.
	IObjCreate *ip;
	SunMasterCreationManager proc;
	public:
		// These two methods just call the creation proc method of the same
		// name. 
		// This creates a new sun master object and starts the editing
		// of the objects parameters.  This is called just before the 
		// command mode is pushed on the stack to begin the creation
		// process.
		void Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight ) 
			{ 
			ip=ioc;
			proc.Begin( ioc, desc, daylight ); }
		// This terminates the editing of the sun masters parameters.
		// This is called just before the command mode is removed from
		// the command stack.
		void End() { proc.End(); }
		// This returns the type of command mode this is.  See the online
		// help under this method for a list of the available choices.
		// In this case we are a creation command mode.
		int Class() { return CREATE_COMMAND; }
		// Returns the ID of the command mode. This value should be the 
		// constant CID_USER plus some random value chosen by the developer.
		int ID() { return CID_RINGCREATE; }
		// This method returns a pointer to the mouse proc that will
		// handle the user/mouse interaction.  It also establishes the number 
		// of points that may be accepted by the mouse proc.  In this case
		// we set the number of points to 100000.  The user process will 
		// terminate before this (!) after the mouse proc returns FALSE.
		// The mouse proc returned from this method is an instance of
		// SunMasterCreationManager.  Note that that class is derived
		// from MouseCallBack.
		MouseCallBack *MouseProc(int *numPoints) 
			{ *numPoints = 100000; return &proc; }
		// This method is called to flag nodes in the foreground plane.
		// We just return the standard CHANGE_FG_SELECTED value to indicate
		// that selected nodes will go into the foreground.  This allows
		// the system to speed up screen redraws.  See the Advanced Topics
		// section on Foreground / Background planes for more details.
		ChangeForegroundCallback *ChangeFGProc() 
			{ return CHANGE_FG_SELECTED; }
		// This method returns TRUE if the command mode needs to change the
		// foreground proc (using ChangeFGProc()) and FALSE if it does not. 
		BOOL ChangeFG( CommandMode *oldMode ) 
			{ return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		// This method is called when a command mode becomes active.  We
		// don't need to do anything at this time so our implementation is NULL
		void EnterMode() {
			SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
			ip->PushPrompt( GetString(IDS_SUN_CREATE_PROMPT));
		}
		// This method is called when the command mode is replaced by 
		// another mode.  Again, we don't need to do anything.
		void ExitMode() {
			ip->PopPrompt();
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		BOOL IsSticky() { return FALSE; }
		
#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		bool IsActive() const { return proc.IsActive(); }
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
		};

// A static instance of the command mode.
static SunMasterCreateMode theSunMasterCreateMode;

#if defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)
bool SunMaster::IsCreateModeActive()
{
	return theSunMasterCreateMode.IsActive();
}
#endif	// defined(NO_MOTION_PANEL) || defined(NO_DAYLIGHT_MOTION_PANEL)

// This initializes a few variables, creates a new sun master object and 
// starts the editing of the objects parameters.
void SunMasterCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight )
	{
	// This just initializes the variables.
	createInterface = ioc;
	cDesc           = desc;	 //class descriptor of the master controller
	createCB        = NULL;
	node0			= NULL;
	theMaster 		= NULL;
	attachedToNode = FALSE;
	daylightSystem  = daylight;
	// This creates a new sun master object and starts the editing
	// of the objects parameters.
	CreateNewMaster(NULL);
	}

// This method sets the ith reference.  We have only one - to the first
// node created in the sun system.
void SunMasterCreationManager::SetReference(int i, RefTargetHandle rtarg) { 
	switch(i) {
		case 0: node0 = (INode *)rtarg; break;
		default: assert(0); 
		}
	}

// This method returns the ith node.  We have only one - to the first node
// created in the ring array.
RefTargetHandle SunMasterCreationManager::GetReference(int i) { 
	switch(i) {
		case 0: return (RefTargetHandle)node0;
		default: assert(0); 
		}
	return NULL;
	}

//SunMasterCreationManager::~SunMasterCreationManager
void SunMasterCreationManager::End()
	{
	if (theMaster) {
#ifndef NO_CREATE_TASK
		theMaster->EndEditParams( (IObjParam*)createInterface, 
	                    	          TRUE/*destroy*/, NULL );
#endif
		if ( !attachedToNode ) {
			delete theMaster;
			theMaster = NULL;
		} else if ( node0 ) {
			 // Get rid of the references.
			DeleteAllRefsFromMe();
			}
		}
	theMaster = NULL;
	}

// This method is used to recieve change notification messages from the 
// item that is referenced - the first created node in the system.
RefResult SunMasterCreationManager::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
	{
	switch (message) {
		case REFMSG_TARGET_SELECTIONCHANGE:
			// This method is sent if the reference target (node0) is
			// selected or deselected.
			// This is used so that if the user deselects this slave node
			// the creation process will begin with a new item (and not 
			// continue to edit the existing item if the UI contorls are 
			// adjusted)
		 	if ( ignoreSelectionChange ) {
				// We select a node ourseleves when the creation process
				// is finished - this just lets us ignore this message
				// in that case.
				break;
				}
		 	if (theMaster) {
				// This will set node0 ==NULL;
				DeleteAllRefsFromMe();
				goto endEdit;
				}
			else 
				return REF_SUCCEED;
			// Fall through

		case REFMSG_TARGET_DELETED:
			if (theMaster) {
				endEdit:
				// This ends the parameter editing process.
				HWND temp = theMaster->hMasterParams;
#ifndef NO_CREATE_TASK
				theMaster->EndEditParams( (IObjParam*)createInterface, FALSE/*destroy*/,NULL );
#endif
				theMaster = NULL;
				node0 = NULL;
				// This creates a new SunMaster object and starts
				// the parameter editing process.
				CreateNewMaster(temp);	
				// Indicate that no nodes have been attached yet - 
				// no system created...
				attachedToNode = FALSE;
				}
			break;		
		}
	return REF_SUCCEED;
	}

// This method is called to create a new sun master object and 
// begin the parameter editing process.
void SunMasterCreationManager::CreateNewMaster(HWND rollup)
	{
	// Allocate a new instance of the SunMaster object.
	theMaster = new SunMaster(daylightSystem);

#ifndef NO_CREATE_TASK
	//Possibly point to an existing dialog rollup
	theMaster->hMasterParams = rollup;
#endif

#ifdef GLOBAL_COORD_SYS
	APPLICATION_ID appid = GetAppID();
	if (appid == kAPP_VIZ || appid == kAPP_VIZR)
	{
		IGcsSession * pSession = NULL;
		HRESULT hr = ::CoCreateInstance(CLSID_GcsSession,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IGcsSession,
			reinterpret_cast<void **>(&pSession));
		DbgAssert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			ICsCoordsysDef * pSys = NULL;
			pSession->GetGlobalCoordSystem(&pSys);
			if (pSys)
			{
#ifndef NO_CREATE_TASK
				theMaster->iObjParams = (IObjParam*)createInterface;
#endif	// NO_CREATE_TASK
				double lat, log;
				pSys->GetOriginLatitude(&lat);
				pSys->GetOriginLongitude(&log);

				//SS 2/12/2002: Sometimes we aren't getting a lat & long; see if we
				// get any other data out of the coordsys def.
				#ifdef _DEBUG
				BSTR loc, country, datum;
				pSys->GetLocation(&loc);
				pSys->GetCntrySt(&country);
				BOOL isgeo;
				pSys->IsGeodetic(&isgeo);
				pSys->GetDatumName(&datum);
				double xoff, yoff;
				pSys->GetOffsets(&xoff, &yoff);
				double lonmin, latmin, lonmax, latmax;
				pSys->GetLatLongBounds(&lonmin, &latmin, &lonmax, &latmax);
				short quad;
				pSys->GetQuadrant(&quad);
				#endif

				if (!(lat == 0.0 && log == 0.0))
				{
					theMaster->SetLat(TimeValue(0), lat);
					theMaster->SetLong(TimeValue(0), -log);
				}
				if (theMaster->GetLat(0) != SF_LAT || theMaster->GetLong(0) != SF_LONG)
				{
					theMaster->SetCity("");
					theMaster->SetZone(TimeValue(0), -(theMaster->GetLong(0) / 15));
				}
#ifndef NO_CREATE_TASK
				theMaster->iObjParams = NULL;
#endif	// NO_CREATE_TASK
				pSys->Release();
			}
			pSession->Release();
		}
	}
#endif
	
#ifndef NO_CREATE_TASK
	// Start the edit parameters process
	theMaster->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE,NULL );
#endif
	}


// This is the method of MouseCallBack that is used to handle the user/mouse
// interaction during the creation process of the system plug-in.
int SunMasterCreationManager::proc( 
				HWND hwnd,
				int msg,
				int point,
				int flag,
				IPoint2 m )
	{
	// This is set to TRUE if we should keep processing and FALSE
	// if the message processing should end.
	int res;
	// This is the helper object at the center of the system.
	static INode *dummyNode = NULL;
	// This is the radius of the sun system.
	float r;

	// The two objects we create here.
	//static HelperObject* compassObj = NULL;
	static CompassRoseObject* compassObj = NULL;
	static GenLight* lightObj = NULL;
	static NatLightAssembly* natLightObj = NULL;

	createInterface->GetMacroRecorder()->Disable();  // JBW, disable for now; systems not creatable in MAXScript

	// Attempt to get the viewport interface from the window
	// handle passed
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );
	TimeValue t = createInterface->GetTime();
	// Set the cursor
	SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));


	// Process the mouse message sent...
	switch ( msg ) {
		case MOUSE_POINT:
		case MOUSE_MOVE: {
			switch (point) {
				case 0: {
					// Mouse Down: This point defines the center of the sun system.
					pt0 = m;
					// Make sure the master object exists
					assert(theMaster);

					// Reset our pointers.
					compassObj = NULL;
					lightObj = NULL;
					natLightObj = NULL;
					dummyNode = NULL;

					mat.IdentityMatrix();
					if ( createInterface->SetActiveViewport(hwnd) ) {
						createInterface->GetMacroRecorder()->Enable();  
						return FALSE;
						}

					// Make sure the view in the viewport is not parallel
					// to the creation plane - if it is the mouse points
					// clicked by the user cannot be converted to 3D points.
					if (createInterface->IsCPEdgeOnInView()) {
						createInterface->GetMacroRecorder()->Enable();  
						return FALSE;
						}

					if ( attachedToNode) {

						// As an experiment we won't allow any other to be created.
						//createInterface->PushPrompt( GetString(IDS_SUN_COMPLETE_PROMPT));
						//return FALSE;

						// A previous sun system exists - terminate the editing
						// of it's parameters and send it on it's way...
						// Hang on to the last one's handle to the rollup
						HWND temp = theMaster->hMasterParams;

#ifndef NO_CREATE_TASK
				   		theMaster->EndEditParams( (IObjParam*)createInterface,0,NULL );
#endif
						// Get rid of the references.  This sets node0 = NULL.
						DeleteAllRefsFromMe();

						// This creates a new SunMaster object (theMaster)
						// and starts the parameter editing process.
						CreateNewMaster(temp);
						}

					// Begin hold for undo
				   	theHold.Begin();

	 				createInterface->RedrawViews(t, REDRAW_BEGIN);

					// Snap the inital mouse point
					//mat.IdentityMatrix();
					center = vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
					mat.SetTrans(center);

					// Create the compass (a helper).
					compassObj = (CompassRoseObject*)createInterface->
							CreateInstance(HELPER_CLASS_ID, COMPASS_CLASS_ID); 			
					assert(compassObj);
					dummyNode = createInterface->CreateObjectNode(compassObj);
					createInterface->SetNodeTMRelConstPlane(dummyNode, mat);
					if (theMaster->daylightSystem) {
						// Force the compass to be created in the XY Plane with
						// up in the +Z direction.
						TimeValue t = createInterface->GetTime();
						Matrix3 tm(1);
						tm.SetTrans(dummyNode->GetNodeTM(t).GetTrans());
						dummyNode->SetNodeTM(t, tm);
					}
					createInterface->RedrawViews(t, REDRAW_INTERACTIVE);

					res = TRUE;
					break;
					}

				case 1: {
					// Mouse Drag: Set on-screen size of the compass
					Point3 p1 = vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
					compassObj->axisLength = Length(p1 - center);
					compassObj->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);

					if (msg == MOUSE_POINT) {

						INode* newNode;
						if (theMaster->daylightSystem) {
							// Create the NaturalLightAssembly
							newNode = NatLightAssembly::CreateAssembly(
								createInterface, natLightObj);
							assert(natLightObj);
							assert (newNode);

							natLightObj->SetMultController(new SlaveControl(theMaster,LIGHT_MULT));
							natLightObj->SetIntenseController(new SlaveControl(theMaster,LIGHT_INTENSE));
							natLightObj->SetSkyCondController(new SlaveControl(theMaster,LIGHT_SKY));
						} else {
							// Create the Light
							lightObj = (GenLight *)createInterface->
									CreateInstance(LIGHT_CLASS_ID,Class_ID(DIR_LIGHT_CLASS_ID,0));
							assert(lightObj);
							SetUpSun(SYSTEM_CLASS_ID, Class_ID(SUNLIGHT_CID1,SUNLIGHT_CID2),
								createInterface, lightObj, SUN_RGB);

							// Here we'd like to assign a controller to the intensity 
							// parameter, but the pblock is not exposed, so we set the 
							// color controller.
							//SlaveControl* multslave = new SlaveControl(theMaster,LIGHT_MULT);
							//lightObj->pblock->SetController(1,multslave);

							// Create a new node given the instance.
							newNode = createInterface->CreateObjectNode(lightObj);
							assert (newNode);
							createInterface->AddLightToScene(newNode);
							TSTR nodename = GetString(IDS_LIGHT_NAME);
							createInterface->MakeNameUnique(nodename);
							newNode->SetName(nodename);

							// We are no longer creating a camera at the light location.
							//
							//// Create a Camera and attach it to the light 
							//GenCamera *cob = (GenCamera *)createInterface->
							//	CreateInstance(CAMERA_CLASS_ID,Class_ID(SIMPLE_CAM_CLASS_ID,0));
							//assert(cob);
							//cob->Enable(1);
							//// Create a new node given the instance.
							//INode *camNode = createInterface->CreateObjectNode(cob);
							//camNode->Hide(TRUE);
							//newNode->AttachChild(camNode);
							//nodename = GetString(IDS_CAM_NAME);
							//createInterface->MakeNameUnique(nodename);
							//camNode->SetName(nodename);
						}

						// Create a new slave controller of the master control.
						SlaveControl* slave = new SlaveControl(theMaster,LIGHT_TM);
						// Set the transform controller used by the node.
						newNode->SetTMController(slave);
						// Attach the new node as a child of the central node.
						dummyNode->AttachChild(newNode);
						theMaster->theObject=newNode;
						theMaster->thePoint=dummyNode;
						//the master references the point so it can get
						//notified when the	user rotates it
						theMaster->MakeRefByID(FOREVER, REF_POINT, dummyNode );
						// Indicate that the system is attached
						attachedToNode = TRUE;

						if (daylightSystem) {
							theMaster->MakeRefByID(FOREVER, REF_LOOKAT, static_cast<Control*>(
								CreateInstance(CTRL_MATRIX3_CLASS_ID, Class_ID(PRS_CONTROL_CLASS_ID,0))));
							Control* lookat = static_cast<Control*>(
								CreateInstance(CTRL_ROTATION_CLASS_ID, Class_ID(LOOKAT_CONSTRAINT_CLASS_ID, 0)));
							assert(lookat != NULL);
							Animatable* a = lookat;
							ILookAtConstRotation* ilookat = GetILookAtConstInterface(a);
							assert(ilookat != NULL);
							ilookat->AppendTarget(theMaster->thePoint);
							ilookat->SetTargetAxis(2);
							ilookat->SetTargetAxisFlip(true);
							ilookat->SetVLisAbs(false);
							theMaster->theLookat->SetRotationController(lookat);

							theMaster->MakeRefByID(FOREVER, REF_MULTIPLIER, static_cast<Control*>(
								CreateInstance(CTRL_FLOAT_CLASS_ID, Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))));
						}

						// Reference the node so we'll get notifications.
						// This is done so when the node is selected or 
						// deselected the parameter editing process can 
						// be started and stopped.
					    MakeRefByID( FOREVER, 0, theMaster->theObject );
						// Set the initial radius of the system to one
						theMaster->SetRad(TimeValue(0),1.0f);
						//mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
						// construction plane.  This is used during creating
						// so you can set the position of the node in terms of 
						// the construction plane and not in world units.
						createInterface->SetNodeTMRelConstPlane(dummyNode, mat);
						if (theMaster->daylightSystem) {
							// Force the compass to be created in the XY Plane with
							// up in the +Z direction.
							TimeValue t = createInterface->GetTime();
							Matrix3 tm(1);
							tm.SetTrans(dummyNode->GetNodeTM(t).GetTrans());
							dummyNode->SetNodeTM(t, tm);
						}
						//theMaster->align_north();
					}

	 				createInterface->RedrawViews(t, REDRAW_INTERACTIVE);
					// This indicates the mouse proc should continue to recieve
					// messages.
					res = TRUE;
					break;
					}

				case 2: {
					// Mouse Pick and release: 
					// The user is dragging the mouse to set the radius
					// (i.e. the distance of the light from the compass center)
					if (node0) {
						// Calculate the radius as the distance from the initial
						// point to the current point
						r = (float)fabs(vpx->SnapLength(vpx->GetCPDisp(center,Point3(0,1,0),pt0,m)));
						// Set the radius at time 0.
						theMaster->SetRad(0, r);
						if (daylightSystem) {
							Object* l = natLightObj->GetSun();
							if (l != NULL && l->SuperClassID() == LIGHT_CLASS_ID)
								static_cast<LightObject*>(l)->SetTDist(t, r);
						} else {
							lightObj->SetTDist(t, r);
						}
						theMaster->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
#ifndef NO_CREATE_TASK
						// Update the UI spinner to reflect the current value.
						theMaster->radSpin->SetValue(r, FALSE );
#endif
					}

					res = TRUE;
					if (msg == MOUSE_POINT) {
						// The mouse has been released - finish the creation 
						// process.  Select the first node so if we go into 
						// the motion branch we'll see it's parameters.

						// We set the flag to ignore the selection change
						// because the NotifyRefChanged() method recieves
						// a message when the node is selected or deselected
						// and terminates the parameter editing process.  This
						// flag causes it to ignore the selection change.
						ignoreSelectionChange = TRUE;
					   	createInterface->SelectNode( theMaster->theObject );
						if (daylightSystem) {
							inCreate = false;
							theMaster->UpdateUI(t);
						}
						ignoreSelectionChange = FALSE;
						// This registers the undo object with the system so
						// the user may undo the creation.
						theHold.Accept(GetString(IDS_UNDO));
						// Set the return value to FALSE to indicate the 
						// creation process is finished.  This mouse proc will
						// no longer be called until the user creates another
						// system.
						res = FALSE;
						}
	 				createInterface->RedrawViews(t, res ? REDRAW_INTERACTIVE : REDRAW_END);
					break;
					}
				}
			}
			break;

	    case MOUSE_PROPCLICK:
			// right click while between creations
			createInterface->RemoveMode(NULL);
			break;
		
		case MOUSE_ABORT:
			// The user has right-clicked the mouse to abort the 
			// creation process.
			assert(theMaster);
			// End the parameter editing process
			HWND temp = theMaster->hMasterParams;
#ifndef NO_CREATE_TASK
			theMaster->EndEditParams( (IObjParam*)createInterface, 0,NULL );
#endif
			theHold.Cancel();  // undo the changes
			// Update the viewports
			createInterface->RedrawViews(t, REDRAW_NORMAL);
			DeleteAllRefsFromMe();
			// This creates a new SunMaster object and starts
			// the parameter editing process.
			CreateNewMaster(temp);	
			// Indicate the nodes have not been
			// hooked up to the system yet.
			attachedToNode = FALSE;
			// Sets the return value to indicate that mouse processing
			// should be stopped.
			res = FALSE;						
			break;
		}
	
	createInterface->ReleaseViewport(vpx); 
	// Returns TRUE if processing should continue and false if it
	// has been aborted.
	createInterface->GetMacroRecorder()->Enable();  
	return res;
	}

/*===========================================================================*\
 | The Class Descriptors
\*===========================================================================*/
class SunMasterClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { APPLICATION_ID id = GetAppID(); return id != kAPP_VIZ && id != kAPP_VIZR; }
	void *			Create(BOOL loading = FALSE) { return new SunMaster(false); }
	// This method returns the name of the class.  This name appears 
	// in the button for the plug-in in the MAX user interface.
	const TCHAR *	ClassName() {
		return GetString(IDS_SUN_CLASS);
	}
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	SClass_ID		SuperClassID() { return SYSTEM_CLASS_ID; } 
	Class_ID		ClassID() 
		{ return Class_ID(SUNLIGHT_CID1,SUNLIGHT_CID2); }
	const TCHAR* 	Category() { return _T("");  }
	};
// A single instance of the class descriptor.
static SunMasterClassDesc mcDesc;
// This returns a pointer to the instance.
ClassDesc* GetSunMasterDesc() { return &mcDesc; }

class DayMasterClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SunMaster(true); }
	// This method returns the name of the class.  This name appears 
	// in the button for the plug-in in the MAX user interface.
	const TCHAR *	ClassName() {
		return GetString(IDS_DAY_CLASS);
	}
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	SClass_ID		SuperClassID() { return SYSTEM_CLASS_ID; } 
	Class_ID		ClassID() 
		{ return Class_ID(DAYLIGHT_CID1,DAYLIGHT_CID2); }
	const TCHAR* 	Category() { return _T("");  }
	};
// A single instance of the class descriptor.
static DayMasterClassDesc mcDayDesc;
// This returns a pointer to the instance.
ClassDesc* GetDayMasterDesc() { return &mcDayDesc; }

class SlaveControlPosClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new SlaveControl(false); }
	const TCHAR *	ClassName() {
		return GetString(IDS_SLAVE_POS_CLASS);
	}
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() 
		{ return Class_ID(SLAVE_CONTROL_CID1,SLAVE_CONTROL_CID2); }
	// The slave controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* 	Category() { return _T("");  }
	};


class SlaveControlFloatClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new SlaveControl(false); }
	const TCHAR *	ClassName() {
		return GetString(IDS_SLAVE_FLOAT_CLASS);
	}
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() 
		{ return Class_ID(SLAVE_CONTROL_CID1,SLAVE_CONTROL_CID2); }
	// The slave controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* 	Category() { return _T("");  }
	};


class DaySlaveControlPosClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new SlaveControl(true); }
	const TCHAR *	ClassName() {
		return GetString(IDS_SLAVE_POS_CLASS);
	}
	SClass_ID		SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	Class_ID		ClassID() 
		{ return Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1,DAYLIGHT_SLAVE_CONTROL_CID2); }
	// The slave controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* 	Category() { return _T("");  }
	};


class DaySlaveControlFloatClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading = FALSE) { return new SlaveControl(true); }
	const TCHAR *	ClassName() {
		return GetString(IDS_SLAVE_FLOAT_CLASS);
	}
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() 
		{ return Class_ID(DAYLIGHT_SLAVE_CONTROL_CID1,DAYLIGHT_SLAVE_CONTROL_CID2); }
	// The slave controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* 	Category() { return _T("");  }
	};

// A single instance of the class descriptor.
static SlaveControlPosClassDesc slvPosDesc;
// This returns a pointer to the instance.
ClassDesc* GetSlavePosControlDesc() { return &slvPosDesc; }

// A single instance of the class descriptor.
static SlaveControlFloatClassDesc slvFloatDesc;
// This returns a pointer to the instance.
ClassDesc* GetSlaveFloatControlDesc() { return &slvFloatDesc; }

// A single instance of the class descriptor.
static DaySlaveControlPosClassDesc daySlvPosDesc;
// This returns a pointer to the instance.
ClassDesc* GetDaySlavePosControlDesc() { return &daySlvPosDesc; }

// A single instance of the class descriptor.
static DaySlaveControlFloatClassDesc daySlvFloatDesc;
// This returns a pointer to the instance.
ClassDesc* GetDaySlaveFloatControlDesc() { return &daySlvFloatDesc; }

// This is the method of the class descriptor that actually begins the 
// creation process in the viewports.
int SunMasterClassDesc::BeginCreate(Interface *i)
	{
	// Save the interface pointer passed in.  This is used to call 
	// methods provided by MAX itself.
	IObjCreate *iob = i->GetIObjCreate();
	
	theSunMasterCreateMode.Begin( iob, this, false );
	// Set the current command mode to the SunMasterCreateMode.
	iob->PushCommandMode( &theSunMasterCreateMode );
	
	return TRUE;
	}

// This is the method of the class descriptor that terminates the 
// creation process.
int SunMasterClassDesc::EndCreate(Interface *i)
	{
	theSunMasterCreateMode.End();
	// Remove the command mode from the command stack.
	i->RemoveMode( &theSunMasterCreateMode );
	return TRUE;
	}

// This is the method of the class descriptor that actually begins the 
// creation process in the viewports.
int DayMasterClassDesc::BeginCreate(Interface *i)
	{
	// Save the interface pointer passed in.  This is used to call 
	// methods provided by MAX itself.
	IObjCreate *iob = i->GetIObjCreate();
	
	theSunMasterCreateMode.Begin( iob, this, true );
	// Set the current command mode to the SunMasterCreateMode.
	iob->PushCommandMode( &theSunMasterCreateMode );
	
	return TRUE;
	}

// This is the method of the class descriptor that terminates the 
// creation process.
int DayMasterClassDesc::EndCreate(Interface *i)
	{
	theSunMasterCreateMode.End();
	// Remove the command mode from the command stack.
	i->RemoveMode( &theSunMasterCreateMode );
	return TRUE;
	}

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;

	if (! controlsInit) {
		controlsInit = TRUE;
		
		// Initialize MAX's custom controls
		InitCustomControls(hInstance);
		
		// Initialize Win95 controls
		InitCommonControls();
	}
	
	return(TRUE);
	}

__declspec(dllexport) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESC); 
	}

__declspec( dllexport ) int 
LibNumberClasses()
{
if (Get3DSMAXVersion() < 120)
return 0;

return 8;
}



__declspec(dllexport) ClassDesc* 
LibClassDesc(int i) { 
	switch(i) {
		case 0:	return GetSunMasterDesc();
		case 1:	return GetDayMasterDesc();
		case 2:	return GetSlavePosControlDesc();
		case 3:	return GetSlaveFloatControlDesc();
		case 4:	return GetDaySlavePosControlDesc();
		case 5:	return GetDaySlaveFloatControlDesc();
		case 6:	return GetCompassRoseDesc();
		case 7: return GetNatLightAssemblyDesc();
		default: return 0;
	};
}

__declspec( dllexport ) ULONG
LibVersion() {  return VERSION_3DSMAX; }


TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
