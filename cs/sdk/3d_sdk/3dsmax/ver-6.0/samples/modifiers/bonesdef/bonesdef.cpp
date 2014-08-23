 /**********************************************************************
 
	FILE: BonesDef.cpp

	DESCRIPTION:  Simple Bones Deformation Plugin

	CREATED BY: Peter Watje

	HISTORY: 8/5/98


 *>	Copyright (c) 1998, All Rights Reserved.


**********************************************************************


5.1.01 adds left/right justification for bone names in the weight table
       adds an action item for right/left justification
	   fixes bug 471559 which was envelope load crasher when there were more inc bones
	   than target bones
5.1.02 fixes bug 481214 which  a load by index to the envelope load dialog bug 
5.1.03 fixes bug 483252 which allows us to toggle the stretch on/off	   





**********************************************************************/


#include "max.h"
#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "surf_api.h"
#include "polyobj.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"
#include "macrorec.h"
#include "modstack.h"
#include "ISkin.h"
#include "MaxIcon.h"



#ifdef _DEBUG
	#undef _DEBUG
	#include <atlbase.h>
	#define _DEBUG
#else
	#include <atlbase.h>
#endif


HWND BonesDefMod::hParam  = NULL;
HWND BonesDefMod::hParamAdvance  = NULL;
HWND BonesDefMod::hParamGizmos = NULL;
CreateCrossSectionMode* BonesDefMod::CrossSectionMode   = NULL;
//CreatePaintMode*        BonesDefMod::PaintMode   = NULL;
ICustButton* BonesDefMod::iCrossSectionButton   = NULL;
ICustButton* BonesDefMod::iEditEnvelopes   = NULL;
ICustButton* BonesDefMod::iLock   = NULL;
ICustButton* BonesDefMod::iAbsolute   = NULL;
ICustButton* BonesDefMod::iEnvelope   = NULL;
ICustButton* BonesDefMod::iFalloff   = NULL;
ICustButton* BonesDefMod::iCopy   = NULL;
ICustButton* BonesDefMod::iPaste   = NULL;
ICustButton* BonesDefMod::iPaintButton  = NULL;
ICustToolbar* BonesDefMod::iParams = NULL;
IGizmoBuffer* BonesDefMod::copyGizmoBuffer = NULL;


static GenSubObjType SOT_DefPoints(32);

//--- ClassDescriptor and class vars ---------------------------------

//IParamMap       *BonesDefMod::pmapParam = NULL;
IObjParam       *BonesDefMod::ip        = NULL;
BonesDefMod     *BonesDefMod::editMod   = NULL;
MoveModBoxCMode *BonesDefMod::moveMode  = NULL;







int MyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
//7-1-99
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
		Nodes.Append(1, (INode **)&rmaker);  
		count++;
		}


	return 0;
	}	


//watje 10-13-99 212156
BOOL BonesDefMod::DependOnTopology(ModContext &mc) {
	BoneModData *bmd = (BoneModData*)mc.localData;
	BOOL topo = FALSE;
	if (bmd) 
		for ( int i =0; i <bmd->VertexData.Count(); i++)
			{
			if (bmd->VertexData[i]->IsModified())
				{
				topo = TRUE;
				i = bmd->VertexData.Count();
				}

			}
	return topo;
}


int BonesDefMod::NumRefs() {
	int ct = refHandleList.Count();
	if (refHandleList.Count() ==0)
		return 7;
	else return refHandleList.Count()+5;

	ct = 1;

	for (int i = 0; i<BoneData.Count();i++)
		{

		if (BoneData[i].RefEndPt1ID > ct) ct = BoneData[i].RefEndPt1ID;
		if (BoneData[i].RefEndPt2ID > ct) ct = BoneData[i].RefEndPt2ID;
		if (BoneData[i].BoneRefID > ct) ct = BoneData[i].BoneRefID;
		for (int j = 0; j<BoneData[i].CrossSectionList.Count();j++)
			{
			if (BoneData[i].CrossSectionList[j].RefInnerID > ct) ct = BoneData[i].CrossSectionList[j].RefInnerID;
			if (BoneData[i].CrossSectionList[j].RefOuterID > ct) ct = BoneData[i].CrossSectionList[j].RefOuterID;
			}
		}
	return (ct+5);
	}



//watje 9-7-99  198721 
class ReevalModEnumProc : public ModContextEnumProc {
public:
	BonesDefMod *lm;
	BOOL ev;
	BOOL rd;
	ReevalModEnumProc(BonesDefMod *l, BOOL e, BOOL r)
		{
		lm = l;
		ev = e;
		rd = r;
		}
private:
	BOOL proc (ModContext *mc);
};

BOOL ReevalModEnumProc::proc (ModContext *mc) {
	if (mc->localData == NULL) return TRUE;

	BoneModData *bmd = (BoneModData *) mc->localData;
	bmd->reevaluate = ev;
	return TRUE;
}

//watje 9-7-99  198721 
void BonesDefMod::Reevaluate(BOOL eval)
{
ReevalModEnumProc lmdproc(this,eval,FALSE);
EnumModContexts(&lmdproc);
}



class ForceRecomputeModEnumProc : public ModContextEnumProc {
public:
	BonesDefMod *lm;
	BOOL ev;
	ForceRecomputeModEnumProc(BonesDefMod *l, BOOL e)
		{
		lm = l;
		ev = e;
		}
private:
	BOOL proc (ModContext *mc);
};

BOOL ForceRecomputeModEnumProc::proc (ModContext *mc) {
	if (mc->localData == NULL) return TRUE;

	BoneModData *bmd = (BoneModData *) mc->localData;
	bmd->forceRecomuteBaseNode = ev;
	return TRUE;
}

//watje 9-7-99  198721 
void BonesDefMod::ForceRecomuteBaseNode(BOOL eval)
{
ForceRecomputeModEnumProc lmdproc(this,eval);
EnumModContexts(&lmdproc);
}

void BonesDefMod::CopyBone()
{

if ((ModeBoneIndex != -1) && (BoneData.Count() > 0))
	{
//get end point1
	Interval v;
	BoneData[ModeBoneIndex].EndPoint1Control->GetValue(currentTime,&CopyBuffer.E1,v,CTRL_ABSOLUTE);
//get end point2
	BoneData[ModeBoneIndex].EndPoint2Control->GetValue(currentTime,&CopyBuffer.E2,v,CTRL_ABSOLUTE);
//need to set in local space

	CopyBuffer.absolute = FALSE;
	if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
		CopyBuffer.absolute = TRUE;

	CopyBuffer.showEnvelope = FALSE;
	if (BoneData[ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
		CopyBuffer.showEnvelope = TRUE;
	
	CopyBuffer.falloffType = BoneData[ModeBoneIndex].FalloffType;


//get cross sections
	CopyBuffer.CList.ZeroCount();
	for (int i = 0; i < BoneData[ModeBoneIndex].CrossSectionList.Count();i++)
		{
		CopyCrossClass c;
		c.u = BoneData[ModeBoneIndex].CrossSectionList[i].u;
		BoneData[ModeBoneIndex].CrossSectionList[i].InnerControl->GetValue(currentTime,&c.inner,v);
		BoneData[ModeBoneIndex].CrossSectionList[i].OuterControl->GetValue(currentTime,&c.outer,v);
		CopyBuffer.CList.Append(1,&c,1);
		}

	if ((iPaste!=NULL) && (CopyBuffer.CList.Count() != 0))
		iPaste->Enable();

	}
}

void BonesDefMod::PasteBone()
{
if (ModeBoneIndex != -1)
	{
//transform end points back

//Delete all old cross sections
	int ct = BoneData[ModeBoneIndex].CrossSectionList.Count();

	if (CopyBuffer.CList.Count() == 0) return;

	CopyBuffer.E1 = CopyBuffer.E1;
	CopyBuffer.E2 = CopyBuffer.E2;

	BoneData[ModeBoneIndex].FalloffType = CopyBuffer.falloffType;

	if (CopyBuffer.absolute)
		BoneData[ModeBoneIndex].flags |= BONE_ABSOLUTE_FLAG;
	else BoneData[ModeBoneIndex].flags &= ~BONE_ABSOLUTE_FLAG;

	if (CopyBuffer.showEnvelope)
		BoneData[ModeBoneIndex].flags |= BONE_DRAW_ENVELOPE_FLAG;
	else BoneData[ModeBoneIndex].flags &= ~BONE_DRAW_ENVELOPE_FLAG;


	UpdatePropInterface();

	for (int i =(ct-1); i >= 0 ; i--)
		RemoveCrossSection(ModeBoneIndex, i);
	for (i =0; i < CopyBuffer.CList.Count() ; i++)
		{
		AddCrossSection(ModeBoneIndex,CopyBuffer.CList[i].u,CopyBuffer.CList[i].inner,CopyBuffer.CList[i].outer);
		}

	}
}

void BonesDefMod::PasteToSomeBones()
{

if (CopyBuffer.CList.Count() == 0) return;
int ctl = pasteList.Count();
for (int m =0; m < ctl; m ++)	
	{
	int k = ConvertSelectedListToBoneID(pasteList[m]);
//transform end points back
	if ((k < BoneData.Count()) && (BoneData[k].Node))
		{
//Delete all old cross sections
		int ct = BoneData[k].CrossSectionList.Count();


		CopyBuffer.E1 = CopyBuffer.E1;
		CopyBuffer.E2 = CopyBuffer.E2;

		BoneData[k].FalloffType = CopyBuffer.falloffType;

		if (CopyBuffer.absolute)
			BoneData[k].flags |= BONE_ABSOLUTE_FLAG;
		else BoneData[k].flags &= ~BONE_ABSOLUTE_FLAG;

		if (CopyBuffer.showEnvelope)
			BoneData[k].flags |= BONE_DRAW_ENVELOPE_FLAG;
		else BoneData[k].flags &= ~BONE_DRAW_ENVELOPE_FLAG;


		UpdatePropInterface();

		for (int i =(ct-1); i >= 0 ; i--)
			RemoveCrossSection(k, i);
		for (i =0; i < CopyBuffer.CList.Count() ; i++)
			{
			AddCrossSection(k,CopyBuffer.CList[i].u,CopyBuffer.CList[i].inner,CopyBuffer.CList[i].outer);
			}

		}
	}
}


void BonesDefMod::PasteToAllBones()
{

if (CopyBuffer.CList.Count() == 0) return;

for (int k =0; k < BoneData.Count(); k ++)	
	{
//transform end points back
	if (BoneData[k].Node)
		{
//Delete all old cross sections
		int ct = BoneData[k].CrossSectionList.Count();


		CopyBuffer.E1 = CopyBuffer.E1;
		CopyBuffer.E2 = CopyBuffer.E2;

		BoneData[k].FalloffType = CopyBuffer.falloffType;

		if (CopyBuffer.absolute)
			BoneData[k].flags |= BONE_ABSOLUTE_FLAG;
		else BoneData[k].flags &= ~BONE_ABSOLUTE_FLAG;

		if (CopyBuffer.showEnvelope)
			BoneData[k].flags |= BONE_DRAW_ENVELOPE_FLAG;
		else BoneData[k].flags &= ~BONE_DRAW_ENVELOPE_FLAG;


		UpdatePropInterface();

		for (int i =(ct-1); i >= 0 ; i--)
			RemoveCrossSection(k, i);
		for (i =0; i < CopyBuffer.CList.Count() ; i++)
			{
			AddCrossSection(k,CopyBuffer.CList[i].u,CopyBuffer.CList[i].inner,CopyBuffer.CList[i].outer);
			}

		}
	}
}




void BonesDefMod::AddCrossSection(int BoneIndex, float u, float inner, float outer, BOOL update)

{
class CrossSectionClass t;
int index = -1;
t. u = u;


int CrossInnerRefID = GetOpenID();
int CrossOuterRefID = GetOpenID();
t.RefInnerID = CrossInnerRefID;
t.RefOuterID  = CrossOuterRefID;
t.InnerControl = NULL;
t.OuterControl = NULL;


if ( (BoneData[BoneIndex].CrossSectionList.Count() == 0) || (BoneData[BoneIndex].CrossSectionList.Count() == 1)) 
	{
	index = BoneData[BoneIndex].CrossSectionList.Count();
	BoneData[BoneIndex].CrossSectionList.Append(1,&t,1);	
	}
else
	{
	for (int i = 0; i < BoneData[BoneIndex].CrossSectionList.Count();i++)
		{
		if (BoneData[BoneIndex].CrossSectionList[i].u>=u)
			{
			index =i;
			i = BoneData[BoneIndex].CrossSectionList.Count();
			}
		}
	if (index ==-1)
		{
		BoneData[BoneIndex].CrossSectionList.Append(1,&t);	
		index = BoneData[BoneIndex].CrossSectionList.Count()-1;
		}
	else BoneData[BoneIndex].CrossSectionList.Insert(index,1,&t);	
	}
//create 2 float controls
MakeRefByID(FOREVER,CrossInnerRefID,NewDefaultFloatController());
MakeRefByID(FOREVER,CrossOuterRefID,NewDefaultFloatController());

AddToRefHandleList(CrossInnerRefID, BoneData[BoneIndex].CrossSectionList[index].InnerControl);
AddToRefHandleList(CrossOuterRefID, BoneData[BoneIndex].CrossSectionList[index].OuterControl);


BoneData[BoneIndex].CrossSectionList[index].InnerControl->SetValue(currentTime,&inner,TRUE,CTRL_ABSOLUTE);
BoneData[BoneIndex].CrossSectionList[index].OuterControl->SetValue(currentTime,&outer,TRUE,CTRL_ABSOLUTE);

if (update)
	NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

/*
float ti,to;
Interval v;
BoneData[BoneIndex].CrossSectionList[index].InnerControl->GetValue(0,&ti,v);
BoneData[BoneIndex].CrossSectionList[index].OuterControl->GetValue(0,&to,v);
*/



}

void BonesDefMod::AddCrossSection(float u)

{
//get current selected bone
// compute the falloff at the u of this bone
class CrossSectionClass t;

float ui,uo,li,lo;
float u_dist;
int index = -1;
t.u = u;
for (int i = 0; i < BoneData[ModeBoneIndex].CrossSectionList.Count();i++)
	{
	if (BoneData[ModeBoneIndex].CrossSectionList[i].u>=u)
		{
		index =i;
		i = BoneData[ModeBoneIndex].CrossSectionList.Count();
		}
	}

int lowerbound, upperbound;
lowerbound = index-1;
upperbound = index;
Interval v;
BoneData[ModeBoneIndex].CrossSectionList[lowerbound].InnerControl->GetValue(currentTime,&li,v);
BoneData[ModeBoneIndex].CrossSectionList[lowerbound].OuterControl->GetValue(currentTime,&lo,v);
BoneData[ModeBoneIndex].CrossSectionList[upperbound].InnerControl->GetValue(currentTime,&ui,v);
BoneData[ModeBoneIndex].CrossSectionList[upperbound].OuterControl->GetValue(currentTime,&uo,v);

u_dist = BoneData[ModeBoneIndex].CrossSectionList[upperbound].u - BoneData[ModeBoneIndex].CrossSectionList[lowerbound].u;
u = (u-BoneData[ModeBoneIndex].CrossSectionList[lowerbound].u) /u_dist;
float Inner = (ui-li) * u + li;
float Outer = (uo-lo) * u + lo;

int CrossInnerRefID = GetOpenID();
int CrossOuterRefID = GetOpenID();
t.RefInnerID = CrossInnerRefID;
t.RefOuterID  = CrossOuterRefID;
t.InnerControl = NULL;
t.OuterControl = NULL;
//create 2 float controls
BoneData[ModeBoneIndex].CrossSectionList.Insert(index,1,&t);



MakeRefByID(FOREVER,CrossInnerRefID,NewDefaultFloatController());
MakeRefByID(FOREVER,CrossOuterRefID,NewDefaultFloatController());

AddToRefHandleList(CrossInnerRefID, BoneData[ModeBoneIndex].CrossSectionList[index].InnerControl);
AddToRefHandleList(CrossOuterRefID, BoneData[ModeBoneIndex].CrossSectionList[index].OuterControl);

BoneData[ModeBoneIndex].CrossSectionList[index].InnerControl->SetValue(currentTime,&Inner,TRUE,CTRL_ABSOLUTE);
BoneData[ModeBoneIndex].CrossSectionList[index].OuterControl->SetValue(currentTime,&Outer,TRUE,CTRL_ABSOLUTE);

if (index <= ModeBoneEnvelopeIndex)
	{
	ModeBoneEnvelopeIndex++;
	if (ModeBoneEnvelopeIndex >= BoneData[ModeBoneIndex].CrossSectionList.Count())
		ModeBoneEnvelopeIndex = BoneData[ModeBoneIndex].CrossSectionList.Count()-1;
	}
NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

}


void BonesDefMod::GetCrossSectionRanges(float &inner, float &outer, int BoneID, int CrossID)

{

Interval v;
BoneData[BoneID].CrossSectionList[CrossID].InnerControl->GetValue(currentTime,&inner,v);
BoneData[BoneID].CrossSectionList[CrossID].OuterControl->GetValue(currentTime,&outer,v);

}

float BonesDefMod::GetU(ViewExp *vpt,Point3 a, Point3 b, IPoint2 p)
{
//mouse spot
INodeTab nodes;
Point2 fp = Point2((float)p.x, (float)p.y);
float u;
if ( !ip ) return 0.0f;

GraphicsWindow *gw = vpt->getGW();
gw->setTransform(Matrix3(1));
Point2 spa = ProjectPointF(gw, a);
Point2 spb = ProjectPointF(gw, b);
u = Length(spa-fp)/Length(spa-spb);

return u;


}





void BonesDefMod::GetEndPoints(BoneModData *bmd, TimeValue t, Point3 &l1, Point3 &l2, int BoneID)
{

ObjectState os;
ShapeObject *pathOb = NULL;
if ((BoneData[BoneID].flags & BONE_SPLINE_FLAG) && (BoneData[BoneID].Node != NULL) )
	{
	ObjectState os = BoneData[BoneID].Node->EvalWorldState(t);
	pathOb = (ShapeObject*)os.obj;
//196241 
	if (pathOb->NumberOfCurves() == 0) 
		{
		l1 = Point3(0.0f,0.0f,0.0f);
		l2 = Point3(0.0f,0.0f,0.0f);
		return;
		}

	l1  = pathOb->InterpPiece3D(t, 0,0 ,0.0f ) * Inverse(BoneData[BoneID].tm);
	l2  = pathOb->InterpPiece3D(t, 0,0 ,1.0f ) * Inverse(BoneData[BoneID].tm);
	}
else
	{
	l1 = bmd->tempTableL1[BoneID];
	l2 = bmd->tempTableL2[BoneID];
	}

}



void BonesDefMod::GetEndPointsLocal(BoneModData *bmd, TimeValue t, Point3 &l1, Point3 &l2, int BoneID)
{

if ((BoneData[BoneID].flags & BONE_SPLINE_FLAG) && (BoneData[BoneID].Node != NULL) )
	{
	ShapeObject *pathOb = NULL;
	ObjectState os = BoneData[BoneID].Node->EvalWorldState(t);
	pathOb = (ShapeObject*)os.obj;
//196241 
	if (pathOb->NumberOfCurves() == 0) 
		{
		l1 = Point3(0.0f,0.0f,0.0f);
		l2 = Point3(0.0f,0.0f,0.0f);
		return;
		}

	l1  = pathOb->InterpPiece3D(t, 0,0 ,0.0f );
	l2  = pathOb->InterpPiece3D(t, 0,0 ,1.0f );
	}

else
	{
	if (stopMessagePropogation)
		{
		Point3 tl1, tl2;

		Interval v;
		BoneData[BoneID].EndPoint1Control->GetValue(currentTime,&tl1,v);
		BoneData[BoneID].EndPoint2Control->GetValue(currentTime,&tl2,v);
		l1 = tl1 * bmd->tmCacheToObjectSpace[BoneID];
		l2 = tl2 * bmd->tmCacheToObjectSpace[BoneID];
		}
	else
		{
		l1 = bmd->tempTableL1ObjectSpace[BoneID];
		l2 = bmd->tempTableL2ObjectSpace[BoneID];
		}
	
	}

}



float BonesDefMod::ModifyU(TimeValue t, float LineU,  int BoneID, int sid)

{
ObjectState os;
ShapeObject *pathOb = NULL;
if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
	os = BoneData[BoneID].Node->EvalWorldState(t);
	pathOb = (ShapeObject*)os.obj;
//196241 
	if (pathOb->NumberOfCurves() == 0) 
		{
		return 0.0f;
		}
	int SubCount = BoneData[BoneID].referenceSpline.Segments();

	float start,inc;
	inc = 1.0f/(float)SubCount;
	start = inc * sid;
	LineU = start + (LineU * inc);
	}
return LineU;


}

float BonesDefMod::ComputeInfluence(TimeValue t, float Influence, float LineU, int BoneID, int StartCross, int EndCross, int sid)

{
float Inner, Outer;
float LInner, LOuter;

GetCrossSectionRanges(Inner, Outer, BoneID, StartCross);
GetCrossSectionRanges(LInner, LOuter, BoneID, EndCross);


LineU = ModifyU(t, LineU,BoneID,sid);

float udist = BoneData[BoneID].CrossSectionList[EndCross].u - BoneData[BoneID].CrossSectionList[StartCross].u;
LineU = LineU - BoneData[BoneID].CrossSectionList[StartCross].u;
float per = LineU/udist;


Inner = Inner + (LInner - Inner) * per;
Outer = Outer + (LOuter - Outer) * per;


//inside inner envelope
if (Influence <= Inner)
	{
	Influence = 1.0f;
	}
// is it oustide  outer
else if (Influence <= Outer)
	{
	float r1,r2;
	r1 = Outer - Inner;
	r2 = Influence - Inner;
	Influence = 1.0f - (r2/r1);
	ComputeFalloff(Influence,BoneData[BoneID].FalloffType);
	}
//outside puter envelope
	else 
	{
	Influence = 0.0f;
	}
return Influence;

}

int BonesDefMod::ConvertSelectedBoneToListID(int fsel)
{
int sel = 0;
for (int i= 0; i < fsel; i++)
	{
	if (i < BoneData.Count())
		{
		if (BoneData[i].Node != NULL) sel++;
		}
	}
return sel;
}

int BonesDefMod::ConvertSelectedListToBoneID(int fsel)
{
int sel = -1;
int ct = 0;
while ((sel!=fsel) && (ct<BoneData.Count()))
	{
	if (BoneData[ct].Node != NULL) sel++;
	ct++;
	}
return ct-1;
}


void BonesDefMod::RefillListBox()
{

if (!hParam) return;

SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_RESETCONTENT ,0,0);

RECT rect;

GetWindowRect(GetDlgItem(hParam,IDC_LIST1), &rect);
int width = rect.right - rect.left-2;

BOOL shorten;
pblock_advance->GetValue(skin_advance_shortennames,0,shorten,FOREVER);



HDC hdc = GetDC(GetDlgItem(hParam,IDC_LIST1));
HFONT hOldFont = (HFONT)SelectObject(hdc, GetCOREInterface()->GetAppHFont());

for (int i=0;i<BoneData.Count();i++)
	{
	TCHAR title[200];

	if (BoneData[i].Node != NULL)
		{
		Class_ID bid(BONE_CLASS_ID,0);
		ObjectState os = BoneData[i].Node->EvalWorldState(RefFrame);

		if (( os.obj->ClassID() == bid) && (BoneData[i].name.Length()) )
			{
			_tcscpy(title,BoneData[i].name);
			}
		else _tcscpy(title,BoneData[i].Node->GetName());

		SIZE size;
		
		int titleLen = _tcslen(title);
		TSTR tempstr;
		tempstr.printf("%s",title);
		GetTextExtentPoint32(hdc,  (LPCTSTR)tempstr, tempstr.Length(),&size);
		
		if ((shorten) && ( size.cx > width))
			{
			BOOL done = FALSE;
			while (!done)
				{
				int len = _tcslen(title);
				int mid = _tcslen(title)/2;
				for (int j = mid; j < (len); j++)
					title[j] = title[j+1];
				GetTextExtentPoint32(hdc, title,len,(LPSIZE)&size);
				if (size.cx < width) 
					{
					title[mid-1] = '.';
					title[mid] = '.';
					title[mid+1] = '.';
					done = TRUE;
					}
				}



				
			}

		SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_ADDSTRING,0,(LPARAM)(TCHAR*)title);


		}
	}
SelectObject(hdc, hOldFont);
ReleaseDC(GetDlgItem(hParam,IDC_LIST1),hdc);

}

void BonesDefMod::RemoveBone()

{

if (inAddBoneMode) AddFromViewEnd();

int fsel;

fsel = SendMessage(GetDlgItem(hParam,IDC_LIST1),
			LB_GETCURSEL ,0,0);
int sel = ConvertSelectedListToBoneID(fsel);

if (sel>=0)
	{
	SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_DELETESTRING  ,(WPARAM) fsel,0);



//nuke reference

//nuke cross sections
	int ct = BoneData[sel].CrossSectionList.Count();
	for (int i =(ct-1); i >= 0 ; i--)
		RemoveCrossSectionNoNotify(sel, i);

//nuke end points

	DeleteReference(BoneData[sel].RefEndPt1ID);
	BoneData[sel].EndPoint1Control = NULL;
	AddToRefHandleList(BoneData[sel].RefEndPt1ID, NULL);

	DeleteReference(BoneData[sel].RefEndPt2ID);
	BoneData[sel].EndPoint2Control = NULL;
	AddToRefHandleList(BoneData[sel].RefEndPt2ID, NULL);

	RefTable[BoneData[sel].RefEndPt1ID-BONES_REF] = 0;
	RefTable[BoneData[sel].RefEndPt2ID-BONES_REF] = 0;
	recompBoneMap = true; //ns
	DeleteReference(BoneData[sel].BoneRefID);
	RefTable[BoneData[sel].BoneRefID-BONES_REF] = 0;
	BoneData[sel].Node = NULL;
	AddToRefHandleList(BoneData[sel].BoneRefID, NULL);

	BoneData[sel].flags= BONE_DEAD_FLAG;

// bug fix 207093 9/8/99	watje
	BoneData[sel].RefEndPt1ID = -1;
	BoneData[sel].RefEndPt2ID = -1;
	BoneData[sel].BoneRefID = -1;

	
	int NodeCount = BoneData.Count();


	ModeBoneIndex = sel;
	ModeBoneEndPoint = -1;
	ModeBoneEnvelopeIndex = -1;
	ModeBoneEnvelopeSubType = -1;


	int fsel = ConvertSelectedBoneToListID(sel);

// bug fix 206160 9/8/99	watje
	BOOL noBonesLeft = FALSE;
	if (SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_SETCURSEL ,fsel,0) == LB_ERR)
		{
		if (fsel != 0) 
			fsel--;
// bug fix 206160 9/8/99	watje
		else noBonesLeft = TRUE;
		SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_SETCURSEL ,fsel,0);
		}
// bug fix 206160 9/8/99	watje
	if (noBonesLeft)
		fsel = -1;
	else fsel = ConvertSelectedListToBoneID(fsel);
	ModeBoneIndex = fsel;
	ModeBoneEndPoint = -1;
	ModeBoneEnvelopeIndex = -1;
	ModeBoneEnvelopeSubType = -1;
	LastSelected = fsel;
	UpdatePropInterface();
	

	BoneMoved = TRUE;
//watje 9-7-99  198721 
	Reevaluate(TRUE);
	cacheValid = FALSE;
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	}
int bct = 0;
for (int i =0; i < BoneData.Count(); i ++)
	{
	if (!(BoneData[i].flags &  BONE_DEAD_FLAG)) bct++;
	}


if  (bct == 0)
	{
	DisableButtons();
	}

NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

//WEIGHTTABLE
weightTableWindow.RecomputeBones();

}

void BonesDefMod::BuildBaseNodeData()
{
MyEnumProc dep;              
EnumDependents(&dep);
//this puts back the original state of the node vc mods and shade state
int nodeCount = 0;
for (int  i = 0; i < dep.Nodes.Count(); i++)
	{
	BoneModData *bmd = GetBMD(dep.Nodes[i]);
	if (bmd) nodeCount++;
	}

loadBaseNodeData.SetCount(nodeCount);
int ct = 0;
for (i = 0; i < dep.Nodes.Count(); i++)
	{
	BoneModData *bmd = GetBMD(dep.Nodes[i]);
	if (bmd)
		{
		loadBaseNodeData[ct].node = dep.Nodes[i];
		loadBaseNodeData[ct].bmd = bmd;
		loadBaseNodeData[ct].matchData = NULL;
		ct++;
		}
	}
//now match inc data to current data using names				
for (i = 0; i < loadBaseNodeData.Count(); i++)
	{
	if (loadBaseNodeData[i].matchData == NULL)
		{
		for (int j = 0; j < vertexLoadList.Count(); j++)
			{
			TCHAR *name = loadBaseNodeData[i].node->GetName();
			TCHAR *name2 = vertexLoadList[j]->name;
			if ( (_tcscmp(name,name2)==0) || ( (loadBaseNodeData.Count() == 1)  &&(vertexLoadList.Count()==1)))
				{
				loadBaseNodeData[i].matchData = vertexLoadList[j];
				vertexLoadList.Delete(j,1);
				j = vertexLoadList.Count();
				}
			}

		}
	}

//now match inc data to current data using vert count				
for (i = 0; i < loadBaseNodeData.Count(); i++)
	{
	if (loadBaseNodeData[i].matchData == NULL)
		{
		for (int j = 0; j < vertexLoadList.Count(); j++)
			{
			if (loadBaseNodeData[i].bmd->VertexData.Count()== vertexLoadList[j]->vertexData.Count())
				{
				loadBaseNodeData[i].matchData = vertexLoadList[j];
				vertexLoadList.Delete(j,1);
				j = vertexLoadList.Count();

				}
			}

		}
	}
Tab<LoadVertexDataClass*> tempList;
tempList.SetCount(vertexLoadList.Count());
for (i = 0; i < vertexLoadList.Count(); i++)
	{
	tempList[i] = vertexLoadList[i];
	vertexLoadList[i] = NULL;
	}
vertexLoadList.SetCount(loadBaseNodeData.Count());
for (i = 0; i < loadBaseNodeData.Count(); i++)
	{
	vertexLoadList[i] = loadBaseNodeData[i].matchData;
	}
for (i = 0; i < tempList.Count(); i++)
	{
	vertexLoadList.Append(1,&tempList[i]);
	}

	
}

void BonesDefMod::RemoveBone(int bid)

{

if (inAddBoneMode) AddFromViewEnd();

int sel;
sel = bid;



int fsel = ConvertSelectedBoneToListID(sel);


if (sel>=0)
	{
	SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_DELETESTRING  ,(WPARAM) fsel,0);


//nuke reference
//nuke cross sections
	BoneDataClass *b = &BoneData[sel];

	if (theHold.Holding() ) 
		{
		theHold.Put(new DeleteBoneRestore(this,sel));

		MyEnumProc dep;              
		EnumDependents(&dep);
//this puts back the original state of the node vc mods and shade state
		for (int  i = 0; i < dep.Nodes.Count(); i++)
			{
			BoneModData *bmd = GetBMD(dep.Nodes[i]);
			if (bmd)
				theHold.Put(new WeightRestore(this,bmd));
			}
				
		}


	int ct = BoneData[sel].CrossSectionList.Count();
	for (int i =(ct-1); i >= 0 ; i--)
		RemoveCrossSectionNoNotify(sel, i);

//nuke end points
	DeleteReference(BoneData[sel].RefEndPt1ID);
	BoneData[sel].EndPoint1Control = NULL;
	AddToRefHandleList(BoneData[sel].RefEndPt1ID, NULL);

	DeleteReference(BoneData[sel].RefEndPt2ID);
	BoneData[sel].EndPoint2Control = NULL;
	AddToRefHandleList(BoneData[sel].RefEndPt2ID, NULL);

	RefTable[BoneData[sel].RefEndPt1ID-BONES_REF] = 0;
	RefTable[BoneData[sel].RefEndPt2ID-BONES_REF] = 0;

	DeleteReference(BoneData[sel].BoneRefID);
	RefTable[BoneData[sel].BoneRefID-BONES_REF] = 0;
	BoneData[sel].Node = NULL;
	AddToRefHandleList(BoneData[sel].BoneRefID, NULL);

	recompBoneMap = true;	//ns
	BoneData[sel].flags= BONE_DEAD_FLAG;

// bug fix 207093 9/8/99	watje
	BoneData[sel].RefEndPt1ID = -1;
	BoneData[sel].RefEndPt2ID = -1;
	BoneData[sel].BoneRefID = -1;

	
	
	int NodeCount = BoneData.Count();



	ModeBoneEndPoint = -1;
	ModeBoneEnvelopeIndex = -1;
	ModeBoneEnvelopeSubType = -1;


	ModeBoneIndex = sel;
	int fsel = ConvertSelectedBoneToListID(sel);
	SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_SETCURSEL ,fsel,0);


	BoneMoved = TRUE;
//watje 9-7-99  198721 
	Reevaluate(TRUE);
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	}
int bct = 0;
for (int i =0; i < BoneData.Count(); i ++)
	{
	if (!(BoneData[i].flags &  BONE_DEAD_FLAG)) bct++;
	}


if  (bct == 0)
	{
	DisableButtons();
	}

NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

//WEIGHTTABLE
weightTableWindow.RecomputeBones();
}


void BonesDefMod::RemoveCrossSection()

{

if ( (ModeBoneEnvelopeIndex <=0)  || (ModeBoneIndex < 0)  ||
	 (ModeBoneEnvelopeIndex >= (BoneData[ModeBoneIndex].CrossSectionList.Count()-1))
	 )
	return;
BoneDataClass b = BoneData[ModeBoneIndex];

DeleteReference(BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefInnerID);
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl = NULL;
AddToRefHandleList(BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefInnerID, NULL);

DeleteReference(BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefOuterID);
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl = NULL;
AddToRefHandleList(BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefOuterID, NULL);

RefTable[BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefInnerID-BONES_REF] = 0;
RefTable[BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefOuterID-BONES_REF] = 0;

BoneData[ModeBoneIndex].CrossSectionList.Delete(ModeBoneEnvelopeIndex,1);

//watje 9-7-99  198721 
Reevaluate(TRUE);
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);


}


void BonesDefMod::RemoveCrossSection(int bid, int eid)

{


DeleteReference(BoneData[bid].CrossSectionList[eid].RefInnerID);
BoneData[bid].CrossSectionList[eid].InnerControl = NULL;
AddToRefHandleList(BoneData[bid].CrossSectionList[eid].RefInnerID, NULL);


DeleteReference(BoneData[bid].CrossSectionList[eid].RefOuterID);
BoneData[bid].CrossSectionList[eid].OuterControl = NULL;
AddToRefHandleList(BoneData[bid].CrossSectionList[eid].RefOuterID, NULL);

RefTable[BoneData[bid].CrossSectionList[eid].RefInnerID-BONES_REF] = 0;
RefTable[BoneData[bid].CrossSectionList[eid].RefOuterID-BONES_REF] = 0;

BoneData[bid].CrossSectionList.Delete(eid,1);

//watje 9-7-99  198721 
Reevaluate(TRUE);
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

}


void BonesDefMod::RemoveCrossSectionNoNotify(int bid, int eid)

{


DeleteReference(BoneData[bid].CrossSectionList[eid].RefInnerID);
BoneData[bid].CrossSectionList[eid].InnerControl = NULL;
AddToRefHandleList(BoneData[bid].CrossSectionList[eid].RefInnerID, NULL);


DeleteReference(BoneData[bid].CrossSectionList[eid].RefOuterID);
BoneData[bid].CrossSectionList[eid].OuterControl = NULL;
AddToRefHandleList(BoneData[bid].CrossSectionList[eid].RefOuterID, NULL);

RefTable[BoneData[bid].CrossSectionList[eid].RefInnerID-BONES_REF] = 0;
RefTable[BoneData[bid].CrossSectionList[eid].RefOuterID-BONES_REF] = 0;

BoneData[bid].CrossSectionList.Delete(eid,1);

//watje 9-7-99  198721 
Reevaluate(TRUE);

}



//--- Affect region mod methods -------------------------------

BonesDefMod::BonesDefMod() 
	{
//5.1.03
	hasStretchTM = TRUE;
//5.1.02
	loadByIndex = TRUE;

	enableFastSubAnimList = TRUE;
	rebuildSubAnimList = TRUE;

	stopEvaluation = FALSE;

	stopMessagePropogation = FALSE;
	updateOnMouseUp = FALSE;

	updateListBox = FALSE;
	inAddBoneMode=FALSE;
	resolvedModify = TRUE;

	splinePresent = FALSE;
	editing = FALSE;
	inRender = FALSE;
	fastUpdate = FALSE;

	ver = 4;
	recompInitTM = false; //ns
	recompBoneMap = true; //ns


	GetBonesDefModDesc()->MakeAutoParamBlocks(this);
	
	pblock_param->SetValue(skin_effect,0,0.0f);
	pblock_param->SetValue(skin_cross_radius,0,10.0f);

	pblock_param->SetValue(skin_filter_vertices,0,0);
	pblock_param->SetValue(skin_filter_bones,0,1);
	pblock_param->SetValue(skin_filter_envelopes,0,1);



	pblock_display->SetValue(skin_display_draw_all_envelopes,0,0);
	pblock_display->SetValue(skin_display_draw_vertices,0,1);



	pblock_param->SetValue(skin_paint_feather,0,0.7f);
	pblock_param->SetValue(skin_paint_radius,0,24.0f);

	pblock_param->SetValue(skin_paint_str,0,0.1f);

	pblock_advance->SetValue(skin_advance_ref_frame,0,0);
	pblock_advance->SetValue(skin_advance_always_deform,0,1);

	RefTable.ZeroCount();

	BoneData.New();
	//watje 9-7-99  198721 
	Reevaluate(FALSE);
	reset = FALSE;
	BoneMoved = FALSE;
	p1Temp = NULL;
	Point3 p(0.0f,0.0f,0.0f);
	MakeRefByID(FOREVER,POINT1_REF,NewDefaultPoint3Controller()); 
	p1Temp->SetValue(0,p,TRUE,CTRL_ABSOLUTE);
	ModeEdit = 0;
	ModeBoneIndex = -1;
	ModeBoneEndPoint  = -1;
	ModeBoneEnvelopeIndex = -1;
	ModeBoneEnvelopeSubType = -1;
	FilterVertices = 0;
	FilterBones = 0;
	FilterEnvelopes = 0;
	DrawEnvelopes = 0;

	cacheValid = FALSE;
	OldVertexDataCount = 0;
	unlockBone = FALSE;
	unlockAllBones = FALSE;

	painting = FALSE;
	inPaint = FALSE;
	reloadSplines = FALSE;

	splineChanged = FALSE;
	updateP = FALSE;

	bindNode = NULL;
	initialXRefTM.IdentityMatrix();
	xRefTM.IdentityMatrix();

	RegisterNotification(NotifyPreDeleteNode, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(NotifyPreDeleteNode, this, NOTIFY_SYSTEM_SHUTDOWN);

	RegisterNotification(NotifyPreSave, this, NOTIFY_FILE_PRE_SAVE);
	RegisterNotification(NotifyPostSave, this, NOTIFY_FILE_POST_SAVE);

//WEIGHTTABLE
	weightTableWindow.InitMod(this);
	hWeightTable = NULL;
	vcState = FALSE;

	backTransform = TRUE;

	loadVertData = TRUE;
	loadExclusionData = TRUE;

	LastSelected = 0;

//get the painterinterface
	ReferenceTarget *ref  = (ReferenceTarget *) (GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID));	
	if (ref)
		{
		pPainterInterface = static_cast<IPainterInterface_V5 *> (ref->GetInterface(PAINTERINTERFACE_V5));
		}

	
	}


BonesDefMod::~BonesDefMod()
	{

	if (hWeightTable)
		{
//		if (!weightTableWindow.isDocked)
//			weightTableWindow.SaveWindowState();
		weightTableWindow.ClearMod();
		DestroyWindow(hWeightTable);
		}

	DeleteAllRefsFromMe();
	p1Temp = NULL;



	for (int i=0;i<BoneData.Count();i++)
        BoneData[i].CrossSectionList.ZeroCount();

	BoneData.New();
	
//	if (copyGizmoBuffer) delete copyGizmoBuffer;

	for (i = 0; i < splineList.Count(); i++)
		{
		if (splineList[i])
			{
			delete splineList[i];
			splineList[i] = NULL;
			}
		}
	UnRegisterNotification(NotifyPreDeleteNode, this,	NOTIFY_SYSTEM_PRE_RESET);
	UnRegisterNotification(NotifyPreDeleteNode, this,	NOTIFY_SYSTEM_SHUTDOWN);

	UnRegisterNotification(NotifyPreSave, this, NOTIFY_FILE_PRE_SAVE);
	UnRegisterNotification(NotifyPostSave, this, NOTIFY_FILE_POST_SAVE);



	}
int BonesDefMod::RenderBegin(TimeValue t, ULONG flags)
{
inRender= TRUE;

if (fastUpdate)
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
return 0;
}
int BonesDefMod::RenderEnd(TimeValue t)
{
inRender= FALSE;
return 0;
}


RefTargetHandle BonesDefMod::Clone(RemapDir& remap)
	{
	BonesDefMod *mod = new BonesDefMod();
	mod->ReplaceReference(PBLOCK_PARAM_REF,pblock_param->Clone(remap));
	mod->ReplaceReference(POINT1_REF,p1Temp->Clone(remap));

	mod->ReplaceReference(PBLOCK_DISPLAY_REF,pblock_display->Clone(remap));
	mod->ReplaceReference(PBLOCK_GIZMOS_REF,pblock_gizmos->Clone(remap));
	mod->ReplaceReference(PBLOCK_ADVANCE_REF,pblock_advance->Clone(remap));
//WEIGHTTABLE
	mod->ReplaceReference(PBLOCK_WEIGHTTABLE_REF,pblock_weighttable->Clone(remap));
//MIRROR
	mod->ReplaceReference(PBLOCK_MIRROR_REF,pblock_mirror->Clone(remap));

	

//copy controls
	mod->RefTable = RefTable;

	for (int i = 0; i<BoneData.Count(); i++)
		{
		BoneDataClass b = BoneData[i];
		b.Node = NULL;
		b.EndPoint1Control= NULL;
		b.EndPoint2Control = NULL;
		b.CrossSectionList.SetCount(BoneData[i].CrossSectionList.Count());
		for (int j = 0; j < b.CrossSectionList.Count(); j++)
			{
			b.CrossSectionList[j].InnerControl = NULL;
			b.CrossSectionList[j].OuterControl = NULL;
			}
		mod->BoneData.Append(b);
		}


	mod->refHandleList.SetCount(refHandleList.Count());
	for (i=0; i < refHandleList.Count(); i++)
		mod->refHandleList[i]= NULL;

	for (i=0;i < BoneData.Count();i++)
		{
		if (BoneData[i].EndPoint1Control)
			mod->ReplaceReference(BoneData[i].RefEndPt1ID,BoneData[i].EndPoint1Control->Clone(remap));
		if (BoneData[i].EndPoint2Control)
			mod->ReplaceReference(BoneData[i].RefEndPt2ID,BoneData[i].EndPoint2Control->Clone(remap));
		if (BoneData[i].Node)
			{
			if( remap.FindMapping( BoneData[i].Node )  ) 
				{
				mod->ReplaceReference (BoneData[i].BoneRefID, remap.CloneRef(BoneData[i].Node) );
				}
			else mod->ReplaceReference(BoneData[i].BoneRefID,BoneData[i].Node);
			}

		for (int j=0;j < BoneData[i].CrossSectionList.Count();j++)
			{
			if (BoneData[i].CrossSectionList[j].InnerControl)
				mod->ReplaceReference(BoneData[i].CrossSectionList[j].RefInnerID,BoneData[i].CrossSectionList[j].InnerControl->Clone(remap));
			if (BoneData[i].CrossSectionList[j].OuterControl)
				mod->ReplaceReference(BoneData[i].CrossSectionList[j].RefOuterID,BoneData[i].CrossSectionList[j].OuterControl->Clone(remap));
			}

		}



	mod->cacheValid = FALSE;

	for (i = 0; i < pblock_gizmos->Count(skin_gizmos_list) ; i++)
		{
		ReferenceTarget *ref;
		ref = mod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,i);
		GizmoClass *gizmo = (GizmoClass *)ref;
		gizmo->bonesMod = mod;
		mod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,i);
		}




	BaseClone(this, mod, remap);
	return mod;
	}



class NullView: public View {
	public:
		Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
		NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
	};


//void BonesDefMod::BuildEnvelopes(INode *bnode, INode *mnode, Point3 l1, Point3 l2, float &el1, float &el2)
void BonesDefMod::BuildEnvelopes(INode *bnode, Object *obj, Point3 l1, Point3 l2, float &el1, float &el2)

{
float closestEnd = -1.0f,closestStart = -1.0f,closestMid = -1.0f;

Matrix3 tm = bnode->GetObjectTM(RefFrame);

l1 = l1 * Inverse(tm);
l2 = l2 * Inverse(tm);

for (int i =0; i < obj->NumPoints(); i++)
	{
	float u;
	float dist;
	Point3 p = obj->GetPoint(i);
	dist = LineToPoint(p, l1, l2, u);
//loop through all points finding finding closest perpendicular
	if ((u>0.0f) && (u <1.0f))
		{
		if ((closestMid < 0.0f) || (dist<closestMid))
			{
			closestMid = dist;
			}
		}
//closest first quarter
	if ((u>0.0f) && (u <0.150f))
		{
		if ((closestStart < 0.0f) || (dist<closestStart))
			{
			closestStart = dist;
			}
		}

//closest last quarter
	if ((u>.85f) && (u < 1.0f))
		{
		if ((closestEnd < 0.0f) || (dist<closestEnd))
			{
			closestEnd = dist;
			}
		}


	}
if (closestMid < 0.0f)
	{
	el1 = Length(l2-l1);
	el2 = Length(l2-l1);
	return;
	}
if (closestEnd < 0.0f)
	{
	closestEnd = closestMid;
	}
if (closestStart < 0.0f)
	{
	closestStart = closestMid;
	}
if (closestStart < el1) el1 = closestStart;
if (closestEnd < el2) el2 = closestEnd;
//get mesh

}


void BonesDefMod::BuildMajorAxis(INode *node, Point3 &l1, Point3 &l2, float &el1, Matrix3 *tm)
	{
//get object state
	ObjectState os;
	
	os = node->EvalWorldState(RefFrame);

//get bounding box
	Box3 bb,bbLocalSpace;
//get longest axis
	os.obj->GetDeformBBox(0,bb);
//	os.obj->GetDeformBBox(0,bbLocalSpace,tm);


	float dx,dy,dz,axislength;
	dx = bb.pmax.x - bb.pmin.x;
	dy = bb.pmax.y - bb.pmin.y;
	dz = bb.pmax.z - bb.pmin.z;
	int axis;
	axislength = dx;
	axis = 0;

	Point3 vecX(0.0f,0.0f,0.0f),vecY(0.0f,0.0f,0.0f),vecZ(0.0f,0.0f,0.0f);
	vecX.x = dx;
	vecY.y = dy;
	vecZ.z = dz;
	vecX = VectorTransform(*tm,vecX ) ;
	vecY = VectorTransform(*tm,vecY );
	vecZ = VectorTransform(*tm,vecZ );
	float vecXLength,vecYLength,vecZLength;

	vecXLength = Length(vecX);
	vecYLength = Length(vecY);
	vecZLength = Length(vecZ);

	
	if (dy > axislength)
		{
		axis = 1;
		axislength = dy;				
		}
	if (dz > axislength)
		{
		axis = 2;
		axislength = dz;				
		}

 	if (axis ==0)
		{
		l1.x = bb.pmax.x;
		l2.x = bb.pmin.x;

		l1.y = (bb.pmax.y + bb.pmin.y) *0.5f;
		l2.y = (bb.pmax.y + bb.pmin.y) *0.5f;
		l1.z = (bb.pmax.z + bb.pmin.z) *0.5f;
		l2.z = (bb.pmax.z + bb.pmin.z) *0.5f;

		if (tm)
			{
			if (vecYLength > vecZLength)
				el1 = vecYLength;
			else el1 =  vecZLength;
			}
		}

	else if (axis ==1)
		{
		l1.y = bb.pmax.y;
		l2.y = bb.pmin.y;

		l1.x = (bb.pmax.x + bb.pmin.x) *0.5f;
		l2.x = (bb.pmax.x + bb.pmin.x) *0.5f;
		l1.z = (bb.pmax.z + bb.pmin.z) *0.5f;
		l2.z = (bb.pmax.z + bb.pmin.z) *0.5f;


		if (tm)
			{
			if (vecXLength > vecZLength)
				el1 = vecXLength;
			else el1 = vecZLength;
			}	

		}

	else if (axis ==2)
		{
		l1.z = bb.pmax.z;
		l2.z = bb.pmin.z;

		l1.x = (bb.pmax.x + bb.pmin.x) *0.5f;
		l2.x = (bb.pmax.x + bb.pmin.x) *0.5f;
		l1.y = (bb.pmax.y + bb.pmin.y) *0.5f;
		l2.y = (bb.pmax.y + bb.pmin.y) *0.5f;


		if (tm)
			{
			if (vecXLength > vecYLength)
				el1 =  vecXLength;
			else el1 =  vecYLength;
			}

		}

	}

Matrix3 BonesDefMod::CompMatrix(TimeValue t,INode *inode,ModContext *mc)
	{
	Interval iv;
	Matrix3 tm(1);	
	if (inode) 
		tm = tm * inode->GetObjectTM(t,&iv);
	return tm;
	}


void BonesDefMod::ClearVertexSelections(BoneModData *bmd)
{
for (int i = 0; i <bmd->selected.GetSize(); i++)
	{
	bmd->selected.Set(i,FALSE);
	}
EnableEffect(FALSE);
ip->RedrawViews(ip->GetTime());

//WEIGHTTABLE
PaintAttribList();
}

void BonesDefMod::ClearBoneEndPointSelections()
{

for (int i = 0; i <BoneData.Count(); i++)
	{
	if (BoneData[i].Node != NULL)
		{
		BoneData[i].end1Selected = FALSE;
		BoneData[i].end2Selected = FALSE;

		}
	}

ip->RedrawViews(ip->GetTime());
}
void BonesDefMod::ClearEnvelopeSelections()
{
for (int i = 0; i <BoneData.Count(); i++)
	{
	if (BoneData[i].Node != NULL)
		{
		for (int j = 0; j <BoneData[i].CrossSectionList.Count(); j++)
			{
			BoneData[i].CrossSectionList[j].innerSelected = FALSE;
			BoneData[i].CrossSectionList[j].outerSelected = FALSE;
			}

		}
	}
ModeBoneEnvelopeIndex = -1;
ip->RedrawViews(ip->GetTime());

}




void BonesDefMod::SyncSelections()
{
for (int i = 0; i < BoneData.Count();i++)
	{
	if (BoneData[i].Node != NULL)
		{
		BoneData[i].end1Selected = FALSE;
		BoneData[i].end2Selected = FALSE;

		for (int j=0;j<BoneData[i].CrossSectionList.Count();j++)
			{
			BoneData[i].CrossSectionList[j].innerSelected = FALSE;
			BoneData[i].CrossSectionList[j].outerSelected = FALSE;
			}

		}	
	}


if (BoneData.Count() == 0)
	{
	ModeBoneIndex = -1;
	}

if ((ModeBoneIndex != -1) && (ModeBoneIndex < BoneData.Count()) && (BoneData[ModeBoneIndex].Node))
	{
	if (ModeBoneEnvelopeIndex == 0)
		{
		BoneData[ModeBoneIndex].end1Selected = TRUE;
		}
	else if (ModeBoneEnvelopeIndex == 1)
		{
		BoneData[ModeBoneIndex].end2Selected = TRUE;
		}
	if (ModeBoneEnvelopeIndex != -1)
		{
		if (ModeBoneEnvelopeSubType < 4)
			BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].innerSelected = TRUE;
		else BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].outerSelected = TRUE;
		EnableRadius(TRUE);

		}
	else
		EnableRadius(FALSE);


	}
//update list box
if (ModeBoneIndex != -1)
	{
	int fsel = ConvertSelectedBoneToListID(ModeBoneIndex);

	SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_SETCURSEL ,fsel,0);

	}
//WEIGHTTABLE
PaintAttribList();
}


int BonesDefMod::HitTest(
		TimeValue t, INode* inode, 
		int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{

	pblock_param->GetValue(skin_filter_vertices,t,FilterVertices,FOREVER);
	pblock_param->GetValue(skin_filter_bones,t,FilterBones,FOREVER);
	pblock_param->GetValue(skin_filter_envelopes,t,FilterEnvelopes,FOREVER);

	FilterVertices = !FilterVertices;
	FilterBones = !FilterBones;
	FilterEnvelopes = !FilterEnvelopes;


	ModeEdit = 0;


	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	HitRegion hr;
	int savedLimits, res = 0;
	Matrix3 tm = CompMatrix(t,inode,mc);

	MakeHitRegion(hr,type, crossing,4,p);


	gw->setHitRegion(&hr);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);	
	gw->setTransform(tm);

// Hit test start point
//loop through points checking for selection
//get selected bone	
	int fsel;
	fsel = SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_GETCURSEL ,0,0);

	int ssel = ConvertSelectedListToBoneID(fsel);

	BoneModData *bmd = (BoneModData *) mc->localData;

	if (bmd==NULL) return 0;

	if (bmd->tmCacheToObjectSpace.Count() != BoneData.Count()) return 0;

	if ( (ssel>=0) && (ip && ip->GetSubObjectLevel() == 1) )

		{

		res = 1;
		ObjectState os;

		os = inode->EvalWorldState(t);
//loop through points checking for selection and then marking for points
//bug fix 276830 Jan 29 2001
		BOOL difTopology = FALSE;
		if (os.obj->NumPoints() != bmd->VertexData.Count())
			difTopology = TRUE;

//hittest gizmos if they are editing them
		GizmoClass *gizmo = NULL;
		if ( (pblock_gizmos->Count(skin_gizmos_list) > 0) && (currentSelectedGizmo<(pblock_gizmos->Count(skin_gizmos_list)) ) )
			{
			ReferenceTarget *ref;
			ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
			gizmo = (GizmoClass *)ref;
			}
		if ( (gizmo) && (gizmo->IsEditing()))
			{
			int iret =  gizmo->HitTest(t,inode, type, crossing, flags, p, vpt,mc, Inverse(tm));
			gw->setRndLimits(savedLimits);
			return iret;

			}
		else if (FilterVertices == 0)
			{
			res =0;
			Interval iv;
			Matrix3 atm = inode->GetObjTMAfterWSM(t,&iv);
			Matrix3 ctm = inode->GetObjectTM(t,&iv);


			BOOL isWorldSpace = FALSE;

			if ((atm.IsIdentity()) && (ip->GetShowEndResult ()))
				isWorldSpace = TRUE;


			for (int i=0;i<bmd->VertexData.Count();i++)
				{
				if ((flags&HIT_SELONLY   &&  bmd->selected[i]) ||
					(flags&HIT_UNSELONLY && !bmd->selected[i]) ||
					!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
					{
					if (!bmd->autoInteriorVerts[i])
						{
						Point3 pt;
						gw->clearHitCode();
//bug fix 276830 Jan 29 2001

						if (difTopology)
							pt = bmd->VertexData[i]->LocalPos;
						else pt = os.obj->GetPoint(i) ;

						if (isWorldSpace)
							pt = pt * Inverse(ctm);

						gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
						gw->marker(&pt,POINT_MRKR);
						if (gw->checkHitCode()) {

							vpt->LogHit(inode, mc, gw->getHitDistance(), i, 
								new BoneHitDataClass(i,-1,-1,-1,-1)); 
							res = 1;
							}
						}
					}
				}
//vertices take priority if selectedd select nothing else
			if (res)
				{
				gw->setRndLimits(savedLimits);	
				return res;
				}

			}

		int ct = bmd->VertexData.Count();

		BOOL showEnvelopes;
		pblock_display->GetValue(skin_display_shownoenvelopes,0,showEnvelopes,FOREVER);
		showEnvelopes = !showEnvelopes;

		 

		for (int i =0;i<BoneData.Count();i++)
			{
			if (BoneData[i].Node != NULL)
				{
//add in envelopes start and end
				ObjectState os;
				ShapeObject *pathOb = NULL;


				if (FilterBones == 0)
					{

					Point3 pta,ptb;

					GetEndPointsLocal(bmd, t,pta, ptb, i);



					Point3 midPt = (pta+ptb) * 0.5f;

					if ((flags&HIT_SELONLY   &&  BoneData[i].end1Selected) ||
						(flags&HIT_UNSELONLY && !BoneData[i].end1Selected) ||
						!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 

						{

					 	gw->clearHitCode();
						gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
						Point3 pt;
						Interval v;

						Point3 invA;
						invA = (ptb-pta) *.1f;
						
						pta += invA;

						gw->marker(&pta,POINT_MRKR);
						Point3 lp[3];
						lp[0] = pta;
						lp[1] = midPt;
						gw->polyline(2, lp, NULL, NULL, 0);

						if (gw->checkHitCode()) {

							vpt->LogHit(inode, mc, gw->getHitDistance(), ct,  
								new BoneHitDataClass(-1,i,0,-1,-1)); 
							res = 1;
							}
						}
					if ((flags&HIT_SELONLY   &&  BoneData[i].end2Selected) ||
						(flags&HIT_UNSELONLY && !BoneData[i].end2Selected) ||
						!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 

						{

						gw->clearHitCode();
						gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
						Point3 pt;
						Interval v;

						Point3 invB;
						invB = (pta-ptb) *.1f;
						
						ptb += invB;



						gw->marker(&ptb,POINT_MRKR);

						Point3 lp[3];
						lp[0] = ptb;
						lp[1] = midPt;
						gw->polyline(2, lp, NULL, NULL, 0);

						if (gw->checkHitCode()) {

							vpt->LogHit(inode, mc, gw->getHitDistance(), ct,
								new BoneHitDataClass(-1,i,1,-1,-1)); 
								
							res = 1;
							}
						}
					}

//add in enevelope inner and outer
				if ((FilterEnvelopes == 0) && (i == ModeBoneIndex) && (showEnvelopes))
					{
					if (BoneData[i].flags & BONE_SPLINE_FLAG)
						{
						ObjectState os = BoneData[i].Node->EvalWorldState(t);
						pathOb = (ShapeObject*)os.obj;
						}
					Point3 l1,l2;
					Interval v;


					GetEndPointsLocal(bmd, t,l1, l2, i);

					Interval valid;
					Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(t,&valid);



					Point3 align = (l2-l1);
					Point3 nvec = align;

					for (int j=0;j<BoneData[i].CrossSectionList.Count();j++)
						{

						Point3 p_edge[8];
						Point3 ept(0.0f,0.0f,0.0f);
						Point3 vec;


//196241 
						if ((pathOb) && (pathOb->NumberOfCurves() != 0) &&(BoneData[i].flags & BONE_SPLINE_FLAG))
							{
							ept = pathOb->InterpCurve3D(t, 0,BoneData[i].CrossSectionList[j].u) * ntm  * bmd->InverseBaseTM;
							align = VectorTransform(ntm  * bmd->InverseBaseTM,pathOb->TangentCurve3D(t, 0,BoneData[i].CrossSectionList[j].u));
							float Inner,Outer;
							BoneData[i].CrossSectionList[j].InnerControl->GetValue(currentTime,&Inner,v);
							BoneData[i].CrossSectionList[j].OuterControl->GetValue(currentTime,&Outer,v);
							GetCrossSection(ept, align, Inner,BoneData[i].temptm, p_edge);
							GetCrossSection(ept, align, Outer,BoneData[i].temptm, &p_edge[4]);

							}
						else
							{
							align = (l2-l1);
							ept = l1;
							float Inner,Outer;
							BoneData[i].CrossSectionList[j].InnerControl->GetValue(currentTime,&Inner,v);
							BoneData[i].CrossSectionList[j].OuterControl->GetValue(currentTime,&Outer,v);


							ept = ept + nvec * BoneData[i].CrossSectionList[j].u;
							GetCrossSection(ept, align, Inner,BoneData[i].temptm, p_edge);
							GetCrossSection(ept, align, Outer,BoneData[i].temptm, &p_edge[4]);

							}



						for (int m=0;m<4;m++)
							{
							if ((flags&HIT_SELONLY   &&  BoneData[i].CrossSectionList[j].innerSelected) ||
								(flags&HIT_UNSELONLY && !BoneData[i].CrossSectionList[j].innerSelected) ||
								!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 


								{
		
								gw->clearHitCode();
								gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);


								
								gw->marker(&p_edge[m],POINT_MRKR);
								if (gw->checkHitCode()) {

									vpt->LogHit(inode, mc, gw->getHitDistance(), ct, 
										new BoneHitDataClass(-1,i,-1,j,m)); 
									res = 1;
									}
								}
							}
						for (m=4;m<8;m++)
							{
							if ((flags&HIT_SELONLY   &&  BoneData[i].CrossSectionList[j].outerSelected) ||
								(flags&HIT_UNSELONLY && !BoneData[i].CrossSectionList[j].outerSelected) ||
								!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
								{
		
								gw->clearHitCode();
								gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);


								
								gw->marker(&p_edge[m],POINT_MRKR);
								if (gw->checkHitCode()) {

									vpt->LogHit(inode, mc, gw->getHitDistance(), ct, 
										new BoneHitDataClass(-1,i,-1,j,m)); 
									res = 1;
									}
								}
							}

						}
					}
				}
			}

		}

	gw->setRndLimits(savedLimits);

	return res;

	}




void BonesDefMod::ZoomToBone(BOOL all)
{
Box3 box;
if (ip)
	{
	Point3 pt1, pt2;
	box.Init();

	float l = 0.0f;
	int i = ModeBoneIndex;
	if ( (i != -1) && (i < BoneData.Count()))
		{
		if (BoneData[i].Node != NULL)
			{

			TimeValue t = ip->GetTime();
						Interval valid;
			Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(t,&valid);


			Point3 pta;
			Interval v;
			BoneData[i].EndPoint1Control->GetValue(currentTime,&pta,v);

			box += pta* ntm;
			BoneData[i].EndPoint2Control->GetValue(currentTime,&pta,v);
			box += pta* ntm;
			for (int k = 0; k < BoneData[i].CrossSectionList.Count();k++)
				{
				float outer;
				Interval v;
				BoneData[i].CrossSectionList[k].OuterControl->GetValue(currentTime,&outer,v);

				if (outer > l) l = outer;
				}
			}
		box.EnlargeBy(l+2.0f);  // this is a fudge since I am using large tick boxes

		ip->ZoomToBounds(all,box);
		}
	}
}

void BonesDefMod::ZoomToGizmo(BoneModData *bmd, BOOL all)
{

Box3 box;
if (ip)
	{
	Point3 pt1, pt2;
	box.Init();

	float l = 0.0f;
	int i = currentSelectedGizmo;
	if ( (i != -1) && (i < bmd->gizmoData.Count() ))
		{
		ip->ZoomToBounds(all,currentGizmoBounds);
		}

	}

}

void BonesDefMod::GetWorldBoundBox(	TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)
	{
	BoneModData *bmd = (BoneModData *) mc->localData;

	if (!bmd) return ;

//MIRROR
	if (mirrorData.Enabled())
		{
		box.Init();
		box += mirrorData.GetMirrorBounds();
		box = box * inode->GetObjTMBeforeWSM(t,&FOREVER);

		for (int i = 0; i < BoneData.Count(); i++)
			{
			INode *node = BoneData[i].Node;
			if (node)
				{
				Point3 pa,pb;
				BoneData[i].EndPoint1Control->GetValue(t,&pa,FOREVER);
				BoneData[i].EndPoint2Control->GetValue(t,&pb,FOREVER);

				Matrix3 tm = node->GetObjectTM(t);
				box +=  pa * tm;
				box +=  pb * tm;
				}
			}


		mirrorData.worldBounds = box;
		return;
		}


	Point3 pt1, pt2;
	box.Init();

	if (ModeEdit == 1)
		{
		Interval iv;
		if ( (ModeBoneEndPoint == 0) || (ModeBoneEndPoint == 1))
			{
			p1Temp->GetValue(t,&pt1,FOREVER,CTRL_ABSOLUTE);
		//	box += pt1;
			}
		}	

	float l = 0.0f;
	for (int i =0;i<BoneData.Count();i++)
		{
		if (BoneData[i].Node != NULL)
			{


			Interval valid;
			Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(t,&valid);


			Point3 pta;
			Interval v;
			BoneData[i].EndPoint1Control->GetValue(currentTime,&pta,v);

			box += pta* ntm;
			BoneData[i].EndPoint2Control->GetValue(currentTime,&pta,v);
			box += pta* ntm;
			for (int k = 0; k < BoneData[i].CrossSectionList.Count();k++)
				{
				float outer;
				Interval v;
				BoneData[i].CrossSectionList[k].OuterControl->GetValue(currentTime,&outer,v);
				if (stopMessagePropogation)	
					{
					if (outer > l) l = (outer*4.0f);
					}
				else if (outer > l) l = (outer);
				}
			}
		}
	box.EnlargeBy(l*1.75f);  // this is a fudge since this  a bounds box and the real volume is a sphere

	box = box * inode->GetObjTMBeforeWSM(t,&FOREVER);


	for(int  j = 0 ; j < bmd->gizmoData.Count() ; j++)
		{
		int id = bmd->gizmoData[j]->whichGizmo;
		int gizmoCount = pblock_gizmos->Count(skin_gizmos_list);
		ReferenceTarget *ref;
		if (id < gizmoCount)
			{
			ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,id);
			if (ref)
				{
				GizmoClass *gizmo = (GizmoClass *)ref;
				Box3 b;
				b.Init();
				if (gizmo)
					{
					gizmo->GetWorldBoundBox(t,inode, vpt, b, mc);
					if (j==currentSelectedGizmo)
						currentGizmoBounds = b;
					box += b;
					}
				}
			}
			
		}

	if (pPainterInterface && pPainterInterface->InPaintMode())
		{
		float *radiusList = pPainterInterface->GetStrokeRadius();
		Point3 *worldHitList = pPainterInterface->GetStrokePointWorld();
		int hitCt = pPainterInterface->GetStrokeCount();
		float r = 0.0f;
		for ( i = 0; i < hitCt; i++)
			{
			box += worldHitList[i];
			if ( radiusList[i] > r) r = radiusList[i];

			}
		box.EnlargeBy(r);
		}

	}

void BonesDefMod::LimitInnerRadius(float outer)
{
float innerRadius;
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl->GetValue(currentTime,&innerRadius,FOREVER);
if (innerRadius > outer)
	{
	innerRadius = outer - 5.0f;
	if (innerRadius < 0.0f) innerRadius  = 0.0f;
	BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl->SetValue(currentTime,&innerRadius,TRUE,CTRL_ABSOLUTE);
	}


}
void BonesDefMod::LimitOuterRadius(float inner)
{

float outerRadius;
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl->GetValue(currentTime,&outerRadius,FOREVER);
if (outerRadius < inner)
	{
	outerRadius = inner + 5.0f;
	BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl->SetValue(currentTime,&outerRadius,TRUE,CTRL_ABSOLUTE);
	}

}

void BonesDefMod::TransformStart(TimeValue t)
	{
	if (updateOnMouseUp)
		stopMessagePropogation = TRUE;

	}
void BonesDefMod::TransformFinish(TimeValue t)
	{
	stopMessagePropogation = FALSE;
	if (updateOnMouseUp)
		{
		NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//		ip->RedrawViews(t);
		ip->ForceCompleteRedraw();
		}

	}
void BonesDefMod::TransformCancel(TimeValue t)
	{
	stopMessagePropogation = FALSE;

	}
void BonesDefMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{

//check of points
//check for envelopes

	if ( !ip ) return;


//MIRROR
	if (mirrorData.Enabled()) return;

	ModContextList mcList;		
	INodeTab nodes;

//hittest gizmos if they are editing them
	GizmoClass *gizmo = NULL;
	if ( (pblock_gizmos->Count(skin_gizmos_list) > 0) && (currentSelectedGizmo<(pblock_gizmos->Count(skin_gizmos_list)) ) )
		{
		ReferenceTarget *ref;

		ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
		gizmo = (GizmoClass *)ref;
		}
	if ( (gizmo) && (gizmo->IsEditing()))
		{

		gizmo->Move(t, partm, tmAxis, val, localOrigin);
		return;
		}


	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	BoneModData *bmd = NULL;

	for ( int i = 0; i < objects; i++ ) 
		{
		BoneModData *tbmd = (BoneModData*)mcList[i]->localData;

		int mode = 1;
		if (mode >0 )
			{
			if (ModeBoneEndPoint == 0)
				{
				tbmd->CurrentCachePiece = -1;



				}
			else if (ModeBoneEndPoint == 1)
				{
				tbmd->CurrentCachePiece = -1;
				}


			}
		if (updateOnMouseUp)
			{
			GetCOREInterface()->NodeInvalidateRect(nodes[i]);
			}

 		if (nodes[i]->Selected())
			{

			bmd = tbmd;
			}
		}


		
	if (bmd == NULL)
		bmd = (BoneModData*)mcList[0]->localData;

	int mode = 1;
	if (mode >0 )
		{
		ModeEdit = 1;
		val = VectorTransform(tmAxis*Inverse(bmd->baseNodeOffsetTM),val);


		if (ModeBoneEndPoint == 0)
			{
			val = VectorTransform(bmd->tmCacheToBoneSpace[ModeBoneIndex],val);


			BoneData[ModeBoneIndex].EndPoint1Control->SetValue(currentTime,&val,TRUE,CTRL_RELATIVE);

			int tempID = ModeBoneIndex;
			tempID = ConvertSelectedBoneToListID(tempID)+1;
			Interval iv;
			BoneData[ModeBoneIndex].EndPoint1Control->GetValue(currentTime,&val,iv,CTRL_ABSOLUTE);
			macroRecorder->FunctionCall(_T("skinOps.setStartPoint"), 3, 0, mr_reftarg, this, 
																			 mr_int, tempID,
																			 mr_point3,&val);



			}
		else if (ModeBoneEndPoint == 1)
			{
			val = VectorTransform(bmd->tmCacheToBoneSpace[ModeBoneIndex],val);


			BoneData[ModeBoneIndex].EndPoint2Control->SetValue(currentTime,&val,TRUE,CTRL_RELATIVE);

			int tempID = ModeBoneIndex;
			tempID = ConvertSelectedBoneToListID(tempID)+1;
			Interval iv;
			BoneData[ModeBoneIndex].EndPoint2Control->GetValue(currentTime,&val,iv,CTRL_ABSOLUTE);
			macroRecorder->FunctionCall(_T("skinOps.setEndPoint"), 3, 0, mr_reftarg, this, 
																			 mr_int, tempID,
																			 mr_point3,&val);



			}

		else
			{
			if ((ModeBoneEnvelopeIndex != -1) && (ModeBoneEnvelopeSubType != -1))
				{
				val = VectorTransform(bmd->tmCacheToBoneSpace[ModeBoneIndex],val);
				p1Temp->SetValue(0,val,TRUE,CTRL_RELATIVE);

				Interval v;


				ObjectState os;
				ShapeObject *pathOb = NULL;
				Point3 nvec;
				Point3 vec;
				Point3 lp;
				Point3 l1,l2;
				BoneData[ModeBoneIndex].EndPoint1Control->GetValue(currentTime,&l1,v);
				BoneData[ModeBoneIndex].EndPoint2Control->GetValue(currentTime,&l2,v);


				Point3 p(0.0f,0.0f,0.0f);
				Interval iv = FOREVER;
				p1Temp->GetValue(0,&p,iv);
					 


				if (BoneData[ModeBoneIndex].flags & BONE_SPLINE_FLAG)
					{
					ObjectState os = BoneData[ModeBoneIndex].Node->EvalWorldState(t);
					pathOb = (ShapeObject*)os.obj;
//196241 
					if (pathOb->NumberOfCurves() != 0) 
						lp = pathOb->InterpCurve3D(t, 0,BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u);					
					}
				else
					{
					nvec = l2-l1;
					lp = l1 + nvec * BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u;
					}


				if (ModeBoneEnvelopeSubType<4)
					{
//10-9-00 new method used to compute the distance constrains the manip axis


					Interval v;
					float angle;
					Point3 vecA,vecB;
					vecA = localVec-localStart;
					vecB = p-localStart;
					float dot = DotProd(Normalize(vecA),Normalize(vecB));
					angle = acos(dot);

					vecB = Inverse(bmd->tmCacheToBoneSpace[ModeBoneIndex]).VectorTransform(vecB);

					float inner;
					if (dot == 1.0f)
						inner = Length(vecB) ;
					else inner = Length(vecB) * cos(angle);

					if (inner < 0.0f) inner = 0.0005f;

					BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl->SetValue(currentTime,&inner,TRUE,CTRL_ABSOLUTE);

					macroRecorder->Disable();
					pblock_param->SetValue(skin_cross_radius,0,inner);
					macroRecorder->Enable();


					LimitOuterRadius(inner);


					int tempID = ModeBoneIndex;
					tempID = ConvertSelectedBoneToListID(tempID)+1;
					macroRecorder->FunctionCall(_T("skinOps.setInnerRadius"), 4, 0, mr_reftarg, this, mr_int, tempID,mr_int,ModeBoneEnvelopeIndex+1, mr_float,inner);
					}
				else if (ModeBoneEnvelopeSubType<8)
					{
//10-9-00 new method used to compute the distance constrains the manip axis

					Interval v;
					float angle;
					Point3 vecA,vecB;
					vecA = localVec-localStart;
					vecB = p-localStart;
					float dot = DotProd(Normalize(vecA),Normalize(vecB));
					angle = acos(dot);

					vecB = Inverse(bmd->tmCacheToBoneSpace[ModeBoneIndex]).VectorTransform(vecB);

					float outer;
					if (dot == 1.0f)
						outer = Length(vecB) ;
					else outer = Length(vecB) * cos(angle);

					if (outer < 0.0f) outer = 0.001f;
	
					int tempID = ModeBoneIndex;
					tempID = ConvertSelectedBoneToListID(tempID)+1;
					BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl->SetValue(currentTime,&outer,TRUE,CTRL_ABSOLUTE);
					
					macroRecorder->Disable();
					pblock_param->SetValue(skin_cross_radius,0,outer);
					macroRecorder->Enable();

					LimitInnerRadius(outer);

					macroRecorder->FunctionCall(_T("skinOps.setOuterRadius"), 4, 0, mr_reftarg, this, mr_int, tempID, mr_int,ModeBoneEnvelopeIndex+1, mr_float,outer);


					}

				}

			}

//move the right controller		
		}
		

	if (updateOnMouseUp)
		{
		ip->RedrawViews(t);
		}

	}

void BonesDefMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{



	BoneModData *bmd = (BoneModData *) mc->localData;

	if (!bmd) return; 

	if (bmd->tmCacheToObjectSpace.Count() != BoneData.Count()) return;

	Matrix3 tm = CompMatrix(t,node,mc);
	Point3 pt(0,0,0), p;

//hittest gizmos if they are editing them
	GizmoClass *gizmo = NULL;
	if ( (pblock_gizmos->Count(skin_gizmos_list) > 0) && (currentSelectedGizmo<(pblock_gizmos->Count(skin_gizmos_list)) ) )
		{
		ReferenceTarget *ref;

		ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
		gizmo = (GizmoClass *)ref;
		}
	if ( (gizmo) && (gizmo->IsEditing()))
		{
		gizmo->GetSubObjectCenters(cb,t,node, Inverse(tm));
		return;
		}


	if (ModeBoneEndPoint == 0)
		{
		Interval iv;
		BoneData[ModeBoneIndex].EndPoint1Control->GetValue(currentTime,&bmd->localCenter,iv,CTRL_ABSOLUTE);
		bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];
		}
	else if (ModeBoneEndPoint == 1)
		{
		Interval iv;
		BoneData[ModeBoneIndex].EndPoint2Control->GetValue(currentTime,&bmd->localCenter,iv,CTRL_ABSOLUTE);

		bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];
		}

	else if ((ModeBoneEnvelopeIndex != -1) && (ModeBoneEnvelopeSubType != -1))
		{
		
//10-9-00		
		Interval v;
		Point3 p;
		p1Temp->GetValue(0,p,v,CTRL_ABSOLUTE);
		float angle;
		Point3 vecA,vecB;


		vecA = localVec-localStart;
		vecB = p-localStart;
		float dot = DotProd(Normalize(vecA),Normalize(vecB));
		angle = acos(dot);

		float dist;
		if (dot == 1.0f)
			dist = Length(vecB) ;
		else dist = Length(vecB) * cos(angle);

		if (dist < 0.0f) dist = 0.0f;

		bmd->localCenter = localStart+(Normalize(vecA) * dist);


		bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];


		}
	else if ((ModeBoneIndex>=0) &&  (ModeBoneIndex<BoneData.Count() ))
		{
		if (BoneData[ModeBoneIndex].Node)
			{
			Interval iv;
			Point3 a,b;
			BoneData[ModeBoneIndex].EndPoint1Control->GetValue(currentTime,&a,iv,CTRL_ABSOLUTE);
			bmd->localCenter = a *bmd->tmCacheToObjectSpace[ModeBoneIndex];

			BoneData[ModeBoneIndex].EndPoint2Control->GetValue(currentTime,&b,iv,CTRL_ABSOLUTE);
			bmd->localCenter += b *bmd->tmCacheToObjectSpace[ModeBoneIndex];
			bmd->localCenter *= 0.05f;
			}

		}
		

	pt = bmd->localCenter;

	tm.PreTranslate(pt);
	cb->Center(tm.GetTrans(),0);
	}

void BonesDefMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{
//hittest gizmos if they are editing them
	Matrix3 tm = CompMatrix(t,node,mc);
	
	GizmoClass *gizmo = NULL;
	if ( (pblock_gizmos->Count(skin_gizmos_list) > 0) && (currentSelectedGizmo<(pblock_gizmos->Count(skin_gizmos_list)) ) )
		{
		ReferenceTarget *ref;

		ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
		gizmo = (GizmoClass *)ref;
		}
	if ( (gizmo) && (gizmo->IsEditing()))
		{
		gizmo->GetSubObjectTMs(cb,t,node,Inverse(tm));
		return;
		}

	BoneModData *bmd = (BoneModData *) mc->localData;

	if (!bmd) return; 

	if (bmd->tmCacheToObjectSpace.Count() != BoneData.Count()) return;

	if ((ModeBoneIndex>=0) &&  (ModeBoneIndex<BoneData.Count() ))
		{
		INode *boneNode = BoneData[ModeBoneIndex].Node;
		if (boneNode)
			{
			tm = boneNode->GetNodeTM(t);
			}
		}


	Point3 pt(0.0f,0.0f,0.0f);
	Interval iv;
	Point3 center;/*,xVec,yVec,zVec;
	Interval iv;
	center = pt;
	xVec = pt;
	yVec = pt;
	zVec = pt;

	BoneData[ModeBoneIndex].EndPoint1Control->GetValue(currentTime,&pt,iv,CTRL_ABSOLUTE);
	BoneData[ModeBoneIndex].EndPoint2Control->GetValue(currentTime,&zVec,iv,CTRL_ABSOLUTE);

	zVec = Normalize(pt - zVec);

	Point3 align = Normalize(zVec);
	Point3 p(0.0f,0.0f,0.0f);
	if (align.x == 1.0f)
		{
		p.z = 1.0f;
		}
	else if (align.y == 1.0f)
		{
		p.x = 1.0f;
		}
	else if (align.z == 1.0f)
		{
		p.y = 1.0f;
		}
	else if (align.x == -1.0f)
		{
		p.z = -1.0f;
		}
	else if (align.y == -1.0f)
		{
		p.x = -1.0f;
		}
	else if (align.z == -1.0f)
		{
		p.y = -1.0f;
		}
	else 
		{
		p = Normalize(align^Point3(1.0f,0.0f,0.0f));
		}

	yVec = Normalize(p);
	xVec = Normalize(CrossProd(zVec,yVec));
*/
	BOOL useLocalAxis = TRUE;

	if (ModeBoneEndPoint == 0)
		{
		
		BoneData[ModeBoneIndex].EndPoint1Control->GetValue(currentTime,&center,iv,CTRL_ABSOLUTE);
		}
	else if (ModeBoneEndPoint == 1)
		{
		
		BoneData[ModeBoneIndex].EndPoint2Control->GetValue(currentTime,&center,iv,CTRL_ABSOLUTE);
		}

	else if ((ModeBoneEnvelopeIndex != -1) && (ModeBoneEnvelopeSubType != -1))
		{
		
//10-9-00		
		Interval v;
		Point3 p;
		p1Temp->GetValue(0,p,v,CTRL_ABSOLUTE);
		float angle;
		Point3 vecA,vecB;


		vecA = localVec-localStart;
		vecB = p-localStart;
		float dot = DotProd(Normalize(vecA),Normalize(vecB));
		angle = acos(dot);

		float dist;
		if (dot == 1.0f)
			dist = Length(vecB) ;
		else dist = Length(vecB) * cos(angle);

		if (dist < 0.0f) dist = 0.0f;

		center = localStart+(Normalize(vecA) * dist);



		}
	else if ((ModeBoneIndex>=0) &&  (ModeBoneIndex<BoneData.Count() ))
		{
		if (BoneData[ModeBoneIndex].Node)
			{
			useLocalAxis = FALSE;
			Interval iv;
			Point3 a,b;
			BoneData[ModeBoneIndex].EndPoint1Control->GetValue(currentTime,&a,iv,CTRL_ABSOLUTE);
			center = a;

			BoneData[ModeBoneIndex].EndPoint2Control->GetValue(currentTime,&b,iv,CTRL_ABSOLUTE);
			center += b;
			center *= 0.05f;
			}

		}
		

	pt = center * tm;
	tm.SetRow(3,pt);

/*
	if (useLocalAxis)
		{
		xVec = VectorTransform(tm,xVec);
		yVec = VectorTransform(tm,yVec);
		zVec = VectorTransform(tm,zVec);
		tm.SetRow(0,xVec);
		tm.SetRow(1,yVec);
		tm.SetRow(2,zVec);
		}
*/

	cb->TM(tm,0);
	}

void BonesDefMod::ResetSelection()
	{
	updateP = TRUE;

	if ( (ModeBoneEnvelopeIndex==-1) &&  (ModeBoneEnvelopeSubType==-1) && (BoneData.Count() >0))
		{	
		if (BoneData[ModeBoneIndex].Node == NULL)
			{
			ModeBoneIndex = -1;
			for (int i = 0; i < BoneData.Count(); i++)
				{
				if (BoneData[i].Node != NULL)
					{
					ModeBoneIndex = i;
					i = BoneData.Count();
					}

				}
			}

		ModeBoneEnvelopeIndex=0;
		ModeBoneEnvelopeSubType=5;
//clear selection flags;
		for (int bct = 0; bct < BoneData.Count(); bct++)
			{
			BoneData[bct].end1Selected = FALSE;
			BoneData[bct].end2Selected = FALSE;
			for (int cct = 0; cct < BoneData[bct].CrossSectionList.Count(); cct++)
				{
				BoneData[bct].CrossSectionList[cct].innerSelected = FALSE;
				BoneData[bct].CrossSectionList[cct].outerSelected = FALSE;
				}
			
			}
		if ( (ModeBoneIndex != -1) && (BoneData[ModeBoneIndex].CrossSectionList.Count()) )
			BoneData[ModeBoneIndex].CrossSectionList[0].outerSelected=TRUE;
		}
	UpdatePropInterface();

}

void BonesDefMod::UpdatePropInterface()

{

if ( (ip) && (ModeBoneIndex >= 0) && (ModeBoneIndex < BoneData.Count()) && (BoneData[ModeBoneIndex].Node))
	{
	if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
		{
		iAbsolute->SetCheck(FALSE);

		}
	else
		{
		iAbsolute->SetCheck(TRUE);
		}


	if (BoneData[ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
		{
		iEnvelope->SetCheck(TRUE);
		}
	else
		{
		iEnvelope->SetCheck(FALSE);
		}
	if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_X_FLAG)
		iFalloff->SetCurFlyOff(0,FALSE);
	else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_SINE_FLAG)
		iFalloff->SetCurFlyOff(1,FALSE);
	else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_X3_FLAG)
		iFalloff->SetCurFlyOff(3,FALSE);
	else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_3X_FLAG)
		iFalloff->SetCurFlyOff(2,FALSE);



	if ((ModeBoneIndex >=0) && (ModeBoneIndex <pblock_param->Count(skin_local_squash)))
		{
		float val;
		pblock_param->GetValue(skin_local_squash,ip->GetTime(), val, FOREVER, ModeBoneIndex);
		ISpinnerControl* spin = SetupFloatSpinner(hParam, IDC_SQUASHSPIN, IDC_SQUASH, 0.0f, 10.0f, val, .1f);
		ReleaseISpinner(spin);
			
		}

	EnableButtons();
	EnableWindow(GetDlgItem(hParam,IDC_REMOVE),TRUE);
	}
else 
	{
	DisableButtons();
	EnableWindow(GetDlgItem(hParam,IDC_REMOVE),FALSE);
	}


}


void BonesDefMod::UpdateP(BoneModData* bmd)
{
if (!ip) return;

if ( (ModeBoneIndex < 0) || (ModeBoneIndex >= BoneData.Count()) ) return;

if (BoneData[ModeBoneIndex].Node == NULL) return;
if ( (ModeBoneEnvelopeIndex < 0) || (ModeBoneEnvelopeIndex >= BoneData[ModeBoneIndex].CrossSectionList.Count()) ) return;


Point3 align;
Point3 vec;

Point3 p_edge[8];
Point3 ept(0.0f,0.0f,0.0f);

ObjectState os;
ShapeObject *pathOb = NULL;

Interval valid;
Matrix3 ntm = BoneData[ModeBoneIndex].Node->GetObjTMBeforeWSM(ip->GetTime(),&valid);

if ((BoneData[ModeBoneIndex].flags & BONE_SPLINE_FLAG) && (BoneData[ModeBoneIndex].Node != NULL))
	{
	ObjectState os = BoneData[ModeBoneIndex].Node->EvalWorldState(ip->GetTime());
	pathOb = (ShapeObject*)os.obj;
//196241
	if (pathOb->NumberOfCurves() != 0) 
		{
		ept = pathOb->InterpCurve3D(ip->GetTime(), 0,BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u);
		align = pathOb->TangentCurve3D(ip->GetTime(), 0,BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u);
		ept = ept * bmd->tmCacheToObjectSpace[ModeBoneIndex];
		align = VectorTransform(bmd->tmCacheToObjectSpace[ModeBoneIndex],align);
		}

	}
else
	{
	Point3 l1,l2;
	Interval v;
	GetEndPointsLocal(bmd,ip->GetTime(),l1,l2, ModeBoneIndex);

	align = l2-l1;
	Point3 nvec = align;
	ept = l1;
	ept = ept + nvec * BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u;
	}

float inner, outer;
Interval v;
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl->GetValue(currentTime,&inner,v);
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl->GetValue(currentTime,&outer,v);
GetCrossSection(ept, align, inner,
				BoneData[ModeBoneIndex].temptm,  p_edge);
GetCrossSection(ept, align, outer,
				BoneData[ModeBoneIndex].temptm,  &p_edge[4]);

if (ModeBoneEnvelopeSubType < 4)
	{
	pblock_param->SetValue(skin_cross_radius,0,inner);
	BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].innerSelected = TRUE;

	}
else{
	pblock_param->SetValue(skin_cross_radius,0,outer);
	BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].outerSelected = TRUE;
	}
//Point3 p = p_edge[ModeBoneEnvelopeSubType] * bmd->BaseTM * Inverse(ntm);
Point3 p = p_edge[ModeBoneEnvelopeSubType] * bmd->tmCacheToBoneSpace[ModeBoneIndex];
bmd->localCenter = p_edge[ModeBoneEnvelopeSubType];

//10-9-00 need to record these positions also
localVec = p;
//localStart = ((p_edge[0]+p_edge[1]+p_edge[2]+p_edge[3]) *.25f)* bmd->BaseTM * Inverse(ntm);
localStart = ((p_edge[0]+p_edge[1]+p_edge[2]+p_edge[3]) *.25f)* bmd->tmCacheToBoneSpace[ModeBoneIndex];



p1Temp->SetValue(0,p,TRUE,CTRL_ABSOLUTE);


}


void BonesDefMod::SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
	{
	
//Passs selection test to gizmo if editing
	GizmoClass *gizmo = NULL;
	if ( (pblock_gizmos->Count(skin_gizmos_list) > 0) && (currentSelectedGizmo<(pblock_gizmos->Count(skin_gizmos_list)) ))
		{
		ReferenceTarget *ref;
		ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
		gizmo = (GizmoClass *)ref;
		}

	if ( (gizmo) && (gizmo->IsEditing()))
		{
		gizmo->SelectSubComponent(hitRec, selected, all, invert);
		return;
		}


	ModeEdit = 0;

	Tab<BoneModData*> bmdList;
	HitRecord *head = hitRec;
	while (hitRec) 
		{
		BoneModData *tbmd = (BoneModData*)hitRec->modContext->localData;
		BOOL found = FALSE;
		for (int i = 0; i < bmdList.Count(); i++)
			{
			if (bmdList[i] == tbmd)
				{
				found = TRUE;
				i = bmdList.Count();

				}
			}	
		if (!found) bmdList.Append(1,&tbmd,1);
		hitRec = hitRec->Next();
		}
	hitRec = head;


	for (int i = 0; i < bmdList.Count(); i++)
		{
		if (theHold.Holding() ) theHold.Put(new SelectionRestore(this,bmdList[i]));
		}


	BOOL add = GetKeyState(VK_CONTROL)<0;
	BOOL sub = GetKeyState(VK_MENU)<0;

	if (!add && !sub) 
		{
		for (int j = 0; j < bmdList.Count(); j++)
			{
			for (int i =0;i<bmdList[j]->selected.GetSize();i++)
				bmdList[j]->selected.Set(i,FALSE);
			}

//MIRROR
		mirrorData.ClearBoneSelection();

		}
	int Count = 0;
	BOOL state = selected;

	BoneHitDataClass *bhd;

	int mode = -1;


	while (hitRec) {
		state = hitRec->hitInfo;
		BoneModData *bmd = (BoneModData*)hitRec->modContext->localData;

//MIRROR
		if (mirrorData.Enabled())
			{
			bhd = (BoneHitDataClass *) hitRec->hitData;
			if (bhd->VertexId == -1)
				{
				if (sub)
					mirrorData.SelectBone(bhd->BoneId,FALSE);
				else mirrorData.SelectBone(bhd->BoneId,TRUE);
				}
			}

		if (sub)
			{
			 if (state < bmd->selected.GetSize()) bmd->selected.Set(state,FALSE);
			}
		else 
			{
			
			bhd = (BoneHitDataClass *) hitRec->hitData;
			if (bhd->VertexId == -1)
				{
				mode = 1;
				if (ModeBoneIndex == bhd->BoneId)
					{
					ModeBoneIndex = bhd->BoneId;
					if ( (ModeBoneEnvelopeIndex != bhd->CrossId) ||
						 (ModeBoneEnvelopeSubType != bhd->CrossHandleId) )
						{
						if (bhd->CrossHandleId < 4)
							macroRecorder->FunctionCall(_T("skinOps.SelectCrossSection"), 3, 0, mr_reftarg, this, mr_int, bhd->CrossId+1, mr_int, 0);
						else macroRecorder->FunctionCall(_T("skinOps.SelectCrossSection"), 3, 0, mr_reftarg, this, mr_int, bhd->CrossId+1, mr_int, 1);
						}
					ModeBoneEnvelopeIndex = bhd->CrossId;


					ModeBoneEndPoint = bhd->EndPoint;
					ModeBoneEnvelopeIndex = bhd->CrossId;
					ModeBoneEnvelopeSubType = bhd->CrossHandleId;
		
					}
				else
					{
					ModeBoneIndex = bhd->BoneId;
					ModeBoneEnvelopeIndex = -1;
					if (bhd->EndPoint ==0)
						BoneData[ModeBoneIndex].end1Selected = TRUE;
					else if (bhd->EndPoint ==1)
						BoneData[ModeBoneIndex].end2Selected = TRUE;
					ModeBoneEndPoint = bhd->EndPoint;	
					ModeBoneEnvelopeIndex = -1;
					ModeBoneEnvelopeSubType = -1;
					int tempID = ModeBoneIndex;
					tempID = ConvertSelectedBoneToListID(tempID)+1;
					macroRecorder->FunctionCall(_T("skinOps.SelectBone"), 2, 0, mr_reftarg, this, mr_int, tempID);

					}

// PW: macro-recorder

				UpdatePropInterface();

				}
			else bmd->selected.Set(state,TRUE);
			}

		
		hitRec = hitRec->Next();
		Count++;
		}	
	for (i = 0; i < bmdList.Count(); i++)
		{
		BoneModData *bmd = bmdList[i];
		
		if (bmd->selected.NumberSet() > 0)
			{
			EnableEffect(TRUE);
// PW: macro-recorder
			macroRecorder->FunctionCall(_T("skinOps.SelectVertices"), 2, 0, mr_reftarg, this, mr_bitarray, &bmd->selected);
			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			}
			else 
			{
			EnableEffect(FALSE);
			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			}

	
		if (mode >0 )
			{
			Point3 p;
//clear selection flags;
			for (int bct = 0; bct < BoneData.Count(); bct++)
				{
				BoneData[bct].end1Selected = FALSE;
				BoneData[bct].end2Selected = FALSE;
				for (int cct = 0; cct < BoneData[bct].CrossSectionList.Count(); cct++)
					{
					BoneData[bct].CrossSectionList[cct].innerSelected = FALSE;
					BoneData[bct].CrossSectionList[cct].outerSelected = FALSE;
					}

				}
			if (ModeBoneEndPoint == 0)
				{
				BoneData[ModeBoneIndex].end1Selected = TRUE;
				Point3 tp;
				if (ip)
					GetEndPoints(bmd, ip->GetTime(), bmd->localCenter, tp, ModeBoneIndex);
// PW: macro-recorder
				macroRecorder->FunctionCall(_T("skinOps.SelectStartPoint"), 1, 0, mr_reftarg, this);

				}
			else if (ModeBoneEndPoint == 1)
				{
				BoneData[ModeBoneIndex].end2Selected = TRUE;
				Point3 tp;
				if (ip)
					GetEndPoints(bmd, ip->GetTime(), tp, bmd->localCenter, ModeBoneIndex);
// PW: macro-recorder
				macroRecorder->FunctionCall(_T("skinOps.SelectEndPoint"), 1, 0, mr_reftarg, this);


				}
			else if (ModeBoneEnvelopeIndex>=0)
				{



				Point3 align;
				Point3 vec;

				Point3 p_edge[8];
				Point3 ept(0.0f,0.0f,0.0f);

				ObjectState os;
				ShapeObject *pathOb = NULL;

				Interval valid;
				Matrix3 ntm = BoneData[ModeBoneIndex].Node->GetObjTMBeforeWSM(ip->GetTime(),&valid);

				if ((BoneData[ModeBoneIndex].flags & BONE_SPLINE_FLAG) && (BoneData[ModeBoneIndex].Node != NULL))
					{
					ObjectState os = BoneData[ModeBoneIndex].Node->EvalWorldState(ip->GetTime());
					pathOb = (ShapeObject*)os.obj;
//196241 
					if (pathOb->NumberOfCurves() != 0) 
						{
						ept = pathOb->InterpCurve3D(ip->GetTime(), 0,BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u);
						align = pathOb->TangentCurve3D(ip->GetTime(), 0,BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u);
						ept = ept * bmd->tmCacheToObjectSpace[ModeBoneIndex];
						align = VectorTransform(bmd->tmCacheToObjectSpace[ModeBoneIndex],align);
						}

					}
				else
					{
					Point3 l1,l2;
					Interval v;
					GetEndPointsLocal(bmd,ip->GetTime(),l1,l2, ModeBoneIndex);

					align = l2-l1;
					Point3 nvec = align;
					ept = l1;
					ept = ept + nvec * BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u;
					}

				float inner, outer;
				Interval v;
				BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl->GetValue(currentTime,&inner,v);
				BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl->GetValue(currentTime,&outer,v);

				GetCrossSection(ept, align, inner,
							BoneData[ModeBoneIndex].temptm,  p_edge);
				GetCrossSection(ept, align, outer,
							BoneData[ModeBoneIndex].temptm,  &p_edge[4]);

				if (ModeBoneEnvelopeSubType < 4)
					{
					pblock_param->SetValue(skin_cross_radius,0,inner);
					BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].innerSelected = TRUE;

					}
				else{
					pblock_param->SetValue(skin_cross_radius,0,outer);
					BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].outerSelected = TRUE;
					}
//				p = p_edge[ModeBoneEnvelopeSubType] * bmd->BaseTM * Inverse(ntm);
				p = p_edge[ModeBoneEnvelopeSubType] * bmd->tmCacheToBoneSpace[ModeBoneIndex];
				bmd->localCenter = p_edge[ModeBoneEnvelopeSubType];
//10-9-00
				localVec = p;
//				localStart = ((p_edge[0]+p_edge[1]+p_edge[2]+p_edge[3]) *.25f)* bmd->BaseTM * Inverse(ntm);
				localStart = ((p_edge[0]+p_edge[1]+p_edge[2]+p_edge[3]) *.25f)* bmd->tmCacheToBoneSpace[ModeBoneIndex];;

				}
		
		
			if  (ModeBoneEnvelopeIndex == -1) EnableRadius(FALSE);
			else EnableRadius(TRUE);


			p1Temp->SetValue(0,p,TRUE,CTRL_ABSOLUTE);

		
//select in list box also 
			int rsel = ConvertSelectedBoneToListID(ModeBoneIndex);

			SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_SETCURSEL ,rsel,0);


			}

		UpdateEffectSpinner(bmd);

		int nset = bmd->selected.NumberSet();
		int total = bmd->selected.GetSize();

		}
	ip->ClearCurNamedSelSet();
	
//WEIGHTTABLE
	PaintAttribList();

//MIRROR
	if (mirrorData.Enabled())
		mirrorData.EmitBoneSelectionScript();
	}



void BonesDefMod::UpdateEffectSpinner(BoneModData*bmd)
{
	if (bmd->selected.NumberSet() > 0)
		{
//get selected bone	
		int rsel = 0;
		rsel = SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_GETCURSEL ,0,0);

		int tsel = ConvertSelectedListToBoneID(rsel);


		int sct = 0;
		float v = -1.0f;
		BOOL first = TRUE;
		BOOL idnt = FALSE;

		
		for (int i = 0; i < bmd->selected.GetSize(); i++)
			{
			if (bmd->selected[i])
				{
				BOOL match = FALSE;
				for (int ct =0 ; ct <bmd->VertexData[i]->d.Count(); ct++)
					{

					if (bmd->VertexData[i]->d[ct].Bones == ModeBoneIndex)
						{
						match = TRUE;
						float tempV = RetrieveNormalizedWeight(bmd,i,ct);
						if (first)
							{
							v = tempV;
							first = FALSE;
							}
						else if (v != tempV)
							{	
							idnt = TRUE;

							}
						}
					}
				 if (!match) 
					{
					if (first)
						{
						v = 0.0f;
						first = FALSE;
						}
					else if (v != 0.0f)
						{
						idnt = TRUE;
						}
					}


				}

			}

		if (idnt)
			{
			ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hParam,IDC_EFFECTSPIN));
			spin2->SetIndeterminate(TRUE);
			}
		else 
			{
			ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hParam,IDC_EFFECTSPIN));
			spin2->SetIndeterminate(FALSE);
			pblock_param->SetValue(skin_effect,0,v);
			bmd->effect = v;
			}


		BOOL unNorm = FALSE;
		BOOL indeter = FALSE;
		BOOL firstVert = TRUE;
		for (i = 0; i < bmd->selected.GetSize(); i++)
			{
			if (bmd->selected[i])
				{
				if (firstVert)
					{
					unNorm = bmd->VertexData[i]->IsUnNormalized();
					firstVert = FALSE;
					}
				else
					{
					BOOL iret = bmd->VertexData[i]->IsUnNormalized();
					if (unNorm != bmd->VertexData[i]->IsUnNormalized())
						indeter = TRUE;
					}
				}
			}

		EnableWindow(GetDlgItem(hParam,IDC_NORMALIZE_CHECK),TRUE);

		if (!indeter)
			{
			if (unNorm)
				CheckDlgButton(hParam,IDC_NORMALIZE_CHECK,BST_UNCHECKED);
			else CheckDlgButton(hParam,IDC_NORMALIZE_CHECK,BST_CHECKED);
			}
		else CheckDlgButton(hParam,IDC_NORMALIZE_CHECK,BST_INDETERMINATE);


		BOOL rigid = FALSE;
		indeter = FALSE;
		firstVert = TRUE;
		for (i = 0; i < bmd->selected.GetSize(); i++)
			{
			if (bmd->selected[i])
				{
				if (firstVert)
					{
					rigid = bmd->VertexData[i]->IsRigid();
					firstVert = FALSE;
					}
				else
					{
					BOOL iret = bmd->VertexData[i]->IsRigid();
					if (rigid != bmd->VertexData[i]->IsRigid())
						indeter = TRUE;
					}
				}
			}

		EnableWindow(GetDlgItem(hParam,IDC_RIGID_CHECK),TRUE);

		if (!indeter)
			{
			if (rigid)
				CheckDlgButton(hParam,IDC_RIGID_CHECK,BST_CHECKED);
			else CheckDlgButton(hParam,IDC_RIGID_CHECK,BST_UNCHECKED);
			}
		else CheckDlgButton(hParam,IDC_RIGID_CHECK,BST_INDETERMINATE);


		BOOL rigidHandle = FALSE;
		indeter = FALSE;
		firstVert = TRUE;
		for (i = 0; i < bmd->selected.GetSize(); i++)
			{
			if (bmd->selected[i])
				{
				if (firstVert)
					{
					rigidHandle = bmd->VertexData[i]->IsRigidHandle();
					firstVert = FALSE;
					}
				else
					{
					BOOL iret = bmd->VertexData[i]->IsRigidHandle();
					if (rigidHandle != bmd->VertexData[i]->IsRigidHandle())
						indeter = TRUE;
					}
				}
			}

		EnableWindow(GetDlgItem(hParam,IDC_RIGIDHANDLES_CHECK),TRUE);

		if (!indeter)
			{
			if (rigidHandle)
				CheckDlgButton(hParam,IDC_RIGIDHANDLES_CHECK,BST_CHECKED);
			else CheckDlgButton(hParam,IDC_RIGIDHANDLES_CHECK,BST_UNCHECKED);
			}
		else CheckDlgButton(hParam,IDC_RIGIDHANDLES_CHECK,BST_INDETERMINATE);



		}
	else
		{
		EnableWindow(GetDlgItem(hParam,IDC_RIGID_CHECK),FALSE);
		EnableWindow(GetDlgItem(hParam,IDC_RIGIDHANDLES_CHECK),FALSE);
		EnableWindow(GetDlgItem(hParam,IDC_NORMALIZE_CHECK),FALSE);
		}

}

void BonesDefMod::ClearSelection(int selLevel)
	{

//MIRROR
	if (mirrorData.Enabled())
		{
		mirrorData.ClearBoneSelection();
		}

	if (selLevel == 1)
		{

//Passs selection test to gizmo if editing
		GizmoClass *gizmo = NULL;
		if ( (pblock_gizmos->Count(skin_gizmos_list) > 0) && (currentSelectedGizmo<(pblock_gizmos->Count(skin_gizmos_list)) ))
			{
			ReferenceTarget *ref;
			ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
			gizmo = (GizmoClass *)ref;
			}

		if ( (gizmo) && (gizmo->IsEditing()))
			{
			gizmo->ClearSelection(selLevel);
			return;
			}

		ModContextList mcList;		
		INodeTab nodes;

		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();


		for ( int i = 0; i < objects; i++ ) 
			{
			BoneModData *bmd = (BoneModData*)mcList[i]->localData;

			if (theHold.Holding() ) theHold.Put(new SelectionRestore(this,bmd));

			for (int j =0;j<bmd->selected.GetSize();j++)
				bmd->selected.Set(j,FALSE);

			UpdateEffectSpinner(bmd);
			INode *n = nodes[i]->GetActualINode();
			GetCOREInterface()->NodeInvalidateRect(n);

//			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			}

		if  (ip) ip->RedrawViews(ip->GetTime());

		ip->ClearCurNamedSelSet();

		}
//WEIGHTTABLE
	PaintAttribList();
	}

void BonesDefMod::SelectAll(int selLevel)
	{

	if (selLevel == 1)
		{
//Passs selection test to gizmo if editing
		GizmoClass *gizmo = NULL;
		if ( (pblock_gizmos->Count(skin_gizmos_list) > 0) && (currentSelectedGizmo<(pblock_gizmos->Count(skin_gizmos_list)) ))
			{
			ReferenceTarget *ref;
			ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
			gizmo = (GizmoClass *)ref;
			}

		if ( (gizmo) && (gizmo->IsEditing()))
			{
			gizmo->SelectAll(selLevel);
			return;
			}


		ModContextList mcList;		
		INodeTab nodes;

		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for ( int i = 0; i < objects; i++ ) 
			{
			BoneModData *bmd = (BoneModData*)mcList[i]->localData;

			if (theHold.Holding() ) theHold.Put(new SelectionRestore(this,bmd));

			for (int i =0;i<bmd->selected.GetSize();i++)
				bmd->selected.Set(i,TRUE);

			UpdateEffectSpinner(bmd);
	
			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			}
		ip->ClearCurNamedSelSet();

		}
//WEIGHTTABLE
	PaintAttribList();
	}


int BonesDefMod::HoldWeights()
	{
	ModContextList mcList;		
	INodeTab nodes;


	theHold.SuperBegin();
	if (ip)
		{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();
		theHold.Begin();
		for ( int i = 0; i < objects; i++ ) 
			{
			BoneModData *bmd = (BoneModData*)mcList[i]->localData;

			theHold.Put(new WeightRestore(this,bmd));


			}
		theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
		
		}
	return 1;
	}

int BonesDefMod::AcceptWeights(BOOL accept)
	{

	if (accept) theHold.SuperAccept(GetString(IDS_PW_WEIGHTCHANGE));
	else theHold.SuperCancel();
	return 1;
	}


void BonesDefMod::InvertSelection(int selLevel)
	{
	if (selLevel == 1)
		{

//Passs selection test to gizmo if editing
		GizmoClass *gizmo = NULL;
		if ( (pblock_gizmos->Count(skin_gizmos_list) > 0) && (currentSelectedGizmo<(pblock_gizmos->Count(skin_gizmos_list)) ))
			{
			ReferenceTarget *ref;
			ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
			gizmo = (GizmoClass *)ref;
			}

		if ( (gizmo) && (gizmo->IsEditing()))
			{
			gizmo->InvertSelection(selLevel);
			return;
			}


		ModContextList mcList;		
		INodeTab nodes;

		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for ( int i = 0; i < objects; i++ ) 
			{
			BoneModData *bmd = (BoneModData*)mcList[i]->localData;

			for (int i =0;i<bmd->selected.GetSize();i++)
				{
				BOOL v = !bmd->selected[i];
				bmd->selected.Set(i,v);
				}

			UpdateEffectSpinner(bmd);

			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			}
		ip->ClearCurNamedSelSet();

		}

//WEIGHTTABLE
	PaintAttribList();

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

	}


void BonesDefMod::EnableRadius(BOOL enable)

{

if (enable)
	SpinnerOn(hParam,IDC_ERADIUSSPIN,IDC_ERADIUS);
else SpinnerOff(hParam,IDC_ERADIUSSPIN,IDC_ERADIUS);



}

void BonesDefMod::EnableEffect(BOOL enable)
{

if (enable)
	SpinnerOn(hParam,IDC_EFFECTSPIN,IDC_EFFECT);
else SpinnerOff(hParam,IDC_EFFECTSPIN,IDC_EFFECT);

}


void BonesDefMod::EnableButtons()
{

			if (iCrossSectionButton!=NULL)
				iCrossSectionButton->Enable();
			if (iPaintButton!=NULL)
				iPaintButton->Enable();
			EnableWindow(GetDlgItem(hParam,IDC_CREATE_REMOVE_SECTION),TRUE);


			BOOL turnOnFilters = FALSE;
			if (ip && ip->GetSubObjectLevel() == 1)
				turnOnFilters = TRUE;
			
			EnableWindow(GetDlgItem(hParam,IDC_FILTER_VERTICES_CHECK),turnOnFilters);
			EnableWindow(GetDlgItem(hParam,IDC_FILTER_BONES_CHECK),turnOnFilters);
			EnableWindow(GetDlgItem(hParam,IDC_FILTER_ENVELOPES_CHECK),turnOnFilters);



			EnableWindow(GetDlgItem(hParam,IDC_DRAWALL_ENVELOPES_CHECK),TRUE);
			EnableWindow(GetDlgItem(hParam,IDC_DRAW_VERTICES_CHECK),TRUE);

			EnableWindow(GetDlgItem(hParamGizmos,IDC_ADD),TRUE);
			EnableWindow(GetDlgItem(hParamGizmos,IDC_REMOVE),TRUE);

			ICustButton *iBut = GetICustButton(GetDlgItem(hParam, IDC_EXCLUDE));
			if (iBut)
				{
				iBut->Enable(TRUE);
				ReleaseICustButton(iBut);
				}
			iBut = GetICustButton(GetDlgItem(hParam, IDC_INCLUDE));
			if (iBut)
				{
				iBut->Enable(TRUE);
				ReleaseICustButton(iBut);
				}
			iBut = GetICustButton(GetDlgItem(hParam, IDC_SELECT_EXCLUDED));
			if (iBut)
				{
				iBut->Enable(TRUE);
				ReleaseICustButton(iBut);
				}


// bug fix 206160 9/8/99	watje
			if (iAbsolute!=NULL)
				iAbsolute->Enable();
			if (iEnvelope!=NULL)
				iEnvelope->Enable();
			if (iFalloff!=NULL)
				iFalloff->Enable();
			if (iCopy!=NULL)
				iCopy->Enable();


			if ((iPaste!=NULL) && (CopyBuffer.CList.Count() != 0))
				iPaste->Enable();


		if ((ModeBoneEnvelopeIndex>=0) && (ModeBoneEnvelopeIndex<BoneData[ModeBoneIndex].CrossSectionList.Count()))
			SpinnerOn(hParam,IDC_ERADIUSSPIN,IDC_ERADIUS);
		else SpinnerOff(hParam,IDC_ERADIUSSPIN,IDC_ERADIUS);
		SpinnerOn(hParam,IDC_SQUASHSPIN,IDC_SQUASH);


		
		ModContextList mcList;		
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		BOOL vertexSelected = FALSE;
		for ( int i = 0; i < objects; i++ ) 
			{
			BoneModData *bmd = (BoneModData*)mcList[i]->localData;
			if (bmd->selected.NumberSet() > 0)
				vertexSelected = TRUE;
			}


		EnableWindow(GetDlgItem(hParam,IDC_NORMALIZE_CHECK),vertexSelected);
		EnableWindow(GetDlgItem(hParam,IDC_RIGID_CHECK),vertexSelected);
		EnableWindow(GetDlgItem(hParam,IDC_RIGIDHANDLES_CHECK),vertexSelected);



}
void BonesDefMod::DisableButtons()
{

			if (iCrossSectionButton!=NULL)
				iCrossSectionButton->Disable();
			if (iPaintButton!=NULL)
				iPaintButton->Disable();

			EnableWindow(GetDlgItem(hParam,IDC_CREATE_REMOVE_SECTION),FALSE);


			EnableWindow(GetDlgItem(hParam,IDC_FILTER_VERTICES_CHECK),FALSE);
			EnableWindow(GetDlgItem(hParam,IDC_FILTER_BONES_CHECK),FALSE);
			EnableWindow(GetDlgItem(hParam,IDC_FILTER_ENVELOPES_CHECK),FALSE);
			EnableWindow(GetDlgItem(hParam,IDC_DRAWALL_ENVELOPES_CHECK),FALSE);
			EnableWindow(GetDlgItem(hParam,IDC_DRAW_VERTICES_CHECK),FALSE);

			EnableWindow(GetDlgItem(hParamGizmos,IDC_ADD),FALSE);
			EnableWindow(GetDlgItem(hParamGizmos,IDC_REMOVE),FALSE);

			ICustButton *iBut = GetICustButton(GetDlgItem(hParam, IDC_EXCLUDE));
			if (iBut) 
				{
				iBut->Enable(FALSE);
				ReleaseICustButton(iBut);
				}
			iBut = GetICustButton(GetDlgItem(hParam, IDC_INCLUDE));
			if (iBut)
				{
				iBut->Enable(FALSE);
				ReleaseICustButton(iBut);
				}
			iBut = GetICustButton(GetDlgItem(hParam, IDC_SELECT_EXCLUDED));
			if (iBut)
				{
				iBut->Enable(FALSE);
				ReleaseICustButton(iBut);
				}

// bug fix 206160 9/8/99	watje
			if (iAbsolute!=NULL)
				iAbsolute->Disable();
			if (iEnvelope!=NULL)
				iEnvelope->Disable();
			if (iFalloff!=NULL)
				iFalloff->Disable();
			if (iCopy!=NULL)
				iCopy->Disable();
			if (iPaste!=NULL)
				iPaste->Disable();
			   

   
		SpinnerOff(hParam,IDC_ERADIUSSPIN,IDC_ERADIUS);
		SpinnerOff(hParam,IDC_SQUASHSPIN,IDC_SQUASH);
   
		EnableWindow(GetDlgItem(hParam,IDC_NORMALIZE_CHECK),FALSE);
		EnableWindow(GetDlgItem(hParam,IDC_RIGID_CHECK),FALSE);
		EnableWindow(GetDlgItem(hParam,IDC_RIGIDHANDLES_CHECK),FALSE);


}


void BonesDefMod::SetVCMode()
	{
	ModContextList list;
	INodeTab NodeTab;
	if (ip && (vcState == FALSE))
		{
		ip->GetModContexts(list,NodeTab);
		for( int i = 0 ; i < NodeTab.Count() ; i++)
			{
			NodeTab[i]->SetShadeCVerts(TRUE);
			NodeTab[i]->SetCVertMode(TRUE);	
			NodeTab[i]->SetVertexColorType(nvct_color);
			}
		}
	vcState = TRUE;

	}

void BonesDefMod::RestoreVCMode()

	{
	MyEnumProc dep;              
	EnumDependents(&dep);
//this puts back the original state of the node vc mods and shade state
	if (vcState)
		{
		for (int  i = 0; i < dep.Nodes.Count(); i++)
			{
			BOOL hit = FALSE;
	
			INode *node = dep.Nodes[i];
			for (int  j = 0; j < vcSaveDataList.Count(); j++)
				{
//need to check to make sure thenode still exist since the user can delete a node without leaving the subobj mode
				if (vcSaveDataList[j].node == node)
					{
					hit = TRUE;
					node->SetShadeCVerts(vcSaveDataList[j].shade);
					node->SetCVertMode(vcSaveDataList[j].vcmode);	
					node->SetVertexColorType(vcSaveDataList[j].type);	
					j = vcSaveDataList.Count();
					}
				}
			}
			
		}
	vcState = FALSE;
	}


void BonesDefMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
	switch (level) {
		case 0:
			{

			RestoreVCMode();

			DisableButtons();
			iEditEnvelopes->SetCheck(FALSE);

//			if ((ip) && (ip->GetCommandMode() == PaintMode)) {
//				ip->SetStdCommandMode(CID_OBJMOVE);
//				return;
//				}


			if ((ip) && (ip->GetCommandMode() == CrossSectionMode)) {
				ip->SetStdCommandMode(CID_OBJMOVE);
				return;
				}
			if (pblock_gizmos->Count(skin_gizmos_list) > 0)
				{
				ReferenceTarget *ref;
				if (currentSelectedGizmo != -1)
					{
					ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
					GizmoClass *gizmo = (GizmoClass *)ref;
					if (gizmo) 
						{	
						gizmo->EndEditing();
						gizmo->EnableEditing(FALSE);
						}
					}
				}
			EnableRadius(FALSE);
			EnableEffect(FALSE);
//MIRROR
			pblock_mirror->SetValue(skin_mirrorenabled,0,FALSE);
			mirrorData.EnableMirrorButton(FALSE);
			break;
			}
		case 1: // Points
			{
			//MIRROR
			mirrorData.EnableMirrorButton(TRUE);

			SetupNamedSelDropDown();
			MyEnumProc dep;              
			EnumDependents(&dep);

//this gets the state of the node vc mods and shade state and stores them
			vcSaveDataList.ZeroCount();

			for (int  i = 0; i < dep.Nodes.Count(); i++)
				{
				vcSaveData tempVCData;
				tempVCData.node = dep.Nodes[i];
				tempVCData.shade = dep.Nodes[i]->GetShadeCVerts();
				tempVCData.vcmode = dep.Nodes[i]->GetCVertMode();	
				tempVCData.type = dep.Nodes[i]->GetVertexColorType();	

				vcSaveDataList.Append(1,&tempVCData);
				}



			int bct = 0;
			for (i =0; i < BoneData.Count(); i ++)
				{
				if (!(BoneData[i].flags &  BONE_DEAD_FLAG)) bct++;
				}
			if (bct > 0)
				{
				EnableButtons();
				}

			if ( (ModeBoneEnvelopeIndex != -1) && (ModeBoneIndex >= 0) && (ModeBoneIndex<BoneData.Count()))
				EnableRadius(TRUE);
			else EnableRadius(FALSE);

			iEditEnvelopes->SetCheck(TRUE);

			if (pblock_gizmos->Count(skin_gizmos_list) > 0)
				{
				ReferenceTarget *ref;
				if (currentSelectedGizmo != -1)
					{
					ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
					GizmoClass *gizmo = (GizmoClass *)ref;
					if (gizmo) 
						{	
						gizmo->EnableEditing(TRUE);
						}
					}
				}

			ModContextList mcList;		
			INodeTab nodes;

			ip->GetModContexts(mcList,nodes);
			int objects = mcList.Count();


			for ( i = 0; i < objects; i++ ) 
				{
				BoneModData *bmd = (BoneModData*)mcList[i]->localData;


				UpdateEffectSpinner(bmd);
				}

			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;		
			}
		}

	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}



Interval BonesDefMod::LocalValidity(TimeValue t)
	{
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t);
	}

//aszabo|feb.06.02 - When LocalValidity is called by ModifyObject,
// it returns NEVER and thus the object channels are marked non valid
// As a result, the mod stack enters and infinite evaluation of the modifier
// ModifyObject now calls GetValidity and CORE calls LocalValidity to
// allow for building a cache on the input of this modifier when it's 
// being edited 
Interval BonesDefMod::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	for (int i =0;i<BoneData.Count();i++)
		{
		if (BoneData[i].Node != NULL) 
			{
			BoneData[i].Node->GetObjTMBeforeWSM(t,&valid);
			if (BoneData[i].flags & BONE_SPLINE_FLAG)
				{
				ObjectState osp = BoneData[i].Node->EvalWorldState (t);
				valid &= osp.obj->ObjectValidity (t);
				}

			}
		}
	for(int j = 0 ; j < pblock_gizmos->Count(skin_gizmos_list) ; j++)
		{

		ReferenceTarget *ref;
		ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,j);
		GizmoClass *gizmo = (GizmoClass *)ref;
		if (gizmo)
			valid &= gizmo->LocalValidity( t);
			
		}


	return valid;
}




Point3 BonesDefMod::VertexAnimation(TimeValue t, BoneModData * bmd, int vertex, int bone, Point3 p)
{
Point3 ps(0.0f,0.0f,0.0f),pr(0.0f,0.0f,0.0f),pdef(0.0f,0.0f,0.0f),pt(0.0f,0.0f,0.0f);
Point3 MovedU,MovedTan;

int bid = bmd->VertexData[vertex]->d[bone].Bones;
if (BoneData[bid].Node == NULL) return p;

ShapeObject *pathOb = NULL;
Interface *ip = GetCOREInterface();
ObjectState os = BoneData[bid].Node->EvalWorldState(t);
pathOb = (ShapeObject*)os.obj;

int cid = bmd->VertexData[vertex]->d[bone].SubCurveIds;
int sid = bmd->VertexData[vertex]->d[bone].SubSegIds;
float u = bmd->VertexData[vertex]->d[bone].u;


Matrix3 ntm = BoneData[bid].Node->GetObjectTM(t);	
Matrix3 tm    = Inverse(bmd->BaseTM * Inverse(ntm));

//196241 
if (pathOb->NumberOfCurves() == 0) return p;
if (sid >= pathOb->NumberOfVertices(t,cid)) return p;

MovedU = pathOb->InterpPiece3D(t, cid,sid ,u );
MovedU = MovedU * tm;
MovedTan = pathOb->TangentPiece3D(t, cid, sid, u);
MovedTan = VectorTransform(tm,MovedTan);

Point3 OPoint;
OPoint = bmd->VertexData[vertex]->d[bone].OPoints * tm;
Point3 OTan = VectorTransform(tm,bmd->VertexData[vertex]->d[bone].Tangents);

float s = 1.0f;  //scale 
float angle = 0.0f;
float influ = RetrieveNormalizedWeight(bmd,vertex,bone);

//float influ = VertexData[vertex].d[bone].Influences;

OTan = Normalize(OTan);
MovedTan = Normalize(MovedTan);
if ( OTan != MovedTan)
 angle = (float) acos(DotProd(OTan,MovedTan)) * influ;



Point3 perp = CrossProd(OTan,MovedTan);
Matrix3 RotateMe(1);

RotateMe = RotAngleAxisMatrix(Normalize(perp), angle);


ps = p-OPoint;
pr = ps * RotateMe + OPoint;
pt = (MovedU - OPoint) * influ;
pdef = pr + pt;
return pdef;

}

class BonesDefDeformer: public Deformer {
	public:
		BonesDefMod *Cluster;
		BoneModData *bmd;
		TimeValue t;
		BonesDefDeformer(BonesDefMod *C, BoneModData *bm, TimeValue tv){Cluster = C;bmd = bm; t= tv;}
		Point3 Map(int i, Point3 p);
//this is a fall back deformer in case the com engine fails
		Point3 Map2(int i, Point3 p)
		{
			
			if (bmd->VertexData.Count()>0)
				{

				if (bmd->VertexData[i]->d.Count() > 0 )
					{

					Point3 tp(0.0f,0.0f,0.0f);
					float influence = 0.0f;
					if (bmd->VertexData[i]->d.Count()==1)
						{
						Point3 vec;
						float influ = Cluster->RetrieveNormalizedWeight(bmd,i,0);

						vec = (p*Cluster->BoneData[bmd->VertexData[i]->d[0].Bones].temptm);
						vec = vec - p;
						vec = vec * influ;
						p += vec;
						int bid;
						bid = bmd->VertexData[i]->d[0].Bones;
						if ((Cluster->BoneData[bid].flags & BONE_SPLINE_FLAG) && (influ != 0.0f))
							{
							p = Cluster->VertexAnimation(t,bmd,i,0,p);
							}

						return p;
						}
					for (int j=0;j<bmd->VertexData[i]->d.Count();j++)
						{
						float influ = Cluster->RetrieveNormalizedWeight(bmd,i,j);

						if (influ != 0.0f)
							{
							tp  += (p*Cluster->BoneData[bmd->VertexData[i]->d[j].Bones].temptm)*influ;
							influence += influ;
							}
						}
//do vertex snimation if it is a splineanimation

					for (j=0;j<bmd->VertexData[i]->d.Count();j++)
						{
						int bid;
						bid = bmd->VertexData[i]->d[j].Bones;

						if (Cluster->BoneData[bid].flags & BONE_SPLINE_FLAG) 
							{
							float influ = Cluster->RetrieveNormalizedWeight(bmd,i,j);

							if (influ != 0.0f)
								
								{
								tp = Cluster->VertexAnimation(t,bmd,i,j,tp);
								}
							}
						}

					if (influence > 0.00001)
						return tp;
	
					}
				else return p;
				}
			return p;
			}

	};



class StaticBonesDefDeformer: public Deformer {
	public:
		BonesDefMod *Cluster;
		BoneModData *bmd;
		TimeValue t;
		StaticBonesDefDeformer(BonesDefMod *C, BoneModData *bm, TimeValue tv){Cluster = C;bmd = bm; t= tv;}
		Point3 Map(int i, Point3 p) {
			
			if (bmd->VertexData.Count()>0)
				{
				if ( i <bmd->VertexData.Count())
					p = bmd->VertexData[i]->LocalPos;
				}
			return p;
			}
	};

void BonesDefMod::RecomputeAllBones(BoneModData *bmd, TimeValue t, ObjectState *os)

{

//watje 9-7-99  198721 
		bmd->reevaluate = FALSE;
		int nv = os->obj->NumPoints();
		if ( (bmd->VertexData.Count() != nv) || (reset))
			{
			reset = FALSE;
//temp point3 weight table
			Tab<VertexListClass*> newWeights;
			newWeights.SetCount(nv);
			for (int i=0; i<nv; i++) 
				{
				newWeights[i] = NULL;
				}
			for (i = 0; i < nv; i++)
				{
				Point3 p = os->obj->GetPoint(i);
				for (int j = 0; j < bmd->VertexData.Count(); j++)
					{

					if (bmd->VertexData[j]->LocalPos ==p)
						{
						if (bmd->VertexData[j]->IsModified())
							{
							VertexListClass *vc;
							vc = new VertexListClass;
							*vc = *bmd->VertexData[j];
							newWeights[i] = vc;
							}
						}
					}
				}

			for (i = 0; i < bmd->VertexData.Count(); i++)
				{
				if (bmd->VertexData[i] != NULL)
					delete (bmd->VertexData[i]);
				bmd->VertexData[i] = NULL;
				}

			bmd->VertexData.ZeroCount();
			bmd->VertexData.SetCount(nv);

			for (i=0; i<nv; i++) {
				if (newWeights[i])
					{	
					bmd->VertexData[i] = newWeights[i];
					}
				else
					{
					VertexListClass *vc;
					vc = new VertexListClass;
					bmd->VertexData[i] = vc;
					bmd->VertexData[i]->Modified (FALSE);
 					bmd->VertexData[i]->d.ZeroCount();;
					}

				}
			}
		else
			{
			for (int i = 0; i < bmd->VertexData.Count(); i++)
				{
				if (!bmd->VertexData[i]->IsModified ())
 					bmd->VertexData[i]->d.ZeroCount();
				else
					{
					for (int j = 0; j < bmd->VertexData[i]->d.Count(); j++)
						{
						int boneIndex = bmd->VertexData[i]->d[j].Bones;
						if ( (boneIndex < 0) || (boneIndex >= BoneData.Count()) || (BoneData[boneIndex].Node == NULL))
							{
							bmd->VertexData[i]->d.Delete(j,1);
							j--;
							}
						}
//					if (bmd->VertexData[i]->d.Count() == 0)
//						bmd->VertexData[i]->Modified (FALSE);


					}
				}
			}



		int bonecount = 0;
		int crosscount = 0;
		for (int i =0;i<BoneData.Count();i++)
			{
			if (BoneData[i].Node != NULL)
				{
				bonecount++;
				for (int ccount = 0; ccount < BoneData[i].CrossSectionList.Count();ccount++)
					crosscount++;
				}
			}

//build bounding box list for hit testing;
		Tab<Box3> BBoxList;
		BBoxList.ZeroCount();
		for ( i =0;i<BoneData.Count();i++) 
			{
			Point3 l1,l2;

	
			Box3 b;
			float Outer,l = 0.0f;
			if (BoneData[i].Node != NULL)
				{
				if ((BoneData[i].flags & BONE_SPLINE_FLAG) 	&& (BoneData[i].Node != NULL))

					{
					ObjectState tos;
					tos = BoneData[i].Node->EvalWorldState(RefFrame);
//get bounding box
					tos.obj->GetDeformBBox(RefFrame,b);
					
					Interval valid;
//watje 10-7-99 212059
					Matrix3 ntm = Inverse(BoneData[i].tm);
					b = b * ntm * bmd->InverseBaseTM;

	
					}
				else
					{
	
					Interval valid;
//watje 3-11-99
					GetEndPoints(bmd,RefFrame,l1, l2, i);

					b.Init();
					b.MakeCube(l1,1.0f);
					b += l2;

					}
				for (int ccount = 0; ccount < BoneData[i].CrossSectionList.Count();ccount++)
					{
					float inner;

					GetCrossSectionRanges(inner, Outer, i, ccount);
					if (inner>Outer) Outer  = inner;
					if (Outer > l ) l = Outer;
					}
				b.EnlargeBy(l);


				}

			BBoxList.Append(1,&b,1);

			}	

//Get largest radius

//New Fallof method
 		for (i=0; i<nv; i++) {
//get total distance

			float TotalDistance = 0.0f;
			Point3 p,BoneCenter;		
			if (!bmd->VertexData[i]->IsModified())
				{

				p = os->obj->GetPoint(i);

				int FullStrength =0;
				for (int j =0;j<BoneData.Count();j++) 
					{	
					BOOL excluded = FALSE;
					if (j < bmd->exclusionList.Count())
						{
						if (bmd->exclusionList[j])
							{
							if (bmd->isExcluded(j,i))
								{
								excluded = TRUE;
								}
							}
						}
					if ((BoneData[j].Node != NULL) && (!excluded))

						{
						if (BBoxList[j].Contains(p)) 
							{
							int Bone;
							float Influence = 1.0f;
							Bone = j;
							Point3 l1,l2;



							GetEndPoints(bmd,t,l1, l2, j);
						

							float LineU,SplineU = 0.0f;
							Point3 op,otan;
							int cid,sid;
							if ((BoneData[j].flags & BONE_SPLINE_FLAG) && (BoneData[j].Node != NULL))
								{

								Interval valid;
//watje 10-7-99 212059
								Matrix3 ntm = BoneData[j].tm;

//watje 10-7-99 212059
								ntm =bmd->BaseTM * ntm;


								Influence = SplineToPoint(p,
														  &BoneData[j].referenceSpline,
														  LineU,op,otan,cid,sid,ntm);

								SplineU = LineU;
								}
							else
								{

								Influence = LineToPoint(p,l1,l2,LineU);
								}

//find cross section that bound this point
							int StartCross = 0, EndCross = 0;
							float tu = ModifyU(t,LineU,  j, sid);

							for (int ccount = 0; ccount < BoneData[j].CrossSectionList.Count();ccount++)
								{
								if (BoneData[j].CrossSectionList[ccount].u>=tu)
									{
									EndCross =ccount;
									ccount = BoneData[j].CrossSectionList.Count();
									}
								}
							StartCross = EndCross -1;
	
							if (StartCross == -1)
								{
								StartCross = 0;
								EndCross++;
								}
							Influence = ComputeInfluence(t,Influence,LineU, j,StartCross, EndCross,sid);

							if (Influence != 0.0f)
								{
								VertexInfluenceListClass td;
								td.Bones = Bone;
								td.Influences = Influence;

								td.u = SplineU;
								td.Tangents = otan;
								td.OPoints = op;
								td.SubCurveIds = cid;
								td.SubSegIds = sid;
								if (!(BoneData[Bone].flags & BONE_LOCK_FLAG))
									{
									int found = -1;
									for (int vdcount = 0; vdcount < bmd->VertexData[i]->d.Count();vdcount++)
										{
										if (bmd->VertexData[i]->d[vdcount].Bones == Bone)
											{
											bmd->VertexData[i]->d[vdcount] = td;
											found = bmd->VertexData[i]->d.Count();
											}
										}
									if (found == -1)
										bmd->VertexData[i]->d.Append(1,&td,1);

									}

								}
							else
								{
								for (int vdcount = 0; vdcount < bmd->VertexData[i]->d.Count();vdcount++)
									{
									if (bmd->VertexData[i]->d[vdcount].Bones == j)
										{
										bmd->VertexData[i]->d.Delete(vdcount,1);
										vdcount = bmd->VertexData[i]->d.Count();
										}
									}
								}


							}

						else
							{
							if (!(BoneData[j].flags & BONE_LOCK_FLAG))
								{
								for (int vdcount = 0; vdcount < bmd->VertexData[i]->d.Count();vdcount++)
									{
									if (bmd->VertexData[i]->d[vdcount].Bones == j)
										{
										bmd->VertexData[i]->d.Delete(vdcount,1);
										vdcount = bmd->VertexData[i]->d.Count();
										}
									}
								}

							}


						}
					else
						{
						for (int vdcount = 0; vdcount < bmd->VertexData[i]->d.Count();vdcount++)
							{
							if (bmd->VertexData[i]->d[vdcount].Bones == j)
								{
								bmd->VertexData[i]->d.Delete(vdcount,1);
								vdcount = bmd->VertexData[i]->d.Count();
								}
							}

						}
					}
				}

			}
/*
for (i = 0; i < bmd->VertexData.Count(); i++)
	{
	DebugPrint("vert %d ",i);
	for (int j = 0; j < bmd->VertexData[i]->d.Count(); j++)	
		{
		DebugPrint(" Bone %d influence %f u %f seg %d SubCurveIds %d",
							bmd->VertexData[i]->d[j].Bones,
							bmd->VertexData[i]->d[j].Influences,
							bmd->VertexData[i]->d[j].u,
							bmd->VertexData[i]->d[j].SubCurveIds,
							bmd->VertexData[i]->d[j].SubSegIds
							);
		}
	DebugPrint("\n");
	}
*/
}



void BonesDefMod::RecomputeBone(BoneModData *bmd, int BoneIndex, TimeValue t, ObjectState *os)

{

		if (BoneData[BoneIndex].Node == NULL) return;

		BuildCache(bmd, BoneIndex,  t, os);
//watje 9-7-99  198721 
		bmd->reevaluate = FALSE;
		int nv = os->obj->NumPoints();

			

//Get largest radius

//New Fallof method
 		for (int i=0; i<nv; i++) {
//get total distance
			float TotalDistance = 0.0f;
			Point3 p,BoneCenter;		
			if (!bmd->VertexData[i]->IsModified())
				{

				p = os->obj->GetPoint(i);

				int FullStrength =0;
				int j = BoneIndex;
					{	
					BOOL excluded = FALSE;
					if (j < bmd->exclusionList.Count())
						{
						if (bmd->exclusionList[j])
							{
							if (bmd->isExcluded(j,i))
								{
								excluded = TRUE;

								}
							}
						}
					

					if ((j < BoneData.Count()) && (BoneData[j].Node != NULL) && (!excluded))
						{
						int Bone;
						float Influence = 1.0f;
						Bone = j;
						Point3 l1,l2;

						GetEndPoints(bmd,t,l1, l2, j);

						float LineU,SplineU = 0.0f;
						Point3 op,otan;
						int cid,sid;
						if (1) 
							{
							Influence = bmd->DistCache[i].dist;
							LineU = bmd->DistCache[i].u;
							SplineU = LineU;
							cid = bmd->DistCache[i].SubCurveIds;
							sid = bmd->DistCache[i].SubSegIds;
							otan = bmd->DistCache[i].Tangents;
							op = bmd->DistCache[i].OPoints;
//find cross section that bound this point
							int StartCross = 0, EndCross = 0;
							float tu = ModifyU(t,LineU,  j, sid);

							for (int ccount = 0; ccount < BoneData[j].CrossSectionList.Count();ccount++)
								{
								if (BoneData[j].CrossSectionList[ccount].u>=tu)
									{
									EndCross =ccount;
									ccount = BoneData[j].CrossSectionList.Count();
									}
								}
							StartCross = EndCross -1;
	
							if (StartCross == -1)
								{
								StartCross = 0;
								EndCross++;
								}

							Influence = ComputeInfluence(t,Influence,LineU, j,StartCross, EndCross, sid);


							if (Influence != 0.0f)
								{
								VertexInfluenceListClass td;
								td.Bones = Bone;
								td.Influences = Influence;
								td.u = SplineU;
								td.Tangents = otan;
								td.OPoints = op;
								td.SubCurveIds = cid;
								td.SubSegIds = sid;

								BOOL found = FALSE;
								if (!(BoneData[Bone].flags & BONE_LOCK_FLAG))
									{

									for (int bic = 0; bic < bmd->VertexData[i]->d.Count(); bic++)
										{
										if (bmd->VertexData[i]->d[bic].Bones == Bone)
											{
											bmd->VertexData[i]->d[bic] = td;
											found = TRUE;
											bic = bmd->VertexData[i]->d.Count();
											}
										}
		
									if (!found)
										bmd->VertexData[i]->d.Append(1,&td,1);
									}

								}
							else
								{
								if (!(BoneData[Bone].flags & BONE_LOCK_FLAG))
									{
									BOOL found = FALSE;
									for (int bic = 0; bic < bmd->VertexData[i]->d.Count(); bic++)
										{
										if (bmd->VertexData[i]->d[bic].Bones == Bone)
											{
											bmd->VertexData[i]->d.Delete(bic,1);
											found = TRUE;
											bic = bmd->VertexData[i]->d.Count();

											}
										}
									}

								}
							}
						else
							{
							for (int bic = 0; bic < bmd->VertexData[i]->d.Count(); bic++)
								{
								if (bmd->VertexData[i]->d[bic].Bones == j)
									{
									bmd->VertexData[i]->d.Delete(bic,1);
									bic = bmd->VertexData[i]->d.Count();
									}
								}
	
							}

						}

					}


				}

			}


}





void BonesDefMod::UnlockBone(BoneModData *bmd,TimeValue t, ObjectState *os)
	{
//loop through verts and remove and associations to thos bone
	for (int i=0;i<bmd->VertexData.Count();i++)
		{
		for (int bic = 0; bic < bmd->VertexData[i]->d.Count(); bic++)
			{
			if (bmd->VertexData[i]->d[bic].Bones == ModeBoneIndex)
				{
				bmd->VertexData[i]->d.Delete(bic,1);
				bic = bmd->VertexData[i]->d.Count();
				}
			}

		}
//loop through find all verts in radius and set them to unmodified

	if (BoneData[ModeBoneIndex].Node == NULL) return;

	cacheValid = FALSE;
	bmd->CurrentCachePiece = -1;


	BuildCache(bmd, ModeBoneIndex,  t, os);
//watje 9-7-99  198721 
	bmd->reevaluate=TRUE;
	int nv =  os->obj->NumPoints();


//Get largest radius

//New Fallof method
	for (i=0; i<nv; i++) {
//get total distance
		float TotalDistance = 0.0f;
		Point3 p,BoneCenter;		
		p = os->obj->GetPoint(i);

		int FullStrength =0;
		int j = ModeBoneIndex;

		BOOL excluded = FALSE;
		if (j < bmd->exclusionList.Count())
			{
		if (bmd->exclusionList[j])
			{
			if (bmd->isExcluded(j,i))
				{
				excluded = TRUE;

				}
			}
		}
					

		if ((j < BoneData.Count()) && (BoneData[j].Node != NULL) && (!excluded))
			{
			int Bone;
			float Influence = 1.0f;
			Bone = j;
			Point3 l1,l2;

			GetEndPoints(bmd,t,l1, l2, j);

			float LineU,SplineU = 0.0f;
			Point3 op,otan;
			int cid,sid;
			Influence = bmd->DistCache[i].dist;
			LineU = bmd->DistCache[i].u;
			SplineU = LineU;
			cid = bmd->DistCache[i].SubCurveIds;
			sid = bmd->DistCache[i].SubSegIds;
			otan = bmd->DistCache[i].Tangents;
			op = bmd->DistCache[i].OPoints;
//find cross section that bound this point
			int StartCross = 0, EndCross = 0;
			float tu = ModifyU(t,LineU,  j, sid);

			for (int ccount = 0; ccount < BoneData[j].CrossSectionList.Count();ccount++)
				{
				if (BoneData[j].CrossSectionList[ccount].u>=tu)
					{
					EndCross =ccount;
					ccount = BoneData[j].CrossSectionList.Count();
					}
				}
			StartCross = EndCross -1;
	
			if (StartCross == -1)
				{
				StartCross = 0;
				EndCross++;
				}

			Influence = ComputeInfluence(t,Influence,LineU, j,StartCross, EndCross, sid);
			if (Influence > 0.0f)
				{
				bmd->VertexData[i]->Modified(FALSE);

				}
			}
		}


	}


void BonesDefMod::UnlockAllBones(BoneModData *bmd,TimeValue t, ObjectState *os)
	{
//loop through verts and remove and associations to thos bone
	int temp = ModeBoneIndex;
	for (int boneid = 0; boneid < BoneData.Count(); boneid++)
		{
		ModeBoneIndex = boneid;
		if (BoneData[boneid].Node)
			{
			for (int i=0;i<bmd->VertexData.Count();i++)
				{
				for (int bic = 0; bic < bmd->VertexData[i]->d.Count(); bic++)
					{
					if (bmd->VertexData[i]->d[bic].Bones == ModeBoneIndex)
						{
						bmd->VertexData[i]->d.Delete(bic,1);
						bic = bmd->VertexData[i]->d.Count();
						}
					}

				}
//loop through find all verts in radius and set them to unmodified

			if (BoneData[ModeBoneIndex].Node == NULL) return;

			cacheValid = FALSE;
			bmd->CurrentCachePiece = -1;


			BuildCache(bmd, ModeBoneIndex,  t, os);
//watje 9-7-99  198721 
			bmd->reevaluate=TRUE;
			int nv =  os->obj->NumPoints();


//Get largest radius

//New Fallof method
			for (i=0; i<nv; i++) {
//get total distance
				float TotalDistance = 0.0f;
				Point3 p,BoneCenter;		
				p = os->obj->GetPoint(i);

				int FullStrength =0;
				int j = ModeBoneIndex;

				BOOL excluded = FALSE;
				if (j < bmd->exclusionList.Count())
					{
					if (bmd->exclusionList[j])
						{
						if (bmd->isExcluded(j,i))
							{
							excluded = TRUE;

							}
						}
					}
					

				if ((j < BoneData.Count()) && (BoneData[j].Node != NULL) && (!excluded))
					{
					int Bone;
					float Influence = 1.0f;
					Bone = j;
					Point3 l1,l2;

					GetEndPoints(bmd,t,l1, l2, j);

					float LineU,SplineU = 0.0f;
					Point3 op,otan;
					int cid,sid;
					Influence = bmd->DistCache[i].dist;
					LineU = bmd->DistCache[i].u;
					SplineU = LineU;
					cid = bmd->DistCache[i].SubCurveIds;
					sid = bmd->DistCache[i].SubSegIds;
					otan = bmd->DistCache[i].Tangents;
					op = bmd->DistCache[i].OPoints;
//find cross section that bound this point
					int StartCross = 0, EndCross = 0;
					float tu = ModifyU(t,LineU,  j, sid);

					for (int ccount = 0; ccount < BoneData[j].CrossSectionList.Count();ccount++)
						{
						if (BoneData[j].CrossSectionList[ccount].u>=tu)
							{
							EndCross =ccount;
							ccount = BoneData[j].CrossSectionList.Count();
							}
						}
					StartCross = EndCross -1;
	
					if (StartCross == -1)
						{
						StartCross = 0;
						EndCross++;
						}

					Influence = ComputeInfluence(t,Influence,LineU, j,StartCross, EndCross, sid);
					if (Influence > 0.0f)
						{
						bmd->VertexData[i]->Modified(FALSE);

						}
					}
				}
			}
		}
	ModeBoneIndex = temp;

	}



void BonesDefMod::UpdateTMCacheTable(BoneModData *bmd, TimeValue t, Interval& valid,BOOL forceCompleteUpdate)
{
	if (bmd == NULL) return;



	if (bmd->tempTableL1.Count() != BoneData.Count())
		{
		bmd->tempTableL1.SetCount(BoneData.Count());
		forceCompleteUpdate = TRUE;
		}
	if (bmd->tempTableL2.Count() != BoneData.Count())
		{
		bmd->tempTableL2.SetCount(BoneData.Count());
		forceCompleteUpdate = TRUE;
		}
	if (bmd->tempTableL1ObjectSpace.Count() != BoneData.Count())
		{
		bmd->tempTableL1ObjectSpace.SetCount(BoneData.Count());
		forceCompleteUpdate = TRUE;
		}
	if (bmd->tempTableL2ObjectSpace.Count() != BoneData.Count())
		{
		bmd->tempTableL2ObjectSpace.SetCount(BoneData.Count());
		forceCompleteUpdate = TRUE;
		}

	if (bmd->tmCacheToBoneSpace.Count() != BoneData.Count())
		{
		bmd->tmCacheToBoneSpace.SetCount(BoneData.Count());
		forceCompleteUpdate = TRUE;
		}
	if (bmd->tmCacheToObjectSpace.Count() != BoneData.Count())
		{
		bmd->tmCacheToObjectSpace.SetCount(BoneData.Count());
		forceCompleteUpdate = TRUE;
		}
	if ((editing) || (bmd->pSE==NULL) || (bmd->reevaluate)  || forceCompleteUpdate)
		{
		for (int j =0;j<BoneData.Count();j++)
			{
			if (BoneData[j].Node != NULL)
				{
				Point3 l1, l2;

				Interval v;
				BoneData[j].EndPoint1Control->GetValue(currentTime,&l1,v);
				BoneData[j].EndPoint2Control->GetValue(currentTime,&l2,v);
		
	//new opt

				Matrix3 inverseBoneDataTm = Inverse(BoneData[j].tm)* bmd->InverseBaseTM;
				bmd->tempTableL1[j] = l1* inverseBoneDataTm ;
				bmd->tempTableL2[j] = l2* inverseBoneDataTm;

//optimize this we really only need to update the cache when a bone is added or when a bone is moved or time has changed
				if ((bmd->lastTMCacheTime != t) || forceCompleteUpdate || (bmd->reevaluate) )
					{
					
		//if BID then 
					Matrix3 ntm;
					ntm = BoneData[j].Node->GetObjTMBeforeWSM(t,&valid);

					Matrix3 ntmByInverseBaseTM;
					ntmByInverseBaseTM;// = ntm * bmd->InverseBaseTM;
					Matrix3 tmCacheToBoneSpace;
					bmd->baseNodeOffsetTM = bmd->BaseTM;
					if (bindNode)
						{
						xRefTM = bindNode->GetObjectTM(t);

						BoneData[j].temptm = bmd->BaseTM * BoneData[j].tm * ntm * 
							initialXRefTM * Inverse(xRefTM) * bmd->InverseBaseTM;
						ntmByInverseBaseTM = ntm * initialXRefTM * Inverse(xRefTM) * bmd->InverseBaseTM;
						tmCacheToBoneSpace = bmd->BaseTM * initialXRefTM * Inverse(xRefTM) * Inverse(ntm);
						}

					else if ( (backTransform) && ( bmd->meshNode))
						{

						Matrix3 backTM = bmd->meshNode->GetNodeTM(t);

						

						BoneData[j].temptm = bmd->BaseTM * BoneData[j].tm * ntm * 
							bmd->BaseNodeTM * Inverse(backTM) * bmd->InverseBaseTM;

						ntmByInverseBaseTM = ntm * Inverse(backTM) * bmd->BaseNodeTM  * bmd->InverseBaseTM;  //fix for envelopes not in the right pos
						tmCacheToBoneSpace = bmd->BaseTM   * Inverse(bmd->BaseNodeTM * Inverse(backTM))  * Inverse(ntm);
						Matrix3 tempTMCacheToBoneSpace = bmd->BaseTM  *  Inverse(ntm);
		//				bmd->baseNodeOffsetTM = bmd->BaseNodeTM * Inverse(backTM);
						bmd->baseNodeOffsetTM = backTM;



						}

					else
						{
						ntmByInverseBaseTM = ntm * bmd->InverseBaseTM;
						BoneData[j].temptm = bmd->BaseTM * BoneData[j].tm * ntmByInverseBaseTM; 
						tmCacheToBoneSpace = bmd->BaseTM * Inverse(ntm);
						}



		//cache for for matrices
					bmd->tmCacheToBoneSpace[j] = tmCacheToBoneSpace;
		//new opt	
					bmd->tmCacheToObjectSpace[j] = ntmByInverseBaseTM;
					}

				bmd->tempTableL1ObjectSpace[j] = l1 * bmd->tmCacheToObjectSpace[j];
				bmd->tempTableL2ObjectSpace[j] = l2 * bmd->tmCacheToObjectSpace[j];

				}
			}
		bmd->lastTMCacheTime = t;
		}

	if ((backTransform) && ( bmd->meshNode))
		{


		Matrix3 baseNodeObjectTM(1);

		baseNodeObjectTM = bmd->meshNode->GetObjectTM(t);



		bmd->gizmoPutBackDoubleOffset = baseNodeObjectTM * Inverse(bmd->BaseTM);
		bmd->gizmoRemoveDoubleOffset =  bmd->BaseTM * Inverse(baseNodeObjectTM);
		}
	else if (bindNode)
		{
		Matrix3 baseNodeObjectTM(1);

		baseNodeObjectTM = bindNode->GetObjectTM(t);

		bmd->gizmoPutBackDoubleOffset = baseNodeObjectTM * Inverse(bmd->BaseTM);
		bmd->gizmoRemoveDoubleOffset =  bmd->BaseTM * Inverse(baseNodeObjectTM);

		}

	else
		{
		bmd->gizmoPutBackDoubleOffset.IdentityMatrix();
		bmd->gizmoRemoveDoubleOffset.IdentityMatrix();
		}
	


}




class XRefEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	  BOOL nukeME;
	};

int XRefEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);            
			nukeME = TRUE;
			}
     return 0;              
	}	

BOOL RecurseXRefTree(INode *n, INode *target)
{
for (int i = 0; i < n->NumberOfChildren(); i++)
	{

	INode *child = n->GetChildNode(i);

	if (child == target) 
		{
		return TRUE;
		}
	else RecurseXRefTree(child,target);
	
	}
return FALSE;
}

void BonesDefMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {
	if (!mc->localData) return;
//6-18-99
	BoneModData *bmd = (BoneModData *) mc->localData;			
	switch (message) {
		case REFMSG_OBJECT_CACHE_DUMPED:
			{
			if (partID & PART_TOPO)
//6-18-99
				if (!bmd->inputObjectIsNURBS)
//watje 9-7-99  198721 
					bmd->reevaluate=TRUE;
			}
		}

}


void BonesDefMod::UpdateEndPointDelta()
{
int deform;
Interval iv;
pblock_advance->GetValue(skin_advance_always_deform,0,deform,iv);
endPointDelta.SetCount(BoneData.Count());
if (!deform)
	{
	ModContextList mcList;		//ns
	INodeTab nodes;				//ns
	ip->GetModContexts(mcList,nodes); //ns

	for (int j =0;j<BoneData.Count();j++)
		{
		if (BoneData[j].Node != NULL)
			{
			Class_ID bid(BONE_CLASS_ID,0);
			Matrix3 ntm =BoneData[j].Node->GetObjectTM(RefFrame);	
		
			BoneData[j].InitObjectTM = ntm; //ns
			BoneData[j].tm = Inverse(ntm);
//5.1.03
			
			BoneData[j].InitStretchTM = BoneData[j].Node->GetStretchTM(RefFrame); //ns
			if (hasStretchTM)
				BoneData[j].InitNodeTM = BoneData[j].Node->GetNodeTM(RefFrame); //ns
			else BoneData[j].InitNodeTM = BoneData[j].InitStretchTM * BoneData[j].Node->GetNodeTM(RefFrame); //ns
//			BoneData[j].InitNodeTM = BoneData[j].Node->GetStretchTM(RefFrame) * BoneData[j].Node->GetNodeTM(RefFrame); //ns

//copy initial reference spline into our spline
			ObjectState sos = BoneData[j].Node->EvalWorldState(RefFrame);
			if (sos.obj->ClassID() == bid)  
				{
//now need to look at child and move
//loop through children looking for a matching name
				int childCount;
				childCount = BoneData[j].Node->NumberOfChildren();
				INode *childNode = NULL;
				for (int ci = 0; ci < childCount; ci++)
					{
					childNode = BoneData[j].Node->GetChildNode(ci);
					TSTR childName;
					childName = childNode->GetName();
					if (childName == BoneData[j].name)
						{
						Point3 l2(0.0f,0.0f,0.0f);
						Matrix3 ChildTM = childNode->GetObjectTM(RefFrame);
						l2 = l2 * ChildTM;
						l2 = l2 * BoneData[j].tm; 
						Point3 d;
						BoneData[j].EndPoint2Control->GetValue(currentTime,&d,iv,CTRL_ABSOLUTE);
						ci = childCount;
						endPointDelta[j] = d-l2;

						}
							
					}
				}



			}
		}

	}	

}


void BonesDefMod::ShadeVerts(Mesh *msh, BoneModData *bmd)
{

		if (ip)
			{
		

			
			int nv = msh->getNumVerts();
			msh->setNumVCFaces(msh->getNumFaces());
			msh->setNumVertCol(nv);
			int nf = msh->getNumFaces();
			Point3 selSoft = GetUIColor(COLOR_SUBSELECTION_SOFT);
			Point3 selMedium = GetUIColor(COLOR_SUBSELECTION_MEDIUM);
			Point3 selHard = GetUIColor(COLOR_SUBSELECTION_HARD);
			Point3 selSubColor = GetUIColor(COLOR_SUBSELECTION);

			for (int i=0;i<bmd->VertexData.Count();i++)
				{
				msh->vertCol[i] = selSoft * 0.65f ;
				for (int j=0;j<bmd->VertexData[i]->d.Count();j++)
					{
					int rigidBoneID;
					if (rigidVerts)
						rigidBoneID = bmd->VertexData[i]->GetMostAffectedBone();
//put in mirror check here
					int mirroredBone = -1;
					if (pPainterInterface)
						{
						if (pPainterInterface->GetMirrorEnable() && pPainterInterface->InPaintMode())
							{
							mirroredBone = mirrorIndex;
							}
						}
					if ( ( (bmd->VertexData[i]->d[j].Bones == ModeBoneIndex) || (bmd->VertexData[i]->d[j].Bones == mirroredBone))
						 && (bmd->VertexData[i]->d[j].Influences != 0.0f)  )
					
						{
						if (!((rigidVerts) && (ModeBoneIndex != rigidBoneID)))
							{
							Point3 pt;
							float infl;

							infl = RetrieveNormalizedWeight(bmd,i,j);
							j = bmd->VertexData[i]->d.Count();
							Color selColor(1.0f,1.0f,1.0f);
							if (infl > 0.0f)
								{
								if ( (infl<0.33f) && (infl > 0.0f))
									{
									selColor = selSoft + ( (selMedium-selSoft) * (infl/0.33f));
									}
								else if (infl <.66f)
									{
									selColor = selMedium + ( (selHard-selMedium) * ((infl-0.1f)/0.66f));
									}
								else if (infl < 0.99f)
									{
									selColor = selHard + ( (selSubColor-selHard) * ((infl-0.66f)/0.33f));
									}

								else 
									{
									selColor = selSubColor;
									}
								}

							msh->vertCol[i] = selColor;

							}
						}
					}
				}
		
				

			for (i=0; i < nf; i++)
				{
				msh->vcFace[i].t[0] = msh->faces[i].v[0];
				msh->vcFace[i].t[1] = msh->faces[i].v[1];
				msh->vcFace[i].t[2] = msh->faces[i].v[2];
				}

			}
		
}

#ifndef NO_PATCHES

void BonesDefMod::ShadeVerts(PatchMesh *msh, BoneModData *bmd)
{
		{
		if (ip)
			{
//get from mesh based on cahne
			PatchTVert *tVerts = NULL;
			TVPatch *tvFace = NULL;
			int CurrentChannel=0;
			if (!msh->getMapSupport(CurrentChannel))
				{
				msh->setNumMaps(CurrentChannel+1);
				}

		

			
			int nv = msh->numVerts;

			msh->setNumMapPatches(CurrentChannel,msh->getNumPatches());
			msh->setNumMapVerts (CurrentChannel,nv);


			tVerts = msh->tVerts[CurrentChannel];
			tvFace = msh->tvPatches[CurrentChannel];

			int nf = msh->numPatches;
			Point3 selSoft = GetUIColor(COLOR_SUBSELECTION_SOFT);
			Point3 selMedium = GetUIColor(COLOR_SUBSELECTION_MEDIUM);
			Point3 selHard = GetUIColor(COLOR_SUBSELECTION_HARD);
			Point3 selSubColor = GetUIColor(COLOR_SUBSELECTION);

			for (int i=0;i<bmd->VertexData.Count();i++)
				{
				Point3 col = selSoft * 0.65f ;
				if ( i < nv)
					tVerts[i] = col;
				for (int j=0;j<bmd->VertexData[i]->d.Count();j++)
					{
					int rigidBoneID;
					if (rigidVerts)
						rigidBoneID = bmd->VertexData[i]->GetMostAffectedBone();
//put in mirror check here
					int mirroredBone = -1;
					if (pPainterInterface)
						{
						if (pPainterInterface->GetMirrorEnable() && pPainterInterface->InPaintMode())
							{
							mirroredBone = mirrorIndex;
							}
						}

					if ( ( (bmd->VertexData[i]->d[j].Bones == ModeBoneIndex) || (bmd->VertexData[i]->d[j].Bones == mirroredBone))
						 && (bmd->VertexData[i]->d[j].Influences != 0.0f)  )
//					if ((bmd->VertexData[i]->d[j].Bones == ModeBoneIndex) && (bmd->VertexData[i]->d[j].Influences != 0.0f))
					
						{
						if (!((rigidVerts) && (ModeBoneIndex != rigidBoneID)))
							{
							Point3 pt;
							float infl;

							infl = RetrieveNormalizedWeight(bmd,i,j);
							j = bmd->VertexData[i]->d.Count();
							Color selColor(1.0f,1.0f,1.0f);
							if (infl > 0.0f)
								{
								if ( (infl<0.33f) && (infl > 0.0f))
									{
									selColor = selSoft + ( (selMedium-selSoft) * (infl/0.33f));
									}
								else if (infl <.66f)
									{
									selColor = selMedium + ( (selHard-selMedium) * ((infl-0.1f)/0.66f));
									}
								else if (infl < 1.0f)
									{
									selColor = selHard + ( (selSubColor-selHard) * ((infl-0.66f)/0.33f));
									}

								else 
									{
									selColor = selSubColor;
									}
								}

							if ( i < nv)
								tVerts[i]=selColor;
							}
						}
					}
				}
		
				

			for (i=0; i < nf; i++)
				{
				
				tvFace[i].tv[0] = msh->patches[i].v[0];
				tvFace[i].tv[1] = msh->patches[i].v[1];
				tvFace[i].tv[2] = msh->patches[i].v[2];
				tvFace[i].tv[3] = msh->patches[i].v[3];
				}

			}
		}

}
#endif


void BonesDefMod::ShadeVerts(MNMesh *msh, BoneModData *bmd)
{
		{
		if (ip)
			{
//get from mesh based on cahne
			Point3 *tVerts = NULL;
			MNMapFace *tvFace = NULL;
			int CurrentChannel=0;

			int numMaps = msh->MNum ();
			if (CurrentChannel >= numMaps) 
				{
				msh->SetMapNum(CurrentChannel+1);
				msh->InitMap(CurrentChannel);
				}


	

			
			int nv = msh->numv;

			tvFace = msh->M(CurrentChannel)->f;	
			
			if (!tvFace)
				{
				msh->InitMap(CurrentChannel);
				tvFace = msh->M(CurrentChannel)->f;
				}

			msh->M(CurrentChannel)->setNumVerts( nv);
			tVerts = msh->M(CurrentChannel)->v;


			int nf = msh->numf;
			Point3 selSoft = GetUIColor(COLOR_SUBSELECTION_SOFT);
			Point3 selMedium = GetUIColor(COLOR_SUBSELECTION_MEDIUM);
			Point3 selHard = GetUIColor(COLOR_SUBSELECTION_HARD);
			Point3 selSubColor = GetUIColor(COLOR_SUBSELECTION);

			for (int i=0;i<bmd->VertexData.Count();i++)
				{
				Point3 col = selSoft * 0.65f ;
				if ( i < nv)
					tVerts[i] = col;
				for (int j=0;j<bmd->VertexData[i]->d.Count();j++)
					{
					int rigidBoneID;
					if (rigidVerts)
						rigidBoneID = bmd->VertexData[i]->GetMostAffectedBone();


//put in mirror check here
					int mirroredBone = -1;
					if (pPainterInterface)
						{
						if (pPainterInterface->GetMirrorEnable() && pPainterInterface->InPaintMode())
							{
							mirroredBone = mirrorIndex;
							}
						}

					if ( ( (bmd->VertexData[i]->d[j].Bones == ModeBoneIndex) || (bmd->VertexData[i]->d[j].Bones == mirroredBone))
						 && (bmd->VertexData[i]->d[j].Influences != 0.0f)  )
//					if ((bmd->VertexData[i]->d[j].Bones == ModeBoneIndex) && (bmd->VertexData[i]->d[j].Influences != 0.0f))
					
						{
						if (!((rigidVerts) && (ModeBoneIndex != rigidBoneID)))
							{
							Point3 pt;
							float infl;

							infl = RetrieveNormalizedWeight(bmd,i,j);
							j = bmd->VertexData[i]->d.Count();
							Color selColor(1.0f,1.0f,1.0f);
							if (infl > 0.0f)
								{
								if ( (infl<0.33f) && (infl > 0.0f))
									{
									selColor = selSoft + ( (selMedium-selSoft) * (infl/0.33f));
									}
								else if (infl <.66f)
									{
									selColor = selMedium + ( (selHard-selMedium) * ((infl-0.1f)/0.66f));
									}
								else if (infl < 1.0f)
									{
									selColor = selHard + ( (selSubColor-selHard) * ((infl-0.66f)/0.33f));
									}

								else 
									{
									selColor = selSubColor;
									}
								}

							if ( i < nv)
								tVerts[i]=selColor;
							}
						}
					}
				}
		
				

			for (i=0; i < nf; i++)
				{
				if (msh->f[i].GetFlag (MN_DEAD)) continue;
				if (tvFace[i].deg != msh->f[i].deg) tvFace[i].SetSize (msh->f[i].deg);
				int ct =  tvFace[i].deg;
				for (int j = 0; j < ct; j++)
					tvFace[i].tv[j] = msh->f[i].vtx[j];
				}

			}
		}

}





BOOL RecursePipeAndMatch(ModContext *smd, Object *obj)
	{
	SClass_ID		sc;
	IDerivedObject* dobj;
	Object *currentObject = obj;

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
		{
		dobj = (IDerivedObject*)obj;
		while (sc == GEN_DERIVOB_CLASS_ID)
			{
			for (int j = 0; j < dobj->NumModifiers(); j++)
				{
				ModContext *mc = dobj->GetModContext(j);
				if (mc == smd)
					{
					return TRUE;
					}

				}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			currentObject = (Object*) dobj;
			sc = dobj->SuperClassID();
			}
		}

	int bct = currentObject->NumPipeBranches(FALSE);
	if (bct > 0)
		{
		for (int bi = 0; bi < bct; bi++)
			{
			Object* bobj = currentObject->GetPipeBranch(bi,FALSE);
			if (RecursePipeAndMatch(smd, bobj)) return TRUE;
			}

		}

	return FALSE;
}

INode* BonesDefMod::GetNodeFromModContext(ModContext *smd, int &which)
	{

	int	i;

    MyEnumProc dep;              
	EnumDependents(&dep);
	for ( i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			if ( RecursePipeAndMatch(smd,obj) )
				{
				which = i;
				return node;
				}
			}
		}
	return NULL;
	}

void BonesDefMod::CheckForXRefs(TimeValue t)
	{

	INode *rootNode = GetCOREInterface()->GetRootNode();
	int xct = rootNode->GetXRefFileCount();

	if (xct > 0)
		{

		XRefEnumProc dep;              
		dep.nukeME = FALSE;
		EnumDependents(&dep);
	
		INode *XRefNode = dep.Nodes[0];


		for (int xid = 0; xid < xct; xid++)
			{
			INode *xroot = rootNode->GetXRefTree(xid);
			BOOL amIanXRef = FALSE;
			if (xroot)
				amIanXRef = RecurseXRefTree(xroot,XRefNode);
			if (amIanXRef)
				{
				INode *tempBindNode = rootNode->GetXRefParent(xid);
				if (tempBindNode == NULL)
					{
					bindNode = NULL;

					}
				else if ((tempBindNode) && (bindNode!=tempBindNode))
					{
					BOOL isThereAnInitialMatrix=FALSE;
					TSTR there("InitialMatrix");
					TSTR entry;
//check if inode has initial matricx property if use that else
					if (bindNode!=NULL)
						{
						bindNode->GetUserPropBool(there,isThereAnInitialMatrix);
						}
					tempBindNode->GetUserPropBool(there,isThereAnInitialMatrix);
					if (isThereAnInitialMatrix) 
						{
//GetUserPropFloat(const TSTR &key,float &val)
						Point3 r1,r2,r3,r4;
						entry.printf(_T("r1x"));
						tempBindNode->GetUserPropFloat(entry,r1.x);
						entry.printf(_T("r1y"));
						tempBindNode->GetUserPropFloat(entry,r1.y);
						entry.printf(_T("r1z"));
						tempBindNode->GetUserPropFloat(entry,r1.z);
						entry.printf(_T("r2x"));
						tempBindNode->GetUserPropFloat(entry,r2.x);
						entry.printf(_T("r2y"));
						tempBindNode->GetUserPropFloat(entry,r2.y);
						entry.printf(_T("r2z"));
						tempBindNode->GetUserPropFloat(entry,r2.z);
						entry.printf(_T("r3x"));
						tempBindNode->GetUserPropFloat(entry,r3.x);
						entry.printf(_T("r3y"));
						tempBindNode->GetUserPropFloat(entry,r3.y);
						entry.printf(_T("r3z"));
						tempBindNode->GetUserPropFloat(entry,r3.z);
						entry.printf(_T("r4x"));
						tempBindNode->GetUserPropFloat(entry,r4.x);
						entry.printf(_T("r4y"));
						tempBindNode->GetUserPropFloat(entry,r4.y);
						entry.printf(_T("rz"));
						tempBindNode->GetUserPropFloat(entry,r4.z);
						initialXRefTM.SetRow(0,r1);
						initialXRefTM.SetRow(1,r2);
						initialXRefTM.SetRow(2,r3);
						initialXRefTM.SetRow(3,r4);

						}
					else 
						{
						initialXRefTM = tempBindNode->GetObjectTM(t);
						tempBindNode->SetUserPropBool(there,TRUE);
						Point3 r1,r2,r3,r4;
						r1 = initialXRefTM.GetRow(0);
						r2 = initialXRefTM.GetRow(1);
						r3 = initialXRefTM.GetRow(2);
						r4 = initialXRefTM.GetRow(3);
						entry.printf(_T("r1x"));
						tempBindNode->SetUserPropFloat(entry,r1.x);
						entry.printf(_T("r1y"));
						tempBindNode->SetUserPropFloat(entry,r1.y);
						entry.printf(_T("r1z"));
						tempBindNode->SetUserPropFloat(entry,r1.z);
						entry.printf(_T("r2x"));
						tempBindNode->SetUserPropFloat(entry,r2.x);
						entry.printf(_T("r2y"));
						tempBindNode->SetUserPropFloat(entry,r2.y);
						entry.printf(_T("r2z"));
						tempBindNode->SetUserPropFloat(entry,r2.z);
						entry.printf(_T("r3x"));
						tempBindNode->SetUserPropFloat(entry,r3.x);
						entry.printf(_T("r3y"));
						tempBindNode->SetUserPropFloat(entry,r3.y);
						entry.printf(_T("r3z"));
						tempBindNode->SetUserPropFloat(entry,r3.z);
						entry.printf(_T("r4x"));
						tempBindNode->SetUserPropFloat(entry,r4.x);
						entry.printf(_T("r4y"));
						tempBindNode->SetUserPropFloat(entry,r4.y);
						entry.printf(_T("rz"));
						tempBindNode->SetUserPropFloat(entry,r4.z);

						}
					bindNode = tempBindNode;

					}
				}
			}
		}
	}

BOOL BonesDefMod::RebuildTMLocalData(TimeValue t, ModContext &mc, BoneModData *bmd)
	{
	int which;
	INode *node = GetNodeFromModContext(&mc, which);

	if (node)
		{

		bmd->meshNode = node;
//		bmd->BaseTM = node->GetObjectTM(RefFrame);
//		bmd->BaseNodeTM = node->GetNodeTM(RefFrame); //ns
//		bmd->InverseBaseTM = Inverse(bmd->BaseTM);
		UpdateTMCacheTable(bmd,t,FOREVER);
		}
	return TRUE;

	}

BOOL BonesDefMod::InitLocalData(TimeValue t, ModContext &mc, ObjectState *os, INode *node, Interval valid)
	{

//	if (ip != NULL)
		{
		int which;
		INode *node = GetNodeFromModContext(&mc, which);
		if (node)
			{
			BoneModData *d  = new BoneModData(this);
			d->meshNode = node;
			d->BaseTM = node->GetObjectTM(RefFrame);
			d->BaseNodeTM = node->GetNodeTM(RefFrame); //ns
			d->InverseBaseTM = Inverse(d->BaseTM);
			UpdateTMCacheTable(d,t,valid);

			if ((OldVertexDataCount != 0))
				{
				d->VertexData.SetCount(OldVertexDataCount);
				for (int j = 0; j < OldVertexDataCount; j++)
					{
					VertexListClass *vc;
					vc = new VertexListClass;
					d->VertexData[j] = vc;

					d->selected.Set(j,OldVertexData[j]->selected);
					d->VertexData[j]->Modified (OldVertexData[j]->modified);
					d->VertexData[j]->LocalPos = OldVertexData[j]->LocalPos;
					d->VertexData[j]->d = OldVertexData[j]->d;
					}
				for	(j = 0; j < OldVertexDataCount; j++)
					delete (OldVertexData[j]);
	
				OldVertexData.ZeroCount();
				OldVertexDataCount = -1;
				}
			
			mc.localData = d;
			}
		}
//create a back pointer to the container entry				
/*		
	else 
		{
		int which;
		BoneModData *d  = new BoneModData(this);
		INode *baseNode = GetNodeFromModContext(&mc, which);
		d->BaseTM = baseNode->GetObjectTM(RefFrame);
		d->BaseNodeTM = baseNode->GetNodeTM(RefFrame); //ns
		d->InverseBaseTM = Inverse(d->BaseTM);
		UpdateTMCacheTable(d,t,valid);
		mc.localData = d;
		NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		valid.SetInstant(t);
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
		return TRUE;
		}
		*/
	return FALSE;
	}

void BonesDefMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{


	if (stopEvaluation) 
		{
		os->obj->UpdateValidity(GEOM_CHAN_NUM,Interval(t,t));
		return;
		}
	currentTime = t;



	

	TimeValue tps = GetTicksPerFrame();

//grabs all our data from the param block
	float ef;
	Interval valid = FOREVER;

	pblock_param->GetValue(skin_effect,t,ef,valid);



	pblock_param->GetValue(skin_filter_vertices,t,FilterVertices,valid);
	pblock_param->GetValue(skin_filter_bones,t,FilterBones,valid);
	pblock_param->GetValue(skin_filter_envelopes,t,FilterEnvelopes,valid);

	FilterVertices = !FilterVertices;
	FilterBones = !FilterBones;
	FilterEnvelopes = !FilterEnvelopes;


	pblock_display->GetValue(skin_display_all_vertices,t,drawAllVertices,valid);
	pblock_display->GetValue(skin_display_all_gizmos,t,displayAllGizmos,valid);
	pblock_display->GetValue(skin_display_draw_all_envelopes,t,DrawEnvelopes,valid);
	pblock_display->GetValue(skin_display_draw_vertices,t,DrawVertices,valid);




	pblock_advance->GetValue(skin_advance_ref_frame,t,RefFrame,valid);
	pblock_advance->GetValue(skin_advance_always_deform,t,AlwaysDeform,valid);

	pblock_advance->GetValue(skin_advance_rigid_verts,t,rigidVerts,valid);
	pblock_advance->GetValue(skin_advance_rigid_handles,t,rigidHandles,valid);
	pblock_advance->GetValue(skin_advance_fast_update,t,fastUpdate,valid);

	pblock_advance->GetValue(skin_advance_fast_update,t,fastUpdate,valid);

	BOOL noUpdate = FALSE;
	pblock_advance->GetValue(skin_advance_no_update,t,noUpdate,valid);

	pblock_advance->GetValue(skin_advance_updateonmouseup,t,updateOnMouseUp,valid);
	pblock_advance->GetValue(skin_advance_bonelimit,t,boneLimit,valid);
  	pblock_advance->GetValue(skin_advance_backtransform,t,backTransform,valid);

  	pblock_advance->GetValue(skin_advance_fastsubanims,t,enableFastSubAnimList,valid);

	BOOL useFastTMCache = TRUE;
	pblock_advance->GetValue(skin_advance_fasttmcache,t,useFastTMCache,valid);

	BOOL useFastVertexWeighting = TRUE;
	pblock_advance->GetValue(skin_advance_fastvertexweighting,t,useFastVertexWeighting,valid);

	pblock_advance->GetValue(skin_advance_fastgizmo,t,fastGizmo,valid);


    if (ModeBoneIndex > BoneData.Count() ) ModeBoneIndex = BoneData.Count()-1;
	
	RefFrame = RefFrame*tps;

	//this updates the list box for when it gets out of sync because of an unod
	if (updateListBox)
		{
		if (ip)
			{
			RefillListBox();
	
			if ( (ModeBoneIndex>=0) && (ModeBoneIndex<BoneData.Count())) 
				{
				int selID = ConvertSelectedBoneToListID(ModeBoneIndex);
				SendMessage(GetDlgItem(hParam,IDC_LIST1),LB_SETCURSEL ,selID,0);
				}
			UpdatePropInterface();
			}
		updateListBox = FALSE;
		}



	CheckForXRefs(t);

	// If this flag is set, we have to recompute the InitNodeTM. That means, that we check the current difference between the node TM
	// and the ObjectOffset TM and remove the inverse of that from the InitObject TM. Thus we get the NodeTM at init time.
	// This only works, if the object offset TM didn't change since the bones were assigned !!! This is done, so we can make Skin
	// work with Node TM's and not with Object TM's as in the shipping version.
    //this will not get called for max 4.5 and beyond files since this data is now saved.
	if(recompInitTM)
		{
		for (int i = 0; i < BoneData.Count(); i++)
			{
			INode *node = BoneData[i].Node;
			Matrix3 diffTM(TRUE);
			Matrix3 stretchTM;
			if(node)
//5.1.03
				{
			
				stretchTM = node->GetStretchTM(RefFrame);
				diffTM =  node->GetNodeTM(RefFrame) * Inverse(node->GetObjectTM(RefFrame));
//				diffTM = node->GetStretchTM(RefFrame) * node->GetNodeTM(RefFrame) * Inverse(node->GetObjectTM(RefFrame));
				}

			if (hasStretchTM)
				BoneData[i].InitNodeTM = diffTM * Inverse(BoneData[i].tm);
			else BoneData[i].InitNodeTM = stretchTM * diffTM * Inverse(BoneData[i].tm);
			BoneData[i].InitStretchTM = stretchTM;
			}
		recompInitTM = false;
		}




//build each instance local data
	if (mc.localData == NULL)
		{
		if (InitLocalData(t, mc, os, node,valid)) return;
		}


	BoneModData *bmd = (BoneModData *) mc.localData;			


	if (bmd == NULL) return;

	if (bmd->mod == NULL) bmd->mod = this;

	//this looks through all our bones seeing if any of their cross sections or
	//end/start points are animated.  If they are they are thrown into the 
	//invalidBones since they need to be recomputed
	Tab<int> invalidBones;

	BOOL animatedEnvelopes = FALSE;
	if (bmd->VertexData.Count() > 0)
		{
		for (int i = 0; i < BoneData.Count(); i++)
			{
			if (BoneData[i].Node)
				{
				Interval boneValid = FOREVER;
				Point3 dummyPoint;
				float dummyFloat;
//we need to get all the end/start point intervals by just getting the value
				BoneData[i].EndPoint1Control->GetValue(t,&dummyPoint,boneValid);
				BoneData[i].EndPoint2Control->GetValue(t,&dummyPoint,boneValid);
//we need to get all the crosssection intervals
				for (int j = 0; j < BoneData[i].CrossSectionList.Count(); j++)
					{
					BoneData[i].CrossSectionList[j].InnerControl->GetValue(t,&dummyFloat,boneValid);
					BoneData[i].CrossSectionList[j].OuterControl->GetValue(t,&dummyFloat,boneValid);
					}
//since these can be animated they must be part of our interval now also
				valid &= boneValid;
//any that are not forever need to be tagged to be recomputed
			

				if (!(boneValid == FOREVER))  //hack we dont have a != method on interval and adding it would be an SDK break
					{
					bmd->reevaluate = TRUE;
					animatedEnvelopes = TRUE;
					invalidBones.Append(1,&i);
					}
				}
			}
		}




	if (bmd->needTMsRebuilt)
		{
		RebuildTMLocalData(t, mc,bmd);
		bmd->needTMsRebuilt = FALSE;
		}

	if (bmd->meshNode == NULL)
		{
		
			MyEnumProc dep;              
			EnumDependents(&dep);
			//this puts back the original state of the node vc mods and shade state
			int nodeCount = 0;
			for (int  i = 0; i < dep.Nodes.Count(); i++)
				{
				if (dep.Nodes[i])
					{
					Object *obj = dep.Nodes[i]->GetObjectRef();

					if (obj->SuperClassID()==SYSTEM_CLASS_ID && obj->ClassID()==Class_ID(XREFOBJ_CLASS_ID,0)) 
						{
						bmd->meshNode = dep.Nodes[i];						
						}
					}

				
				
				}
			if (bmd->meshNode == NULL)
			{
				int which;
				bmd->meshNode =  GetNodeFromModContext(&mc, which);
			}
				
			
		}
	if ((bmd->meshNode) && (bmd->meshNode->SuperClassID()!=BASENODE_CLASS_ID))  //hack a alert because of compound objects the mesh node may dissappearl
		bmd->meshNode = NULL;


	// If this flag is set, we have to recompute the InitMeshTM. That means, that we check the current difference between the node TM
	// and the ObjectOffset TM and remove the inverse of that from the InitObject TM. Thus we get the NodeTM at init time.
	// This only works, if the object offset TM didn't change since the bones were assigned !!! This is done, so we can make Skin
	// work with Node TM's and not with Object TM's as in the shipping version.

	if(bmd->recompInitMeshTM && ip)
		{
		ModContextList mcList;
		INodeTab nodes;
		
		ip->GetModContexts(mcList,nodes);
			
		for (int i = 0; i < mcList.Count(); i++)
			{
			if(bmd == mcList[i]->localData)
				{
				Matrix3 diffTM(TRUE);
				//5.1.03
				Matrix3 stretchTM;
				
				stretchTM = nodes[i]->GetStretchTM(RefFrame);
				diffTM = nodes[i]->GetNodeTM(RefFrame) * Inverse(nodes[i]->GetObjectTM(RefFrame));
//				diffTM = nodes[i]->GetStretchTM(RefFrame) * nodes[i]->GetNodeTM(RefFrame) * Inverse(nodes[i]->GetObjectTM(RefFrame));
				
				bmd->BaseNodeTM = diffTM * bmd->BaseTM;
				}
			}
		bmd->recompInitMeshTM = false;
		}

//6-18-99
	bmd->inputObjectIsNURBS = os->obj->ClassID() == EDITABLE_SURF_CLASS_ID;	

	resolvedModify = TRUE;

	if (BoneData.Count() == 0) return;

//	if ((editing) || (bmd->pSE==NULL))
	BOOL forceRebuildCache = TRUE;
	if (useFastTMCache ) forceRebuildCache = FALSE;
	UpdateTMCacheTable(bmd,t,valid,forceRebuildCache);



	if ( (os->obj->NumPoints()==0) || (noUpdate))
		{
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
		return;
		}

#ifndef NO_PATCHES
	if (os->obj->IsSubClassOf(patchObjectClassID))
		{
		PatchObject *pobj = (PatchObject*)os->obj;
		
		int knots = pobj->patch.numVerts;
		bmd->autoInteriorVerts.SetSize(knots+pobj->patch.numVecs);
//		if (rigidHandles)
			{
			bmd->vecOwnerList.SetCount(knots+pobj->patch.numVecs);
			for (int vecID = 0; vecID < knots+pobj->patch.numVecs; vecID++)
				bmd->vecOwnerList[vecID] = -1;
			}

		bmd->autoInteriorVerts.ClearAll();
		for (int ipatch=0;ipatch<pobj->patch.numPatches;ipatch++)
			{
			int pc = 3;
			if (pobj->patch.patches[ipatch].type == PATCH_QUAD) pc = 4;
			if ((pobj->patch.patches[ipatch].flags &  PATCH_AUTO))
				{
				for (int ivec = 0; ivec < pc; ivec++)
//need to check if manual interio and mark if the  manuaul interio bug gets fixed
					bmd->autoInteriorVerts.Set(knots + pobj->patch.patches[ipatch].interior[ivec]);
				}
			}
//		if (rigidHandles)
			{
			for (int ivec = 0; ivec < pobj->patch.numVecs; ivec++)
//need to check if manual interio and mark if the  manuaul interio bug gets fixed
				{
				PatchVec pv = pobj->patch.vecs[ivec];
				bmd->vecOwnerList[knots + ivec] = pv.vert;

				}
			}	

		}
#endif

	splinePresent = FALSE;
	for (int i =0;i<BoneData.Count();i++)
		{
		if ((BoneData[i].Node != NULL) &&  (BoneData[i].flags & BONE_SPLINE_FLAG) )
			splinePresent = TRUE;
		}




	if (!painting) 
		{
		if (splineChanged)
			{
			if (whichSplineChanged < BoneData.Count())
				{
				if (BoneData[whichSplineChanged].Node)
					{

					ObjectState os = BoneData[whichSplineChanged].Node->EvalWorldState(t);
					if (os.obj->SuperClassID()!=SHAPE_CLASS_ID)
						{
//if not convert it to a regular node
						BoneData[whichSplineChanged].flags &= ~BONE_SPLINE_FLAG;
//watje 9-7-99  198721 
						bmd->reevaluate=TRUE;
						Point3 a,b;
						Interval v;
						float el1;
						Matrix3 ident(1);
						BuildMajorAxis(BoneData[whichSplineChanged].Node,a,b,el1,&ident); 

						BoneData[whichSplineChanged].EndPoint1Control->SetValue(currentTime,a,TRUE,CTRL_ABSOLUTE);
						BoneData[whichSplineChanged].EndPoint2Control->SetValue(currentTime,b,TRUE,CTRL_ABSOLUTE);
						cacheValid = FALSE;
						}
					else
						{
						ObjectState sos = BoneData[whichSplineChanged].Node->EvalWorldState(RefFrame);
						BezierShape bShape;
						ShapeObject *shape = (ShapeObject *)sos.obj;
						if (shape)
							{
							if(shape->CanMakeBezier())
//watje 9-7-99  195862 
								shape->MakeBezier(RefFrame, bShape);
							else {
								PolyShape pShape;
//watje 9-7-99  195862 
								shape->MakePolyShape(RefFrame, pShape);
								bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
								}
							}
						if ((shape) && (bShape.splineCount >0) &&
							(bShape.splines[0]->Segments() != BoneData[whichSplineChanged].referenceSpline.Segments()))
							{
							BoneData[whichSplineChanged].referenceSpline = *bShape.splines[0];
//watje 9-7-99  198721 
							bmd->reevaluate=TRUE;
							
							for (int j = 0; j < bmd->VertexData.Count(); j++)
								{

								if (bmd->VertexData[j]->IsModified())
									{
									int numberBones = bmd->VertexData[j]->d.Count();
									BOOL resetVert = FALSE;
									for (int k = 0; k < numberBones; k++)
										{
										int bid = bmd->VertexData[j]->d[k].Bones;
										if (bid == whichSplineChanged)
											{
											resetVert = TRUE;
											}
										}
									if (resetVert) bmd->VertexData[j]->Modified(FALSE);
									}
								
									
								}
							
							}

						}

					}


				}
			splineChanged = FALSE;
			}




		if (bmd->unlockVerts)
			{
			bmd->unlockVerts = FALSE;
//watje 9-7-99  198721 
			bmd->reevaluate=TRUE;
			for (int i=0;i<bmd->VertexData.Count();i++)
				{
				if (bmd->selected[i])
					bmd->VertexData[i]->Modified (FALSE);
				}
			}

		if (updateP)
			{
			updateP = FALSE;
			UpdateP(bmd);
			}

		if ((((t == (RefFrame)) && (BoneMoved)) && (!AlwaysDeform))  || bmd->forceRecomuteBaseNode)
			{
//readjust TMs for frame 0
			BoneMoved = FALSE;
//watje 9-7-99  198721 
			if (AlwaysDeform)
				bmd->reevaluate=TRUE;
			cacheValid = FALSE;
			bmd->forceRecomuteBaseNode = FALSE;
			bmd->CurrentCachePiece = -1;

			int which;

			INode *node = GetNodeFromModContext(&mc, which);
			if (node)
				{
				bmd->meshNode = node;
				bmd->BaseTM = node->GetObjectTM(RefFrame);
				bmd->BaseNodeTM = node->GetNodeTM(RefFrame); //ns
				bmd->InverseBaseTM = Inverse(bmd->BaseTM);
				}
/*
			if (ip != NULL)
				{
				ModContextList mcList;
				INodeTab nodes;
				ip->GetModContexts(mcList,nodes);

				for (int i = 0; i < nodes.Count(); i++)
					{
					BoneModData *d  = (BoneModData *) mcList[i]->localData;
					if (d)
						{
						d->BaseTM = nodes[i]->GetObjectTM(RefFrame);
						d->BaseNodeTM = nodes[i]->GetNodeTM(RefFrame); //ns
						d->InverseBaseTM = Inverse(d->BaseTM);
						}
					}
				}
*/
	
			for (int j =0;j<BoneData.Count();j++)
				{
				if (BoneData[j].Node != NULL)
					{
					Class_ID bid(BONE_CLASS_ID,0);
					Matrix3 ntm =BoneData[j].Node->GetObjectTM(RefFrame);	

					BoneData[j].InitObjectTM = ntm; //ns
					BoneData[j].tm = Inverse(ntm);
//5.1.03
					BoneData[j].InitStretchTM = BoneData[j].Node->GetStretchTM(RefFrame); //ns
					if (hasStretchTM)
						BoneData[j].InitNodeTM = BoneData[j].Node->GetNodeTM(RefFrame); //ns
					else BoneData[j].InitNodeTM = BoneData[j].InitStretchTM * BoneData[j].Node->GetNodeTM(RefFrame); //ns
//					BoneData[j].InitNodeTM = BoneData[j].Node->GetStretchTM(RefFrame) * BoneData[j].Node->GetNodeTM(RefFrame); //ns
//copy initial reference spline into our spline

					ObjectState sos = BoneData[j].Node->EvalWorldState(RefFrame);
					if (BoneData[j].flags & BONE_SPLINE_FLAG)
						{
						BezierShape bShape;
						ShapeObject *shape = (ShapeObject *)sos.obj;
						if (shape)
							{
							if(shape->CanMakeBezier())
								shape->MakeBezier(t, bShape);
							else {
								PolyShape pShape;
								shape->MakePolyShape(t, pShape);
								bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
								}
							if (bShape.splineCount >0) 
								BoneData[j].referenceSpline = *bShape.splines[0];
							}

						}
//need to readjust the child endpoints for bone type objects also
					else if (sos.obj->ClassID() == bid)  
						{
//now need to look at child and move
//loop through children looking for a matching name
						int childCount;
						childCount = BoneData[j].Node->NumberOfChildren();
						INode *childNode = NULL;
						for (int ci = 0; ci < childCount; ci++)
							{
							childNode = BoneData[j].Node->GetChildNode(ci);
							TSTR childName;
							childName = childNode->GetName();
							if (childName == BoneData[j].name)
								{
								Point3 l2(0.0f,0.0f,0.0f);
								Matrix3 ChildTM = childNode->GetObjectTM(RefFrame);
								l2 = l2 * ChildTM;
								l2 = l2 * BoneData[j].tm; 
								if (j < endPointDelta.Count())
									{
									l2 = l2+endPointDelta[j];
									BoneData[j].EndPoint2Control->SetValue(currentTime,&l2,TRUE,CTRL_ABSOLUTE);
									}
								else BoneData[j].EndPoint2Control->SetValue(currentTime,&l2,TRUE,CTRL_ABSOLUTE);
								ci = childCount;

								}
							
							}

						}



					}
				}
			UpdateTMCacheTable(bmd,t,valid);

			}
		if (reloadSplines)
			{
			for (int j =0;j<BoneData.Count();j++)
				{
				if (BoneData[j].Node != NULL)
					{
//copy initial reference spline into our spline
					if (BoneData[j].flags & BONE_SPLINE_FLAG)
						{
						ObjectState sos = BoneData[j].Node->EvalWorldState(RefFrame);
						
						if (sos.obj->SuperClassID()==SHAPE_CLASS_ID) 
							{

							BezierShape bShape;
							ShapeObject *shape = (ShapeObject *)sos.obj;
							if (shape)
								{
								if(shape->CanMakeBezier())
									shape->MakeBezier(t, bShape);
								else {
									PolyShape pShape;
									shape->MakePolyShape(t, pShape);
									bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
									}
								if (bShape.splineCount >0) 
									BoneData[j].referenceSpline = *bShape.splines[0];
								}
							}
						else BoneData[j].flags &= ~BONE_SPLINE_FLAG;

						}


					}
				}
			reloadSplines = FALSE;

			}

		if (bmd->selected.GetSize() != os->obj->NumPoints())
			bmd->selected.SetSize(os->obj->NumPoints(),TRUE);




//get selected bone	
		int rsel = 0;

		rsel = SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_GETCURSEL ,0,0);
		int tsel = ConvertSelectedListToBoneID(rsel);


	


		if ( (tsel>=0) && (ip && ip->GetSubObjectLevel() == 1) )
			{
			ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hParam,IDC_EFFECTSPIN));

			if ((!spin2->IsIndeterminate()) && (ef != bmd->effect))
				{
				bmd->effect = ef;
				SetSelectedVertices(bmd,tsel, bmd->effect);
				}
			}

//set validty based on TM's
		for (int i =0;i<BoneData.Count();i++)
			{
			if (BoneData[i].Node != NULL) 
				{
				BoneData[i].Node->GetObjectTM(t,&valid);
				if (BoneData[i].flags & BONE_SPLINE_FLAG)
					{
					ObjectState osp = BoneData[i].Node->EvalWorldState (t);
					valid &= osp.obj->ObjectValidity (t);
					}
				}

			}

//get selected bone	
		if (bmd->VertexData.Count() != os->obj->NumPoints())
			{
//readjust vertices using nearest vertices as sample
//watje 9-7-99  198721 
			bmd->reevaluate = TRUE;
			bmd->CurrentCachePiece = -1;
			if (bmd->VertexData.Count() != 0)
				{
				Box3 bbox;
				os->obj->GetDeformBBox(t, bbox);
				float threshold = Length(bbox.pmin-bbox.pmax)/10.0f;
				RemapExclusionData(bmd, threshold, -1,os->obj);
				RemapLocalGimzoData(bmd, threshold, -1,os->obj);
				RemapVertData(bmd, threshold, -1,os->obj);
				bmd->CleanUpExclusionLists();

				
				}
			}

		if (unlockBone)
			{
			unlockBone = FALSE;
//watje 9-7-99  198721 
			bmd->reevaluate = TRUE;
			UnlockBone(bmd,t,os);
			}

		if (unlockAllBones)
			{
			unlockAllBones = FALSE;
			bmd->reevaluate = TRUE;
			UnlockAllBones(bmd,t,os);
			}


//Need to recompute any bone that was not valid, this will completely fubar our cache but not much we can do about it
//Will need to rethink how to cache this
		BOOL repaintWeightTable = FALSE;
		for (i = 0; i < invalidBones.Count(); i++)
			{
			if (!((ip) || (bmd->reevaluate)))
				RecomputeBone(bmd,invalidBones[i],t,os);
			}
		

		if ((ip) || (bmd->reevaluate))
			{
			if ( (bmd->reevaluate) )
				{
				RecomputeAllBones(bmd,t,os);
				cacheValid = FALSE;
				bmd->CurrentCachePiece = -1;
				//WEIGHTTABLE
				repaintWeightTable = TRUE;
//since we rebuild all the bones we need to renormalize all our weights agsin				
				bmd->rebuildWeights = TRUE;
				}	

			else if ( (ModeBoneIndex!=-1) && (ip && ip->GetSubObjectLevel() == 1))//&& (ModeEdit ==1) )
				{
				RecomputeBone(bmd,ModeBoneIndex,t,os);
				//WEIGHTTABLE
				repaintWeightTable = TRUE;
//since we rebuild all the bones we need to renormalize all our weights agsin				
				bmd->rebuildWeights = TRUE;				
				}
			}

		if (repaintWeightTable) PaintAttribList();




		for (i = 0; i < os->obj->NumPoints(); i++)
			bmd->VertexData[i]->LocalPos = os->obj->GetPoint(i);

		}
	valid &= GetValidity(t);

#ifndef NO_PATCHES
	if ((os->obj->IsSubClassOf(patchObjectClassID)) && (rigidHandles))
		{
//loop through vecs looking for owners
		for (int i = 0; i < bmd->vecOwnerList.Count(); i++)
			{
			if (bmd->vecOwnerList[i] > 0)
				{
				int owner = bmd->vecOwnerList[i];
				bmd->VertexData[i]->d = bmd->VertexData[owner]->d;
				}
			}
		}
#endif

	if (os->obj->IsSubClassOf(triObjectClassID))
		{
		bmd->isMesh = TRUE;
		bmd->isPatch = FALSE;
		}
#ifndef NO_PATCHES
	else if (os->obj->IsSubClassOf(patchObjectClassID))
		{
		bmd->isMesh = FALSE;
		bmd->isPatch = TRUE;
		}
#endif
	else if (os->obj->IsParamSurface()) 
		{
			
		}
	else 

		{
//ask if can convert to mesh
		if (os->obj->CanConvertToType(triObjectClassID))
			{
			bmd->isMesh = TRUE;
			bmd->isPatch = FALSE;
			}
		else
			{
			bmd->isMesh = FALSE;
			bmd->isPatch = FALSE;
			}

		}



	if (1)
		{
//new fix 2
		gizmoDefList.SetCount(pblock_gizmos->Count(skin_gizmos_list));
		gizmoDefListCount = pblock_gizmos->Count(skin_gizmos_list);
		for(int  i = 0 ; i < bmd->gizmoData.Count() ; i++)
			{
			int id = bmd->gizmoData[i]->whichGizmo;
			int gizmoCount = pblock_gizmos->Count(skin_gizmos_list);
			ReferenceTarget *ref;
			if (id < gizmoCount)
				{
				ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,id);
				if (ref)
					{
					GizmoClass *gizmo = (GizmoClass *)ref;
					if (gizmo->IsVolumeBased())
						bmd->gizmoData[i]->FreeDeformingList();
					else bmd->gizmoData[i]->BuildDeformingList();

					}
				}
			}
		
		for (i = 0; i < pblock_gizmos->Count(skin_gizmos_list); i++) 
			{
			gizmoDefList[i] = NULL;
			ReferenceTarget *ref;
			ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,i);

//lets the gizmo know about the double transform 
			IGizmoClass2 *giz2 = (IGizmoClass2 *) ref->GetInterface(I_GIZMO2);
			if (giz2)
				giz2->SetBackTransFormMatrices(bmd->gizmoRemoveDoubleOffset,bmd->gizmoPutBackDoubleOffset);


			GizmoClass *gizmo = (GizmoClass *)ref;
			gizmoDefList[i] = gizmo;
			if (gizmo) gizmo->PreDeformSetup( t);
			}

		for (i = 0; i < splineList.Count(); i++)
			{
			if (splineList[i])
				{
				delete splineList[i];
				splineList[i] = NULL;
				}
			}
		splineList.SetCount(BoneData.Count());
		for (i = 0; i < BoneData.Count(); i++)
			{
			splineList[i] = NULL;
			if ((BoneData[i].Node) && (BoneData[i].flags & BONE_SPLINE_FLAG))
				{

				ObjectState sos = BoneData[i].Node->EvalWorldState(t);
				BezierShape bShape;
				ShapeObject *shape = (ShapeObject *)sos.obj;
				if (shape->NumberOfCurves() > 0)
					{
					int sct = shape->NumberOfPieces(t,0);
					int rct = BoneData[i].referenceSpline.Segments();

					if ((shape) && (sct != rct))
						{
						splineList[i] = new Spline3D();

						if(shape->CanMakeBezier())
							shape->MakeBezier(t, bShape);
						else {
							PolyShape pShape;
							shape->MakePolyShape(t, pShape);
							bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
							}
					
						*splineList[i] = *bShape.splines[0];
						splineList[i]->InvalidateGeomCache();
						splineList[i]->ComputeBezPoints();
						}
					}


				}
			}


	// Here comes the COM engine setup
		if (bmd->pSE)
			{
//5.1.03
			BOOL ignoreBoneScale;
			pblock_advance->GetValue(skin_advance_ignorebonescale,0,ignoreBoneScale,FOREVER);

			bmd->pSE->SetNumPoints(os->obj->NumPoints());
			bmd->pSE->SetInitTM((float *)&bmd->BaseTM);
			bmd->pSE->SetNumBones(BoneData.Count());
			for( i = 0 ; i < BoneData.Count() ; i++)
			{
				if (BoneData[i].Node != NULL) 
				{
					Matrix3 ntm;// = BoneData[i].Node->GetObjectTM(RefFrame);
			
					if(BoneData[i].flags & BONE_SPLINE_FLAG)
					{
				// In case we got a spline we want to use the object offset TM
		
						ntm = BoneData[i].Node->GetObjectTM(t);
						if (bindNode)
							{
							Matrix3 xRefTM = bindNode->GetObjectTM(t);
							ntm =  ntm *  Inverse(xRefTM) * initialXRefTM ;
							}
						else if ((backTransform) && (bmd->meshNode))
							{
							Matrix3 backTM = bmd->meshNode->GetNodeTM(t);
							ntm =  ntm *  Inverse(backTM) * bmd->BaseNodeTM ;
							}

						bmd->pSE->SetBoneTM(i,(float *) &ntm);
						bmd->pSE->SetInitBoneTM(i,(float *) &BoneData[i].InitObjectTM);
					}
					else
					{
//5.1.03
						if (!hasStretchTM)
							ntm = GetStretchTM(t, BoneData[i].Node, i) * BoneData[i].Node->GetNodeTM(t);
						else if (ignoreBoneScale)
							ntm =  BoneData[i].Node->GetNodeTM(t);
						else ntm = GetStretchTM(t, BoneData[i].Node, i) * BoneData[i].Node->GetNodeTM(t);

						if (bindNode)
							{
							Matrix3 xRefTM = bindNode->GetObjectTM(t);
							ntm =   ntm *  Inverse(xRefTM) * initialXRefTM ;
							}
						else if ((backTransform) && (bmd->meshNode))
							{
							Matrix3 backTM = bmd->meshNode->GetNodeTM(t);
							ntm =  ntm *  Inverse(backTM) * bmd->BaseNodeTM ;
							}

						bmd->pSE->SetBoneTM(i,(float *) &ntm);	
//5.1.03						
						if (!hasStretchTM)
							bmd->pSE->SetInitBoneTM(i,(float *) &(BoneData[i].InitNodeTM));
						else if (ignoreBoneScale)
							bmd->pSE->SetInitBoneTM(i,(float *) &(BoneData[i].InitNodeTM));
						else bmd->pSE->SetInitBoneTM(i,(float *) &(BoneData[i].InitStretchTM*BoneData[i].InitNodeTM));
					}				
				}
				bmd->pSE->SetBoneFlags(i,BoneData[i].flags);
			}
		}
		


	
		bmd->CurrentTime = t;
	
		if ((t == RefFrame) && (!AlwaysDeform))
			{
			}
		else
			{
//Optimize this we only need to build this is they are editing or there is an animated bone
			if ((useFastVertexWeighting == FALSE) || animatedEnvelopes || editing || bmd->rebuildWeights)
				{
				BuildNormalizedWeight(bmd);
				bmd->rebuildWeights = FALSE;
				}

			BonesDefDeformer deformer(this,bmd,t);
			os->obj->Deform(&deformer, TRUE);
			}

//new fix 2
		for(i = 0 ; i < bmd->gizmoData.Count() ; i++)
			{
			int id = bmd->gizmoData[i]->whichGizmo;
			int gizmoCount = pblock_gizmos->Count(skin_gizmos_list);
			ReferenceTarget *ref;
			if (id < gizmoCount)
				{
				ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,id);
				if (ref)
					{
					GizmoClass *gizmo = (GizmoClass *)ref;
					bmd->gizmoData[i]->FreeDeformingList();
					}
				}
			}

	
		for (i = 0; i < pblock_gizmos->Count(skin_gizmos_list); i++) 
			{
			ReferenceTarget *ref;
			ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,i);
			GizmoClass *gizmo = (GizmoClass *)ref;
			if (gizmo) gizmo->PostDeformSetup( t);
			}
		}


	pblock_display->GetValue(skin_display_shadeweights,0,shadeVC,FOREVER);
	if ((ip) &&  (shadeVC) && (ip->GetSubObjectLevel() == 1) )
		{
		if (os->obj->IsSubClassOf(triObjectClassID))
			{
			SetVCMode();

			TriObject *tobj = (TriObject*)os->obj;
			Mesh &msh = tobj->GetMesh();
			ShadeVerts(&msh,bmd);
			os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM, Interval(t,t));

			}
#ifndef NO_PATCHES
		else if (os->obj->IsSubClassOf(patchObjectClassID))
			{
			SetVCMode();

			PatchObject *pobj = (PatchObject*)os->obj;
			PatchMesh &msh = pobj->patch;
			ShadeVerts(&msh,bmd);
			os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM, Interval(t,t));

			}
#endif
		else if (os->obj->IsSubClassOf(polyObjectClassID)) 
			{
			SetVCMode();

			PolyObject *tobj = (PolyObject*)os->obj;
// Apply our mapping
			MNMesh &mesh = tobj->GetMesh();
			ShadeVerts(&mesh,bmd);
			os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM, Interval(t,t));

			}
		else shadeVC = FALSE;
		}

	
	for (i = 0; i < os->obj->NumPoints(); i++)
	 	bmd->VertexData[i]->LocalPosPostDeform = os->obj->GetPoint(i);




	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	

	}

float BonesDefMod::GetSquash(TimeValue t, INode *node)
{
Matrix3 tm = node->GetStretchTM(t);
int dir = node->GetBoneAxis();
Point3 a,b,c;
a = tm.GetRow(0);
b = tm.GetRow(1);
c = tm.GetRow(2);
if (dir == 0) return b[1];
else if (dir == 1) return c[2];
else return a[0];

}

Matrix3 BonesDefMod::GetStretchTM(TimeValue t, INode *node, int index)
{

if ( (index>=pblock_param->Count(skin_initial_squash))  || (index>=pblock_param->Count(skin_local_squash)) )
	{
	for (int i =0; i < index+1; i++)
		{
		INode *n = BoneData[i].Node;
		if (n)
			{
			float f = 1.0f;
			pblock_param->Append(skin_local_squash,1,&f);
			f = GetSquash(RefFrame, n);
			pblock_param->Append(skin_initial_squash,1,&f);
			}
		else
			{
			float f = 1.0f;
			pblock_param->Append(skin_local_squash,1,&f);
			pblock_param->Append(skin_initial_squash,1,&f);

			}
		}


	}
Matrix3 tm = node->GetStretchTM(t);
if ((index >=0) &&  (index<pblock_param->Count(skin_local_squash)) && (index<pblock_param->Count(skin_initial_squash)))
	{
	float squash;
	float initialSquash;
	pblock_param->GetValue(skin_local_squash,t,squash,FOREVER,index);
	pblock_param->GetValue(skin_initial_squash,t,initialSquash,FOREVER,index);
	float currentSquash = GetSquash(t, node) ;
	squash = 1.0f + (1.0f - squash);
	squash= initialSquash - ((initialSquash- currentSquash )*squash);

	Point3 scale(squash,squash,squash);
	int dir = node->GetBoneAxis();
	Point3 a;
	a = tm.GetRow(dir);
	scale[dir] = a[dir];
	a = tm.GetRow(0);
	a[0] = scale[0];
	tm.SetRow(0,a);

	a = tm.GetRow(1);
	a[1] = scale[1];
	tm.SetRow(1,a);

	a = tm.GetRow(2);
	a[2] = scale[2];
	tm.SetRow(2,a);


	}
return tm;
}

#define ID_CHUNK 0x1000

IOResult BonesDefMod::SaveLocalData(ISave *isave, LocalModData *pld)
{

ULONG		nb;

BoneModData *bmd = (BoneModData*)pld;


	isave->BeginChunk(BASE_TM_CHUNK);
	bmd->BaseTM.Save(isave);
	isave->EndChunk();



	int c = bmd->VertexData.Count();
//save vertex influence info
	isave->BeginChunk(VERTEX_COUNT_CHUNK);
	isave->Write(&c,sizeof(c),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(VERTEXV5_DATA_CHUNK);
	for (int i = 0; i < c; i++)
		{
//write number of influences
		int ic;
		ic = bmd->VertexData[i]->d.Count();
		isave->Write(&ic,sizeof(ic),&nb);
		int save_i;
		float save_f;
//		BOOL save_b;
//		save_b = bmd->VertexData[i]->modified;
//		isave->Write(&save_b,sizeof(save_b),&nb);

		DWORD save_dword;
		save_dword = bmd->VertexData[i]->flags;
		isave->Write(&save_dword,sizeof(save_dword),&nb);

		for (int j = 0; j < ic; j++)
			{
			save_i = bmd->VertexData[i]->d[j].Bones;
			save_f = bmd->VertexData[i]->d[j].Influences;
			isave->Write(&save_i,sizeof(save_i),&nb);
			isave->Write(&save_f,sizeof(save_f),&nb);

			save_i = bmd->VertexData[i]->d[j].SubCurveIds;
			isave->Write(&save_i,sizeof(save_i),&nb);
			save_i = bmd->VertexData[i]->d[j].SubSegIds;
			isave->Write(&save_i,sizeof(save_i),&nb);

			save_f = bmd->VertexData[i]->d[j].u;
			isave->Write(&save_f,sizeof(save_f),&nb);

			Point3 save_p;
			save_p = bmd->VertexData[i]->d[j].Tangents;
			isave->Write(&save_p,sizeof(save_p),&nb);

			save_p = bmd->VertexData[i]->d[j].OPoints;
			isave->Write(&save_p,sizeof(save_p),&nb);



			}

		}
	isave->EndChunk();

	isave->BeginChunk(EXCLUSION_CHUNK);
//count then data
	c = bmd->exclusionList.Count();
	isave->Write(&c,sizeof(c),&nb);
//active count
	int exct = 0;
	for (i = 0; i < c; i++)
		{
		if (bmd->exclusionList[i])
			exct++;
		}
	isave->Write(&exct,sizeof(exct),&nb);

//tab of which ones
	for ( i =0; i < c; i ++)
		{
//then data
		if (bmd->exclusionList[i])
			{	
			isave->Write(&i,sizeof(i),&nb);
			bmd->exclusionList[i]->Save(isave);
			}
		}
	isave->EndChunk();


	isave->BeginChunk(GIZMOCOUNT_CHUNK);
//count then data
	c = bmd->gizmoData.Count();
	isave->Write(&c,sizeof(c),&nb);
	for ( i =0; i < c; i ++)
		isave->Write(&bmd->gizmoData[i]->whichGizmo,sizeof(bmd->gizmoData[i]->whichGizmo),&nb);

	isave->EndChunk();

	isave->BeginChunk(GIZMODATA_CHUNK);
//count then data
	c = bmd->gizmoData.Count();
	for ( i =0; i < c; i ++)
		{
		bmd->gizmoData[i]->Save(isave);
		}
	isave->EndChunk();

	isave->BeginChunk(NAMEDSEL_BITSCOUNT_CHUNK);
//count then data
	c = bmd->namedSelSets.Count();
	isave->Write(&c,sizeof(c),&nb);
	isave->EndChunk();

	for (i =0; i < c; i++)
		{
		if (bmd->namedSelSets[i])
			{
			isave->BeginChunk(NAMEDSEL_BITSID_CHUNK);
			int id  =i;
			isave->Write(&i,sizeof(i),&nb);
			isave->EndChunk();

			isave->BeginChunk(NAMEDSEL_BITS_CHUNK);
			bmd->namedSelSets[i]->Save(isave);
			isave->EndChunk();

			}
		}

	isave->BeginChunk(BASENODE_TM_CHUNK);
	bmd->BaseNodeTM.Save(isave);
	isave->EndChunk();

//5.1.03	
	if (bmd->meshNode)
		{
		ULONG id = isave->GetRefID(bmd->meshNode);

		isave->BeginChunk(MESHNODEBACKPATCH_CHUNK);
		isave->Write(&id,sizeof(ULONG),&nb);
		isave->EndChunk();
		}


return IO_OK;
}

IOResult BonesDefMod::LoadLocalData(ILoad *iload, LocalModData **pld)

{
	IOResult	res;
	ULONG		nb;

	BoneModData *bmd = new BoneModData(this);
	*pld = bmd;
	bmd->effect = -1.0f;
	bmd->reevaluate = TRUE;


	int exsize,exsizeActive, exID,k;
	int lastID = 0;

	bmd->recompInitMeshTM = true;
	
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {

			case NAMEDSEL_BITSCOUNT_CHUNK: 
				{
				int ct = 0;
				iload->Read(&ct,sizeof(ct), &nb);
				bmd->namedSelSets.SetCount(ct);
				for (int i =0; i < ct; i++)
					bmd->namedSelSets[i] = NULL;
				break;
				}
			case NAMEDSEL_BITSID_CHUNK: 
				{
				iload->Read(&lastID,sizeof(lastID), &nb);
				break;
				}

			case NAMEDSEL_BITS_CHUNK: {
				

				BitArray *bits = new BitArray;
				bits->Load(iload);
				bmd->namedSelSets[lastID] = bits;
				break;
				}


			case GIZMOCOUNT_CHUNK:
				{
				int c;
				iload->Read(&c,sizeof(int), &nb);
				bmd->gizmoData.SetCount(c);
				for (k = 0; k < c; k++)
					{
					bmd->gizmoData[k] = new LocalGizmoData();
					iload->Read(&bmd->gizmoData[k]->whichGizmo,sizeof(bmd->gizmoData[k]->whichGizmo), &nb);

					}


				break;
				}
			case GIZMODATA_CHUNK:
				{

				for (k = 0; k < bmd->gizmoData.Count(); k++)
					{
					bmd->gizmoData[k]->Load(iload);
					}

				break;
				}

			case EXCLUSION_CHUNK:
//read size		
				iload->Read(&exsize,sizeof(int), &nb);
				iload->Read(&exsizeActive,sizeof(int), &nb);
				bmd->exclusionList.SetCount(exsize);
				for (k = 0; k < exsize; k++)
					bmd->exclusionList[k] = NULL;
				for (k = 0; k < exsizeActive; k++)
					{
					iload->Read(&exID,sizeof(int), &nb);

					bmd->exclusionList[exID] = new ExclusionListClass;
					bmd->exclusionList[exID]->Load(iload);
					}
				



				break;

			case BASE_TM_CHUNK: 
				{
				bmd->BaseTM.Load(iload);
				bmd->InverseBaseTM = Inverse(bmd->BaseTM);
				
				// Make sure, the InitMesh Node TM will be recomputed (see ModifyObject) ns
				bmd->recompInitMeshTM = true;
				break;
				}


			case VERTEX_COUNT_CHUNK:
				{
				int c;
				iload->Read(&c,sizeof(c),&nb);
				bmd->VertexData.ZeroCount();
				bmd->VertexData.SetCount(c);
				for (int i=0; i<c; i++) {
					VertexListClass *vc;
					vc = new VertexListClass;
					bmd->VertexData[i] = vc;
					bmd->VertexData[i]->Modified (FALSE);
//					bmd->VertexData[i]->Selected (FALSE);
					bmd->VertexData[i]->flags = 0;
 					bmd->VertexData[i]->d.ZeroCount();
					}

				break;

				}
//version 4.2 and less
			case VERTEX_DATA_CHUNK:
				{
				for (int i=0; i < bmd->VertexData.Count(); i++)
					{
					int c;
					BOOL load_b;
					iload->Read(&c,sizeof(c),&nb);
					bmd->VertexData[i]->d.SetCount(c);

					iload->Read(&load_b,sizeof(load_b),&nb);
					bmd->VertexData[i]->Modified (load_b);
					float load_f;
					int load_i;
					Point3 load_p;
					float sum = 0.0f;
					for (int j=0; j<c; j++) {
						iload->Read(&load_i,sizeof(load_i),&nb);
						iload->Read(&load_f,sizeof(load_f),&nb);
 						bmd->VertexData[i]->d[j].Bones = load_i;
/*						if ((c ==1 ) && (bmd->VertexData[i]->IsModified()))
							{
							load_f = 1.0f;
							}*/
						bmd->VertexData[i]->d[j].Influences =load_f;
						sum += load_f;

						iload->Read(&load_i,sizeof(load_i),&nb);
						bmd->VertexData[i]->d[j].SubCurveIds =load_i;
						iload->Read(&load_i,sizeof(load_i),&nb);
						bmd->VertexData[i]->d[j].SubSegIds =load_i;

						iload->Read(&load_f,sizeof(load_f),&nb);
 						bmd->VertexData[i]->d[j].u = load_f;

						iload->Read(&load_p,sizeof(load_p),&nb);
 						bmd->VertexData[i]->d[j].Tangents = load_p;

						iload->Read(&load_p,sizeof(load_p),&nb);
 						bmd->VertexData[i]->d[j].OPoints = load_p;


						}
					if (bmd->VertexData[i]->IsModified() && (fabs(sum-1.0f) > 0.01f))
						{
						
						if (c==1) 
							{	
							int boneIndex = bmd->VertexData[i]->d[0].Bones;
							if ((boneIndex >=0) && (boneIndex < BoneData.Count()) && (!(BoneData[boneIndex].flags&BONE_ABSOLUTE_FLAG)))
								bmd->VertexData[i]->UnNormalized(TRUE);
							else 	bmd->VertexData[i]->d[0].Influences = 1.0f;
							}
						else bmd->VertexData[i]->UnNormalized(FALSE);
						}

					}

				break;

				}
//version 4.5 and greater
			case VERTEXV5_DATA_CHUNK:
				{
				for (int i=0; i < bmd->VertexData.Count(); i++)
					{
					int c;
//					BOOL load_b;
					DWORD load_dword;
					iload->Read(&c,sizeof(c),&nb);
					bmd->VertexData[i]->d.SetCount(c);


					iload->Read(&load_dword,sizeof(load_dword),&nb);
					bmd->VertexData[i]->flags = load_dword;

					float load_f;
					int load_i;
					Point3 load_p;
					for (int j=0; j<c; j++) {
						iload->Read(&load_i,sizeof(load_i),&nb);
						iload->Read(&load_f,sizeof(load_f),&nb);
 						bmd->VertexData[i]->d[j].Bones = load_i;
						bmd->VertexData[i]->d[j].Influences =load_f;

						iload->Read(&load_i,sizeof(load_i),&nb);
						bmd->VertexData[i]->d[j].SubCurveIds =load_i;
						iload->Read(&load_i,sizeof(load_i),&nb);
						bmd->VertexData[i]->d[j].SubSegIds =load_i;

						iload->Read(&load_f,sizeof(load_f),&nb);
 						bmd->VertexData[i]->d[j].u = load_f;

						iload->Read(&load_p,sizeof(load_p),&nb);
 						bmd->VertexData[i]->d[j].Tangents = load_p;

						iload->Read(&load_p,sizeof(load_p),&nb);
 						bmd->VertexData[i]->d[j].OPoints = load_p;


						}
					}

				break;

				}
			case BASENODE_TM_CHUNK: 
				{
				bmd->BaseNodeTM.Load(iload);
				bmd->recompInitMeshTM = false;
				break;
				}
//5.1.03
/*
			case MESHNODEBACKPATCH_CHUNK:
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&bmd->meshNode);
					}
					
				break;
*/				

			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}


    int c = bmd->VertexData.Count();
//add m crossection
	bmd->selected.SetSize(c);
	bmd->selected.ClearAll();
	bmd->CurrentCachePiece = -1;

return IO_OK;

}

void BonesDefMod::SaveEnvelopeDialog(BOOL defaultToBinary)
{
static TCHAR fname[256] = {'\0'};
OPENFILENAME ofn;
memset(&ofn,0,sizeof(ofn));
FilterList fl;
fl.Append( GetString(IDS_PW_ENVFILES));
fl.Append( _T("*.env"));		
fl.Append( GetString(IDS_PW_ENVTEXTFILES));
fl.Append( _T("*.envASCII"));		
TSTR title = GetString(IDS_PW_SAVEENVELOPES);

ofn.lStructSize     = sizeof(OPENFILENAME);
ofn.hwndOwner       = GetCOREInterface()->GetMAXHWnd();
ofn.lpstrFilter     = fl;
ofn.lpstrFile       = fname;
ofn.nMaxFile        = 256;    
ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
if (defaultToBinary)
	ofn.lpstrDefExt     = _T("env");
else ofn.lpstrDefExt     = _T("envASCII");
ofn.lpstrTitle      = title;

tryAgain:
if (GetSaveFileName(&ofn)) {
	if (DoesFileExist(fname)) {
		TSTR buf1;
		TSTR buf2 = GetString(IDS_PW_SAVEENVELOPES);
		buf1.printf(GetString(IDS_PW_FILEEXISTS),fname);
		if (IDYES!=MessageBox(
			hParam,
			buf1,buf2,MB_YESNO|MB_ICONQUESTION)) {
			goto tryAgain;
			}
		}
//save stuff here
// this is timed slice so it will not save animation not sure how to save controller info but will neeed to later on in other plugs

	TSTR name(fname),p,f,e;
	SplitFilename(name,&p, &f, &e);


	TSTR scriptName;
	for (int i = 0; i < name.Length();i++)
		{
		TSTR appendName(scriptName);
		if (appendName.Length() > 0)
			scriptName.printf("%s%c",appendName,fname[i]);
		else scriptName.printf("%c",fname[i]);
		if (fname[i] == '\\') 
			{
			TSTR appendName2(scriptName);
			if (appendName2.Length() > 0)
				scriptName.printf("%s\\",appendName2);
			else scriptName.printf("\\");

			}
		}

	if (_tcscmp(e,".envASCII") == 0)
		{

		macroRecorder->FunctionCall(_T("skinOps.SaveEnvelopeAsASCII"), 2,0, mr_reftarg, this,mr_string,scriptName );
		SaveEnvelope(fname,TRUE);
		}
	else
		{
		macroRecorder->FunctionCall(_T("skinOps.SaveEnvelope"), 2,0, mr_reftarg, this,mr_string,scriptName );
		SaveEnvelope(fname,FALSE);
		}

	}

}
void BonesDefMod::SaveEnvelope(TCHAR *fname, BOOL asText)
{
FILE *file = NULL;
	if (asText)
		file = fopen(fname,_T("wt"));
	else file = fopen(fname,_T("wb"));

//ver
	int ver = 3;
	if (asText)
		fprintf(file,"ver %d\n",ver);
	else fwrite(&ver, sizeof(ver), 1,file);

//number bones
	int ct = 0;
	for (int i =0; i < BoneData.Count(); i++)
		{
//bone name length
		if (BoneData[i].Node != NULL) ct++;
		}

	if (asText)
		fprintf(file,"numberBones %d\n",ct);		
	else fwrite(&ct, sizeof(ct), 1,file);

	for (i =0; i < BoneData.Count(); i++)
		{
		if (BoneData[i].Node != NULL)
			{
			Class_ID bid(BONE_CLASS_ID,0);
			ObjectState os = BoneData[i].Node->EvalWorldState(RefFrame);
			TCHAR title[255];
			if (( os.obj->ClassID() == bid) && (BoneData[i].name.Length()) )
				{
				_tcscpy(title,BoneData[i].name);
				}
			else _tcscpy(title,BoneData[i].Node->GetName());

			int fnameLength = _tcslen(title)+1;

			if (asText)
				fprintf(file,"[boneName] %s\n",title);
			else
				{
//bone name length size
				fwrite(&fnameLength, sizeof(fnameLength), 1,file);
//bone name
				fwrite(title, sizeof(TCHAR)*fnameLength, 1,file);
				}
			if (asText)
				fprintf(file,"[boneID] %d\n",i);
			else
				fwrite(&i, sizeof(i), 1,file);
			
//flags
			if (asText)
				{
				fprintf(file,"  boneFlagLock %d\n",BoneData[i].flags&BONE_LOCK_FLAG);
				fprintf(file,"  boneFlagAbsolute %d\n",BoneData[i].flags&BONE_ABSOLUTE_FLAG);
				fprintf(file,"  boneFlagSpline %d\n",BoneData[i].flags&BONE_SPLINE_FLAG);
				fprintf(file,"  boneFlagSplineClosed %d\n",BoneData[i].flags&BONE_SPLINECLOSED_FLAG);
				fprintf(file,"  boneFlagDrawEnveloe %d\n",BoneData[i].flags&BONE_DRAW_ENVELOPE_FLAG);
				fprintf(file,"  boneFlagIsOldBone %d\n",BoneData[i].flags&BONE_BONE_FLAG);
				fprintf(file,"  boneFlagDead %d\n",BoneData[i].flags&BONE_DEAD_FLAG);
				}
			else fwrite(&BoneData[i].flags, sizeof(BoneData[i].flags), 1,file);
//FalloffType;
			if (asText)
				{
				fprintf(file,"  boneFalloff %d\n",BoneData[i].FalloffType);
				}
			else fwrite(&BoneData[i].FalloffType, sizeof(BoneData[i].FalloffType), 1,file);
//start point
			Point3 p;
			Interval iv;
			BoneData[i].EndPoint1Control->GetValue(currentTime,&p,iv,CTRL_ABSOLUTE);
			if (asText)
				{
				fprintf(file,"  boneStartPoint %f %f %f\n",p.x,p.y,p.z);
				}
			else fwrite(&p, sizeof(p), 1,file);
//end point
			BoneData[i].EndPoint2Control->GetValue(currentTime,&p,iv,CTRL_ABSOLUTE);
			if (asText)
				{
				fprintf(file,"  boneEndPoint %f %f %f\n",p.x,p.y,p.z);
				}
			else fwrite(&p, sizeof(p), 1,file);
//number cross sections
			int crossCount = BoneData[i].CrossSectionList.Count();
			if (asText)
				{
				fprintf(file,"  boneCrossSectionCount %d\n",crossCount);
				}
			else fwrite(&crossCount, sizeof(crossCount), 1,file);

			for (int j=0;j<crossCount; j++)
				{
	//inner
	//outer
				float inner, outer,u;
				BoneData[i].CrossSectionList[j].InnerControl->GetValue(currentTime,&inner,iv,CTRL_ABSOLUTE);
				BoneData[i].CrossSectionList[j].OuterControl->GetValue(currentTime,&outer,iv,CTRL_ABSOLUTE);
				u = BoneData[i].CrossSectionList[j].u;

				if (asText)
					{
					fprintf(file,"    boneCrossSectionInner%d %f\n",j,inner);
					fprintf(file,"    boneCrossSectionOuter%d %f\n",j,outer);
					fprintf(file,"    boneCrossSectionU%d %f\n",j,u);
					}
				else
					{
					fwrite(&inner, sizeof(inner), 1,file);
					fwrite(&outer, sizeof(outer), 1,file);
					fwrite(&u, sizeof(u), 1,file);
					}

				}
			}

		}

//put in vertex data here
	if (asText)
		{
		fprintf(file,"[Vertex Data]\n");
		}
	MyEnumProc dep;              
	EnumDependents(&dep);
//number of nodes instanced to this one modifier
	int nodeCount = 0; //dep.Nodes.Count();


	for (i = 0; i < dep.Nodes.Count(); i++)
		{
		BoneModData *bmd = GetBMD(dep.Nodes[i]);
		if (bmd) nodeCount++;
		}


	if (asText)
		fprintf(file,"  nodeCount %d\n",nodeCount);
	else fwrite(&nodeCount, sizeof(nodeCount), 1,file);
	for (i = 0; i < dep.Nodes.Count(); i++)
			{
			BoneModData *bmd = GetBMD(dep.Nodes[i]);
			if (bmd)
				{
	//write node name
				TCHAR title[255];
				ObjectState os = dep.Nodes[i]->EvalWorldState(RefFrame);
				_tcscpy(title,dep.Nodes[i]->GetName());

				int fnameLength = _tcslen(title)+1;

				if (asText)
					fprintf(file,"  [baseNodeName] %s\n",title);
				else
					{
	//bone name length size
					fwrite(&fnameLength, sizeof(fnameLength), 1,file);
	//bone name
					fwrite(title, sizeof(TCHAR)*fnameLength, 1,file);
					}

	//write number of vertices
				int vertexCount = bmd->VertexData.Count();
				if (asText)
					fprintf(file,"    vertexCount %d\n",vertexCount);
				else fwrite(&vertexCount, sizeof(vertexCount), 1,file);

				for (int j = 0; j < vertexCount; j++)
					{
					if (asText)
						fprintf(file,"    [vertex%d]\n",j);			
					int weightCount = bmd->VertexData[j]->d.Count();

	//write flags modified, rigid, rigid handle, unnormalized
					if (asText)
						{
						fprintf(file,"      vertexIsModified %d\n",bmd->VertexData[j]->IsModified());
						fprintf(file,"      vertexIsRigid %d\n",bmd->VertexData[j]->IsRigid());
						fprintf(file,"      vertexIsRigidHandle %d\n",bmd->VertexData[j]->IsRigidHandle());
						fprintf(file,"      vertexIsUnNormalized %d\n",bmd->VertexData[j]->IsUnNormalized());
					} 
					else fwrite(&bmd->VertexData[j]->flags, sizeof(bmd->VertexData[j]->flags), 1,file);
	//local pos
					if (asText)
						{
						fprintf(file,"      vertexLocalPosition %f %f %f\n",bmd->VertexData[j]->LocalPos.x,bmd->VertexData[j]->LocalPos.y,bmd->VertexData[j]->LocalPos.z);
						}
					else fwrite(&bmd->VertexData[j]->LocalPos, sizeof(bmd->VertexData[j]->LocalPos), 1,file);

	//write number weights
					if (asText)
						fprintf(file,"      vertexWeightCount %d\n",weightCount);
					else fwrite(&weightCount, sizeof(weightCount), 1,file);

	//write weights
					if (asText)
						fprintf(file,"      vertexWeight");

					for (int k = 0; k < weightCount; k++)
						{
						int id;
						float weight;
						id = bmd->VertexData[j]->d[k].Bones;
						if (!bmd->VertexData[j]->IsModified())
							weight = bmd->VertexData[j]->d[k].normalizedInfluences;
						else weight = bmd->VertexData[j]->d[k].Influences;

						if ( (weightCount == 0) && (!bmd->VertexData[j]->IsUnNormalized()))
							weight = 1.0f;

						if (asText)
							fprintf(file," %d,%f",id,weight);
						else 
							{
							fwrite(&id, sizeof(id), 1,file);
							fwrite(&weight, sizeof(weight), 1,file);
							}


						}
					if (asText)
						fprintf(file,"\n");


	//write spline data
					if (asText)
						fprintf(file,"      vertexSplineData");

					for (k = 0; k < weightCount; k++)
						{
						
						float u = bmd->VertexData[j]->d[k].u;
						int curve = bmd->VertexData[j]->d[k].SubCurveIds;
						int seg = bmd->VertexData[j]->d[k].SubSegIds;
						Point3 tan = bmd->VertexData[j]->d[k].Tangents;
						Point3 p = bmd->VertexData[j]->d[k].OPoints;
						


						if (asText)
							{
							TSTR test;
							test.printf("%f",u);
							if ( (strcmp("-1.#QNAN0",test) == 0) || (strcmp("1.#QNAN0",test) == 0) )
								u = 0.0f;

							test.printf("%f",p.x);
							if ((strcmp("-1.#QNAN0",test) == 0) || (strcmp("1.#QNAN0",test) == 0) )
								p.x = 0.0f;
							test.printf("%f",p.y);
							if ((strcmp("-1.#QNAN0",test) == 0) ||(strcmp("1.#QNAN0",test) == 0) )
								p.y = 0.0f;
							test.printf("%f",p.z);
							if ((strcmp("-1.#QNAN0",test) == 0) || (strcmp("1.#QNAN0",test) == 0)) 
								p.z = 0.0f;

							test.printf("%f",tan.x);
							if ((strcmp("-1.#QNAN0",test) == 0) || (strcmp("1.#QNAN0",test) == 0) )
								tan.x = 0.0f;
							test.printf("%f",tan.y);
							if ((strcmp("-1.#QNAN0",test) == 0) || (strcmp("1.#QNAN0",test) == 0) )
								tan.y = 0.0f;
							test.printf("%f",tan.z);
							if ((strcmp("-1.#QNAN0",test) == 0) || (strcmp("1.#QNAN0",test) == 0) )
								tan.z = 0.0f;


							fprintf(file," %f %d %d %f %f %f %f %f %f   ",u,curve,seg,p.x,p.y,p.z,tan.x,tan.y,tan.z);
							}
						else 
							{
							fwrite(&u, sizeof(u), 1,file);
							fwrite(&curve, sizeof(curve), 1,file);
							fwrite(&seg, sizeof(seg), 1,file);
							fwrite(&p, sizeof(p), 1,file);
							fwrite(&tan, sizeof(tan), 1,file);
							}


						}
					if (asText)
						fprintf(file,"\n");
					}


				
//write exclusion lists
				int exclusionTotal = bmd->exclusionList.Count();
				if (asText)
					{
					fprintf(file,"  numberOfExclusinList %d\n",exclusionTotal);
					}
				else fwrite(&exclusionTotal, sizeof(exclusionTotal), 1,file);
				
				for (j = 0; j < exclusionTotal; j++)
					{
					int exclusionCount = 0;
					if (asText)
						fprintf(file,"  [exclusionListForBone %d]\n",j);
					if (bmd->exclusionList[j]) exclusionCount = bmd->exclusionList[j]->Count();
					if (asText)
						{
						fprintf(file,"    exclusionVertexCount %d\n",exclusionCount);
						}
					else fwrite(&exclusionCount, sizeof(exclusionCount), 1,file);
					if (exclusionCount > 0)
						{
						if (asText)
							fprintf(file,"    excludedVerts ");

						for (int k = 0; k < exclusionCount; k++)
							{
							int id = bmd->exclusionList[j]->Vertex(k);
							if (asText)
								fprintf(file," %d",id);
							else fwrite(&id, sizeof(id), 1,file);
							}

						if (asText)
							fprintf(file,"\n");
						}
					}
				}
			}
fclose(file);
}



void BonesDefMod::LoadEnvelopeDialog(BOOL defaultToBinary)
{
static TCHAR fname[256] = {'\0'};
OPENFILENAME ofn;
memset(&ofn,0,sizeof(ofn));
FilterList fl;
fl.Append( GetString(IDS_PW_ENVFILES));
fl.Append( _T("*.env"));		
fl.Append( GetString(IDS_PW_ENVTEXTFILES));
fl.Append( _T("*.envASCII"));		

TSTR title = GetString(IDS_PW_LOADENVELOPES);

ofn.lStructSize     = sizeof(OPENFILENAME);
ofn.hwndOwner       = GetCOREInterface()->GetMAXHWnd();
ofn.lpstrFilter     = fl;
ofn.lpstrFile       = fname;
ofn.nMaxFile        = 256;    
//ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
if (defaultToBinary)
	ofn.lpstrDefExt     = _T("env");
else ofn.lpstrDefExt     = _T("envASCII");
ofn.lpstrTitle      = title;

if (GetOpenFileName(&ofn)) {
//load stuff here
	TSTR name(fname),p,f,e;
	SplitFilename(name,&p, &f, &e);

	TSTR scriptName;
	for (int i = 0; i < name.Length();i++)
		{
		TSTR appendName(scriptName);
		if (appendName.Length() > 0)
			scriptName.printf("%s%c",appendName,fname[i]);
		else scriptName.printf("%c",fname[i]);
		if (fname[i] == '\\') 
			{
			TSTR appendName2(scriptName);
			if (appendName2.Length() > 0)
				scriptName.printf("%s\\",appendName2);
			else scriptName.printf("\\");

			}
		}

	if (_tcscmp(e,".envASCII")==0)
		{
		macroRecorder->FunctionCall(_T("skinOps.LoadEnvelopeAsASCII"), 2,0, mr_reftarg, this,mr_string,scriptName );
		LoadEnvelope(fname,TRUE);
		}
	else
		{
		macroRecorder->FunctionCall(_T("skinOps.LoadEnvelope"), 2,0, mr_reftarg, this,mr_string,scriptName );
		LoadEnvelope(fname);
		}
	}

}



void* BonesDefMod::GetInterface(ULONG id)
{
	switch(id)
	{
		case I_SKIN : return (ISkin *) this;
			break;
		case I_SKINIMPORTDATA : return (ISkinImportData *) this;
			break;
		case PAINTERCANVASINTERFACE_V5 : return (IPainterCanvasInterface_V5 *) this;
			break;
		case PAINTERCANVASINTERFACE_V5_1 : return (IPainterCanvasInterface_V5_1 *) this;
			break;
		case I_SKIN2 : return (ISkin2 *) this;
			break;			
		default: return Modifier::GetInterface(id);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// ISkin Interface :
///////////////////////////////////////////////////////////////////////////////////////////////////////

int BonesDefMod::GetBoneInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset)
{
	UpdateBoneMap();

	for(int i = 0 ; i < GetNumBones() ; i++)
	{
		if(pNode == BoneData[BoneMap[i]].Node)
		{
			if(bObjOffset)
				InitTM = BoneData[BoneMap[i]].InitObjectTM;
			else
				InitTM = BoneData[BoneMap[i]].InitNodeTM;
			return SKIN_OK;
		}
	}
	return SKIN_INVALID_NODE_PTR;
}

int BonesDefMod::GetSkinInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset)
{
	BoneModData *bmd = GetBMD(pNode);
	
	if(!bmd) return SKIN_INVALID_NODE_PTR;

	if(bmd->recompInitMeshTM)
	{
		Matrix3 diffTM(TRUE);
//5.1.03
		Matrix3 stretchTM(TRUE);
		stretchTM = pNode->GetStretchTM(RefFrame);
		diffTM =  pNode->GetNodeTM(RefFrame) * Inverse(pNode->GetObjectTM(RefFrame));
		
		bmd->BaseNodeTM = diffTM * bmd->BaseTM;

		bmd->recompInitMeshTM = false;
	}
	
	
	if(bObjOffset)
		InitTM = bmd->BaseTM;
	else
		InitTM = bmd->BaseNodeTM;

	return SKIN_OK;
}

int BonesDefMod::GetNumBones()
{
	UpdateBoneMap();
	return BoneMap.Count();
}

INode *BonesDefMod::GetBone(int idx)
{
	UpdateBoneMap();
	return BoneData[BoneMap[idx]].Node;
}

void BonesDefMod::UpdateBoneMap()
{
	int cnt = 0;

	if(recompBoneMap)
	{
		BoneMap.SetCount(BoneData.Count());
		
		
		for( int i = 0 ; i < BoneData.Count() ; i++ )
		{
			if(BoneData[i].Node != NULL)
			{
				BoneMap[cnt++] = i;
			}
		}
		BoneMap.SetCount(cnt);
		recompBoneMap = false;
	}
}

DWORD BonesDefMod::GetBoneProperty(int idx)
{
	return BoneData[idx].flags;
}

int BonesDefMod::GetSelectedBone()
{
	return ModeBoneIndex;
}


void BonesDefMod::GetEndPoints(int id, Point3 &l1, Point3 &l2)
{
if ((id <0) || (id >= BoneData.Count())) return;
BoneData[id].EndPoint1Control->GetValue(currentTime,&l1,FOREVER);
BoneData[id].EndPoint2Control->GetValue(currentTime,&l2,FOREVER);
}

Matrix3 BonesDefMod::GetBoneTm(int id)
{
if ((id <0) || (id >= BoneData.Count())) return Matrix3(1);
return BoneData[id].tm;
}

INode *BonesDefMod::GetBoneFlat(int idx)
{
	if ((idx <0) || (idx >= BoneData.Count())) return NULL;
	return BoneData[idx].Node;
}

int BonesDefMod::GetNumBonesFlat()
{
	return BoneData.Count();
}

int BonesDefMod::GetRefFrame()
{
	return RefFrame;
}

ISkinContextData *BonesDefMod::GetContextInterface(INode *pNode) 
{ 
	BoneModData *bmd = GetBMD(pNode);
	if(!bmd) 
		return NULL;
	else
		return (ISkinContextData*) bmd;
}


BoneModData *BonesDefMod::GetBMD(INode *pNode)
{
	ModContext *mc = NULL;

	Object* pObj = pNode->GetObjectRef();

	if (!pObj) return NULL;

	
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID && mc == NULL)
	{
		IDerivedObject* pDerObj = (IDerivedObject *)(pObj);
			
		int Idx = 0;

		while (Idx < pDerObj->NumModifiers())
		{
			// Get the modifier. 
			Modifier* mod = pDerObj->GetModifier(Idx);

			
			if (mod->ClassID() == SKIN_CLASSID)
			{
				// is this the correct Physique Modifier based on index?
				BonesDefMod *skin = (BonesDefMod*)mod;
				if (skin == this)
					{
					mc = pDerObj->GetModContext(Idx);
					break;
					}
			}

			Idx++;
		}

		pObj = pDerObj->GetObjRef();
	}

	if(!mc) return NULL;

	if ( !mc->localData ) return NULL;

	BoneModData *bmd = (BoneModData *) mc->localData;

	return bmd;

}

int BonesDefMod::NumSubObjTypes() 
{ 
	return 1;
}

ISubObjType *BonesDefMod::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_DefPoints.SetName(GetString(IDS_RB_BONESDEFPOINTS));
	}

	switch(i)
	{
	case 0:
		return &SOT_DefPoints;
	}
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// LocalModData Implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////

void BoneModData::InitSkinEngine()
{
	CoInitialize(NULL);

	pSE = NULL;
	
	// This is two steps in one, CreateInstance IUnknown + QureyInterface IFlexEngine
	HRESULT hr = CoCreateInstance( CLSID_SkinEngine, NULL, CLSCTX_INPROC_SERVER,IID_ISkinEngine,(void **)&pSE);

	if( FAILED(hr))
	{
//if failed try to register the dll
		TCHAR *path = GetCOREInterface()->GetDir(APP_MAXROOT_DIR);
		ShellExecute(GetCOREInterface()->GetMAXHWnd(), "open", "regsvr32.exe", "/s atl.dll", NULL, SW_SHOWNORMAL);
		Sleep(3000);
		ShellExecute(GetCOREInterface()->GetMAXHWnd(), "open", "regsvr32.exe", "/s MAXComponents.dll", path, SW_SHOWNORMAL);
		Sleep(3000);
		hr = CoCreateInstance( CLSID_SkinEngine, NULL, CLSCTX_INPROC_SERVER,IID_ISkinEngine,(void **)&pSE);

//if failed again something bad has happened and bail
		if( FAILED(hr))
			{	
			MessageBox(GetCOREInterface()->GetMAXHWnd(),"CoCreateInstance() failed\nPlease check your registry entries\nCLSID {F088EA74-2E87-11D3-B1F3-00C0F03C37D3} and make sure you are logged in as an administrator","COM Error",MB_OK);
			CoUninitialize();
			return;
			}
	}

	// Register the callback
	if (!pSkinCallback) 
	{
		
		pSkinCallback	= new CSkinCallback(this);//CComObject<CFlexEngineEvents>;
		HRESULT hr	= AtlAdvise(pSE,(IUnknown *)pSkinCallback,IID__ISkinEngineEvents,&cookie);
		if (FAILED(hr)) 
		{
			pSkinCallback = 0;
		}		
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// ISkinContextData Interface
///////////////////////////////////////////////////////////////////////////////////////////////////////

int BoneModData::GetNumPoints()
{
	return VertexData.Count();
}
		
int BoneModData::GetNumAssignedBones(int pointIdx) 
{
	return VertexData[pointIdx]->d.Count();
}

int BoneModData::GetAssignedBone(int pointIdx, int boneIdx) 
{
	
	mod->UpdateBoneMap();

	for(int i = 0 ; i < mod->BoneMap.Count() ; i++)
	{
		if(mod->BoneMap[i] == VertexData[pointIdx]->d[boneIdx].Bones)
			return i;
	}
	return -1;
}

float BoneModData::GetBoneWeight(int pointIdx, int boneIdx) 
{
	return VertexData[pointIdx]->d[boneIdx].normalizedInfluences;
}
    
int BoneModData::GetSubCurveIndex(int pointIdx, int boneIdx)
{
	return VertexData[pointIdx]->d[boneIdx].SubCurveIds;	
}

int BoneModData::GetSubSegmentIndex(int pointIdx, int boneIdx)
{
	return VertexData[pointIdx]->d[boneIdx].SubSegIds;;
}

float BoneModData::GetSubSegmentDistance(int pointIdx, int boneIdx)
{
	return VertexData[pointIdx]->d[boneIdx].u;
}

Point3 BoneModData::GetTangent(int pointIdx, int boneIdx)
{
	return VertexData[pointIdx]->d[boneIdx].Tangents;
}

Point3 BoneModData::GetOPoint(int pointIdx, int boneIdx)
{
	return VertexData[pointIdx]->d[boneIdx].OPoints;
}


BOOL ExclusionListClass::isInList(int vertID, int &where)
{
	for (int i = 0; i < exList.Count(); i++)
		{
		if (exList[i] == vertID) 
			{
			where= i;
			return TRUE;
			}

		}
	return FALSE;
}

BOOL ExclusionListClass::isExcluded(int vertID)
{
	for (int i = 0; i < exList.Count(); i++)
		{
		if (exList[i] == vertID) return TRUE;
		}
	return FALSE;
}

void ExclusionListClass::ExcludeVerts(Tab<int> exclusionList)
{
	for (int i = 0; i < exclusionList.Count(); i++)
		{
		int vertID = exclusionList[i];
		int where;
		if (!isInList(vertID,where))
			{
			exList.Append(1,&vertID);
			}
		}
}

void ExclusionListClass::SetExclusionVerts(Tab<int> exclusionList)
{
	exList.ZeroCount();
	exList = exclusionList;
}


void ExclusionListClass::IncludeVerts(Tab<int> inclusionList)
{
	for (int i = 0; i < inclusionList.Count(); i++)
		{
		int vertID = inclusionList[i];
		int where;
		if (isInList(vertID,where))
			{
			exList.Delete(where,1);
			}
		}
}

IOResult ExclusionListClass::Save(ISave *isave)
{
unsigned long nb;
int c = exList.Count();
IOResult ior;
ior = isave->Write(&c,sizeof(c),&nb);
for (int i = 0; i < exList.Count(); i++)
	{
	c = exList[i];
	ior = isave->Write(&c,sizeof(c),&nb);
	}
return ior;
}

IOResult ExclusionListClass::Load(ILoad *iload)
{
unsigned long nb;
int c;
IOResult ior;
ior = iload->Read(&c,sizeof(c),&nb);
exList.SetCount(c);
for (int i = 0; i < exList.Count(); i++)
	{
	int id;
	ior = iload->Read(&id,sizeof(id),&nb);
	exList[i] = id;
	}


return ior ;
}

BOOL BoneModData::isExcluded(int BoneID, int vertID)
{
if (exclusionList[BoneID])
	{
	return exclusionList[BoneID]->isExcluded(vertID);
	}
return FALSE;
}

void BoneModData::ExcludeVerts(int boneID, Tab<int> exList, BOOL cleanUpVerts)
{
if (boneID>=exclusionList.Count())
	{
	int oldCount = exclusionList.Count();
	exclusionList.SetCount(boneID+1);
	for (int i = oldCount; i < exclusionList.Count(); i++)
		exclusionList[i] = NULL;
	}
if (!exclusionList[boneID])
	{
	exclusionList[boneID] = new ExclusionListClass();
	exclusionList[boneID]->ExcludeVerts(exList);
	}
else exclusionList[boneID]->ExcludeVerts(exList);

//now loop through all vertices on this bone remove them if they are excluded
if (cleanUpVerts)
	{
	for (int i =0; i < VertexData.Count(); i++)
		{
		for (int  j = VertexData[i]->d.Count()-1; j >= 0; j--)
			{
			if ( isExcluded(boneID, i) && (VertexData[i]->d[j].Bones == boneID))
				VertexData[i]->d.Delete(j,1);
			}
		}
	}


}
void BoneModData::CleanUpExclusionLists()
	{
	for (int m=0; m < exclusionList.Count(); m++)
		{
		if (exclusionList[m])
			{
			for (int i =0; i < VertexData.Count(); i++)
				{
				for (int  j = VertexData[i]->d.Count()-1; j >= 0; j--)
					{
					if ( isExcluded(m, i) && (VertexData[i]->d[j].Bones == m))
						VertexData[i]->d.Delete(j,1);
					}
				}
			}
		}
	}

void BoneModData::IncludeVerts(int boneID, Tab<int> inList)
{

if (boneID>=exclusionList.Count())
	return;
else
	{
	if (exclusionList[boneID])
		{
		exclusionList[boneID]->IncludeVerts(inList);

		}
	}


}

void BoneModData::SelectExcludedVerts(int boneID)
{
if (boneID>=exclusionList.Count())
	return;
else
	{
	if (exclusionList[boneID])
		{
		selected.ClearAll();
		for (int i = 0; i < VertexData.Count(); i++)
			selected.Set (i,FALSE);

		for (i = 0; i < exclusionList[boneID]->Count(); i++)
			{
			int id = exclusionList[boneID]->Vertex(i);
			selected.Set(id,1);
//			VertexData[id]->Selected (TRUE);

			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Deformer Implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////


Point3 BonesDefDeformer::Map(int i, Point3 p) 
{	

	Point3 pout(0,0,0);

	Point3 initialP = p;

	if (bmd->isPatch)
		{
		if (bmd->autoInteriorVerts[i]) return p;
		}	

	for( int j = 0 ; j < bmd->VertexData[i]->d.Count() ; j++)
	{
		// This sets the normalizedInfluences values
		Cluster->RetrieveNormalizedWeight(bmd,i,j);
	}
	if (bmd->pSE)
		{
		if	( (Cluster->fastUpdate && (!Cluster->inRender) && (bmd->VertexData[i]->d.Count() > 0)) ||
		  ((Cluster->rigidVerts) && (bmd->VertexData[i]->d.Count() > 0))
			)
			{
			int boneID = bmd->VertexData[i]->GetMostAffectedBoneID();
			float val = 1.0f;

	
			bmd->pSE->SetPointData(i, 1, 
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[boneID].Bones, 
			sizeof(VertexInfluenceListClass), &val,
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[boneID].SubCurveIds,
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[boneID].SubSegIds,
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[boneID].u,
			sizeof(VertexInfluenceListClass), (float *) &bmd->VertexData[i]->d[boneID].Tangents,
			sizeof(VertexInfluenceListClass), (float *) &bmd->VertexData[i]->d[boneID].OPoints);

			}
		else if(bmd->VertexData[i]->d.Count() > 0)
			bmd->pSE->SetPointData(i, bmd->VertexData[i]->d.Count(), 
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[0].Bones, 
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[0].normalizedInfluences,
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[0].SubCurveIds,
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[0].SubSegIds,
			sizeof(VertexInfluenceListClass), &bmd->VertexData[i]->d[0].u,
			sizeof(VertexInfluenceListClass), (float *) &bmd->VertexData[i]->d[0].Tangents,
			sizeof(VertexInfluenceListClass), (float *) &bmd->VertexData[i]->d[0].OPoints);
		else
			bmd->pSE->SetPointData(i, bmd->VertexData[i]->d.Count(), 
			sizeof(VertexInfluenceListClass), NULL, 
			sizeof(VertexInfluenceListClass), NULL,
			sizeof(VertexInfluenceListClass), NULL,
			sizeof(VertexInfluenceListClass), NULL,
			sizeof(VertexInfluenceListClass), NULL,
			sizeof(VertexInfluenceListClass), NULL,
			sizeof(VertexInfluenceListClass), NULL);


		bmd->pSE->MapPoint(i,p,pout);
		}	
	else
		pout = Map2(i,p);

	if	( (!Cluster->fastUpdate) || (Cluster->inRender) )
		{
		for( j = 0 ; j < bmd->gizmoData.Count() ; j++)
			{
			int id = bmd->gizmoData[j]->whichGizmo;
//			int gizmoCount = Cluster->pblock_gizmos->Count(skin_gizmos_list);
//			ReferenceTarget *ref;
			if (id < Cluster->gizmoDefListCount/*gizmoCount*/)
				{
//				ref = Cluster->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,id);
				GizmoClass *gizmo = Cluster->gizmoDefList[id];//(GizmoClass *)ref;
				if ((gizmo) && (gizmo->IsEnabled()))
					{
					if  (gizmo->IsVolumeBased() && gizmo->IsInVolume(initialP,bmd->BaseTM))
						{
						if (gizmo)
							{
							if (Cluster->backTransform) pout = pout * bmd->gizmoPutBackDoubleOffset; 						
							pout = gizmo->DeformPoint(t,i,initialP,pout,bmd->BaseTM);
							if (Cluster->backTransform) pout = pout * bmd->gizmoRemoveDoubleOffset; 

							}
			
						}
					else 
						{
						BOOL inList = FALSE;
					 	if (Cluster->fastGizmo)
							inList = bmd->gizmoData[j]->IsInDeformingList(i);
						else inList = bmd->gizmoData[j]->IsAffected(i);

						if (!gizmo->IsVolumeBased() && inList) 
							{
							if (gizmo)
								{
								if (Cluster->backTransform) pout = pout * bmd->gizmoPutBackDoubleOffset; 						
								pout = gizmo->DeformPoint(t,i,initialP,pout,bmd->BaseTM);
								if (Cluster->backTransform) pout = pout * bmd->gizmoRemoveDoubleOffset; 
								}
							}
						}
					}
				}
			}
		}


	return pout;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Skin Callback implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////

ULONG CSkinCallback::AddRef()
{
	return ++m_cRef;
}

ULONG CSkinCallback::Release()
{
	if(--m_cRef != 0)
		return m_cRef;
	delete this;
	return 0;
}

HRESULT CSkinCallback::QueryInterface(REFIID riid, void** ppv)
{
	if(riid == IID_IUnknown)
	{
		*ppv = (IUnknown*)this;
	}
	else if(riid == IID__ISkinEngineEvents)
	{
		*ppv = (_ISkinEngineEvents*)this;
	}
	else 
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CSkinCallback::GetInterpCurvePiece3D( int BoneId,int CurveId,int SegId,float distance,float __RPC_FAR *pPoint)
{
	Point3 *pp = (Point3 *) pPoint;
	ObjectState os = bmd->mod->BoneData[BoneId].Node->EvalWorldState(bmd->CurrentTime);
	ShapeObject *pathOb = (ShapeObject*)os.obj;
 
	if (pathOb->NumberOfVertices(bmd->CurrentTime) > 0)
		{
		if ( (bmd->mod->splineList[BoneId]) && (SegId < bmd->mod->splineList[BoneId]->Segments()))
			{
			*pp = bmd->mod->splineList[BoneId]->InterpBezier3D(SegId, distance);
			}
		else *pp = pathOb->InterpPiece3D(bmd->CurrentTime, CurveId, SegId , distance );
		return S_OK;
		}
	else return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CSkinCallback::GetTangentPiece3D(int BoneId,int CurveId,int SegId,float distance,float __RPC_FAR *pPoint)
{
	Point3 *pp = (Point3 *) pPoint;
	ObjectState os = bmd->mod->BoneData[BoneId].Node->EvalWorldState(bmd->CurrentTime);
	ShapeObject *pathOb = (ShapeObject*)os.obj;
	if (pathOb->NumberOfVertices(bmd->CurrentTime) > 0)
		{
		if ((bmd->mod->splineList[BoneId])  && (SegId < bmd->mod->splineList[BoneId]->Segments()))
			{
			*pp = bmd->mod->splineList[BoneId]->TangentBezier3D(SegId, distance);
			}
		else *pp = pathOb->TangentPiece3D(bmd->CurrentTime, CurveId, SegId , distance );
		return S_OK;
		}
	else return S_FALSE;

}



//--- Named selection sets -----------------------------------------

int BonesDefMod::FindSelSetIndex(int index) 
{
	int ct = 0;
	for (int i =0; i < namedSel.Count(); i++)
		{
		if (namedSel[i])
			{
			if (ct == index) return ct;
			ct++;
			}

		}
	return 0;
}
int BonesDefMod::FindSet(TSTR &setName) 
{
	for (int i=0; i<namedSel.Count(); i++) 
		{
		if ((namedSel[i]) && (setName == *namedSel[i])) return i;
		}
	return -1;
}

DWORD BonesDefMod::AddSet(TSTR &setName) 
{
	DWORD id = 0;
	TSTR *name = new TSTR(setName);
	for (int i = 0; i < namedSel.Count(); i ++)
		{
		if (namedSel[i] == NULL)
			{
			namedSel[i] = name;
			return i;
			}
		}
	namedSel.Append(1,&name);
	return namedSel.Count()-1;
}

void BonesDefMod::RemoveSet(TSTR &setName) 
{
	int i = FindSet(setName);
	if (i<0) return;
	delete namedSel[i];
	namedSel[i] = NULL;
}

void BonesDefMod::ClearSetNames() 
{
	for (int j=0; j<namedSel.Count(); j++) 
		{
		delete namedSel[j];
		namedSel[j] = NULL;
		}
}

void BonesDefMod::ActivateSubSelSet(TSTR &setName) 
{
	ModContextList mcList;
	INodeTab nodes;
	int index = FindSet (setName);	
	if (index<0 || !ip) return;

	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) 
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (!bmd) continue;

		bmd->selected.ClearAll();

		BitArray *list = bmd->namedSelSets[index];
		if (list)
			{
			for (int j = 0; j < list->GetSize(); j++)
				{
				if ( (*list)[j] )
					{
					if ((j <0) || (j >= bmd->selected.GetSize()))
						{
						}
					else bmd->selected.Set(j,TRUE);
					}
				}
			}

		GetCOREInterface()->NodeInvalidateRect(nodes[0]);
		}
	if  (ip) ip->RedrawViews(ip->GetTime());

//	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//	ip->RedrawViews(ip->GetTime());
}


void BonesDefMod::NewSetFromCurSel(TSTR &setName) {
	ModContextList mcList;
	INodeTab nodes;
	DWORD id = -1;
	int index = FindSet(setName);
	if (index<0) id = AddSet(setName);
	else id = index;

	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) 
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (!bmd) continue;

		BitArray *set = new BitArray();

		int s = bmd->selected.GetSize();
		set->SetSize(s);
		*set = bmd->selected;

		int ct = bmd->namedSelSets.Count();
		if (id >= ct)
			{		
			bmd->namedSelSets.SetCount(id+1);
			for (int j = ct; j < bmd->namedSelSets.Count(); j++)
				bmd->namedSelSets[j] = NULL;

			}

		if (bmd->namedSelSets[id] != NULL)
			delete bmd->namedSelSets[id];
		bmd->namedSelSets[id] = set;
		}
		
}

void BonesDefMod::RemoveSubSelSet(TSTR &setName) {
	int index = FindSet (setName);
	if (index<0 || !ip) return;		

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);


	for (int i = 0; i < mcList.Count(); i++) 
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (!bmd) continue;

		if ( (index >= 0) && (index < bmd->namedSelSets.Count()))
			{
			if (bmd->namedSelSets[index])
				{
				delete bmd->namedSelSets[index];
				bmd->namedSelSets[index] = NULL;
				}
			}
		}
	
	RemoveSet(setName);
	nodes.DisposeTemporary();
}

void BonesDefMod::SetupNamedSelDropDown() {


	ip->ClearSubObjectNamedSelSets();
	for (int i=0; i<namedSel.Count(); i++)
		{
		if (namedSel[i]) ip->AppendSubObjectNamedSelSet(*namedSel[i]);
		}
}

int BonesDefMod::NumNamedSelSets() {
	int ct = 0;
	for (int i =0; i < namedSel.Count(); i++)
		{
		if (namedSel[i])
			{
			ct++;
			}

		}
	return ct;
}

TSTR BonesDefMod::GetNamedSelSetName(int i) {
	int index = 0;
	index = FindSelSetIndex(i);
	return *namedSel[index];
}


void BonesDefMod::SetNamedSelSetName(int i,TSTR &newName) {
	int index = 0;
	index = FindSelSetIndex(i);

	*namedSel[index] = newName;
}


void BonesDefMod::NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) {
	ModContextList mcList;
	INodeTab nodes;
	

	DWORD id = -1;
	int index = FindSet(newName);
	if (index<0) id = AddSet(newName);
	else id = index;


	BOOL delSet = TRUE;
	ip->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) {

		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (!bmd) continue;

		BitArray *bits = new BitArray();

		int index = FindSelSetIndex(sets[0]);
		*bits = *bmd->namedSelSets[index];

		for (int i=1; i<sets.Count(); i++) {
			index = FindSelSetIndex(sets[i]);
			if (bmd->namedSelSets[index])
				{
				switch (op) {
				case NEWSET_MERGE:
					*bits |= *bmd->namedSelSets[index];
					break;

				case NEWSET_INTERSECTION:
					*bits &= *bmd->namedSelSets[index];
					break;

				case NEWSET_SUBTRACT:
					*bits &= ~(*bmd->namedSelSets[index]);
					break;
				}
			}

		int ct = bmd->namedSelSets.Count();
		if (id >= ct)
			{		
			bmd->namedSelSets.SetCount(id+1);
			for (int j = ct; j < bmd->namedSelSets.Count(); j++)
				bmd->namedSelSets[j] = NULL;

			}

		if (bmd->namedSelSets[id] != NULL)
			delete bmd->namedSelSets[id];
		bmd->namedSelSets[id] = bits;

		}
		if (bits->NumberSet()) delSet = FALSE;

		
	}
	if (delSet) RemoveSubSelSet(newName);
}



void BonesDefMod::NotifyPreDeleteNode(void* parm, NotifyInfo* arg)
	{
	BonesDefMod* mod = (BonesDefMod*)parm;
	if(mod == NULL) return;
	mod->bindNode = NULL;

}

void BonesDefMod::NotifyPreSave(void* parm, NotifyInfo* arg)
	{
	
	BonesDefMod* mod = (BonesDefMod*)parm;
	if(mod == NULL) return;
//put back to normal

	if ((mod->ip)  && (mod->ip->GetSubObjectLevel() == 1) &&  (mod->shadeVC) )
		{
		mod->RestoreVCMode();
		}


	}

void BonesDefMod::NotifyPostSave(void* parm, NotifyInfo* arg)
	{
	BonesDefMod* mod = (BonesDefMod*)parm;
	if(mod == NULL) return;
//put back vc

	if ((mod->ip) &&  (mod->shadeVC) && (mod->ip->GetSubObjectLevel() == 1) )
		{
		mod->SetVCMode();
		}


	}



BOOL BonesDefMod::AssignController(Animatable *control,int subAnim)
	{
	int refID = -1;
	refID = SubNumToRefNum(subAnim);

	if (refID >= 0)
		{
		ReplaceReference(refID,(Control*)control);
		NotifyDependents(FOREVER,PART_GEOM,REFMSG_CHANGE);
		NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
		return TRUE;
		}
	else return FALSE;
	}

void BonesDefMod::GetVertexSelection(INode *skinNode, BitArray &sel)
{
	BoneModData *bmd = GetBMD(skinNode);
	if (bmd!= NULL)
	{
		sel.SetSize(bmd->selected.GetSize());
		sel = bmd->selected;
	}
	
}

void BonesDefMod::SetVertexSelection(INode *skinNode, BitArray &sel)
{
	BoneModData *bmd = GetBMD(skinNode);
	if ((bmd!= NULL) && (sel.GetSize() == bmd->selected.GetSize()))
	{
		bmd->selected = sel;
		GetCOREInterface()->NodeInvalidateRect(skinNode);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());		
	}
}



void BonesDefMod::UnlockVerts()
{
	ModContextList mcList;
	INodeTab nodes;
	

	GetCOREInterface()->GetModContexts(mcList,nodes);
	for (int i = 0; i < mcList.Count(); i++) 
	{
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;
		if (bmd) bmd->unlockVerts = TRUE;
	}


}