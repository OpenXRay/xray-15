/**********************************************************************
 *<
	FILE:			EvalCol.cpp
	DESCRIPTION:	Vertex Color Renderer
	CREATED BY:		Christer Janson
	HISTORY:		Created Monday, December 12, 1996

 *>	Copyright (c) 1997 Kinetix, All Rights Reserved.
 **********************************************************************/
//***************************************************************************
// December 12/13   1996	CCJ
// January  8		1997	CCJ  Bugfix
// June		1		1997	CCJ  Port to MAX 2.0
// June		6		1997	CCJ  Implemented full ShadeContext
// Dec		1		1998	CCJ  Shadow casting n' colors per face (unmixed vertex colors)
//
// Description:
// These functions calculates the diffuse, ambient or pre-lit color at each
// vertex or face of an INode.
//
// Exports:
// BOOL calcMixedVertexColors(INode*, TimeValue, int, ColorTab&);
//      This function calculates the interpolated diffuse or ambient 
//      color at each vetex of an INode.
//      Usage: Pass in a node pointer and the TimeValue to generate
//      a list of Colors corresponding to each vertex in the mesh
//      Use the int flag to specify if you want to have diffuse or 
//      ambient colors, or if you want to use the scene lights.
//      Note: 
//        You are responsible for deleting the Color objects in the table.
//      Additional note:
//        Since materials are assigned by face, this function renders each
//        face connected to the specific vertex (at the point of the vertex)
//        and mixes the colors afterwards. If this is not what you want
//        you can use the calcFaceColors() to calculate the color at the
//        centerpoint of each face.
//
//***************************************************************************

#include "max.h"
#include "bmmlib.h"
#include "evalcol.h"
#include "radiosity.h"
#include "toneop.h"
#include "notify.h"
#include "enumeration.h"

// Enable this to print out debug information
// #define EVALCOL_DEBUG

class SContext;
class RefEnumProc;
class MeshInstance;

// Information about closest hit for shadow casting
struct HitInfo {
	MeshInstance*	instance;
	float			distance;	// Distance from light to shaded point
	Point3			hitPos;
	};

typedef float Plane[4];	// A plane definition.

// Factor by which the evaluation point is moved, from the vertex towards the face center
static const float vertexEvalEspilon = 1e-2f;
// Distance, along the vertex normal, from which the evaluation ray is cast.
static const float vertexEvalDistance = 1e-3f;

Point3		interpVertexNormal(Mesh* mesh, Matrix3 tm, unsigned int vxNo, BitArray& faceList);
void		AddSceneLights(INode* node, SContext* sc, MtlBaseLib* mtls, int* numLights, RadiosityEffect* radiosity, const EvalVCOptions& evalOptions);
int			LoadMapFiles(INode* node, SContext* sc, MtlBaseLib& mtls, TimeValue t);
void		EnumRefs(ReferenceMaker *rm, RefEnumProc &proc);
TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);

// Shadow raycasting
// These functions are essentially ripped out of context from the
// sample renderer. For full information please refer to
// maxsdk\samples\cjrender
BOOL		intersectMesh(Ray* ray, HitInfo& hitInfo);
BOOL		intersectTri(Ray* ray, Point3* tri, Point3& baryCoord, Point3& normal, Point3& hitPos);
void		CompPlaneEqn(Plane plane, const Point3& p0, const Point3& p1, const Point3& p2);
BOOL		rayBoundHit(Ray* ray, Box3 boundingBox);
BOOL		TMNegParity(Matrix3 &m);
Point3		CalcBaryCoords(Point3 p0, Point3 p1, Point3 p2, Point3 p);

#define BIGFLOAT (float(1.0e30))


//***************************************************************************
//* The is the map enumerator class used for collecting projector lights for
//* spotlights
//***************************************************************************

class GetMaps: public RefEnumProc {
	MtlBaseLib *mlib;
	public:
	void proc(ReferenceMaker *rm);
	GetMaps(MtlBaseLib *mbl);
};

//***************************************************************************
//* The is the Light descriptor object for default lights
//***************************************************************************
class DefObjLight : public ObjLightDesc 
{
	public:
		Color intensCol;   // intens*color 
		DefObjLight(DefaultLight *l);
		void DeleteThis() {delete this;}
		int Update(TimeValue t, const RendContext& rc, RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged);
		int UpdateViewDepParams(const Matrix3& worldToCam);
		BOOL Illuminate(ShadeContext& sc, Point3& normal, Color& color, Point3 &dir, float &dot_nl, float &diffCoef);
};

class LightInfo {
public:
	LightInfo(INode* node, MtlBaseLib* mtls);
	LightInfo(DefaultLight* l);
    LightInfo(ObjLightDesc* objectLightDesc, LightObject* lightObject = NULL);
	~LightInfo();

	ObjLightDesc* lightDesc;
	LightObject *light;
	BOOL		bIsObscured;
	float		distanceToShadedPoint;
};

typedef Tab<LightInfo*> LightTab;

//***************************************************************************
//* The MeshInstance list is a list of all meshes in world space
//***************************************************************************

class MeshInstance {
	public:

	MeshInstance(INode* node);
	~MeshInstance();

	INode*	node;
	Mesh*	mesh;	// A copy of the mesh

	BOOL	negParity;

	Point3	center;	// Bounding sphere
	float	radsq;

	Box3	boundingBox;
	};

MeshInstance::MeshInstance(INode* node)
	{
	mesh = NULL;
	int	v;

	this->node = node;

	BOOL	deleteIt;
	TriObject* tri = GetTriObjectFromNode(node, GetCOREInterface()->GetTime(), deleteIt);
	if (tri) {
		mesh = new Mesh;
		*mesh = *&tri->GetMesh();

		Point3 vx;
		Matrix3 objToWorld = node->GetObjTMAfterWSM(GetCOREInterface()->GetTime());

		negParity = TMNegParity(objToWorld);

		mesh->buildRenderNormals();

		// Transform the vertices
		for (v=0; v<mesh->numVerts; v++) {
			vx =  objToWorld * mesh->getVert(v);
			mesh->setVert(v, vx);
			}

		Matrix3 normalObjToWorld(1);
		// Calculate the inverse-transpose of objToWorld for transforming normals.
		for (int it=0; it<3; it++) {
			Point4 p = Inverse(objToWorld).GetColumn(it);
			normalObjToWorld.SetRow(it,Point3(p[0],p[1],p[2]));
			}

		// Transform the face normals
		for (int nf = 0; nf < mesh->numFaces; nf++) {
			Point3	fn = mesh->getFaceNormal(nf);
			Point3	nfn = VectorTransform(normalObjToWorld, fn);
			mesh->setFaceNormal(nf, nfn);
		}

		boundingBox = mesh->getBoundingBox();

		// Get the bounding sphere
		center = 0.5f*(boundingBox.pmin+boundingBox.pmax);
		radsq = 0.0f;
		Point3 d;
		float nr;
		for (v= 0; v<mesh->numVerts; v++) {
			d = mesh->verts[v] - center;
			nr = DotProd(d,d);
			if (nr>radsq) radsq = nr;
			}

		if (deleteIt) {
			delete tri;
			}
		}
	}

MeshInstance::~MeshInstance()
	{
	if (mesh) {
		mesh->DeleteThis();
		}
	}

typedef Tab<MeshInstance*> MeshInstanceTab;

void InitInstanceList(INode* node, MeshInstanceTab& ilist)
	{
	if (!node->IsRootNode()) {
		if (!node->IsHidden()) {
			ObjectState os = node->EvalWorldState(GetCOREInterface()->GetTime());
			if (os.obj && os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID) {
				MeshInstance* inst = new MeshInstance(node);
				ilist.Append(1, &inst, 50);
				}
			}
		}

	int numChildren = node->NumberOfChildren();
	for (int c=0; c<numChildren; c++) {
		InitInstanceList(node->GetChildNode(c), ilist);
		}
	}

void FreeInstanceList(MeshInstanceTab& ilist)
	{
	int numItems = ilist.Count();
	for (int i=0; i<numItems; i++) {
		delete ilist[i];
		}
	ilist.ZeroCount();
	ilist.Shrink();
	}

MeshInstance* GetNodeFromMeshInstance(INode* node, MeshInstanceTab& ilist)
	{
	int numItems = ilist.Count();
	for (int i=0; i<numItems; i++) {
		if (ilist[i]->node == node) {
			return ilist[i];
			}
		}
	return NULL;
	}

//***************************************************************************
//* RendContext is used to evaluate the lights
//***************************************************************************

 class RContext: public RendContext {
	public:
		Matrix3	WorldToCam() const { return Matrix3(1); }
		ShadowBuffer*	NewShadowBuffer() const;
		ShadowQuadTree*	NewShadowQuadTree() const;
		Color	GlobalLightLevel() const;
		int	Progress(int done, int total) {
			return 1;
		}
};

//***************************************************************************
// ShadeContext for evaluating materials
//***************************************************************************

class SContext : public ShadeContext {
public:
	SContext();
	~SContext();

	TimeValue CurTime();
	int NodeID();
	INode* Node();
	Point3 BarycentricCoords();
	int FaceNumber();
	Point3 Normal();
	float Curve();

	LightDesc*	Light(int lightNo);
	Point3	GNormal(void);
	Point3	ReflectVector(void);
	Point3	RefractVector(float ior);
	Point3	CamPos(void);
	Point3	V(void);
	Point3	P(void);
	Point3	DP(void);
	Point3	PObj(void);
	Point3	DPObj(void);
	Box3	ObjectBox(void);
	Point3	PObjRelBox(void);
	Point3	DPObjRelBox(void);
	void	ScreenUV(Point2 &uv,Point2 &duv);
	IPoint2	ScreenCoord(void);
	Point3	UVW(int chan);
	Point3	DUVW(int chan);
	void	DPdUVW(Point3 [], int chan);
	void	GetBGColor(Color &bgCol, Color &transp, int fogBG);
	Point3	PointTo(const Point3 &p, RefFrame ito);
	Point3	PointFrom(const Point3 &p, RefFrame ito);
	Point3	VectorTo(const Point3 &p, RefFrame ito);
	Point3	VectorFrom(const Point3 &p, RefFrame ito);
	int		InMtlEditor();
	void	SetView(Point3 v);

	int		ProjType();
	void	SetNodeAndTime(INode* n, TimeValue tm);
	void	SetMesh(Mesh* m);
	void	SetBaryCoord(Point3 bary);
	void	SetFaceNum(int f);
	void	SetMtlNum(int mNo);
	void	SetTargetPoint(Point3 tp);
	void	SetViewPoint(Point3 vp);
	void	SetViewDir(Point3 vd);
	void	CalcNormals();
	void	CalcBoundObj();
	void	ClearLights();
	void	AddLight(LightInfo* li);
	void	SetAmbientLight(Color c);
	void	UpdateLights();
	void	calc_size_ratio();
	float	RayDiam() {return 0.1f;}
	void	getTVerts(int chan);
	void	getObjVerts();

	int		NRenderElements();
	IRenderElement*	GetRenderElement(int n);

	void	CalcShadow(MeshInstanceTab& ilist);
	void	TurnOffObscuredLights(MeshInstance* mi);
	void	SetDiffuseOnly(bool d);

public:
	LightTab lightTab;
	LightTab allLights;
	Matrix3 tmAfterWSM;
	Point3 vxNormals[3];

private:
	INode* node;
	Mesh* mesh;
	Point3 baryCoord;
	int faceNum;
	Point3 targetPt;
	Point3 viewDir;
	Point3 viewPoint;
	TimeValue t;
	UVVert tv[MAX_MESHMAPS][3];
	Point3 bumpv[MAX_MESHMAPS][3];
	Box3 boundingObj;
	RContext rc;
	Point3	obpos[3];
	Point3	dobpos;
	float ratio;
	float curve;
	bool	bDiffuseOnly;
};

//***************************************************************************
//* Dummy Material : Simple Phong shader using Node color
//* This material is assigned to each node that does not have a material
//* previously assigned. The diffuse color is assigned based on the 
//* wireframe color.
//* This way we can assume that all nodes have a material assigned.
//***************************************************************************

#define DUMMTL_CLASS_ID	Class_ID(0x4efd2694, 0x37c809f4)

#define DUMSHINE	.20f
#define DUMSPEC		.20f

class DumMtl: public Mtl {
	Color diff, spec, ambientColor;
	float phongexp;
	public:
		DumMtl(Color c);
		void Update(TimeValue t, Interval& valid);
		void Reset();
		Interval Validity(TimeValue t);
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
		Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
		Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
		float GetShininess(int mtlNum=0, BOOL backFace=FALSE);
		float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
		float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
		void SetAmbient(Color c, TimeValue t);
		void SetDiffuse(Color c, TimeValue t);
		void SetSpecular(Color c, TimeValue t);
		void SetShininess(float v, TimeValue t);
		Class_ID ClassID();
		void DeleteThis();
    	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);
		void Shade(ShadeContext& sc);
};


// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	//if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
	//	TriObject *tri = (TriObject *) obj->ConvertToType(t, 
	//		Class_ID(TRIOBJ_CLASS_ID, 0));
	//	 //Note that the TriObject should only be deleted
	//	 //if the pointer to it is not equal to the object
	//	 //pointer that called ConvertToType()
	//	if (obj != tri) deleteIt = TRUE;
	//	return tri;
	//}

	// MAB - 05/28/03 - Rewitten for the new vertex paint
	// The new vertex paint converts its input to a poly. If we convert the object to a tri-mesh and
	// caluclate colors on that, then the verts for each face may be in the wrong order compared to
	// the poly version of the object used by the modifier.
	// This is a hack fix; the object is converted to poly and then to tri-mesh to ensure a matching vert order.

	if( obj->CanConvertToType(Class_ID(POLYOBJ_CLASS_ID, 0)) ) {
		TriObject *tri = new TriObject;
		PolyObject *poly = (PolyObject*) obj->ConvertToType(t, Class_ID(POLYOBJ_CLASS_ID, 0));
		MNMesh& polyMesh = poly->GetMesh();
		polyMesh.OutToTri( tri->GetMesh() );
		if( poly!=obj ) delete poly;
		deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}

// Dummy renderer class to pass with the render global context. This class simply does nothing useful.
class NullRenderer : public Renderer  {
public:
    class NullRendParamDlg : public RendParamDlg {
        virtual void AcceptParams() {}
        virtual void DeleteThis() { delete this; }
    };

    virtual int Open(INode *scene, INode *vnode,ViewParams *viewPar,RendParams &rp, HWND hwnd, DefaultLight* defaultLights=NULL,int numDefLights=0) { return 0; }
    virtual int Render(TimeValue t, Bitmap *tobm, FrameRendParams &frp, HWND hwnd, RendProgressCallback *prog=NULL, ViewParams *viewPar=NULL) { return 0; }
    virtual void Close (HWND hwnd) {}
    virtual RendParamDlg *CreateParamDialog(IRendParams *ir,BOOL prog=FALSE) { return new NullRendParamDlg;  }
    virtual void ResetParams() {}

    static NullRenderer m_theInstance;
};

NullRenderer NullRenderer::m_theInstance;

// Initializes the render global context. Most of the values are fake, since all
// we really need is the tone operator for the radiosity
void InitRenderGlobalContext(RenderGlobalContext& renderGC) {

    renderGC.renderer = &NullRenderer::m_theInstance;
    DbgAssert(renderGC.renderer != NULL);
    renderGC.projType = PROJ_PARALLEL;
    // The width and height can't be set to 1. We get a crash with regathering
    // if they are both set to 1.
    renderGC.devWidth = 100;
    renderGC.devHeight = 100;
    renderGC.xscale = 1.0;
    renderGC.yscale = 1.0;
    renderGC.xc = 0.0;
    renderGC.yc = 0.0;
    renderGC.antialias = false;
    renderGC.camToWorld.IdentityMatrix();
    renderGC.worldToCam.IdentityMatrix();
    renderGC.nearRange = 0.0;
    renderGC.farRange = 0.0;        // Not too sure if this will work
    renderGC.devAspect = 1.0;
    renderGC.frameDur = 1.0;
    renderGC.envMap = NULL;
    renderGC.globalLightLevel = Color(1,1,1);   // Not sure what this does
    renderGC.atmos = NULL;
    renderGC.pToneOp = NULL;
    renderGC.time = GetCOREInterface()->GetTime();
    renderGC.wireMode = false;
    renderGC.wire_thick = 1.0;
    renderGC.force2Side = true;    // Not sure, but this shouldn't do any harm
    renderGC.inMtlEdit = false;
    renderGC.fieldRender = false;   // Not sure what this does
    renderGC.first_field = false;
    renderGC.field_order = false;
    renderGC.objMotBlur = false;
    renderGC.nBlurFrames = 0;

    // Set the tone operator
    ToneOperatorInterface* toneOpInt = static_cast<ToneOperatorInterface*>(
        GetCOREInterface(TONE_OPERATOR_INTERFACE));
    if(toneOpInt != NULL) {
        renderGC.pToneOp = toneOpInt->GetToneOperator();
    }
}

// Initializes a RendParams structure with appropriate values.
void InitRendParams(RendParams& rendParams, RadiosityEffect* radiosity) {

    // Default values taken from the RendParams constructor
    rendParams.rendType = RENDTYPE_NORMAL;
    rendParams.isNetRender = FALSE;
    rendParams.fieldRender = FALSE;
    rendParams.fieldOrder = 0;
    rendParams.frameDur = 0;
    rendParams.colorCheck = 0;
    rendParams.vidCorrectMethod = 0;
    rendParams.ntscPAL = 0;
    rendParams.superBlack = 0;
    rendParams.sbThresh = 0;
    rendParams.rendHidden = 0;
    rendParams.force2Side = 0;
    rendParams.inMtlEdit = 0;
    rendParams.mtlEditTile = 0;
    rendParams.mtlEditAA = 0;
    rendParams.multiThread = 0;
    rendParams.useEnvironAlpha = 0;
    rendParams.dontAntialiasBG = 0;
    rendParams.useDisplacement = 0;
    rendParams.envMap = NULL;
    rendParams.atmos = NULL;
    rendParams.effect = NULL;
    rendParams.pToneOp = NULL;
    rendParams.firstFrame = 0;
    rendParams.scanBandHeight = 0;
    rendParams.extraFlags = 0;
    rendParams.width= rendParams.height = 0;
    rendParams.filterBG = 0;

    // Non-default values
    rendParams.pRadiosity = radiosity;
    rendParams.computeRadiosity = false;
    rendParams.useRadiosity = (radiosity != NULL);
}

// Returns the active radiosity engine, if compatible with this utility. NULL otherwise.
RadiosityEffect* GetCompatibleRadiosity() {
    RadiosityInterface* radInterface = static_cast<RadiosityInterface*>(
        GetCOREInterface(RADIOSITY_INTERFACE));

    if(radInterface != NULL) {
        RadiosityEffect* radiosity = radInterface->GetRadiosity();
        // Check if the radiosity engine is compatible using its classID.
        // Add class IDs here if needed.
        if((radiosity != NULL) 
            && (radiosity->ClassID() == Class_ID(0x795c4168, 0x669a1835)))
        {
            return radiosity;
        }
    }

    return NULL;
}

// This function calls EnumRefHierarchy() on render references, using the
// given RefEnumProc.
void EnumRenderRefs(RendParams& rendParams, RefEnumProc& refEnumProc) {

    // Enumerate every node in the scene
    SceneRefEnumerator sceneRefEnumerator(refEnumProc);
    sceneRefEnumerator.enumerate();

    // Enumerate on the radiosity plugin and tone operator
    if(rendParams.pRadiosity != NULL)
        EnumRefHierarchy(rendParams.pRadiosity, refEnumProc);
    if(rendParams.pToneOp != NULL)
        EnumRefHierarchy(rendParams.pToneOp, refEnumProc);
}

// This function does pre-render initializations. These are at least required
// by radiosity to initialize the raytrace/regather engine.
void PreRenderInit(RendParams& rendParams) {

    // First, broadcast PRE_RENDER notification
    BroadcastNotification(NOTIFY_PRE_RENDER, &rendParams);

    // Call RenderBegin() on every animatable
    // First clear the A1_WORK flag
    ClearA1Enum clearA1Enum;
    EnumRenderRefs(rendParams, clearA1Enum);
    RenderBeginEnum renderBeginEnum(GetCOREInterface()->GetTime(), 0);
    EnumRenderRefs(rendParams, renderBeginEnum);
}

// This function does the opposite of PreRenderInit(), and should be called
// when done rendering
void PostRenderInit(RendParams& rendParams) {

    // broadcast POST_RENDER notification
    BroadcastNotification(NOTIFY_POST_RENDER);

    // Call RenderEnd() on every animatable
    // First clear the A1_WORK flag
    ClearA1Enum clearA1Enum;
    EnumRenderRefs(rendParams, clearA1Enum);
    RenderEndEnum renderEndEnum(GetCOREInterface()->GetTime());
    EnumRenderRefs(rendParams, renderEndEnum);
}

//***************************************************************************
// Calculate ambient or diffuse color at each vertex.
//***************************************************************************
BOOL calcFaceColors(INode* node, TimeValue t, const EvalVCOptions& evalOptions, FaceColorTab& faceColTab, EvalColProgressCallback* fn)
{
	ObjectState ostate;
	Mesh* mesh;
	SContext sc;
	DefaultLight dl1, dl2;
	MtlBaseLib mtls;
	Matrix3 tm;
	MeshInstanceTab	instanceList;

    bool castShadows = evalOptions.castShadows && 
		((evalOptions.lightingModel == LightingModel::kLightingOnly) 
          || (evalOptions.lightingModel == LightingModel::kShadedLighting));

	sc.SetNodeAndTime(node, t);

	InitInstanceList(GetCOREInterface()->GetRootNode(), instanceList);

	MeshInstance* mi = GetNodeFromMeshInstance(node, instanceList);
	if (!mi) {
		FreeInstanceList(instanceList);
		return TRUE;	// Returning FALSE would prevent us from processing the next node.
		}

	mesh = mi->mesh;
	if (!mesh) {
		FreeInstanceList(instanceList);
		return TRUE;	// Returning FALSE would prevent us from processing the next node.
		}

	// If the node doesn't have a material attached,
	// we create a dummy material.
	Mtl* mtl = node->GetMtl();
	if (!mtl) {
		mtl = new DumMtl(Color(node->GetWireColor()));
	}

	faceColTab.ZeroCount();
	faceColTab.Shrink();

	sc.SetMesh(mesh);
	sc.CalcBoundObj();
	sc.doMaps = evalOptions.useMaps;
	sc.SetDiffuseOnly(evalOptions.lightingModel == LightingModel::kShadedOnly);

    // Create a RenderGlobalContext, necessary for using the tone operator with radiosity
    RenderGlobalContext renderGlobalContext;
    InitRenderGlobalContext(renderGlobalContext);
    sc.globContext = &renderGlobalContext;

	// Add the material to the list
	mtls.AddMtl(mtl);

    RadiosityEffect* radiosity = NULL;

	// If we're using the real lights, we need to find them first
	if ((evalOptions.lightingModel == LightingModel::kLightingOnly) 
        || (evalOptions.lightingModel == LightingModel::kShadedLighting)) 
    {
        // Retrieve the radiosity plugin if needed
        if(evalOptions.useRadiosity) {
            radiosity = GetCompatibleRadiosity();
        }

        // Add the lights to the shade context
		int numLights = 0;
		AddSceneLights(node, &sc, &mtls, &numLights, radiosity, evalOptions);

		// Add default lights if there are no lights in the scene
		if (numLights == 0) {

			dl1.ls.intens = 1.0f;
			dl1.ls.color = Color(1.0f, 1.0f, 1.0f);
			dl1.ls.type = OMNI_LGT;
			dl1.tm = TransMatrix(1000.0f * Point3(-900.0f, -1000.0f, 1500.0f));

			dl2.ls.intens = 1.0f;
			dl2.ls.color = Color(1.0f, 1.0f, 1.0f);
			dl2.ls.type = OMNI_LGT;
			dl2.tm = TransMatrix(-1000.0f * Point3(-900.0f, -1000.0f, 1500.0f));

            // Add the default lights only if radiosity accepts them
            IRadiosityEffectExtension* radiosityExtension = 
                (radiosity == NULL) ? NULL : static_cast<IRadiosityEffectExtension*>(
                radiosity->GetInterface(IRADIOSITYEFFECT_EXTENSION_INTERFACE));

            if((radiosityExtension == NULL) || radiosityExtension->UseDefaultLight(dl1))
			    sc.AddLight(new LightInfo(&dl1));
            if((radiosityExtension == NULL) || radiosityExtension->UseDefaultLight(dl2))
			    sc.AddLight(new LightInfo(&dl2));
        }

		sc.SetAmbientLight(GetCOREInterface()->GetAmbient(t, FOREVER));
        
        // Add the radiosity LightDesc to the shade context
        if(radiosity != NULL) {
            int count = radiosity->NumLightDesc();
            ObjLightDesc** lightDescBuffer = new ObjLightDesc*[count];
            radiosity->CreateLightDesc(lightDescBuffer);
            for(int i = 0; i < count; ++i) {
                sc.AddLight(new LightInfo(lightDescBuffer[i]));
            }
            delete[] lightDescBuffer;
        }
	}

	if (castShadows) {
		for (int i=0; i<sc.lightTab.Count(); i++) {
			sc.allLights.Append(1, &(sc.lightTab[i]), 5);
		}
	}

	sc.UpdateLights();
	// Update material
	mtl->Update(t, FOREVER);

    // Do the necessary pre-render stuff
    RendParams rendParams;
    InitRendParams(rendParams, radiosity);
    PreRenderInit(rendParams);

	int numFaces = mesh->numFaces;
	for (unsigned int f = 0; f < (unsigned)numFaces; f++) {
		if (fn) {
			if (fn->progress(float(f)/float(numFaces))) {
				FreeInstanceList(instanceList);

				mtls.Empty();

				if (mtl->ClassID() == DUMMTL_CLASS_ID) {
					delete mtl;
				}

				return FALSE;
			}
		}

		FaceColor* faceCol = new FaceColor();

        // Calculate the coordinate of the center of the face
        Point3 faceCenter = 
            (mesh->verts[mesh->faces[f].v[0]] + mesh->verts[mesh->faces[f].v[1]] + mesh->verts[mesh->faces[f].v[2]]);
        faceCenter /= 3;

		sc.SetFaceNum(f);
		Face* face = &mesh->faces[f];
		sc.SetMtlNum(face->getMatID());
		sc.CalcNormals();

		for (int fv=0; fv<3; fv++) {

			Point3 vxNormal = sc.vxNormals[fv];
			Point3 viewTarget = mesh->verts[mesh->faces[f].v[fv]];

            // Move the view point slightly towards to center of the face to avoid floating-point precision problems
            viewTarget += (faceCenter - viewTarget) * vertexEvalEspilon;

			Point3 viewPoint = viewTarget + vertexEvalDistance*vxNormal;
			Point3 viewDir = -vxNormal;
			Point3 lightPos = viewPoint;

			// render vertex for this face.
			sc.SetViewPoint(viewPoint);
			sc.SetTargetPoint(viewTarget);
			sc.SetViewDir(viewDir);

			// Setup the barycentric coordinate
            float primaryVertexWeight = 1.0f - (vertexEvalEspilon  * (2.0f/3.0f));
            float secondaryVertexWeight = (1.0f - primaryVertexWeight) / 2.0f;
			if (fv == 0)
				sc.SetBaryCoord(Point3(primaryVertexWeight, secondaryVertexWeight, secondaryVertexWeight));
			else if (fv == 1)
				sc.SetBaryCoord(Point3(secondaryVertexWeight, primaryVertexWeight, secondaryVertexWeight));
			else if (fv == 2)
				sc.SetBaryCoord(Point3(secondaryVertexWeight, secondaryVertexWeight, primaryVertexWeight));

			// Use diffuse color instead of ambient
			// The only difference is that we create a special light
			// located at the viewpoint and we set the ambient light to black.
			if (evalOptions.lightingModel == LightingModel::kShadedOnly) {
				dl1.ls.intens = 1.0f;
				dl1.ls.color = Color(0.8f, 0.8f, 0.8f);
				dl1.ls.type = OMNI_LGT;
				dl1.tm = TransMatrix(lightPos);

				sc.ClearLights();
				sc.AddLight(new LightInfo(&dl1));
				sc.UpdateLights();
			}
			else {
				if (castShadows) {
					sc.CalcShadow(instanceList);
				}
			}

            // If we only want the illumination...
            if(evalOptions.lightingModel == LightingModel::kLightingOnly) {
                Color lightIllum;
                Color totalIllum(0,0,0);
                Point3 dummyDir;
                float dummyDot_NL;
                float dummyDiffuseCoef;

                for(int i = 0; i < sc.nLights; ++i) {
                    LightDesc* lightDesc = sc.Light(i);
                    if(lightDesc->Illuminate(sc, sc.Normal(), lightIllum, dummyDir,
                        dummyDot_NL, dummyDiffuseCoef))
                    {
                        totalIllum += lightIllum;
                    }
                }

                faceCol->colors[fv] = totalIllum;
            }
            // Else, shade the vertex
            else {
			    mtl->Shade(sc);
                faceCol->colors[fv].r = sc.out.c.r;
			    faceCol->colors[fv].g = sc.out.c.g;
			    faceCol->colors[fv].b = sc.out.c.b;
            }

            // Apply the tone operator
            if((sc.globContext != NULL) 
                && (sc.globContext->pToneOp != NULL)
                && !sc.globContext->pToneOp->GetIndirectOnly())
            {
                sc.globContext->pToneOp->ScaledToRGB(faceCol->colors[fv]);
            }

            faceCol->colors[fv].ClampMinMax();
		}

		// Append the Color to the table. If the array needs
		// to be realloc'ed, allocate extra space for 100 points.
		faceColTab.Append(1, &faceCol, 100);
    }

    // Do the necessary post-render stuff
    PostRenderInit(rendParams);

	if (castShadows) {
		sc.lightTab.ZeroCount();
		for (int i=0; i<sc.allLights.Count(); i++) {
			sc.lightTab.Append(1, &(sc.allLights[i]), 5);
		}
	}

	mtls.Empty();

	if (mtl->ClassID() == DUMMTL_CLASS_ID) {
		delete mtl;
	}

	FreeInstanceList(instanceList);

	return TRUE;
}



//***************************************************************************
// Calculate ambient or diffuse color at each vertex.
// Pass in TRUE as the "diffuse" parameter to calculate the diffuse color.
// If FALSE is passed in, ambient color is calculated.
//***************************************************************************
BOOL calcMixedVertexColors(INode* node, TimeValue t, const EvalVCOptions& evalOptions, ColorTab& vxColTab, EvalColProgressCallback* fn)
{
	ObjectState ostate;
	Mesh* mesh;
	SContext sc;
	DefaultLight dl1, dl2;
	MtlBaseLib mtls;
	Matrix3 tm;
	MeshInstanceTab	instanceList;

    bool castShadows = evalOptions.castShadows && 
        ((evalOptions.lightingModel == LightingModel::kLightingOnly) 
          || (evalOptions.lightingModel == LightingModel::kShadedLighting));

	sc.SetNodeAndTime(node, t);

	InitInstanceList(GetCOREInterface()->GetRootNode(), instanceList);

	MeshInstance* mi = GetNodeFromMeshInstance(node, instanceList);
	if (!mi) {
		FreeInstanceList(instanceList);
		return TRUE;	// Returning FALSE would prevent us from processing the next node.
		}

	mesh = mi->mesh;
	if (!mesh) {
		FreeInstanceList(instanceList);
		return TRUE;	// Returning FALSE would prevent us from processing the next node.
		}

	// If the node doesn't have a material attached,
	// we create a dummy material.
	Mtl* mtl = node->GetMtl();
	if (!mtl) {
		mtl = new DumMtl(Color(node->GetWireColor()));
	}

	vxColTab.ZeroCount();
	vxColTab.Shrink();

	sc.SetMesh(mesh);
	sc.CalcBoundObj();
	sc.doMaps = evalOptions.useMaps;
	sc.SetDiffuseOnly(evalOptions.lightingModel == LightingModel::kShadedOnly);

    // Create a RenderGlobalContext, necessary for using the tone operator with radiosity
    RenderGlobalContext renderGlobalContext;
    InitRenderGlobalContext(renderGlobalContext);
    sc.globContext = &renderGlobalContext;

	// Add the material to the list
	mtls.AddMtl(mtl);

    RadiosityEffect* radiosity = NULL;

	// If we're using the real lights, we need to find them first
	if ((evalOptions.lightingModel == LightingModel::kLightingOnly) 
        || (evalOptions.lightingModel == LightingModel::kShadedLighting)) 
    {
        // Retrieve the radiosity plugin if needed
        if(evalOptions.useRadiosity) {
            radiosity = GetCompatibleRadiosity();
        }

        // Add the lights to the shade context
		int numLights = 0;
		AddSceneLights(node, &sc, &mtls, &numLights, radiosity, evalOptions);

		// Add default lights if there are no lights in the scene
		if (numLights == 0) {

			dl1.ls.intens = 1.0f;
			dl1.ls.color = Color(1.0f, 1.0f, 1.0f);
			dl1.ls.type = OMNI_LGT;
			dl1.tm = TransMatrix(1000.0f * Point3(-900.0f, -1000.0f, 1500.0f));

			dl2.ls.intens = 1.0f;
			dl2.ls.color = Color(1.0f, 1.0f, 1.0f);
			dl2.ls.type = OMNI_LGT;
			dl2.tm = TransMatrix(-1000.0f * Point3(-900.0f, -1000.0f, 1500.0f));

            // Add the default lights only if radiosity accepts them
            IRadiosityEffectExtension* radiosityExtension = 
                (radiosity == NULL) ? NULL : static_cast<IRadiosityEffectExtension*>(
                radiosity->GetInterface(IRADIOSITYEFFECT_EXTENSION_INTERFACE));

            if((radiosityExtension == NULL) || radiosityExtension->UseDefaultLight(dl1))
                sc.AddLight(new LightInfo(&dl1));
            if((radiosityExtension == NULL) || radiosityExtension->UseDefaultLight(dl2))
			    sc.AddLight(new LightInfo(&dl2));
		}

		sc.SetAmbientLight(GetCOREInterface()->GetAmbient(t, FOREVER));
        
        // Add the radiosity LightDesc to the shade context
        if(radiosity != NULL) {
            int count = radiosity->NumLightDesc();
            ObjLightDesc** lightDescBuffer = new ObjLightDesc*[count];
            radiosity->CreateLightDesc(lightDescBuffer);
            for(int i = 0; i < count; ++i) {
                sc.AddLight(new LightInfo(lightDescBuffer[i]));
            }
            delete[] lightDescBuffer;
        }
	}

	if (castShadows) {
		for (int i=0; i<sc.lightTab.Count(); i++) {
			sc.allLights.Append(1, &(sc.lightTab[i]), 5);
			}
		}

	sc.UpdateLights();
	// Update material
	mtl->Update(t, FOREVER);
	
    // Do the necessary pre-render stuff
    RendParams rendParams;
    InitRendParams(rendParams, radiosity);
    PreRenderInit(rendParams);

	int numVerts = mesh->numVerts;
	for (unsigned int v = 0; v < (unsigned)numVerts; v++) {

		if (fn) {
			if (fn->progress(float(v)/float(numVerts))) {
				FreeInstanceList(instanceList);

				mtls.Empty();

				if (mtl->ClassID() == DUMMTL_CLASS_ID) {
					delete mtl;
				}

				return FALSE;
			}
		}

		// Create a new entry
		Color* vxCol = new Color;
		Point3 tmpCol(0.0f, 0.0f, 0.0f);

		int numShades = 0;
		BitArray faceList;
		faceList.SetSize(mesh->numFaces, 0);

		// Get vertex normal
		// We also pass in a BitArray that will be filled in with
		// to inform us to which faces this vertex belongs.
		// We could do this manually, but we need to do it to get
		// the vertex normal anyway so this is done to speed things
		// up a bit.
		Point3 vxNormal = interpVertexNormal(mesh, tm, v, faceList);
		Point3 viewDir = -vxNormal;
		Point3 viewPoint = mesh->verts[v] + vertexEvalDistance*vxNormal;
		Point3 lightPos = viewPoint;
		Point3 viewTarget = mesh->verts[v];

		// We now have a viewpoint and a view target.
		// Now we just have to shade this point on the mesh in order
		// to get it's color.
		// Note: 
		// Since materials are assigned on Face basis we need to render each
		// vertex as many times as it has connecting faces.
		// the colors collected are mixed to get the resulting
		// color at each vertex.

		for (int nf = 0; nf < faceList.GetSize(); nf++) {
			if (faceList[nf]) {

                // Calculate the coordinate of the center of the face
                Point3 faceCenter = 
                    (mesh->verts[mesh->faces[nf].v[0]] + mesh->verts[mesh->faces[nf].v[1]] + mesh->verts[mesh->faces[nf].v[2]]);
                faceCenter /= 3;

				// render vertex for this face.
				sc.SetViewPoint(viewPoint);
				sc.SetTargetPoint(viewTarget + ((faceCenter - viewTarget) * vertexEvalEspilon));
				sc.SetViewDir(viewDir);
				sc.SetFaceNum(nf);
				Face* f = &mesh->faces[nf];
				sc.SetMtlNum(f->getMatID());
				sc.CalcNormals();

				// Setup the barycentric coordinate
                float primaryVertexWeight = 1.0f - (vertexEvalEspilon  * (2.0f/3.0f));
                float secondaryVertexWeight = (1.0f - primaryVertexWeight) / 2.0f;
				if (mesh->faces[nf].v[0] == v)
				    sc.SetBaryCoord(Point3(primaryVertexWeight, secondaryVertexWeight, secondaryVertexWeight));
				else if (mesh->faces[nf].v[1] == v)
				    sc.SetBaryCoord(Point3(secondaryVertexWeight, primaryVertexWeight, secondaryVertexWeight));
				else if (mesh->faces[nf].v[2] == v)
				    sc.SetBaryCoord(Point3(secondaryVertexWeight, secondaryVertexWeight, primaryVertexWeight));

				// Use diffuse color instead of ambient
				// The only difference is that we create a special light
				// located at the viewpoint and we set the ambient light to black.
			if (evalOptions.lightingModel == LightingModel::kShadedOnly) {
					dl1.ls.intens = 1.0f;
					dl1.ls.color = Color(0.8f, 0.8f, 0.8f);
					dl1.ls.type = OMNI_LGT;
					dl1.tm = TransMatrix(lightPos);

					sc.ClearLights();
					sc.AddLight(new LightInfo(&dl1));
					sc.UpdateLights();
				}
				else {
					if (castShadows) {
						sc.CalcShadow(instanceList);
					}
				}
 
                // If we only want the illumination...
                if(evalOptions.lightingModel == LightingModel::kLightingOnly) {
                    Color lightIllum;
                    Color totalIllum(0,0,0);
                    Point3 dummyDir;
                    float dummyDot_NL;
                    float dummyDiffuseCoef;

                    for(int i = 0; i < sc.nLights; ++i) {
                        LightDesc* lightDesc = sc.Light(i);
                        if(lightDesc->Illuminate(sc, sc.Normal(), lightIllum, dummyDir,
                            dummyDot_NL, dummyDiffuseCoef))
                        {
                            totalIllum += lightIllum;
                        }
                    }

                    tmpCol += totalIllum;
                }
                // Else, shade the vertex
                else {
				    // Shade the vertex
				    mtl->Shade(sc);

				    tmpCol.x += sc.out.c.r;
				    tmpCol.y += sc.out.c.g;
				    tmpCol.z += sc.out.c.b;
                }

				numShades++;
			}
		}

        // The color mixes. We just add the colors together and 
		// then divide with as many colors as we added.
		if (numShades > 0) {
			tmpCol = tmpCol / (float)numShades;
		}
		
		vxCol->r = tmpCol.x;
		vxCol->g = tmpCol.y;
		vxCol->b = tmpCol.z;

        // Apply the tone operator
        if((sc.globContext != NULL) 
            && (sc.globContext->pToneOp != NULL)
            && !sc.globContext->pToneOp->GetIndirectOnly())
        {
            sc.globContext->pToneOp->ScaledToRGB(*vxCol);
        }
		
		vxCol->ClampMinMax();

		
		// Append the Color to the table. If the array needs
		// to be realloc'ed, allocate extra space for 100 points.
		vxColTab.Append(1, &vxCol, 100);
	}

    // Do the necessary psot-render stuff
    PostRenderInit(rendParams);

	if (castShadows) {
		sc.lightTab.ZeroCount();
		for (int i=0; i<sc.allLights.Count(); i++) {
			sc.lightTab.Append(1, &(sc.allLights[i]), 5);
			}
		}

	FreeInstanceList(instanceList);

	mtls.Empty();

	if (mtl->ClassID() == DUMMTL_CLASS_ID) {
		delete mtl;
	}

	return TRUE;
}


// Since vertices might have different normals depending on the face
// you are accessing it through, we get the normal for each face that
// connects to this vertex and interpolate these normals to get a single
// vertex normal fairly perpendicular to the mesh at the point of
// this vertex.
Point3 interpVertexNormal(Mesh* mesh, Matrix3 tm, unsigned int vxNo, BitArray& faceList)
{
	Point3 iNormal = Point3(0.0f, 0.0f, 0.0f);
	int numNormals = 0;

	for (int f = 0; f < mesh->numFaces; f++) {
		for (int fi = 0; fi < 3; fi++) {
			if (mesh->faces[f].v[fi] == vxNo) {
				Point3& fn = VectorTransform(tm, mesh->getFaceNormal(f));
				iNormal += fn;
				numNormals++;
				faceList.Set(f);
			}
		}
	}

	iNormal = iNormal / (float)numNormals;

	return Normalize(iNormal);
}


//***************************************************************************
// LightInfo encapsulates the light descriptor for standard and default lights
//***************************************************************************

LightInfo::LightInfo(INode* node, MtlBaseLib* mtls)
{
	ObjectState ostate = node->EvalWorldState(0);

	light = (LightObject*)ostate.obj;
	lightDesc = light->CreateLightDesc(node);
	bIsObscured = FALSE;

	// Process projector maps
	GetMaps getmaps(mtls);
	EnumRefs(light,getmaps);
}

LightInfo::LightInfo(DefaultLight* l)
{
	lightDesc = new DefObjLight(l);
	light = NULL;
}

LightInfo::LightInfo(ObjLightDesc* objectLightDesc, LightObject* lightObject) {

    lightDesc = objectLightDesc;
    light = lightObject;
    bIsObscured = FALSE;
}

LightInfo::~LightInfo()
{
	if (lightDesc) {
		delete lightDesc;
	}
}


//***************************************************************************
// Light Descriptor for the diffuse light we use
//***************************************************************************

DefObjLight::DefObjLight(DefaultLight *l) : ObjLightDesc(NULL)
{
	inode = NULL;
	ls = l->ls;
	lightToWorld = l->tm;
	worldToLight = Inverse(lightToWorld);
}


//***************************************************************************
// Update
//***************************************************************************

int DefObjLight::Update(TimeValue t, const RendContext& rc, RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged)
{
	intensCol  = ls.intens*ls.color;
	return 1;
}


//***************************************************************************
// Update viewdependent parameters
//***************************************************************************

int DefObjLight::UpdateViewDepParams(const Matrix3& worldToCam)
{
	lightToCam = lightToWorld * worldToCam;
	camToLight = Inverse(lightToCam);
	lightPos   = lightToCam.GetRow(3);  // light pos in camera space
	return 1;
}


//***************************************************************************
// Illuminate method for default lights
// This is a special illumination method in order to evaluate diffuse color
// only, with no specular etc.
//***************************************************************************

BOOL DefObjLight::Illuminate(ShadeContext& sc, Point3& normal, Color& color, Point3 &dir, float &dot_nl, float &diffCoef)
{
	dir = FNormalize(lightPos-sc.P());
	color = intensCol;
	diffCoef = dot_nl = DotProd(normal,dir);
	return (dot_nl<=0.0f)?0:1;
}

static inline Point3 pabs(Point3 p) { return Point3(fabs(p.x),fabs(p.y),fabs(p.z)); }

// The ShadeContext used to shade a material a a specific point.
// This ShadeContext is setup to have full ambient light and no other
// lights until you call SetLight(). This will cause the ambient light to
// go black.
SContext::SContext()
{
	mode = SCMODE_NORMAL;
	doMaps = TRUE;
	filterMaps = TRUE;
	shadow = FALSE;
	backFace = FALSE;
	ambientLight = Color(1.0f, 1.0f, 1.0f);
	mtlNum = 0;

	nLights = 0;
}

SContext::~SContext()
{
	ClearLights();
}


// When the mesh and face number is specified we calculate 
// and store the vertex normals
void SContext::CalcNormals()
{
	RVertex* rv[3];
	Face* f = &mesh->faces[faceNum];
	DWORD smGroup = f->smGroup;
	int numNormals;

	// Get the vertex normals
	for (int i = 0; i < 3; i++) {
		rv[i] = mesh->getRVertPtr(f->getVert(i));

		// Is normal specified
		// SPCIFIED is not currently used, but may be used in future versions.
		if (rv[i]->rFlags & SPECIFIED_NORMAL) {
			vxNormals[i] = rv[i]->rn.getNormal();
		}
		// If normal is not specified it's only available if the face belongs
		// to a smoothing group
        else {
            numNormals = rv[i]->rFlags & NORCT_MASK;
            if ((numNormals > 0) && (smGroup != 0)) {
			    // If there is only one vertex is found in the rn member.
			    if (numNormals == 1) {
                    DbgAssert(rv[i]->rn.getSmGroup() & smGroup);
				    vxNormals[i] = rv[i]->rn.getNormal();
			    }
			    else {
				    // If two or more vertices are there you need to step through them
				    // and find the vertex with the same smoothing group as the current face.
				    // You will find multiple normals in the ern member.
				    for (int j = 0; j < numNormals; j++) {
					    if (rv[i]->ern[j].getSmGroup() & smGroup) {
						    vxNormals[i] = rv[i]->ern[j].getNormal();
                            break;
					    }
				    }
			    }
		    }
		    else {
			    vxNormals[i] = mesh->getFaceNormal(faceNum);
		    }
        }
	}
	vxNormals[0] = Normalize(VectorTransform(tmAfterWSM, vxNormals[0]));
	vxNormals[1] = Normalize(VectorTransform(tmAfterWSM, vxNormals[1]));
	vxNormals[2] = Normalize(VectorTransform(tmAfterWSM, vxNormals[2]));
}

void SContext::SetBaryCoord(Point3 bary)
{
	baryCoord = bary;
}

int SContext::ProjType()
	{
	return bDiffuseOnly ? PROJ_PARALLEL : PROJ_PERSPECTIVE;
	}

void SContext::SetNodeAndTime(INode* n, TimeValue tv)
{
	node = n;
	t = tv;
	tmAfterWSM = node->GetObjTMAfterWSM(t,NULL);
}

void SContext::SetFaceNum(int f)
{
	faceNum = f;
}

void SContext::SetMtlNum(int mNo)
{
	mtlNum = mNo;
}

void SContext::SetViewPoint(Point3 vp)
{
	viewPoint = vp;
}

void SContext::SetTargetPoint(Point3 tp)
{
	targetPt = tp;
}

void SContext::SetViewDir(Point3 vd)
{
	viewDir = vd;
}

void SContext::SetMesh(Mesh * m)
{
	mesh = m;
}

void SContext::AddLight(LightInfo* li)
{
	lightTab.Append(1, &li);
}

void SContext::ClearLights()
{
	for (int i=0; i<lightTab.Count(); i++) {
		delete lightTab[i];
	}
	lightTab.ZeroCount();
	lightTab.Shrink();
	allLights.ZeroCount();
	allLights.Shrink();

	nLights = 0;
}

void SContext::UpdateLights()
{
	for (int i=0; i<lightTab.Count(); i++) {
		((LightInfo*)lightTab[i])->lightDesc->Update(t, rc, globContext, FALSE, TRUE);
		((LightInfo*)lightTab[i])->lightDesc->UpdateViewDepParams(Matrix3(1));
	}

	nLights = lightTab.Count();
}

void SContext::SetAmbientLight(Color c)
{
	ambientLight = c;
}

void SContext::CalcBoundObj()
{
	if (!mesh)
		return;

	boundingObj.Init();

	// Include each vertex in the bounding box
	for (int nf = 0; nf < mesh->numFaces; nf++) {
		Face* f = &(mesh->faces[nf]);

		boundingObj += mesh->getVert(f->getVert(0));
		boundingObj += mesh->getVert(f->getVert(1));
		boundingObj += mesh->getVert(f->getVert(2));
	}
}

// Return current time
TimeValue SContext::CurTime()
{
	return t;
}

int SContext::NodeID()
{
	return -1;
}

INode* SContext::Node()
{
	return node;
}

Point3 SContext::BarycentricCoords()
{
	return baryCoord;
}

int SContext::FaceNumber()
{
	return faceNum;
}


// Interpolated normal
Point3 SContext::Normal()
{
	return Normalize(baryCoord.x*vxNormals[0] + baryCoord.y*vxNormals[1] + baryCoord.z*vxNormals[2]);
}

// Geometric normal (face normal)
Point3 SContext::GNormal(void)
{
    // [dl | 18june2002] The face normals are already in world space.
    // see: MeshInstance::MeshInstance()

	// The face normals are already in camera space <== [dl] yeah, but camera space = world space
	//return VectorTransform(tmAfterWSM, mesh->getFaceNormal(faceNum));

    return mesh->getFaceNormal(faceNum);
}

// Return a Light descriptor
LightDesc *SContext::Light(int lightNo)
{
	return ((LightInfo*)lightTab[lightNo])->lightDesc;
}

// Return reflection vector at this point.
// We do it like this to avoid specular color to show up.
Point3 SContext::ReflectVector(void)
	{
	Point3 nrm;
	if (bDiffuseOnly) {
		// If we aren't using scene lights, then we fake a light
		// just above the render point, therefore we need to remove
		// all specularity (otherwise it's all gonna be white)
		// We can do this by pointing the reflection vector away
		// from the viewpoint
		nrm = -Normal();
		}
	else {
		Point3 N = Normal();
		float VN = -DotProd(viewDir,N);
		nrm = Normalize(2.0f*VN*N + viewDir);
		}

	return nrm;
	}

// Foley & vanDam: Computer Graphics: Principles and Practice, 
//     2nd Ed. pp 756ff.
Point3 SContext::RefractVector(float ior)
{
	Point3 N = Normal();
	float VN,nur,k;
	VN = DotProd(-viewDir,N);
	if (backFace) nur = ior;
	else nur = (ior!=0.0f) ? 1.0f/ior: 1.0f;
	k = 1.0f-nur*nur*(1.0f-VN*VN);
	if (k<=0.0f) {
		// Total internal reflection: 
		return ReflectVector();
	}
	else {
		return (nur*VN-(float)sqrt(k))*N + nur*viewDir;
	}
}

Point3 SContext::CamPos(void)
{
	return viewPoint;
}

// Screen coordinate beeing rendered
IPoint2 SContext::ScreenCoord(void)
{
	return IPoint2(0,0);
}

// Background color
void SContext::GetBGColor(class Color &bgCol,class Color &transp,int fogBG)
{
	bgCol = Color(0.0f, 0.0f, 0.0f);
	transp = Color(0.0f, 0.0f, 0.0f);
}

// Transforms the specified point from internal camera space to the specified space.
Point3 SContext::PointTo(const class Point3 &p, RefFrame ito)
{
	if (ito==REF_OBJECT) {
		return Inverse(tmAfterWSM) * p;
	}

	return p;
}

// Transforms the specified point from the specified coordinate system
// to internal camera space.
Point3 SContext::PointFrom(const class Point3 &p, RefFrame ito)
{
	if (ito==REF_OBJECT) {
		return tmAfterWSM * p;
	}
	return p;
}

// Transform the vector from internal camera space to the specified space.
Point3 SContext::VectorTo(const class Point3 &p, RefFrame ito)
{
	if (ito==REF_OBJECT) {
		return VectorTransform(Inverse(tmAfterWSM), p);
	}
	return p;
}

// Transform the vector from the specified space to internal camera space.
Point3 SContext::VectorFrom(const class Point3 &p, RefFrame ito)
{
	if (ito==REF_OBJECT) {
		return VectorTransform(tmAfterWSM, p);
	}
	return p;
}

// This method returns the unit view vector, from the camera towards P,
// in camera space.
Point3 SContext::V(void)
{
	return viewDir;
}

// Returns the point to be shaded in camera space.
Point3 SContext::P(void)
{
	return targetPt;
}

// This returns the derivative of P, relative to the pixel.
// This gives the renderer or shader information about how fast the position
// is changing relative to the screen.
// TBD

#define DFACT .1f

Point3 SContext::DP(void)
{
	float d = (1.0f+DFACT)*(RayDiam())/(DFACT+(float)fabs(DotProd(Normal(),viewDir)));
	return Point3(d,d,d);
}

// Retrieves the point relative to the screen where the lower left
// corner is 0,0 and the upper right corner is 1,1.
void SContext::ScreenUV(class Point2 &uv,class Point2 &duv)
{
	Point2 p;

	uv.x = .5f;
	uv.y = .5f;
	duv.x = 1.0f;
	duv.y = 1.0f;
}

// Bounding box in object coords
Box3 SContext::ObjectBox(void)
{
	return boundingObj;
}

// Returns the point to be shaded relative to the object box where each
// component is in the range of -1 to +1.
Point3 SContext::PObjRelBox(void)
{
	Point3 q;
	Point3 p = PObj();
	Box3 b = ObjectBox(); 
	q.x = 2.0f*(p.x-b.pmin.x)/(b.pmax.x-b.pmin.x) - 1.0f;
	q.y = 2.0f*(p.y-b.pmin.y)/(b.pmax.y-b.pmin.y) - 1.0f;
	q.z = 2.0f*(p.z-b.pmin.z)/(b.pmax.z-b.pmin.z) - 1.0f;
	return q;
}

// Returns the derivative of PObjRelBox().
// This is the derivative of the point relative to the object box where
// each component is in the range of -1 to +1.
Point3 SContext::DPObjRelBox(void)
{
	Box3 b = ObjectBox(); 
	Point3 d = DPObj();
	d.x *= 2.0f/(b.pmax.x-b.pmin.x); 
	d.y *= 2.0f/(b.pmax.y-b.pmin.y); 
	d.z *= 2.0f/(b.pmax.z-b.pmin.z); 
	return d;
}

// Returns the point to be shaded in object coordinates.
Point3 SContext::PObj(void)
{
	return Inverse(tmAfterWSM) * P();
}

// Returns the derivative of PObj(), relative to the pixel.
// TBD
Point3 SContext::DPObj(void)
{
	Point3 d = DP();
	return VectorTransform(Inverse(tmAfterWSM),d);
}

// Returns the UVW coordinates for the point.
Point3 SContext::UVW(int chan)
{
	Point3 uvw = Point3(0.0f, 0.0f, 0.0f);
	UVVert tverts[3];

	if (mesh->mapSupport(chan)) {
		TVFace* tvf = &mesh->mapFaces(chan)[faceNum];
		tverts[0] = mesh->mapVerts(chan)[tvf->getTVert(0)];
		tverts[1] = mesh->mapVerts(chan)[tvf->getTVert(1)];
		tverts[2] = mesh->mapVerts(chan)[tvf->getTVert(2)];

		uvw = baryCoord.x*tverts[0] +
				baryCoord.y*tverts[1] +
				baryCoord.z*tverts[2];
	}

	return uvw;
}

static Point3 basic_tva[3] = { Point3(0.0,0.0,0.0),Point3(1.0,0.0,0.0),Point3(1.0,1.0,0.0)};
static Point3 basic_tvb[3] = { Point3(1.0,1.0,0.0),Point3(0.0,1.0,0.0),Point3(0.0,0.0,0.0)};
static int nextpt[3] = {1,2,0};
static int prevpt[3] = {2,0,1};

void MakeFaceUV(Face *f, UVVert *tv)
{
	int na,nhid,i;
	Point3 *basetv;
	/* make the invisible edge be 2->0 */
	nhid = 2;
	if (!(f->flags&EDGE_A))  nhid=0; 
	else if (!(f->flags&EDGE_B)) nhid = 1;
	else if (!(f->flags&EDGE_C)) nhid = 2;
	na = 2-nhid;
	basetv = (f->v[prevpt[nhid]]<f->v[nhid]) ? basic_tva : basic_tvb;
	for (i=0; i<3; i++) {
		tv[i] = basetv[na]; 
		na = nextpt[na];
	}
}

void SContext::getTVerts(int chan) 
{
	if (chan!=0&&(node->GetMtl()->Requirements(mtlNum)&MTLREQ_FACEMAP)) {
		MakeFaceUV(&mesh->faces[faceNum],tv[0]);
	}
	else {
		Mesh* m = mesh;
		if (!m->mapSupport(chan))
			return;

		UVVert* tverts = m->mapVerts(chan);
		TVFace* tvf = &m->mapFaces(chan)[faceNum];
		tv[chan][0] = tverts[tvf->t[0]];
		tv[chan][1] = tverts[tvf->t[1]];
		tv[chan][2] = tverts[tvf->t[2]];
	}
}

int SContext::NRenderElements()
{
	return 0;
}

IRenderElement* SContext::GetRenderElement(int n)
{
	return NULL;
}

void SContext::getObjVerts()
{
	// TBD
}

// Returns the UVW derivatives for the point.
Point3 SContext::DUVW(int chan)
{
	getTVerts(chan);
	calc_size_ratio();
	return 0.5f*(pabs(tv[chan][1]-tv[chan][0])+pabs(tv[chan][2]-tv[chan][0]))*ratio;
}

// This returns the bump basis vectors for UVW in camera space.
void SContext::DPdUVW(Point3 dP[3], int chan)
{
	getTVerts(chan);
	calc_size_ratio();
	Point3 bv[3];
	getObjVerts();
	ComputeBumpVectors(tv[chan], obpos, bv);
	bumpv[chan][0] = Normalize(bv[0]);
	bumpv[chan][1] = Normalize(bv[1]);
	bumpv[chan][2] = Normalize(bv[2]);
	dP[0] = bumpv[chan][0];
	dP[1] = bumpv[chan][1];
	dP[2] = bumpv[chan][2];
}

//--------------------------------------------------------------------
// Computes the average curvature per unit surface distance in the face
//--------------------------------------------------------------------
float ComputeFaceCurvature(Point3 *n, Point3 *v, Point3 bc)
{
	Point3 nc = (n[0]+n[1]+n[2])/3.0f;
	Point3 dn0 = n[0]-nc;
	Point3 dn1 = n[1]-nc;
	Point3 dn2 = n[2]-nc;
	Point3 c = (v[0] + v[1] + v[2]) /3.0f;
	Point3 v0 = v[0]-c;
	Point3 v1 = v[1]-c;
	Point3 v2 = v[2]-c;
	float d0 = DotProd(dn0,v0)/LengthSquared(v0);
	float d1 = DotProd(dn1,v1)/LengthSquared(v1);
	float d2 = DotProd(dn2,v2)/LengthSquared(v2);
	float ad0 = (float)fabs(d0);
	float ad1 = (float)fabs(d1);
	float ad2 = (float)fabs(d2);
	return (ad0>ad1)? (ad0>ad2?d0:d2): ad1>ad2?d1:d2;
}

static inline float size_meas(Point3 a, Point3 b, Point3 c)
{
	double d  = fabs(b.x-a.x);
	d += fabs(b.y-a.y);
	d += fabs(b.z-a.z);
	d += fabs(c.x-a.x);
	d += fabs(c.y-a.y);
	d += fabs(c.z-a.z);
	return float(d/6.0);
}

// This is an estimate of how fast the normal is varying.
// For example if you are doing enviornment mapping this value may be used to
// determine how big an area of the environment to sample.
// If the normal is changing very fast a large area must be sampled otherwise
// you'll get aliasing.  This is an estimate of dN/dsx, dN/dsy put into a
// single value.
// Signed curvature:
float SContext::Curve() {
	Point3 tpos[3];
	Face &f = mesh->faces[faceNum];
	tpos[0] = mesh->verts[f.v[0]];
	tpos[1] = mesh->verts[f.v[1]];
	tpos[2] = mesh->verts[f.v[2]];
	float d = ComputeFaceCurvature(vxNormals,tpos,baryCoord);
	curve = d*RayDiam();
	return backFace?-curve:curve;
}

#define SZFACT 1.5f

// Approximate how big fragment is relative to whole face.
void SContext::calc_size_ratio()
{
	Point3 dp = DP();
	Point3 cv[3];
	cv[0] = *mesh->getVertPtr((&mesh->faces[faceNum])->v[0]);
	cv[1] = *mesh->getVertPtr((&mesh->faces[faceNum])->v[1]);
	cv[2] = *mesh->getVertPtr((&mesh->faces[faceNum])->v[2]);
	float d = size_meas(cv[0], cv[1], cv[2]);
	ratio = SZFACT*(float)fabs(dp.x)/d;
}

int	SContext::InMtlEditor()
{
	return FALSE;
}

void SContext::SetView(Point3 v)
{
	viewPoint = v;
}

void SContext::SetDiffuseOnly(bool d)
	{
	bDiffuseOnly = d;
	}

/****************************************************************************
// Shadow buffer
 ***************************************************************************/

ShadowBuffer* RContext::NewShadowBuffer() const
{
	return NULL;
}

ShadowQuadTree* RContext::NewShadowQuadTree() const
{
	return NULL;
}

Color	RContext::GlobalLightLevel() const
{
	return Color(1,1,1); // TBD
}


/****************************************************************************
// Scan the scene for all lights and add them for the ShadeContext's lightTab
 ***************************************************************************/

void sceneLightEnum(INode* recvNode, INode* node, SContext* sc, MtlBaseLib* mtls, int* numLights, RadiosityEffect* radiosity, const EvalVCOptions& evalOptions)
{
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		sceneLightEnum(recvNode, node->GetChildNode(c), sc, mtls, numLights, radiosity, evalOptions);
	}

	// Get the ObjectState.
	// The ObjectState is the structure that flows up the pipeline.
	// It contains a matrix, a material index, some flags for channels,
	// and a pointer to the object in the pipeline.
	ObjectState ostate = node->EvalWorldState(0);
	if (ostate.obj==NULL) 
		return;

	// Examine the superclass ID in order to figure out what kind
	// of object we are dealing with.
	if (ostate.obj->SuperClassID() == LIGHT_CLASS_ID) {
		// Get the light object from the ObjectState
		LightObject *light = (LightObject*)ostate.obj;

		(*numLights)++;

		// Is this light turned on, and does the radiosity engine want it?
        // Note that RadiosityEffect::UseLight() needs to be called on every light
        // which is ON (GI Joe needs it to work correctly with skylight).
		if (node->Renderable()
			  && light->GetUseLight() 
            && ((radiosity == NULL) || (radiosity->UseLight(node) && !evalOptions.radiosityOnly))
        ) 
        {
			bool bUseLight = true;

			ExclList* nt = light->GetExclList(); // DS 8/31/00- changed from NameTab
			if (nt && nt->TestFlag(NT_AFFECT_ILLUM)) {
				if (light->Include()) {
					// Inclusion list
					if (nt->FindNode(recvNode) == -1) {
						bUseLight = false;
						}
					}
				else {
					if (nt->FindNode(recvNode) != -1) {
						bUseLight = false;
						}
					}
				}

			// Create a RenderLight and append it to our list of lights
			if (bUseLight) {
				LightInfo* li = new LightInfo(node, mtls);
				sc->lightTab.Append(1, &li);
				}
			}
		}
	}

void AddSceneLights(INode* node, SContext* sc, MtlBaseLib* mtls, int* numLights, RadiosityEffect* radiosity, const EvalVCOptions& evalOptions)
{
	INode* scene = GetCOREInterface()->GetRootNode();
	for (int i=0; i<scene->NumberOfChildren(); i++) {
		sceneLightEnum(node, scene->GetChildNode(i), sc, mtls, numLights, radiosity, evalOptions);
	}

    // Add radiosity if desired
    if(radiosity != NULL) {


    }
}


/****************************************************************************
// Material enumerator functions
// Before evaluating a material we need to load the maps used by the material
// and then tell the material to prepare for evaluation.
 ***************************************************************************/

class CheckFileNames: public NameEnumCallback {
	public:
		NameTab* missingMaps;
		BitmapInfo bi;
		CheckFileNames(NameTab* n);
		void RecordName(TCHAR *name);
};

//***************************************************************************
// Class to manage names of missing maps
//***************************************************************************

CheckFileNames::CheckFileNames(NameTab* n)
{
	missingMaps = n;
}

//***************************************************************************
// Add a name to the list if it's not already there
//***************************************************************************

void CheckFileNames::RecordName(TCHAR *name)
{ 
	if (name) {
		if (name[0]!=0) {
			if (missingMaps->FindName(name)<0) {
			    missingMaps->AddName(name);
			}
		}
	}
}

class MtlEnum {
	public:
		virtual int proc(MtlBase *m, int subMtlNum) = 0;
};

class MapLoadEnum:public MtlEnum {
	public:
		TimeValue t;

	   	MapLoadEnum(TimeValue time);
		virtual int proc(MtlBase *m, int subMtlNum);
};

//***************************************************************************
// Constructor of map loader
//***************************************************************************

MapLoadEnum::MapLoadEnum(TimeValue time)
{ 
	t = time; 
}

//***************************************************************************
// Map loader enum proc
//***************************************************************************

int MapLoadEnum::proc(MtlBase *m, int subMtlNum)
{
	Texmap *tm = (Texmap *)m;
	tm->LoadMapFiles(t);
	return 1;
}


int EnumMaps(MtlBase *mb, int subMtl,  MtlEnum &tenum)
{
	if (IsTex(mb)) {
		if (!tenum.proc(mb,subMtl)) {
			return 0;
		}
	}
	for (int i=0; i<mb->NumSubTexmaps(); i++) {
		Texmap *st = mb->GetSubTexmap(i); 
		if (st) {
			int subm = (mb->IsMultiMtl()&&subMtl<0)?i:subMtl;
			if (mb->SubTexmapOn(i)) {
				if (!EnumMaps(st,subm,tenum)) {
					return 0;
				}
			}
		}
	}
	if (IsMtl(mb)) {
		Mtl *m = (Mtl *)mb;
		for (i=0; i<m->NumSubMtls(); i++) {
			Mtl *sm = m->GetSubMtl(i);
			if (sm) {
				int subm = (mb->IsMultiMtl()&&subMtl<0)?i:subMtl;
				if (!EnumMaps(sm,subm,tenum)) {
					return 0;
				}
			}
		}
	}
	return 1;
}

void EnumRefs(ReferenceMaker *rm, RefEnumProc &proc)
{
	proc.proc(rm);
	for (int i=0; i<rm->NumRefs(); i++) {
		ReferenceMaker *srm = rm->GetReference(i);
		if (srm) {
			EnumRefs(srm,proc);		
		}
	}
}

//***************************************************************************
// Constructor of map enumerator
//***************************************************************************

GetMaps::GetMaps(MtlBaseLib *mbl)
{
	mlib = mbl;
}

//***************************************************************************
// Implementation of the map enumerator
//***************************************************************************

void GetMaps::proc(ReferenceMaker *rm)
{
	if (IsTex((MtlBase*)rm)) {
		mlib->AddMtl((MtlBase *)rm);
	}
}


int LoadMapFiles(INode* node, SContext* sc, MtlBaseLib& mtls, TimeValue t)
{
	NameTab mapFiles;
	CheckFileNames checkNames(&mapFiles);

	node->EnumAuxFiles(checkNames, FILE_ENUM_MISSING_ONLY | FILE_ENUM_1STSUB_MISSING);

	// Check the lights
	for (int i = 0; i < sc->lightTab.Count(); i++) {
		if (((LightInfo*)sc->lightTab[i])->light != NULL) {
			((LightInfo*)sc->lightTab[i])->light->EnumAuxFiles(checkNames, 
				FILE_ENUM_MISSING_ONLY | FILE_ENUM_1STSUB_MISSING);
		}
	}

	if (mapFiles.Count()) {
		// Error! Missing maps.
		// not sure how to handle this so we gladly continue.
			
		//if (MessageBox(hWnd, "There are missing maps.\nDo you want to render anyway?", "Warning!", MB_YESNO) != IDYES) {
		//	return 0;
		//}
	}

	// Load the maps
	MapLoadEnum mapload(t);
	for (i=0; i<mtls.Count(); i++) {
		EnumMaps(mtls[i],-1, mapload);
	}

	return 1;
}

//***************************************************************************
// This material is used when a node does not have a material assigned.
//***************************************************************************

DumMtl::DumMtl(Color c)
{ 
    // [dl | 10june2002] Make the ambient color the same as the diffuse color.
	diff = ambientColor = c; spec = Color(DUMSPEC,DUMSPEC,DUMSPEC); 
	phongexp = (float)pow(2.0, DUMSHINE*10.0);
}

void DumMtl::Update(TimeValue t, Interval& valid)
{
}

void DumMtl::Reset()
{
}

Interval DumMtl::Validity(TimeValue t)
{
	return FOREVER;
}

ParamDlg* DumMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{
	return NULL;
}

Color DumMtl::GetAmbient(int mtlNum, BOOL backFace)
{
	return diff;
}

Color DumMtl::GetDiffuse(int mtlNum, BOOL backFace)
{
	return diff;
}

Color DumMtl::GetSpecular(int mtlNum, BOOL backFace)
{
	return spec;
}

float DumMtl::GetShininess(int mtlNum, BOOL backFace)
{
	return 0.0f;
}

float DumMtl::GetShinStr(int mtlNum, BOOL backFace)
{
	return 0.0f;
}

float DumMtl::GetXParency(int mtlNum, BOOL backFace)
{
	return 0.0f;
}

void DumMtl::SetAmbient(Color c, TimeValue t)
{
}		

void DumMtl::SetDiffuse(Color c, TimeValue t)
{
}

void DumMtl::SetSpecular(Color c, TimeValue t)
{
}

void DumMtl::SetShininess(float v, TimeValue t)
{
}

Class_ID DumMtl::ClassID()
{
	return DUMMTL_CLASS_ID;
}

void DumMtl::DeleteThis()
{
	delete this;
}

RefResult DumMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message)
{
	return REF_SUCCEED;
}

//***************************************************************************
// Shade method for the dummy material
// If a node does not have a material assigned we create
// a dummy material that inherits the wireframe color of
// the node
//***************************************************************************

void DumMtl::Shade(ShadeContext& sc)
{
	Color lightCol;
	Color diffwk(0.0f,0.0f,0.0f);
	Color specwk(0.0f,0.0f,0.0f);
	Color ambwk(0.0f,0.0f,0.0f);
	Point3 N = sc.Normal();
	Point3	R = sc.ReflectVector();
	LightDesc *l;
	for (int i = 0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;
		if (!l->Illuminate(sc, N, lightCol, L, NL, diffCoef))
			continue;

		if (l->ambientOnly) {
            // [dl | 10june2002] Addition of ambient color.
			ambwk += lightCol * ambientColor;
			continue;
			}
		// diffuse
		if (l->affectDiffuse)
			diffwk += diffCoef*lightCol;
		// specular
		if (l->affectSpecular) {
			float c = DotProd(L,R);
			if (c>0.0f) {
				c = (float)pow((double)c, (double)phongexp); 
				specwk += c*lightCol*NL;   // multiply by NL to SOFTEN 
			}
		}
	}
	sc.out.t = Color(0.0f,0.0f,0.0f);
	sc.out.c = (.3f*sc.ambientLight + diffwk)*diff + specwk*spec+ambwk;		
}

void SContext::CalcShadow(MeshInstanceTab& ilist)
	{
	int i;
	for (i=0; i<allLights.Count(); i++) {
		allLights[i]->bIsObscured = FALSE;
		// Calculate the distance from the vertex to the light position
		allLights[i]->distanceToShadedPoint = Length(allLights[i]->lightDesc->LightPosition()-targetPt);
		}

	if (node->RcvShadows()) {
		for (i=0; i<ilist.Count(); i++) {
			TurnOffObscuredLights(ilist[i]);
			}
		}

	lightTab.ZeroCount();
	for (i=0; i<allLights.Count(); i++) {
		if (!allLights[i]->bIsObscured) {
			lightTab.Append(1, &(allLights[i]), 5);
			}
		}

	nLights = lightTab.Count();
	}

#define LSQ(v) (v.x*v.x + v.y*v.y + v.z*v.z)

void SContext::TurnOffObscuredLights(MeshInstance* inst)
	{
	Mesh*		mesh = inst->mesh;
	LightDesc*	ld;
	Ray			r;
	Point3		pos;
	HitInfo		hitInfo;

	if (inst->node == node)
		return;

	if (inst->node && !inst->node->CastShadows())
		return;

	hitInfo.instance = inst;

	int numLights = allLights.Count();
	for (int l=0; l<numLights; l++) {
		ld = ((LightInfo*)allLights[l])->lightDesc;

		if (allLights[l]->light) {	// CCJ 11/13/99 - This is NULL for default lights
			ExclList* nt = allLights[l]->light->GetExclList();  //DS 8/31/00 changed from NameTab
			if (nt && nt->TestFlag(NT_AFFECT_SHADOWCAST)) {
				if (allLights[l]->light->Include()) {
					// If it's not found in the inclusion list, it's out
					if (nt->FindNode(node) == -1) {
						allLights[l]->bIsObscured = FALSE;
						continue;
						}
					}
				else {
					// If it's found in the exclusion list, it's out
					if (nt->FindNode(node) != -1) {
						allLights[l]->bIsObscured = FALSE;
						continue;
					}
				}
			}
		}

		if (!allLights[l]->bIsObscured) {
			if (allLights[l]->light && allLights[l]->light->GetShadowMethod()!=LIGHTSHADOW_NONE) {

				pos = ld->LightPosition();
				r.p = pos;
				r.dir = targetPt-pos;
				Normalize(r.dir);

				// Bounding sphere test
				Point3 pc = inst->center - r.p;
				float v = DotProd(pc,r.dir);
				if ((inst->radsq - LSQ(pc) + v*v) >= 0.0f) {
					// Bounding box test
					if (rayBoundHit(&r, inst->boundingBox)) {
						if (intersectMesh(&r, hitInfo)) {
							if (Length(hitInfo.hitPos-r.p) < allLights[l]->distanceToShadedPoint) {
								allLights[l]->bIsObscured = TRUE;
							}
						}
					}
				}
			}
		}
	}
}





//***************************************************************************
// Determine if the ray hits the bounding box of the node.
// This is done for trivial hit rejection.
//***************************************************************************

BOOL rayBoundHit(Ray* ray, Box3 boundingBox)
{
	float t, tmin, tmax;
	float dir, pos;

	tmax = BIGFLOAT;
	tmin = 0.0f;

	dir = ray->dir.x;
	pos = ray->p.x;

	if (dir < 0.0f) {
		t = (boundingBox.pmin.x - pos) / dir;
		if (t < tmin)
			return FALSE;
		if (t <= tmax)
			tmax = t;
		t = (boundingBox.pmax.x - pos) / dir;
		if (t >= tmin) {
			if (t > tmax)
				return FALSE;
			tmin = t;
		}
	} else if (dir > 0.0f) {
		t = (boundingBox.pmax.x - pos) / dir;
		if (t < tmin)
			return FALSE;
		if (t <= tmax)
			tmax = t;
		t = (boundingBox.pmin.x - pos) / dir;
		if (t >= tmin) {
			if (t > tmax)
				return FALSE;
			tmin = t;
		}
	} else if (pos < boundingBox.pmin.x || pos > boundingBox.pmax.x)
		return FALSE;

	dir = ray->dir.y;
	pos = ray->p.y;

	if (dir < 0.0f) {
		t = (boundingBox.pmin.y - pos) / dir;
		if (t < tmin)
			return FALSE;
		if (t <= tmax)
			tmax = t;
		t = (boundingBox.pmax.y - pos) / dir;
		if (t >= tmin) {
			if (t > tmax)
				return FALSE;
			tmin = t;
		}
	} else if (dir > 0.0f) {
		t = (boundingBox.pmax.y - pos) / dir;
		if (t < tmin)
			return FALSE;
		if (t <= tmax)
			tmax = t;
		t = (boundingBox.pmin.y - pos) / dir;
		if (t >= tmin) {
			if (t > tmax)
				return FALSE;
			tmin = t;
		}
	} else if (pos < boundingBox.pmin.y || pos > boundingBox.pmax.y)
		return FALSE;

	dir = ray->dir.z;
	pos = ray->p.z;

	if (dir < 0.0f) {
		t = (boundingBox.pmin.z - pos) / dir;
		if (t < tmin)
			return FALSE;
		if (t <= tmax)
			tmax = t;
		t = (boundingBox.pmax.z - pos) / dir;
		if (t >= tmin) {
			if (t > tmax)
				return FALSE;
			tmin = t;
		}
	} else if (dir > 0.0f) {
		t = (boundingBox.pmax.z - pos) / dir;
		if (t < tmin)
			return FALSE;
		if (t <= tmax)
			tmax = t;
		t = (boundingBox.pmin.z - pos) / dir;
		if (t >= tmin) {
			if (t > tmax)
				return FALSE;
			tmin = t;
		}
	} else if (pos < boundingBox.pmin.z || pos > boundingBox.pmax.z)
		return FALSE;

	return TRUE;
}


//***************************************************************************
// Intersect a ray with a mesh
//***************************************************************************

// BOOL intersectMesh(Ray* ray, RenderNode* rn, int& faceNo, Point3& baryCoord, Point3& normal, Point3& hitPos)
BOOL intersectMesh(Ray* ray, HitInfo& hitInfo)
{
	Point3 tri[3];
	BOOL hit = FALSE;
	float dist = BIGFLOAT;
	Mesh* mesh = hitInfo.instance->mesh;

	// For each face in the mesh...
	for (int nf = 0; nf < mesh->numFaces; nf++) {
		Face* f = &(mesh->faces[nf]);

		/*
		// We have a pre-calculated bounding sphere for each face.
		// here we can do a trivial hit rejection to see if we can
		// discard the face.

		BoundingSphere* bs = hitInfo.instance->faceBoundSpheres[nf];
		Point3 pc = bs->bsCenter - ray->p;
		float v = DotProd(pc,ray->dir);
		if (bs->bsRadSq - LSQ(pc) + v*v < 0.0f) {
			// No point proceeding with this face...
			continue;
		}
		*/

		// Get the vertices
		if (!hitInfo.instance->negParity) {
			tri[0] = mesh->getVert(f->getVert(0));
			tri[1] = mesh->getVert(f->getVert(1));
			tri[2] = mesh->getVert(f->getVert(2));
		}
		else {
			// Scaling is negative, get the vertives
			// counter clockwise.
			tri[0] = mesh->getVert(f->getVert(2));
			tri[1] = mesh->getVert(f->getVert(1));
			tri[2] = mesh->getVert(f->getVert(0));
		}

		/* TBD: This slowed things down in some test scenes, but might work better
		// for real life scenes..
		// Do hit rejection on the triangle bouding box
		// to get a few extra triangles out of the way
		Box3 triBound;
		triBound.Init();

		triBound += tri[0];
		triBound += tri[1];
		triBound += tri[2];

		// If we don't hit the bounding box we don't need to hit test the
		// triangle
		if (!rayBoundHit(ray, triBound)) {
			continue;
		}
		*/

		// Intersect ray with triangle
		Point3 bc;
		Point3 n;
		Point3 hp;
		if (intersectTri(ray, tri, bc, n, hp)) {
			float dl = Length(hp);
			if (dl < dist) {
				hitInfo.hitPos = hp;
				dist = dl;
				hit = TRUE;
			}
		}
	}

	return hit;
}


//***************************************************************************
// Intersect a ray with a triangle.
//***************************************************************************

BOOL intersectTri(Ray* ray, Point3* tri, Point3& baryCoord,
				  Point3& normal, Point3& hitPos)
{
	Plane p;

	CompPlaneEqn(p, tri[0], tri[1], tri[2]);

	Point3 Pn = Point3(p[0], p[1], p[2]);
	Point3 Rd = ray->dir;
	Point3 R0 = ray->p;

	float Vd = Pn.x * Rd.x + Pn.y * Rd.y + Pn.z * Rd.z;
	if (Vd >= 0.0f)
		return FALSE;

	float V0 = -(Pn.x * R0.x + Pn.y * R0.y + Pn.z * R0.z + p[3]);
	float t = V0 / Vd;

	if (t < 0.0f)
		return FALSE;

	// Intersection with plane.
	Point3 Pi = Point3(R0.x+Rd.x*t, R0.y+Rd.y*t, R0.z+Rd.z*t);

	// Get the barycentric coordinates of the hitPoint.
	// If any of the components are > 1.0 the hit is outside the triangle
	baryCoord = CalcBaryCoords(tri[0], tri[1], tri[2], Pi);

	if (baryCoord.x >= 0.0f && baryCoord.x <= 1.0f) {
		if (baryCoord.y >= 0.0f && baryCoord.y <= 1.0f) {
			if (baryCoord.z >= 0.0f && baryCoord.z <= 1.0f) {
				normal = Point3(0.0f,0.0f,1.0f); // Not used!
				hitPos = Pi; // Intersection point (the point we render)
				return TRUE;
			}
		}
	}

	return FALSE;
}


//***************************************************************************
// Compute the plane equation for the three points making up the plane.
//***************************************************************************

void CompPlaneEqn(Plane plane, const Point3& p0,
				  const Point3& p1, const Point3& p2)
{
	Point3 e1 = p1-p0;
	Point3 e2 = p2-p0;
	Point3 p = CrossProd(e1,e2);
	p = Normalize(p);
	plane[0] = p.x;
	plane[1] = p.y;
	plane[2] = p.z;
	plane[3] = -DotProd(p0,p);
}

//***************************************************************************
// Determine is the node has negative scaling.
// This is used for mirrored objects for example. They have a negative scale
// so when calculating the normal we take the vertices counter clockwise.
// If we don't compensate for this the objects will be 'inverted'
//***************************************************************************

BOOL TMNegParity(Matrix3 &m)
{
    return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}


//***************************************************************************
// Calculate the determinant
// Thanks to Ruediger Hoefert, Absolute Software
//***************************************************************************

static float det2x2( float a, float b, float c, float d )
{
    float ans;
    ans = a * d - b * c;
    return ans;
}

//***************************************************************************
// 
// float = det3x3(  a1, a2, a3, b1, b2, b3, c1, c2, c3 )
//   
// calculate the determinant of a 3x3 matrix
// in the form
//
//     | a1,  b1,  c1 |
//     | a2,  b2,  c2 |
//     | a3,  b3,  c3 |
//
// Thanks to Ruediger Hoefert, Absolute Software
//***************************************************************************

static float det3x3( Point3 a,Point3 b,Point3 c )

{
   float a1, a2, a3, b1, b2, b3, c1, c2, c3;    
   float ans;
   
   a1 = a.x ; a2 = a.y ; a3 = a.z ;   
   b1 = b.x ; b2 = b.y ; b3 = b.z ;   
   c1 = c.x ; c2 = c.y ; c3 = c.z ;   

   ans = a1 * det2x2( b2, b3, c2, c3 )
       - b1 * det2x2( a2, a3, c2, c3 )
       + c1 * det2x2( a2, a3, b2, b3 );
   return ans;
}


//***************************************************************************
// Given three points in space forming a triangle (p0,p1,p2), 
// and a fourth point in the plane of that triangle, returns the
// barycentric coords of that point relative to the triangle.
// Thanks to Ruediger Hoefert, Absolute Software
//***************************************************************************

Point3 CalcBaryCoords(Point3 p0, Point3 p1, Point3 p2, Point3 p)
{ 
	Point3 tpos[3];
	Point3 cpos;
	Point3 bary;

	tpos[0] = p0;
	tpos[1] = p1;
	tpos[2] = p2;
	cpos = p;

 /*
 
 S.304 Curves+Surfaces for Computer aided design:

     u + v + w = 1

         area(p,b,c)    area(a,p,c)   area(a,b,p)
 u = -----------, v = -----------, w = -----------, 
    area(a,b,c)    area(a,b,c)   area(a,b,c)
 
   ax  bx  cx
   
 area(a,b,c) = 0.5 * ay  by  cy
   
   az  bz  cz
    
   
 */

	float area_abc, area_pbc, area_apc, area_abp; 
	
	area_abc= det3x3(tpos[0],tpos[1],tpos[2]);
	area_abc=1.0f/(area_abc == 0 ? 0.001f : area_abc);
	
	area_pbc =det3x3(cpos   ,tpos[1],tpos[2]); 
	area_apc =det3x3(tpos[0],cpos   ,tpos[2]);
	area_abp =det3x3(tpos[0],tpos[1],cpos   );
	
	bary.x = area_pbc *area_abc ;
	bary.y = area_apc *area_abc ;
	bary.z = area_abp *area_abc ;
	
	return bary;
}
