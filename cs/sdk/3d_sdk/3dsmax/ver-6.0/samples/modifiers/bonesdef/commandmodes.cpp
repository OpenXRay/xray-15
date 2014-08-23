 /**********************************************************************
 
	FILE: CommandModes.cpp

	DESCRIPTION:  Bones def varius command modes
				  Plus scripter access

	CREATED BY: Peter Watje

	HISTORY: 8/5/98




 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/


#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "iiksys.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "decomp.h"
#include "bonesdef.h"
#include "macrorec.h"

#include "Maxscrpt.h"
#include "Strings.h"
#include "arrays.h"
#include "3DMath.h"
#include "Numbers.h"
#include "definsfn.h"

def_struct_primitive( bakeWeights,skinOps, "bakeSelectedVerts" );


def_struct_primitive( isRigidHandle,skinOps, "isRigidHandle" );
def_struct_primitive( rigidHandle,skinOps, "rigidHandle" );


def_struct_primitive( Invalidate,skinOps, "Invalidate" );

def_struct_primitive( isRigidVertex,skinOps, "isRigidVertex" );
def_struct_primitive( rigidVertex,skinOps, "rigidVertex" );


def_struct_primitive( isUnNormalizeVertex,skinOps, "unNormalizeVertex" );
def_struct_primitive( unNormalizeVertex,skinOps, "isUnNormalizeVertex" );

//reset the current gizmos rotation plane
//skinops.GizmoResetRotationPlane $.modifiers[#Skin] 
def_struct_primitive( gizmoResetRotationPlane,skinOps, "gizmoResetRotationPlane" );

//add add gizmo
//skinops.buttonAddGizmo $.modifiers[#Skin] 
def_struct_primitive( buttonAddGizmo,skinOps, "buttonAddGizmo" );
//remove gizmo
//skinops.buttonRemoveGizmo $.modifiers[#Skin] 
def_struct_primitive( buttonRemoveGizmo,skinOps, "buttonRemoveGizmo" );
//CopyGizmo
//skinops.buttonCopyGizmo $.modifiers[#Skin] 
def_struct_primitive( buttonCopyGizmo,skinOps, "buttonCopyGizmo" );
//PasteGizmo
//skinops.buttonPasteGizmo $.modifiers[#Skin] 
def_struct_primitive( buttonPasteGizmo,skinOps, "buttonPasteGizmo" );

//setCurrentSelectGizmo
//skinops.selectGizmo $.modifiers[#Skin] selectedGizmo
def_struct_primitive( selectGizmo,skinOps, "selectGizmo" );
//getSelectedGizmo
//skinops.getSelectedGizmo $.modifiers[#Skin] 
def_struct_primitive( getSelectedGizmo,skinOps, "getSelectedGizmo" );
//getNumberOfGizmos
//skinops.getNumberOfGizmos $.modifiers[#Skin] 
def_struct_primitive( getNumberOfGizmos,skinOps, "getNumberOfGizmos" );

//enableGizmo
//skinops.enableGizmo $.modifiers[#Skin] gizmoid enable
def_struct_primitive( enableGizmo,skinOps, "enableGizmo" );

//setCurrentSelectGizmoType
//skinops.selectGizmoType $.modifiers[#Skin] selectedGizmoType
def_struct_primitive( selectGizmoType,skinOps, "selectGizmoType" );
//getCurrentSelectGizmoType
//skinops.getCurrentSelectGizmoType $.modifiers[#Skin] getSelectedGizmoType
def_struct_primitive( getSelectedGizmoType,skinOps, "getSelectedGizmoType" );
//getNumberOfGizmoTypes
//skinops.getNumberOfGizmoTypes $.modifiers[#Skin] 
def_struct_primitive( getNumberOfGizmoTypes,skinOps, "getNumberOfGizmoTypes" );


// Maxscript stuff
// need to doc

// russom - 07/30/01
#ifndef USE_GMAX_SECURITY
//skinops.buttonExclude $.modifiers[#Skin] 
def_struct_primitive( saveEnvelope,skinOps, "saveEnvelope" );
def_struct_primitive( saveEnvelopeText,skinOps, "SaveEnvelopeAsASCII" );
#endif // USE_GMAX_SECURITY

//skinops.buttonExclude $.modifiers[#Skin] 
def_struct_primitive( loadEnvelope,skinOps, "loadEnvelope" );
def_struct_primitive( loadEnvelopeText,skinOps, "loadEnvelopeAsASCII" );

//skinops.buttonExclude $.modifiers[#Skin] 
def_struct_primitive( buttonExclude,skinOps, "buttonExclude" );
//skinops.buttonInclude $.modifiers[#Skin] 
def_struct_primitive( buttonInclude,skinOps, "buttonInclude" );
//skinops.buttonSelectExclude $.modifiers[#Skin] 
def_struct_primitive( buttonSelectExcluded,skinOps, "buttonSelectExcluded" );

//skinops.buttonPaint $.modifiers[#Skin] 
def_struct_primitive( buttonPaint,skinOps, "buttonPaint" );

//skinops.buttonAdd $.modifiers[#Skin] 
def_struct_primitive( buttonAdd,skinOps, "buttonAdd" );
//skinops.buttonRemove $.modifiers[#Skin] 
def_struct_primitive( buttonRemove,skinOps, "buttonRemove" );

//skinops.buttonAddCrossSection $.modifiers[#Skin] 
def_struct_primitive( buttonAddCrossSection,skinOps, "buttonAddCrossSection" );
//skinops.buttonRemoveCrossSection $.modifiers[#Skin] 
def_struct_primitive( buttonRemoveCrossSection,skinOps, "buttonRemoveCrossSection" );


//skinops.selectEndPoint $.modifiers[#Skin] 
def_struct_primitive( selectEndPoint,skinOps, "selectEndPoint" );
//skinops.selectStartPoint $.modifiers[#Skin] 
def_struct_primitive( selectStartPoint,skinOps, "selectStartPoint" );

//skinops.selectCrossSection $.modifiers[#Skin] crossid inner
def_struct_primitive( selectCrossSection,skinOps, "selectCrossSection" );


//skinops.copySelectedBone $.modifiers[#Skin] 
def_struct_primitive( copySelectedBone,skinOps, "copySelectedBone" );
//skinops.pasteToSelectedBone $.modifiers[#Skin]
def_struct_primitive( pasteToSelectedBone,skinOps, "pasteToSelectedBone" );
//skinops.pasteToAllBones $.modifiers[#Skin]
def_struct_primitive( pasteToAllBones,skinOps, "pasteToAllBones" );
//skinops.pasteToBone $.modifiers[#Skin] boneID
def_struct_primitive( pasteToBone,skinOps, "pasteToBone" );

//skinops.setSelectedBonePropRelative $.modifiers[#Skin] Relative
def_struct_primitive( setSelectedBonePropRelative,skinOps, "setSelectedBonePropRelative" );
//skinops.getSelectedBonePropRelative $.modifiers[#Skin]
def_struct_primitive( getSelectedBonePropRelative,skinOps, "getSelectedBonePropRelative" );

//skinops.setSelectedBonePropEnvelopeVisible $.modifiers[#Skin] visible
def_struct_primitive( setSelectedBonePropEnvelopeVisible,skinOps, "setSelectedBonePropEnvelopeVisible" );
//skinops.getSelectedBonePropEnvelopeVisible $.modifiers[#Skin]
def_struct_primitive( getSelectedBonePropEnvelopeVisible,skinOps, "getSelectedBonePropEnvelopeVisible" );

//skinops.setSelectedBonePropFalloff $.modifiers[#Skin] falloff
def_struct_primitive( setSelectedBonePropFalloff,skinOps, "setSelectedBonePropFalloff" );
//skinops.getSelectedBonePropEnvelopeVisible $.modifiers[#Skin]
def_struct_primitive( getSelectedBonePropFalloff,skinOps, "getSelectedBonePropFalloff" );


//skinops.setBonePropRelative $.modifiers[#Skin] BoneID Relative
def_struct_primitive( setBonePropRelative,skinOps, "setBonePropRelative" );
//skinops.getBonePropRelative $.modifiers[#Skin] BoneID
def_struct_primitive( getBonePropRelative,skinOps, "getBonePropRelative" );

//skinops.setBonePropEnvelopeVisible $.modifiers[#Skin] BoneID visible
def_struct_primitive( setBonePropEnvelopeVisible,skinOps, "setBonePropEnvelopeVisible" );
//skinops.getBonePropEnvelopeVisible $.modifiers[#Skin] BoneID
def_struct_primitive( getBonePropEnvelopeVisible,skinOps, "getBonePropEnvelopeVisible" );

//skinops.setBonePropFalloff $.modifiers[#Skin] BoneID falloff
def_struct_primitive( setBonePropFalloff,skinOps, "setBonePropFalloff" );
//skinops.getBonePropEnvelopeVisible $.modifiers[#Skin] BoneID
def_struct_primitive( getBonePropFalloff,skinOps, "getBonePropFalloff" );



//skinops.resetSelectedVerts $.modifiers[#Skin] 
def_struct_primitive( resetSelectedVerts,skinOps, "resetSelectedVerts" );

//skinops.resetSelectedBone $.modifiers[#Skin] 
def_struct_primitive( resetSelectedBone,skinOps, "resetSelectedBone" );
//skinops.resetAllBone $.modifiers[#Skin] 
def_struct_primitive( resetAllBones,skinOps, "resetAllBones" );


//skinops.isBoneSelected $.modifiers[#Skin] index
def_struct_primitive( isBoneSelected,skinOps, "isBoneSelected" );



//skinops.removebone $.modifiers[#Skin] 
//skinops.removebone $.modifiers[#Skin] index
def_struct_primitive( removeBone,skinOps, "RemoveBone" );

//skinops.addbone $.modifiers[#Skin] Node Update
def_struct_primitive( addBone,skinOps, "AddBone" );


//skinops.addCrossSection $.modifiers[#Skin] BoneID U InnerRadius OuterRadius
//skinops.addCrossSection $.modifiers[#Skin] U InnerRadius OuterRadius
//skinops.addCrossSection $.modifiers[#Skin] U 
def_struct_primitive( addCrossSection,skinOps, "AddCrossSection" );

//skinops.RemoveCrossSection $.modifiers[#Skin] 
//skinops.RemoveCrossSection $.modifiers[#Skin] BoneID CrossSectionID
def_struct_primitive( removeCrossSection,skinOps, "RemoveCrossSection" );




//skinops.GetCrossSectionU $.modifiers[#Skin] BoneID CrossSectionID
def_struct_primitive( getCrossSectionU,skinOps, "GetCrossSectionU" );
//skinops.SetCrossSectionU $.modifiers[#Skin] BoneID CrossSectionID UValue
def_struct_primitive( setCrossSectionU,skinOps, "SetCrossSectionU" );

//skinops.GetEndPoint $.modifiers[#Skin] BoneID 
def_struct_primitive( getEndPoint,skinOps, "GetEndPoint" );
//skinops.SetEndPoint $.modifiers[#Skin] BoneID [float,float,float]
def_struct_primitive( setEndPoint,skinOps, "SetEndPoint" );
def_struct_primitive( getStartPoint,skinOps, "GetStartPoint" );
def_struct_primitive( setStartPoint,skinOps, "SetStartPoint" );






//just returns the number of bones in the system
def_struct_primitive( getNumberBones,skinOps, "GetNumberBones" );


//just returns the number of vertice in the system
def_struct_primitive (getNumberVertices,skinOps,	"GetNumberVertices" );
//skinGetVertexWeightCount vertexid
//returns the number of bones influencing that vertex
def_struct_primitive (getVertexWeightCount,	skinOps, "GetVertexWeightCount" );
//skinGetBoneName boneID
//just returns the name of the bone
def_struct_primitive (getBoneName, skinOps,			"GetBoneName" );


//skinGetVertexWeight vertexid nthbone
//returns the inlfuence of the nth bone affecting that vertex
def_struct_primitive (getVertexWeight,skinOps,		"GetVertexWeight" );
//skinGetVertexWeightBoneID vertexid nthbone
//returns the bone id of the nth bone affecting that vertex
def_struct_primitive (getVertexWeightBoneID,skinOps,	"GetVertexWeightBoneID" );


//skinSelectVertices number/array/bitarray
//selects the vertices specified
def_struct_primitive (selectSkinVerts, skinOps,			"SelectVertices" );

//skinSetVertexWeights VertexID BoneID Weights
//assigns vertex to BoneID with Weight n
//it does not erase any previous weight info
//BoneID and Weights can be arrays or just numbers but if they are arrays they need to be the same length
def_struct_primitive (setVertWeights,skinOps,				"SetVertexWeights" );
//skinReplaceVertexWeights VertexID BoneID Weights
//assigns vertex to BoneID with Weight n
//it erases any previous bone weight info that vertex before assignment
//BoneID and Weights can be arrays or just numbers but if they are arrays they need to be the same length
def_struct_primitive (replaceVertWeights,skinOps,			"ReplaceVertexWeights" );

//skinIsVertexModified vertID
//just returns if the vertex has been modified
def_struct_primitive (isVertexModified, skinOps,			"IsVertexModified" );


//skinSelectBone BoneID
//selects that bone
def_struct_primitive (selectBone, skinOps,			"SelectBone" );
//GetSelectedBone 
//get the current selected bone
def_struct_primitive (getSelectedBone, skinOps,			"GetSelectedBone" );


//getNumberCrossSections boneID
//returns the number of cross sections for that bone
def_struct_primitive (getNumberCrossSections, skinOps,			"GetNumberCrossSections" );

//getinnerradius boneid crossSectionID
//returns the inner crossscetion radius
def_struct_primitive (getInnerRadius, skinOps,			"GetInnerRadius" );
//getOuterRadius boneid crossSectionID
//returns the inner crossscetion radius
def_struct_primitive (getOuterRadius, skinOps,			"GetOuterRadius" );

//setinnerradius boneid crossSectionID radius
//sets the inner radius of a cross section
def_struct_primitive (setInnerRadius, skinOps,			"SetInnerRadius" );
//setOuterRadius boneid crossSectionID radius
//sets the outer radius of a cross section
def_struct_primitive (setOuterRadius, skinOps,			"SetOuterRadius" );



//IsVertexSelected vertID
//just returns if the vertex has been selected
def_struct_primitive (isVertexSelected, skinOps,		"IsVertexSelected" );
//takes a bool which determines if all viewports are zoomed
def_struct_primitive (zoomToBone, skinOps,		"ZoomToBone" );
def_struct_primitive (zoomToGizmo, skinOps,		"ZoomToGizmo" );

def_struct_primitive (selectNextBone, skinOps,		"selectNextBone" );
def_struct_primitive (selectPreviousBone, skinOps,		"selectPreviousBone" );


def_struct_primitive (addBoneFromViewStart, skinOps,		"addBoneFromViewStart" );
def_struct_primitive (addBoneFromViewEnd, skinOps,		"addBoneFromViewEnd" );

def_struct_primitive (multiRemove, skinOps,		"multiRemove" );

def_struct_primitive (paintWeights, skinOps,		"paintWeightsButton" );
def_struct_primitive (paintOptions, skinOps,		"paintOptionsButton" );


//MIRROR
def_struct_primitive( mirrorPaste,skinOps, "mirrorPaste" );
def_struct_primitive( mirrorPasteBone,skinOps, "mirrorPasteBone" );
def_struct_primitive( selectMirrorBones,skinOps, "selectMirrorBones" );
def_struct_primitive( setMirrorTM,skinOps, "setMirrorTM" );
def_struct_primitive( updateMirror,skinOps, "updateMirror" );

def_struct_primitive( pasteAllBones,skinOps, "pasteAllBones" );
def_struct_primitive( pasteAllVerts,skinOps, "pasteAllVerts" );

def_struct_primitive( blurSelected,skinOps, "blendSelected" );



#define get_bonedef_mod()																\
	Modifier *mod = arg_list[0]->to_modifier();										\
	Class_ID id = mod->ClassID();													\
	if ( id != Class_ID(9815843,87654) )	\
		throw RuntimeError(GetString(IDS_PW_NOT_BONESDEF_ERROR), arg_list[0]);			\
	BonesDefMod *bmod = (BonesDefMod*)mod;			

static int 
check_bone_index(BonesDefMod* bmod, int index)
{
	int ct = 0;
	for (int i = 0; i < bmod->BoneData.Count(); i++)
		if (bmod->BoneData[i].Node) ct++;
	if (index < 0 || index >= ct) 
		throw RuntimeError(_T("Bone index out of range: "), Integer::intern(index + 1));
	return bmod->ConvertSelectedListToBoneID(index);
}


Value*
blurSelected_cf(Value** arg_list, int count)
{
	check_arg_count(blurSelected, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;


	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	if (objects > 0)
	{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (bmd)
		{
			bmod->HoldWeights();
			bmod->AcceptWeights(TRUE);

			bmd->BuildEdgeList();
			bmd->BuildBlurData(bmod->ModeBoneIndex);

			for (int i = 0; i < bmd->VertexData.Count(); i++)
			{
				if (bmd->selected[i])
				{
					int numberOfBones = bmd->VertexData[i]->d.Count();
						
					BOOL hit = FALSE;
					int whichIndex = -1;
					
					for (int k = 0; k <numberOfBones; k++)
					{
						int boneID = bmd->VertexData[i]->d[k].Bones;
						if (boneID == bmod->ModeBoneIndex)
						{
							hit = TRUE;
							whichIndex = k;
						}
					}
					if (hit) 
					{
						//get the remainder
						float newWeight =  bmd->VertexData[i]->d[whichIndex].normalizedInfluences;
						float initialRemainder = 1.0f - newWeight;
						newWeight = bmd->blurredWeights[i];
						float remainder = 1.0f-newWeight;

						for (int k = 0; k <numberOfBones; k++)
						{
							int boneID = bmd->VertexData[i]->d[k].Bones;
							if (boneID != bmod->ModeBoneIndex)
							{
								float w = bmd->VertexData[i]->d[k].normalizedInfluences;
								if (w != 0.0f)
								{
									w = initialRemainder/w;
									w = w * remainder;
									bmd->VertexData[i]->d[k].Influences = w;
								}
							}
							else bmd->VertexData[i]->d[k].Influences = newWeight;
						}

						

						bmd->VertexData[i]->Modified(TRUE);


					}
				}

			}

			bmd->FreeEdgeList();
		}
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());

	}
	return &ok;	

}

static PickControlNode thePickMode;

void BonesDefMod::AddFromViewStart()
{

if (inAddBoneMode)
	{
	inAddBoneMode = FALSE;
	ip->ClearPickMode();
	}

if (ip && (!inAddBoneMode))
	{
	inAddBoneMode = TRUE;
	thePickMode.mod  = this;					
	ip->SetPickMode(&thePickMode);
	}

}
void BonesDefMod::AddFromViewEnd()
{
if (ip)
	{
	inAddBoneMode = FALSE;
	ip->ClearPickMode();
	}

}

static INT_PTR CALLBACK DeleteDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void BonesDefMod::MultiDelete()
{
int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_REMOVE_DIALOG),
				hParam,DeleteDlgProc,(LPARAM)this);

if (iret)
	{	
	int ctl = removeList.Count();
	for (int m =(ctl-1); m >=0 ; m --)	
		{
		int k = ConvertSelectedListToBoneID(removeList[m]);
//transform end points back
		if ((k < BoneData.Count()) && (BoneData[k].Node))
			{
//Delete all old cross sections

			RemoveBone(k);

			}

		}	
	int ct = 0;
	for (int i =0; i < BoneData.Count(); i++)
		{
		if (BoneData[i].Node) ct++;
		}
	if (ct ==0)
		EnableWindow(GetDlgItem(hParam,IDC_REMOVE),FALSE);
	else EnableWindow(GetDlgItem(hParam,IDC_REMOVE),TRUE);

	int fsel = SendMessage(GetDlgItem(hParam,IDC_LIST1),
							LB_GETCURSEL,0,0);	

	int sel = ConvertSelectedListToBoneID(fsel);

	cacheValid = TRUE;
	ModeBoneIndex = sel;


	UpdatePropInterface();
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
	}


}


void BonesDefMod::SelectNextBone()
{
int totalCount = 0;
for (int i =0; i < BoneData.Count(); i ++)
	{
	if (BoneData[i].Node)
		totalCount++;
	}
if (totalCount > 1)
	{

	int boneID;
	int selID;
	selID = SendMessage(GetDlgItem(hParam,IDC_LIST1),
			LB_GETCURSEL ,0,0);
	selID++;
	int listCount;
	listCount = SendMessage(GetDlgItem(hParam,IDC_LIST1),
			LB_GETCOUNT ,0,0);

	if (selID >= listCount)
		selID = 0;
	
	boneID = selID;
	boneID = check_bone_index(this, boneID);

	ModeBoneIndex = boneID;
	SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_SETCURSEL ,selID,0);
	UpdatePropInterface();
//watje 9-7-99  198721 
	Reevaluate(TRUE);
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

	ip->RedrawViews(ip->GetTime());
	}


}
void BonesDefMod::SelectPreviousBone()
{
int totalCount = 0;
for (int i =0; i < BoneData.Count(); i ++)
	{
	if (BoneData[i].Node)
		totalCount++;
	}
if (totalCount > 1)
	{

	int boneID;
	int selID;
	selID = SendMessage(GetDlgItem(hParam,IDC_LIST1),
			LB_GETCURSEL ,0,0);
	selID--;
	int listCount;
	listCount = SendMessage(GetDlgItem(hParam,IDC_LIST1),
			LB_GETCOUNT ,0,0);

	if (selID < 0)
		selID = listCount-1;
	
	boneID = selID;
	boneID = check_bone_index(this, boneID);

	ModeBoneIndex = boneID;
	SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_SETCURSEL ,selID,0);
	UpdatePropInterface();
//watje 9-7-99  198721 
	Reevaluate(TRUE);
	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

	ip->RedrawViews(ip->GetTime());
	}


}


//MIRROR
Value*
mirrorPaste_cf(Value** arg_list, int count)
{
	check_arg_count(mirrorPaste, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;


	if (bmod->mirrorData.Enabled())
		bmod->mirrorData.Paste();

	return &ok;	

}

Value*
pasteAllBones_cf(Value** arg_list, int count)
{
	check_arg_count(pasteAllBones, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	BOOL BtoG =  arg_list[1]->to_bool();

	if (bmod->mirrorData.Enabled())
		bmod->mirrorData.PasteAllBones(BtoG);

	return &ok;	

}

Value*
pasteAllVerts_cf(Value** arg_list, int count)
{
	check_arg_count(pasteAllVerts, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	BOOL BtoG =  arg_list[1]->to_bool();

	if (bmod->mirrorData.Enabled())
		bmod->mirrorData.PasteAllVertices(BtoG);

	return &ok;	

}



Value*
mirrorPasteBone_cf(Value** arg_list, int count)
{
	check_arg_count(mirrorPasteBone, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	int sourceBone = arg_list[1]->to_int()-1;
	int destBone = arg_list[2]->to_int()-1;

	sourceBone = check_bone_index(bmod, sourceBone);
	destBone = check_bone_index(bmod, destBone);

	if (bmod->mirrorData.Enabled())
		{
		theHold.Begin();
	//hold all our bones
		theHold.Put(new PasteToAllRestore(bmod));

		Matrix3 mtm;
		int mirrorPlaneDir;

		TimeValue t = GetCOREInterface()->GetTime();

		bmod->mirrorData.GetMirrorTM(mtm, mirrorPlaneDir);


		bmod->mirrorData.PasteBones(t,mtm,mirrorPlaneDir,sourceBone,destBone);

		theHold.Accept(GetString(IDS_MIRRORPASTE));
		}


	return &ok;	
}


Value*
setMirrorTM_cf(Value** arg_list, int count)
{
	check_arg_count(setMirrorTM, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	Matrix3 tm = arg_list[1]->to_matrix3();

	if (bmod->mirrorData.Enabled())
		{
		bmod->mirrorData.SetInitialTM(tm);
		}


	return &ok;	
}

Value*
updateMirror_cf(Value** arg_list, int count)
{
	check_arg_count(setMirrorTM, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;


	if (bmod->mirrorData.Enabled())
		{
		bmod->mirrorData.BuildBonesMirrorData();
		bmod->mirrorData.BuildVertexMirrorData();
		}


	return &ok;	
}


Value*
selectMirrorBones_cf(Value** arg_list, int count)
{
	check_arg_count(selectMirrorBones, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	Value* ival = arg_list[1];

	if (objects != 0)
		{
		bmod->mirrorData.ClearBoneSelection();

		int index;
		if (is_number(ival))   // single index
			{
			index = ival->to_int()-1;
			bmod->mirrorData.SelectBone(index, TRUE);
			}

		else if (is_array(ival))   // array of indexes
			{
			Array* aval = (Array*)ival;
			for (int i = 0; i < aval->size; i++)
				{
				ival = aval->data[i];
				if (is_number(ival))   // single index
					{
					index = ival->to_int()-1;
					bmod->mirrorData.SelectBone(index, TRUE);
					}
				}

			}
		else if (is_BitArrayValue(ival))   // array of indexes
			{
			BitArrayValue *list = (BitArrayValue *) ival;
			bmod->mirrorData.SelectBones(list->bits);

			}


		GetCOREInterface()->NodeInvalidateRect(nodes[0]);
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}



	return &ok;	
}




Value*
paintWeights_cf(Value** arg_list, int count)
{
	check_arg_count(paintWeights, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;


	bmod->StartPaintMode();

	return &ok;	

}


Value*
paintOptions_cf(Value** arg_list, int count)
{
	check_arg_count(paintOptions, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;


	bmod->PaintOptions();

	return &ok;	

}


Value*
bakeWeights_cf(Value** arg_list, int count)
{
	check_arg_count(bakeWeights, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;


	bmod->BakeSelectedVertices();
	bmod->PaintAttribList();

	return &ok;	

}


Value*
isRigidHandle_cf(Value** arg_list, int count)
{
	check_arg_count(isRigidHandle, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	int vertID = arg_list[1]->to_int()-1;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	BOOL rigid = FALSE;

	if ( objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if ((vertID > 0) && (vertID < bmd->VertexData.Count()))
			rigid = bmd->VertexData[vertID]->IsRigidHandle(); 
		}
	
	return Integer::intern(rigid);	;	

}


Value*
rigidHandle_cf(Value** arg_list, int count)
{
	check_arg_count(rigidHandle, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	int vertID = arg_list[1]->to_int()-1;
	int rigid = arg_list[2]->to_bool();

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	if ( objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if ((vertID > 0) && (vertID < bmd->VertexData.Count()))
			{
			bmd->VertexData[vertID]->RigidHandle(rigid);
			bmd->validVerts.Set(vertID,FALSE);
			}
		}
	return &ok;	

}



Value*
Invalidate_cf(Value** arg_list, int count)
{
	if (count == 1)
		{
		check_arg_count(Invalidate, 1, count);
		}
	else 
		{
		check_arg_count(Invalidate, 2, count);
		}
	

	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

//	int vertID = arg_list[1]->to_int()-1;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	BOOL rigid = TRUE;

	if ( objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		bmd->validVerts.ClearAll();
		bmod->Reevaluate(TRUE);
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

		if (bmod->ip) bmod->ip->RedrawViews(bmod->ip->GetTime());
		}
	
	return Integer::intern(rigid);	;	

}


Value*
isRigidVertex_cf(Value** arg_list, int count)
{
	check_arg_count(isRigidVertex, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	int vertID = arg_list[1]->to_int()-1;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	BOOL rigid = FALSE;

	if ( objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if ((vertID > 0) && (vertID < bmd->VertexData.Count()))
			rigid = bmd->VertexData[vertID]->IsRigid(); 
		}
	
	return Integer::intern(rigid);	;	

}


Value*
rigidVertex_cf(Value** arg_list, int count)
{
	check_arg_count(rigidVertex, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	int vertID = arg_list[1]->to_int()-1;
	int rigid = arg_list[2]->to_bool();

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	if ( objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if ((vertID > 0) && (vertID < bmd->VertexData.Count()))
			{
			bmd->VertexData[vertID]->Rigid(rigid);
			bmd->validVerts.Set(vertID,FALSE);
			}
		}
	return &ok;	

}


Value*
isUnNormalizeVertex_cf(Value** arg_list, int count)
{
	check_arg_count(isUnNormalizeVertex, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	int vertID = arg_list[1]->to_int()-1;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	
	BOOL unNormalize = FALSE;

	if ( objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if ((vertID > 0) && (vertID < bmd->VertexData.Count()))
			unNormalize = bmd->VertexData[vertID]->IsUnNormalized(); 
		}
	
	return Integer::intern(unNormalize);	;	

}


Value*
unNormalizeVertex_cf(Value** arg_list, int count)
{
	check_arg_count(unNormalizeVertex, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	int vertID = arg_list[1]->to_int()-1;
	int unNormalize = arg_list[2]->to_bool();

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	if ( objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		bmod->NormalizeWeight(bmd,vertID, unNormalize);
		}
	return &ok;	

}


Value*
multiRemove_cf(Value** arg_list, int count)
{
	check_arg_count(multiRemove, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	if ( (objects != 0) && (bmod->ModeBoneIndex != -1))
		{
		bmod->MultiDelete();
		}
	return &ok;	

}


Value*
addBoneFromViewStart_cf(Value** arg_list, int count)
{
	check_arg_count(addBoneFromViewStart, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	if ( (objects != 0)/* && (bmod->ModeBoneIndex != -1)*/)
		{
		bmod->AddFromViewStart();
		}
	return &ok;	

}

Value*
addBoneFromViewEnd_cf(Value** arg_list, int count)
{
	check_arg_count(addBoneFromViewEnd, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	if ( (objects != 0)/* && (bmod->ModeBoneIndex != -1)*/)
		{
		bmod->AddFromViewEnd();
		}
	return &ok;	

}



Value*
selectNextBone_cf(Value** arg_list, int count)
{
	check_arg_count(selectNextBone, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int totalCount = 0;

	if ( (objects != 0) && (bmod->ModeBoneIndex != -1))
		{
		bmod->SelectNextBone();
		}
	return &ok;	
}

Value*
selectPreviousBone_cf(Value** arg_list, int count)
{
	check_arg_count(selectPreviousBone, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	if ( (objects != 0) && (bmod->ModeBoneIndex != -1))
		{
		bmod->SelectPreviousBone();
		}
	return &ok;	

}

Value*
zoomToBone_cf(Value** arg_list, int count)
{
	check_arg_count(zoomToBone, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int all = arg_list[1]->to_bool();

	if (objects != 0)
		{
		bmod->ZoomToBone(all);
		}


	return &ok;	

}


Value*
zoomToGizmo_cf(Value** arg_list, int count)
{
	check_arg_count(zoomToGizmo, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int all = arg_list[1]->to_bool();
	
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		bmod->ZoomToGizmo(bmd,all);
		}


	return &ok;	

}


Value*
selectGizmo_cf(Value** arg_list, int count)
{
	check_arg_count(selectGizmo, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int giz = arg_list[1]->to_int()-1;

	if (objects != 0)
		{
		bmod->SelectGizmo(giz);
		}


	return &ok;	

}

Value*
getSelectedGizmo_cf(Value** arg_list, int count)
{
	check_arg_count(getSelectedGizmo, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	return Integer::intern(bmod->currentSelectedGizmo+1);		

}

Value*
getNumberOfGizmos_cf(Value** arg_list, int count)
{
	check_arg_count(getNumberOfGizmos, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	return Integer::intern(bmod->pblock_gizmos->Count(skin_gizmos_list));		

}

//enableGizmo
//skinops.enableGizmo $.modifiers[#Skin] gizmoID enable
Value*
enableGizmo_cf(Value** arg_list, int count)
{
	check_arg_count(enableGizmo, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);


	ModContextList mcList;		
	INodeTab nodes;

	int gizID = arg_list[1]->to_int()-1;
	int enable = arg_list[2]->to_bool();;


	if ((gizID >= 0) && (gizID <bmod->pblock_gizmos->Count(skin_gizmos_list)))
		{
		ReferenceTarget *ref;
		ref = bmod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,gizID);
		GizmoClass *gizmo = (GizmoClass *)ref;
		if (gizmo)
			gizmo->Enable(enable);

		}

	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());


	return &ok;	

}



Value*
selectGizmoType_cf(Value** arg_list, int count)
{
	check_arg_count(selectGizmoType, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int giz = arg_list[1]->to_int()-1;

	if (objects != 0)
		{
		bmod->SelectGizmoType(giz);
		}


	return &ok;	

}

Value*
getSelectedGizmoType_cf(Value** arg_list, int count)
{
	check_arg_count(getSelectedGizmoType, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int did = SendMessage(GetDlgItem(bmod->hParamGizmos,IDC_DEFORMERS),
							           CB_GETCURSEL,0,0)+1;
	return Integer::intern(did);		

}

Value*
getNumberOfGizmoTypes_cf(Value** arg_list, int count)
{
	check_arg_count(getNumberOfGizmoTypes, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();


	return Integer::intern(bmod->gizmoIDList.Count());		

}




Value*
buttonCopyGizmo_cf(Value** arg_list, int count)
{
	check_arg_count(buttonCopyGizmo, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParamGizmos,WM_COMMAND,IDC_COPY,NULL);
		}

	return &ok;	

}

Value*
buttonPasteGizmo_cf(Value** arg_list, int count)
{
	check_arg_count(buttonPasteGizmo, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParamGizmos,WM_COMMAND,IDC_PASTE,NULL);
		}

	return &ok;	

}


Value*
gizmoResetRotationPlane_cf(Value** arg_list, int count)
{
	check_arg_count(gizmoResetRotationPlane, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		if ((bmod->currentSelectedGizmo >= 0) && (bmod->currentSelectedGizmo<bmod->pblock_gizmos->Count(skin_gizmos_list)))
			{
			ReferenceTarget *ref;
			ref = bmod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,bmod->currentSelectedGizmo);
			IGizmoClass3 *giz3 = (IGizmoClass3 *) ref->GetInterface(I_GIZMO3);
			if (giz3)
				{
				giz3->ResetRotationPlane();
				bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
				bmod->ip->RedrawViews(bmod->ip->GetTime());
				}
			}
		
		}

	return &ok;	

}


Value*
buttonAddGizmo_cf(Value** arg_list, int count)
{
	check_arg_count(buttonAddGizmo, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParamGizmos,WM_COMMAND,IDC_ADD,NULL);
		}

	return &ok;	

}

Value*
buttonRemoveGizmo_cf(Value** arg_list, int count)
{
	check_arg_count(buttonRemoveGizmo, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParamGizmos,WM_COMMAND,IDC_REMOVE,NULL);
		}

	return &ok;	

}


// russom - 07/30/01
#ifndef USE_GMAX_SECURITY

Value*
saveEnvelopeText_cf(Value** arg_list, int count)
{
	TCHAR *fname;
	if (count == 1)
		{
		}
	else if (count == 2)
		{
		fname = arg_list[1]->to_string();
		}
	else check_arg_count(saveEnvelopeText, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	if ( count == 1)
		{
		bmod->SaveEnvelopeDialog(FALSE);
		}
	else
		{
		bmod->SaveEnvelope(fname,TRUE);
		}	
	return &ok;	
}


Value*
saveEnvelope_cf(Value** arg_list, int count)
{
	TCHAR *fname;
	if (count == 1)
		{
		}
	else if (count == 2)
		{
		fname = arg_list[1]->to_string();
		}
	else check_arg_count(saveEnvelope, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	if ( count == 1)
		{
		bmod->SaveEnvelopeDialog();
		}
	else
		{
		bmod->SaveEnvelope(fname);
		}	
	return &ok;	
}


#endif // USE_GMAX_SECURITY


Value*
loadEnvelope_cf(Value** arg_list, int count)
{
	TCHAR *fname;
	if (count == 1)
		{
		}
	else if (count == 2)
		{
		fname = arg_list[1]->to_string();
		}
	else check_arg_count(loadEnvelope, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	if ( count == 1)
		{
		bmod->LoadEnvelopeDialog();
		}
	else
		{
		bmod->LoadEnvelope(fname);
		}	
	return &ok;	
}

Value*
loadEnvelopeText_cf(Value** arg_list, int count)
{
	TCHAR *fname;
	if (count == 1)
		{
		}
	else if (count == 2)
		{
		fname = arg_list[1]->to_string();
		}
	else check_arg_count(loadEnvelopeText, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	if ( count == 1)
		{
		bmod->LoadEnvelopeDialog(FALSE);
		}
	else
		{
		bmod->LoadEnvelope(fname,TRUE);
		}	
	return &ok;	
}


Value*
buttonInclude_cf(Value** arg_list, int count)
{
	check_arg_count(buttonInclude, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParam,WM_COMMAND,IDC_INCLUDE,NULL);
		}
	return &ok;	
}

Value*
buttonExclude_cf(Value** arg_list, int count)
{
	check_arg_count(buttonExclude, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParam,WM_COMMAND,IDC_EXCLUDE,NULL);
		}
	return &ok;	
}

Value*
buttonSelectExcluded_cf(Value** arg_list, int count)
{
	check_arg_count(buttonSelectExcluded, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParam,WM_COMMAND,IDC_SELECT_EXCLUDED,NULL);
		}
	return &ok;	
}




Value*
buttonPaint_cf(Value** arg_list, int count)
{
	check_arg_count(buttonPaint, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0) 
		{
		SendMessage(bmod->hParam,WM_COMMAND,IDC_PAINT,NULL);
		}

	return &ok;	

}


Value*
buttonAddCrossSection_cf(Value** arg_list, int count)
{
	check_arg_count(buttonAddCrossSection, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if ((bmod->iCrossSectionButton) && (bmod->iCrossSectionButton->IsEnabled()))
		{

		if (objects != 0)
			{
			SendMessage(bmod->hParam,WM_COMMAND,IDC_CREATE_CROSS_SECTION,NULL);
			}
		}

	return &ok;	

}

Value*
buttonRemoveCrossSection_cf(Value** arg_list, int count)
{
	check_arg_count(buttonRemoveCrossSection, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	BOOL enabled = IsWindowEnabled(GetDlgItem(bmod->hParam,IDC_CREATE_REMOVE_SECTION));

	if ((objects != 0) && (enabled))
		{
		SendMessage(bmod->hParam,WM_COMMAND,IDC_CREATE_REMOVE_SECTION,NULL);
		}

	return &ok;	

}


Value*
buttonAdd_cf(Value** arg_list, int count)
{
	check_arg_count(buttonAdd, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParam,WM_COMMAND,IDC_ADD,NULL);
		}

	return &ok;	

}

Value*
buttonRemove_cf(Value** arg_list, int count)
{
	check_arg_count(buttonRemove, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		SendMessage(bmod->hParam,WM_COMMAND,IDC_REMOVE,NULL);
		}

	return &ok;	

}

Value*
selectCrossSection_cf(Value** arg_list, int count)
{
//skinops.setSelectedBonePropRelative $.modifiers[#Skin] crossid inner
	check_arg_count(selectCrossSection, 3, count);
	get_bonedef_mod();

	int crossID = arg_list[1]->to_int()-1;
	int inner = arg_list[2]->to_int();

	if ((bmod->ModeBoneIndex >= 0) && (bmod->BoneData[bmod->ModeBoneIndex].Node) )
		{
		if ( (crossID < bmod->BoneData[bmod->ModeBoneIndex ].CrossSectionList.Count()) &&
			 (crossID >= 0)
			 )
			{
			bmod->BoneData[bmod->ModeBoneIndex].end1Selected = FALSE;
			bmod->BoneData[bmod->ModeBoneIndex].end2Selected = FALSE;
			bmod->ModeBoneEndPoint = -1;
			bmod->ModeBoneEnvelopeIndex = crossID;
			bmod->ModeBoneEnvelopeSubType = inner*4;
				
			}
		else throw RuntimeError(GetString(IDS_PW_EXCEEDED_CROSS_COUNT), arg_list[0]);
		}
	bmod->updateP = TRUE;
	bmod->UpdatePropInterface();
	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());
	return &ok;	
}


Value*
selectStartPoint_cf(Value** arg_list, int count)
{
	check_arg_count(selectStartPoint, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int ct = 0;




//	BitArray selList = arg_list[1]->to_bitarray();
	if ( (objects != 0) && (bmod->ModeBoneIndex!=-1) && (bmod->ModeBoneIndex>=0) &&
		(bmod->ModeBoneIndex < bmod->BoneData.Count()) )
		{

		bmod->ModeBoneEndPoint = 0;
		bmod->BoneData[bmod->ModeBoneIndex].end1Selected = TRUE;
		bmod->BoneData[bmod->ModeBoneIndex].end2Selected = FALSE;

		bmod->UpdatePropInterface();
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->ip->RedrawViews(bmod->ip->GetTime());

		}

	return &ok;	

}


Value*
selectEndPoint_cf(Value** arg_list, int count)
{
	check_arg_count(selectEndPoint, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int ct = 0;

//	BitArray selList = arg_list[1]->to_bitarray();
	if ( (objects != 0) && (bmod->ModeBoneIndex!=-1) && (bmod->ModeBoneIndex>=0) &&
		(bmod->ModeBoneIndex < bmod->BoneData.Count()) )
		{

		bmod->ModeBoneEndPoint = 1;
		bmod->BoneData[bmod->ModeBoneIndex].end2Selected = TRUE;
		bmod->BoneData[bmod->ModeBoneIndex].end1Selected = FALSE;

		bmod->UpdatePropInterface();
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->ip->RedrawViews(bmod->ip->GetTime());

		}

	return &ok;	

}



Value*
copySelectedBone_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropRelative $.modifiers[#Skin]
	check_arg_count(copySelectedBone, 1, count);
	get_bonedef_mod();

	if (bmod->ModeBoneIndex >= 0)
		{
		bmod->CopyBone();
		if (bmod->iPaste) bmod->iPaste->Enable(TRUE);		

		}
	return &ok;	
}

Value*
pasteToSelectedBone_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropRelative $.modifiers[#Skin]
	check_arg_count(pasteToSelectedBone, 1, count);
	get_bonedef_mod();

	if (bmod->ModeBoneIndex >= 0)
		{
		bmod->PasteBone();	
//		bmod->Reevaluate(TRUE);
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->ip->RedrawViews(bmod->ip->GetTime());

		}
	return &ok;	
}

Value*
pasteToAllBones_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropRelative $.modifiers[#Skin]
	check_arg_count(pasteToAllBones, 1, count);
	get_bonedef_mod();

	if (bmod->ModeBoneIndex >= 0)
		{
		bmod->PasteToAllBones();	
//		bmod->Reevaluate(TRUE);
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->ip->RedrawViews(bmod->ip->GetTime());

		}
	return &ok;	
}


Value*
pasteToBone_cf(Value** arg_list, int count)
{
//skinops.pasteToBone $.modifiers[#Skin] boneID
	check_arg_count(pasteToBone, 2, count);
	get_bonedef_mod();
	int boneID = arg_list[1]->to_int();
//	boneID = check_bone_index(bmod,boneID);

	if (bmod->ModeBoneIndex >= 0)
		{
		bmod->pasteList.SetCount(1);
		bmod->pasteList[0] = boneID;
		bmod->PasteToSomeBones();	
//		bmod->Reevaluate(TRUE);
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->ip->RedrawViews(bmod->ip->GetTime());

		}
	return &ok;	
}


Value*
setSelectedBonePropRelative_cf(Value** arg_list, int count)
{
//skinops.setSelectedBonePropRelative $.modifiers[#Skin] relative
	check_arg_count(setSelectedBonePropRelative, 2, count);
	get_bonedef_mod();

	int relative = arg_list[1]->to_int();

	if (bmod->ModeBoneIndex >= 0)
		{
		if (!relative)
			bmod->BoneData[bmod->ModeBoneIndex].flags |= BONE_ABSOLUTE_FLAG;
		else bmod->BoneData[bmod->ModeBoneIndex].flags &= ~BONE_ABSOLUTE_FLAG;
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->UpdatePropInterface();
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}
	return &ok;	
}

Value*
getSelectedBonePropRelative_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropRelative $.modifiers[#Skin]
	check_arg_count(getSelectedBonePropRelative, 1, count);
	get_bonedef_mod();

	if (bmod->ModeBoneIndex >= 0)
		{
		
		if (bmod->BoneData[bmod->ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
			return Integer::intern(0);
		else return Integer::intern(1);
		}
	return &ok;	
}


Value*
setSelectedBonePropFalloff_cf(Value** arg_list, int count)
{
//skinops.setSelectedBonePropFalloff $.modifiers[#Skin] relative
	check_arg_count(setSelectedBonePropFalloff, 2, count);
	get_bonedef_mod();

	int foff = arg_list[1]->to_int();

	if (bmod->ModeBoneIndex >= 0)
		{

		if (foff == 1)
			bmod->BoneData[bmod->ModeBoneIndex].FalloffType = BONE_FALLOFF_X_FLAG;
		else if (foff == 2)
			bmod->BoneData[bmod->ModeBoneIndex].FalloffType = BONE_FALLOFF_SINE_FLAG;
		else if (foff == 3)
			bmod->BoneData[bmod->ModeBoneIndex].FalloffType = BONE_FALLOFF_3X_FLAG;
		else if (foff == 4)
			bmod->BoneData[bmod->ModeBoneIndex].FalloffType = BONE_FALLOFF_X3_FLAG;
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->UpdatePropInterface();
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}
	return &ok;	
}

Value*
getSelectedBonePropFalloff_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropFalloff $.modifiers[#Skin]
	check_arg_count(getSelectedBonePropFalloff, 1, count);
	get_bonedef_mod();

	if (bmod->ModeBoneIndex >= 0)
		{
		if (bmod->BoneData[bmod->ModeBoneIndex].FalloffType == BONE_FALLOFF_X_FLAG)
			return Integer::intern(1);
		else if (bmod->BoneData[bmod->ModeBoneIndex].FalloffType == BONE_FALLOFF_SINE_FLAG)
			return Integer::intern(2);
		else if (bmod->BoneData[bmod->ModeBoneIndex].FalloffType == BONE_FALLOFF_3X_FLAG)
			return Integer::intern(3);
		else if (bmod->BoneData[bmod->ModeBoneIndex].FalloffType == BONE_FALLOFF_X3_FLAG)
			return Integer::intern(4);
		}
	return &ok;	
}


Value*
setSelectedBonePropEnvelopeVisible_cf(Value** arg_list, int count)
{
//skinops.setSelectedBonePropFalloff $.modifiers[#Skin] relative
	check_arg_count(setSelectedBonePropEnvelopeVisible, 2, count);
	get_bonedef_mod();

	int vis = arg_list[1]->to_int();

	if (bmod->ModeBoneIndex >= 0)
		{

		if (vis == 0)
			bmod->BoneData[bmod->ModeBoneIndex].flags &= ~BONE_DRAW_ENVELOPE_FLAG;
		else 
			bmod->BoneData[bmod->ModeBoneIndex].flags |= BONE_DRAW_ENVELOPE_FLAG;
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->UpdatePropInterface();
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}
	return &ok;	
}

Value*
getSelectedBonePropEnvelopeVisible_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropFalloff $.modifiers[#Skin]
	check_arg_count(getSelectedBonePropEnvelopeVisible, 1, count);
	get_bonedef_mod();

	if (bmod->ModeBoneIndex >= 0)
		{
		if (bmod->BoneData[bmod->ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
			return Integer::intern(1);
		else return Integer::intern(0);

		}
	return &ok;	
}


Value*
setBonePropRelative_cf(Value** arg_list, int count)
{
//skinops.setSelectedBonePropRelative $.modifiers[#Skin] relative
	check_arg_count(setBonePropRelative, 3, count);
	get_bonedef_mod();

	int boneID = arg_list[1]->to_int()-1;
	boneID = check_bone_index(bmod,boneID);
	int relative = arg_list[2]->to_int();
	

	if ( (boneID >=0) && (boneID < bmod->BoneData.Count() ))
		{
		if (!relative)
			bmod->BoneData[boneID].flags |= BONE_ABSOLUTE_FLAG;
		else bmod->BoneData[boneID].flags &= ~BONE_ABSOLUTE_FLAG;
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->UpdatePropInterface();
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}
	return &ok;	
}

Value*
getBonePropRelative_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropRelative $.modifiers[#Skin]
	check_arg_count(getBonePropRelative, 2, count);
	get_bonedef_mod();

	int boneID = arg_list[1]->to_int()-1;
	boneID = check_bone_index(bmod,boneID);

	if ( (boneID >= 0) && (boneID < bmod->BoneData.Count() ))
		{
		
		if (bmod->BoneData[boneID].flags & BONE_ABSOLUTE_FLAG)
			return Integer::intern(0);
		else return Integer::intern(1);
		}
	return &ok;	
}


Value*
setBonePropFalloff_cf(Value** arg_list, int count)
{
//skinops.setSelectedBonePropFalloff $.modifiers[#Skin] relative
	check_arg_count(setBonePropFalloff, 3, count);
	get_bonedef_mod();

	int boneID = arg_list[1]->to_int()-1;
	boneID = check_bone_index(bmod,boneID);

	int foff = arg_list[2]->to_int();

	if ( (boneID >= 0) && (boneID < bmod->BoneData.Count() ))
		{

		if (foff == 1)
			bmod->BoneData[boneID].FalloffType = BONE_FALLOFF_X_FLAG;
		else if (foff == 2)
			bmod->BoneData[boneID].FalloffType = BONE_FALLOFF_SINE_FLAG;
		else if (foff == 3)
			bmod->BoneData[boneID].FalloffType = BONE_FALLOFF_3X_FLAG;
		else if (foff == 4)
			bmod->BoneData[boneID].FalloffType = BONE_FALLOFF_X3_FLAG;
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->UpdatePropInterface();
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}
	return &ok;	
}

Value*
getBonePropFalloff_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropFalloff $.modifiers[#Skin]
	check_arg_count(getBonePropFalloff, 2, count);
	get_bonedef_mod();

	int boneID = arg_list[1]->to_int()-1;
	boneID = check_bone_index(bmod,boneID);

	if ( (boneID >= 0) && (boneID < bmod->BoneData.Count() ))
		{
		if (bmod->BoneData[boneID].FalloffType == BONE_FALLOFF_X_FLAG)
			return Integer::intern(1);
		else if (bmod->BoneData[boneID].FalloffType == BONE_FALLOFF_SINE_FLAG)
			return Integer::intern(2);
		else if (bmod->BoneData[boneID].FalloffType == BONE_FALLOFF_3X_FLAG)
			return Integer::intern(3);
		else if (bmod->BoneData[boneID].FalloffType == BONE_FALLOFF_X3_FLAG)
			return Integer::intern(4);
		}
	return &ok;	
}


Value*
setBonePropEnvelopeVisible_cf(Value** arg_list, int count)
{
//skinops.setSelectedBonePropFalloff $.modifiers[#Skin] relative
	check_arg_count(setBonePropEnvelopeVisible, 3, count);
	get_bonedef_mod();

	int boneID = arg_list[1]->to_int()-1;
	boneID = check_bone_index(bmod,boneID);
	int vis = arg_list[2]->to_int();

	if ( (boneID >= 0) && (boneID < bmod->BoneData.Count() ))
		{

		if (vis == 0)
			bmod->BoneData[boneID].flags &= ~BONE_DRAW_ENVELOPE_FLAG;
		else 
			bmod->BoneData[boneID].flags |= BONE_DRAW_ENVELOPE_FLAG;
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->UpdatePropInterface();
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}
	return &ok;	
}

Value*
getBonePropEnvelopeVisible_cf(Value** arg_list, int count)
{
//skinops.getSelectedBonePropFalloff $.modifiers[#Skin]
	check_arg_count(getBonePropEnvelopeVisible, 2, count);
	get_bonedef_mod();

	int boneID = arg_list[1]->to_int()-1;
	boneID = check_bone_index(bmod,boneID);

	if ( (boneID >= 0) && (boneID < bmod->BoneData.Count() ))
		{
		if (bmod->BoneData[boneID].flags & BONE_DRAW_ENVELOPE_FLAG)
			return Integer::intern(1);
		else return Integer::intern(0);
		}
	return &ok;	
}





Value*
resetSelectedVerts_cf(Value** arg_list, int count)
{
//skinops.resetSelectedVerts $.modifiers[#Skin]
	check_arg_count(resetSelectedVerts, 1, count);
	get_bonedef_mod();

	bmod->UnlockVerts();
	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());

	return &ok;	
}



Value*
resetSelectedBone_cf(Value** arg_list, int count)
{
//skinops.resetSelectedBone $.modifiers[#Skin]
	check_arg_count(resetSelectedBone, 1, count);
	get_bonedef_mod();

	bmod->unlockBone = TRUE;
	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());

	return &ok;	
}

Value*
resetAllBones_cf(Value** arg_list, int count)
{
//skinops.resetSelectedBone $.modifiers[#Skin]
	check_arg_count(resetAllBones, 1, count);
	get_bonedef_mod();

	bmod->unlockAllBones = TRUE;
	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());

	return &ok;	
}


Value*
isBoneSelected_cf(Value** arg_list, int count)
{
//skinops.isBoneSelected $.modifiers[#Skin] index
	check_arg_count(isBoneSelected, 2, count);
	get_bonedef_mod();

	int boneID = arg_list[1]->to_int();
	boneID = check_bone_index(bmod,boneID);

	if (bmod->ModeBoneIndex == boneID)
		return Integer::intern(1);	
	else return Integer::intern(0);	
}




Value*
removeBone_cf(Value** arg_list, int count)
{
//skinops.addbone $.modifiers[#Skin] index
	get_bonedef_mod();
	int boneID;
	if (count == 1)
		{
		boneID = bmod->ModeBoneIndex;
		}
	else if (count == 2)
		{
		boneID = arg_list[1]->to_int()-1;
		boneID = check_bone_index(bmod,boneID);

		}
	else check_arg_count(removeBone, 2, count);
	if (boneID >=0) 
		{
		bmod->RemoveBone(boneID);
		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}

	return &ok;	
}


Value*
addBone_cf(Value** arg_list, int count)
{
//skinops.addbone $.modifiers[#Skin] $Bone02 1
	check_arg_count(addBone, 3, count);
	get_bonedef_mod();

	int update = arg_list[2]->to_int();
	INode *node = arg_list[1]->to_node();
	
	bmod->AddBone(node,update);

	return &ok;	
}


Value*
addCrossSection_cf(Value** arg_list, int count)
{
//skinops.addCrossSection $.modifiers[#Skin] BoneID U InnerRadius OuterRadius
//skinops.addCrossSection $.modifiers[#Skin] U InnerRadius OuterRadius
//skinops.addCrossSection $.modifiers[#Skin] U 
	int boneID;
	float u, inner, outer;
	get_bonedef_mod();
	if (count == 5)
		{
		boneID = arg_list[1]->to_int()-1;
		boneID = check_bone_index(bmod,boneID);

		u = arg_list[2]->to_float();
		inner = arg_list[3]->to_float();
		outer = arg_list[4]->to_float();
		}
	else if (count ==4)
		{
		boneID = bmod->ModeBoneIndex ;
		u = arg_list[1]->to_float();
		inner = arg_list[2]->to_float();
		outer = arg_list[3]->to_float();
		}

	else if (count ==2)
		{
		u = arg_list[1]->to_float();
		}

	else check_arg_count(addCrossSection, 5, count);

	if (boneID >=0)
		{
		if ((count == 4) || (count ==5)	)
			bmod->AddCrossSection(boneID, u, inner, outer);
		else bmod->AddCrossSection(u);


		if (bmod->ModeBoneIndex != boneID)
			bmod->Reevaluate(TRUE);

		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}

//	int ct = bmod->BoneData.Count();
	return &ok;	
}


Value*
removeCrossSection_cf(Value** arg_list, int count)
{
//skinops.removeCrossSection $.modifiers[#Skin]
//skinops.removeCrossSection $.modifiers[#Skin] BoneID CrossSectionID
	get_bonedef_mod();
	int boneID;
	int crossID;
	if (count == 1)
		{
		boneID = bmod->ModeBoneIndex;
		crossID = bmod->ModeBoneEnvelopeIndex;
		}
	else if (count = 3)
		{
		boneID = arg_list[1]->to_int()-1;
		boneID = check_bone_index(bmod,boneID);


		Value* crossval = arg_list[2];
	
		if (!is_number(crossval)) 
			{
			throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[2]);
			}
		else crossID = crossval->to_int()-1;
		}
	else check_arg_count(removeCrossSection, 3, count);



	if ( (boneID >= 0) && (boneID < bmod->BoneData.Count()) && (bmod->BoneData[boneID].Node) )
		{
		if ( (crossID < bmod->BoneData[boneID].CrossSectionList.Count()) &&
			 (crossID >= 0)
			 )
			{
			bmod->RemoveCrossSection(boneID, crossID);
				
			}
		else throw RuntimeError(GetString(IDS_PW_EXCEEDED_CROSS_COUNT), arg_list[0]);
		}



	if (bmod->ModeBoneIndex != boneID)
		bmod->Reevaluate(TRUE);

	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());

	return &ok;	
}



Value*
getCrossSectionU_cf(Value** arg_list, int count)
{
//skinops.GetCrossSectionU $.modifiers[#Skin] 3 3
	check_arg_count(getCrossSectionU, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	float w = 0;

	Value* vertval = arg_list[1];
	Value* crossval = arg_list[2];

	int boneID;
	int crossID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
//	boneID = bmod->ConvertSelectedListToBoneID(boneID);
	boneID = check_bone_index(bmod,boneID);

	if (!is_number(crossval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else crossID = crossval->to_int()-1;


	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (boneID >= bmod->BoneData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (boneID < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (bmod->BoneData[boneID].Node)
			{
			if ( (crossID < bmod->BoneData[boneID].CrossSectionList.Count()) &&
				 (crossID >= 0)
				 )
				{
				Interval v;
				w = bmod->BoneData[boneID].CrossSectionList[crossID].u;
					
				}
			else throw RuntimeError(GetString(IDS_PW_EXCEEDED_CROSS_COUNT), arg_list[0]);
			}
		}

	return Float::intern(w);			

}


Value*
setCrossSectionU_cf(Value** arg_list, int count)
{
	check_arg_count(setCrossSectionU, 4, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	float w = 0;

	Value* vertval = arg_list[1];
	Value* crossval = arg_list[2];
	Value* radiusval = arg_list[3];

	int boneID;
	int crossID;
	float radius;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
//	boneID = bmod->ConvertSelectedListToBoneID(boneID);
	boneID = check_bone_index(bmod,boneID);

	if (!is_number(crossval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else crossID = crossval->to_int()-1;

	if (!is_number(radiusval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else radius = radiusval->to_float();


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (boneID >= bmod->BoneData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (boneID < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (bmod->BoneData[boneID].Node)
			{
			if ( (crossID < bmod->BoneData[boneID].CrossSectionList.Count()) &&
				 (crossID >= 0)
				 )
				{
				Interval v;
				bmod->BoneData[boneID].CrossSectionList[crossID].u = radius;
					
				}
			else throw RuntimeError(GetString(IDS_PW_EXCEEDED_CROSS_COUNT), arg_list[0]);
			}
		}

	if (bmod->ModeBoneIndex != boneID)
		bmod->Reevaluate(TRUE);
//	bmod->reevaluate =TRUE;

	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());


	return &ok;			

}



Value*
getEndPoint_cf(Value** arg_list, int count)
{
//skinops.GetEndPoint $.modifiers[#Skin] 1 
	check_arg_count(getEndPoint, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	Point3 w(0.0f,0.0f,0.0f);

	Value* vertval = arg_list[1];

	int boneID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
//	boneID = bmod->ConvertSelectedListToBoneID(boneID);
	boneID = check_bone_index(bmod,boneID);



	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (boneID >= bmod->BoneData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (boneID < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (bmod->BoneData[boneID].Node)
			{
			Interval v;
			bmod->BoneData[boneID].EndPoint2Control->GetValue(bmod->currentTime,&w,v);
					
			}
		}
	return new Point3Value(w);
//	return Point3Value(w);			

}


Value*
setEndPoint_cf(Value** arg_list, int count)
{
//skinops.GetEndPoint $.modifiers[#Skin] 1 [3.3,3.3,3.3]
	check_arg_count(setEndPoint, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	float w = 0;

	Value* vertval = arg_list[1];
	Value* pval = arg_list[2];

	int boneID;
	Point3 l2;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
//	boneID = bmod->ConvertSelectedListToBoneID(boneID);
	boneID = check_bone_index(bmod,boneID);

	if (!is_point3(pval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else l2 = pval->to_point3();



//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (boneID >= bmod->BoneData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (boneID < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (bmod->BoneData[boneID].Node)
			{
			Interval v;
			bmod->BoneData[boneID].EndPoint2Control->SetValue(bmod->currentTime,&l2);
			}
		}

	if (bmod->ModeBoneIndex != boneID)
		bmod->Reevaluate(TRUE);
//	bmod->reevaluate =TRUE;

	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());


	return &ok;			

}


Value*
getStartPoint_cf(Value** arg_list, int count)
{
//skinops.GetEndPoint $.modifiers[#Skin] 1 
	check_arg_count(getStartPoint, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	Point3 w(0.0f,0.0f,0.0f);

	Value* vertval = arg_list[1];

	int boneID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
//	boneID = bmod->ConvertSelectedListToBoneID(boneID);
	boneID = check_bone_index(bmod,boneID);



	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (boneID >= bmod->BoneData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (boneID < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (bmod->BoneData[boneID].Node)
			{
			Interval v;
			bmod->BoneData[boneID].EndPoint1Control->GetValue(bmod->currentTime,&w,v);
					
			}
		}
	return new Point3Value(w);
//	return Point3Value(w);			

}


Value*
setStartPoint_cf(Value** arg_list, int count)
{
	check_arg_count(setStartPoint, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	float w = 0;

	Value* vertval = arg_list[1];
	Value* pval = arg_list[2];

	int boneID;
	Point3 l2;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
//	boneID = bmod->ConvertSelectedListToBoneID(boneID);
	boneID = check_bone_index(bmod,boneID);

	if (!is_point3(pval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else l2 = pval->to_point3();



//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (boneID >= bmod->BoneData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (boneID < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_BONE_COUNT), arg_list[0]);
		if (bmod->BoneData[boneID].Node)
			{
			Interval v;
			bmod->BoneData[boneID].EndPoint1Control->SetValue(bmod->currentTime,&l2);
			}
		}

	if (bmod->ModeBoneIndex != boneID)
		bmod->Reevaluate(TRUE);
	//		bmod->reevaluate =TRUE;

	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());


	return &ok;			

}



Value*
getNumberBones_cf(Value** arg_list, int count)
{
	check_arg_count(getNumberBones, 1, count);
	get_bonedef_mod();
	int ct = 0;
	for (int i = 0; i < bmod->BoneData.Count(); i++)
		if (bmod->BoneData[i].Node) ct++;

//	int ct = bmod->BoneData.Count();
	return Integer::intern(ct);	
}


Value*
getNumberVertices_cf(Value** arg_list, int count)
{
	check_arg_count(getNumberVertices, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		ct = bmd->VertexData.Count();
		}
	return Integer::intern(ct);	
}

Value*
getVertexWeightCount_cf(Value** arg_list, int count)
{
	check_arg_count(getVertexWeightCount, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	int index = arg_list[1]->to_int()-1;
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (index >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		if (index < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);

		ct = bmd->VertexData[index]->d.Count();
		}
	return Integer::intern(ct);	
}


Value*
getBoneName_cf(Value** arg_list, int count)
{
	check_arg_count(getBoneName, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	int index = arg_list[1]->to_int()-1;
	int listName = arg_list[2]->to_int();
	index = check_bone_index(bmod, index);

	if (bmod->BoneData[index].Node != NULL)
		{
		if (listName)
			{
			Class_ID bid(BONE_CLASS_ID,0);
			ObjectState os = bmod->BoneData[index].Node->EvalWorldState(bmod->RefFrame);
			if (( os.obj->ClassID() == bid) && (bmod->BoneData[index].name.Length()) )
				{
				return new String((TCHAR*)bmod->BoneData[index].name);			
				}
			else return new String((TCHAR*)bmod->BoneData[index].Node->GetName()); 
			
			}
		else return new String((TCHAR*)bmod->BoneData[index].Node->GetName()); 
		}
	else return &undefined;

}



Value*
getVertexWeight_cf(Value** arg_list, int count)
{
	check_arg_count(getVertexWeight, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	float ct = 0.0f;
	int vindex = arg_list[1]->to_int()-1;
	int subindex = arg_list[2]->to_int()-1;
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (vindex >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		if (subindex >= bmd->VertexData[vindex]->d.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_SUBVERTEX_COUNT), arg_list[0]);
		if (vindex < 0  ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		if (subindex < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_SUBVERTEX_COUNT), arg_list[0]);

		ct = bmod->RetrieveNormalizedWeight(bmd,vindex,subindex);
		}
	return Float::intern(ct);	
}

Value*
getVertexWeightBoneID_cf(Value** arg_list, int count)
{
	check_arg_count(getVertexWeightBoneID, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	int vindex = arg_list[1]->to_int()-1;
	int subindex = arg_list[2]->to_int()-1;
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (vindex >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		if (subindex >= bmd->VertexData[vindex]->d.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_SUBVERTEX_COUNT), arg_list[0]);
		if (vindex < 0  ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		if (subindex < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_SUBVERTEX_COUNT), arg_list[0]);

		ct = bmd->VertexData[vindex]->d[subindex].Bones;
//watje 10-26-99 Skin fix 185932
		ct = bmod->ConvertSelectedBoneToListID(ct)+1;
		}
	return Integer::intern(ct);	
}




Value*
selectSkinVerts_cf(Value** arg_list, int count)
{
	check_arg_count(selectSkinVerts, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	Value* ival = arg_list[1];

//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		bmd->selected.ClearAll();

		int index;
		if (is_number(ival))   // single index
			{
			index = ival->to_int()-1;
			if ((index <0) || (index >= bmd->selected.GetSize()) )
				throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
			else bmd->selected.Set(index,TRUE);
			}

		else if (is_array(ival))   // array of indexes
			{
			Array* aval = (Array*)ival;
			for (int i = 0; i < aval->size; i++)
				{
				ival = aval->data[i];
				if (is_number(ival))   // single index
					{
					index = ival->to_int()-1;
					if ((index <0) || (index >= bmd->selected.GetSize()))
						throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
					else bmd->selected.Set(index,TRUE);

					}
				}

			}
		else if (is_BitArrayValue(ival))   // array of indexes
			{
			BitArrayValue *list = (BitArrayValue *) ival;
			for (int index = 0; index < list->bits.GetSize(); index++)
				{
				if (list->bits[index])
					{
					if ((index <0) || (index >= bmd->selected.GetSize()))
						throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
					else bmd->selected.Set(index,TRUE);
					}
				}

			}


		GetCOREInterface()->NodeInvalidateRect(nodes[0]);
		bmod->ip->RedrawViews(bmod->ip->GetTime());
		}



	return &ok;	
}


Value*
setVertWeights_cf(Value** arg_list, int count)
{
	check_arg_count(setVertWeights, 4, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;

	Value* vertval = arg_list[1];
	Value* bonesval = arg_list[2];
	Value* weightsval = arg_list[3];

	int vertID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else vertID = vertval->to_int()-1;


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (vertID >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);

//		bmd->selected.ClearAll();

//		int index;
		if (is_number(weightsval) && is_number(bonesval))  // single index
			{
			int boneID = bonesval->to_int()-1;
			boneID = check_bone_index(bmod, boneID);

			float weight = weightsval->to_float();
			if (weight < 0.0f ) weight = 0.0f;
			if (weight > 1.0f ) weight = 1.0f;
			if (bmod->BoneData[boneID].Node != NULL)
				{
//				bmd->VertexData[vertID]->d.ZeroCount();
				bmod->SetVertex(bmd,vertID, boneID, weight);
				}
			}

		else if (is_array(weightsval) && is_array(bonesval))   // array of indexes
			{

			Array* wval = (Array*)weightsval;
			Array* bval = (Array*)bonesval;
			if (wval->size != bval->size) throw RuntimeError(GetString(IDS_PW_WEIGHT_BONE_COUNT), arg_list[0]);

//			bmd->VertexData[vertID]->d.ZeroCount();
			Tab<int> b;
			Tab<float> v;
			for (int i = 0; i < wval->size; i++)
				{
				Value *boneval = bval->data[i];
				Value *weightval = wval->data[i];
				if ( (is_number(boneval)) && (is_number(weightval)))  // single index
					{
					int boneID = boneval->to_int()-1;
//watje 10-26-99 Skin fix 185932
//					boneID = bmod->ConvertSelectedListToBoneID(boneID);
					boneID = check_bone_index(bmod, boneID);

					float weight = weightval->to_float();
					if (weight < 0.0f ) weight = 0.0f;
					if (weight > 1.0f ) weight = 1.0f;
					if (bmod->BoneData[boneID].Node != NULL)
						{
						b.Append(1,&boneID,1);
						v.Append(1,&weight,1);
						}

					}
				}
			bmod->SetVertices(bmd,vertID, b, v);


			}



		}
//watje 9-7-99  198721 
	bmod->Reevaluate(TRUE);
	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());


	return &ok;	
}


Value*
replaceVertWeights_cf(Value** arg_list, int count)
{
	check_arg_count(replaceVertWeights, 4, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;

	Value* vertval = arg_list[1];
	Value* bonesval = arg_list[2];
	Value* weightsval = arg_list[3];

	int vertID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else vertID = vertval->to_int()-1;


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (vertID >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);

//		bmd->selected.ClearAll();

//		int index;
		if (is_number(weightsval) && is_number(bonesval))  // single index
			{
			int boneID = bonesval->to_int()-1;
			boneID = check_bone_index(bmod, boneID);
			float weight = weightsval->to_float();
			if (weight < 0.0f ) weight = 0.0f;
			if (weight > 1.0f ) weight = 1.0f;
			if (bmod->BoneData[boneID].Node != NULL)
				{
				bmd->VertexData[vertID]->d.ZeroCount();
				bmod->SetVertex(bmd,vertID, boneID, weight);
				}
			}

		else if (is_array(weightsval) && is_array(bonesval))   // array of indexes
			{

			Array* wval = (Array*)weightsval;
			Array* bval = (Array*)bonesval;
			if (wval->size != bval->size) throw RuntimeError(GetString(IDS_PW_WEIGHT_BONE_COUNT), arg_list[0]);
			bmd->VertexData[vertID]->d.ZeroCount();

			Tab<int> b;
			Tab<float> v;

			bmd->VertexData[vertID]->d.ZeroCount();

			for (int i = 0; i < wval->size; i++)
				{
				Value *boneval = bval->data[i];
				Value *weightval = wval->data[i];
				if ( (is_number(boneval)) && (is_number(weightval)))  // single index
					{
					int boneID = boneval->to_int()-1;
					boneID = check_bone_index(bmod, boneID);
					float weight = weightval->to_float();
					if (weight < 0.0f ) weight = 0.0f;
					if (weight > 1.0f ) weight = 1.0f;
					if (bmod->BoneData[boneID].Node != NULL)
						{
						b.Append(1,&boneID,1);
						v.Append(1,&weight,1);
//						bmod->SetVertex(bmd,vertID, boneID, weight);
						}

					}
				}
			bmod->SetVertices(bmd,vertID, b, v);


			}



		}
//watje 9-7-99  198721 
	bmod->Reevaluate(TRUE);
	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());


	return &ok;	
}


Value*
isVertexModified_cf(Value** arg_list, int count)
{
	check_arg_count(isVertexModified, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int ct = 0;

	Value* vertval = arg_list[1];

	int vertID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else vertID = vertval->to_int()-1;


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (vertID >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		if (vertID < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		ct = bmd->VertexData[vertID]->IsModified();
		}
	return Integer::intern(ct);	


}


Value*
isVertexSelected_cf(Value** arg_list, int count)
{
	check_arg_count(isVertexSelected, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int ct = 0;

	Value* vertval = arg_list[1];

	int vertID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else vertID = vertval->to_int()-1;


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (vertID >= bmd->selected.GetSize() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		if (vertID < 0 ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
		ct = bmd->selected[vertID];
		}
	return Integer::intern(ct);	


}


Value*
selectBone_cf(Value** arg_list, int count)
{
	check_arg_count(selectBone, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int ct = 0;

	Value* vertval = arg_list[1];

	int boneID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
	int selID = boneID;
	boneID = check_bone_index(bmod, boneID);


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		SendMessage(GetDlgItem(bmod->hParam,IDC_LIST1),
					LB_SETCURSEL ,selID,0);
//		bmod->reevaluate =TRUE;
		bmod->UpdatePropInterface();
		if (bmod->ModeBoneIndex != boneID)
//watje 9-7-99  198721 
		bmod->Reevaluate(TRUE);
		bmod->ModeBoneIndex = boneID;
		bmod->updateP = TRUE;

		bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

//		SendMessage(GetDlgItem(bmod->hParam,IDC_LIST1),
//					WM_COMMAND ,MAKEWPARAM(IDC_LIST1,LBN_SELCHANGE),NULL);
		bmod->ip->RedrawViews(bmod->ip->GetTime());

		}

	return &ok;	

}


Value*
getSelectedBone_cf(Value** arg_list, int count)
{
	check_arg_count(getSelectedBone, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int ct = 0;

	ct = SendMessage(GetDlgItem(bmod->hParam,IDC_LIST1),LB_GETCURSEL,0,0)+1;
//	ct = bmod->ModeBoneIndex+1;

	return Integer::intern(ct);		

}


Value*
getNumberCrossSections_cf(Value** arg_list, int count)
{
	check_arg_count(getNumberCrossSections, 2, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	int ct = 0;

	Value* vertval = arg_list[1];

	int boneID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
	boneID = check_bone_index(bmod, boneID);


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (bmod->BoneData[boneID].Node)
			ct = bmod->BoneData[boneID].CrossSectionList.Count();
		}

	return Integer::intern(ct);			

}


Value*
getInnerRadius_cf(Value** arg_list, int count)
{
	check_arg_count(getInnerRadius, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	float w = 0;

	Value* vertval = arg_list[1];
	Value* crossval = arg_list[2];

	int boneID;
	int crossID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
	boneID = check_bone_index(bmod, boneID);

	if (!is_number(crossval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else crossID = crossval->to_int()-1;


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (bmod->BoneData[boneID].Node)
			{
			if ( (crossID < bmod->BoneData[boneID].CrossSectionList.Count()) &&
				 (crossID >= 0)
				 )
				{
				Interval v;
				bmod->BoneData[boneID].CrossSectionList[crossID].InnerControl->GetValue(bmod->currentTime,&w,v);
				}
			else throw RuntimeError(GetString(IDS_PW_EXCEEDED_CROSS_COUNT), arg_list[0]);
			}
		}

	return Float::intern(w);			

}

Value*
getOuterRadius_cf(Value** arg_list, int count)
{
	check_arg_count(getOuterRadius, 3, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	float w = 0;

	Value* vertval = arg_list[1];
	Value* crossval = arg_list[2];

	int boneID;
	int crossID;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
	boneID = check_bone_index(bmod, boneID);

	if (!is_number(crossval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else crossID = crossval->to_int()-1;


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (bmod->BoneData[boneID].Node)
			{
			if ( (crossID < bmod->BoneData[boneID].CrossSectionList.Count()) &&
				 (crossID >= 0)
				 )
				{
				Interval v;
				bmod->BoneData[boneID].CrossSectionList[crossID].OuterControl->GetValue(bmod->currentTime,&w,v);
					
				}
			else throw RuntimeError(GetString(IDS_PW_EXCEEDED_CROSS_COUNT), arg_list[0]);
			}
		}

	return Float::intern(w);			

}


Value*
setInnerRadius_cf(Value** arg_list, int count)
{
	check_arg_count(setInnerRadius, 4, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	float w = 0;

	Value* vertval = arg_list[1];
	Value* crossval = arg_list[2];
	Value* radiusval = arg_list[3];

	int boneID;
	int crossID;
	float radius;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
	boneID = check_bone_index(bmod, boneID);

	if (!is_number(crossval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else crossID = crossval->to_int()-1;

	if (!is_number(radiusval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else radius = radiusval->to_float();


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (bmod->BoneData[boneID].Node)
			{
			if ( (crossID < bmod->BoneData[boneID].CrossSectionList.Count()) &&
				 (crossID >= 0)
				 )
				{
				Interval v;
				bmod->BoneData[boneID].CrossSectionList[crossID].InnerControl->SetValue(bmod->currentTime,&radius);
					
				}
			else throw RuntimeError(GetString(IDS_PW_EXCEEDED_CROSS_COUNT), arg_list[0]);
			}
		}

	if (bmod->ModeBoneIndex != boneID)
//watje 9-7-99  198721 
		bmod->Reevaluate(TRUE);

	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());


	return &ok;			

}

Value*
setOuterRadius_cf(Value** arg_list, int count)
{
	check_arg_count(setOuterRadius, 4, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	float w = 0;

	Value* vertval = arg_list[1];
	Value* crossval = arg_list[2];
	Value* radiusval = arg_list[3];

	int boneID;
	int crossID;
	float radius;
	if (!is_number(vertval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else boneID = vertval->to_int()-1;
	boneID = check_bone_index(bmod, boneID);

	if (!is_number(crossval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else crossID = crossval->to_int()-1;

	if (!is_number(radiusval)) 
		{
		throw RuntimeError(GetString(IDS_PW_INVALID_DATA), arg_list[0]);
		}
	else radius = radiusval->to_float();


//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		BoneModData *bmd = (BoneModData*)mcList[0]->localData;
		if (bmod->BoneData[boneID].Node)
			{
			if ( (crossID < bmod->BoneData[boneID].CrossSectionList.Count()) &&
				 (crossID >= 0)
				 )
				{
				Interval v;
				bmod->BoneData[boneID].CrossSectionList[crossID].OuterControl->SetValue(bmod->currentTime,&radius);
				}
			else throw RuntimeError(GetString(IDS_PW_EXCEEDED_CROSS_COUNT), arg_list[0]);
			}
		}

	if (bmod->ModeBoneIndex != boneID)
//watje 9-7-99  198721 
		bmod->Reevaluate(TRUE);

	bmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	bmod->ip->RedrawViews(bmod->ip->GetTime());


	return &ok;			

}



/*-------------------------------------------------------------------*/
/*																	*/
/*				Create Cross Section Command Mode					*/
/*																	*/
/*-------------------------------------------------------------------*/

HCURSOR CreateCrossSectionMouseProc::GetTransformCursor() 
        { 
        static HCURSOR hCur = NULL;

        if ( !hCur ) {
                hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_SEGREFINECUR)); 
                }

        return hCur; 
        }



BOOL CreateCrossSectionMouseProc::HitTest( 
                ViewExp *vpt, IPoint2 *p, int type, int flags )

        {

//do a poly hit test
        int savedLimits, res = 0;
        GraphicsWindow *gw = vpt->getGW();


//		float scale = 1.0f;



        HitRegion hr;
        MakeHitRegion(hr,type, 1,8,p);
        gw->setHitRegion(&hr);
        gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
        gw->setTransform(Matrix3(1));
        gw->clearHitCode();

        gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);


		if (mod->BoneData[mod->ModeBoneIndex].flags & BONE_SPLINE_FLAG)
			{
			ShapeObject *pathOb = NULL;
			ObjectState os = mod->BoneData[mod->ModeBoneIndex].Node->EvalWorldState(mod->ip->GetTime());
			pathOb = (ShapeObject*)os.obj;
//196241 
			if (pathOb->NumberOfCurves() != 0)
				{
				Matrix3 tm = mod->BoneData[mod->ModeBoneIndex].Node->GetObjectTM(mod->ip->GetTime());
		        Point3 plist[2];
				SplineU = -1.0f;
				float u = 0.0f;
				for (int spid = 0; spid < 100; spid++)
					{
					plist[0] = pathOb->InterpCurve3D(mod->ip->GetTime(), 0,u) * tm;
					plist[1] = pathOb->InterpCurve3D(mod->ip->GetTime(), 0,u+0.01f) * tm;
					u += 0.01f;
				    gw->polyline(2, plist, NULL, NULL, 0);
					if (gw->checkHitCode()) 
						{
				        res = TRUE;
					    gw->clearHitCode();
						a = plist[0];
						b = plist[1];
						SplineU = u;
						spid = 100;
						}
					gw->clearHitCode();
					}
		
				}

			}
		else
			{
	        Point3 plist[2];
		    plist[0] = mod->Worldl1;
			plist[1] = mod->Worldl2;

	        gw->polyline(2, plist, NULL, NULL, 0);
		    if (gw->checkHitCode()) {
			        res = TRUE;
//              vpt->CtrlLogHit(NULL,gw->getHitDistance(),0,0);

				    gw->clearHitCode();
					}
			gw->clearHitCode();
			}

        gw->setRndLimits(savedLimits);

        return res;


        }

int CreateCrossSectionMouseProc::proc(
                        HWND hwnd, 
                        int msg, 
                        int point, 
                        int flags, 
                        IPoint2 m )
        {
        ViewExp *vpt = iObjParams->GetViewport(hwnd);   
        int res = TRUE;
		if ( !mod->ip ) return FALSE;


			switch ( msg ) {
                case MOUSE_PROPCLICK:
                        iObjParams->SetStdCommandMode(CID_OBJMOVE);
                        break;

                case MOUSE_POINT:
                        if(HitTest(vpt,&m,HITTYPE_POINT,0) ) {

								theHold.Begin();
								theHold.Put(new PasteRestore(mod));
								theHold.Accept(GetString(IDS_PW_ADDCROSSSECTION));


//transfrom mouse point to world
								float u;
								if (mod->BoneData[mod->ModeBoneIndex].flags & BONE_SPLINE_FLAG)
									GetHit(u);
								else u = mod->GetU(vpt,mod->Worldl1,mod->Worldl2, m);
									
								if (u <= 0.0f) u = 0.0001f;
								if (u >= 1.0f) u = 0.9999f;
                                mod->AddCrossSection(u);
								macroRecorder->FunctionCall(_T("skinOps.addCrossSection"), 2,0, mr_reftarg, mod,
 															 mr_float,u
															 );
                                BOOL s = FALSE;

//watje 9-7-99  198721 
                                mod->Reevaluate(TRUE);
                                mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
                                mod->ip->RedrawViews(mod->ip->GetTime());
                                }
                        res = FALSE;
                        break;
                
                case MOUSE_FREEMOVE:
                        if ( HitTest(vpt,&m,HITTYPE_POINT,HIT_ABORTONHIT) ) {
                                SetCursor(LoadCursor(NULL,IDC_CROSS ));
                                 }
                        else {
                                SetCursor(LoadCursor(NULL,IDC_ARROW));
                                }
                        break;
                        
                }

        if ( vpt ) iObjParams->ReleaseViewport(vpt);
        return res;
        }


/*-------------------------------------------------------------------*/

void CreateCrossSectionMode::EnterMode()
        {
        mod->iCrossSectionButton->SetCheck(TRUE);
        }

void CreateCrossSectionMode::ExitMode()
        {
        mod->iCrossSectionButton->SetCheck(FALSE);
        }

void BonesDefMod::StartCrossSectionMode(int type)
        {
        if ( !ip ) return;

		if (ip->GetCommandMode() == CrossSectionMode) {
			ip->SetStdCommandMode(CID_OBJMOVE);
			return;
			}


        CrossSectionMode->SetType(type);
        ip->SetCommandMode(CrossSectionMode);
        }





/*-------------------------------------------------------------------*/

static void BoneXORDottedLine( HWND hwnd, IPoint2 p0, IPoint2 p1 )
	{
	HDC hdc;
	hdc = GetDC( hwnd );
	SetROP2( hdc, R2_XORPEN );
	SetBkMode( hdc, TRANSPARENT );
	SelectObject( hdc, CreatePen( PS_DOT, 0, ComputeViewportXORDrawColor() ) );
	MoveToEx( hdc, p0.x, p0.y, NULL );
	LineTo( hdc, p1.x, p1.y );		
	DeleteObject( SelectObject( hdc, GetStockObject( BLACK_PEN ) ) );
	ReleaseDC( hwnd, hdc );
	}

static void BoneXORDottedCircle( HWND hwnd, IPoint2 p0, float Radius )
	{
	HDC hdc;
	hdc = GetDC( hwnd );
	SetROP2( hdc, R2_XORPEN );
	SetBkMode( hdc, TRANSPARENT );
	SelectObject( hdc, CreatePen( PS_DOT, 0, ComputeViewportXORDrawColor() ) );
	MoveToEx( hdc, p0.x +(int)Radius, p0.y, NULL );
	float angle = 0.0f;
	float inc = 2.0f*PI/20.f;
	IPoint2 p1;
	for (int i = 0; i < 20; i++)
		{
		angle += inc;
		p1.x = (int)(Radius * sin(angle) + Radius * cos(angle));
		p1.y = (int)(Radius * sin(angle) - Radius * cos(angle));
		LineTo( hdc, p0.x + p1.x, p0.y+p1.y );		
		}

	DeleteObject( SelectObject( hdc, GetStockObject( BLACK_PEN ) ) );
	ReleaseDC( hwnd, hdc );
	}



//watje 9-7-99 198721 

class CacheModEnumProc : public ModContextEnumProc {
public:
	BonesDefMod *lm;
	CacheModEnumProc(BonesDefMod *l)
		{
		lm = l;
		}
private:
	BOOL proc (ModContext *mc);
};

BOOL CacheModEnumProc::proc (ModContext *mc) {
	if (mc->localData == NULL) return TRUE;

	BoneModData *bmd = (BoneModData *) mc->localData;
	bmd->CurrentCachePiece = -1;
	return TRUE;
}



/*-------------------------------------------------------------------*/
/*																	*/
/*				Select Bone Dialog Mode									*/
/*																	*/
/*-------------------------------------------------------------------*/



#define PERCENT_LENGTH	0.3f
#define PERCENT_LENGTH_CLOSED	0.1f
//#define INNER_MULT		1.0ff
//#define OUTER_MULT		3.5f

void BonesDefMod::AddBone(INode *node, BOOL update)
{


	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	assert(nodes.Count());
	Matrix3 ourTM;
	ourTM = nodes[0]->GetObjectTM(RefFrame);

//	BaseTM = ourTM;

	Class_ID bid(BONE_CLASS_ID,0);

	BOOL staticEnvelope;
	float staticInnerPercent;
	float staticOuterPercent;
	float staticOuter;
	float staticInner;
	pblock_param->GetValue(skin_initial_staticenvelope,0,staticEnvelope,FOREVER);
	pblock_param->GetValue(skin_initial_envelope_innerpercent,0,staticInnerPercent,FOREVER);
	pblock_param->GetValue(skin_initial_envelope_outerpercent,0,staticOuterPercent,FOREVER);
	pblock_param->GetValue(skin_initial_envelope_inner,0,staticInner,FOREVER);
	pblock_param->GetValue(skin_initial_envelope_outer,0,staticOuter,FOREVER);

	
	
	
	


//need to add subcount for shapes also
	int subcount = 1;
	ObjectState os = node->EvalWorldState(RefFrame);
	if (os.obj->ClassID() == bid)  
		{
//		subcount = nodeTab[i]->NumberOfChildren();
//		if (subcount == 0) subcount = 1;
		}
	else if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
		{
//get spline piecs
		}


	for (int j = 0; j < subcount; j++)
		{

//		Object *obj = nodes[0]->EvalWorldState(RefFrame).obj;

		BoneDataClass t;
		t.Node = node;
		TCHAR title[200];
		_tcscpy(title,node->GetName());


		int current=-1;
		BOOL found = FALSE;
		for (int bct = 0; bct < BoneData.Count();bct++)
			{
			if (BoneData[bct].Node == NULL) 
				{
				current = bct;
				found = TRUE;
				bct = BoneData.Count();
				}
			}
		if (!found)
			current = BoneData.Count();
		int BoneRefID = GetOpenID();
		int End1RefID = GetOpenID();
		int End2RefID = GetOpenID();

		if (current != -1) {

//			Matrix3 ntm = t.Node->GetObjectTM(RefFrame);	
			Matrix3 otm = t.Node->GetObjectTM(RefFrame);  //ns	
//5.1.03		
			Matrix3 stretchTM = t.Node->GetStretchTM(RefFrame);
			Matrix3 ntm =  t.Node->GetNodeTM(RefFrame);	


//append a new bone
			BoneDataClass t;
			if (!found)
				BoneData.Append(t);
//5.1.03
			BoneData[current].InitStretchTM =  stretchTM;
			BoneData[current].Node = NULL;
			BoneData[current].EndPoint1Control = NULL;
			BoneData[current].EndPoint2Control = NULL;
		
			BoneData[current].InitObjectTM = otm;		//ns
			BoneData[current].tm    = Inverse(otm);
			if (hasStretchTM)
				BoneData[current].InitNodeTM = ntm;
			else BoneData[current].InitNodeTM = stretchTM * ntm;
//			BoneData[current].tm    = Inverse(ntm);

			for(int k = 0 ; k < mcList.Count() ; k++)
				{
				BoneModData *bmd = (BoneModData*)mcList[k]->localData;
				if (bmd->pSE)
					bmd->pSE->SetNumBones(BoneData.Count());
//				bmd->pSE->SetInitBoneTM(current,(float *) &otm);
				}
			
			BoneData[current].CrossSectionList.ZeroCount();


			Point3 l1(0.0f,0.0f,0.0f),l2(0.0f,0.0f,0.0f);
		

//object is bone use its first child as the axis
			BoneData[current].flags = 0;
			if (os.obj->ClassID() == bid) 
				{
				l1.x = 0.0f;
				l1.y = 0.0f;
				l1.z = 0.0f;
				l2.x = 0.0f;
				l2.y = 0.0f;
				l2.z = 0.0f;
//get child node
				INode* parent = node->GetParentNode();
//				ntm = t.Node->GetObjectTM(RefFrame);	
//				ntm = parent->GetObjectTM(RefFrame);	
				otm = parent->GetObjectTM(RefFrame);	//ns
//5.1.03
				stretchTM = parent->GetStretchTM(RefFrame);
				ntm =  parent->GetNodeTM(RefFrame);
//				BoneData[current].tm    = Inverse(ntm);
				BoneData[current].InitObjectTM = otm;  //ns
				BoneData[current].tm    = Inverse(otm);
//5.1.03
				if (hasStretchTM)
					BoneData[current].InitNodeTM = ntm;
				else BoneData[current].InitNodeTM = stretchTM * ntm;
				BoneData[current].InitStretchTM = stretchTM;


				for(int k = 0 ; k < mcList.Count() ; k++)
					{
					BoneModData *bmd = (BoneModData*)mcList[k]->localData;
					bmd->pSE->SetNumBones(BoneData.Count());
//					bmd->pSE->SetInitBoneTM(current,(float *) &otm);
					}


//				if (nodeTab[i]->NumberOfChildren() > 0)
				if (1)
					{
//					INode *child = nodeTab[i]->GetChildNode(j);
					Matrix3 ChildTM = node->GetObjectTM(RefFrame);

					_tcscpy(title,node->GetName());


					l2 = l2 * ChildTM;
					l2 = l2 * Inverse(otm);   //ns
					Point3 Vec = (l2-l1);
					l1 += Vec * 0.1f;
					l2 -= Vec * 0.1f;
					}
				else 
					{
					l2.x = 0.0f;
					l2.y = 0.0f;
					l2.z = 50.0f;
					}
				float el1 = 9999999999999.0f,el2 = 999999999.0f;
//				float d = Length(l2-l1);
//				if (d < 0.1f) d = 10.f;
//				el1 = d * PERCENT_LENGTH;

				int objects = mcList.Count();
				Point3 ll1 = l1 * Inverse(BoneData[current].tm);
				Point3 ll2 = l2 * Inverse(BoneData[current].tm);

				for (int nc = 0; nc < nodes.Count(); nc++)
					{
					ObjectState base_os = nodes[nc]->EvalWorldState(ip->GetTime());
					
					BuildEnvelopes(nodes[nc], base_os.obj, ll1, ll2, el1, el2);
					}



				float e_inner, e_outer;
				e_inner = el1*staticInnerPercent ;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el2*staticInnerPercent ;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				AddCrossSection(current, 1.0f, e_inner,e_outer);
				BoneData[current].flags = BONE_BONE_FLAG;

				}
//object is bone use its first child as the axis
			else if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
				{
//build distance based on spline
				BoneData[current].flags = BoneData[current].flags|BONE_SPLINE_FLAG;
				ShapeObject *pathOb = NULL;
				ObjectState os = node->EvalWorldState(RefFrame);


				BezierShape bShape;
				ShapeObject *shape = (ShapeObject *)os.obj;
				if(shape->CanMakeBezier())
//watje 9-7-99  195862 
					shape->MakeBezier(RefFrame, bShape);
//					shape->MakeBezier(ip->GetTime(), bShape);
				else {
					PolyShape pShape;
//watje 9-7-99  195862 
					shape->MakePolyShape(RefFrame, pShape);
//					shape->MakePolyShape(ip->GetTime(), pShape);
					bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
					}

				pathOb = (ShapeObject*)os.obj;

				if (bShape.splines[0]->Closed() )
					BoneData[current].flags = BoneData[current].flags|BONE_SPLINECLOSED_FLAG;

//watje 9-7-99  195862 
				l1 = pathOb->InterpCurve3D(RefFrame, 0, 0.0f, SPLINE_INTERP_SIMPLE);			
//				l1 = pathOb->InterpCurve3D(0, 0, 0.0f, SPLINE_INTERP_SIMPLE);			
//watje 9-7-99  195862 
				l2 = pathOb->InterpCurve3D(RefFrame, 0, 1.0f, SPLINE_INTERP_SIMPLE);			
//				l2 = pathOb->InterpCurve3D(0, 0, 1.0f, SPLINE_INTERP_SIMPLE);			

				float el1 = 0.0f,el2 = 0.0f;
				float s1 = bShape.splines[0]-> SplineLength();
				
				Matrix3 tempTM = Inverse(BoneData[current].tm)*Inverse(ourTM);
				BuildMajorAxis(node,l1,l2,el1,&tempTM); 
				if (el1< 0.1f) el1 = staticInner;
				el1 += 10.0f;
				el1 = el1 * 0.5f;
				el2= el1;

/*
				if (bShape.splines[0]->Closed() )
					el1 = s1 * PERCENT_LENGTH_CLOSED;
				else el1 = s1 * PERCENT_LENGTH;
*/
				float e_inner, e_outer;
				e_inner = el1 *staticInnerPercent;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el1 *staticInnerPercent;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				AddCrossSection(current, 1.0f, e_inner,e_outer);

//copy initial reference spline into our spline
//				SplineShape *shape = (SplineShape *)os.obj;

				BoneData[current].referenceSpline = *bShape.splines[0];

				}

			else 
				{
				float el1 = 99999999.0f,el2 = 99999999.0f;
				Matrix3 tempTM;
				tempTM = Inverse(BoneData[current].tm)*Inverse(ourTM);
				BuildMajorAxis(node,l1,l2,el1,&tempTM); 

				el1 = el1 * 0.5f;
				el2= el1;
//				float d = Length(l2-l1);
//				if (d < 0.1f) d = 10.f;
//				el1 = d * PERCENT_LENGTH;

//				ip->GetModContexts(mcList,nodes);
				int objects = mcList.Count();
				Point3 ll1 = l1 * Inverse(BoneData[current].tm);
				Point3 ll2 = l2 * Inverse(BoneData[current].tm);

//				for (int nc = 0; nc < nodes.Count(); nc++)
//					{
//					ObjectState base_os = nodes[nc]->EvalWorldState(ip->GetTime());
					
//					BuildEnvelopes(nodes[nc], base_os.obj, ll1, ll2, el1, el2);
//					}


				float e_inner, e_outer;
				e_inner = el1 *staticInnerPercent;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				if (e_inner <= 0.001f) e_inner = 5.0f;
				if (e_outer <= 0.001f) e_outer = 10.f;

				if (Length(l1-l2) < 0.05f)  
					{
					l1.x *= 5.0f;
					l2.x *= 5.0f;
					}

				AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el2 *staticInnerPercent;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				if (e_inner <= 0.001f) e_inner = 5.0f;
				if (e_outer <= 0.001f) e_outer = 10.f;

				AddCrossSection(current, 1.0f, e_inner,e_outer);

				}

			l1 = l1;// * Inverse(BoneData[current].tm);
			l2 = l2;// * Inverse(BoneData[current].tm);
			BoneData[current].flags = BoneData[current].flags|BONE_ABSOLUTE_FLAG;
			BoneData[current].FalloffType = 0;

			BoneData[current].BoneRefID = BoneRefID;
			BoneData[current].RefEndPt1ID = End1RefID;
			BoneData[current].RefEndPt2ID = End2RefID;

			BoneData[current].end1Selected = FALSE;
			BoneData[current].end2Selected = FALSE;


			if (os.obj->ClassID() == bid) 
				{
//get child node
				INode* parent = node->GetParentNode();
				ReplaceReference(BoneRefID,parent,FALSE);
				BoneData[current].name = title;
				}
			else ReplaceReference(BoneRefID,node,FALSE);

			INode *n = BoneData[current].Node;
			if (n)
				{
				float squash = GetSquash(ip->GetTime(), n);
				if (current>=pblock_param->Count(skin_local_squash))
					{
					float f = 1.0f;
					pblock_param->Append(skin_local_squash,1,&f);
					}
				else 
					{
					pblock_param->SetValue(skin_local_squash,ip->GetTime(),1.0f,current);
					}

				if (current>=pblock_param->Count(skin_initial_squash))
					{
					pblock_param->Append(skin_initial_squash,1,&squash);
					}
				else 
					{
					pblock_param->SetValue(skin_initial_squash,ip->GetTime(),squash,current);
					}


				MakeRefByID(FOREVER,End1RefID,NewDefaultPoint3Controller());
				MakeRefByID(FOREVER,End2RefID,NewDefaultPoint3Controller());
				BoneData[current].EndPoint1Control->SetValue(currentTime,&l1,TRUE,CTRL_ABSOLUTE);
				BoneData[current].EndPoint2Control->SetValue(currentTime,&l2,TRUE,CTRL_ABSOLUTE);


				int rsel = ConvertSelectedBoneToListID(current);

				SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_INSERTSTRING,(WPARAM) rsel,(LPARAM)(TCHAR*)title);
				}


			nodes.DisposeTemporary();
			}
		

		ModeBoneIndex = current;
		ModeBoneEndPoint = -1;
		ModeBoneEnvelopeIndex = -1;
		ModeBoneEnvelopeSubType = -1;
		SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_SETCURSEL ,current,0);

/*		if (BoneData[ModeBoneIndex].flags & BONE_LOCK_FLAG)
			pblock_param->SetValue(PB_LOCK_BONE,0,1);
		else
			pblock_param->SetValue(PB_LOCK_BONE,0,0);


		if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
			pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
		else
			pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);
*/
//watje 9-7-99  198721 
		Reevaluate(TRUE);
		}

if (update)
	{
	
	if ( (BoneData.Count() >0) && (ip && ip->GetSubObjectLevel() == 1) )
		{
		EnableButtons();
		}

	if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
		{
		iAbsolute->SetCheck(FALSE);

	//	pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
		}
	else
		{
		iAbsolute->SetCheck(TRUE);
	//	pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);
		}


	if (BoneData[ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
		{
//		pblock_param->SetValue(PB_DRAW_BONE_ENVELOPE,0,1);
		iEnvelope->SetCheck(TRUE);

		}
	else
		{
//		pblock_param->SetValue(PB_DRAW_BONE_ENVELOPE,0,0);
		iEnvelope->SetCheck(FALSE);
		}

	if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_X_FLAG)
		iFalloff->SetCurFlyOff(0,FALSE);
	else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_SINE_FLAG)
		iFalloff->SetCurFlyOff(1,FALSE);
	else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_X3_FLAG)
		iFalloff->SetCurFlyOff(3,FALSE);
	else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_3X_FLAG)
		iFalloff->SetCurFlyOff(2,FALSE);

	NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//eo->CurrentCachePiece = -1;
	cacheValid = FALSE;
	}
//WEIGHTTABLE
weightTableWindow.RecomputeBones();
NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);


}



BOOL BonesDefMod::AddBoneEx(INode *node, BOOL update)
{

//make sure the node is valid
	node->BeginDependencyTest();
	NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
//check to make sure it is not circular
	if (node->EndDependencyTest()) return FALSE;

//check to make sure it is not a nurbs curve
	ObjectState os = node->EvalWorldState(0);
	if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
		{
		if ( (os.obj->ClassID()==EDITABLE_SURF_CLASS_ID) )
			return FALSE;
//196241 
		ShapeObject *pathOb = (ShapeObject*)os.obj;
		if (pathOb->NumberOfCurves() == 0) return FALSE;
		}	


//	ModContextList mcList;
//	INodeTab nodes;
//	ip->GetModContexts(mcList,nodes);
	Tab<INode*> nodes;

	MyEnumProc dep;              
	EnumDependents(&dep);
	nodes = dep.Nodes;

	if (nodes.Count() == 0) return FALSE;


	Matrix3 ourTM;
	ourTM = nodes[0]->GetObjectTM(RefFrame);

//	BaseTM = ourTM;

	Class_ID bid(BONE_CLASS_ID,0);

	BOOL staticEnvelope;
	float staticInnerPercent;
	float staticOuterPercent;
	float staticOuter;
	float staticInner;
	pblock_param->GetValue(skin_initial_staticenvelope,0,staticEnvelope,FOREVER);
	pblock_param->GetValue(skin_initial_envelope_innerpercent,0,staticInnerPercent,FOREVER);
	pblock_param->GetValue(skin_initial_envelope_outerpercent,0,staticOuterPercent,FOREVER);
	pblock_param->GetValue(skin_initial_envelope_inner,0,staticInner,FOREVER);
	pblock_param->GetValue(skin_initial_envelope_outer,0,staticOuter,FOREVER);

	
	
	
	


//need to add subcount for shapes also
	int subcount = 1;
	os = node->EvalWorldState(RefFrame);
	if (os.obj->ClassID() == bid)  
		{
//		subcount = nodeTab[i]->NumberOfChildren();
//		if (subcount == 0) subcount = 1;
		}
	else if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
		{
//get spline piecs
		}


	for (int j = 0; j < subcount; j++)
		{

//		Object *obj = nodes[0]->EvalWorldState(RefFrame).obj;

		BoneDataClass t;
		t.Node = node;
		TCHAR title[200];
		_tcscpy(title,node->GetName());


		int current=-1;
		BOOL found = FALSE;
		for (int bct = 0; bct < BoneData.Count();bct++)
			{
			if (BoneData[bct].Node == NULL) 
				{
				current = bct;
				found = TRUE;
				bct = BoneData.Count();
				}
			}
		if (!found)
			current = BoneData.Count();
		int BoneRefID = GetOpenID();
		int End1RefID = GetOpenID();
		int End2RefID = GetOpenID();

		if (current != -1) {

//			Matrix3 ntm = t.Node->GetObjectTM(RefFrame);	
			Matrix3 otm = t.Node->GetObjectTM(RefFrame);  //ns	
//5.1.03
//			Matrix3 ntm = t.Node->GetStretchTM(RefFrame) * t.Node->GetNodeTM(RefFrame);	
			Matrix3 stretchTM = t.Node->GetStretchTM(RefFrame);
			Matrix3 ntm = t.Node->GetNodeTM(RefFrame);	


//append a new bone
			BoneDataClass t;
			if (!found)
				BoneData.Append(t);
			BoneData[current].Node = NULL;
			BoneData[current].EndPoint1Control = NULL;
			BoneData[current].EndPoint2Control = NULL;
//5.1.03		
			BoneData[current].InitStretchTM = stretchTM;

			BoneData[current].InitObjectTM = otm;		//ns
			BoneData[current].tm    = Inverse(otm);
			if (hasStretchTM)
				BoneData[current].InitNodeTM = ntm;
			else BoneData[current].InitNodeTM = stretchTM * ntm;
//			BoneData[current].tm    = Inverse(ntm);

			
			BoneData[current].CrossSectionList.ZeroCount();


			Point3 l1(0.0f,0.0f,0.0f),l2(0.0f,0.0f,0.0f);
		

//object is bone use its first child as the axis
			BoneData[current].flags = 0;
			if (os.obj->ClassID() == bid) 
				{
				l1.x = 0.0f;
				l1.y = 0.0f;
				l1.z = 0.0f;
				l2.x = 0.0f;
				l2.y = 0.0f;
				l2.z = 0.0f;
//get child node
				INode* parent = node->GetParentNode();
//				ntm = t.Node->GetObjectTM(RefFrame);	
//				ntm = parent->GetObjectTM(RefFrame);	
				otm = parent->GetObjectTM(RefFrame);	//ns
//5.1.03
				stretchTM = parent->GetStretchTM(RefFrame);
				ntm = parent->GetNodeTM(RefFrame);
//				BoneData[current].tm    = Inverse(ntm);
				BoneData[current].InitObjectTM = otm;  //ns
//5.1.03
				BoneData[current].InitStretchTM = stretchTM;  //ns

				BoneData[current].tm    = Inverse(otm);

				if (hasStretchTM)
					BoneData[current].InitNodeTM = ntm;
				else BoneData[current].InitNodeTM = stretchTM * ntm;



//				if (nodeTab[i]->NumberOfChildren() > 0)
				if (1)
					{
//					INode *child = nodeTab[i]->GetChildNode(j);
					Matrix3 ChildTM = node->GetObjectTM(RefFrame);

					_tcscpy(title,node->GetName());


					l2 = l2 * ChildTM;
					l2 = l2 * Inverse(otm);   //ns
					Point3 Vec = (l2-l1);
					l1 += Vec * 0.1f;
					l2 -= Vec * 0.1f;
					}
				else 
					{
					l2.x = 0.0f;
					l2.y = 0.0f;
					l2.z = 50.0f;
					}
				float el1 = 9999999999999.0f,el2 = 999999999.0f;
//				float d = Length(l2-l1);
//				if (d < 0.1f) d = 10.f;
//				el1 = d * PERCENT_LENGTH;

				Point3 ll1 = l1 * Inverse(BoneData[current].tm);
				Point3 ll2 = l2 * Inverse(BoneData[current].tm);

				for (int nc = 0; nc < nodes.Count(); nc++)
					{
					ObjectState base_os = nodes[nc]->EvalWorldState(GetCOREInterface()->GetTime());
					
					BuildEnvelopes(nodes[nc], base_os.obj, ll1, ll2, el1, el2);
					}



				float e_inner, e_outer;
				e_inner = el1*staticInnerPercent ;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el2*staticInnerPercent ;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				AddCrossSection(current, 1.0f, e_inner,e_outer);
				BoneData[current].flags = BONE_BONE_FLAG;

				}
//object is bone use its first child as the axis
			else if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
				{
//build distance based on spline
				BoneData[current].flags = BoneData[current].flags|BONE_SPLINE_FLAG;
				ShapeObject *pathOb = NULL;
				ObjectState os = node->EvalWorldState(RefFrame);


				BezierShape bShape;
				ShapeObject *shape = (ShapeObject *)os.obj;
				if(shape->CanMakeBezier())
//watje 9-7-99  195862 
					shape->MakeBezier(RefFrame, bShape);
//					shape->MakeBezier(ip->GetTime(), bShape);
				else {
					PolyShape pShape;
//watje 9-7-99  195862 
					shape->MakePolyShape(RefFrame, pShape);
//					shape->MakePolyShape(ip->GetTime(), pShape);
					bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
					}

				pathOb = (ShapeObject*)os.obj;

				if (bShape.splines[0]->Closed() )
					BoneData[current].flags = BoneData[current].flags|BONE_SPLINECLOSED_FLAG;

//watje 9-7-99  195862 
				l1 = pathOb->InterpCurve3D(RefFrame, 0, 0.0f, SPLINE_INTERP_SIMPLE);			
//				l1 = pathOb->InterpCurve3D(0, 0, 0.0f, SPLINE_INTERP_SIMPLE);			
//watje 9-7-99  195862 
				l2 = pathOb->InterpCurve3D(RefFrame, 0, 1.0f, SPLINE_INTERP_SIMPLE);			
//				l2 = pathOb->InterpCurve3D(0, 0, 1.0f, SPLINE_INTERP_SIMPLE);			

				float el1 = 0.0f,el2 = 0.0f;
				float s1 = bShape.splines[0]-> SplineLength();
				
				Matrix3 tempTM = Inverse(BoneData[current].tm)*Inverse(ourTM);
				BuildMajorAxis(node,l1,l2,el1,&tempTM); 
				if (el1< 0.1f) el1 = staticInner;
				el1 += 10.0f;
				el1 = el1 * 0.5f;
				el2= el1;

				float e_inner, e_outer;
				e_inner = el1 *staticInnerPercent;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el1 *staticInnerPercent;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}

				AddCrossSection(current, 1.0f, e_inner,e_outer);

//copy initial reference spline into our spline
//				SplineShape *shape = (SplineShape *)os.obj;

				BoneData[current].referenceSpline = *bShape.splines[0];

				}

			else 
				{
				float el1 = 99999999.0f,el2 = 99999999.0f;
				Matrix3 tempTM;
				tempTM = Inverse(BoneData[current].tm)*Inverse(ourTM);
				BuildMajorAxis(node,l1,l2,el1,&tempTM); 

				el1 = el1 * 0.5f;
				el2= el1;

				Point3 ll1 = l1 * Inverse(BoneData[current].tm);
				Point3 ll2 = l2 * Inverse(BoneData[current].tm);



				float e_inner, e_outer;
				e_inner = el1 *staticInnerPercent;
				e_outer = e_inner *staticOuterPercent;

				if (e_inner <= 0.001f) e_inner = 5.0f;
				if (e_outer <= 0.001f) e_outer = 10.0f;

				if (Length(l1-l2) < 0.05f)  
					{
					l1.x *= 5.0f;
					l2.x *= 5.0f;
					}


				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}
				AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el2 *staticInnerPercent;
				e_outer = e_inner *staticOuterPercent;
				if (staticEnvelope)
					{
					e_inner=staticInner;
					e_outer=staticOuter;
					}


				if (e_inner <= 0.001f) e_inner = 5.0f;
				if (e_outer <= 0.001f) e_outer = 10.0f;

				AddCrossSection(current, 1.0f, e_inner,e_outer);

				}

			l1 = l1;// * Inverse(BoneData[current].tm);
			l2 = l2;// * Inverse(BoneData[current].tm);
			BoneData[current].flags = BoneData[current].flags|BONE_ABSOLUTE_FLAG;
			BoneData[current].FalloffType = 0;

			BoneData[current].BoneRefID = BoneRefID;
			BoneData[current].RefEndPt1ID = End1RefID;
			BoneData[current].RefEndPt2ID = End2RefID;

			BoneData[current].end1Selected = FALSE;
			BoneData[current].end2Selected = FALSE;


			if (os.obj->ClassID() == bid) 
				{
//get child node
				INode* parent = node->GetParentNode();
				ReplaceReference(BoneRefID,parent,FALSE);
				BoneData[current].name = title;
				}
			else ReplaceReference(BoneRefID,node,FALSE);

			INode *n = BoneData[current].Node;
			if (n)
				{
				float squash = GetSquash(GetCOREInterface()->GetTime(), n);
				if (current>=pblock_param->Count(skin_local_squash))
					{
					float f = 1.0f;
					pblock_param->Append(skin_local_squash,1,&f);
					}
				else 
					{
					pblock_param->SetValue(skin_local_squash,GetCOREInterface()->GetTime(),1.0f,current);
					}

				if (current>=pblock_param->Count(skin_initial_squash))
					{
					pblock_param->Append(skin_initial_squash,1,&squash);
					}
				else 
					{
					pblock_param->SetValue(skin_initial_squash,GetCOREInterface()->GetTime(),squash,current);
					}


				MakeRefByID(FOREVER,End1RefID,NewDefaultPoint3Controller());
				MakeRefByID(FOREVER,End2RefID,NewDefaultPoint3Controller());
				BoneData[current].EndPoint1Control->SetValue(currentTime,&l1,TRUE,CTRL_ABSOLUTE);
				BoneData[current].EndPoint2Control->SetValue(currentTime,&l2,TRUE,CTRL_ABSOLUTE);


				int rsel = ConvertSelectedBoneToListID(current);

				SendMessage(GetDlgItem(hParam,IDC_LIST1),
					LB_INSERTSTRING,(WPARAM) rsel,(LPARAM)(TCHAR*)title);
				}


			}
		

		ModeBoneIndex = current;
		ModeBoneEndPoint = -1;
		ModeBoneEnvelopeIndex = -1;
		ModeBoneEnvelopeSubType = -1;
		SendMessage(GetDlgItem(hParam,IDC_LIST1),
				LB_SETCURSEL ,current,0);

//watje 9-7-99  198721 
		Reevaluate(TRUE);
		}

	if ((ip) && (update))
		{
	
		if ( (BoneData.Count() >0) && (ip && ip->GetSubObjectLevel() == 1) )
			{
			EnableButtons();
			}

		if (BoneData[ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
			{
			iAbsolute->SetCheck(FALSE);

			}
		else
			{
			iAbsolute->SetCheck(TRUE);
			}


		if (BoneData[ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
			{
			iEnvelope->SetCheck(TRUE);

			}
		else
			{
			iEnvelope->SetCheck(FALSE);
			}

		if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_X_FLAG)
			iFalloff->SetCurFlyOff(0,FALSE);
		else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_SINE_FLAG)
			iFalloff->SetCurFlyOff(1,FALSE);
		else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_X3_FLAG)
			iFalloff->SetCurFlyOff(3,FALSE);
		else if (BoneData[ModeBoneIndex].FalloffType == BONE_FALLOFF_3X_FLAG)
			iFalloff->SetCurFlyOff(2,FALSE);

		NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//eo->CurrentCachePiece = -1;
		cacheValid = FALSE;
		}

//WEIGHTTABLE
	weightTableWindow.RecomputeBones();
	NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
}



BOOL BonesDefMod::SetSkinTm(INode *skinNode, Matrix3 objectTm, Matrix3 nodeTm)
	{
//if no node bail
	if (skinNode == NULL) return FALSE;
//get the bmd
	BoneModData *bmd = GetBMD(skinNode);
	if (bmd == NULL) return FALSE;
	bmd->BaseTM = objectTm;
	bmd->BaseNodeTM = nodeTm; //ns
	bmd->InverseBaseTM = Inverse(bmd->BaseTM);

	return TRUE;
	}
BOOL BonesDefMod::SetBoneTm(INode *boneNode, Matrix3 objectTm, Matrix3 nodeTm)
	{
	if (boneNode == NULL) return FALSE;

	BOOL hit =  FALSE;
	for (int i=0; i < BoneData.Count(); i++)
			{
			if (boneNode == BoneData[i].Node)
				{
				BoneData[i].InitObjectTM = objectTm;		//ns
				BoneData[i].InitNodeTM = nodeTm;
				BoneData[i].tm    = Inverse(objectTm);
				BoneData[i].InitStretchTM.IdentityMatrix();
				return TRUE;
				}
			}


	return FALSE;
	}


BOOL BonesDefMod::AddWeights(INode *node, int vertexID, Tab<INode*> &nodeList, Tab<float> &weights)
{

//if no node bail
	if (node == NULL) return FALSE;
//get the local mod data if null return false
//check to make sure teh weight ad node list are the same size 
	if (nodeList.Count() != weights.Count()) return FALSE;
//get the bmd
	BoneModData *bmd = GetBMD(node);
	if (bmd == NULL) return FALSE;
//get bone indices from the node, if any do not exist bail
	Tab<int> boneIDList;
	boneIDList.SetCount(nodeList.Count());
	for (int i =0; i < nodeList.Count(); i++)
		{
		if (nodeList[i] == NULL) return FALSE;
		BOOL hit =  FALSE;
		for (int j=0; j < BoneData.Count(); j++)
			{
			if (nodeList[i] == BoneData[j].Node)
				{
				boneIDList[i] = j;
				hit = TRUE;
				j = BoneData.Count(); 
				}
			}
		if (!hit) return FALSE;
		}
//set the vertex data to modified
	if ((vertexID >=0) && (vertexID < bmd->VertexData.Count()))
		{
		bmd->VertexData[vertexID]->Modified(TRUE);
		bmd->VertexData[vertexID]->d.SetCount(boneIDList.Count());
		for (i = 0; i < boneIDList.Count(); i++)
			{
//set the weigtht list
			bmd->VertexData[vertexID]->d[i].Influences = weights[i];
			bmd->VertexData[vertexID]->d[i].normalizedInfluences = weights[i];
//set the bone list
			bmd->VertexData[vertexID]->d[i].Bones = boneIDList[i];
			}
		}
	else return FALSE;


return TRUE;
}

BOOL BonesDefMod::SetBoneStretchTm(INode *boneNode, Matrix3 stretchTm)
	{
	if (boneNode == NULL) return FALSE;

	BOOL hit =  FALSE;
	for (int i=0; i < BoneData.Count(); i++)
			{
			if (boneNode == BoneData[i].Node)
				{
				BoneData[i].InitStretchTM = stretchTm;
				return TRUE;
				}
			}


	return FALSE;
	}

Matrix3 BonesDefMod::GetBoneStretchTm(INode *boneNode)
	{
	if (boneNode == NULL) return FALSE;

	Matrix3 tm(1);
	BOOL hit =  FALSE;
	for (int i=0; i < BoneData.Count(); i++)
			{
			if (boneNode == BoneData[i].Node)
				{
				return BoneData[i].InitStretchTM;
				}
			}


	return tm;
	}

//This handles the output of particle info
//Hit Dialog




void DumpHitDialog::proc(INodeTab &nodeTab)

{


int nodeCount = nodeTab.Count(); 

if (nodeCount == 0) return;


for (int i=0;i<nodeTab.Count();i++)
	{

	eo->AddBone(nodeTab[i], FALSE);
	macroRecorder->FunctionCall(_T("skinOps.addBone"), 3,0, mr_reftarg, eo,
											  mr_reftarg, nodeTab[i], 
															 mr_int,1
															 );
	 macroRecorder->EmitScript();

/*
	ModContextList mcList;
	INodeTab nodes;
	eo->ip->GetModContexts(mcList,nodes);
	assert(nodes.Count());
	Matrix3 ourTM;
	ourTM = nodes[0]->GetObjectTM(eo->RefFrame);

//	eo->BaseTM = ourTM;

	Class_ID bid(BONE_CLASS_ID,0);




//need to add subcount for shapes also
	int subcount = 1;
	ObjectState os = nodeTab[i]->EvalWorldState(eo->RefFrame);
	if (os.obj->ClassID() == bid)  
		{
//		subcount = nodeTab[i]->NumberOfChildren();
//		if (subcount == 0) subcount = 1;
		}
	else if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
		{
//get spline piecs
		}


	for (int j = 0; j < subcount; j++)
		{

//		Object *obj = nodes[0]->EvalWorldState(eo->RefFrame).obj;

		BoneDataClass t;
		t.Node = nodeTab[i];
		TCHAR title[200];
		_tcscpy(title,nodeTab[i]->GetName());


		int current=-1;
		BOOL found = FALSE;
		for (int bct = 0; bct < eo->BoneData.Count();bct++)
			{
			if (eo->BoneData[bct].Node == NULL) 
				{
				current = bct;
				found = TRUE;
				bct = eo->BoneData.Count();
				}
			}
		if (!found)
			current = eo->BoneData.Count();
		int BoneRefID = eo->GetOpenID();
		int End1RefID = eo->GetOpenID();
		int End2RefID = eo->GetOpenID();

		if (current != -1) {

			Matrix3 ntm = t.Node->GetObjectTM(eo->RefFrame);	


//append a new bone
			BoneDataClass t;
			if (!found)
				eo->BoneData.Append(t);
			eo->BoneData[current].Node = NULL;
			eo->BoneData[current].EndPoint1Control = NULL;
			eo->BoneData[current].EndPoint2Control = NULL;
		
			eo->BoneData[current].tm    = Inverse(ntm);
			eo->BoneData[current].CrossSectionList.ZeroCount();


			Point3 l1(0.0f,0.0f,0.0f),l2(0.0f,0.0f,0.0f);
		

//object is bone use its first child as the axis
			eo->BoneData[current].flags = 0;
			if (os.obj->ClassID() == bid) 
				{
				l1.x = 0.0f;
				l1.y = 0.0f;
				l1.z = 0.0f;
				l2.x = 0.0f;
				l2.y = 0.0f;
				l2.z = 0.0f;
//get child node
				INode* parent = nodeTab[i]->GetParentNode();
//				ntm = t.Node->GetObjectTM(eo->RefFrame);	
				ntm = parent->GetObjectTM(eo->RefFrame);	
				eo->BoneData[current].tm    = Inverse(ntm);

//				if (nodeTab[i]->NumberOfChildren() > 0)
				if (1)
					{
//					INode *child = nodeTab[i]->GetChildNode(j);
					Matrix3 ChildTM = nodeTab[i]->GetObjectTM(eo->RefFrame);

					_tcscpy(title,nodeTab[i]->GetName());


					l2 = l2 * ChildTM;
					l2 = l2 * Inverse(ntm); 
					Point3 Vec = (l2-l1);
					l1 += Vec * 0.1f;
					l2 -= Vec * 0.1f;
					}
				else 
					{
					l2.x = 0.0f;
					l2.y = 0.0f;
					l2.z = 50.0f;
					}
				float el1 = 9999999999999.0f,el2 = 999999999.0f;
//				float d = Length(l2-l1);
//				if (d < 0.1f) d = 10.f;
//				el1 = d * PERCENT_LENGTH;

				int objects = mcList.Count();
				Point3 ll1 = l1 * Inverse(eo->BoneData[current].tm);
				Point3 ll2 = l2 * Inverse(eo->BoneData[current].tm);

				for (int nc = 0; nc < nodes.Count(); nc++)
					{
					ObjectState base_os = nodes[nc]->EvalWorldState(eo->ip->GetTime());
					
					eo->BuildEnvelopes(nodes[nc], base_os.obj, ll1, ll2, el1, el2);
					}



				float e_inner, e_outer;
				e_inner = el1*INNER_MULT ;
				e_outer = e_inner *OUTER_MULT;
				eo->AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el2*INNER_MULT ;
				e_outer = e_inner *OUTER_MULT;
				eo->AddCrossSection(current, 1.0f, e_inner,e_outer);
				eo->BoneData[current].flags = BONE_BONE_FLAG;

				}
//object is bone use its first child as the axis
			else if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
				{
//build distance based on spline
				eo->BoneData[current].flags = eo->BoneData[current].flags|BONE_SPLINE_FLAG;
				ShapeObject *pathOb = NULL;
				ObjectState os = nodeTab[i]->EvalWorldState(eo->RefFrame);


				BezierShape bShape;
				ShapeObject *shape = (ShapeObject *)os.obj;
				if(shape->CanMakeBezier())
//watje 9-7-99  195862 
					shape->MakeBezier(eo->RefFrame, bShape);
//					shape->MakeBezier(eo->ip->GetTime(), bShape);
				else {
					PolyShape pShape;
//watje 9-7-99  195862 
					shape->MakePolyShape(eo->RefFrame, pShape);
//					shape->MakePolyShape(eo->ip->GetTime(), pShape);
					bShape = pShape;	// UGH -- Convert it from a PolyShape -- not good!
					}

				pathOb = (ShapeObject*)os.obj;

				if (bShape.splines[0]->Closed() )
					eo->BoneData[current].flags = eo->BoneData[current].flags|BONE_SPLINECLOSED_FLAG;

//watje 9-7-99  195862 
				l1 = pathOb->InterpCurve3D(eo->RefFrame, 0, 0.0f, SPLINE_INTERP_SIMPLE);			
//				l1 = pathOb->InterpCurve3D(0, 0, 0.0f, SPLINE_INTERP_SIMPLE);			
//watje 9-7-99  195862 
				l2 = pathOb->InterpCurve3D(eo->RefFrame, 0, 1.0f, SPLINE_INTERP_SIMPLE);			
//				l2 = pathOb->InterpCurve3D(0, 0, 1.0f, SPLINE_INTERP_SIMPLE);			

				float el1 = 0.0f,el2 = 0.0f;
				float s1 = bShape.splines[0]-> SplineLength();
				if (s1< 0.1f) s1 = 10.f;
				if (bShape.splines[0]->Closed() )
					el1 = s1 * PERCENT_LENGTH_CLOSED;
				else el1 = s1 * PERCENT_LENGTH;

				float e_inner, e_outer;
				e_inner = el1;
				e_outer = e_inner *OUTER_MULT;
				eo->AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el1;
				e_outer = e_inner *OUTER_MULT;
				eo->AddCrossSection(current, 1.0f, e_inner,e_outer);

//copy initial reference spline into our spline
//				SplineShape *shape = (SplineShape *)os.obj;

				eo->BoneData[current].referenceSpline = *bShape.splines[0];

				}

			else 
				{
				float el1 = 99999999.0f,el2 = 99999999.0f;
				Matrix3 tempTM;
				tempTM = Inverse(eo->BoneData[current].tm)*Inverse(ourTM);
				eo->BuildMajorAxis(nodeTab[i],l1,l2,el1,&tempTM); 

				el1 = el1 * 0.5f;
				el2= el1;
//				float d = Length(l2-l1);
//				if (d < 0.1f) d = 10.f;
//				el1 = d * PERCENT_LENGTH;

//				eo->ip->GetModContexts(mcList,nodes);
				int objects = mcList.Count();
				Point3 ll1 = l1 * Inverse(eo->BoneData[current].tm);
				Point3 ll2 = l2 * Inverse(eo->BoneData[current].tm);

//				for (int nc = 0; nc < nodes.Count(); nc++)
//					{
//					ObjectState base_os = nodes[nc]->EvalWorldState(eo->ip->GetTime());
					
//					eo->BuildEnvelopes(nodes[nc], base_os.obj, ll1, ll2, el1, el2);
//					}


				float e_inner, e_outer;
				e_inner = el1;
				e_outer = e_inner *OUTER_MULT*0.75f;
				eo->AddCrossSection(current, 0.0f, e_inner,e_outer);
				e_inner = el2;
				e_outer = e_inner *OUTER_MULT*0.75f;
				eo->AddCrossSection(current, 1.0f, e_inner,e_outer);

				}

			l1 = l1;// * Inverse(eo->BoneData[current].tm);
			l2 = l2;// * Inverse(eo->BoneData[current].tm);
			eo->BoneData[current].flags = eo->BoneData[current].flags|BONE_ABSOLUTE_FLAG;
			eo->BoneData[current].FalloffType = 0;

			eo->BoneData[current].BoneRefID = BoneRefID;
			eo->BoneData[current].RefEndPt1ID = End1RefID;
			eo->BoneData[current].RefEndPt2ID = End2RefID;

			eo->BoneData[current].end1Selected = FALSE;
			eo->BoneData[current].end2Selected = FALSE;


			if (os.obj->ClassID() == bid) 
				{
//get child node
				INode* parent = nodeTab[i]->GetParentNode();
				eo->ReplaceReference(BoneRefID,parent,FALSE);
				eo->BoneData[current].name = title;
				}
			else eo->ReplaceReference(BoneRefID,nodeTab[i],FALSE);


			eo->MakeRefByID(FOREVER,End1RefID,NewDefaultPoint3Controller());
			eo->MakeRefByID(FOREVER,End2RefID,NewDefaultPoint3Controller());
			eo->BoneData[current].EndPoint1Control->SetValue(0,&l1,TRUE,CTRL_ABSOLUTE);
			eo->BoneData[current].EndPoint2Control->SetValue(0,&l2,TRUE,CTRL_ABSOLUTE);


			int rsel = eo->ConvertSelectedBoneToListID(current);

			SendMessage(GetDlgItem(eo->hParam,IDC_LIST1),
				LB_INSERTSTRING,(WPARAM) rsel,(LPARAM)(TCHAR*)title);


			nodes.DisposeTemporary();
			}
		

		eo->ModeBoneIndex = current;
		eo->ModeBoneEndPoint = -1;
		eo->ModeBoneEnvelopeIndex = -1;
		eo->ModeBoneEnvelopeSubType = -1;
		SendMessage(GetDlgItem(eo->hParam,IDC_LIST1),
				LB_SETCURSEL ,current,0);

		if (eo->BoneData[eo->ModeBoneIndex].flags & BONE_LOCK_FLAG)
			eo->pblock_param->SetValue(PB_LOCK_BONE,0,1);
		else
			eo->pblock_param->SetValue(PB_LOCK_BONE,0,0);

		if (eo->BoneData[eo->ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
			eo->pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
		else
			eo->pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);
//watje 9-7-99  198721 
		eo->Reevaluate(TRUE);
		}
	*/
	}
	
if ( (eo->BoneData.Count() >0) && (eo->ip && eo->ip->GetSubObjectLevel() == 1) )
	{
	eo->EnableButtons();
	}

if (eo->BoneData[eo->ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
	{
	eo->iAbsolute->SetCheck(FALSE);

//	eo->pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,1);
	}
else
	{
	eo->iAbsolute->SetCheck(TRUE);
//	eo->pblock_param->SetValue(PB_ABSOLUTE_INFLUENCE,0,0);
	}


if (eo->BoneData[eo->ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
	{
//	eo->pblock_param->SetValue(PB_DRAW_BONE_ENVELOPE,0,1);
	eo->iEnvelope->SetCheck(TRUE);

	}
else
	{
//	eo->pblock_param->SetValue(PB_DRAW_BONE_ENVELOPE,0,0);
	eo->iEnvelope->SetCheck(FALSE);
	}

if (eo->BoneData[eo->ModeBoneIndex].FalloffType == BONE_FALLOFF_X_FLAG)
	eo->iFalloff->SetCurFlyOff(0,FALSE);
else if (eo->BoneData[eo->ModeBoneIndex].FalloffType == BONE_FALLOFF_SINE_FLAG)
	eo->iFalloff->SetCurFlyOff(1,FALSE);
else if (eo->BoneData[eo->ModeBoneIndex].FalloffType == BONE_FALLOFF_X3_FLAG)
	eo->iFalloff->SetCurFlyOff(3,FALSE);
else if (eo->BoneData[eo->ModeBoneIndex].FalloffType == BONE_FALLOFF_3X_FLAG)
	eo->iFalloff->SetCurFlyOff(2,FALSE);

eo->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
//eo->CurrentCachePiece = -1;
//watje 9-7-99  198721 
CacheModEnumProc lmdproc(eo);
eo->EnumModContexts(&lmdproc);

eo->cacheValid = FALSE;
}


int DumpHitDialog::filter(INode *node)

{

	TCHAR name1[200];
	_tcscpy(name1,node->GetName());

	node->BeginDependencyTest();
	eo->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
		{		
		return FALSE;
		} 
	else if (node->GetTMController()->ClassID() == IKCHAINCONTROL_CLASS_ID)
		{
		return FALSE;
		}
	else 
		{

		ObjectState os = node->GetObjectRef()->Eval(0);

		Class_ID bid(BONE_CLASS_ID,0);
		for (int i = 0;i < eo->BoneData.Count(); i++)
			{
			
			if (eo->BoneData[i].Node) 
				{
				ObjectState bos = eo->BoneData[i].Node->EvalWorldState(0);

				if ( (node == eo->BoneData[i].Node) &&
					 (os.obj->ClassID() != bid)  )
					return FALSE;

				}



			}


		if (os.obj == NULL)   return FALSE;

		if (os.obj->ClassID() == bid)  
				{

				int found = SendMessage(GetDlgItem(eo->hParam,IDC_LIST1),
							LB_FINDSTRING,(WPARAM) 0,(LPARAM)(TCHAR*)name1);
				if (found != LB_ERR ) return FALSE;

				}

			//}

//check for end nodes

		if (os.obj->ClassID() == bid)  
			{
//get parent if
			INode* parent = node->GetParentNode();
			if (parent->IsRootNode()) return FALSE;
			if (parent == NULL) return FALSE;
			
			ObjectState pos = parent->EvalWorldState(0);
			if (pos.obj->ClassID() != bid)  return FALSE;
		
//			int subcount = node->NumberOfChildren();
//			if (subcount == 0) return FALSE;
			}
		if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
			{
			if ( (os.obj->ClassID()==EDITABLE_SURF_CLASS_ID)
				)
				return FALSE;
//196241 
			ShapeObject *pathOb = (ShapeObject*)os.obj;
			if (pathOb->NumberOfCurves() == 0) return FALSE;
			}	


		}

 	return TRUE;

}


static INT_PTR CALLBACK DeleteDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
			mod = (BonesDefMod*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			for (int i=0; i < mod->BoneData.Count(); i++)
				{
				TCHAR *temp;
				temp = mod->GetBoneName(i);
				if (temp)
					{
					TCHAR title[200];

					_tcscpy(title,temp);

					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_ADDSTRING,0,(LPARAM)(TCHAR*)title);



					}

				}

			break;

			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDOK:
					{
					int listCt = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETCOUNT,0,0);
					int selCt =  SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETSELCOUNT ,0,0);
					int *selList;
					selList = new int[selCt];

					SendMessage(GetDlgItem(hWnd,IDC_LIST1),
						LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);
					mod->removeList.SetCount(selCt);
					for (int i=0; i < selCt; i++)
						{
						mod->removeList[i] = selList[i];
						}

					delete [] selList;

					EndDialog(hWnd,1);
					break;
					}
				case IDCANCEL:
					mod->removeList.ZeroCount();
					EndDialog(hWnd,0);
					break;
				}
			break;

		
		case WM_CLOSE:
			mod->removeList.ZeroCount();
			EndDialog(hWnd, 0);
			break;

	
		default:
			return FALSE;
		}
	return TRUE;
	}

BOOL PickControlNode::Filter(INode *node)
	{
	node->BeginDependencyTest();
	mod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) 
		{		
		return FALSE;
		} 
	else if (node->GetTMController()->ClassID() == IKCHAINCONTROL_CLASS_ID)
		{
		return FALSE;
		}
	else 
		{
		for (int i =0; i < mod->BoneData.Count(); i++)
			{
			if (mod->BoneData[i].Node == node)
				return FALSE;
			}
		ObjectState os = node->EvalWorldState(0);
		if (os.obj->SuperClassID()==SHAPE_CLASS_ID)
			{
			if ((os.obj->ClassID()==EDITABLE_SURF_CLASS_ID))
				return FALSE;
			
//196241 
			ShapeObject *pathOb = (ShapeObject*)os.obj;
			if (pathOb->NumberOfCurves() == 0) return FALSE;
			}
		return TRUE;
		}
	return TRUE;
	}

BOOL PickControlNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}

BOOL PickControlNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) 
		{
		theHold.Begin();

		theHold.Put(new AddBoneRestore(mod));

		BMDModEnumProc lmdproc(mod);
		mod->EnumModContexts(&lmdproc);

		for ( int i = 0; i < lmdproc.bmdList.Count(); i++ ) 
			{
			BoneModData *bmd = (BoneModData*)lmdproc.bmdList[i];
			theHold.Put(new WeightRestore(mod,bmd));
			}

		mod->AddBone(node,TRUE);

		theHold.Accept(GetString(IDS_PW_ADDBONE));

		if (ip)
			{

			if ( (mod->BoneData.Count() >0) && (mod->ip && mod->ip->GetSubObjectLevel() == 1) )
				{
				mod->EnableButtons();
				}

			if ( (mod->ModeBoneIndex >= 0) && (mod->ModeBoneIndex < mod->BoneData.Count()) && (mod->BoneData[mod->ModeBoneIndex].Node!= NULL))
				EnableWindow(GetDlgItem(mod->hParam,IDC_REMOVE),TRUE);


			if (mod->BoneData[mod->ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
				{
				mod->iAbsolute->SetCheck(FALSE);

				}
			else
				{
				mod->iAbsolute->SetCheck(TRUE);
				}


			if (mod->BoneData[mod->ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
				{
				mod->iEnvelope->SetCheck(TRUE);

				}
			else
				{
				mod->iEnvelope->SetCheck(FALSE);
				}

			if (mod->BoneData[mod->ModeBoneIndex].FalloffType == BONE_FALLOFF_X_FLAG)
				mod->iFalloff->SetCurFlyOff(0,FALSE);
			else if (mod->BoneData[mod->ModeBoneIndex].FalloffType == BONE_FALLOFF_SINE_FLAG)
				mod->iFalloff->SetCurFlyOff(1,FALSE);
			else if (mod->BoneData[mod->ModeBoneIndex].FalloffType == BONE_FALLOFF_X3_FLAG)
				mod->iFalloff->SetCurFlyOff(3,FALSE);
			else if (mod->BoneData[mod->ModeBoneIndex].FalloffType == BONE_FALLOFF_3X_FLAG)
				mod->iFalloff->SetCurFlyOff(2,FALSE);

			mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			//eo->CurrentCachePiece = -1;
			//watje 9-7-99  198721 
			CacheModEnumProc lmdproc(mod);
			mod->EnumModContexts(&lmdproc);

			mod->cacheValid = FALSE;
			}



		if (mod->ip)
			mod->ip->RedrawViews(mod->ip->GetTime());
		}
	return FALSE;
	}

void PickControlNode::EnterMode(IObjParam *ip)
	{


	}
HCURSOR PickControlNode::GetDefCursor(IObjParam *ip)
	{
    static HCURSOR hCur = NULL;

    if ( !hCur ) 
		{
        hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDBONECUR)); 
        }
	return hCur;
	}

HCURSOR PickControlNode::GetHitCursor(IObjParam *ip)
	{
    static HCURSOR hCur = NULL;

    if ( !hCur ) 
		{
        hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDBONECUR1)); 
        }
	return hCur;
	}


void PickControlNode::ExitMode(IObjParam *ip)
	{
	mod->inAddBoneMode = FALSE;
	mod->ResetSelection();
	}


