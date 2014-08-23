/**********************************************************************
 *<
	FILE: pmesh.h

	DESCRIPTION:  simple Polygon class module

	CREATED BY: greg finch

	HISTORY: created 1 december, 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "meshadj.h"

#define COPLANAR_NORMAL_EPSILON 0.1 // in degrees
#define LineEPSILON             1e-03f

struct AEdge {
    int e0;
    int e1;
};

AEdge AdjoiningEdge(DWORD*, DWORD*);

enum PType { OutputTriangles, OutputQuads, OutputNGons, OutputVisibleEdges };
enum EType { HiddenEdge, CoplanarEdge, VisibleEdge };

/*
#define     Edge_A     (1<<0)
#define     Edge_B     (1<<1)
#define     Edge_C     (1<<2)
*/
class PMEdge {
    EType       mType;          // type of edge (hidden, etc...)
    int         mVertex0;       // index of the first vertex
    int         mVertex1;       // index of the ccw vertex
    int         mFace0;         // the face on the left
    int         mFace0Flags;    // this is the the ccw index numbers ei. AB, BC, CA
    int         mFace1;         // the face on the right
    int         mFace1Flags;    // this is the the ccw index numbers ei. AB, BC, CA
public:
    PMEdge() {
        mType = HiddenEdge;
        mVertex0 = -1;
        mVertex1 = -1;
        mFace0   = -1;
        mFace1   = -1;
    }
    int     GetVIndex(int i)
                { return (i ? mVertex1 : mVertex0); }
    void    SetVIndex(int i, int v)
                { (i ? mVertex1 = v : mVertex0 = v); }
    int     GetFaceFlags(int i)
                { return (i ? mFace1Flags : mFace0Flags); }
    int     GetFace(int i)
                { return (i ? mFace1 : mFace0); }
    void    SetFace(int i, int face, int flags)
                { (i ? mFace1 = face : mFace0 = face);
                  (i ? mFace1Flags = flags : mFace0Flags = flags); }
    void    SetEdgeVisiblity(EType type)
                { mType = type; }
    EType   GetEdgeVisiblity()
                { return mType; }
};

class PMPoly {
    PType       mType;      // type of polygon tri, quad, ngon, etc...
    Point3      mFNormal;   // the face normal
    Tab<PMEdge> mEdges;     // the polygon edges
    //Tab<PMEdge> mTEdges;    // the polygon texture edges
    Tab<int>    mPolygon;   // the the ccw vert indices
    Tab<int>    mTPolygon;  // the the ccw texture vert indices
    Tab<int>    mTriFaces;  // the coplanar tri faces indices
    Tab<UVVert> mTVerts;    // the polygon texture verts
    Tab<Point3> mVNormals;  // the vertex normals
    Tab<Point3> mVerts;     // the polygon verts
public:
    PMPoly() {
        mType    = OutputTriangles;
    }
    ~PMPoly() {
        if (mEdges.Count())
            mEdges.Delete(0,    mEdges.Count());
        //if (mTEdges.Count())
        //    mTEdges.Delete(0,   mTEdges.Count());
        if (mPolygon.Count())
            mPolygon.Delete(0,  mPolygon.Count());
        if (mTPolygon.Count())
            mTPolygon.Delete(0, mTPolygon.Count());
        if (mTriFaces.Count())
            mTriFaces.Delete(0, mTriFaces.Count());
        if (mTVerts.Count())
            mTVerts.Delete(0,   mTVerts.Count());
        if (mVNormals.Count())
            mVNormals.Delete(0, mVNormals.Count());
        if (mVerts.Count())
            mVerts.Delete(0,    mVerts.Count());
    }

    void    Shrink()
                { mEdges.Shrink();
                  //mTEdges.Shrink();
                  mPolygon.Shrink();
                  mTPolygon.Shrink();
                  mTriFaces.Shrink();
                  //mNormals.Shrink();
                  mTVerts.Shrink();
                  mVerts.Shrink(); }
    PType   GetType()
                { return mType; }
    Point3  GetFNormal()
                { return mFNormal; }
    void    SetFNormal(Point3 fNormal)
                { mFNormal = fNormal; }
    int     GetEdgeCnt()
                { return mEdges.Count(); }
    PMEdge  GetEdge(int i)
                { return mEdges[i]; }
    int     AddEdge(PMEdge* newEdge)
             // return the location inserted
                { return mEdges.Insert(mEdges.Count(), 1, newEdge);}
    /*
    int     GetTEdgeCnt()
                { return mTEdges.Count(); }
    PMEdge  GetTEdge(int i)
                { return mTEdges[i]; }
    int     AddTEdge(PMEdge* newEdge)
             // return the location inserted
                { return mTEdges.Insert(mTEdges.Count(), 1, newEdge);}
    */
    int     GetTriFaceCnt()
                { return mTriFaces.Count(); }
    int     GetTriFace(int i)
                { return mTriFaces[i]; }
    int     AddTriFace(int* index)
                { return mTriFaces.Insert(mTriFaces.Count(), 1, index); }
    int     GetVertCnt()
                { return mVerts.Count(); }
    Point3  GetVert(int i)
                { return mVerts[i]; }
    int     AddVert(Point3* newVert)
                { return mVerts.Insert(mVerts.Count(), 1, newVert); }
    int     RemoveLastVert()
                { return mVerts.Delete(mVerts.Count() - 1, 1); }
    int     RemoveFirstVert()
                { return mVerts.Delete(0, 1); }
    int     GetTVertCnt()
                { return mTVerts.Count(); }
    UVVert  GetTVert(int i)
                { return mTVerts[i]; }
    int     AddTVert(UVVert* newTVert)
                { return mTVerts.Insert(mTVerts.Count(), 1, newTVert); }
    int     RemoveLastTVert()
                { return mTVerts.Delete(mTVerts.Count() - 1, 1); }
    int     RemoveFirstTVert()
                { return mTVerts.Delete(0, 1); }
    int     GetVNormalCnt()
                { return mVNormals.Count(); }
    Point3  GetVNormal(int i)
                { return mVNormals[i]; }
    int     AddVNormal(Point3* newVNormal)
                { return mVNormals.Insert(mVNormals.Count(), 1, newVNormal); }
    int     RemoveFirstVNormal()
                { return mVNormals.Delete(0, 1); }
    int     RemoveLastVNormal()
                { return mVNormals.Delete(mVNormals.Count() - 1, 1); }
    int     GetVIndexCnt()
                { return mPolygon.Count(); }
    int     GetVIndex(int i)
                { return mPolygon[i]; }
    int     AddToPolygon(int index)
                { return mPolygon.Insert(mPolygon.Count(), 1, &index); }
    int     RemoveLastFromPolygon()
                { return mPolygon.Delete(mPolygon.Count() - 1, 1); }
    int     RemoveFirstFromPolygon()
                { return mPolygon.Delete(0, 1); }
    int     GetTVIndexCnt()
                { return mTPolygon.Count(); }
    int     GetTVIndex(int i)
                { return mTPolygon[i]; }
    int     AddToTPolygon(int index)
                { return mTPolygon.Insert(mTPolygon.Count(), 1, &index); }
    int     RemoveLastFromTPolygon()
                { return mTPolygon.Delete(mTPolygon.Count() - 1, 1); }
    int     RemoveFirstFromTPolygon()
                { return mTPolygon.Delete(0, 1); }
};

class PMesh {
public:
	PMesh(Mesh&, PType, BOOL);
    ~PMesh();

    BOOL    GenPolygons();          // generate the polygons:
                                    //    returns TRUE if concave
    int     GetPolygonCnt()         // get the number of polygons
                { return mPolygons.Count(); };
    PMPoly* GetPolygon(int num)     // get a polygon
                { return &mPolygons[num]; };
    int     GetVertexCnt()          // get the number of vertices
                { return mVertices.Count(); };
    Point3  GetVertex(int num)      // get a vertex
                { return mVertices[num]; };
    int     GetTVertexCnt()         // get the number of vertices
                { return mTVertices.Count(); };
    Point3  GetTVertex(int num)     // get a vertex
                { return mTVertices[num]; };
    int     LookUpVert(int num);    // get the pmesh to trimesh mapping
    int     LookUpTVert(int num);   // get the pmesh to trimesh mapping

private:
	Mesh            mOrgMesh;               // the orginal mesh which PMesh is generated from
    PType           mType;                  // enum { OutputTriangles, OutputQuads, OutputNGons, OutputVisibleEdges };
    BOOL            mIsTextured;            // has UV coords
    double          mCosEps;                // allowable normals angular delta
    AdjEdgeList*    mAdjEdges;              // used to check for coplanar faces
    AdjFaceList*    mAdjFaces;              // used to check for coplanar faces
    int*            mSuppressEdges;         // internal, suppressed edges

    Tab<PMPoly> mPolygons;                  // used to store the polygons
    int         AddPolygon(PMPoly* newPoly) // return the location inserted
                    { return mPolygons.Insert(mPolygons.Count(), 1, newPoly); }

    Tab<Point3> mVertices;          // used to store the meshes verticies
    BitArray    mVMapping;          // used to store the vertices used
    int         AddVertex(Point3* newVert)
                    { return mVertices.Insert(mVertices.Count(), 1, newVert); }
    
    Tab<Point3> mTVertices;         // used to store the meshes verticies
    BitArray    mTVMapping;         // used to store the texture vertices used
    int         AddTVertex(Point3* newVert) 
                    { return mTVertices.Insert(mTVertices.Count(), 1, newVert); }

    void    GenCPFaces();           // generate the coplanar faces
    void    AddCPTriFaces(int faceNum, int polyCnt, BitArray& uFaces, BitArray& wFaces); //add tri face to polygon
    void    GenEdges();             // generate the visible edges
    void    GenVertices();          // generate the vertices
    void    CheckAdjacentFaces(int, BitArray&);
    BOOL    CheckIfMatIDsMatch(int faceNum, int adjFaceNum);    // make sure matID is the same
    BOOL    CheckIfUVVertMatch(AEdge edge, int faceNum, int adjFaceNum);    // make sure the UVVert is the same
    BOOL    CheckIfCoplanar(int, int);      // check if a face and an adj face are coplanar
    BOOL    CheckIfColinear(Point3 first, Point3 second, Point3 third);  // check if three verts are colinear
    void    GetNextEdge(int polyNum, int v0Prev, int edgeNum, BitArray& uVerts, BitArray& uEdges); // traverse the edges
    BOOL    IsValidFace(int);               // check for bad faces
};
