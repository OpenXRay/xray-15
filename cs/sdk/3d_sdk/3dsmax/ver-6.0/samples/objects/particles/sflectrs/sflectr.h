#ifndef __SFLECTRS__H
#define __SFLECTRS__H

#include "Max.h"
#include "sflectrs.h"
#include "Simpobj.h"
#include "dynamic.h"
#include "macrorec.h"

TCHAR *GetString(int id);

extern ClassDesc* GetPSpawnDeflObjDesc();
extern ClassDesc* GetPSpawnDeflModDesc();
extern ClassDesc* GetSSpawnDeflObjDesc();
extern ClassDesc* GetSSpawnDeflModDesc();
extern ClassDesc* GetUSpawnDeflObjDesc();
extern ClassDesc* GetUSpawnDeflModDesc();
extern ClassDesc* GetPDynaDeflObjDesc();
extern ClassDesc* GetPDynaDeflModDesc();
extern ClassDesc* GetSDynaDeflObjDesc();
extern ClassDesc* GetSDynaDeflModDesc();
extern ClassDesc* GetUDynaDeflObjDesc();
extern ClassDesc* GetUDynaDeflModDesc();
//extern ClassDesc* GetBasicFlectorObjDesc();
//extern ClassDesc* GetMonoFlectorDesc();
//extern ClassDesc* GetBasicFlectorModDesc();

extern HINSTANCE hInstance;
extern int RNDSign();
extern float RND01();
extern float RND11();
extern int RND0x(int maxnum);

const float FLOAT_EPSILON=0.005f;
const float HalfPI=1.570796327f;
const float PIOver5=0.62831853f;
const float FTOIEPS=0.000001f;
const float PRECISION_LIMIT=1.0e-15f;
const float SQR2=1.1414f;
const float SMALL_EPSILON=0.005f;
const int row3size=3*sizeof(float);
const Point3 v111=Point3(0.450f,0.218f,0.732f);
extern Point3 Zero;
const Point3 deftex=Point3(0.5f,0.5f,0.0f);
const int NoAni=-9999;

typedef float Matrix4By4[4][4];
typedef float Matrix3By4[3][4];

static Matrix3 ident(1);

class VNormal {
	public:
		Point3 norm;
		DWORD smooth;
		VNormal *next;
		BOOL init;

		VNormal() {smooth=0;next=NULL;init=FALSE;norm=Point3(0,0,0);}
		VNormal(Point3 &n,DWORD s) {next=NULL;init=TRUE;norm=n;smooth=s;}
		~VNormal() {delete next;}
		void AddNormal(Point3 &n,DWORD s);
		Point3 &GetNormal(DWORD s);
		void Normalize();
	};
extern void GetVFLst(Mesh* dmesh,VNormal* vnorms,Point3* fnorms);
extern int RayIntersectP(Ray& ray, float& at, Point3& norm,Mesh *amesh,VNormal* vnorms,Point3 *fnorms);
extern void Mult4X4(Matrix4By4 A,Matrix4By4 B,Matrix4By4 C);
extern void RotatePoint(Matrix3By4 Pin,float *Q, float *W,float Theta);
extern void RotateOnePoint(float *Pin,float *Q, float *W,float Theta);
extern int FloatEQ0(float number);
extern int SmallerEQ0(float number);
extern int FGT0(Point3 p1);
extern void Mult4X1(float *A,Matrix4By4 B,float *C);
extern int MatrixInvert(Matrix4By4 in,Matrix4By4 out);
extern INT_PTR CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
extern void TurnButton(HWND hWnd,int SpinNum,BOOL ison);
extern void SpinnerOn(HWND hWnd,int SpinNum,int Winnum);
extern void SpinnerOff(HWND hWnd,int SpinNum,int Winnum);
extern Point3 CalcSpread(float divangle,Point3 oldnorm);
#define MAXNAME 24
extern void FormatName(TCHAR *name);
extern void ZeroMesh(Mesh *pm);
extern TriObject *TriIsUseable(Object *pobj,TimeValue t);
extern void VectorVar(Point3 *vel,float R,float MaxAngle);
extern void MakeNodeList(INode *node,INodeTab *ntab,int subtree,TimeValue t);
extern void MakeGroupNodeList(INode *node,INodeTab *ntab,int subtree,TimeValue t);
extern void FormatName(TCHAR *name);
extern float FricCoef(float friction, float normLen, float fricLen, float contactDur);
extern void AddInheritVelocity(Point3 &bounceVec, Point3 &frictionVec, Point3 &partVec,
						Point3 &inheritVec, float vinher, float normLen, float fricLen);

#endif
