/**********************************************************************
 *<
	FILE: istdplug.h

	DESCRIPTION:  Interfaces into some of the standard plug-ins 
	              that ship with MAX

	CREATED BY: Rolf Berteig	

	HISTORY: created 20 January 1996

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __ISTDPLUG__
#define __ISTDPLUG__

#include "iFnPub.h"
#include "units.h"

//----------------------------------------------------------------
// The following are parameter block IDs for procedural objects

// Arc
#define ARC_RADIUS		0
#define ARC_FROM		1
#define ARC_TO			2
#define ARC_PIE			3
#define ARC_REVERSE		4

// Box object
#define BOXOBJ_LENGTH	0
#define BOXOBJ_WIDTH	1
#define BOXOBJ_HEIGHT	2
#define BOXOBJ_WSEGS	3
#define BOXOBJ_LSEGS	4
#define BOXOBJ_HSEGS	5
#define BOXOBJ_GENUVS	6

// Circle
#define CIRCLE_RADIUS		0

// Cone
#define CONE_RADIUS1		0
#define CONE_RADIUS2		1
#define CONE_HEIGHT			2
#define CONE_SEGMENTS		3
#define CONE_CAPSEGMENTS	4
#define CONE_SIDES			5
#define CONE_SMOOTH			6
#define CONE_SLICEON		7
#define CONE_PIESLICE1		8
#define CONE_PIESLICE2		9
#define CONE_GENUVS			10

// Cylinder
#define CYLINDER_RADIUS			0
#define CYLINDER_HEIGHT			1
#define CYLINDER_SEGMENTS		2
#define CYLINDER_CAPSEGMENTS	3
#define CYLINDER_SIDES			4
#define CYLINDER_SMOOTH			5
#define CYLINDER_SLICEON		6
#define CYLINDER_PIESLICE1		7
#define CYLINDER_PIESLICE2		8
#define CYLINDER_GENUVS			9

// Donut
#define DONUT_RADIUS1		0
#define DONUT_RADIUS2		1

// Ellipse
#define ELLIPSE_LENGTH		0
#define ELLIPSE_WIDTH		1

// Hedra
#define HEDRA_RADIUS	0
#define HEDRA_FAMILY	1
#define HEDRA_P			2
#define HEDRA_Q			3
#define HEDRA_SCALEP	4
#define HEDRA_SCALEQ	5
#define HEDRA_SCALER	6
#define HEDRA_VERTS		7
#define HEDRA_GENUVS	8

// Helix
#define HELIX_RADIUS1		0
#define HELIX_RADIUS2		1
#define HELIX_HEIGHT		2
#define HELIX_TURNS			3
#define HELIX_BIAS			4
#define HELIX_DIRECTION		5

// NGon
#define NGON_RADIUS			0
#define NGON_SIDES			1
#define NGON_CIRCULAR		2

// PatchGrid
#define PATCHGRID_LENGTH	0
#define PATCHGRID_WIDTH		1
#define PATCHGRID_WSEGS		2
#define PATCHGRID_LSEGS		3
#define PATCHGRID_TEXTURE	4

// Rain/snow
#define RSPART_VPTPARTICLES		0
#define RSPART_RNDPARTICLES		1
#define RSPART_DROPSIZE			2
#define RSPART_SPEED			3
#define RSPART_VARIATION		4
#define RSPART_DISPTYPE			5
#define RSPART_STARTTIME		6
#define RSPART_LIFETIME			7
#define RSPART_EMITTERWIDTH		8
#define RSPART_EMITTERHEIGHT	9
#define RSPART_HIDEEMITTER		10
#define RSPART_BIRTHRATE		11
#define RSPART_CONSTANT			12
#define RSPART_RENDER			13
#define RSPART_TUMBLE			14
#define RSPART_SCALE			15

// Rectangle
#define RECTANGLE_LENGTH	0
#define RECTANGLE_WIDTH		1
#define RECTANGLE_FILLET	2

// Sphere
#define SPHERE_RADIUS	0
#define SPHERE_SEGS		1
#define SPHERE_SMOOTH	2
#define SPHERE_HEMI		3
#define SPHERE_SQUASH	4
#define SPHERE_RECENTER	5
#define SPHERE_GENUVS	6

// Star
#define START_RADIUS1		0
#define START_RADIUS2		1
#define START_POINTS		2
#define START_DISTORT		3
#define START_FILLET1		4
#define START_FILLET2		5


// Tea Pot
#define TEAPOT_RADIUS	0
#define TEAPOT_SEGS		1
#define TEAPOT_SMOOTH	2
#define TEAPOT_TEAPART	3
#define TEAPOT_BODY		4
#define TEAPOT_HANDLE	5
#define TEAPOT_SPOUT	6
#define TEAPOT_LID		7
#define TEAPOT_GENUVS	8


// Text
#define TEXT_SIZE 0
#define TEXT_KERNING 1
#define TEXT_LEADING 2

// torus
#define TORUS_RADIUS		0
#define TORUS_RADIUS2		1
#define TORUS_ROTATION		2
#define TORUS_TWIST			3
#define TORUS_SEGMENTS		4
#define TORUS_SIDES			5
#define TORUS_SMOOTH		6
#define TORUS_SLICEON		7
#define TORUS_PIESLICE1		8
#define TORUS_PIESLICE2		9
#define TORUS_GENUVS		10

// Tube
#define TUBE_RADIUS			0
#define TUBE_RADIUS2		1
#define TUBE_HEIGHT			2
#define TUBE_SEGMENTS		3
#define TUBE_CAPSEGMENTS	4
#define TUBE_SIDES			5
#define TUBE_SMOOTH			6
#define TUBE_SLICEON		7
#define TUBE_PIESLICE1		8
#define TUBE_PIESLICE2		9
#define TUBE_GENUVS			10

// Grid
#define GRIDHELP_LENGTH			0
#define GRIDHELP_WIDTH			1
#define GRIDHELP_GRID			2


//----------------------------------------------------------------
// The following are parameter block IDs for modifiers

// Bend
#define BEND_ANGLE		0
#define BEND_DIR		1
#define BEND_AXIS		2
#define BEND_DOREGION	3
#define BEND_FROM		4
#define BEND_TO			5

// Bomb
#define BOMB_STRENGTH		0
#define BOMB_GRAVITY		1
#define BOMB_CHAOS			2
#define BOMB_DETONATION		3

// Deflector
#define DEFLECTOR_BOUNCE	0
#define DEFLECTOR_WIDTH		1
#define DEFLECTOR_HEIGHT	2

// Displace (modifier and space warp object)
#define DISPLACE_MAPTYPE		0
#define DISPLACE_UTILE			1
#define DISPLACE_VTILE			2
#define DISPLACE_WTILE			3
#define DISPLACE_BLUR			4
#define DISPLACE_USEMAP			5
#define DISPLACE_APPLYMAP		6
#define DISPLACE_STRENGTH		7
#define DISPLACE_DECAY			8
#define DISPLACE_CENTERLUM		9
#define DISPLACE_UFLIP			10
#define DISPLACE_VFLIP			11
#define DISPLACE_WFLIP			12
#define DISPLACE_CENTERL		13
#define DISPLACE_CAP			14
#define DISPLACE_LENGTH			15
#define DISPLACE_WIDTH			16
#define DISPLACE_HEIGHT			17
#define DISPLACE_AXIS			18


// Extrude
#define EXTRUDE_AMOUNT			0
#define EXTRUDE_SEGS			1
#define EXTRUDE_CAPSTART		2
#define EXTRUDE_CAPEND			3
#define EXTRUDE_CAPTYPE			4
#define EXTRUDE_OUTPUT			5
#define EXTRUDE_MAPPING			6
#define EXTRUDE_GEN_MATIDS		7
#define EXTRUDE_USE_SHAPEIDS	8
#define EXTRUDE_SMOOTH			9

// Gravity
#define GRAVITY_STRENGTH		0
#define GRAVITY_DECAY			1
#define GRAVITY_TYPE			2
#define GRAVITY_DISPLENGTH		3

// Wind
#define WIND_STRENGTH		0
#define WIND_DECAY			1
#define WIND_TYPE			2
#define WIND_DISPLENGTH		3
#define WIND_TURBULENCE		4
#define WIND_FREQUENCY		5
#define WIND_SCALE			6

// UVW map
#define UVWMAP_MAPTYPE		0
#define UVWMAP_UTILE		1
#define UVWMAP_VTILE		2
#define UVWMAP_WTILE		3
#define UVWMAP_UFLIP		4
#define UVWMAP_VFLIP		5
#define UVWMAP_WFLIP		6
#define UVWMAP_CAP			7
#define UVWMAP_CHANNEL		8
#define UVWMAP_LENGTH		9
#define UVWMAP_WIDTH		10
#define UVWMAP_HEIGHT		11
#define UVWMAP_AXIS			12


// Noise mod
#define NOISEMOD_SEED			0
#define NOISEMOD_SCALE			1
#define NOISEMOD_FRACTAL		2
#define NOISEMOD_ROUGH			3
#define NOISEMOD_ITERATIONS		4
#define NOISEMOD_ANIMATE		5
#define NOISEMOD_FREQ			6
#define NOISEMOD_PHASE			7
#define NOISEMOD_STRENGTH		8

// Optimize
#define OPTMOD_RENDER			0
#define OPTMOD_VIEWS			1
#define OPTMOD_FACETHRESH1		2
#define OPTMOD_EDGETHRESH1		3
#define OPTMOD_BIAS1			4
#define OPTMOD_PRESERVEMAT1		5
#define OPTMOD_PRESERVESMOOTH1	6
#define OPTMOD_MAXEDGE1			7
#define OPTMOD_FACETHRESH2		8
#define OPTMOD_EDGETHRESH2		9
#define OPTMOD_BIAS2			10
#define OPTMOD_PRESERVEMAT2		11
#define OPTMOD_PRESERVESMOOTH2	12
#define OPTMOD_MAXEDGE2			13
#define OPTMOD_AUTOEDGE			14
#define OPTMOD_MANUPDATE		15

// Volume selection modifier
#define VOLSEL_LEVEL	0
#define VOLSEL_METHOD	1
#define VOLSEL_TYPE		2
#define VOLSEL_VOLUME	3
#define VOLSEL_INVERT	4

// Ripple/Wave space warp object and object space modifier
#define RWAVE_AMPLITUDE		0
#define RWAVE_AMPLITUDE2	1
#define RWAVE_WAVELEN		2
#define RWAVE_PHASE			3
#define RWAVE_DECAY			4

#define RWAVE_CIRCLES		5 // These last three are only valid for space warp objects
#define RWAVE_SEGMENTS		6
#define RWAVE_DIVISIONS		7

// Ripple/Wave binding (modifier)
#define RWAVE_FLEX			0

// Skew
#define SKEW_AMOUNT		0
#define SKEW_DIR		1
#define SKEW_AXIS		2
#define SKEW_DOREGION	3
#define SKEW_FROM		4
#define SKEW_TO			5

// Material modifier
#define MATMOD_MATID 0

// Smoothing group modifier
#define SMOOTHMOD_AUTOSMOOTH 	0
#define SMOOTHMOD_THRESHOLD		1
#define SMOOTHMOD_SMOOTHBITS	2

// Normal modifier
#define NORMMOD_UNIFY	0
#define NORMMOD_FLIP 	1

// SurfRev (Lathe) modifier
#define SURFREV_DEGREES			0
#define SURFREV_SEGS			1
#define SURFREV_CAPSTART		2
#define SURFREV_CAPEND			3
#define SURFREV_CAPTYPE			4
#define SURFREV_WELDCORE		5
#define SURFREV_OUTPUT			6
#define SURFREV_MAPPING			7
// Taper 
#define TAPER_AMT			0
#define TAPER_CRV			1
#define TAPER_AXIS			2
#define TAPER_EFFECTAXIS	3
#define TAPER_SYMMETRY		4
#define TAPER_DOREGION		5
#define TAPER_FROM			6
#define TAPER_TO			7

// Twist
#define TWIST_ANGLE		0
#define TWIST_BIAS		1
#define TWIST_AXIS		2
#define TWIST_DOREGION	3
#define TWIST_FROM		4
#define TWIST_TO		5

// Material mod
#define MATMOD_MATID	0

// Smooth mod
#define SMOOTH_AUTOSMOOTH 	0
#define SMOOTH_THRESHOLD	1
#define SMOOTH_SMOOTHBITS	2

// Normal mod
#define NORMALMOD_UNIFY		0
#define NORMALMOD_FLIP 		1

// Tessellation mod
#define TESSMOD_TYPE		0
#define TESSMOD_TENSION		1
#define TESSMOD_ITERATIONS	2
#define TESSMOD_FACE_TYPE	3

// UVW xform
#define UVWXFORM_UTILE		0
#define UVWXFORM_VTILE		1
#define UVWXFORM_WTILE		2
#define UVWXFORM_UOFFSET	3
#define UVWXFORM_VOFFSET	4
#define UVWXFORM_WOFFSET	5
#define UVWXFORM_UFLIP		6
#define UVWXFORM_VFLIP		7
#define UVWXFORM_WFLIP		8
#define UVWXFORM_CHANNEL	9


//-- Text shape object interface -------------------------

// Use GetTextObjectInterface() to get a pointer to an 
// ITextObject given a pointer to an Object. 


// Flags passed to ChangeFont()
#define TEXTOBJ_ITALIC		(1<<1)
#define TEXTOBJ_UNDERLINE	(1<<2)

// Alignment types
#define TEXTOBJ_LEFT 0
#define TEXTOBJ_CENTER 1
#define TEXTOBJ_RIGHT 2
#define TEXTOBJ_JUSTIFIED 3

class ITextObject {
	public:
		// Returns TRUE if string is changed. Can't change string if current font is not installed
		virtual BOOL ChangeText(TSTR string)=0;
		
		// Returns TRUE if font is successfully changed.
		virtual BOOL ChangeFont(TSTR name, DWORD flags)=0;

		// Get fount and string
		virtual TSTR GetFont()=0;
		virtual TSTR GetString()=0;
		
		// Get/Set styles
		virtual BOOL GetItalic()=0;
		virtual BOOL GetUnderline()=0;
		virtual void SetItalic(BOOL sw)=0;
		virtual void SetUnderline(BOOL sw)=0;

		// Get/Set alignment
		virtual BOOL SetAlignment(int type)=0;
		virtual int GetAlignment()=0;
	};



//-- Controller interfaces -------------------------------

// Base key class
class IKey {
	public:
		TimeValue time;
		DWORD flags;
		IKey() {time=0;flags=0;}
	};

//--- TCB keys -------------

class ITCBKey : public IKey {
	public:		
		float tens, cont, bias, easeIn, easeOut;
	};

class ITCBFloatKey : public ITCBKey {
	public:
		float val;		
	};

class ITCBPoint4Key : public ITCBKey {
public:
	Point4 val;		
};

class ITCBPoint3Key : public ITCBKey {
	public:
		Point3 val;		
	};

class ITCBRotKey : public ITCBKey {
	public:
		AngAxis val;		
	};

class ITCBScaleKey : public ITCBKey {
	public:
		ScaleValue val;		
	};


//--- Bezier keys -------------

class IBezFloatKey : public IKey {
	public:
		float intan, outtan;
		float val;
//watje horizontal handles
//these are the length of the handles
		float inLength, outLength;
	};

class IBezPoint3Key : public IKey {
	public:
		Point3 intan, outtan;
		Point3 val;
//watje horizontal handles
//these are the length of the handles
		Point3 inLength, outLength;
	};

class IBezQuatKey : public IKey  {
	public:		
		Quat val;
	};

class IBezScaleKey : public IKey  {
	public:
		Point3 intan, outtan;
		ScaleValue val;
//watje horizontal handles
//these are the length of the handles
		Point3 inLength, outLength;
	};

class IBezPoint4Key : public IKey {
public:
	Point4 intan, outtan;
	Point4 val;
	//watje horizontal handles
	//these are the length of the handles
	Point4 inLength, outLength;
};


//--- Linear Keys --------------

class ILinFloatKey : public IKey {
	public:
		float val;
	};

class ILinPoint3Key : public IKey {
	public:
		Point3 val;
	};

class ILinRotKey : public IKey {
	public:
		Quat val;
	};

class ILinScaleKey : public IKey {
	public:
		ScaleValue val;
	};

//--- Boolean Controller Keys -------------- AG: 11/08/01

class IBoolFloatKey : public IKey {
	public:
		float val;
	};



// --- Flag bits for keys -------------------------------

// General flags
#define IKEY_SELECTED	(1<<0)
#define IKEY_XSEL		(1<<1)
#define IKEY_YSEL		(1<<2)
#define IKEY_ZSEL		(1<<3)
#define IKEY_WSEL		(1<<30)
#define IKEY_FLAGGED	(1<<31)  //13
#define IKEY_TIME_LOCK	(1<<14)

#define IKEY_ALLSEL		(IKEY_SELECTED|IKEY_XSEL|IKEY_YSEL|IKEY_ZSEL|IKEY_WSEL)

#define IKEY_VALLOCK_SHIFT	16
#define IKEY_VALX_LOCK		(1<<IKEY_VALLOCK_SHIFT)
#define IKEY_VALY_LOCK		(1<<(IKEY_VALLOCK_SHIFT+1))
#define IKEY_VALZ_LOCK		(1<<(IKEY_VALLOCK_SHIFT+2))
#define IKEY_VALA_LOCK		(1<<(IKEY_VALLOCK_SHIFT+3))

// TCB specific key flags
#define TCBKEY_QUATVALID	(1<<4) // When this bit is set the angle/axis is derived from the quat instead of vice/versa

// Bezier specific key flags
#define BEZKEY_XBROKEN		(1<<4) // Broken means not locked
#define BEZKEY_YBROKEN		(1<<5)
#define BEZKEY_ZBROKEN		(1<<6)
#define BEZKEY_WBROKEN		(1<<21)

// The in and out types are stored in bits 7-13
#define BEZKEY_NUMTYPEBITS	3
#define BEZKEY_INTYPESHIFT	7
#define	BEZKEY_OUTTYPESHIFT	(BEZKEY_INTYPESHIFT+BEZKEY_NUMTYPEBITS)
#define BEZKEY_TYPEMASK		7

// Bezier tangent types
#define BEZKEY_SMOOTH	0
#define BEZKEY_LINEAR	1
#define BEZKEY_STEP		2
#define BEZKEY_FAST		3
#define BEZKEY_SLOW		4
#define BEZKEY_USER		5
#define BEZKEY_FLAT		6

#define NUM_TANGENTTYPES	7

// This key is interpolated using arclength as the interpolation parameter
#define BEZKEY_CONSTVELOCITY	(1<<15)
//watje determines whether a user handle is limited
#define BEZKEY_UNCONSTRAINHANDLE		(1<<20)

#define TangentsLocked(f,j) (!(f&(j <= 2 ? (BEZKEY_XBROKEN<<j) : BEZKEY_WBROKEN)))
#define SetTangentLock(f,j,l) {if (l) (f)=(f)&(~(j <= 2 ? (BEZKEY_XBROKEN<<j) : BEZKEY_WBROKEN)); else (f)|=(j <= 2 ? (BEZKEY_XBROKEN<<j) : BEZKEY_WBROKEN);}

// Macros to access hybrid tangent types
#define GetInTanType(f)  int(((f)>>BEZKEY_INTYPESHIFT)&BEZKEY_TYPEMASK)
#define GetOutTanType(f) int(((f)>>BEZKEY_OUTTYPESHIFT)&BEZKEY_TYPEMASK)
#define SetInTanType(f,t)  {(f) = ((f)&(~(BEZKEY_TYPEMASK<<BEZKEY_INTYPESHIFT)))|(t<<BEZKEY_INTYPESHIFT);}
#define SetOutTanType(f,t) {(f) = ((f)&(~(BEZKEY_TYPEMASK<<BEZKEY_OUTTYPESHIFT)))|(t<<BEZKEY_OUTTYPESHIFT);}

// HitTrackRecord flags
// KEY_XSEL, KEY_YSEL, KEY_ZSEL, and KEY_WSEL are also used to identify the component
#define HITKEY_INTAN	(1<<10)
#define HITKEY_OUTTAN	(1<<11)

// Track flags
#define TFLAG_CURVESEL			(1<<0)
#define TFLAG_RANGE_UNLOCKED	(1<<1)
#define TFLAG_LOOPEDIN			(1<<3)
#define TFLAG_LOOPEDOUT			(1<<4)
#define TFLAG_COLOR				(1<<5)	// Set for Bezier Point3/Point4 controlers that are color controllers
#define TFLAG_HSV				(1<<6)	// Set for color controls that interpolate in HSV
#define TRACK_XLOCKED			(1<<7)	// Used by controller to lock Y and Z to X.
#define KT_FLAG_DELAY_KEYSCHANGED (1<<8)
#define TFLAG_NOTKEYABLE		(1<<9)
#define TFLAG_TCBQUAT_NOWINDUP	(1<<10)

//-------------------------------------------------------
// This is an interface into key frame controllers. 
// To get a pointer to the IKeyControl interface given a pointer to a controller,
// use the macro defined in animtbl.h: GetKeyControlInterface()
// Use class AnyKey as wrapper for IKey for automatic handling of memory. See example below

class IKeyControl {
	public:
		// Total number of keys.
		virtual int GetNumKeys()=0;
		
		// Sets the number of keys allocated. 
		// May add blank keys or delete existing keys
		virtual void SetNumKeys(int n)=0;
		
		// Fill in 'key' with the ith key
		virtual void GetKey(int i,IKey *key)=0;
		
		// Set the ith key
		virtual void SetKey(int i,IKey *key)=0;

		// Append a new key onto the end. Note that the
		// key list will ultimately be sorted by time. Returns
		// the key's index.
		virtual int AppendKey(IKey *key)=0;

		// If any changes are made that would require the keys to be sorted
		// this method should be called.
		virtual void SortKeys()=0;		

		// Access track flags
		virtual DWORD &GetTrackFlags()=0;

		// Specify the max size of a key in bytes. Just need to implement if 
		// size of IKey is greater than this default value.
		virtual int GetKeySize() {return 128;}
	};

class AnyKey
{
public:
	Tab<char> data;
	AnyKey(int size = 128) { data.SetCount(size); } // 128 is default from IKeyControl::GetKeySize()
	void SetSize(int size) { data.SetCount(size); }
	operator IKey*() { return (IKey*)data.Addr(0); }
};


// ------- example:
//	IKeyControl* ki = GetKeyControlInterface(controller);
//	if (ki != NULL)
//	{
//		if (key_index >= ki->GetNumKeys())
//			throw RuntimeError (GetString(IDS_KEY_NO_LONGER_EXISTS_IN_CONTROLLER), controller);
//		AnyKey ak(ki->GetKeySize()); IKey* k = ak;
//		ki->GetKey(key_index, k);


//--------------------------------------------------------------
// The following interface is an FP interface to flag TFLAG_TCBQUAT_NOWINDUP
// of IKeyControl::GetTrackFlags(). Specifically,
//   IRotWindup::GetRotWindup() ==
//		!(IKeyControl::GetTrackFlags()&TFLAG_TCBQUAT_NOWINDUP)
// However, IRotWindup::SetRotWindup() will notify dependents and handle
// undo/redo, etc.
// This interface is only available to TCB rotation controller (of class id,
// TCBINTERP_ROTATION_CLASS_ID).
// R4.5 and later only.
//
#define ROTWINDUP_INTERFACE Interface_ID(0x13a3032c, 0x381345ca)
class IRotWindup : public FPMixinInterface {
public:
	static IRotWindup* GetIRotWindup(Animatable& a) {
		return static_cast<IRotWindup*>(a.GetInterface(ROTWINDUP_INTERFACE));}

	FPInterfaceDesc* GetDesc() { return GetDescByID(ROTWINDUP_INTERFACE); }

	virtual bool GetRotWindup() const =0;
	virtual void SetRotWindup(bool) =0;

	enum FuncID {
		kRotWindupGet, kRotWindupSet
	};

BEGIN_FUNCTION_MAP
	PROP_FNS(kRotWindupGet, GetRotWindup, kRotWindupSet, SetRotWindup, TYPE_bool)
END_FUNCTION_MAP
};

// Access to default key parameters
CoreExport void SetBezierDefaultTangentType(int in, int out);
CoreExport void GetBezierDefaultTangentType(int &in, int &out);

CoreExport void SetTCBDefaultParams(float t, float c, float b, float easeIn, float easeOut);
CoreExport void GetTCBDefaultParams(float &t, float &c, float &b, float &easeIn, float &easeOut);

// An interface for supporting mutiple tangent adjustment.  
// If the user adjusts a tangent from a different track this tells the controller what angle to apply to its selected keys
// The first track hit is called to adjust its own handles and return the difference in angle and the 
// percentage difference in length.  All other tracks use this info to adjust their own handles.
// R4.5 and later only
class IAdjustMultipleTangents {
	public:
		virtual void AdjustInitialTangents(TrackHitRecord hit,ParamDimensionBase *dim,Rect& rcGraph,
							float tzoom,int tscroll,float vzoom,int vscroll,int dx,int dy,DWORD flags, 
							float &angle, float &length)=0;  // the angle and percentage of length offsets are retrieved here
		virtual void AdjustSecondaryTangents(DWORD hitFlags,ParamDimensionBase *dim,Rect& rcGraph,
							float tzoom,int tscroll,float vzoom,int vscroll,float angle, float length, DWORD flags)=0;

	};

// An interface for computing and gathering weighted key info
// If a keyframe controller wants to support soft selections, they should return an instance of this class when 
// GetInterface(I_SOFTSELECT) is called.  In order to limit the amount of data held by keyframe controllers 
// the controller should build a weight table when ComputeWeights is called, and delete the weight table when ReleaseWeights()
// is called.  The weight table is only valid while in Soft Selection mode.  The controller should keep track of how many times
// GetInterface is called and only ReleaseWeights when the last client calls it.
// R4.5 and later only
class ISoftSelect {
	public:
		virtual void ComputeWeights(TimeValue range, float falloff)=0;
		virtual float GetWeight(int i)=0;
		virtual void ReleaseWeights()=0;
	};

//-----------------------------------------------------------
// A plug-in can register itself to read a particular APP_DATA 
// chunk when a 3DS file is loaded. If a chunk is encountered
// that matches a registered plug-in, that plug-in will be
// asked to create an instance of itself based on the contents
// of the APP_DATA chunk.

class TriObject;

class ObjectDataReaderCallback {
	public:
		// Chunk name
		virtual char *DataName()=0;

		// Create an instance of an object based on the data and the original mesh object
		virtual Object *ReadData(TriObject *obj, void *data, DWORD len)=0;

		virtual void DeleteThis()=0;
	};
 
CoreExport void RegisterObjectAppDataReader(ObjectDataReaderCallback *cb);
 
CoreExport Object *ObjectFromAppData(TriObject *obj, char *name, void *data, DWORD len);


// Note about 3DS App Data:
// If app data is encountered and no plug-in has registered to
// convert it, then it is just hung off the object (or INode in
// the case of KXP app data).
// For object app data, TriObject's super class and class ID are used
// to identify the chunk and the sub ID is set to 0.
// For node app data, INode's super class and class ID are used
// to identify the chunk and the sub ID is set to 0.
//
// This single MAX app data chunk will contain the entire
// 3DS app data chunk, which may have sub chunks (see IPAS SDK).
// The following routines will aid in parsing 3DS app data.

// Get the ID string out of an XDATA_ENTRY chunk and null terminates it
CoreExport void GetIDStr(char *chunk, char *idstring);

// Returns the offset into 'appd' of the specified chunk 
// or -1 if it is not found
CoreExport int FindAppDataChunk(void *appd, DWORD len, char *idstring);

// Similar to Find, but actually returns a pointer to the chunk
// or NULL if it is not found
CoreExport void *GetAppDataChunk(void *appd, DWORD len, char *idstring);

// Adds the chunk to the appdata chunk, preserving existing chunks.
// 'chunk' should point to the new chunk header followed by its data.
CoreExport int SetAppDataChunk(void **pappd, DWORD &len, void *chunk);

// Deletes a chunk from the appdata while preserving other chunks.
CoreExport int DeleteAppDataChunk(void **pappd, DWORD &len, char *idstring);

// Known sub chunks inside an appdata chunk
#define XDATA_ENTRY		0x8001
#define XDATA_APPNAME	0x8002



//---------------------------------------------------------
// Interface into MAX's default WAV sound object
// use the Interface method GetSoundObject() to get a pointer
// to the current sound object and then use the
// GetWaveSoundInterface() on the result to see if it supports
// this interface.

class IWaveSound {
	public:
		// Retreives the name of the current sound file
		virtual TSTR GetSoundFileName()=0;
		
		// Sets the sound file. This will cause the WAV to
		// be loaded into the tack view. Returns FALSE if
		// the file can't be opened or no wave track exist.
		virtual BOOL SetSoundFileName(TSTR name)=0;

		// Set the time offset for the wave
		virtual void SetStartTime(TimeValue t)=0;

		// Get the time offset for the wave
		virtual TimeValue GetStartTime()=0;		
		virtual TimeValue GetEndTime()=0;
	};


//-----------------------------------------------------------
//
// Access to the boolean object's parameters. Given a pointer to
// an object whose class ID is Class_ID(BOOLOBJ_CLASS_ID,0) or
// NEWBOOL_CLASS_ID, you can cast that pointer to the following
// class.  Note that some options do not work in the old Boolean
// (BOOLOBJ_CLASS_ID), and there is no Optimize parameter in
// the new Boolean.
//

#define BOOLOP_UNION			0
#define BOOLOP_INTERSECTION		1
#define BOOLOP_SUB_AB			2
#define BOOLOP_SUB_BA			3
#define BOOLOP_CUT				4

#define BOOLOP_CUT_REFINE  0
#define BOOLOP_CUT_SEPARATE  1
#define BOOLOP_CUT_REMOVE_IN  2
#define BOOLOP_CUT_REMOVE_OUT  3

#define BOOLUPDATE_ALWAYS		0
#define BOOLUPDATE_SELECTED		1
#define BOOLUPDATE_RENDER		2
#define BOOLUPDATE_MANUAL		3

#define BOOL_ADDOP_REFERENCE 0
#define BOOL_ADDOP_INSTANCE 1
#define BOOL_ADDOP_COPY 2
#define BOOL_ADDOP_MOVE 3

#define BOOL_MAT_NO_MODIFY 0
#define BOOL_MAT_IDTOMAT 1
#define BOOL_MAT_MATTOID 2
#define BOOL_MAT_DISCARD_ORIG 3
#define BOOL_MAT_DISCARD_NEW 4

class IBoolObject : public GeomObject {
public:
	virtual BOOL GetOperandSel(int which)=0;
	virtual void SetOperandSel(int which,BOOL sel)=0;
	virtual int GetBoolOp()=0;
	virtual void SetBoolOp(int op)=0;
	virtual int GetBoolCutType()=0;
	virtual void SetBoolCutType(int ct)=0;
	virtual BOOL GetDisplayResult()=0;
	virtual void SetDisplayResult(BOOL onOff)=0;
	virtual BOOL GetShowHiddenOps()=0;
	virtual void SetShowHiddenOps(BOOL onOff)=0;
	virtual int GetUpdateMode()=0;
	virtual void SetUpdateMode(int mode)=0;
	virtual BOOL GetOptimize()=0;
	virtual void SetOptimize(BOOL onOff)=0;
	virtual void SetOperandA (TimeValue t, INode *node)=0;
	virtual void SetOperandB (TimeValue t, INode *node, INode *boolNode,
		int addOpMethod=0, int matMergeMethod=0, bool *canUndo=NULL)=0;
};

// The boolean object has five references. 2 references to the
// operand objects, 2 references to transform controllers 
// providing a transformation matrix for the 2 operands,
// and one to the parameter block.
#define BOOLREF_OBJECT1		0
#define BOOLREF_OBJECT2		1
#define BOOLREF_CONT1		2
#define BOOLREF_CONT2		3
#define BOOLREF_PBLOCK     4

//-------------------------------------------------------------
// Access to path controller's parameters.
//
//Function Publishing interface Added by Ambarish Goswami (8-26-2000)
//***********************************************************

class IPathPosition;

#define PATH_CONSTRAINT_INTERFACE Interface_ID(0x79d15f78, 0x1f901f8e)
#define GetIPathConstInterface(cd) \
		(IPathPosition*)(cd)->GetInterface(PATH_CONSTRAINT_INTERFACE)


class IPathPosition : public Control, public FPMixinInterface  
{
	public:

		enum {	get_num_targets,		get_node,			get_target_weight,		
				set_target_weight,		append_target,		delete_target,};

		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
		FN_0(get_num_targets,		TYPE_INT,  GetNumTargets);
		FN_1(get_node,				TYPE_INODE, GetNode, TYPE_INDEX);
		FN_1(get_target_weight,		TYPE_FLOAT, GetTargetWeight, TYPE_INDEX);
		FN_2(set_target_weight,		TYPE_BOOL, SetTargetWeight, TYPE_INDEX, TYPE_FLOAT);
		FN_2(append_target,			TYPE_BOOL, AppendTarget, TYPE_INODE, TYPE_FLOAT);
		FN_1(delete_target,			TYPE_BOOL, DeleteTarget, TYPE_INDEX);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 

		//End of Function Publishing system code 
		//***********************************

		virtual int GetNumTargets()=0;
		virtual INode* GetNode(int targetNumber)=0;
		virtual	float GetTargetWeight(int targetNumber)=0;
		virtual BOOL SetTargetWeight(int targetNumber, float weight)=0;
		virtual BOOL AppendTarget(INode *target, float weight=50.0)=0;
		virtual BOOL DeleteTarget(int selection)=0;	

		virtual void SetFollow(BOOL f)=0;
		virtual BOOL GetFollow()=0;
		virtual void SetBankAmount(float a)=0;
		virtual float GetBankAmount()=0;
		virtual void SetBank(BOOL b)=0;
		virtual BOOL GetBank()=0;
		virtual void SetTracking(float t)=0;		// smoothness
		virtual float GetTracking()=0;
		virtual void SetAllowFlip(BOOL f)=0;
		virtual BOOL GetAllowFlip()=0;
		virtual void SetConstVel(BOOL cv)=0;
		virtual BOOL GetConstVel()=0;
		virtual void SetFlip(BOOL onOff)=0;
		virtual BOOL GetFlip()=0;
		virtual void SetAxis(int axis)=0;
		virtual int GetAxis()=0;
		virtual void SetLoop(BOOL l)=0;			// AG added
		virtual BOOL GetLoop()=0;				// AG added
		virtual void SetRelative(BOOL rel)=0;	// AG added
		virtual BOOL GetRelative()=0;	

	};

// block IDs
enum { path_params, path_joint_params };

// path_params param IDs
enum {	path_percent,			path_path,			path_follow,  
		path_bank,				path_bank_amount,	path_smoothness, 
		path_allow_upsidedown,	path_constant_vel,	path_axis, 
		path_axis_flip,			path_path_weight,	path_path_list, 
		path_loop,				path_relative,};

// Bank and tracking are scaled in the UI.
#define BANKSCALE 100.0f
#define FromBankUI(a) ((a)*BANKSCALE)
#define ToBankUI(a)	  ((a)/BANKSCALE)

#define TRACKSCALE 0.04f
#define FromTrackUI(a) ((a)*TRACKSCALE)
#define ToTrackUI(a)   ((a)/TRACKSCALE)

// percent controller, path node and paramblock2 refs
// #define PATHPOS_PERCENT_REF	0    // obsolete in Ed. 2, percent is an animatable in the ParamBlock
#define PATHPOS_PATH_REF	1
#define PATHPOS_PBLOCK_REF	2


//-------------------------------------------------------------
// // Access to Position Constraint controller's parameters.
//	Ambarish Goswami implemented April, 2000

//Function Publishing interface Added by Adam Felt (5-16-00)
//***********************************************************

class IPosConstPosition;

#define POS_CONSTRAINT_INTERFACE Interface_ID(0x32040779, 0x794a1278)
#define GetIPosConstInterface(cd) \
		(IPosConstPosition*)(cd)->GetInterface(POS_CONSTRAINT_INTERFACE)

//***********************************************************

class IPosConstPosition : public Control, public FPMixinInterface {
	public:

		enum {	get_num_targets,		get_node,			get_target_weight,		
				set_target_weight,		append_target,		delete_target,};
		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
		FN_0(get_num_targets,				TYPE_INT, GetNumTargets);
		FN_1(get_node,						TYPE_INODE, GetNode, TYPE_INDEX);
		FN_1(get_target_weight,				TYPE_FLOAT, GetTargetWeight, TYPE_INDEX);
		FN_2(set_target_weight,				TYPE_BOOL, SetTargetWeight, TYPE_INDEX, TYPE_FLOAT);
		FN_2(append_target,					TYPE_BOOL, AppendTarget, TYPE_INODE, TYPE_FLOAT);
		FN_1(delete_target,					TYPE_BOOL, DeleteTarget, TYPE_INDEX);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		virtual int GetNumTargets()=0;
		virtual INode* GetNode(int targetNumber)=0;
		virtual	float GetTargetWeight(int targetNumber)=0;
		virtual BOOL SetTargetWeight(int targetNumber, float weight)=0;
		virtual BOOL AppendTarget(INode *target, float weight=50.0)=0;
		virtual BOOL DeleteTarget(int selection)=0;	
	};

#define POSPOS_PBLOCK_REF	0


//-------------------------------------------------------------
// // Access to Orientation Constraint controller's parameters.
//	Ambarish Goswami implemented May, 2000

//Function Publishing interface Added by Ambarish Goswami 6/18/2000 adapted from Adam Felt (5-16-00)
//**************************************************************************************************

class IOrientConstRotation;

#define ORIENT_CONSTRAINT_INTERFACE Interface_ID(0x71e2231b, 0x72522ab2)
#define GetIOrientConstInterface(cd) \
		(IOrientConstRotation*)(cd)->GetInterface(ORIENT_CONSTRAINT_INTERFACE)

//***********************************************************

class IOrientConstRotation : public Control, public FPMixinInterface {
	public:

		enum {	get_num_targets,		get_node,			get_target_weight,		
				set_target_weight,		append_target,		delete_target,};

		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
		FN_0(get_num_targets,		TYPE_INT,  GetNumTargets);
		FN_1(get_node,				TYPE_INODE, GetNode, TYPE_INDEX);
		FN_1(get_target_weight,		TYPE_FLOAT, GetTargetWeight, TYPE_INDEX);
		FN_2(set_target_weight,		TYPE_BOOL, SetTargetWeight, TYPE_INDEX, TYPE_FLOAT);
		FN_2(append_target,			TYPE_BOOL, AppendTarget, TYPE_INODE, TYPE_FLOAT);
		FN_1(delete_target,			TYPE_BOOL, DeleteTarget, TYPE_INDEX);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		virtual int GetNumTargets()=0;
		virtual INode* GetNode(int targetNumber)=0;
		virtual	float GetTargetWeight(int targetNumber)=0;
		virtual BOOL SetTargetWeight(int targetNumber, float weight)=0;
		virtual BOOL AppendTarget(INode *target, float weight=50.0)=0;
		virtual BOOL DeleteTarget(int selection)=0;
	};

#define ORIENT_ROT_PBLOCK_REF	0



//-------------------------------------------------------------
// // Access to LookAt Constraint controller's parameters.
//	Ambarish Goswami implemented May, 2000

//Function Publishing interface Added by Ambarish Goswami 5/24/2000 adapted from Adam Felt (5-16-00)
//**************************************************************************************************


class ILookAtConstRotation;

#define LOOKAT_CONSTRAINT_INTERFACE Interface_ID(0x5dbe7ad8, 0x1d1b488b)
#define GetILookAtConstInterface(cd) \
		(ILookAtConstRotation*)(cd)->GetInterface(LOOKAT_CONSTRAINT_INTERFACE)

//***********************************************************

class ILookAtConstRotation : public Control , public FPMixinInterface {
	public:

		enum {	get_num_targets,		get_node,			get_target_weight,		
				set_target_weight,		append_target,		delete_target,};

		// FUNCTION_PUBLISHING
		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
		FN_0(get_num_targets,		TYPE_INT,  GetNumTargets);
		FN_1(get_node,				TYPE_INODE, GetNode, TYPE_INDEX);
		FN_1(get_target_weight,		TYPE_FLOAT, GetTargetWeight, TYPE_INDEX);
		FN_2(set_target_weight,		TYPE_BOOL, SetTargetWeight, TYPE_INDEX, TYPE_FLOAT);
		FN_2(append_target,			TYPE_BOOL, AppendTarget, TYPE_INODE, TYPE_FLOAT);
		FN_1(delete_target,			TYPE_BOOL, DeleteTarget, TYPE_INDEX);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		virtual int GetNumTargets()=0;
		virtual INode* GetNode(int targetNumber)=0;
		virtual	float GetTargetWeight(int targetNumber)=0;
		virtual BOOL SetTargetWeight(int targetNumber, float weight)=0;
		virtual BOOL AppendTarget(INode *target, float weight=50.0)=0;
		virtual BOOL DeleteTarget(int selection)=0;


		virtual BOOL GetRelative()=0;
		virtual BOOL GetVLisAbs()=0;
		virtual BOOL GetUpnodeWorld()=0;
		virtual BOOL GetStoUPAxisFlip()=0;
		virtual BOOL GetTargetAxisFlip()=0;
		virtual BOOL Get_SetOrientation()=0;
		virtual int GetTargetAxis()=0;
		virtual int GetUpNodeAxis()=0;
		virtual int Get_StoUPAxis()=0;
		virtual int Get_upnode_control()=0;
		virtual void SetRelative(BOOL rel)=0;
		virtual void SetVLisAbs(BOOL rel)=0;
		virtual void SetUpnodeWorld(BOOL uw)=0;
		virtual void SetTargetAxisFlip(BOOL rel)=0;
		virtual void SetStoUPAxisFlip(BOOL rel)=0;
		virtual void Set_SetOrientation(BOOL rel)=0;
		virtual void Set_Reset_Orientation()=0;
		virtual void SetTargetAxis(int axis)=0;
		virtual void SetUpNodeAxis(int axis)=0;
		virtual void Set_StoUPAxis(int axis)=0;


	};

#define LOOKAT_ROT_PBLOCK_REF	0


//-------------------------------------------------------------
// Access to noise controller's parameters.
// All noise controllers are derived from this class
//

class INoiseControl : public StdControl {
	public:
		virtual void SetSeed(int seed)=0;
		virtual int GetSeed()=0;
		virtual void SetFrequency(float f)=0;
		virtual float GetFrequency()=0;
		virtual void SetFractal(BOOL f)=0;
		virtual BOOL GetFractal()=0;
		virtual void SetRoughness(float f)=0;
		virtual float GetRoughness()=0;
		virtual void SetRampIn(TimeValue in)=0;
		virtual TimeValue GetRampIn()=0;
		virtual void SetRampOut(TimeValue out)=0;
		virtual TimeValue GetRampOut()=0;
		virtual void SetPositiveOnly(int which,BOOL onOff)=0;
		virtual BOOL GetPositiveOnly(int which)=0;
		virtual Control *GetStrengthController()=0;
		virtual void SetStrengthController(Control *c)=0;
	};

//-------------------------------------------------------------
// Access to SurfPosition controller
//

class ISurfPosition : public Control {
	public:
		virtual void SetSurface(INode *node)=0;
		virtual int GetAlign()=0;
		virtual void SetAlign(int a)=0;
		virtual BOOL GetFlip()=0;
		virtual void SetFlip(BOOL f)=0;
	};

// Surface controller references
#define SURFCONT_U_REF			0
#define SURFCONT_V_REF			1
#define SURFCONT_SURFOBJ_REF	2


//-------------------------------------------------------------
// Access to the LinkCtrl
//

class ILinkCtrl;
#define LINK_CONSTRAINT_INTERFACE Interface_ID(0x32f03b37, 0x6700693a)
#define GetLinkConstInterface(cd) \
		(LinkConstTransform*)(cd)->GetInterface(LINK_CONSTRAINT_INTERFACE)
#define ADD_WORLD_LINK  PROPID_USER + 10

class ILinkCtrl : public Control, public FPMixinInterface  
{
	public:

		enum {	get_num_targets,		get_node,			set_node,
				get_frame_no,			set_frame_no,		add_target,		
				delete_target,			add_world, };
		

		// FUNCTION_PUBLISHING		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP

			FN_0(get_num_targets,	TYPE_INT,  GetNumTargets);
			FN_1(get_node,			TYPE_INODE, GetNode, TYPE_INDEX);
//			FN_2(set_node,			TYPE_BOOL, SetNode, TYPE_INODE, TYPE_INT);
			FN_1(get_frame_no,		TYPE_INT, GetFrameNumber, TYPE_INDEX);
			FN_2(set_frame_no,		TYPE_BOOL, SetFrameNumber, TYPE_INDEX, TYPE_INT);
			FN_2(add_target,		TYPE_BOOL, AddTarget, TYPE_INODE, TYPE_INT);
			FN_1(delete_target,		TYPE_BOOL, DeleteTarget, TYPE_INDEX);
			FN_1(add_world,			TYPE_INT, AddWorld, TYPE_INT);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************{
//	public:
		virtual int GetNumTargets()=0;
		virtual TimeValue GetLinkTime(int i)=0;
		virtual void SetLinkTime(int i,TimeValue t)=0;
		virtual void LinkTimeChanged()=0; // call after changing  Link times
		virtual void AddNewLink(INode *node,TimeValue t)=0;
		virtual BOOL DeleteTarget(int frameNo)=0;

		virtual int GetFrameNumber(int targetNumber)=0;
		virtual BOOL SetFrameNumber(int targetNumber, int frameNumber)=0;
//		virtual BOOL SetNode(INode *target, int targetNumber)=0;
		virtual BOOL AddTarget(INode *target, int frameNo)=0;
//		BOOL AddWorld(int frameNo)=0;
		virtual INode* GetNode(int targetNumber)=0;

	private:

		virtual int AddWorld(int frameNo)
		{
			if (frameNo==-99999) frameNo = GetCOREInterface()->GetTime()/GetTicksPerFrame();
			return SetProperty(ADD_WORLD_LINK, &frameNo);
		}

	};

// LinkCtrl references
#define LINKCTRL_CONTROL_REF		0	// the TM controller
#define LINKCTRL_FIRSTPARENT_REF	1	// parent nodes... refs 1-n
#define LINKCTRL_PBLOCK_REF			2   // added for Paramblock implementation


//-------------------------------------------------------------
// Access to the OLD LookatControl
//

class ILookatControl : public Control {
	public:
		virtual void SetFlip(BOOL f)=0;
		virtual BOOL GetFlip()=0;
		virtual void SetAxis(int a)=0;
		virtual int GetAxis()=0;
	};

// References for the lookat controller
#define LOOKAT_TARGET_REF	0
#define LOOKAT_POS_REF		1
#define LOOKAT_ROLL_REF		2
#define LOOKAT_SCL_REF		3

//-------------------------------------------------------------
//Access to the New Boolean Controller AG 11/08/01
//


class IBoolCntrl;
#define BOOL_CONTROL_INTERFACE Interface_ID(0x5d511b6, 0x52a302db)
#define GetIBoolCntrlInterface(cd) \
		(IBoolCntrl*)(cd)->GetInterface(BOOL_CONTROL_INTERFACE)
class IBoolCntrl: public StdControl, public IKeyControl {

};

// References for the boolean controller controller
#define BOOL_PBLOCK_REF		0



//-------------------------------------------------------------
// [dl | 22mar2002]
// Interface for exposing the new parameter "Use Target as Up Node" of the
// LookAtControl
#define ILOOKATCONTROL_EXTENSION Interface_ID(0x40ce4981, 0x3ea31c3b)

class ILookatControl_Extension : public FPMixinInterface {
public:

    virtual void SetTargetIsUpNode(bool val) = 0;
    virtual bool GetTargetIsUpNode() const = 0;

    // -- From BaseInterface
    virtual Interface_ID GetID() { return ILOOKATCONTROL_EXTENSION; }
};


//-------------------------------------------------------------
//Access to the List Controller
//
#define FLOATLIST_CONTROL_CLASS_ID		0x4b4b1000
#define POINT3LIST_CONTROL_CLASS_ID		0x4b4b1001
#define POSLIST_CONTROL_CLASS_ID		0x4b4b1002
#define ROTLIST_CONTROL_CLASS_ID		0x4b4b1003
#define SCALELIST_CONTROL_CLASS_ID		0x4b4b1004
#define DUMMY_CONTROL_CLASS_ID			0xeeefffff
#define MASTERLIST_CONTROL_CLASS_ID		0x4b4b1015
#define POINT4LIST_CONTROL_CLASS_ID		0x4b4b1005

class IListControl;

#define LIST_CONTROLLER_INTERFACE Interface_ID(0x444e7687, 0x722e6e36)

#define GetIListControlInterface(cd) \
		(IListControl*)(cd)->GetInterface(LIST_CONTROLLER_INTERFACE)

class IListControl : public Control, public FPMixinInterface {
	public:
		
		enum{ list_getNumItems, list_setActive, list_getActive, list_cutItem, list_pasteItem, 
			  list_deleteItem, list_count, list_setActive_prop, list_getActive_prop, 
			  list_getName, list_setName, };

		// FUNCTION_PUBLISHING		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
			FN_0		(list_getNumItems,	TYPE_INT,	GetListCount				);
			VFN_1		(list_setActive,				SetActive,		TYPE_INDEX	);
			FN_0		(list_getActive,	TYPE_INDEX,	GetActive					);
			VFN_1		(list_deleteItem,				DeleteItem,		TYPE_INDEX	);
			VFN_1		(list_cutItem,					CutItem,		TYPE_INDEX	);
			VFN_1		(list_pasteItem,				PasteItem,		TYPE_INDEX	);
			FN_1		(list_getName,	  TYPE_TSTR_BV,	GetName,		TYPE_INDEX	);
			VFN_2		(list_setName,					SetName,		TYPE_INDEX, TYPE_STRING );
			RO_PROP_FN	(list_count,					GetListCount,	TYPE_INT	); 
			PROP_FNS	(list_getActive_prop, GetActive, list_setActive_prop, SetActive, TYPE_INDEX	); 
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		virtual int	 GetListCount()=0;
		virtual void SetActive(int index)=0;
		virtual int	 GetActive()=0;
		virtual void DeleteItem(int index)=0;
		virtual void CutItem(int index)=0;
		virtual void PasteItem(int index)=0;
		virtual void SetName(int index, TSTR name)=0;
		virtual TSTR GetName(int index)=0;
};


//-------------------------------------------------------------
// Access to Spline IK Control modifier
//
class ISplineIKControl;
#define SPLINEIK_CONTROL_INTERFACE Interface_ID(0x7c93607a, 0x47d54f80)
#define GetISplineIKControlInterface(cd) \
		(ISplineIKControl*)(cd)->GetInterface(SPLINEIK_CONTROL_INTERFACE)
class ISplineIKControl: public Modifier, public FPMixinInterface {

	public:

		enum { SplineIKControl_params };

		enum {  
			sm_point_node_list,		sm_helpersize,	sm_helper_centermarker,		sm_helper_axistripod, 
			sm_helper_cross,		sm_helper_box,	sm_helper_screensize,		sm_helper_drawontop,
			sm_link_types,

		};

		enum{ getHelperCount, getKnotCount, link_allToRoot, link_allinHierarchy, link_none, create_hlpr};

		// FUNCTION_PUBLISHING		
		// Function Map for Function Publish System 
		//***********************************
		BEGIN_FUNCTION_MAP
			FN_0		(getHelperCount,		TYPE_INT,	GetHelperCount		);
			FN_0		(getKnotCount,			TYPE_INT,	GetKnotCount		);
			FN_0		(link_allToRoot,		TYPE_BOOL,	LinkToRoot			);
			FN_0		(link_allinHierarchy,	TYPE_BOOL,	LinkInHierarchy		);
			FN_0		(link_none,				TYPE_BOOL,	UnLink				);
			FN_1		(create_hlpr,			TYPE_BOOL,	CreateHelpers,		TYPE_INT);
		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		virtual int	 GetHelperCount()=0;
		virtual int	 GetKnotCount()=0;
		virtual BOOL LinkToRoot() = 0;
		virtual BOOL LinkInHierarchy() = 0;
		virtual BOOL UnLink() = 0;
		virtual BOOL CreateHelpers(int knotCt) = 0;

};

// References for the splineIK controller
#define SPLINEIKCONTROL_PBLOCK_REF		0



// The following two enums are transfered from helpers\pthelp.cpp by AG: 01/20/2002 
// in order to access the parameters for use in Spline IK Control modifier
// and the Spline IK Solver

// block IDs
enum { pointobj_params, };

// pointobj_params IDs

 enum { 
	pointobj_size, pointobj_centermarker, pointobj_axistripod, 
	pointobj_cross, pointobj_box, pointobj_screensize, pointobj_drawontop };



// The following two enums are transfered from modifiers\nspline.cpp  by AG: 01/20/2002 
// in order to access the parameters for use in Spline IK Solver

// block IDs
enum { nspline_params};

// nspline_params ID
enum { nspline_length};


//-------------------------------------------------------------
// Access to FFD modifiers
//

// Can either be casted to IFFDMod<Modifier> or IFFDMod<WSMObject> based on the ClassID
template <class T>
class IFFDMod : public T {
	public:
		virtual int			NumPts()=0;								// number of lattice control points 
		virtual int			NumPtConts()=0;							// number of CP's having controllers
		virtual Control*	GetPtCont(int i)=0;						// get i'th CP controller
		virtual void		SetPtCont(int i,Control *c)=0;			// set i'th CP controller
		virtual Point3		GetPt(int i)=0;							// get i'th CP
		virtual	void		SetPt(int i, Point3 p)=0;				// set i'th CP
		virtual	void		SetGridDim(IPoint3 d) { }				// set the lattice dimensions
		virtual	IPoint3		GetGridDim() { return IPoint3(0,0,0); }	// get the lattice dimensions
		virtual void		AnimateAll() { }						// assign controllers to all CP's				
		virtual	void		Conform() { }							// not valid for WSMObject's
		virtual void		SelectPt(int i, BOOL sel, BOOL clearAll=FALSE) { }
		virtual void		PlugControllers(TimeValue t,BOOL all)=0;
};

//-------------------------------------------------------------
// Access to mesh selections in editable mesh and edit mesh mod
//

#include "namesel.h"

// Selection levels:
#define IMESHSEL_OBJECT 0
#define IMESHSEL_VERTEX 1
#define IMESHSEL_FACE 2
#define IMESHSEL_EDGE 3

class IMeshSelect {
public:
	virtual DWORD GetSelLevel()=0;
	virtual void SetSelLevel(DWORD level)=0;
	virtual void LocalDataChanged()=0;
	virtual BOOL HasWeightedVertSel () { return FALSE; }
	virtual BOOL CanAssignWeightedVertSel () { return FALSE; }
};

class IMeshSelectData {
public:
	virtual BitArray GetVertSel()=0;
	virtual BitArray GetFaceSel()=0;
	virtual BitArray GetEdgeSel()=0;
	
	virtual void SetVertSel(BitArray &set, IMeshSelect *imod, TimeValue t)=0;
	virtual void SetFaceSel(BitArray &set, IMeshSelect *imod, TimeValue t)=0;
	virtual void SetEdgeSel(BitArray &set, IMeshSelect *imod, TimeValue t)=0;

	virtual GenericNamedSelSetList & GetNamedVertSelList ()=0;
	virtual GenericNamedSelSetList & GetNamedEdgeSelList ()=0;
	virtual GenericNamedSelSetList & GetNamedFaceSelList ()=0;

	virtual void GetWeightedVertSel (int nv, float *sel) {}
	virtual void SetWeightedVertSel (int nv, float *sel, IMeshSelect *imod, TimeValue t) {}
};

//-------------------------------------------------------------
// Access to spline selections and operations in SplineShape and EditSplineMod
//

// selection levels defined in splshape.h   

class ISplineSelect					// accessed via GetInterface(I_SPLINESELECT)
{
public:
	virtual DWORD GetSelLevel()=0;
	virtual void SetSelLevel(DWORD level)=0;
	virtual void LocalDataChanged()=0;
};

class ISplineSelectData				// accessed via GetInterface(I_SPLINESELECTDATA)
{
public:
	// access spline sub-object selections, current & named
	virtual BitArray GetVertSel()=0;
	virtual BitArray GetSegmentSel()=0;
	virtual BitArray GetSplineSel()=0;
	
	virtual void SetVertSel(BitArray &set, ISplineSelect *imod, TimeValue t)=0;
	virtual void SetSegmentSel(BitArray &set, ISplineSelect *imod, TimeValue t)=0;
	virtual void SetSplineSel(BitArray &set, ISplineSelect *imod, TimeValue t)=0;

	virtual GenericNamedSelSetList & GetNamedVertSelList ()=0;
	virtual GenericNamedSelSetList & GetNamedSegmentSelList ()=0;
	virtual GenericNamedSelSetList & GetNamedSplineSelList ()=0;
};

enum splineCommandMode { ScmCreateLine, ScmAttach, ScmInsert, ScmConnect, ScmRefine, ScmFillet, ScmChamfer, 
					     ScmBind, ScmRefineConnect, ScmOutline, ScmTrim, ScmExtend, ScmCrossInsert,
						 ScmBreak, ScmUnion, ScmSubtract, ScmCrossSection, ScmCopyTangent, ScmPasteTangent, };
enum splineButtonOp    { SopHide, SopUnhideAll, SopDelete, SopDetach, SopDivide, SopCycle,
						 SopUnbind, SopWeld, SopMakeFirst, SopAttachMultiple, SopExplode, SopReverse, 
						 SopClose, SopIntersect, SopMirrorHoriz, SopMirrorVert,
						 SopMirrorBoth, SopSelectByID, SopFuse, };
// LAM: added 9/3/00
enum splineUIParam {  };

class ISplineOps				// accessed via GetInterface(I_SPLINEOPS)
{
public:
	// start up interactive command mode, uses mode enum above
	virtual void StartCommandMode(splineCommandMode mode)=0;
	// perform button op, uses op enum above
	virtual void ButtonOp(splineButtonOp opcode)=0;
// LAM: added 9/3/00
	// UI controls access
	virtual void GetUIParam (splineUIParam uiCode, int & ret) { }
	virtual void SetUIParam (splineUIParam uiCode, int val) { }
	virtual void GetUIParam (splineUIParam uiCode, float & ret) { }
	virtual void SetUIParam (splineUIParam uiCode, float val) { }
};

//-------------------------------------------------------------
// Access to spline selections and operations in PatchObject and EditPatchMod
//

// selection levels defined in patchobj.h   

class IPatchSelect					// accessed via GetInterface(I_PATCHSELECT)
{
public:
	virtual DWORD GetSelLevel()=0;
	virtual void SetSelLevel(DWORD level)=0;
	virtual void LocalDataChanged()=0;
};

class IPatchSelectData				// accessed via GetInterface(I_PATCHSELECTDATA)
{
public:
	// access patch sub-object selections, current & named
	virtual BitArray GetVecSel()=0;
	virtual BitArray GetVertSel()=0;
	virtual BitArray GetEdgeSel()=0;
	virtual BitArray GetPatchSel()=0;
	
	virtual void SetVecSel(BitArray &set, IPatchSelect *imod, TimeValue t)=0;
	virtual void SetVertSel(BitArray &set, IPatchSelect *imod, TimeValue t)=0;
	virtual void SetEdgeSel(BitArray &set, IPatchSelect *imod, TimeValue t)=0;
	virtual void SetPatchSel(BitArray &set, IPatchSelect *imod, TimeValue t)=0;

	virtual GenericNamedSelSetList & GetNamedVecSelList ()=0;
	virtual GenericNamedSelSetList & GetNamedVertSelList ()=0;
	virtual GenericNamedSelSetList & GetNamedEdgeSelList ()=0;
	virtual GenericNamedSelSetList & GetNamedPatchSelList ()=0;
};

enum patchCommandMode { PcmAttach, PcmExtrude, PcmBevel, PcmBind, PcmCreate, PcmWeldTarget,
						PcmFlipNormal, PcmCopyTangent, PcmPasteTangent };
enum patchButtonOp    { PopUnbind, PopHide, PopUnhideAll, PopWeld, PopDelete, PopSubdivide,
						PopAddTri, PopAddQuad, PopDetach, PopSelectOpenEdges, PopBreak, 
						PopCreateShapeFromEdges, PopFlipNormal, PopUnifyNormal, PopSelectByID, 
						PopSelectBySG, PopClearAllSG, PopPatchSmooth, PopSelectionShrink, PopSelectionGrow,
						PopEdgeRingSel, PopEdgeLoopSel, PopShadedFaceToggle };
// LAM: added 9/3/00
enum patchUIParam {  };

class IPatchOps				// accessed via GetInterface(I_PATCHOPS)
{
public:
	// start up interactive command mode, uses mode enum above
	virtual void StartCommandMode(patchCommandMode mode)=0;
	// perform button op, uses op enum above
	virtual void ButtonOp(patchButtonOp opcode)=0;
// LAM: added 9/3/00
	// UI controls access
	virtual void GetUIParam (patchUIParam uiCode, int & ret) { }
	virtual void SetUIParam (patchUIParam uiCode, int val) { }
	virtual void GetUIParam (patchUIParam uiCode, float & ret) { }
	virtual void SetUIParam (patchUIParam uiCode, float val) { }
};


//----------------------------------------------------------------
// Access to the new Assign Vertex Color utility - MAB - 6/04/03

#define APPLYVC_UTIL_CLASS_ID	Class_ID(0x6e989195, 0x5dfb41b7)
#define IASSIGNVERTEXCOLORS_INTERFACE_ID Interface_ID(0x4f913fd8, 0x422a32af)

class IAssignVertexColors : public FPStaticInterface {
public:
	DECLARE_DESCRIPTOR( IAssignVertexColors );

	typedef enum {
		kLightingOnly = 0,      // Store lighting only
		kShadedLighting = 1,    // Store shaded color with lighting
		kShadedOnly = 2         // Store shaded color without lighting
	} LightingModel;

	// The options used when calculating the vertex colors
	typedef struct {
		int mapChannel;
		bool mixVertColors; // face colors is false, or mixed vertex colors if true
		bool castShadows;
		bool useMaps;
		bool useRadiosity;
		bool radiosityOnly;
		LightingModel lightingModel;
	} Options;

	//Assigns colors into the iVertexPaint object (if given), or applies an instanced modifier across the nodes
	virtual int		ApplyNodes( Tab<INode*>* nodes, ReferenceTarget* iVertexPaint=NULL ) = 0;
	virtual void	GetOptions( Options& options ) = 0;
	virtual void	SetOptions( Options& options ) = 0;
};

//----------------------------------------------------------------
// Access to the new Vertex Paint modifier in 3ds max 6 - MAB - 5/15/03

#define PAINTLAYERMOD_CLASS_ID	Class_ID(0x7ebb4645, 0x7be2044b)
#define IVERTEXPAINT_INTERFACE_ID Interface_ID(0x3e262ef9, 0x220e7190)

class IVertexPaint : public FPMixinInterface {
public:
	typedef struct FaceColor_tab { // Stores a color for each vertex of a triangle face.
		Color colors[3];
	} FaceColor;
	typedef Tab<Color*> VertColorTab;
	typedef Tab<FaceColor*> FaceColorTab;
	typedef IAssignVertexColors::Options Options;

	//Assumes colors were calculated on the same object as seen by the modifier (converted to a tri-mesh)
	virtual int		SetColors( INode* node, VertColorTab& vertColors ) = 0;
	virtual int		SetColors( INode* node, FaceColorTab& faceColors ) = 0;
	virtual void	GetOptions( Options& options ) = 0;
	virtual void	SetOptions( Options& options ) = 0;
};


// Interface for the TimeSlider
#define TIMESLIDER_INTERFACE Interface_ID(0x829e89e5, 0x878ef6e5)

class ITimeSlider : public FPStaticInterface {
public:
	virtual void	SetVisible(BOOL bVisible, BOOL bPersistent = TRUE) = 0;
	virtual BOOL	IsVisible() = 0;
};

// Interface for the StatusPanel
#define STATUSPANEL_INTERFACE Interface_ID(0x94357f0, 0x623e71c2)

class IStatusPanel : public FPStaticInterface {
public:
	virtual void	SetVisible(BOOL bVisible) = 0;
	virtual BOOL	IsVisible() = 0;
};

// Interface for the BMP I/O plug-in
#define BMPIO_INTERFACE Interface_ID(0x374f288f, 0x19e460d6)
// Valid types are, BMM_PALETTED, BMM_TRUE_24 and BMM_NO_TYPE (=BMM_TRUE_24) 
class IBitmapIO_Bmp : public FPStaticInterface {
public:
	virtual int		GetType() = 0;
	virtual void	SetType(int type) = 0;
	};

// Interface for the JPeg I/O plug-in
#define JPEGIO_INTERFACE Interface_ID(0x466c7964, 0x2db94ff2)
class IBitmapIO_Jpeg : public FPStaticInterface {
public:
	virtual int		GetQuality() = 0;
	virtual void	SetQuality(int quality) = 0;
	virtual int		GetSmoothing() = 0;
	virtual void	SetSmoothing(int smoothing) = 0;
	};

// Interface for the Png I/O plug-in
#define PNGIO_INTERFACE Interface_ID(0x1d7c41db, 0x328c1142)
class IBitmapIO_Png : public FPStaticInterface {
public:
	// Valid type are:
	// BMM_PALETTED
	// BMM_TRUE_24
	// BMM_TRUE_48
	// BMM_GRAY_8
	// BMM_GRAY_16
	virtual int		GetType() = 0;
	virtual void	SetType(int type) = 0;
	virtual BOOL	GetAlpha() = 0;
	virtual void	SetAlpha(BOOL alpha) = 0;
	virtual BOOL	GetInterlaced() = 0;
	virtual void	SetInterlaced(BOOL interlaced) = 0;
	};

// Interface for the Targa I/O plug-in
#define TGAIO_INTERFACE Interface_ID(0x21d673b7, 0x1d34198d)
class IBitmapIO_Tga : public FPStaticInterface {
public:
	// 16, 24, 32
	virtual int		GetColorDepth() = 0;
	virtual void	SetColorDepth(int bpp) = 0;
	virtual BOOL	GetCompressed() = 0;
	virtual void	SetCompressed(BOOL compressed) = 0;
	virtual BOOL	GetAlphaSplit() = 0;
	virtual void	SetAlphaSplit(BOOL alphaSplit) = 0;
	virtual BOOL	GetPreMultAlpha() = 0;
	virtual void	SetPreMultAlpha(BOOL preMult) = 0;
	};

// Interface for the RLA I/O plug-in
//Fetching the interface using either RLAIO_INTERFACE or RPFIO_INTERFACE returns an object
//of the same class.  But the object affects only the RLA or RPF settings accordingly.
//The "RPF-specific" methods have undefined behavior unless used with an RPFIO_INTERFACE object
#define RLAIO_INTERFACE Interface_ID(0x282c2f79, 0x68f7373d)
#define RPFIO_INTERFACE Interface_ID(0x25a87871, 0x2e265a49)
class IBitmapIO_RLA : public FPStaticInterface {
public:
	// 8, 16, 32
	virtual int		GetColorDepth() = 0;
	virtual void	SetColorDepth(int bpp) = 0;
	virtual BOOL	GetStoreAlpha() = 0;
	virtual void	SetStoreAlpha(BOOL storeAlpha) = 0;
	virtual BOOL	GetPremultAlpha() = 0;
	virtual void	SetPremultAlpha(BOOL preMult) = 0;

	virtual TSTR	GetDescription() = 0;
	virtual void	SetDescription(TSTR description) = 0;
	virtual TSTR	GetAuthor() = 0;
	virtual void	SetAuthor(TSTR author) = 0;

	virtual BOOL	GetZChannel() = 0;
	virtual void	SetZChannel(BOOL b) = 0;
	virtual BOOL	GetMtlIDChannel() = 0;
	virtual void	SetMtlIDChannel(BOOL b) = 0;
	virtual BOOL	GetNodeIDChannel() = 0;
	virtual void	SetNodeIDChannel(BOOL b) = 0;
	virtual BOOL	GetUVChannel() = 0;
	virtual void	SetUVChannel(BOOL b) = 0;
	virtual BOOL	GetNormalChannel() = 0;
	virtual void	SetNormalChannel(BOOL b) = 0;
	virtual BOOL	GetRealpixChannel() = 0;
	virtual void	SetRealpixChannel(BOOL b) = 0;
	virtual BOOL	GetCoverageChannel() = 0;
	virtual void	SetCoverageChannel(BOOL b) = 0;

	// RPF-specific methods
	virtual BOOL	GetNodeRenderIDChannel() = 0;
	virtual void	SetNodeRenderIDChannel(BOOL b) = 0;
	virtual BOOL	GetColorChannel() = 0;
	virtual void	SetColorChannel(BOOL b) = 0;
	virtual BOOL	GetTranspChannel() = 0;
	virtual void	SetTranspChannel(BOOL b) = 0;
	virtual BOOL	GetVelocChannel() = 0;
	virtual void	SetVelocChannel(BOOL b) = 0;
	virtual BOOL	GetWeightChannel() = 0;
	virtual void	SetWeightChannel(BOOL b) = 0;
	virtual BOOL	GetMaskChannel() = 0;
	virtual void	SetMaskChannel(BOOL b) = 0;
};

#endif //__ISTDPLUG__
