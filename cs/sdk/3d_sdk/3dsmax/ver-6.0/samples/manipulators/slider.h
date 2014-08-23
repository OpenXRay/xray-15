/**********************************************************************
 *<
	FILE: Slider.h

	DESCRIPTION:  2D viewport slider manipulator

	CREATED BY: Scott Morrison

	HISTORY: created 7/12/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Manipulator.h"

#pragma once

#define SliderManip_CLASS_ID Class_ID(0x1fb167d4, 0x63eb5814)
#define PBLOCK_REF_NO	 0
ClassDesc* GetSliderManipDesc();

// Manipulator to specify a solid angle, such as a spotlight falloff angle
class SliderManipulator: public SimpleManipulator 
{
public:
    SliderManipulator();
    SliderManipulator(INode* pNode);

    CreateMouseCallBack* GetCreateMouseCallBack();
    void BeginEditParams( IObjParam  *ip, ULONG flags, Animatable *prev);
    void EndEditParams( IObjParam *ip, ULONG flags, Animatable *next);
    RefTargetHandle Clone(RemapDir& remap = NoRemap());

    TCHAR *GetObjectName() {
        return GetString(IDS_SLIDER_MANIP);
    }
    
    void DeleteThis() {
        delete this;
    }
    Class_ID ClassID() {
        return SliderManip_CLASS_ID;
    }
    int	NumParamBlocks() {
        return 1;
    }
    IParamBlock2* GetParamBlock(int i) {
        return mpPblock;
    } 
    IParamBlock2* GetParamBlockByID(BlockID id) {
        return (mpPblock->ID() == id) ? mpPblock : NULL;
    } 
    
    void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);

    void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);

    virtual void SetFloatValue(float value);
    virtual void SetWidthValue(float width);
    virtual void SetHideValue(BOOL hide);
    virtual void SetPositionValue(float xPos, float yPos);

    void UpdateShapes(TimeValue t, TSTR& toolTip);
    void GenerateShapes(TimeValue t);

    void SetValues(float value, float min, float max, float xPos,
                   float yPos, float width, BOOL hide, TCHAR* pName);

    static const int kSliderOffset;
};

// Parameter block description for slider angle manipulators

// block IDs
enum { kSliderParams };

// Parameter IDs
enum { kSliderValue,
       kSliderMin,
       kSliderMax,
       kSliderXPos,
       kSliderYPos,
       kSliderWidth,
       kSliderHide,
       kSliderName,
};

