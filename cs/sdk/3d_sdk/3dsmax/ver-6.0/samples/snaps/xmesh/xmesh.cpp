/*****************************************************************************
 *<
	FILE: xmesh.cpp

	DESCRIPTION:  Sample Implementation of an Osnap for geomobjects

	CREATED BY: John Hutchinson		

	HISTORY: created 12/12/96

 *>	Copyright (c) 1994, All Rights Reserved.
 *****************************************************************************/
// The main MAX include file.
#include "max.h"

//master include file for osnaps
#include "osnapapi.h"

#include "mesh.h"
#include "meshacc.h"
#include "polyshp.h"

#include "data.h"
#include "resource.h"

#include "dummy.h"

#include <assert.h>

#define SUBCOUNT 6 // the number of snaps this has
#define NOSUB -1
#define VERT_SUB 0
#define END_SUB 1
#define EDGE_SUB 2
#define MID_SUB 3
#define FACE_SUB 4
#define C_FACE_SUB 5

#define EPSILON 0.0001f

#define BIT1	1
#define BIT2	2

HINSTANCE hInstance;

// Special structure for recording edge snap

typedef struct {
	Point3 from;
	Point3 to;
	float distance;
	} BestEdge;


TCHAR *GetString(int id);

#define XMESH_SNAP_CLASS_ID Class_ID(0x52256183, 0x106b5fda)

class XmeshSnap : public Osnap {
private:
	OsnapMarker markerdata[SUBCOUNT];
	TSTR name[SUBCOUNT];
	HBITMAP tools;
	HBITMAP masks;
	HitRegion hr;

public:

	XmeshSnap();//constructor
	virtual ~XmeshSnap();

	virtual int numsubs(){return SUBCOUNT;} //the number of subsnaps this guy has
	virtual TSTR *snapname(int index); // the snap’s name to be displayed in the UI
	virtual boolean ValidInput(SClass_ID scid, Class_ID cid);// the type of object that it recognizes SPHERE_CLASS_ID 
	Class_ID ClassID() { return XMESH_SNAP_CLASS_ID; }

	virtual OsnapMarker *GetMarker(int index){return &(markerdata[index]);} // single object might contain subsnaps
	virtual HBITMAP getTools(){return tools;} 
	virtual HBITMAP getMasks(){return masks;} 
	virtual WORD AccelKey(int index);
	virtual void Snap(Object* pobj, IPoint2 *p, TimeValue t);
	virtual Point3 ReEvaluate(TimeValue t, OsnapHit *hit, Object *pobj);

	void SnapToEdge(GraphicsWindow *gw, Mesh *m, int ifrom,int ito, Point2 cursor,BestEdge *best);
	void SnapToEdge(GraphicsWindow *gw, PolyLine &line, int ifrom,int ito, Point2 cursor,BestEdge *best);

};

TSTR *XmeshSnap::snapname(int index){
	return &(name[index]);
}


//Some hit subclasses

//A hit class for vertices
class VertexHit : public OsnapHit {
	friend class XmeshSnap;
public:
	VertexHit(Point3 p3, Osnap* s, int sub, HitMesh *m, int topoindex);
	OsnapHit *clone();
private:
	int vertindex;
};

VertexHit::VertexHit(Point3 p3, Osnap* s, int sub, HitMesh *m, int topoindex)  : OsnapHit(p3, s, sub, m)
{
	vertindex = topoindex;
}

OsnapHit *VertexHit::clone()
{
	return new VertexHit(*this);
}

//A hit class for edges
class EdgeHit : public OsnapHit {
	friend class XmeshSnap;
public:
	EdgeHit(Point3 p3, Osnap* s, int sub, HitMesh *m, int ifrom, int ito, float pct);
	~EdgeHit(){};
	OsnapHit *clone();

private:
	int m_ifrom;
	int m_ito;
	float m_pct;
};

EdgeHit::EdgeHit(Point3 p3, Osnap* s, int sub, HitMesh *m, int ifrom, int ito, float pct)  : OsnapHit(p3, s, sub, m)
{
	m_pct = pct;
	m_ifrom = ifrom;
	m_ito = ito;
}

OsnapHit *EdgeHit::clone()
{
	return new EdgeHit(*this);
}

//A hit class for faces
class FaceHit : public OsnapHit {
	friend class XmeshSnap;
public:
	FaceHit(Point3 p3, Osnap* s, int sub, HitMesh *m, int topoindex, Point3& bary);
	OsnapHit *clone();
private:
	int m_faceindex;
	Point3 m_bary;
};

FaceHit::FaceHit(Point3 p3, Osnap* s, int sub, HitMesh *m, int topoindex, Point3& bary)  : OsnapHit(p3, s, sub, m)
{
	m_faceindex = topoindex;
	m_bary = bary;
}

OsnapHit *FaceHit::clone()
{
	return new FaceHit(*this);
}


XmeshSnap::XmeshSnap(){ //constructor

	name[END_SUB] = TSTR( GetString(IDS_END));
	name[EDGE_SUB] = TSTR( GetString(IDS_EDGE));
	name[VERT_SUB] = TSTR( GetString(IDS_VERT));
	name[MID_SUB] = TSTR( GetString(IDS_MID));
	name[FACE_SUB] = TSTR( GetString(IDS_FACE));
	name[C_FACE_SUB] = TSTR( GetString(IDS_C_FACE));

	tools = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ICONS));
	masks = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MASK));

	markerdata[0]=OsnapMarker(4,mark0verts,mark0es);
	markerdata[1]=OsnapMarker(6,mark1verts,mark1es);
	markerdata[2]=OsnapMarker(5,mark2verts,mark2es);
	markerdata[3]=OsnapMarker(6,mark3verts,mark3es);
	markerdata[4]=OsnapMarker(4,mark4verts,mark4es);
	markerdata[5]=OsnapMarker(4,mark5verts,mark5es);
};

XmeshSnap::~XmeshSnap()
{
	DeleteObject(tools);
	DeleteObject(masks);
}

boolean XmeshSnap::ValidInput(SClass_ID scid, Class_ID cid){
	boolean c_ok = FALSE, sc_ok = FALSE;
	sc_ok |= (scid == GEOMOBJECT_CLASS_ID)? TRUE : FALSE;
	sc_ok |= (scid == SHAPE_CLASS_ID)? TRUE : FALSE;
	sc_ok |= (scid == CAMERA_CLASS_ID)? TRUE : FALSE;
	sc_ok |= (scid == LIGHT_CLASS_ID)? TRUE : FALSE;
	sc_ok |= (scid == HELPER_CLASS_ID)? TRUE : FALSE;
//	c_ok |= (cid == Class_ID(0xe44f10b3,0))? TRUE : FALSE; 
	return sc_ok;
}

WORD XmeshSnap::AccelKey(int index){
	switch (index){
	case VERT_SUB:
		return 0x56;
		break;
	case END_SUB:
		return 0x45;
		break;
	case EDGE_SUB:
		return 0x47;
		break;
	case MID_SUB:
		return 0x4d;
		break;
	case FACE_SUB:
		return 0x46;
		break;
	case C_FACE_SUB:
		return 0x43;
		break;
	default:
		return 0;
	}
}


// This is from GraphicsGems II, p10 (Jack C. Morrison)
// Squared distance from point p to line a->b 
static float DistPointToLine(Point2 p, Point2 a, Point2 b) {
	float x1 = b.x - a.x;
	float y1 = b.y - a.y;
	float a2 = (p.y-a.y)*x1-(p.x-a.x)*y1; // 2* area of triangle abp
	return (float)sqrt(a2*a2/(x1*x1+y1*y1));
	}

static float DistPointToLine(Point2 p, Point3 a, Point3 b) {
	float x1 = b.x - a.x;
	float y1 = b.y - a.y;
	float a2 = (p.y-a.y)*x1-(p.x-a.x)*y1; // 2* area of triangle abp
	return (float)sqrt(a2*a2/(x1*x1+y1*y1));
	}

#define EDGETOL 0.0001

// Checks if the cursor point is closer to the given edge than to any previous edge
void XmeshSnap::SnapToEdge(GraphicsWindow *gw, Mesh *m, int ifrom,int ito,Point2 cursor,BestEdge *best)
{

	Point3 from = m->getVert(ifrom);
	Point3 to = m->getVert(ito);

	float edgelen;
	Point3 xyz[3];
	xyz[0] = from;
	xyz[1] = to;
	gw->clearHitCode();
	gw->polyline(2, xyz, NULL/*clr*/, NULL/* tex*/, 0/*open*/, NULL);
	if(gw->checkHitCode())
	{
		// Potential hit -- Make sure of its real distance and that it isn't past the end of the line
		Point3 sf,st;
		gw->transPoint(&from,&sf);
		gw->transPoint(&to,&st);
		float ybias = (float)gw->getWinSizeY() - 1.0f;
		Point2 sf2(sf.x, /*ybias - */sf.y);
		Point2 st2(st.x, /*ybias - */st.y);
		if((edgelen = Length(st2 - sf2)) < EDGETOL )
			return;

		float distance = DistPointToLine(cursor,sf2,st2);
//		float distance = DistPointToLine(cursor,sf,st);
		if(distance > ceil(best->distance))
			return;
		else
		{
			// Now make sure it isn't past the endpoints of the line!
			Point2 pa,ab,pb,ba;

			// Note the following Kludge:
			// The cursor point may have been converted	to Ipoint2 and back so that the 
			// cursor representation of an endpoint may fail the following test without 
			// taking the floor of these endpoints
			sf2.x = (float)floor(sf2.x);
			sf2.y = (float)floor(sf2.y);
			st2.x = (float)floor(st2.x);
			st2.y = (float)floor(st2.y);

			if((edgelen = Length(st2 - sf2)) < EDGETOL )
				return;

			pa = sf2 - cursor;
			ab = st2 - sf2;
			pb = st2 - cursor;
			ba = sf2 - st2;
//			Before the kludge
//			if((pa.x * ab.x + pa.y * ab.y) < 0.0f && (pb.x * ba.x + pb.y * ba.y) < 0.0f)
//			The equality should take care of coincident case
			if((pa.x * ab.x + pa.y * ab.y) <= 0.0f && (pb.x * ba.x + pb.y * ba.y) <= 0.0f)
			{
				best->distance = distance;
				best->from = from;
				best->to = to;

				//add the candidate points based on the active subsnaps
				 if(GetActive(EDGE_SUB))
				 {	
					HitMesh *hitmesh = new HitMesh(3);
					hitmesh->setVert(0, from);
					hitmesh->setVert(1, to);

					float dap = Length(cursor - sf2);
					assert(Length(st2 - sf2)>=0.0f);
					float pct = (float)sqrt(fabs(dap*dap - distance*distance)) / Length(st2 - sf2);
					Point3 cand;
					float pctout = gw->interpWorld(&xyz[0],&xyz[1],pct,&cand);
//					AddCandidate(new Point3(cand),EDGE_SUB,2,from,to);
					theman->RecordHit(new EdgeHit(cand, this, EDGE_SUB, hitmesh, ifrom, ito, pct));
				 }
				 if(GetActive(END_SUB))
				 {
//					 AddCandidate(new Point3(xyz[0]),END_SUB,2,from,to);
//					 AddCandidate(new Point3(xyz[1]),END_SUB,2,from,to);
					HitMesh *hitmesh = new HitMesh(3);
					hitmesh->setVert(0, from);
					hitmesh->setVert(1, to);

					theman->RecordHit(new VertexHit(from, this, END_SUB, hitmesh, ifrom));

					hitmesh = new HitMesh(3);
					hitmesh->setVert(0, from);
					hitmesh->setVert(1, to);
					theman->RecordHit(new VertexHit(to, this, END_SUB, hitmesh, ito));
				 }
				 if(GetActive(MID_SUB))
				 { 
					HitMesh *hitmesh = new HitMesh(3);
					hitmesh->setVert(0, from);
					hitmesh->setVert(1, to);

					Point3 mid=((xyz[0]+xyz[1])/2.0f);
//					AddCandidate(new Point3(mid),MID_SUB,2,from,to);
					theman->RecordHit(new EdgeHit(mid, this, MID_SUB, hitmesh, ifrom, ito, 0.5f));
				 }
			}
		}
	}
}


void FindEnclosingKnots(PolyLine &line,const int ifrom,const int ito, int *pa, int *pb)
{
	*pa = ifrom;
	*pb = ito;
	while(line.pts[*pb].flags & POLYPT_INTERPOLATED)
	{
		(*pb)++;
		if(*pb > (line.numPts - 1))
		{
			if(line.IsClosed())
				*pb = 0;
			else
				assert(0);//the last point should have been a knot
		}

	}
	while(line.pts[*pa].flags & POLYPT_INTERPOLATED && *pa>0)
		(*pa)--;
}


//This version works on splines
void XmeshSnap::SnapToEdge(GraphicsWindow *gw, PolyLine &line, int ifrom,int ito, Point2 cursor,BestEdge *best)
{
	PolyPt ppt1 = line.pts[ifrom];
	PolyPt ppt2 = line.pts[ito];

	Point3 from =ppt1.p;
	Point3 to = ppt2.p;

	float edgelen;
	Point3 xyz[3];
	xyz[0] = from;
	xyz[1] = to;
	gw->clearHitCode();
	gw->polyline(2, xyz, NULL/*clr*/, NULL/* tex*/, 0/*open*/, NULL);
	if(gw->checkHitCode())
	{
		// Potential hit -- Make sure of its real distance and that it isn't past the end of the line
		Point3 sf,st;
		gw->transPoint(&from,&sf);
		gw->transPoint(&to,&st);
		float ybias = (float)gw->getWinSizeY() - 1.0f;
		Point2 sf2(sf.x, /*ybias - */sf.y);
		Point2 st2(st.x, /*ybias - */st.y);
		if((edgelen = Length(st2 - sf2)) < EDGETOL )
			return;

		float distance = DistPointToLine(cursor,sf2,st2);
//		float distance = DistPointToLine(cursor,sf,st);
		if(distance > ceil(best->distance))
			return;
		else
		{
			// Now make sure it isn't past the endpoints of the line!
			Point2 pa,ab,pb,ba;

			// Note the following Kludge:
			// The cursor point may have been converted	to Ipoint2 and back so that the 
			// cursor representation of an endpoint may fail the following test without 
			// taking the floor of these endpoints
			sf2.x = (float)floor(sf2.x);
			sf2.y = (float)floor(sf2.y);
			st2.x = (float)floor(st2.x);
			st2.y = (float)floor(st2.y);

			if((edgelen = Length(st2 - sf2)) < EDGETOL )
				return;

			pa = sf2 - cursor;
			ab = st2 - sf2;
			pb = st2 - cursor;
			ba = sf2 - st2;
//			Before the kludge
//			if((pa.x * ab.x + pa.y * ab.y) < 0.0f && (pb.x * ba.x + pb.y * ba.y) < 0.0f)
//			The equality should take care of coincident case
			if((pa.x * ab.x + pa.y * ab.y) <= 0.0f && (pb.x * ba.x + pb.y * ba.y) <= 0.0f)
			{
				best->distance = distance;
				best->from = from;
				best->to = to;

				//add the candidate points based on the active subsnaps
				 if(GetActive(EDGE_SUB))
				 {	
					HitMesh *hitmesh = new HitMesh(3);
					hitmesh->setVert(0, from);
					hitmesh->setVert(1, to);

					float dap = Length(cursor - sf2);
					assert(Length(st2 - sf2)>=0.0f);
					float pct = (float)sqrt(fabs(dap*dap - distance*distance)) / Length(st2 - sf2);
					Point3 cand;
					float pctout = gw->interpWorld(&xyz[0],&xyz[1],pct,&cand);
//					AddCandidate(new Point3(cand),EDGE_SUB,2,from,to);
					theman->RecordHit(new EdgeHit(cand, this, EDGE_SUB, hitmesh, ifrom, ito, pct));
				 }
				 if(GetActive(END_SUB))
				 {
					int a, b;
					BOOL closure = FALSE;
					FindEnclosingKnots(line,ifrom,ito, &a, &b);
					if(b == 0)
					{
						b = line.numPts - 1;
						closure = TRUE;
					}
					assert(b>=a);//should always be ordered now
					int segs = b-a;
					int size = segs + 2 + (closure?1:0);
					assert(size>2);

					HitMesh *hitmesha = new HitMesh(size);
					HitMesh *hitmeshb = new HitMesh(size);
					for(int i = 0; i<=segs; i++)//segs + 1 vertices 
					{
						Point3 curpt = line.pts[a + i].p;
						hitmesha->setVert(i, curpt);
						hitmeshb->setVert(i, curpt);
					}
					if(closure)
					{
						hitmesha->setVert(segs + 1, line.pts[0].p);
						hitmeshb->setVert(segs + 1, line.pts[0].p);
						b=0;
					}

					theman->RecordHit(new VertexHit(line.pts[a].p, this, END_SUB, hitmesha, ifrom));

					theman->RecordHit(new VertexHit(line.pts[b].p, this, END_SUB, hitmeshb, ito));
				 }
				 if(GetActive(MID_SUB))
				 { 
					HitMesh *hitmesh = new HitMesh(3);
					hitmesh->setVert(0, from);
					hitmesh->setVert(1, to);

					Point3 mid=((xyz[0]+xyz[1])/2.0f);
//					AddCandidate(new Point3(mid),MID_SUB,2,from,to);
					theman->RecordHit(new EdgeHit(mid, this, MID_SUB, hitmesh, ifrom, ito, 0.5f));
				 }
			}
		}
	}
}

//10/20/99
//added code to allow hits on backfacing faces
static BOOL IntersectFaces(const Ray& ray,Mesh *m, Point3& point, int& findex, Point3& retbary, int backcull)
{
	float at = (float)9.99999e+30; //some silly large number. mdl 4/24/98
	BOOL first = FALSE;
	int faceCt = m->getNumFaces();
	Face *face = m->faces;

	for(int i=0; i<faceCt; ++i, ++face) 
		{	
			DWORD *v;
			Point3 v0,v1,v2,n,norm,bary,p;
			if(face->Hidden())
				continue;
			float rn,d,a;
			v = face->getAllVerts();
			v0 = m->verts[face->v[0]];
			v1 = m->verts[face->v[1]];
			v2 = m->verts[face->v[2]];
			n  = Normalize((v1-v0)^(v2-v1));
			
			// See if the ray intersects the plane (backfaced)
			rn = DotProd(ray.dir,n);
			if (backcull && (rn > -EPSILON)) continue;
			
			// Use a point on the plane to find d
			d = DotProd(v0,n);

			// Find the point on the ray that intersects the plane
			a = (d - DotProd(ray.p,n)) / rn;

			// Must be positive...
			if (a < 0.0f) continue;

			// Must be closer than the closest at so far
			if (first) {
				if (a > at) continue;
				}

			// The point on the ray and in the plane.
			p = ray.p + a*ray.dir;

			bary = m->BaryCoords(i,p);

			// barycentric coordinates must sum to 1 and each component must
			// be in the range 0-1
			if (bary.x<0.0f || bary.x>1.0f || bary.y<0.0f || bary.y>1.0f || bary.z<0.0f || bary.z>1.0f) continue;
			if (fabs(bary.x + bary.y + bary.z - 1.0f) > EPSILON) continue;

			// Hit!
			first = TRUE;
			norm  = n;
			at    = a;
			findex = i;
			point = p;
			retbary = bary;
		}// endfor
	return first;
}

void XmeshSnap::Snap(Object* pobj, IPoint2 *p, TimeValue t)
{
	//this osnap snaps to topological features common to meshes and splines

	//local copy of the cursor position
	Point2 fp = Point2((float)p->x, (float)p->y);

	//JH 5/9/99
	//suspend the hold since we may be amking temporary objects
	bool wasHolding = false;
	if(theHold.Holding())
	{
		wasHolding = true;
		theHold.Suspend();
	}

	//if it's a spline we'll get a poly shape from it
	PolyShape vertshape;

	//if it's a tri we'll use the following
	TriObject* ptri = NULL;
	Mesh* m = NULL;
	int vertCt;
	char* sv;
	BOOL meshsnapdata ;
	BOOL del = FALSE;
	MeshAccess *maccess = NULL;

	if (pobj->SuperClassID() == SHAPE_CLASS_ID) {
		((ShapeObject *)pobj)->MakePolyShape(t, vertshape, PSHAPE_BUILTIN_STEPS,0);
	}

	
	//convert this object to a triobject if it isn't already one
	else if (pobj->IsSubClassOf(triObjectClassID)) {
		ptri = (TriObject *) pobj;
		m = &ptri->GetMesh();
	}
/*	else if (pobj->IsSubClassOf(Class_ID(DUMMY_CLASS_ID,0))) {
		DummyObject *pdummy = (DummyObject *) pobj;
		m = pdummy->mesh;
		del = FALSE;
	}
*/

	else if(pobj->CanConvertToType(triObjectClassID)){
		ptri = (TriObject *) pobj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID,0));
		m= &ptri->GetMesh();
		del = TRUE;
	}
	else
	{
		if(wasHolding)
		{
			theHold.Resume();
			wasHolding = false;//silly since we're gonna return
		}
		return;
	}


	if(m)//we've got a mesh
	{
	maccess = new MeshAccess(m);
	// Build the snap data structures
//	int templimits = m_hitgw->getRndLimits();
//	m_hitgw->setRndLimits(templimits /*| GW_WIREFRAME*/ | ~GW_BACKCULL);
	meshsnapdata = maccess->BuildSnapData(m_hitgw,TRUE,TRUE);
//	m_hitgw->setRndLimits(templimits );

	// Set usable flag in snapV structure
	sv = maccess->GetsnapV();
	vertCt = m->getNumVerts();
	}

	// do the vertex snapping if its active	
	if(GetActive(VERT_SUB)) {
		Point3 screen3;
		Point2 screen2;
		Point3 vert;
  
		if(vertshape.numLines)
		{
			for(int i = 0; i< vertshape.numLines; i++)
			{
				PolyLine line = vertshape.lines[i];
				BOOL vis = TRUE;

				for(int k = 0; k< line.numPts; k++)
				{
					PolyPt ppt = line.pts[k];
					PolyPt ppt1;
					PolyPt ppt2;
					vis = TRUE;

					if(ppt.flags & POLYPT_NO_SNAP)
						vis = FALSE;
					else
					if (k ==0)
						{
						if (line.IsClosed())
							{
							ppt1 = line.pts[0];
							ppt2 = line.pts[line.numPts-1];
							if ( (ppt1.flags & POLYPT_INVIS_EDGE) &&
								 (ppt2.flags & POLYPT_INVIS_EDGE) )
								vis = FALSE;

							}
						else
							{
							ppt1 = line.pts[0];
							if (ppt1.flags & POLYPT_INVIS_EDGE)
								vis = FALSE;
							}


						}
/*
					else if (k==(line.numPts-1))
						{
						if (line.IsClosed())
							{
							ppt1 = line.pts[0];
							ppt2 = line.pts[line.numPts-2];
							if ( (ppt1.flags & POLYPT_INVIS_EDGE) &&
								 (ppt2.flags & POLYPT_INVIS_EDGE) )
								vis = FALSE;

							}
						else
							{
							ppt1 = line.pts[k];
							if (ppt1.flags & POLYPT_INVIS_EDGE)
								vis = FALSE;
							}

						}
*/					else

						{
						ppt1 = line.pts[k-1	];
						ppt2 = line.pts[k	];
						if ((ppt1.flags & POLYPT_INVIS_EDGE) &&
							(ppt2.flags & POLYPT_INVIS_EDGE) )
							vis = FALSE;
						}

					if (vis)
						if (CheckPotentialHit(&(ppt.p),0, fp))
							theman->RecordHit(new VertexHit(ppt.p, this, VERT_SUB, NULL, i));
				}
			}
		}
		else if(m && meshsnapdata)//must be a bone fide mesh
		{
			for(int i = 0; i < vertCt; ++i)
			{
				if(!sv[i] || m->vertHide[i])
					continue;
				if (CheckPotentialHit(m->verts,i, fp))
					theman->RecordHit(new VertexHit(m->verts[i], this, VERT_SUB, NULL, i));
			}
		}

	}

	// do the edge snapping if its active	
	if(GetActive(EDGE_SUB) || GetActive(END_SUB) || GetActive(MID_SUB)) {
		Point2 cursor;
		BOOL got_one = FALSE;

		cursor.x = (float)p->x;
		cursor.y = (float)p->y;

	 	hr.type = POINT_RGN;
		hr.epsilon = theman->GetSnapStrength();
		hr.pt.x = p->x;
		hr.pt.y = p->y;

		int savedlimits = m_hitgw->getRndLimits();
		m_hitgw->setRndLimits((savedlimits | GW_PICK) & ~GW_ILLUM);
		m_hitgw->setHitRegion(&hr);
		m_hitgw->clearHitCode();

		// Zip thru the visible edges and see if we get a snap
		// puts the best edge found into the bestedge structure
		int i;
		BestEdge best;
		best.distance = 9999.0f;	// Reset to ridiculous distance

		if(m && meshsnapdata)
		{
			int faceCt = m->getNumFaces();
			char *f = maccess->GetsnapF();
			DWORD *v;
			Face *face = m->faces;

			sv = maccess->GetsnapV();

			//10/20/99 Allow snapping to invisible edges when shown
			int all_edges = m_hitgw->getRndLimits() & GW_ALL_EDGES;

			for(i=0; i<faceCt; i++,f++,face++) {
				if(*f && !face->Hidden()) {
					v = face->getAllVerts();
					if((*f & 1) && (all_edges || face->getEdgeVis(0)))
						if(sv[v[0]] & sv[v[1]] & BIT1 )
							SnapToEdge(m_hitgw,m, v[0],v[1],cursor,&best);
					if((*f & 2) && (all_edges || face->getEdgeVis(1)))
						if(sv[v[1]] & sv[v[2]] & BIT1 )
							SnapToEdge(m_hitgw,m, v[1],v[2],cursor,&best);
					if((*f & 4) && (all_edges || face->getEdgeVis(2)))
						if(sv[v[2]] & sv[v[0]] & BIT1 )
							SnapToEdge(m_hitgw,m, v[2],v[0],cursor,&best);
					}
				}
		}
		//special code for handling splines
//		else if(!gotsdata)
		else if(vertshape.numLines)
		{
			for(i = 0; i< vertshape.numLines; i++)
			{
				PolyLine line = vertshape.lines[i];
				BOOL vis = TRUE;

				for(int k = 0; k< line.numPts - 1; k++)
				{
					PolyPt ppt = line.pts[k];
					if (ppt.flags & POLYPT_VISEDGE)
						vis = TRUE;
					else if (ppt.flags & POLYPT_INVIS_EDGE)
						vis = FALSE;

					if (vis)
						SnapToEdge(m_hitgw, line, k, k+1,cursor,&best);
				}
				if(line.IsClosed())
				{
					PolyPt ppt = line.pts[k];
					if (ppt.flags & POLYPT_VISEDGE)
						vis = TRUE;
					else if (ppt.flags & POLYPT_INVIS_EDGE)
						vis = FALSE;
					if (vis)
						SnapToEdge(m_hitgw, line, k, 0,cursor,&best);
				}

			}
		}

/* Now done in snaptoedge
		//now zip through the candidates record them with the manager
		HitMesh *hitmesh;
		for(int j = 0; j< NumCandidates(); j++)
		{
			hitmesh = new HitMesh;
			GetCandidateMesh(j,hitmesh);
			theman->RecordHit(new MeshHit(*GetCandidatePoint(j), this, GetCandidateType(j), hitmesh));
		}
*/

		m_hitgw->setRndLimits(savedlimits);
//		ClearCandidates();
	}



	// do the face snapping if its active	
	if(m && meshsnapdata && (GetActive(FACE_SUB) || GetActive(C_FACE_SUB))) {
		Point2 cursor;
		Matrix3 itm = Inverse(theman->GetObjectTM());

		cursor.x = (float)p->x;
		cursor.y = (float)p->y;

		//compute a ray for intersecting with the faces of the mesh
		Ray ray;
		theman->GetVpt()->MapScreenToWorldRay(float(cursor.x), float(cursor.y),ray);
		// Transform the rays into the ref coord system space
		ray.p = itm * ray.p;
		ray.dir = VectorTransform( itm, ray.dir );

		// Intersect the cursor with the faces of the mesh
		int bestindex;
		Point3 p;
		Face *face = m->faces;
		Point3 bary;

		BOOL gotone = IntersectFaces(ray,m,p,bestindex,bary, m_hitgw->getRndLimits() & GW_BACKCULL);

		if(gotone)
		{//got a face hit
			HitMesh *hitmesh;

			// We got a face
			Point3 screen3,mspoint;
			Point2 screen,screen2;
			Point3 p0,p1,p2;
			
			//get the vertices of the winning face
			p0 = m->verts[m->faces[bestindex].v[0]];
			p1 = m->verts[m->faces[bestindex].v[1]];
			p2 = m->verts[m->faces[bestindex].v[2]];


			if(GetActive(FACE_SUB))
				mspoint = p;
			else if(GetActive(C_FACE_SUB))
			{
				mspoint = (p0 + p1 + p2)/3.0f;
				bary.x=bary.y=bary.z=0.333333333f;
			}


			hitmesh = new HitMesh(5);
			hitmesh->setVert(0, p0);
			hitmesh->setVert(1, p1);
			hitmesh->setVert(2, p2);
			hitmesh->setVert(3, p0);

			theman->RecordHit(new FaceHit(mspoint, this, GetActive(FACE_SUB)?FACE_SUB:C_FACE_SUB, hitmesh, bestindex, bary));
		}//end gotone

	}// endif FACE STUFF

	if(del) 
	{
		ptri->DeleteAllRefsFromMe();//may reference a controller but no one should reference it.
		ptri->DeleteThis();
	}

	if(wasHolding)
	{
		theHold.Resume();
		wasHolding = false;//silly since we're gonna return
	}

	if(maccess)
		delete maccess;
		
};

void interp_line(Point3 *pt1, Point3* pt2, float sParam, Point3* interpPt)
{
if(interpPt)
		*interpPt = (*pt1 + (*pt2 - *pt1) * sParam);
}



Point3 XmeshSnap::ReEvaluate(TimeValue t, OsnapHit * hit, Object * pobj)
{
	Mesh* m;

	//JH 5/9/99
	//suspend the hold since we may be amking temporary objects
	bool wasHolding = false;
	if(theHold.Holding())
	{
		wasHolding = true;
		theHold.Suspend();
	}

	//convert this object to a triobject if it isn't already one
	BOOL del = TRUE;
	TriObject* ptri = NULL;
	if (pobj->IsSubClassOf(triObjectClassID)) {
		ptri = (TriObject *) pobj;
		m = &ptri->GetMesh();
		del = FALSE;
	}
/*	else if (pobj->IsSubClassOf(Class_ID(DUMMY_CLASS_ID,0))) {
		DummyObject *pdummy = (DummyObject *) pobj;
		m = pdummy->mesh;
		del = FALSE;
	}
*/	
	else if(pobj->CanConvertToType(triObjectClassID)){
//		HCURSOR modecursor = GetCursor(); //save the active command mode's cursor
		ptri = (TriObject *) pobj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID,0));
		m= &ptri->GetMesh();
//		SetCursor(modecursor);//Fixed the bug in polyshape::MakeCap
	}

	Point3 objpoint;

	switch(hit->GetSubsnap())
	{
	case VERT_SUB:
	case END_SUB:
		objpoint = m->getVert(((VertexHit *)hit)->vertindex);
		break;
	case EDGE_SUB:
	case MID_SUB:
		interp_line(m->getVertPtr(((EdgeHit *)hit)->m_ifrom),m->getVertPtr(((EdgeHit *)hit)->m_ito),((EdgeHit *)hit)->m_pct,&objpoint);
		break;
	case FACE_SUB:
	case C_FACE_SUB:
		{
			Point3 bary = ((FaceHit *)hit)->m_bary; 
			int f = ((FaceHit *)hit)->m_faceindex;
			objpoint = m->verts[m->faces[f].v[0]] * bary.x +
				    m->verts[m->faces[f].v[1]] * bary.y +
					m->verts[m->faces[f].v[2]] * bary.z;
		}
		break;
	default:
		break;
	}

	if(del) 
	{
		ptri->DeleteAllRefsFromMe();//may reference a controller but no one should reference it.
		ptri->DeleteThis();
	}

	if(wasHolding)
	{
		theHold.Resume();
		wasHolding = false;//silly since we're gonna return
	}
	return objpoint;
}


TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}


/*===========================================================================*\
 | The Class Descriptor
\*===========================================================================*/

class OsnapClassDesc:public ClassDesc {
	public:
	// The IsPublic() method should return TRUE if the plug-in can be picked
	// and assigned by the user. Some plug-ins may be used privately by other
	// plug-ins implemented in the same DLL and should not appear in lists for
	// user to choose from, so these plug-ins would return FALSE.
	int 			IsPublic() { return 0; }
	// This is the method that actually creates a new instance of
	// a plug-in class.  By having the system call this method,
	// the plug-in may use any memory manager it wishes to 
	// allocate its objects.  The system calls the correspoding 
	// DeleteThis() method of the plug-in to free the memory.  Our 
	// implementations use 'new' and 'delete'.
	void *			Create(BOOL loading = FALSE) {return new XmeshSnap();}
//	void *			Create(OsnapManager *pman) { return new XmeshSnap(pman); }
	// This is used for debugging purposes to give the class a 
	// displayable name.  It is also the name that appears on the button
	// in the MAX user interface.
	const TCHAR *	ClassName() { return _T("XmeshSnap"); }
	// The system calls this method at startup to determine the type of object
	// this is.  In our case, we're a geometric object so we return 
	// GEOMOBJECT_CLASS_ID.  The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return OSNAP_CLASS_ID; }
	// The system calls this method to retrieve the unique
	// class id for this object.
	Class_ID		ClassID() { 
		return XMESH_SNAP_CLASS_ID; }
	// The category is selected
	// in the bottom most drop down list in the create branch.
	// If this is set to be an exiting category (i.e. "Primatives", ...) then
	// the plug-in will appear in that category. If the category doesn't
	// yet exists then it is created.  We use the new How To category for
	// all the example plug-ins in the How To sections.
	const TCHAR* 	Category() { return _T(""); }
	};

// Declare a static instance of the class descriptor.
static OsnapClassDesc sampDesc;
// This function returns the address of the descriptor.  We call it from 
// the LibClassDesc() function, which is called by the system when loading
// the DLLs at startup.
ClassDesc* GetSampDesc() { return &sampDesc; }

/*===========================================================================*\
 | The DLL Functions
\*===========================================================================*/
// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;

	if (! controlsInit) {
		controlsInit = TRUE;
		
		// Initialize MAX's custom controls
		InitCustomControls(hInstance);
		
		// Initialize Win95 controls
		InitCommonControls();
	}
	
	return(TRUE);
	}

// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int LibNumberClasses() {return 1;}

// This function return the ith class descriptor
__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetSampDesc();		
		default: return 0;
		}
	}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG
LibVersion() {  return VERSION_3DSMAX; }

