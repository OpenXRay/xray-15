//***************************************************************************
// CJRender - [raytrace.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Implementation of the raytrace core
//
//***************************************************************************

#include "maxincl.h"
#include "cjrender.h"
#include "scontext.h"
#include "rendutil.h"

typedef float Plane[4];	// A plane definition.

BOOL intersectMesh(Ray* ray, HitInfo& hitInfo);
BOOL intersectTri(Ray* ray, Point3* tri, Point3& baryCoord, Point3& normal, Point3& hitPos);
void CompPlaneEqn(Plane plane, const Point3& p0, const Point3& p1, const Point3& p2);
BOOL rayBoundHit(Ray* ray, Box3 boundingBox);

int CJRenderer::RenderImage(CJRenderParams& rp, TimeValue t, Bitmap* tobm, RendProgressCallback *prog)
{
	Point3 camPos = Point3(0,0,0);
	Point3 targPos = Point3(0,0,-1);
	Color24 black24 = { 0, 0, 0 };

	BGContext bc(&rp);
	SContext sc(this, &bc);

	for (int y = rp.nMiny; y < rp.nMaxy; y++) {
		// GBuffer support
		if (rp.gbufWriter) rp.gbufWriter->StartLine(y);

		for (int x = rp.nMinx; x < rp.nMaxx; x++) {

			int gbufIndex = y*rp.devWidth + x;

			// GBuffer support
			if (rp.gbufWriter) rp.gbufWriter->StartPixel(x);
			if (rp.gbufWriter) rp.gbufWriter->StartNextLayer();

			// Clear the G-Buffer entries
			if (rp.gbufChan[GBUF_Z]) {
				pGbufZ[gbufIndex] = -BIGFLOAT;
			}
			if (rp.gbufChan[GBUF_MTLID]) {
				pGbufMtlID[gbufIndex] = 0;
			}
			if (rp.gbufChan[GBUF_NODEID]) {
				pGbufNodeID[gbufIndex] = -1;
			}
			if (rp.gbufChan[GBUF_UV]) {
				pGbufUV[gbufIndex] = Point2(0.0f, 0.0f);
			}
			if (rp.gbufChan[GBUF_NORMAL]) {
				pGbufNormal[gbufIndex] = 0;
			}
			if (rp.gbufChan[GBUF_REALPIX]) {
				RealPixel rpix;
				rpix.r = rpix.g = rpix.b = rpix.e = 0;
				pGbufRealPix[gbufIndex] = rpix;
			}
			if (rp.gbufChan[GBUF_COVER]) {
				pGbufCov[gbufIndex] = 0;
			}

			if (rp.gbufChan[GBUF_BG]) {
				pGbufBg[gbufIndex] = black24;
			}
			if (rp.gbufChan[GBUF_NODE_RENDER_ID]) {
				pGbufNodeIndex[gbufIndex] = 0;
			}
			if (rp.gbufChan[GBUF_COLOR]) {
				pGbufColor[gbufIndex] = black24;
			}
			if (rp.gbufChan[GBUF_TRANSP]) {
				pGbufTransp[gbufIndex] = black24;
			}
			if (rp.gbufChan[GBUF_VELOC]) {
				pGbufVeloc[gbufIndex] = Point2(0.0f, 0.0f);
			}

			// Cast multiple rays for a pseudo anti-aliasing.
			// Cast one ray through the center of the pixel,
			// and one ray in each corner of the pixel.
			// Since the corner rays are shared between pixels, we effectively
			// only casts a little more than one ray per pixel extra.
			// Physically this won't give us a hight sample rate than casting two
			// rays only, but it softens the image a little.
			Color colMix;
			Color colCenter;

			Color colUpperLeft;
			Color colUpperRight;
			Color colLowerLeft;
			Color colLowerRight;
			float numCols;

			Ray ray;
			ray.p = camPos;
			sc.SetScreenPos(IPoint2(x, y));
			sc.SetCamPos(ray.p);
			bc.SetCamPos(ray.p);
			bc.SetScreenPos(x, y, rp.devWidth, rp.devHeight);

			GBufInfo gbInfo;

			// Center of pixel
			ray.dir = rp.RayDirection((float)x+0.5f, (float)y+0.5f);
			sc.SetViewDir(ray.dir);
			bc.SetViewDir(ray.dir);
			CastRay(rp, &ray, &sc, 0, colCenter, &gbInfo);
			colMix = colCenter;


			// GBuffer support
			if (rp.gbufChan[GBUF_Z]) {
				pGbufZ[y*rp.devWidth + x] = gbInfo.distance;
				}
			if (rp.gbufChan[GBUF_NORMAL]) {
				pGbufNormal[y*rp.devWidth + x] = CompressNormal(gbInfo.normal);
				}
			if (rp.gbufChan[GBUF_NODEID]) {
				pGbufNodeID[y*rp.devWidth + x] = gbInfo.instance ? gbInfo.instance->nodeID : -1;
				}
			if (rp.gbufChan[GBUF_COVER]) {
				pGbufCov[y*rp.devWidth + x] = gbInfo.foundHit ? 255 : 0;
				}

			numCols = 1.0f;

			if (rp.nAntiAliasLevel > 0) {

				// Lower right - always render this
				ray.dir = rp.RayDirection((float)x+0.75f, (float)y+0.75f);
				sc.SetViewDir(ray.dir);
				bc.SetViewDir(ray.dir);
				CastRay(rp, &ray, &sc, 0, colLowerRight, NULL);

				// Upper right
				ray.dir = rp.RayDirection((float)x+0.75f, (float)y+0.25f);
				sc.SetViewDir(ray.dir);
				bc.SetViewDir(ray.dir);
				CastRay(rp, &ray, &sc, 0, colUpperRight, NULL);

				// Lower left
				ray.dir = rp.RayDirection((float)x+0.25f, (float)y+0.75f);
				sc.SetViewDir(ray.dir);
				bc.SetViewDir(ray.dir);
				CastRay(rp, &ray, &sc, 0, colLowerLeft, NULL);


				// Upper left
				ray.dir = rp.RayDirection((float)x+0.25f, (float)y+0.25f);
				sc.SetViewDir(ray.dir);
				bc.SetViewDir(ray.dir);
				CastRay(rp, &ray, &sc, 0, colUpperLeft, NULL);

				colMix = colCenter + colLowerRight + colLowerLeft + colUpperLeft + colUpperRight;
				numCols = 5.0f;
			}

			if (rp.nAntiAliasLevel > 1) {

				// Lower right
				ray.dir = rp.RayDirection((float)x+1.0f, (float)y+1.0f);
				sc.SetViewDir(ray.dir);
				bc.SetViewDir(ray.dir);
				CastRay(rp, &ray, &sc, 0, colLowerRight, NULL);

				// Upper right
				ray.dir = rp.RayDirection((float)x+1.0f, (float)y);
				sc.SetViewDir(ray.dir);
				bc.SetViewDir(ray.dir);
				CastRay(rp, &ray, &sc, 0, colUpperRight, NULL);

				// Lower left
				ray.dir = rp.RayDirection((float)x, (float)y+1.0f);
				sc.SetViewDir(ray.dir);
				bc.SetViewDir(ray.dir);
				CastRay(rp, &ray, &sc, 0, colLowerLeft, NULL);


				// Upper left
				ray.dir = rp.RayDirection((float)x, (float)y);
				sc.SetViewDir(ray.dir);
				bc.SetViewDir(ray.dir);
				CastRay(rp, &ray, &sc, 0, colUpperLeft, NULL);

				colMix = colCenter + colLowerRight + colLowerLeft + colUpperLeft + colUpperRight;
				numCols = 9.0f;
			}

			colMix = colMix / numCols;

			BMM_Color_64 col64; // What we output to the bitmap in the end

			col64 = colTo64(colMix);	// Clamp and convert to col64

			tobm->PutPixels(x, y, 1, &col64);
		}

		// Update the output every 10 scanlines or so
		// Updating the display is slow, so keep the update rate down to a reasonable level.
		if (y%10 == 0 || y == rp.devHeight-1) {
			Rect r;
			r.top = y-10;
			r.bottom = y;
			r.left = 0;
			r.right = tobm->Width();

			tobm->ShowProgressLine(y-1);

			tobm->RefreshWindow(&r);
		}

		// Update progress bar and check for cancel
		if (prog && (prog->Progress(y, rp.devHeight-1) == RENDPROG_ABORT)) {
			tobm->ShowProgressLine(-1);	// Clear progress line
			return 0;
			break;
		}

	}

	tobm->ShowProgressLine(-1);	// Clear progress line

	return 1;
}

#define LSQ(v) (v.x*v.x + v.y*v.y + v.z*v.z)

BOOL CJRenderer::CastRay(CJRenderParams& rp, Ray* ray, SContext* sc, int depth, Color& col, GBufInfo* gbInfo)
{
	HitInfo	hitInfo;
	bool	foundHit = false;
	float	dist = BIGFLOAT;
	int		nNumNodes = instTab.Count();

	for (int n = 0; n < nNumNodes; n++) {
		HitInfo testHitInfo;

		if (!instTab[n]->mesh) {
			continue;
		}

		// Reject object based on bounding sphere.
		Point3 pc = instTab[n]->center - ray->p;
		float v = DotProd(pc,ray->dir);
		if (instTab[n]->radsq - LSQ(pc) + v*v < 0.0f) {
			continue;
		}

		// Reject objects based on bounding box
		if (!rayBoundHit(ray, instTab[n]->camBox)) {
			continue;
		}

		testHitInfo.instance = instTab[n];

		// Check intersection with ray and object
		if (intersectMesh(ray, testHitInfo)) {

			// Only use this hit if it is closer than the previously recorded hit
			float dl = Length(testHitInfo.hitPos);

			if (dl < dist) {
				hitInfo = testHitInfo;
				dist = dl;
				foundHit = true;
			}
		}
	}

	if (gbInfo) {
		gbInfo->foundHit = foundHit;
		gbInfo->distance = -dist;
		gbInfo->instance = foundHit ? hitInfo.instance : NULL;
		gbInfo->normal = foundHit ? hitInfo.normalAtHitPoint : Point3(-1,-1,-1);
		}

	if (!foundHit) {
		// Render background...
		Color bg;

		if (rp.envMap) {
			AColor abg = rp.envMap->EvalColor(*sc->bc);
			bg.r = abg.r;
			bg.g = abg.g;
			bg.b = abg.b;
		}
		else {
			if (rp.pFrp) {
				bg = rp.pFrp->background;
			}
			else {
				bg = Color(0.0f, 0.0f, 0.0f);
			}
		}

		if (rp.atmos) {
			Color xp;
			rp.atmos->Shade(*sc->bc, ray->p, ray->p * TransMatrix(-FARZ * ray->dir), bg, xp, TRUE);
		}

		col = bg;
		return FALSE;
	}


	// Shade point

	Face* f = &(hitInfo.instance->mesh->faces[hitInfo.faceNum]);
	MtlID mid = 0;
	int nNumSubs = hitInfo.instance->mtl->NumSubMtls();

	if (nNumSubs) {
		// Get sub material ID of the face.
		mid = f->getMatID();
		// the material ID of the face can be larger than the
		// total number of sub materials (because it is user
		// configurable).
		// Here I use a modulus function to bring the number
		// down to a legal value.
		mid = mid % nNumSubs;
	}

	// Shade the face that was closest to the camera
	sc->SetInstance(hitInfo.instance);
	sc->SetMtlNum(mid);
	sc->SetFaceNum(hitInfo.faceNum);
	sc->SetHitPos(hitInfo.hitPos);
	sc->SetBary(hitInfo.baryCoord);
	sc->CalcNormals();

	// Go off and do the actual shading.
	hitInfo.instance->mtl->Shade(*sc);

	// Save these for rendering atmospherics
	Point3 shadeCamPos = ray->p;
	Point3 shadeHitPos = sc->P();

	Ray reflectRay;
	Ray refractRay;

	Color thisColor = sc->out.c;
	Color colReflect(1.0f, 1.0f, 1.0f);
	Color colRefract(1.0f, 1.0f, 1.0f);

	/*
	// Total hack!
	float partRefract = hitInfo.instance->mtl->GetXParency();
	//float partRefract = Length(sc->out.t)/1.73f; // Bring down to range 0 - 1.
	float partReflect = (1.0f - partRefract) * hitInfo.instance->mtl->GetShininess();
	float partCol = 1.0f - (partReflect+partRefract);
	*/

	float partReflect = hitInfo.instance->mtl->GetShininess();
	float partCol = 1.0f - partReflect;

	if ((depth < rp.nMaxDepth) && (partReflect > 0.0f)) { // || partRefract > 0.0f)) {
		// Calculate reflection
		reflectRay.p = hitInfo.hitPos;
		reflectRay.dir = sc->ReflectVector();
		sc->SetCamPos(hitInfo.hitPos);
		sc->bc->SetCamPos(hitInfo.hitPos);
		sc->SetViewDir(reflectRay.dir);
		sc->bc->SetViewDir(reflectRay.dir);

		BOOL reflHit = CastRay(rp, &reflectRay, sc, depth+1, colReflect, NULL);
		if (!reflHit && !rp.bReflectEnv) {
			partCol = 1.0f;
			partReflect = 0.0f;
		}

		/*
		// Don't look!
		if (MaxVal(sc->out.t) > 0.0f) {
			// calculate refraction
			refractRay.p = hitInfo.hitPos;
			refractRay.dir = sc->RefractVector(sc->out.ior);
			sc->SetCamPos(hitInfo.hitPos);
			sc->bc->SetCamPos(hitInfo.hitPos);
			sc->SetViewDir(refractRay.dir);
			sc->bc->SetViewDir(refractRay.dir);
			CastRay(rp, &refractRay, sc, depth+1, colRefract);
		}
		*/
	}
	else {
		partCol = 1.0f;
		partReflect = 0.0f;
	}

	Color colorMix;

	// mix colors

	colorMix = partCol * thisColor + partReflect * colReflect; // + partRefract * colRefract;

	if (rp.atmos) {
		rp.atmos->Shade(*sc->bc, shadeCamPos, shadeHitPos, colorMix, colRefract, FALSE);
	}

	col = colorMix;
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

		// Get the vertices
		if (!hitInfo.instance->TestFlag(INST_TM_NEGPARITY)) {
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
				hitInfo.faceNum = nf;
				hitInfo.baryCoord = bc;
				hitInfo.normalAtHitPoint = n;
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


