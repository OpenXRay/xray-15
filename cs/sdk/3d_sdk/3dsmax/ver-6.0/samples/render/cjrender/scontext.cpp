//***************************************************************************
// CJRender - [scontext.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Implementation of the ShadeContext for materials and backgrounds.
//
//***************************************************************************

#include "maxincl.h"
#include "cjrender.h"
#include "scontext.h"
#include "rendutil.h"


/****************************************************************************
 ShadeContext
 In order to evaluate a material in MAX, you need first of all to derive
 an object from the ShadeContext class.
 Once the ShadeContext is in place, it will provide the shader (the material)
 with enough information to evaluate itself at a specific point in space.
 The ShadeContext provides, among other things, the following information
 to the shader:
 * The current time
 * A Pointer to the node
 * Surface normal at the rendered point
 * Barycentric coordinates of the face at the rendered point
 * The number of the face currently rendered
 * Information about lights
 * Lots of other things...

 ***************************************************************************/

// Enable this definition if you want debug output from ShadeContext
// Note: The status output will come for every pixel and it will be
// *very* slow - even when not ran through the debugger.

//#define SCONTEXT_DEBUG 1

#ifdef SCONTEXT_DEBUG
void SCDebugPrint(char* msg)
{
    DebugPrint(msg);
}
#else 
#define SCDebugPrint(msg)
#endif

static inline Point3 pabs(Point3 p) { return Point3(fabs(p.x),fabs(p.y),fabs(p.z)); }

SContext::SContext(CJRenderer* r, BGContext* bgc)
{
	SCDebugPrint("SContext::SContext\n");

	renderer = r;
	globContext = &r->rendParams;

	bc = bgc;
	pInst = NULL;
	cameraPos = Point3(0.0f, 0.0f, 0.0f);
	mode = SCMODE_NORMAL;
	doMaps = TRUE;
	filterMaps = TRUE;
	shadow = TRUE;
	ambientLight = renderer->rendParams.pFrp->ambient;
	backFace = FALSE;

	nLights = renderer->lightTab.Count();
}

// When the mesh and face number is specified we need to calculate 
// and store the vertex normals
void SContext::CalcNormals()
{
	SCDebugPrint("SContext::CalcNormals\n");

	RVertex* rv[3];
	Face* f = &pInst->mesh->faces[faceNum];
	DWORD smGroup = f->smGroup;
	int numNormals;
	int vxNum;

	// Get the vertex normals
	for (int i = 0, cc = 2; i < 3; i++, cc--) {

		// We need to get the vertices in counter clockwise order
		// if the object has negative scaling.
		if (!pInst->TestFlag(INST_TM_NEGPARITY)) {
			vxNum = i;
		}
		else {
			vxNum = cc;
		}


		rv[i] = pInst->mesh->getRVertPtr(f->getVert(vxNum));

		// Is normal specified
		// SPCIFIED is not currently used, but may be used in future versions.
		if (rv[i]->rFlags & SPECIFIED_NORMAL) {
			vxNormals[i] = VectorTransform(pInst->normalObjToCam, rv[i]->rn.getNormal());
		}
		// If normal is not specified it's only available if the face belongs
		// to a smoothing group
		else if ((numNormals = rv[i]->rFlags & NORCT_MASK) && smGroup) {
			// If there is only one vertex is found in the rn member.
			if (numNormals == 1) {
				vxNormals[i] = VectorTransform(pInst->normalObjToCam, rv[i]->rn.getNormal());
			}
			else {
				// If two or more vertices are there you need to step through them
				// and find the vertex with the same smoothing group as the current face.
				// You will find multiple normals in the ern member.
				for (int j = 0; j < numNormals; j++) {
					if (rv[i]->ern[j].getSmGroup() & smGroup) {
						vxNormals[i] = VectorTransform(pInst->normalObjToCam, rv[i]->ern[j].getNormal());
					}
				}
			}
		}
		else {
			vxNormals[i] = pInst->mesh->getFaceNormal(faceNum);
		}
	}
	vxNormals[0] = Normalize(vxNormals[0]);
	vxNormals[1] = Normalize(vxNormals[1]);
	vxNormals[2] = Normalize(vxNormals[2]);
}

void SContext::SetBary(Point3 bary)
{
	SCDebugPrint("SContext::SetBary\n");

	baryCoord = bary;
}

int SContext::ProjType()
{
	SCDebugPrint("SContext::ProjType\n");

	return renderer->rendParams.projType;
}

void SContext::SetInstance(Instance* instance)
{
	SCDebugPrint("SContext::SetInstance\n");
	pInst = instance;
	matreq = instance->mtl->Requirements(0);
}

void SContext::SetFaceNum(int f)
{
	SCDebugPrint("SContext::SetFaceNum\n");
	faceNum = f;
}

void SContext::SetScreenPos(IPoint2 pt)
{
	SCDebugPrint("SContext::SetScreenPos\n");
	screenPos = pt;
}

void SContext::SetMtlNum(int mNum)
{
	SCDebugPrint("SContext::SetMtlNum\n");

	mtlNum = mNum;
	matreq = pInst->mtl->Requirements(mNum);
}

void SContext::SetHitPos(Point3 p)
{
	SCDebugPrint("SContext::SetHitPos\n");
	hitPos = p;
}

void SContext::SetViewDir(Point3 v)
{
	SCDebugPrint("SContext::SetViewDir\n");
	viewDir = v;
}


// Return current time
TimeValue SContext::CurTime()
{
	SCDebugPrint("SContext::CurTime\n");
	return renderer->rendParams.time;
}

// Return NodeID.
// This is just a counter with a unique value for each node.
int SContext::NodeID()
{
	SCDebugPrint("SContext::NodeID\n");
	return pInst->nodeID;
}

// The INode we are currently rendering
INode* SContext::Node()
{
	SCDebugPrint("SContext::Node\n");
	return pInst->pNode;
}

// The barycentric coordinates of the point relative to the face
Point3 SContext::BarycentricCoords()
{
	SCDebugPrint("SContext::BarycentricCoords\n");
	return baryCoord;
}

// The face number
int SContext::FaceNumber()
{
	SCDebugPrint("SContext::FaceNumber\n");
	return faceNum;
}


// Interpolated normal
Point3 SContext::Normal()
{
	// The face normals are already in camera space
	SCDebugPrint("SContext::Normal\n");

	return Normalize(baryCoord.x*vxNormals[0] + baryCoord.y*vxNormals[1] + baryCoord.z*vxNormals[2]);
}

// Geometric normal (face normal)
Point3 SContext::GNormal(void)
{
	// The face normals are already in camera space
	SCDebugPrint("SContext::GNormal\n");
	return pInst->mesh->getFaceNormal(faceNum);
}

// Return a Light descriptor
LightDesc *SContext::Light(int lightNum)
{
	RenderLight* rLight = (RenderLight*)renderer->lightTab[lightNum];
	SCDebugPrint("SContext::Light\n");
	return rLight->pDesc;
}

// Return reflection vector at this point
Point3 SContext::ReflectVector(void)
{
	SCDebugPrint("SContext::ReflectVector\n");
	Point3 N = Normal();
	float VN = -DotProd(viewDir,N);
    return Normalize(2.0f*VN*N + viewDir);
}

// Foley & vanDam: Computer Graphics: Principles and Practice, 
//     2nd Ed. pp 756ff.
Point3 SContext::RefractVector(float ior)
{
	SCDebugPrint("SContext::RefractVector\n");
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

// We're in camera space, so this is always 0,0,0
Point3 SContext::CamPos(void)
{
	SCDebugPrint("SContext::CamPos\n");
	return cameraPos;
}

// Screen coordinate beeing rendered
IPoint2 SContext::ScreenCoord(void)
{
	SCDebugPrint("SContext::ScreenPos\n");
	return screenPos;
}

// Background color
void SContext::GetBGColor(class Color &bgCol,class Color &transp,int fogBG)
{
	SCDebugPrint("SContext::GetBGColor\n");

	if (renderer->rendParams.envMap) {
		AColor abg = renderer->rendParams.envMap->EvalColor(*bc);
		bgCol.r = abg.r;
		bgCol.g = abg.g;
		bgCol.b = abg.b;
	}
	else {
		bgCol = renderer->rendParams.pFrp->background;
	}

	transp = Color(0.0f, 0.0f, 0.0f);	// TBD
}

// Transforms the specified point from internal camera space to the specified space.
Point3 SContext::PointTo(const class Point3 &p, RefFrame ito)
{
	Point3 pt;

	SCDebugPrint("SContext::PointTo\n");
	Matrix3 camToObj = Inverse(pInst->objToWorld * renderer->view.affineTM);

	switch (ito) {
		case REF_WORLD:  pt = renderer->rendParams.camToWorld*p; break;
		case REF_OBJECT: pt = camToObj * p; break;
		default: pt = p; break;
	}

	return pt;
}

// Transforms the specified point from the specified coordinate system
// to internal camera space.
Point3 SContext::PointFrom(const class Point3 &p, RefFrame ito)
{
	Point3 pt;
	Matrix3 objToCam = pInst->objToWorld * renderer->view.affineTM;
	SCDebugPrint("SContext::PointFrom\n");

	switch (ito) {
		case REF_WORLD:  pt = renderer->rendParams.worldToCam*p; break;
		case REF_OBJECT: pt = objToCam * p; break;
		default: pt = p; break;
	}

	return pt;
}

// Transform the vector from internal camera space to the specified space.
Point3 SContext::VectorTo(const class Point3 &p, RefFrame ito)
{
	Point3 pt;
	Matrix3 camToObj = Inverse(pInst->objToWorld * renderer->view.affineTM);
	SCDebugPrint("SContext::VectorTo\n");

	switch (ito) {
		case REF_WORLD:  pt = VectorTransform(renderer->rendParams.camToWorld,p); break;
		case REF_OBJECT: pt = VectorTransform(camToObj,p); break;
		default: pt = p; break;
	}

	return pt;
}

// Transform the vector from the specified space to internal camera space.
Point3 SContext::VectorFrom(const class Point3 &p, RefFrame ito)
{
	Point3 pt;
	Matrix3 objToCam = pInst->objToWorld * renderer->view.affineTM;

	SCDebugPrint("SContext::VectorFrom\n");
	switch (ito) {
		case REF_WORLD:  pt = VectorTransform(renderer->rendParams.worldToCam,p); break;
		case REF_OBJECT: pt = VectorTransform(objToCam,p); break;
		default: pt = p;
	}

	return pt;
}

// This method returns the unit view vector, from the camera towards P,
// in camera space.
Point3 SContext::V(void)
{
	SCDebugPrint("SContext::V\n");
	return viewDir;
}

// Returns the point to be shaded in camera space.
Point3 SContext::P(void)
{
	SCDebugPrint("SContext::P\n");
	return hitPos;
}

// This returns the derivative of P, relative to the pixel.
// This gives the renderer or shader information about how fast the position
// is changing relative to the screen.

#define DFACT .1f

Point3 SContext::DP(void)
{
	SCDebugPrint("SContext::DP\n");

	float d = (1.0f+DFACT)*0.1f/(DFACT+(float)fabs(DotProd(Normal(),viewDir)));
	dp = Point3(d,d,d);
	return dp;
}

// Retrieves the point relative to the screen where the lower left
// corner is 0,0 and the upper right corner is 1,1.
void SContext::ScreenUV(class Point2 &uv,class Point2 &duv)
{
	Point2 p;

	SCDebugPrint("SContext::ScreenUV\n");

	p.x = .5f*(((float)screenPos.x-renderer->rendParams.xc+0.5f) / 
			renderer->rendParams.xc+1.0f);
	p.y = .5f*(-((float)screenPos.y)/renderer->rendParams.yc+1.0f);
	uv = p;
	duv = renderer->rendParams.scrDUV;
}

// Bounding box in object coords
Box3 SContext::ObjectBox(void)
{
	SCDebugPrint("SContext::ObjectBox\n");
	return pInst->obBox;
}

// Returns the point to be shaded relative to the object box where each
// component is in the range of -1 to +1.
Point3 SContext::PObjRelBox(void)
{
	SCDebugPrint("SContext::PObjRelBox\n");

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
	SCDebugPrint("SContext::DPObjRelBox\n");
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
	SCDebugPrint("SContext::PObj\n");
	Matrix3 camToObj = Inverse(pInst->objToWorld * renderer->view.affineTM);
	return camToObj * P();
}


// Returns the derivative of PObj(), relative to the pixel.
Point3 SContext::DPObj(void)
{
	SCDebugPrint("SContext::DPObj\n");

	Point3 d = DP();
	dobpos = VectorTransform(pInst->camToObj,d);

	return dobpos;
}

void SContext::getTVerts(int chan)
{
	if (chan!=0&&(matreq&MTLREQ_FACEMAP)) {
		MakeFaceUV(&pInst->mesh->faces[faceNum],tv[0]);
		}
	else {
		Mesh* m = pInst->mesh;
		if (!m->mapSupport(chan))
			return;

		UVVert* tverts = m->mapVerts(chan);
		TVFace* tvf = &m->mapFaces(chan)[faceNum];

		tv[chan][0] = tverts[tvf->t[0]];
		tv[chan][1] = tverts[tvf->t[1]];
		tv[chan][2] = tverts[tvf->t[2]];
		}
	}

#define SZFACT 1.5f

// Approximate how big fragment is relative to whole face.
void SContext::calc_size_ratio()
{
	Point3 dp = DP();
	Point3 cv[3];
	pInst->GetCamVerts(faceNum,cv);
	float d = size_meas(cv[0], cv[1], cv[2]);
	ratio = SZFACT*(float)fabs(dp.x)/d;  // SZFACT is just a hack to adjust the blur
}

Point3 SContext::UVW(int chan)
{ 
	SCDebugPrint("SContext::UVW\n");

	getTVerts(chan);
	uvw[chan] = baryCoord.x*tv[chan][0] + baryCoord.y*tv[chan][1] + baryCoord.z*tv[chan][2];
	return uvw[chan];
}	

// Returns the UVW derivatives for the point.
Point3 SContext::DUVW(int chan)
{
	SCDebugPrint("SContext::DUVW\n");

	getTVerts(chan);
	calc_size_ratio();
	duvw[chan] = 0.5f*(pabs(tv[chan][1]-tv[chan][0])+pabs(tv[chan][2]-tv[chan][0]))*ratio;
	return duvw[chan];
}

// This returns the bump basis vectors for UVW in camera space.
void SContext::DPdUVW(Point3 dP[3], int chan)
{
	SCDebugPrint("SContext::DPdUVW\n");

	getTVerts(chan);
	calc_size_ratio();
	Point3 bv[3];

	pInst->GetObjVerts(faceNum, obpos);

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

// This is an estimate of how fast the normal is varying.
// For example if you are doing enviornment mapping this value may be used to
// determine how big an area of the environment to sample.
// If the normal is changing very fast a large area must be sampled otherwise
// you'll get aliasing.  This is an estimate of dN/dsx, dN/dsy put into a
// single value.
float SContext::Curve()
{
	SCDebugPrint("SContext::Curve\n");

	Point3 tpos[3];
	Face &f = pInst->mesh->faces[faceNum];
	tpos[0] = pInst->mesh->verts[f.v[0]];
	tpos[1] = pInst->mesh->verts[f.v[1]];
	tpos[2] = pInst->mesh->verts[f.v[2]];
	float d = ComputeFaceCurvature(vxNormals,tpos,baryCoord);
	curve = d*RayDiam();
	return backFace?-curve:curve;
}




//***************************************************************************
// Background Context
// The is the ShadeContext we use for rendering environments
//***************************************************************************

BGContext::BGContext(CJRenderParams* rpar) { 
	//rp = rpar; (now in globContext)
	globContext = rpar;
	nLights = 0;
	doMaps = TRUE;
	filterMaps  = TRUE;
	shadow = FALSE; 
	cpos = BGGetPoint();
	viewDir = globContext->projType ?  Point3(0.0f,0.0f,-1.0f) : Normalize(cpos);
	backFace = FALSE;
	mtlNum  =0;
	scrDUV = rpar->scrDUV;
} 

void BGContext::SetScreenPos(int x, int y, int width, int height)
{
	SCDebugPrint("BGContext::SetScreenPos\n");
	iscr = IPoint2(x,y);
	scrPos.x = float(x)-globContext->xc;
	scrPos.y = float(y)-globContext->yc;

	cpos = BGGetPoint();
	viewDir = globContext->projType ?  Point3(0.0f,0.0f,-1.0f) : Normalize(cpos);
}

//-------------------------------------------------------------------
// BGGetPoint: returns a point along the current view ray at
// a very large distance from the camera, for applying fog to background.
Point3 BGContext::BGGetPoint(float z) {
	SCDebugPrint("BGContext::BGGetPoint\n");
	
	Point3 p;
	if (globContext->projType) {
		p.x = scrPos.x/globContext->xscale;
		p.y = scrPos.y/globContext->yscale;
		p.z = z;
	}
	else  {
		p.x = scrPos.x*z/globContext->xscale;
		p.y = scrPos.y*z/globContext->yscale;
		p.z = z;
	}
	return p;
}


void BGContext::ScreenUV(Point2& uv, Point2 &duv) {
	SCDebugPrint("BGContext::ScreenUV\n");
	Point2 p;
	p.x = .5f*(scrPos.x/globContext->xc + 1.0f);
	p.y = .5f*(-scrPos.y/globContext->yc + 1.0f);
	uv = p;
	duv = scrDUV;
}


Point3 BGContext::PointTo(const Point3& p, RefFrame ito) {
	SCDebugPrint("BGContext::PointTo\n");

	switch (ito) {
		case REF_WORLD:  return globContext->camToWorld*p;
		case REF_OBJECT: return p;
		default: return p;
	}		
}

Point3 BGContext::PointFrom(const Point3& p, RefFrame ifrom){ 
	SCDebugPrint("BGContext::PointFrom\n");

	switch (ifrom) {
		case REF_WORLD:  return globContext->worldToCam*p;
		case REF_OBJECT: return p;
		default: return p;
	}		
} 

Point3 BGContext::VectorTo(const Point3& p, RefFrame ito) {
	SCDebugPrint("BGContext::VectorTo\n");
	switch (ito) {
		case REF_WORLD:  return VectorTransform(globContext->camToWorld,p);
		case REF_OBJECT: return p;
		default: return p;
	}		
}

Point3 BGContext::VectorFrom(const Point3& p, RefFrame ifrom){ 
	SCDebugPrint("BGContext::VectorFrom\n");
	switch (ifrom) {
		case REF_WORLD:  return VectorTransform(globContext->worldToCam,p);
		case REF_OBJECT: return p;
		default: return p;
	}		
} 

//***************************************************************************
// RenderMapsContext
// The is the context needed to render AutoReflect, AutoMirror and RayTrace maps
//***************************************************************************

INode *CJRenderMapsContext::GetNode()
{
	return inst->pNode;
}

int CJRenderMapsContext::NodeRenderID()
{
	return inst->nodeID;
}

void CJRenderMapsContext::GetCurrentViewParams(ViewParams &vp)
{
	vp = cjr->view;
}

void CJRenderMapsContext::GetSubRendParams(SubRendParams &srp)
{
	srp.rendType	= cjr->rendParams.rendType;
	srp.fieldRender	= cjr->rendParams.bRenderFields;
	srp.evenLines	= 0;
	srp.doingMirror	= 0;
	srp.doEnvMap	= 0;
	srp.devWidth	= cjr->rendParams.devWidth;
	srp.devHeight	= cjr->rendParams.devHeight;
	srp.devAspect	= cjr->rendParams.devAspect;
	srp.xorg		= 0;
	srp.yorg		= 0;
	srp.xmin		= cjr->rendParams.nMinx;
	srp.xmax		= cjr->rendParams.nMaxx;
	srp.ymin		= cjr->rendParams.nMiny;
	srp.ymax		= cjr->rendParams.nMaxy;
}

int CJRenderMapsContext::SubMtlIndex()
{
	return subMtl;
}

void CJRenderMapsContext::SetSubMtlIndex(int mindex)
{
	subMtl = mindex;
}

void CJRenderMapsContext::FindMtlPlane(float pl[4])
{
}

void CJRenderMapsContext::FindMtlScreenBox(Rect &sbox, Matrix3* viewTM,int mtlIndex)
{
}

Box3 CJRenderMapsContext::CameraSpaceBoundingBox()
{
	return inst->camBox;
}

Box3 CJRenderMapsContext::ObjectSpaceBoundingBox()
{
	return Box3();// TBD
}

Matrix3 CJRenderMapsContext::ObjectToWorldTM()
{
	return inst->objToWorld;
}

RenderGlobalContext *CJRenderMapsContext::GetGlobalContext()
{
	return &cjr->rendParams;
}

int CJRenderMapsContext::Render(Bitmap *bm, ViewParams &vp, SubRendParams &srp, Point4 *clipPlanes, int nClipPlanes)
{
	CJRenderParams cjrp;
	// Setup ViewParams:
	cjrp.devWidth = srp.devWidth;
	cjrp.devHeight = srp.devHeight;
	cjrp.devAspect = srp.devAspect;
	cjrp.projType = cjr->rendParams.projType;

	cjrp.ComputeViewParams(vp);

	cjrp.nMinx = srp.xmin;
	cjrp.nMiny = srp.ymin;
	cjrp.nMaxx = srp.xmax;
	cjrp.nMaxy = srp.ymax;

	cjrp.renderer = cjr;



	return cjr->RenderImage(cjrp, cjr->rendParams.time, bm, NULL);
}


