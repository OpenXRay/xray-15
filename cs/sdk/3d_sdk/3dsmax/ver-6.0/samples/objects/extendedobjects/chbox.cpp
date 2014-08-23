/*****************************************************************************
 *<
	FILE: chbox.cpp

	DESCRIPTION: Chbox - builds filleted/chamfered boxes

	CREATED BY:  Audrey Peterson
	Copyright (c) 1996 All Rights Reserved
 *>
 *****************************************************************************/
// Resource include file.
#include "solids.h"
#include "iparamm.h"
#include "simpobj.h"

// Unique Class ID.  It is specified as two 32-bit quantities.
#define SCS_C_CLASS_ID1 0x1AD73F40
#define SCS_C_CLASS_ID2 0x48EA0F97
#define sidesmooth 0
#define chsmooth 1
#define topsquare 0
#define messyedge 1
#define middle 2
#define bottomedge 3
#define bottomsquare 4

typedef struct{
  int surface,deltavert;
} chinfo;

class ChBoxObject : public SimpleObject, public IParamArray {
	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;		
		static IObjParam *ip;
		static int dlgLSegs;
		static int dlgWSegs;
		static int dlgHSegs;
		static int dlgCSegs;
		static int createMeth;
		static Point3 crtPos;		
		static float crtWidth, crtHeight, crtLength, crtRadius;

		ChBoxObject();
		
		// From Object
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);

		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_RB_CHNAME); }
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);

		// Animatable methods
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID(SCS_C_CLASS_ID1,SCS_C_CLASS_ID2); }  
		
		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);

		// From IParamArray
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);		
	};				


// Misc stuff
#define CMIN_RADIUS		float(0.0)
#define BMIN_LENGTH		float(0.1)
#define BMAX_LENGTH		float(1.0E30)
#define BMIN_WIDTH		float(0.1)
#define BMAX_WIDTH		float(1.0E30)
#define BMIN_HEIGHT		float(-1.0E30)
#define BMAX_HEIGHT		float(1.0E30)

#define BDEF_DIM		float(0)
#define BDEF_SEGS		1
#define CDEF_SEGS		3

#define MIN_SEGMENTS	1
#define MAX_SEGMENTS	200


//--- Parameter map/block descriptors -------------------------------

// The parameter map descriptors define the properties of a parameter
// such as the type (spinner, radio button, check box, etc.), which
// resource ID they refer to, and which index into the virtual array
// they use.

// Parameter block indices
#define PB_LENGTH	0
#define PB_WIDTH	1
#define PB_HEIGHT	2
#define PB_RADIUS	3
#define PB_LSEGS	4
#define PB_WSEGS	5
#define PB_HSEGS	6
#define PB_CSEGS	7
#define PB_GENUVS	8
#define PB_SMOOTH	9

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_LENGTH		2
#define PB_TI_WIDTH			3
#define PB_TI_HEIGHT		4
#define PB_TI_RADIUS		5

//	Creation method

static int createMethIDs[] = {IDC_CC_CREATEBOX,IDC_CC_CREATECUBE};

static ParamUIDesc descCreate[] = {
	// Diameter/radius
	ParamUIDesc(PB_CREATEMETHOD,TYPE_RADIO,createMethIDs,2)
	};
#define CREATEDESC_LENGTH 1

// Type in
static ParamUIDesc descTypeIn[] = {
	
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_CC_POSX,IDC_CC_POSXSPIN,
		IDC_CC_POSY,IDC_CC_POSYSPIN,
		IDC_CC_POSZ,IDC_CC_POSZSPIN,
		float(-1.0E30),float(1.0E30),
		SPIN_AUTOSCALE),
	
	// Length
	ParamUIDesc(
		PB_TI_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_CC_LENGTH,IDC_CC_LENGTHSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Width
	ParamUIDesc(
		PB_TI_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_CC_WIDTH,IDC_CC_WIDTHSPIN,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Height
	ParamUIDesc(
		PB_TI_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_CC_HEIGHT,IDC_CC_HEIGHTSPIN,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	

	// Radius
	ParamUIDesc(
		PB_TI_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_CC_RADIUS,IDC_CC_RADIUSSPIN,
		CMIN_RADIUS,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	};
#define TYPEINDESC_LENGTH 5

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_CC_LENGTH,IDC_CC_LENGTHSPIN,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
	
	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_CC_WIDTH,IDC_CC_WIDTHSPIN,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	

	// Height
	ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_CC_HEIGHT,IDC_CC_HEIGHTSPIN,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	

	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_CC_RADIUS,IDC_CC_RADIUSSPIN,
		CMIN_RADIUS,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
	
	// Length Segments
	ParamUIDesc(
		PB_LSEGS,
		EDITTYPE_INT,
		IDC_CC_LENSEGS,IDC_CC_LENSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Width Segments
	ParamUIDesc(
		PB_WSEGS,
		EDITTYPE_INT,
		IDC_CC_WIDSEGS,IDC_CC_WIDSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Height Segments
	ParamUIDesc(
		PB_HSEGS,
		EDITTYPE_INT,
		IDC_CC_HGTSEGS,IDC_CC_HGTSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

		// Fillet Segments
	ParamUIDesc(
		PB_CSEGS,
		EDITTYPE_INT,
		IDC_CC_RADSEGS,IDC_CC_RADSEGSSPIN,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),

	// Gen Smoothing
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_OT_SMOOTH),			
	
	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),			
	};
#define PARAMDESC_LENGTH 10


ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 }, 
	{ TYPE_INT, NULL, TRUE, 4 }, 
	{ TYPE_INT, NULL, TRUE, 5 }, 
	{ TYPE_INT, NULL, TRUE, 6 }, 
	{ TYPE_INT, NULL, TRUE, 7 }, 
	{ TYPE_INT, NULL, FALSE, 8 }, 
	};

ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 }, 
	{ TYPE_INT, NULL, TRUE, 4 }, 
	{ TYPE_INT, NULL, TRUE, 5 }, 
	{ TYPE_INT, NULL, TRUE, 6 }, 
	{ TYPE_INT, NULL, TRUE, 7 }, 
	{ TYPE_INT, NULL, FALSE, 8 }, 
	{ TYPE_INT, NULL, FALSE, 9 }, 
	};

#define PBLOCK_LENGTH	10

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,9,0),	
	};
#define NUM_OLDVERSIONS	1

#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);

//--- TypeInDlgProc --------------------------------

class ChBoxTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		ChBoxObject *ob;

		ChBoxTypeInDlgProc(ChBoxObject *o) {ob=o;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL ChBoxTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CC_CREATE: {					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ob->TestAFlag(A_OBJ_CREATING)) {
						ob->pblock->SetValue(PB_LENGTH,0,ob->crtLength);
						ob->pblock->SetValue(PB_WIDTH,0,ob->crtWidth);
						ob->pblock->SetValue(PB_HEIGHT,0,ob->crtHeight);
						ob->pblock->SetValue(PB_RADIUS,0,ob->crtRadius);
						}

					Matrix3 tm(1);
					tm.SetTrans(ob->crtPos);					
					ob->suspendSnap = FALSE;
					ob->ip->NonMouseCreate(tm);					
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}


//--- Box methods -------------------------------

// Constructor
ChBoxObject::ChBoxObject()
	{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer1,PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_LSEGS,0,dlgLSegs);
	pblock->SetValue(PB_WSEGS,0,dlgWSegs);
	pblock->SetValue(PB_HSEGS,0,dlgHSegs);	
	pblock->SetValue(PB_CSEGS,0,dlgCSegs);	
	pblock->SetValue(PB_LENGTH,0,crtLength);
	pblock->SetValue(PB_WIDTH,0,crtWidth);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	pblock->SetValue(PB_RADIUS,0,crtRadius);
	pblock->SetValue(PB_SMOOTH,0,1);

	pblock->SetValue(PB_GENUVS,0,TRUE);

	}
// Called by MAX when the sphere object is loaded from disk.
IOResult ChBoxObject::Load(ILoad *iload) 
	{	
	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the MAX file 
	// being loaded.
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}

// This method is called by the system when the user needs 
// to edit the objects parameters in the command panel.  
void ChBoxObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	// We subclass off SimpleObject so we must call its
	// BeginEditParams() method first.
	SimpleObject::BeginEditParams(ip,flags,prev);
	// Save the interface pointer.
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last box ceated
		pmapCreate->SetParamBlock(this);
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			// Here we create each new rollup page in the command panel
			// using our descriptors.
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_CHAMFERCUBE1),
				GetString(IDS_RB_CREATE_DIALOG),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_CHAMFERCUBE2),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);			
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_CHAMFERCUBE3),
			GetString(IDS_AP_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		// This handles processing the Create button in the 
		// Keyboard Entry rollup page.
		pmapTypeIn->SetUserDlgProc(new ChBoxTypeInDlgProc(this));
		}
	}
		
// This is called by the system to terminate the editing of the
// parameters in the command panel.  
void ChBoxObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{		
	SimpleObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {
		// Remove the rollup pages from the command panel.
		if (pmapCreate) DestroyCPParamMap(pmapCreate);
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
		pmapCreate = NULL;
		}

	// Save these values in class variables so the next object 
	// created will inherit them.
	pblock->GetValue(PB_LSEGS,ip->GetTime(),dlgLSegs,FOREVER);
	pblock->GetValue(PB_WSEGS,ip->GetTime(),dlgWSegs,FOREVER);
	pblock->GetValue(PB_HSEGS,ip->GetTime(),dlgHSegs,FOREVER);	
	pblock->GetValue(PB_CSEGS,ip->GetTime(),dlgCSegs,FOREVER);	
	}

#define POSX 0	// right
#define POSY 1	// back
#define POSZ 2	// top
#define NEGX 3	// left
#define NEGY 4	// front
#define NEGZ 5	// bottom

int direction(Point3 *v) {
	Point3 a = v[0]-v[2];
	Point3 b = v[1]-v[0];
	Point3 n = CrossProd(a,b);
	switch(MaxComponent(n)) {
		case 0: return (n.x<0)?NEGX:POSX;
		case 1: return (n.y<0)?NEGY:POSY;
		case 2: return (n.z<0)?NEGZ:POSZ;
		}
	return 0;
	}

// Remap the sub-object material numbers so that the top face is the first one
// The order now is:
// Top / Bottom /  Left/ Right / Front / Back
static int mapDir[6] ={ 3, 5, 0, 2, 4, 1 };
static int hsegs,wsegs,csegs,fcount,scount,curvertex;
static float wincr,hincr,cincr;
static Point3 NewPt,Toppt,CornerPt;
static int boxpos,wsegcount,hseg;

void CalculateHeightDivisions(int plus,int *PtRotation)
{float deltay;

  curvertex++;
  if (hseg==hsegs)
    { (*PtRotation)++;
      hseg=0;
      NewPt.y=(plus?Toppt.y:-CornerPt.y);}
  else
   { deltay=hincr*hseg;
     NewPt.y=(plus?-CornerPt.y+deltay:Toppt.y-deltay);
     hseg++;
   }
}
void CalculateWidthDivisions(int plus,int *PtRotation)
{float deltax;

  curvertex++;
   if (wsegcount==wsegs)
       {NewPt.x=(plus?CornerPt.x:-CornerPt.x);
       wsegcount=0;(*PtRotation)++;}
   else
    { deltax=wincr*wsegcount;
      NewPt.x=(plus?-CornerPt.x+deltax:CornerPt.x-deltax);
      wsegcount++;
    }
}

void FillinSquare(int *PtRotation,int *cornervert,float CurRadius)
{ if (hseg>0) CalculateHeightDivisions(((*PtRotation)>1),PtRotation);
  else if (wsegcount>0) CalculateWidthDivisions((*PtRotation)<3,PtRotation);
  else
   { switch (*PtRotation){
       case 0: NewPt.x=-CornerPt.x-CurRadius;
               NewPt.y=CornerPt.y;
               hseg++;
               break;
       case 1: NewPt.x=-CornerPt.x;
               NewPt.y=-CornerPt.y-CurRadius;
               wsegcount++;
               break;
       case 2: NewPt.x=CornerPt.x+CurRadius;
               NewPt.y=-CornerPt.y;
               hseg++;
               break;
       case 3: NewPt.x=CornerPt.x;
               NewPt.y=CornerPt.y+CurRadius;
               wsegcount++;
               break;
       default:;
      }
      curvertex+=csegs;
      if ((*PtRotation)==2) cornervert[*PtRotation]=curvertex-csegs;
      else cornervert[*PtRotation]=curvertex;
    }
}

void CalculateNewPt(float dx,float dy,int *PtRotation,int *cornervert,int deltapt)
{
    (*PtRotation)++;
    switch (*PtRotation){
      case 1: NewPt.y=-CornerPt.y-dy;
              NewPt.x=-CornerPt.x-dx;
              curvertex=cornervert[1]-deltapt;
              break;
      case 2: NewPt.x=CornerPt.x+dx;
              NewPt.y=-CornerPt.y-dy;
              curvertex=cornervert[2]+deltapt;
              break;
      case 3: NewPt.y=CornerPt.y+dy;
              curvertex=cornervert[3]-deltapt;
              *PtRotation=6;
              break;
   }
}

static int sidenum,endpt,face,topchamferfaces,chamferstart;
static int SidesPerSlice,topnum,tstartpt,maxfaces;
static int circleseg,firstface,cstartpt;
static chinfo chamferinfo[4];

//int ChBoxObject::getnextfirstvertex()
int getnextfirstvertex()
{ int c;

  if (boxpos==bottomedge)
  { c=curvertex -=chamferinfo[sidenum].deltavert;
    if (curvertex==endpt)
    { circleseg=1;firstface=0;
      if (endpt!=cstartpt) sidenum++;
       endpt-=chamferinfo[sidenum].deltavert*(chamferinfo[sidenum].surface==1?hsegs:wsegs);
     }
  }
  else
  { c=++curvertex;
    if (boxpos==messyedge)
     { c=(face==topchamferfaces+chamferstart-2?cstartpt:curvertex);
       if ((circleseg>0)&&(circleseg++>csegs)) circleseg=0;
     }
    else  if (boxpos==middle)
     c=(++fcount==SidesPerSlice?fcount=0,curvertex-SidesPerSlice:curvertex);

  }
  return(c);
}

int getnextsecondvertex()
{ int c;

  if (boxpos==messyedge)
  { c=topnum +=chamferinfo[sidenum].deltavert;
    if (topnum==endpt)
    { circleseg=1;firstface=1;
      if (endpt!=tstartpt) sidenum++;
      else {topnum=cstartpt;boxpos=middle;circleseg=0;firstface=0;}
       endpt+=chamferinfo[sidenum].deltavert*(chamferinfo[sidenum].surface==1?hsegs:wsegs);
     }
  }
  else
  { c=++topnum;
   if (boxpos==bottomedge)
    { c=(face==maxfaces-chamferstart-1?tstartpt:topnum);
      if ((circleseg>0)&&(circleseg++>csegs)) circleseg=0;
    }
    else if (boxpos==middle)
       c=(++scount==SidesPerSlice?scount=0,topnum-SidesPerSlice:topnum);
    else
       {if (++scount==wsegs) {scount=0;c=topnum++;}
       }
  }
  return(c);
}

void AddFace(Face *f,int smooth_group,int tdelta,TVFace *tvface,int genUVs)
{ int a,b,c;
  f[0].setSmGroup(smooth_group);
  f[0].setMatID((MtlID)0); 	 /*default */
  a=topnum; 
  if (firstface)
  { b=curvertex;
	f[0].setVerts(a, b, c=getnextfirstvertex());
    f[0].setEdgeVisFlags(1,1,0);
  }
  else
  { if (((boxpos==topsquare)||(boxpos==bottomsquare))&&(++fcount==wsegs))
    {fcount=0;b=curvertex++;}
    else if (boxpos==messyedge?face==topchamferfaces+chamferstart-1:scount==SidesPerSlice-1)
       b=(boxpos==middle?curvertex-SidesPerSlice:cstartpt);
	else b=curvertex;
	f[0].setVerts(a, b, c=getnextsecondvertex());
    f[0].setEdgeVisFlags(0,1,1);
  }
  if (genUVs)
  { a+=tdelta;b+=tdelta;c+=tdelta;
	tvface[0].setTVerts(a,b,c);
  }
  face++;
}

BOOL ChBoxObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void ChBoxObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

void ChBoxObject::BuildMesh(TimeValue t)
	{
	int smooth,dsegs,vertices;
	int WLines,HLines,DLines,CLines,VertexPerSlice;
	int VertexPerFace,FacesPerSlice,chamferend;
	float usedw,usedd,usedh,cradius,zdelta,CurRadius;
	Point3 va,vb,p;
	float depth, width, height;
	int genUVs = 1,sqvertex,CircleLayers;
	BOOL bias = 0,minusd;

	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;	
	pblock->GetValue(PB_LENGTH,t,height,ivalid);
	pblock->GetValue(PB_WIDTH,t,width,ivalid);
	pblock->GetValue(PB_HEIGHT,t,depth,ivalid);
	minusd=depth<0.0f;
	depth=(float)fabs(depth);
	pblock->GetValue(PB_RADIUS,t,cradius,ivalid);
	pblock->GetValue(PB_LSEGS,t,hsegs,ivalid);
	pblock->GetValue(PB_WSEGS,t,wsegs,ivalid);
	pblock->GetValue(PB_HSEGS,t,dsegs,ivalid);
	pblock->GetValue(PB_CSEGS,t,csegs,ivalid);
	pblock->GetValue(PB_GENUVS,t,genUVs,ivalid);
	pblock->GetValue(PB_SMOOTH,t,smooth,ivalid);
	
	LimitValue(csegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(dsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(wsegs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(hsegs, MIN_SEGMENTS, MAX_SEGMENTS);

	smooth=(smooth>0?1:0);
	mesh.setSmoothFlags(smooth);
	float twocrad,usedm,mindim=(height>width?(width>depth?depth:width):(height>depth?depth:height));
	usedm=mindim-2*cradius;
	if (usedm<0.01f) cradius=(mindim-0.01f)/2.0f;
	twocrad=2.0f*cradius;
    usedh=height-twocrad;
    usedw=width-twocrad;
    usedd=depth-twocrad;
	float cangincr=PI/(2.0f*csegs),cudelta,udist;
    CircleLayers=csegs;
	cudelta=cradius*(float)sqrt(2.0f*(1.0f-(float)cos(cangincr)));
	udist=4.0f*csegs*cudelta+2.0f*width+2.0f*height-4.0f*cradius;
	chamferinfo[0].surface=1;chamferinfo[0].deltavert=1;
	chamferinfo[1].surface=2;chamferinfo[1].deltavert=1;
	chamferinfo[2].surface=1;chamferinfo[2].deltavert=-1;
	chamferinfo[3].surface=2;chamferinfo[3].deltavert=-1;
    WLines=wsegs-1;
    HLines=hsegs-1;
    DLines=dsegs-1;
    CLines=csegs+1;
    VertexPerSlice=2*(WLines+HLines)+4*CLines;
/* WLines*HLines on middle, 2*Clines*(WLines+HLines) on sides, 4*CLines*csegs+4 for circles */
    VertexPerFace=WLines*HLines+2*CLines*(WLines+HLines+2*csegs)+4;
    vertices=VertexPerFace*2+VertexPerSlice*DLines;
    sqvertex=(wsegs+1)*(hsegs+1);
/* 4 vertices, 2 faces/cseg + 2 each hseg & wseg sides, each seg w/ 2 faces*/
    SidesPerSlice=2*(2*csegs+hsegs+wsegs);
    FacesPerSlice=SidesPerSlice*2;
/* this one only has 1 face/ cseg */
    topchamferfaces=4*(csegs+hsegs+wsegs);
/*top chamfer + top face(2 faces/seg)(*2 for bottom) plus any depth faces*/
    maxfaces=2*(topchamferfaces+2*hsegs*wsegs)+(2*(CircleLayers-1)+dsegs)*FacesPerSlice;
    chamferstart=2*hsegs*wsegs;
    chamferend=chamferstart+topchamferfaces+(CircleLayers-1)*FacesPerSlice;
    chamferinfo[0].deltavert +=wsegs;
    chamferinfo[2].deltavert -=wsegs;
	int bottomvertex,vertexnum,tverts;
	int twomapped,endvert=vertices+(twomapped=2*VertexPerSlice);
	float xmax,ymax;
	mesh.setNumVerts(vertices);
	mesh.setNumFaces(maxfaces);
	tverts=endvert+DLines+2;
	if (genUVs)
	{ mesh.setNumTVerts(tverts);
	  mesh.setNumTVFaces(maxfaces);
	}
	else
	{ mesh.setNumTVerts(0);
	  mesh.setNumTVFaces(0);
	}
    zdelta=depth/2;
    wsegcount=0;vertexnum=0;
    bottomvertex=vertices-1;
    CornerPt.z=zdelta;
    CornerPt.x=(xmax=width/2)-cradius;
    CornerPt.y=(ymax=height/2)-cradius;
    NewPt.x=Toppt.x=-CornerPt.x;
    NewPt.y=Toppt.y=CornerPt.y;
    NewPt.z=Toppt.z=zdelta;
      /* Do top and bottom faces */
	hincr=usedh/hsegs;		//yincr
	wincr=usedw/wsegs;		//xincr
	int segcount,topvertex,tvcounter=0,tvbottom=endvert-1;
	float udiv=2.0f*xmax,vdiv=2.0f*ymax,u,v;
	for (hseg=0;hseg<=hsegs;hseg++)
	{ if (hseg>0) 
	  {NewPt.y=(hseg==hsegs?-CornerPt.y:Toppt.y-hseg*hincr); NewPt.x=Toppt.x; }
	  for (segcount=0;segcount<=wsegs;segcount++)
	  { /* make top point */
	   NewPt.z=Toppt.z;
       NewPt.x=(segcount==wsegs?CornerPt.x:Toppt.x+segcount*wincr);
	   if (genUVs) 
		 mesh.setTVert(vertexnum,u=(xmax+NewPt.x)/udiv,v=(ymax+NewPt.y)/vdiv,0.0f);
	   mesh.setVert(vertexnum++,NewPt);
		/* make bottom pt */
       NewPt.z=-zdelta;
	   if (genUVs) 
	     mesh.setTVert(tvbottom--,u,1.0f-v,0.0f);
	   mesh.setVert(bottomvertex--,NewPt);
	  }
	}
    /* start on the chamfer */
	int layer,vert;
    layer=0;
    hseg=0;
	tvcounter=vertexnum;
    bottomvertex-=(VertexPerSlice-1);
    topvertex=vertexnum;
	BOOL done,atedge;
	float dincr=usedd/dsegs,cincr=2.0f*CircleLayers,RotationAngle;
	float dx,dy;
	int cornervert[4],PtRotation;
     for (layer=1;layer<=CircleLayers;layer++)	   /* add chamfer layer */
	 { if (layer==CircleLayers)	{zdelta=cradius;CurRadius=cradius;}
	   else
	   { RotationAngle=(PI*layer)/cincr;
	 	 zdelta=cradius-(cradius*(float)cos(RotationAngle));
		 CurRadius=cradius*(float)sin(RotationAngle);
	   }
	   zdelta=CornerPt.z-zdelta;
       atedge=(layer==CircleLayers);
	   int vfromedge=0,oldside=0,vfromstart=0;
	   sidenum=0;
	   float u1,v1;
	   BOOL atstart=TRUE;
       while (vertexnum<topvertex+csegs)	/* add vertex loop */
	   { PtRotation=hseg=wsegcount=0;done=FALSE;
         RotationAngle=(vertexnum-topvertex)*cangincr;
         curvertex=vert=vertexnum;
		 NewPt.x=Toppt.x-(dx=CurRadius*(float)sin(RotationAngle));
         NewPt.y=Toppt.y+(dy=CurRadius*(float)cos(RotationAngle));
         NewPt.z=zdelta;
		 while (!done)
		 { mesh.setVert(vert,NewPt);
		   if (genUVs) 
		    mesh.setTVert(vert,u1=(xmax+NewPt.x)/udiv,v1=(ymax+NewPt.y)/vdiv,0.0f);
           /* reflected vertex to second face */
           vert=bottomvertex+curvertex-topvertex;
           NewPt.z=-zdelta;
		   mesh.setVert(vert,NewPt);
		   if (genUVs)
		     mesh.setTVert(vert+twomapped,u1,1.0f-v1,0.0f);
           if ((atedge)&&(DLines>0))	 /* add non-corner points */
		    for (segcount=1;segcount<=DLines;segcount++)
		    { NewPt.z=zdelta-segcount*dincr;
		      mesh.setVert(vert=curvertex+VertexPerSlice*segcount,NewPt);
		    }
		   /* Rotate Pt */
		   if (!(done=PtRotation>5))
		   { if (vertexnum==topvertex) 
		     { FillinSquare(&PtRotation,cornervert,CurRadius);
		       if (curvertex==topvertex+VertexPerSlice-1) (PtRotation)=6;
		     }
			 else
				CalculateNewPt(dx,dy,&PtRotation,cornervert,vertexnum-topvertex);
		     vert=curvertex;
			 NewPt.z=zdelta;
		   }
		 }
	     vertexnum++;	   /* done rotation */
	   }
       vertexnum=topvertex +=VertexPerSlice;
       bottomvertex -=VertexPerSlice;  
	}
	float dfromedge=0.0f;
	int tvnum,j,i,chsegs=csegs+1,cwsegs=wsegs;
	if (genUVs)
	{ u=0.0f;
	  dfromedge=-cudelta;
	  tvnum=vertexnum;
	  vertexnum=topvertex-VertexPerSlice;
	  for (j=0;j<2;j++)
	  {
	  for (int gverts=0;gverts<chsegs;gverts++)
	  {	dfromedge+=cudelta;
		mesh.setTVert(tvnum,u=dfromedge/udist,1.0f,0.0f);
	    for (i=1;i<=dsegs;i++)
	     mesh.setTVert(tvnum+VertexPerSlice*i,u,1.0f-(float)i/dsegs,0.0f);
		vertexnum++;
		tvnum++;
	  }
	  chsegs=csegs;
	  for (gverts=0;gverts<hsegs;gverts++)
	  { dfromedge+=(float)fabs(mesh.verts[vertexnum].y-mesh.verts[vertexnum-1].y);
		mesh.setTVert(tvnum,u=dfromedge/udist,1.0f,0.0f);
	    for (i=1;i<=dsegs;i++)
	     mesh.setTVert(tvnum+VertexPerSlice*i,u,1.0f-(float)i/dsegs,0.0f);
		vertexnum++;
		tvnum++;
	  }
	  for (gverts=0;gverts<csegs;gverts++)
	  {	dfromedge+=cudelta;
		mesh.setTVert(tvnum,u=dfromedge/udist,1.0f,0.0f);
	    for (i=1;i<=dsegs;i++)
	     mesh.setTVert(tvnum+VertexPerSlice*i,u,1.0f-(float)i/dsegs,0.0f);
		vertexnum++;
		tvnum++;
	  }
	  if (j==1) cwsegs--;
	  for (gverts=0;gverts<cwsegs;gverts++)
	  { dfromedge+=(float)fabs(mesh.verts[vertexnum].x-mesh.verts[vertexnum-1].x);
		mesh.setTVert(tvnum,u=dfromedge/udist,1.0f,0.0f);
	    for (i=1;i<=dsegs;i++)
	     mesh.setTVert(tvnum+VertexPerSlice*i,u,1.0f-(float)i/dsegs,0.0f);
		vertexnum++;
		tvnum++;
	  }
	  }
	  int lastvert=endvert;
	  mesh.setTVert(lastvert++,1.0f,1.0f,0.0f);
	  for (j=1;j<dsegs;j++)
	    mesh.setTVert(lastvert++,1.0f,1.0f-(float)j/dsegs,0.0f);
	  mesh.setTVert(lastvert,1.0f,0.0f,0.0f);
	}
    /* all vertices calculated - Now specify faces*/
	 int tvdelta=0;
    sidenum=topnum=face=fcount=scount=circleseg=0;
    curvertex=wsegs+1;
    firstface=layer=1;
//	smooth=(csegs>1?1:0);
    tstartpt=cstartpt=endpt=0;
    boxpos=topsquare;cstartpt=chamferinfo[0].deltavert;
	AddFace(&mesh.faces[face],smooth,tvdelta,&mesh.tvFace[face],genUVs);
      while (face<chamferstart)   /* Do Square Layer */
	  { firstface=!firstface;
		AddFace(&mesh.faces[face],smooth,tvdelta,&mesh.tvFace[face],genUVs);
      }  
      boxpos=messyedge;firstface=1;
      topnum=tstartpt=0;
      cstartpt=curvertex=topnum+sqvertex;circleseg=1;
      endpt=hsegs*(wsegs+1);
      /* Do Chamfer */
	  while (face<chamferend)
      { AddFace(&mesh.faces[face],smooth,tvdelta,&mesh.tvFace[face],genUVs);
		if (circleseg==0) firstface=!firstface;
	  }
      fcount=scount=0;
      boxpos=middle;tvdelta+=VertexPerSlice;
     /*Do box sides */
	  int tpt,lastv=tverts-1;
	  BOOL inside=TRUE;
      while (face<maxfaces-chamferstart-topchamferfaces)
	  { tpt=face;
		AddFace(&mesh.faces[face],smooth,tvdelta,&mesh.tvFace[face],genUVs);
		if (genUVs && inside)
		{ if ((firstface)&&(mesh.tvFace[tpt].t[2]<mesh.tvFace[tpt].t[1]))
			mesh.tvFace[tpt].t[2]=endvert+1;
		  else if (mesh.tvFace[tpt].t[2]<mesh.tvFace[tpt].t[0])
		  { mesh.tvFace[tpt].t[1]=endvert+1;
			mesh.tvFace[tpt].t[2]=endvert;
			endvert++;
			if (!(inside=endvert<lastv)) 
			  tvdelta+=VertexPerSlice;
		  }
		}	
		firstface=!firstface;
	  }
      /* back in chamfer */
      circleseg=2;firstface=0;
      boxpos=bottomedge;
      sidenum=0;tstartpt=topnum;
      cstartpt=curvertex=vertices-1;
      endpt=cstartpt-hsegs*chamferinfo[0].deltavert;
	  while (face<maxfaces-chamferstart)  /* Do Second Chamfer */
	  { AddFace(&mesh.faces[face],smooth,tvdelta,&mesh.tvFace[face],genUVs);
		if (circleseg==0) firstface=!firstface;
	  }
      boxpos=bottomsquare;
      curvertex=topnum;
      topnum=curvertex+chamferinfo[0].deltavert;
      firstface=1;fcount=0;
	  while (face<maxfaces)
	  { AddFace(&mesh.faces[face],smooth,tvdelta,&mesh.tvFace[face],genUVs);
        firstface=!firstface;
	  }
    float deltaz=(minusd?-depth/2.0f:depth/2.0f);
	for (i=0;i<vertices;i++)
	{ mesh.verts[i].z+=deltaz;
	}
	mesh.InvalidateTopologyCache();
	}

Object* ChBoxObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
		return SimpleObject::ConvertToType(t,obtype);
	}

int ChBoxObject::CanConvertToType(Class_ID obtype)
	{
	if (obtype==triObjectClassID) 
	{
		return 1;
	} else {
		return SimpleObject::CanConvertToType(obtype);
		}
	}


class ChBoxObjCreateCallBack: public CreateMouseCallBack {
	ChBoxObject *ob;
	Point3 p0,p1;
	IPoint2 sp0, sp1,sp2;
	BOOL square;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(ChBoxObject *obj) { ob = obj; }
	};

int ChBoxObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) 
{	Point3 d;
#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m,m,NULL, snapdim);
	}
#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) 
	{switch(point) 
		{ case 0:
				sp0 = m;
				ob->pblock->SetValue(PB_WIDTH,0,0.0f);
				ob->pblock->SetValue(PB_LENGTH,0,0.0f);
				ob->pblock->SetValue(PB_HEIGHT,0,0.0f);
				ob->suspendSnap = TRUE;								
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				p1 = p0 + Point3(.01,.01,.01);
				mat.SetTrans(float(.5)*(p0+p1));				
#ifdef BOTTOMPIV
				{
				Point3 xyz = mat.GetTrans();
				xyz.z = p0.z;
				mat.SetTrans(xyz);
				}
#endif
				break;
			case 1:
				sp1 = m;
				p1 = vpt->SnapPoint(m,m,NULL,snapdim);
				p1.z = p0.z +(float).01; 
				if (ob->createMeth || (flags&MOUSE_CTRL)) 
				{  mat.SetTrans(p0);
				} else 
				{	mat.SetTrans(float(.5)*(p0+p1));
#ifdef BOTTOMPIV 					
					Point3 xyz = mat.GetTrans();
					xyz.z = p0.z;
					mat.SetTrans(xyz);					
#endif
				}
				d = p1-p0;
				square = FALSE;
				if (ob->createMeth) 
				{
					// Constrain to cube
					d.x = d.y = d.z = Length(d)*2.0f;
				} else 
				if (flags&MOUSE_CTRL) 
				{
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
					square = TRUE;
				}
				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_HEIGHT,0,d.z);
				ob->pmapParam->Invalidate();										
				if (msg==MOUSE_POINT && ob->createMeth) 
				{ if (Length(sp1-sp0)<3) CREATE_ABORT;					
				} else if (msg==MOUSE_POINT && 
						(Length(sp1-sp0)<3 || Length(d)<0.1f)) 
				{  return CREATE_ABORT;
				}
				break;
			case 2:
			sp2=m;
			if (!ob->createMeth)
				{
#ifdef _OSNAP
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p0,Point3(0,0,1),sp1,m,TRUE));
#else
				p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));
#endif
				d = p1-p0;
				if (square) 
				{ // Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;					
				}
				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_HEIGHT,0,d.z);
				ob->pmapParam->Invalidate();
			}
			else
			{   d.x =vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,1,0),sp1,m));
			    if (d.x<0.0f) d.x=0.0f;
				ob->pblock->SetValue(PB_RADIUS,0,d.x);
				ob->pmapParam->Invalidate();				
				if (msg==MOUSE_POINT && ob->createMeth) 
				{ ob->suspendSnap = FALSE;
					return CREATE_STOP;					
				}
			}
				break;
			case 3:
				d.x =vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,1,0),sp2,m));
				if (d.x<0.0f) d.x=0.0f;
				ob->pblock->SetValue(PB_RADIUS,0,d.x);
				ob->pmapParam->Invalidate();				
				if (msg==MOUSE_POINT) 
				{  ob->suspendSnap = FALSE;					
				   return CREATE_STOP;
				}
				break;
			}
	}	
	else
	if (msg == MOUSE_ABORT) 
	 {return CREATE_ABORT;}

	return TRUE;
}

static ChBoxObjCreateCallBack chboxCreateCB;

CreateMouseCallBack* ChBoxObject::GetCreateMouseCallBack() {
	chboxCreateCB.SetObj(this);
	return(&chboxCreateCB);
	}


BOOL ChBoxObject::OKtoDisplay(TimeValue t) 
	{
	/*
	float l, w, h;
	pblock->GetValue(PB_LENGTH,t,l,FOREVER);
	pblock->GetValue(PB_WIDTH,t,w,FOREVER);
	pblock->GetValue(PB_HEIGHT,t,h,FOREVER);
	if (l==0.0f || w==0.0f || h==0.0f) return FALSE;
	else return TRUE;
	*/
	return TRUE;
	}


// From ParamArray
BOOL ChBoxObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: createMeth = v; break;
		}		
	return TRUE;
	}

BOOL ChBoxObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_LENGTH: crtLength = v; break;
		case PB_TI_WIDTH:  crtWidth = v; break;
		case PB_TI_HEIGHT: crtHeight = v; break;
		case PB_TI_RADIUS: crtRadius = v; break;
		}	
	return TRUE;
	}

BOOL ChBoxObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL ChBoxObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = createMeth; break;
		}
	return TRUE;
	}

BOOL ChBoxObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {				
		case PB_TI_LENGTH: v = crtLength; break;
		case PB_TI_WIDTH:  v = crtWidth; break;
		case PB_TI_HEIGHT: v = crtHeight; break;
		case PB_TI_RADIUS: v = crtRadius; break;
		}
	return TRUE;
	}

BOOL ChBoxObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}


void ChBoxObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

ParamDimension *ChBoxObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH:return stdWorldDim;
		case PB_WIDTH: return stdWorldDim;
		case PB_HEIGHT:return stdWorldDim;
		case PB_RADIUS:return stdWorldDim;
		case PB_SMOOTH:return stdNormalizedDim;
		case PB_WSEGS: return stdSegmentsDim;
		case PB_LSEGS: return stdSegmentsDim;
		case PB_HSEGS: return stdSegmentsDim;		
		case PB_CSEGS: return stdSegmentsDim;
		default: return defaultDim;
		}
	}

TSTR ChBoxObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH: return TSTR(GetString(IDS_RB_LENGTH));
		case PB_WIDTH:  return TSTR(GetString(IDS_RB_WIDTH));
		case PB_HEIGHT: return TSTR(GetString(IDS_RB_HEIGHT));
		case PB_RADIUS: return TSTR(GetString(IDS_RB_FILLET));
		case PB_WSEGS:  return TSTR(GetString(IDS_RB_WSEGS));
		case PB_LSEGS:  return TSTR(GetString(IDS_RB_LSEGS));
		case PB_HSEGS:  return TSTR(GetString(IDS_RB_HSEGS));
		case PB_CSEGS:  return TSTR(GetString(IDS_RB_CSEGS));
		case PB_GENUVS:  return TSTR(GetString(IDS_MXS_GENUVS));
		case PB_SMOOTH:  return TSTR(GetString(IDS_MXS_SMOOTH));
		default: return TSTR(_T(""));
		}
	}

RefTargetHandle ChBoxObject::Clone(RemapDir& remap) 
	{
	ChBoxObject* newob = new ChBoxObject();
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}



// This is the method that actually handles the user input
// during the box creation.

// From Object

class ChBoxObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new ChBoxObject;}
	const TCHAR *	ClassName() { return GetString(IDS_AP_CHBOX_CLASS); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SCS_C_CLASS_ID1,SCS_C_CLASS_ID2); }
	const TCHAR* 	Category() { return GetString(IDS_RB_EXTENDED);}	
	void			ResetClassParams(BOOL fileReset);
	};

static ChBoxObjClassDesc chboxObjDesc;

ClassDesc* GetChBoxobjDesc() { return &chboxObjDesc; }
// class variable for sphere class.
IObjParam *ChBoxObject::ip         = NULL;
int ChBoxObject::dlgLSegs          = BDEF_SEGS;
int ChBoxObject::dlgWSegs          = BDEF_SEGS;
int ChBoxObject::dlgHSegs          = BDEF_SEGS;
int ChBoxObject::dlgCSegs          = CDEF_SEGS;
IParamMap *ChBoxObject::pmapCreate = NULL;
IParamMap *ChBoxObject::pmapTypeIn = NULL;
IParamMap *ChBoxObject::pmapParam  = NULL;	
Point3 ChBoxObject::crtPos         = Point3(0,0,0);		
float ChBoxObject::crtWidth        = 0.1f; 
float ChBoxObject::crtHeight       = 0.1f;
float ChBoxObject::crtLength       = 0.1f;
float ChBoxObject::crtRadius       = 0.01f;
int ChBoxObject::createMeth        = 0;

void ChBoxObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	ChBoxObject::dlgLSegs   = BDEF_SEGS;
	ChBoxObject::dlgWSegs   = BDEF_SEGS;
	ChBoxObject::dlgHSegs   = BDEF_SEGS;
	ChBoxObject::dlgCSegs   = BDEF_SEGS;
	ChBoxObject::crtWidth   = 0.0f; 
	ChBoxObject::crtHeight  = 0.0f;
	ChBoxObject::crtLength  = 0.0f;
	ChBoxObject::crtRadius  = 0.0f;
	ChBoxObject::createMeth = 0;
	ChBoxObject::crtPos         = Point3(0,0,0);
	}

