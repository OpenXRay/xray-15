/**********************************************************************
 *<
	FILE: EditPoly.cpp

	DESCRIPTION: Edit Poly Modifier

	CREATED BY: Steve Anderson, based on Face Extrude modifier by Berteig, and my own Poly Select modifier.

	HISTORY: created March 2002

 *>	Copyright (c) 2002 Discreet, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"
#include "iparamm2.h"
#include "MeshDLib.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"
#include "EditPoly.h"

void PolyOperation::RecordSelection(MNMesh & mesh)
{
	if (MeshSelectionLevel() == MNM_SL_OBJECT) return;
	int i;
	switch (MeshSelectionLevel())
	{
	case MNM_SL_VERTEX:
		mSelection.SetSize (mesh.numv);
		for (i=0; i<mesh.numv; i++)
		{
			if (mesh.v[i].GetFlag(MN_USER)) mSelection.Set (i);
		}
		break;

	case MNM_SL_EDGE:
		mSelection.SetSize (mesh.nume);
		for (i=0; i<mesh.nume; i++)
		{
			if (mesh.e[i].GetFlag (MN_USER)) mSelection.Set (i);
		}
		break;

	case MNM_SL_FACE:
		mSelection.SetSize (mesh.numf);
		for (i=0; i<mesh.numf; i++)
		{
			if (mesh.f[i].GetFlag (MN_USER)) mSelection.Set (i);
		}
		break;
	}
}

void PolyOperation::RestoreSelection(MNMesh & mesh)
{
	if (MeshSelectionLevel() == MNM_SL_OBJECT) return;
	int i, max = mSelection.GetSize ();
	if (max == 0) return;
	switch (MeshSelectionLevel())
	{
	case MNM_SL_VERTEX:
		if (max>mesh.numv) max = mesh.numv;
		for (i=0; i<max; i++)
			mesh.v[i].SetFlag(MN_USER, mSelection[i]?true:false);
		break;

	case MNM_SL_EDGE:
		if (max>mesh.nume) max = mesh.nume;
		for (i=0; i<max; i++)
			mesh.e[i].SetFlag(MN_USER, mSelection[i]?true:false);
		break;

	case MNM_SL_FACE:
		if (max>mesh.numf) max = mesh.numf;
		for (i=0; i<max; i++)
			mesh.f[i].SetFlag(MN_USER, mSelection[i]?true:false);
		break;
	}
}

void PolyOperation::CopyBasics (PolyOperation *pCopyTo)
{
	pCopyTo->mSelection = mSelection;
	Tab<int> paramList;
	GetParameters (paramList);
	for (int i=0; i<paramList.Count (); i++) {
		int id = paramList[i];
		memcpy (pCopyTo->Parameter(id), Parameter(id), ParameterSize(id));
	}
}

const USHORT kSelection = 0x400;
const USHORT kParameter = 0x410;

IOResult PolyOperation::SaveBasics (ISave *isave)
{
	isave->BeginChunk (kSelection);
	mSelection.Save (isave);
	isave->EndChunk ();

	ULONG nb;
	Tab<int> paramList;
	GetParameters (paramList);
	for (int i=0; i<paramList.Count (); i++)
	{
		int id = paramList[i];
		isave->BeginChunk (kParameter);
		isave->Write (&id, sizeof(int), &nb);
		isave->Write (Parameter(id), ParameterSize(id), &nb);
		isave->EndChunk ();
	}
	return IO_OK;
}

IOResult PolyOperation::LoadBasics (ILoad *iload)
{
	IOResult res;
	ULONG nb;
	int id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kSelection:
			res = mSelection.Load (iload);
			break;

		case kParameter:
			res = iload->Read (&id, sizeof(int), &nb);
			if (res != IO_OK) break;
			void *prmtr = Parameter(id);
			if (prmtr == NULL) return IO_ERROR;
			res = iload->Read (prmtr, ParameterSize(id), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

void PolyOperation::GetValues (IParamBlock2 *pBlock, TimeValue t, Interval & ivalid)
{
	Tab<int> paramList;
	GetParameters (paramList);
	for (int i=0; i<paramList.Count(); i++)
	{
		int id = paramList[i];
		void *prmtr = Parameter(id);
		if (prmtr == NULL) continue;
		ParamType2 type = pBlock->GetParameterType (id);
		if (type == TYPE_INT)
		{
			int *iparam = (int *) prmtr;
			pBlock->GetValue (paramList[i], t, *iparam, ivalid);
		}
		else if (type == TYPE_FLOAT)
		{
			float *fparam = (float *)prmtr;
			pBlock->GetValue (id, t, *fparam, ivalid);
		}
	}
}

const USHORT kPolyOperationBasics = 0x0600;

IOResult PolyOperation::Save (ISave *isave)
{
	isave->BeginChunk (kPolyOperationBasics);
	SaveBasics (isave);
	isave->EndChunk ();
	return IO_OK;
}

IOResult PolyOperation::Load (ILoad *iload)
{
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kPolyOperationBasics:
			res = LoadBasics (iload);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpWeldVertex : public PolyOperation
{
private:
	float mThreshold;
	
public:
	PolyOpWeldVertex () : mThreshold(0.0f) { }

	// Implement PolyOperation methods:
	int OpID () { return kOpWeldVertex; }
	TCHAR *Name () { return GetString (IDS_EP_WELD_VERTEX); }
	int DialogID () { return IDD_EP_WELD; }
	int MeshSelectionLevel() { return MNM_SL_VERTEX; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = kEPWeldThreshold;
	}
	void *Parameter (int paramID) {
		return (paramID == kEPWeldThreshold) ? &mThreshold : NULL;
	}
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpWeldVertex *ret = new PolyOpWeldVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }

	// Local methods:
	bool WeldShortPolyEdges (MNMesh & mesh, DWORD vertFlag);
};

void PolyOpWeldVertex::Do(MNMesh & mesh)
{
	// Weld the suitable border vertices:
	bool haveWelded = false;
	if (mesh.WeldBorderVerts (mThreshold, MN_USER)) {
		mesh.CollapseDeadStructs ();
		haveWelded = true;
	}

	// Weld vertices that share short edges:
	if (WeldShortPolyEdges (mesh, MN_USER)) haveWelded = true;

	if (haveWelded) {
		mesh.InvalidateTopoCache ();
		mesh.FillInMesh ();
	}
}

// This method needs to be moved to class MNMesh when the SDK opens up again.
// Note that it's an exact copy of some code in the EditablePoly source.
bool MNMeshCollapseEdges (MNMesh & mm, DWORD edgeFlag) {
	// Collect average locations ourselves
	// We do this by collecting information about which vertex
	// or map vertex each eliminated point was welded to - its destination.
	// Then we average things out at the end.
	Tab<int> pointDest;
	pointDest.SetCount (mm.numv);
	for (int i=0; i<mm.numv; i++) pointDest[i] = -1;

	Tab<Tab<int> *> mapPointDest;
	mapPointDest.SetCount (mm.numm + NUM_HIDDENMAPS);
	for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
		int nhm = mapChannel + NUM_HIDDENMAPS;
		if (mm.M(mapChannel)->GetFlag (MN_DEAD)) mapPointDest[nhm] = NULL;
		else {
			int nv = mm.M(mapChannel)->numv;
			if (!nv) mapPointDest[nhm] = NULL;
			else {
				mapPointDest[nhm] = new Tab<int>;
				mapPointDest[nhm]->SetCount (nv);
				int *md = mapPointDest[nhm]->Addr(0);
				for (i=0; i<nv; i++) md[i] = -1;
			}
		}
	}

	// Perform topological welding, edge by edge
	bool ret=false;
	for (i=0; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (!mm.e[i].GetFlag (MN_USER)) continue;
		int v1 = mm.e[i].v1;
		int v2 = mm.e[i].v2;
		int f1 = mm.e[i].f1;
		int f2 = mm.e[i].f2;
		int eid1 = mm.f[f1].EdgeIndex (i);
		int eid2 = (f2>-1) ? mm.f[f2].EdgeIndex (i) : -1;
		Tab<int> mv1, mv2;
		mv1.SetCount ((mm.numm+NUM_HIDDENMAPS)*2);
		mv2.SetCount ((mm.numm+NUM_HIDDENMAPS)*2);
		for (int mapChannel = -NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
			MNMap *map = mm.M(mapChannel);
			int nhm = (NUM_HIDDENMAPS + mapChannel)*2;
			if (map->GetFlag (MN_DEAD)) {
				mv1[nhm] = -1;
				continue;
			}
			mv1[nhm] = map->f[f1].tv[eid1];
			mv1[nhm+1] = (f2>-1) ? map->f[f2].tv[(eid2+1)%mm.f[f2].deg] : mv1[nhm];
			mv2[nhm] = map->f[f1].tv[(eid1+1)%mm.f[f1].deg];
			mv2[nhm+1] = (f2>-1) ? map->f[f2].tv[eid2] : mv2[nhm];
		}
		if (mm.WeldEdge (i)) {
			pointDest[v2] = v1;
			for (int nhm=0; nhm<mapPointDest.Count(); nhm++) {
				if (mapPointDest[nhm] == NULL) continue;
				if (!mapPointDest[nhm]->Count()) continue;
				if (mv1[nhm*2]<0) continue;
				int *mpd = mapPointDest[nhm]->Addr(0);
				mpd[mv2[nhm*2]] = mv1[nhm*2];
				if (mv2[nhm*2+1] != mv2[nhm*2]) mpd[mv2[nhm*2+1]] = mv1[nhm*2+1];
			}
			ret = true;
		}
	}

	// Then set all the welded vertices to the correct averaged locations
	if (ret) {
		for (mapChannel = -NUM_HIDDENMAPS-1; mapChannel<mm.numm; mapChannel++) {
			// note - -NUM_HIDDENMAPS-1 is not a valid map channel,
			// We're using it for a convenient extra loop for the actual geometry.
			int nhm = mapChannel+NUM_HIDDENMAPS;
			if ((nhm>-1) && mapPointDest[nhm] == NULL) continue;
			Tab<int> & destinations = (nhm<0) ? pointDest : *(mapPointDest[nhm]);

			for (i=0; i<destinations.Count(); i++) {
				if (destinations[i] < 0) continue;
				if (destinations[destinations[i]] > -1) {
					// We have nested destinations - straighten them out.
					// Form a stack of all the intermediate destinations:
					Tab<int> intermediate;
					for (int last=i; destinations[last]>-1; last=destinations[last]) {
						intermediate.Append (1, &last, 2);
					}
					// Now destinations[current] = -1, which means it's the final destination of all of these.
					// Correct whole chain.
					for (int k=0; k<intermediate.Count()-1; k++) destinations[intermediate[k]] = last;
				}

				if (nhm<0) mm.v[destinations[i]].p += mm.v[i].p;
				else mm.M(mapChannel)->v[destinations[i]] += mm.M(mapChannel)->v[i];

				destinations[destinations[i]]--;
			}

			for (i=0; i<destinations.Count(); i++) {
				if (destinations[i] > -2) continue;
				if (nhm<0) mm.v[i].p /= float(-destinations[i]);
				else mm.M(mapChannel)->v[i] /= float(-destinations[i]);
			}

			// Free memory
			if (nhm>-1) {
				delete mapPointDest[nhm];
				mapPointDest[nhm] = NULL;
			}
		}
	}

	return ret;
}

bool PolyOpWeldVertex::WeldShortPolyEdges (MNMesh & mesh, DWORD vertFlag) {
	// In order to collapse vertices, we turn them into edge selections,
	// where the edges are shorter than the weld threshold.
	bool canWeld = false;
	mesh.ClearEFlags (MN_USER);
	float threshSq = mThreshold*mThreshold;
	for (int i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.v[mesh.e[i].v1].GetFlag (vertFlag)) continue;
		if (!mesh.v[mesh.e[i].v2].GetFlag (vertFlag)) continue;
		if (LengthSquared (mesh.P(mesh.e[i].v1) - mesh.P(mesh.e[i].v2)) > threshSq) continue;
		mesh.e[i].SetFlag (MN_USER);
		canWeld = true;
	}
	if (!canWeld) return false;

	return MNMeshCollapseEdges (mesh, MN_USER);
}

class PolyOpExtrudeFace : public PolyOperation
{
private:
	float mHeight;
	int mType;
public:
	PolyOpExtrudeFace () : mHeight(0.0f), mType(0) { }

	// Implement PolyOperation methods:
	int OpID () { return kOpExtrudeFace; }
	TCHAR * Name () { return GetString (IDS_EP_EXTRUDE_FACE); }
	int DialogID () { return IDD_EP_F_EXTRUDE; }
	int MeshSelectionLevel() { return MNM_SL_FACE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = kEPExtrudeHeight;
		paramList[1] = kEPExtrudeType;
	}
	void *Parameter (int paramID) {
		if (paramID == kEPExtrudeHeight) return &mHeight;
		if (paramID == kEPExtrudeType) return &mType;
		return NULL;
	}
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpExtrudeFace *ret = new PolyOpExtrudeFace();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

void PolyOpExtrudeFace::Do(MNMesh & mesh)
{
	MNChamferData chamData;
	if (mType<2) {
		MNFaceClusters fclust(mesh, MN_USER);
		if (!mesh.ExtrudeFaceClusters (fclust)) return;
		if (mType == 0) {
			// Get fresh face clusters:
			MNFaceClusters fclustAfter(mesh, MN_USER);
			Tab<Point3> clusterNormals, clusterCenters;
			fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
			mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
		} else {
			mesh.GetExtrudeDirection (&chamData, MN_USER);
		}
	} else {
		// Polygon-by-polygon extrusion.
		if (!mesh.ExtrudeFaces (MN_USER)) return;
		MNFaceClusters fclustAfter(mesh, MN_USER);
		Tab<Point3> clusterNormals, clusterCenters;
		fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
		mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
	}

	// Move vertices
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		mesh.v[i].p += chamData.vdir[i]*mHeight;
	}
}

class PolyOpExtrudeVertex : public PolyOperation
{
private:
	float mWidth, mHeight;

public:
	PolyOpExtrudeVertex () : mWidth(0.0f), mHeight(0.0f) { }

	// Implement PolyOperation methods:
	int OpID () { return kOpExtrudeVertex; }
	TCHAR * Name () { return GetString (IDS_EP_EXTRUDE_VERTEX); }
	int DialogID () { return IDD_EP_VE_EXTRUDE; }
	int MeshSelectionLevel() { return MNM_SL_VERTEX; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = kEPExtrudeHeight;
		paramList[1] = kEPExtrudeWidth;
	}
	void *Parameter (int paramID) {
		if (paramID == kEPExtrudeHeight) return &mHeight;
		if (paramID == kEPExtrudeWidth) return &mWidth;
		return NULL;
	}
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpExtrudeVertex *ret = new PolyOpExtrudeVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

void PolyOpExtrudeVertex::Do (MNMesh & mesh)
{
	MNChamferData chamData;
	chamData.InitToMesh(mesh);
	Tab<Point3> tUpDir;
	tUpDir.SetCount (mesh.numv);

	// Topology change:
	if (!mesh.ExtrudeVertices (MN_USER, &chamData, tUpDir)) return;

	// Apply map changes based on base width:
	int i;
	Tab<UVVert> tMapDelta;
	for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
		if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		chamData.GetMapDelta (mesh, mapChannel, mWidth, tMapDelta);
		UVVert *pMapVerts = mesh.M(mapChannel)->v;
		if (!pMapVerts) continue;
		for (i=0; i<mesh.M(mapChannel)->numv; i++) pMapVerts[i] += tMapDelta[i];
	}

	// Apply geom changes based on base width:
	Tab<Point3> tDelta;
	chamData.GetDelta (mWidth, tDelta);
	for (i=0; i<mesh.numv; i++) mesh.v[i].p += tDelta[i];

	// Move the points up:
	for (i=0; i<tUpDir.Count(); i++) mesh.v[i].p += tUpDir[i]*mHeight;
}

class PolyOpExtrudeEdge : public PolyOperation
{
private:
	float mWidth, mHeight;

public:
	PolyOpExtrudeEdge () : mWidth(0.0f), mHeight(0.0f) { }

	// Implement PolyOperation methods:
	int OpID () { return kOpExtrudeEdge; }
	TCHAR * Name () { return GetString (IDS_EP_EXTRUDE_EDGE); }
	int DialogID () { return IDD_EP_VE_EXTRUDE; }
	int MeshSelectionLevel() { return MNM_SL_EDGE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (2);
		paramList[0] = kEPExtrudeHeight;
		paramList[1] = kEPExtrudeWidth;
	}
	void *Parameter (int paramID) {
		if (paramID == kEPExtrudeHeight) return &mHeight;
		if (paramID == kEPExtrudeWidth) return &mWidth;
		return NULL;
	}
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpExtrudeEdge *ret = new PolyOpExtrudeEdge();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

void PolyOpExtrudeEdge::Do (MNMesh & mesh) {
	MNChamferData chamData;
	chamData.InitToMesh(mesh);
	Tab<Point3> tUpDir;
	tUpDir.SetCount (mesh.numv);

	// Topology change:
	if (!mesh.ExtrudeEdges (MN_USER, &chamData, tUpDir)) return;

	// Apply map changes based on base width:
	int i;
	Tab<UVVert> tMapDelta;
	for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
		if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		chamData.GetMapDelta (mesh, mapChannel, mWidth, tMapDelta);
		UVVert *pMapVerts = mesh.M(mapChannel)->v;
		if (!pMapVerts) continue;
		for (i=0; i<mesh.M(mapChannel)->numv; i++) pMapVerts[i] += tMapDelta[i];
	}

	// Apply geom changes based on base width:
	Tab<Point3> tDelta;
	chamData.GetDelta (mWidth, tDelta);
	for (i=0; i<mesh.numv; i++) mesh.v[i].p += tDelta[i];

	// Move the points up:
	for (i=0; i<tUpDir.Count(); i++) mesh.v[i].p += tUpDir[i]*mHeight;
}

class PolyOpChamferVertex : public PolyOperation
{
private:
	float mAmount;

public:
	PolyOpChamferVertex () : mAmount(0.0f) { }

	// Implement PolyOperation methods:
	int OpID () { return kOpChamferVertex; }
	TCHAR * Name () { return GetString (IDS_EP_CHAMFER_VERTEX); }
	int DialogID () { return IDD_EP_CHAMFER; }
	int MeshSelectionLevel() { return MNM_SL_VERTEX; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = kEPChamferAmount;
	}
	void *Parameter (int paramID) {
		if (paramID == kEPChamferAmount) return &mAmount;
		return NULL;
	}
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpChamferVertex *ret = new PolyOpChamferVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

void PolyOpChamferVertex::Do (MNMesh & mesh)
{
	MNChamferData chamData;
	mesh.ChamferVertices (MN_USER, &chamData);
	Tab<UVVert> mapDelta;
	for (int mapChannel = -NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
		if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		chamData.GetMapDelta (mesh, mapChannel, mAmount, mapDelta);
		for (int i=0; i<mapDelta.Count(); i++) mesh.M(mapChannel)->v[i] += mapDelta[i];
	}

	Tab<Point3> vertexDelta;
	chamData.GetDelta (mAmount, vertexDelta);
	for (int i=0; i<vertexDelta.Count(); i++) mesh.P(i) += vertexDelta[i];
}

class PolyOpChamferEdge : public PolyOperation
{
private:
	float mAmount;

public:
	PolyOpChamferEdge () : mAmount(0.0f) { }

	// Implement PolyOperation methods:
	int OpID () { return kOpChamferEdge; }
	TCHAR * Name () { return GetString (IDS_EP_CHAMFER_EDGE); }
	int DialogID () { return IDD_EP_CHAMFER; }
	int MeshSelectionLevel() { return MNM_SL_EDGE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (1);
		paramList[0] = kEPChamferAmount;
	}
	void *Parameter (int paramID) {
		if (paramID == kEPChamferAmount) return &mAmount;
		return NULL;
	}
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpChamferEdge *ret = new PolyOpChamferEdge();
		CopyBasics (ret);
		ret->mAmount = mAmount;
		return ret;
	}
	void DeleteThis () { delete this; }
};

void PolyOpChamferEdge::Do (MNMesh & mesh)
{
	MNChamferData *pMeshChamData = new MNChamferData;
	mesh.ChamferEdges (MN_USER, pMeshChamData);
	Tab<UVVert> mapDelta;
	for (int mapChannel = -NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
		if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		pMeshChamData->GetMapDelta (mesh, mapChannel, mAmount, mapDelta);
		for (int i=0; i<mapDelta.Count(); i++) mesh.M(mapChannel)->v[i] += mapDelta[i];
	}

	Tab<Point3> vertexDelta;
	pMeshChamData->GetDelta (mAmount, vertexDelta);
	for (int i=0; i<vertexDelta.Count(); i++) mesh.P(i) += vertexDelta[i];
}

class PolyOpBevelFace : public PolyOperation
{
private:
	float mHeight, mOutline;
	int mType;

public:
	PolyOpBevelFace () : mHeight(0.0f), mOutline(0.0f), mType(0) { }

	// Implement PolyOperation methods:
	int OpID () { return kOpBevelFace; }
	TCHAR * Name () { return GetString (IDS_EP_BEVEL_FACE); }
	int DialogID () { return IDD_EP_BEVEL; }
	int MeshSelectionLevel() { return MNM_SL_FACE; }
	void GetParameters (Tab<int> & paramList) {
		paramList.SetCount (3);
		paramList[0] = kEPBevelHeight;
		paramList[1] = kEPExtrudeType;
		paramList[2] = kEPBevelOutline;
	}
	void *Parameter (int paramID) {
		if (paramID == kEPBevelHeight) return &mHeight;
		if (paramID == kEPExtrudeType) return &mType;
		if (paramID == kEPBevelOutline) return &mOutline;
		return NULL;
	}
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpBevelFace *ret = new PolyOpBevelFace();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

void PolyOpBevelFace::Do (MNMesh & mesh)
{
	// Topological extrusion first:
	MNChamferData chamData;
	if (mType<2) {
		MNFaceClusters fclust(mesh, MN_USER);
		if (!mesh.ExtrudeFaceClusters (fclust)) return;
		if (mType == 0) {
			// Get fresh face clusters:
			MNFaceClusters fclustAfter(mesh, MN_USER);
			Tab<Point3> clusterNormals, clusterCenters;
			fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
			mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
		} else {
			mesh.GetExtrudeDirection (&chamData, MN_USER);
		}
	} else {
		// Polygon-by-polygon extrusion.
		if (!mesh.ExtrudeFaces (MN_USER)) return;
		MNFaceClusters fclustAfter(mesh, MN_USER);
		Tab<Point3> clusterNormals, clusterCenters;
		fclustAfter.GetNormalsCenters (mesh, clusterNormals, clusterCenters);
		mesh.GetExtrudeDirection (&chamData, &fclustAfter, clusterNormals.Addr(0));
	}

	int i;
	if (mHeight) {
		for (i=0; i<mesh.numv; i++) mesh.v[i].p += chamData.vdir[i]*mHeight;
	}

	if (mOutline) {
		MNTempData temp(&mesh);
		Tab<Point3> *outDir;
		if (mType == 0) outDir = temp.OutlineDir (MESH_EXTRUDE_CLUSTER, MN_USER);
		else outDir = temp.OutlineDir (MESH_EXTRUDE_LOCAL, MN_USER);
		if (outDir && outDir->Count()) {
			Point3 *od = outDir->Addr(0);
			for (i=0; i<mesh.numv; i++) mesh.v[i].p += od[i]*mOutline;
		}
	}
}

class PolyOpBreakVertex : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return kOpBreakVertex; }
	TCHAR * Name () { return GetString (IDS_EP_BREAK_VERTEX); }
	// No dialog ID.
	int MeshSelectionLevel() { return MNM_SL_VERTEX; }
	// No parameters.
	void Do (MNMesh & mesh) { mesh.SplitFlaggedVertices (MN_USER); }
	PolyOperation *Clone () {
		PolyOpBreakVertex *ret = new PolyOpBreakVertex();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

// class PolyOpExtrudeAlongSpline is located in header file, because we need access to it in other modules.

void PolyOpExtrudeAlongSpline::GetValues (IParamBlock2 *pblock, TimeValue t, Interval & ivalid)
{
	PolyOperation::GetValues (pblock, t, ivalid);

	// Special case for the spline & transform.
	if (!mSplineValidity.InInterval (t))
	{
		mSplineValidity.SetInfinite ();
		INode *pSplineNode;
		pblock->GetValue (kEPExtrudeSpline, t, pSplineNode, mSplineValidity);
		if (pSplineNode == NULL) mpSpline = NULL;
		else
		{
			bool del = FALSE;
			SplineShape *pSplineShape = NULL;
			ObjectState os = pSplineNode->GetObjectRef()->Eval(t);
			if (os.obj->IsSubClassOf(splineShapeClassID)) pSplineShape = (SplineShape *) os.obj;
			else {
				if (!os.obj->CanConvertToType(splineShapeClassID)) return;
				pSplineShape = (SplineShape*)os.obj->ConvertToType (t, splineShapeClassID);
				if (pSplineShape!=os.obj) del = TRUE;
			}
			BezierShape & bezShape = pSplineShape->GetShape();

			Matrix3 mSplineXfm = pSplineNode->GetObjectTM (t, &mSplineValidity);
			if (mpSpline != NULL) delete mpSpline;
			mpSpline = new Spline3D(*bezShape.GetSpline(0));

			if (del) delete pSplineShape;
		}
	}

	ivalid &= mSplineValidity;
}

void PolyOpExtrudeAlongSpline::ClearSpline ()
{
	if (mpSpline != NULL)
	{
		delete mpSpline;
		mpSpline = NULL;
	}
	mSplineValidity.SetEmpty ();
}

class FrenetFinder {
	BasisFinder mBasisFinder;
public:
	void ConvertPathToFrenets (Spline3D *pSpline, Matrix3 relativeTransform, Tab<Matrix3> & tFrenets,
						   int numSegs, bool align, float rotateAroundZ);
};

static FrenetFinder theFrenetFinder;

void FrenetFinder::ConvertPathToFrenets (Spline3D *pSpline, Matrix3 relativeTransform, Tab<Matrix3> & tFrenets,
						   int numSegs, bool align, float rotateAroundZ) {
	// Given a path, a sequence of points in 3-space, create transforms
	// for putting a cross-section around each of those points, loft-style.

	// bezShape is provided by user, tFrenets contains output, numSegs is one less than the number of transforms requested.

	// Strategy: The Z-axis is mapped along the path, and the X and Y axes 
	// are chosen in a well-defined manner to get an orthonormal basis.

	int i;

	if (numSegs < 1) return;
	tFrenets.SetCount (numSegs+1);

	int numIntervals = pSpline->Closed() ? numSegs+1 : numSegs;
	float denominator = float(numIntervals);
	Point3 xDir, yDir, zDir, location, tangent, axis;
	float position, sine, cosine, theta;
	Matrix3 rotation;

	// Find initial x,y directions:
	location = relativeTransform * pSpline->InterpCurve3D (0.0f);
	tangent = relativeTransform.VectorTransform (pSpline->TangentCurve3D (0.0f));
	zDir = tangent;

	Matrix3 inverseBasisOfSpline(1);
	if (align) {
		xDir = Point3(0.0f, 0.0f, 0.0f);
		yDir = xDir;
		mBasisFinder.BasisFromZDir (zDir, xDir, yDir);
		if (rotateAroundZ) {
			Matrix3 rotator(1);
			rotator.SetRotate (AngAxis (zDir, rotateAroundZ));
			xDir = xDir * rotator;
			yDir = yDir * rotator;
		}
		Matrix3 basisOfSpline(1);
		basisOfSpline.SetRow (0, xDir);
		basisOfSpline.SetRow (1, yDir);
		basisOfSpline.SetRow (2, tangent);
		basisOfSpline.SetTrans (location);
		inverseBasisOfSpline = Inverse (basisOfSpline);
	} else {
		inverseBasisOfSpline.SetRow (3, -location);
	}

	// Make relative transform take the spline from its own object space to our object space,
	// and from there into the space defined by its origin and initial direction:
	relativeTransform = relativeTransform * inverseBasisOfSpline;
	// (Note left-to-right evaluation order: Given matrices A,B, point x, x(AB) = (xA)B

	// The first transform is necessarily the identity:
	tFrenets[0].IdentityMatrix ();

	// Set up xDir, yDir, zDir to match our first-point basis:
	xDir = Point3 (1,0,0);
	yDir = Point3 (0,1,0);
	zDir = Point3 (0,0,1);

	for (i=1; i<=numIntervals; i++) {
		position = float(i) / denominator;
		location = relativeTransform * pSpline->InterpCurve3D (position);
		tangent = relativeTransform.VectorTransform (pSpline->TangentCurve3D (position));

		// This is the procedure we follow at each step in the path: find the
		// orthonormal basis with the right orientation, then compose with
		// the translation putting the origin at the path-point.

		// As we proceed along the path, we apply minimal rotations to
		// our original basis to keep the Z-axis tangent to the curve.
		// The X and Y axes follow in a natural manner.

		// xDir, yDir, zDir still have their values from last time...
		// Create a rotation matrix which maps the last zDir onto the current tangent:
		axis = zDir ^ tangent;	// gives axis, scaled by sine of angle.
		sine = FLength(axis);	// positive - keeps angle value in (0,PI) range.
		cosine = DotProd (zDir, tangent);	// Gives cosine of angle.
		theta = atan2f (sine, cosine);
		rotation.SetRotate (AngAxis (Normalize(axis), theta));
		Point3 testVector = rotation * zDir;
		xDir = Normalize (rotation * xDir);
		yDir = Normalize (rotation * yDir);
		zDir = tangent;

		if (i<=numSegs) {
			tFrenets[i].IdentityMatrix ();
			tFrenets[i].SetRow (0, xDir);
			tFrenets[i].SetRow (1, yDir);
			tFrenets[i].SetRow (2, tangent);
			tFrenets[i].SetTrans (location);
		}
	}
}

void PolyOpExtrudeAlongSpline::Do (MNMesh & mesh) {
	if (mpSpline == NULL) return;
	
	Tab<Matrix3> tTransforms;
	theFrenetFinder.ConvertPathToFrenets (mpSpline, mSplineXfm, tTransforms,
		mSegments, mAlign?true:false, mRotation);

	// Apply taper and twist to transforms:
	float denom = float(tTransforms.Count()-1);
	for (int i=1; i<tTransforms.Count(); i++) {
		float amount = float(i)/denom;
		// This equation taken from Taper modifier:
		float taperAmount =	1.0f + amount*mTaper + mTaperCurve*amount*(1.0f-amount);
		if (taperAmount != 1.0f) {
			// Pre-scale matrix by taperAmount.
			tTransforms[i].PreScale (Point3(taperAmount, taperAmount, taperAmount));
		}
		if (mTwist != 0.0f) {
			float twistAmount = mTwist * amount;
			tTransforms[i].PreRotateZ (twistAmount);
		}
	}

	// Note:
	// If there are multiple face clusters, the first call to ExtrudeFaceClusterAlongPath
	// will bring mesh.numf and fClust.clust.Count() out of synch - fClust isn't updated.
	// So we fix that here.
	MNFaceClusters fClust (mesh, MN_USER);
	for (i=0; i<fClust.count; i++) {
		if (mesh.ExtrudeFaceClusterAlongPath (tTransforms, fClust, i, mAlign?true:false)) {
			if (i+1<fClust.count) {
				// New faces not in any cluster.
				int oldnumf = fClust.clust.Count();
				fClust.clust.SetCount (mesh.numf);
				for (int j=oldnumf; j<mesh.numf; j++) fClust.clust[j] = -1;
			}
		}
	}
}

//const USHORT kPolyOperationBasics = 0x0600;
const USHORT kPolyOpExtrudeSpline = 0x0640;
const USHORT kPolyOpExtrudeSplineXfm = 0x0644;

IOResult PolyOpExtrudeAlongSpline::Save (ISave *isave)
{
	isave->BeginChunk (kPolyOperationBasics);
	SaveBasics (isave);
	isave->EndChunk ();

	if (mpSpline != NULL)
	{
		isave->BeginChunk (kPolyOpExtrudeSplineXfm);
		mSplineXfm.Save (isave);
		isave->EndChunk ();

		isave->BeginChunk (kPolyOpExtrudeSpline);
		mpSpline->Save (isave);
		isave->EndChunk ();
	}
	return IO_OK;
}

IOResult PolyOpExtrudeAlongSpline::Load (ILoad *iload)
{
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID ())
		{
		case kPolyOperationBasics:
			res = LoadBasics (iload);
			break;
		case kPolyOpExtrudeSplineXfm:
			res = mSplineXfm.Load (iload);
			break;
		case kPolyOpExtrudeSpline:
			mpSpline = new Spline3D();
			res = mpSpline->Load (iload);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

class PolyOpRetriangulate : public PolyOperation
{
public:
	// Implement PolyOperation methods:
	int OpID () { return kOpRetriangulate; }
	TCHAR * Name () { return GetString (IDS_EP_RETRIANGULATE); }
	// No dialog ID.
	int MeshSelectionLevel() { return MNM_SL_FACE; }
	// No parameters.
	void Do (MNMesh & mesh);
	PolyOperation *Clone () {
		PolyOpRetriangulate *ret = new PolyOpRetriangulate();
		CopyBasics (ret);
		return ret;
	}
	void DeleteThis () { delete this; }
};

void PolyOpRetriangulate::Do (MNMesh & mesh)
{
	int i;
	for (i=0; i<mesh.numf; i++) {
		if (mesh.f[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.f[i].GetFlag (MN_USER)) continue;
		mesh.RetriangulateFace (i);
	}
}

void EditPolyMod::InitializeOperationList ()
{
	if (mOpList.Count() > 0) return;	// already initialized.
	mOpList.SetCount (40);	// overallocate, so we don't have to count carefully.
	int numOps=0;
	mOpList[numOps++] = new PolyOperation ();
	mOpList[numOps++] = new PolyOpWeldVertex ();
	mOpList[numOps++] = new PolyOpChamferVertex ();
	mOpList[numOps++] = new PolyOpExtrudeVertex ();
	mOpList[numOps++] = new PolyOpBreakVertex ();
	mOpList[numOps++] = new PolyOpChamferEdge ();
	mOpList[numOps++] = new PolyOpExtrudeEdge ();
	mOpList[numOps++] = new PolyOpExtrudeFace ();
	mOpList[numOps++] = new PolyOpBevelFace ();
	mOpList[numOps++] = new PolyOpExtrudeAlongSpline ();
	mOpList[numOps++] = new PolyOpRetriangulate ();

	// correct the count:
	mOpList.SetCount (numOps);
	mOpList.Shrink ();
}

void EditPolyMod::ClearOperationList ()
{
	for (int i=0; i<mOpList.Count (); i++)
	{
		mOpList[i]->DeleteThis ();
		mOpList[i] = NULL;
	}
	mOpList.ZeroCount ();
}

