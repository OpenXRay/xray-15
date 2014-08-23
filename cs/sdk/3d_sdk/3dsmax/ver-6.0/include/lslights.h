//*****************************************************************************/
// Copyright (c) 1997,1998 Discreet Logic, Inc.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Discreet Logic and
// are protected by Federal copyright law. They may not be disclosed
// to third parties or copied or duplicated in any form, in whole or
// in part, without the prior written consent of Discreet Logic.
//*****************************************************************************/

#ifndef INCL_MAX_LIGHTS_LSLIGHTS
#define INCL_MAX_LIGHTS_LSLIGHTS

/*==============================================================================
 * EXTERNAL DECLARATIONS
 *============================================================================*/

#ifndef INCL_STD_MAX
#include <max.h>
#define INCL_STD_MAX
#endif

/*==============================================================================
 * FORWARD DECLARATIONS
 *============================================================================*/

class ToneOperator;
class ShadowType;

#define LIGHTSCAPE_LIGHT_CLASS Class_ID(0x30331006, 0x2454bd3)

#define LS_POINT_LIGHT_ID Class_ID(0x32375fcc, 0xb025cf0)
#define LS_LINEAR_LIGHT_ID Class_ID(0x78207401, 0x357f1d58)
#define LS_AREA_LIGHT_ID Class_ID(0x36507d92, 0x105a1a47)
#define LS_POINT_LIGHT_TARGET_ID Class_ID(0x658d4f97, 0x72cd4259)
#define LS_LINEAR_LIGHT_TARGET_ID Class_ID(0x45076885, 0x40791449)
#define LS_AREA_LIGHT_TARGET_ID Class_ID(0x71794f9d, 0x70ae52f2)

/*==============================================================================
 * CLASS LightscapeLight
 *============================================================================*/

// <summary>Common Lightscape light</summary>

// LightscapeLight is the base class for all Lightscape lights.
// LightscapeLight handles the distribution for the light.

class LightscapeLight : public GenLight {

public: 

   /*----- constants -----*/

   /*----- types and enumerations ----*/

   // Types for the lights
   enum LightTypes {
      TYPE_BASE                  = 532,
      TARGET_POINT_TYPE          = TYPE_BASE + 0,
      POINT_TYPE                 = TYPE_BASE + 1,
      TARGET_LINEAR_TYPE         = TYPE_BASE + 2,
      LINEAR_TYPE                = TYPE_BASE + 3,
      TARGET_AREA_TYPE           = TYPE_BASE + 4,
      AREA_TYPE                  = TYPE_BASE + 5,
      LAST_TYPE                  = AREA_TYPE
   };

   // Types for the distributions
   enum DistTypes {
      ISOTROPIC_DIST             = 0,
      SPOTLIGHT_DIST             = 1,
      DIFFUSE_DIST               = 2,
      WEB_DIST                   = 3,
      LAST_DIST                  = WEB_DIST
   };

   // IDs for the references kept by this object
   enum ReferenceIDs {
      // Reference to the common parameters
      REF_PARAMS                 = 0,
      // Reference to the shadow parameters
      REF_SHADOW_PARAMS          = 1,
      // Reference to the distribution parameters
      REF_SPOT_PARAMS            = 2,
      // Reference to the distribution parameters
      REF_WEB_PARAMS             = 3,
      // Reference to the shadow generator
      REF_SHADOWGEN              = 4,
      // Reference to the extended parameters
      REF_EXT_PARAMS             = 5,
      LAST_REF                   = REF_EXT_PARAMS
   };

   // IDs for the parameters blocks for Lightscape lights
   enum ParameterBlockID {
      PB_GENERAL                 = 0,
      PB_SHADOW                  = 1,
      PB_SPOT                    = 2,
      PB_WEB                     = 3,
      PB_EXT                     = 4,
      LAST_PB                    = PB_EXT
   };

   // This enum is used to hold the General parameter IDs
   enum ParameterIDs {
      // The light distribution
      PB_DIST_TYPE               = 1,
      // Use the light
      PB_USE_LIGHT               = 2,
      // Cast shadows
      PB_CAST_SHADOWS            = 3,
      // Light Color
      PB_LIGHT_COLOR             = 4,
      // Filter Color
      PB_FILTER_COLOR            = 5,
      // Intensity
      PB_INTENSITY               = 6,
      // Light color in degrees kelvin
      PB_KELVIN                  = 8,
      // Flag to use Kelvin or RGB
      PB_USE_KELVIN              = 10,
      // Intensity At
      PB_INTENSITY_AT            = 13,
      // Intensity Type
      PB_INTENSITY_TYPE          = 14,
      // Flux
      PB_FLUX                    = 15,
      // Original Intensity
      PB_ORIGINAL_INTENSITY      = 16,
      // Original Flux
      PB_ORIGINAL_FLUX           = 17,
		// use the dimmer slider/spinner
		PB_USE_DIMMER					= 18,
		// Dimmer/Multiplier
		PB_DIMMER						= 19,
      // Last id in list
      LAST_GEN_PB                = PB_DIMMER
   };

   enum ExtParamIDs {
      // Contrast between diffuse and ambient
      PB_CONTRAST                = 0,
      // Softening between diffuse and ambient
      PB_DIFFSOFT                = 1,
      // Use projector map
      PB_PROJECTOR               = 2,
      // The projector map
      PB_PROJECTOR_MAP           = 3,
      // Affect diffuse channel
      PB_AFFECT_DIFFUSE          = 4,
      // Affect specular channel
      PB_AFFECT_SPECULAR         = 5,
      // Ambient only
      PB_AMBIENT_ONLY            = 6,
      // Target distance
      PB_TDIST                   = 7,
//		PB_LENGTH						= 8,
//		PB_WIDTH							= 9,  
//		PB_GARBAGE						= 10,
//		WARNING:  DO NOT REDEFINE ABOVE IDs.  THIS MAY CAUSE A CRASH WHEN LOADING OLD
//					 FILES
//    Change made August 27, 2001 David Cunningham
//    for B24 of VIZ R4
		// Light length for both linear and area lights
		PB_LENGTH						= 11,
		// Area light height
		PB_WIDTH							= 12, 
      // Last id in list
      LAST_EXT_PB                = PB_WIDTH
   };

   // This enum is used to hold the Shadow parameter IDs
   enum ShadowIDs {
      // Atmospheric shadows
      PB_ATMOS_SHAD              = 0,
      // Atmosphere opacity
      PB_ATMOS_OPACITY           = 1,
      // Atmosphere color influence
      PB_ATMOS_COLAMT            = 2,
      // Shadow Density
      PB_SHADMULT                = 3,
      // Shadow Color Map Enabled
      PB_SHAD_COLMAP             = 4,
      // Shadow color
      PB_SHADCOLOR               = 5,
      // Light affects shadow color
      PB_LIGHT_AFFECTS_SHADOW    = 6,
      // Use global shadow generator
      PB_USE_GLOBAL_PARAMS       = 7,
      // Shadow Projection map
      PB_SHAD_PROJ_MAP           = 8,
      // Last shadow id
      LAST_SHAD_PB               = PB_SHAD_PROJ_MAP
   };

   // This enum is used to hold the Spotlight parameter IDs
   enum SpotlightIDs {
      // Beam angle
      PB_BEAM_ANGLE              = 0,
      // Field angle
      PB_FIELD_ANGLE             = 1,
      // Display the spot light cone
      PB_CONE_DISPLAY            = 2,
      // Last distribution id
      LAST_SPOT_PB               = PB_CONE_DISPLAY
   };

   // This enum is used to hold the Photometric Web parameter IDs
   enum DistributionIDs {
      // The name of the webfile
      PB_WEB_FILE_NAME           = 0,
      // Rotate X
      PB_WEB_ROTATE_X            = 1,
      // Rotate Y
      PB_WEB_ROTATE_Y            = 2,
      // Rotate Z
      PB_WEB_ROTATE_Z            = 3,
      // Last distribution id
      LAST_WEB_PB               = PB_WEB_ROTATE_Z
   };

   // This enum is used to hold the intensity types
   enum IntensityType {
      // Intensity is in lumens
      LUMENS                     = 0,
      // Intensity is in candelas
      CANDELAS                   = 1,
      // Intensity is in Lux at distance
      LUX_AT                     =2,
   };

   // Shadow types for MAX
   enum MaxShadowType {
      // No Shadow Generator
      NO_SHADOW_GENERATOR        = -1,
      // Bitmap Shadows
      BITMAP_SHADOWS             = 0,
      // RayTraceShadows
      RAYTRACE_SHADOWS           = 1,
      // Other shadow generator
      OTHER_SHADOWS              = 0xffff
   };

   /*----- classes -----*/

   /*----- static member functions -----*/

   /*----- member functions -----*/

   virtual void SetType( int type ) = 0;
   virtual void SetType( const TCHAR* name ) = 0;
   virtual int Type( ) = 0;
   virtual const TCHAR* TypeName( ) = 0;

   virtual void SetDistribution( DistTypes dist ) = 0;
   virtual DistTypes GetDistribution( ) const = 0;

   virtual void SetIntensityAt( float f ) = 0;
   virtual float GetIntensityAt( ) = 0;
   virtual void SetIntensityType( IntensityType t ) = 0;
   virtual IntensityType GetIntensityType( ) = 0;
   virtual void SetFlux(
      TimeValue t,
      float     flux
   ) = 0;
   virtual float GetFlux(
      TimeValue t,
      Interval &valid = Interval( 0,0 )
   ) const = 0;

   virtual void SetRGBFilter(
      TimeValue t,
      Point3& rgb
   ) = 0;
   virtual Point3 GetRGBFilter(
      TimeValue t,
      Interval &valid = Interval( 0,0 )
   ) = 0;
   virtual void SetHSVFilter(
      TimeValue t,
      Point3& hsv
   ) = 0;
   virtual Point3 GetHSVFilter(
      TimeValue t,
      Interval &valid = Interval( 0,0 )
   ) = 0;


   // Plug-in shadow generator
   virtual ShadowType* ActiveShadowType( ) = 0;
   virtual ShadowType* GetShadowGenerator( ) = 0;
   virtual const TCHAR* GetShadowGeneratorName( ) = 0;
   virtual void SetShadowGenerator( ShadowType* s ) = 0;
   virtual void SetShadowGenerator( const TCHAR* name ) = 0;

   virtual void SetUseShadowColorMap(
      TimeValue t,
      int onOff
   ) = 0;
   virtual int GetUseShadowColorMap( TimeValue t ) = 0;

   virtual void SetInclude( BOOL onOff ) = 0;

   virtual void UpdateTargDistance(
      TimeValue t,
      INode* inode
   ) = 0;

   virtual BOOL SetKelvinControl( Control *c ) = 0;
   virtual BOOL SetFilterControl( Control *c ) = 0;
   virtual Control* GetKelvinControl( ) = 0;
   virtual Control* GetFilterControl( ) = 0;

   virtual float GetKelvin(
      TimeValue t,
      Interval& v = Interval(0,0)
   ) = 0;
   virtual void SetKelvin(
      TimeValue t,
      float value
   ) = 0;
   virtual BOOL GetUseKelvin( ) = 0;
   virtual void SetUseKelvin( BOOL useKelvin ) = 0;

   virtual const TCHAR* GetWebFileName( ) const = 0;
   virtual const TCHAR* GetFullWebFileName( ) const = 0;
   virtual void SetWebFileName( const TCHAR* name ) = 0;

   virtual float GetWebRotateX( ) const = 0;
   virtual void SetWebRotateX( float degrees ) = 0;
   virtual float GetWebRotateY( ) const = 0;
   virtual void SetWebRotateY( float degrees ) = 0;
   virtual float GetWebRotateZ( ) const = 0;
   virtual void SetWebRotateZ( float degrees ) = 0;

   virtual float GetDimmerValue(TimeValue t, Interval &valid = Interval( 0,0 )) const = 0;
   virtual void SetDimmerValue(TimeValue t, float) = 0;
   virtual BOOL GetUseMultiplier() const = 0;
   virtual void SetUseMultiplier(BOOL) = 0;
   virtual float GetResultingIntensity(TimeValue t, Interval &valid = Interval( 0,0 )) const = 0;
   virtual float GetResultingFlux(TimeValue t, Interval &valid = Interval( 0,0 )) const = 0;

   virtual Point3 GetCenter( ) const = 0;
   virtual void SetShape(
      int            count,
      const Point3*  points
   ) = 0;
   virtual int GetShape(
      Point3*  points,
      int      bufSize
   ) const = 0;

   virtual float GetOriginalFlux( ) const = 0;
   virtual void SetOriginalFlux( float flux ) = 0;
   virtual float GetOriginalIntensity( ) const = 0;
   virtual void SetOriginalIntensity( float candelas ) = 0;
};

#endif
