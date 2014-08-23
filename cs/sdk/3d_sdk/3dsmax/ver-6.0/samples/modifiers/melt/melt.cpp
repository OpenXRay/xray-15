/*===========================================================================*\
 | 
 |  FILE:	melt.cpp
 |			Simple melting modifier
 | 
 |  AUTHOR: Harry Denholm
 |			All Rights Reserved. Copyright(c) Kinetix 1998
 |
 |	HIST:	3-6-98 : Ported
 |					 This is pretty self-contained. Bit messy..
 | 
\*===========================================================================*/


#include "max.h"
#include "iparamm.h"
#include "simpmod.h"
#include "resource.h"


#define BIGFLOAT	float(999999)
float sign(float x) { return (x < 0.0f ? -1.0f : 1.0f); }


#define MELT_ID1 0x36d04fa5
#define MELT_ID2 0x500727b3


// The DLL instance handle
HINSTANCE hInstance;
extern TCHAR *GetString(int sid);



/*===========================================================================*\
 | Melt Modifier
\*===========================================================================*/

class MeltMod : public SimpleMod {	
	public:
		static IParamMap *pmapParam;

		MeltMod();

		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= TSTR(GetString(IDS_MELTMOD)); }  
		virtual Class_ID ClassID() { return Class_ID(MELT_ID1,MELT_ID2);}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_MELTMOD); }
						
		IOResult Load(ILoad *iload);

		// From simple mod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,
			Matrix3& invmat);		
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void InvalidateUI() {if (pmapParam) pmapParam->Invalidate();}
};


/*===========================================================================*\
 | Deformer (map) function
\*===========================================================================*/

class MeltDeformer: public Deformer {
	public:
		float cx, cy, cz, xsize, ysize, zsize, size;
		Matrix3 tm,invtm;
		TimeValue time;
		Box3 bbox;

		float bulger;
		float ybr,zbr,visvaluea;
		int confiner,axis,vistypea,negaxis;


		MeltDeformer();
		MeltDeformer(
			TimeValue t, ModContext &mc,
			float bulgea,float yba,float zba ,int confinea, int vistype, 
			float visvalue, int axisa, int negaxisa,
			Matrix3& modmat, Matrix3& modinv);
		void SetAxis(Matrix3 &tmAxis);

		Point3 Map(int i, Point3 p); 
	};



IParamMap *MeltMod::pmapParam = NULL;




/*===========================================================================*\
 | Variable Handling Stuff & PUID
\*===========================================================================*/

#define PB_MELTAMT	0
#define PB_CONFINE	1
#define PB_YB		2
#define PB_ZB		3

#define PB_VISTYPE  4
#define PB_VISVAL   5

#define PB_AXIS		6
#define PB_NEGAXIS	7


static int visIDs[] = {IDC_S1,IDC_S2,IDC_S3,IDC_S4,IDC_S5};
static int axisIDs[] = {IDC_AX,IDC_AY,IDC_AZ};


static ParamUIDesc descParam[] = {
	ParamUIDesc(
		PB_MELTAMT,
		EDITTYPE_FLOAT,
		IDC_AMT_EDIT,IDC_AMT_SPIN,
		0.0f,1000.0f,
		1.0f),
	ParamUIDesc(
		PB_YB,
		EDITTYPE_FLOAT,
		IDC_SPREAD_EDIT,IDC_SPREAD_SPIN,
		0.0f,100.0f,
		0.1f),
	ParamUIDesc(PB_VISTYPE,TYPE_RADIO,visIDs,5),
	ParamUIDesc(
		PB_VISVAL,
		EDITTYPE_FLOAT,
		IDC_VIS_EDIT,IDC_VIS_SPIN,
		0.2f,30.0f,
		0.02f),
	ParamUIDesc(PB_AXIS,TYPE_RADIO,axisIDs,3),
	ParamUIDesc(PB_NEGAXIS,TYPE_SINGLECHEKBOX,IDC_NEGATIVE),
};

#define PARAMDESC_LENGH 6



/*===========================================================================*\
 | ParamBlocks
\*===========================================================================*/

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
 };

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
 };

static ParamBlockDescID descVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
 };

static ParamBlockDescID descVer4[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_INT, NULL, FALSE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_FLOAT, NULL, TRUE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_INT, NULL, FALSE, 7 },
 };

#define PBLOCK_LENGTH	8

/*===========================================================================*\
 | Old Versions (as this is a port, keep it compatible)
\*===========================================================================*/

static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer1,4,1),
	ParamVersionDesc(descVer2,6,2),
	ParamVersionDesc(descVer3,7,3)
	};
#define NUM_OLDVERSIONS	3


#define CURRENT_VERSION	4
static ParamVersionDesc curVersion(descVer4,PBLOCK_LENGTH,CURRENT_VERSION);




/*===========================================================================*\
 | Register PLCB at load time
\*===========================================================================*/

IOResult MeltMod::Load(ILoad *iload)
	{
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,
			SIMPMOD_PBLOCKREF));
	SimpleMod::Load(iload);
	return IO_OK;
	}



/*===========================================================================*\
 | Melt Stuff
\*===========================================================================*/

MeltMod::MeltMod() : SimpleMod()
	{
	MakeRefByID(FOREVER, SIMPMOD_PBLOCKREF, 
		CreateParameterBlock(descVer4, PBLOCK_LENGTH, CURRENT_VERSION));

	pblock->SetValue(PB_MELTAMT,0,0);

	pblock->SetValue(PB_YB,0,19.0f);
	pblock->SetValue(PB_ZB,0,0);

	pblock->SetValue(PB_CONFINE,0,0);
	pblock->SetValue(PB_VISTYPE,0,0);
	pblock->SetValue(PB_VISVAL,0,1.0f);

	pblock->SetValue(PB_AXIS,0,0);
	pblock->SetValue(PB_NEGAXIS,0,0);
	}



/*===========================================================================*\
 | BeginEditParams - called when user opens mod
\*===========================================================================*/

void MeltMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleMod::BeginEditParams(ip,flags,prev);
		
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_MELT),
		GetString(IDS_TITLE),
		0);	
	}
		


/*===========================================================================*\
 | EndEditParams - Hmm guess what this does
\*===========================================================================*/

void MeltMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	SimpleMod::EndEditParams(ip,flags,next);
	DestroyCPParamMap(pmapParam);
	}



/*===========================================================================*\
 | Calculate validity
\*===========================================================================*/

Interval MeltMod::GetValidity(TimeValue t)
	{
	float f;

	Interval valid = FOREVER;

	pblock->GetValue(PB_MELTAMT,t,f,valid);
	pblock->GetValue(PB_YB,t,f,valid);
	pblock->GetValue(PB_ZB,t,f,valid);
	pblock->GetValue(PB_CONFINE,t,f,valid);
	pblock->GetValue(PB_VISTYPE,t,f,valid);
	pblock->GetValue(PB_VISVAL,t,f,valid);
	pblock->GetValue(PB_AXIS,t,f,valid);
	pblock->GetValue(PB_NEGAXIS,t,f,valid);

	return valid;
}



/*===========================================================================*\
 | Clone Method
\*===========================================================================*/

RefTargetHandle MeltMod::Clone(RemapDir& remap) {	
	MeltMod* newmod = new MeltMod();	
	newmod->ReplaceReference(SIMPMOD_PBLOCKREF,pblock->Clone(remap));
	newmod->SimpleModClone(this);
	BaseClone(this, newmod, remap);
	return(newmod);
	}




/*===========================================================================*\
 | Melt Deformer
\*===========================================================================*/

MeltDeformer::MeltDeformer() 
	{ 
	tm.IdentityMatrix();
	invtm = Inverse(tm);
	time = 0;	

	// to make purify happy:
	cx=cy=cz=xsize=ysize=zsize=size=bulger=ybr=zbr=visvaluea=0.0f;
	bbox.Init();
	confiner=axis=vistypea=negaxis=0;
	}

void MeltDeformer::SetAxis(Matrix3 &tmAxis)
	{
	Matrix3 itm = Inverse(tmAxis);
	tm    = tm*tmAxis;
	invtm =	itm*invtm;
	}



/*===========================================================================*\
 | Actual deforming function
\*===========================================================================*/

Point3 MeltDeformer::Map(int i, Point3 p)
	{

	float x, y, z;
	float xw,yw,zw;
	float vdist,mfac,dx,dy;
	float defsinex,coldef,realmax;
	
	// Mult by mc
	p = p*tm;


	x = p.x; y = p.y; z = p.z;
	xw= x-cx; yw= y-cy; zw= z-cz;

	if(xw==0.0 && yw==0.0 && zw==0.0) xw=yw=zw=1.0f; // Kill singularity for XW,YW,ZW
	if(x==0.0 && y==0.0 && z==0.0) x=y=z=1.0f; // Kill singularity for XYZ

	// Find distance from centre
	vdist=(float) sqrt(xw*xw+yw*yw+zw*zw);
	
	mfac=size/vdist;

	if(axis==0){
		dx = xw+sign(xw)*((float) (fabs(xw*mfac))*(bulger*ybr));
		dy = yw+sign(yw)*((float) (fabs(yw*mfac))*(bulger*ybr));
		x=(dx+cx);
		y=(dy+cy);
	}
	if(axis==1){
		dx = xw+sign(xw)*((float) (fabs(xw*mfac))*(bulger*ybr));
		dy = zw+sign(zw)*((float) (fabs(zw*mfac))*(bulger*ybr));
		x=(dx+cx);
		z=(dy+cz);
	}
	if(axis==2){
		dx = zw+sign(zw)*((float) (fabs(zw*mfac))*(bulger*ybr));
		dy = yw+sign(yw)*((float) (fabs(yw*mfac))*(bulger*ybr));
		z=(dx+cz);
		y=(dy+cy);
	}


	if(axis==0) if(p.z<(bbox.pmin.z+zbr)) goto skipmelt;
	if(axis==1) if(p.y<(bbox.pmin.y+zbr)) goto skipmelt;
	if(axis==2) if(p.x<(bbox.pmin.x+zbr)) goto skipmelt;


	if(axis==0) realmax = (float)hypot( (bbox.pmax.x-cx),(bbox.pmax.y-cy) );
	if(axis==1) realmax = (float)hypot( (bbox.pmax.x-cx),(bbox.pmax.z-cz) );
	if(axis==2) realmax = (float)hypot( (bbox.pmax.z-cz),(bbox.pmax.y-cy) );


	if(axis==0){
	defsinex = (float)hypot( (x-cx),(y-cy) );
	coldef = realmax - (float)hypot( (x-cx),(y-cy) );
	}
	if(axis==1){
	defsinex = (float)hypot( (x-cx),(z-cz) );
	coldef = realmax - (float)hypot( (x-cx),(z-cz) );
	}
	if(axis==2){
	defsinex = (float)hypot( (z-cz),(y-cy) );
	coldef = realmax - (float)hypot( (z-cz),(y-cy) );
	}


	if (coldef<0.0f) coldef=0.0f;


	defsinex+=(coldef/visvaluea);

	// Melt me!
	if(axis==0){
		if(!negaxis)
		{
			z-=(defsinex*bulger);
			if(z<=bbox.pmin.z) z=bbox.pmin.z;
			if(z<=(bbox.pmin.z+zbr)) z=(bbox.pmin.z+zbr);
		}
		else
		{
			z+=(defsinex*bulger);
			if(z>=bbox.pmax.z) z=bbox.pmax.z;
			if(z>=(bbox.pmax.z+zbr)) z=(bbox.pmax.z+zbr);
		}
	}
	if(axis==1){
		if(!negaxis)
		{
			y-=(defsinex*bulger);
			if(y<=bbox.pmin.y) y=bbox.pmin.y;
			if(y<=(bbox.pmin.y+zbr)) y=(bbox.pmin.y+zbr);
		}
		else
		{
			y+=(defsinex*bulger);
			if(y>=bbox.pmax.y) y=bbox.pmax.y;
			if(y>=(bbox.pmax.y+zbr)) y=(bbox.pmax.y+zbr);
		}
	}
	if(axis==2){
		if(!negaxis)
		{
			x-=(defsinex*bulger);
			if(x<=bbox.pmin.x) x=bbox.pmin.x;
			if(x<=(bbox.pmin.x+zbr)) x=(bbox.pmin.x+zbr);
		}
		else
		{
			x+=(defsinex*bulger);
			if(x>=bbox.pmax.x) x=bbox.pmax.x;
			if(x>=(bbox.pmax.x+zbr)) x=(bbox.pmax.x+zbr);
		}
	}


	// [jump point] don't melt this point...
	skipmelt:

	p.x = x; p.y = y; p.z = z;
	p = p*invtm;
	return p;

}


MeltDeformer::MeltDeformer(
		TimeValue t, ModContext &mc,
			float bulgea, float yba, float zba,int confinea, int vistype, 
			float visvalue, int axisa, int negaxisa,
		Matrix3& modmat, Matrix3& modinv) 
	{
	// Save the tm and inverse tm
	tm = modmat; invtm = modinv;
	time = t; // mjm - 5.14.99

	ybr = yba;
	zbr = zba;
	bulger = bulgea;
	confiner = confinea;
	vistypea=vistype;
	visvaluea=visvalue;
	axis=axisa;
	negaxis=negaxisa;

	// Save the bounding box
	assert(mc.box);
	bbox = *mc.box;
	cx = bbox.Center().x;
	cy = bbox.Center().y;
	cz = bbox.Center().z;
	// Compute the size and center
	xsize = bbox.pmax.x - bbox.pmin.x;
	ysize = bbox.pmax.y - bbox.pmin.y;
	zsize = bbox.pmax.z - bbox.pmin.z;
	size=(xsize>ysize) ? xsize:ysize;
	size=(zsize>size) ? zsize:size;
	size /= 2.0f;
	ybr/= 100.0f;
	zbr/= 10.0f;
	bulger/=100.0f;
} 


// Provides a reference to our callback object to handle the deformation.
Deformer& MeltMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	float yb,zb,bulge,visvaluer;
	int confine,vistyper,axis,negaxis;
	pblock->GetValue(PB_MELTAMT,t,bulge,FOREVER);

	pblock->GetValue(PB_YB,t,yb,FOREVER);
	pblock->GetValue(PB_ZB,t,zb,FOREVER);
	pblock->GetValue(PB_CONFINE,t,confine,FOREVER);

	pblock->GetValue(PB_VISTYPE,t,vistyper,FOREVER);
	pblock->GetValue(PB_VISVAL,t,visvaluer,FOREVER);
	pblock->GetValue(PB_AXIS,t,axis,FOREVER);
	pblock->GetValue(PB_NEGAXIS,t,negaxis,FOREVER);

	// Evaluate the presets to values
	// used to be called Viscosity, which is why this is vis<...>
	if(vistyper==0) visvaluer=2.0f;
	if(vistyper==1) visvaluer=12.0f;
	if(vistyper==2) visvaluer=0.4f;
	if(vistyper==3) visvaluer=0.7f;
	if(vistyper==4) visvaluer=visvaluer;

	// Build and return deformer
	static MeltDeformer deformer;
	deformer = MeltDeformer(t,mc,bulge,yb,zb,confine,vistyper,visvaluer,
		axis,negaxis,mat,invmat);

	return deformer;
	}



/*===========================================================================*\
 | Get parameter types
\*===========================================================================*/

ParamDimension *MeltMod::GetParameterDim(int pbIndex)
	{
	return defaultDim;  
	}



/*===========================================================================*\
 | Returns parameter names
\*===========================================================================*/

TSTR MeltMod::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_MELTAMT:		return TSTR(GetString(IDS_MELTAMOUNT));
		case PB_YB:			return TSTR(GetString(IDS_SPREAD));
		case PB_ZB:			return TSTR(GetString(IDS_CUTOFF));
		case PB_CONFINE:	return TSTR(GetString(IDS_CONFINE));
		case PB_VISTYPE:	return TSTR(GetString(IDS_SOLIDITY));
		case PB_VISVAL:		return TSTR(GetString(IDS_SOLIDITYVAL));
		case PB_AXIS:		return TSTR(GetString(IDS_AXIS));
		case PB_NEGAXIS:	return TSTR(GetString(IDS_NEGAXIS));
		default:			return TSTR(_T(""));
		}
	}



/*===========================================================================*\
 | The Class Descriptor
\*===========================================================================*/
class DecayClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new MeltMod; }
	const TCHAR *	ClassName() { return GetString(IDS_MELTMOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(MELT_ID1,MELT_ID2); }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }
	};

static DecayClassDesc decayDesc;
ClassDesc* GetMeltModDesc() { return &decayDesc; }




/*===========================================================================*\
 | The DLL Functions
\*===========================================================================*/

int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;

	if (! controlsInit) {
		controlsInit = TRUE;
		
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	
	return(TRUE);
	}




/*===========================================================================*\
 | Plugin interface code
\*===========================================================================*/

__declspec( dllexport ) int LibNumberClasses() {return 1;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetMeltModDesc();		
		default: return 0;
		}
	}

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESC); }

__declspec( dllexport ) ULONG
LibVersion() {  return VERSION_3DSMAX; }

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
