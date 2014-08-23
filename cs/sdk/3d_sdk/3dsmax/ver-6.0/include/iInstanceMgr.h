 /**********************************************************************
 
	FILE: IInstanceMgr.h

	DESCRIPTION:  Public interface for working with object instances

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created Aug 26, 2002

 *>	Copyright (c) 1998-2002, All Rights Reserved.
 **********************************************************************/

#ifndef _IINSTANCEMGR_H_
#define _IINSTANCEMGR_H_

#include "iFnPub.h"
#include "maxtypes.h"

// --- Forward declaration
class INode;
class Mtl;

// --- Interface IDs
#define INSTANCE_MGR_INTERFACE Interface_ID(0x5ecd70b5, 0x59092257)

// Make Unique options for multiple nodes
//
// These options control whether a group of instanced objects are
// made unique with respect to each other or made unique with respect 
// to the remaining instanced objects.  These options are only relevant
// when more than one nodes are passed in.
//
// INSTANCE_MGR_MAKE_UNIQUE_PROMPT - display dialog box to prompt user
// INSTANCE_MGR_MAKE_UNIQUE_INDIVIDUAL - make objects individually unique
// INSTANCE_MGR_MAKE_UNIQUE_GROUP - make group of objects unique
//
#define INSTANCE_MGR_MAKE_UNIQUE_PROMPT		0
#define INSTANCE_MGR_MAKE_UNIQUE_INDIVIDUAL	1
#define INSTANCE_MGR_MAKE_UNIQUE_GROUP		2

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// class IInstanceMgr
//
// This interface allows different parts of the system (max) and plugins
// to work with instances of objects
//________________________________________________________________________
class IInstanceMgr : public FPStaticInterface
{
	public:
		// Retrieves instances and references of an object. Instances of ADT styles
		// will be collected only if the ADTObjects plugin is present in the system.
		//
		// INode& source				- This node represents the object whose instances 
		//												should be retrieved. For an object with multiple 
		//												instances, any of the nodes representing 
		//												an instance can be specified here as a parameter
		// INodeTab& instances	- An array of nodes representing the instances 
		//												of the object referenced by the first parameter. 
		//                        This array is cleared before collection of instances 
		//                        starts. The first parameter is also included in this 
		//                        array since every object can be thought of as having 
		//												at least one instance. 
		// Return value					- The number of instances found. 
		//												There's always at least 1 instance found
		virtual unsigned long GetInstances(INode& source, INodeTab& instances) const = 0;

		// Sets the provided material on all instances of a given object. 
		// The object is specified through a node that references it.
		//
		// INode& srcNode				- This node represents the object whose instances 
		//												should be set the material on. For an object with multiple 
		//												instances, any of the nodes representing 
		//												an instance can be specified here as a parameter
		// Mtl* newMtl					- The material to be set. This can be NULL which means that
		//												the material is being removed
		// Return value					- The number of instances found. 
		virtual unsigned long SetMtlOnInstances(INode& source, Mtl* newMtl) = 0;
		
		// Methods to work with the Automatic Material Propagation flag.
		// When ON, material assignments to any object type will get automatically
		// propagated to all instances and references of that obejct. No user 
		// confirmation required.
		// When OFF, material assignments to object won't get propagated to their instances 
		// and references. The material of an instance can be still propagated 
		// by calling the SetMtlOnInstances method 
		// The initial value of this flag can be set through the AutomaticMtlPropagation
		// setting, in the InstanceMgr section of the application's ini file.
		// When the application closes, the current value of the flag is saved in 
		// the ini file by the system
		virtual bool					GetAutoMtlPropagation() const = 0;
		// Returns previous value of flag
		virtual bool					SetAutoMtlPropagation(bool autoPropagate) = 0;
		
		// Methods for making base objects or derived objects of a list of nodes unique.
		//
		// INodeTab& tabNodes			- An array of nodes corresponding to the objects
		//								that will be made unique.
		// Return value					- true if array of nodes contains at least one
		//								object that can be made unique.
		virtual bool					CanMakeObjectsUnique(INodeTab& tabNodes) const = 0;
		// Return value					- true if method successful
		virtual bool					MakeObjectsUnique(INodeTab& tabNodes, int iMultiNodeOption = INSTANCE_MGR_MAKE_UNIQUE_PROMPT) const = 0;

		// Methods for making an array of modifiers unique.  The array of modifiers will
		// be made unique within the array of nodes.
		//
		// INodeTab& tabNodes			- An array of nodes corresponding to the modifiers
		//								that will be made unique.
		// Tab<ReferenceTarget*>& tabMods - An array of modifiers to be made unique.
		// Return value					- true if array of nodes contains at least one
		//								modifier that can be made unique.
		virtual bool					CanMakeModifiersUnique(INodeTab& tabNodes, Tab<ReferenceTarget*>& tabMods) const = 0;
		// Return value					- true if method successful
		virtual bool					MakeModifiersUnique(INodeTab& tabNodes, Tab<ReferenceTarget*>& tabMods, int iMultiNodeOption = INSTANCE_MGR_MAKE_UNIQUE_PROMPT) const = 0;

		// Methods for making an array of controllers unique.  The array of controllers will
		// be made unique within the array of nodes.
		//
		// INodeTab& tabNodes			- An array of nodes corresponding to the controllers
		//								that will be made unique.
		// Tab<ReferenceTarget*>& tabMods - An array of controllers to be made unique.
		// Return value					- true if array of nodes contains at least one
		//								controller that can be made unique.
		virtual bool					CanMakeControllersUnique(INodeTab& tabNodes, Tab<ReferenceTarget*>& tabConts) const = 0;
		// Return value					- true if method successful
		virtual bool					MakeControllersUnique(INodeTab& tabNodes, Tab<ReferenceTarget*>& tabConts, int iMultiNodeOption = INSTANCE_MGR_MAKE_UNIQUE_PROMPT) const = 0;

		// --- File I/O 
		virtual IOResult Save(ISave* isave) const = 0;
		virtual IOResult Load(ILoad* iload) = 0;

		static IInstanceMgr* GetInstanceMgr()
		{
			return static_cast<IInstanceMgr*>(GetCOREInterface(INSTANCE_MGR_INTERFACE));
		}
}; 




#endif //_IINSTANCEMGR_H_
