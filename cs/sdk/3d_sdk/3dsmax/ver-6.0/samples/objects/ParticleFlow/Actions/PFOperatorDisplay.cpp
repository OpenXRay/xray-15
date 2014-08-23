/**********************************************************************
 *<
	FILE: PFOperatorDisplay.h

	DESCRIPTION: Viewport/Render Operator implementation
				 The Operator is used to define appearance of the particles
				 in the current Event (or globally if it's a global
				 Operator) for viewports

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

#include "PFOperatorDisplay.h"

#include "PFOperatorDisplay_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IParticleGroup.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Display operator creates a particle channel to store a successive order number
// when a particle appeares in the event. The number is used to determine if
// the particle is visible given the current visibility percent
#define PARTICLECHANNELVISIBLER_INTERFACE Interface_ID(0x2de61303, 0x1eb34500) 
#define PARTICLECHANNELVISIBLEW_INTERFACE Interface_ID(0x2de61303, 0x1eb34501) 
#define GetParticleChannelVisibleRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELVISIBLER_INTERFACE)) 
#define GetParticleChannelVisibleWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELVISIBLEW_INTERFACE)) 

DWORD GetNewObjectColor();

// static members
Object*				PFOperatorDisplay::m_editOb	 = NULL;
IObjParam*			PFOperatorDisplay::m_ip      = NULL;

// constructors/destructors
PFOperatorDisplay::PFOperatorDisplay()
{ 
	RegisterParticleFlowNotification();
	_pblock() = NULL;
	GetClassDesc()->MakeAutoParamBlocks(this); 
	_activeIcon() = _inactiveIcon() = NULL;
	if (pblock() != NULL) // set random initial color
		pblock()->SetValue(kDisplay_color, 0, Color(GetNewObjectColor()));
}

PFOperatorDisplay::~PFOperatorDisplay()
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	DeleteAllRefsFromMe();
	if (wasHolding) theHold.Resume();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From InterfaceServer									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorDisplay::GetInterface(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return (IPFAction*)this;
	if (id == PFOPERATOR_INTERFACE) return (IPFOperator*)this;
	if (id == PFVIEWPORT_INTERFACE) return (IPFViewport*)this;
	if (id == PVIEWITEM_INTERFACE) return (IPViewItem*)this;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFOperatorDisplay::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &display_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &display_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &display_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFOperatorDisplay::DeleteThis()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_DISPLAY_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorDisplay::ClassID()
{
	return PFOperatorDisplay_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PFOperatorDisplay::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PFOperatorDisplay::SubAnimName(int i)
{
	return PFActions::GetString(IDS_PARAMETERS);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PFOperatorDisplay::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorDisplay::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorDisplay::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PFOperatorDisplay::GetReference(int i)
{
	if (i==0) return (RefTargetHandle)pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==0) _pblock() = (IParamBlock2*)rtarg;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefResult PFOperatorDisplay::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	Color newColor;
	DWORD theColor;

	switch (message) {
		case REFMSG_CHANGE:
			if (pblock() == hTarget) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch (lastUpdateID)
				{
				case kDisplay_type:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDisplay_visible:
					if (lastUpdateID == kDisplay_visible) recalcVisibility();
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDisplay_color: 
				case kDisplay_showNumbering:
				case kDisplay_selectedType:
					newColor = pblock()->GetColor(kDisplay_color, 0);
					theColor = newColor.toRGB();
					NotifyDependents( FOREVER, (PartID)theColor, kPFMSG_UpdateWireColor );
					UpdatePViewUI(lastUpdateID);
					return REF_STOP;
				default:
					UpdatePViewUI(lastUpdateID);
				}
			}
			break;
	}	
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFOperatorDisplay::Clone(RemapDir &remap)
{
	PFOperatorDisplay* newOp = new PFOperatorDisplay();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
int PFOperatorDisplay::RemapRefOnLoad(int iref)
{
	return iref;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorDisplay::Save(ISave *isave)
{
	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorDisplay::Load(ILoad *iload)
{
	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFOperatorDisplay::GetObjectName()
{
	return GetString(IDS_OPERATOR_DISPLAY_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFOperatorDisplay::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	_totalParticles(pCont) = 0;
	return true;
}

bool PFOperatorDisplay::Release(IObject* pCont)
{
	return true;
}

const ParticleChannelMask& PFOperatorDisplay::ChannelsUsed(const Interval& time) const
{
								//  read								&	write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_ID|PCU_Position|PCU_Speed|PCU_Shape|PCU_ShapeTexture|PCU_MtlIndex, 0);
	return mask;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFOperator								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFOperatorDisplay::Proceed(IObject* pCont, 
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

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFViewport								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool AssignMeshMaterialIndices(Mesh* mesh, IParticleChannelIntR* chMtlIndex, int particleIndex)
{
	if (chMtlIndex == NULL) return false;
	int mtlIndex = chMtlIndex->GetValue(particleIndex);
	if (mtlIndex >= 0) {
		for(int i=0; i<mesh->getNumFaces(); i++)
			mesh->setFaceMtlIndex(i, mtlIndex);
		return true;
	}
	return false;
}

bool AssignMeshMapping(Mesh* mesh, IParticleChannelMeshMapR* chMapping, int particleIndex)
{
	if (chMapping == NULL) return false;
	bool hasMappingAssigned = false;
	for(int i=0; i<chMapping->GetNumMaps(); i++) {
		if (!chMapping->MapSupport(i)) continue;	
		IParticleChannelMapR* map = chMapping->GetMapReadChannel(i);
		if (map == NULL) continue;
		map->Apply(mesh, particleIndex, i);
		hasMappingAssigned = true;
	}
	return hasMappingAssigned;
}

int PFOperatorDisplay::Display(IObject* pCont,  TimeValue time, Object* pSystem, INode* psNode, INode* pgNode, ViewExp *vpt, int flags)
{
	int displayType = pblock()->GetInt(kDisplay_type);
	int selectedType = pblock()->GetInt(kDisplay_selectedType);
	int showNum = pblock()->GetInt(kDisplay_showNumbering);
	float visPercent = GetPFFloat(pblock(), kDisplay_visible, time);

	DbgAssert(pCont); DbgAssert(pSystem); DbgAssert(psNode); DbgAssert(pgNode);
	if (pCont == NULL) return 0;
	if (pSystem == NULL) return 0;
	if (psNode == NULL) return 0;
	if (pgNode == NULL) return 0;
	IPFSystem* iSystem = PFSystemInterface(pSystem);
	if (iSystem == NULL) return 0; // invalid particle system

	// all particles are visible in 'select particles' mode
	bool subSelPartMode = (psNode->Selected() && (pSystem->GetSubselState() == 1));
	if (subSelPartMode) {
		if (displayType == kDisplay_type_none)
			displayType = kDisplay_type_dots;
		if (selectedType == kDisplay_type_none)
			selectedType = kDisplay_type_dots;
		visPercent = 1.0f;
	}
	if ((displayType == kDisplay_type_none) && (selectedType == kDisplay_type_none))
		return 0;

	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return 0; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return 0;		// there no particles to draw
	int visCount = visPercent * count;
	if (visCount <= 0) return 0;	// there no particles to draw
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);	
	if (chPos == NULL) return 0;	// can't find particle position in the container
	IParticleChannelPoint3R* chSpeed = NULL;
	if ((displayType == kDisplay_type_lines) || (selectedType == kDisplay_type_lines))
	{
		chSpeed = GetParticleChannelSpeedRInterface(pCont);
		// if no speed channel then change drawing type to dots
		if (chSpeed == NULL) {
			if (displayType == kDisplay_type_lines)
				displayType = kDisplay_type_dots;
			if (selectedType == kDisplay_type_lines)
				selectedType = kDisplay_type_dots;
		}
	}
	IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);
	// if no ID channel then it's not possible to show numbers & selections
	if (chID == NULL) showNum = 0;
	
	IParticleChannelQuatR* chOrient = NULL;
	IParticleChannelPoint3R* chScale = NULL;
	IParticleChannelMeshR* chShape = NULL;
	IParticleChannelIntR* chMtlIndex = NULL;
	IParticleChannelMeshMapR* chMapping = NULL;

	if ((displayType == kDisplay_type_boundingBoxes) ||
		(displayType == kDisplay_type_geometry) ||
		(selectedType == kDisplay_type_boundingBoxes) ||
		(selectedType == kDisplay_type_geometry))
	{
		chOrient = GetParticleChannelOrientationRInterface(pCont);
		chScale = GetParticleChannelScaleRInterface(pCont);
		chShape = GetParticleChannelShapeRInterface(pCont);
		// if no shape channel then change drawing type to X marks
		if (chShape == NULL) displayType = kDisplay_type_Xs;
		chMtlIndex = GetParticleChannelMtlIndexRInterface(pCont);
		chMapping = GetParticleChannelShapeTextureRInterface(pCont);
	}

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	IParticleChannelIntR* chVisibleR = (IParticleChannelIntR*)chCont->GetPrivateInterface(PARTICLECHANNELVISIBLER_INTERFACE, (Object*)this);

	int i, j, index, num, numMtls=1;
	float sizeFactor;
	Tab<int> blockNon, blockSel;
	int selNum=0, nonNum=0;
	IParticleChannelBoolR* chSelect = GetParticleChannelSelectionRInterface(pCont);
	blockNon.SetCount(count); blockSel.SetCount(count);
	bool doVisibleFilter = ((visPercent < 1.0f) && (chVisibleR != NULL));
	if (chSelect != NULL) {
		for(i=0; i<count; i++) {
			if (doVisibleFilter)
				if (isInvisible(chVisibleR->GetValue(i))) continue;
			if (chSelect->GetValue(i))
				blockSel[selNum++] = i;
			else
				blockNon[nonNum++] = i;
		}
	} else {
		for(i=0; i<count; i++) {
			if (doVisibleFilter)
				if (isInvisible(chVisibleR->GetValue(i))) continue;
			blockNon[nonNum++] = i;
		}
	}
	blockNon.SetCount(nonNum);
	blockSel.SetCount(selNum);

	GraphicsWindow* gw = vpt->getGW();
	DWORD oldRndLimits, newRndLimits;
	newRndLimits = oldRndLimits = gw->getRndLimits();

	// define color for particles
	bool doPrimColor = false;
	bool doSelColor = false;
	Color primColor, selColor;
	Color subselColor = GetColorManager()->GetColorAsPoint3(PFSubselectionColorId);
	if (psNode->Selected()) {
		switch (pSystem->GetSubselState())
		{
		case 0:
			doPrimColor = doSelColor = ((oldRndLimits & GW_WIREFRAME) != 0);
			primColor = Color(GetSelColor());
			selColor = Color(GetSelColor());
			break;
		case 1: 
			doSelColor = true;
			primColor = pblock()->GetColor(kDisplay_color);
			selColor = Color(subselColor);
			break;
		case 2: {
			primColor = pblock()->GetColor(kDisplay_color);
			selColor = pblock()->GetColor(kDisplay_color);
			IParticleGroup* iPGroup = ParticleGroupInterface(pgNode);
			if (iPGroup != NULL)
				if (iSystem->IsActionListSelected(iPGroup->GetActionList())) {
					doPrimColor = doSelColor = true;
					primColor = Color(subselColor);
					selColor = Color(subselColor);
				}
			}
			break;
		}
	} else if (psNode->IsFrozen()) {
		doPrimColor = doSelColor = true;
		primColor = Color(GetFreezeColor());
		selColor = Color(GetFreezeColor());
	} else {
		primColor = pblock()->GetColor(kDisplay_color);
		selColor = pblock()->GetColor(kDisplay_color);
	}

	Material* nodeMtl = pgNode->Mtls();
	Matrix3 gwTM;
	gwTM.IdentityMatrix();
	gw->setTransform(gwTM);
	Material* curMtl;
	Material vpMtl;
	Point3 pos[33], speed;
	int edgeVis[33];

	enum { noneCat, markerCat, lineCat, geomCat };
	int ordCat, selCat;
	switch (displayType) {
	case kDisplay_type_boundingBoxes:
	case kDisplay_type_geometry:
		ordCat = geomCat;
		break;
	case kDisplay_type_lines:
		ordCat = lineCat;
		break;
	case kDisplay_type_none:
		ordCat = noneCat;
		break;
	default:
		ordCat = markerCat;
	}
	switch (selectedType) {
	case kDisplay_type_boundingBoxes:
	case kDisplay_type_geometry:
		selCat = geomCat;
		break;
	case kDisplay_type_lines:
		selCat = lineCat;
		break;
	case kDisplay_type_none:
		selCat = noneCat;
		break;
	default:
		selCat = markerCat;
	}

	if ((ordCat == geomCat) || (selCat == geomCat))
	{
		DWORD prevLimits = newRndLimits;
		DWORD boxLimits = newRndLimits;
		if (!(boxLimits&GW_BOX_MODE)) boxLimits |= GW_BOX_MODE;

		vpMtl = *(SysUtil::GetParticleMtl());
		for(int runType = 0; runType < 2; runType++)
		{
			Tab<int> *block = NULL;
			if (runType) {
				if (selNum == 0) continue;
				if (selCat != geomCat) continue;
				if (selectedType == kDisplay_type_boundingBoxes)
					gw->setRndLimits(boxLimits);
				else
					gw->setRndLimits(prevLimits);
				vpMtl.Kd = vpMtl.Ks = selColor;
				gw->setColor(LINE_COLOR, selColor);
				if ((nodeMtl != NULL) && (!doSelColor)) {
					curMtl = nodeMtl;
					numMtls = pgNode->NumMtls();
				} else {
					curMtl = &vpMtl;
					if (doSelColor) curMtl->Kd = curMtl->Ks = selColor;
					else curMtl->Kd = curMtl->Ks = pblock()->GetColor(kDisplay_color);
					numMtls = 1;
				}
				block = &blockSel;
			} else {
				if (nonNum == 0) continue;
				if (ordCat != geomCat) continue;
				if (displayType == kDisplay_type_boundingBoxes)
					gw->setRndLimits(boxLimits);
				else
					gw->setRndLimits(prevLimits);
				gw->setColor(LINE_COLOR, primColor);
				if ((nodeMtl != NULL) && (!doPrimColor)) {
					curMtl = nodeMtl;
					numMtls = pgNode->NumMtls();
				} else {
					curMtl = &vpMtl;
					if (doPrimColor) curMtl->Kd = curMtl->Ks = primColor;
					else curMtl->Kd = curMtl->Ks = pblock()->GetColor(kDisplay_color);
					numMtls = 1;
				}
				block = &blockNon;
			}
			for(i=0; i<numMtls; i++) gw->setMaterial(curMtl[i], i);

			for(i=0; i<block->Count(); i++) {
				index = (*block)[i];
				Mesh* pMesh = NULL;
				if (chShape != NULL)
					pMesh = const_cast <Mesh*>(chShape->GetValue(index));
				Mesh curMesh;
				if (pMesh) {
					gwTM.IdentityMatrix();
					if (chOrient) gwTM.SetRotate(chOrient->GetValue(index));
					if (chScale) gwTM.PreScale(chScale->GetValue(index));
					gwTM.SetTrans(chPos->GetValue(index));
					gw->setTransform(gwTM);
					// set vertex color
					switch (psNode->GetVertexColorType()) {
					case nvct_color:
						if (pMesh->curVCChan == 0) break;
						pMesh->setVCDisplayData (0);
						break;
					case nvct_illumination:
						if (pMesh->curVCChan == MAP_SHADING) break;
						pMesh->setVCDisplayData (MAP_SHADING);
						break;
					case nvct_alpha:
						if (pMesh->curVCChan == MAP_ALPHA) break;
						pMesh->setVCDisplayData (MAP_ALPHA);
						break;
					// CAL-06/15/03: add a new option to view map channel as vertex color. (FID #1926)
					case nvct_map_channel:
						if (pMesh->curVCChan == psNode->GetVertexColorMapChannel()) break;
						pMesh->setVCDisplayData (psNode->GetVertexColorMapChannel());
						break;
					}

					// seed SDK mesh->render() method remark for necessity of this call
					if (numMtls > 1) gw->setMaterial(curMtl[0], 0);

					if ((chMtlIndex != NULL) || (chMapping != NULL)) {
						curMesh = *pMesh;
						bool assignedMtlIndices = AssignMeshMaterialIndices(&curMesh, chMtlIndex, index);
						bool assignedMapping = AssignMeshMapping(&curMesh, chMapping, index);
						if (assignedMtlIndices || assignedMapping) 
							pMesh = &curMesh;
						if (assignedMtlIndices) {
							int mtlIndex = chMtlIndex->GetValue(index);
							if (mtlIndex < numMtls)
								gw->setMaterial(curMtl[mtlIndex], mtlIndex);
						}
						pMesh->InvalidateStrips();
					}
					pMesh->render(gw, curMtl, (flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL, numMtls);
				} else {
					pos[0] = chPos->GetValue(index);
					gw->marker(pos, X_MRKR);
				}
			}
		}

		gwTM.IdentityMatrix();
		gw->setTransform(gwTM);
		gw->setRndLimits(prevLimits);
	}

	if ((ordCat == lineCat) || (selCat == lineCat))
	{
		sizeFactor = GetTicksPerFrame();
		for(i=0; i<32; i++)
			edgeVis[i] = (i%2) ? GW_EDGE_SKIP : GW_EDGE_VIS;
		edgeVis[32] = GW_EDGE_SKIP;

		if (displayType == kDisplay_type_lines)
		{
			gw->setColor(LINE_COLOR, primColor);
			j = 0;
			num = blockNon.Count();
			for(index=0; index<num; index++)
			{
				i = blockNon[index];
				pos[j] = chPos->GetValue(i);
				pos[j+1] = pos[j] + sizeFactor*chSpeed->GetValue(i);
				j += 2;
				if (j == 30) {
					gw->polyline(j, pos, NULL, NULL, 0, edgeVis);
					j = 0;
				}
			}
			if (j) gw->polyline(j, pos, NULL, NULL, 0, edgeVis);
		}

		if (selectedType == kDisplay_type_lines)
		{
			gw->setColor(LINE_COLOR, selColor);
			j = 0;
			num = blockSel.Count();
			for(index=0; index<num; index++)
			{
				i = blockSel[index];
				pos[j] = chPos->GetValue(i);
				pos[j+1] = pos[j] + sizeFactor*chSpeed->GetValue(i);
				j += 2;
				if (j == 30) {
					gw->polyline(j, pos, NULL, NULL, 0, edgeVis);
					j = 0;
				}
			}
			if (j) gw->polyline(j, pos, NULL, NULL, 0, edgeVis);
		}
	}

	if ((ordCat == markerCat) || (selCat == markerCat))
	{
		MarkerType mType;
		int num;
		if ((ordCat == markerCat) && (blockNon.Count()>0))
		{
			switch (displayType)
			{
				case kDisplay_type_dots: mType = POINT_MRKR; break;
				case kDisplay_type_ticks: mType = PLUS_SIGN_MRKR; break;
				case kDisplay_type_circles: mType = CIRCLE_MRKR; break;
				case kDisplay_type_diamonds: mType = DIAMOND_MRKR; break;
				case kDisplay_type_boxes: mType = HOLLOW_BOX_MRKR; break;
				case kDisplay_type_asterisks: mType = ASTERISK_MRKR; break;
				case kDisplay_type_triangles: mType = TRIANGLE_MRKR; break;
				case kDisplay_type_Xs: mType = X_MRKR; break;
			}
			gw->setColor(LINE_COLOR, primColor);
			gw->startMarkers();
			num = blockNon.Count();
			for (i=0; i<num; i++) {
				pos[0] = chPos->GetValue(blockNon[i]);
				gw->marker(pos, mType);
			}
			gw->endMarkers();									
		}
		if ((selCat == markerCat) && (blockSel.Count()>0))
		{
			switch (selectedType)
			{
				case kDisplay_type_dots: mType = POINT_MRKR; break;
				case kDisplay_type_ticks: mType = PLUS_SIGN_MRKR; break;
				case kDisplay_type_circles: mType = CIRCLE_MRKR; break;
				case kDisplay_type_diamonds: mType = DIAMOND_MRKR; break;
				case kDisplay_type_boxes: mType = HOLLOW_BOX_MRKR; break;
				case kDisplay_type_asterisks: mType = ASTERISK_MRKR; break;
				case kDisplay_type_triangles: mType = TRIANGLE_MRKR; break;
				case kDisplay_type_Xs: mType = X_MRKR; break;
			}
			gw->setColor(LINE_COLOR, selColor);
			gw->startMarkers();
			num = blockSel.Count();
			for (i=0; i<num; i++) {
				pos[0] = chPos->GetValue(blockSel[i]);
				gw->marker(pos, mType);
			}
			gw->endMarkers();									
		}
	}

	if (showNum)
	{ // show born index along with each particle
		TCHAR numText[16];
		num = blockNon.Count();
		if (num > 0) {
			gw->setColor(TEXT_COLOR, primColor);
			for(i=0; i<num; i++) {
				index = blockNon[i];
				sprintf(numText, "%d", chID->GetParticleBorn(index)+1); // indices shown are 1-based
				pos[0] = chPos->GetValue(index);
				gw->text(pos, numText);
			}
		}
		num = blockSel.Count();
		if (num > 0) {
			gw->setColor(TEXT_COLOR, selColor);
			for(i=0; i<num; i++) {
				index = blockSel[i];
				sprintf(numText, "%d", chID->GetParticleBorn(index)+1); // indices shown are 1-based
				pos[0] = chPos->GetValue(index);
				gw->text(pos, numText);
			}
		}
	}

	return 0;
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
int PFOperatorDisplay::HitTest(IObject* pCont, TimeValue time, Object* pSystem, INode* psNode, INode* pgNode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
	int displayType = pblock()->GetInt(kDisplay_type);
	int selectedType = pblock()->GetInt(kDisplay_selectedType);
	float visPercent = GetPFFloat(pblock(), kDisplay_visible, time);

	DbgAssert(pCont); DbgAssert(pSystem); DbgAssert(psNode); DbgAssert(pgNode);
	if (pCont == NULL) return 0;
	if (pSystem == NULL) return 0;
	if (psNode == NULL) return 0;
	if (pgNode == NULL) return 0;
	IPFSystem* iSystem = PFSystemInterface(pSystem);
	if (iSystem == NULL) return 0; // invalid particle system

	IParticleGroup* iPGroup = ParticleGroupInterface(pgNode);
	INode* theActionListNode = (iPGroup != NULL) ? iPGroup->GetActionList() : NULL;

	BOOL selOnly    = flags&SUBHIT_SELONLY		? TRUE : FALSE;
	BOOL unselOnly  = flags&SUBHIT_UNSELONLY	? TRUE : FALSE;
	BOOL abortOnHit = flags&SUBHIT_ABORTONHIT	? TRUE : FALSE;

	// can't process a situation when selOnly and unselOnly both are TRUE;
	DbgAssert((selOnly && unselOnly) == FALSE);
	if (selOnly && unselOnly) return 0;

	// all particles are visible in 'select particles' mode
	bool subSelPartMode = (psNode->Selected() && (pSystem->GetSubselState() == 1));
	if (subSelPartMode) {
		if (displayType == kDisplay_type_none)
			displayType = kDisplay_type_dots;
		if (selectedType == kDisplay_type_none)
			selectedType = kDisplay_type_dots;
		visPercent = 1.0f;
	}
	if ((displayType == kDisplay_type_none) && (selectedType == kDisplay_type_none))
		return 0;

	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return 0; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return 0;		// there no particles to hit
	int visCount = visPercent * count;
	if (visCount <= 0) return 0;	// there no particles to hit
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);	
	if (chPos == NULL) return 0;	// can't find particle position in the container
	IParticleChannelPoint3R* chSpeed = NULL;
	if ((displayType == kDisplay_type_lines) || (selectedType == kDisplay_type_lines))
	{
		chSpeed = GetParticleChannelSpeedRInterface(pCont);
		// if no speed channel then change drawing type to dots
		if (chSpeed == NULL) {
			if (displayType == kDisplay_type_lines)
				displayType = kDisplay_type_dots;
			if (selectedType == kDisplay_type_lines)
				selectedType = kDisplay_type_dots;
		}
	}
	IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);
	// if no ID channel then it's not possible to process selected/unselected
	if (chID == NULL) {
		selOnly = FALSE; unselOnly = FALSE; 
	}
	
	IParticleChannelQuatR* chOrient = NULL;
	IParticleChannelPoint3R* chScale = NULL;
	IParticleChannelMeshR* chShape = NULL;
	if ((displayType == kDisplay_type_boundingBoxes) ||
		(displayType == kDisplay_type_geometry) ||
		(selectedType == kDisplay_type_boundingBoxes) ||
		(selectedType == kDisplay_type_geometry))
	{
		chOrient = GetParticleChannelOrientationRInterface(pCont);
		chScale = GetParticleChannelScaleRInterface(pCont);
		chShape = GetParticleChannelShapeRInterface(pCont);
		// if no shape channel then change drawing type to X marks
		if (chShape == NULL) displayType = kDisplay_type_Xs;
	}

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	IParticleChannelIntR* chVisibleR = (IParticleChannelIntR*)chCont->GetPrivateInterface(PARTICLECHANNELVISIBLER_INTERFACE, (Object*)this);

	int i, index, num, numMtls;
	float sizeFactor;
	Tab<int> blockNon, blockSel;
	int selNum=0, nonNum=0;
	IParticleChannelBoolR* chSelect = GetParticleChannelSelectionRInterface(pCont);
	blockNon.SetCount(count); blockSel.SetCount(count);
	bool doVisibleFilter = ((visPercent < 1.0f) && (chVisibleR != NULL));
	if (chSelect != NULL) {
		for(i=0; i<count; i++) {
			if (doVisibleFilter)
				if (isInvisible(chVisibleR->GetValue(i))) continue;
			if (chSelect->GetValue(i))
				blockSel[selNum++] = i;
			else
				blockNon[nonNum++] = i;
		}
	} else {
		for(i=0; i<count; i++) {
			if (doVisibleFilter)
				if (isInvisible(chVisibleR->GetValue(i))) continue;
			blockNon[nonNum++] = i;
		}
	}
	blockNon.SetCount(nonNum);
	blockSel.SetCount(selNum);

	if (psNode->Selected()) {

		switch (pSystem->GetSubselState())
		{
		case 0:
			selOnly = FALSE;
			unselOnly = FALSE;
			break;
		case 1:
			chSelect = GetParticleChannelSelectionRInterface(pCont);
			if (chSelect == NULL)
				selOnly = unselOnly = FALSE;
			break;
		case 2: {
			if (iPGroup != NULL) {
				if (iSystem->IsActionListSelected(theActionListNode)) {
					if (unselOnly) return 0; // all particles in the group are selected
					if (selOnly) {
						selOnly = FALSE; // work with all particles
					}
				} else {
					if (selOnly) return 0; // particles in the group are not selected
					if (unselOnly) {
						unselOnly = FALSE; // work with all particles
					}
				}
			} else {
				return 0; // invalid pgroup
			}
				}
				
			break;
		}
	} else {
		selOnly = FALSE;
		unselOnly = FALSE;
	}

	enum { noneCat, markerCat, lineCat, geomCat };
	int ordCat, selCat;
	switch (displayType) {
	case kDisplay_type_boundingBoxes:
	case kDisplay_type_geometry:
		ordCat = geomCat;
		break;
	case kDisplay_type_lines:
		ordCat = lineCat;
		break;
	case kDisplay_type_none:
		ordCat = noneCat;
		break;
	default:
		ordCat = markerCat;
	}
	switch (selectedType) {
	case kDisplay_type_boundingBoxes:
	case kDisplay_type_geometry:
		selCat = geomCat;
		break;
	case kDisplay_type_lines:
		selCat = lineCat;
		break;
	case kDisplay_type_none:
		selCat = noneCat;
		break;
	default:
		selCat = markerCat;
	}

	DWORD savedLimits;
	Matrix3 gwTM;
	Point3 pos[33], speed;
	int res=0;
	HitRegion hr;
	Material* curMtl;
	PFHitData* hitData = NULL;
	bool firstHit = true;
	
	GraphicsWindow* gw = vpt->getGW();
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~ GW_ILLUM);
	gwTM.IdentityMatrix();
	gw->setTransform(gwTM);
	MakeHitRegion(hr, type, crossing, 4, p);
	gw->setHitRegion(&hr);
	gw->clearHitCode();

	if ((ordCat == lineCat) || (selCat == lineCat))
	{
		sizeFactor = GetTicksPerFrame();
	
		num = blockNon.Count();
		if ((displayType == kDisplay_type_lines) && (!selOnly) && (num > 0))
		{
			for(i=0; i<num; i++)
			{
				int index = blockNon[i];
				pos[0] = chPos->GetValue(index);
				pos[1] = pos[0] + sizeFactor*chSpeed->GetValue(index);
				gw->polyline(2, pos, NULL, NULL, 1, NULL);
				if (gw->checkHitCode()) {
					res = TRUE;
					if (firstHit) hitData = new PFHitData(pgNode, theActionListNode);
					vpt->LogHit(psNode, (ModContext*)pSystem, gw->getHitDistance(), chID ? chID->GetParticleBorn(index) : 0, hitData);
					if (firstHit) {
						hitData = NULL;
						firstHit = false;
					}
					gw->clearHitCode();
					if (abortOnHit) goto hasHit;
				}
			}
		}

		num = blockSel.Count();
		if ((selectedType == kDisplay_type_lines) && (!unselOnly) && (num > 0))
		{
			for(i=0; i<num; i++)
			{
				int index = blockSel[i];
				pos[0] = chPos->GetValue(index);
				pos[1] = pos[0] + sizeFactor*chSpeed->GetValue(index);
				gw->polyline(2, pos, NULL, NULL, 1, NULL);
				if (gw->checkHitCode()) {
					res = TRUE;
					if (firstHit) hitData = new PFHitData(pgNode, theActionListNode);
					vpt->LogHit(psNode, (ModContext*)pSystem, gw->getHitDistance(), chID ? chID->GetParticleBorn(index) : 0, hitData);
					if (firstHit) {
						hitData = NULL;
						firstHit = false;
					}
					gw->clearHitCode();
					if (abortOnHit) goto hasHit;
				}
			}
		}
	}

	if ((ordCat == markerCat) || (selCat == markerCat))
	{
		MarkerType mType;
		int num = blockNon.Count();
		if ((ordCat == markerCat) && (num>0) && (!selOnly))
		{
			switch (displayType)
			{
				case kDisplay_type_dots: mType = POINT_MRKR; break;
				case kDisplay_type_ticks: mType = PLUS_SIGN_MRKR; break;
				case kDisplay_type_circles: mType = CIRCLE_MRKR; break;
				case kDisplay_type_diamonds: mType = DIAMOND_MRKR; break;
				case kDisplay_type_boxes: mType = HOLLOW_BOX_MRKR; break;
				case kDisplay_type_asterisks: mType = ASTERISK_MRKR; break;
				case kDisplay_type_triangles: mType = TRIANGLE_MRKR; break;
				case kDisplay_type_Xs: mType = X_MRKR; break;
			}
			for(i=0; i<num; i++)
			{
				int index = blockNon[i];
				pos[0] = chPos->GetValue(index);
				gw->marker(pos, mType);
				if (gw->checkHitCode()) {
					res = TRUE;
					if (firstHit) hitData = new PFHitData(pgNode, theActionListNode);
					vpt->LogHit(psNode, (ModContext*)pSystem, gw->getHitDistance(), chID ? chID->GetParticleBorn(index) : 0, hitData);
					if (firstHit) {
						hitData = NULL;
						firstHit = false;
					}
					gw->clearHitCode();
					if (abortOnHit) goto hasHit;
				}
			}
		}

		num = blockSel.Count();
		if ((selCat == markerCat) && (num>0) && (!unselOnly))
		{
			switch (selectedType)
			{
				case kDisplay_type_dots: mType = POINT_MRKR; break;
				case kDisplay_type_ticks: mType = PLUS_SIGN_MRKR; break;
				case kDisplay_type_circles: mType = CIRCLE_MRKR; break;
				case kDisplay_type_diamonds: mType = DIAMOND_MRKR; break;
				case kDisplay_type_boxes: mType = HOLLOW_BOX_MRKR; break;
				case kDisplay_type_asterisks: mType = ASTERISK_MRKR; break;
				case kDisplay_type_triangles: mType = TRIANGLE_MRKR; break;
				case kDisplay_type_Xs: mType = X_MRKR; break;
			}
			for(i=0; i<num; i++)
			{
				int index = blockSel[i];
				pos[0] = chPos->GetValue(index);
				gw->marker(pos, mType);
				if (gw->checkHitCode()) {
					res = TRUE;
					if (firstHit) hitData = new PFHitData(pgNode, theActionListNode);
					vpt->LogHit(psNode, (ModContext*)pSystem, gw->getHitDistance(), chID ? chID->GetParticleBorn(index) : 0, hitData);
					if (firstHit) {
						hitData = NULL;
						firstHit = false;
					}
					gw->clearHitCode();
					if (abortOnHit) goto hasHit;
				}
			}
		}
	}

	if ((ordCat == geomCat) || (selCat == geomCat))
	{
		DWORD prevLimits = savedLimits;
		DWORD boxLimits = savedLimits;
		if (!(boxLimits&GW_BOX_MODE)) boxLimits |= GW_BOX_MODE;

		curMtl = SysUtil::GetParticleMtl();
		numMtls = 1;
		if (pgNode->Mtls() != NULL) {
			curMtl = pgNode->Mtls();
			numMtls = pgNode->NumMtls();
		}

		for(int runType = 0; runType < 2; runType++)
		{
			Tab<int> *block = NULL;
		
			if (runType) {
				if (unselOnly) continue;
				if (selNum == 0) continue;
				if (selCat != geomCat) continue;
				if (selectedType == kDisplay_type_boundingBoxes)
					gw->setRndLimits(boxLimits);
				else
					gw->setRndLimits(prevLimits);
				block = &blockSel;
			} else {
				if (selOnly) continue;
				if (nonNum == 0) continue;
				if (ordCat != geomCat) continue;
				if (displayType == kDisplay_type_boundingBoxes)
					gw->setRndLimits(boxLimits);
				else
					gw->setRndLimits(prevLimits);
				block = &blockNon;
			}

			for(i=0; i<block->Count(); i++) {
				index = (*block)[i];
				Mesh* pMesh = NULL;
				if (chShape != NULL)
					pMesh = const_cast <Mesh*>(chShape->GetValue(index));
				if (pMesh) {
					gwTM.IdentityMatrix();
					if (chOrient) gwTM.SetRotate(chOrient->GetValue(index));
					if (chScale) gwTM.PreScale(chScale->GetValue(index));
					gwTM.SetTrans(chPos->GetValue(index));
					gw->setTransform(gwTM);
					pMesh->select(gw, curMtl, &hr, TRUE, numMtls);
				} else {
					pos[0] = chPos->GetValue(index);
					gw->marker(pos, X_MRKR);
				}

				if (gw->checkHitCode()) {
					res = TRUE;
					if (firstHit) hitData = new PFHitData(pgNode, theActionListNode); // the extended data information is attached to the hit of the first particle only to save on memory allocations
					vpt->LogHit(psNode, (ModContext*)pSystem, gw->getHitDistance(), chID ? chID->GetParticleBorn(index) : 0, hitData);
					if (firstHit) {
						hitData = NULL;
						firstHit = false;
					}
					gw->clearHitCode();
					if (abortOnHit) goto hasHit;
				}
			}
		}
		gwTM.IdentityMatrix();
		gw->setTransform(gwTM);
		gw->setRndLimits(prevLimits);
	}

hasHit:
	gw->setRndLimits(savedLimits);
	return res;
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::GetWorldBoundBox(IObject* pCont, TimeValue time, Object* pSystem, INode* inode, ViewExp* vp, Box3& box )
{
	int displayType = pblock()->GetInt(kDisplay_type);
	int selectedType = pblock()->GetInt(kDisplay_selectedType);

	// all particles are visible in 'select particles' mode
	if (pSystem == NULL) return;
	bool subSelPartMode = (pSystem->GetSubselState() == 1);
	if (subSelPartMode) {
		if (displayType == kDisplay_type_none)
			displayType = kDisplay_type_dots;
		if (selectedType == kDisplay_type_none)
			selectedType = kDisplay_type_dots;
	}

	if ((displayType == kDisplay_type_none) &&
		(selectedType == kDisplay_type_none)) return; // nothing to draw

	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return; // there no particles to draw
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);	
	if (chPos == NULL) return; // can't find particle position in the container

	box += chPos->GetBoundingBox();
	
	if ((displayType == kDisplay_type_lines) || (selectedType == kDisplay_type_lines))
	{
		IParticleChannelPoint3R* chSpeed = GetParticleChannelSpeedRInterface(pCont);
		if (chSpeed != NULL) 
			box.EnlargeBy(GetTicksPerFrame()*chSpeed->GetMaxLengthValue());
	}
	else if ((displayType == kDisplay_type_boundingBoxes) || 
		     (displayType == kDisplay_type_geometry) ||
			 (selectedType == kDisplay_type_boundingBoxes) || 
		     (selectedType == kDisplay_type_geometry))
	{
		IParticleChannelMeshR* chShape = GetParticleChannelShapeRInterface(pCont);
		if (chShape != NULL)	{
			Box3 meshBox = chShape->GetMaxBoundingBox();
			if (!meshBox.IsEmpty())	{
				Point3 pmin = meshBox.Min();
				Point3 pmax = meshBox.Max();
				Point3 h( max(fabs(pmin.x),fabs(pmax.x)), max(fabs(pmin.y),fabs(pmax.y)), max(fabs(pmin.z),fabs(pmax.z)) );
				float enlargeFactor = Length(h);
				IParticleChannelPoint3R* chScale = GetParticleChannelScaleRInterface(pCont);
				if (chScale)
					enlargeFactor *= chScale->GetMaxLengthValue();
				box.EnlargeBy(enlargeFactor);
			}
		}
	}
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::GetLocalBoundBox(IObject* pCont, TimeValue time, Object* pSystem, INode* inode, ViewExp* vp, Box3& box )
{
	Box3 pbox;
	GetWorldBoundBox(pCont, time, pSystem, inode, vp, pbox);
	if (!pbox.IsEmpty())
		box += pbox*Inverse(inode->GetObjTMBeforeWSM(time));
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
DWORD PFOperatorDisplay::GetWireColor() const
{
	return (pblock()->GetColor(kDisplay_color, 0)).toRGB();
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::SetWireColor(DWORD color)
{
	pblock()->SetValue(kDisplay_color, 0, Color(color));
}

//+--------------------------------------------------------------------------+
//|							From IPFViewport								 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::MaybeEnlargeViewportRect(IObject* pCont, TimeValue time, GraphicsWindow *gw, Rect &rect)
{
	// check out if writing particles IDs
	bool showNumbers = (pblock()->GetInt(kDisplay_showNumbering, time) != 0);
	if (!showNumbers) return;

	// acquire particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return; // there no particles to draw
	IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);	
	if (chID == NULL) return; // can't find particle position in the container
	int maxNumber = 0;
	for(int i=0; i<count; i++) {
		int curNum = chID->GetParticleBorn(i);
		if (curNum > maxNumber) maxNumber = curNum;
	}
	TCHAR dummy[256];
	SIZE size;
	sprintf(dummy, "%d", maxNumber);
	gw->getTextExtents(dummy, &size);
	rect.SetW(rect.w() + size.cx);
	rect.SetY(rect.y() - size.cy);
	rect.SetH(rect.h() + size.cy);
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorDisplay							 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::UpdatePViewUI(int updateID) const
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
//|							From PFOperatorDisplay							 |
//+--------------------------------------------------------------------------+
bool PFOperatorDisplay::isInvisible(int index)
// index is 1-based
{
	if (index <= 0) return false;
	if (index > invisibleParticles().GetSize()) recalcVisibility(index+1023);	
	return (invisibleParticles()[index-1] != 0);
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorDisplay							 |
//+--------------------------------------------------------------------------+
void PFOperatorDisplay::recalcVisibility(int amount)
{
	if (amount == 0) {
		_invisibleParticles().SetSize(0);
		return;
	}
	if (amount < invisibleParticles().GetSize()) {
		_invisibleParticles().SetSize(amount, 1);
		return;
	}
	float visPercent = GetPFFloat(pblock(), kDisplay_visible, 0);
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
HBITMAP PFOperatorDisplay::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DISPLAY_ACTIVEICON),IMAGE_BITMAP,
//									kActionImageWidth, kActionImageHeight, LR_LOADTRANSPARENT|LR_SHARED);
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorDisplay::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DISPLAY_INACTIVEICON),IMAGE_BITMAP,
//									kActionImageWidth, kActionImageHeight, LR_LOADTRANSPARENT|LR_SHARED);
									kActionImageWidth, kActionImageHeight, LR_SHARED);

	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorDisplay::HasDynamicName(TSTR& nameSuffix)
{
	int type	= pblock()->GetInt(kDisplay_type, 0);
	int ids;
	switch(type) {
	case kDisplay_type_none:			ids = IDS_DISPLAYTYPE_NONE;		break;
	case kDisplay_type_dots:			ids = IDS_DISPLAYTYPE_DOTS;		break;
	case kDisplay_type_ticks:			ids = IDS_DISPLAYTYPE_TICKS;	break;
	case kDisplay_type_circles:			ids = IDS_DISPLAYTYPE_CIRCLES;	break;
	case kDisplay_type_lines:			ids = IDS_DISPLAYTYPE_LINES;	break;
	case kDisplay_type_boundingBoxes:	ids = IDS_DISPLAYTYPE_BBOXES;	break;
	case kDisplay_type_geometry:		ids = IDS_DISPLAYTYPE_GEOM;		break;
	case kDisplay_type_diamonds:		ids = IDS_DISPLAYTYPE_DIAMONDS;	break;
	case kDisplay_type_boxes:			ids = IDS_DISPLAYTYPE_BOXES;	break;
	case kDisplay_type_asterisks:		ids = IDS_DISPLAYTYPE_ASTERISKS;break;
	case kDisplay_type_triangles:		ids = IDS_DISPLAYTYPE_TRIANGLES;break;
	}
	nameSuffix = GetString(ids);
	return true;
}

ClassDesc* PFOperatorDisplay::GetClassDesc() const
{
	return GetPFOperatorDisplayDesc();
}

// here are the 256 default object colors
struct ObjectColors { BYTE	r, g, b; }; 
static ObjectColors objectColors[] = {
	// basic colors
		0xFF,0xB9,0xEF, 0xEE,0xFF,0xB9, 0xB9,0xFF,0xFF, 0xFD,0xAA,0xAA,
		0xF9,0x60,0x60, 0xC4,0x1D,0x1D, 0x96,0x07,0x07, 0xFE,0xCD,0xAB,
		0xFA,0x9F,0x61, 0xC5,0x62,0x1E, 0x96,0x42,0x09, 0xFE,0xEE,0xAB,
		0xFA,0xDD,0x61, 0xC5,0xA5,0x1E, 0x96,0x7B,0x09, 0xEE,0xFE,0xAB,
		0xDD,0xFA,0x61, 0xA5,0xC5,0x1E, 0x7E,0x96,0x07, 0xCD,0xFE,0xAB,
		0x9F,0xFA,0x61, 0x62,0xC5,0x1E, 0x44,0x96,0x07, 0xAB,0xFE,0xAB,
		0x61,0xFA,0x61, 0x1E,0xC5,0x1E, 0x07,0x96,0x07, 0xAB,0xFE,0xCD,
		0x61,0xFA,0x9F, 0x1E,0xC5,0x62, 0x07,0x96,0x41, 0xAB,0xFE,0xEE,
		0x61,0xFA,0xDD, 0x1E,0xC5,0xA5, 0x07,0x96,0x7E, 0xAC,0xEF,0xFF,
		0x62,0xDE,0xFB, 0x20,0xA6,0xC5, 0x09,0x7B,0x96, 0xAC,0xCE,0xFF,
		0x62,0xA0,0xFB, 0x20,0x63,0xC5, 0x09,0x44,0x9A, 0xAC,0xAC,0xFF,
		0x62,0x62,0xFB, 0x20,0x20,0xC5, 0x09,0x09,0x98, 0xCD,0xAD,0xFF,
		0x9C,0x62,0xFB, 0x5F,0x20,0xC5, 0x40,0x09,0x98, 0xED,0xAC,0xFF,
		0xDA,0x62,0xFB, 0xA2,0x20,0xC5, 0x79,0x09,0x98, 0xFF,0xAC,0xEF,
		0xFB,0x62,0xDE, 0xC5,0x20,0xA6, 0x9A,0x09,0x7B, 0xFE,0xAB,0xCD,
		0xFA,0x61,0x9F, 0xC5,0x1E,0x62, 0x9D,0x08,0x41, 0x60,0x60,0x60
};

DWORD GetNewObjectColor()
{
	int index = rand()%64 + 1;
	return RGB(objectColors[index].r, objectColors[index].g, objectColors[index].b);
}



} // end of namespace PFActions