	 /**********************************************************************
 
	FILE: IAssembly.h

	DESCRIPTION:  Public interface for setting and getting assembly flags

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created April 03, 2001

 *>	Copyright (c) 1998-2000, All Rights Reserved.
 **********************************************************************/

#ifndef __IASSEMBLY__H
#define __IASSEMBLY__H

#include "iFnPub.h"
#include "maxtypes.h"

// This type is not being used currently 
typedef int AssemblyCode;

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// class IAssembly
//
// This interface allows for setting and retrieving assembly membership
// information to\from nodes. All methods of the interface are implemented 
// by the system (Max).
// Client code can query an INode for this interface:
// INode* n;
// IAssembly* a = GetAssemblyInterface(n);
//________________________________________________________________________
class IAssembly : public FPMixinInterface 
{
	public:
		//
		// -- Methods for setting assembly flags
		//

		// NOTE: nodes can be both assembly members and heads in the same time.
		
		// If b=TRUE, sets a node as an assembly member.
		// If b=FALSE sets a closed or open assembly member as not an assembly member
		// If the member is open, it closes it first then removes the member flag
		// To close an assembly member, call SetAssemblyMemberOpen(FALSE)
		virtual void SetAssemblyMember(BOOL b) = 0;
		
		// Should only be called on assembly members
		// If b=TRUE opens an assembly member
		// If b=FALSE closes an assembly member
		virtual void SetAssemblyMemberOpen(BOOL b) = 0;

		// If b=TRUE sets a node as an assembly head
		// If b=FALSE sets a closed or open assembly head as not an assembly head
		// If the head is open, it closes it first then removes the head flag
		// To close an assembly head, call SetAssemblyHeadOpen(FALSE)
		virtual void SetAssemblyHead(BOOL b) = 0;
		
		// Should only be called on assembly heads
		// If b=TRUE opens an assembly head
		// If b=FALSE closes an assembly head
		virtual void SetAssemblyHeadOpen(BOOL b) = 0;
		
		//
		// -- Methods for querying the assembly flags
		//
		
		// NOTE: to detect closed assembly members\heads, check for both 
		// the assembly member\head flag and open member\head flags:
		// IsAssemblyHead() && !IsAssemblyMemberOpen()

		// Returns TRUE for both closed and open assembly members\heads
		virtual BOOL IsAssemblyMember() const = 0;
		virtual BOOL IsAssemblyHead() const = 0;
		
		// Returns TRUE for open assembly members\heads
		virtual BOOL IsAssemblyMemberOpen() const = 0;
		virtual BOOL IsAssemblyHeadOpen() const = 0;
		
		// This method is used for detecting assemblies in assemblies. 
    // The method checks if this assembly node is a head node and whether it's a
    // member of the assembly head node passed in as parameter.
		virtual BOOL IsAssemblyHeadMemberOf(const IAssembly* const assemblyHead) const = 0;

		// Allow persistance of info kept in object implementing this interface
		virtual IOResult Save(ISave* isave) = 0;
		virtual IOResult Load(ILoad* iload) = 0;

		// -- IAssembly function publishing
		// Methods IDs
		enum 
		{ 
			E_SET_ASSEMBLY_MEMBER, 
			E_GET_ASSEMBLY_MEMBER, 
			E_SET_ASSEMBLY_HEAD, 
			E_GET_ASSEMBLY_HEAD, 
			E_SET_ASSEMBLY_MEMBER_OPEN, 
			E_GET_ASSEMBLY_MEMBER_OPEN, 
			E_SET_ASSEMBLY_HEAD_OPEN,
			E_GET_ASSEMBLY_HEAD_OPEN,
		}; 
	
}; 

// Assembly interface ID
#define ASSEMBLY_INTERFACE Interface_ID(0x2512714b, 0x4b456518)

inline IAssembly* GetAssemblyInterface(BaseInterface* baseIfc)	
{ DbgAssert( baseIfc != NULL); return static_cast<IAssembly*>(baseIfc->GetInterface(ASSEMBLY_INTERFACE)); }

//¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
// class IAssembly2
//
// This new version of the assembly interface extends IAssembly
// SDK programmers are encouraged to use this version of the assembly interface
// 
// All methods of the interface are implemented by the system (Max).
// Client code can query an INode for this interface:
// INode* n;
// IAssembly2* a = GetAssemblyInterface2(n);
//________________________________________________________________________
class IAssembly2 : public IAssembly
{
	public:
		// Methods to control the display of assembly world 
		// bounding box of open assemblies, on a per assembly basis
		// These methods should be called on assembly head nodes only
		// If called on assembly member nodes, the display of the world 
		// bounding box won't get turned off.
		virtual void SetAssemblyBBoxDisplay(BOOL b) = 0;
		virtual BOOL GetAssemblyBBoxDisplay() = 0;

		// -- IAssembly2 function publishing
		// Methods IDs
		enum 
		{ 
			E_SET_ASSEMBLY_BBOX_DISPLAY = IAssembly::E_GET_ASSEMBLY_HEAD_OPEN + 1, 
			E_GET_ASSEMBLY_BBOX_DISPLAY, 
		};

};
#define ASSEMBLY_INTERFACE2 Interface_ID(0x6fd5515a, 0x353c6734)
inline IAssembly2* GetAssemblyInterface2(BaseInterface* baseIfc)	
{ DbgAssert( baseIfc != NULL); return static_cast<IAssembly2*>(baseIfc->GetInterface(ASSEMBLY_INTERFACE2)); }

#endif
