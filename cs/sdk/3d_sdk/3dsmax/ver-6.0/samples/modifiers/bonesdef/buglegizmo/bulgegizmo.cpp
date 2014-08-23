 /**********************************************************************
 
	FILE: BonesDef.cpp

	DESCRIPTION:  Simple Bones Deformation Plugin

	CREATED BY: Peter Watje

	HISTORY: 8/5/98


 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#include "max.h"
#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "surf_api.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"
#include "macrorec.h"
#include "modstack.h"
#include "ISkin.h"
#include "MaxIcon.h"


#include "..\BonesDefComponents\MAXComponents_i.c"

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
CreatePaintMode*        BonesDefMod::PaintMode   = NULL;
ICustButton* BonesDefMod::iCrossSectionButton   = NULL;
ICustButton* BonesDefMod::iLock   = NULL;
ICustButton* BonesDefMod::iAbsolute   = NULL;
ICustButton* BonesDefMod::iEnvelope   = NULL;
ICustButton* BonesDefMod::iFalloff   = NULL;
ICustButton* BonesDefMod::iCopy   = NULL;
ICustButton* BonesDefMod::iPaste   = NULL;
ICustButton* BonesDefMod::iPaintButton  = NULL;
ICustToolbar* BonesDefMod::iParams = NULL;

static IconResourceHandler theIconResHandler(IDB_SUBOBJTYPES,IDB_MASK_SUBOBJTYPES, 1);	
static GenSubObjType SOT_DefPoints(NULL,&theIconResHandler,0);

//--- ClassDescriptor and class vars ---------------------------------

//IParamMap       *BonesDefMod::pmapParam = NULL;
IObjParam       *BonesDefMod::ip        = NULL;
BonesDefMod     *BonesDefMod::editMod   = NULL;
MoveModBoxCMode *BonesDefMod::moveMode  = NULL;
int			    BonesDefMod::LastSelected = 0;		

class BonesDefClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new BonesDefMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_BONESDEFMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
											   
	Class_ID		ClassID() { return Class_ID(9815843,87654); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("skin"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle



	};

static BonesDefClassDesc bonesDefDesc;
extern ClassDesc* GetBonesDefModDesc() {return &bonesDefDesc;}


BonesRightMenu rMenu;

//watje 10-13-99 212156
BOOL BonesDefMod::DependOnTopology(ModContext &mc) {
	BoneModData *bmd = (BoneModData*)mc.localData;
	BOOL topo = FALSE;
	if (bmd) 
		for ( int i =0; i <bmd->VertexData.Count(); i++)
			{
			if (bmd->VertexData[i]->modified)
				{
				topo = TRUE;
				i = bmd->VertexData.Count();
				}

			}
	return topo;
}


int BonesDefMod::NumRefs() {
	int ct = 1;

	for (int i = 0; i<BoneData.Count();i++)
		{

		if (BoneData[i].RefEndPt1ID > ct) ct = BoneData[i].RefEndPt1ID;
		if (BoneData[i].RefEndPt2ID > ct) ct = BoneData[i].RefEndPt2ID;
		if (BoneData[i].BoneRefID > ct) ct = BoneData[i].BoneRefID;
		for (int j = 0; j<BoneData[i].CrossSectionList.Count();j++)
			{
			if (BoneData[i].CrossSectionList[j].RefInnerID > ct) ct = BoneData[i].CrossSectionList[j].RefInnerID;
			if (BoneData[i].CrossSectionList[j].RefOuterID > ct) ct = BoneData[i].CrossSectionList[j].RefOuterID;
//			ct += BoneData[i].CrossSectionList.Count()*2;
			}
		}
	return (ct+1);
	}



//watje 9-7-99  198721 
class ReevalModEnumProc : public ModContextEnumProc {
public:
	BonesDefMod *lm;
	BOOL ev;
	ReevalModEnumProc(BonesDefMod *l, BOOL e)
		{
		lm = l;
		ev = e;
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
ReevalModEnumProc lmdproc(this,eval);
EnumModContexts(&lmdproc);

}

void BonesDefMod::CopyBone()
{

if ((ModeBoneIndex != -1) && (BoneData.Count() > 0))
	{
//get end point1
	Interval v;
	BoneData[ModeBoneIndex].EndPoint1Control->GetValue(0,&CopyBuffer.E1,v,CTRL_ABSOLUTE);
//get end point2
	BoneData[ModeBoneIndex].EndPoint2Control->GetValue(0,&CopyBuffer.E2,v,CTRL_ABSOLUTE);
//need to set in local space
//	CopyBuffer.E1 = CopyBuffer.E1*BoneData[ModeBoneIndex].tm;
//	CopyBuffer.E2 = CopyBuffer.E2*BoneData[ModeBoneIndex].tm;

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
		BoneData[ModeBoneIndex].CrossSectionList[i].InnerControl->GetValue(0,&c.inner,v);
		BoneData[ModeBoneIndex].CrossSectionList[i].OuterControl->GetValue(0,&c.outer,v);
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

//	BoneData[ModeBoneIndex].EndPoint1Control->SetValue(0,&CopyBuffer.E1,TRUE,CTRL_ABSOLUTE);
//	BoneData[ModeBoneIndex].EndPoint2Control->SetValue(0,&CopyBuffer.E2,TRUE,CTRL_ABSOLUTE);
//	theHold.Suspend();
	for (int i =(ct-1); i >= 0 ; i--)
		RemoveCrossSection(ModeBoneIndex, i);
	for (i =0; i < CopyBuffer.CList.Count() ; i++)
		{
		AddCrossSection(ModeBoneIndex,CopyBuffer.CList[i].u,CopyBuffer.CList[i].inner,CopyBuffer.CList[i].outer);
		}
//	theHold.Resume();

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

//	BoneData[ModeBoneIndex].EndPoint1Control->SetValue(0,&CopyBuffer.E1,TRUE,CTRL_ABSOLUTE);
//	BoneData[ModeBoneIndex].EndPoint2Control->SetValue(0,&CopyBuffer.E2,TRUE,CTRL_ABSOLUTE);
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

//	BoneData[ModeBoneIndex].EndPoint1Control->SetValue(0,&CopyBuffer.E1,TRUE,CTRL_ABSOLUTE);
//	BoneData[ModeBoneIndex].EndPoint2Control->SetValue(0,&CopyBuffer.E2,TRUE,CTRL_ABSOLUTE);
		for (int i =(ct-1); i >= 0 ; i--)
			RemoveCrossSection(k, i);
		for (i =0; i < CopyBuffer.CList.Count() ; i++)
			{
			AddCrossSection(k,CopyBuffer.CList[i].u,CopyBuffer.CList[i].inner,CopyBuffer.CList[i].outer);
			}

		}
	}
}




void BonesDefMod::AddCrossSection(int BoneIndex, float u, float inner, float outer)

{
class CrossSectionClass t;
int index = -1;
t. u = u;
//t.Inner = inner;
//t.Outer = outer;


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

BoneData[BoneIndex].CrossSectionList[index].InnerControl->SetValue(0,&inner,TRUE,CTRL_ABSOLUTE);
BoneData[BoneIndex].CrossSectionList[index].OuterControl->SetValue(0,&outer,TRUE,CTRL_ABSOLUTE);


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
BoneData[ModeBoneIndex].CrossSectionList[lowerbound].InnerControl->GetValue(0,&li,v);
BoneData[ModeBoneIndex].CrossSectionList[lowerbound].OuterControl->GetValue(0,&lo,v);
BoneData[ModeBoneIndex].CrossSectionList[upperbound].InnerControl->GetValue(0,&ui,v);
BoneData[ModeBoneIndex].CrossSectionList[upperbound].OuterControl->GetValue(0,&uo,v);

//li = BoneData[ModeBoneIndex].CrossSectionList[lowerbound].Inner;
//lo = BoneData[ModeBoneIndex].CrossSectionList[lowerbound].Outer;
//ui = BoneData[ModeBoneIndex].CrossSectionList[upperbound].Inner;
//uo = BoneData[ModeBoneIndex].CrossSectionList[upperbound].Outer;
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

BoneData[ModeBoneIndex].CrossSectionList[index].InnerControl->SetValue(0,&Inner,TRUE,CTRL_ABSOLUTE);
BoneData[ModeBoneIndex].CrossSectionList[index].OuterControl->SetValue(0,&Outer,TRUE,CTRL_ABSOLUTE);

if (index <= ModeBoneEnvelopeIndex)
	{
	ModeBoneEnvelopeIndex++;
	if (ModeBoneEnvelopeIndex >= BoneData[ModeBoneIndex].CrossSectionList.Count())
		ModeBoneEnvelopeIndex = BoneData[ModeBoneIndex].CrossSectionList.Count()-1;
	}
/*
int c = sel.Count();
sel.SetCount(c+12);
*/
//append bone to this list.

}


void BonesDefMod::GetCrossSectionRanges(float &inner, float &outer, int BoneID, int CrossID)

{

Interval v;
//if ( (BoneID < BoneData.Count()) && 
//	 (CrossID < BoneData[BoneID].CrossSectionList.Count()) 
  //  )
	{
	BoneData[BoneID].CrossSectionList[CrossID].InnerControl->GetValue(0,&inner,v);
	BoneData[BoneID].CrossSectionList[CrossID].OuterControl->GetValue(0,&outer,v);
	}

}

float BonesDefMod::GetU(ViewExp *vpt,Point3 a, Point3 b, IPoint2 p)
{
//mouse spot
//ModContextList mcList;		
INodeTab nodes;
Point2 fp = Point2((float)p.x, (float)p.y);
float u;
if ( !ip ) return 0.0f;

//ip->GetModContexts(mcList,nodes);
//for ( int i = 0; i < mcList.Count(); i++ ) 
	{
	// Find the location on the segment where the user clicked
//	INode *inode = nodes[i];
	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(Matrix3(1));
	Point2 spa = ProjectPointF(gw, a);
	Point2 spb = ProjectPointF(gw, b);
	u = Length(spa-fp)/Length(spa-spb);
	}

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
/*
	Interval v;
	BoneData[BoneID].EndPoint1Control->GetValue(t,&l1,v);
	BoneData[BoneID].EndPoint2Control->GetValue(t,&l2,v);
	
	l1 = l1 * Inverse(BoneData[BoneID].tm) * Inverse(bmd->BaseTM);
	l2 = l2 * Inverse(BoneData[BoneID].tm) * Inverse(bmd->BaseTM);
*/
	}

}



void BonesDefMod::GetEndPointsLocal(BoneModData *bmd, TimeValue t, Point3 &l1, Point3 &l2, int BoneID)
{

//ObjectState os;
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

//	Interval v;
//	BoneData[BoneID].EndPoint1Control->GetValue(t,&l1,v);
//	BoneData[BoneID].EndPoint2Control->GetValue(t,&l2,v);
	l1 = bmd->tempTableL1ObjectSpace[BoneID];
	l2 = bmd->tempTableL2ObjectSpace[BoneID];
	
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

	int SubCount = pathOb->NumberOfPieces(t,0);
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
/*
if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
	os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
	pathOb = (ShapeObject*)os.obj;
	int SubCount = pathOb->NumberOfPieces(ip->GetTime(),0);
	float start,inc;
	inc = 1.0f/(float)SubCount;
	start = inc * sid;
	LineU = start + (LineU * inc);
	}
*/
float udist = BoneData[BoneID].CrossSectionList[EndCross].u - BoneData[BoneID].CrossSectionList[StartCross].u;
LineU = LineU - BoneData[BoneID].CrossSectionList[StartCross].u;
float per = LineU/udist;


Inner = Inner + (LInner - Inner) * per;
Outer = Outer + (LOuter - Outer) * per;

//float Influence = 0.0f;

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
	if (BoneData[i].Node != NULL) sel++;
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

		SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_ADDSTRING,0,(LPARAM)(TCHAR*)title);


		}
	}

}

void BonesDefMod::RemoveBone()

{
int fsel;

fsel = SendMessage(GetDlgItem(hParam,IDC_LIST1),
			LB_GETCURSEL ,0,0);
int sel = ConvertSelectedListToBoneID(fsel);

if (sel>=0)
	{
	SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_DELETESTRING  ,(WPARAM) fsel,0);

//	if (theHold.Holding() ) theHold.Put(new DeleteBoneRestore(this,sel));


//nuke reference

//nuke cross sections
	int ct = BoneData[sel].CrossSectionList.Count();
	for (int i =(ct-1); i >= 0 ; i--)
		RemoveCrossSectionNoNotify(sel, i);

//nuke end points

	DeleteReference(BoneData[sel].RefEndPt1ID);
	BoneData[sel].EndPoint1Control = NULL;
	DeleteReference(BoneData[sel].RefEndPt2ID);
	BoneData[sel].EndPoint2Control = NULL;

	RefTable[BoneData[sel].RefEndPt1ID-BONES_REF] = 0;
	RefTable[BoneData[sel].RefEndPt2ID-BONES_REF] = 0;
//	BoneData[sel].Node = NULL;
	recompBoneMap = true; //ns
	DeleteReference(BoneData[sel].BoneRefID);
	RefTable[BoneData[sel].BoneRefID-BONES_REF] = 0;
	BoneData[sel].Node = NULL;
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
/*
	if (BoneData[ModeBoneIndex].flags & BONE_LOCK_FLAG)
		pblock_param->SetValue(PB_LOCK_BONE,0,1);
	else
		pblock_param->SetValue(PB_LOCK_BONE,0,0);

	if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
		pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
	else
		pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);
*/

//	if (sel != 0)
//		sel--;

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


//if  (BoneData.Count() ==0)
if  (bct == 0)
	{
	DisableButtons();
	}


}

void BonesDefMod::RemoveBone(int bid)

{
int sel;
//sel = SendMessage(GetDlgItem(hParams,IDC_LIST1),
//			LB_GETCURSEL ,0,0);
sel = bid;



int fsel = ConvertSelectedBoneToListID(sel);


if (sel>=0)
	{
	SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_DELETESTRING  ,(WPARAM) fsel,0);


//nuke reference
//nuke cross sections
	BoneDataClass *b = &BoneData[sel];

	if (theHold.Holding() ) theHold.Put(new DeleteBoneRestore(this,sel));


	int ct = BoneData[sel].CrossSectionList.Count();
	for (int i =(ct-1); i >= 0 ; i--)
		RemoveCrossSectionNoNotify(sel, i);

//nuke end points
	DeleteReference(BoneData[sel].RefEndPt1ID);
	BoneData[sel].EndPoint1Control = NULL;
	DeleteReference(BoneData[sel].RefEndPt2ID);
	BoneData[sel].EndPoint2Control = NULL;

	RefTable[BoneData[sel].RefEndPt1ID-BONES_REF] = 0;
	RefTable[BoneData[sel].RefEndPt2ID-BONES_REF] = 0;

	DeleteReference(BoneData[sel].BoneRefID);
	RefTable[BoneData[sel].BoneRefID-BONES_REF] = 0;
	BoneData[sel].Node = NULL;
	recompBoneMap = true;	//ns
	BoneData[sel].flags= BONE_DEAD_FLAG;

// bug fix 207093 9/8/99	watje
	BoneData[sel].RefEndPt1ID = -1;
	BoneData[sel].RefEndPt2ID = -1;
	BoneData[sel].BoneRefID = -1;

	
	
	int NodeCount = BoneData.Count();
	for (int j=sel;j<(BoneData.Count()-1);j++)
		{
//		ReplaceReference(j+2,BoneData[j+1].Node);
//now copy the data down also 
//		mod->BoneData[j].CrossSectionList.ZeroCount();
//		BoneData[j].CrossSectionList=BoneData[j+1].CrossSectionList;

//fix this need to copy contolers around 
//		BoneData[j].l1 = BoneData[j+1].l1;
//		BoneData[j].l2 = BoneData[j+1].l2;
//		BoneData[j].flags = BoneData[j+1].flags;
		}

//	DeleteReference(NodeCount-1+2);

//	BoneData[NodeCount-1].Node = NULL;
//	BoneData[NodeCount-1].CrossSectionList.ZeroCount();


	ModeBoneEndPoint = -1;
	ModeBoneEnvelopeIndex = -1;
	ModeBoneEnvelopeSubType = -1;

/*
	if (BoneData[sel].flags & BONE_LOCK_FLAG)
		pblock_param->SetValue(PB_LOCK_BONE,0,1);
	else
		pblock_param->SetValue(PB_LOCK_BONE,0,0);

	if (BoneData[sel].flags & BONE_ABSOLUTE_FLAG)
		pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
	else
		pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);
*/

//	if (sel != 0)
//		sel--;
	ModeBoneIndex = sel;
	int fsel = ConvertSelectedBoneToListID(sel);
	SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_SETCURSEL ,fsel,0);


//this is a complete hack to remove the null node off the end of the linked list
//since there are no deletion tools in the template
/*
	BoneDataClassList TempBoneData;

	TempBoneData.New();
	for (i = 0; i<BoneData.Count(); i++)
		{
		if (BoneData[i].Node != NULL) 
			{
			TempBoneData.Append(BoneData[i]);
			}
		}
	BoneData.New();
	for (i = 0; i<TempBoneData.Count(); i++)
		{
		BoneData.Append(TempBoneData[i]);
		}


	TempBoneData.New();
*/
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

//if  (BoneData.Count() ==0)

if  (bct == 0)
	{
	DisableButtons();
	}


/*
	{
	SendMessage(GetDlgItem(hParams,IDC_LIST1),
					LB_DELETESTRING  ,(WPARAM) sel,0);


//nuke reference

	int NodeCount = BoneData.Count();
	for (int j=sel;j<(BoneData.Count()-1);j++)
		{
		ReplaceReference(j+2,BoneData[j+1].Node);
//now copy the data down also 
//		mod->BoneData[j].CrossSectionList.ZeroCount();
		BoneData[j].CrossSectionList=BoneData[j+1].CrossSectionList;

//fix this need to copy contolers around 
//		BoneData[j].l1 = BoneData[j+1].l1;
//		BoneData[j].l2 = BoneData[j+1].l2;
		BoneData[j].flags = BoneData[j+1].flags;
		}
	DeleteReference(NodeCount-1+2);

	BoneData[NodeCount-1].Node = NULL;
	BoneData[NodeCount-1].CrossSectionList.ZeroCount();

	if (sel != 0)
		sel--;

	
	SendMessage(GetDlgItem(hParams,IDC_LIST1),
				LB_SETCURSEL ,sel,0);

	ModeBoneIndex = sel;
	ModeBoneEndPoint = -1;
	ModeBoneEnvelopeIndex = -1;
	ModeBoneEnvelopeSubType = -1;
	if (BoneData[ModeBoneIndex].flags & BONE_LOCK_FLAG)
		pblock->SetValue(PB_LOCK_BONE,0,1);
	else
		pblock->SetValue(PB_LOCK_BONE,0,0);
	if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
		pblock->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
	else
		pblock->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);



//this is a complete hack to remove the null node off the end of the linked list
//since there are no deletion tools in the template

	BoneDataClassList TempBoneData;

	TempBoneData.New();
	for (int i = 0; i<BoneData.Count(); i++)
		{
		if (BoneData[i].Node != NULL) 
			{
			TempBoneData.Append(BoneData[i]);
			}
		}
	BoneData.New();
	for (i = 0; i<TempBoneData.Count(); i++)
		{
		BoneData.Append(TempBoneData[i]);
		}


	TempBoneData.New();

	BoneMoved = TRUE;
	reevaluate = TRUE;
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	}
*/
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
DeleteReference(BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefOuterID);
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl = NULL;

RefTable[BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefInnerID-BONES_REF] = 0;
RefTable[BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].RefOuterID-BONES_REF] = 0;

BoneData[ModeBoneIndex].CrossSectionList.Delete(ModeBoneEnvelopeIndex,1);

//watje 9-7-99  198721 
Reevaluate(TRUE);
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

}


void BonesDefMod::RemoveCrossSection(int bid, int eid)

{
/*
if ( (ModeBoneEnvelopeIndex <=0)  || (ModeBoneIndex < 0)  ||
	 (ModeBoneEnvelopeIndex >= (BoneData[ModeBoneIndex].CrossSectionList.Count()-1))
	 )
	return;
	*/


DeleteReference(BoneData[bid].CrossSectionList[eid].RefInnerID);
BoneData[bid].CrossSectionList[eid].InnerControl = NULL;
DeleteReference(BoneData[bid].CrossSectionList[eid].RefOuterID);
BoneData[bid].CrossSectionList[eid].OuterControl = NULL;

RefTable[BoneData[bid].CrossSectionList[eid].RefInnerID-BONES_REF] = 0;
RefTable[BoneData[bid].CrossSectionList[eid].RefOuterID-BONES_REF] = 0;

BoneData[bid].CrossSectionList.Delete(eid,1);

//watje 9-7-99  198721 
Reevaluate(TRUE);
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

}


void BonesDefMod::RemoveCrossSectionNoNotify(int bid, int eid)

{


DeleteReference(BoneData[bid].CrossSectionList[eid].RefInnerID);
BoneData[bid].CrossSectionList[eid].InnerControl = NULL;
DeleteReference(BoneData[bid].CrossSectionList[eid].RefOuterID);
BoneData[bid].CrossSectionList[eid].OuterControl = NULL;

RefTable[BoneData[bid].CrossSectionList[eid].RefInnerID-BONES_REF] = 0;
RefTable[BoneData[bid].CrossSectionList[eid].RefOuterID-BONES_REF] = 0;

BoneData[bid].CrossSectionList.Delete(eid,1);

//watje 9-7-99  198721 
Reevaluate(TRUE);

}






// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.
// JBW: since the old path controller kept all parameters as instance data members, this setter callback
// is implemented to to reduce changes to existing code 
class BonesDefPBAccessor : public PBAccessor
{ 
public:
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
		BonesDefMod* p = (BonesDefMod*)owner;
		int val;
		switch (id)
		{
			case skin_draw_all_envelopes:
				if (p->pblock_display)
					{
					p->pblock_display->GetValue(skin_display_draw_all_envelopes,t,val,FOREVER);
					v.i = val;
					}
				break;
			case skin_draw_vertices:
				if (p->pblock_display)
					{
					p->pblock_display->GetValue(skin_display_draw_vertices,t,val,FOREVER);
					v.i = val;
					}
				break;
			case skin_ref_frame:
				if (p->pblock_advance)
					{
					p->pblock_advance->GetValue(skin_advance_ref_frame,t,val,FOREVER);
					v.i = val;
					}
				break;
			case skin_always_deform:
				if (p->pblock_advance)
					{
					p->pblock_advance->GetValue(skin_advance_always_deform,t,val,FOREVER);
					v.i = val;
					}
				break;

		}
	}

	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		BonesDefMod* p = (BonesDefMod*)owner;
		int val;
		switch (id)
		{
			case skin_draw_all_envelopes:
				if (p->pblock_display)
					{
					val = v.i;
					p->pblock_display->SetValue(skin_display_draw_all_envelopes,t,val);
					
					}
				break;
			case skin_draw_vertices:
				if (p->pblock_display)
					{
					val = v.i;
					p->pblock_display->SetValue(skin_display_draw_vertices,t,val);
					}
				break;
			case skin_ref_frame:
				if (p->pblock_advance)
					{
					val = v.i;
					p->pblock_advance->SetValue(skin_advance_ref_frame,t,val);
					}
				break;
			case skin_always_deform:
				if (p->pblock_advance)
					{
					val = v.i;
					p->pblock_advance->SetValue(skin_advance_always_deform,t,val);
					}
				break;

		}
	}
};

static BonesDefPBAccessor bonesdef_accessor;


static int outputIDs[] = {IDC_RADIO1,IDC_RADIO2,IDC_RADIO3,IDC_RADIO4,IDC_RADIO5,IDC_RADIO6};

//
// Parameters


#define PARAMDESC_LENGTH	18
// per instance param block
static ParamBlockDesc2 skin_param_blk ( skin_params, _T("Parameters"),  0, &bonesDefDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_PARAM_REF, 
	//rollout
	IDD_BONESDEFPARAM, IDS_PW_PARAMETER, 0, 0, NULL,
	// params
	skin_effect,  _T("effect"),	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_PW_EFFECT, 
		p_default, 		1.0f,	
		p_range, 		0.f, 1.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EFFECT,IDC_EFFECTSPIN,  SPIN_AUTOSCALE,
		end, 

	skin_filter_vertices, 	_T("filter_vertices"),		TYPE_BOOL, 		0,				IDS_PW_FILTERVERTICES,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FILTER_VERTICES_CHECK, 
		end, 
	skin_filter_bones, 	_T("filter_cross_sections"),		TYPE_BOOL, 		0,				IDS_PW_FILTERENVELOPES,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FILTER_BONES_CHECK, 
		end, 
	skin_filter_envelopes, 	_T("filter_envelopes"),		TYPE_BOOL, 		0,				IDS_PW_FILTERBONES,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FILTER_ENVELOPES_CHECK, 
		end, 

	skin_draw_all_envelopes, 	_T("draw_all_envelopes"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_DRAWALLENVELOPES,
		p_default, 		FALSE, 
//		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_DRAWALL_ENVELOPES_CHECK, 
		p_accessor,		&bonesdef_accessor,
		end, 
		
	skin_draw_vertices, 	_T("draw_vertices"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_COLORVERTS,
		p_default, 		TRUE, 
//		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_DRAW_VERTICES_CHECK, 
		p_accessor,		&bonesdef_accessor,
		end, 
		
	skin_ref_frame,  _T("ref_frame"),	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_REFFRAME, 
		p_default, 		0,	
//		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_REF_FRAME,IDC_REF_FRAME_SPIN,  SPIN_AUTOSCALE,
		p_accessor,		&bonesdef_accessor,
		end, 
	skin_paint_radius,  _T("paint_radius"),	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_PW_RADIUS, 
		p_default, 		20.0f,	
		p_range, 		0.f, 5000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SRADIUS,IDC_SRADIUSSPIN,  SPIN_AUTOSCALE,
		end, 
	   

	skin_paint_feather,  _T("paint_feather"),	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_PW_FEATHER, 
		p_default, 		0.7f,	
		p_range, 		0.f, 1.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FEATHER,IDC_FEATHERSPIN,  SPIN_AUTOSCALE,
		end, 

	skin_cross_radius,  _T("paint_cross_radius"),	TYPE_FLOAT, 	0, 	IDS_PW_RADIUS, 
		p_default, 		10.f,	
		p_range, 		0.f, 1000000.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ERADIUS,IDC_ERADIUSSPIN,  SPIN_AUTOSCALE,
		end, 
	  
	skin_always_deform, 	_T("always_deform"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_ALWAYS_DEFORM,
		p_default, 		TRUE, 
		p_accessor,		&bonesdef_accessor,
//		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_ALWAYSDEFORM_CHECK, 
		end, 

	skin_paint_str,  _T("paint_str"),	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_PW_PAINTSTR, 
		p_default, 		1.0f,	
		p_range, 		0.f, 1.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PAINT_STR2,IDC_PAINT_STR_SPIN2,  SPIN_AUTOSCALE,
		end, 

	end
	);

// per instance display block
static ParamBlockDesc2 skin_display_blk ( skin_display, _T("display"),  0, &bonesDefDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_DISPLAY_REF, 
	//rollout
	IDD_BONESDEFDISPLAY, IDS_PW_DISPLAY, 0, APPENDROLL_CLOSED, NULL,
	// params
	skin_display_draw_all_envelopes, 	_T("draw_all_envelopes"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_DRAWALLENVELOPES,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_DRAWALL_ENVELOPES_CHECK, 
		end, 
		
	skin_display_draw_vertices, 	_T("draw_vertices"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_COLORVERTS,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_DRAW_VERTICES_CHECK, 
		end, 

	skin_display_all_gizmos, 	_T("draw_all_gizmos"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_DRAWALLGIZMOS,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_SHOW_ALL_GIZMOS_CHECK, 
		end, 

	skin_display_all_vertices, 	_T("draw_all_vertices"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_DRAWALLVERTS,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_SHOW_ALL_VERTICES_CHECK, 
		end, 


	end
	);



// per instance display block
static ParamBlockDesc2 skin_advance_blk ( skin_advance, _T("advance"),  0, &bonesDefDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_ADVANCE_REF, 
	//rollout
	IDD_BONESDEFADVANCE, IDS_PW_ADVANCE, 0, APPENDROLL_CLOSED, NULL,
	// params
	skin_advance_ref_frame,  _T("ref_frame"),	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_REFFRAME, 
		p_default, 		0,	
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_REF_FRAME,IDC_REF_FRAME_SPIN,  SPIN_AUTOSCALE,
		end, 
	skin_advance_always_deform, 	_T("always_deform"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_ALWAYS_DEFORM,
		p_default, 		TRUE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_ALWAYSDEFORM_CHECK, 
		end, 

	skin_advance_rigid_verts, 	_T("rigid_vertices"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_RIGIDVERTS,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_RIGID_VERTICES_CHECK, 
		end, 

	skin_advance_rigid_handles, 	_T("rigid_handles"),		TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_RIGIDHANDLES,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_RIGID_HANDLES_CHECK, 
		end, 
	skin_advance_fast_update, 	_T("fast_update"),	TYPE_BOOL, 		P_RESET_DEFAULT,  IDS_PW_FAST,
		p_default, 		FALSE, 
		p_ui, 			TYPE_SINGLECHEKBOX, 	IDC_FAST_CHECK, 
		end, 

	end
	);

// per instance gizmo block
static ParamBlockDesc2 skin_gizmos_blk ( skin_gizmos, _T("gizmos"),  0, &bonesDefDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_GIZMOS_REF, 
	//rollout
	IDD_BONESDEFGIZMOS, IDS_PW_GIZMOS, 0, APPENDROLL_CLOSED, NULL,
	// params
	skin_gizmos_list, 	_T("gizmos"),		TYPE_REFTARG_TAB, 	0,	0,  IDS_PW_GIZMOS,
		end, 

	end
	);

static ParamBlockDescID descVer0[17] = {
	{ TYPE_FLOAT, NULL, FALSE,  skin_effect },		// Effect	
	{ TYPE_INT,   NULL, FALSE, -1 },		// Lock Bone
	{ TYPE_INT,   NULL, FALSE, -1 },		// Absolute Influence
	{ TYPE_INT,   NULL, FALSE, skin_filter_vertices },		// Filter Vertices
	{ TYPE_INT,   NULL, FALSE, skin_filter_bones },		// Filter Bones
	{ TYPE_INT,   NULL, FALSE, skin_filter_envelopes },		// Filter Envelopes
	{ TYPE_INT,   NULL, FALSE, skin_draw_all_envelopes },		// Draw All envelopes
	{ TYPE_INT,   NULL, FALSE, skin_draw_vertices },		// Draw vertice
	{ TYPE_INT,   NULL, FALSE, skin_ref_frame },		// Ref Frame
	{ TYPE_FLOAT, NULL, FALSE,  skin_paint_feather},		// Radius	
	{ TYPE_INT, NULL, FALSE,  -1},		// Project through	
	{ TYPE_INT, NULL, FALSE,  -1},		// falloff	
	{ TYPE_INT, NULL, FALSE,  -1},		// bone falloff	
	{ TYPE_FLOAT, NULL, FALSE,  skin_paint_feather},		// feather	
	{ TYPE_INT, NULL, FALSE,  -1},		// Draw bone envelope	
	{ TYPE_FLOAT, NULL, FALSE,  skin_cross_radius},	// envelope raduis	
	{ TYPE_INT, NULL, FALSE,  skin_always_deform}		// always deform
	};


static ParamBlockDescID descVer1[18] = {
	{ TYPE_FLOAT, NULL, FALSE,  skin_effect },		// Effect	
	{ TYPE_INT,   NULL, FALSE, -1 },		// Lock Bone
	{ TYPE_INT,   NULL, FALSE, -1 },		// Absolute Influence
	{ TYPE_INT,   NULL, FALSE, skin_filter_vertices },		// Filter Vertices
	{ TYPE_INT,   NULL, FALSE, skin_filter_bones },		// Filter Bones
	{ TYPE_INT,   NULL, FALSE, skin_filter_envelopes },		// Filter Envelopes
	{ TYPE_INT,   NULL, FALSE, skin_draw_all_envelopes },		// Draw All envelopes
	{ TYPE_INT,   NULL, FALSE, skin_draw_vertices },		// Draw vertice
	{ TYPE_INT,   NULL, FALSE, skin_ref_frame },		// Ref Frame
	{ TYPE_FLOAT, NULL, FALSE,  skin_paint_feather},		// Radius	
	{ TYPE_INT, NULL, FALSE,  -1},		// Project through	
	{ TYPE_INT, NULL, FALSE,  -1},		// falloff	
	{ TYPE_INT, NULL, FALSE,  -1},		// bone falloff	
	{ TYPE_FLOAT, NULL, FALSE,  skin_paint_feather},		// feather	
	{ TYPE_INT, NULL, FALSE,  -1},		// Draw bone envelope	
	{ TYPE_FLOAT, NULL, FALSE,  skin_cross_radius},	// envelope raduis	
	{ TYPE_INT, NULL, FALSE,  skin_always_deform},		// always deform
	{ TYPE_FLOAT, NULL, FALSE,  skin_paint_str}	// paint str	
	};

#define PBLOCK_LENGTH	18

#define CURRENT_VERSION	2


static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,17,0),
	ParamVersionDesc(descVer1,18,1)
	};
#define NUM_OLDVERSIONS	2

// Current version
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- Affect region mod methods -------------------------------

BonesDefMod::BonesDefMod() 
	{

	inRender = FALSE;
	fastUpdate = FALSE;

	copyGizmoBuffer = NULL;
	wereAddingGizmos = FALSE;
	ver = 4;
	recompInitTM = false; //ns
	recompBoneMap = true; //ns

//	MakeRefByID(
//	/	FOREVER, PBLOCK_PARAM_REF, 
//		CreateParameterBlock(
//			descVer1, PBLOCK_LENGTH, CURRENT_VERSION));

	GetBonesDefModDesc()->MakeAutoParamBlocks(this);
	
	pblock_param->SetValue(skin_effect,0,0.0f);
	pblock_param->SetValue(skin_cross_radius,0,10.0f);

	pblock_param->SetValue(skin_filter_vertices,0,0);
	pblock_param->SetValue(skin_filter_bones,0,1);
	pblock_param->SetValue(skin_filter_envelopes,0,1);



	pblock_display->SetValue(skin_display_draw_all_envelopes,0,0);
	pblock_display->SetValue(skin_display_draw_vertices,0,1);



//	pblock_param->SetValue(PB_PROJECT_THROUGH,0,1);
//	pblock_param->SetValue(PB_FALLOFF,0,1);
	pblock_param->SetValue(skin_paint_feather,0,0.7f);
	pblock_param->SetValue(skin_paint_radius,0,24.0f);

	pblock_param->SetValue(skin_paint_str,0,0.1f);

	pblock_advance->SetValue(skin_advance_ref_frame,0,0);
	pblock_advance->SetValue(skin_advance_always_deform,0,1);

	RefTable.ZeroCount();

//	NodeCount = 0;
//	effect = -1.0f;
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
	FilterVertices = 0;
	FilterBones = 0;
	FilterEnvelopes = 0;
	DrawEnvelopes = 0;
	paintStr  = 0.1f;

//	VertexData = NULL;
//	VertexDataCount = 0;
//	CurrentCachePiece = -1;
//	bmd = NULL;
	cacheValid = FALSE;
	unlockVerts = FALSE;
	OldVertexDataCount = 0;
	unlockBone = FALSE;
	unlockAllBones = FALSE;

	painting = FALSE;
	inPaint = FALSE;
	reloadSplines = FALSE;

	splineChanged = FALSE;
	forceRecomuteBaseNode = FALSE;
	updateP = FALSE;

	bindNode = NULL;
	initialXRefTM.IdentityMatrix();
	xRefTM.IdentityMatrix();

	}


BonesDefMod::~BonesDefMod()
	{
	DeleteAllRefsFromMe();
	p1Temp = NULL;


//	VertexData.ZeroCount();

	for (int i=0;i<BoneData.Count();i++)
        BoneData[i].CrossSectionList.ZeroCount();

	BoneData.New();
	
	if (copyGizmoBuffer) delete copyGizmoBuffer;


	}
int BonesDefMod::RenderBegin(TimeValue t, ULONG flags)
{
inRender= TRUE;
}
int BonesDefMod::RenderEnd(TimeValue t)
{
inRender= FALSE;
}


void BonesDefMod::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;
	this->flags = flags;
	this->prev = prev;

	editMod  = this;

	// Add our sub object type
	// TSTR type1(GetString(IDS_RB_BONESDEFPOINTS));
	// const TCHAR *ptype[] = {type1};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 1);

	// Create sub object editing modes.
	moveMode    = new MoveModBoxCMode(this,ip);
	CrossSectionMode    = new CreateCrossSectionMode(this,ip);
	PaintMode    = new CreatePaintMode(this,ip);
	
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

//	bonesDefDesc.BeginEditParams(ip, this, flags, prev);
//	bones_param_blk.SetUserDlgProc(new MapDlgProc(this));
//	bones_paint_blk.SetUserDlgProc(new PaintDlgProc(this));
//	bones_filter_blk.SetUserDlgProc(new FilterDlgProc(this));
//	bones_advance_blk.SetUserDlgProc(new AdvanceDlgProc(this));

	bonesDefDesc.BeginEditParams(ip, this, flags, prev);
	// install a callback for the type in.
	skin_param_blk.SetUserDlgProc(new MapDlgProc(this));
	skin_advance_blk.SetUserDlgProc(new AdvanceMapDlgProc(this));
	skin_gizmos_blk.SetUserDlgProc(new GizmoMapDlgProc(this));

/*	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGTH,
		pblock_param,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_BONESDEFPARAM),
		GetString(IDS_PW_PARAMETERS),
		0);	

	pmapParam->SetUserDlgProc(new MapDlgProc(this));
*/


	}

void BonesDefMod::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{

	if (pblock_gizmos->Count(skin_gizmos_list) > 0)
		{
		ReferenceTarget *ref;
		if (currentSelectedGizmo != -1)
			{
			ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
			GizmoClass *gizmo = (GizmoClass *)ref;
			if (gizmo) gizmo->EndEditParams(ip, flags,prev);
			}
		}

	this->ip = NULL;
	this->prev = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->DeleteMode(moveMode);	
	if (moveMode) delete moveMode;
	moveMode = NULL;	

	ip->DeleteMode(CrossSectionMode);	
	if (CrossSectionMode) delete CrossSectionMode;
	CrossSectionMode = NULL;	

	ip->DeleteMode(PaintMode);	
	if (PaintMode) delete PaintMode;
	PaintMode = NULL;	


	iCrossSectionButton = NULL;
	ReleaseICustButton(iCrossSectionButton);
	iCrossSectionButton = NULL;

	iLock = NULL;
	ReleaseICustButton(iLock);
	iLock = NULL;

	iAbsolute = NULL;
	ReleaseICustButton(iAbsolute);
	iAbsolute = NULL;

	iEnvelope = NULL;
	ReleaseICustButton(iEnvelope);
	iEnvelope = NULL;

	iFalloff = NULL;
	ReleaseICustButton(iFalloff);
	iFalloff = NULL;

	iCopy = NULL;
	ReleaseICustButton(iCopy);
	iCopy = NULL;

	iPaste = NULL;
	ReleaseICustButton(iPaste);
	iPaste = NULL;


	iPaintButton = NULL;
	ReleaseICustButton(iPaintButton);
	iPaintButton = NULL;



	ip->GetRightClickMenuManager()->Unregister(&rMenu);


	ReleaseICustToolbar(iParams);
	iParams = NULL;
	bonesDefDesc.EndEditParams(ip, this, flags, next);

//	DestroyCPParamMap(pmapParam);
	}

RefTargetHandle BonesDefMod::Clone(RemapDir& remap)
	{
	BonesDefMod *mod = new BonesDefMod();
	mod->ReplaceReference(PBLOCK_PARAM_REF,pblock_param->Clone(remap));
	mod->ReplaceReference(POINT1_REF,p1Temp->Clone(remap));

//copy controls
	mod->RefTable = RefTable;

	for (int i = 0; i<BoneData.Count(); i++)
		{
		BoneDataClass b = BoneData[i];
		b.Node = NULL;
		b.EndPoint1Control= NULL;
		b.EndPoint2Control = NULL;
		for (int j = 0; j < b.CrossSectionList.Count(); j++)
			{
			b.CrossSectionList[j].InnerControl = NULL;
			b.CrossSectionList[j].OuterControl = NULL;
			}
		mod->BoneData.Append(b);
		}


	for (i=0;i < BoneData.Count();i++)
		{
		if (BoneData[i].EndPoint1Control)
			mod->ReplaceReference(BoneData[i].RefEndPt1ID,BoneData[i].EndPoint1Control->Clone(remap));
		if (BoneData[i].EndPoint2Control)
			mod->ReplaceReference(BoneData[i].RefEndPt2ID,BoneData[i].EndPoint2Control->Clone(remap));
		if (BoneData[i].Node)
			mod->ReplaceReference(BoneData[i].BoneRefID,BoneData[i].Node);

		for (int j=0;j < BoneData[i].CrossSectionList.Count();j++)
			{
			if (BoneData[i].CrossSectionList[j].InnerControl)
				mod->ReplaceReference(BoneData[i].CrossSectionList[j].RefInnerID,BoneData[i].CrossSectionList[j].InnerControl->Clone(remap));
			if (BoneData[i].CrossSectionList[j].OuterControl)
				mod->ReplaceReference(BoneData[i].CrossSectionList[j].RefOuterID,BoneData[i].CrossSectionList[j].OuterControl->Clone(remap));
			}

		}


	mod->forceRecomuteBaseNode = TRUE;





//	for (int i = 0; i < ; i++)
//		mod->ReplaceReference(i,);
//copy boneData


	BaseClone(this, mod, remap);
	return mod;
	}



void BonesDefMod::SetVertex(BoneModData *bmd,int vertID, int BoneID, float amount)

{
//Tab<int> vsel;

if (BoneData[BoneID].flags & BONE_LOCK_FLAG)
	return;

ObjectState os;
ShapeObject *pathOb = NULL;

if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
	os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
	pathOb = (ShapeObject*)os.obj;
	}

/*
int selcount = bmd->selected.GetSize();

for (int i = 0 ; i <bmd->VertexDataCount;i++)
	{
	if (bmd->selected[i]) vsel.Append(1,&i,1);
	}
*/

float effect,originaleffect;
//for ( i = 0; i < vsel.Count();i++)
	{
	int found = 0;

//	int k = vsel[i];
	int k = vertID;
	for (int j =0; j<bmd->VertexData[k]->d.Count();j++)
		{
		if ( (bmd->VertexData[k]->d[j].Bones == BoneID)&& (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG)))

			{
			originaleffect = bmd->VertexData[k]->d[j].Influences;
			bmd->VertexData[k]->d[j].Influences = amount;
//			bmd->VertexData[k]->d[j].normalizedInfluences = -1.0f;  //FIX?
			found = 1;
			effect = bmd->VertexData[k]->d[j].Influences;
			j = bmd->VertexData[k]->d.Count();

			}
		}

	if ((found == 0) && (amount > 0.0f))
		{

		VertexInfluenceListClass td;
		td.Bones = BoneID;
		td.Influences = amount;
//		td.normalizedInfluences = -1.0f;  //FIX?
//check if spline if so add approriate spline data info also
// find closest spline
		
		if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
			{
			Interval valid;
			Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
			ntm = bmd->BaseTM * Inverse(ntm);

			float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
//										pathOb,
										&BoneData[BoneID].referenceSpline,
			                            td.u,
										td.OPoints,td.Tangents,
										td.SubCurveIds,td.SubSegIds,
										ntm);
//										BoneData[BoneID].tm);
			}


		bmd->VertexData[k]->d.Append(1,&td,1);
		effect = amount;
		originaleffect = 0.0f;
		found = 1;
		}

	if (found == 1)
		{
		int bc = bmd->VertexData[k]->d.Count();

//remove 0 influence bones otherwise they skew the reweigthing
		for (j=0;j<bmd->VertexData[k]->d.Count();j++)
			{
			if (bmd->VertexData[k]->d[j].Influences==0.0f)
				{
				bmd->VertexData[k]->d.Delete(j,1);
				j--;
				}
			}


//rebalance rest
		float remainder = 1.0f - effect;
		originaleffect = 1.0f - originaleffect;
		if (bmd->VertexData[k]->d.Count() > 1)
			{
			for (j=0;j<bmd->VertexData[k]->d.Count();j++)
				{
	
				if (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG))
					{
					if (bmd->VertexData[k]->d[j].Bones!=BoneID)
						{
						if (originaleffect == 0.0f)
							 bmd->VertexData[k]->d[j].Influences = remainder/(bmd->VertexData[k]->d.Count()-1.0f);
						else 
							bmd->VertexData[k]->d[j].Influences = bmd->VertexData[k]->d[j].Influences/originaleffect * remainder;
//						bmd->VertexData[k]->d[j].normalizedInfluences = -1.0f; //FIX?

						}
					}
				}
			}

		bmd->VertexData[k]->modified = TRUE;

		}
	}
}



void BonesDefMod::SetVertices(BoneModData *bmd,int vertID, Tab<int> BoneIDList, Tab<float> amountList)

{
//Tab<int> vsel;


ObjectState os;
ShapeObject *pathOb = NULL;



float effect,originaleffect;
int k = vertID;

for (int i = 0; i < amountList.Count();i++)
	{
	int found = 0;

	float amount = amountList[i];
	int BoneID = BoneIDList[i];
	if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
		{
		os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
		pathOb = (ShapeObject*)os.obj;
		}

	for (int j =0; j<bmd->VertexData[k]->d.Count();j++)
		{
		if ( (bmd->VertexData[k]->d[j].Bones == BoneID)&& (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG)))

			{
			originaleffect = bmd->VertexData[k]->d[j].Influences;
			bmd->VertexData[k]->d[j].Influences = amount;
//			bmd->VertexData[k]->d[j].normalizedInfluences = -1.0f; //FIX?
			found = 1;
			effect = bmd->VertexData[k]->d[j].Influences;
			j = bmd->VertexData[k]->d.Count();

			}
		}

	if ((found == 0) && (amount > 0.0f))
		{

		VertexInfluenceListClass td;
		td.Bones = BoneID;
		td.Influences = amount;
//		td.normalizedInfluences = -1.0f;  //FIX?
//check if spline if so add approriate spline data info also
// find closest spline
		
		if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
			{
			Interval valid;
			Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
			ntm = bmd->BaseTM * Inverse(ntm);

			float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
//										pathOb,
										&BoneData[BoneID].referenceSpline,
			                            td.u,
										td.OPoints,td.Tangents,
										td.SubCurveIds,td.SubSegIds,
										ntm);
//										BoneData[BoneID].tm);
			}


		bmd->VertexData[k]->d.Append(1,&td,1);
		effect = amount;
		originaleffect = 0.0f;
		found = 1;
		}

	if (found == 1)
		{
		int bc = bmd->VertexData[k]->d.Count();

//remove 0 influence bones otherwise they skew the reweigthing
		for (j=0;j<bmd->VertexData[k]->d.Count();j++)
			{
			if (bmd->VertexData[k]->d[j].Influences==0.0f)
				{
				bmd->VertexData[k]->d.Delete(j,1);
				j--;
				}
			}


//rebalance rest

		}
	}
bmd->VertexData[k]->modified = TRUE;
if (bmd->VertexData[k]->d.Count() > 1)
	{
	float totalSum = 0.0f;
	for (int j=0;j<bmd->VertexData[k]->d.Count();j++)
		{
		if (bmd->VertexData[k]->d[j].Influences==0.0f)
			{
			bmd->VertexData[k]->d.Delete(j,1);
			j--;
			}
		}

	for (j=0;j<bmd->VertexData[k]->d.Count();j++)
		totalSum += bmd->VertexData[k]->d[j].Influences;
	for (j=0;j<bmd->VertexData[k]->d.Count();j++)
		{
		 bmd->VertexData[k]->d[j].Influences = bmd->VertexData[k]->d[j].Influences/totalSum;
		}
	}


}



  
void BonesDefMod::SetSelectedVertices(BoneModData *bmd, int BoneID, float amount)

{
Tab<int> vsel;

if (BoneData[BoneID].flags & BONE_LOCK_FLAG)
	return;

ObjectState os;
ShapeObject *pathOb = NULL;

if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
	os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
	pathOb = (ShapeObject*)os.obj;
	}


int selcount = bmd->selected.GetSize();

for (int i = 0 ; i <bmd->VertexDataCount;i++)
	{
	if (bmd->selected[i]) vsel.Append(1,&i,1);
	}

float effect,originaleffect;
for ( i = 0; i < vsel.Count();i++)
	{
	int found = 0;

	int k = vsel[i];
	for (int j =0; j<bmd->VertexData[k]->d.Count();j++)
		{
		if ( (bmd->VertexData[k]->d[j].Bones == BoneID)&& (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG)))

			{
			originaleffect = bmd->VertexData[k]->d[j].Influences;
			bmd->VertexData[k]->d[j].Influences = amount;
//			bmd->VertexData[k]->d[j].normalizedInfluences = -1.0f;//FIX?
			found = 1;
			effect = bmd->VertexData[k]->d[j].Influences;
			j = bmd->VertexData[k]->d.Count();

			}
		}

	if ((found == 0) && (amount > 0.0f))
		{

		VertexInfluenceListClass td;
		td.Bones = BoneID;
		td.Influences = amount;
//		td.normalizedInfluences = -1.0f; //FIX?
//check if spline if so add approriate spline data info also
// find closest spline
		
		if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
			{
			Interval valid;
			Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
			ntm = bmd->BaseTM * Inverse(ntm);

			float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,
//										pathOb,
										&BoneData[BoneID].referenceSpline,
			                            td.u,
										td.OPoints,td.Tangents,
										td.SubCurveIds,td.SubSegIds,
										ntm);
//										BoneData[BoneID].tm);
			}


		bmd->VertexData[k]->d.Append(1,&td,1);
		effect = amount;
		originaleffect = 0.0f;
		found = 1;
		}

	if (found == 1)
		{
		int bc = bmd->VertexData[k]->d.Count();

//remove 0 influence bones otherwise they skew the reweigthing
		for (j=0;j<bmd->VertexData[k]->d.Count();j++)
			{
			if ((bmd->VertexData[k]->d[j].Influences==0.0f) && (bmd->VertexData[k]->d[j].Bones==BoneID))
//			if (bmd->VertexData[k]->d[j].Influences==0.0f)
				{
				bmd->VertexData[k]->d.Delete(j,1);
				j--;
				}
			}



//rebalance rest
		float remainder = 1.0f - effect;
		originaleffect = 1.0f - originaleffect;
		if (bmd->VertexData[k]->d.Count() > 1)
			{
			for (j=0;j<bmd->VertexData[k]->d.Count();j++)
				{
	
				if (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG))
					{
					if (bmd->VertexData[k]->d[j].Bones!=BoneID)
						{
//						if (originaleffect == 0.0f)
							 bmd->VertexData[k]->d[j].Influences = remainder/(bmd->VertexData[k]->d.Count()-1.0f);
//						else 
//							bmd->VertexData[k]->d[j].Influences = bmd->VertexData[k]->d[j].Influences/originaleffect * remainder;
//						bmd->VertexData[k]->d[j].normalizedInfluences = -1.0f; //FIX?

						}
					}
				}
			}

		bmd->VertexData[k]->modified = TRUE;

		}
	}
}


void BonesDefMod::IncrementVertices(BoneModData *bmd, int BoneID, Tab<float> amount, int flip )

{

ObjectState os;
ShapeObject *pathOb = NULL;


if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
	os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
	pathOb = (ShapeObject*)os.obj;
	}

float effect,originaleffect;
int found;
float val;
for ( int i = 0; i < amount.Count();i++)
	{
	if (amount[i] != -10.0f)
		{
		found = 0;
		val = amount[i];
		for (int j =0; j<bmd->VertexData[i]->d.Count();j++)
			{
			if  (bmd->VertexData[i]->d[j].Bones == BoneID) 
				{
				originaleffect = bmd->VertexData[i]->d[j].Influences;
				if (flip)
					{
					bmd->VertexData[i]->d[j].Influences -= amount[i];
					if (bmd->VertexData[i]->d[j].Influences < 0.0f ) bmd->VertexData[i]->d[j].Influences = 0.0f;
//					bmd->VertexData[i]->d[j].normalizedInfluences = -1.0f; //FIX?
					}
				else
					{
					bmd->VertexData[i]->d[j].Influences += amount[i];
					if (bmd->VertexData[i]->d[j].Influences > 1.0f ) bmd->VertexData[i]->d[j].Influences = 1.0f;
//					bmd->VertexData[i]->d[j].normalizedInfluences = -1.0f;//FIX?
					}

				found = 1;
				effect = bmd->VertexData[i]->d[j].Influences;
				j = bmd->VertexData[i]->d.Count();

				}
			}

		if ((!found) && (amount[i] > 0.0f) && !(flip) )
			{
			VertexInfluenceListClass td;
			td.Bones = BoneID;
			td.Influences = amount[i];
//			td.normalizedInfluences = -1.0f;//FIX?
			if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
				{
				Interval valid;
				Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
				ntm = bmd->BaseTM * Inverse(ntm);

				float garbage = SplineToPoint(bmd->VertexData[i]->LocalPos,
//										pathOb,
										&BoneData[BoneID].referenceSpline,
			                            td.u,
										td.OPoints,td.Tangents,
										td.SubCurveIds,td.SubSegIds,
										ntm);
				}

			bmd->VertexData[i]->d.Append(1,&td,1);
			effect = amount[i];
			originaleffect = 0.0f;
			found = 1;
			}

		if (found==1)
			{

//remove 0 influence bones otherwise they skew the reweigthing
/*	for (j=0;j<bmd->VertexData[i]->d.Count();j++)
				{
				if (bmd->VertexData[i]->d[j].Influences==0.0f)
					{
					bmd->VertexData[i]->d.Delete(j,1);
					}
				}
*/
			float remainder = 1.0f - effect;
			originaleffect = 1.0f - originaleffect;
			if (bmd->VertexData[i]->d.Count() > 1)
				{
				for (j=0;j<bmd->VertexData[i]->d.Count();j++)
					{
					if (bmd->VertexData[i]->d[j].Bones!=BoneID)
						{
//				if (originaleffect == 0.0f)
						 bmd->VertexData[i]->d[j].Influences = remainder/(bmd->VertexData[i]->d.Count()-1.0f);
//						else	 
//							bmd->VertexData[i]->d[j].Influences = bmd->VertexData[i]->d[j].Influences/originaleffect * remainder;
//						bmd->VertexData[i]->d[j].normalizedInfluences = -1.0f; //FIX?
						}
					}
				}

			bmd->VertexData[i]->modified = TRUE;
			}

		}
	}

/*
ObjectState os;
ShapeObject *pathOb = NULL;

if (BoneData[BoneID].flags & BONE_LOCK_FLAG)
	return;

if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
	{
	os = BoneData[BoneID].Node->EvalWorldState(ip->GetTime());
	pathOb = (ShapeObject*)os.obj;
	}

Tab<int> vsel;
for (int i = 0 ; i <bmd->VertexDataCount;i++)
	{
	if (bmd->selected[i]) vsel.Append(1,&i,1);
	}
float effect,originaleffect;
for ( i = 0; i < vsel.Count();i++)
	{
	int found = 0;

	int k = vsel[i];
	for (int j =0; j<bmd->VertexData[k]->d.Count();j++)
		{
		if ( (bmd->VertexData[k]->d[j].Bones == BoneID) && (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG)))
			{
			originaleffect = bmd->VertexData[k]->d[j].Influences;
			if (flip)
				{
				if ((1.0f - amount[i]) < bmd->VertexData[k]->d[j].Influences)
					{
					bmd->VertexData[k]->d[j].Influences = (1.0f-amount[i]);
//					bmd->VertexData[k]->d[j].normalizedInfluences = -1.0f;
					}
				}
			else
				{
				if (amount[i] > bmd->VertexData[k]->d[j].Influences)
					{
					bmd->VertexData[k]->d[j].Influences = amount[i];
//					bmd->VertexData[k]->d[j].normalizedInfluences = -1.0f;
					}
				}

			found = 1;
			effect = bmd->VertexData[k]->d[j].Influences;
			j = bmd->VertexData[k]->d.Count();

			}
		}

	if (flip)
		{
		}
	else
		{
		if ((!found) && (amount[i] > 0.0f))
			{

			VertexInfluenceListClass td;
			td.Bones = BoneID;
			td.Influences = amount[i];
//			td.normalizedInfluences = -1.0f;
			if (BoneData[BoneID].flags & BONE_SPLINE_FLAG)
				{
				Interval valid;
				Matrix3 ntm = BoneData[BoneID].Node->GetObjTMBeforeWSM(RefFrame,&valid);
				ntm = bmd->BaseTM * Inverse(ntm);

				float garbage = SplineToPoint(bmd->VertexData[k]->LocalPos,pathOb,
			                            td.u,
										td.OPoints,td.Tangents,
										td.SubCurveIds,td.SubSegIds,
										ntm);
				}


			bmd->VertexData[k]->d.Append(1,&td,1);
			effect = amount[i];
			originaleffect = 0.0f;
			found = 1;
			}
		}

	if (found==1)
		{

//remove 0 influence bones otherwise they skew the reweigthing
		for (j=0;j<bmd->VertexData[k]->d.Count();j++)
			{
			if (bmd->VertexData[k]->d[j].Influences==0.0f)
				{
				bmd->VertexData[k]->d.Delete(j,1);
				}
			}

		float remainder = 1.0f - effect;
		originaleffect = 1.0f - originaleffect;
		for (j=0;j<bmd->VertexData[k]->d.Count();j++)
			{
			if (!(BoneData[bmd->VertexData[k]->d[j].Bones].flags & BONE_LOCK_FLAG))
				{
				if (bmd->VertexData[k]->d[j].Bones!=BoneID)
					{
					if (originaleffect == 0.0f)
						 bmd->VertexData[k]->d[j].Influences = remainder/(bmd->VertexData[k]->d.Count()-1.0f);
					else 
						bmd->VertexData[k]->d[j].Influences = bmd->VertexData[k]->d[j].Influences/originaleffect * remainder;
//					bmd->VertexData[k]->d[j].normalizedInfluences = -1.0f;
					}
				}
			}

		bmd->VertexData[k]->modified = TRUE;
		}

	}
*/
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
//Object *obj = mnode->EvalWorldState(RefFrame).obj;

//ObjectState os = mnode->EvalWorldState(RefFrame);
//if (os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) return;
/*
if (obj->SuperClassID()!=GEOMOBJECT_CLASS_ID) return;

//BOOL needDel;
//NullView nullView;
//Mesh *mesh = ((GeomObject*)os.obj)->GetRenderMesh(RefFrame,mnode,nullView,needDel);
el1 = BIGFLOAT;
el2 = BIGFLOAT;
//if (!mesh) return;


//tranform x,y,z into object space



Matrix3 tm = bnode->GetObjectTM(RefFrame);

//tm = tm * Inverse(BaseTM);

l1 = l1 * tm;
l2 = l2 * tm;

Point3 p_edge[4];
GetCrossSectionLocal(l1, Normalize(l2-l1), 1.0f, p_edge);

///intersect axis  rays with mesh;
Ray ray;
Point3 norm;
float at;

float dist = 0.0f;
int ct = 0;
	
// See if we hit the object
for (int i=0; i < 4; i++)
	{
	ray.p   = l1 + Normalize(p_edge[i]-l1) * 50000.0f;
	ray.dir = -Normalize(p_edge[i]-l1);
	BOOL hit = FALSE;
	while  (obj->IntersectRay(RefFrame,ray,at,norm)) 
		{
		Point3 tp = ray.dir * (at+0.01f) ;
		ray.p = ray.p + tp;
		hit = TRUE;
		}
	if (hit)
		{
		dist += Length(ray.p - l1);
		ct++;
		}
	}

//if (ct == 0)
//el1 = Length(l2-l1) *.1f;
if (ct!=0) el1 = dist/(float) ct;


dist = 0.0f;
int ct2 = 0;
GetCrossSectionLocal(l2, Normalize(l2-l1), 1.0f, p_edge);
ct2 = 0;	
// See if we hit the object
for (i=0; i < 4; i++)
	{
	ray.p   = l2 + Normalize(p_edge[i]-l2) * 50000.0f;
	ray.dir = -Normalize(p_edge[i]-l2);
	BOOL hit = FALSE;
	while  (obj->IntersectRay(RefFrame,ray,at,norm)) 
		{
		Point3 tp = ray.dir * (at+0.01f) ;
		ray.p = ray.p + tp;
		hit = TRUE;
		}
	if (hit)
		{
		dist += Length(ray.p - l2);
		ct2++;
		}
	}

//if (ct == 0)
//el2 = Length(l2-l1) *.1f;
if (ct2!=0) el2 = dist/(float) ct2;

if ((ct==0) && (ct2!=0))
	el1 = el2;
else if ((ct!=0) && (ct2==0))
	el2 = el1;
else if ((ct==0) && (ct2==0))
	{
	el1 = Length(l2-l1) *.5f;
	el2 = el1;
	}

*/

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
//	if (mc && mc->tm) 
//		tm = Inverse(*(mc->tm));
	if (inode) 
//		tm = tm * inode->GetObjTMBeforeWSM(t,&iv);
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
}

void BonesDefMod::ClearBoneEndPointSelections()
{

for (int i = 0; i <BoneData.Count(); i++)
	{
	if (BoneData[i].Node != NULL)
		{
		BoneData[i].end1Selected = FALSE;
		BoneData[i].end2Selected = FALSE;
/*		bmd->sel[ct++] = FALSE;
		bmd->sel[ct++] = FALSE;
		for (int j = 0; j <BoneData[i].CrossSectionList.Count(); j++)
			ct += 8;
*/

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
//		ct++;
//		ct++;
		for (int j = 0; j <BoneData[i].CrossSectionList.Count(); j++)
			{
			BoneData[i].CrossSectionList[j].innerSelected = FALSE;
			BoneData[i].CrossSectionList[j].outerSelected = FALSE;
//			for (int k = 0; k <8; k++)
//			sel[ct++] =FALSE;
			}

		}
	}
ModeBoneEnvelopeIndex = -1;
ip->RedrawViews(ip->GetTime());

}


void BonesDefMod::SelectFlexibleVerts(BoneModData *bmd)
{
for (int i =0;i<bmd->VertexDataCount;i++)
	{
	bmd->selected.Set(i,FALSE);
	for (int j =0; j <bmd->VertexData[i]->d.Count();j++)
		{
		if (bmd->VertexData[i]->d[j].Bones==ModeBoneIndex)
			{
			if (bmd->VertexData[i]->d[j].Influences != 1.0f)
				bmd->selected.Set(i,TRUE);
//				sel[i] = TRUE;
			}
		}
	}
NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//ip->RedrawViews(mod->ip->GetTime());

}
void BonesDefMod::BuildFalloff(BoneModData *bmd)
{
for (int i =0;i<bmd->VertexDataCount;i++)
	{
	if (bmd->selected[i])
		{
		float f1 = 999999999.9f, f2 = 999999999.9f;
		BOOL found1 = FALSE,found2 = FALSE;
//find closest non selected red vert
		for (int j =0;j<bmd->VertexDataCount;j++)
			{
			bmd->VertexData[i]->modified = TRUE;
			if (!bmd->selected[j])
				{
				for (int k =0;k<bmd->VertexData[j]->d.Count();k++)
					{
					if ( (bmd->VertexData[j]->d[k].Bones == ModeBoneIndex) &&
						 (bmd->VertexData[j]->d[k].Influences == 1.0f) )
						{
						float dist = Length(bmd->VertexData[j]->LocalPos - bmd->VertexData[i]->LocalPos);
						if (dist < f1 ) 
							{
							f1 = dist;
							found1 = TRUE;
							}
						}
					}

				}
			}
//find closest non selected no influence vert
		for (j =0;j<bmd->VertexDataCount;j++)
			{
			BOOL tfound = FALSE;
			if (!bmd->selected[j])
				{

				for (int k =0;k<bmd->VertexData[j]->d.Count();k++)
					{
					if ( (bmd->VertexData[j]->d[k].Bones == ModeBoneIndex) &&
						 (bmd->VertexData[j]->d[k].Influences == 0.0f) )
						{
						float dist = Length(bmd->VertexData[j]->LocalPos - bmd->VertexData[i]->LocalPos);
						if (dist < f2 ) 
							{
							f2 = dist;
							found2 = TRUE;
							}
						}
					if ( (bmd->VertexData[j]->d[k].Bones == ModeBoneIndex) &&
						 (bmd->VertexData[j]->d[k].Influences != 0.0f) )
						 tfound = TRUE;

					}
				if (!tfound)
					{
					float dist = Length(bmd->VertexData[j]->LocalPos - bmd->VertexData[i]->LocalPos);
					if (dist < f2 ) 
						{
						f2 = dist;
						found2 = TRUE;
						}

					}

				}
			}
		if ((found1) && (found2))
			{
			float influ = f2/(f1+f2);
			ComputeFalloff(influ,BoneData[ModeBoneIndex].FalloffType);

			BOOL tfound = false;
			for (int k =0;k<bmd->VertexData[i]->d.Count();k++)
				{
				if (bmd->VertexData[i]->d[k].Bones == ModeBoneIndex)
					{
					tfound = TRUE;
					bmd->VertexData[i]->d[k].Influences = influ;
//					bmd->VertexData[i]->d[k].normalizedInfluences = -1.0f;//FIX?

					k = bmd->VertexData[i]->d.Count();
					}
				}
			if (!tfound)
				{

				VertexInfluenceListClass td;
				td.Bones = ModeBoneIndex;
				td.Influences = influ;
//				td.normalizedInfluences = -1.0f;//FIX?
				bmd->VertexData[i]->d.Append(1,&td,1);

				}
//now rebablance
			float remainder = 1.0f - influ;
			if (bmd->VertexData[i]->d.Count() > 1)
				{
				remainder = remainder /(bmd->VertexData[i]->d.Count()-1);
				for (k =0;k<bmd->VertexData[i]->d.Count();k++)
					{
					if (bmd->VertexData[i]->d[k].Bones != ModeBoneIndex)
						{
						bmd->VertexData[i]->d[k].Influences = remainder;
//						bmd->VertexData[i]->d[k].normalizedInfluences = -1.0f;//FIX?
						}
					}
				}
			else if (bmd->VertexData[i]->d.Count() == 1 )
				{
				if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG) 
					{
					bmd->VertexData[i]->d[0].Influences = 1.0f;
//					bmd->VertexData[i]->d[0].normalizedInfluences = 1.0f;//FIX?
					}

				}



			}
		}
	}

NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);


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


if (ModeBoneIndex != -1)
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


	if ( (ssel>=0) && (ip && ip->GetSubObjectLevel() == 1) )

		{


		ObjectState os;

		os = inode->EvalWorldState(t);
//loop through points checking for selection and then marking for points
		if (FilterVertices == 0)
			{
			Interval iv;
			Matrix3 atm = inode->GetObjTMAfterWSM(t,&iv);
			Matrix3 ctm = inode->GetObjectTM(t,&iv);


			BOOL isWorldSpace = FALSE;

			if ((atm.IsIdentity()) && (ip->GetShowEndResult ()))
				isWorldSpace = TRUE;


			for (int i=0;i<bmd->VertexDataCount;i++)
				{
				if ((flags&HIT_SELONLY   &&  bmd->selected[i]) ||
					(flags&HIT_UNSELONLY && !bmd->selected[i]) ||
					!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
					{
					if (!bmd->autoInteriorVerts[i])
						{
						Point3 pt;
						gw->clearHitCode();
						if (isWorldSpace)
							pt = os.obj->GetPoint(i) * Inverse(ctm);
						else pt = os.obj->GetPoint(i);
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
		int ct = bmd->VertexDataCount;
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
//					GetEndPoints(bmd, t,pta, ptb, i);

					GetEndPointsLocal(bmd, t,pta, ptb, i);

//					Interval valid;
//					Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(t,&valid);

//					pta = pta * ntm  * bmd->InverseBaseTM;
//					ptb = ptb * ntm  * bmd->InverseBaseTM;


/*					if ((flags&HIT_SELONLY   &&  sel[ct]) ||
						(flags&HIT_UNSELONLY && !sel[ct]) ||
						!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) */

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
						if (gw->checkHitCode()) {

							vpt->LogHit(inode, mc, gw->getHitDistance(), ct,  
								new BoneHitDataClass(-1,i,0,-1,-1)); 
							res = 1;
							}
						}
					if ((flags&HIT_SELONLY   &&  BoneData[i].end2Selected) ||
						(flags&HIT_UNSELONLY && !BoneData[i].end2Selected) ||
						!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 

/*					if ((flags&HIT_SELONLY   &&  sel[ct]) ||

						(flags&HIT_UNSELONLY && !sel[ct]) ||
						!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) */
						{

						gw->clearHitCode();
						gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
						Point3 pt;
						Interval v;

						Point3 invB;
						invB = (pta-ptb) *.1f;
						
						ptb += invB;



						gw->marker(&ptb,POINT_MRKR);
						if (gw->checkHitCode()) {

							vpt->LogHit(inode, mc, gw->getHitDistance(), ct,
								new BoneHitDataClass(-1,i,1,-1,-1)); 
								
							res = 1;
							}
						}
					}

//add in enevelope inner and outer
				if ((FilterEnvelopes == 0) && (i == ModeBoneIndex))
					{
					if (BoneData[i].flags & BONE_SPLINE_FLAG)
						{
						ObjectState os = BoneData[i].Node->EvalWorldState(t);
						pathOb = (ShapeObject*)os.obj;
						}
					Point3 l1,l2;
					Interval v;

//					GetEndPoints(bmd, t,l1, l2, i);

					GetEndPointsLocal(bmd, t,l1, l2, i);

					Interval valid;
					Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(t,&valid);

//					l1 = l1 * ntm  * bmd->InverseBaseTM;
//					l2 = l2 * ntm  * bmd->InverseBaseTM;


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
//							ept = pathOb->InterpCurve3D(t, 0,BoneData[i].CrossSectionList[j].u) * Inverse(BoneData[i].tm);
//							align = VectorTransform(Inverse(BoneData[i].tm),pathOb->TangentCurve3D(t, 0,BoneData[i].CrossSectionList[j].u));
							ept = pathOb->InterpCurve3D(t, 0,BoneData[i].CrossSectionList[j].u) * ntm  * bmd->InverseBaseTM;
							align = VectorTransform(ntm  * bmd->InverseBaseTM,pathOb->TangentCurve3D(t, 0,BoneData[i].CrossSectionList[j].u));
							float Inner,Outer;
							BoneData[i].CrossSectionList[j].InnerControl->GetValue(0,&Inner,v);
							BoneData[i].CrossSectionList[j].OuterControl->GetValue(0,&Outer,v);
							GetCrossSection(ept, align, Inner,BoneData[i].temptm, p_edge);
							GetCrossSection(ept, align, Outer,BoneData[i].temptm, &p_edge[4]);

							}
						else
							{
//							align = (BoneData[i].l2-BoneData[i].l1);
//							ept = BoneData[i].l1;
							align = (l2-l1);
							ept = l1;
							float Inner,Outer;
							BoneData[i].CrossSectionList[j].InnerControl->GetValue(0,&Inner,v);
							BoneData[i].CrossSectionList[j].OuterControl->GetValue(0,&Outer,v);


							ept = ept + nvec * BoneData[i].CrossSectionList[j].u;
							GetCrossSection(ept, align, Inner,BoneData[i].temptm, p_edge);
							GetCrossSection(ept, align, Outer,BoneData[i].temptm, &p_edge[4]);
//							GetCrossSection(ept, align, BoneData[i].CrossSectionList[j].Inner,BoneData[i].temptm, p_edge);
//							GetCrossSection(ept, align, BoneData[i].CrossSectionList[j].Outer,BoneData[i].temptm, &p_edge[4]);

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



int BonesDefMod::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flagst, ModContext *mc)
	{

	if (inode)
		{
		if (ip)
			{
			int nodeCount = ip->GetSelNodeCount();
			BOOL found = FALSE;
			for (int nct =0; nct < nodeCount; nct++)
				{
				if (inode == ip->GetSelNode(nct))
					{
					found = TRUE;
					nct = nodeCount;
					}
				}
			if (!found) return 0;
			}

		}


	BoneModData *bmd = (BoneModData *) mc->localData;

	if (!bmd) return 0;

	Interval iv;
	Matrix3 atm = inode->GetObjTMAfterWSM(t,&iv);
	Matrix3 ctm = inode->GetObjectTM(t,&iv);
	BOOL isWorldSpace = FALSE;
	if ((atm.IsIdentity()) && (ip->GetShowEndResult ()))
		isWorldSpace = TRUE;


	GraphicsWindow *gw = vpt->getGW();
	Point3 pt[4];
	Matrix3 tm = CompMatrix(t,inode,mc);
	int savedLimits;

//	obtm = tm;
//	iobtm = Inverse(tm);

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);

//get selected bone	
	int fsel;
	fsel = SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_GETCURSEL ,0,0);

	int tsel = ConvertSelectedListToBoneID(fsel);




	if ((tsel>=0) && (ip && ip->GetSubObjectLevel() == 1) )
		{
//draw 	gizmos
		if (pblock_gizmos->Count(skin_gizmos_list) > 0)
			{
			ReferenceTarget *ref;

			if (displayAllGizmos)
				{
				for (int currentGiz = 0; currentGiz < pblock_gizmos->Count(skin_gizmos_list); currentGiz++)
					{	
					if (currentSelectedGizmo == currentGiz)
						{
						Point3 gizColor = GetUIColor(COLOR_SEL_GIZMOS);
						float r = gizColor.x;
						float g = gizColor.y;
						float b = gizColor.z;
		
						gw->setColor(LINE_COLOR, r,g,b);
						}
					else
						{	
						Point3 gizColor = GetUIColor(COLOR_GIZMOS);
						float r = gizColor.x;
						float g = gizColor.y;
						float b = gizColor.z;
		
						gw->setColor(LINE_COLOR, r,g,b);
						}
					ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentGiz);
					GizmoClass *gizmo = (GizmoClass *)ref;
					if (gizmo)
						gizmo->Display(t,gw, Inverse(tm));

					}
				}
			else
				{
				if ((currentSelectedGizmo > 0) && (currentSelectedGizmo<pblock_gizmos->Count(skin_gizmos_list)))
					{
					ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,currentSelectedGizmo);
					GizmoClass *gizmo = (GizmoClass *)ref;
					if (gizmo)
						{
						Point3 gizColor = GetUIColor(COLOR_SEL_GIZMOS);
						float r = gizColor.x;
						float g = gizColor.y;
						float b = gizColor.z;
	
						gw->setColor(LINE_COLOR, r,g,b);
	
						gizmo->Display(t,gw, Inverse(tm));
						}
					}

				}

			}

		if (inPaint)
			{
			if (bmd->isHit)
				{
//draw 3d cursor
				Point3 x(1.0f,0.0f,0.0f),y(0.0f,1.0f,0.0f),z(0.0f,0.0f,1.0f);
				gw->setColor(LINE_COLOR, 1.0f,1.0f,0.0f);

				DrawCrossSectionNoMarkers(bmd->hitPoint, x, Radius, gw); // optimize these can be moved out of the loop
				DrawCrossSectionNoMarkers(bmd->hitPoint, y, Radius, gw); // optimize these can be moved out of the loop
				DrawCrossSectionNoMarkers(bmd->hitPoint, z, Radius, gw); // optimize these can be moved out of the loop

				}
			}
//		else
		{
		ObjectState os;

		os = inode->EvalWorldState(t);
//loop through points checking for selection and then marking
		float r,g,b;

		BOOL isPatch = FALSE;
		int knots = 0;
		PatchMesh *pmesh;
//		BitArray interiorVecs;

		Interval iv;
		Matrix3 atm = inode->GetObjTMAfterWSM(t,&iv);
		Matrix3 btm = inode->GetObjTMBeforeWSM(t,&iv);

		if (os.obj->IsSubClassOf(patchObjectClassID))
			{
			PatchObject *pobj = (PatchObject*)os.obj;
			pmesh = &(pobj->patch);
			isPatch = TRUE;
			knots = pmesh->numVerts;
			}


		for (int i=0;i<bmd->VertexDataCount;i++)
			{

			for (int j=0;j<bmd->VertexData[i]->d.Count();j++)
				{
				int rigidBoneID;
				if (rigidVerts)
					rigidBoneID = bmd->VertexData[i]->GetMostAffectedBone();
				if ((bmd->VertexData[i]->d[j].Bones == tsel) && (bmd->VertexData[i]->d[j].Influences != 0.0f) &&
					 (DrawVertices ==1) )
					
					{
					if (!((rigidVerts) && (tsel != rigidBoneID)))
						{
						Point3 pt;
						if (isWorldSpace)
							pt = os.obj->GetPoint(i) *Inverse(ctm);
						else pt = os.obj->GetPoint(i);
//					pt = bmd->VertexData[i]->LocalPos;
//gte red is strongest,green, blue is weakest based on influence
						float infl;
//					infl = VertexData[i].d[j].Influences;

						infl = RetrieveNormalizedWeight(bmd,i,j);
						if (rigidVerts) infl = 1.0f;
						Point3 selColor(0.0f,0.0f,0.0f);
						Point3 selSoft = GetUIColor(COLOR_SUBSELECTION_SOFT);
						Point3 selMedium = GetUIColor(COLOR_SUBSELECTION_MEDIUM);
						Point3 selHard = GetUIColor(COLOR_SUBSELECTION_HARD);

						if (infl > 0.0f)
							{
							if ( (infl<0.5) && (infl > 0.0f))
								{
								selColor = selSoft + ( (selMedium-selSoft) * (infl/0.5f));
//							r =0.0f;
//							g = 0.0f;
//							b = 1.0f;
								}
							else if (infl<1.0)
								{
								selColor = selMedium + ( (selHard-selMedium) * ((infl-0.5f)/0.5f));
//								r =0.0f;
//							g = 1.0f;
//							b = 0.0f;
								}
							else 
								{
								selColor = GetUIColor(COLOR_SUBSELECTION);
//								r =1.0f;
//							g = 0.0f;
//							b = 0.0f;
								}
							r = selColor.x;
							g = selColor.y;
							b = selColor.z;

							gw->setColor(LINE_COLOR, r,g,b);
							if (isPatch)
								{
								if (i< knots)
									{
									gw->marker(&pt,PLUS_SIGN_MRKR);
									if (bmd->selected[i] == FALSE)
										{
//it is a knot draw the handle
										PatchVert pv = pmesh->getVert(i);
										Point3 lp[3];
										lp[0] = pt;
										gw->setColor(LINE_COLOR, 0.8f,0.8f,0.8f);

										for (int vec_count = 0; vec_count < pv.vectors.Count(); vec_count++)
											{
											int	idv = pv.vectors[vec_count];
											if (isWorldSpace)
												lp[1] = pmesh->getVec(idv).p * Inverse(ctm);
											else lp[1] = pmesh->getVec(idv).p;
											gw->polyline(2, lp, NULL, NULL, 0);
		
											}
										}

									}
								else 
									{
									if (!bmd->autoInteriorVerts[i])
										gw->marker(&pt,SM_HOLLOW_BOX_MRKR);
									}
								}
							else gw->marker(&pt,PLUS_SIGN_MRKR);
							j = bmd->VertexData[i]->d.Count()+1;
							}
				
						}
					}	
				}
			if (bmd->selected[i] == TRUE) 
				{
				Point3 pt;
				if (isWorldSpace)
					pt = os.obj->GetPoint(i) *Inverse(ctm);
				else pt = os.obj->GetPoint(i);
				gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
				if (isPatch)
					{
					if (!bmd->autoInteriorVerts[i])
						gw->marker(&pt,HOLLOW_BOX_MRKR);
					}
				else gw->marker(&pt,HOLLOW_BOX_MRKR);

				if ((i< knots) && (isPatch))
					{
//it is a knot draw the handle
					PatchVert pv = pmesh->getVert(i);
					Point3 lp[3];
					lp[0] = pt;
					gw->setColor(LINE_COLOR, 0.8f,0.8f,0.8f);

					for (int vec_count = 0; vec_count < pv.vectors.Count(); vec_count++)
						{
						int idv = pv.vectors[vec_count];
						if (isWorldSpace)
							lp[1] = pmesh->getVec(idv).p * Inverse(ctm);
						else lp[1] = pmesh->getVec(idv).p;
						gw->polyline(2, lp, NULL, NULL, 0);

						}
					}
				else if (isPatch)
					{
					Point3 lp[3];
					lp[0] = pt;
					int vecIndex = i - knots;
					PatchVec pv = pmesh->getVec(vecIndex);
					int vertIndex = pv.vert;
					if ((!bmd->selected[vertIndex]) && (pv.flags != PVEC_INTERIOR))
						{
						if (isWorldSpace)
							lp[1] = pmesh->getVert(vertIndex).p * Inverse(ctm);
						else lp[1] = pmesh->getVert(vertIndex).p;
						gw->setColor(LINE_COLOR, 0.8f,0.8f,0.8f);
						gw->polyline(2, lp, NULL, NULL, 0);

						}
					}
				}
			else if (drawAllVertices)
				{
				Point3 pt;
				if (isWorldSpace)
					pt = os.obj->GetPoint(i) *Inverse(ctm);
				else pt = os.obj->GetPoint(i);
				gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
				if (isPatch)
					{
					if (!bmd->autoInteriorVerts[i])
						gw->marker(&pt,POINT_MRKR);
					}
				else gw->marker(&pt,POINT_MRKR);

				
				if ((i< knots) && (isPatch))
					{
//it is a knot draw the handle
					PatchVert pv = pmesh->getVert(i);
					Point3 lp[3];
					lp[0] = pt;
					gw->setColor(LINE_COLOR, 0.8f,0.8f,0.8f);

					for (int vec_count = 0; vec_count < pv.vectors.Count(); vec_count++)
						{
						int idv = pv.vectors[vec_count];
						if (isWorldSpace)
							lp[1] = pmesh->getVec(idv).p * Inverse(ctm);
						else lp[1] = pmesh->getVec(idv).p;
						gw->polyline(2, lp, NULL, NULL, 0);
	
						}
					}

					
				}

			if (tsel < bmd->exclusionList.Count() )
				{
				if (bmd->exclusionList[tsel])
					{
//					if ((*bmd->exclusionList[tsel])[i])
					if (bmd->isExcluded(tsel,i))
						{
						Point3 pt;
						if (isWorldSpace)
							pt = os.obj->GetPoint(i) *Inverse(ctm);
						else pt = os.obj->GetPoint(i);
						gw->setColor(LINE_COLOR, 0.2f,0.2f,0.2f);
						gw->marker(&pt,SM_DIAMOND_MRKR);

						}
					}
				}

//kinda sucks need to load this up so the paint has an array of world psace points to work with
//			bmd->VertexData[i]->LocalPos = os.obj->GetPoint(i);

			}

//draw selected bone

		for (i =0;i<BoneData.Count();i++)
			{
			if (BoneData[i].Node != NULL)
				{

				if (i== tsel)
					{
					r = 1.0f;
					g = 1.0f;
					b = 0.0f;
					Point3 l1,l2;
					Interval v;
					BoneData[i].EndPoint1Control->GetValue(0,&l1,v);
					BoneData[i].EndPoint2Control->GetValue(0,&l2,v);

					Interval valid;
					Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(t,&valid);

					Worldl1 = l1  * ntm;  
					Worldl2 = l2  * ntm;  

					}
				else
					{
					r = 0.3f;
					g = 0.3f;
					b = 0.3f;
					}

				Point3 pta, ptb;
				Point3 pta_tm,ptb_tm;
				Point3 plist[2];

				
				GetEndPointsLocal(bmd, t,pta, ptb, i);


				gw->setColor(LINE_COLOR, r,g,b);

				ObjectState os;
				ShapeObject *pathOb = NULL;

//				pta = (pta ) * bmd->tmCacheToObjectSpace[i];
//				ptb = (ptb ) * bmd->tmCacheToObjectSpace[i];


				if (BoneData[i].flags & BONE_SPLINE_FLAG)
					{


					ObjectState os = BoneData[i].Node->EvalWorldState(t);
					pathOb = (ShapeObject*)os.obj;

					float su = 0.0f;
					float eu = 0.1f;
					float inc = 0.1f;
					Point3 sp_line[10];
					
					Point3 l1,l2;

//196241 
					if (pathOb->NumberOfCurves() != 0) 
						{

						l1 = pathOb->InterpPiece3D(t, 0,0 ,0.0f ) * bmd->tmCacheToObjectSpace[i];
						l2 = pathOb->InterpPiece3D(t, 0,0 ,1.0f ) * bmd->tmCacheToObjectSpace[i];

						pta = l1;
						ptb = l2;

						plist[0] = pta;
						plist[1] = ptb;


						for (int cid = 0; cid < pathOb->NumberOfCurves(); cid++)
							{
							for (int sid = 0; sid < pathOb->NumberOfPieces(t,cid); sid++)
								{
								
								for (int spid = 0; spid < 4; spid++)
									{
									sp_line[spid] = pathOb->InterpPiece3D(t, cid,sid ,su ) * bmd->tmCacheToObjectSpace[i];  //optimize reduce the count here 
									su += inc;
									}
								gw->polyline(4, sp_line, NULL, NULL, 0);
	
								}
							}
						}
					}
				else
					{
					Point3 invA,invB;
	

					invA = (ptb-pta) *.1f;
					invB = (pta-ptb) *.1f;

					plist[0] = pta + invA;
					plist[1] = ptb + invB;

					gw->polyline(2, plist, NULL, NULL, 0);
					}

				if ((BoneData[i].end1Selected) && (i== tsel))
					gw->setColor(LINE_COLOR, 1.0f,0.0f,1.0f);
				else gw->setColor(LINE_COLOR, .3f,.3f,0.3f);


				gw->marker(&plist[0],BIG_BOX_MRKR);

				if ((BoneData[i].end2Selected) && (i== tsel))
					gw->setColor(LINE_COLOR, 1.0f,0.0f,1.0f);
				else gw->setColor(LINE_COLOR, .3f,.3f,0.3f);

				gw->marker(&plist[1],BIG_BOX_MRKR);

//Draw Cross Sections
				Tab<Point3> CList;
				Tab<float> InnerList, OuterList;

				for (int ccount = 0; ccount < BoneData[i].CrossSectionList.Count();ccount++)
					{
					Point3 m;
					float inner;
					float outer;
					Interval v;
					BoneData[i].CrossSectionList[ccount].InnerControl->GetValue(0,&inner,v);
					BoneData[i].CrossSectionList[ccount].OuterControl->GetValue(0,&outer,v);

					GetCrossSectionRanges(inner, outer, i, ccount);

					if (tsel == i)
						{
						gw->setColor(LINE_COLOR, 1.0f,0.0f,0.0f);
						if ( (ModeBoneEnvelopeIndex == ccount) && (ModeBoneEnvelopeSubType<4))
							gw->setColor(LINE_COLOR, 1.0f,0.0f,1.0f);
						}

					Point3 nvec;
					Matrix3 rtm;
					if ((DrawEnvelopes ==1) || (tsel == i) || (BoneData[i].flags & BONE_DRAW_ENVELOPE_FLAG))
						{
						Point3 vec;

						InnerList.Append(1,&inner,1);
						OuterList.Append(1,&outer,1);

//196241 
						if ((pathOb) && (pathOb->NumberOfCurves() != 0) &&(BoneData[i].flags & BONE_SPLINE_FLAG))
							{
							vec = pathOb->InterpCurve3D(t, 0,BoneData[i].CrossSectionList[ccount].u) * bmd->tmCacheToObjectSpace[i]; 

							CList.Append(1,&(vec),1);
							nvec = VectorTransform(bmd->tmCacheToObjectSpace[i],pathOb->TangentCurve3D(t, 0,BoneData[i].CrossSectionList[ccount].u));
							DrawCrossSection(vec, nvec, inner, BoneData[i].temptm, gw);  // optimize these can be moved out of the loop
							}
						else
							{
							nvec = (ptb-pta);
							vec = nvec * BoneData[i].CrossSectionList[ccount].u;
							CList.Append(1,&(pta+vec),1);
							DrawCrossSection(pta+vec, nvec, inner, BoneData[i].temptm, gw); // optimize these can be moved out of the loop
							}

						}
					if (tsel == i)
						{	
						gw->setColor(LINE_COLOR, 0.5f,0.0f,0.0f);
						if ( (ModeBoneEnvelopeIndex == ccount) && (ModeBoneEnvelopeSubType>=4))
							gw->setColor(LINE_COLOR, 1.0f,0.0f,1.0f);
						}

					if ((DrawEnvelopes ==1) || (tsel == i) || (BoneData[i].flags & BONE_DRAW_ENVELOPE_FLAG))
						{
						Point3 vec;
//196241 
						if ((pathOb) && (pathOb->NumberOfCurves() != 0) && (BoneData[i].flags & BONE_SPLINE_FLAG))
							{
							vec = pathOb->InterpCurve3D(t, 0,BoneData[i].CrossSectionList[ccount].u) * bmd->tmCacheToObjectSpace[i];
							nvec = VectorTransform(bmd->tmCacheToObjectSpace[i],pathOb->TangentCurve3D(t, 0,BoneData[i].CrossSectionList[ccount].u));  // optimize these can be moved out of the loop
							DrawCrossSection(vec, nvec, outer, BoneData[i].temptm, gw);

							}
						else
							{

							nvec = (ptb-pta);
							vec = nvec * BoneData[i].CrossSectionList[ccount].u;
							DrawCrossSection(pta+vec, nvec, outer, BoneData[i].temptm, gw);
							}

						}
					}

				if ((DrawEnvelopes ==1) || (tsel == i) || (BoneData[i].flags & BONE_DRAW_ENVELOPE_FLAG))
					{
					if (!(BoneData[i].flags & BONE_SPLINE_FLAG))
						{
						gw->setColor(LINE_COLOR, 1.0f,0.0f,0.0f);

						DrawEnvelope(CList, InnerList, CList.Count(), BoneData[i].temptm,  gw);
						gw->setColor(LINE_COLOR, 0.5f,0.0f,0.0f);
						DrawEnvelope(CList, OuterList, CList.Count(), BoneData[i].temptm,  gw);
						}
					}
				}
			}

		}


	}
	gw->setRndLimits(savedLimits);

	return 0;

	}




void BonesDefMod::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, 
		Box3& box, ModContext *mc)
	{

	BoneModData *bmd = (BoneModData *) mc->localData;

	if (!bmd) return ;



//	Matrix3 tm = CompMatrix(t,inode,mc); ns
	Point3 pt1, pt2;
	box.Init();

	if (ModeEdit == 1)
		{
		Interval iv;
		if ( (ModeBoneEndPoint == 0) || (ModeBoneEndPoint == 1))
			{
			p1Temp->GetValue(t,&pt1,FOREVER,CTRL_ABSOLUTE);
			box += pt1;// * BoneData[ModeBoneIndex].temptm * tm;
			}
		}	

	float l = 0.0f;
	for (int i =0;i<BoneData.Count();i++)
		{
		if (BoneData[i].Node != NULL)
			{


			Interval valid;
			Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(t,&valid);
//			pta = pta * ntm  * bmd->InverseBaseTM;
//			ptb = ptb * ntm  * bmd->InverseBaseTM;


			Point3 pta;
			Interval v;
			BoneData[i].EndPoint1Control->GetValue(0,&pta,v);

			box += pta* ntm;//  * bmd->InverseBaseTM;;// * BoneData[i].temptm * tm;
			BoneData[i].EndPoint2Control->GetValue(0,&pta,v);
			box += pta* ntm;//  * bmd->InverseBaseTM;;//  * BoneData[i].temptm * tm;
			for (int k = 0; k < BoneData[i].CrossSectionList.Count();k++)
				{
				float outer;
				Interval v;
				BoneData[i].CrossSectionList[k].OuterControl->GetValue(0,&outer,v);

				if (outer > l) l = outer;
				}
			}
		}
	box.EnlargeBy(l+10.0f);  // this is a fudge since I am using large tick boxes


	for(int  j = 0 ; j < bmd->gizmoData.Count() ; j++)
		{
		int id = bmd->gizmoData[j]->whichGizmo;
		ReferenceTarget *ref;
		ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,id);
		GizmoClass *gizmo = (GizmoClass *)ref;
		if (gizmo)
			gizmo->GetWorldBoundBox(t,inode, vpt, box, mc);
			
		}

	
	}


void BonesDefMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{

//check of points
//check for envelopes

	if ( !ip ) return;

	ModContextList mcList;		
	INodeTab nodes;

	ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	for ( int i = 0; i < objects; i++ ) 
		{
		BoneModData *bmd = (BoneModData*)mcList[i]->localData;

		int mode = 1;
		if (mode >0 )
			{
			ModeEdit = 1;
			val = VectorTransform(tmAxis*Inverse(partm),val);



			if (ModeBoneEndPoint == 0)
				{
				val = VectorTransform(bmd->tmCacheToBoneSpace[ModeBoneIndex],val);

				bmd->CurrentCachePiece = -1;
				BoneData[ModeBoneIndex].EndPoint1Control->SetValue(0,&val,TRUE,CTRL_RELATIVE);

				int tempID = ModeBoneIndex;
				tempID = ConvertSelectedBoneToListID(tempID)+1;
				Interval iv;
				BoneData[ModeBoneIndex].EndPoint1Control->GetValue(0,&val,iv,CTRL_ABSOLUTE);
				macroRecorder->FunctionCall(_T("skinOps.setStartPoint"), 3, 0, mr_reftarg, this, 
																			 mr_int, tempID,
																			 mr_point3,&val);


//				Interval iv;
//				BoneData[ModeBoneIndex].EndPoint1Control->GetValue(0,&bmd->localCenter,iv);
//				bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];

				}
			else if (ModeBoneEndPoint == 1)
				{
				val = VectorTransform(bmd->tmCacheToBoneSpace[ModeBoneIndex],val);

				bmd->CurrentCachePiece = -1;
				BoneData[ModeBoneIndex].EndPoint2Control->SetValue(0,&val,TRUE,CTRL_RELATIVE);

				int tempID = ModeBoneIndex;
				tempID = ConvertSelectedBoneToListID(tempID)+1;
				Interval iv;
				BoneData[ModeBoneIndex].EndPoint2Control->GetValue(0,&val,iv,CTRL_ABSOLUTE);
				macroRecorder->FunctionCall(_T("skinOps.setEndPoint"), 3, 0, mr_reftarg, this, 
																			 mr_int, tempID,
																			 mr_point3,&val);


//				Interval iv;
//				BoneData[ModeBoneIndex].EndPoint2Control->GetValue(0,&bmd->localCenter,iv);

//				bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];

				}

			else
				{
				if ((ModeBoneEnvelopeIndex != -1) && (ModeBoneEnvelopeSubType != -1))
					{

					val = VectorTransform(bmd->tmCacheToBoneSpace[ModeBoneIndex],val);
					p1Temp->SetValue(0,val,TRUE,CTRL_RELATIVE);

					Interval v;
//					p1Temp->GetValue(0,bmd->localCenter,v);
//					bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];


					ObjectState os;
					ShapeObject *pathOb = NULL;
					Point3 nvec;
					Point3 vec;
					Point3 lp;
					Point3 l1,l2;
					BoneData[ModeBoneIndex].EndPoint1Control->GetValue(0,&l1,v);
					BoneData[ModeBoneIndex].EndPoint2Control->GetValue(0,&l2,v);


					Point3 p(0.0f,0.0f,0.0f);
					Interval iv = FOREVER;
					p1Temp->GetValue(0,&p,iv);
					 
//					p = val;


					if (BoneData[ModeBoneIndex].flags & BONE_SPLINE_FLAG)
						{
						ObjectState os = BoneData[ModeBoneIndex].Node->EvalWorldState(t);
						pathOb = (ShapeObject*)os.obj;
//						lp = pathOb->InterpCurve3D(t, 0,BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].u) * Inverse(BoneData[ModeBoneIndex].tm);					
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
						lp = lp * Inverse(bmd->tmCacheToBoneSpace[ModeBoneIndex]);
						p = p * Inverse(bmd->tmCacheToBoneSpace[ModeBoneIndex]);
						float inner = Length(lp-p);

						BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl->SetValue(0,&inner,TRUE,CTRL_ABSOLUTE);
						pblock_param->SetValue(skin_cross_radius,0,inner);

						int tempID = ModeBoneIndex;
						tempID = ConvertSelectedBoneToListID(tempID)+1;
						macroRecorder->FunctionCall(_T("skinOps.setInnerRadius"), 4, 0, mr_reftarg, this, mr_int, tempID,mr_int,ModeBoneEnvelopeIndex+1, mr_float,inner);

						}
					else if (ModeBoneEnvelopeSubType<8)
						{
						lp = lp * Inverse(bmd->tmCacheToBoneSpace[ModeBoneIndex]);
						p = p * Inverse(bmd->tmCacheToBoneSpace[ModeBoneIndex]);

						float outer = Length(lp-p);
	
						int tempID = ModeBoneIndex;
						tempID = ConvertSelectedBoneToListID(tempID)+1;
						BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl->SetValue(0,&outer,TRUE,CTRL_ABSOLUTE);
						pblock_param->SetValue(skin_cross_radius,0,outer);

						macroRecorder->FunctionCall(_T("skinOps.setOuterRadius"), 4, 0, mr_reftarg, this, mr_int, tempID, mr_int,ModeBoneEnvelopeIndex+1, mr_float,outer);


						}
					}

				}

			}


//move the right controller		
		}

	}

void BonesDefMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{
	BoneModData *bmd = (BoneModData *) mc->localData;

	if (!bmd) return; 

	Matrix3 tm = CompMatrix(t,node,mc);
	Point3 pt(0,0,0), p;
	if (ModeBoneEndPoint == 0)
		{
		Interval iv;
		BoneData[ModeBoneIndex].EndPoint1Control->GetValue(0,&bmd->localCenter,iv,CTRL_ABSOLUTE);
		bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];
		}
	else if (ModeBoneEndPoint == 1)
		{
		Interval iv;
		BoneData[ModeBoneIndex].EndPoint2Control->GetValue(0,&bmd->localCenter,iv,CTRL_ABSOLUTE);

		bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];
		}

	else
		{
		if ((ModeBoneEnvelopeIndex != -1) && (ModeBoneEnvelopeSubType != -1))
			{
			Interval v;
			p1Temp->GetValue(0,bmd->localCenter,v,CTRL_ABSOLUTE);
			bmd->localCenter = bmd->localCenter *bmd->tmCacheToObjectSpace[ModeBoneIndex];
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
	Matrix3 tm = CompMatrix(t,node,mc);
	cb->TM(tm,0);
	}


void BonesDefMod::UpdatePropInterface()

{

if ( (ModeBoneIndex >= 0) && (ModeBoneIndex < BoneData.Count()) )
	{
	if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
		{
		iAbsolute->SetCheck(FALSE);

//		pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
		}
	else
		{
		iAbsolute->SetCheck(TRUE);
//		pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);
		}


	if (BoneData[ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
		{
//		pblock_param->SetValue(PB_DRAW_BONE_ENVELOPE,0,1);
		iEnvelope->SetCheck(TRUE);
		}
	else
		{
//		pblock_param->SetValue(PB_DRAW_BONE_ENVELOPE,0,0);
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
	}


}

void BonesDefMod::UpdateP(BoneModData* bmd)
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
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl->GetValue(0,&inner,v);
BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl->GetValue(0,&outer,v);
GetCrossSection(ept, align, inner,
				BoneData[ModeBoneIndex].temptm,  p_edge);
GetCrossSection(ept, align, outer,
				BoneData[ModeBoneIndex].temptm,  &p_edge[4]);

if (ModeBoneEnvelopeSubType < 4)
	{
//	pblock_param->SetController(skin_cross_radius,0, BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl, FALSE);
	pblock_param->SetValue(skin_cross_radius,0,inner);
	BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].innerSelected = TRUE;

	}
else{
//	pblock_param->SetController(skin_cross_radius,0, BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl, FALSE);
	pblock_param->SetValue(skin_cross_radius,0,outer);
	BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].outerSelected = TRUE;
	}
Point3 p = p_edge[ModeBoneEnvelopeSubType] * bmd->BaseTM * Inverse(ntm);
bmd->localCenter = p_edge[ModeBoneEnvelopeSubType];


p1Temp->SetValue(0,p,TRUE,CTRL_ABSOLUTE);

//NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//ip->RedrawViews(ip->GetTime());

}


void BonesDefMod::SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
	{
	//if (theHold.Holding()) theHold.Put(new SelRestore(this));
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
		}
	int Count = 0;
	BOOL state = selected;

	BoneHitDataClass *bhd;

	int mode = -1;


	while (hitRec) {
		state = hitRec->hitInfo;
		BoneModData *bmd = (BoneModData*)hitRec->modContext->localData;
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
					ModeBoneEndPoint = -1;
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


			}
			else EnableEffect(FALSE);

	
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
//			EnableRadius(TRUE);



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
				BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl->GetValue(0,&inner,v);
				BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl->GetValue(0,&outer,v);

				GetCrossSection(ept, align, inner,
							BoneData[ModeBoneIndex].temptm,  p_edge);
				GetCrossSection(ept, align, outer,
							BoneData[ModeBoneIndex].temptm,  &p_edge[4]);

				if (ModeBoneEnvelopeSubType < 4)
					{
//					pblock_param->SetController(skin_cross_radius,0, BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].InnerControl, FALSE);
					pblock_param->SetValue(skin_cross_radius,0,inner);
					BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].innerSelected = TRUE;

					}
				else{
//					pblock_param->SetController(skin_cross_radius,0, BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].OuterControl, FALSE);
					pblock_param->SetValue(skin_cross_radius,0,outer);
					BoneData[ModeBoneIndex].CrossSectionList[ModeBoneEnvelopeIndex].outerSelected = TRUE;
					}
				p = p_edge[ModeBoneEnvelopeSubType] * bmd->BaseTM * Inverse(ntm);
				bmd->localCenter = p_edge[ModeBoneEnvelopeSubType];

				}
		
		
			if  (ModeBoneEnvelopeIndex == -1) EnableRadius(FALSE);
			else EnableRadius(TRUE);


			p1Temp->SetValue(0,p,TRUE,CTRL_ABSOLUTE);

		
//select in list box also 
			int rsel = ConvertSelectedBoneToListID(ModeBoneIndex);

			SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_SETCURSEL ,rsel,0);

/*			if (BoneData[ModeBoneIndex].flags & BONE_LOCK_FLAG)
				pblock_param->SetValue(PB_LOCK_BONE,0,1);
			else
				pblock_param->SetValue(PB_LOCK_BONE,0,0);
*/
	//		if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
	//			pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
	//		else
	//			pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);




			}

		UpdateEffectSpinner(bmd);

		int nset = bmd->selected.NumberSet();
		int total = bmd->selected.GetSize();

		NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
		}
	}



void BonesDefMod::UpdateEffectSpinner(BoneModData*bmd)
{
	if (bmd->selected.NumberSet() > 0)
//   if ((Count == 1) && (state<bmd->VertexDataCount))
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
						if (first)
							{
							v = RetrieveNormalizedWeight(bmd,i,ct);
//							v = bmd->VertexData[i]->d[ct].Influences;
							first = FALSE;
							}
						else if (v != RetrieveNormalizedWeight(bmd,i,ct))
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



		}

}

void BonesDefMod::ClearSelection(int selLevel)
	{
	//if (theHold.Holding()) theHold.Put(new SelRestore(this));

	if (selLevel == 1)
		{
		ModContextList mcList;		
		INodeTab nodes;

		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();


		for ( int i = 0; i < objects; i++ ) 
			{
			BoneModData *bmd = (BoneModData*)mcList[i]->localData;

			if (theHold.Holding() ) theHold.Put(new SelectionRestore(this,bmd));

			for (int i =0;i<bmd->selected.GetSize();i++)
				bmd->selected.Set(i,FALSE);

			UpdateEffectSpinner(bmd);

			NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
			}
		}
	}

void BonesDefMod::SelectAll(int selLevel)
	{
	if (selLevel == 1)
		{
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
		}
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

//	theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
	if (accept) theHold.SuperAccept(GetString(IDS_PW_WEIGHTCHANGE));
	else theHold.SuperCancel();
	return 1;
	}


void BonesDefMod::InvertSelection(int selLevel)
	{
	if (selLevel == 1)
		{
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
		}
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


				EnableWindow(GetDlgItem(hParam,IDC_FILTER_VERTICES_CHECK),TRUE);
				EnableWindow(GetDlgItem(hParam,IDC_FILTER_BONES_CHECK),TRUE);
				EnableWindow(GetDlgItem(hParam,IDC_FILTER_ENVELOPES_CHECK),TRUE);
				EnableWindow(GetDlgItem(hParam,IDC_DRAWALL_ENVELOPES_CHECK),TRUE);
				EnableWindow(GetDlgItem(hParam,IDC_DRAW_VERTICES_CHECK),TRUE);

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
			   

   
   


}

void BonesDefMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
//	static BOOL isAnimating;
	switch (level) {
		case 0:
/*			GetCOREInterface()->EnableAnimateButton(TRUE);
			if (isAnimating) 
				{
				AnimateOn();
				GetCOREInterface()->SetAnimateButtonState(TRUE);
				}
*/
			DisableButtons();


			if ((ip) && (ip->GetCommandMode() == PaintMode)) {
				ip->SetStdCommandMode(CID_OBJMOVE);
				return;
				}


			if ((ip) && (ip->GetCommandMode() == CrossSectionMode)) {
				ip->SetStdCommandMode(CID_OBJMOVE);
				return;
				}


			break;
		case 1: // Points
			{
/*			isAnimating = Animating();
			GetCOREInterface()->EnableAnimateButton(FALSE);
			GetCOREInterface()->SetAnimateButtonState(FALSE);
			AnimateOff();
*/
			int bct = 0;
			for (int i =0; i < BoneData.Count(); i ++)
				{
				if (!(BoneData[i].flags &  BONE_DEAD_FLAG)) bct++;
				}
//			if (BoneData.Count() > 0)
			if (bct > 0)
				{
				EnableButtons();
				}

			if (ModeBoneEnvelopeIndex != -1)
				EnableRadius(TRUE);
			else EnableRadius(FALSE);


			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;		
			}
		}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}


int BonesDefMod::SubNumToRefNum(int subNum)
	{
	return -1;
	}

int BonesDefMod::RemapRefOnLoad(int iref) 
	{
	if (ver < 4)
		{
		if (iref > 1)
			iref += 8;
		}
	return iref;
	}


RefTargetHandle BonesDefMod::GetReference(int i)

	{
	if (i==PBLOCK_PARAM_REF)
		{
		return (RefTargetHandle)pblock_param;
		}
	else if (i==PBLOCK_DISPLAY_REF)
		{
		return (RefTargetHandle)pblock_display;
		}
	else if (i==PBLOCK_GIZMOS_REF)
		{
		return (RefTargetHandle)pblock_gizmos;
		}
	else if (i==PBLOCK_ADVANCE_REF)
		{
		return (RefTargetHandle)pblock_advance;
		}
	else if (i == POINT1_REF)
		{
		return (RefTargetHandle)p1Temp;
		}
	else 
		{
//		int id = i -2;
//		RefTargetHandle t;
		for (int ct = 0; ct < BoneData.Count(); ct++)
			{
			if (i == BoneData[ct].BoneRefID)
				{
				return (RefTargetHandle)BoneData[ct].Node;
				}
			else if (i == BoneData[ct].RefEndPt1ID)
				{
				return (RefTargetHandle)BoneData[ct].EndPoint1Control;
				}
			else if (i == BoneData[ct].RefEndPt2ID)
				{
				return (RefTargetHandle)BoneData[ct].EndPoint2Control;
				}
			else
				{
				for (int j=0;j<BoneData[ct].CrossSectionList.Count();j++)
					{
					if (i == BoneData[ct].CrossSectionList[j].RefInnerID)
						{
						return (RefTargetHandle)BoneData[ct].CrossSectionList[j].InnerControl;
						}
					else if (i == BoneData[ct].CrossSectionList[j].RefOuterID)
						{
						return (RefTargetHandle)BoneData[ct].CrossSectionList[j].OuterControl;
						}

					}
				}
			}
		
//		else return NULL;
		}
	return NULL;
	}

void BonesDefMod::SetReference(int i, RefTargetHandle rtarg)
	{
	if (i==PBLOCK_PARAM_REF)
		{
		pblock_param = (IParamBlock2*)rtarg;
		}
	else if (i==PBLOCK_DISPLAY_REF)
		{
		pblock_display = (IParamBlock2*)rtarg;
		}
	else if (i==PBLOCK_GIZMOS_REF)
		{
		pblock_gizmos = (IParamBlock2*)rtarg;
		}
	else if (i==PBLOCK_ADVANCE_REF)
		{
		pblock_advance = (IParamBlock2*)rtarg;
		}
	else if (i == POINT1_REF)
		{

		p1Temp     = (Control*)rtarg; 
		}
	else 
		{
//		int id = i -2;
		for (int ct = 0; ct < BoneData.Count(); ct++)
			{
			if (i == BoneData[ct].BoneRefID)
				{
				BoneData[ct].Node = (INode*)rtarg;
				// Recalculate the Bonemap, since the BoneData has changed ! ns
				recompBoneMap = true;
				}
			if (i == BoneData[ct].RefEndPt1ID)
				{
				BoneData[ct].EndPoint1Control = (Control*)rtarg;
				}
			if (i == BoneData[ct].RefEndPt2ID)
				{
				BoneData[ct].EndPoint2Control = (Control*)rtarg;

				}
			for (int j=0;j<BoneData[ct].CrossSectionList.Count();j++)
				{
				if (i == BoneData[ct].CrossSectionList[j].RefInnerID)
					{

					BoneData[ct].CrossSectionList[j].InnerControl  = (Control*)rtarg ;
					}
				if (i == BoneData[ct].CrossSectionList[j].RefOuterID)
					{
					BoneData[ct].CrossSectionList[j].OuterControl  = (Control*)rtarg ;
					}

				}
			}
		}
/*
		{

		int id = i-2;
		BoneData[id].Node = (INode*)rtarg;
		}
*/

	}

TSTR BonesDefMod::SubAnimName(int i)
	{
	return _T("");
	}

RefResult BonesDefMod::NotifyRefChanged(
		Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message)
	{
	int i;
	Interface *tip;


	switch (message) {
		case REFMSG_CHANGE:
			if ((editMod==this) && (hTarget == pblock_param))
				{
				ParamID changing_param = pblock_param->LastNotifyParamID();
				skin_param_blk.InvalidateUI(changing_param);
				}
			else if ((editMod==this) && (hTarget == pblock_display))
				{
				ParamID changing_param = pblock_display->LastNotifyParamID();
				skin_display_blk.InvalidateUI(changing_param);
				}
			else if ((editMod==this) && (hTarget == pblock_gizmos))
				{
				ParamID changing_param = pblock_gizmos->LastNotifyParamID();
				skin_gizmos_blk.InvalidateUI(changing_param);
				}
			else if ((editMod==this) && (hTarget == pblock_advance))
				{
				ParamID changing_param = pblock_advance->LastNotifyParamID();
				skin_advance_blk.InvalidateUI(changing_param);
				}



			
			tip = GetCOREInterface();
			if (tip != NULL)
				{
				for (i =0;i<BoneData.Count();i++)
//				for (i =0;i<MAX_NUMBER_BONES;i++)
					{
					if ((BoneData[i].Node != NULL) && 
						(BoneData[i].Node == hTarget) && 
						(tip->GetTime() == RefFrame) )
						{
						BoneMoved = TRUE;
						}
					if ((BoneData[i].Node != NULL) && 
						(BoneData[i].Node == hTarget)  
						)
						{
//check if bone was spline 
						if (BoneData[i].flags & BONE_SPLINE_FLAG)
							{
							splineChanged = TRUE;
							whichSplineChanged = i;
							}

						}

					}
				}

			break;

/*		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_EFFECT:	 gpd->dim = stdNormalizedDim; break;
				case PB_REF_FRAME:	 gpd->dim = stdNormalizedDim; break;
				}
			return REF_STOP; 
			}


		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;			
			switch (gpn->index) {

				case PB_EFFECT: gpn->name = GetString(IDS_RB_EFFECT); break;
				}
			return REF_STOP; 
			}
*/
		case REFMSG_TARGET_DELETED: {
				for (int j =0;j<BoneData.Count();j++)
					{
					if (hTarget==BoneData[j].Node) 
						{
						RemoveBone(j);
						}
							
					}
				break;
				}

		}
	return REF_SUCCEED;
	}


class BonesDefModPostLoad : public PostLoadCallback {
	public:
		BonesDefMod *n;
		BonesDefModPostLoad(BonesDefMod *ns) {n = ns;}
		void proc(ILoad *iload) {  

			for (int  i = 0; i < n->pblock_gizmos->Count(skin_gizmos_list); i++) 
				{
				ReferenceTarget *ref;
				ref = n->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,i);
				GizmoClass *gizmo = (GizmoClass *)ref;
				gizmo->bonesMod = n;
				}

			delete this; 


			} 
	};


IOResult BonesDefMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	IOResult res = IO_OK;
	int NodeID = 0;
	

//	VertexData= NULL;
/* FIX THIS move to local mod load
	for (int i = 0; i< bmd->VertexData.Count(); i++)
		{
		if (bmd->VertexData[i] != NULL)
			delete (bmd->VertexData[i]);
		bmd->VertexData[i] = NULL;
		}

	bmd->VertexData.ZeroCount();
	sel.ZeroCount();

*/
//	BoneData.ZeroCount();
	BoneData.New();
	int currentvt = -1;
	ULONG nb;
	int bonecount = 0;
	int MatrixCount = 0;
	reloadSplines = TRUE;

	int bct = 0;

	ver = 3;
	while (IO_OK==(res=iload->OpenChunk())) {
		int id = iload->CurChunkID();
		switch(id)  {

			case BASE_TM_CHUNK: 
				{
				OldBaseTM.Load(iload);
				OldInverseBaseTM = Inverse(OldBaseTM);
				break;
				}
			case VER_CHUNK: 
				{
				
				iload->Read(&ver,sizeof(ver),&nb);
				break;
				}

				
			case BONE_COUNT_CHUNK: 
				{
				int c;
				iload->Read(&c,sizeof(c),&nb);
//				BoneData.SetCount(c);

				for (int i=0; i<c; i++)  
					{
					BoneDataClass t;
					BoneData.Append(t);
					BoneData[i].Node = NULL;
					BoneData[i].EndPoint1Control = NULL;
					BoneData[i].EndPoint2Control = NULL;
					BoneData[i].CrossSectionList.ZeroCount();
					}
				break;
				}
			case BONE_DATATM_CHUNK: 
				{
//				for (int i = 0; i < BoneData.Count(); i++)
				BoneData[MatrixCount++].tm.Load(iload);
				BoneData[MatrixCount-1].InitObjectTM = Inverse(BoneData[MatrixCount-1].tm); //ns
				recompInitTM = true;		//ns
				break;	
				}
			case BONE_NAME_CHUNK: 
				{
				NameTab names;
				int c = BoneData.Count();
				names.Load(iload);

				for (int i = 0; i < c; i++)
					{
					TSTR temp(names[i]);
					BoneData[i].name = temp;
					}

				break;	
				}

			case BONE_DATA_CHUNK: 
				{
				float load_f;
				Point3 load_p;
				int load_i;
				BYTE load_b;

				for (int i = 0; i < BoneData.Count(); i++)
					{
					iload->Read(&load_i,sizeof(load_i),&nb);
					BoneData[i].CrossSectionList.SetCount(load_i);
					for (int j=0;j<BoneData[i].CrossSectionList.Count();j++)
						{
//						iload->Read(&load_f,sizeof(load_f),&nb);
//						BoneData[i].CrossSectionList[j].Inner = load_f;
//						iload->Read(&load_f,sizeof(load_f),&nb);
//						BoneData[i].CrossSectionList[j].Outer = load_f;
						iload->Read(&load_f,sizeof(load_f),&nb);
						BoneData[i].CrossSectionList[j].u = load_f;
						iload->Read(&load_i,sizeof(load_i),&nb);
						BoneData[i].CrossSectionList[j].RefInnerID = load_i;
						iload->Read(&load_i,sizeof(load_i),&nb);
						BoneData[i].CrossSectionList[j].RefOuterID = load_i;

						BoneData[i].CrossSectionList[j].InnerControl = NULL;
						BoneData[i].CrossSectionList[j].OuterControl = NULL;

						BoneData[i].CrossSectionList[j].outerSelected = FALSE;
						BoneData[i].CrossSectionList[j].innerSelected = FALSE;
						BoneData[i].name.Resize(0);
						}
//					iload->Read(&load_p,sizeof(load_p),&nb);
//					BoneData[i].l1 = load_p;
//					iload->Read(&load_p,sizeof(load_p),&nb);
//					BoneData[i].l2 = load_p;
					
					iload->Read(&load_b,sizeof(load_b),&nb);
					BoneData[i].flags = load_b;

					iload->Read(&load_b,sizeof(load_b),&nb);
					BoneData[i].FalloffType = load_b;

					iload->Read(&load_i,sizeof(load_i),&nb);
					BoneData[i].BoneRefID = load_i;

					iload->Read(&load_i,sizeof(load_i),&nb);
					BoneData[i].RefEndPt1ID = load_i;

					iload->Read(&load_i,sizeof(load_i),&nb);
					BoneData[i].RefEndPt2ID = load_i;

					BoneData[i].end1Selected = FALSE;
					BoneData[i].end2Selected = FALSE;


					
					}

				break;
				}
			case BONE_SPLINE_CHUNK: 
				{
				reloadSplines = FALSE;
				for (int i = bct; i < BoneData.Count(); i++)
					{
					if (BoneData[i].flags & BONE_SPLINE_FLAG) 
						{
						BoneData[i].referenceSpline.Load(iload);
						bct= i+1;
						i = BoneData.Count();
						}
					}
				break;
				}



			case VERTEX_COUNT_CHUNK:
				{
				int c;
				iload->Read(&c,sizeof(c),&nb);
				OldVertexDataCount = c;
				OldVertexData.ZeroCount();
				OldVertexData.SetCount(c);
				for (int i=0; i<c; i++) {
					VertexListClass *vc;
					vc = new VertexListClass;
					OldVertexData[i] = vc;
					OldVertexData[i]->modified = FALSE;
					OldVertexData[i]->selected = FALSE;
 					OldVertexData[i]->d.ZeroCount();
					}

				break;

				}
			case VERTEX_DATA_CHUNK:
				{
				for (int i=0; i < OldVertexDataCount; i++)
					{
					int c;
					BOOL load_b;
					iload->Read(&c,sizeof(c),&nb);
					OldVertexData[i]->d.SetCount(c);

					iload->Read(&load_b,sizeof(load_b),&nb);
					OldVertexData[i]->modified = load_b;
					float load_f;
					int load_i;
					Point3 load_p;
					for (int j=0; j<c; j++) {
						iload->Read(&load_i,sizeof(load_i),&nb);
						iload->Read(&load_f,sizeof(load_f),&nb);
 						OldVertexData[i]->d[j].Bones = load_i;
						OldVertexData[i]->d[j].Influences =load_f;
//						OldVertexData[i]->d[j].normalizedInfluences = -1.0f;  //FIX?

						iload->Read(&load_i,sizeof(load_i),&nb);
						OldVertexData[i]->d[j].SubCurveIds =load_i;
						iload->Read(&load_i,sizeof(load_i),&nb);
						OldVertexData[i]->d[j].SubSegIds =load_i;

						iload->Read(&load_f,sizeof(load_f),&nb);
 						OldVertexData[i]->d[j].u = load_f;

						iload->Read(&load_p,sizeof(load_p),&nb);
 						OldVertexData[i]->d[j].Tangents = load_p;

						iload->Read(&load_p,sizeof(load_p),&nb);
 						OldVertexData[i]->d[j].OPoints = load_p;


						}
					}

				break;

				}
			case BONE_BIND_CHUNK: 
				{
				initialXRefTM.Load(iload);
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&bindNode);
					}
				break;
				}

			case DELTA_COUNT_CHUNK: 
				{
				int c;
				iload->Read(&c,sizeof(c),&nb);
				endPointDelta.SetCount(c);
				break;
				}

			case DELTA_DATA_CHUNK: 
				{
				for (int i = 0; i < endPointDelta.Count();i++)
					{
					Point3 p;
					iload->Read(&p,sizeof(p),&nb);
					endPointDelta[i] = p;
					}
				break;
				}



			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}	


//in R3 the bone and cross section ref ids followed right after the pblock and a point3 ref
//this gives us 8 blank spaces before them for r4 for addin more pblocks and other refs that 
//might be needed that are not dynamically allocated
	if (ver<4)
		{
		for (int i=0;i < BoneData.Count();i++)
			{
			BoneData[i].RefEndPt1ID+=8;
			BoneData[i].RefEndPt2ID+=8;
			BoneData[i].BoneRefID+=8;
			for (int j=0;j < BoneData[i].CrossSectionList.Count();j++)
				{
				BoneData[i].CrossSectionList[j].RefInnerID+=8;
				BoneData[i].CrossSectionList[j].RefOuterID+=8;
				}
			}

		}

		
//build reftable
	int ref_size = 0;
	for (int i=0;i < BoneData.Count();i++)
		{
		if (BoneData[i].RefEndPt1ID > ref_size) ref_size = BoneData[i].RefEndPt1ID;
		if (BoneData[i].RefEndPt2ID > ref_size) ref_size = BoneData[i].RefEndPt2ID;
		if (BoneData[i].BoneRefID > ref_size) ref_size = BoneData[i].BoneRefID;
		for (int j=0;j < BoneData[i].CrossSectionList.Count();j++)
			{
			if (BoneData[i].CrossSectionList[j].RefInnerID > ref_size) ref_size = BoneData[i].CrossSectionList[j].RefInnerID;
			if (BoneData[i].CrossSectionList[j].RefOuterID > ref_size) ref_size = BoneData[i].CrossSectionList[j].RefOuterID;

			}

		}
	RefTable.SetCount(ref_size+BONES_REF);
	for (i=0;i < RefTable.Count();i++)
		RefTable[i] = 0;
	for (i=0;i < BoneData.Count();i++)
		{
		if (BoneData[i].flags != BONE_DEAD_FLAG)
			{
			RefTable[BoneData[i].RefEndPt1ID-BONES_REF] = 1;
			RefTable[BoneData[i].RefEndPt2ID-BONES_REF] = 1;
			RefTable[BoneData[i].BoneRefID-BONES_REF] = 1; 
			for (int j=0;j < BoneData[i].CrossSectionList.Count();j++)
				{
				RefTable[BoneData[i].CrossSectionList[j].RefInnerID-BONES_REF] = 1;
				RefTable[BoneData[i].CrossSectionList[j].RefOuterID-BONES_REF] = 1;
				}
			}

		}


	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &skin_param_blk, this, PBLOCK_PARAM_REF);
	iload->RegisterPostLoadCallback(plcb);


	iload->RegisterPostLoadCallback(new BonesDefModPostLoad(this));

	return IO_OK;
	}

IOResult BonesDefMod::Save(ISave *isave)
	{
	Modifier::Save(isave);
	ULONG nb;


/*
	isave->BeginChunk(BASE_TM_CHUNK);
	bmd->BaseTM.Save(isave);
	isave->EndChunk();
*/

	int c = BoneData.Count();
	isave->BeginChunk(BONE_COUNT_CHUNK);
	isave->Write(&c,sizeof(c),&nb);
	isave->EndChunk();


//write bone chunks

	for (int i = 0; i < c; i++)
		{
		isave->BeginChunk(BONE_DATATM_CHUNK);
		BoneData[i].tm.Save(isave);
		isave->EndChunk();
		}
	
	isave->BeginChunk(BONE_DATA_CHUNK);
	for (i = 0; i < c; i++)
		{
		Point3 save_pt;
		float save_f;
		BYTE save_b;
		int save_i;

		save_i = BoneData[i].CrossSectionList.Count();
		isave->Write(&save_i,sizeof(save_i),&nb);

		for (int j = 0; j < BoneData[i].CrossSectionList.Count(); j++)
			{
//			save_f = BoneData[i].CrossSectionList[j].Inner;
//			isave->Write(&save_f,sizeof(save_f),&nb);
//			save_f = BoneData[i].CrossSectionList[j].Outer;
//			isave->Write(&save_f,sizeof(save_f),&nb);
			save_f = BoneData[i].CrossSectionList[j].u;
			isave->Write(&save_f,sizeof(save_f),&nb);

			save_i = BoneData[i].CrossSectionList[j].RefInnerID;
			isave->Write(&save_i,sizeof(save_i),&nb);

			save_i = BoneData[i].CrossSectionList[j].RefOuterID;
			isave->Write(&save_i,sizeof(save_i),&nb);

			}

//		save_pt = BoneData[i].l1;
//		isave->Write(&save_pt,sizeof(save_pt),&nb);

//		save_pt = BoneData[i].l2;
//		isave->Write(&save_pt,sizeof(save_pt),&nb);

		save_b = BoneData[i].flags;
		isave->Write(&save_b,sizeof(save_b),&nb);

		save_b = BoneData[i].FalloffType;
		isave->Write(&save_b,sizeof(save_b),&nb);

		save_i = BoneData[i].BoneRefID;
		isave->Write(&save_i,sizeof(save_i),&nb);

		save_i = BoneData[i].RefEndPt1ID;
		isave->Write(&save_i,sizeof(save_i),&nb);

		save_i = BoneData[i].RefEndPt2ID;
		isave->Write(&save_i,sizeof(save_i),&nb);



		}
	isave->EndChunk();


	for (i = 0; i < c; i++)
		{
		if ((BoneData[i].flags & BONE_SPLINE_FLAG) && (BoneData[i].Node != NULL) )
			{
			isave->BeginChunk(BONE_SPLINE_CHUNK);
			BoneData[i].referenceSpline.Save(isave);
			isave->EndChunk();
			}
		}


	if (bindNode)
		{
		isave->BeginChunk(BONE_BIND_CHUNK);
		initialXRefTM.Save(isave);
		
		ULONG id = isave->GetRefID(bindNode);
		isave->Write(&id,sizeof(ULONG),&nb);

		isave->EndChunk();
		}


	NameTab names;
//	names.SetCount(c);
	for (i = 0; i < c; i++)
		{	
		TSTR temp(BoneData[i].name);
		names.AddName(temp);
		}

	isave->BeginChunk(BONE_NAME_CHUNK);
	names.Save(isave);
	isave->EndChunk();


	c = endPointDelta.Count();
	isave->BeginChunk(DELTA_COUNT_CHUNK);
	isave->Write(&c,sizeof(c),&nb);
	isave->EndChunk();

	isave->BeginChunk(DELTA_DATA_CHUNK);
	for (i = 0; i < c; i++)
		{	
		Point3 p = endPointDelta[i];
		isave->Write(&p,sizeof(p),&nb);
		}
	isave->EndChunk();
//need to update for each vers of max
	int cver = 4;
	isave->BeginChunk(VER_CHUNK);
	isave->Write(&cver,sizeof(cver),&nb);
	isave->EndChunk();


	return IO_OK;
	}

Interval BonesDefMod::LocalValidity(TimeValue t)
	{
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	return GetValidity(t);
	}

Interval BonesDefMod::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
//	p1Temp->GetValue(t,&pt,iv,CTRL_ABSOLUTE);
//	for (int i = 0; i<MAX_NUMBER_BONES;i++)
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
//TimeValue t;
//t = ip->GetTime();
ObjectState os = BoneData[bid].Node->EvalWorldState(t);
pathOb = (ShapeObject*)os.obj;

int cid = bmd->VertexData[vertex]->d[bone].SubCurveIds;
int sid = bmd->VertexData[vertex]->d[bone].SubSegIds;
float u = bmd->VertexData[vertex]->d[bone].u;


Matrix3 ntm = BoneData[bid].Node->GetObjectTM(t);	
Matrix3 tm    = Inverse(bmd->BaseTM * Inverse(ntm));

//Matrix3 tm = bmd->BaseTM * Inverse(ntm);
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
//RotateMe.Translate(-OPoint);


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
/*		{
			
			if (bmd->VertexDataCount>0)
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
						vec = vec * influ;//Cluster->VertexData[i].d[0].Influences;
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
*/
	};



class StaticBonesDefDeformer: public Deformer {
	public:
		BonesDefMod *Cluster;
		BoneModData *bmd;
		TimeValue t;
		StaticBonesDefDeformer(BonesDefMod *C, BoneModData *bm, TimeValue tv){Cluster = C;bmd = bm; t= tv;}
		Point3 Map(int i, Point3 p) {
			
			if (bmd->VertexDataCount>0)
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
		if ( (bmd->VertexDataCount != nv) || (reset))
			{
			reset = FALSE;
			bmd->VertexDataCount = nv;
			for (int i = 0; i < bmd->VertexData.Count(); i++)
				{
				if (bmd->VertexData[i] != NULL)
					delete (bmd->VertexData[i]);
				bmd->VertexData[i] = NULL;
				}
			bmd->VertexData.ZeroCount();
			bmd->VertexData.SetCount(nv);
//new char[sizeof( str )];

			for (i=0; i<nv; i++) {
				VertexListClass *vc;
				vc = new VertexListClass;
				bmd->VertexData[i] = vc;
				bmd->VertexData[i]->modified = FALSE;
				bmd->VertexData[i]->selected = FALSE;
 				bmd->VertexData[i]->d.ZeroCount();;

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
//					Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(RefFrame,&valid);

//					Point3 pt = p * ntm  * bmd->InverseBaseTM;

//					b = b*Inverse(BoneData[i].tm);
					b = b * ntm * bmd->InverseBaseTM;

	
					}
				else
					{
	
//					GetEndPointsLocal(bmd,t,l1, l2, i);
					Interval valid;
//watje 3-11-99
					GetEndPoints(bmd,RefFrame,l1, l2, i);

//					BoneData[i].EndPoint1Control->GetValue(RefFrame,&l1,valid);
//					BoneData[i].EndPoint2Control->GetValue(RefFrame,&l2,valid);

//					Matrix3 ntm = BoneData[i].Node->GetObjTMBeforeWSM(RefFrame,&valid);

//					l1 = l1 * ntm * bmd->InverseBaseTM;
//					l2 = l2 * ntm * bmd->InverseBaseTM;
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
			if (!bmd->VertexData[i]->modified)
				{

//				VertexData[i]->d.ZeroCount();
				p = os->obj->GetPoint(i);

				int FullStrength =0;
				for (int j =0;j<BoneData.Count();j++) 
					{	
					BOOL excluded = FALSE;
					if (j < bmd->exclusionList.Count())
						{
						if (bmd->exclusionList[j])
							{
//							BitArray *t = 
							if (bmd->isExcluded(j,i))
								{
								excluded = TRUE;
/*
								for (int m = 0; m < bmd->VertexData[i]->d.Count(); m++)
									{
									if (bmd->VertexData[i]->d[m].Bones == j)
										bmd->VertexData[i]->d.Delete(j,1);
									}
*/
								}
							}
						}
					if ((BoneData[j].Node != NULL) && (!excluded))

						{
						if (BBoxList[j].Contains(p)) 
//						if (1) 
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
//3-29-99								ShapeObject *pathOb = NULL;
//								ObjectState os = BoneData[j].Node->EvalWorldState(RefFrame);
//								pathOb = (ShapeObject*)os.obj;

								Interval valid;
//watje 10-7-99 212059
								Matrix3 ntm = BoneData[j].tm;
//								Matrix3 ntm = BoneData[j].Node->GetObjTMBeforeWSM(RefFrame,&valid);

//watje 10-7-99 212059
								ntm =bmd->BaseTM * ntm;
//								ntm =bmd->BaseTM * Inverse(ntm);


//								Influence = SplineToPoint(p,pathOb,LineU,op,otan,cid,sid,ntm);
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
//								td.normalizedInfluences = -1.0f;//FIX?

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
			if (!bmd->VertexData[i]->modified)
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
/*
								for (int m = 0; m < bmd->VertexData[i]->d.Count(); m++)
									{
									if (bmd->VertexData[i]->d[m].Bones == j)
										bmd->VertexData[i]->d.Delete(j,1);
									}
*/

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
//								td.normalizedInfluences = -1.0f;//FIX?
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


void BonesDefMod::DumpVertexList()

{
/*
for (int i=0; i<bmd->VertexData.Count(); i++) 
	{
	DebugPrint("Vertex %d ",i);
	for (int j=0; j<bmd->VertexData[i]->d.Count(); j++) 
		{
//		int bid = VertexData[i]->d[j].Bones;
		float inf = RetrieveNormalizedWeight(bmd,i, j);
		DebugPrint("%d/%f ",bmd->VertexData[i]->d[j].Bones,inf);

		}
	DebugPrint("\n");

	}
*/
}
void BonesDefMod::LockThisBone(int bid)
{
/*
for (int i=0; i<bmd->VertexData.Count(); i++) 
	{
	for (int j=0; j<bmd->VertexData[i]->d.Count(); j++) 
		{
		if (bmd->VertexData[i]->d[j].Bones == bid)
			{
			bmd->VertexData[i]->d[j].Influences = RetrieveNormalizedWeight(bmd,i, j);
//			bmd->VertexData[i]->d[j].normalizedInfluences = bmd->VertexData[i]->d[j].Influences;
			}

		}
	}
*/
}

float BonesDefMod::RetrieveNormalizedWeight(BoneModData *bmd, int vid, int bid)
{
//need to reqeight based on remainder
double tempdist=0.0f;
double w;

int bd = bmd->VertexData[vid]->d[bid].Bones;

if (BoneData[bd].Node == NULL)
	{
	bmd->VertexData[vid]->d[bid].normalizedInfluences = 0.0f;
	bmd->VertexData[vid]->d[bid].Influences = 0.0f;
	return 0.0f;
	}

//if (bmd->VertexData[vid]->d[bid].normalizedInfluences != -1.0f) return bmd->VertexData[vid]->d[bid].normalizedInfluences; //ns

//if more than one bone use a weigthed system
if (bmd->VertexData[vid]->d.Count() >1) 
	{
	double remainder = 0.0f; 
	double offset =0.0f;
	tempdist = 0.0f;
	for (int j=0; j<bmd->VertexData[vid]->d.Count(); j++) 
		{
		float infl = bmd->VertexData[vid]->d[j].Influences;
		int bone=bmd->VertexData[vid]->d[j].Bones;
		if (!(BoneData[bone].flags & BONE_LOCK_FLAG))
			tempdist += infl;
		else 
			offset += infl;

		}
	offset = 1.0f-offset;
	double vinflu = bmd->VertexData[vid]->d[bid].Influences;
	int bn=bmd->VertexData[vid]->d[bid].Bones;
	if 	(!(BoneData[bn].flags & BONE_LOCK_FLAG))
		w = ((bmd->VertexData[vid]->d[bid].Influences)/tempdist) *offset;
	else w = bmd->VertexData[vid]->d[bid].Influences;
	}
else if (bmd->VertexData[vid]->d.Count() == 1) 
	{
//if only one bone and absolute control set to it to max control
	if ( (BoneData[bmd->VertexData[vid]->d[0].Bones].flags & BONE_ABSOLUTE_FLAG) )
//		&& !(bmd->VertexData[vid]->modified) )
		{
//		VertexData[vid].d[0].Influences = 1.0f;
		w = 1.0f;
		}
	else w = bmd->VertexData[vid]->d[0].Influences;


	}
bmd->VertexData[vid]->d[bid].normalizedInfluences = (float)w; //ns
return (float)w;
}

void BonesDefMod::UnlockBone(BoneModData *bmd,TimeValue t, ObjectState *os)
	{
//loop through verts and remove and associations to thos bone
	for (int i=0;i<bmd->VertexDataCount;i++)
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
/*
				for (int m = 0; m < bmd->VertexData[i]->d.Count(); m++)
					{
					if (bmd->VertexData[i]->d[m].Bones == j)
						bmd->VertexData[i]->d.Delete(j,1);
					}
*/

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
				bmd->VertexData[i]->modified = FALSE;

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
			for (int i=0;i<bmd->VertexDataCount;i++)
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
/*
							for (int m = 0; m < bmd->VertexData[i]->d.Count(); m++)
								{
								if (bmd->VertexData[i]->d[m].Bones == j)
									bmd->VertexData[i]->d.Delete(j,1);
								}
*/

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
						bmd->VertexData[i]->modified = FALSE;

						}
					}
				}
			}
		}
	ModeBoneIndex = temp;

	}



void BonesDefMod::UpdateTMCacheTable(BoneModData *bmd, TimeValue t, Interval& valid)
{
	if (bmd == NULL) return;



	if (bmd->tempTableL1.Count() != BoneData.Count())
		bmd->tempTableL1.SetCount(BoneData.Count());

	if (bmd->tempTableL2.Count() != BoneData.Count())
		bmd->tempTableL2.SetCount(BoneData.Count());

	if (bmd->tempTableL1ObjectSpace.Count() != BoneData.Count())
		bmd->tempTableL1ObjectSpace.SetCount(BoneData.Count());

	if (bmd->tempTableL2ObjectSpace.Count() != BoneData.Count())
		bmd->tempTableL2ObjectSpace.SetCount(BoneData.Count());


	if (bmd->tmCacheToBoneSpace.Count() != BoneData.Count())
		bmd->tmCacheToBoneSpace.SetCount(BoneData.Count());

	if (bmd->tmCacheToObjectSpace.Count() != BoneData.Count())
		bmd->tmCacheToObjectSpace.SetCount(BoneData.Count());
	for (int j =0;j<BoneData.Count();j++)
		{
		if (BoneData[j].Node != NULL)
			{
			Point3 l1, l2;

			Interval v;
			BoneData[j].EndPoint1Control->GetValue(t,&l1,v);
			BoneData[j].EndPoint2Control->GetValue(t,&l2,v);
	
///hmmm can these go away if we are not editing?
			bmd->tempTableL1[j] = l1* Inverse(BoneData[j].tm) * Inverse(bmd->BaseTM);
			bmd->tempTableL2[j] = l2* Inverse(BoneData[j].tm) * Inverse(bmd->BaseTM);


//if BID then 
			Matrix3 ntm;
			ntm = BoneData[j].Node->GetObjTMBeforeWSM(t,&valid);

			if (bindNode)
				{
				xRefTM = bindNode->GetObjectTM(t);

				BoneData[j].temptm = bmd->BaseTM * BoneData[j].tm * ntm * 
					initialXRefTM * Inverse(xRefTM) * bmd->InverseBaseTM;
				}
			else BoneData[j].temptm = bmd->BaseTM * BoneData[j].tm * ntm * bmd->InverseBaseTM; 

//			BoneData[j].temptm = bmd->BaseTM * BoneData[j].tm * ntm * bmd->InverseBaseTM;

//cache for for matrices
			bmd->tmCacheToBoneSpace[j] = bmd->BaseTM * Inverse(ntm);
			bmd->tmCacheToObjectSpace[j] = ntm * bmd->InverseBaseTM;

///hmmm can these go away if we are not editing?
			bmd->tempTableL1ObjectSpace[j] = l1 * bmd->tmCacheToObjectSpace[j];
			bmd->tempTableL2ObjectSpace[j] = l2 * bmd->tmCacheToObjectSpace[j];

			}
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
			BoneData[j].InitNodeTM = BoneData[j].Node->GetNodeTM(RefFrame); //ns

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
						BoneData[j].EndPoint2Control->GetValue(0,&d,iv,CTRL_ABSOLUTE);
						ci = childCount;
						endPointDelta[j] = d-l2;

						}
							
					}
				}



			}
		}

	}	

}

void BonesDefMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	Interval valid = FOREVER;
	TimeValue tps = GetTicksPerFrame();


    XRefEnumProc dep;              
	dep.nukeME = FALSE;
	EnumDependents(&dep);
	
	INode *XRefNode = dep.Nodes[0];
	INode *rootNode = GetCOREInterface()->GetRootNode();
	int xct = rootNode->GetXRefFileCount();

	// If this flag is set, we have to recompute the InitNodeTM. That means, that we check the current difference between the node TM
	// and the ObjectOffset TM and remove the inverse of that from the InitObject TM. Thus we get the NodeTM at init time.
	// This only works, if the object offset TM didn't change since the bones were assigned !!! This is done, so we can make Skin
	// work with Node TM's and not with Object TM's as in the shipping version.

	if(recompInitTM)
	{
		for (int i = 0; i < BoneData.Count(); i++)
		{
			INode *node = BoneData[i].Node;
			Matrix3 diffTM(TRUE);
			if(node)
				diffTM = node->GetNodeTM(0) * Inverse(node->GetObjectTM(0));

			BoneData[i].InitNodeTM = diffTM * Inverse(BoneData[i].tm);
		}
		recompInitTM = false;
	}

	for (int xid = 0; xid < xct; xid++)
		{
		INode *xroot = rootNode->GetXRefTree(xid);
		BOOL amIanXRef = RecurseXRefTree(xroot,XRefNode);
		if (amIanXRef)
			{
			INode *tempBindNode = rootNode->GetXRefParent(xid);
			if ((tempBindNode) && (bindNode!=tempBindNode))
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




//loop through bone nodes to get new interval
	float ef;
	pblock_param->GetValue(skin_effect,t,ef,valid);


//	pblock_param->GetValue(PB_LOCK_BONE,t,LockBone,valid);
//	pblock_param->GetValue(PB_ABSOLUTE_INFLUENCE,t,AbsoluteInfluence,valid);

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


	pblock_param->GetValue(skin_paint_radius,t,Radius,valid);
	pblock_param->GetValue(skin_paint_feather,t,Feather,valid);

//	pblock_param->GetValue(PB_PROJECT_THROUGH,t,ProjectThrough,valid);
//	pblock_param->GetValue(PB_FALLOFF,t,Falloff,valid);

	pblock_advance->GetValue(skin_advance_ref_frame,t,RefFrame,valid);
	pblock_advance->GetValue(skin_advance_always_deform,t,AlwaysDeform,valid);

	pblock_advance->GetValue(skin_advance_rigid_verts,t,rigidVerts,valid);
	pblock_advance->GetValue(skin_advance_rigid_handles,t,rigidHandles,valid);
	pblock_advance->GetValue(skin_advance_fast_update,t,fastUpdate,valid);


	RefFrame = RefFrame*tps;


//build each instance local data
	if (ip != NULL)
		{
		ModContextList mcList;
		INodeTab nodes;
		if (mc.localData == NULL)
			{
			ip->GetModContexts(mcList,nodes);

			for (int i = 0; i < nodes.Count(); i++)
				{
				BoneModData *d  = new BoneModData(this);

				d->BaseTM = nodes[i]->GetObjectTM(RefFrame);
				d->BaseNodeTM = nodes[i]->GetNodeTM(RefFrame); //ns
				d->InverseBaseTM = Inverse(d->BaseTM);
				UpdateTMCacheTable(d,t,valid);

				if ((OldVertexDataCount != 0) && (i==0))
					{
					d->VertexDataCount = OldVertexDataCount;
					d->VertexData.SetCount(OldVertexDataCount);
					for (int j = 0; j < OldVertexDataCount; j++)
						{
						VertexListClass *vc;
						vc = new VertexListClass;
						d->VertexData[j] = vc;

						d->VertexData[j]->selected = OldVertexData[j]->selected;
						d->VertexData[j]->modified = OldVertexData[j]->modified;
						d->VertexData[j]->LocalPos = OldVertexData[j]->LocalPos;
						d->VertexData[j]->d = OldVertexData[j]->d;
						}
					for (j = 0; j < OldVertexDataCount; j++)
						delete (OldVertexData[j]);

					OldVertexData.ZeroCount();

					OldVertexDataCount = -1;
					}
				mcList[i]->localData = d;


				}
			NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			valid.SetInstant(t);
			os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	

			return;

			}
//create a back pointer to the container entry				
		}

	BoneModData *bmd = (BoneModData *) mc.localData;			


	if (bmd == NULL) return;

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
				
				diffTM = nodes[i]->GetNodeTM(0) * Inverse(nodes[i]->GetObjectTM(0));
				
				bmd->BaseNodeTM = diffTM * bmd->BaseTM;
			}
		}
		bmd->recompInitMeshTM = false;
	}

//6-18-99
	bmd->inputObjectIsNURBS = os->obj->ClassID() == EDITABLE_SURF_CLASS_ID;	


	if (BoneData.Count() == 0) return;

	UpdateTMCacheTable(bmd,t,valid);

//	DebugPrint("num verts %d bmd %d\n",os->obj->NumPoints(),bmd->VertexDataCount);


	if (os->obj->NumPoints()==0) 
		{
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	
		return;
		}


	if (os->obj->IsSubClassOf(patchObjectClassID))
		{
		PatchObject *pobj = (PatchObject*)os->obj;
		
		int knots = pobj->patch.numVerts;
		bmd->autoInteriorVerts.SetSize(knots+pobj->patch.numVecs);
		if (rigidHandles)
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
		if (rigidHandles)
			{
			for (int ivec = 0; ivec < pobj->patch.numVecs; ivec++)
//need to check if manual interio and mark if the  manuaul interio bug gets fixed
				{
				PatchVec pv = pobj->patch.vecs[ivec];
				bmd->vecOwnerList[knots + ivec] = pv.vert;

				}
			}	

		}



	if ((!painting) || (bmd->forceUpdate))
		{


		bmd->forceUpdate = FALSE;	

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
						BuildMajorAxis(BoneData[whichSplineChanged].Node,a,b,el1); 

						BoneData[whichSplineChanged].EndPoint1Control->SetValue(0,a,TRUE,CTRL_ABSOLUTE);
						BoneData[whichSplineChanged].EndPoint2Control->SetValue(0,b,TRUE,CTRL_ABSOLUTE);
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
//								shape->MakeBezier(t, bShape);
							else {
								PolyShape pShape;
//watje 9-7-99  195862 
								shape->MakePolyShape(RefFrame, pShape);
//								shape->MakePolyShape(t, pShape);
								bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
								}
							}

						if ((shape) && (bShape.splineCount >0) &&
							(bShape.splines[0]->Segments() != BoneData[whichSplineChanged].referenceSpline.Segments()))
							{
							BoneData[whichSplineChanged].referenceSpline = *bShape.splines[0];
//watje 9-7-99  198721 
							bmd->reevaluate=TRUE;
							}

/*						ObjectState sos = BoneData[whichSplineChanged].Node->EvalWorldState(RefFrame);
						SplineShape *shape = (SplineShape *)sos.obj;
//check for a topo change is so recreate reference spline and recompuetall
						if ((shape) && (shape->shape.splines[0]->Segments() != BoneData[whichSplineChanged].referenceSpline.Segments()))
							{
							BoneData[whichSplineChanged].referenceSpline = *shape->shape.splines[0];
							reevaluate = TRUE;
							}
*/
						}

					}


				}
			splineChanged = FALSE;
			}




		if (unlockVerts)
			{
			unlockVerts = FALSE;
//watje 9-7-99  198721 
			bmd->reevaluate=TRUE;
			for (int i=0;i<bmd->VertexDataCount;i++)
				{
				if (bmd->selected[i])
					bmd->VertexData[i]->modified = FALSE;
				}
			}

		if (updateP)
			{
			updateP = FALSE;
			UpdateP(bmd);
			}

		if ((((t == (RefFrame)) && (BoneMoved)) && (!AlwaysDeform))  || forceRecomuteBaseNode)
//	if (((t == (RefFrame *tps)) && (BoneMoved)) )
			{
//readjust TMs for frame 0
			BoneMoved = FALSE;
//watje 9-7-99  198721 
			bmd->reevaluate=TRUE;
			cacheValid = FALSE;
			forceRecomuteBaseNode = FALSE;
			bmd->CurrentCachePiece = -1;

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

	
			for (int j =0;j<BoneData.Count();j++)
				{
				if (BoneData[j].Node != NULL)
					{
					Class_ID bid(BONE_CLASS_ID,0);
					Matrix3 ntm =BoneData[j].Node->GetObjectTM(RefFrame);	

					BoneData[j].InitObjectTM = ntm; //ns
					BoneData[j].tm = Inverse(ntm);
					BoneData[j].InitNodeTM = BoneData[j].Node->GetNodeTM(RefFrame); //ns
//copy initial reference spline into our spline
					ObjectState sos = BoneData[j].Node->EvalWorldState(RefFrame);
					if (BoneData[j].flags & BONE_SPLINE_FLAG)
						{
//						SplineShape *shape = (SplineShape *)sos.obj;
//						BoneData[j].referenceSpline = *shape->shape.splines[0];
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
									BoneData[j].EndPoint2Control->SetValue(0,&l2,TRUE,CTRL_ABSOLUTE);
									}
								else BoneData[j].EndPoint2Control->SetValue(0,&l2,TRUE,CTRL_ABSOLUTE);
								ci = childCount;

								}
							
							}
						}



					}
				}

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
//						SplineShape *shape = (SplineShape *)sos.obj;
//						BoneData[j].referenceSpline = *shape->shape.splines[0];
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
//			spin2->SetIndeterminate(TRUE);

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
		if (bmd->VertexDataCount != os->obj->NumPoints())
			{
//readjust vertices using nearest vertices as sample
//DebugPrint("    Reevaluating \n");
//watje 9-7-99  198721 
			bmd->reevaluate = TRUE;
			bmd->CurrentCachePiece = -1;
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



		if ((ip) || (bmd->reevaluate))
			{
			if ( (bmd->reevaluate) )
				{
				RecomputeAllBones(bmd,t,os);
				cacheValid = FALSE;
				bmd->CurrentCachePiece = -1;

				}	

			else if ( (ModeBoneIndex!=-1) && (ip && ip->GetSubObjectLevel() == 1))//&& (ModeEdit ==1) )
				{
				RecomputeBone(bmd,ModeBoneIndex,t,os);
				}
			}




		for (i = 0; i < os->obj->NumPoints(); i++)
			bmd->VertexData[i]->LocalPos = os->obj->GetPoint(i);

		}

	for (int  i = 0; i < pblock_gizmos->Count(skin_gizmos_list); i++) 
		{
		ReferenceTarget *ref;
		ref = pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,i);
		GizmoClass *gizmo = (GizmoClass *)ref;
		gizmo->PreDeformSetup( t);
		}


	valid &= GetValidity(t);

	// Here comes the COM engine setup
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
				bmd->pSE->SetBoneTM(i,(float *) &ntm);
				bmd->pSE->SetInitBoneTM(i,(float *) &BoneData[i].InitObjectTM);
			}
			else
			{
				ntm = BoneData[i].Node->GetNodeTM(t);
				bmd->pSE->SetBoneTM(i,(float *) &ntm);				
				bmd->pSE->SetInitBoneTM(i,(float *) &BoneData[i].InitNodeTM);
			}				
		}
		bmd->pSE->SetBoneFlags(i,BoneData[i].flags);
	}

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
	
	bmd->CurrentTime = t;

	if ((t == RefFrame) && (!AlwaysDeform))
		{
		}
	else
		{
		BonesDefDeformer deformer(this,bmd,t);
		os->obj->Deform(&deformer, TRUE);
		}

//		}
//		else
//		{
//		StaticBonesDefDeformer staticdeformer(this,bmd,t);
//		os->obj->Deform(&staticdeformer, TRUE);

//		}
	
	if ((inPaint) && (!painting))
		{
		bmd->hitState = os;
		if (os->obj->IsSubClassOf(triObjectClassID))
			{
			bmd->isMesh = TRUE;
			bmd->isPatch = FALSE;
			}
		else if (os->obj->IsSubClassOf(patchObjectClassID))
			{
			bmd->isMesh = FALSE;
			bmd->isPatch = TRUE;
			}

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
		}

//	DebugPrint("End bonesdef\n");

	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);	

	}
	


#define ID_CHUNK 0x1000

IOResult BonesDefMod::SaveLocalData(ISave *isave, LocalModData *pld)
{

//IOResult	res;
ULONG		nb;

BoneModData *bmd = (BoneModData*)pld;

//isave->BeginChunk(ID_CHUNK);
//res = isave->Write(&p->id, sizeof(int), &nb);
//isave->EndChunk();

	isave->BeginChunk(BASE_TM_CHUNK);
	bmd->BaseTM.Save(isave);
	isave->EndChunk();



	int c = bmd->VertexDataCount;
//save vertex influence info
	isave->BeginChunk(VERTEX_COUNT_CHUNK);
	isave->Write(&c,sizeof(c),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(VERTEX_DATA_CHUNK);
	for (int i = 0; i < c; i++)
		{
//write number of influences
		int ic;
		ic = bmd->VertexData[i]->d.Count();
		isave->Write(&ic,sizeof(ic),&nb);
		int save_i;
		float save_f;
		BOOL save_b;
		save_b = bmd->VertexData[i]->modified;
		isave->Write(&save_b,sizeof(save_b),&nb);

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

return IO_OK;
}

IOResult BonesDefMod::LoadLocalData(ILoad *iload, LocalModData **pld)

{
	IOResult	res;
	ULONG		nb;

	BoneModData *bmd = new BoneModData(this);
	*pld = bmd;
	bmd->effect = -1.0f;


//	int id;
	int exsize,exsizeActive, exID,k;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
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
				bmd->VertexDataCount = c;
//				if (VertexData != NULL) delete VertexData;
//				VertexData = new VertexListClass[c]; 
				bmd->VertexData.ZeroCount();
				bmd->VertexData.SetCount(c);
				for (int i=0; i<c; i++) {
					VertexListClass *vc;
					vc = new VertexListClass;
					bmd->VertexData[i] = vc;
					bmd->VertexData[i]->modified = FALSE;
					bmd->VertexData[i]->selected = FALSE;
 					bmd->VertexData[i]->d.ZeroCount();
//					VertexData[i].Influences.ZeroCount();
					}

				break;

				}
			case VERTEX_DATA_CHUNK:
				{
				for (int i=0; i < bmd->VertexDataCount; i++)
					{
					int c;
					BOOL load_b;
					iload->Read(&c,sizeof(c),&nb);
					bmd->VertexData[i]->d.SetCount(c);

//					VertexData[i].Influences.SetCount(c);
					iload->Read(&load_b,sizeof(load_b),&nb);
					bmd->VertexData[i]->modified = load_b;
					float load_f;
					int load_i;
					Point3 load_p;
					for (int j=0; j<c; j++) {
						iload->Read(&load_i,sizeof(load_i),&nb);
						iload->Read(&load_f,sizeof(load_f),&nb);
 						bmd->VertexData[i]->d[j].Bones = load_i;
						bmd->VertexData[i]->d[j].Influences =load_f;
//						bmd->VertexData[i]->d[j].normalizedInfluences = -1.0f; ns

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

			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}


    int c = bmd->VertexDataCount;
//add m crossection
	bmd->selected.SetSize(c);
	bmd->selected.ClearAll();
	bmd->CurrentCachePiece = -1;

//	for (i=0; i<c; i++) 
//		sel[i] = FALSE;

return IO_OK;

}

void BonesDefMod::SaveEnvelopeDialog()
{
static TCHAR fname[256] = {'\0'};
OPENFILENAME ofn;
memset(&ofn,0,sizeof(ofn));
FilterList fl;
fl.Append( GetString(IDS_PW_ENVFILES));
fl.Append( _T("*.env"));		
TSTR title = GetString(IDS_PW_SAVEENVELOPES);

ofn.lStructSize     = sizeof(OPENFILENAME);
ofn.hwndOwner       = GetCOREInterface()->GetMAXHWnd();
ofn.lpstrFilter     = fl;
ofn.lpstrFile       = fname;
ofn.nMaxFile        = 256;    
//ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
ofn.lpstrDefExt     = _T("env");
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

	macroRecorder->FunctionCall(_T("skinOps.SaveEnvelopes"), 2,0, mr_reftarg, this,mr_string,fname );
	SaveEnvelope(fname);
	}

}
void BonesDefMod::SaveEnvelope(TCHAR *fname)
{
FILE *file = fopen(fname,_T("wb"));
//ver
	int ver = 1;
	fwrite(&ver, sizeof(ver), 1,file);

//number bones
	int ct = 0;
	for (int i =0; i < BoneData.Count(); i++)
		{
//bone name length
		if (BoneData[i].Node != NULL) ct++;
		}

	fwrite(&ct, sizeof(ct), 1,file);

	for (i =0; i < ct; i++)
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
//bone name length size
			fwrite(&fnameLength, sizeof(fnameLength), 1,file);
//bone name
			fwrite(title, sizeof(TCHAR)*fnameLength, 1,file);
			}
//flags
		fwrite(&BoneData[i].flags, sizeof(BoneData[i].flags), 1,file);
//FalloffType;
		fwrite(&BoneData[i].FalloffType, sizeof(BoneData[i].FalloffType), 1,file);
//start point
		Point3 p;
		Interval iv;
		BoneData[i].EndPoint1Control->GetValue(0,&p,iv,CTRL_ABSOLUTE);
		fwrite(&p, sizeof(p), 1,file);
//end point
		BoneData[i].EndPoint2Control->GetValue(0,&p,iv,CTRL_ABSOLUTE);
		fwrite(&p, sizeof(p), 1,file);
//number cross sections
		int crossCount = BoneData[i].CrossSectionList.Count();
		fwrite(&crossCount, sizeof(crossCount), 1,file);
		for (int j=0;j<crossCount; j++)
			{
	//inner
	//outer
			float inner, outer,u;
			BoneData[i].CrossSectionList[j].InnerControl->GetValue(0,&inner,iv,CTRL_ABSOLUTE);
			BoneData[i].CrossSectionList[j].OuterControl->GetValue(0,&outer,iv,CTRL_ABSOLUTE);
			BoneData[i].CrossSectionList[j].OuterControl->GetValue(0,&outer,iv,CTRL_ABSOLUTE);
			u = BoneData[i].CrossSectionList[j].u;
			fwrite(&inner, sizeof(inner), 1,file);
			fwrite(&outer, sizeof(outer), 1,file);
			fwrite(&u, sizeof(u), 1,file);

			}

		}
fclose(file);
}



void BonesDefMod::LoadEnvelopeDialog()
{
static TCHAR fname[256] = {'\0'};
OPENFILENAME ofn;
memset(&ofn,0,sizeof(ofn));
FilterList fl;
fl.Append( GetString(IDS_PW_ENVFILES));
fl.Append( _T("*.env"));		
TSTR title = GetString(IDS_PW_SAVEENVELOPES);

ofn.lStructSize     = sizeof(OPENFILENAME);
ofn.hwndOwner       = GetCOREInterface()->GetMAXHWnd();
ofn.lpstrFilter     = fl;
ofn.lpstrFile       = fname;
ofn.nMaxFile        = 256;    
//ofn.lpstrInitialDir = ip->GetDir(APP_EXPORT_DIR);
ofn.Flags           = OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
ofn.lpstrDefExt     = _T("env");
ofn.lpstrTitle      = title;

if (GetOpenFileName(&ofn)) {
//save stuff here
	macroRecorder->FunctionCall(_T("skinOps.LoadEnvelopes"), 2,0, mr_reftarg, this,mr_string,fname );

	LoadEnvelope(fname);
	}

}



void* BonesDefMod::GetInterface(ULONG id)
{
	switch(id)
	{
		case I_SKIN : return (ISkin *) this;
			break;
//		case I_RESMAKER_INTERFACE : (ResourceMakerCallback *) this;
//			break;
		default: return NULL;
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
		
		diffTM = pNode->GetNodeTM(0) * Inverse(pNode->GetObjectTM(0));
		
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
	/*
	for(int i = 0 ; i < cnt ; i++)
	{
		DebugPrint("Bone_%d: %s %s Map: %d  \n",i,BoneData[BoneMap[i]].Node->GetName(),BoneData[i].name,BoneMap[i]);
	}
	*/
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
BoneData[id].EndPoint1Control->GetValue(RefFrame,&l1,FOREVER);
BoneData[id].EndPoint2Control->GetValue(RefFrame,&l2,FOREVER);
}

Matrix3 BonesDefMod::GetBoneTm(int id)
{
return BoneData[id].tm;
}

INode *BonesDefMod::GetBoneFlat(int idx)
{
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
				
				mc = pDerObj->GetModContext(Idx);
				break;
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
		theIconResHandler.SetHInstance(hInstance);
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
		ShellExecute(GetCOREInterface()->GetMAXHWnd(), "open", "regsvr32.exe", "/s MAXComponents.dll", path, SW_SHOWNORMAL);
		Sleep(3000);
		hr = CoCreateInstance( CLSID_SkinEngine, NULL, CLSCTX_INPROC_SERVER,IID_ISkinEngine,(void **)&pSE);

//if failed again something bad has happened and bail
		if( FAILED(hr))
			{	
			MessageBox(GetCOREInterface()->GetMAXHWnd(),"CoCreateInstance() failed\nPlease check your registry entries\nCLSID {F088EA74-2E87-11D3-B1F3-00C0F03C37D3}","COM Error",MB_OK);
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
	assert(0);
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

void ExclusionListClass::IncludeVerts(Tab<int> inclusionList)
{
	for (int i = 0; i < inclusionList.Count(); i++)
		{
		int vertID = inclusionList[i];
		int where;
		if (isInList(vertID,where))
			{
			exList.Delete(1,where);
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

void BoneModData::ExcludeVerts(int boneID, Tab<int> exList)
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
for (int i =0; i < VertexData.Count(); i++)
	{
	for (int  j = VertexData[i]->d.Count()-1; j >= 0; j--)
		{
		if (VertexData[i]->d[j].Bones == boneID)
			VertexData[i]->d.Delete(j,1);
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
			VertexData[i]->selected = FALSE;

		for (i = 0; i < exclusionList[boneID]->Count(); i++)
			{
			int id = exclusionList[boneID]->Vertex(i);
			selected.Set(id,1);
			VertexData[id]->selected = TRUE;

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

	for( j = 0 ; j < bmd->gizmoData.Count() ; j++)
		{
		int id = bmd->gizmoData[j]->whichGizmo;
		ReferenceTarget *ref;
		ref = Cluster->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,id);
		GizmoClass *gizmo = (GizmoClass *)ref;
		if (gizmo->IsEnabled())
			{
			if  (gizmo->IsVolumeBased() && gizmo->IsInVolume(initialP,bmd->BaseTM))
				{
				if (gizmo)
					pout = gizmo->DeformPoint(t,i,initialP,pout,bmd->BaseTM);
			
				}
			else if (!gizmo->IsVolumeBased() && bmd->gizmoData[j]->IsAffected(i)) 
				{
				if (gizmo)
					pout = gizmo->DeformPoint(t,i,initialP,pout,bmd->BaseTM);
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
	//DebugPrint("AddRef %d\n", m_cRef+1);
	return ++m_cRef;
}

ULONG CSkinCallback::Release()
{
	//DebugPrint("Release %d\n", m_cRef-1);
	if(--m_cRef != 0)
		return m_cRef;
	delete this;
	//DebugPrint("Release DELETE !!\n");
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
	*pp = pathOb->InterpPiece3D(bmd->CurrentTime, CurveId, SegId , distance );
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CSkinCallback::GetTangentPiece3D(int BoneId,int CurveId,int SegId,float distance,float __RPC_FAR *pPoint)
{
	Point3 *pp = (Point3 *) pPoint;
	ObjectState os = bmd->mod->BoneData[BoneId].Node->EvalWorldState(bmd->CurrentTime);
	ShapeObject *pathOb = (ShapeObject*)os.obj;
	*pp = pathOb->TangentPiece3D(bmd->CurrentTime, CurveId, SegId , distance );
	return S_OK;

}

