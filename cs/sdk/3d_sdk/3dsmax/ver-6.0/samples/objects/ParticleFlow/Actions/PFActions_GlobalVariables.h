/**********************************************************************
 *<
	FILE:			PFActions_GlobalVariables.h

	DESCRIPTION:	Collection of global variables and constants (declaration).
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFACTIONS_GLOBALVARIABLES_H_
#define _PFACTIONS_GLOBALVARIABLES_H_

#include "max.h"

#include "PFOperatorDisplayDesc.h"
#include "PFOperatorRenderDesc.h"
#include "PFOperatorSimpleBirthDesc.h"
#include "PFOperatorSimplePositionDesc.h"
#include "PFOperatorSimpleSpeedDesc.h"
#include "PFOperatorForceSpaceWarpDesc.h"
#include "PFOperatorSimpleOrientationDesc.h"
#include "PFOperatorSimpleSpinDesc.h"
#include "PFOperatorSimpleShapeDesc.h"
#include "PFOperatorSimpleMappingDesc.h"
#include "PFOperatorInstanceShapeDesc.h"
#include "PFOperatorFacingShapeDesc.h"
#include "PFOperatorMarkShapeDesc.h"
#include "PFOperatorExitDesc.h"
#include "PFOperatorMaterialStaticDesc.h"
#include "PFOperatorMaterialDynamicDesc.h"
#include "PFOperatorMaterialFrequencyDesc.h"
#include "PFOperatorPositionOnObjectDesc.h"
#include "PFOperatorSpeedSurfaceNormalsDesc.h"
#include "PFOperatorSpeedCopyDesc.h"
#include "PFOperatorSpeedKeepApartDesc.h"
#include "PFOperatorCommentsDesc.h"
#include "PFOperatorSimpleScaleDesc.h"
#include "PFTestDurationDesc.h"
#include "PFTestSpawnDesc.h"
#include "PFTestSpawnOnCollisionDesc.h"
#include "PFTestCollisionSpaceWarpDesc.h"
#include "PFTestSpeedGoToTargetDesc.h"
#include "PFTestGoToNextEventDesc.h"
#include "PFTestSplitByAmountDesc.h"
#include "PFTestSplitBySourceDesc.h"
#include "PFTestSplitSelectedDesc.h"
#include "PFTestGoToRotationDesc.h"
#include "PFTestScaleDesc.h"
#include "PFTestSpeedDesc.h"

namespace PFActions {

extern HINSTANCE hInstance;

extern PFOperatorDisplayDesc				ThePFOperatorDisplayDesc;
extern PFOperatorRenderDesc					ThePFOperatorRenderDesc;
extern PFOperatorSimpleBirthDesc			ThePFOperatorSimpleBirthDesc;
extern PFOperatorSimplePositionDesc			ThePFOperatorSimplePositionDesc;
extern PFOperatorSimpleShapeDesc			ThePFOperatorSimpleShapeDesc;
extern PFOperatorInstanceShapeDesc			ThePFOperatorInstanceShapeDesc;
extern PFOperatorFacingShapeDesc			ThePFOperatorFacingShapeDesc;
extern PFOperatorMarkShapeDesc				ThePFOperatorMarkShapeDesc;
extern PFOperatorSimpleSpeedDesc			ThePFOperatorSimpleSpeedDesc;
extern PFOperatorSimpleOrientationDesc		ThePFOperatorSimpleOrientationDesc;
extern PFOperatorSimpleSpinDesc				ThePFOperatorSimpleSpinDesc;
extern PFOperatorSimpleMappingDesc			ThePFOperatorSimpleMappingDesc;
extern PFOperatorForceSpaceWarpDesc			ThePFOperatorForceSpaceWarpDesc;
extern PFOperatorExitDesc					ThePFOperatorExitDesc;
extern PFOperatorMaterialStaticDesc			ThePFOperatorMaterialStaticDesc;
extern PFOperatorMaterialDynamicDesc		ThePFOperatorMaterialDynamicDesc;
extern PFOperatorMaterialFrequencyDesc		ThePFOperatorMaterialFrequencyDesc;
extern PFOperatorPositionOnObjectDesc		ThePFOperatorPositionOnObjectDesc;
extern PFOperatorSpeedSurfaceNormalsDesc	ThePFOperatorSpeedSurfaceNormalsDesc;
extern PFOperatorSpeedCopyDesc				ThePFOperatorSpeedCopyDesc;
extern PFOperatorSpeedKeepApartDesc			ThePFOperatorSpeedKeepApartDesc;
extern PFOperatorCommentsDesc				ThePFOperatorCommentsDesc;
extern PFOperatorSimpleScaleDesc			ThePFOperatorSimpleScaleDesc;

extern PFTestDurationDesc				ThePFTestDurationDesc;
extern PFTestSpawnDesc					ThePFTestSpawnDesc;
extern PFTestSpawnOnCollisionDesc		ThePFTestSpawnOnCollisionDesc;
extern PFTestCollisionSpaceWarpDesc		ThePFTestCollisionSpaceWarpDesc;
extern PFTestSpeedGoToTargetDesc		ThePFTestSpeedGoToTargetDesc;
extern PFTestGoToNextEventDesc			ThePFTestGoToNextEventDesc;
extern PFTestSplitByAmountDesc			ThePFTestSplitByAmountDesc;
extern PFTestSplitBySourceDesc			ThePFTestSplitBySourceDesc;
extern PFTestSplitSelectedDesc			ThePFTestSplitSelectedDesc;
extern PFTestGoToRotationDesc			ThePFTestGoToRotationDesc;
extern PFTestScaleDesc					ThePFTestScaleDesc;
extern PFTestSpeedDesc					ThePFTestSpeedDesc;

extern PFOperatorSimpleBirthStateDesc		ThePFOperatorSimpleBirthStateDesc;
extern PFOperatorSimplePositionStateDesc	ThePFOperatorSimplePositionStateDesc;
extern PFOperatorInstanceShapeStateDesc		ThePFOperatorInstanceShapeStateDesc;
extern PFOperatorFacingShapeStateDesc		ThePFOperatorFacingShapeStateDesc;
extern PFOperatorMarkShapeStateDesc			ThePFOperatorMarkShapeStateDesc;
extern PFOperatorMaterialStaticStateDesc	ThePFOperatorMaterialStaticStateDesc;
extern PFOperatorMaterialDynamicStateDesc	ThePFOperatorMaterialDynamicStateDesc;
extern PFOperatorMaterialFrequencyStateDesc	ThePFOperatorMaterialFrequencyStateDesc;
extern PFOperatorPositionOnObjectStateDesc	ThePFOperatorPositionOnObjectStateDesc;
extern PFTestSplitByAmountStateDesc			ThePFTestSplitByAmountStateDesc;

#define PFOperatorSimpleBirthState_Class_ID			Class_ID(0x400d37f8,0x2086191d)
#define PFOperatorSimplePositionState_Class_ID		Class_ID(0x647d5a0f,0x23a7989)
#define PFOperatorInstanceShapeState_Class_ID		Class_ID(0x6670190f,0x56a4683a)
#define PFOperatorFacingShapeState_Class_ID			Class_ID(0x2d325931,0x5ef3235b)
#define PFOperatorMarkShapeState_Class_ID			Class_ID(0x7db93a92,0x3436206f)
#define PFOperatorPositionOnObjectState_Class_ID	Class_ID(0x16641292,0x23a005a)
#define PFTestSplitByAmountState_Class_ID			Class_ID(0x55617141,0x9591f1d)
#define PFOperatorMaterialStaticState_Class_ID		Class_ID(0x30830ede,0x500a26b9)
#define PFOperatorMaterialDynamicState_Class_ID		Class_ID(0x3078380f,0x1e7a4dd5)
#define PFOperatorMaterialFrequencyState_Class_ID	Class_ID(0x733f4c92,0x1ad479cd)

} // end of namespace EPDElements

#endif // _PFACTIONS_GLOBALVARIABLES_H_