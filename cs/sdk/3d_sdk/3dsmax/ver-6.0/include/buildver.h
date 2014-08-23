#ifndef _BUILD_VER_

#define _BUILD_VER_

// Don't use! Edu version of MAX now keyed off serial number
// Define EDU_VERSION to build the educational version of MAX
//#define EDU_VERSION

// Define BETA_VERSION to use Beta lock
//#define BETA_VERSION

// Define STUDENT_VER to build the student version of MAX
// #define STUDENT_VER
#ifdef STUDENT_VER
#define WIN95_ONLY
#endif

//TURN ON SNAPPING FOR INTEGRATION TO ATHENA
#define _OSNAP TRUE

//TURN ON PRIMITIVE CREATION WITH 3D SNAPPING
#define _3D_CREATE

// Turn on sub material assignment : 1/19/98 - CCJ
#define _SUBMTLASSIGNMENT

//////////////////////////////
// Defines to build Viz / Kahn
//
//#define DESIGN_VER
//#define RENDER_VER
//#define RENDER_DEV_VER

#ifdef DESIGN_VER
// Enable bitmap printing in VIZ
#define _ENABLE_BITMAP_PRINTING_

// Define to disable mouse capture
//#define BARCELONA_MOUSE_CAPTURE
//#define COMMANDMODE_STATE_SAVING

#endif // DESIGN_VER

// Define turn on layers
#define _LAYERS_ //enables layers at the file compatibility level
#define _LAYERS_LEVEL2_ //make layer system operational at runtime

// Define to Enable Status Bar Function Publishing : 11/14/01
#define USE_STATUSPANEL_FP

// Define to Enable Create Panel Function Publishing : 02/21/03
#define USE_CREATE_PANEL_FP

//turn on features for Lightscape radiosity into MAX
#define NOVA_VER

// Define to build a version with no NURBS
//#define NO_NURBS

// Turned on for R6 VC7 - LAM - 2/25/03
// Turn this on to use the new version of the ObjectDBX SDK. The libraries and
// dlls for the old version are ac*15; for the new version, ac*16.
// This macro is primarily used in ObjectDBX_libraries.h.
#define USE_OBJECTDBX16_SDK

#ifdef RENDER_VER	// put here (at least for now) 030219  --prs.

//remove schematic view from HAL
// 7/30/01 - CA: NO_SCHEMATICVIEW breaks compatibility between MAX and VIZ
// so I removed the define. The problem is that we don't round trip
// the schematic view data. So I am turning schematic view back on
// until we can figure out how to deal with this.
// aszabo|Sep.08.01 - Roundtriping all max data is not a requirement anymore
// so I'm turning off SchematicView.
#define NO_SCHEMATICVIEW

// Define APL_DBCS for double-byte character set versions (i.e. Japanese, Chinese)
//#define APL_DBCS

// Define to build a version with EDP (Event-Driven Particles)
//#define EDP_VER

#define VIZR_RESOURCE_OVERRIDE // Turn this off if you want the original VIZ resource files


/********************************************************************
***                   FUNCTIONALITY REMOVED                       ***
********************************************************************/

//////////////////////
// Lights removal
//
#define NO_LIGHT_AFFECT_SURFACE     

    /////////////////////////////////////////////// 
    // The shadow generator wanted has to be set
    // in SHADOW_GENERATOR_STR
    //
	#define USING_SINGLE_SHADOW_GENERATOR   
	#define SHADOW_GENERATOR_STR   _T("Flow Ray Traced")//"Flow Ray Traced" "Adv. Ray Traced" "Ray Traced Shadow" "Shadow Map"
    /////////////////////////////////////////////// 

#define NO_SHADOW_GENERATOR_SPECIFIC_PARAM
#define NO_SHADOW_PARAMETERS_ROLLOUT

///////////////////////////
// Objects removal
//
#define NO_OBJECT_BOOL
#define NO_OBJECT_BONE
#define NO_OBJECT_CONNECT
#define NO_OBJECT_CAPSULE
#define NO_OBJECT_HEDRA
#define NO_OBJECT_LOFT
#define NO_OBJECT_MORPH
#define NO_OBJECT_PRISM
#define NO_OBJECT_RINGARRAY
#define NO_OBJECT_SCATTER
#define NO_OBJECT_SHAPE_MERGE
#define NO_OBJECT_SLIDING_DOOR
#define NO_OBJECT_SLIDING_WINDOW
//#define NO_OBJECT_STANDARD_PRIMITIVES // Uncomment to turn on the removal of the 'Standard Primitive'
#define NO_OBJECT_TEAPOT
#define OBJECT_TERRAIN_PRIVATE

// Splines
//#define NO_OBJECT_SHAPES_SPLINES // Uncomment to turn on the removal of the 'Shapes Splines'

// Lights
#define NO_OBJECT_DIRECT_LIGHT
#define NO_OBJECT_SPOT_LIGHT
//#define NO_OBJECT_OMNI_LIGHT
#define PRIVATE_INCOMPATIBLE_OMNI_LIGHT	Class_ID(0x396431cc, 0x62c20b0)

///////////////////////////
// Modifiers removal
//
#define MODIFIER_SKEW_PRIVATE
#define MODIFIER_EXTRUDE_PRIVATE
#define NO_MODIDIFIER_SUBDIVIDE_WSM
#define NO_MODIFIER_MATERIAL
#define NO_MODIFIER_EDIT_NORMAL
#define NO_MODIFIER_DELETE_MESH
#define NO_MODIFIER_BEND
#define NO_MODIFIER_AFFECTREGION
#define NO_MODIFIER_MIRROR 
#define NO_MODIFIER_NOISE 
#define NO_MODIFIER_OPTIMIZE 
#define NO_MODIFIER_RIPPLE
#define NO_MODIFIER_TAPER 
#define NO_MODIFIER_TESSELATE 
#define NO_MODIFIER_TWIST
#define NO_MODIFIER_XFORM  //also called Cluster modifier
#define NO_MODIFIER_DELETE_SPLINE
#define NO_MODIFIER_DISP_APPROX
#define NO_MODIFIER_DISPLACEMESH
#define NO_MODIFIER_NORMALIZE_SPLINE 
#define NO_MODIFIER_FACE_EXTRUDE
#define NO_MODIFIER_DISPLACE
#define NO_MODIFIER_LATHE
#define NO_MODIFIER_POLY_SELECT
#define NO_MODIFIER_SPLINESELECT
#define NO_MODIFIER_SLICE
#define NO_MODIFIER_CAMERA_MAP
#define NO_MODIFIER_STRETCH
#define NO_MODIFIER_PATH_DEFORM
#define NO_MODIFIER_FFD
#define NO_MODIFIER_CAPHOLES
#define NO_MODIFIER_PRESERVE
#define NO_MODIFIER_MATERIALBYELEMENT
#define NO_MODIFIER_LATTICE
#define NO_MODIFIER_SQUEEZE
#define NO_MODIFIER_VOLUME_SELECT
#define NO_PATCHES

///////////////////////////
// Modifiers simplification
//
#define USE_SIMPLIFIED_SMOOTH_MODIFIER_UI
#define USE_SIMPLIFIED_UVWMAP_UI
#define USE_EMESH_SIMPLE_UI
#define USE_EMESH_SIMPLE_UI

///////////////////////////
// Modifiers list
//
#define MODIFIER_LIST_AUTO_HEIGHT

///////////////////////////
// Utility removal
//
#define NO_UTILITY_LINKINFO
#define NO_UTILITY_RESCALE
#define NO_UTILITY_COLLAPSE
#define NO_UTILITY_FOLLOWBANK
#define NO_UTILITY_POLYGONCOUNTER
#define NO_UTILITY_RESETXFORM
#define NO_UTILITY_SHAPECHECK
#define NO_UTILITY_MEASURE
#define NO_UTILITY_IFL_MANAGER
#define NO_UTILITY_MAXSCRIPT

///////////////////////////
// Controllers removal
//
#define NO_CONTROLLER_AUDIO_POSITION
#define NO_CONTROLLER_AUDIO_ROTATION
#define NO_CONTROLLER_AUDIO_SCALE
#define NO_CONTROLLER_SLAVE_POSITION
#define NO_CONTROLLER_SLAVE_ROTATION
#define NO_CONTROLLER_SLAVE_SCALE
#define NO_CONTROLLER_SURFACE

/////////////////////////////
// Rendering / Environment
//
#define NO_RENDERING_LINEAR_EXPOSURE_CONTROL
#define NO_ATMOSPHERICS
#define NO_REMOVE_EXPOSURE_CONTROL

//////////////////////////////
// Preferences
//
#define NO_PREF_GAMMA
#define NO_PREF_MAXSCRIPT
#define NO_PREF_ANIMATION
#define NO_PREF_INVERSE_KINEMATICS
#define NO_PREF_TMGIZMO
#define NO_PREF_VIZ
#define NO_STROKES

/////////////////////////////
// Configure Path
//
// #define NO_CFGPATH_GENERAL // xavier robitaille | 03.01.24 | revert to configurable paths
#define NO_CFGPATH_PLUGINS
#define NO_CFGPATH_XREF
#define RENDER_VER_CONFIG_PATHS  // xavier robitaille | 03.01.24 | modify kahn's config. paths
// xavier robitaille | 03.02.05 | add textures dir. to bitmap paths
#define TEXTURES_DIR_BMP_SEARCH_PATH  
//#define NO_FILELINK_PLUGINS_PATH //SS 6/16/2003: enable this to prevent adding a third plugins path

/////////////////////////////
//  Command panel removal
//
#define NO_CREATE_TASK     // Must define NO_OBJECT_DEFAULT_MATERIALS because the code for that feature is not ready for the create panel removal!
#define NO_HIERARCHY_TASK  
#define NO_MOTION_TASK     
#define NO_DISPLAY_TASK    
//#define NO_UTILITY_TASK  // HAVE TO REMOVE \UI\Macro_Archive.mcr and \UI\Macro_Panoramic.mcr
#define USE_TEXT_TAB_LABELS // Use text labels in the command panel instead of icons

//#define NO_COMMAND_PANEL_TAB_DISPLAYED
#define TRAJECTORY_EDIT_DIALOG

/////////////////////////////
// Controllers removal
//
#define NO_CONTROLLER_MORPH
#define NO_CONTROLLER_SCRIPTED

/////////////////////////////
//  Helpers
//
#define NO_ATMOSGIZMOS
#define NO_HELPER_FOG
#define NO_HELPER_PROTRACTOR
#define NO_HELPER_TAPE
#define PRIVATE_HELPER_COMPASS


//////////////////////
// Material Editor Simplification
//
#define NO_MTLEDITOR_OPTIONS
#define NO_MTLEDITOR_PUTTOLIB
//#define NO_MTLEDITOR_EFFECTSCHANNELS
#define NO_MTLEDITOR_MAKEPREVIEW
#define NO_MTLEDITOR_VIDCOLORCHECK
#define NO_MTLEDITOR_SAMPLEUVTILING

        //pfbreton 23 sept 2002: added new #defines to simplify even more the Meditor
#define NO_MTLEDITOR_SELECTBYMATERIAL
//#define NO_MTLEDITOR_PICKMATERIAL		Turn off temporarily for testing
//#define NO_MTLEDITOR_MATERIALTYPE		// Need the type button
#define NO_MTLEDITOR_GETMATERIAL
#define NO_MTLEDITOR_PUTTOSCENE
#define NO_MTLEDITOR_ASSIGNMTLTOSELECTION
#define NO_MTLEDITOR_CLEARMATERIAL
#define NO_MTLEDITOR_CLONE
#define NO_MTLEDITOR_SHOWMAPINVPORT
#define NO_MTLEDITOR_HSCROLLVSCROLL
#define NO_MTLEDITOR_SAMPLETYPE
#define NO_MTLEDITOR_SIBLING
#define NO_MTLEDITOR_SHOWENDRESULT
#define MTLEDITOR_ONE_MTL_ONE_MAP_PREVIEW	// Use a two pane preview, one for materials and one for maps
#define NO_MTLEDITOR_NOISE_ROLLOUT
#define NO_MTLEDITOR_XYZ_ROLLOUT
#define MTLEDITOR_SHARE_UVGEN_AND_TEXOUT_PANELS
#define NO_MTLEDITOR_BITMAP_CROPPING
#define NO_MTLEDITOR_BITMAP_TIME_ROLLOUT
#define MTLEDITOR_EDIT_PUBLIC_ONLY
#define MTLEDITOR_CONVERT_TO_ARCHMAT
#define NO_MTLEDIT_KEEP_OLD
#define NO_MTLBROWSER_BROWSE_LIBRARIES

//#define NO_MTLEDIT_EDIT_MTL_TEX_NAMES

/////////////////////////////
//  Camera
//
#define NO_CAMERA_MULTIPASS
#define NO_CAMERA_ENVRANGE
#define USE_VIZR3_CAMERA_MESH

/////////////////////////////
//  Skylight
//
#define SKYLIGHT_COVERAGE_RADIO_BUTTON
#define SKYLIGHT_CAN_RENDER_WITHOUT_RADIOSITY
#define NO_SKYLIGHT_AFFECT_DIFFUSE_UI
#define NO_SKYLIGHT_AFFECT_SPECULAR_UI
#define NO_SKYLIGHT_EXPONENT_UI
#define NO_SKYLIGHT_RAY_BIAS_UI
#define NO_TEXTURE_SKYLIGHT
//#define TEXTURE_SKYLIGHT_IS_PRIVATE
//#define NO_IES_SKYLIGHT
//#define IES_SKYLIGHT_IS_PRIVATE

/////////////////////////////
//  Sunlight
//
#define NO_SUNLIGHT_TARGETED_UI

/////////////////////////////
//  Daylight system
//
#define NO_DAYLIGHT_MOTION_PANEL    // If No_MOTION_PANEL is defined, this is forced to be defined
#define NO_DAYLIGHT_SELECTOR
#define NO_DAYLIGHT_SKY_COVERAGE_SLIDER
#define DAYLIGHT_DEFAULT_TIME_SUMMER_SOLSTICE
#define DAYLIGHT_CITY_PICKER_BIG_CITY_DEFAULT

/////////////////////////////
//  Snapping
//
#define CAN_HAVE_MULTIPLE_SNAP_TOGGLE_BUTTON

/////////////////////////////
//  Bitmaps
//
#define NO_OUTPUT_GAMMA
#define NO_OUTPUT_DEVICES

/////////////////////////////
//  Render
//
#define NO_RENDER_ELEMENTS_UI
#define NO_EMAIL_NOTIFICATION
#define NO_DRAFT_RENDERER
#define ONLY_BITMAPS_FOR_ENVIRONMENT_MAP

/////////////////////////////
//  Layer properties simplification
//
//  The layer properties simplifications are intended to be used in conjuction
//  with the object properties simplification. Using one without the other will
//  create confusing situation for a user. For example, if the layer properties
//  simplification is turn on but not the object properties simplification, then
//  a user will be able to set the properties of an object to 'By Layer' but
//  will not be able to control the properties of the layer.
#define LAYER_PROP_KAHN_DEFAULTS
#define NO_LAYER_DLG_RADIOSITY_RENDERING_COLUMNS
#define NO_LAYER_DLG_OBJECT_IN_LIST
#define NO_LAYER_DLG_DETAILS_TAB
#define NO_LAYER_DLG_DETAILS_NAME
#define NO_LAYER_DLG_DETAILS_COLOR_SWATCH
#define LAYER_PROP_GENERAL_DETAILS_WITHOUT_TAB
#define NO_LAYER_DROP_DOWN_RENDERING_COLUMN
#define NO_LAYER_DROP_DOWN_RADIOSITY_COLUMN

/////////////////////////////
// Object properties simplification
//
// See Layer properties simplification.
#define OBJ_PROP_KAHN_DEFAULTS
#define NO_OBJ_PROP_MOTION_BLUR_GROUP
#define NO_OBJ_PROP_DISPLAY_GROUP
#define NO_OBJ_PROP_DIMENSION
#define NO_OBJ_PROP_POLYGON_COUNT
#define NO_OBJ_PROP_PARENT_NAME
#define NO_OBJ_PROP_GROUP_NAME
#define NO_OBJ_PROP_VISIBILITY
#define NO_OBJ_PROP_RCV_SHADOW
#define NO_OBJ_PROP_APPLY_ATMOSPHERICS
#define NO_OBJ_PROP_RENDERABLE
#define OBJ_PROP_MERGED_GEN_RADIOSITY_PAGE
#define NO_OBJ_PROP_RADIOSITY_LIGHT_EXCLUDED
#define NO_OBJ_PROP_RADIOSITY_SPECULAR
#define NO_OBJ_PROP_RADIOSITY_RCV_ILLUM
#define NO_OBJ_PROP_RADIOSITY_RAY_MULT
#define NO_OBJ_PROP_RADIOSITY_GEOM_EXCLUDED
#define NO_OBJ_PROP_BYOBJECT_BYLAYER
// Not yet enabled in debug build. Required for file link testing.
#if defined(NDEBUG) && !defined(RENDER_DEV_VER)
#define NO_OBJ_PROP_USER_PROPERTIES
#endif
#define NO_MENTAL_RAY_PROPERTIES  

/////////////////////////////
// Main toolbar
//
#define USE_SELECTION_FILTERS_BASIC_FILTERS_ONLY
#define USE_RENDER_REGION_SIMPLIFIED

/////////////////////////////
// Trackbar/Trackview
//
#define NO_TRACK_VIEW
#define NO_TRACKBAR_MAXIMIZE
#define NO_TRACKBAR_FILTER_MATERIAL
#define NO_TRACKBAR_CONFIG_SHOW_AUDIO

/////////////////////////////
// Action items
//
#define NO_ACTION_GLOBAL_EXCLUDE

///////////////////////////
//  Others
//
#define NO_EXCLUSION_LIST_DLG
#define NO_MAKE_PREVIEW_USE_DEVICE
#define NO_CUI_TABBED_PANELS
#define NO_NURBS          //SS 6/6/2002: turned off NURBS
#define NO_ACTIVESHADE
// #define NO_MANIPULATORS
#define NO_CUSTOM_ATTRIBUTES
#define NO_TOOL_DISPLAY_FLOATER
#define NO_SPACEWARPS
#define NO_PARTICLES
#define NO_AUTOHEIGHT
#define NO_OBJECT_DEFAULT_MATERIALS
#define NO_SAFE_FRAME_CUSTOMIZATION
#define NO_CROSSHAIR_CURSOR
#define NO_VIEWPORT_BG_DIALOG
#define NO_TIME_CONFIG_VIDEO_OPTIONS
#define NO_ANIMATABLE_PARAMETERS
#define NO_PARAMETER_WIRING_UI
#define NO_LINK_UI
#define NO_UNLINK_UI
// xavier robitaille - 03/01/13
// The main toolbar is not created if it is not defined in the .CUI file.
#define NO_DEFAULT_MAIN_TOOLBAR 

#if defined(NDEBUG) && !defined(RENDER_DEV_VER)
// not enabled yet	#define NO_MAXSCRIPT_UI	// russom - 10/14/02
#endif

/********************************************************************
***                     FUNCTIONALITY ADDED                       ***
********************************************************************/

/////////////////////////////
//  Status panel
//
#define USE_MINSTATPANEL  // reduce status panel
#define NO_MXS_MINILISTENER
#define USE_STATPANEL_VIZ3_SNAP_TOGGLE_UI
#define NO_GRID_POLAR
#define NO_GRID_ORTHO


/////////////////////////////
//  Select By Name Dlg (SBN)
//
// If defined this causes the Display Subtree option of the SBN to be on by default 
#define SBN_SHOWSUBTREE_DEFAULT_ON
// If defined, this causes the groups to appear at the top and xrefs at the bottom 
// of a flat node list in the SBN. Enabling this degrades the performance of the SBN,
// so it's disabled in Kahn. As a result, the ordering of the group and xref nodes
// in Kahn's SBN is not the same as in VIZ and MAX.
//#define SBN_ENABLE_GROUPANDXREF_SORT
// This replaces the Select Dependents check box with the Select Instances checkbox
#define SBN_SELECT_INSTANCES

/////////////////////////////
//  FILE LINK
//
// Adds flag to Preferences dialog
#define FILELINK_DISPLAY_ALL_MSG

/////////////////////////////
//  Others
//
#define USE_TOOLBOX
#define USE_FLOATING_VIEWPORT_NAVIGATION_TOOLS // Need USE_TOOLBOX
#define USE_HARDCODED_SYSTEM_UNIT
#define USE_NEW_CUI_IO_METHODS
#define USE_CUI_FILE_VERSION
// aszabo|Oct.29.02
// Turns on automatic selection of children nodes when the head of an ADT obj 
// hierarchy is selected. Should be off for Kahn. 
//#define SELECT_ADTOBJ_HIERARCHY 
// Turns on automatic selection of children nodes when the head of a ACAD Block 
// hierarchy is selected. Should be off for Kahn
//#define SELECT_BLOCK_HIERARCHY

/////////////////////////////
//  Maxscript encryped files
//
#define USE_ENCRYPTED_MSX_FILES
#if defined(NDEBUG) && !defined(RENDER_DEV_VER)
	#define USE_ENCRYPTED_MSX_FILES_ONLY
#endif

/////////////////////////////
//  Missing Dlls dialog
//
// Enable the "Do not display this message again" check box.
#define USE_MISSING_DLL_DIALOG_OPTION

//////////////////////////////////////////
// Limiting max commands on linked objects
//
// Layer assignments can't be changed on actively (file) linked objects
#define LINKEDOBJ_NO_LAYER_ASSIGNMENT
// Actively (file) linked objects can't be deleted
#define LINKEDOBJ_NO_DELETE
// Actively (file) linked objects can't be cloned (copied, instanced, or referenced)
#define LINKEDOBJ_NO_CLONE
// Components of actively (file) linked objects can't be unlinked from their parent node
#define LINKEDOBJ_NO_UNLINK_NODE
// Actively (file) linked objects cannot be collapsed. A "Bind object" command
// would be more appropriate to implement. 
#define LINKEDOBJ_NO_COLLAPSE
// Inform the the user on the potential problems caused by applying a topology dependent 
// modifier on actively (file) linked objects and allow him to cancel modifier application
#define LINKEDOBJ_OK_TO_APPLY_TOPODEPENDENT_MODIFIER

//////////////////////////////////////////
// Limiting max commands on substituted objects
//
// Substituted objects can't be deleted
#define SUBSTOBJ_NO_DELETE
// Substituted objects can't be cloned (copied, instanced, or referenced)
#define SUBSTOBJ_NO_CLONE
// Nodes cannot be linked to substituted objects
// Substituted objects can't be unlinked from their parent node
#define SUBSTOBJ_NO_LINK_UNLINK_NODE

// Put the render output and options into separate rollouts.
// Define these to a control moved from the common params rollupt
// to the output and options dialog respectively. If this control
// is in the common params rollup, the separate rollup is not
// presented. So, you can have different UIs for video post.
#define RENDER_OUTPUT_IN_SEPARATE_ROLLOUT	IDC_REND_SAVEFILE
#define RENDER_OPTION_IN_SEPARATE_ROLLOUT	IDC_REND_FORCE2SIDED

// Various additions to the render dialog.
#define PRINT_WIZARD_IN_RENDER_DIALOG	// Add Print Size Wizard button to render dialog
#define RAYTRACE_IN_RENDER_DIALOG		// Add Raytrace controls to render dialog
#define ADV_SHADOWS_IN_RENDER_DIALOG	// Add Advanced shadow controls to render dialog

#define SINGLE_SUPERSAMPLE_IN_RENDER	// Use a single super sampler, kept by the renderer
//#define SINGLE_SUPERSAMPLE_CLASS_ID	Class_ID(0x25773211, 0)		//  R25_SAMPLER_CLASS_ID
//#define SINGLE_SUPERSAMPLE_CLASS_ID	Class_ID(0x25773214, 0)		//  HAMMERSLEY_SAMPLER_CLASS_ID
#define SINGLE_SUPERSAMPLE_CLASS_ID	Class_ID(0x25773216, 0)		//  AHALTON_SAMPLER_CLASS_ID
//#define SINGLE_SUPERSAMPLE_CLASS_ID	Class_ID(0x25773217, 0)		//  ACMJ_SAMPLER_CLASS_ID

#define USE_RENDER_PRESETS				// Use the render presets

#define SINGLE_RADIOSITY_PLUGIN			// Only have a single radiosity plugin

// Define the default
#define DEFAULT_RADIOSITY_CLASS_ID		Class_ID(0x795c4168, 0x669a1835)	// Radiosity
//#define DEFAULT_RADIOSITY_CLASS_ID		Class_ID(0x39ff19ec, 0x772d5bff)	// LightTracer

// Define this to be the class id for the default tone operator class.
// If it isn't defined, no tone operator is assigned by default.
#define DEFAULT_TONE_OPERATOR_CLASS_ID Class_ID(0x58b4684b, 0x507f76e9)		// Logarithmic
//#define DEFAULT_TONE_OPERATOR_CLASS_ID Class_ID(0x786c6aaa, 0x449349db)	// Adaptive
//#define DEFAULT_TONE_OPERATOR_CLASS_ID Class_ID(0x55897318, 0x34db21ac)	// Linear
//#define DEFAULT_TONE_OPERATOR_CLASS_ID Class_ID(0x575e3dff, 0x60e13d9a)	// Pseudo color
#endif	// RENDER_VER

// SDK version defines : 02/26/01 - russom
//	Multiple File Support - russom - 12/02/02
// aszabo - may.09.03 - Turned on support for DRF files in R6
// aszabo - july.07.03 - Added explicit support for File Replace
#define MULTI_FILE_SUPPORT
#ifdef MULTI_FILE_SUPPORT

	// File format support tokens
	#define FILE_SUPPORT_NONE		0x000
	#define FILE_SUPPORT_OPEN		0x001
	#define FILE_SUPPORT_SAVE		0x002
	#define FILE_SUPPORT_DEFAULT	0x004
	#define FILE_SUPPORT_EXPORT		0x008
	#define FILE_SUPPORT_IMPORT		0x010
	#define FILE_SUPPORT_MERGE		0x020
	#define FILE_SUPPORT_REPLACE	0x040
	#define FILE_SUPPORT_XREF			0x080

	#define FILE_SUPPORT_NATIVE		(FILE_SUPPORT_SAVE|FILE_SUPPORT_OPEN|FILE_SUPPORT_MERGE|FILE_SUPPORT_REPLACE|FILE_SUPPORT_XREF|FILE_SUPPORT_DEFAULT)

	// File formats
	#ifdef RENDER_VER
		#if defined(NDEBUG) && !defined(RENDER_DEV_VER)
			#define FILE_FORMAT_MAX		FILE_SUPPORT_MERGE
		#else
			#define FILE_FORMAT_MAX		(FILE_SUPPORT_SAVE|FILE_SUPPORT_OPEN|FILE_SUPPORT_MERGE|FILE_SUPPORT_REPLACE|FILE_SUPPORT_XREF)
		#endif
	#elif defined(DESIGN_VER)
		// TODO
	#else // Asumes MAX for now, but all applications need to be added to this logic
		#define FILE_FORMAT_MAX		FILE_SUPPORT_NATIVE
		#define FILE_FORMAT_VIZR	FILE_SUPPORT_OPEN
	#endif

#endif // MULTI_FILE_SUPPORT

// SDK version defines - russom - 07/15/02
//#define USE_PRODUCT_SDK_VER
#ifdef USE_PRODUCT_SDK_VER

	// Do not change SDK_ORDER_VER or SDK_RESERVED_VER without notifying the build team.
	#define SDK_ORDER_VER		3
	#define SDK_RESERVED_VER	1

	#define SDK_BASE_METHOD(a,b) virtual void sdkReservedMethod##a##b (void){return;}

	#if (SDK_RESERVED_VER > 1)
		#define SDK_RESERVED_METHOD(a) \
			SDK_BASE_METHOD(a,1); \
			SDK_BASE_METHOD(a,2);
	#elif (SDK_RESERVED_VER > 0)
		#define SDK_RESERVED_METHOD(a) \
			SDK_BASE_METHOD(a,1);
	#else
		#define SDK_RESERVED_METHOD(a)
	#endif

#else // USE_PRODUCT_SDK_VER
#define SDK_RESERVED_METHOD(a)
#define SDK_ORDER_VER	0
#endif // USE_PRODUCT_SDK_VER

// enable dll integrity checks - russom - 07/15/02
//#define USE_INTEGRITY_CHECK

// no longer used by MAX
#if !defined(EDU_VERSION) && !defined(STUDENT_VER) && !defined(DESIGN_VER) && !defined(BETA_VERSION)
#define ORDINARY_VER
#endif

// errors that will no longer occur
#if defined(EDU_VERSION) && defined(STUDENT_VER)
#error "Both EDU_VERSION and STUDENT_VER defined in buildver.h!"
#endif

#if defined(EDU_VERSION) && defined(BETA_VERSION)
#error "Both EDU_VERSION and BETA_VERSION defined in buildver.h!"
#endif

// Enables mapping of a generic resource id to a product specific one
// through the resource overwrite mechanism 
#define RESOURCE_OVERRIDE

//	LIMITED_USER
//	Puts most subdirectories of \exe\ into Documents and Settings \ <user>
#ifdef RENDER_VER	// 021215  --prs.
#define LIMITED_USER_CUI
//#define LIMITED_USER
#ifdef LIMITED_USER_CUI	// so one definition in builder can do it all
#define LIMITED_USER
#endif // LIMITED_USER_CUI
#endif	// RENDER_VER

#define NO_DRAFT_RENDERER

#if (_MSC_VER >= 1300)  // Visual Studio .NET
//SS 6/7/2002: globally turning off certain warnings we now receive from
//the VC7 compiler. This may only be temporary.
//NH 02/12/03 Added this to R6 but I only kept the first pragma.  This was to get 
//a nicer looking build and so we could concentrate on the easier fixes.  We may want to 
//address this again
// Temporarily re-enabled the other two pragmas to help find bigger problems in the
// build logs, 020319  --prs.

// conversion from XXX to YYY, possible loss of data
#pragma warning(disable: 4244)

// signed/unsigned mismatch (for ==, >, etc.)
#pragma warning(disable: 4018) 

// loop control variable declared in the for-loop
// is used outside the for-loop scope; it conflicts with the declaration in 
// the outer scope
#pragma warning(disable: 4288) 

#endif

//JH 6/3/03 Putting this here for now
//define to add support for rendering area lights as point lights
#define SIMPLIFY_AREA_LIGHTS

#ifdef RENDER_VER
/********************************************************************
***                     FUNCTIONALITY MODIFIED                    ***
********************************************************************/

// xavier robitaille | 03.02.05 | bezier default position controller
#define BEZIER_DEFAULT_POS_CTRL

// xavier robitaille | 03.02.07 | fixed values appropriate for metric units
#define METRIC_UNITS_FIXED_VALUES
// xavier robitaille | 03.02.07 | increments proportional to the spinner value
#define AUTOSCALE_SPINNER_INCREMENTS
#endif // RENDER_VER

// xavier robitaille | 2003.03.17 | enables the calculation 
// of screen scale increments for the move transform type ins.
#define MOVE_TTI_SCREENSCALE_INCREMENTS
// xavier robitaille | 2003.03.17 | enables the calculation 
// of screen scale increments for the viewport navigation tools.
#define VIEWPORT_NAVIG_SCREENSCALE_INCREMENTS

#ifndef APSTUDIO_INVOKED
// Don't define this when APPSTUDIO is parsing resource
// files. This allows TABBED_RENDER_DIALOG to be used
// as a condition in the resource files.
// CA - 3/24/03 - Turn on tabbed render dialog in MAX

#define TABBED_RENDER_DIALOG

#endif

/////////////////////////////
//  Skylight
//
#ifndef APSTUDIO_INVOKED
// Don't define this when APPSTUDIO is parsing resource
// files. This allows TABBED_RENDER_DIALOG to be used
// as a condition in the resource files.
#define SKYLIGHT_COVERAGE_RADIO_BUTTON
#define NO_DAYLIGHT_SKY_COVERAGE_SLIDER
#endif
#define SKYLIGHT_CAN_RENDER_WITHOUT_RADIOSITY
//#define NO_SKYLIGHT_AFFECT_DIFFUSE_UI
//#define NO_SKYLIGHT_AFFECT_SPECULAR_UI
//#define NO_SKYLIGHT_EXPONENT_UI
//#define NO_SKYLIGHT_RAY_BIAS_UI
//#define NO_TEXTURE_SKYLIGHT
//#define TEXTURE_SKYLIGHT_IS_PRIVATE
//#define NO_IES_SKYLIGHT
//#define IES_SKYLIGHT_IS_PRIVATE

/////////////////////////////
//  Daylight system
//
//#define NO_DAYLIGHT_MOTION_PANEL    // If No_MOTION_PANEL is defined, this is forced to be defined
//#define NO_DAYLIGHT_SELECTOR
//#define NO_DAYLIGHT_SKY_COVERAGE_SLIDER
#define DAYLIGHT_DEFAULT_TIME_SUMMER_SOLSTICE
#define DAYLIGHT_CITY_PICKER_BIG_CITY_DEFAULT

/********************************************************************
***                      FUNCTIONALITY TURNED ON                  ***
********************************************************************/

// --- FILE LINK ---------------------------------------------------

// --- GLOBAL COORDONATE SYSTEM\GEOTIFF ----------------------------
// !!! aszabo|may.02.2003 
// Support for GCS is turned off in until we figure out the requirements 
// for this feature in R6. 
// Enables access to GCsXXX (Global Coord Sys) interfaces 
//#define GLOBAL_COORD_SYS
// Enables the GeoTiff feature
//#define GEOREFSYS_UVW_MAPPING

// --- ACAD MAXSCRIPT EXTENSIONS ------------------------------------
// aszabo|may.26.03| This turns ON symbols necessary to build the 
// ACAD maxscript extensions (src\dll\acadfuns.vcproj)
#define ACAD_MAXSCRIPT_EXTENSIONS

// --- MAPPING MODIFIERS ------------------------------------
// alexc - june.11.2003
// Turn on Map Scaler OSM in MAX (was inside #ifdef RENDER_VER before)
#define MAP_SCALER_OSM

// alexc - june.27.2003
// Turn on Sub-Object functionality for the UVW XForm modifier
// (was inside #ifdef RENDER_VER before)
#define UVW_XFORM_ON_SELECTED_FACES

// --- CUI ------------------------------------
// aszabo|jul.23.03
// When this is not defined, the 'shelf' frame that holds all tab panels 
// is created at startup and 'test' toolbars are added to it.
// Otherwise, the creation of the 'shelf' is postponed to when the user
// converts a toolbar to a tab panel, unless the current CUI file defines
// tab panels which have to live on the 'shelf'
#define TAB_PANEL_SHELF_LAZY_CREATION
// aszabo|aug.06.03
// Define this symbol if the product doesn't hardcode web links used by
// the help menu items, but rather adopts R6 approach which is
// to implemented them as maxscript macros.
#define HELP_WEB_LINKS_AS_MAXSCRIPTS

// aszabo|aug.12.03|
// Define this symbol if the product should use the new logic of handling 
// UI description files. This symbol should be removed from the build
// after testing and before shipping
#define UIFILES_NEW_HANDLING

/********************************************************************
***                      FUNCTIONALITY TURNED OFF                 ***
********************************************************************/

// --- FILE LINK ---------------------------------------------------
// aszabo - apr.24.03
// VIZBlocks are used in VIZ4. In R6, FileLink uses node hierarchies
// to represent ACAD blocks and ADT style based objects.
// This flag turns off material assignment propagation to instances of 
// a VIZBlock when a material is assigned to one of the instances.
#define NO_MTL_PROPAGATION_FOR_VIZBLOCKS
#define NO_FILELINK_MANAGER_UI

// --- SUBSTITUTION MANAGER -----------------------------------------
// aszabo - may.21.03
// The substitution manager is needed in R6 to load DRF files, 
// but it should not be exposed in the UI
#define PRIVATE_SUBSTITUTION_MGR

// --- SUBSTITUTE MODIFIER -----------------------------------------
// alexc - june.06.03
// Substitute Modifier: Private for backward compatibility 
#define PRIVATE_SUBSTITUTE_MOD

// --- XML MATERIAL EXPORT\IMPORT -----------------------------------
// aszabo - may.21.03
// The XML Material Export Utility is exposed in the Utility panel in R6
//#define PRIVATE_XML_MATERIAL_EXPORTER

// --- INSTANCE MANAGER FEATURES ------------------------------------
// aszabo - may.26.03 - Turn on these symbols in order to enable the 
// corresponding feature
//#define INSTANCEMGR_MODIFIER_PROPAGATION
//#define INSTANCEMGR_NODE_PROPERTIES_PROPAGATION
//#define INSTANCEMGR_MAKE_UNIQUE

// --- ASSEMBLIES ---------------------------------------------------
// aszabo - june.02.03 - Turn on this symbol in order to make the
// luminaire helper object private 
//#define PRIVATE_LUMINAIRE_HELPER_OBJECT

// When turned on, the Create Assembly dialog shows all helper objects
// otherwise, it shows only those part of the Assembly Head Category
//#define ASSEMBLY_SHOW_ALL_HELPERS

#endif // _BUILD_VER_

