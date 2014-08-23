/*****************************************************************************
 *<
	FILE: solids.cpp

	DESCRIPTION: Extended Primitives Support Files

	CREATED BY:  Audrey Peterson
	Copyright (c) 1996 All Rights Reserved
 *>
 *****************************************************************************/
#include "solids.h"
#include "buildver.h"

HINSTANCE hInstance;
static int controlsInit = FALSE;

// russom - 11/19/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

#define MAX_PRIM_OBJECTS	13
static ClassDesc *classDescArray[MAX_PRIM_OBJECTS];
static int classDescCount = 0;

void initClassDescArray(void)
{
	classDescArray[classDescCount++] = GetChBoxobjDesc();
	classDescArray[classDescCount++] = GetChCylinderDesc();
	classDescArray[classDescCount++] = GetOilTnkDesc();
	classDescArray[classDescCount++] = GetSpindleDesc();
#ifndef NO_OBJECT_CAPSULE
	classDescArray[classDescCount++] = GetScubaDesc();
#endif
	classDescArray[classDescCount++] = GetGengonDesc();
#ifndef NO_OBJECT_PRISM
	classDescArray[classDescCount++] = GetPrismDesc();
#endif
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
	classDescArray[classDescCount++] = GetPyramidDesc();
#endif
#ifndef DESIGN_VER
	classDescArray[classDescCount++] = GetCExtDesc();
	classDescArray[classDescCount++] = GetLextDesc();
 #ifndef NO_OBJECT_SPRING	// russom - 11/19/01
	classDescArray[classDescCount++] = GetSpringDesc();
 #endif
 #ifndef NO_OBJECT_DAMPER	// russom - 11/19/01
	classDescArray[classDescCount++] = GetDamperDesc();
 #endif
	classDescArray[classDescCount++] = GetHoseDesc();
#endif	// DESIGN_VER

}

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------
TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_AP_SCSLIB); }

// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int 
LibNumberClasses() { return classDescCount; }

// This function return the ith class descriptor. We have one.
__declspec( dllexport ) ClassDesc* 
LibClassDesc(int i) {
	if( i < classDescCount )
		return classDescArray[i];
	else
		return NULL;
 }

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;

  	if (!controlsInit) {
		controlsInit = TRUE;		

		// russom - 11/19/01
		initClassDescArray();

	// Initialize MAX's custom controls
	InitCustomControls(hInstance);
		
	// Initialize Win95 controls
	InitCommonControls();
	}
	
	return(TRUE);
	}


// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

__declspec( dllexport ) ULONG 
CanAutoDefer() { return 1; }


void AddFace(Face *f,int a,int b,int c,int evis,int smooth_group)
{ f[0].setSmGroup(smooth_group);
  f[0].setMatID((MtlID)0); 	 /*default */
  if (evis==0) f[0].setEdgeVisFlags(1,1,0);
  else if (evis==1) f[0].setEdgeVisFlags(0,1,1);
  else if (evis==2) f[0].setEdgeVisFlags(0,0,1);
  else if (evis==ALLF) f[0].setEdgeVisFlags(1,1,1);
  else f[0].setEdgeVisFlags(1,0,1);	
  f[0].setVerts(a,b,c);
}

void TurnButton(HWND hWnd,int SpinNum,BOOL ison)
{	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(hWnd,SpinNum));
	if (iBut) 
	{ if (ison) iBut->Enable(); else iBut->Disable();
	}
	ReleaseICustButton(iBut);
};

float RND01()
{ float num;

  num=(float)rand();
  return(num/IntMax);
}

float RND11()
{ float num;

   num=(float)rand()-HalfIntMax;
   return(num/HalfIntMax);
}

float GetMeterMult()
{	int type;
	float scale;
	GetMasterUnitInfo(&type,&scale);
	switch(type)
	{	case UNITS_INCHES:		return scale*0.0254f;
		case UNITS_FEET:		return scale*0.3048f;
		case UNITS_MILES:		return scale*1609.3f;
		case UNITS_MILLIMETERS:	return scale*0.001f;
		case UNITS_CENTIMETERS:	return scale*0.01f;
		case UNITS_METERS:		return scale;
		case UNITS_KILOMETERS:	return scale*1000.0f;	
		default:				return 0;
	}
}

void Mult1X4(float *A,Matrix4By4 B,float *C)
{
   C[0]=A[0]*B[0][0]+A[1]*B[1][0]+A[2]*B[2][0]+A[3]*B[3][0];
   C[1]=A[0]*B[0][1]+A[1]*B[1][1]+A[2]*B[2][1]+A[3]*B[3][1];
   C[2]=A[0]*B[0][2]+A[1]*B[1][2]+A[2]*B[2][2]+A[3]*B[3][2];
   C[3]=A[0]*B[0][3]+A[1]*B[1][3]+A[2]*B[2][3]+A[3]*B[3][3];
}

void Mult1X3(float *A,Matrix3By3 B,float *C)
{
   C[0]=A[0]*B[0][0]+A[1]*B[1][0]+A[2]*B[2][0];
   C[1]=A[0]*B[0][1]+A[1]*B[1][1]+A[2]*B[2][1];
   C[2]=A[0]*B[0][2]+A[1]*B[1][2]+A[2]*B[2][2];
}

void SetUpRotation(float *Q, float *W,float Theta,Matrix4By4 Rq)
{ float ww1,ww2,ww3,w12,w13,w23,CosTheta,SinTheta,MinCosTheta;
  Point3 temp;
  Matrix3By3 R;

 ww1=W[0]*W[0];ww2=W[1]*W[1];ww3=W[2]*W[2];
 w12=W[0]*W[1];w13=W[0]*W[2];w23=W[1]*W[2];
 CosTheta=(float)cos(Theta);MinCosTheta=1.0f-CosTheta;SinTheta=(float)sin(Theta);
 R[0][0]=ww1+(1.0f-ww1)*CosTheta;
 R[0][1]=w12*MinCosTheta+W[2]*SinTheta;
 R[0][2]=w13*MinCosTheta-W[1]*SinTheta;
 R[1][0]=w12*MinCosTheta-W[2]*SinTheta;
 R[1][1]=ww2+(1.0f-ww2)*CosTheta;
 R[1][2]=w23*MinCosTheta+W[0]*SinTheta;
 R[2][0]=w13*MinCosTheta+W[1]*SinTheta;
 R[2][1]=w23*MinCosTheta-W[0]*SinTheta;
 R[2][2]=ww3+(1.0f-ww3)*CosTheta;
 Mult1X3(Q,R,&temp.x);
 memcpy(Rq[0],R[0],row3size);
 memcpy(Rq[1],R[1],row3size);
 memcpy(Rq[2],R[2],row3size);
 Rq[3][0]=Q[0]-temp.x;Rq[3][1]=Q[1]-temp.y;Rq[3][2]=Q[2]-temp.z;
 Rq[0][3]=Rq[1][3]=Rq[2][3]=0.0f;Rq[3][3]=1.0f;
}

void RotateOnePoint(float *Pin,float *Q, float *W,float Theta)
{ Matrix4By4 Rq;
  float Pout[4],Pby4[4];

 SetUpRotation(Q,W,Theta,Rq);
 memcpy(Pby4,Pin,row3size);Pby4[3]=1.0f;
 Mult1X4(Pby4,Rq,Pout);
 memcpy(Pin,Pout,row3size);
}
void SpinnerOff(HWND hWnd,int SpinNum)
{	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
	spin2->Disable();
	ReleaseISpinner(spin2);
}
void SpinnerOn(HWND hWnd,int SpinNum)
{	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
	spin2->Enable();
	ReleaseISpinner(spin2);
}
void FixFSpinnerLimits(HWND hWnd,int SpinNum,float min,float max,BOOL notify)
{	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
	spin2->SetLimits(min, max, notify);
	ReleaseISpinner(spin2);
}


