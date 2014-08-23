//*****************************************************************************/
// Copyright 2000 Autodesk, Inc.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc. and
// are protected by Federal copyright law. They may not be disclosed
// to third parties or copied or duplicated in any form, in whole or
// in part, without the prior written consent of Autodesk, Inc.
//*****************************************************************************/

#ifndef INCL_MAX_EXPCTL_PSEUDOMAP
#define INCL_MAX_EXPCTL_PSEUDOMAP

static const int   NB_PSEUDO_COLORS            = 100;    // Should be at least >= 50
static const int   NB_MAX_PSEUDO_COLORS        = 240;

/*==============================================================================
 * EXTERNAL DECLARATIONS
 *============================================================================*/

#include <toneOp.h>

#define PSEUDO_EXP_CTL_CLASS_ID   Class_ID(0x575e3dff, 0x60e13d9a)

class ClassDesc2;


/*==============================================================================
 * CLASS PseudoMap
 *============================================================================*/

class PseudoMap : public ToneOperator, public IToneOperatorExtension
{
public:
   enum Reference {
      REF_PARAMS     = 0,
      REF_RENDER_ELEMENT = 1,
      LAST_REF       = REF_RENDER_ELEMENT
   };

   enum ParamBlock {
      PB_MAIN       = 0
   };

   enum ParamID {
      PB_QUANTITY   = 0,
      PB_DISPLAY    = 1,
      PB_MINIMUM    = 2,
      PB_MAXIMUM    = 3,
      PB_SCALE      = 4,
      PB_AUTOMATIC  = 5,
      PB_SCALE_FUNC = 6,
      PB_UNITSYSTEM_USED = 7,
	  PB_ACTIVE		= 8,
	  PB_PROC_BG	= 9
   };

   enum Display {
	   DSP_COLORED = 0, 
	   DSP_GRAY    = 1      
   };

   enum ScaleFunction {
       SCALE_LOGARITHMIC = 0,
       SCALE_LINEAR      = 1
   };

	PseudoMap() : ToneOperator() {}

   static ClassDesc2* GetClassDesc();

   virtual void SetMaximum(
      TimeValue   t,
      float    max
   ) = 0;
   virtual float GetMaximum(
      TimeValue   t,
      Interval&   valid
   ) const = 0;

   virtual void SetMinimum(
      TimeValue   t,
      float min
   ) = 0;
   virtual float GetMinimum(
      TimeValue   t,
      Interval&   valid
   ) const = 0;

   virtual void    SetDisplay( Display dsp ) = 0;
   virtual Display GetDisplay( ) const       = 0;

   virtual void SetAutomatic( bool on ) = 0;
   virtual bool GetAutomatic( ) const   = 0;

   virtual void GetEnergyRangeFromIndex(int colorIndex, float& floor, float& ceiling) = 0;

   virtual void GetPseudoColorMap(ULONG * pPseudoColorMap, int iSizeOf) = 0;


};

#endif
