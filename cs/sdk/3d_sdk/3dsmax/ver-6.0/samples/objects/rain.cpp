/**********************************************************************
 *<
	FILE: RAIN.CPP

	DESCRIPTION: Rain and snow particle systems 

	CREATED BY: Rolf Berteig	

	HISTORY: created 29 October 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "prim.h"
#include "iparamm.h"
#include "Simpobj.h"
#include "texutil.h"

#ifndef NO_PARTICLES

void CacheData(ParticleSys *p0,ParticleSys *p1);

class GenParticle : public SimpleParticle {
	public:
		static IParamMap *pmapParam;		
		int stepSize;

		GenParticle(BOOL rain);
		void BirthParticle(INode *node,TimeValue bt,int index);
		void ComputeParticleStart(TimeValue t0,INode *node);
		int CountLive();
		Mesh *GetFacingRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
		int	GetParticleCount();				
		TimeValue ParticleLife(TimeValue t, int i);
		void RescaleWorldUnits(float f);

		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);		
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);		
		void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box);
			
		// Animatable methods		
		void DeleteThis() {delete this;}
		IOResult Load(ILoad *iload);
		void MapKeys(TimeMap *map,DWORD flags);
		int RenderBegin(TimeValue t, ULONG flags);		
		int RenderEnd(TimeValue t);
		
		// From SimpleParticle
		void UpdateParticles(TimeValue t,INode *node);
		void BuildEmitter(TimeValue t, Mesh& amesh);
		Interval GetValidity(TimeValue t);		
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		BOOL EmitterVisible();		
		Point3 ParticlePosition(TimeValue t,int i);
		Point3 ParticleVelocity(TimeValue t,int i);		

		virtual int DialogID()=0;
		virtual ParamUIDesc *ParamDesc()=0;
		virtual int DescCount()=0;
	};

class RainParticle;

class RainParticleDraw : public CustomParticleDisplay {
	public:
		TimeValue life;
		BOOL DrawParticle(GraphicsWindow *gw,ParticleSys &parts,int i);
	};

class RainParticle : public GenParticle {
	public:
		RainParticleDraw theRainDraw;

		RainParticle() : GenParticle(TRUE) {}

		// From BaseObject
		TCHAR *GetObjectName() {return GetString(IDS_RB_SPRAY);}
		
		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
		int IsInstanceDependent() {return 1;}

		// Animatable methods				
		Class_ID ClassID() {return Class_ID(RAIN_CLASS_ID,0);} 

		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		

		// From SimpleParticle
		MarkerType GetMarkerType();		
		float ParticleSize(TimeValue t,int i);
		int ParticleCenter(TimeValue t,int i);

		int DialogID() {return IDD_RAINPARAM;}
		ParamUIDesc *ParamDesc();
		int DescCount();
	};

class SnowParticle;

class SnowParticleDraw : public CustomParticleDisplay {
	public:
		float scale, tumble;
		TimeValue life;
		SnowParticle *obj;

		SnowParticleDraw() {obj=NULL;}
		BOOL DrawParticle(GraphicsWindow *gw,ParticleSys &parts,int i);
	};

class SnowParticle : public GenParticle {
	public:
		SnowParticleDraw theSnowDraw;

		SnowParticle();

		// From BaseObject
		TCHAR *GetObjectName() {return GetString(IDS_RB_SNOW);}
		
		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
		int IsInstanceDependent() {return 1;}

		// Animatable methods				
		Class_ID ClassID() {return Class_ID(SNOW_CLASS_ID,0);} 

		// From ref
		RefTargetHandle Clone(RemapDir& remap = NoRemap());		

		// From SimpleParticle
		MarkerType GetMarkerType();		
		void UpdateParticles(TimeValue t,INode *node);
		float ParticleSize(TimeValue t,int i);
		int ParticleCenter(TimeValue t,int i);

		int DialogID() {return IDD_SNOWPARAM;}
		ParamUIDesc *ParamDesc();
		int DescCount();

		Matrix3 TumbleMat(int index,float amount, float scale);
	};


//--- ClassDescriptor and class vars ---------------------------------

class RainClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new RainParticle;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_SPRAY_CLASS);}
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(RAIN_CLASS_ID,0);}
	const TCHAR* 	Category() {return GetString(IDS_RB_PARTICLESYSTEMS);}
	};

static RainClassDesc rainDesc;
ClassDesc* GetRainDesc() {return &rainDesc;}

class SnowClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new SnowParticle;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_SNOW_CLASS);}
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(SNOW_CLASS_ID,0);}
	const TCHAR* 	Category() {return GetString(IDS_RB_PARTICLESYSTEMS);}
	};

static SnowClassDesc snowDesc;
ClassDesc* GetSnowDesc() {return &snowDesc;}

IParamMap *GenParticle::pmapParam;


//--- Parameter map/block descriptors -------------------------------

#define RAINSIZEFACTOR (float(TIME_TICKSPERSEC)/120.0f)
#define PARTICLE_SEED	0x8d6a65bc


#define PB_VPTPARTICLES		0
#define PB_RNDPARTICLES		1
#define PB_DROPSIZE			2
#define PB_SPEED			3
#define PB_VARIATION		4
#define PB_DISPTYPE			5
#define PB_STARTTIME		6
#define PB_LIFETIME			7
#define PB_EMITTERWIDTH		8
#define PB_EMITTERHEIGHT	9
#define PB_HIDEEMITTER		10
#define PB_BIRTHRATE		11
#define PB_CONSTANT			12
#define PB_RENDER			13
#define PB_TUMBLE			14
#define PB_SCALE			15


// render types
#define RENDTYPE1	0
#define RENDTYPE2	1
#define RENDTYPE3	2

#define A_RENDER			A_PLUGIN1

class BirthRateDimension : public ParamDimension {
	public:
		DimType DimensionType() {return DIM_CUSTOM;}
		float Convert(float value) {return value*(float)GetTicksPerFrame();}
		float UnConvert(float value) {return value/(float)GetTicksPerFrame();}
	};
static BirthRateDimension theBirthRateDim;

#define MAX_PARTICLE_COUNT	500000

//
//
// Parameters

static int typeIDs[] = {IDC_PARTICLE_CUST,IDC_PARTICLE_DOTS,IDC_PARTICLE_TICKS};
static int rendIDs[] = {IDC_PARTICLE_REND1,IDC_PARTICLE_REND2,IDC_PARTICLE_REND3};

static ParamUIDesc descParamRain[] = {
	// Viewport particles
	ParamUIDesc(
		PB_VPTPARTICLES,
		EDITTYPE_INT,
		IDC_VPT_PARTICLES,IDC_VPT_PARTICLESSPIN,
		1.0f,(float)MAX_PARTICLE_COUNT,
		1.0f),

	// Render particles
	ParamUIDesc(
		PB_RNDPARTICLES,
		EDITTYPE_INT,
		IDC_RND_PARTICLES,IDC_RND_PARTICLESSPIN,
		1.0f,(float)MAX_PARTICLE_COUNT,
		1.0f),

	// Particle size
	ParamUIDesc(
		PB_DROPSIZE,
		EDITTYPE_UNIVERSE,
		IDC_PARTICLE_SIZE,IDC_PARTICLE_SIZESPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Particle speed
	ParamUIDesc(
		PB_SPEED,
		EDITTYPE_FLOAT,
		IDC_PARTICLE_SPEED,IDC_PARTICLE_SPEEDSPIN,
		-999999999.0f,999999999.0f,
		0.005f),

	// Particle Variation
	ParamUIDesc(
		PB_VARIATION,
		EDITTYPE_FLOAT,
		IDC_PARTICLE_VARIATION,IDC_PARTICLE_VARIATIONSPIN,
		0.0f,999999999.0f,
		0.005f),

	// Display type
	ParamUIDesc(PB_DISPTYPE,TYPE_RADIO,typeIDs,3),

	// Start time
	ParamUIDesc(
		PB_STARTTIME,
		EDITTYPE_TIME,
		IDC_PARTICLE_START,IDC_PARTICLE_STARTSPIN,
		-999999999.0f,999999999.0f,
		10.0f),

	// Life time
	ParamUIDesc(
		PB_LIFETIME,
		EDITTYPE_TIME,
		IDC_PARTICLE_LIFE,IDC_PARTICLE_LIFESPIN,
		0.0f,999999999.0f,
		10.0f),

	// Birth Rate
	ParamUIDesc(
		PB_BIRTHRATE,
		EDITTYPE_FLOAT,
		IDC_PARTICLE_BIRTHRATE,IDC_PARTICLE_BIRTHRATESPIN,
		0.0f,999999999.0f,
		0.1f,
		&theBirthRateDim),

	// Constant birth rate
	ParamUIDesc(PB_CONSTANT,TYPE_SINGLECHEKBOX,IDC_PARTICLE_CONSTANTRATE),

	// Emitter width
	ParamUIDesc(
		PB_EMITTERWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_EMITTER_WIDTH,IDC_EMITTER_WIDTHSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Emitter height
	ParamUIDesc(
		PB_EMITTERHEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_EMITTER_HEIGHT,IDC_EMITTER_HEIGHTSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Hide Emitter
	ParamUIDesc(PB_HIDEEMITTER,TYPE_SINGLECHEKBOX,IDC_HIDE_EMITTER),
	
	// Render
	ParamUIDesc(PB_RENDER,TYPE_RADIO,rendIDs,3),
	};
#define RAINPARAMDESC_LENGH 14

static ParamUIDesc descParamSnow[] = {
	// Viewport particles
	ParamUIDesc(
		PB_VPTPARTICLES,
		EDITTYPE_INT,
		IDC_VPT_PARTICLES,IDC_VPT_PARTICLESSPIN,
		1.0f,(float)MAX_PARTICLE_COUNT,
		1.0f),

	// Render particles
	ParamUIDesc(
		PB_RNDPARTICLES,
		EDITTYPE_INT,
		IDC_REND_PARTICLES,IDC_REND_PARTICLESSPIN,
		1.0f,(float)MAX_PARTICLE_COUNT,
		1.0f),

	// Particle size
	ParamUIDesc(
		PB_DROPSIZE,
		EDITTYPE_UNIVERSE,
		IDC_PARTICLE_SIZE,IDC_PARTICLE_SIZESPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Particle speed
	ParamUIDesc(
		PB_SPEED,
		EDITTYPE_FLOAT,
		IDC_PARTICLE_SPEED,IDC_PARTICLE_SPEEDSPIN,
		-999999999.0f,999999999.0f,
		0.005f),

	// Particle Variation
	ParamUIDesc(
		PB_VARIATION,
		EDITTYPE_FLOAT,
		IDC_PARTICLE_VARIATION,IDC_PARTICLE_VARIATIONSPIN,
		0.0f,999999999.0f,
		0.005f),

	// Particle Tumble
	ParamUIDesc(
		PB_TUMBLE,
		EDITTYPE_FLOAT,
		IDC_PARTICLE_TUMBLE,IDC_PARTICLE_TUMBLESPIN,
		0.0f,1.0f,
		0.005f),

	// Tumble scale
	ParamUIDesc(
		PB_SCALE,
		EDITTYPE_FLOAT,
		IDC_PARTICLE_SCALE,IDC_PARTICLE_SCALESPIN,
		0.0f,999999999.0f,
		0.01f),

	// Display type
	ParamUIDesc(PB_DISPTYPE,TYPE_RADIO,typeIDs,3),

	// Start time
	ParamUIDesc(
		PB_STARTTIME,
		EDITTYPE_TIME,
		IDC_PARTICLE_START,IDC_PARTICLE_STARTSPIN,
		-999999999.0f,999999999.0f,
		10.0f),

	// Life time
	ParamUIDesc(
		PB_LIFETIME,
		EDITTYPE_TIME,
		IDC_PARTICLE_LIFE,IDC_PARTICLE_LIFESPIN,
		0.0f,999999999.0f,
		10.0f),

	// Birth Rate
	ParamUIDesc(
		PB_BIRTHRATE,
		EDITTYPE_FLOAT,
		IDC_PARTICLE_BIRTHRATE,IDC_PARTICLE_BIRTHRATESPIN,
		0.0f,999999999.0f,
		0.1f,
		&theBirthRateDim),

	// Constant birth rate
	ParamUIDesc(PB_CONSTANT,TYPE_SINGLECHEKBOX,IDC_PARTICLE_CONSTANTRATE),

	// Emitter width
	ParamUIDesc(
		PB_EMITTERWIDTH,
		EDITTYPE_UNIVERSE,
		IDC_EMITTER_WIDTH,IDC_EMITTER_WIDTHSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Emitter height
	ParamUIDesc(
		PB_EMITTERHEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_EMITTER_HEIGHT,IDC_EMITTER_HEIGHTSPIN,
		0.0f,999999999.0f,
		SPIN_AUTOSCALE),	

	// Hide Emitter
	ParamUIDesc(PB_HIDEEMITTER,TYPE_SINGLECHEKBOX,IDC_HIDE_EMITTER),
	
	// Render
	ParamUIDesc(PB_RENDER,TYPE_RADIO,rendIDs,3),
	};
#define SNOWPARAMDESC_LENGH 16


static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_INT, NULL, FALSE, 9 }};

static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_FLOAT, NULL, TRUE, 8 },
	{ TYPE_INT, NULL, FALSE, 9 },
	{ TYPE_FLOAT, NULL, TRUE, 10 }, // birth rate
	{ TYPE_INT, NULL, FALSE, 11 }	// constant
	};

static ParamBlockDescID descVer2[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	// count
	{ TYPE_FLOAT, NULL, TRUE, 1 },	// size
	{ TYPE_FLOAT, NULL, TRUE, 2 },	// speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	// variation
	{ TYPE_FLOAT, NULL, TRUE, 12 },	// tumble
	{ TYPE_FLOAT, NULL, TRUE, 13 },	// tumble scale
	{ TYPE_INT, NULL, FALSE, 4 },	// viewport type	
	{ TYPE_INT, NULL, FALSE, 5 },	// start
	{ TYPE_INT, NULL, FALSE, 6 },	// life
	{ TYPE_FLOAT, NULL, TRUE, 7 },	// width
	{ TYPE_FLOAT, NULL, TRUE, 8 },	// height
	{ TYPE_INT, NULL, FALSE, 9 },	// hide emitter
	{ TYPE_FLOAT, NULL, TRUE, 10 }, // birth rate
	{ TYPE_INT, NULL, FALSE, 11 },	// constant
	{ TYPE_INT, NULL, FALSE, 14 },	// render type
	};

static ParamBlockDescID descVer3[] = {
	{ TYPE_INT, NULL, FALSE, 0 },	// vpt count
	{ TYPE_INT, NULL, FALSE, 15 },	// rnd count	
	{ TYPE_FLOAT, NULL, TRUE, 1 },	// size
	{ TYPE_FLOAT, NULL, TRUE, 2 },	// speed
	{ TYPE_FLOAT, NULL, TRUE, 3 },	// variation	
	{ TYPE_INT, NULL, FALSE, 4 },	// viewport type	
	{ TYPE_INT, NULL, FALSE, 5 },	// start
	{ TYPE_INT, NULL, FALSE, 6 },	// life
	{ TYPE_FLOAT, NULL, TRUE, 7 },	// width
	{ TYPE_FLOAT, NULL, TRUE, 8 },	// height
	{ TYPE_INT, NULL, FALSE, 9 },	// hide emitter
	{ TYPE_FLOAT, NULL, TRUE, 10 }, // birth rate
	{ TYPE_INT, NULL, FALSE, 11 },	// constant
	{ TYPE_INT, NULL, FALSE, 14 },	// render type
	{ TYPE_FLOAT, NULL, TRUE, 12 },	// tumble
	{ TYPE_FLOAT, NULL, TRUE, 13 },	// tumble scale
	};


// Note that tumble and scale are only used by snow
#define PBLOCK_LENGTH_RAIN 14
#define PBLOCK_LENGTH_SNOW 16


// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,10,0),
	ParamVersionDesc(descVer1,12,1),
	ParamVersionDesc(descVer2,15,2)	
	};
#define NUM_OLDVERSIONS	3

// Current version
#define CURRENT_VERSION	3
static ParamVersionDesc curVersionRain(descVer3,PBLOCK_LENGTH_RAIN,CURRENT_VERSION);
static ParamVersionDesc curVersionSnow(descVer3,PBLOCK_LENGTH_SNOW,CURRENT_VERSION);


//-- ParticleDlgProc ------------------------------------------------

class ParticleDlgProc : public ParamMapUserDlgProc {
	public:
		GenParticle *po;

		ParticleDlgProc(GenParticle *p) {po=p;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
		void SetupMaxRate(HWND hWnd,IParamBlock *pblock,TimeValue t);
		void SetupConstant(HWND hWnd);
		void EnableTumble(HWND hWnd, IParamBlock *pblock, TimeValue t); // for Snow
	};

void ParticleDlgProc::SetupMaxRate(HWND hWnd,IParamBlock *pblock,TimeValue t)
	{
	int count, life;
	pblock->GetValue(PB_RNDPARTICLES,t,count,FOREVER);
	pblock->GetValue(PB_LIFETIME,t,life,FOREVER);
	if (life <= 0) life = 1;
	TSTR buf;
	buf.printf(_T("%.1f"),(float)GetTicksPerFrame()*float(count)/float(life));
	SetWindowText(GetDlgItem(hWnd,IDC_PARTICLE_MAXRATE),buf);
	}

void ParticleDlgProc::SetupConstant(HWND hWnd)
	{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_PARTICLE_BIRTHRATESPIN));
	if (IsDlgButtonChecked(hWnd,IDC_PARTICLE_CONSTANTRATE)) {
		spin->Disable();
		EnableWindow(GetDlgItem(hWnd, IDC_PARTICLE_BIRTHRATE_TXT), FALSE);
	} else {
		spin->Enable();
		EnableWindow(GetDlgItem(hWnd, IDC_PARTICLE_BIRTHRATE_TXT), TRUE);
		}
	ReleaseISpinner(spin);
	}

void ParticleDlgProc::EnableTumble(HWND hWnd, IParamBlock *pblock, TimeValue t) // for Snow
{
	if (po->DialogID() != IDD_SNOWPARAM) return; // it's not snow

	int renderType;
	pblock->GetValue(PB_RENDER,t,renderType,FOREVER);
	BOOL tumble = (renderType != RENDTYPE3); // Snow: RENDTYPE3 = Facing

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd, IDC_PARTICLE_TUMBLE));
	spin->Enable(tumble);
	ReleaseISpinner(spin);
	spin = GetISpinner(GetDlgItem(hWnd, IDC_PARTICLE_SCALE));
	spin->Enable(tumble);
	ReleaseISpinner(spin);
	EnableWindow(GetDlgItem(hWnd, IDC_PARTICLE_TUMBLE_TXT), tumble);
	EnableWindow(GetDlgItem(hWnd, IDC_PARTICLE_SCALE_TXT), tumble);
}

#define MAX_EVAL_TIME_DELTA		TIME_TICKSPERSEC*1000

BOOL ParticleDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			SetupMaxRate(hWnd,(IParamBlock*)map->GetParamBlock(),t);
			SetupConstant(hWnd);
			EnableTumble(hWnd,(IParamBlock*)map->GetParamBlock(),t);
			break;			
			
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) {
				case IDC_PARTICLE_STARTSPIN: {
					ISpinnerControl *spin = (ISpinnerControl*)lParam;
					TimeValue st = spin->GetIVal();
					TimeValue ct = GetCOREInterface()->GetTime();
					if (st+MAX_EVAL_TIME_DELTA < ct) {
						st = ct - MAX_EVAL_TIME_DELTA;
						map->GetParamBlock()->SetValue(PB_STARTTIME,ct, st);
						}
					break;
					}

				case IDC_VPT_PARTICLESSPIN:
				case IDC_PARTICLE_LIFESPIN: 
					SetupMaxRate(hWnd,(IParamBlock*)map->GetParamBlock(),t);
					break;					
				}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_PARTICLE_CONSTANTRATE:
					SetupConstant(hWnd);
					break;					
				case IDC_PARTICLE_REND1:
				case IDC_PARTICLE_REND2:
				case IDC_PARTICLE_REND3:
					EnableTumble(hWnd,(IParamBlock*)map->GetParamBlock(),t);
					break;
				}
			break;	
		}
	return FALSE;
	}


static float CompParticleSize(
		TimeValue age, TimeValue life, float size)
	{
	float u = float(age)/float(life);
	if (u>0.75f) {
		return (1.0f-smoothstep(0.75f,1.0f,u)) * size;
	} else {
		return size;
		}
	}

//--- For multiple collision per integration step (frame) --------

class CollisionCollection: public CollisionObject {
public:
	static int MAX_COLLISIONS_PER_STEP;

	// CollisionObject methods
	BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct, BOOL UpdatePastCollide);
	Object *GetSWObject();

	// CollisionCollection methods
	void Init(const Tab<CollisionObject*> &cobjs);
	BOOL FindClosestCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct);

private:
	Tab<CollisionObject*> m_cobjs;
};

int CollisionCollection::MAX_COLLISIONS_PER_STEP = 10;

void CollisionCollection::Init(const Tab<CollisionObject*> &cobjs)
{
	m_cobjs.SetCount(cobjs.Count());
	for(int i=0; i<cobjs.Count(); i++) m_cobjs[i] = cobjs[i];
}

Object* CollisionCollection::GetSWObject()
{
	if (m_cobjs.Count()) return m_cobjs[0]->GetSWObject();
	 else return NULL;
}

BOOL CollisionCollection::CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct, BOOL UpdatePastCollide)
{
	TimeValue nextTime, curTime = t;
	BOOL collide=FALSE, maybeStuck=FALSE;
	
	if (UpdatePastCollide)
	{
		for(int i=0; i<MAX_COLLISIONS_PER_STEP; i++)
		{
			if (FindClosestCollision(curTime, pos, vel, dt, index, ct))
			{
				collide = TRUE;
				nextTime = curTime + (TimeValue)ceil(*ct);
				dt -= nextTime - curTime;
				if (dt <= 0.0f) break; // time limit for the current integration step
				curTime = nextTime; // for the next micro-step
			}
			else break;
			// particle may still have a collision in the current integration step;
			// since particle reaches the limit of collision check per integration step,
			// we'd better hold on the particle movements for the current frame
			if (i==MAX_COLLISIONS_PER_STEP-1) maybeStuck = TRUE;
		}
		if ((dt > 0.0f) && (!maybeStuck)) // final interval without collisions
			pos += vel*dt;
	}
	else
		collide = FindClosestCollision(t, pos, vel, dt, index, ct);		

	return collide;
}

BOOL CollisionCollection::FindClosestCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct)
{
	Point3 curPos, curVel, resPos, resVel;
	float curTime, minTime = dt+1.0f;
	BOOL collide = FALSE;

	for(int i=0; i<m_cobjs.Count(); i++)
	{
		curPos = pos; curVel = vel;
		if (m_cobjs[i]->CheckCollision(t, curPos, curVel, dt, index, &curTime, FALSE))
			if (curTime < minTime) // the collision is the closest one
			{
				collide = TRUE;
				minTime = curTime;
				resPos = curPos;
				resVel = curVel;
			}
	}
	if (collide)
	{
		pos = resPos;
		vel = resVel;
		*ct = minTime;
	}
	return collide;
}

//--- GenParticle Methods--------------------------------------------

static int Parity(Matrix3 tm)
	{
	Point3 v = tm.GetRow(0) ^ tm.GetRow(1);
	if (DotProd(v,tm.GetRow(2)) < 0.0f) return 1;
	return 0;
	}

static void FlipAllMeshFaces(Mesh *mesh)
	{
	for (int i=0; i<mesh->getNumFaces(); i++) 
		mesh->FlipNormal(i);
	}


GenParticle::GenParticle(BOOL rain)
	{
	int length = rain ? PBLOCK_LENGTH_RAIN: PBLOCK_LENGTH_SNOW;
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer3, length, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_VPTPARTICLES,0,100);
	pblock->SetValue(PB_RNDPARTICLES,0,100);
	pblock->SetValue(PB_DROPSIZE,0,2.0f);
	pblock->SetValue(PB_SPEED,0,10.0f);
	pblock->SetValue(PB_DISPTYPE,0,0);
	pblock->SetValue(PB_STARTTIME,0,0);
	pblock->SetValue(PB_LIFETIME,0,TIME_TICKSPERSEC);
	pblock->SetValue(PB_CONSTANT,0,1);
	if (!rain) pblock->SetValue(PB_SCALE,0,1.0f);
	stepSize = 0;
	}

// RB 3/3/99: Implemented this to also scale speed and variation
void GenParticle::RescaleWorldUnits(float f)
	{
	if (TestAFlag(A_WORK1))
		return;
	
	// Call the base class's rescale (this sets the A_WORK1 flag)
	SimpleParticle::RescaleWorldUnits(f);

	pblock->RescaleParam(PB_SPEED, f);
	pblock->RescaleParam(PB_VARIATION, f);	
	}

class ParticlePostLoadCallback : public PostLoadCallback {
	public:
		ParamBlockPLCB *cb;
		ParticlePostLoadCallback(ParamBlockPLCB *c) {cb=c;}
		void proc(ILoad *iload) {
			DWORD oldVer = ((GenParticle*)(cb->targ))->pblock->GetVersion();
			ReferenceTarget *targ = cb->targ;
			cb->proc(iload);
			if (oldVer==0) {				
				((GenParticle*)targ)->pblock->SetValue(PB_CONSTANT,0,1);
				}
			if (oldVer<3) {
				// Set render particles to vpt particles.
				int vc;				
				((GenParticle*)targ)->pblock->GetValue(PB_VPTPARTICLES,0,vc,FOREVER);
				((GenParticle*)targ)->pblock->SetValue(PB_RNDPARTICLES,0,vc);
				}
			delete this;
			}
	};

IOResult GenParticle::Load(ILoad *iload)
	{
	if (ClassID()==Class_ID(RAIN_CLASS_ID,0)) {
		iload->RegisterPostLoadCallback(
			new ParticlePostLoadCallback(
				new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersionRain,this,0)));
	} else {
		iload->RegisterPostLoadCallback(
			new ParticlePostLoadCallback(
				new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersionSnow,this,0)));
		}
	return IO_OK;
	}

TimeValue GenParticle::ParticleLife(TimeValue t, int i)
	{
	TimeValue life;	
	pblock->GetValue(PB_LIFETIME,t,life,FOREVER);
	return life;
	}

Point3 GenParticle::ParticlePosition(TimeValue t,int i)
	{
	return parts.points[i];
	}

Point3 GenParticle::ParticleVelocity(TimeValue t,int i)
	{
	return parts.vels[i];
	}


// RB 3-38-96
// Changed render begin and end to always invalidate the particles.
// This ensures that the particles will always update completely
// once before a rendering session and allows the integration
// step size to change to the size of a field instead of a frame.

int GenParticle::RenderBegin(TimeValue t, ULONG flags)
	{
	SetAFlag(A_RENDER);
	ParticleInvalid();		
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return 0;

#if 0
	int vc, rc;
	pblock->GetValue(PB_VPTPARTICLES,0,vc,FOREVER);
	pblock->GetValue(PB_RNDPARTICLES,0,rc,FOREVER);
	SetAFlag(A_RENDER);
	if (vc!=rc) {
		ParticleInvalid();		
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}
	return 0;
#endif
	}

int GenParticle::RenderEnd(TimeValue t)
	{
	ClearAFlag(A_RENDER);
	ParticleInvalid();		
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return 0;

#if 0
	int vc, rc;
	pblock->GetValue(PB_VPTPARTICLES,0,vc,FOREVER);
	pblock->GetValue(PB_RNDPARTICLES,0,rc,FOREVER);
	ClearAFlag(A_RENDER);
	if (vc!=rc) {
		ParticleInvalid();		
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		}
	return 0;
#endif
	}

int	GenParticle::GetParticleCount()
	{
	int c;
	if (TestAFlag(A_RENDER)) pblock->GetValue(PB_RNDPARTICLES,0,c,FOREVER);
	else pblock->GetValue(PB_VPTPARTICLES,0,c,FOREVER);
	if (c > MAX_PARTICLE_COUNT) c = MAX_PARTICLE_COUNT;
	return c;
	}

void GenParticle::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SimpleParticle::BeginEditParams(ip,flags,prev);
	if (pmapParam) {
		pmapParam->SetParamBlock(pblock);
	} else {		
		
		pmapParam = CreateCPParamMap(
			ParamDesc(),DescCount(),
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(DialogID()),
			GetString(IDS_RB_PARAMETERS),
			0);		
		}
	pmapParam->SetUserDlgProc(new ParticleDlgProc(this));
	}

void GenParticle::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{
	SimpleParticle::EndEditParams(ip,flags,next);

	if (flags&END_EDIT_REMOVEUI) {
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		}
	}

void GenParticle::MapKeys(TimeMap *map,DWORD flags)
	{
	Animatable::MapKeys(map,flags);

	TimeValue start, life;
	pblock->GetValue(PB_STARTTIME,0,start,FOREVER);
	pblock->GetValue(PB_LIFETIME,0,life,FOREVER);

	start = map->map(start);
	life  = map->map(life);

	pblock->SetValue(PB_STARTTIME,0,start);
	pblock->SetValue(PB_LIFETIME,0,life);
	}

Interval GenParticle::GetValidity(TimeValue t)
	{
	// For now...
	return Interval(t,t);
	}

void GenParticle::BuildEmitter(TimeValue t, Mesh& amesh)
	{
	float width, height;
	mvalid = FOREVER;
	pblock->GetValue(PB_EMITTERWIDTH,t,width,mvalid);
	pblock->GetValue(PB_EMITTERHEIGHT,t,height,mvalid);
	width  *= 0.5f;
	height *= 0.5f;

	mesh.setNumVerts(7);
	mesh.setNumFaces(6);
	mesh.setVert(0, Point3(-width,-height, 0.0f));
	mesh.setVert(1, Point3( width,-height, 0.0f));
	mesh.setVert(2, Point3( width, height, 0.0f));
	mesh.setVert(3, Point3(-width, height, 0.0f));
	mesh.setVert(4, Point3(  0.0f,   0.0f, 0.0f));
	mesh.setVert(5, Point3(  0.0f,   0.0f, -(width+height)/2.0f));
	mesh.setVert(6, Point3(  0.0f,   0.0f, 0.0f));

	mesh.faces[0].setEdgeVisFlags(1,0,1);
	mesh.faces[0].setSmGroup(1);
	mesh.faces[0].setVerts(0,1,3);

	mesh.faces[1].setEdgeVisFlags(1,1,0);
	mesh.faces[1].setSmGroup(1);
	mesh.faces[1].setVerts(1,2,3);

	mesh.faces[2].setEdgeVisFlags(1,1,0);
	mesh.faces[2].setSmGroup(1);
	mesh.faces[2].setVerts(4,5,6);

	mesh.faces[3].setEdgeVisFlags(1,0,1);
	mesh.faces[3].setSmGroup(1);
	mesh.faces[3].setVerts(0,3,1);

	mesh.faces[4].setEdgeVisFlags(0,1,1);
	mesh.faces[4].setSmGroup(1);
	mesh.faces[4].setVerts(1,3,2);

	mesh.faces[5].setEdgeVisFlags(1,0,0);
	mesh.faces[5].setSmGroup(1);
	mesh.faces[5].setVerts(5,4,6);

	mesh.InvalidateGeomCache();
	}

int GenParticle::CountLive()
	{
	int c=0;
	for (int i=0; i<parts.Count(); i++) {
		if (parts.Alive(i)) c++;
		}
	return c;
	}

void GenParticle::ComputeParticleStart(TimeValue t0,INode *node)
	{
	int c = GetParticleCount();	
	parts.SetCount(c,PARTICLE_VELS|PARTICLE_AGES);
	for (int i=0; i<parts.Count(); i++) {
		parts.ages[i] = -1;
		}
	tvalid = t0;
	valid  = TRUE;
	}


#define VEL_SCALE	(-0.01f*1200.0f/float(TIME_TICKSPERSEC))
#define VAR_SCALE	(0.01f*1200.0f/float(TIME_TICKSPERSEC))

void GenParticle::BirthParticle(INode *node,TimeValue bt,int index)
	{
	float initVel, var;
	float width, height;
	Point3 pos, vel;
	Matrix3 tm = node->GetObjTMBeforeWSM(bt);

	pblock->GetValue(PB_EMITTERWIDTH,bt,width,FOREVER);
	pblock->GetValue(PB_EMITTERHEIGHT,bt,height,FOREVER);
	pblock->GetValue(PB_SPEED,bt,initVel,FOREVER);
	pblock->GetValue(PB_VARIATION,bt,var,FOREVER);
	initVel *= VEL_SCALE; // UI Scaling

	vel = Point3(0.0f, 0.0f, initVel);
	if (var!=0.0f) {
		var *= VAR_SCALE;
		vel.x = -var + float(rand())/float(RAND_MAX) * 2.0f*var;
		vel.y = -var + float(rand())/float(RAND_MAX) * 2.0f*var;
		vel.z = initVel - var + float(rand())/float(RAND_MAX) * 2.0f*var;
		}

	parts.ages[index] = 0;
	parts.vels[index] = VectorTransform(tm,vel);
			
	pos.x = -width/2.0f + float(rand())/float(RAND_MAX) * width;
	pos.y = -height/2.0f + float(rand())/float(RAND_MAX) * height;
	pos.z = 0.0f;
	
	parts[index] = pos * tm;
	}



void GenParticle::UpdateParticles(TimeValue t,INode *node)
	{
	TimeValue t0, life, dt,oneframe;
	int i, j, constant = TRUE, total, birth=0;
	float brate, brateFactor = 1.0f;
	Point3 force;		
	// variable for new collision scheme (Bayboro 2/5/01)
	CollisionCollection cc;
	// initialization for new collision scheme (Bayboro 2/5/01)
	cc.Init(cobjs);
	
	// The size of steps we take to integrate will be 
	// frame size steps. Unless we're rendering in which
	// case we'll take field size steps to ensure that
	// particle are synchronized on multiple machines
	// rendering fields
	if (stepSize!=(oneframe=GetTicksPerFrame())) {
			stepSize = GetTicksPerFrame();
			valid = FALSE;
			}
	pblock->GetValue(PB_STARTTIME,t,t0,FOREVER);
	pblock->GetValue(PB_LIFETIME,t,life,FOREVER);
	pblock->GetValue(PB_DROPSIZE,t,parts.size,FOREVER);	
	pblock->GetValue(PB_CONSTANT,t,constant,FOREVER);	
	total = GetParticleCount();

	if (life <= 0) life = 1;
	if (constant) {
		brate = float(total)/float(life);		
	} else if (!TestAFlag(A_RENDER)) {
		int vc, rc;
		pblock->GetValue(PB_RNDPARTICLES,0,rc,FOREVER);
		pblock->GetValue(PB_VPTPARTICLES,0,vc,FOREVER);
		if (rc > MAX_PARTICLE_COUNT) rc = MAX_PARTICLE_COUNT;
		if (vc > MAX_PARTICLE_COUNT) vc = MAX_PARTICLE_COUNT;
		brateFactor = float(vc)/float(rc);
		}

	if (t < t0) {
		// Before the start time, nothing is happening
		parts.FreeAll();
		tvalid = t;
		valid  = TRUE;
		return;
		}

	if (!valid || t<tvalid || tvalid<t0) {
		// Set the particle system to the initial condition
		ComputeParticleStart(t0,node);
		}
	valid = TRUE;
	
	BOOL fullframe;
	if (!TestAFlag(A_RENDER))
	{ int offby=t%oneframe;
	  if (offby>0) t-=offby;
	}
	while (tvalid < t) {
		int born = 0;		

		// Compute our step size
		if (tvalid%stepSize !=0) {
			dt = stepSize - tvalid%stepSize;
		} else {
			dt = stepSize;
			}
		if (tvalid + dt > t) {
			dt = t-tvalid;
			}

		// Increment time
		tvalid += dt;

		
		// Compute the number of particles that should be born
		fullframe=(tvalid%oneframe==0);
		if (fullframe)
		{if (!constant) {pblock->GetValue(PB_BIRTHRATE,tvalid,brate,FOREVER);}
		birth = int((tvalid-t0)*brate*brateFactor) 
			- int((tvalid-t0-dt)*brate*brateFactor);
		}		

		// First increment age and kill off old particles
		for (j=0; j<parts.Count(); j++) {
			if (!parts.Alive(j)) continue;
			parts.ages[j] += dt;
			if (parts.ages[j] >= life) {
				parts.ages[j] = -1;
				}
			}

		
		// We want to seed the random number generator for all particles
		// born at this time. Construct the seed based on this time by
		// pulling out each of the 4 bytes, Perm() each one, and put
		// them back in the opposite byte order.
		int seed1 = (tvalid*1200)/TIME_TICKSPERSEC;
		int seed2 = seed1 >> 8;
		int seed3 = seed2 >> 8;
		int seed4 = seed2 >> 8;
		seed1 &= 0xFF;
		seed2 &= 0xFF;
		seed3 &= 0xFF;
		seed4 &= 0xFF;
		srand(
			(Perm(seed1)<<24) + 
			(Perm(seed2)<<16) +
			(Perm(seed3)<<8)  +
			Perm(seed4) +
			int(PARTICLE_SEED));
		
		// Next, birth particles at the birth rate
		for (j=0; j<parts.Count(); j++) {
			if (born>=birth) break;
			if (!parts.Alive(j)) {
				BirthParticle(node,tvalid,j);
				born++;
				}
			}

		// Apply forces to modify velocity
		if (fullframe)
		for (i=0; i<fields.Count(); i++) {		
			for (j=0; j<parts.Count(); j++) {
				if (!parts.Alive(j)) continue;
				force = fields[i]->Force(tvalid,parts[j],parts.vels[j],j);
				parts.vels[j] += force * float(dt);
				}
			}
		
		// Increment the positions
		for (j=0; j<parts.Count(); j++) {
			if (!parts.Alive(j)) continue;
			
			// Check for collisions
			BOOL collide = FALSE;
			float collisionTime, remTime = (float)dt;
			BOOL maybeStuck = FALSE;
			// for (int i=0; i<cobjs.Count(); i++)
			for (int i=0; i<cc.MAX_COLLISIONS_PER_STEP; i++)
			{
				// if (cobjs[i]->CheckCollision(
				//		tvalid,parts[j],parts.vels[j], float(dt), j)) {
				if (cc.CheckCollision(tvalid, parts[j], parts.vels[j], remTime, j, &collisionTime, FALSE))
				{
					collide = TRUE;
					remTime -= collisionTime;
					if (remTime <= 0.0f) break; // time limit for the current inegration step
				}
				else break;
				if (i==cc.MAX_COLLISIONS_PER_STEP-1) maybeStuck = TRUE;
			}
			if (collide)
				if (!maybeStuck) // if particle stuck we can't risk to propagate particle movement for the current frame
					parts[j] += parts.vels[j] * remTime;

			// If we didn't collide, then increment.
			if (!collide) parts[j] += parts.vels[j] * float(dt);			
			}
		}
	
	assert(tvalid==t);
	}

class EmitterCreateCallback : public CreateMouseCallBack {
	public:
		GenParticle *rain;
		Point3 p0,p1;
		IPoint2 sp0, sp1;
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	};

int EmitterCreateCallback::proc(
		ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat)
	{

	#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _3D_CREATE
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
			vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
	}
	#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point)  {
			case 0:
				sp0 = m;
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p0);
				rain->pblock->SetValue(PB_EMITTERWIDTH,0,0.01f);
				rain->pblock->SetValue(PB_EMITTERHEIGHT,0,0.01f);
				rain->pmapParam->Invalidate();
				break;

			case 1: {
				mat.IdentityMatrix();
				sp1 = m;
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				Point3 center = (p0+p1)/float(2);
				mat.SetTrans(center);
				rain->pblock->SetValue(PB_EMITTERWIDTH,0,
					(float)fabs(p1.x-p0.x));
				rain->pblock->SetValue(PB_EMITTERHEIGHT,0,
					(float)fabs(p1.y-p0.y));
				rain->pmapParam->Invalidate();

				if (msg==MOUSE_POINT) {
					if (Length(m-sp0)<3 || Length(p1-p0)<0.1f) {						
						return CREATE_ABORT;
					} else {
						return CREATE_STOP;
						}
					}
				break;
				}

			}
	} else {
		if (msg == MOUSE_ABORT)
			return CREATE_ABORT;
		}
	return 1;
	}

static EmitterCreateCallback emitterCallback;

CreateMouseCallBack* GenParticle::GetCreateMouseCallBack() 
	{
	emitterCallback.rain = this;
	return &emitterCallback;
	}

void GenParticle::InvalidateUI()
	{
	if (pmapParam) pmapParam->Invalidate();
	}

BOOL GenParticle::EmitterVisible()
	{
	int hide;
	pblock->GetValue(PB_HIDEEMITTER,0,hide,FOREVER);
	return !hide;
	}

ParamDimension *GenParticle::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_EMITTERWIDTH:
		case PB_EMITTERHEIGHT:
		case PB_DROPSIZE:		return stdWorldDim;

		case PB_STARTTIME:
		case PB_LIFETIME:		return stdTimeDim;
		
		case PB_BIRTHRATE:		return &theBirthRateDim;
		default: return defaultDim;
		}
	}

TSTR GenParticle::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_VPTPARTICLES:	return GetString(IDS_RB_VIEWPARTICLES);		
		case PB_SPEED:			return GetString(IDS_RB_SPEED);
		case PB_VARIATION:		return GetString(IDS_RB_VARIATION);
		case PB_STARTTIME:		return GetString(IDS_RB_STARTTIME);
		case PB_LIFETIME:		return GetString(IDS_RB_LIFETIME);
		case PB_EMITTERWIDTH:	return GetString(IDS_RB_WIDTH);
		case PB_EMITTERHEIGHT:	return GetString(IDS_RB_LENGTH);
		case PB_BIRTHRATE:		return GetString(IDS_RB_BIRTHRATE);
		case PB_TUMBLE:			return GetString(IDS_RB_TUMBLE);
		case PB_SCALE:			return GetString(IDS_RB_TUMBLERATE);
		case PB_DROPSIZE:		
			if (ClassID()==Class_ID(RAIN_CLASS_ID,0)) {
				return GetString(IDS_RB_DROPSIZE);
			} else {
				return GetString(IDS_RB_FLAKESIZE);
				}
			break;

		default: 				return TSTR(_T(""));
		}
	}


Mesh *GenParticle::GetFacingRenderMesh(
		TimeValue t, INode *inode, View& view, BOOL& needDelete)
	{
	Mesh *pm = new Mesh;	
	int ix=0, nx=0, count;
	float size, sz;
	Matrix3 tm = Inverse(inode->GetObjTMAfterWSM(t));
	Matrix3 cam = Inverse(view.worldToView);
	Point3 v, v0,v1, camV = cam.GetRow(3);	
	TimeValue life;

	pblock->GetValue(PB_DROPSIZE,t,size,FOREVER);
	pblock->GetValue(PB_LIFETIME,t,life,FOREVER);
	if (life<=0) life = 1;
	
	ParticleSys lastparts;
	TimeValue offtime=t%GetTicksPerFrame();
	BOOL midframe;
	midframe=offtime>0;
	if (midframe) 
	{ Update(t-offtime,inode);
	  CacheData(&parts,&lastparts);
	  Update(t,inode);
	}
	else Update(t,inode);
	count = CountLive();

	pm->setNumFaces(count*2);
	pm->setNumVerts(count*4);
	pm->setNumTVerts(count*4);
	pm->setNumTVFaces(count*2);

	for (int i=0; i<parts.Count(); i++) {
		if (!parts.Alive(i)) continue;

		// Compute this particle's size
		sz = CompParticleSize(parts.ages[i],life,size);

		// Compute a vector from the particle to the camera
		v  = Normalize(camV-parts[i]);		
		v0 = Normalize(Point3(0,0,1)^v) * sz;
		v1 = Normalize(v0^v) * sz;
		
		pm->verts[ix  ] = (parts[i]+v0+v1) * tm;
		pm->verts[ix+1] = (parts[i]-v0+v1) * tm;
		pm->verts[ix+2] = (parts[i]-v0-v1) * tm;
		pm->verts[ix+3] = (parts[i]+v0-v1) * tm;

		pm->faces[nx  ].setSmGroup(1);		
		pm->faces[nx  ].setVerts(ix+1,ix ,ix+2);
		pm->faces[nx  ].setMatID((MtlID)i);
		pm->faces[nx  ].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible
		pm->faces[nx+1].setSmGroup(1);		
		pm->faces[nx+1].setVerts(ix+3,ix+2,ix  );
		pm->faces[nx+1].setMatID((MtlID)i);
		pm->faces[nx+1].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible

		pm->setTVert(ix  ,1.0f,1.0f,0.0f);
		pm->setTVert(ix+1,0.0f,1.0f,0.0f);
		pm->setTVert(ix+2,0.0f,0.0f,0.0f);
		pm->setTVert(ix+3,1.0f,0.0f,0.0f);

		pm->tvFace[nx  ].setTVerts(ix+1,ix  ,ix+2);
		pm->tvFace[nx+1].setTVerts(ix+3,ix+2,ix  );
		
		ix += 4;
		nx += 2;
		}
	
	if (midframe) { 
		CacheData(&lastparts,&parts);
		tvalid=t-offtime;
		}
	if (Parity(tm)) FlipAllMeshFaces(pm);

	mesh.InvalidateGeomCache();
	needDelete = TRUE;
	return pm;
	}


void GenParticle::GetWorldBoundBox(
		TimeValue t, INode *inode, ViewExp* vpt, Box3& box)
	{
	int type, life;
	pblock->GetValue(PB_DISPTYPE,0,type,FOREVER);
	pblock->GetValue(PB_LIFETIME,0,life,FOREVER);
	BOOL rain = ClassID()==Class_ID(RAIN_CLASS_ID,0);
	
	if (type==0) {
		Box3 pbox;
		Matrix3 mat = inode->GetObjTMBeforeWSM(t);	
		UpdateMesh(t);
		box  = mesh.getBoundingBox();
		box  = box * mat;
		for (int i=0; i<parts.points.Count(); i++) {
			if (!parts.Alive(i)) {
				continue;
				}
			float sz = 
				CompParticleSize(parts.ages[i],life,parts.size);			
			
			Point3 pt = parts.points[i];
			pbox += pt;
			if (rain) {				
				pbox += pt - parts.vels[i] * RAINSIZEFACTOR * sz;
			} else {
				pbox += pt + Point3( sz, 0.0f, 0.0f);
				pbox += pt + Point3(-sz, 0.0f, 0.0f);
				pbox += pt + Point3( 0.0f, sz, 0.0f);
				pbox += pt + Point3( 0.0f,-sz, 0.0f);
				pbox += pt + Point3( 0.0f, 0.0f, sz);
				pbox += pt + Point3( 0.0f, 0.0f,-sz);
				}
			}
		if (!pbox.IsEmpty()) box += pbox;
	} else {
		SimpleParticle::GetWorldBoundBox(t,inode,vpt,box);
		}
	}


//--- Rain particle -----------------------------------------------


ParamUIDesc *RainParticle::ParamDesc()
	{
	return descParamRain;
	}

int RainParticle::DescCount()
	{
	return RAINPARAMDESC_LENGH;
	}

RefTargetHandle RainParticle::Clone(RemapDir& remap) 
	{
	RainParticle* newob = new RainParticle();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->mvalid.SetEmpty();	
	newob->tvalid = FALSE;
	BaseClone(this, newob, remap);
	return newob;
	}

BOOL RainParticleDraw::DrawParticle(
		GraphicsWindow *gw,ParticleSys &parts,int i)
	{
	Point3 pt[3];
	float sz = CompParticleSize(parts.ages[i],life,parts.size);
	pt[0] = parts[i];
	pt[1] = parts[i] - parts.vels[i] * RAINSIZEFACTOR * sz;
	gw->polyline(2,pt,NULL,NULL,FALSE,NULL);
	if (GetAsyncKeyState (VK_ESCAPE)) return TRUE;
	return 0;
	}

MarkerType RainParticle::GetMarkerType() 
	{
	int type;
	
	pblock->GetValue(PB_DISPTYPE,0,type,FOREVER);	

	switch (type) {
		case 0:
			parts.SetCustomDraw(&theRainDraw);
			pblock->GetValue(PB_LIFETIME,0,theRainDraw.life,FOREVER);
			if (theRainDraw.life<=0) theRainDraw.life = 1;
			return POINT_MRKR;
		case 1:
			parts.SetCustomDraw(NULL);
			return POINT_MRKR;			
		case 2:
			parts.SetCustomDraw(NULL);
			return PLUS_SIGN_MRKR;
		default:
			return PLUS_SIGN_MRKR;
		}
	}

// The ratio of the base of the tetrahedron to its height
#define BASEFACTOR	0.05f
#define THIRD 		(1.0f/3.0f)
#define SIXTH 		(1.0f/6.0f)
#define TWOTHIRD 	(2.0f/3.0f)
#define FIVESIXTHS 	(5.0f/6.0f)

void CacheData(ParticleSys *p0,ParticleSys *p1)
{ p1->points.SetCount(p0->points.Count());
  if (p0->points.Count()>0)
  {	for (int pc=0;pc<p0->points.Count();pc++)
		p1->points[pc]=p0->points[pc];
    p1->ages.SetCount(p0->ages.Count());
	for (pc=0;pc<p0->ages.Count();pc++)
		p1->ages[pc]=p0->ages[pc];
    p1->radius.SetCount(p0->radius.Count());
	for (pc=0;pc<p0->radius.Count();pc++)
		p1->radius[pc]=p0->radius[pc];
    p1->tension.SetCount(p0->tension.Count());
	for (pc=0;pc<p0->tension.Count();pc++)
		p1->tension[pc]=p0->tension[pc];
  }
}

float RainParticle::ParticleSize(TimeValue t,int i)
	{
	int type;
	TimeValue life;
	pblock->GetValue(PB_RENDER,t,type,FOREVER);	
	pblock->GetValue(PB_LIFETIME,t,life,FOREVER);
	float sz = CompParticleSize(parts.ages[i],life,parts.size);
	if (type==RENDTYPE2) return sz;
	else return Length(parts.vels[i]) * RAINSIZEFACTOR * sz;
	}

int RainParticle::ParticleCenter(TimeValue t,int i)
	{
	int type;
	pblock->GetValue(PB_RENDER,t,type,FOREVER);	
	if (type==RENDTYPE2) return PARTCENTER_CENTER;
	else return PARTCENTER_HEAD;
	}

Mesh* RainParticle::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete)
	{
	Point3 v, v0, v1;
	int ix=0, nx=0, count, type;
	Matrix3 tm = Inverse(inode->GetObjTMAfterWSM(t));
	TimeValue life;
	float sz;

	pblock->GetValue(PB_RENDER,t,type,FOREVER);	
	pblock->GetValue(PB_LIFETIME,t,life,FOREVER);
	if (life<=0) life = 1;
	if (type==RENDTYPE2) return GetFacingRenderMesh(t,inode,view,needDelete);

	Mesh *pm = new Mesh;
	ParticleSys lastparts;
	TimeValue offtime=t%GetTicksPerFrame();
	BOOL midframe;
	midframe=offtime>0;
	if (midframe) 
	{ Update(t-offtime,inode);
	  CacheData(&parts,&lastparts);
	  Update(t,inode);
	}
	else Update(t,inode);
	count = CountLive();

	pm->setNumFaces(count*4);
	pm->setNumVerts(count*4);
	pm->setNumTVerts(count*9);
	pm->setNumTVFaces(count*4);

	for (int i=0; i<parts.Count(); i++) {
		if (!parts.Alive(i)) continue;

		// Compute this particle's size
		sz = CompParticleSize(parts.ages[i],life,parts.size);

		// v0 and v1 will former a basis for the plane the tetrahedron is in
		float len;
		v  = parts.vels[i] * RAINSIZEFACTOR * sz;
		len = Length(v) * BASEFACTOR;
		if (v!=Point3(0,1,0)) {
			v0 = Normalize(v^Point3(0,1,0))*len;
		} else {
			v0 = Point3(0.0f,0.0f,len);
			}
		v1 = Normalize(v^v0)*len;
		
		pm->verts[ix  ] = (parts[i]-v) * tm;
		pm->verts[ix+1] = (parts[i]+v0) * tm;
		pm->verts[ix+2] = (parts[i]+v1-v0) * tm;
		pm->verts[ix+3] = (parts[i]-v1-v0) * tm;

		pm->faces[ix  ].setSmGroup(1);		
		pm->faces[ix  ].setVerts(ix+2,ix+1,ix);
		pm->faces[ix  ].setMatID((MtlID)i);
		pm->faces[ix  ].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible
		pm->faces[ix+1].setSmGroup(1);		
		pm->faces[ix+1].setVerts(ix+3,ix+2,ix);
		pm->faces[ix+1].setMatID((MtlID)i);
		pm->faces[ix+1].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible
		pm->faces[ix+2].setSmGroup(1);		
		pm->faces[ix+2].setVerts(ix+1,ix+3,ix);
		pm->faces[ix+2].setMatID((MtlID)i);
		pm->faces[ix+2].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible
		pm->faces[ix+3].setSmGroup(1);		
		pm->faces[ix+3].setVerts(ix+3,ix+1,ix+2);
		pm->faces[ix+3].setMatID((MtlID)i);
		pm->faces[ix+3].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible
		
		pm->setTVert(nx  ,0.0f ,0.0f,0.0f);
		pm->setTVert(nx+1,THIRD,0.0f,0.0f);
		pm->setTVert(nx+2,SIXTH,1.0f,0.0f);
		pm->setTVert(nx+3,THIRD   ,0.0f,0.0f);
		pm->setTVert(nx+4,TWOTHIRD,0.0f,0.0f);
		pm->setTVert(nx+5,0.5f    ,1.0f,0.0f);
		pm->setTVert(nx+6,TWOTHIRD  ,0.0f,0.0f);
		pm->setTVert(nx+7,1.0f      ,0.0f,0.0f);
		pm->setTVert(nx+8,FIVESIXTHS,1.0f,0.0f);
		
		pm->tvFace[ix  ].setTVerts(nx  ,nx+1, nx+2);
		pm->tvFace[ix+1].setTVerts(nx+3,nx+4, nx+5);
		pm->tvFace[ix+2].setTVerts(nx+6,nx+7, nx+8);
		pm->tvFace[ix+3].setTVerts(nx  ,nx+1, nx+2);
		
		ix += 4;
		nx += 9;
		}

	if (midframe) { 
		CacheData(&lastparts,&parts);
		tvalid=t-offtime;
		}
	if (Parity(tm)) FlipAllMeshFaces(pm);
	mesh.InvalidateGeomCache();
	needDelete = TRUE;
	return pm;
	}


//--- Snow particle -----------------------------------------------

SnowParticle::SnowParticle() : GenParticle(FALSE)
	{
	pblock->SetValue(PB_VARIATION,0,2.0f);
	}

ParamUIDesc *SnowParticle::ParamDesc()
	{
	return descParamSnow;
	}

int SnowParticle::DescCount()
	{
	return SNOWPARAMDESC_LENGH;
	}

float SnowParticle::ParticleSize(TimeValue t,int i)
	{	
	TimeValue life;	
	pblock->GetValue(PB_LIFETIME,t,life,FOREVER);
	return CompParticleSize(parts.ages[i],life,parts.size);	
	}

int SnowParticle::ParticleCenter(TimeValue t,int i)
	{
	return PARTCENTER_CENTER;	
	}

void SnowParticle::UpdateParticles(TimeValue t,INode *node)
	{
	// Make sure the snow draw callback is up to date.
	pblock->GetValue(PB_TUMBLE,t,theSnowDraw.tumble,FOREVER);
	pblock->GetValue(PB_SCALE,t,theSnowDraw.scale,FOREVER);	
	theSnowDraw.scale /= 50.0f;
	GenParticle::UpdateParticles(t,node);
	}

RefTargetHandle SnowParticle::Clone(RemapDir& remap) 
	{
	SnowParticle* newob = new SnowParticle();	
	newob->ReplaceReference(0,pblock->Clone(remap));	
	newob->mvalid.SetEmpty();	
	newob->tvalid = FALSE;
	BaseClone(this, newob, remap);
	return newob;
	}

BOOL SnowParticleDraw::DrawParticle(
		GraphicsWindow *gw,ParticleSys &parts,int i)
	{
	Point3 pt[3];	
	float size = CompParticleSize(parts.ages[i],life,parts.size);
	Matrix3 mat = obj->TumbleMat(i,tumble,scale);
	Point3 vx, vy, vz;

	vx = mat.GetRow(0) * size;
	vy = mat.GetRow(1) * size;
	vz = mat.GetRow(2) * size;

	pt[0] = parts[i] - vx;
	pt[1] = parts[i] + vx;
	gw->polyline(2,pt,NULL,NULL,FALSE,NULL);

	pt[0] = parts[i] - vy;
	pt[1] = parts[i] + vy;
	gw->polyline(2,pt,NULL,NULL,FALSE,NULL);

	pt[0] = parts[i] - vz;
	pt[1] = parts[i] + vz;
	gw->polyline(2,pt,NULL,NULL,FALSE,NULL);

	vx *= 0.5f;
	vy *= 0.5f;
	vz *= 0.5f;
	
	pt[0] = parts[i] - vx - vy - vz;
	pt[1] = parts[i] + vx + vy + vz;	
	gw->polyline(2,pt,NULL,NULL,FALSE,NULL);
	
	pt[0] = parts[i] + vx - vy - vz;
	pt[1] = parts[i] - vx + vy + vz;	
	gw->polyline(2,pt,NULL,NULL,FALSE,NULL);
	
	pt[0] = parts[i] - vx + vy - vz;
	pt[1] = parts[i] + vx - vy + vz;
	gw->polyline(2,pt,NULL,NULL,FALSE,NULL);

	pt[0] = parts[i] - vx - vy + vz;
	pt[1] = parts[i] + vx + vy - vz;	
	gw->polyline(2,pt,NULL,NULL,FALSE,NULL);	
	if (GetAsyncKeyState (VK_ESCAPE)) return TRUE;
	return 0;
	}

MarkerType SnowParticle::GetMarkerType() 
	{
	int type;
	pblock->GetValue(PB_DISPTYPE,0,type,FOREVER);	

	switch (type) {
		case 0: {			
			theSnowDraw.obj = this;
			pblock->GetValue(PB_LIFETIME,0,theSnowDraw.life,FOREVER);
			if (theSnowDraw.life<=0) theSnowDraw.life = 1;
			parts.SetCustomDraw(&theSnowDraw);			
			return POINT_MRKR;
			}

		case 1:
			parts.SetCustomDraw(NULL);
			return POINT_MRKR;			
		case 2:
			parts.SetCustomDraw(NULL);
			return PLUS_SIGN_MRKR;
		default:
			return PLUS_SIGN_MRKR;
		}
	}

Matrix3 SnowParticle::TumbleMat(int index,float amount, float scale)
	{
	Matrix3 mat;
	Quat q;
	float ang[3];

	srand(int(PARTICLE_SEED) * Perm(index) + int(PARTICLE_SEED));
	
	for (int i=0; i<3; i++) {
		ang[i] = (float(2*rand())/float(RAND_MAX) - 1.0f);
		if (amount>0.0f) {
			float off = 8725.0f*i;
			ang[i] += noise3((parts[index]+Point3(off,off,off))*scale)*amount;
			}
		ang[i] *= TWOPI;
		}
	
	EulerToQuat(ang,q);
	q.MakeMatrix(mat);
	return mat;
	}

Mesh* SnowParticle::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete)
	{
	BOOL single = FALSE;	
	float size, tumble, scale, sz;
	int type, count;
	TimeValue life;	

	pblock->GetValue(PB_DROPSIZE,t,size,FOREVER);
	pblock->GetValue(PB_TUMBLE,t,tumble,FOREVER);
	pblock->GetValue(PB_SCALE,t,scale,FOREVER);
	pblock->GetValue(PB_RENDER,t,type,FOREVER);
	pblock->GetValue(PB_LIFETIME,t,life,FOREVER);
	if (life<=0) life = 1;
	scale /= 50.0f;
	single = type==RENDTYPE2;
	if (type==RENDTYPE3) return GetFacingRenderMesh(t,inode,view,needDelete);

	Matrix3 tm = Inverse(inode->GetObjTMAfterWSM(t));
	int ix=0, nx=0, numV = single ? 3 : 6, numF = single ? 1 : 2;

	Mesh *pm = new Mesh;		
	ParticleSys lastparts;
	TimeValue offtime=t%GetTicksPerFrame();
	BOOL midframe;
	midframe=offtime>0;
	if (midframe) 
	{ Update(t-offtime,inode);
	  CacheData(&parts,&lastparts);
	  Update(t,inode);
	}
	else Update(t,inode);
	count = CountLive();

	if (!single) {
		pm->setNumFaces(count*2);
		pm->setNumVerts(count*6);
		pm->setNumTVerts(count*6);
		pm->setNumTVFaces(count*2);
	} else {
		pm->setNumFaces(count);
		pm->setNumVerts(count*3);
		pm->setNumTVerts(count*3);
		pm->setNumTVFaces(count);
		}

	for (int i=0; i<parts.Count(); i++) {
		if (!parts.Alive(i)) continue;
		
		// Compute this particle's size
		sz = CompParticleSize(parts.ages[i],life,size);

		for (int j=0; j<numV; j++) {
			pm->tVerts[ix+j].x = pm->verts[ix+j].x = 
				(float)cos(TWOPI*float(j)/float(numV));
			pm->tVerts[ix+j].z = pm->verts[ix+j].y = 0.0f;
			pm->tVerts[ix+j].y = pm->verts[ix+j].z = 
				(float)sin(TWOPI*float(j)/float(numV));
			pm->verts[ix+j]  = (pm->verts[ix+j]*TumbleMat(i,tumble,scale)) * sz;
			pm->verts[ix+j]  += parts[i];
			pm->tVerts[ix+j] *= 0.5f;
			pm->tVerts[ix+j] += Point3(0.5f,0.5f,0.0f);
			
			// Particles are in world space so we transform them back
			// into object space because that's what the renderer expects.
			pm->verts[ix+j] = tm * pm->verts[ix+j];
			}
		
		if (single) {
			pm->faces[nx].setSmGroup(0);
			pm->faces[nx].setVerts(ix,ix+1,ix+2);
			pm->faces[nx].setMatID((MtlID)i);
			pm->faces[nx  ].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible
			pm->tvFace[nx].setTVerts(ix,ix+1,ix+2);
		} else {
			pm->faces[nx  ].setSmGroup(0);
			pm->faces[nx  ].setVerts(ix  ,ix+2,ix+4);
			pm->faces[nx  ].setMatID((MtlID)i);
			pm->faces[nx  ].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible
			pm->faces[nx+1].setSmGroup(0);
			pm->faces[nx+1].setVerts(ix+1,ix+3,ix+5);
			pm->faces[nx+1].setMatID((MtlID)i);
			pm->faces[nx+1].setEdgeVisFlags(1,1,1); // RB 3/4/99: added this so snapshots would be visible
			pm->tvFace[nx  ].setTVerts(ix  ,ix+2,ix+4);
			pm->tvFace[nx+1].setTVerts(ix+1,ix+3,ix+5);
			}
		
		ix += numV;
		nx += numF;
		}
	
	if (midframe) { 
		CacheData(&lastparts,&parts);
		tvalid=t-offtime;
		}
	if (Parity(tm)) FlipAllMeshFaces(pm);
	mesh.InvalidateGeomCache();
	needDelete = TRUE;
	return pm;
	}

#endif // NO_PARTICLES