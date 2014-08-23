#ifndef _ge2Util_h
#define _ge2Util_h

/*======================================================*/
/* Header file for ge2.0 util functions and classes		*/
/*======================================================*/
#define MAYA_SRC

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifdef AW_NEW_IOSTREAMS
#include <iostream>
#else
#include <iostream.h>
#endif
#include <math.h>

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
#include <maya/MFnSpotLight.h>
#include <maya/MFnCamera.h>
#include <maya/MAnimControl.h>
#include <maya/MTime.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h> 
#include <maya/MItDag.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFileObject.h>

#define PI 3.1415926535897932

#define TEXT_FILE_WRITER_DEFAULT_NUM_TAB_STOPS			0
#define TEXT_FILE_WRITER_DEFAULT_PRECISION				5
#define TEXT_FILE_WRITER_MAX_FORMAT_LENGTH				512
#define	TEXT_FILE_WRITER_MAX_SINGLE_CALL_LENGTH			512
#define TEXT_FILE_WRITER_MAX_TAB_LENGTH					512

int cameraGetUpDir( int cameraID, float& xUp, float& yUp, float& zUp );
int cameraGetWidthAngle( int cameraID, float& viewAngle );
int lightGetUseDecayRegions( int lightID, bool& useDecay );
int lightGetName( int lightID, char* name );
int lightGetDistanceToCenter( int lightID, float& distance );
int shapeGetShapeName( int shapeID, char * name );
int shapeGetNumEdges( int shapeID, int& numEdges );
int shapeGetEdge( int shapeID, int edgeID, long2& edge );
bool fileExists( MString fullPath );
bool addElement( MIntArray& intArray, int newElem );
bool getBoundingBox( int shapeIdx, DtVec3f *boundingBox );
MIntArray shapeGetTRSAnimKeys( int shapeID );
MIntArray shapeGetVertAnimKeys( int shapeID );

// To determine some characteristics of shapes that ge tracks in materials
int getFaceType( int shapeID );
int isFlatShaded( int shapeID );
int isColoredPerVertex( int shapeID );
int isVertexNormalPerFace( int shapeID, int groupID );
int isPreLighted( int shapeID );

class MTextFileWriter {
// class methods:
public:
	MTextFileWriter();
	~MTextFileWriter();

	void			clearFile();
	void			setFile( const MString newFilePath );	
	void			writeFile();

	void			setPrecision( const int precision );
	void			addTab();
	void			removeTab();

	void			print( const char* format, ... );

	void			setUseTabs( bool newUseTabs );

protected:
	void			reset();
	void			resetFileAndBuffer();
	void			printTabs( char* storage );

// class data:
private:
	MStringArray	buffer;
	MString			filePath;

	int				numTabStops;
	int				precision;

	bool			useTabs;
};

inline void MTextFileWriter::setPrecision( const int prec )
{
	precision = prec;
}

inline void MTextFileWriter::addTab()
{
	numTabStops++;
}

inline void MTextFileWriter::removeTab()
{
	numTabStops--;
}

inline void MTextFileWriter::setUseTabs( bool newUseTabs )
{
	useTabs = newUseTabs;
}

#endif // _ge2Util_h
