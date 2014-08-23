/*==============================================*/
/* Header file for the VRML2.0 translator	*/
/*==============================================*/

#ifndef _vrml2Export_h
#define _vrml2Export_h

#define MAYA_SRC

#if defined (_WIN32)
#define Boolean	bool
#elif defined (OSMac_)
#	if defined(OSMac_CFM_)
#		include <types.h>
#	else
#		include <sys/types.h>
#	endif
#else
#include <X11/Intrinsic.h>
#endif

#include <maya/MIntArray.h>

typedef float	VRMLType;

enum { VRHRC_WORLD=0,VRHRC_FLAT,VRHRC_FULL };
enum { VRSEL_ALL=0,VRSEL_PICKED,VRSEL_ACTIVE };
enum { VRVIEWER_NOT_DEFINED=0,VRVIEWER_NONE,
							VRVIEWER_WALK,VRVIEWER_EXAMINE,VRVIEWER_FLY };
enum { VRTEXTURE_INLINE=0,VRTEXTURE_SGI_IMAGEFILE };

#define	VR_OUTBUFFER_SIZE	8192
#define	VR_MAX_LINE_SIZE	 512


enum { VRROT_NO_EULER=0,VRROT_XYZ,VRROT_XZY,
						VRROT_YXZ,VRROT_YZX,VRROT_ZXY,VRROT_ZYX };

typedef struct
{
 VRMLType	Translate[3];
 VRMLType	Scale[3];
 VRMLType	RotationAngles[3];	// Euler angles
 VRMLType	Orientation[4];		// Quaternion
 VRMLType	RotMatrix[3][3];	// Rotation matrix
} DECOMP_MAT;


typedef struct
{
 VRMLType	X,Y,Z;
} VRVertexRec;

typedef struct
{
 VRMLType	Translate[3];
 VRMLType	Scale[3];
 VRMLType	Orientation[4];		// Quaternion
} VRTransformRec;


typedef struct
{
 VRMLType	Location[3];
 VRMLType	ColorR,ColorG,ColorB;
 VRMLType	CutOffAngle;
 VRMLType	Intensity;
 VRMLType	Direction[3];		// Direction vector
 Boolean	On;
} VRLightValRec;


typedef struct
{
 int		Type;
 VRLightValRec*	Animation;
 Boolean	LocationAnimated;
 Boolean	ColorAnimated;
 Boolean	IntensityAnimated;
 Boolean	CutOffAngleAnimated;
 Boolean	DirectionAnimated;
 Boolean	OnAnimated;
 MIntArray  *trsAnimKeyFrames;
 MIntArray  *AnimKeyFrames;
} VRLightRec;

typedef struct
{
 VRMLType	Location[3];
 VRMLType	Orientation[4];		// Direction vector
 Boolean	On;
} VRCameraValRec;


typedef struct
{
 int		Type;
 VRCameraValRec*	Animation;
 Boolean	LocationAnimated;
 Boolean	DirectionAnimated;
 Boolean	OnAnimated;
 MIntArray  *trsAnimKeyFrames;
 MIntArray  *AnimKeyFrames;
} VRCameraRec;

typedef struct
{
 float	AmbientR,AmbientG,AmbientB;
 float	DiffuseR,DiffuseG,DiffuseB;
 float	SpecularR,SpecularG,SpecularB;
 float	EmissiveR,EmissiveG,EmissiveB;
 float	Shininess;
 float	Transparency;
} VRMaterialValRec;


typedef struct
{
 VRMaterialValRec*	Animation;
 Boolean		AmbientColAnimated;
 Boolean		DiffuseColAnimated;
 Boolean		SpecularColAnimated;
 Boolean		EmissiveColAnimated;
 Boolean		ShininessAnimated;
 Boolean		TransparencyAnimated;
 MIntArray      *AnimKeyFrames;
} VRMaterialRec;



typedef struct VRShapeRec
{
 char			Name[256];
 int			ID;
 int			NumOfAnimatedVertices;
 int*			AnimatedVertices;
 VRVertexRec*		VertexAnimation;
 VRTransformRec*	TransformAnimation;
 struct VRShapeRec*	Parent;
 struct VRShapeRec*	Child;
 struct VRShapeRec*	RightSibling;
 MIntArray			*trsAnimKeyFrames;
 MIntArray			*vtxAnimKeyFrames;
 Boolean		TransAnimated,OriAnimated,ScaleAnimated,ShapeAnimated;
 Boolean		Anchor,Collision,Billboard,Added,MultipleChld;
} VRShapeRec;

#define	MAXLENGTHOFBLINDDATA	32
#define	MAXNUMBEROFBLINDDATA	4

typedef struct
{
int	StartFrame;
int	EndFrame;
char	BlindData[MAXNUMBEROFBLINDDATA][MAXLENGTHOFBLINDDATA+1];
int*	LinkData;
} MCySnippet;

typedef struct
{
int	StartFrame;
int	EndFrame;
int	NextSnippet;
int	NextSnippetEntryFrame;
char	BlindData[MAXNUMBEROFBLINDDATA][MAXLENGTHOFBLINDDATA+1];
} MCyArc;

typedef struct
{
int	StartEntryOfSnippets;	// in the Main-Snippet table (MCySnippet**)
int	EndEntryOfSnippets;
int	StartEntryOfArcs;	// in the Main-Arc table (MCyArc**)
int	EndEntryOfArcs;
char	BlindData[MAXNUMBEROFBLINDDATA][MAXLENGTHOFBLINDDATA+1];
} MCyMainTable;


#define VRDegToRad	57.295779513082320877

#define	M_SetIndentStr(MIndentStr,MCurrentIndent,MSetTo)	MIndentStr[MCurrentIndent]=' ',MCurrentIndent=MSetTo,MIndentStr[MCurrentIndent]='\0'
#define	M_IncIndentStr(MIndentStr,MCurrentIndent)	MIndentStr[MCurrentIndent]=' ',MCurrentIndent++,MIndentStr[MCurrentIndent]='\0'
#define	M_DecIndentStr(MIndentStr,MCurrentIndent)	MIndentStr[MCurrentIndent]=' ',MCurrentIndent--,MIndentStr[MCurrentIndent]='\0'

#define	M_AddLine(MBuffer,MBufferPtr,MIndentStr,MStr)	MBufferPtr+=VR_StrCpyL(MBuffer+MBufferPtr,MIndentStr),MBufferPtr+=VR_StrCpyL(MBuffer+MBufferPtr,MStr)

#define M_QuaternionToVRML(q,vr) \
 (vr)=(q);	\
 LF=(vr)[QW];(vr)[QW]=acos(LF)*2.0;	\
 LF=sqrt((vr)[QX]*(vr)[QX]+(vr)[QY]*(vr)[QY]+(vr)[QZ]*(vr)[QZ]);	\
 if(LF>0.0) { LF=1.0/LF;(vr)[QX]*=LF;(vr)[QY]*=LF;(vr)[QZ]*=LF; }	\
 else { (vr)[QX]=0.0;(vr)[QY]=0.0;(vr)[QZ]=0.0;(vr)[QZ]=0.0; }

#define	QX	0
#define	QY	1
#define	QZ	2
#define	QW	3


/*
//
// Vrml2 Tag definitions
//
// Originally defined with Blind Data for PowerAnimator.
// For Maya using dynamic Attributes.  Most of the concepts are the same.
//
*/

enum { VRPRIM_NONE=0,VRPRIM_BOX,VRPRIM_CONE,
					VRPRIM_CYLINDER,VRPRIM_SPHERE,VRPRIM_ELEVATIONGRID };
enum { VRCOLL_NONE=0,VRCOLL_OBJECT,VRCOLL_BBOX,VRCOLL_BSPHERE };
enum { VRBB_NONE=0,VRBB_XROT,VRBB_YROT,VRBB_SCRALIGN };
enum { VRSENS_NONE=0,VRSENS_CYLINDER,VRSENS_SPHERE,
			VRSENS_PLANE,VRSENS_PROXIMITY,VRSENS_TOUCH,VRSENS_VISIBILITY };

/*
//
//	These blinddata IDs are not used in the Maya version
//  (keep for version comparisions
//
*/

#define VRML2_BLINDDATA_ID      6020
#define VRML2_INLINE_BLINDDATA_ID   6040
#define VRML_LINK_BLINDDATA_ID  20000

typedef struct
{
 int    PrimitiveType;
 int    CollisionType;
 int    BillboardType;
 int    SensorType;
 float  CylDiskAngle,CylMinAngle,CylMaxAngle,CylOffset;
 char   Link[1024];
 char   Description[1024];
} VRML2BlindDataRec;

typedef struct
{
 char   Link[1024];
} VRML2InlineBlindDataRec;

typedef struct
{
 char   name[1024];
 char   description[1024];
 int    map;
} VRMLLinkRec;

#endif // _vrml2Export_h
