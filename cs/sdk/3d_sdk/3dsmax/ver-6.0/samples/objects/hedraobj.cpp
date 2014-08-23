/**********************************************************************
 *<
	FILE: hedraobj.cpp

	DESCRIPTION:  Hedra

	CREATED BY: Rolf Berteig (from Dan's hedra code)

	HISTORY: created 10/14/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "prim.h"
#include "iparamm.h"
#include "Simpobj.h"

#if !defined(NO_EXTENDED_PRIMITIVES) && !defined(NO_OBJECT_HEDRA)

class HedraObject : public SimpleObject {
	public:			
		// Class vars
		static IParamMap *pmapParam;
		static int dlgFamily;		
		static IObjParam *ip;

		HedraObject();		
		
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_RB_HEDRA);}
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
				
		// Animatable methods		
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return Class_ID(HEDRA_CLASS_ID,0); } 
		
		// From ReferenceTarget
		IOResult Load(ILoad *iload);
						
		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
	};




//--- ClassDescriptor and class vars ---------------------------------

// The class descriptor for hedra
class HedraClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new HedraObject; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_HEDRA_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(HEDRA_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_AP_EXTENDED);  }
	void			ResetClassParams(BOOL fileReset);
	};

static HedraClassDesc hedraDesc;
extern ClassDesc* GetHedraDesc() { return &hedraDesc; }


// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

int HedraObject::dlgFamily        = 0;
IParamMap *HedraObject::pmapParam = NULL;
IObjParam *HedraObject::ip        = NULL;

void HedraClassDesc::ResetClassParams(BOOL fileReset)
	{
	HedraObject::dlgFamily = 0;
	}


//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_RADIUS	0
#define PB_FAMILY	1
#define PB_P		2
#define PB_Q		3
#define PB_SCALEP	4
#define PB_SCALEQ	5
#define PB_SCALER	6
#define PB_VERTS	7
#define PB_GENUVS	8

//
//
// Parameters

static int familyIDs[] = {IDC_FAM_TETRA,IDC_FAM_OCTA,IDC_FAM_DODEC,IDC_FAM_STAR1,IDC_FAM_STAR2};
static int vertIDs[] = {IDC_VERT_BASIC,IDC_VERT_CENTER,IDC_VERT_SIDES};

static ParamUIDesc descParam[] = {
	// Family
	ParamUIDesc(PB_FAMILY,TYPE_RADIO,familyIDs,5),

	// P
	ParamUIDesc(
		PB_P,
		EDITTYPE_FLOAT,
		IDC_PARAM_P,IDC_PARAM_PSPIN,
		0.0f, 1.0f,
		0.01f),	
	
	// Q
	ParamUIDesc(
		PB_Q,
		EDITTYPE_FLOAT,
		IDC_PARAM_Q,IDC_PARAM_QSPIN,
		0.0f, 1.0f,
		0.01f),	
	
	// Axis scale P
	ParamUIDesc(
		PB_SCALEP,
		EDITTYPE_FLOAT,
		IDC_SCALE_P,IDC_SCALE_PSPIN,
		0.0f, float(1.0E30),
		0.5f,
		stdPercentDim),	
	
	// Axis scale Q
	ParamUIDesc(
		PB_SCALEQ,
		EDITTYPE_FLOAT,
		IDC_SCALE_Q,IDC_SCALE_QSPIN,
		0.0f, float(1.0E30),
		0.5f,
		stdPercentDim),
	
	// Axis scale R
	ParamUIDesc(
		PB_SCALER,
		EDITTYPE_FLOAT,
		IDC_SCALE_R,IDC_SCALE_RSPIN,
		0.0f, float(1.0E30),
		0.5f,
		stdPercentDim),	
	
	// Vertices
	ParamUIDesc(PB_VERTS,TYPE_RADIO,vertIDs,3),

	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADIUSSPIN,
		0.0f, float(1.0E30),
		SPIN_AUTOSCALE),

	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),
	};
#define PARAMDESC_LENGH 9

#define PB_RADIUS	0
#define PB_FAMILY	1
#define PB_P		2
#define PB_Q		3
#define PB_SCALEP	4
#define PB_SCALEQ	5
#define PB_SCALER	6
#define PB_VERTS	7

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_FLOAT, NULL, TRUE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		// radius
	{ TYPE_INT, NULL, TRUE, 1 },		// family
	{ TYPE_FLOAT, NULL, TRUE, 2 },		// P
	{ TYPE_FLOAT, NULL, TRUE, 3 },		// Q
	{ TYPE_FLOAT, NULL, TRUE, 4 },		// Scale P
	{ TYPE_FLOAT, NULL, TRUE, 5 },		// Scale Q
	{ TYPE_FLOAT, NULL, TRUE, 6 },		// Scale R
	{ TYPE_INT, NULL, FALSE, 7 },		// verts
	{ TYPE_INT, NULL, FALSE, 8 } };		// gen UVs

#define PBLOCK_LENGTH	9

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,8,0),
	ParamVersionDesc(descVer1,9,1)	
	};
#define NUM_OLDVERSIONS	2

#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


// --- HedraDlgProc ------------------------------
//
// Constrain P+Q < 1

class HedraDlgProc : public ParamMapUserDlgProc {
	public:
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {}
	};

BOOL HedraDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_SCALE_RESET: {
					IParamArray *pa = map->GetParamBlock();
					pa->SetValue(PB_SCALEP,t,1.0f);
					pa->SetValue(PB_SCALEQ,t,1.0f);
					pa->SetValue(PB_SCALER,t,1.0f);
					map->Invalidate();
					return REDRAW_VIEWS;
					}
				}
			break;

		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {				
				case IDC_PARAM_PSPIN: {
					IParamArray *pa = map->GetParamBlock();
					float p, q;
					pa->GetValue(PB_P,t,p,FOREVER);
					pa->GetValue(PB_Q,t,q,FOREVER);
					if (p+q>1.0f) {
						q = 1.0f-p;
						pa->SetValue(PB_Q,t,q);
						map->Invalidate();
						}
					return TRUE;
					}

				case IDC_PARAM_QSPIN: {
					IParamArray *pa = map->GetParamBlock();
					float p, q;
					pa->GetValue(PB_P,t,p,FOREVER);
					pa->GetValue(PB_Q,t,q,FOREVER);
					if (p+q>1.0f) {
						p = 1.0f-q;
						pa->SetValue(PB_P,t,p);
						map->Invalidate();
						}
					return TRUE;
					}
				}
			break;
		}
	return FALSE;
	}
static HedraDlgProc theHedraProc;


//--- Hedra methods -------------------------------

HedraObject::HedraObject()
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
		
	pblock->SetValue(PB_FAMILY,0,dlgFamily);		
	pblock->SetValue(PB_SCALEP,0,1.0f);
	pblock->SetValue(PB_SCALEQ,0,1.0f);
	pblock->SetValue(PB_SCALER,0,1.0f);

	pblock->SetValue(PB_GENUVS,0,TRUE);
	}

IOResult HedraObject::Load(ILoad *iload) 
	{	
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}

void HedraObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapParam) {
		
		// Left over from last Hedra ceated		
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.		
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_HEDRAPARAM),
			GetString(IDS_RB_PARAMETERS),
			0);
		}
	
	pmapParam->SetUserDlgProc(&theHedraProc);
	}
		
void HedraObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{		
	SimpleObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {		
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;		
		}

	// Save these values in class variables so the next object created will inherit them.
	pblock->GetValue(PB_FAMILY,ip->GetTime(),dlgFamily,FOREVER);	
	}


extern void CreateHedron(Mesh &mesh, 
		int family, float fp, float fq, float radius,
		int axis, int vts, float *scale_axis);


BOOL HedraObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void HedraObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}


void HedraObject::BuildMesh(TimeValue t)
	{
	int family;
	float fp, fq, radius, scale_axis[3];
	int axis = 0, vts;
	int genUVs = TRUE;
	
	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	pblock->GetValue(PB_FAMILY, t, family, ivalid);
	pblock->GetValue(PB_P, t, fp, ivalid);
	pblock->GetValue(PB_Q, t, fq, ivalid);
	pblock->GetValue(PB_SCALEP, t, scale_axis[0], ivalid);
	pblock->GetValue(PB_SCALEQ, t, scale_axis[1], ivalid);
	pblock->GetValue(PB_SCALER, t, scale_axis[2], ivalid);
	pblock->GetValue(PB_VERTS, t, vts, ivalid);
	pblock->GetValue(PB_GENUVS, t, genUVs, ivalid);
	
	LimitValue(radius, 0.0f, float(1.0E30));
	LimitValue(family, 0, 4);
	LimitValue(vts, 0, 2);
	LimitValue(fp, 0.0f, 1.0f);
	LimitValue(fq, 0.0f, 1.0f);

	// Constrain p+q < 1
	if (fp+fq>1.0f) {
		float s = 1.0f/(fp+fq);
		fp *= s;
		fq *= s;
		}

	CreateHedron(mesh, family, fp, fq, radius, axis, vts, scale_axis);
	mesh.InvalidateGeomCache();
	Box3 box = mesh.getBoundingBox();

	if (genUVs) {
		// Do a simple box mapping on individual faces
		mesh.setNumTVerts(mesh.getNumFaces()*3);
		mesh.setNumTVFaces(mesh.getNumFaces());
		int nv=0, nf=0;
		for (int i=0; i<mesh.getNumFaces(); i++) {
			Point3 v[] = {mesh.verts[mesh.faces[i].v[0]],mesh.verts[mesh.faces[i].v[1]],mesh.verts[mesh.faces[i].v[2]]};			
			Point3 norm = Normalize((v[1]-v[0])^(v[2]-v[1]));
			int m=0;
			if (fabs(norm[1])>fabs(norm[m])) m = 1;
			if (fabs(norm[2])>fabs(norm[m])) m = 2;
			int i1 = (m+1)%3;
			int i2 = (m+2)%3;
			mesh.setTVert(nv++,
				v[0][i1]/box.Width()[i1],
				v[0][i2]/box.Width()[i2],
				v[0][m]/box.Width()[m]);
			mesh.setTVert(nv++,
				v[1][i1]/box.Width()[i1],
				v[1][i2]/box.Width()[i2],
				v[1][m]/box.Width()[m]);
			mesh.setTVert(nv++,
				v[2][i1]/box.Width()[i1],
				v[2][i2]/box.Width()[i2],
				v[2][m]/box.Width()[m]);
			mesh.tvFace[nf++].setTVerts(nv-3,nv-2,nv-1);
			}
		}

	mesh.InvalidateTopologyCache();
	}

class HedraObjCreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;
	HedraObject *ob;
	Point3 p0;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		void SetObj(HedraObject *obj) {ob = obj;}
	};

int HedraObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	Point3 p1,center;
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _OSNAP
			#ifdef _3D_CREATE
				vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
			#else
				vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
			#endif
		#endif
	}
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:  // only happens with MOUSE_POINT msg
				ob->pblock->SetValue(PB_RADIUS,0,0.0f);
				ob->suspendSnap = TRUE;				
				sp0 = m;
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p0);
				break;
			
			case 1:
				mat.IdentityMatrix();				
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				r = Length(p1-p0);
				mat.SetTrans(p0);
				 
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p1.y-p0.y,p1.x-p0.x);					
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) {										
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3 || Length(p1-p0)<0.1f)?CREATE_ABORT:CREATE_STOP;
					}
				break;					   
			}
		}
	else
	if (msg == MOUSE_ABORT) {		
		return CREATE_ABORT;
		}

	return TRUE;
	}

static HedraObjCreateCallBack hedraCreateCB;

CreateMouseCallBack* HedraObject::GetCreateMouseCallBack() 
	{
	hedraCreateCB.SetObj(this);
	return(&hedraCreateCB);
	}


BOOL HedraObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}

void HedraObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *HedraObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:	return stdWorldDim;
		case PB_FAMILY:	return defaultDim;
		case PB_P:		return stdNormalizedDim;
		case PB_Q:		return stdNormalizedDim;
		case PB_SCALEP:	return stdPercentDim;
		case PB_SCALEQ:	return stdPercentDim;
		case PB_SCALER:	return stdPercentDim;		
		default: return defaultDim;
		}
	}

TSTR HedraObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:	return GetString(IDS_RB_RADIUS);
		case PB_FAMILY:	return GetString(IDS_RB_FAMILY);
		case PB_P:		return GetString(IDS_RB_PVALUE);
		case PB_Q:		return GetString(IDS_RB_QVALUE);
		case PB_SCALEP:	return GetString(IDS_RB_PSCALE);
		case PB_SCALEQ:	return GetString(IDS_RB_QSCALE);
		case PB_SCALER:	return GetString(IDS_RB_RSCALE);
		default: return TSTR(_T("Parameter"));
		}
	}

RefTargetHandle HedraObject::Clone(RemapDir& remap) 
	{
	HedraObject* newob = new HedraObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

#endif // !defined(NO_EXTENDED_PRIMITIVES) && !defined(NO_OBJECT_HEDRA)

