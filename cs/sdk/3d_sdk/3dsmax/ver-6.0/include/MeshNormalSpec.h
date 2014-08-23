/**********************************************************************
 *<
	FILE: MeshNormalSpec.h

	DESCRIPTION:  User-specifiable normals for Triangle Meshes

	CREATED BY: Steve Anderson

	HISTORY: created July 2002 during SIGGraph

 *>	Copyright (c) 2002 Autodesk, Inc., All Rights Reserved.
 **********************************************************************/

// Necessary prior inclusions...?

#ifndef __MESH_NORMALS_H_
#define __MESH_NORMALS_H_

#include "baseinterface.h"
#include "ipipelineclient.h"

#define MESH_NORMAL_SPEC_INTERFACE Interface_ID(0xa4e770b, 0x47aa3cf9)

class MeshNormalFace {
	int mNormalID[3];
	UBYTE mSpecified;

public:
	MeshNormalFace () : mSpecified(0x0) { }
	DllExport void Clear();

	// Low-level - do not use mSpecified data!
	int GetNormalID(int corner) { return ((corner>-1) && (corner<3)) ? mNormalID[corner] : -1; }
	void SetNormalID (int corner, int norm) { if ((corner>-1) && (corner<3)) mNormalID[corner] = norm; }

	bool GetSpecified (int corner) { return ((corner>-1) && (corner<3) && (mSpecified & (1<<corner))) ? true : false; }
	DllExport void SetSpecified (int corner, bool value=true);
	DllExport void SpecifyNormalID (int corner, int norm);

	DllExport void Flip ();	// Reverses order of verts.  0 remains start.

	DllExport MeshNormalFace & operator= (const MeshNormalFace & from);
	DllExport void MyDebugPrint (bool printAll=false);

	DllExport IOResult Save (ISave *isave);
	DllExport IOResult Load (ILoad *iload);
};

// Class MeshNormalSpec flags:
#define MESH_NORMAL_NORMALS_BUILT 0x01
#define MESH_NORMAL_NORMALS_COMPUTED 0x02
#define MESH_NORMAL_DISPLAY_HANDLES 0x04
#define MESH_NORMAL_FACE_ANGLES 0x08	// Used face angles last time we computed normals
#define MESH_NORMAL_MODIFIER_SUPPORT 0x20	// When modifying mesh, indicates whether the current modifier supports these Normals.

// Default length for normal display
#define MESH_NORMAL_LENGTH_DEFAULT 10.0f

class MeshNormalSpec : public IPipelineClient {
private:
	int mNumNormalAlloc, mNumFaceAlloc;
	int mNumNormals, mNumFaces;
	MeshNormalFace *mpFace;
	Point3 *mpNormal;
	BitArray mNormalExplicit;	// Indicates whether mpNormal[i] is explicit or computed from face normals.

	// Display and selection data:
	BitArray mNormalSel;
	float mDisplayLength;
	DWORD mFlags;

	// We also maintain a pointer to the parent Mesh
	// (NOTE that the Mesh MUST keep this pointer updated at all times!)
	Mesh *mpParent;

public:
	MeshNormalSpec () : mpFace(NULL), mpNormal(NULL), mNumNormalAlloc(0),
		mNumFaceAlloc(0), mNumNormals(0), mNumFaces(0), mpParent(NULL), mFlags(0),
		mDisplayLength(MESH_NORMAL_LENGTH_DEFAULT) { }
	~MeshNormalSpec () { ClearAndFree (); }

	// Flag Accessors:
	void SetFlag (DWORD fl, bool val=true) { if (val) mFlags |= fl; else mFlags &= ~fl; }
	void ClearFlag (DWORD fl) { mFlags &= ~fl; }
	bool GetFlag (DWORD fl) const { return (mFlags & fl) ? true : false; }

	// Initialization, allocation:
	DllExport void Initialize ();	// Initializes all data members - do not use if already allocated!
	DllExport bool NAlloc (int num, bool keep=TRUE);
	DllExport void NShrink ();	// shrinks allocation down to actual number of normals.
	DllExport bool FAlloc (int num, bool keep=TRUE);
	DllExport void FShrink ();
	DllExport void Clear ();	// Deletes everything.
	DllExport void ClearAndFree ();	// Deletes everything, frees all memory

	// Data access:
	// Lowest level:
	int GetNumFaces () const { return mNumFaces; }
	DllExport bool SetNumFaces (int numFaces);
	int GetNumNormals () const { return mNumNormals; }
	DllExport bool SetNumNormals (int numNormals);

	Point3 & Normal (int normID) const { return mpNormal[normID]; }
	Point3 * GetNormalArray () const { return mpNormal; }
	bool GetNormalExplicit (int normID) const { return mNormalExplicit[normID] ? true : false; }
	void SetNormalExplicit (int normID, bool value) { mNormalExplicit.Set (normID, value); }
	MeshNormalFace & Face(int faceID) const { return mpFace[faceID]; }
	MeshNormalFace * GetFaceArray () const { return mpFace; }

	void SetParent (Mesh *pMesh) { mpParent = pMesh; }

	// Data access - higher level:
	DllExport Point3 & GetNormal (int face, int corner);
	DllExport void SetNormal (int face, int corner, Point3 & normal);
	DllExport int GetNormalIndex (int face, int corner);
	DllExport void SetNormalIndex (int face, int corner, int normalIndex);
	DllExport int NewNormal (Point3 normal, bool explic=true);

	DllExport void SetSelection (BitArray & newSelection);
	BitArray & GetSelection() { return mNormalSel; }
	void SetDisplayLength (float displayLength) { mDisplayLength = displayLength; }
	float GetDisplayLength () { return mDisplayLength; }

	// Display and hit testing - note that these require an accurate mpParent pointer.
	DllExport void Display (GraphicsWindow *gw, bool showSel);
	DllExport bool HitTest (GraphicsWindow *gw, HitRegion *hr, DWORD flags, SubObjHitList& hitList);
	DllExport Box3 GetBoundingBox (Matrix3 *tm=NULL, bool selectedOnly=false);

	// This method dumps all unspecified normals.  Best to use only from within CheckNormals.
	DllExport void ClearNormals ();

	// Fills in the mpSpecNormal data by building all the unspecified normals,
	// and computing non-explicit ones.
	// Does nothing if normal faces not allocated yet!
	// Requires an accurate mpParent pointer.
	DllExport void BuildNormals ();

	// This method just recomputes the directions of non-explicit normals,
	// without rebuilding the normal list.
	// Requires an accurate mpParent pointer.
	DllExport void ComputeNormals ();

	// This checks our flags and calls BuildNormals, ComputeNormals as needed.
	// Requires an accurate mpParent pointer.
	DllExport void CheckNormals ();

	// operators and debug printing
	DllExport MeshNormalSpec & operator= (const MeshNormalSpec & from);
	DllExport void CopySpecified (const MeshNormalSpec & from);	// Like operator=, but omits unspecified.
	DllExport MeshNormalSpec & operator+= (const MeshNormalSpec & from);
	DllExport void MyDebugPrint (bool printAll=false);
	DllExport bool CheckAllData (int numParentFaces);

	DllExport IOResult Save (ISave *isave);
	DllExport IOResult Load (ILoad *iload);

	// From BaseInterface:
	Interface_ID GetID() {return MESH_NORMAL_SPEC_INTERFACE;}
	DllExport void DeleteInterface();
	DllExport BaseInterface* GetInterface(Interface_ID id);
	DllExport BaseInterface* CloneInterface(void* remapDir = NULL);

	// --- IPipelineClient methods
	DllExport void ShallowCopy( IPipelineClient* from, ULONG_PTR channels );
	DllExport void DeepCopy( IPipelineClient* from, ULONG_PTR channels );
	DllExport void NewAndCopyChannels( ULONG_PTR channels );
	DllExport void FreeChannels( ULONG_PTR channels, int zeroOthers = 1 );
	DllExport void ZeroChannels( ULONG_PTR channels );
	DllExport void AppendAllChannels( IPipelineClient* from );

	// Actual operations:
	DllExport bool Transform (Matrix3 & xfm, BOOL useSel=false, BitArray *normalSelection=NULL);
	DllExport bool Translate (Point3 & translate, BOOL useSel=true, BitArray *normalSelection=NULL);
	DllExport bool BreakNormals (BOOL useSel=true, BitArray *normalSelection=NULL, BOOL toAverage=false);
	// Requires an accurate mpParent pointer:
	DllExport bool UnifyNormals (BOOL useSel=true, BitArray *normalSelection=NULL, BOOL toAverage=false);
	DllExport bool AverageNormals (BOOL useThresh=false, float threshold=0.0f, BOOL useSel=true, BitArray *normalSelection=NULL);
	DllExport bool SpecifyNormals (BOOL useSel=true, BitArray *normalSelection=NULL);
	DllExport bool MakeNormalsExplicit (BOOL useSel=true, BitArray *normalSelection=NULL, bool value=true);
	DllExport bool ResetNormals (BOOL useSel=true, BitArray *normalSelection=NULL);
};

#endif //__MESH_NORMALS_H_
