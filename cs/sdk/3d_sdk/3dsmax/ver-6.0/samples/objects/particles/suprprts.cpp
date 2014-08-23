/**********************************************************************
 *<
	FILE: suprprts.cpp

	DESCRIPTION: Blizzard, Particle Array, and SuperSpray  Support Files

	CREATED BY: Audrey Peterson

	HISTORY: 12/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "SuprPrts.h"

HINSTANCE hInstance;
static int controlsInit = FALSE;

// russom - 10/11/01
// The classDesc array was created because it was way to difficult
// to maintain the switch statement return the Class Descriptors
// with so many different #define used in this module.

#define MAX_PARTICLE_OBJECTS 12
ClassDesc *classDescArray[MAX_PARTICLE_OBJECTS];
int classDescCount = 0;

void initClassDescArray(void)
{
	classDescArray[classDescCount++] = GetPBombObjDesc();
	classDescArray[classDescCount++] = GetPBombModDesc();
	classDescArray[classDescCount++] = GetSphereDefDesc();
	classDescArray[classDescCount++] = GetSphereDefModDesc();
#ifndef NO_PARTICLES_SUPERSPRAY
	classDescArray[classDescCount++] = GetSuprSprayDesc();
#endif
#ifndef NO_PARTICLES_BLIZZARD
	classDescArray[classDescCount++] = GetBlizzardDesc();
#endif
#ifndef NO_PARTICLES_PARRAY
	classDescArray[classDescCount++] = GetPArrayDesc();
#endif
	classDescArray[classDescCount++] = GetPFollowDesc();
	classDescArray[classDescCount++] = GetPFollowModDesc();
#ifndef NO_PARTICLES_PCLOUD
	classDescArray[classDescCount++] = GetPCloudDesc();
#endif
	classDescArray[classDescCount++] = GetUniDefDesc();
	classDescArray[classDescCount++] = GetUniDefModDesc();
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
LibDescription() { return GetString(IDS_AP_SPRTSLIB); }

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

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

/* rand returns a number between 0 and 32767 */
/* number between 0 and 1 */
const float IntMax=32767.0f;
const float IntMax1=32768.0f;
const float HalfIntMax=16383.5f;
typedef float Matrix3By3[3][3];
typedef float Matrix4By3[4][3];

int FloatEQ0(float number)
{return((FLOAT_EPSILON>=number)&&(number>=-FLOAT_EPSILON));
}
int SmallerEQ0(float number)
{return((SMALL_EPSILON>=number)&&(SMALL_EPSILON>=-FLOAT_EPSILON));
}
int FGT0(Point3 p1)
{
	return((fabs(p1[0])>SMALL_EPSILON)||(fabs(p1[1])>SMALL_EPSILON)||(fabs(p1[2])>SMALL_EPSILON));
}

#define EPSILON 0.00001f

float sign(float sval)
{	return(sval>=0.0f?1.0f:-1.0f);
}

void MakeInterpRotXform(Matrix3 InTmBegin,Matrix3 InTmEnd,float portion,Matrix3& OutTm)
{	InTmBegin.NoTrans();
	InTmEnd.NoTrans();
	if (portion<EPSILON) {OutTm=InTmBegin; return;}
	else if ((fabs(1.0f-portion))<EPSILON) {OutTm=InTmEnd; return;}
	Matrix3 TmBetween=Inverse(InTmBegin)*InTmEnd;
	Matrix3 InterpTm;
	Point3 N,O,A;
	float theta,bigtheta;
	N=TmBetween.GetRow(0);
	O=TmBetween.GetRow(1);
	A=TmBetween.GetRow(2);
	float Nx=N.x,Ny=N.y,Nz=N.z;
	float Ox=O.x,Oy=O.y,Oz=O.z;
	float Ax=A.x,Ay=A.y,Az=A.z;
	float Kx,Ky,Kz;
	bigtheta=(float)atan(sqrt((Ox-Ay)*(Oz-Ay)+(Ax-Nz)*(Ax-Nz)+(Ny-Ox)*(Ny-Ox))/(Nx+Oy+Az-1));
	if (bigtheta<EPSILON) {Kx=1.0f;Ky=Kz=0.0f;}
	else if (bigtheta<HALFPI)
	{	float stheta2=2.0f*(float)sin(bigtheta);
		Kx=(Oz-Ay)/stheta2;Ky=(Ax-Nz)/stheta2;Kz=(Ny-Ox)/stheta2;
	}
	else if (bigtheta<PI)
	{	float costh=(float)cos(bigtheta),costh1=1.0f-costh,costh12=2.0f*costh1;
		Kx=sign(Oz-Ay)*(float)sqrt((Nx-costh)/costh1);
		Ky=sign(Ax-Nz)*(float)sqrt((Oy-costh)/costh1);
		Kz=sign(Ny-Ox)*(float)sqrt((Az-costh)/costh1);
		if ((Kx>Ky)&&(Kx>Kz))
		{	float xcos=Kx*costh12;
			Ky=(Ny+Ox)/xcos;
			Kz=(Ax+Nz)/xcos;
		}
		else if (Ky>Kz)
		{	float ycos=Ky*costh12;
			Kx=(Ny+Ox)/ycos;
			Kz=(Oz+Ay)/ycos;
		}
		else 
		{	float zcos=Kz*costh12;
			Ky=(Oz+Ay)/zcos;
			Kx=(Ax+Nz)/zcos;
		}
	}
	theta=portion*bigtheta;
	float kx2=Kx*Kx,ky2=Ky*Ky,kz2=Kz*Kz;
	float kxky=Kx*Ky,kxkz=Kx*Kz,kykz=Ky*Kz;
	float ctheta=(float)cos(theta);
	float stheta=(float)sin(theta);
	float vtheta=1.0f-ctheta;
	float kx2vtheta=kx2*vtheta,ky2vtheta=ky2*vtheta,kz2vtheta=kz2*vtheta;
	float kxkyvtheta=kxky*vtheta,kxkzvtheta=kxkz*vtheta,kykzvtheta=kykz*vtheta;
	float kxstheta=Kx*stheta,kystheta=Ky*stheta,kzstheta=Kz*stheta;
	float W11=kx2vtheta+ctheta,W12=kxkyvtheta-kzstheta,W13=kxkzvtheta+kystheta;
	float W21=kxkyvtheta+kzstheta,W22=ky2vtheta+ctheta,W23=kykzvtheta-kxstheta;
	float W31=kxkzvtheta-kystheta,W32=kykzvtheta+kxstheta,W33=kz2vtheta+ctheta;
	InterpTm.SetRow(0,Point3(W11,W21,W31));
	InterpTm.SetRow(1,Point3(W12,W22,W32));
	InterpTm.SetRow(2,Point3(W13,W23,W33));
	InterpTm.SetRow(3,Point3(0.0f,0.0f,0.0f));
	OutTm=InTmBegin*InterpTm;
}

void Mult1X4(float *A,Matrix4By4 B,float *C)
{
   C[0]=A[0]*B[0][0]+A[1]*B[1][0]+A[2]*B[2][0]+A[3]*B[3][0];
   C[1]=A[0]*B[0][1]+A[1]*B[1][1]+A[2]*B[2][1]+A[3]*B[3][1];
   C[2]=A[0]*B[0][2]+A[1]*B[1][2]+A[2]*B[2][2]+A[3]*B[3][2];
   C[3]=A[0]*B[0][3]+A[1]*B[1][3]+A[2]*B[2][3]+A[3]*B[3][3];
}

void Mult4X1(float *A,Matrix4By4 B,float *C)
{
   C[0]=A[0]*B[0][0]+A[1]*B[0][1]+A[2]*B[0][2]+A[3]*B[0][3];
   C[1]=A[0]*B[1][0]+A[1]*B[1][1]+A[2]*B[1][2]+A[3]*B[1][3];
   C[2]=A[0]*B[2][0]+A[1]*B[2][1]+A[2]*B[2][2]+A[3]*B[2][3];
   C[3]=A[0]*B[3][0]+A[1]*B[3][1]+A[2]*B[3][2]+A[3]*B[3][3];
}

void Mult1X3(float *A,Matrix3By3 B,float *C)
{
   C[0]=A[0]*B[0][0]+A[1]*B[1][0]+A[2]*B[2][0];
   C[1]=A[0]*B[0][1]+A[1]*B[1][1]+A[2]*B[2][1];
   C[2]=A[0]*B[0][2]+A[1]*B[1][2]+A[2]*B[2][2];
}

void Mult3X4(Matrix3By4 A,Matrix4By4 B,Matrix3By4 C)
{
   C[0][0]=A[0][0]*B[0][0]+A[0][1]*B[1][0]+A[0][2]*B[2][0]+A[0][3]*B[3][0];
   C[0][1]=A[0][0]*B[0][1]+A[0][1]*B[1][1]+A[0][2]*B[2][1]+A[0][3]*B[3][1];
   C[0][2]=A[0][0]*B[0][2]+A[0][1]*B[1][2]+A[0][2]*B[2][2]+A[0][3]*B[3][2];
   C[0][3]=A[0][0]*B[0][3]+A[0][1]*B[1][3]+A[0][2]*B[2][3]+A[0][3]*B[3][3];
   C[1][0]=A[1][0]*B[0][0]+A[1][1]*B[1][0]+A[1][2]*B[2][0]+A[1][3]*B[3][0];
   C[1][1]=A[1][0]*B[0][1]+A[1][1]*B[1][1]+A[1][2]*B[2][1]+A[1][3]*B[3][1];
   C[1][2]=A[1][0]*B[0][2]+A[1][1]*B[1][2]+A[1][2]*B[2][2]+A[1][3]*B[3][2];
   C[1][3]=A[1][0]*B[0][3]+A[1][1]*B[1][3]+A[1][2]*B[2][3]+A[1][3]*B[3][3];
   C[2][0]=A[2][0]*B[0][0]+A[2][1]*B[1][0]+A[2][2]*B[2][0]+A[2][3]*B[3][0];
   C[2][1]=A[2][0]*B[0][1]+A[2][1]*B[1][1]+A[2][2]*B[2][1]+A[2][3]*B[3][1];
   C[2][2]=A[2][0]*B[0][2]+A[2][1]*B[1][2]+A[2][2]*B[2][2]+A[2][3]*B[3][2];
   C[2][3]=A[2][0]*B[0][3]+A[2][1]*B[1][3]+A[2][2]*B[2][3]+A[2][3]*B[3][3];
}

void Mult4X3(Matrix4By3 A,Matrix4By4 B,Matrix4By3 C)
{
   C[0][0]=A[0][0]*B[0][0]+A[1][0]*B[0][1]+A[2][0]*B[0][2]+A[3][0]*B[0][3];
   C[1][0]=A[0][0]*B[1][0]+A[1][0]*B[1][1]+A[2][0]*B[1][2]+A[3][0]*B[1][3];
   C[2][0]=A[0][0]*B[2][0]+A[1][0]*B[2][1]+A[2][0]*B[2][2]+A[3][0]*B[2][3];
   C[3][0]=A[0][0]*B[3][0]+A[1][0]*B[3][1]+A[2][0]*B[3][2]+A[3][0]*B[3][3];
   C[0][1]=A[0][1]*B[0][0]+A[1][1]*B[0][1]+A[2][1]*B[0][2]+A[3][1]*B[0][3];
   C[1][1]=A[0][1]*B[1][0]+A[1][1]*B[1][1]+A[2][1]*B[1][2]+A[3][1]*B[1][3];
   C[2][1]=A[0][1]*B[2][0]+A[1][1]*B[2][1]+A[2][1]*B[2][2]+A[3][1]*B[2][3];
   C[3][1]=A[0][1]*B[3][0]+A[1][1]*B[3][1]+A[2][1]*B[3][2]+A[3][1]*B[3][3];
   C[0][2]=A[0][2]*B[0][0]+A[1][2]*B[0][1]+A[2][2]*B[0][2]+A[3][2]*B[0][3];
   C[1][2]=A[0][2]*B[1][0]+A[1][2]*B[1][1]+A[2][2]*B[1][2]+A[3][2]*B[1][3];
   C[2][2]=A[0][2]*B[2][0]+A[1][2]*B[2][1]+A[2][2]*B[2][2]+A[3][2]*B[2][3];
   C[3][2]=A[0][2]*B[3][0]+A[1][2]*B[3][1]+A[2][2]*B[3][2]+A[3][2]*B[3][3];
}

void Mult4X4(Matrix4By4 A,Matrix4By4 B,Matrix4By4 C)
{
   C[0][0]=A[0][0]*B[0][0]+A[0][1]*B[1][0]+A[0][2]*B[2][0]+A[0][3]*B[3][0];
   C[0][1]=A[0][0]*B[0][1]+A[0][1]*B[1][1]+A[0][2]*B[2][1]+A[0][3]*B[3][1];
   C[0][2]=A[0][0]*B[0][2]+A[0][1]*B[1][2]+A[0][2]*B[2][2]+A[0][3]*B[3][2];
   C[0][3]=A[0][0]*B[0][3]+A[0][1]*B[1][3]+A[0][2]*B[2][3]+A[0][3]*B[3][3];
   C[1][0]=A[1][0]*B[0][0]+A[1][1]*B[1][0]+A[1][2]*B[2][0]+A[1][3]*B[3][0];
   C[1][1]=A[1][0]*B[0][1]+A[1][1]*B[1][1]+A[1][2]*B[2][1]+A[1][3]*B[3][1];
   C[1][2]=A[1][0]*B[0][2]+A[1][1]*B[1][2]+A[1][2]*B[2][2]+A[1][3]*B[3][2];
   C[1][3]=A[1][0]*B[0][3]+A[1][1]*B[1][3]+A[1][2]*B[2][3]+A[1][3]*B[3][3];
   C[2][0]=A[2][0]*B[0][0]+A[2][1]*B[1][0]+A[2][2]*B[2][0]+A[2][3]*B[3][0];
   C[2][1]=A[2][0]*B[0][1]+A[2][1]*B[1][1]+A[2][2]*B[2][1]+A[2][3]*B[3][1];
   C[2][2]=A[2][0]*B[0][2]+A[2][1]*B[1][2]+A[2][2]*B[2][2]+A[2][3]*B[3][2];
   C[2][3]=A[2][0]*B[0][3]+A[2][1]*B[1][3]+A[2][2]*B[2][3]+A[2][3]*B[3][3];
   C[3][0]=A[3][0]*B[0][0]+A[3][1]*B[1][0]+A[3][2]*B[2][0]+A[3][3]*B[3][0];
   C[3][1]=A[3][0]*B[0][1]+A[3][1]*B[1][1]+A[3][2]*B[2][1]+A[3][3]*B[3][1];
   C[3][2]=A[3][0]*B[0][2]+A[3][1]*B[1][2]+A[3][2]*B[2][2]+A[3][3]*B[3][2];
   C[3][3]=A[3][0]*B[0][3]+A[3][1]*B[1][3]+A[3][2]*B[2][3]+A[3][3]*B[3][3];
}

float det2x2(float a,float b,float c,float d)
{ return(a*d-b*c);
}
float det3x3(float a1,float a2,float a3,float b1,float b2,float b3,float c1,float c2,float c3)
{ return(a1*det2x2(b2,b3,c2,c3)-b1*det2x2(a2,a3,c2,c3)+c1*det2x2(a2,a3,b2,b3));
}

void Adjoint(Matrix4By4 in, Matrix4By4 out,float det)
{float a1,a2,a3,a4,b1,b2,b3,b4;
 float c1,c2,c3,c4,d1,d2,d3,d4;

 a1=in[0][0];b1=in[0][1];c1=in[0][2];d1=in[0][3];
 a2=in[1][0];b2=in[1][1];c2=in[1][2];d2=in[1][3];
 a3=in[2][0];b3=in[2][1];c3=in[2][2];d3=in[2][3];
 a4=in[3][0];b4=in[3][1];c4=in[3][2];d4=in[3][3];
 out[0][0]= det3x3(b2,b3,b4,c2,c3,c4,d2,d3,d4)/det;
 out[1][0]=-det3x3(a2,a3,a4,c2,c3,c4,d2,d3,d4)/det;
 out[2][0]= det3x3(a2,a3,a4,b2,b3,b4,d2,d3,d4)/det;
 out[3][0]=-det3x3(a2,a3,a4,b2,b3,b4,c2,c3,c4)/det;
 out[0][1]=-det3x3(b1,b3,b4,c1,c3,c4,d1,d3,d4)/det;
 out[1][1]= det3x3(a1,a3,a4,c1,c3,c4,d1,d3,d4)/det;
 out[2][1]=-det3x3(a1,a3,a4,b1,b3,b4,d1,d3,d4)/det;
 out[3][1]= det3x3(a1,a3,a4,b1,b3,b4,c1,c3,c4)/det;
 out[0][2]= det3x3(b1,b2,b4,c1,c2,c4,d1,d2,d4)/det;
 out[1][2]=-det3x3(a1,a2,a4,c1,c2,c4,d1,d2,d4)/det;
 out[2][2]= det3x3(a1,a2,a4,b1,b2,b4,d1,d2,d4)/det;
 out[3][2]=-det3x3(a1,a2,a4,b1,b2,b4,c1,c2,c4)/det;
 out[0][3]=-det3x3(b1,b2,b3,c1,c2,c3,d1,d2,d3)/det;
 out[1][3]= det3x3(a1,a2,a3,c1,c2,c3,d1,d2,d3)/det;
 out[2][3]=-det3x3(a1,a2,a3,b1,b2,b3,d1,d2,d3)/det;
 out[3][3]= det3x3(a1,a2,a3,b1,b2,b3,c1,c2,c3)/det;
}

float det4x4(Matrix4By4 m)
{float a1,a2,a3,a4,b1,b2,b3,b4;
 float c1,c2,c3,c4,d1,d2,d3,d4,ans;

 a1=m[0][0];b1=m[0][1];c1=m[0][2];d1=m[0][3];
 a2=m[1][0];b2=m[1][1];c2=m[1][2];d2=m[1][3];
 a3=m[2][0];b3=m[2][1];c3=m[2][2];d3=m[2][3];
 a4=m[3][0];b4=m[3][1];c4=m[3][2];d4=m[3][3];
 ans= a1*det3x3(b2,b3,b4,c2,c3,c4,d2,d3,d4)
     -b1*det3x3(a2,a3,a4,c2,c3,c4,d2,d3,d4)
     +c1*det3x3(a2,a3,a4,b2,b3,b4,d2,d3,d4)
     -d1*det3x3(a2,a3,a4,b2,b3,b4,c2,c3,c4);
 return(ans);
}

int MatrixInvert(Matrix4By4 in,Matrix4By4 out)
{ float det;

  det=det4x4(in);
  if (fabs(det)<PRECISION_LIMIT)  /* NO INVERSE */
    return(0);
  Adjoint(in,out,det);
  return(1);
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
 memcpy(Rq[0],R[0],row3size);memcpy(Rq[1],R[1],row3size);memcpy(Rq[2],R[2],row3size);
 Rq[3][0]=Q[0]-temp.x;Rq[3][1]=Q[1]-temp.y;Rq[3][2]=Q[2]-temp.z;
 Rq[0][3]=Rq[1][3]=Rq[2][3]=0.0f;Rq[3][3]=1.0f;
}

void RotatePoint(Matrix3By4 Pin,float *Q, float *W,float Theta)
{ Matrix3By4 Pout;
  Matrix4By4 Rq;

 SetUpRotation(Q,W,Theta,Rq);
 Mult3X4(Pin,Rq,Pout);
 memcpy(Pin, Pout, sizeof(Matrix3By4));
}

void RotateOnePoint(float *Pin,float *Q, float *W,float Theta)
{ Matrix4By4 Rq;
  float Pout[4],Pby4[4];

 SetUpRotation(Q,W,Theta,Rq);
 memcpy(Pby4,Pin,row3size);Pby4[3]=1.0f;
 Mult1X4(Pby4,Rq,Pout);
 memcpy(Pin,Pout,row3size);
}

float RND01()
{ float num;

  num=(float)rand();
  return(num/IntMax);
}

// number between -1 and 1 
float RND11()
{ float num;

   num=(float)rand()-HalfIntMax;
   return(num/HalfIntMax);
}

int RNDSign()
{
  return((RND11()<0?-1:1));
}

float RND55()
{ float num;

  num=RND11();
  return(num/2);
}
int RND0x(int maxnum)
{ float num;
  int newnum;

   num=(float)rand();
   if (maxnum==0) return(0);
   newnum=(int)floor((++maxnum)*num/IntMax1);
   return(newnum>maxnum?maxnum:newnum);
}


INT_PTR CALLBACK DefaultSOTProc(
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	IObjParam *ip = (IObjParam*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			if (ip) ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
			return FALSE;

		default:
			return FALSE;
		}
	return TRUE;
	}
void TurnButton(HWND hWnd,int SpinNum,BOOL ison)
{	ICustButton *iBut;
	iBut=GetICustButton(GetDlgItem(hWnd,SpinNum));
	if (iBut) 
	{ if (ison) iBut->Enable(); else iBut->Disable();
	}
	ReleaseICustButton(iBut);
};
void SpinnerOn(HWND hWnd,int SpinNum,int Winnum)
{	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
	spin2->Enable();
	EnableWindow(GetDlgItem(hWnd,Winnum),TRUE);
	ReleaseISpinner(spin2);

};
void SpinnerOff(HWND hWnd,int SpinNum,int Winnum)
{	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,SpinNum));
	spin2->Disable();
	EnableWindow(GetDlgItem(hWnd,Winnum),FALSE);
	ReleaseISpinner(spin2);
};

// Note to developer: If any change with the method is made, please copy 
// the change in RandGenerator::CalcSpread(...) method (just copy&paste) (Bayboro)
Point3 CalcSpread(float divangle,Point3 oldnorm)
{ float Q[3];
  Point3 r;

  Q[0]=Q[1]=Q[2]=0.0f;
  // Martell 4/14/01: Fix for order of ops bug.
  float z=RND11(); float y=RND11(); float x=RND11();
  r=Point3(x,y,z);
  r=Normalize(r^oldnorm);
  RotateOnePoint(&oldnorm.x,Q,&r.x,RND01()*divangle);
  return(oldnorm);
}

float FigureOutSize(TimeValue age,float size,TimeValue grow,TimeValue fade,TimeValue life,float grate,float frate)
{ TimeValue timeleft=life-age;
  if (timeleft<fade)
   return(size*(timeleft*frate+M));
  else if (age<grow)
   return(size*(age*grate+M));
  else return(size);
}

void AddFace(int a, int b, int c,int face,Mesh *pm,int curmtl)
{ pm->faces[face].setSmGroup(0);
  pm->faces[face].setVerts(a,b,c);
  pm->faces[face].setMatID((MtlID)curmtl);
  pm->faces[face].setEdgeVisFlags(1,1,0);
}

Point3 RotateAboutAxis(float Angle,Point3 C,Point3 L,Point3 W,InDirInfo indir)
{ Point3 V,A,A1,newpt;
  float CAngle=(float)cos(Angle),SAngle=(float)sin(Angle),K,len;

  len=Length(indir.vel);
  if (len <= 0.0f) { 
	  len=1.0f; V=v111; K=1.0f;
  }else { 
	V=indir.vel/len;
	K=((len*indir.oneframe-1.0f)*indir.K+100.0f)/100.0f;
  }
  A=Normalize(W^V);
  A1=Normalize(V^A);
  // K=((len*indir.oneframe-1.0f)*indir.K+100.0f)/100.0f; // included in the "else" case
  newpt=Point3((A*CAngle+A1*SAngle)*L.x+(-A*SAngle+A1*CAngle)*L.y+V*K*L.z)+C;
  return newpt;
}
void PlotSpecial(float radius,int vertexnum,int face,Mesh *pm,float Angle,float *W,int curmtl,Point3 *pt,InDirInfo indir)
{float radius_5,mradius_5,yradius_5,myradius_5,zradius_5,mzradius_5,halfr,AVertex[3];

  AddFace(vertexnum,vertexnum+1,vertexnum+2,face,pm,curmtl);
  AddFace(vertexnum+2,vertexnum+3,vertexnum,++face,pm,curmtl);
  AddFace(vertexnum+4,vertexnum+5,vertexnum+6,++face,pm,curmtl);
  AddFace(vertexnum+6,vertexnum+7,vertexnum+4,++face,pm,curmtl);
  AddFace(vertexnum+8,vertexnum+9,vertexnum+10,++face,pm,curmtl);
  AddFace(vertexnum+10,vertexnum+11,vertexnum+8,++face,pm,curmtl);
  halfr=radius*0.5f;
  if (indir.inaxis)
  {pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(halfr,0.0f,halfr),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(halfr,0.0f,-halfr),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-halfr,0.0f,-halfr),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-halfr,0.0f,halfr),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-halfr,-halfr,0.0f),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(halfr,-halfr,0.0f),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(halfr,halfr,0.0f),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-halfr,halfr,0.0f),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f,-halfr,halfr),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f,-halfr,-halfr),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f,halfr,-halfr),W,indir);
pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f,halfr,halfr),W,indir);
  }
  else
  {
  radius_5=pt->x+halfr;
  mradius_5=pt->x-halfr;
  yradius_5=pt->y+halfr;
  myradius_5=pt->y-halfr;
  zradius_5=pt->z+halfr;
  mzradius_5=pt->z-halfr;
  AVertex[0]=radius_5;AVertex[1]=pt->y;AVertex[2]=zradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=radius_5;AVertex[1]=pt->y;AVertex[2]=mzradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mradius_5;AVertex[1]=pt->y;AVertex[2]=mzradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mradius_5;AVertex[1]=pt->y;AVertex[2]=zradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mradius_5;AVertex[1]=myradius_5;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=radius_5;AVertex[1]=myradius_5;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=radius_5;AVertex[1]=yradius_5;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mradius_5;AVertex[1]=yradius_5;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=pt->x;AVertex[1]=myradius_5;AVertex[2]=zradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=pt->x;AVertex[1]=myradius_5;AVertex[2]=mzradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=pt->x;AVertex[1]=yradius_5;AVertex[2]=mzradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=pt->x;AVertex[1]=yradius_5;AVertex[2]=zradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  }
}

void PlotCube8(float radius,int vertexnum,int face, Mesh *pm, float Angle,float *W,int curmtl,Point3* pt,InDirInfo indir)
{float radius_5,mradius_5,yradius_5,myradius_5,zradius_5,mzradius_5,halfr,AVertex[3];

  halfr=radius*0.5f;
  AddFace(vertexnum+0,vertexnum+1,vertexnum+2,face,pm,curmtl);
  AddFace(vertexnum+2,vertexnum+3,vertexnum,++face,pm,curmtl);
  AddFace(vertexnum+4,vertexnum+5,vertexnum,++face,pm,curmtl);
  AddFace(vertexnum,vertexnum+3,vertexnum+4,++face,pm,curmtl);
  AddFace(vertexnum+5,vertexnum+6,vertexnum+1,++face,pm,curmtl);
  AddFace(vertexnum+1,vertexnum+0,vertexnum+5,++face,pm,curmtl);
  AddFace(vertexnum+6,vertexnum+7,vertexnum+2,++face,pm,curmtl);
  AddFace(vertexnum+2,vertexnum+1,vertexnum+6,++face,pm,curmtl);
  AddFace(vertexnum+7,vertexnum+4,vertexnum+3,++face,pm,curmtl);
  AddFace(vertexnum+3,vertexnum+2,vertexnum+7,++face,pm,curmtl);
  AddFace(vertexnum+5,vertexnum+4,vertexnum+7,++face,pm,curmtl);
  AddFace(vertexnum+7,vertexnum+6,vertexnum+5,++face,pm,curmtl);
  if (indir.inaxis)
  {pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(halfr,halfr,halfr),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(halfr,halfr,-halfr),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-halfr,halfr,-halfr),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-halfr,halfr,halfr),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-halfr,-halfr,halfr),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(halfr,-halfr,halfr),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(halfr,-halfr,-halfr),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-halfr,-halfr,-halfr),W,indir);
  }
  else
  {
  radius_5=pt->x+halfr;
  mradius_5=pt->x-halfr;
  yradius_5=pt->y+halfr;
  myradius_5=pt->y-halfr;
  zradius_5=pt->z+halfr;
  mzradius_5=pt->z-halfr;
  AVertex[0]=radius_5;AVertex[1]=yradius_5;AVertex[2]=zradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=radius_5;AVertex[1]=yradius_5;AVertex[2]=mzradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mradius_5;AVertex[1]=yradius_5;AVertex[2]=mzradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mradius_5;AVertex[1]=yradius_5;AVertex[2]=zradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mradius_5;AVertex[1]=myradius_5;AVertex[2]=zradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=radius_5;AVertex[1]=myradius_5;AVertex[2]=zradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=radius_5;AVertex[1]=myradius_5;AVertex[2]=mzradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mradius_5;AVertex[1]=myradius_5;AVertex[2]=mzradius_5;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  }
}

void Plot6PT(float radius,int vertexnum,int face, Mesh *pm, float Angle,float *W,int curmtl,Point3* pt,InDirInfo indir)
{float halfr,r73,AVertex[3],radius_5,mradius_5,r_r73,mr_r73;

  halfr=radius*0.5f;
  r73=radius*0.73f;
  AddFace(vertexnum+0,vertexnum+1,vertexnum+2,face,pm,curmtl);
  pm->faces[face].setEdgeVisFlags(1,1,1);
  AddFace(vertexnum+3,vertexnum+4,vertexnum+5,++face,pm,curmtl);
  pm->faces[face].setEdgeVisFlags(1,1,1);
  if (indir.inaxis)
  { pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f,-radius,0.0f),W,indir);
 pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(r73,halfr,0.0f),W,indir);
 pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-r73,halfr,0.0f),W,indir);
 pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-r73,-halfr,0.0f),W,indir);
 pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(r73,-halfr,0.0f),W,indir);
 pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f,radius,0.0f),W,indir);
  }
  else
  {
  radius_5=pt->y+halfr;
  mradius_5=pt->y-halfr;
  r_r73=pt->x+r73;
  mr_r73=pt->x-r73;
  AVertex[0]=pt->x;AVertex[1]=pt->y-radius;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=r_r73;AVertex[1]=radius_5;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mr_r73;AVertex[1]=radius_5;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=mr_r73;AVertex[1]=mradius_5;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=r_r73;AVertex[1]=mradius_5;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  AVertex[0]=pt->x;AVertex[1]=pt->y+radius;AVertex[2]=pt->z;
  RotateOnePoint(AVertex,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  }
}

void PlotTet(float radius,int vertexnum,int face, Mesh *pm, float Angle,Point3 W,int curmtl,Point3* pt,InDirInfo indir)
{float AVertex[3],radius_5,r_r73;
 Point3 r1,r2,r3,r4,tmp;
 
  radius=radius/6.0f;
  radius_5=0.5f*radius;
  r_r73=0.732f*radius;
  r1=Point3(radius,0.0f,0.0f);
  r2=Point3(-radius_5,r_r73,0.0f);
  r3=Point3(-radius,-r_r73,0.0f);
  r4=Point3(0.0f,0.0f,-8.0f*radius);
  pm->tvFace[face].setTVerts(0,0,0);  
  pm->tvFace[face+1].setTVerts(0,1,0);  
  pm->tvFace[face+2].setTVerts(0,1,0);  
  pm->tvFace[face+3].setTVerts(0,1,0);  
  AddFace(vertexnum,vertexnum+1,vertexnum+2,face,pm,curmtl);
  pm->faces[face].setEdgeVisFlags(1,1,1);
  AddFace(vertexnum,vertexnum+3,vertexnum+1,++face,pm,curmtl);
  pm->faces[face].setEdgeVisFlags(1,1,1);
  AddFace(vertexnum+1,vertexnum+3,vertexnum+2,++face,pm,curmtl);
  pm->faces[face].setEdgeVisFlags(1,1,1);
  AddFace(vertexnum+2,vertexnum+3,vertexnum,++face,pm,curmtl);
  pm->faces[face].setEdgeVisFlags(1,1,1);
  if (indir.inaxis)
  { pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,r1,W,indir);
	pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,r2,W,indir);
	pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,r3,W,indir);
	pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,r4,W,indir);
  }
  else
  {
  tmp=(*pt)+r1;
  AVertex[0]=tmp.x;AVertex[1]=tmp.y;AVertex[2]=tmp.z;
  RotateOnePoint(AVertex,&pt->x,&W.x,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  tmp=(*pt)+r2;
  AVertex[0]=tmp.x;AVertex[1]=tmp.y;AVertex[2]=tmp.z;
  RotateOnePoint(AVertex,&pt->x,&W.x,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  tmp=(*pt)+r3;
  AVertex[0]=tmp.x;AVertex[1]=tmp.y;AVertex[2]=tmp.z;
  RotateOnePoint(AVertex,&pt->x,&W.x,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  tmp=(*pt)+r4;
  AVertex[0]=tmp.x;AVertex[1]=tmp.y;AVertex[2]=tmp.z;
  RotateOnePoint(AVertex,&pt->x,&W.x,Angle);
  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
  }
}
							 
void PlotTriangle(float radius,int vertexnum,int face, Mesh *pm, float Angle,float *W,int curmtl,Point3 *pt,InDirInfo indir)
{float radius_33,radius_5;
 Matrix3By4 VertexPts;

  radius_33=radius*0.33f;
  radius_5=0.5f*radius;
  AddFace(vertexnum,vertexnum+1,vertexnum+2,face,pm,curmtl);
  pm->faces[face].setEdgeVisFlags(1,1,1);
  if (indir.inaxis)
  {pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(radius_5,-radius_33,0.0f),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f,2*radius_33,0.0f),W,indir);
   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(-radius_5,-radius_33,0.0f),W,indir);
  }
  else
  {
  VertexPts[0][0]=pt->x+radius_5;
  VertexPts[1][0]=pt->x;
  VertexPts[2][0]=pt->x-radius_5;
  VertexPts[1][1]=pt->y+2*radius_33;
  VertexPts[0][1]=VertexPts[2][1]=pt->y-radius_33;
  VertexPts[0][2]=VertexPts[1][2]=VertexPts[2][2]=pt->z;
  VertexPts[0][3]=VertexPts[1][3]=VertexPts[2][3]=1.0f;
  RotatePoint(VertexPts,&pt->x,W,Angle);
  pm->verts[vertexnum++]=Point3(VertexPts[0][0],VertexPts[0][1],VertexPts[0][2]);
  pm->verts[vertexnum++]=Point3(VertexPts[1][0],VertexPts[1][1],VertexPts[1][2]);
  pm->verts[vertexnum++]=Point3(VertexPts[2][0],VertexPts[2][1],VertexPts[2][2]);
  }
}
void GetMeshInfo(int type,int count,Mesh *pm,int *numF,int *numV)
{	if (type==RENDTYPE1) 
	{   pm->setNumFaces(count);
		pm->setNumVerts(count*3);
		pm->setNumTVerts(count);
		pm->setNumTVFaces(count);
		*numV=3;*numF=1;
	}
	else if (type==RENDTYPE2)
	{	pm->setNumFaces(count*12);
		pm->setNumVerts(count*8);
		pm->setNumTVerts(count);
		pm->setNumTVFaces(count*12);
		*numV=8;*numF=12;
	}
	else if (type==RENDTYPE3)
	{	pm->setNumFaces(count*6);
		pm->setNumVerts(count*12);
		pm->setNumTVerts(count);
		pm->setNumTVFaces(count*6);
		*numV=12;*numF=6;
	}
	else if (type==RENDTYPE5)
	{	pm->setNumFaces(count*2);
		pm->setNumVerts(count*4);
		pm->setNumTVerts(count);
		pm->setNumTVFaces(count*2);
		*numV=4;*numF=2;
	}
	else if (type==RENDTYPE6)
	{	pm->setNumFaces(count*10);
		pm->setNumVerts(count*11);
		pm->setNumTVerts(count);
		pm->setNumTVFaces(count*10);
		*numV=11;*numF=10;
	}
	else if (type==RENDTET)
	{ pm->setNumFaces(count*4);
	  pm->setNumVerts(count*4);
	  pm->setNumTVerts(2);
	  pm->setNumTVFaces(count*4);
	  pm->tVerts[0]=Point3(0.0f,0.5f,0.0f);
	  pm->tVerts[1]=Point3(1.0f,0.5f,0.0f);
      *numV=4;*numF=4;
	}
	else if (type==REND6PT)
	{ pm->setNumFaces(count*2);
	  pm->setNumVerts(count*6);
	  pm->setNumTVerts(count);
	  pm->setNumTVFaces(count*2);
      *numV=6;*numF=2;
	}
	else if (type==RENDSPHERE)
	{ int segs=10,rows=(segs/2-1);
	  int nverts = rows * segs + 2;
	  int tnf,nfaces = rows * segs * 2;
	  pm->setNumVerts(count*nverts);
	  pm->setNumFaces(tnf=count*nfaces);
	  pm->setNumTVerts(count);
	  pm->setNumTVFaces(tnf);
      *numV=nverts;*numF=nfaces;
	}
}
void PlotFacing(int type,float radius,int vertexnum,int face,Mesh *pm,float Angle,int curmtl,Point3* pt,Point3 camV,Point3 a,Point3 b)
{ 	Point3 v, v0,v1;
	if (RENDTYPE5==type)
	{  v  = Normalize(camV-(*pt));
	   v0 = Normalize(Point3(0,0,1)^v) * radius;
	   v1 = Normalize(v0^v) * radius;}
	else
	{ float R=Length((*pt)-camV);
	  v0=radius*R*a;
	  v1=radius*R*b;
    }
   pm->verts[vertexnum] = ((*pt)+v0+v1);
   pm->verts[vertexnum+1] = ((*pt)-v0+v1);
   pm->verts[vertexnum+2] = ((*pt)-v0-v1);
   pm->verts[vertexnum+3] = ((*pt)+v0-v1);
   if (type==RENDTYPE5)
   { for (int l=0;l<4;l++)
       RotateOnePoint(pm->verts[vertexnum+l],&pt->x,&v.x,Angle);
     AddFace(vertexnum+3,vertexnum+2,vertexnum+1,face,pm,curmtl);
     AddFace(vertexnum+1,vertexnum,vertexnum+3,face+1,pm,curmtl);
   }
   else
   {AddFace(vertexnum,vertexnum+1,vertexnum+2,face,pm,curmtl);
    AddFace(vertexnum+2,vertexnum+3,vertexnum,face+1,pm,curmtl);
   }
}

void PlotCustom(float radius,int i,int vertexnum,Mesh *pm, float Angle,float *W,Mesh *clst,Point3* pt,int nv,InDirInfo indir)
{ Point3 nextpt,pt1;

  for (int j=0;j<nv;j++)
  if (indir.inaxis)
  {pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,radius*clst->verts[j],W,indir);
  }
  else
  { nextpt=(radius*clst->verts[j])+(*pt);
    RotateOnePoint(&nextpt.x,&pt->x,W,Angle);
    pm->verts[vertexnum++]=nextpt;
  }
}

void PlotSphere(float radius,int vertexnum,int face, Mesh *pm, float Angle,float *W,int curmtl,Point3 *pt,InDirInfo indir)
{	int segs=10;
radius*=0.5f;
	float delta,delta2,alt;float AVertex[3];

	delta  = 2.0f*PI/(float)segs;
	delta2 = delta;
	int rows=(segs/2-1);
	int nverts = rows * segs + 2;
	float startAng = HALFPI;
	int nc,jx,ix;
	// Make top conic cap
	for(ix=1; ix<=segs; ++ix)
	{	nc=(ix==segs)?1:ix+1;
		AddFace(vertexnum,vertexnum+ix,vertexnum+nc,face,pm,curmtl);
		pm->faces[face].setSmGroup(1);
		face++;
	}
	/* Make midsection */
	int na,nb,nd;
	for(ix=1; ix<rows; ++ix)
	{	jx=(ix-1)*segs+1;
		for(int kx=0; kx<segs; ++kx) 
		{	na = jx+kx;
			nb = na+segs;
			nc = (kx==(segs-1))? jx+segs: nb+1;
			nd = (kx==(segs-1))? jx : na+1;	
			AddFace(vertexnum+na,vertexnum+nb,vertexnum+nc,face,pm,curmtl);
			pm->faces[face].setSmGroup(1);
			face++;
			AddFace(vertexnum+na,vertexnum+nc,vertexnum+nd,face,pm,curmtl);
			pm->faces[face].setSmGroup(1);
			face++;
		}
	}
	// Make bottom conic cap
	na = nverts-1;
	jx = (rows-1)*segs+1;
	for(ix=0; ix<segs; ++ix)
	{	nc = ix + jx;
		nb = (ix==segs-1)?jx:nc+1;
		AddFace(vertexnum+na, vertexnum+nb, vertexnum+nc,face,pm,curmtl);
		pm->faces[face].setSmGroup(1);
		face++;
	} face--;

	// Top vertex 
	 if (indir.inaxis)
	   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f, 0.0f,radius),W,indir);
	 else
	 { AVertex[0]=pt->x;AVertex[1]=pt->y;AVertex[2]=pt->z+radius;
       RotateOnePoint(AVertex,&pt->x,W,Angle);
       pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
	 }
	// Middle vertices 
	alt=delta;
	float a,b,c,secrad,secang;
	for(ix=1; ix<=rows; ix++) {		
		a = (float)cos(alt)*radius;		
		secrad = (float)sin(alt)*radius;
		secang = startAng; //0.0f
		for(int jx=0; jx<segs; ++jx) {
			b = (float)cos(secang)*secrad;
			c = (float)sin(secang)*secrad;
			if (indir.inaxis)
			  pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(b,c,a),W,indir);
			else
			{ AVertex[0]=pt->x+b;AVertex[1]=pt->y+c;AVertex[2]=pt->z+a;
			  RotateOnePoint(AVertex,&pt->x,W,Angle);
			  pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
			}
			secang+=delta2;
			}		
		alt+=delta;		
		}
	/* Bottom vertex */
	 if (indir.inaxis)
	   pm->verts[vertexnum++]=RotateAboutAxis(Angle,*pt,Point3(0.0f, 0.0f,-radius),W,indir);
	 else
	 { AVertex[0]=pt->x;AVertex[1]=pt->y;AVertex[2]=pt->z-radius;
       RotateOnePoint(AVertex,&pt->x,W,Angle);
       pm->verts[vertexnum++]=Point3(AVertex[0],AVertex[1],AVertex[2]);
	 }
}
void CacheData(ParticleSys *p0,ParticleSys *p1)
{ p1->SetCount(p0->points.Count(),PARTICLE_VELS|PARTICLE_AGES|PARTICLE_RADIUS|PARTICLE_TENSION);
  if (p0->points.Count()>0)
  {	for (int pc=0;pc<p0->points.Count();pc++)
		p1->points[pc]=p0->points[pc];
	for (pc=0;pc<p0->points.Count();pc++)
		p1->vels[pc]=p0->vels[pc];
    p1->ages.SetCount(p0->ages.Count());
	for (pc=0;pc<p0->ages.Count();pc++)
		p1->ages[pc]=p0->ages[pc];
    p1->radius.SetCount(p0->radius.Count());
	for (pc=0;pc<p0->radius.Count();pc++)
		p1->radius[pc]=p0->radius[pc];
    p1->tension.SetCount(p0->tension.Count());
	for (pc=0;pc<p0->tension.Count();pc++)
		p1->tension[pc]=p0->tension[pc];
  }
}
int TimeFound(TimeLst times,int showframe,int gen)
{ int found=0,n=0,tnums=0;
  if (times.tl.Count()>0) 
  { tnums=times.tl.Count();
    while ((n<tnums)&&(!found))
    { found=((gen==times.tl[n].gennum)&&(showframe==times.tl[n].tl));
	  n++;
	}
	n--;
  } else {found=1;n=0;}
  return (found?n:NoAni);
}

TimeValue GetCurTime(TimeValue showframe,TimeValue ages,int anifr)
{ TimeValue tframe=showframe+ages;
//  if ((tframe>=anifr)&&(anifr!=0)) tframe=tframe % anifr;
  return tframe;
}

// Note to developer: If any change with the method is made, please copy 
// the change in RandGenerator::VectorVar(...) method (just copy&paste) (Bayboro)
void VectorVar(Point3 *vel,float R,float MaxAngle)
{   
  // Martell 4/14/01: Fix for order of ops bug.
  float z=RND11(); float y=RND11(); float x=RND11();
  Point3 X=Point3(x,y,z);
  Point3 c=Normalize(X^*vel);
  float Theta=MaxAngle*R*RND01();
  Point3 zero=Zero;
  RotateOnePoint(&(*vel).x,&zero.x,&c.x,Theta);
}

// Note to developer: If any change with the method is made, please copy 
// the change in RandGenerator::DoSpawnVars(...) method (just copy&paste) (Bayboro)
Point3 DoSpawnVars(SpawnVars spvars,Point3 pv,Point3 holdv,float *radius,Point3 *sW)
{ Point3 vels=holdv;
  if (!FloatEQ0(spvars.dirchaos))
    VectorVar(&vels,spvars.dirchaos,PI);
  float dovar=(spvars.spsign==0?-RND01():(spvars.spsign==1?RND01():RND11()));
  if (spvars.spconst) dovar=(dovar>0.0f?1.0f:-1.0f);
  float tmp=(1+dovar*spvars.spchaos);
  vels*=(tmp<0.0f?0.0f:tmp);
  dovar=(spvars.scsign==0?-RND01():(spvars.scsign==1?RND01():RND11()));
  if (spvars.scconst) dovar=(dovar>0.0f?1.0f:-1.0f);
  tmp=(1.0f+dovar*spvars.scchaos);
  (*radius)*=(tmp<0.0f?0.0f:tmp);
  if (spvars.invel) vels+=pv;
  if (spvars.axisentered==2)
  {	*sW=Normalize(spvars.Axis);
	if (spvars.axisvar>0.0f)
		VectorVar(sW,spvars.axisvar,PI);
  }
  else
  {
	  // Martell 4/14/01: Fix for order of ops bug.
	  float z=RND11(); float y=RND11(); float x=RND11();
	  *sW=Normalize(Point3(x,y,z));
  }
  return vels;
} 

float Smallest(Point3 pmin) {return (pmin.x<pmin.y?(pmin.z<pmin.x?pmin.z:pmin.x):(pmin.z<pmin.y?pmin.z:pmin.y));}
float Largest(Point3 pmax) {return (pmax.x>pmax.y?(pmax.z>pmax.x?pmax.z:pmax.x):(pmax.z>pmax.y?pmax.z:pmax.y));}

TriObject *TriIsUseable(Object *pobj,TimeValue t)
{ 
  	if (pobj->IsSubClassOf(triObjectClassID)) 
      return (TriObject*)pobj;
    else 
	{ if (pobj->CanConvertToType(triObjectClassID)) 
	  	return (TriObject*)pobj->ConvertToType(t,triObjectClassID);			
	}
  return NULL;
}
void SpinStuff(HWND hWnd,BOOL ison,BOOL isphase)
{ if (ison)
 {SpinnerOn(hWnd,IDC_SP_SPINAXISXSPIN,IDC_SP_SPINAXISX);
  SpinnerOn(hWnd,IDC_SP_SPINAXISYSPIN,IDC_SP_SPINAXISY);
  SpinnerOn(hWnd,IDC_SP_SPINAXISZSPIN,IDC_SP_SPINAXISZ);
  SpinnerOn(hWnd,IDC_SP_SPINAXISVARSPIN,IDC_SP_SPINAXISVAR);

 } else
 { SpinnerOff(hWnd,IDC_SP_SPINAXISXSPIN,IDC_SP_SPINAXISX);
   SpinnerOff(hWnd,IDC_SP_SPINAXISYSPIN,IDC_SP_SPINAXISY);
   SpinnerOff(hWnd,IDC_SP_SPINAXISZSPIN,IDC_SP_SPINAXISZ);
   SpinnerOff(hWnd,IDC_SP_SPINAXISVARSPIN,IDC_SP_SPINAXISVAR);
 }
 EnableWindow(GetDlgItem(hWnd,IDC_SP_SPINAXISX_TXT),ison);
 EnableWindow(GetDlgItem(hWnd,IDC_SP_SPINAXISY_TXT),ison);
 EnableWindow(GetDlgItem(hWnd,IDC_SP_SPINAXISZ_TXT),ison);
 EnableWindow(GetDlgItem(hWnd,IDC_SP_SPINAXISVAR_TXT),ison);
 EnableWindow(GetDlgItem(hWnd,IDC_SP_SPINAXISVAR_DEG),ison);
 if (isphase)
 { SpinnerOn(hWnd,IDC_SP_SPINPHASPIN,IDC_SP_SPINPHA);
   SpinnerOn(hWnd,IDC_SP_SPINPHAVARSPIN,IDC_SP_SPINPHAVAR);
 }
 else
 { SpinnerOff(hWnd,IDC_SP_SPINPHASPIN,IDC_SP_SPINPHA);
   SpinnerOff(hWnd,IDC_SP_SPINPHAVARSPIN,IDC_SP_SPINPHAVAR);
 }
}
void SpinMainStuff(HWND hWnd,BOOL ison)
{if (ison)
 { SpinnerOn(hWnd,IDC_SP_SPINSPIN,IDC_SP_SPIN);
   SpinnerOn(hWnd,IDC_SP_SPINVARSPIN,IDC_SP_SPINVAR);
 } else
 { SpinnerOff(hWnd,IDC_SP_SPINSPIN,IDC_SP_SPIN);
   SpinnerOff(hWnd,IDC_SP_SPINVARSPIN,IDC_SP_SPINVAR);
 }
 EnableWindow(GetDlgItem(hWnd,IDC_AP_PARTICLEDIRRND),ison);
 EnableWindow(GetDlgItem(hWnd,IDC_AP_PARTICLEDIRUSER),ison);
}

void StdStuff(HWND hWnd,BOOL ison)
{ EnableWindow(GetDlgItem(hWnd,IDC_SP_TYPETRI),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_TYPECUB),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_TYPESPC),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_TYPEFAC),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_TYPEPIX),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_TYPE6PNT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_TYPETET),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_TYPESPHERE),ison);
}

void MetaOff(HWND hWnd)
{ SpinnerOff(hWnd,IDC_SP_METTENSSPIN,IDC_SP_METTENS);
  SpinnerOff(hWnd,IDC_SP_METTENSVARSPIN,IDC_SP_METTENSVAR);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENS_TXT),FALSE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENSVAR_TXT),FALSE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METTENSVAR_PCNT),FALSE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_AUTOCOARSE),FALSE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_DRAFTMODE),FALSE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_MANYBLOBS),FALSE);
  SpinnerOff(hWnd,IDC_SP_METCOURSESPIN,IDC_SP_METCOURSE);
  SpinnerOff(hWnd,IDC_SP_METCOURSEVSPIN,IDC_SP_METCOURSEV);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_COARSENESS_TXT),FALSE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METCOURSE_TXT),FALSE);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_METCOURSEV_TXT),FALSE);
}

void InstStuff(HWND hWnd,BOOL ison,HWND hparam,HWND spawn,BOOL dist)
{ EnableWindow(GetDlgItem(hWnd,IDC_AP_INSTANCESRCNAME),ison);
  TurnButton(hWnd,IDC_AP_OBJECTPICK,ison);
  TurnButton(hWnd,IDC_AP_TREEPICK,ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_USESUBTREE),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_ANIMATIONOFFSET_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_NOANIOFF),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIOFFBIRTH),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIOFFRND),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_SP_MAPCUSTOMINST),ison);
//  if (!dist) EnableWindow(GetDlgItem(hWnd,IDC_AP_CUSTOMMTLL2),ison);
  EnableWindow(GetDlgItem(hparam,IDC_SP_VIEWDISPBOX),ison);
  EnableWindow(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_ANIRNDFR_TXT),ison);
if (ison)
  SpinnerOn(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);  
else
  SpinnerOff(hWnd,IDC_AP_ANIRNDFRSPIN,IDC_AP_ANIRNDFR);  
}
void SpawnWithStype(int stype,HWND spawn,int repi)
{ EnableWindow(GetDlgItem(spawn,IDC_AP_OBJECTQUEUE),stype>1);
  TurnButton(spawn,IDC_AP_OBJECTQUEUEPICK,(stype>1));
   TurnButton(spawn,IDC_AP_OBJQUEUEREPLACE,(stype>1)&&(repi>-1));
   TurnButton(spawn,IDC_AP_OBJQUEUEDELETE,(stype>1)&&(repi>-1));
}

void ObjectMutQueOn(int stype,HWND spawn,int repi)
{ SpawnWithStype(stype,spawn,repi);
}

void ObjectMutQueOff(HWND spawn)
{ TurnButton(spawn,IDC_AP_OBJECTQUEUE,FALSE);
  TurnButton(spawn,IDC_AP_OBJECTQUEUEPICK,FALSE);
  TurnButton(spawn,IDC_AP_OBJQUEUEREPLACE,FALSE);
  TurnButton(spawn,IDC_AP_OBJQUEUEDELETE,FALSE);
}

void SpawnStuff(HWND hWnd,int stype)
{ BOOL ison=(stype>1);
  if (ison)
  { SpinnerOn(hWnd,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);  
	SpinnerOn(hWnd,IDC_AP_NUMBERVARSPIN,IDC_AP_NUMBERVAR);  
	SpinnerOn(hWnd,IDC_AP_PARENTPERCENTSPIN,IDC_AP_PARENTPERCENT);  
	SpinnerOn(hWnd,IDC_AP_NUMBERVARVARSPIN,IDC_AP_NUMBERVARVAR);  
	SpinnerOn(hWnd,IDC_AP_CHAOSANGLESPIN,IDC_AP_CHAOSANGLE);  
	SpinnerOn(hWnd,IDC_AP_CHAOSSPEEDSPIN,IDC_AP_CHAOSSPEED);  
	SpinnerOn(hWnd,IDC_AP_CHAOSSCALESPIN,IDC_AP_CHAOSSCALE);  
	SpinnerOn(hWnd,IDC_AP_QUEUELIFESPANSPIN,IDC_AP_QUEUELIFESPAN);  
  }
  else 
  { SpinnerOff(hWnd,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);  
	SpinnerOff(hWnd,IDC_AP_NUMBERVARSPIN,IDC_AP_NUMBERVAR);  
	SpinnerOff(hWnd,IDC_AP_PARENTPERCENTSPIN,IDC_AP_PARENTPERCENT);  
	SpinnerOff(hWnd,IDC_AP_NUMBERVARVARSPIN,IDC_AP_NUMBERVARVAR);  
	SpinnerOff(hWnd,IDC_AP_CHAOSANGLESPIN,IDC_AP_CHAOSANGLE);  
	SpinnerOff(hWnd,IDC_AP_CHAOSSPEEDSPIN,IDC_AP_CHAOSSPEED);  
	SpinnerOff(hWnd,IDC_AP_CHAOSSCALESPIN,IDC_AP_CHAOSSCALE);  
	SpinnerOff(hWnd,IDC_AP_QUEUELIFESPANSPIN,IDC_AP_QUEUELIFESPAN);  
	EnableWindow(GetDlgItem(hWnd,IDC_AP_OBJECTQUEUE),FALSE);
  }
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MAXSPAWNGENS_TXT),ison && (stype!=4));
  EnableWindow(GetDlgItem(hWnd,IDC_AP_PARENTPERCENT_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_PARENTPERCENT_PCNT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_NUMBERVAR_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_NUMBERVARVAR_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_NUMBERVARVAR_PCNT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_CHAOSANGLE_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_CHAOSANGLE_PCNT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_CHAOSSPEED_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_CHAOSSPEED_PCNT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_CHAOSSCALE_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_CHAOSSCALE_PCNT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_QUEUELIFESPAN_TXT),ison);
  if (stype==1)
  { SpinnerOn(hWnd,IDC_AP_MAXSPAWNDIEAFTERSPIN,IDC_AP_MAXSPAWNDIEAFTER);
	SpinnerOn(hWnd,IDC_AP_MAXSPAWNDIEAFTERVARSPIN,IDC_AP_MAXSPAWNDIEAFTERVAR);
  }
  else 
  { SpinnerOff(hWnd,IDC_AP_MAXSPAWNDIEAFTERSPIN,IDC_AP_MAXSPAWNDIEAFTER);
	SpinnerOff(hWnd,IDC_AP_MAXSPAWNDIEAFTERVARSPIN,IDC_AP_MAXSPAWNDIEAFTERVAR);
  }
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MAXSPAWNDIEAFTER_TXT),stype==1);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MAXSPAWNDIEAFTERVAR_TXT),stype==1);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_MAXSPAWNDIEAFTERVAR_PCNT),stype==1);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SPEEDLESS),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SPEEDMORE),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SPEEDBOTH),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SPAWNSUMV),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SPAWNSPEEDFIXED),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SCALEDOWN),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SCALEUP),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SCALEBOTH),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SPAWNSCALEFIXED),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_LIFEQUEUE),ison);
  TurnButton(hWnd,IDC_AP_LIFEQUEUEADD,ison);
  TurnButton(hWnd,IDC_AP_LIFEQUEUEDEL,ison);
  TurnButton(hWnd,IDC_AP_LIFEQUEUEREPL,ison);
  if (stype==4)
	  SpinnerOff(hWnd,IDC_AP_MAXSPAWNGENSSPIN,IDC_AP_MAXSPAWNGENS);
 /*  EnableWindow(GetDlgItem(hWnd,IDC_AP_OBJECTQUEUE),ison);
  TurnButton(hWnd,IDC_AP_OBJECTQUEUEPICK,ison);
  TurnButton(hWnd,IDC_AP_OBJQUEUEDELETE,ison);
  TurnButton(hWnd,IDC_AP_OBJQUEUEREPLACE,ison);*/
}

void AllSpawnBad(HWND hWnd,int stype,BOOL notbad)
{ SpawnStuff(hWnd,stype);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_COLLIDESPAWN),notbad);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_DEATHSPAWN),notbad);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_SPAWNTRAILS),notbad);
  EnableWindow(GetDlgItem(hWnd,IDC_AP_COLLIDEDIE),notbad);
}

void IPCControls(HWND hWnd,HWND spwnd,int stype,BOOL ison)
{ if (ison)
  { SpinnerOn(hWnd,IDC_INTERP_NSTEPSSPIN,IDC_INTERP_NSTEPS);
    SpinnerOn(hWnd,IDC_INTERP_BOUNCESPIN,IDC_INTERP_BOUNCE);
    SpinnerOn(hWnd,IDC_INTERP_BOUNCEVARSPIN,IDC_INTERP_BOUNCEVAR);
	AllSpawnBad(spwnd,0,FALSE);
  }
  else
  { SpinnerOff(hWnd,IDC_INTERP_NSTEPSSPIN,IDC_INTERP_NSTEPS);
    SpinnerOff(hWnd,IDC_INTERP_BOUNCESPIN,IDC_INTERP_BOUNCE);
    SpinnerOff(hWnd,IDC_INTERP_BOUNCEVARSPIN,IDC_INTERP_BOUNCEVAR);
	AllSpawnBad(spwnd,stype,TRUE);
  }
  EnableWindow(GetDlgItem(hWnd,IDC_INTERP_NSTEPS_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_INTERP_BOUNCE_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_INTERP_BOUNCE_PCNT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_INTERP_BOUNCEVAR_TXT),ison);
  EnableWindow(GetDlgItem(hWnd,IDC_INTERP_BOUNCEVAR_PCNT),ison);
}

BOOL IsStdMtl(INode *cnode)
{ Mtl *mtl;
  int nummtls,multi=0;
  if (!cnode) return(TRUE);
  mtl=cnode->GetMtl();
  if (mtl)
  { Class_ID mc=Class_ID(MULTI_CLASS_ID,0);
    Class_ID oc=mtl->ClassID();
	if (multi=(oc==mc))
	{ nummtls=mtl->NumSubMtls();if (nummtls==0) multi=0;}
  }
  return (!multi);
}

void MakeNodeList(INode *node,INodeTab *ntab,int subtree,TimeValue t)
{ int nc;
  if (subtree)
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	  MakeNodeList(node->GetChildNode(j),ntab,subtree,t);
  TriObject *triOb=NULL;
  Object *pobj=NULL;
  if ((!node->IsGroupHead())&&((triOb=TriIsUseable(pobj = node->EvalWorldState(t).obj,t))!=NULL))
   (*ntab).Append(1,&node,0);
  if ((triOb) &&(triOb!=pobj)) triOb->DeleteThis();
}

void MakeGroupNodeList(INode *node,INodeTab *ntab,int subtree,TimeValue t)
{ int nc;
  if ((nc=node->NumberOfChildren())>0)
	for (int j=0;j<nc;j++)
	{ INode *nxtnode=node->GetChildNode(j);
	  if (nxtnode->IsGroupHead()) MakeGroupNodeList(nxtnode,ntab,subtree,t);
	  else if ((subtree)||(nxtnode->IsGroupMember())) MakeNodeList(nxtnode,ntab,subtree,t);
	}
}

void FormatName(TCHAR *name)
{ int len= _tcslen(name);
  if (len>MAXNAME) name[MAXNAME]='\0';
}

ParticleMtl::ParticleMtl():Material() {
	Kd[0] = PARTICLE_R;
	Kd[1] = PARTICLE_G;
	Kd[2] = PARTICLE_B;
	Ks[0] = PARTICLE_R;
	Ks[1] = PARTICLE_G;
	Ks[2] = PARTICLE_B;
	shininess = (float)0.0;
	shadeLimit = GW_WIREFRAME;
	selfIllum = (float)1.0;
	}
void SetFlag(ULONG &flags,ULONG f, ULONG val) {
	if (val) flags|=f; 
	else flags &= ~f;
	}
void ZeroMesh(Mesh *pm)
{ pm->setNumVerts(0);
  pm->setNumFaces(0);
  pm->setNumTVerts(0);
  pm->setNumTVFaces(0);
}
BOOL ReadInt(int *buf,FILE *f)
{ return(fread(buf,isize,1,f)==1);
}
BOOL WriteInt(int *buf,FILE *f)
{ return(fwrite(buf,isize,1,f)==1);
}

BOOL GenNewSaveFile(int osize,int size,int custsettings,FILE *f,TCHAR *filename,int vers)
{ int future=0;BOOL ok=TRUE;
  fclose(f);
  if ((f = _tfopen(filename, _T("r+b"))) == NULL) return FALSE;
  ok=(WriteInt(&custsettings,f)&&WriteInt(&vers,f)&&WriteInt(&size,f)&&WriteInt(&future,f));
 if (custsettings>0)
 { int cset=(custsettings-1),odata=osize+NLEN;
   AName Name;int ofs=(size-osize),tofs=cset*ofs;
   BYTE *data=new BYTE[size];for (int x=0;x<size;x++) data[x]=0;
   cset*=odata;cset+=HLEN;
   while (cset>=HLEN)
   { fseek(f,cset,SEEK_SET);
      if (fread(Name,NLEN,1,f)==1)
	 { if (fread(data,osize,1,f)!=1) goto badend;
       fseek(f,cset+tofs,SEEK_SET);
		if (fwrite(Name,1,NLEN,f)!=NLEN) goto badend;
	   if (fwrite(data,size,1,f)!=1) goto badend;
	 } else goto badend;
	 cset-=odata;tofs-=ofs;
   }
   delete[] data;
   rewind(f);
 }
 return ok;
badend: fclose(f);return FALSE;
 }
BOOL GenNewNameLen(int size,int custsettings,FILE *f,TCHAR *filename,int vers)
{ int Namelen=NLEN;BOOL ok=TRUE;
  fclose(f);
  if ((f = _tfopen(filename, _T("r+b"))) == NULL) return FALSE;
  ok=(WriteInt(&custsettings,f)&&WriteInt(&vers,f)&&WriteInt(&size,f)&&WriteInt(&Namelen,f));
  if (custsettings>0)
  { int cset=(custsettings-1),odata=size+ONLEN,ofs=(NLEN-ONLEN),tofs=cset*ofs;
    AName Name;
		BYTE *data=new BYTE[size];
		memset(Name,' ',sizeof(Name));
		cset*=odata;cset+=HLEN;
		while (cset>=HLEN)
    { fseek(f,cset,SEEK_SET);
      if (fread(Name,ONLEN,1,f)==1)
			{ if (fread(data,size,1,f)!=1) goto badend;
        fseek(f,cset+tofs,SEEK_SET);
			  if (fwrite(Name,1,NLEN,f)!=NLEN) goto badend;
			  if (fwrite(data,size,1,f)!=1) goto badend;
			} else goto badend;
		  cset-=odata;tofs-=ofs;
    }
    delete[] data;
    rewind(f);
 }
 return ok;
badend: fclose(f);return FALSE;
}

void SwitchVerts(Mesh *pm)
{ int numf=pm->getNumFaces(),tmpv;
  for (int i=0;i<numf;i++) 
  { tmpv=pm->faces[i].v[1];
    pm->faces[i].v[1]=pm->faces[i].v[2];
	pm->faces[i].v[2]=tmpv;
  }
}
BOOL CheckMtlChange(Mtl *mtl,BOOL wasmulti)
{ BOOL multi=FALSE;
  if (mtl)
  { Class_ID mc=Class_ID(MULTI_CLASS_ID,0);
    Class_ID oc=mtl->ClassID();
    if (multi=(oc==mc))
    { if (mtl->NumSubMtls()==0) multi=0;}
  }
  return (wasmulti!=multi);
}
float GetLen(Point3 vels,int K)
{ float len=Length(vels);
  int aframe=GetTicksPerFrame();
  if (len <= 0.0f) len=1.0f;
  else len=((len*aframe-1.0f)*K+100.0f)/100.0f;
  return len;
}

int __cdecl limsort( const void *arg1, const void *arg2 )
{  float v1=((limdata *)arg1)->val,v2=((limdata *)arg2)->val;
	if (v1<v2) return 1;
	if (v1==v2) return 0;
	return -1;
}
int __cdecl pairsort( const void *arg1, const void *arg2 )
{ int p11=((pairs *)arg1)->p1,p12=((pairs *)arg1)->p2;
  int p21=((pairs *)arg2)->p1,p22=((pairs *)arg2)->p2;
  if (p11<p12) return 1;
  if (p11==p12) return (p12<p22?1:(p12==p22?0:-1));
  return -1;
}
int CountLive(ParticleSys &parts)
{	int c=0;
	for (int i=0; i<parts.Count(); i++)
	  {if (parts.Alive(i)) c++;}
	return c;
}

void fillinBB(boxplus *bbox,ParticleSys &parts,int dt)
{ int c=0;
  for (int i=0;i<parts.Count();i++)
  { if (parts.Alive(i))
	{ Point3 pos=(float)dt*parts.vels[i];
	  bbox[c].refnum=i;
      bbox[c].bbox.MakeCube(parts.points[i],parts.radius[i]);
      if (parts.vels[i].x<0) bbox[c].bbox.pmin.x+=pos.x;else bbox[c].bbox.pmax.x+=pos.x;
      if (parts.vels[i].y<0) bbox[c].bbox.pmin.y+=pos.y;else bbox[c].bbox.pmax.y+=pos.y;
      if (parts.vels[i].z<0) bbox[c].bbox.pmin.z+=pos.z;else bbox[c].bbox.pmax.z+=pos.z;
	  c++;
	}
  }
}
void AddToActive(int *active,BOOL *inlist,int &num,int partnum)
{ active[num]=partnum;
  inlist[partnum]=TRUE;
  num++;
}
void RemFromActive(int *active,BOOL *inlist,int &len,int partnum)
{ for (int i=0;(i<len)&&(active[i]!=partnum);i++);
  int mnum=len-i-1;
  if (mnum>0) { memcpy(&active[i],&active[i+1],isize*mnum);}
  inlist[partnum]=FALSE;
  len--;
}

void CollideParticle::PossibleCollide(int *active,BOOL *used,int acount)
{ int last=acount-1;
  used[active[last]]=TRUE;
  for (int i=0;i<last;i++)
  { if (xcnt==xmaxlen) 
	{ int oldlen=xmaxlen;xmaxlen+=count;pairs *tmp=xlst;xlst=new pairs[xmaxlen];assert(xlst);
	  memcpy(xlst,tmp,oldlen*sizeof(pairs));delete[] tmp;
	}
	if (active[i]<active[last]) {xlst[xcnt].p1=active[i];xlst[xcnt].p2=active[last];}
    else {xlst[xcnt].p1=active[last];xlst[xcnt].p2=active[i];}
	xcnt++;
	used[active[i]]=TRUE;
  }
}

int CollideParticle::FindXLst(int &pos,int num)
{ while (pos<xcnt)
  { if (xlst[pos].p1==num) return xlst[pos].p2; 
    if (xlst[pos].p2==num) return xlst[pos].p1;
	pos++;
  }
  return -1;
}

void CollideParticle::PossibleYZCollide(int *active,BOOL *used,int acount,int num)
{ int i=0,cpos=0,last=acount-1,tstnum=0;BOOL found=FALSE,ok=TRUE;
  while (ok && (i<acount))
  { if (ok=((tstnum=FindXLst(cpos,num))>-1))
	{ while ((i<acount)&&(tstnum>active[i])) i++;
	  if (i<acount)
	  { if (tstnum==active[i]) found=TRUE;
	    else
		{ int dist=xcnt-cpos-1;
		  if (dist) memcpy(&xlst[cpos],&xlst[cpos+1],sizeof(pairs)*dist);
		  xcnt--;
		}
		i++;
	  } 
	}
  }
  if (!found) used[num]=FALSE;
  else
  {	if (num<active[acount-1])
	{ int i=acount-2;
      while ((i>-1)&&(num<active[i])) i--;
	  i++;
      memcpy(&active[i+1],&active[i],isize*(acount-i));
	 } else i=acount;
	active[i]=num;
	acount++;
  }
}

void CollideParticle::RemoveThisPart(int num)
{ int i=0;
  while (i<xcnt)
  { if ((xlst[i].p1==num)||(xlst[i].p2==num))
	{ int dist=xcnt-i-1;
	  if (dist) memcpy(&xlst[i],&xlst[i +1],sizeof(pairs)*dist);
	  xcnt--;
	} else i++;
  }
}

int CollideParticle::PredetectParticleCollisions()
{ int acount=0;xcnt=0;
  int limnum=2*count;limdata *limits=new limdata[limnum];assert(limits);
  int j=0;
  for (int i=0;i<count;i++) 
  { limits[j].val=bbox[i].bbox.pmin.x;limits[j].num=i;j++;
    limits[j].val=bbox[i].bbox.pmax.x;limits[j].num=i;j++;
  }
  qsort(limits,j,sizeof(limdata),limsort);
  int *active=new int[count];assert(active);memset(active,0,isize*count);
  int *inlist=new BOOL[count];assert(inlist);memset(inlist,0,bsize*count);
  BOOL *used=new BOOL[count];assert(used);memset(used,0,bsize*count);
  for (i=0;i<limnum;i++)
  { int num=limits[i].num;
    if (inlist[num]) RemFromActive(active,inlist,acount,num);
	else 
	{ AddToActive(active,inlist,acount,num);
	  if (acount>1) PossibleCollide(active,used,acount);
	}
  }
  qsort(xlst,xcnt,sizeof(pairs),pairsort);
  BOOL added=FALSE;
  for (int k=0;(xcnt>0)&&(k<2);k++)
  { j=0;
	for (i=0;i<count;i++) 
	{ limits[j].val=(k==0?bbox[i].bbox.pmin.y:bbox[i].bbox.pmin.z);limits[j].num=i;j++;
	limits[j].val=(k==0?bbox[i].bbox.pmax.y:bbox[i].bbox.pmax.z);limits[j].num=i;j++;
	} 
	acount=0;
    qsort(limits,j,sizeof(limdata),limsort);
    for (i=0;(xcnt>0)&&(i<limnum);i++)
	{ int num=limits[i].num;
	  if (used[num])
	  { if (inlist[num]) 
		{ if ((acount==1)&&added)
		  { used[num]=FALSE;RemoveThisPart(num);}
		  RemFromActive(active,inlist,acount,num);added=FALSE;
		}
		else
		{ if (acount>0) PossibleYZCollide(active,used,acount,num);
		  AddToActive(active,inlist,acount,num);
		  added=TRUE;
		}
	  }
	}
  }
  if (limits) delete[] limits;
  if (active) delete[] active;
  if (inlist) delete[] inlist;
  if (used) delete[] used;
  return xcnt;
}

TimeValue CollideParticle::DetectParticleCollsion(ParticleSys &parts,pairs x,int t,float B,float Vb,mindata &tmpdata)
{ float width1=parts.radius[bbox[x.p1].refnum],width2=parts.radius[bbox[x.p2].refnum];
  float L=(width1+width2)/2.0f;
  Point3 v1=parts.vels[bbox[x.p1].refnum],v2=parts.vels[bbox[x.p2].refnum];
  Point3 Aj=parts.points[bbox[x.p1].refnum], Am=parts.points[bbox[x.p2].refnum],Bj=Aj+v1*(float)t,Bm=Am+v2*(float)t;
  Point3 V=Aj-Am;float tmpl=Length(V);
  if (Length(V)<L) return -1;
  Point3 Va=Bj-Aj+Am-Bm;
  float AA=LengthSquared(Va),BB=2.0f*(Va.x*V.x+Va.y*V.y+Va.z*V.z),CC=LengthSquared(V)-L*L;
  float BBSQ=BB*BB,AC=4.0f*AA*CC;
  if ((BBSQ-AC)<0.0f) return -1;
  float A2=2.0f*AA;
  float mult=(float)sqrt(BBSQ-AC);
  float alpha1=(-BB+mult)/A2,alpha2=(-BB-mult)/A2,alpha;
  if ((alpha1>1.0f)&&(alpha2>1.0f)) return -1;
  if ((alpha1<0.0f)&&(alpha2<0.0f)) return -1;
  alpha=(alpha1<alpha2?(alpha1>0.0f?alpha1:alpha2):(alpha2<0.0f?alpha1:alpha2));
  Aj+=v1*alpha;Am+=v2*alpha;
  Point3 collx=Normalize(Am-Aj);
  float size1=parts.radius[bbox[x.p1].refnum],size2=parts.radius[bbox[x.p2].refnum];
  float mass1=size1*size1*size1,mass2=size2*size2*size2;
  float eps=B*(1.0f-Vb*RND11());
  float vmag1=DotProd(collx,v1),vmag2=DotProd(collx,v2);
  Point3 V1=vmag1*collx,V2=vmag2*collx;
  v1-=V1;v2-=V2;
  float mass=mass1/mass2;
  float vmag11=(vmag1*mass+vmag2-eps*vmag1+eps*vmag2)/(1.0f+mass);
  float vmag21=eps*(vmag1-vmag2)+vmag11;
  v1+=vmag11*collx;v2+=vmag21*collx;
  tmpdata.svel1=v1;tmpdata.svel2=v2;tmpdata.spos1=Aj;tmpdata.spos2=Am;
  return (int)alpha;
}

TimeValue CollideParticle::FindMinPartCollide(ParticleSys &parts,int remtime,float B,float Vb)
{	TimeValue tmptime=0,smalltime=remtime;
	mindata tmpdata;
	for (int i=0;i<xcnt;i++)
	{	if ((tmptime=DetectParticleCollsion(parts,xlst[i],remtime,B,Vb,tmpdata)) > -1)
			if (tmptime<smalltime)
			{	minpt=tmpdata;
				minpt.min=xlst[i].p1;
				minpt.min2=xlst[i].p2;
			}; // is this semicol necessary?
	}
	return smalltime;
}

BOOL MaybeStuck(int hit,int colobj,stuck sl,int max)
{ int found=FALSE;int i=0;
  while ((i<max)&&(!(found=((hit==sl.ppts[i].p1)&&(colobj==sl.ppts[i].p2)) ))  ) i++;
  return (found);
}

mindata CollideParticle::InterPartCollide(ParticleSys &parts,Tab<CollisionObject*> &cobjs,int &remtime,int &stepnum,float B,float Vb,int &t,oldipc &l)
{	
	// variable for new collision scheme (Bayboro 3/5/01)
	CollisionCollection cc;
	// initialization for new collision scheme (Bayboro 3/5/01)
	cc.Init(cobjs);
	
	count = CountLive(parts);
	xlst = new pairs[count];
	xmaxlen = count;
	BOOL found = FALSE;
	bbox = new boxplus[count];
	assert(bbox);
	fillinBB(bbox,parts,remtime);
    int possible = PredetectParticleCollisions();
/*f (minpt.collide>-1) 
	{	l.lastmin = minpt.min;
		l.lastcollide = minpt.collide;
	}*/
	if (!(stucklist.stuckt==t)) 
	{ stucklist.stuckt=t;sused=0;
	}
    minpt.min = -1;
	minpt.min2 = -1;
	minpt.mintime = -1;
	minpt.collide = -1;
	int alivep=0;
	float tleft;
    minpt.mintime = FindMinPartCollide(parts,remtime,B,Vb);
    for (int j=0; j<parts.Count(); j++)
	{	if (parts.Alive(j))
		{	for (int k=0; k<cobjs.Count(); k++)
			{	BOOL duplicateCobj = FALSE;
				for (int kk=0; kk<k; kk++)
				{ if (cobjs[k]->GetSWObject()==cobjs[kk]->GetSWObject())
					{	duplicateCobj = TRUE;
						break;
					}
				}
				if (!duplicateCobj)
				{	Point3 pos = parts.points[j],
						   vel = parts.vels[j];
					float tmpt = (float)remtime;
					tleft = tmpt;
					BOOL stuck = MaybeStuck(bbox[alivep].refnum,k,stucklist,sused);
					if (!stuck)
					{ BOOL atlast=(l.lastmin==bbox[alivep].refnum)&&(k==l.lastcollide);
					  if (atlast) 
					  {	pos += vel;
						tleft--;
						int num=sused;sused++; 
						if (sused>=ssize) {ssize+=5;stucklist.ppts.Resize(ssize);}
						stucklist.ppts.SetCount(sused);
						stucklist.ppts[num].p1=bbox[alivep].refnum;stucklist.ppts[num].p2=k;
					  }
					  if ((tleft>0)&&(cobjs[k]->CheckCollision(t,pos,vel,tleft, j,&tmpt,FALSE)) )
					  {	if (atlast) tmpt += 1;
						if ((minpt.min < 0) || (tmpt < minpt.mintime))
						{	minpt.mintime = (TimeValue)tmpt;
							minpt.spos1 = pos;
							minpt.svel1 = vel;
							minpt.min = alivep;
							minpt.min2 = -1;
							minpt.collide = k;
						}
					  }
					}
				}
			}
			alivep++;
		}
	}
	if (minpt.min>-1)
	{	parts.points[bbox[minpt.min].refnum] = minpt.spos1;
		parts.vels[bbox[minpt.min].refnum] = minpt.svel1;
		minpt.min = bbox[minpt.min].refnum;
		l.lastmin=minpt.min;l.lastcollide=minpt.collide;
		if (minpt.min2>-1)
		{	parts.points[bbox[minpt.min2].refnum] = minpt.spos2;
			parts.vels[bbox[minpt.min2].refnum] = minpt.svel2;
			minpt.min2=bbox[minpt.min2].refnum;
		}
		found=TRUE;
	} 
	remtime -= minpt.mintime;
	t += minpt.mintime;
	if (bbox) 
		delete[] bbox;
	if (remtime==0) 
		stepnum++;
	if (xlst) 
		delete[] xlst;
	return minpt;
}

BOOL IsGEOM(Object *obj)
{ if (obj!=NULL) 
  { if (obj->IsParticleSystem()) return FALSE;
    if (obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
    { if (obj->IsSubClassOf(triObjectClassID)) 
        return TRUE;
      else 
	  { if (obj->CanConvertToType(triObjectClassID)) 
	  	return TRUE;			
	  }
	}
  }
  return FALSE;
}

void MirrorFace(Face *f)
{ int tmp=f->v[1];
  f->v[1]=f->v[2];
  f->v[2]=tmp;
  f->setEdgeVisFlags(f->getEdgeVis(2),f->getEdgeVis(1),f->getEdgeVis(0));
}

void MirrorTVs(TVFace *f)
{ int tmp=f->t[1];
  f->t[1]=f->t[2];
  f->t[2]=tmp;
}

///////////////////////////////////////////////////////////////////////
//																	 //
//				RandGeneratorParticles implementation				 //
//																	 //
///////////////////////////////////////////////////////////////////////

const float RandGeneratorParticles::IntMax = float(4294967295.0);
const float RandGeneratorParticles::IntMax1 = float(4294967296.0);
const float RandGeneratorParticles::HalfIntMax = float(0.5*4294967295.0);

Point3 RandGeneratorParticles::CalcSpread(float divangle,Point3 oldnorm)
{ float Q[3];
  Point3 r;

  Q[0]=Q[1]=Q[2]=0.0f;
  // Martell 4/14/01: Fix for order of ops bug.
  float z=RND11(); float y=RND11(); float x=RND11();
  r=Point3(x,y,z);
  r=Normalize(r^oldnorm);
  RotateOnePoint(&oldnorm.x,Q,&r.x,RND01()*divangle);
  return(oldnorm);
}

void RandGeneratorParticles::VectorVar(Point3 *vel,float R,float MaxAngle)
{ 
  // Martell 4/14/01: Fix for order of ops bug.
  float z=RND11(); float y=RND11(); float x=RND11();
  Point3 X=Point3(x,y,z);
  Point3 c=Normalize(X^*vel);
  float Theta=MaxAngle*R*RND01();
  Point3 zero=Zero;
  RotateOnePoint(&(*vel).x,&zero.x,&c.x,Theta);
}

Point3 RandGeneratorParticles::DoSpawnVars(SpawnVars spvars,Point3 pv,Point3 holdv,float *radius,Point3 *sW)
{ Point3 vels=holdv;
  if (!FloatEQ0(spvars.dirchaos))
    VectorVar(&vels,spvars.dirchaos,PI);
  float dovar=(spvars.spsign==0?-RND01():(spvars.spsign==1?RND01():RND11()));
  if (spvars.spconst) dovar=(dovar>0.0f?1.0f:-1.0f);
  float tmp=(1+dovar*spvars.spchaos);
  vels*=(tmp<0.0f?0.0f:tmp);
  dovar=(spvars.scsign==0?-RND01():(spvars.scsign==1?RND01():RND11()));
  if (spvars.scconst) dovar=(dovar>0.0f?1.0f:-1.0f);
  tmp=(1.0f+dovar*spvars.scchaos);
  (*radius)*=(tmp<0.0f?0.0f:tmp);
  if (spvars.invel) vels+=pv;
  if (spvars.axisentered==2)
  {	*sW=Normalize(spvars.Axis);
	if (spvars.axisvar>0.0f)
		VectorVar(sW,spvars.axisvar,PI);
  }
  else 
  {
      // Martell 4/14/01: Fix for order of ops bug.
      float z=RND11(); float y=RND11(); float x=RND11();
	  *sW=Normalize(Point3(x,y,z));
  }
  return vels;
} 


// Multiple Map Channels Support for Instanced Geometry (Bayboro)
void CopyMultipleMapping(Mesh *cmesh, TriObject *triOb, const int *numF, const int tface, 
						const Point3& deftex, const TVFace& Zerod, const int subf, const BOOL mirror)
{
	int i, j;
	int tvnumMap, tottvMap, subtvnumMap, ismappedMap;
	int numMaps = triOb->GetMesh().getNumMaps();
	if (cmesh->getNumMaps()<numMaps)
		cmesh->setNumMaps( numMaps, TRUE );
	for ( i=2; i<numMaps; i++ )
	{
		if (triOb->GetMesh().mapSupport(i) || cmesh->mapSupport(i))
		{
			ismappedMap = cmesh->mapSupport(i);
			cmesh->setMapSupport( i, TRUE );

			tvnumMap = ( cmesh->mapSupport(i) ? cmesh->getNumMapVerts(i) : 0 );
			subtvnumMap = ( triOb->GetMesh().mapSupport(i) ? triOb->GetMesh().getNumMapVerts(i) : 0);
			tottvMap = tvnumMap + subtvnumMap;
			cmesh->setNumMapVerts(i, tottvMap, TRUE );

			if ((!ismappedMap) && (*numF>0))
			{
				cmesh->setNumMapFaces(i, tface, FALSE, 0);				
				tvnumMap = 1;
				cmesh->setNumMapVerts(i, tottvMap + 1);
				cmesh->setMapVert(i, 0, deftex);
				for ( j=0; j<*numF; j++)
					cmesh->mapFaces(i)[j] = Zerod;
			}
			else
				cmesh->setNumMapFaces(i, tface, (*numF>0 ? true : false), *numF);

			if ((subf>0) && (subtvnumMap>0))
			{	
				for(j=0; j<subf; j++)
					cmesh->mapFaces(i)[*numF+j] = triOb->GetMesh().mapFaces(i)[j];
			}

			for ( j=(*numF); j<tface; j++)
			{
				if (subtvnumMap>0)
				{	
					cmesh->mapFaces(i)[j].t[0] += tvnumMap;
					cmesh->mapFaces(i)[j].t[1] += tvnumMap;
					cmesh->mapFaces(i)[j].t[2] += tvnumMap;
					if (mirror) MirrorTVs( &(cmesh->mapFaces(i)[j]) );
				}
				else if (ismappedMap)
					cmesh->mapFaces(i)[j] = Zerod;
			}

			if (subtvnumMap>0)
				for ( j=0; j<subtvnumMap; j++)
					cmesh->setMapVert(i, tvnumMap+j, triOb->GetMesh().mapVerts(i)[j]);
		}
	}
}

//--------------------------------------------------------------//
//					ChannelMapMerger							//
//--------------------------------------------------------------//

void ChannelMapMerger::setNumTVertsTVFaces(Mesh* pm, Mesh* cmesh, int const mcnt, int const channel)
{
	if (mcnt == 0) return; // it's not a custom material
						   // therefore there is no need in multiple channel mapping
	int mc, tmptvs, gtvnum=0;
	BOOL alltex = TRUE;
	
	for (mc=0; mc<mcnt; mc++)
	{	tmptvs=(cmesh[mc].mapSupport(channel)) ? cmesh[mc].getNumMapVerts(channel) : 0;
		if (tmptvs==0) alltex=FALSE;
		else gtvnum+=tmptvs;
	}
	if ((!alltex)&&(gtvnum>0))
	{	m_defface.setTVerts(gtvnum,gtvnum,gtvnum); gtvnum++; }

	if (gtvnum==0) return; // there is no need to set the Channel Map

	m_tVertsOffset.SetCount(mcnt);
	m_tVertsOffset[0]=0;

	pm->setMapSupport(channel, TRUE);
	pm->setNumMapVerts(channel, gtvnum);

	int tvs=0, mtvs=0, imtv;
	for (mc=0; mc<mcnt; mc++)
	{	if (mc) m_tVertsOffset[mc]=m_tVertsOffset[mc-1]+mtvs;
		mtvs=(cmesh[mc].mapSupport(channel)) ? cmesh[mc].getNumMapVerts(channel) : 0;
		if (mtvs>0)
		{	for (imtv=0; imtv<mtvs; imtv++)
				pm->setMapVert(channel, tvs++, cmesh[mc].mapVerts(channel)[imtv] );
		}
	}
	if (!alltex) pm->setMapVert(channel, tvs, deftex);
	
	pm->setNumMapFaces(channel, pm->getNumFaces() );
}

void ChannelMapMerger::setNumTVertsTVFaces(Mesh* pm, Mesh* cmesh, const int custmtl, int const mcnt, int const channel)
{
	int mc, tmptvs, gtvnum=0;
	BOOL alltex = TRUE;
	
	if (!custmtl) return; // no support if mapping from PArray icon;
	if (custmtl==1) return; // no support if fragmented materials only
	if (mcnt == 0) return; // gain zero tVerts for instanced geometry

	for (mc=0; mc<mcnt; mc++)
	{	tmptvs=(cmesh[mc].mapSupport(channel)) ? cmesh[mc].getNumMapVerts(channel) : 0;
		if (tmptvs==0) alltex=FALSE;
		else gtvnum+=tmptvs;
	}
	if ((!alltex)&&(gtvnum>0))
	{	m_defface.setTVerts(gtvnum,gtvnum,gtvnum); gtvnum++; }

	if (gtvnum==0) return; // there is no need to set the Channel Map

	m_tVertsOffset.SetCount(mcnt);
	m_tVertsOffset[0]=0;

	pm->setMapSupport(channel, TRUE);
	pm->setNumMapVerts(channel, gtvnum);

	int tvs=0, mtvs=0, imtv;

	for (mc=0; mc<mcnt; mc++)
	{	if (mc) m_tVertsOffset[mc]=m_tVertsOffset[mc-1]+mtvs;
		mtvs=(cmesh[mc].mapSupport(channel)) ? cmesh[mc].getNumMapVerts(channel) : 0;
		if (mtvs>0)
		{	for (imtv=0; imtv<mtvs; imtv++)
				pm->setMapVert(channel, tvs++, cmesh[mc].mapVerts(channel)[imtv] );
		}
	}

	if (!alltex) pm->setMapVert(channel, tvs, deftex);
	
	pm->setNumMapFaces(channel, pm->getNumFaces() );
}

void ChannelMapMerger::setTVFaces(Mesh *pm, Mesh* cmesh, const int face, const int mnum, const int channel)
{
	if (!pm->mapSupport(channel)) return;

	int j, k;
	int numF = cmesh[mnum].getNumFaces();

	if (cmesh[mnum].getNumMapVerts(channel)>0)
		for (j=0; j<numF; j++)
			for (k=0; k<3; k++)
				pm->mapFaces(channel)[j+face].t[k] = cmesh[mnum].mapFaces(channel)[j].t[k]+m_tVertsOffset[mnum];
	else
		for (j=0; j<numF; j++)
			pm->mapFaces(channel)[j+face] = m_defface;
}


//--------------------------------------------------------------//
//				MultipleChannelMapMerger						//
//--------------------------------------------------------------//

void MultipleChannelMapMerger::setNumTVertsTVFaces(Mesh* pm, Mesh* cmesh, int const mcnt)
{
	for (int i=2; i<MAX_MESHMAPS; i++)
		m_map[i].setNumTVertsTVFaces(pm, cmesh, mcnt, i);
}

void MultipleChannelMapMerger::setNumTVertsTVFaces(Mesh* pm, Mesh* cmesh, const int custmtl, int const mcnt)
{
	for (int i=2; i<MAX_MESHMAPS; i++)
		m_map[i].setNumTVertsTVFaces(pm, cmesh, custmtl, mcnt, i);
}

void MultipleChannelMapMerger::setTVFaces(Mesh *pm, Mesh* cmesh, const int face, const int mnum)
{
	for (int i=2; i<MAX_MESHMAPS; i++)
		m_map[i].setTVFaces(pm, cmesh, face, mnum, i);
}


//--------------------------------------------------------------//
//		To resolve with long preset names						//
//--------------------------------------------------------------//

// The method is a copy of the method in "maxsdk\Samples\Controllers\ctrl.cpp
// Please duplicate any changes there as well
int computeHorizontalExtent(HWND hListBox, BOOL useTabs, int cTabs, LPINT lpnTabs)
{
	int i;
	int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
	HFONT font;
	HDC dc;
	int width=0;
	LPSTR Buffer;
	int saved;
	SIZE margin;

	// first, we must make sure we have a buffer
	// large enough to hold the longest string
	for (i=0; i < count; i++)
	{	/* compute buffer size */
		int len = SendMessage(hListBox, LB_GETTEXTLEN, i, 0);
		width = max(width, len);
	} 

	// if the list box is empty, jsut return 0
	if (width ==0)
		return 0;

	// allocate a buffer to hold the string
	// including the terminating NULL character
	Buffer = (TCHAR*)malloc ((width+1) * sizeof(TCHAR));
	if (Buffer == NULL)
		return 0;

	// we will need a DC for string length computation
	dc = GetDC(hListBox);

	// save the DC so we can restore it later
	saved = SaveDC(dc);

	font = GetWindowFont (hListBox);

	// if our font is other then the system font select it into the DC
	if (font != NULL)
		SelectFont (dc, font);

	// we now compute the longest actual string length
	width = 0;
	for(i=0; i < count; i++)
	{ /* compute the buffer size */
		int cx=0;
		SendMessage(hListBox, LB_GETTEXT, i, (LPARAM) (LPCTSTR) Buffer);
		if (useTabs) {
			DWORD sz = GetTabbedTextExtent(dc, Buffer, _tcslen(Buffer), cTabs, lpnTabs );
			cx = LOWORD(sz);
		}
		else {
			SIZE sz;
			GetTextExtentPoint32 (dc, Buffer, _tcslen(Buffer), &sz);
			cx = sz.cx;
		}		
		width = max (width, cx );
	}	
	GetTextExtentPoint32 (dc, _T(" "), _tcslen(_T(" ")), &margin);

	// we no longer need the buffer or DC; free them
	free (Buffer);
	RestoreDC(dc, saved);
	ReleaseDC(hListBox, dc);

	// deal with the (possible) presence of a scroll bar
	width += GetSystemMetrics(SM_CXVSCROLL) + 2*margin.cx;
	return width;
}

void UpdatePresetListBox(HWND hWndListBox, int num, AName* nameLst)
{
	SendMessage(hWndListBox,LB_RESETCONTENT,0,0);
	for (int i=0; i<num; i++) 
		SendMessage(hWndListBox, LB_ADDSTRING,0,(LPARAM)(TCHAR*)nameLst[i]);
	int width = computeHorizontalExtent(hWndListBox, FALSE, 0, NULL);
	SendMessage(hWndListBox, LB_SETHORIZONTALEXTENT, width, 0);
}

//--------------------------------------------------------------//
//		For multiple collisions per integration step (frame)	//
//--------------------------------------------------------------//

int CollisionCollection::MAX_COLLISIONS_PER_STEP = 2000;

void CollisionCollection::Init(const Tab<CollisionObject*> &cobjs)
{
	m_cobjs.SetCount(cobjs.Count());
	for(int i=0; i<cobjs.Count(); i++) m_cobjs[i] = cobjs[i];
}

Object* CollisionCollection::GetSWObject()
{
	if (m_cobjs.Count()) return m_cobjs[0]->GetSWObject();
	 else return NULL;
}

BOOL CollisionCollection::CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct, BOOL UpdatePastCollide)
{
	BOOL collide=FALSE, maybeStuck=FALSE;
	
	if (UpdatePastCollide)
	{
		for(int i=0; i<MAX_COLLISIONS_PER_STEP; i++)
		{
			if (FindClosestCollision(t, pos, vel, dt, index, ct))
			{
				collide = TRUE;
				dt -= *ct;
				if (dt <= 0.0f) break; // time limit for the current integration step
			}
			else break;
			// particle may still have a collision in the current integration step;
			// since particle reaches the limit of collision check per integration step,
			// we'd better hold on the particle movements for the current frame
			if (i==MAX_COLLISIONS_PER_STEP-1) maybeStuck = TRUE;
		}
		if ((dt > 0.0f) && (!maybeStuck)) // final interval without collisions
			pos += vel*dt;
	}
	else
		collide = FindClosestCollision(t, pos, vel, dt, index, ct);		

	return collide;
}

BOOL CollisionCollection::FindClosestCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct)
{
	Point3 curPos, curVel, resPos, resVel;
	float curTime, minTime = dt+1.0f;
	BOOL collide = FALSE;

	for(int i=0; i<m_cobjs.Count(); i++)
	{
		curPos = pos; curVel = vel;
		if (m_cobjs[i]->CheckCollision(t, curPos, curVel, dt, index, &curTime, FALSE))
			if (curTime < minTime) // the collision is the closest one
			{
				collide = TRUE;
				minTime = curTime;
				resPos = curPos;
				resVel = curVel;
			}
	}
	if (collide)
	{
		pos = resPos;
		vel = resVel;
		*ct = minTime;
	}
	return collide;
}

//----------------------------------------------------------//
//		birth position & speed interpolation				//
//		To resolve puffing effect when particle born		//
//		and make subframe sampling smooth along path		//
//----------------------------------------------------------//

void BirthPositionSpeed::Init(INode* node, TimeValue t, TimeValue stepSize)
{
	Point3 A = (node->GetObjTMBeforeWSM(t - 2*stepSize)).GetTrans();
	Point3 B = (node->GetObjTMBeforeWSM(t - stepSize)).GetTrans();
	Point3 C = (node->GetObjTMBeforeWSM(t)).GetTrans();
	Point3 D = (node->GetObjTMBeforeWSM(t + stepSize)).GetTrans();

	if (((A==B) && (B==C)) || ((B==C) && (C==D)))
		m_type = 0; //static
	else
		m_type = 1;

	m_a = B;
	m_b = Point3::Origin;
	if (m_type == 0) return;
	m_b = 0.5f*(C-A);
	m_c = A - 2.5f*B + 2*C - 0.5f*D;
	m_d = -0.5f*A + 1.5f*B - 1.5f*C + 0.5f*D;
	Point3 spA, spB, spC, spD;
	Point3 P = (node->GetObjTMBeforeWSM(t - 3*stepSize)).GetTrans();
	Point3 R = (node->GetObjTMBeforeWSM(t + 2*stepSize)).GetTrans();
	spA = 0.5f*(B-P);
	spB = 0.5f*(C-A);
	spC = 0.5f*(D-B);
	spD = 0.5f*(R-C);
	m_spA = spB;
	m_spB = 0.5f*(spC-spA);
	m_spC = spA - 2.5f*spB + 2*spC - 0.5f*spD;
	m_spD = -0.5f*spA + 1.5f*spB - 1.5f*spC + 0.5f*spD;
}

Point3& BirthPositionSpeed::Position(float t)
{
	if (m_type == 0) return m_a;
	m_res = m_a + t*(m_b + t*(m_c + t*m_d));
	return m_res;
}

Point3& BirthPositionSpeed::Speed(float t)
{
	if (m_type == 0) return m_b;
	m_res = m_spA + t*(m_spB + t*(m_spC + t*m_spD));
	return m_res;
}
const float angleCoefSmall=0.035f;
const float angleCoefBig=2.0f*angleCoefSmall;

float FricCoef(float friction, float normLen, float fricLen, float contactDur)
{
	float fricCoef;
	if (friction == 0.0f) fricCoef = 1.0f;
	else if (friction == 1.0f) fricCoef = 0.0f;
	else if (normLen >= angleCoefBig*fricLen) // angle of reflection is more than 4 degrees: not a "slide"
		fricCoef = 1.0f - friction;
	else if (normLen < angleCoefSmall*fricLen) // angle of reflection is less than 2 degrees: it's a "slide"
		fricCoef = (float)exp(contactDur*log(1-friction)/GetTicksPerFrame());
	else {
		float mix = (normLen/fricLen)/angleCoefSmall - 1.0f;
		fricCoef = mix*(1.0f-friction) 
					+ (1.0f-mix)*(float)exp(contactDur*log(1-friction)/GetTicksPerFrame());
	}
	return fricCoef;
}

void AddInheritVelocity(Point3 &bounceVec, Point3 &frictionVec, Point3 &partVec,
						Point3 &inheritVec, float vinher, float normLen, float fricLen)
{
	if (vinher == 0.0f) return; 
	if (normLen >= angleCoefBig*fricLen) // angle of reflection is more than 4 degrees: not a "slide"
	{
		partVec += vinher*inheritVec;
		return;
	}
	if (DotProd(inheritVec, inheritVec) == 0.0f) return;
	if (DotProd(bounceVec, bounceVec) == 0.0f) // no bounce component
	{
		partVec += vinher*inheritVec;
		return;
	}
	Point3 bounceAxe = FNormalize(bounceVec);
	float partZ = DotProd(bounceAxe, partVec);
	Point3 partAxe = partVec - partZ*bounceAxe;
	if (DotProd(partAxe, partAxe) < 0.000001f) 
	{ // particle velocity coinsides to bounceVec: add inherited Vel
		partVec += vinher*inheritVec;
		return;
	}
	partAxe = FNormalize(partAxe);
	float partX = DotProd(partAxe, partVec);
	Point3 ortAxe = bounceAxe^partAxe;
	float partY = DotProd(ortAxe, partVec);
	Point3 inhVec = vinher*inheritVec;
	float inhX = DotProd(partAxe, inhVec);
	float inhY = DotProd(ortAxe, inhVec);
	float inhZ = DotProd(bounceAxe, inhVec);
	if (inhX > partX) partX = inhX;
	else if (inhX < 0.0f) partX += inhX;
	Point3 addInherVel = partX*partAxe + (partY+inhY)*ortAxe + (partZ+inhZ)*bounceVec - partVec;

	if (normLen < angleCoefSmall*fricLen) // angle of reflection is less than 2 degrees: it's a "slide"
	{
		partVec = partX*partAxe + (partY+inhY)*ortAxe + (partZ+inhZ)*bounceVec;
	}
	else
	{
		float mix = (normLen/fricLen)/angleCoefSmall - 1.0f;
		Point3 addInherVel = partX*partAxe + (partY+inhY)*ortAxe + (partZ+inhZ)*bounceVec - partVec;
		partVec += mix*inhVec + (1.0f-mix)*addInherVel;
	}
}
