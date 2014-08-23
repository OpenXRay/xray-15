#ifndef __SOLIDS__H
#define __SOLIDS__H

#include "Max.h"
#include "scs.h"
#include "dynamic.h"

TCHAR *GetString(int id);
void AddFace(Face *f,int a,int b,int c,int evis,int smooth_group);
void TurnButton(HWND hWnd,int SpinNum,BOOL ison);
void SpinnerOff(HWND hWnd,int SpinNum);
void SpinnerOn(HWND hWnd,int SpinNum);

float RND01();
float RND11();
#define ALLF  4
const float IntMax=32767.0f;
const float IntMax1=32768.0f;
const float HalfIntMax=16383.5f;

const int row3size=3*sizeof(float);

typedef float Matrix4By4[4][4];
typedef float Matrix3By3[3][3];
const Point3 Zero=Point3(0.0f,0.0f,0.0f);

float GetMeterMult();
void Mult1X4(float *A,Matrix4By4 B,float *C);
void Mult1X3(float *A,Matrix3By3 B,float *C);
void SetUpRotation(float *Q, float *W,float Theta,Matrix4By4 Rq);
void RotateOnePoint(float *Pin,float *Q, float *W,float Theta);
void FixFSpinnerLimits(HWND hWnd,int SpinNum,float min,float max,BOOL notify=FALSE);

extern ClassDesc* GetChBoxobjDesc();
extern ClassDesc* GetChCylinderDesc();
extern ClassDesc* GetOilTnkDesc();
extern ClassDesc* GetSpindleDesc();
#ifndef NO_OBJECT_CAPSULE
extern ClassDesc* GetScubaDesc();
#endif
extern ClassDesc* GetGengonDesc();
#ifndef NO_OBJECT_PRISM
extern ClassDesc* GetPrismDesc();
#endif
#ifndef NO_OBJECT_STANDARD_PRIMITIVES
extern ClassDesc* GetPyramidDesc();
#endif
extern ClassDesc* GetCExtDesc();
extern ClassDesc* GetLextDesc();
extern ClassDesc* GetSpringDesc();
extern ClassDesc* GetDamperDesc();
extern ClassDesc* GetHoseDesc();


extern HINSTANCE hInstance;
#endif
