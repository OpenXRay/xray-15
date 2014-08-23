/**********************************************************************
 *<
	FILE: IConveriosnManager.h

	DESCRIPTION:	Tools to convert from one coordinate system to another

	CREATED BY:		Neil Hazzard

	HISTORY:		10|12|2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IConversionManager.h
\brief IGame Coordinate conversion Interfaces.
*/

#ifndef __ICONVERSIONMANAGER__H
#define __ICONVERSIONMANAGER__H
#pragma once

#include "Max.h"

#define IGAMEEXPORT __declspec( dllexport )

//!A User definable Coordintate System
/*! The developer can use this to define the Coordinate System that they are using.  Rotation specifies whether
it is a Right or Left handed system.  The Axis define which way the primary axis point.  This will mean that the 
data extracted is converted correctly, and the winding order is correct for Left and Right handed systems.
\n
In Max this could be defined as
\n
<pre>
UserCoord = {
1,	//Right Handed
1,	//X axis goes right
4,	//Y Axis goes in
2,	//Z Axis goes up.
1,	//U Tex axis is left
0,  //V Tex axis is Up
}
</pre>
	
*/
struct UserCoord{
	//! Handedness
	/*! 0 specifies Left Handed, 1 specifies Right Handed.
	*/
	int rotation;	

	//! The X axis 
	/*! It can be one of the following values 0 = left, 1 = right, 2 = Up, 3 = Down, 4 = in, 5 = out.
	*/
	int xAxis;
	//! The Y axis 
	/*! It can be one of the following values 0 = left, 1 = right, 2 = Up, 3 = Down, 4 = in, 5 = out.
	*/
	int yAxis;
	//! The Z axis 
	/*! It can be one of the following values 0 = left, 1 = right, 2 = Up, 3 = Down, 4 = in, 5 = out.
	*/
	int zAxis;

	//! The U Texture axis 
	/*! It can be one of the following values 0 = left, 1 = right
	*/
	int uAxis;

	//! The V Texture axis 
	/*! It can be one of the following values 0 = Up, 1 = down
	*/
	int vAxis;
	
};

//! A developer can use this class to define what Coord Systems IGame exports the data in
/*! IGame will convert data from the standard Max RH Z up system to any defined system.  At the moment direct support
for DirectX and OpenGL are provided.  This means that all Matrix and vertex data will have been converted ready to use 
on your target system
\nYou should set up the coordinate system straight after initialising IGame.  The default is to provide everything in Max
native formats.
*/

class IGameConversionManager
{
public:
	//! The supported Coordinate Systems
	/*! These are used to tell IGame how to format the data
	*/
	enum CoordSystem{
		IGAME_MAX,	/*!<Max RH Z up & +Y*/
		IGAME_D3D,	/*!<DirectX LH Y up & +Z*/
		IGAME_OGL,	/*!<OpenGL RH Y up & -Z*/
		IGAME_USER  /*!<User defined Coord System*/
	};

	//!Set IGame up for the Coordinate System you are wanting the data to be present in
	/*! The default system is the MAX system. 
	\param Coord  The Coordinate system to use	If Coord is IGAME_USER then you must set 
	this data via SetUserCoordSystem
	*/
	virtual void SetCoordSystem(CoordSystem Coord) =0;

	//!Set the User defined Coordinate system, if the CoordSystem has been defined as IGAME_USER
	/*! Allow a user definable Coordinate System.  See comments above.
	\param UC THe data to define the system
	*/
	virtual void SetUserCoordSystem(UserCoord UC) =0;

};


typedef float GRow[4];
//! this is very minimal, it is used purely to store 4x4 data with minimum functionality to support that.
class GMatrix
{
	float m[4][4];
	friend IGAMEEXPORT Point3  operator*(const Point3& V, const GMatrix& A);
public:

	Point4& operator[](int i) { return((Point4&)(*m[i]));}
	const Point4& operator[](int i) const { return((Point4&)(*m[i])); }

	//!Returns the Address of the GMatrix.  This allows direct access via the [] operator
	/*!
	\returns The address of the GMatrix
	*/
	GRow * GetAddr(){return (GRow *)(m);}

	//!Returns the Address of the GMatrix.  This allows direct access via the [] operator.  
	/*! This method is const aware.
	\returns The address of the GMatrix
	*/
	const GRow* GetAddr() const { return (GRow *)(m); }

	//!constructor from a Matrix3
	IGAMEEXPORT GMatrix(Matrix3);

	//! default constructor
	IGAMEEXPORT GMatrix();
	
	//! Sets all values to 0.0f
	void ResetMatrix();
	
	//! Set the Standard Identity Matrix
	void SetIdentity();

	/*! Access to the matrix column.
	\param i The number of the column to retrieve
	\returns A Point4 containing the column
	*/
	IGAMEEXPORT Point4 GetColumn(int i) const; 
	
	/*! Set the the matrix column.
	\param i The number of the column to set
	\param col Point4 containing the column to set
	*/
	IGAMEEXPORT void SetColumn(int i,  Point4 col); 

	/*! Access to the matrix row.
	\param i The number of the row to retrieve
	\returns A Point4 containing the row
	*/
	Point4 GetRow(int i) const { return (*this)[i]; }	

	/*! Set to the matrix row to the dsired data.
	\param i The number of the row to set
	\param row A Point4 containing the row
	*/
	IGAMEEXPORT void SetRow(int i, Point4 row);


	//!Provide matrix multiplication
	IGAMEEXPORT GMatrix operator*(const GMatrix&)const;

	//!Assignment operator from a Matrix3
	IGAMEEXPORT GMatrix& operator=(const Matrix3&);
	
	//!Extract a Matrix3 from the GMatrix
	/*!This is for backward compatibility.  This is only of use if you use Max as a coordinate system, other wise
	standard max allegebra might not be correct for your format.
	\returns A max Matrix3 form of the GMatrix
	*/
	IGAMEEXPORT Matrix3 ExtractMatrix3()const ;
	
};

//! Multiplies a GMatrix with a Point3
IGAMEEXPORT Point3 operator*(const Point3& V, const GMatrix& A);

//!External access to the conversion manager
IGAMEEXPORT IGameConversionManager * GetConversionManager();

#endif

