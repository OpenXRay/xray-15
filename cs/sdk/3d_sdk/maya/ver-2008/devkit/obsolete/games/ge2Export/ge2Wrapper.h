#ifndef _ge2Wrapper_h
#define _ge2Wrapper_h

/*==============================================*/
/* Header file for the ge2.0 wrapper class		*/
/*==============================================*/
#define MAYA_SRC

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#ifdef AW_NEW_IOSTREAMS
#include <iostream>
#else
#include <iostream.h>
#endif

#include <MDt.h>
#include <MDtExt.h>

#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MVector.h>
#include <maya/MFnTransform.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnMesh.h>
#include <maya/MMatrix.h>
#include <maya/MFnLight.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnSpotLight.h>
#include <maya/MFnCamera.h>
#ifndef MAYA1X
#include <maya/MAnimControl.h>
#endif
#include <maya/MTime.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h> 
#include <maya/MItDag.h>

#ifndef MAYA1X
#include <maya/MFnIkJoint.h>
#endif

#include <maya/MComputation.h>

#include <maya/ilib.h>

#include "iffwriter.h"
#include "ge2Util.h"

#define PI 3.1415926535897932

#define GE_MATRIX_SIZE				4
#define GE_NUM_SPACES_PER_CHILD		3

#define GE2_MAX_INDICES_IN_LINE		15

// Return values for writeXXX() methods -- whether or not there was an error or data
#define GE_WRITE_FILE_ERROR			-1
#define GE_WRITE_NO_DATA			0
#define GE_WRITE_OK					1

#define	GE_MATERIAL_STATS_NONE		-2
#define GE_MATERIAL_STATS_INVALID	-1

#define GE_NUM_LIGHT_TYPES			4 // Defined in MDt.h

#define GE_OUTBUFFER_SIZE			8192

#ifdef WIN32
#ifndef MAXPATHLEN
#define MAXPATHLEN					512
#endif
#endif

class ge2Wrapper {

public:

	class MShapeInfo {
	public:
		class MFrameInfo {
		public:
			int					numVertices;
			MFloatVectorArray	vertexPosition;
			MFloatVector		translation;
			MFloatVector		rotation;
			MFloatVector		scale;

			MFrameInfo();
			~MFrameInfo();
		};

		MFrameInfo	*frameInfo;
		int			index;
		MIntArray	keyFrames;

		MShapeInfo();
		~MShapeInfo();
		void		allocFrameInfo( int numFrames );

		bool		isKeyFrame( int frame );
		void		addKeyFrame( int frame );
		void		addKeyFrames( MIntArray& keyArray );
		int			getFirstKeyBetween( int start, int end );
		int			getLastKeyBetween( int start, int end );

		void		fill( int frame, bool transforms, bool vertices );
	};

	class MShader {
	public:
		enum GEDomainBool {
			kDomainBoolInvalid = -1,
			kDomainBoolFalse,
			kDomainBoolTrue
		};

		enum GEDomainFaceControl {
			kDomainFaceInvalid = -1,
			kDomainFaceFront,
			kDomainFaceBack,
			kDomainFaceBoth
		};

		enum GEDomainNormalSrc {
			kDomainNormalInvalid = -1,
			kDomainNormalFace,
			kDomainNormalVertex
		};

		class MTextureMap {
		public:
			MString		name;
			int			width,
						height,
						channels,
						bitsPerChannel;
			MTextureMap();
			~MTextureMap();
		};
		

		// To add a field to one of the domains, simply add the appropriate type
		// and variable to whichever domain-class you need. You should initialize
		// it to an invalid type in the constructor for the class, and modifiy the
		// MShader::fill and MShader::output methods to utilize this new variable
		// If it's a new type altogether, you'll have to add it to the template as well.

		// N64		
		class MN64 {
		public:
			GEDomainBool		useTextureMap;
			GEDomainFaceControl faceControl;
			GEDomainBool		omitFace;
			GEDomainNormalSrc	normalSource;
			GEDomainBool		preShade;
/*
texture-bit-depth,
face-alpha
*/
			GEDomainBool		useLightOption;
/*
texture-type,
texture-size
*/
			GEDomainBool		blendColor;
			GEDomainBool		useZBuffer;
/*
antialias-type,
use-texture-edge,
render-mode-opa,
render-mode-surf
*/

			GEDomainBool		useFog;
			GEDomainBool		useReflectionMapping;

			MN64();
			~MN64();
		};

		class MPSX {
		public:
			GEDomainBool		useTextureMap;
			GEDomainBool		omitFace;			
			GEDomainBool		lightCalculation;
			GEDomainFaceControl faceControl;
			GEDomainBool		abe;
			int					abr;
			GEDomainNormalSrc	normalSource;
			GEDomainBool		preShade;
			GEDomainBool		subdiv;
			GEDomainBool		activeSubdiv;
			GEDomainBool		clip;
			GEDomainBool		useMipmap;

			MPSX();
			~MPSX();
		};

		class MGL {
		public:
			GEDomainNormalSrc	normalSource;
			GEDomainBool		colorByVertex;

			MGL();
			~MGL();
		};

		// MDt related
		MString			materialName;
		int				materialID;

		// base domain
		MTextureMap*	map;
		
		MN64*			N64Domain;
		MPSX*			PSXDomain;
		MGL*			GLDomain;

		MShader();
		~MShader();

		void		getFaceControl( GEDomainFaceControl & faceControl );

		void		fill( int matID, ge2Wrapper &caller );

		void		outputBoolIfValid( MTextFileWriter &writer, char* str, GEDomainBool boolVal );
		void		output( MTextFileWriter &writer );		

		void		writeTexture( MTextFileWriter& writer, const char* name, 
							int width, int height, int channels, int bits );

	}; // end of MShader


	enum GESelType {
		kSelInvalid = 1,
		kSelAll,
		kSelPicked,
		kSelActive
	};
	
	// Start at 0 here so that the rest match up with what's output from
	// optionsBox
	enum GEHrcMode {
		kHrcInvalid = 0,
		kHrcWorld,
		kHrcFlat,
		kHrcFull
	};

	enum GEFaceType {
		kFaceInvalid = -2,
		kFaceNone,
		kFaceBoth,
		kFaceFront,
		kFaceBack
	};

	enum GEVertexDisplacementType {
		kVDInvalid = -1,
		kVDAbsolute,
		kVDRelative
	};

	enum GEDiffType {
		kDiffTranslate,
		kDiffRotate,
		kDiffScale,
		kDiffVertex
	};

public:

	ge2Wrapper();
	virtual	~ge2Wrapper();

	// set member variables so export can happen even if mel doesn't set them
	void			setDefaults();

	void			initScene();
	void			loadSceneNew();
	void			loadScene();
	int				writeScene();
	void			killScene();	

	MString			getBaseFileName();
	void			setBaseFileName( const MString& fileName );
	MString			getPathName();
	void			setPathName( const MString& path );

	void			ntify( char* str );

protected:
	float			translateTolAvg();
	float			translateTolSingle( int numElements );
	float			rotateTolAvg();
	float			rotateTolSingle( int numElements );
	float			scaleTolAvg();
	float			scaleTolSingle( int numElements );
	float			vertexTolAvg();
	float			vertexTolSingle( int numElements );

	int				frameConvertLocalToDt( int localFrame );
	int				frameConvertDtToLocal( int dtFrame );
	int				getNextKeyToSample( int lastKey );

	bool			areDifferent( const MFloatVectorArray& first, 
						const MFloatVectorArray& second, 
						GEDiffType diffType );

	void			collectMaterials();
	void			collectGeometry();
	void			setAllKeyFrames();
	void			setAnimCurveKeyFrames();
	void			setSampledKeyFrames();	

	int				getCameraForOutput();

	void			outputIncludeFile( MTextFileWriter &fileWriter, char * ext );
	void			outputHeader( MTextFileWriter &fileWriter );

	void			outputLGTTemplates( MTextFileWriter& fileWriter );
	void			outputCAMTemplates( MTextFileWriter& fileWriter );
	void			outputGOFTemplates( MTextFileWriter& fileWriter );
	void			outputGMFTemplates( MTextFileWriter& fileWriter );
	void			outputGAFTemplates( MTextFileWriter& fileWriter );
	void			outputGSFTemplates( MTextFileWriter& fileWriter );

	void			outputDisplacement( int shapeID );

	int				outputLightsToFile();
	int				outputCameraToFile( int cameraIndex );
	int				outputBodyToFile( const int shapeIdx );
	int				outputGObjectToFile( const int shapeIdx, bool topLevelCall );
//	int				outputBoneToFile( MFnIkJoint &joint );
	
	int				writeGAF(); // write animation file
	int				writeGOF(); // write object (geometry) file
	int				writeGMF(); // write material (shader) file
	int				writeLGT(); // write light file
	int 			writeCAM(); // write camera file
	int				writeGSF(); // write skeleton file

public:
	MString					pluginVersion;
	MString					geVersion;

	// User-specified flags (public so that geTranslator can access
	// without declaring it a friend or writing this many accessor methods)

	bool					useDomainGL;
	bool					useDomainPSX;
	bool					useDomainN64;
	bool					useDomainCustom;

	bool					enableAnim;
	bool					animVertices;
	GEVertexDisplacementType vertexDisplacement;
	bool					animTransforms;
	bool					animShaders;
	bool					animLights;
	bool					animCamera;
	bool					keyCurves;
	bool					keySample;
	int						sampleRate;
	float					sampleTolerance;

	bool					outputNormals;
	bool					oppositeNormals;
	bool					outputTextures;
	bool					outputLights;
	bool					outputCamera;
	bool					outputGeometry;
	bool					outputJoints;
	bool					reverseWinding;

	bool					useOriginalFileTextures;

	bool					verboseGeom;
	bool					verboseLgt;
	bool					verboseCam;
				
	int						saveFrame;
	int						frameStart;
	int						frameEnd;
	int						frameStep;
	int						numFrames;

	GEHrcMode				hrcMode;
	GESelType				selType;
	int						sampleTextures;
	int						evalTextures;
	int						xTexRes;
	int						yTexRes;
	int						xMaxTexRes;
	int						yMaxTexRes;
	MString					texType;
	int						precision;
	bool					useTabs;
	bool					writeComments;
	MString					userScript;
	bool					scriptAppendFileName;

private:

//	geAbstractNode	*root;

	MShapeInfo		*shapeInfo;
	int				numShapes;

	MShader			*shaders;
	int				numShaders;

	// different extensions will get added to this:
	MString			pathName;			// directory where files are going
	char			charPathName[512];	// just so i can see, for now
	MString			baseFileName;		// fileName (with no extensions) that user's calling this -- no directories
	char			charBaseFileName[512];	// just so i can see, for now

	MTextFileWriter	writer;

	MStringArray	processedDtShape;
	MStringArray	bodyName;
	
	static MString	trueString;
	static MString	falseString;
	static char		gmfTemplate[];
	static char		gofTemplate[];
	static char		gafTemplate[];
	static char		gsfTemplate[];

	// Default light settings
	static float	defaultAmbientLightBrightness;
	static float	defaultAmbientLightColor[3];
	static float	defaultInfiniteLightBrightness;
	static float	defaultInfiniteLightColor[3];
	static float	defaultInfiniteLightDirection[3];
	static float	defaultPointLightBrightness;
	static float	defaultPointLightColor[3];
	static float	defaultPointLightPosition[3];
	static float	defaultPointLightRadius;
	static float	defaultSpotLightBrightness;
	static float	defaultSpotLightColor[3];
	static float	defaultSpotLightPosition[3];
	static float	defaultSpotLightAimPoint[3];
	static float	defaultSpotLightRadius;
	static float	defaultSpotLightFalloffAngle;
	static float	defaultSpotLightAspectRatio[2];
	// Default camera settings
	static float	defaultCameraViewAngle;
	static float	defaultCameraHitherDistance;
	static float	defaultCameraYonDistance;
	static float	defaultCameraLocation[3];
	static float	defaultCameraAimPoint[3];
	static float	defaultCameraLookupVector[3];
};

inline MString ge2Wrapper::getPathName()
{
	return pathName;
}

inline MString ge2Wrapper::getBaseFileName()
{
	return baseFileName;
}

#endif // _ge2Wrapper_h
