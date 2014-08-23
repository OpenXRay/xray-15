/**********************************************************************
 *<
	FILE: sflectrs.cpp

	DESCRIPTION: SpawnFlectors common source

	CREATED BY: Eric Peterson

	HISTORY: 7/97

 **********************************************************************/
#include "sflectr.h"

HINSTANCE hInstance;
static int controlsInit = FALSE;


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return _T("SpawnFlectors objects (Discreet)"); }

// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int 
//LibNumberClasses() { return 15; }
LibNumberClasses() { return 12; } // removed monoflector

// This function return the ith class descriptor. We have one.
/*__declspec( dllexport ) ClassDesc* 
LibClassDesc(int i) {
	switch(i){
	case  0:return GetPSpawnDeflObjDesc();
	case  1:return GetPSpawnDeflModDesc();
	case  2:return GetSSpawnDeflObjDesc();
	case  3:return GetSSpawnDeflModDesc();
	case  4:return GetUSpawnDeflObjDesc();
	case  5:return GetUSpawnDeflModDesc();
	case  6:return GetPDynaDeflObjDesc();
	case  7:return GetPDynaDeflModDesc();
	case  8:return GetSDynaDeflObjDesc();
	case  9:return GetSDynaDeflModDesc();
	case 10:return GetUDynaDeflObjDesc();
	case 11:return GetUDynaDeflModDesc();

    default:return 0;}
 }*/
__declspec( dllexport ) ClassDesc* 
LibClassDesc(int i) {
	switch(i){
	case  0:return GetPSpawnDeflObjDesc();
	case  1:return GetPSpawnDeflModDesc();
	case  2:return GetPDynaDeflObjDesc();
	case  3:return GetPDynaDeflModDesc();
	case  4:return GetSSpawnDeflObjDesc();
	case  5:return GetSSpawnDeflModDesc();
	case  6:return GetSDynaDeflObjDesc();
	case  7:return GetSDynaDeflModDesc();
	case  8:return GetUSpawnDeflObjDesc();
	case  9:return GetUSpawnDeflModDesc();
	case 10:return GetUDynaDeflObjDesc();
	case 11:return GetUDynaDeflModDesc();
//	case 12:return GetBasicFlectorObjDesc();
//	case 13:return GetMonoFlectorDesc();
//	case 14:return GetBasicFlectorModDesc();
    default:return 0;}
 }
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;

  	if (!controlsInit) {
		controlsInit = TRUE;		

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

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
/* rand returns a number between 0 and 32767 */
/* number between 0 and 1 */
const float IntMax=32767.0f;
const float IntMax1=32768.0f;
const float HalfIntMax=16383.5f;
typedef float Matrix3By3[3][3];
typedef float Matrix4By3[4][3];
Point3 Zero=Point3(0.0f,0.0f,0.0f); 

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

void VNormal::AddNormal(Point3 &n,DWORD s)
	{
	if (!(s&smooth) && init) {
		if (next) next->AddNormal(n,s);
		else {
			next = new VNormal(n,s);
			}
	} else {
		norm   += n;
		smooth |= s;
		init    = TRUE;
		}
	}

Point3 &VNormal::GetNormal(DWORD s)
	{
	if (smooth&s || !next) return norm;
	else return next->GetNormal(s);	
	}

void VNormal::Normalize()
	{
	VNormal *ptr = next, *prev = this;
	while (ptr) {
		if (ptr->smooth&smooth) {
			norm += ptr->norm;			
			prev->next = ptr->next;
			delete ptr;
			ptr = prev->next;
		} else {
			prev = ptr;
			ptr  = ptr->next;
			}
		}
	norm = ::Normalize(norm);
	if (next) next->Normalize();
	}

void GetVFLst(Mesh* dmesh,VNormal* vnorms,Point3* fnorms)	 
{ int nv=dmesh->getNumVerts();	
  int nf=dmesh->getNumFaces();	
  Face *face = dmesh->faces;
  for (int i=0; i<nv; i++) 
    vnorms[i] = VNormal();
  Point3 v0, v1, v2;
  for (i=0; i<nf; i++,face++) 
  {	// Calculate the surface normal
	v0 = dmesh->verts[face->v[0]];
	v1 = dmesh->verts[face->v[1]];
	v2 = dmesh->verts[face->v[2]];
	fnorms[i] = (v1-v0)^(v2-v1);
	for (int j=0; j<3; j++) 
	   vnorms[face->v[j]].AddNormal(fnorms[i],face->smGroup);
    fnorms[i] = Normalize(fnorms[i]);
  }
  for (i=0; i<nv; i++) 
	vnorms[i].Normalize();
}

int RayIntersectP(Ray& ray, float& at, Point3& norm,Mesh *amesh,VNormal* vnorms,Point3 *fnorms)
	{
	DWORD fi;
	Point3 bary;
	Face *face = amesh->faces;	
	Point3 v0, v1, v2;
	Point3 n, sum, p, bry;
	float d, rn, a;
	Matrix3 vTM(1);
	BOOL first = FALSE;
	fi = 0xFFFFFFFF;

	for (int i=0; i<amesh->getNumFaces(); i++,face++) {
		n = fnorms[i];
		
		// See if the ray intersects the plane (backfaced)
		rn = DotProd(ray.dir,n);
		if (rn > -EPSILON) continue;
		
		// Use a point on the plane to find d
		d = DotProd(amesh->verts[face->v[0]],n);

		// Find the point on the ray that intersects the plane
		a = (d - DotProd(ray.p,n)) / rn;

		// Must be positive...
		if (a < 0.0f) continue;

		// Must be closer than the closest at so far
		if (first) {
			if (a > at) continue;
			}

		// The point on the ray and in the plane.
		p = ray.p + a*ray.dir;

		// Compute barycentric coords.
		bry = amesh->BaryCoords(i,p);

		// barycentric coordinates must sum to 1 and each component must
		// be in the range 0-1
		if (bry.x<0.0f || bry.x>1.0f || bry.y<0.0f || bry.y>1.0f || bry.z<0.0f || bry.z>1.0f) continue;
		if (fabs(bry.x + bry.y + bry.z - 1.0f) > EPSILON) continue;

		// Hit!
		first = TRUE;		
		at    = a;
		fi    = (DWORD)i;		
//		bary.x  = bry.z;
//		bary.y  = bry.x;
//		bary.z  = bry.y;
		bary  = bry;	// DS 3/8/97
		
		// Use interpolated normal instead.
		if (!face->smGroup) {
			norm  = n;
		} else {
			norm = 
				vnorms[face->v[0]].GetNormal(face->smGroup) * bary.x +
				vnorms[face->v[1]].GetNormal(face->smGroup) * bary.y +
				vnorms[face->v[2]].GetNormal(face->smGroup) * bary.z;
			norm = Normalize(norm);
			}
		}

	return first;
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

/* number between -1 and 1 */
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

void VectorVar(Point3 *vel,float R,float MaxAngle)
{ 
    // Martell 4/14/01: Fix for order of ops bug.
  float z=RND11(); float y=RND11(); float x=RND11();
  Point3 X=Point3(x,y,z);
  Point3 c=Normalize(X^*vel);
  float Theta=MaxAngle*R*RND01();
  RotateOnePoint(&(*vel).x,&Zero.x,&c.x,Theta);
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