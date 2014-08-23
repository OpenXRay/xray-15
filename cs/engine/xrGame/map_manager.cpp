#include "pch_script.h"
#include "map_manager.h"
#include "alife_registry_wrappers.h"
#include "inventoryowner.h"
#include "level.h"
#include "actor.h"
#include "relation_registry.h"
#include "GameObject.h"
#include "map_location.h"
#include "GameTaskManager.h"
#include "xrServer.h"
#include "game_object_space.h"
#include "script_callback_ex.h"

struct FindLocationBySpotID{
	shared_str	spot_id;
	u16			object_id;
	FindLocationBySpotID(const shared_str& s, u16 id):spot_id(s),object_id(id){}
	bool operator () (const SLocationKey& key){
		return (spot_id==key.spot_type)&&(object_id==key.object_id);
	}
};
struct FindLocationByID{
	u16			object_id;
	FindLocationByID(u16 id):object_id(id){}
	bool operator () (const SLocationKey& key){
		return (object_id==key.object_id);
	}
};

struct FindLocation{
	CMapLocation*			ml;
	FindLocation(CMapLocation* m):ml(m){}
	bool operator () (const SLocationKey& key){
		return (ml==key.location);
	}
};

void SLocationKey::save(IWriter &stream)
{
	stream.w		(&object_id,sizeof(object_id));

	stream.w_stringZ(spot_type);
	stream.w_u8		(0);
	location->save	(stream);
}
	
void SLocationKey::load(IReader &stream)
{
	stream.r		(&object_id,sizeof(object_id));

	stream.r_stringZ(spot_type);
	stream.r_u8		();

	location  = xr_new<CMapLocation>(*spot_type, object_id);

	location->load	(stream);
}

void SLocationKey::destroy()
{
	delete_data(location);
}

void CMapLocationRegistry::save(IWriter &stream)
{
	stream.w_u32			((u32)objects().size());
	iterator				I = m_objects.begin();
	iterator				E = m_objects.end();
	for ( ; I != E; ++I) {
		u32					size = 0;
		Locations::iterator	i = (*I).second.begin();
		Locations::iterator	e = (*I).second.end();
		for ( ; i != e; ++i) {
			VERIFY			((*i).location);
			if ((*i).location->Serializable())
				++size;
		}
		stream.w			(&(*I).first,sizeof((*I).first));
		stream.w_u32		(size);
		i					= (*I).second.begin();
		for ( ; i != e; ++i)
			if ((*i).location->Serializable())
				(*i).save	(stream);
	}
}


CMapManager::CMapManager()
{
	m_locations_wrapper = xr_new<CMapLocationWrapper>();
	m_locations_wrapper->registry().init(1);
	m_locations = NULL;
}

CMapManager::~CMapManager()
{
	delete_data		(m_locations_wrapper);
}

CMapLocation* CMapManager::AddMapLocation(const shared_str& spot_type, u16 id)
{
	FindLocationBySpotID key(spot_type, id);
	Locations_it it = std::find_if(Locations().begin(),Locations().end(),key);
	if( it == Locations().end() )
	{
		CMapLocation* l = xr_new<CMapLocation>(*key.spot_id, key.object_id);
		Locations().push_back( SLocationKey(key.spot_id, key.object_id) );
		Locations().back().location = l;
		if (IsGameTypeSingle()&& g_actor)
			Actor()->callback(GameObject::eMapLocationAdded)(*spot_type, id);
	
		return l;
	}else
		(*it).location->AddRef();

	return (*it).location;
}

CMapLocation* CMapManager::AddRelationLocation(CInventoryOwner* pInvOwner)
{
	if(!Level().CurrentViewEntity())return NULL;

	ALife::ERelationType relation = ALife::eRelationTypeFriend;
	CInventoryOwner* pActor = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());
	relation =  RELATION_REGISTRY().GetRelationType(pInvOwner, pActor);
	shared_str sname = RELATION_REGISTRY().GetSpotName(relation);

	CEntityAlive* pEntAlive = smart_cast<CEntityAlive*>(pInvOwner);
	if( !pEntAlive->g_Alive() ) sname = "deadbody_location";


	FindLocationBySpotID key(sname, pInvOwner->object_id());
	Locations_it it = std::find_if(Locations().begin(),Locations().end(),key);
	if( it == Locations().end() )
	{
		CMapLocation* l = xr_new<CRelationMapLocation>(*key.spot_id, key.object_id, pActor->object_id());
		Locations().push_back( SLocationKey(key.spot_id, key.object_id) );
		Locations().back().location = l;
		return l;
	}else
		(*it).location->AddRef();

	return (*it).location;
}

void CMapManager::RemoveMapLocation(const shared_str& spot_type, u16 id)
{
	FindLocationBySpotID key(spot_type, id);
	Locations_it it = std::find_if(Locations().begin(),Locations().end(),key);
	if( it!=Locations().end() )
	{
		if( 1==(*it).location->RefCount() )
		{
			if(IsGameTypeSingle())
				Level().GameTaskManager().MapLocationRelcase((*it).location);
			delete_data				(*it);
			Locations().erase		(it);
		}else
			(*it).location->Release();
	}
}

void CMapManager::RemoveMapLocationByObjectID(u16 id) //call on destroy object
{
	FindLocationByID key(id);
	Locations_it it = std::find_if(Locations().begin(),Locations().end(),key);
	while( it!= Locations().end() )
	{
		if(IsGameTypeSingle())
			Level().GameTaskManager().MapLocationRelcase((*it).location);

		delete_data				(*it);
		Locations().erase		(it);

		it = std::find_if(Locations().begin(),Locations().end(),key);
	}
}

void CMapManager::RemoveMapLocation(CMapLocation* ml)
{
	VERIFY(ml->CanBeUserRemoved());
	FindLocation key(ml);

	Locations_it it = std::find_if(Locations().begin(),Locations().end(),key);
	if( it!=Locations().end() )
	{
		if(IsGameTypeSingle())
			Level().GameTaskManager().MapLocationRelcase((*it).location);

		delete_data				(*it);
		Locations().erase		(it);
	}

}

bool CMapManager::GetMapLocationsForObject(u16 id, xr_vector<CMapLocation*>& res)
{
	res.clear_not_free();
	Locations_it it = Locations().begin();
	Locations_it it_e = Locations().end();
	for(; it!=it_e;++it)
	{
		if((*it).actual && (*it).object_id==id)
			res.push_back((*it).location);
	}
	return (res.size()!=0);
}

u16 CMapManager::HasMapLocation(const shared_str& spot_type, u16 id)
{
	CMapLocation* l = GetMapLocation(spot_type, id);
	
	return (l)?l->RefCount():0;
}

CMapLocation* CMapManager::GetMapLocation(const shared_str& spot_type, u16 id)
{
	FindLocationBySpotID key(spot_type, id);
	Locations_it it = std::find_if(Locations().begin(),Locations().end(),key);
	if( it!=Locations().end() )
		return (*it).location;
	
	return 0;
}

void CMapManager::Update()
{
	Locations_it it = Locations().begin();
	Locations_it it_e = Locations().end();

	for(u32 idx=0; it!=it_e;++it,++idx)
	{
		bool bForce		= Device.dwFrame%3 == idx%3;
		(*it).actual	= (*it).location->Update();

		if((*it).actual && bForce)
			(*it).location->CalcPosition();
	}
	std::sort( Locations().begin(),Locations().end() );

	while( (!Locations().empty())&&(!Locations().back().actual) )
	{
		if(IsGameTypeSingle())
			Level().GameTaskManager().MapLocationRelcase(Locations().back().location);

		delete_data(Locations().back().location);
		Locations().pop_back();
	}
}

void CMapManager::DisableAllPointers()
{
	Locations_it it = Locations().begin();
	Locations_it it_e = Locations().end();

	for(; it!=it_e;++it)
		(*it).location->DisablePointer	();
}


Locations&	CMapManager::Locations	() 
{
	if(!m_locations)
	{
		m_locations = &m_locations_wrapper->registry().objects();
#ifdef DEBUG
		Msg("m_locations size=%d",m_locations->size());
#endif // #ifdef DEBUG
	}
	return *m_locations;
}

void CMapManager::OnObjectDestroyNotify(u16 id)
{
	RemoveMapLocationByObjectID(id);
}

#ifdef DEBUG
void CMapManager::Dump						()
{
	Msg("begin of map_locations dump");
	Locations_it it = Locations().begin();
	Locations_it it_e = Locations().end();
	for(; it!=it_e;++it)
	{
		Msg("spot_type=[%s] object_id=[%d]",*((*it).spot_type), (*it).object_id);
		(*it).location->Dump();
	}

	Msg("end of map_locations dump");
}
#endif