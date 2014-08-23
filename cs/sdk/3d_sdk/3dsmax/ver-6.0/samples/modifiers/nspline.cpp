/**********************************************************************
 *<
	FILE: nspline.cpp

	DESCRIPTION:  Normalize Spline

	CREATED BY: Peter Watje

	HISTORY: created Jan 20, 1997

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "mods.h"

#ifndef NO_MODIFIER_NORMALIZE_SPLINE // JP Morel - July 23th 2002

#include "iparamm2.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"

//--- NormalizeSpline -----------------------------------------------------------

#define EDITSPL_CHANNELS (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO)

#define MIN_AMOUNT		float(-1.0E30)
#define MAX_AMOUNT		float(1.0E30)

#define SPLINETYPE 0
#define CIRCULAR 0


#define PW_PATCH_TO_SPLINE1 0x1c450e5c
#define PW_PATCH_TO_SPLINE2 0x2e0e0902

//#define DEBUG 1



class NormalizeSpline: public Modifier {
	
	protected:
		IParamBlock2 *pblock;

		static IObjParam *ip;
		
	public:
		static IParamMap *pmapParam;
		
		static float nlength;



		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_PW_NSPLINE); }  
		virtual Class_ID ClassID() { return Class_ID(PW_PATCH_TO_SPLINE1,PW_PATCH_TO_SPLINE2);}		
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_PW_NSPLINE); }
		IOResult Load(ILoad *iload);

		NormalizeSpline();
		virtual ~NormalizeSpline();

		ChannelMask ChannelsUsed()  { return EDITSPL_CHANNELS; }
		ChannelMask ChannelsChanged() { return EDITSPL_CHANNELS; }


		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

		Class_ID InputType() { return Class_ID(SPLINESHAPE_CLASS_ID,0); }
		
		Interval LocalValidity(TimeValue t);

		// From BaseObject
		BOOL ChangeTopology() {return TRUE;}
//		IParamArray *GetParamBlock() {return pblock;}
//		int GetParamBlockIndex(int id) {return id;}

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock2*)rtarg;}

 		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i) { return pblock; }
		TSTR SubAnimName(int i) { return TSTR(GetString(IDS_RB_PARAMETERS));}		

		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message);

		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		void BuildSkin(TimeValue t,ModContext &mc, ObjectState * os);

		void UpdateUI(TimeValue t) {}
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { if (i == 0) return pblock; 
											else return NULL;
												} // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) {if (pblock->ID() == id) return pblock ;
													else return  NULL; } // return id'd ParamBlock

	};

class NormalizeSplineClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new NormalizeSpline; }
	const TCHAR *	ClassName() { return GetString(IDS_PW_NSPLINE); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return  Class_ID(PW_PATCH_TO_SPLINE1,PW_PATCH_TO_SPLINE2); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("Normalize_Spline"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static NormalizeSplineClassDesc normalizeSplineDesc;
extern ClassDesc* GetNormalizeSplineDesc() { return &normalizeSplineDesc; }

IObjParam*		NormalizeSpline::ip        = NULL;
IParamMap *		NormalizeSpline::pmapParam = NULL;
float			NormalizeSpline::nlength = 20.0f;
 

//--- Parameter map/block descriptors -------------------------------

static int outputIDs[] = {IDC_RADIO1,IDC_RADIO2,IDC_RADIO3,IDC_RADIO4};


//#define PB_SPLINETYPE	0
//#define PB_CIRCULAR		0
//#define PB_NLENGTH		1

#define PBLOCK_REF		0

//enum { nspline_params};

//enum { nspline_length};



// per instance geosphere block
static ParamBlockDesc2 nspline_param_blk ( nspline_params, _T("Parameters"),  0, &normalizeSplineDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_NSPLINE, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params

	nspline_length,  _T("Length"),	TYPE_FLOAT, 	0, 	IDS_RB_LENGTH, 
		p_default, 		20.0f,	
		p_range, 		1.0f,999999999.0f, 
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LENGTH_EDIT,IDC_LENGTH_SPIN,  1.0f,
		end, 

	end
	);


//
//
// Parameters
/*

static ParamUIDesc descParam[] = {
	// Spline type
//	ParamUIDesc(PB_SPLINETYPE,TYPE_RADIO,outputIDs,4)
//															 Length
	// Circular
   ParamUIDesc(PB_CIRCULAR,TYPE_SINGLECHEKBOX,IDC_TEST_CHECK),
	// NLnegth
	ParamUIDesc(
		PB_NLENGTH,
		EDITTYPE_FLOAT,
		IDC_LENGTH_EDIT,IDC_LENGTH_SPIN,
		0.01f,999999999.0f,
		0.01f)


	};
*/
#define PARAMDESC_LENGTH 2



static ParamBlockDescID descVer3[] = {
	{ TYPE_INT, NULL, FALSE, -1 },
	{ TYPE_FLOAT, NULL, TRUE, nspline_length }
	 };

#define PBLOCK_LENGTH	2

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer3,2,5),	
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	5
//   static ParamVersionDesc curVersion(descVer3,PBLOCK_LENGTH,CURRENT_VERSION);

NormalizeSpline::NormalizeSpline()
	{
/*
	MakeRefByID(FOREVER, 0, 
		CreateParameterBlock(descVer3, PBLOCK_LENGTH, CURRENT_VERSION));
	pblock->SetValue(PB_NLENGTH, TimeValue(0), nlength);
*/

	GetNormalizeSplineDesc()->MakeAutoParamBlocks(this);

	
	}

NormalizeSpline::~NormalizeSpline()
	{	
	}

Interval NormalizeSpline::LocalValidity(TimeValue t)
	{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	Interval valid = GetValidity(t);	
	return valid;
	}

RefTargetHandle NormalizeSpline::Clone(RemapDir& remap)
	{
	NormalizeSpline* newmod = new NormalizeSpline();	
	newmod->ReplaceReference(0,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
	}

void NormalizeSpline::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
	{	
//DebugPrint("Extrude modifying object\n");

	// Get our personal validity interval...
	Interval valid = GetValidity(t);
	// and intersect it with the channels we use as input (see ChannelsUsed)
	valid &= os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	valid &= os->obj->ChannelValidity(t,GEOM_CHAN_NUM);
	Matrix3 modmat,minv;
	
//	int output;


//	pblock->GetValue(PB_NLENGTH,t,nlength,valid);
	pblock->GetValue(nspline_length,t,nlength,valid);

	if (nlength < 0.0001f) nlength = 0.0001f;
	
	

	BuildSkin(t, mc, os);

	


	os->obj->SetChannelValidity(TOPO_CHAN_NUM, valid);
	os->obj->SetChannelValidity(GEOM_CHAN_NUM, valid);
	os->obj->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
	os->obj->SetChannelValidity(MTL_CHAN_NUM, valid);
	os->obj->SetChannelValidity(SELECT_CHAN_NUM, valid);
	os->obj->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
	os->obj->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

//	os->obj = pat;

	os->obj->UnlockObject();
//	os->obj->InvalidateGeomCache();

	}


void NormalizeSpline::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	this->ip = ip;

	TimeValue t = ip->GetTime();

	// Disable show end result.
	ip->EnableShowEndResult(FALSE);



	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);

	SetAFlag(A_MOD_BEING_EDITED);

	normalizeSplineDesc.BeginEditParams(ip, this, flags, prev);
/*
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGTH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_WELD),
		GetString(IDS_RB_PARAMETERS),
		0);	
*/
	}

void NormalizeSpline::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
	{
	this->ip = NULL;
//reanable show end result
	ip->EnableShowEndResult(TRUE);
	
	TimeValue t = ip->GetTime();

	// aszabo|feb.05.02 This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	normalizeSplineDesc.EndEditParams(ip, this, flags, next);

//	DestroyCPParamMap(pmapParam);


	}

Interval NormalizeSpline::GetValidity(TimeValue t)
	{
	float f;
	Interval valid = FOREVER;

	// Start our interval at forever...
	// Intersect each parameters interval to narrow it down.
	pblock->GetValue(nspline_length,t,f,valid);

	return valid;
	}

RefResult NormalizeSpline::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
/*	switch (message) {

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}

		}
*/
	return(REF_SUCCEED);
	}

ParamDimension *NormalizeSpline::GetParameterDim(int pbIndex)
	{
	return defaultDim;
	}

TSTR NormalizeSpline::GetParameterName(int pbIndex)
	{
	return TSTR(_T(""));
//	switch (pbIndex) {
//		case PB_NLENGTH:	return TSTR(_T("Normal Length"));
//		default:		
//		}
	}

IOResult NormalizeSpline::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &nspline_param_blk, this, PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);

//	iload->RegisterPostLoadCallback(
//		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}




void NormalizeSpline::BuildSkin(TimeValue t,ModContext &mc, ObjectState * os) {


SplineShape *shape = (SplineShape *)os->obj->ConvertToType(t,splineShapeClassID);


int polys = shape->shape.splineCount;
int poly;


float cinc = 0.0f;
float TotalLength = 0.0f;

for(poly = 0; poly < polys; ++poly) 
	{
	shape->shape.splines[poly]->ComputeBezPoints();

	TotalLength += shape->shape.splines[poly]->SplineLength();
	}

cinc = 	nlength/TotalLength;

cinc = cinc/10.0f;
if (cinc>0.001f) cinc = 0.001f;

for(poly = 0; poly < polys; ++poly) 
	{
//get spline
//get number segs

//get points
	float SegLength;

	SegLength = nlength*nlength;

	float inc = 0.001f;
	float CurrentPercent = 0.0f;

	inc = cinc;
	Point3 CurrentPoint,NextPoint;
	Tab<Point3> PointList;
	PointList.ZeroCount();

	while (CurrentPercent < 1.0)
		{
		CurrentPoint = shape->shape.splines[poly]->InterpCurve3D(CurrentPercent);
		PointList.Append(1,&CurrentPoint,1);
		NextPoint = CurrentPoint;
		while ((LengthSquared(CurrentPoint-NextPoint)<SegLength) && (CurrentPercent <1.0f))
			{
			CurrentPercent += inc;
			NextPoint = shape->shape.splines[poly]->InterpCurve3D(CurrentPercent);
			}
		}
   int i,closed;
   closed = shape->shape.splines[poly]->Closed();
   if (!shape->shape.splines[poly]->Closed())
		{
		NextPoint = shape->shape.splines[poly]->GetKnotPoint(shape->shape.splines[poly]->KnotCount()-1);
		PointList.Append(1,&NextPoint,1);
		}
	shape->shape.splines[poly]->NewSpline();
//add new points
	if (closed)
		shape->shape.splines[poly]->SetClosed();
		else shape->shape.splines[poly]->SetOpen();

	for (i=0;i<PointList.Count();i++)
		{
		shape->shape.splines[poly]->AddKnot(SplineKnot(KTYPE_AUTO,LTYPE_CURVE,
					PointList[i],PointList[i],PointList[i]));
		}

	if (shape->shape.splines[poly]->KnotCount() == 1)
		shape->shape.splines[poly]->AddKnot(SplineKnot(KTYPE_AUTO,LTYPE_CURVE,
					PointList[PointList.Count()-1],PointList[PointList.Count()-1],PointList[PointList.Count()-1]));

	shape->shape.splines[poly]->ComputeBezPoints();
	for (i=0;i<shape->shape.splines[poly]->KnotCount();i++)
		shape->shape.splines[poly]->SetKnotType(i,KTYPE_AUTO);


	shape->shape.splines[poly]->ComputeBezPoints();


	}

shape->shape.UpdateSels();	// Make sure it readies the selection set info
shape->shape.InvalidateGeomCache();

			


}


#endif // NO_MODIFIER_NORMALIZE_SPLINE