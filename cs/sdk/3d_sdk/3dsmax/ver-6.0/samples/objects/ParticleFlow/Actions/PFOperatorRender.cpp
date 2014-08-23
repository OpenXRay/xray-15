/**********************************************************************
 *<
	FILE: PFOperatorRender.h

	DESCRIPTION: Viewport/Render Operator implementation
				 The Operator is used to define appearance of the particles
				 in the current Event (or globally if it's a global
				 Operator) during render

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"
#include "max.h"
#include "iparamm2.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorRender.h"

#include "PFOperatorRender_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IParticleGroup.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Render operator creates a particle channel to store a successive order number
// when a particle appeares in the event. The number is used to determine if
// the particle is visible given the current visibility percent
#define PARTICLECHANNELVISIBLER_INTERFACE Interface_ID(0x6a284215, 0x1eb34500) 
#define PARTICLECHANNELVISIBLEW_INTERFACE Interface_ID(0x6a284215, 0x1eb34501) 
#define GetParticleChannelVisibleRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELVISIBLER_INTERFACE)) 
#define GetParticleChannelVisibleWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELVISIBLEW_INTERFACE)) 

// static members
Object*				PFOperatorRender::m_editOb	 = NULL;
IObjParam*			PFOperatorRender::m_ip      = NULL;

// constructors/destructors
PFOperatorRender::PFOperatorRender()
{ 
	RegisterParticleFlowNotification();
	_pblock() = NULL;
	GetClassDesc()->MakeAutoParamBlocks(this); 
	_activeIcon() = _inactiveIcon() = NULL;
}

PFOperatorRender::~PFOperatorRender()
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	DeleteAllRefsFromMe();
	if (wasHolding) theHold.Resume();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From InterfaceServer									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorRender::GetInterface(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return (IPFAction*)this;
	if (id == PFOPERATOR_INTERFACE) return (IPFOperator*)this;
	if (id == PFRENDER_INTERFACE) return (IPFRender*)this;
	if (id == PVIEWITEM_INTERFACE) return (IPViewItem*)this;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFOperatorRender::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &render_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &render_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &render_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFOperatorRender::DeleteThis()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorRender::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_RENDER_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorRender::ClassID()
{
	return PFOperatorRender_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorRender::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorRender::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PFOperatorRender::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PFOperatorRender::SubAnimName(int i)
{
	return PFActions::GetString(IDS_PARAMETERS);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PFOperatorRender::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorRender::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorRender::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void* PFOperatorRender::GetInterface(ULONG id)
{
	if (id == I_NEWMTLINTERFACE) return (IChkMtlAPI*)this;
	return Object::GetInterface(id);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PFOperatorRender::GetReference(int i)
{
	if (i==0) return (RefTargetHandle)pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFOperatorRender::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==0) _pblock() = (IParamBlock2*)rtarg;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefResult PFOperatorRender::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if (pblock() == hTarget) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kRender_type:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					// the break is omitted on purpose (bayboro 11-20-02)
				case kRender_splitType:
					RefreshUI(kRender_message_type);
					break;
				case kRender_visible:
					recalcVisibility();
					break;
				}
				UpdatePViewUI(lastUpdateID);

				return REF_STOP;
			}
			break;
		case kRender_RefMsg_InitDlg:
			RefreshUI(kRender_message_type,(IParamMap2*)partID);
			return REF_STOP;
	}	
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFOperatorRender::Clone(RemapDir &remap)
{
	PFOperatorRender* newOp = new PFOperatorRender();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
int PFOperatorRender::RemapRefOnLoad(int iref)
{
	return iref;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorRender::Save(ISave *isave)
{
	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorRender::Load(ILoad *iload)
{
	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFOperatorRender::GetObjectName()
{
	return GetString(IDS_OPERATOR_RENDER_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFOperatorRender::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	_totalParticles(pCont) = 0;
	return true;
}

bool PFOperatorRender::Release(IObject* pCont)
{
	return true;
}

const ParticleChannelMask& PFOperatorRender::ChannelsUsed(const Interval& time) const
{
								//  read								&	write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_ID|PCU_Position|PCU_Speed|PCU_Shape|PCU_ShapeTexture|PCU_MtlIndex, 0);
	return mask;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFOperator								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFOperatorRender::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count <= 0) return true; // no particles to worry about
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	if (chNew->IsAllOld()) return true; // everything has been initialized
	
	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	// first try to get the private channel.
	// If it's not possible then create a new one
	bool initVisible = false;
	IParticleChannelIntR* chVisibleR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELVISIBLER_INTERFACE,
													ParticleChannelInt_Class_ID,
													true,	PARTICLECHANNELVISIBLER_INTERFACE,
															PARTICLECHANNELVISIBLEW_INTERFACE,
													true, actionNode, (Object*)this, // no transfer & private
													&initVisible);
	IParticleChannelIntW*chVisibleW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELVISIBLEW_INTERFACE, (Object*)this);
	if ((chVisibleR == NULL) || (chVisibleW == NULL)) return false; // can't find/create Visible channel

	int i;
	int total = _totalParticles(pCont);
	for(i=0; i<count; i++) {
		if (chNew->IsNew(i)) {
			int visIndex = chVisibleR->GetValue(i);
			if (visIndex == 0) {
				total += 1;
				chVisibleW->SetValue(i, total);
			}
		}
	}
	_totalParticles(pCont) = total;
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFRender									 |
//+--------------------------------------------------------------------------+
int PFOperatorRender::HasRenderableGeometry()
{
	int displayType = pblock()->GetInt(kRender_type);
	if (displayType == kRender_type_none) return 0;
	return 1;
}

void SwapIndices(int& ind1, int& ind2)
{
	int temp = ind1;
	ind1 = ind2;
	ind2 = temp;
}

// creates mesh as a bounding box from a given mesh
Mesh* BoxMesh(Mesh* mesh)
{
	static Mesh nullMesh;
	static Mesh boxMesh;
	static bool boxBuilt = false;

	if (mesh == NULL) return &nullMesh;

	if (!boxBuilt) {
		boxMesh.setNumVerts(8);
		boxMesh.setNumFaces(12);

		boxMesh.faces[0].setVerts(0,2,3); boxMesh.faces[0].smGroup = 2; boxMesh.faces[0].flags = 65603;
		boxMesh.faces[1].setVerts(3,1,0); boxMesh.faces[1].smGroup = 2; boxMesh.faces[1].flags = 65603;
		boxMesh.faces[2].setVerts(4,5,7); boxMesh.faces[2].smGroup = 4; boxMesh.faces[2].flags = 67;
		boxMesh.faces[3].setVerts(7,6,4); boxMesh.faces[3].smGroup = 4; boxMesh.faces[3].flags = 67;
		boxMesh.faces[4].setVerts(0,1,5); boxMesh.faces[4].smGroup = 8; boxMesh.faces[4].flags = 262211;
		boxMesh.faces[5].setVerts(5,4,0); boxMesh.faces[5].smGroup = 8; boxMesh.faces[5].flags = 262211;
		boxMesh.faces[6].setVerts(1,3,7); boxMesh.faces[6].smGroup = 16; boxMesh.faces[6].flags = 196675;
		boxMesh.faces[7].setVerts(7,5,1); boxMesh.faces[7].smGroup = 16; boxMesh.faces[7].flags = 196675;
		boxMesh.faces[8].setVerts(3,2,6); boxMesh.faces[8].smGroup = 32; boxMesh.faces[8].flags = 327747;
		boxMesh.faces[9].setVerts(6,7,3); boxMesh.faces[9].smGroup = 32; boxMesh.faces[9].flags = 327747;
		boxMesh.faces[10].setVerts(2,0,4); boxMesh.faces[10].smGroup = 64; boxMesh.faces[10].flags = 131139;
		boxMesh.faces[11].setVerts(4,6,2); boxMesh.faces[11].smGroup = 64; boxMesh.faces[11].flags = 131139;

		boxBuilt = true;
	}

	Box3 box = mesh->getBoundingBox();
	boxMesh.setVert(0, box.pmin);
	boxMesh.setVert(1, Point3(box.pmax.x, box.pmin.y, box.pmin.z));
	boxMesh.setVert(2, Point3(box.pmin.x, box.pmax.y, box.pmin.z));
	boxMesh.setVert(3, Point3(box.pmax.x, box.pmax.y, box.pmin.z));
	boxMesh.setVert(4, Point3(box.pmin.x, box.pmin.y, box.pmax.z));
	boxMesh.setVert(5, Point3(box.pmax.x, box.pmin.y, box.pmax.z));
	boxMesh.setVert(6, Point3(box.pmin.x, box.pmax.y, box.pmax.z));
	boxMesh.setVert(7, box.pmax);

	return &boxMesh;
}

//+--------------------------------------------------------------------------+
//|							From IPFRender									 |
//+--------------------------------------------------------------------------+
Mesh* PFOperatorRender::GetRenderMesh(IObject* pCont, TimeValue time, Object* pSystem, INode *inode, View& view, BOOL& needDelete)
{
	static Mesh nullMesh;
	needDelete = false;
	_faceToParticle().ZeroCount();

	int displayType = pblock()->GetInt(kRender_type);
	if ((displayType == kRender_type_none) || (displayType == kRender_type_phantom))
		return &nullMesh;
	
	float visPercent = GetPFFloat(pblock(), kRender_visible, time);
	if (visPercent <= 0.0f) return &nullMesh;
	
	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return &nullMesh; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return &nullMesh;		// there no particles to render
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);	
	if (chPos == NULL) return &nullMesh;	// can't find particle position in the container
	IParticleChannelMeshR* chShape = GetParticleChannelShapeRInterface(pCont);
	if (chShape == NULL) return &nullMesh; // can't get particle shape in the container
	IParticleChannelQuatR* chOrient = GetParticleChannelOrientationRInterface(pCont);
	IParticleChannelPoint3R* chScale = GetParticleChannelScaleRInterface(pCont);
	IParticleChannelIntR* chMtlIndex = GetParticleChannelMtlIndexRInterface(pCont);
	IParticleChannelMeshMapR* chMapping = GetParticleChannelShapeTextureRInterface(pCont);

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	IParticleChannelIntR* chVisibleR = (IParticleChannelIntR*)chCont->GetPrivateInterface(PARTICLECHANNELVISIBLER_INTERFACE, (Object*)this);
	bool doVisibleFilter = ((visPercent < 1.0f) && (chVisibleR != NULL));

	int i, vertNum, faceNum;
	vertNum = faceNum = 0;
	for(i=0; i<count; i++)
	{
		if (doVisibleFilter)
			if (isInvisible(chVisibleR->GetValue(i))) continue;
		int curVertNum, curFaceNum;
		if (getParticleNumData(chShape, i, displayType, curVertNum, curFaceNum)) {
			int nextVertNum = vertNum + curVertNum;
			int nextFaceNum = faceNum + curFaceNum;
			if ((nextVertNum > kRender_vertCount_limit) ||
				(nextFaceNum > kRender_faceCount_limit)) {
				count = i; break;
			}
			vertNum = nextVertNum;
			faceNum = nextFaceNum;
		}
	}

	if (vertNum == 0) return &nullMesh; // nothing to render

	Mesh* renderMesh = new Mesh();
	needDelete = true;
	if (!renderMesh->setNumVerts(vertNum)) return &nullMesh;
	if (!renderMesh->setNumFaces(faceNum)) return &nullMesh;
	_faceToParticle().SetCount(faceNum);
	int vertOffset=0, faceOffset=0;
	int tvertOffset[MAX_MESHMAPS];
	for(i=0; i<MAX_MESHMAPS; i++) tvertOffset[i] = 0;

	Matrix3 meshTM, nodeTM, inverseTM, totalTM;
	nodeTM = inode->GetObjectTM(time);
	inverseTM = Inverse(nodeTM);

	TVFace tvFace;
	tvFace.setTVerts(0, 0, 0);
	Mesh* pMesh;
	BOOL curNeedDelete;
	int j, k, mp, curIndex, curNumVerts, curNumFaces;
	for(i=0; i<count; i++)
	{
		if (doVisibleFilter)
			if (isInvisible(chVisibleR->GetValue(i))) continue;
		pMesh = getParticleMesh(chShape, chMtlIndex, chMapping, i, displayType, curNeedDelete);
		curNumVerts = pMesh->getNumVerts();
		if (curNumVerts == 0) { // empty mesh
			if (curNeedDelete) {
				pMesh->FreeAll();
				delete pMesh;
			}
			pMesh = NULL;
			continue; // empty mesh
		}
		curNumFaces = pMesh->getNumFaces();
		meshTM.IdentityMatrix();
		if (chOrient) meshTM.SetRotate(chOrient->GetValue(i));
		if (chScale) meshTM.PreScale(chScale->GetValue(i));
		meshTM.SetTrans(chPos->GetValue(i));
		totalTM = meshTM*inverseTM;

		for(j=0, curIndex=vertOffset; j<curNumVerts; j++, curIndex++)
			renderMesh->verts[curIndex] = pMesh->verts[j]*totalTM;
		for(j=0, curIndex=faceOffset; j<curNumFaces; j++, curIndex++) {
			renderMesh->faces[curIndex] = pMesh->faces[j];
			for(k=0; k<3; k++)
				renderMesh->faces[curIndex].v[k] += vertOffset;
			_faceToParticle(curIndex) = i;
		}

		int numMaps = pMesh->getNumMaps();
		for(mp=0; mp<numMaps; mp++) {
			int tvertsToAdd = pMesh->mapSupport(mp) ? pMesh->getNumMapVerts(mp) : 0;
			if (tvertsToAdd == 0) continue;
			if (tvertOffset[mp] == 0) { // the map channel needs expansion
				renderMesh->setMapSupport(mp, TRUE);
				renderMesh->setNumMapVerts(mp, 3*faceNum+1); // triple number of faces covers the maximum + one extra
				renderMesh->setMapVert(mp, 0, Point3::Origin);
				renderMesh->setNumMapFaces(mp, faceNum);
				for(j=0; j<faceOffset; j++)
					renderMesh->mapFaces(mp)[j] = tvFace;
				tvertOffset[mp] = 1;
			}
			// verify that the tverts array is in proper array range
			if (tvertOffset[mp] + tvertsToAdd > renderMesh->getNumMapVerts(mp))
				renderMesh->setNumMapVerts(mp, tvertOffset[mp] + tvertsToAdd, TRUE);
			for(j=0, curIndex=tvertOffset[mp]; j<tvertsToAdd; j++, curIndex++)
				renderMesh->setMapVert(mp, curIndex, pMesh->mapVerts(mp)[j] );
			for(j=0, curIndex=faceOffset; j<curNumFaces; j++, curIndex++) {
				renderMesh->mapFaces(mp)[curIndex] = pMesh->mapFaces(mp)[j];
				for(k=0; k<3; k++)
					renderMesh->mapFaces(mp)[curIndex].t[k] += tvertOffset[mp];
			}
			tvertOffset[mp] += tvertsToAdd;
		}
		// release the current mesh
		if (curNeedDelete) {
			pMesh->FreeAll();
			delete pMesh;
		}
		pMesh = NULL;

		vertOffset += curNumVerts;
		faceOffset += curNumFaces;
	}
	
	// adjust the texture vert arrays to the actual size
	for(mp=0; mp<renderMesh->getNumMaps(); mp++) {
		if (tvertOffset[mp])
			renderMesh->setNumMapVerts(mp, tvertOffset[mp], TRUE); // TRUE to keep previous values	
	}

	switch (inode->GetVertexColorType()) {
	case nvct_color:
		if (renderMesh->curVCChan == 0) break;
		renderMesh->setVCDisplayData (0);
		break;
	case nvct_illumination:
		if (renderMesh->curVCChan == MAP_SHADING) break;
		renderMesh->setVCDisplayData (MAP_SHADING);
		break;
	case nvct_alpha:
		if (renderMesh->curVCChan == MAP_ALPHA) break;
		renderMesh->setVCDisplayData (MAP_ALPHA);
		break;
	// CAL-06/15/03: add a new option to view map channel as vertex color. (FID #1926)
	case nvct_map_channel:
		if (renderMesh->curVCChan == inode->GetVertexColorMapChannel()) break;
		renderMesh->setVCDisplayData (inode->GetVertexColorMapChannel());
		break;
	}

	return renderMesh;
}

//+--------------------------------------------------------------------------+
//|							From IPFRender									 |
//+--------------------------------------------------------------------------+
int PFOperatorRender::NumberOfRenderMeshes(IObject* pCont, TimeValue t, Object* pSystem)
// returns 0 if doesn't want to be called via GetMultipleRenderMesh
{
	int displayType = pblock()->GetInt(kRender_type);
	if ((displayType == kRender_type_none) || (displayType == kRender_type_phantom)) return 0;

	int splitType = pblock()->GetInt(kRender_splitType);
	if (splitType == kRender_splitType_single) return 0; // use GetRenderMesh instead of GetMultipleRenderMesh

	if (splitType == kRender_splitType_multiple)
		return pblock()->GetInt(kRender_meshCount, t);

	// mesh per particle
	float visPercent = GetPFFloat(pblock(), kRender_visible, t);
	// find number of particles
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return 0; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return 1;		// there no particles to render; will return single empty mesh
	int visCount = visPercent * count;
	if (visCount <= 0) visCount = 1; // there no particles to render; will return single empty mesh
	return visCount;
}

//+--------------------------------------------------------------------------+
//|							From IPFRender									 |
//+--------------------------------------------------------------------------+
Mesh* PFOperatorRender::GetMultipleRenderMesh(IObject* pCont, TimeValue t, Object* pSystem, INode *inode, 
							View& view, BOOL& needDelete, int meshNumber)
{
	static Mesh nullMesh;
	needDelete = false;
	_faceToParticle().ZeroCount();

	int displayType = pblock()->GetInt(kRender_type);
	if ((displayType == kRender_type_none) || (displayType == kRender_type_phantom))
		return &nullMesh;

	int splitType = pblock()->GetInt(kRender_splitType);
	DbgAssert(splitType != kRender_splitType_single); // GetRenderMesh should be used instead of GetMultipleRenderMesh

	// acquire particle count
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return &nullMesh; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return &nullMesh;		// there no particles to render

	int numMeshes = pblock()->GetInt(kRender_meshCount, t);
	int perMesh = pblock()->GetInt(kRender_perMeshCount, t);
	if (splitType == kRender_splitType_multiple)
		if (numMeshes == count)
			splitType = kRender_splitType_particle; // if match then switch to mesh per particle
	if (splitType == kRender_splitType_particle)
		perMesh = 1;
	bool doTransform = (splitType != kRender_splitType_particle);

	int fromP = meshNumber*perMesh;
	int toP = fromP + perMesh;
	if (fromP >= count) return &nullMesh;
	if (toP > count) toP = count;

	float visPercent = GetPFFloat(pblock(), kRender_visible, t); DbgAssert(visPercent > 0.0f);
	if (visPercent <= 0.0f) return &nullMesh;

	// acquire particle channels
	IParticleChannelMeshR* chShape = GetParticleChannelShapeRInterface(pCont);
	if (chShape == NULL) return &nullMesh; // can't get particle shape in the container
	IParticleChannelPoint3R* chPos = NULL;
	IParticleChannelQuatR* chOrient = NULL;
	IParticleChannelPoint3R* chScale = NULL;
	if (doTransform) {
		chPos = GetParticleChannelPositionRInterface(pCont);	
		if (chPos == NULL) doTransform = false;
		chOrient = GetParticleChannelOrientationRInterface(pCont);
		chScale = GetParticleChannelScaleRInterface(pCont);
	}
	IParticleChannelIntR* chMtlIndex = GetParticleChannelMtlIndexRInterface(pCont);
	IParticleChannelMeshMapR* chMapping = GetParticleChannelShapeTextureRInterface(pCont);

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	IParticleChannelIntR* chVisibleR = (IParticleChannelIntR*)chCont->GetPrivateInterface(PARTICLECHANNELVISIBLER_INTERFACE, (Object*)this);
	bool doVisibleFilter = ((visPercent < 1.0f) && (chVisibleR != NULL));

	int i, vertNum, faceNum;
	vertNum = faceNum = 0;
	for(i=fromP; i<toP; i++)
	{
		if (doVisibleFilter)
			if (isInvisible(chVisibleR->GetValue(i))) continue;
		int curVertNum, curFaceNum;
		if (getParticleNumData(chShape, i, displayType, curVertNum, curFaceNum)) {
			int nextVertNum = vertNum + curVertNum;
			int nextFaceNum = faceNum + curFaceNum;
			if ((nextVertNum > kRender_vertCount_limit) ||
				(nextFaceNum > kRender_faceCount_limit)) {
				toP = i; break;
			}
			vertNum = nextVertNum;
			faceNum = nextFaceNum;
		}
	}

	if (vertNum == 0) return &nullMesh; // nothing to render

	Mesh* renderMesh = new Mesh();
	needDelete = true;
	if (!renderMesh->setNumVerts(vertNum)) return &nullMesh;
	if (!renderMesh->setNumFaces(faceNum)) return &nullMesh;
	_faceToParticle().SetCount(faceNum);
	int vertOffset=0, faceOffset=0;
	int tvertOffset[MAX_MESHMAPS];
	for(i=0; i<MAX_MESHMAPS; i++) tvertOffset[i] = 0;

	Matrix3 meshTM, nodeTM, inverseTM, totalTM;
	nodeTM = inode->GetObjectTM(t);
	inverseTM = Inverse(nodeTM);

	TVFace tvFace;
	tvFace.setTVerts(0, 0, 0);
	Mesh* pMesh;
	BOOL curNeedDelete;
	int j, k, mp, curIndex, curNumVerts, curNumFaces;
	for(i=fromP; i<toP; i++)
	{
		if (doVisibleFilter)
			if (isInvisible(chVisibleR->GetValue(i))) continue;
		pMesh = getParticleMesh(chShape, chMtlIndex, chMapping, i, displayType, curNeedDelete);
		curNumVerts = pMesh->getNumVerts();
		if (curNumVerts == 0) { // empty mesh
			if (curNeedDelete) {
				pMesh->FreeAll();
				delete pMesh;
			}
			pMesh = NULL;
			continue; // empty mesh
		}
		if (doTransform) {
			meshTM.IdentityMatrix();
			if (chOrient) meshTM.SetRotate(chOrient->GetValue(i));
			if (chScale) meshTM.PreScale(chScale->GetValue(i));
			meshTM.SetTrans(chPos->GetValue(i));
			totalTM = meshTM*inverseTM;
			for(j=0, curIndex=vertOffset; j<curNumVerts; j++, curIndex++)
				renderMesh->verts[curIndex] = pMesh->verts[j]*totalTM;
		} else {
			for(j=0, curIndex=vertOffset; j<curNumVerts; j++, curIndex++)
				renderMesh->verts[curIndex] = pMesh->verts[j];
		}
		curNumFaces = pMesh->getNumFaces();
		for(j=0, curIndex=faceOffset; j<curNumFaces; j++, curIndex++) {
			renderMesh->faces[curIndex] = pMesh->faces[j];
			for(k=0; k<3; k++)
				renderMesh->faces[curIndex].v[k] += vertOffset;
			_faceToParticle(curIndex) = i;
		}

		int numMaps = pMesh->getNumMaps();
		for(mp=0; mp<numMaps; mp++) {
			int tvertsToAdd = pMesh->mapSupport(mp) ? pMesh->getNumMapVerts(mp) : 0;
			if (tvertsToAdd == 0) continue;
			if (tvertOffset[mp] == 0) { // the map channel needs expansion
				renderMesh->setMapSupport(mp, TRUE);
				renderMesh->setNumMapVerts(mp, 3*faceNum+1); // triple number of faces covers the maximum + one extra
				renderMesh->setMapVert(mp, 0, Point3::Origin);
				renderMesh->setNumMapFaces(mp, faceNum);
				for(j=0; j<faceOffset; j++)
					renderMesh->mapFaces(mp)[j] = tvFace;
				tvertOffset[mp] = 1;
			}
			// verify that the tverts array is in proper array range
			if (tvertOffset[mp] + tvertsToAdd > renderMesh->getNumMapVerts(mp))
				renderMesh->setNumMapVerts(mp, tvertOffset[mp] + tvertsToAdd, TRUE);
			for(j=0, curIndex=tvertOffset[mp]; j<tvertsToAdd; j++, curIndex++)
				renderMesh->setMapVert(mp, curIndex, pMesh->mapVerts(mp)[j] );
			for(j=0, curIndex=faceOffset; j<curNumFaces; j++, curIndex++) {
				renderMesh->mapFaces(mp)[curIndex] = pMesh->mapFaces(mp)[j];
				for(k=0; k<3; k++)
					renderMesh->mapFaces(mp)[curIndex].t[k] += tvertOffset[mp];
			}
			tvertOffset[mp] += tvertsToAdd;
		}
		// release the current mesh
		if (curNeedDelete) {
			pMesh->FreeAll();
			delete pMesh;
		}
		pMesh = NULL;

		vertOffset += curNumVerts;
		faceOffset += curNumFaces;
	}
	
	// adjust the texture vert arrays to the actual size
	for(mp=0; mp<renderMesh->getNumMaps(); mp++) {
		if (tvertOffset[mp])
			renderMesh->setNumMapVerts(mp, tvertOffset[mp], TRUE); // TRUE to keep previous values	
	}

	switch (inode->GetVertexColorType()) {
	case nvct_color:
		if (renderMesh->curVCChan == 0) break;
		renderMesh->setVCDisplayData (0);
		break;
	case nvct_illumination:
		if (renderMesh->curVCChan == MAP_SHADING) break;
		renderMesh->setVCDisplayData (MAP_SHADING);
		break;
	case nvct_alpha:
		if (renderMesh->curVCChan == MAP_ALPHA) break;
		renderMesh->setVCDisplayData (MAP_ALPHA);
		break;
	// CAL-06/15/03: add a new option to view map channel as vertex color. (FID #1926)
	case nvct_map_channel:
		if (renderMesh->curVCChan == inode->GetVertexColorMapChannel()) break;
		renderMesh->setVCDisplayData (inode->GetVertexColorMapChannel());
		break;
	}

	return renderMesh;
}

//+--------------------------------------------------------------------------+
//|							From IPFRender									 |
//+--------------------------------------------------------------------------+
void PFOperatorRender::GetMultipleRenderMeshTM(IObject* pCont, TimeValue t, Object* pSystem, INode *inode, 
							 View& view, int meshNumber, Matrix3& meshTM, Interval& meshTMValid)
{
	int splitType = pblock()->GetInt(kRender_splitType);
	DbgAssert(splitType != kRender_splitType_single); // GetRenderMesh should be used instead of GetMultipleRenderMesh
	meshTMValid.SetInstant(t);

	// acquire particle count
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return;		// there no particles to render

	int numMeshes = pblock()->GetInt(kRender_meshCount, t);
	int perMesh = pblock()->GetInt(kRender_perMeshCount, t);
	if (splitType == kRender_splitType_multiple) {
		if (numMeshes == count)
			splitType = kRender_splitType_particle; // if match then switch to mesh per particle
	}

	meshTM.IdentityMatrix();
	if (splitType == kRender_splitType_particle) {
		if (meshNumber >= count) return;
		// acquire particle channels
		IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);	
		if (chPos == NULL) return;	// can't find particle position in the container
		IParticleChannelQuatR* chOrient = GetParticleChannelOrientationRInterface(pCont);
		IParticleChannelPoint3R* chScale = GetParticleChannelScaleRInterface(pCont);

		if (chOrient) meshTM.SetRotate(chOrient->GetValue(meshNumber));
		if (chScale) meshTM.PreScale(chScale->GetValue(meshNumber));
		meshTM.SetTrans(chPos->GetValue(meshNumber));
		meshTM = meshTM*Inverse(inode->GetObjectTM(t));
	}
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorRender							 |
//+--------------------------------------------------------------------------+
void PFOperatorRender::RefreshUI(WPARAM message, IParamMap2* map) const
{
	HWND hWnd;
	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
		}
		return;
	}

	if ( pblock() == NULL ) return;

	map = pblock()->GetMap();
	if ( (map == NULL) && (NumPViewParamMaps() == 0) ) return;

	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
		}
	}
	for(int i=0; i<NumPViewParamMaps(); i++) {
		hWnd = GetPViewParamMap(i)->GetHWnd();
		SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
	}
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorRender							 |
//+--------------------------------------------------------------------------+
void PFOperatorRender::UpdatePViewUI(int updateID) const
{
	for(int i=0; i<NumPViewParamMaps(); i++) {
		IParamMap2* map = GetPViewParamMap(i);
		map->Invalidate(updateID);
		Interval currentTimeInterval;
		currentTimeInterval.SetInstant(GetCOREInterface()->GetTime());
		map->Validity() = currentTimeInterval;
	}
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorRender							 |
//+--------------------------------------------------------------------------+
bool PFOperatorRender::isInvisible(int index)
// index is 1-based
{
	if (index <= 0) return false;
	if (index > invisibleParticles().GetSize()) recalcVisibility(index+1023);	
	return (invisibleParticles()[index-1] != 0);
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorRender							 |
//+--------------------------------------------------------------------------+
void PFOperatorRender::recalcVisibility(int amount)
{
	if (amount == 0) {
		_invisibleParticles().SetSize(0);
		return;
	}
	if (amount < invisibleParticles().GetSize()) {
		_invisibleParticles().SetSize(amount, 1);
		return;
	}
	float visPercent = GetPFFloat(pblock(), kRender_visible, 0);
	DbgAssert(visPercent < 1.0f);

	int oldNum = invisibleParticles().GetSize();
	int visible = oldNum - _invisibleParticles().NumberSet();
	_invisibleParticles().SetSize(amount, 1);
	if (oldNum == 0) {
		visible = 1;
		_invisibleParticles().Clear(0);
		oldNum = 1;
	}
	for(int i=oldNum; i<amount; i++) {
		if (float(visible)/i > visPercent) {
			_invisibleParticles().Set(i);
		} else {
			_invisibleParticles().Clear(i);
			visible++;
		}
	}
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorRender::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RENDER_ACTIVEICON),IMAGE_BITMAP,
//									kActionImageWidth, kActionImageHeight, LR_LOADTRANSPARENT|LR_SHARED);
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorRender::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_RENDER_INACTIVEICON),IMAGE_BITMAP,
//									kActionImageWidth, kActionImageHeight, LR_LOADTRANSPARENT|LR_SHARED);
									kActionImageWidth, kActionImageHeight, LR_SHARED);

	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorRender::HasDynamicName(TSTR& nameSuffix)
{
	int type	= pblock()->GetInt(kRender_type, 0);
	int ids;
	switch(type) {
	case kRender_type_none:			ids = IDS_DISPLAYTYPE_NONE;		break;
	case kRender_type_boundingBoxes:ids = IDS_DISPLAYTYPE_BBOXES;	break;
	case kRender_type_geometry:		ids = IDS_DISPLAYTYPE_GEOM;		break;
	case kRender_type_phantom:		ids = IDS_DISPLAYTYPE_PHANTOM;	break;
	}
	nameSuffix = GetString(ids);
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IChkMtlAPI									 |
//+--------------------------------------------------------------------------+
BOOL PFOperatorRender::SupportsParticleIDbyFace()
{
	return TRUE;
}

//+--------------------------------------------------------------------------+
//|							From IChkMtlAPI									 |
//+--------------------------------------------------------------------------+
int  PFOperatorRender::GetParticleFromFace(int faceID)
{
	if (faceID < faceToParticle().Count())
		return faceToParticle(faceID);
	return 0;
}

ClassDesc* PFOperatorRender::GetClassDesc() const
{
	return GetPFOperatorRenderDesc();
}

bool PFOperatorRender::getParticleNumData(IParticleChannelMeshR* chShape,
										  int particleIndex,
										  int displayType,
										  int& vertNum, int& faceNum)
{
	Mesh* pMesh = const_cast <Mesh*>(chShape->GetValue(particleIndex));
	if (pMesh == NULL) return false; // mesh is underfined
	if (displayType == kRender_type_boundingBoxes) {
		vertNum = 8;
		faceNum = 12;
	} else {
		vertNum = pMesh->getNumVerts();
		faceNum = pMesh->getNumFaces();
	}
	return true;
}

Mesh* PFOperatorRender::getParticleMesh(IParticleChannelMeshR* chShape,
										IParticleChannelIntR* chMtlIndex,
										IParticleChannelMeshMapR* chMapping,
										int particleIndex, int displayType, BOOL& needDelete)
{
	static Mesh nullMesh;
	needDelete = false;
	Mesh* pMesh = const_cast <Mesh*>(chShape->GetValue(particleIndex));
	if (pMesh == NULL) return &nullMesh;

	if ((displayType == kRender_type_geometry) && (chMapping == NULL) && (chMtlIndex == NULL))
		return pMesh;

	Mesh* renderMesh = new Mesh();
	needDelete = true;
	if (displayType == kRender_type_boundingBoxes)
		renderMesh->DeepCopy(BoxMesh(pMesh), GEOM_CHANNEL|TOPO_CHANNEL);
	else
		renderMesh->DeepCopy(pMesh, ALL_CHANNELS);
	if (chMapping != NULL) {
		for(int i=0; i<chMapping->GetNumMaps(); i++) {
			if (chMapping->MapSupport(i)) {
				IParticleChannelMapR* map = chMapping->GetMapReadChannel(i);
				if (map != NULL) map->Apply(renderMesh, particleIndex, i);
			}
		}
	}
	if (chMtlIndex != NULL)
		ApplyMtlIndex(renderMesh, chMtlIndex->GetValue(particleIndex));

	return renderMesh;
}


} // end of namespace PFActions