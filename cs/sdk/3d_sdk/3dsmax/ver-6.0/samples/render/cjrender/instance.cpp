//***************************************************************************
// CJRender - [instance.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Implementation of the Instance and RenderLight classes
//
//***************************************************************************


#include "maxincl.h"
#include "cjrender.h"
#include "rendutil.h"
#include "refenum.h"

Instance::Instance(INode* node, MtlBaseLib* mtls, int nodeID)
{
	pNode	= node;
	mesh	= NULL;
	flags	= 0;

	// Get material for node
	mtl = node->GetMtl();
	if (!mtl) {
		// Node has no material, create a dummy material based on wireframe color
		// This is done so the  renderer does not have to worry about nodes
		// without materials. It will, in effect, assure that every node has
		// a valid material.
		mtl = new DumMtl( (Color)(node->GetWireColor()) );
		DebugPrint("\tCreated dummy material for: %s.\n", node->GetName());
	}

	// Add material to our list
	mtls->AddMtl(mtl);

	this->nodeID = nodeID;
}

Instance::~Instance()
{
	FreeAll();

	// If this is a dummy material we need to delete it
	if (mtl->ClassID() == DUMMTL_CLASS_ID) {
		delete mtl;
		DebugPrint("\tDeleted dummy material.\n");
	}
}


int Instance::Update(TimeValue t, View& vw, CJRenderer* pRenderer)
{
	FreeAll();

	// Check visibility
	vis = pNode->GetVisibility(t);
	if (vis < 0.0f) {
		vis = 0.0f;
		SetFlag(INST_HIDE, 1);
		return 1;
	}
	if (vis > 1.0f) vis = 1.0f;
	SetFlag(INST_HIDE, 0);

	// TM's
	Interval tmValid(FOREVER);
	objToWorld = pNode->GetObjTMAfterWSM(t,&tmValid);

	// Is this node negatively scaled
	SetFlag(INST_TM_NEGPARITY, TMNegParity(objToWorld));

	// Get Object
	ObjectState os = pNode->EvalWorldState(t);
	pObject = os.obj;

	//******************************************************************
	// Mesh access.
	// In this sample renderer we retrieve the mesh for all nodes,
	// then we transform all the vertices to camera space and finally
	// we render the frame.
	// Problem: When we retrieve the mesh from instanced objects
	// we will get the same mesh for all instances.
	// As we, in this case, transform the vertices we would do multiple
	// (and invalid) transformations of the mesh for instanced objects.
	// To solve this in an easy (but expensive way) we make copies of
	// all meshes. We can get away with that, because we are
	// a sample renderer - however, you are not.
	//******************************************************************

	// Make a complete copy of the mesh...
	BOOL deleteMesh;
	mesh = new Mesh;
	Mesh* nodeMesh = ((GeomObject*)pObject)->GetRenderMesh(t, pNode, vw, deleteMesh);

	/*
	mesh->DeepCopy(nodeMesh, GEOM_CHANNEL | TOPO_CHANNEL | TEXMAP_CHANNEL |
		MTL_CHANNEL | DISP_ATTRIB_CHANNEL | TM_CHANNEL);
	*/

	*mesh = *nodeMesh;

	// If the mesh is not a part of the cache, delete it.
	if (deleteMesh) {
		delete nodeMesh;
	}

	for (int i=0; i<mesh->numFaces; i++) {
		BoundingSphere* bs = new BoundingSphere;
		faceBoundSpheres.Append(1, &bs, 25);
	}
	pRenderer->nNumFaces+=mesh->numFaces;

	return 1;
}

void Instance::UpdateViewTM(Matrix3 affineTM)
{
	objToCam = objToWorld * affineTM;
	camToObj = Inverse(objToCam);
	normalObjToCam.IdentityMatrix();

	// Calculate the inverse-transpose of objToCam for transforming normals.
	for (int it=0; it<3; it++) {
		Point4 p = Inverse(objToCam).GetColumn(it);
		normalObjToCam.SetRow(it,Point3(p[0],p[1],p[2]));
	}

	normalCamToObj = Inverse(normalObjToCam);

	CalcBoundingSphere();
}

void Instance::TransformGeometry(Matrix3 pointMat, Matrix3 vecMat)
{
	if (!mesh)
		return;

	// TBD: Do I need to transform back in reverse order?
	// TBD: Do I need to build normals?

	// Transform the mesh points to camera space
	for (int nv = 0; nv < mesh->numVerts; nv++) {
		Point3 vx = pointMat * mesh->getVert(nv);
		mesh->setVert(nv, vx);
	}

	// transform the face normals to camera space
	for (int nf = 0; nf < mesh->numFaces; nf++) {
		Point3 fn = VectorTransform(vecMat, mesh->getFaceNormal(nf));
		mesh->setFaceNormal(nf, fn);
	}
}

void Instance::CalcBoundingSphere()
{
	// Get the boundingbox first
	obBox.Init();

	Point3 vx[3];

	// Include each vertex in the bounding box
	for (int nf = 0; nf < mesh->numFaces; nf++) {
		Face* f = &(mesh->faces[nf]);
		vx[0] = mesh->getVert(f->getVert(0));
		vx[1] = mesh->getVert(f->getVert(1));
		vx[2] = mesh->getVert(f->getVert(2));

		obBox += vx[0];
		obBox += vx[1];
		obBox += vx[2];

		// Pre-calculate bounding spheres for the faces
		BoundingSphere* bs = (BoundingSphere*)faceBoundSpheres[nf];
		bs->bsCenter = objToCam * ((vx[0] + vx[1] + vx[2]) / 3.0f);
		bs->bsRadSq = 0.0f;
		Point3 d;
		float nr = 0.0f;
		for (int i=0; i<3; i++) {
			d = objToCam * vx[i] - bs->bsCenter;
			nr = DotProd(d,d);
			if (nr>bs->bsRadSq) bs->bsRadSq = nr;
		}
	}

	camBox = obBox * objToCam;

	// Now get the bounding sphere forthe object itself
	center = 0.5f*(objToCam*obBox.pmin+objToCam*obBox.pmax);
	radsq = 0.0f;
	for (int i= 0; i<mesh->numVerts; i++) {
		Point3 d = objToCam*mesh->verts[i] - center;
		float nr = DotProd(d,d);
		if (nr>radsq) radsq = nr;		
	}
}

void Instance::FreeAll()
{
	if (mesh) {
		delete mesh;
		mesh = NULL;
	}

	for (int i=0; i<faceBoundSpheres.Count(); i++) {
		delete faceBoundSpheres[i];
	}
	faceBoundSpheres.ZeroCount();
	faceBoundSpheres.Shrink();
}

RenderInstance* Instance::Next()
{
	return next;
}

Interval Instance::MeshValidity()
{
	return FOREVER;
}

int Instance::NumLights()
{
	return 0;
}

LightDesc* Instance::Light(int n)
{
	return NULL;
}

int Instance::NumShadLights()
{
	return NULL;
}

LightDesc* Instance::ShadLight(int n)
{
	return NULL;
}

INode* Instance::GetINode()
{
	return pNode;
}

Object* Instance::GetEvalObject()
{
	return pObject;
}

unsigned long Instance::MtlRequirements(int mtlNum, int faceNum)
{
	return mtl->Requirements(mtlNum);
}

Mtl* Instance::GetMtl(int faceNum)
{
	return NULL;
}

Point3 Instance::GetFaceNormal(int faceNum)
{
	return Point3(0,0,0);
}

Point3 Instance::GetFaceVertNormal(int faceNum, int vertNum)
{
	return Point3(0,0,0);
}

void Instance::GetFaceVertNormals(int faceNum, Point3 n[3])
{
}

Point3 Instance::GetCamVert(int vertNum)
{
	return Point3(0,0,0);
}


void Instance::GetObjVerts(int fnum, Point3 obp[3])
{
	Face* f = &mesh->faces[fnum];
	obp[0] = mesh->verts[f->v[0]];
	obp[1] = mesh->verts[f->v[1]];
	obp[2] = mesh->verts[f->v[2]];
}

void Instance::GetCamVerts(int fnum, Point3 cp[3])
{
	Face* f = &mesh->faces[fnum];
	cp[0] = objToCam*mesh->verts[f->v[0]];
	cp[1] = objToCam*mesh->verts[f->v[1]];
	cp[2] = objToCam*mesh->verts[f->v[2]];
}

TCHAR* Instance::GetName()
{
	return pNode->GetName();
}

int Instance::CastsShadowsFrom(const ObjLightDesc& lt)
	{
	return TRUE;
	}
