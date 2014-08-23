/**********************************************************************
 *<
	FILE: IGameError.h

	DESCRIPTION: Access to the IGame Errors.

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

/*!\file IGameError.h
\brief IGame Error Access.

Internal IGame methods maintain a global error state, these methods provide access to this data.  Many IGame methods return
pointers or boolean status flags.  If either of these are null or false, then you can use these method to access the error.
*/
#ifndef __IGAMEERROR__H
#define __IGAMEERROR__H

#include "max.h"

#pragma once

#define IGAMEEEXPORT __declspec( dllexport )


//!Enumeration of the Error codes produced by IGame
enum IGameError{
	IG_NO_ERROR,						/*!<No Error*/
	IG_NO_KEY_INTERFACE,				/*!<No Key interface for the controller*/
	IG_INDEX_OUT_OF_BOUNDS,				/*!<The index into the array is out of bounds*/
	IG_ERROR_LOADING_PROPERTIES_FILE,	/*!<Errors finding or parsing the Properties file*/
	IG_COM_ERROR,						/*!<Various COM errors*/
	IG_NODE_NOT_FOUND,					/*!<IGameNode not found*/
	IG_UNSUPPORTED_CONT,				/*!<The controller for the basic type is not supported*/
	IG_OBJECT_NOT_SUPPORTED,			/*!<Object not supported by IGame*/
	IG_MAPPING_CHANNEL_ERROR,			/*!<Mapping Channel not found*/
	IG_MATERIAL_ERROR,					/*!<Material not found*/
	IG_NO_SKIN_MOD,						/*!<No skin modifier*/
	IG_NO_CONTROLLER_KEY,				/*!<No Keys setr on the controller*/
};



//!Error callback 
/*!Define a callback for error reporting.  This will be called when ever an error has been reported by the system.
The developer can then call GetIGameErrorText to retrieve a more detailed error description.  The callback can be 
set by using SetErrorCallBack()
*/
class IGameErrorCallBack
{
public:
	//!The error callback
	/*!This needs to be implemented by the developer and is used by the system to report the error
	\param error The error code of the last error
	*/
	virtual void ErrorProc(IGameError error)=0;
};


/*! Retrieve the last error set by the system
\returns The error code
*/
IGAMEEEXPORT IGameError GetLastIGameError();

/*! Get the detailed description of the last error set by the system
\returns The error text
*/
IGAMEEEXPORT TCHAR * GetLastIGameErrorText();

/*! Set the callback for the error logging
\param *proc  A pointer the the IGameErrorCallback object created by the developer
*/
IGAMEEEXPORT void SetErrorCallBack(IGameErrorCallBack * proc);

/*! Resets the last error stored by the system.  The global error will only change when IGame sets the last error.
Using this method will override this.
*/
IGAMEEEXPORT void ResetError();

#endif