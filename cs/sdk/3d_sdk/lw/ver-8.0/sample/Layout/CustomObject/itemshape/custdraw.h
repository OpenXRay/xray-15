 // Custom Object Class Drawing Routines
// Arnie Cachelin, Copyright 2001 NewTek, Inc.

#include "minvert.h"

void co_Arc(const LWCustomObjAccess *cobjAcc, double pos[3], double startAngle,double endAngle, double rad, int csys, int ax);
void co_FillArc(const LWCustomObjAccess *cobjAcc, double pos[3], double startAngle,double endAngle, double rad, int csys, int ax);
void co_Grid(const LWCustomObjAccess *cobjAcc, double pos[3], double siz[3], int div, int csys, int axis);
void co_FillRect(const LWCustomObjAccess *cobjAcc, double pos[3], double w, double h, int csys, int axis);
void co_Rectangle(const LWCustomObjAccess *cobjAcc, double pos[3], double w, double h, int csys, int axis);
void co_Line(const LWCustomObjAccess *cobjAcc, LWDVector p0, LWDVector p1, int csys);

#define HUDF_FILL		1
#define HUDF_SIDE		2
#define HUDF_DOWN		4
#define HUDF_LOCK		1024
#define HUDF_OVERMAX		2048
#define HUDF_UNDERMIN		4096

double HUD_Depth(LWViewportInfo *ViewGlobal, int view);
double HUD_Transform(LWViewportInfo *ViewGlobal, int view, Matrix xf);
void HUDPosition(LWViewportInfo *ViewGlobal, int view, LWDVector pos, const LWDVector dir);
void HUD_Point(LWViewportInfo *ViewGlobal, const LWCustomObjAccess *cob, Matrix m, LWDVector pt);
void HUD_Box(LWViewportInfo *ViewGlobal, const LWCustomObjAccess *cob, Matrix m, LWDVector corn, double w, double h, int flags);
void HUD_Knob(LWViewportInfo *ViewGlobal, const LWCustomObjAccess *cob, Matrix m, LWDVector cent, double siz, int flags);
void HUD_Line(LWViewportInfo *ViewGlobal, const LWCustomObjAccess *cob, Matrix m, LWDVector corn, double w, int flags);
