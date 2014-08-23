/**********************************************************************
 *<
	FILE:			PFOperatorMarkShape_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for MarkShape Operator (declaration)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-29-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORMARKSHAPE_PARAMBLOCK_H_
#define  _PFOPERATORMARKSHAPE_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorMarkShapeDesc.h"

namespace PFActions {

const int	kMarkShape_minNumMtls = 1;
const float kMarkShape_minRate = 0.01f;
const float kMarkShape_maxRate = 100.0f;


// block IDs
enum { kMarkShape_mainPBlockIndex };


// param IDs
enum {	kMarkShape_contactObj,
		kMarkShape_surfAnimate,
		kMarkShape_alignTo,
		kMarkShape_divergence,
		kMarkShape_sizeSpace,
		kMarkShape_width,
		kMarkShape_length,
		kMarkShape_inheritedScale,
		kMarkShape_variation,
		kMarkShape_angleDistortion,
		kMarkShape_maxDistortion,
		kMarkShape_markType,
		kMarkShape_height,
		kMarkShape_multiElements,
		kMarkShape_pivotOffset,
		kMarkShape_surfaceOffset,
		kMarkShape_constantUpdate,
		kMarkShape_materialObsolete,
		kMarkShape_numSubMtlsObsolete,
		kMarkShape_subsPerFrameObsolete,
		kMarkShape_generateMapping,
		kMarkShape_randomSeed,
		kMarkShape_vertexNoise,
		kMarkShape_surfaceOffsetVar };

enum {	kMarkShape_alignTo_speed,
		kMarkShape_alignTo_particleX,
		kMarkShape_alignTo_particleY,
		kMarkShape_alignTo_particleZ,
		kMarkShape_alignTo_random,
		kMarkShape_alignTo_num=5 };

enum {	kMarkShape_sizeSpace_world,
		kMarkShape_sizeSpace_local,
		kMarkShape_sizeSpace_num=2 };

enum {	kMarkShape_markType_rectangle,
		kMarkShape_markType_boxIntersect,
		kMarkShape_markType_num=2 };


// User Defined Reference Messages from PB
enum {	kMarkShape_RefMsg_InitDlg = REFMSG_USER + 13279,
		kMarkShape_RefMsg_NewRand };

// dialog messages
enum {	kMarkShape_message_alignTo = 100,		// align to parameter
		kMarkShape_message_sizeSpace,			// size/width parameters
		kMarkShape_message_angleDistort,		// use angle distortion parameter
		kMarkShape_message_markType }; 			// mark type parameter or constant update parameter


class PFOperatorMarkShapeDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	static void UpdateDivergenceDlg( HWND hWnd, int alignTo );
	static void UpdateSizeDlg( HWND hWnd, int sizeSpace );
	static void UpdateAngleDistortionDlg( HWND hWnd, int alignTo, BOOL angleDistortion );
	static void UpdateBoxIntersectionDlg( HWND hWnd, int markType, int constantUpdate );
};

class PFOperatorMarkShapePBValidator : public PBValidator
{
public:
	BOOL Validate(PB2Value& v);
	static INode* GetHighestClosedGroupNode(INode *iNode);
};

class PFOperatorMarkShapePBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);
};


extern PFOperatorMarkShapeDesc ThePFOperatorMarkShapeDesc;
extern ParamBlockDesc2 markShape_paramBlockDesc;
extern FPInterfaceDesc markShape_action_FPInterfaceDesc;
extern FPInterfaceDesc markShape_operator_FPInterfaceDesc;
extern FPInterfaceDesc markShape_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORMARKSHAPE_PARAMBLOCK_H_