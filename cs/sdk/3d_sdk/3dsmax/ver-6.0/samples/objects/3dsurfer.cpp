/*
	3dsurf.cpp
	By Steve Anderson
	Importing details for 3dsurfer models from 3ds classic.
*/

#include "prim.h" 
#include "splshape.h"
#include "linshape.h"
#include "iparamm.h"
#include "simpspl.h"
#include "3dsurfer.h"

/* Types of splines */
#define MSPL_LIN	0
#define MSPL_CARD	1
#define MSPL_BSPL	2

/* Macros for accessing parameter info, which is hidden in userdata: */

/* Spline macros */
#define MSPL_TYPE	vals[0]
#define MSPL_TENS	vals[1]	/* 1000x fixed-point */
#define MSPL_STEP	vals[2]
#define MSPL_CLOS	vals[3]

/* Patch macros */
#define MPAT_UNUM	vals[0]
#define MPAT_UTYPE	vals[1]
#define MPAT_UTENS	vals[2]	/* 1000x fixed-point */
#define MPAT_USTEP	vals[3]
#define MPAT_UCLOS	vals[4]
#define MPAT_VNUM	vals[5]
#define MPAT_VTYPE	vals[6]
#define MPAT_VTENS	vals[7]	/* 1000x fixed-point */
#define MPAT_VSTEP	vals[8]
#define MPAT_VCLOS	vals[9]
#define MPAT_TCAP	vals[10]
#define MPAT_TCAPH	vals[11]	/* 1000x fixed-point */
#define MPAT_BCAP	vals[12]
#define MPAT_BCAPH	vals[13]	/* 1000x fixed-point */

Point3 & InputVert (Mesh & mesh, int unum, int vnum, int u, int v)
{
	if (u<0) u=u+unum;
	if (u>unum-1) u=u%unum;
	if (v<0) v=v+vnum;
	if (v>vnum-1) v=v%vnum;
	return mesh.verts[u*vnum+v];
}

#define IN_VERT(u,v) InputVert(msh,unum,vnum,u,v)

Object *SurferPatchDataReaderCallback::ReadData (TriObject *tri, void *data, DWORD len)
{
#ifndef NO_PATCHES
	int i, vals[14];
	char *ptr;

	ptr = (char *) data;

	if (len != 107) return tri;
	memcpy (vals, ptr+50, 56);

	Mesh & msh = tri->GetMesh();
	if (msh.numVerts != MPAT_UNUM*MPAT_VNUM) return tri;

	int vrtnum, vecnum, pnum, cnum;

	PatchObject *pobj;
	pobj = new PatchObject;
	PatchMesh & pch = pobj->patch;
	PatchVert vtemp;
	vrtnum = msh.numVerts;
	vecnum = 8*vrtnum;
	pnum = vrtnum;
	if (!MPAT_UCLOS) pnum -= MPAT_VNUM;
	if (!MPAT_VCLOS) pnum -= MPAT_UNUM;
	if ((!MPAT_VCLOS) && (!MPAT_UCLOS)) pnum++;

	cnum = 0;
	if ((MPAT_VCLOS) && (!MPAT_UCLOS)) {
		cnum = MPAT_TCAP + MPAT_BCAP;
		pnum += MPAT_VNUM*cnum;
		vecnum += MPAT_VNUM*cnum*4;
	}
	if ((!MPAT_VCLOS) && (MPAT_UCLOS)) {
		cnum = MPAT_TCAP + MPAT_BCAP;
		pnum += MPAT_UNUM*cnum;
		vecnum += MPAT_UNUM*cnum*4;
	}
	vrtnum += cnum;
	pch.setNumVerts (vrtnum);
	pch.setNumVecs  (vecnum);
	pch.setNumPatches (pnum);

	int u, v, ustart, uend, vstart, vend, uoff, voff, vnum, unum, uu, vv;
	Point3 center, vup, vdown, uup, udown;

	unum = MPAT_UNUM;
	vnum = MPAT_VNUM;
	if (unum<3) MPAT_UTYPE = MSPL_LIN;
	if (vnum<3) MPAT_VTYPE = MSPL_LIN;

	float utens = MPAT_UTENS/3000.0f;
	float vtens = MPAT_VTENS/3000.0f;
	if (MPAT_UTYPE == MSPL_BSPL) utens = 1.0f/6.0f;
	if (MPAT_VTYPE == MSPL_BSPL) vtens = 1.0f/6.0f;

	pnum = vecnum = vrtnum = 0;
	int interior = unum*vnum*4;
	for (u=0; u<unum; u++) {
		ustart = !(MPAT_UCLOS || u);
		uend = !(MPAT_UCLOS || (u<unum-1));
		uoff = (!ustart) && (!uend) && (MPAT_UTYPE == MSPL_BSPL);
		for (v=0; v<vnum; v++) {
			vstart = !(MPAT_VCLOS || v);
			vend = !(MPAT_VCLOS || (v<vnum-1));
			voff = (!vstart) && (!vend) && (MPAT_VTYPE == MSPL_BSPL);
			if (voff && uoff) {
				center = IN_VERT(u,v)*4.0f;
				center += IN_VERT(u-1,v);
				center += IN_VERT(u+1,v);
				center += IN_VERT(u,v+1);
				center += IN_VERT(u,v-1);
				center *= 4.0f;
				center += IN_VERT(u-1,v-1);
				center += IN_VERT(u-1,v+1);
				center += IN_VERT(u+1,v-1);
				center += IN_VERT(u+1,v+1);
				center = center/36.0f;
			}
			if (uoff && (!voff)) {
				center = IN_VERT(u,v)*4.0f;
				center += IN_VERT(u-1,v);
				center += IN_VERT(u+1,v);
				center = center/6.0f;
			}
			if ((!uoff) && voff) {
				center = IN_VERT(u,v)*4.0f;
				center += IN_VERT(u,v-1);
				center += IN_VERT(u,v+1);
				center = center/6.0f;
			}
			if ((!uoff) && (!voff)) center = IN_VERT(u,v);
			
			pch.setVert (vrtnum++, center);
			
			switch (MPAT_UTYPE) {
			case MSPL_LIN:
				if (ustart)	udown = (IN_VERT(u,v) - IN_VERT(u+1,v))/3.0f;
				else		udown = (IN_VERT(u-1,v) - IN_VERT(u,v))/3.0f;
				if (uend)	uup   = (IN_VERT(u,v) - IN_VERT(u-1,v))/3.0f;
				else		uup   = (IN_VERT(u+1,v) - IN_VERT(u,v))/3.0f;
				break;
			case MSPL_CARD:
				if ((!ustart) && (!uend)) uup = (IN_VERT(u+1,v) - IN_VERT(u-1,v))*utens;
				if (ustart) uup = (3.0f*IN_VERT(u+1,v) - 2.0f*IN_VERT(u,v) - IN_VERT(u+2,v))*utens;
				if (uend)	uup = (2.0f*IN_VERT(u,v) + IN_VERT(u-2,v) - 3.0f*IN_VERT(u-1,v))*utens;
				udown = -uup;
				break;
			case MSPL_BSPL:
				if ((!ustart) && (!uend)) uup = (IN_VERT(u+1,v) - IN_VERT(u-1,v))*utens;
				if (ustart) uup = (IN_VERT(u+1,v) - IN_VERT(u,v))*2.0f*utens;
				if (uend)	uup = (IN_VERT(u,v) - IN_VERT(u-1,v))*2.0f*utens;
				udown = -uup;
				break;
			}
			udown += center;
			uup   += center;
			
			switch (MPAT_VTYPE) {
			case MSPL_LIN:
				if (vstart) vdown = (IN_VERT(u,v) - IN_VERT(u,v+1))/3.0f;
				else		vdown = (IN_VERT(u,v-1) - IN_VERT(u,v))/3.0f;
				if (vend)	vup   = (IN_VERT(u,v) - IN_VERT(u,v-1))/3.0f;
				else		vup   = (IN_VERT(u,v+1) - IN_VERT(u,v))/3.0f;
				break;
			case MSPL_CARD:
				if ((!vstart) && (!vend)) vup = (IN_VERT(u,v+1) - IN_VERT(u,v-1))*vtens;
				if (vstart) vup = (3.0f*IN_VERT(u,v+1) - 2.0f*IN_VERT(u,v) - IN_VERT(u,v+2))*2.0f*vtens;
				if (vend)	vup = (2.0f*IN_VERT(u,v) + IN_VERT(u,v-2) - 3.0f*IN_VERT(u,v-1))*2.0f*vtens;
				vdown = -vup;
				break;
			case MSPL_BSPL:
				if ((!vstart) && (!vend)) vup = (IN_VERT(u,v+1) - IN_VERT(u,v-1))*vtens;
				if (vstart) vup = (IN_VERT(u,v+1) - IN_VERT(u,v))*2.0f*vtens;
				if (vend)	vup = (IN_VERT(u,v) - IN_VERT(u,v-1))*2.0f*vtens;
				vdown = -vup;
				break;
			}
			vdown += center;
			vup   += center;

			pch.setVec (vecnum++, udown);
			pch.setVec (vecnum++, vdown);
			pch.setVec (vecnum++, uup);
			pch.setVec (vecnum++, vup);
			
			if ((!vend) && (!uend)) {
				uu=(u+1)%unum;
				vv=(v+1)%vnum;

				pch.patches[pnum].SetType (PATCH_QUAD);
				pch.patches[pnum].setVerts (u*vnum+v, u*vnum+vv, uu*vnum+vv, uu*vnum+v);
				pch.patches[pnum].setVecs ((u*vnum+v)*4+3, (u*vnum+vv)*4+1,
					(u*vnum+vv)*4+2, (uu*vnum+vv)*4, (uu*vnum+vv)*4+1,
					(uu*vnum+v)*4+3, (uu*vnum+v)*4, (u*vnum+v)*4+2);
				pch.patches[pnum].setInteriors (interior, interior+1, interior+2, interior+3);
				interior += 4;
				pch.patches[pnum].smGroup = 16;
				pnum++;
			}
		}
	}
	
	// Capping:
	float radius, area, veclen, dp, hfactor;
	Point3 normal, p0, newvec;
	int offset, offset2, offset3=unum*vnum*8;
	if ((cnum>0) && (MPAT_VCLOS)) {
		if (MPAT_TCAP) {
			normal = center = Point3(0.0f,0.0f,0.0f);
			offset=(unum-1)*vnum;
			for (v=0; v<vnum; v++) {
				center += pch.verts[offset+v].p;
			}
			center = center/((float)vnum);
			p0=pch.verts[offset].p;
			for (v=1; v<vnum-1; v++) {
				normal += (pch.verts[offset+v].p - p0)^(pch.verts[offset+v+1].p - p0); 
			}
			
			// normal now has the normal direction and the length of the area.
			// Area more or less equals pi r squared.
			area = Length(normal);
			normal = normal/area;
			radius = (float) sqrt(area/3.14159265);

			// Factor in dot products of upward-pointing vectors with normal.
			offset2 = (unum-1)*vnum*4;
			for (v=0,dp=0.0f; v<vnum; v++) {
				p0 = pch.vecs[offset2+v*4+2].p - pch.verts[offset+v].p;
				veclen = Length(p0);
				if (veclen<.001) continue;
				dp += DotProd (normal, p0)/veclen;
			}
			dp = dp/vnum;
			hfactor = radius*dp*MPAT_TCAPH/1000.0f;

			// Correct upward-pointing vectors to something better for capping.
			for (v=0; v<vnum; v++) {
				p0 = pch.vecs[offset2+v*4+2].p - pch.verts[offset+v].p;
				veclen = Length(p0);
				if (veclen<.001) continue;
				p0 *= (hfactor/veclen)/2.0f;
				pch.vecs[offset2+v*4+2].p = pch.verts[offset+v].p + p0;
			}

			// Produce correct cap point
			p0 = center + normal*hfactor;
			pch.setVert (vrtnum++, p0);

			// Construct tri patches between main patch and cap point.
			for (v=0; v<vnum; v++) {
				vv = (v+1)%vnum;
				newvec = (pch.verts[offset+v].p - p0)/2.0f;
				newvec = newvec - DotProd(newvec,normal)*normal;
				pch.setVec (offset3++, newvec+p0);

				pch.patches[pnum].SetType (PATCH_TRI);
				pch.patches[pnum].setVerts (offset+v, offset+vv, vrtnum-1);
				pch.patches[pnum].setVecs (offset2+v*4+3, offset2+vv*4+1,
					offset2+vv*4+2, offset3-1+(vv-v)*4, offset3-1, offset2+v*4+2);
				pch.patches[pnum].setInteriors (offset3, offset3+1, offset3+2);
				offset3 += 3;
				pch.patches[pnum].smGroup = 16;
				pnum++;
			}
		}

		if (MPAT_BCAP) {
			normal = center = Point3(0.0f,0.0f,0.0f);
			offset=0;
			for (v=0; v<vnum; v++) {
				center += pch.verts[offset+v].p;
			}
			center = center/((float)vnum);
			p0=pch.verts[offset].p;
			for (v=1; v<vnum-1; v++) {
				normal += (pch.verts[offset+v].p - p0)^(pch.verts[offset+v+1].p - p0); 
			}
			
			// normal now has the normal direction and the length of the area.
			// Area more or less equals pi r squared.
			area = Length(normal);
			normal = -normal/area;
			radius = (float) sqrt(area/3.14159265);

			// Factor in dot products of downward-pointing vectors with normal.
			offset2 = 0;
			for (v=0,dp=0.0f; v<vnum; v++) {
				p0 = pch.vecs[offset2+v*4+0].p - pch.verts[offset+v].p;
				veclen = Length(p0);
				if (veclen<.001) continue;
				dp += DotProd (normal, p0)/veclen;
			}
			dp = dp/vnum;
			hfactor = dp*radius*MPAT_BCAPH/1000.0f;

			// Correct downward-pointing vectors to something better for capping.
			for (v=0; v<vnum; v++) {
				p0 = pch.vecs[offset2+v*4+0].p - pch.verts[offset+v].p;
				veclen = Length(p0);
				if (veclen<.001) continue;
				p0 *= (hfactor/veclen)/2.0f;
				pch.vecs[offset2+v*4+0].p = pch.verts[offset+v].p + p0;
			}

			// Produce correct cap point
			p0 = center + normal*hfactor;
			pch.setVert (vrtnum++, p0);

			// Construct tri patches between main patch and cap point.
			for (v=0; v<vnum; v++) {
				vv = (v+1)%vnum;
				newvec = (pch.verts[offset+v].p - p0)/2.0f;
				newvec = newvec - DotProd(newvec,normal)*normal;
				pch.setVec (offset3++, newvec+p0);

				pch.patches[pnum].SetType (PATCH_TRI);
				pch.patches[pnum].setVerts (offset+vv, offset+v, vrtnum-1);
				pch.patches[pnum].setVecs (offset2+vv*4+1, offset2+v*4+3,
					offset2+v*4+0, offset3-1, offset3-1+(vv-v)*4, offset2+vv*4+0);
				pch.patches[pnum].setInteriors (offset3, offset3+1, offset3+2);
				offset3 += 3;
				pch.patches[pnum].smGroup = 16;
				pnum++;
			}
		}
	}

	if ((cnum>0) && (MPAT_UCLOS)) {	// Mutually exclusive with above.
		if (MPAT_TCAP) {
			normal = center = Point3(0.0f,0.0f,0.0f);
			offset = vnum-1;
			for (u=0; u<unum; u++) {
				center += pch.verts[offset+u*vnum].p;
			}
			center = center/((float)unum);
			p0=pch.verts[offset].p;
			for (u=1; u<unum-1; u++) {
				normal += (pch.verts[offset+u*vnum].p - p0)^(pch.verts[offset+(u+1)*vnum].p - p0); 
			}
			
			// normal now has the normal direction and the length of the area.
			// Area more or less equals pi r squared.
			area = Length(normal);
			normal = -normal/area;
			radius = (float) sqrt(area/3.14159265);

			// Factor in dot products of upward-pointing vectors with normal.
			offset2 = (vnum-1)*4;
			for (u=0,dp=0.0f; u<unum; u++) {
				p0 = pch.vecs[offset2+u*vnum*4+3].p - pch.verts[offset+u*vnum].p;
				veclen = Length(p0);
				if (veclen<.001) continue;
				dp += DotProd (normal, p0)/veclen;
			}
			dp = dp/unum;
			hfactor = radius*dp*MPAT_TCAPH/1000.0f;

			// Correct upward-pointing vectors to something better for capping.
			for (u=0; u<unum; u++) {
				p0 = pch.vecs[offset2+u*vnum*4+3].p - pch.verts[offset+u*vnum].p;
				veclen = Length(p0);
				if (veclen<.001) continue;
				p0 *= (hfactor/veclen)/2.0f;
				pch.vecs[offset2+u*vnum*4+3].p = pch.verts[offset+u*vnum].p + p0;
			}

			// Produce correct cap point
			p0 = center + normal*hfactor;
			pch.setVert (vrtnum++, p0);

			// Construct tri patches between main patch and cap point.
			for (u=0; u<unum; u++) {
				uu = (u+1)%unum;
				newvec = (pch.verts[offset+u*vnum].p - p0)/2.0f;
				newvec = newvec - DotProd(newvec,normal)*normal;
				pch.setVec (offset3++, newvec+p0);

				pch.patches[pnum].SetType (PATCH_TRI);
				pch.patches[pnum].setVerts (offset+uu*vnum, offset+u*vnum, vrtnum-1);
				pch.patches[pnum].setVecs (offset2+uu*vnum*4+0, offset2+u*vnum*4+2,
					offset2+u*vnum*4+3, offset3-1, offset3-1+(uu-u)*4, offset2+uu*vnum*4+3);
				pch.patches[pnum].setInteriors (offset3, offset3+1, offset3+2);
				offset3 += 3;
				pch.patches[pnum].smGroup = 16;
				pnum++;
			}
		}

		if (MPAT_BCAP) {
			normal = center = Point3(0.0f,0.0f,0.0f);
			for (u=0; u<unum; u++) {
				center += pch.verts[u*vnum].p;
			}
			center = center/((float)unum);
			p0=pch.verts[0].p;
			for (u=1; u<unum-1; u++) {
				normal += (pch.verts[u*vnum].p - p0)^(pch.verts[(u+1)*vnum].p - p0); 
			}
			
			// normal now has the normal direction and the length of the area.
			// Area more or less equals pi r squared.
			area = Length(normal);
			normal = normal/area;
			radius = (float) sqrt(area/3.14159265);

			// Factor in dot products of downward-pointing vectors with normal.
			for (u=0,dp=0.0f; u<unum; u++) {
				p0 = pch.vecs[u*vnum*4+1].p - pch.verts[u*vnum].p;
				veclen = Length(p0);
				if (veclen<.001) continue;
				dp += DotProd (normal, p0)/veclen;
			}
			dp = dp/unum;
			hfactor = dp*radius*MPAT_BCAPH/1000.0f;

			// Correct downward-pointing vectors to something better for capping.
			for (u=0; u<unum; u++) {
				p0 = pch.vecs[u*vnum*4+1].p - pch.verts[u*vnum].p;
				veclen = Length(p0);
				if (veclen<.001) continue;
				p0 *= (hfactor/veclen)/2.0f;
				pch.vecs[u*vnum*4+1].p = pch.verts[u*vnum].p + p0;
			}

			// Produce correct cap point
			p0 = center + normal*hfactor;
			pch.setVert (vrtnum++, p0);

			// Construct tri patches between main patch and cap point.
			for (u=0; u<unum; u++) {
				uu = (u+1)%unum;
				newvec = (pch.verts[u*vnum].p - p0)/2.0f;
				newvec = newvec - DotProd(newvec,normal)*normal;
				pch.setVec (offset3++, newvec+p0);

				pch.patches[pnum].SetType (PATCH_TRI);
				pch.patches[pnum].setVerts (u*vnum, uu*vnum, vrtnum-1);
				pch.patches[pnum].setVecs (u*vnum*4+2, uu*vnum*4+0,
					uu*vnum*4+1, offset3-1+(uu-u)*4, offset3-1, u*vnum*4+1);
				pch.patches[pnum].setInteriors (offset3, offset3+1, offset3+2);
				offset3 += 3;
				pch.patches[pnum].smGroup = 16;
				pnum++;
			}
		}
	}

	for (i=0; i<pnum; i++) pch.patches[i].computeInteriors (&pch);
	pch.SetMeshSteps ((MPAT_USTEP+MPAT_VSTEP)/2-1);
	pch.buildLinkages ();
	pch.InvalidateGeomCache ();
	
	return pobj;
#else
	return NULL;
#endif // NO_PATCHES
}

Object *SurferSplineDataReaderCallback::ReadData (TriObject *tri, void *data, DWORD len)
{
	int i, vals[4];
	char *ptr;

	ptr = (char *) data;

	if (len != 69) return tri;
	memcpy (vals, ptr+52, 16);

	Mesh & mesh=tri->GetMesh();
	if (mesh.numVerts < 2) return tri;

	SplineShape *oshape;
	oshape = new SplineShape;
	BezierShape & shp = oshape->shape;
	shp.NewShape();
	shp.steps = MSPL_STEP-1;
	Spline3D *spl = shp.NewSpline();
	Point3 p, in, out, *pt=mesh.verts;

	int up, dn, start, end, vnum = mesh.numVerts;
	if (vnum<3) MSPL_TYPE = MSPL_LIN;
	float tens = MSPL_TENS/3000.0f;
	if (MSPL_TYPE == MSPL_BSPL) tens = 1.0f/6.0f;
	if (MSPL_TYPE == MSPL_LIN)  tens = 1.0f/3.0f;

	for (i=0; i<vnum; i++) {
		up = (i+1)%vnum;
		dn = (i+vnum-1)%vnum;
		start = ((!MSPL_CLOS) && (!i));
		end   = ((!MSPL_CLOS) && (i==vnum-1));
		if ((MSPL_TYPE != MSPL_BSPL) || start || end) {	p = pt[i]; }
		else { p = (pt[i]*4.0f + pt[up] + pt[dn])/6.0f; }

		switch (MSPL_TYPE) {
		case MSPL_CARD:
			if ((!start) && (!end)) out = (pt[up] - pt[dn])*tens;
			if (start)	out = (3.0f*pt[up]-2.0f*pt[i]-pt[i+2])*tens;
			if (end)	out = (2.0f*pt[i]+pt[i-2]-3.0f*pt[dn])*tens;
			in = -out;
			spl->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p+in,p+out));
			break;
		case MSPL_BSPL:
			if ((!start) && (!end)) out = (pt[up] - pt[dn])*tens;
			if (start)	out = (pt[up]-pt[i])*tens*2.0f;
			if (end)	out = (pt[i]-pt[dn])*tens*2.0f;
			in = -out;
			spl->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p+in,p+out));
			break;
		case MSPL_LIN:
			if (start)	in = (pt[i]-pt[up])*tens;
			else		in = (pt[dn]-pt[i])*tens;
			if (end)	out= (pt[i]-pt[dn])*tens;
			else		out= (pt[up]-pt[i])*tens;
			spl->AddKnot(SplineKnot(KTYPE_BEZIER_CORNER,LTYPE_CURVE,p,p+in,p+out));
		}
	}
	if (MSPL_CLOS)	spl->SetClosed();
	else			spl->SetOpen();
	spl->ComputeBezPoints ();
	shp.UpdateSels();
	shp.InvalidateGeomCache();
	return oshape;
}
