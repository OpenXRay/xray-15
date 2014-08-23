/**********************************************************************
 *<
	FILE: IGameProperty.h

	DESCRIPTION: IGameProperty interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef __IGAMEPROPERTY__H
#define __IGAMEPROPERTY__H

#pragma once

/*!\file IGameProperty.h
\brief IParamBlock and IPAramBlock2 property access.

All properties found by IGame are stored as an IGameProperty.  This gives developers a unified way of accessing
IParamBlock and IParamBlock2 based properties used in max
*/




enum PropType{
	IGAME_UNKNOWN_PROP,	/*!<Unkown property Type*/
	IGAME_FLOAT_PROP,	/*!<Property of Type float*/
	IGAME_POINT3_PROP,	/*!<Property of Type Point3*/
	IGAME_INT_PROP,		/*!<Property of Type int*/
	IGAME_STRING_PROP,  /*!<Property of Type TCHAR*/
};

class IGameControl;

//!Main property definition
/*! IGameProperty provides a wrapper around the standard max ParamBlock system.  It works for both 
IParamBlock and IParamBlock2.  It provides access to IGameControl and also data access for floats, ints, and Point3
It performs the type checking for you so the Paramblock system will not assert  if you ask for the wrong data type
The properties the IGame stores are defined in the IGameProp.XML file.  THe data there is used to find the paramters 
in the various paramblock hosted by the objects,
\n
The data is provided by look up from the IGameProperties.xml file.  This provides the properties that IGame will look for in 
its evaluation.  This included User Properties and general Paramblock data.
*/

class IGameProperty
{

public:
	//!The name of the Property
	/*! This is the name defined in the XML file
	\return The parameter name
	*/
	virtual TCHAR * GetName() =0;

	//! Is the parameter a IParamBlock2 or not
	/*!
	\return TRUE if IParamBlock2 else it is a IParamBlock
	*/
	virtual bool IsPBlock2()=0;
	//! Is is animated
	/*!  Use this to decided whether you want to access the controller
	\return TRUE if animated
	*/
	virtual bool IsPropAnimated()=0;

	//! The controller for the Property
	/*!
	\return A pointer to IGameContorl
	*/
	virtual IGameControl * GetIGameControl() =0;

	//! Direct access to the IParamBlock2 
	/*! for those who like a little more control
	\return a pointer to IParamBlock2
	*/
	virtual IParamBlock2 * GetMaxParamBlock2() =0;

	//! Direct access to the IParamBlock 
	/*! for those who like a little more control
	\return a pointer to IParamBlock
	*/
	virtual IParamBlock * GetMaxParamBlock() =0;

	//! Is the parameter ParamBlock based
	/*! Specifies whether this parameter is based on either IParamBlock, or IParamBlock2.  This is useful as some IGameProperties
	are based on Non paramblocks.  For example node/user data is accessed as an IGame Property but in max has no Paramblock representation
	\return TRUE if it is based on a either IParamBlock or IParamBlock2.
	*/
	virtual bool IsParamBlock()=0;

	//! The data type of the Property
	/*! This is used to find out the data type of the property.  It is used so the correct GetPropertyValue method can be 
	used
	\return The data type.  This will be a value from PropType enumeration.
	*/
	virtual PropType GetType() =0;

	//! Whether this is Paramter is directly supported and has a entry in the Properties file
	/*! Uses this method to decide whether a property is directly supported via the XML file
	\return True if it directly supported with access from the XML file
	*/

	virtual bool IsParameterSupported() = 0;

	//! The index of the parameter
	/*! The actual index of the parameter as used by the Paramblock system.  This can be used for direct access to the
	property from the parmblock container.  This can be used for GetValue/SetValue calls.
	\return The index in the paramter block
	*/
	virtual int GetParamBlockIndex() = 0;

	//! Access to the actual Parameter Data
	/*!
	\param &f  The float to receive the data
	\param t  The time to retrieve the value.  If the default is used then the static frame is used.  This is set by SetStaticFrame
	\return TRUE if successfull
	\sa IGameScene::SetStaticFrame
	*/
	virtual bool GetPropertyValue(float &f, TimeValue t=TIME_NegInfinity)=0;

	//! Access to the actual Parameter Data
	/*!
	\param &i  The int to receive the data
	\param t  The time to retrieve the value.  If the default is used then the static frame is used.  This is set by SetStaticFrame
	\return TRUE if successfull
	\sa IGameScene::SetStaticFrame
	*/	
	virtual bool GetPropertyValue(int &i, TimeValue t=TIME_NegInfinity)=0;
	
	//! Access to the actual Parameter Data
	/*!
	\param &p  The Point3 to receive the data
	\param t  The time to retrieve the value.  If the default is used then the static frame is used.  This is set by SetStaticFrame
	\return TRUE if successfull
	\sa IGameScene::SetStaticFrame
	*/	
	virtual bool GetPropertyValue(Point3 &p, TimeValue t=TIME_NegInfinity)=0;

	//! Access to the actual Parameter Data
	/*!
	\param v  The TCHAR to receive the data
	\param t  The time to retrieve the value.  If the default is used then the static frame is used.  This is set by SetStaticFrame
	\return TRUE if successfull
	\sa IGameScene::SetStaticFrame
	*/	
	virtual bool GetPropertyValue(TCHAR*& v, TimeValue t=TIME_NegInfinity)=0;

};

//!Property Enumeration
/*! PropertyEnum allows a developer to define a callback for use with EnumerateProperties.  It will be called for 
every parameter stored in the system
*/
class PropertyEnum
{
public:
	
	//! The call back function
	/*! This is called for every property in the system, providing a way of stopping the enumeration if need be
	\param *prop The actual property found
	\return TRUE to stop the enumeration
	*/
	virtual bool Proc(IGameProperty* prop) = 0;
};

//! Property container
/*! This class provide an extension mechanism that IGame can use - an Entity is free to use them
the idea here, is that a developer can extend the properties that are "known" to IGame
this way the property can be retrieved directly by the developer.  As it is "known" to
the developer the property type is also known and can be accessed directly
*/
class IPropertyContainer
{
public:

	//! Property Access
	/*!Using the unique ID in the XML file, the property can be queried directly
	\param PropID The indentifier used in the XML file
	\return A pointer to IGameProperty if found - NULL if not
	*/
	virtual IGameProperty * QueryProperty(DWORD PropID) {return NULL;}

	//! The number of Properties for the Entity
	/*!
	\return The number of properties found.  The default is 0. This only counts the supported properties as defined
	in the property file
	*/
	virtual int GetNumberOfProperties(){return 0;}
	//!Direct Property Access
	/*! The property can be accessed direclty from the index provided from the XML file.  You can use the IGame Editor to
	write out the index for the properties as a Header file for easy access.  This only works for Supported Parameters, 
	i.e. Properties found in the properties file
	\param index The index of the property to return
	\return A pointer to the IGameProperty.  The default is NULL
	*/
	virtual IGameProperty * GetProperty(int index) {return NULL;}

	//! Property Access
	/*!Using the name in the XML file, the property can be queried directly
	\param propName The name indentifier used in the XML file
	\return A pointer to IGameProperty if found  The default is NULL
	*/
	virtual IGameProperty * QueryProperty(const TCHAR * propName) {return NULL;}

	//!Enumerate the Properties
	/*!All properties can be enumerated by using this method.  A user defined callback is used to provide access to 
	the properties.
	\param &Enum The callback object to use.
	*/
	virtual void EnumerateProperties(PropertyEnum & Enum) = 0;


};

//!Main property access
/*!base Class used by all exporters wanting to take part in the properties system
*/
class IExportEntity
{
public:
	//!Retrieve the Property Container
	/*!
	\return The PropertyContainer for this entity.
	*/
	virtual IPropertyContainer * GetIPropertyContainer(){return NULL;}

	//! Is the Entity directly supported
	/*!IGame provides direct support for certain max object and materials.  If the IGameProp.xml file
	contains additional properties on unknown ClassIDs, for example a new Object then this method is used to find out whether IGame supports 
	them directly.  IGame supports the standard material directly, i.e provides direct API calls to get the properties.  If a material was 
	found that was not a standard material, then the paramters can be access by using the IGameProp file and the IPropertyContainer file.
	\return TRUE if IGame supports the parameters for a particular ClassID directly through its API
	*/
	virtual bool IsEntitySupported() {return false;}
};


#endif