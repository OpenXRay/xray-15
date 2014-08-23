/**********************************************************************
 *<
	FILE: CreatedChannelLinker.h

	DESCRIPTION: Class definitions for CreatedChannelLinker
				CreatedChannelLinker keeps track of all channels created for
				a specific particle container. If an action creates a channel
				only under certain condition (for example, if the channel 
				doesn't exists) then it's the action responsibility to 
				initialized channel value for all new particles. Therefore
				the action should keep track of all channels created by the
				action. The class makes this task easier.

	CREATED BY: Oleg Bayborodin

	HISTORY:	created 03-11-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _CREATEDCHANNELLINKER_H 
#define _CREATEDCHANNELLINKER_H

#include "max.h"

#include "PFExport.h"

namespace PF {

class CreatedChannelLinker {
public:

	PFExport CreatedChannelLinker();
	PFExport ~CreatedChannelLinker();

	// to indicated that the channel was created in this container
	PFExport bool RegisterCreatedChannel(IObject* pCont, Interface_ID channelID);
	// to unregister all created channels in the container
	PFExport void Release(IObject* pCont);
	// verify if the channel was created for the container
	PFExport bool IsCreatedChannel(IObject* pCont, Interface_ID channelID) const;

private:
	// const access to class members
	const Tab<IObject*>&		particleContainers()			const { return m_particleContainers; }
	IObject*					particleContainer(int index)	const { return m_particleContainers[index]; }
	const Tab<Interface_ID>&	IDs()							const { return m_IDs; }
	Interface_ID				ID(int index)					const { return m_IDs[index]; }

	// access to class members
	Tab<IObject*>&			_particleContainers()			{ return m_particleContainers; }
	IObject*&				_particleContainer(int index)	{ return m_particleContainers[index]; }
	Tab<Interface_ID>&		_IDs()							{ return m_IDs; }
	Interface_ID&			_ID(int index)					{ return m_IDs[index]; }

protected:
	Tab<IObject*> m_particleContainers;
	Tab<Interface_ID> m_IDs;
};


} // end of namespace PF


#endif // _CREATEDCHANNELLINKER_H_

