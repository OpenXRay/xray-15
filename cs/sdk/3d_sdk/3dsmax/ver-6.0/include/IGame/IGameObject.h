/**********************************************************************
 *<
	FILE: IGameObject.h

	DESCRIPTION: Object interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGameObject.h
\brief IGame supported Object Interfaces.
*/
#ifndef __IGAMEOBJECT__H
#define __IGAMEOBJECT__H

#pragma once

#include "Max.h"
#include "IGameProperty.h"

#define IGAMEEXPORT __declspec( dllexport )


class IGameNode;
class IGameModifier;
class IGameSkin;

//!Simple wrapper for max objects
/*! This is heart of the interaction with Max objects.  Max objects are converted to IGame objects to provide
simpler interfaces.  Specific support is given to Meshes, Splines, Helper (including bones) and Cameras.  Any object 
that is not supported by IGame directly is supported as a generic object, so that properties can be extracted easily.  At the present time IGame only 
supports Geom Objects that can be converted to a Tri Object. 
\n
A note about XRef items.  IGame will search out XRef Objects and performs the opertations on the original Object, so any
modifiers will be extracted and the original object data such as mesh will be evaluated.  Currently IGame does not support
the actual XRefObject in the scene, so external file names and proxies are not available.
\n
NB: Some of the interfaces returned by IGameObject require that the nodes be parsed by IGame first.
\sa IGameNode, IGameMesh, IGameLight, IGameCamera, IGameSupportObject
\sa IExportEntity
*/
class IGameObject : public IExportEntity
{
	Tab <IGameModifier *> gameModTab;
	Object * gameObject;

protected:
	INode * gameNode;
	void SetNode (INode * n);
public:
	//!default constructor
	IGameObject();
	
	//! Various IGame Object types
	/*! These are the objects supported by IGame
	*/
	enum ObjectTypes{
		IGAME_UNKNOWN,	/*!< An unknown object*/
		IGAME_LIGHT,	/*!< A Light Object */
		IGAME_MESH,		/*!< A Mesh Object */
		IGAME_SPLINE,	/*!< A Spline Object */
		IGAME_CAMERA,	/*!< A Camera Object */
		IGAME_HELPER,	/*!< A Helper Object */
		IGAME_BONE,		/*!< A Bone Object */
		IGAME_IKCHAIN	/*!< An IK Chain Object*/	
	};

	//! Various Max Object types
	/*! These are Max object - Developers can use this to cast the Object pointer to one of the relevent Max classes
	*/
	enum MaxType{
		IGAME_MAX_UNKNOWN,	/*!< An unknown object*/
		IGAME_MAX_GEOM,		/*!< A Geom Object*/
		IGAME_MAX_SHAPE,	/*!< A Shape Object*/
		IGAME_MAX_LIGHT,	/*!< A Light Object*/
		IGAME_MAX_CAMERA,	/*!< A Camera Object*/
		IGAME_MAX_BONE,		/*!< A Bone Object*/
		IGAME_MAX_HELPER,	/*!< A Helper Object*/
	};

	//! The bounding box
	/*! The bounding box of the object
	\param bb The box3 to receive the value
	*/
	IGAMEEXPORT void GetBoundingBox(Box3 & bb);

	//!Is this object renderable
	/*!
	\return TRUE if it is renderable
	*/
	IGAMEEXPORT bool IsRenderable();

	/*! Return the Type of IGameObject it represents
	\return The object type This is value from the ObjectTypes Enum
	*/
	virtual ObjectTypes GetIGameType() =0;
	
	//IGameObject();
	/*! This provide access to the actual max object, if the developer wants further direct access
	\return a pointer to a max Object class
	*/
	IGAMEEXPORT Object * GetMaxObject();

	/*! This lets the developer know what type of object - Light, Geom, Helper etc...
	This is different to the IGame types, as these may group more than one type of max object together
	the information here can be used to cast Object returned from GetMaxObject to the appropriate class.
	\return The type of max object.  THis is defined as the MaxObject enum
	*/
	IGAMEEXPORT MaxType GetMaxType() ;
	
	/*! The number of modifiers active on this object
	\return The number of modifiers
	*/
	IGAMEEXPORT int GetNumModifiers();

	/*! Get a pointer to the IGame representation of the modifier
	\param index The index of the modifier to return
	\return A pointer to the IGameModifier
	*/
	IGAMEEXPORT IGameModifier * GetIGameModifier(int index);

	/*! The IGameObject constructor
	\param *node  The max node to initialise
	*/
	IGAMEEXPORT IGameObject(INode  * node);

	//! Does this object cast shdows
	/*!
	\return true if is casts shadows
	*/
	IGAMEEXPORT bool CastShadows();
	
	/*! Is this object skinned with either Physique of Skin
	\return TRUE if skinned
	*/
	IGAMEEXPORT bool IsObjectSkinned();

	/*! Provide access to the skin interface
	\return a pointer to IGameSkin or NULL if not present
	*/
	IGAMEEXPORT IGameSkin * GetIGameSkin();

	/*! Is this an XRef Object.  This will in future allow access to additional XRef data, but for now you just know
	what you are getting
	\returns TRUE if it is an XRef Object
	*/
	IGAMEEXPORT bool IsObjectXRef();

	/*! Get the ObjectTM.  This is the matric needed to calculated world space
	\return A matrix containing the Object TM.  Use this to calculate world space.
	*/
	IGAMEEXPORT GMatrix GetIGameObjectTM();

	//! This will tell the object to extract the max data into IGame data
	/*! Some extraction processes are time and memory consuming.  This method allows the developer to specify when 
	they want the data to be converted - this prevents any unwanted data being converted.  This is important for 
	IGameMesh class - if you are just after parameter data, you don't want the whole vertex array being sorted.  Calling
	this tells the object that you want the data converted.
	\returns It will return FALSE when data has not been converted, this object should not be exported.  Usually this is 
	due to a standin or in the case of a GeomObject, it can be converted to a Tri Object.
	*/
	virtual bool InitializeData(){return false;}

	virtual ~IGameObject() = 0;
};

//!Simple wrapper for light objects
/*! An IGame Wrapper around Max's lights.  This is a generic interface for all the lights
*/
class IGameLight : public IGameObject  {
public:
	//! Various Light types used by Max
	enum LightType{
		IGAME_OMNI,		/*!< Omnidirectional Light*/
		IGAME_TSPOT,	/*!< Targeted Spot Light*/
		IGAME_DIR,		/*!< Directional Light*/
		IGAME_FSPOT,	/*!< Free spot Light*/
		IGAME_TDIR,		/*!< Targeted Directional Light*/
	};

	//! Get the Light Color Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightColor()=0;

	//! Get the Light Multiplier Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightMultiplier()=0;

	//! Get the Light Attenuation End Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightAttenEnd()=0;

	//! Get the Light Attenuation Start Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightAttenStart()=0;

	//! Get the Light Falloff Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightFallOff()=0;

	//! Get the Light Hot spot Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightHotSpot()=0;

	//! Get the Aspect Ratio Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightAspectRatio()=0;
	
	//! Get the Light type as defined in the UI
	/*!
	\return the Light Type - This can be OMNI_LIGHT etc..
	*/
	virtual LightType GetLightType()=0;


	/*! Does the light overshoot
	\return TRUE if the light supports overshoot
	*/
	virtual int GetLightOvershoot()=0;

	/*! The shape of the light
	\return  The shape can be one of the following
	\n
	RECT_LIGHT\n
	CIRCLE_LIGHT\n
	*/
	virtual int GetSpotLightShape()=0;

	/*! If the light is of type Spot light then this provides access to the target
	\return a pointer to IGameNode for the target.  This will be NULL for non target lights
	*/
	virtual IGameNode * GetLightTarget()=0;

	/*! Is the light on or not
	\return True if it is on
	*/
	virtual bool IsLightOn() = 0;

	/*! This determines whether the exclude list actually maintains a list that is infact included by the light
	\return TRUE if it maintains an included list
	*/
	virtual bool IsExcludeListReversed()=0;

	/*! Get the number of excluded nodes from the light.  This list contains nodes that should not be included in lighing
	calculations.  It can also contain a list of only those lights that SHOULD be included.  This all depends on the state 
	of IsExcludedListReversed.
	\return The total number of excluded nodes
	*/
	virtual int GetExcludedNodesCount() = 0;

	/*! Get the excluded node based on the index pass in
	\param index The index of the node to access
	\return An IGameNode pointer for the excluded node
	*/
	virtual IGameNode * GetExcludedNode(int index) = 0;



};

//!Simple wrapper for camera objects
/*! An IGame Wrapper around Max's cameras.  This is a generic interface for all the cameras
*/
class IGameCamera : public IGameObject {

public:

	//! Get the Camera Field of View Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetCameraFOV()=0;

	//! Get the Camera Far Clip plane Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetCameraFarClip()=0;

	//! Get the Camera Near Clip plane Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetCameraNearClip()=0;

	//! Get the Camera Target Distance
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetCameraTargetDist()=0;

	/*! If the camera is target camera then this provides access to the target
	\return a pointer to IGameNode for the target.  This will be NULL for non target cameras
	*/
	virtual IGameNode *  GetCameraTarget()=0;


};
//!Simple extension to the max Face class
/*!simple class to store extended data about the face.  The indexing works as a regular
max Face but mirroring has been taken account of, in the contruction.
*/

class FaceEx
{
public:

	//! Index into the vertex array
	DWORD vert[3];
	//! Index into the standard mapping channel
	DWORD texCoord[3];
	//! Index into the normal array
	DWORD norm[3];
	//! Index into the vertex color array
	DWORD color[3];
	//! Index into the vertex illumination array
	DWORD illum[3];
	//! Index into the vertex alpha array
	DWORD alpha[3];
	//! The smoothing group
	DWORD smGrp;
	//! The material ID of the face
	int matID;
	//! additional flags
	DWORD flags;
};

//!Simple wrapper for tri mesh objects
/*! An IGame wrapper around the standard max Mesh class.  It provides unified support for Vertex colors and normals
Mirroring is taken into account so the data you retreive is swapped to take this into account.\n
Many of the geometry lookups used by IGameObject use the Max Template Class Tab.  You can use the Tab returned to find out whether the call 
was successful as the Tab count would be greater then zero.
*/
class IGameMesh: public IGameObject {
public:
	//! Number of Vertices
	/*! The total number of vertices found in the mesh
	\return The number of Vertices
	*/
	virtual int GetNumberOfVerts()=0;

	//! Number of Texture Vertices
	/*! The total number of Texture vertices found in the mesh
	\return The number of TextureVertices
	*/	
	virtual int GetNumberOfTexVerts()=0;

	//!Get the actual Vertex
	/*! Get the vertex at the specified index.  This is in the World Space Coordinate System
	\param index The index of the vertex 
	\return A Point3 representing the position of the vertex
	*/
	virtual Point3 GetVertex(int index)=0;

	//!Get the actual Vertex
	/*! Get the vertex at the specified index.  This is in the World Space Coordinate System
	\param index The index of the vertex 
	\param &vert A Point3 to receieve the data
	\return TRUE if successful
	*/
	virtual bool GetVertex(int index, Point3 & vert) = 0;

	//!Get the actual Texture Vertex
	/*! Get the Texture vertex at the specified index
	\param index The index of the Texture vertex 
	\return A Point2 representing the Texture vertex
	*/
	virtual Point2 GetTexVertex(int index)=0;

	//!Get the actual Texture Vertex
	/*! Get the Texture vertex at the specified index
	\param index The index of the Texture vertex 
	\param &tex A Point2 to receieve the data.
	\return TRUE if successful
	*/
	virtual bool GetTexVertex(int index, Point2 & tex) = 0;

	//! Specifies whether normals are calculated based on face angles
	/*! To tell Igame to calculate normals based on a weight made from the angle of the edges at the vertex, the developer
	needs to call this <b><i>before</i></b> IGameObject::InitializeData() is called.  The default is not to use weighted normals
	which is also the default in 3ds max 4
	*/
	virtual void SetUseWeightedNormals() = 0;
	
	//! Number of Normals
	/*! The total number of normals found in the mesh
	\return The number of normals
	*/
	virtual int GetNumberOfNormals()=0;

	//!Get the actual normal
	/*! Get the normal at the specified index
	\param index The index of the normal
	\return A Point3 representing the normal
	*/
	virtual Point3 GetNormal(int index)=0;

	//!Get the actual normal
	/*! Get the normal at the specified index
	\param index The index of the normal
	\param &norm A Point3 to receieve the data.
	\return TRUE if successful
	*/
	virtual bool GetNormal(int index, Point3 & norm) = 0;

	
	//! Number of Illuminated Vertices
	/*! The total number of Illuminated Vertices found in the mesh
	\return The number of Illuminated Vertices
	*/
	virtual int GetNumberOfIllumVerts()=0;

	//! Number of Alpha Vertices
	/*! The total number of Alpha Vertices found in the mesh
	\return The number of Alpha Vertices
	*/
	virtual int GetNumberOfAlphaVerts()=0;

    //! Number of  Vertex Colors
	/*! The total number of Vertex Colors found in the mesh
	\return The number of Vertex Colors
	*/	
	virtual int GetNumberOfColorVerts()=0;

	//!Get the actual Color Vertex
	/*! Get the color vertex at the specified index
	\param index The index of the color vertex 
	\return A Point3 representing the color of the vertex.  This will be Point3(-1,-1,-1) if the index is invalid
	*/
	virtual Point3 GetColorVertex(int index)=0;

	//!Get the actual Color Vertex
	/*! Get the color vertex at the specified index
	\param index The index of the color vertex 
	\param &col A Point3 to receieve the color data
	\return TRUE if successful
	*/
	virtual bool GetColorVertex(int index, Point3 & col) = 0;

	//!Get the actual Alpha Vertex
	/*! Get the Alpha vertex at the specified index
	\param index The index of the Alpha vertex 
	\return A float representing the Alpha value of the vertex.  This will be -1 if the index is invalid 
	*/
	virtual float  GetAlphaVertex(int index)=0;


	//!Get the actual Alpha Vertex
	/*! Get the Alpha vertex at the specified index
	\param index The index of the Alpha vertex 
	\param &alpha A float to receive the value
	\return TRUE if successful
	*/
	virtual bool GetAlphaVertex(int index, float & alpha) = 0;

	//!Get the actual Illuminated Vertex
	/*! Get the Illuminated vertex at the specified index
	\param index The index of the Illuminated vertex 
	\return A float representing the Illuminated value of the vertex.  This will be -1 if the index is invalid
	*/
	virtual float  GetIllumVertex(int index)=0;
	
	//!Get the actual Illuminated Vertex
	/*! Get the Illuminated vertex at the specified index
	\param index The index of the Illuminated vertex 
	\param &illum A float to receieve the data
	\return TRUE if successful
	*/
	virtual bool GetIllumVertex(int index, float &illum) = 0;

	//! The num of faces in the mesh
	/*! The total number fo faces contained in the mesh
	\return The number fo faces
	*/
	virtual int GetNumberOfFaces()=0;
	
	//! The actual face 
	/*! The face represented by the index.  The data in FaceEx can be used to lookup into the various arrays
	\param index The index of the face to return
	\return A pointer to FaceEx, or NULL if an invalid index is passed in
	*/
	virtual FaceEx * GetFace(int index)=0;
	
	//!The number of verts in a mapping channel
	/*! Get the number of the vertices for a particular mapping channel
	\param ch The mapping channel to use
	\return The number of verts
	*/
	virtual int GetNumberOfMapVerts(int ch) = 0;

	//! The mapping vert
	/*! Get the actual mapping vert for the channel
	\param ch The channel to query
	\param index The vertex index
	\return The actual mapping data.  This will be zero if the Mapping channel is not found
	*/
	virtual Point3 GetMapVertex(int ch, int index) = 0;

	//! The mapping vert
	/*! Get the actual mapping vert for the channel
	\param ch The channel to query
	\param index The vertex index
	\param &mVert A Point3 to receive the data
	\return TRUE if successful
	*/
	virtual bool GetMapVertex(int ch, int index, Point3 & mVert) = 0;

	//! The the active mapping channels
	/*! Extracts the active mapping channels in use by the object.  This will not include the standard channels such as 
	Texture Coordinates and Vertex Colors, Illum, and Alpha.
	\return A tab containing the active Mapping channels.  The size of the Tab will be zero if no channels were found
	*/
	virtual Tab<int> GetActiveMapChannelNum() = 0;
	
	//! Get the face index into the mapping channel array
	/*! Get the actual index into the mapping channel for the supplied face.  
	\param ch The mappping channel to use
	\param faceNum The face to use
	\param index An array of three indices to receive the indexing into the vertices
	\return TRUE if the channel was accessed correctly.  False will mean that the channel was not present.
	*/
	virtual bool GetMapFaceIndex(int ch, int faceNum, DWORD *index) = 0;
	
	//!Get all the smoothing groups found on a mesh
	/*!
	\return A tab containing the smoothing groups.  If the count is zero it means that no smoothing groups were found
	*/
	virtual Tab<DWORD> GetActiveSmgrps() = 0;

	//!Get all the material IDs found on a mesh
	/*!
	\return A tab containing the Material IDs
	*/
	virtual Tab<int> GetActiveMatIDs() = 0;

	//!Get the face for a particular smoothing group
	/*! Get all the faces belonging to a particular smoothing group
	\param smgrp THe smoothing group to use
	\return A tab containing all the faces
	*/
	virtual Tab<FaceEx *> GetFacesFromSmgrp(DWORD smgrp) = 0;

	//!Get the face index for a particular smoothing group
	/*! Get all the faces belonging to a particular smoothing group as a set of indexes into the main face list
	\param smgrp THe smoothing group to use
	\return A tab containing all the indexes
	*/
	virtual	Tab<int> GetFaceIndexFromSmgrp(DWORD smgrp)=0;

	//! Get the faces for a particular Material ID
	/*! Get all the faces belonging to a particular material ID
	\param matID The material ID to use
	\return A tab containing all the faces
	*/	
	virtual Tab<FaceEx *> GetFacesFromMatID(int matID) = 0;

	//!The actual material used by the Face
	/*!This will provide access to the material used by the Face whose index is passed in.  This means the mesh can be broken down
	into smaller meshes if the material is a subObject material.  This can be used in conjunction with GetFacesFromMatID() to rebuild a face 
	with the material assigned via a material ID.
	\param FaceNum The index of the face whose material is needed.
	\returns A pointer to a material.  The is the actual material, so in the case of the SubObject material the material whose mat ID
	matches.
	*/
	virtual IGameMaterial * GetMaterialFromFace(int FaceNum) = 0;

	//!The actual material used by the Face
	/*!This will provide access to the material used by the Face.  This means the mesh can be broken down
	into smaller meshes if the material is a subObject material.  This can be used in conjunction with GetFacesFromMatID() to rebuild a face 
	with the material assigned via a material ID.
	\param face A pointer to the face whose material is needed.
	\returns A pointer to a material.  The is the actual material, so in the case of the SubObject material the material whose mat ID
	matches.
	*/
	virtual IGameMaterial * GetMaterialFromFace(FaceEx * face) = 0;


	//! The actual max Mesh representation
	/*! The Mesh pointer used by max.  This allows the developer further access if required to the mesh or data structures
	\return A Mesh pointer
	*/
	virtual Mesh * GetMaxMesh()=0;

	//! Access the color data for the face specified.
	/*! The surface color can be obtained from the RenderedSurface interface.  Before this function can be used the Surface
	data needs to be initialised before hand.  Please see the IGameRenderedSurface for more information
	\param FaceIndex The face index whose color is being evaluated
	\param *result A pointer to a Color Array that receives the 3 vertices for the face.  This should initialised as Color res[3]
	\return TRUE is successful.  Possible errors include the object not being renderable or is hidden.
	\sa IGameRenderedSurface
	*/
	virtual bool EvaluateSurface(int FaceIndex, Color * result) = 0;



};

//!Simple wrapper for spline knots
/*! An IGame wrapper for Knot information
*/
class IGameKnot
{
public:
	enum KnotType{
		KNOT_AUTO,				/*!< Auto generate Knot*/
		KNOT_CORNER,			/*!< A corner knot*/
		KNOT_BEZIER,			/*!< A bezier knot*/
		KNOT_BEZIER_CORNER,		/*!< A bezier corner knot*/
	};

	enum KnotData{
		KNOT_INVEC,				/*!< The in vector*/
		KNOT_OUTVEC,			/*!< The out vector*/
		KNOT_POINT,				/*!< The actual knot position*/
	};

	//! Get the in vector
	/*!
	\return the in vector
	*/
	virtual Point3 GetInVec()=0;
	//! Get the out vector
	/*!
	\return the out vector
	*/
	virtual Point3 GetOutVec()=0;

	//! Get actual knot position
	/*!
	\return The knot position
	*/
	virtual Point3 GetKnotPoint()=0;

	//! The type of knot
	/*!
	\return The knot as a KnotType
	*/
	virtual KnotType GetKnotType()=0;
	//! The knot controller
	/*! Get the actual IGameController for the knot - this provides access to any animated data
	\param kd THe knot to access
	\return The controller for the specified knot
	*/
	virtual IGameControl * GetKnotControl(KnotData kd)=0;
};

//! A wrapper class for splines
/*! This  provides information about the actual splines making up the spline object in Max.  Access to the knot data is 
provided with this class
*/
class IGameSpline3D
{
	
public:
	//! Access the individual knot
	/*!
	\return The knot for the index passed in
	*/
	virtual IGameKnot * GetIGameKnot(int index) = 0;
	//! The knot count
	/*!
	\return The total number of knots in the spline
	*/
	virtual int GetIGameKnotCount()=0;

};

//!Simple wrapper for Splines
/*! An IGame wrapper around the standard Max spline object.  IGameSpline acts as a container for all the individual splines
that make up the object
*/
class IGameSpline : public IGameObject
{


public:
	//! The number of splines
	/*! The number of splines that make up this object
	\return The total number of splines
	*/
	virtual int GetNumberOfSplines()=0;

	//! Get an individual Spline
	/*! Get a spline based on the index.
	\param index The index of the spline to access
	\return a pointer to the spline.
	*/
	virtual IGameSpline3D * GetIGameSpline3D(int index) =0;

	//! Get the Max object
	/*!
	\return The ShapeObject used by max
	*/
	virtual ShapeObject * GetMaxShape()=0;

//	IGameSpline(INode * node) : IGameObject(node){};

};
//!Simple wrapper for IKChains
/*! An IGame wrapper around the IKChain object. 
\br
This object be used as a basis for character export.  All nodes used in the chain are maintained by the IGameIKChain
interface.  This allows animation to be exported based on whether it is in IK or FK mode.  If the IK is enabled then
the IGameControl retrieved from the IGameNode will be that of the End Effector for the chain.  If it is in FK mode then 
the IGameControl from the nodes in the chain would be used for the FK calculation. 
*/
class IGameIKChain : public IGameObject
{
public:

	//! The number of nodes that make up the chain
	/*!
	\return The number of nodes in the chain
	*/
	virtual int GetNumberofBonesinChain() =0;

	//! Access to the n'th node in the chain
	/*!
	\param index The index of the node to access
	\return An IGameNode representation of the node
	*/
	virtual IGameNode * GetIGameNodeInChain(int index) =0;

	//! The swivel data
	/*! The swivel data used in the IK calculation
	\return The IGameProperty for the swivel data
	*/
	virtual IGameProperty * GetSwivelData() = 0;

	//! The controller for the IK enable
	/*! Access to the Enabled controller - this defines whether IK or FK are used.  When IK the end effector 
	is used to control the transforms, in FK the individual nodes can be positioned indepedent of the effector
	\return An IGameControl pointer for the Enabled controller.  This controller does not have direct access, so 
	should be sampled using IGAME_FLOAT
	*/
	virtual IGameControl * GetIKEnabledController() = 0;


};

//! Base class for "support" objects, such as bone, helpers dummies etc..  
/*!These types of objects are really supported for their parameter access.  However a pointer 
to the Mesh representation is provided if for example bone geometry is needed.  This class can be used
to check for BONES, DUMMYS etc..
*/
class IGameSupportObject : public IGameObject
{

public:


	//! Access to the mesh
	/*! If required access to the mesh is provided here
	\return A pointer to an IGameMesh object
	*/
	virtual IGameMesh * GetMeshObject() = 0;

	//!Is the bone a funcky r4 bone with wings ?
	/*!
	\return TRUE if it is an R4 bone.
	*/
	virtual bool PreR4Bone() = 0;

	
};

//! An interface into an XRef Object Currently not implemented
class IGameXRefObject : public IGameObject
{
public:
	virtual TCHAR * GetOriginalFileName() = 0;

};

//! A Generic Object for IGame
/*! This object represents any object that is unknown to IGame - this could be a new pipeline object for example
It will return IGAME_UNKOWN for its ObjectTypes
*/
class IGameGenObject : public IGameObject
{
public:
	
	virtual IPropertyContainer * GetIPropertyContainer()=0;
};


#endif