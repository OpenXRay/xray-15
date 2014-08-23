//-------------------------------------------------------------
// Access to the Camera Map modifier
//
#include "iFnPub.h"

// The unique ClassIDs
#define OBJ_CLASS_ID		Class_ID(0x1cfc3603, 0x40282400)
#define WSMOD_CLASS_ID		Class_ID(0x16dc4353, 0x2dc047b1)
#define MOD_CLASS_ID		Class_ID(0x21ce6ea4, 0x7ed434db)

/***************************************************************
Function Publishing System stuff   
****************************************************************/

#define CAMMAP_WSM_INTERFACE Interface_ID(0x2f1d2c9b, 0x7c141d3e)
#define GetICamMapWSMInterface(cd) \
			(ICamMapWSMod *)(cd)->GetInterface(CAMMAP_WSM_INTERFACE)

#define CAMMAP_MOD_INTERFACE Interface_ID(0x6c3b67a1, 0x769b2299)
#define GetICamMapModInterface(cd) \
			(ICamMapMod *)(cd)->GetInterface(CAMMAP_MOD_INTERFACE)

//****************************************************************


class ICamMapWSMod : public WSModifier, public FPMixinInterface {
	public:
		
		//Function Publishing System
		enum {  get_cam_node, set_cam_node, get_map_channel, set_map_channel, 
				};
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			PROP_FNS(get_cam_node, getCamNode, set_cam_node, setCamNode, TYPE_INODE);
			PROP_FNS(get_map_channel, getMapChannel, set_map_channel, setMapChannel, TYPE_INT);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//**************************************************

		virtual INode*	getCamNode()=0;
		virtual void	setCamNode(INode* cam)=0;
		virtual int		getMapChannel()=0;
		virtual void	setMapChannel(int mapChan)=0;
	};

class ICamMapMod : public Modifier, public FPMixinInterface {
	public:
		
		//Function Publishing System
		enum {  get_cam_node, set_cam_node, get_channel, set_channel, get_map_channel, set_map_channel, 
				};
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			PROP_TFNS(get_cam_node, getCamNode, set_cam_node, setCamNode, TYPE_INODE);
			PROP_FNS(get_map_channel, getMapChannel, set_map_channel, setMapChannel, TYPE_INT);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//**************************************************

		virtual INode*	getCamNode(TimeValue t=0)=0;
		virtual void	setCamNode(INode* cam, TimeValue t)=0;
		virtual int		getMapChannel()=0;
		virtual void	setMapChannel(int mapChan)=0;
	};


// ref enumerator to find a scene base object's node
class FindSceneObjNodeDEP : public DependentEnumProc 
{
public:		
	INode*			node;
	ReferenceTarget* dobj;
	ReferenceTarget* cur_obj;
	FindSceneObjNodeDEP(ReferenceTarget* obj) { node = NULL; dobj = NULL; cur_obj = obj; }
	int proc(ReferenceMaker* rmaker)
	{
		if (rmaker == cur_obj)
			return 0;
		// hunt for either a node or derived object
		SClass_ID sid = rmaker->SuperClassID();
		if (sid == BASENODE_CLASS_ID)
		{
			node = (INode*)rmaker;
			return 1;
		}
		else if (sid == GEN_DERIVOB_CLASS_ID ||  
				 sid == DERIVOB_CLASS_ID ||   
				 sid == WSM_DERIVOB_CLASS_ID ||
				 sid == MODAPP_CLASS_ID ||
				 sid == OBREF_MODAPP_CLASS_ID)    
		{
//			dobj = (ReferenceTarget*)rmaker;
//			return 1;
			return 0;
		}
		else 
			return DEP_ENUM_SKIP;
	}
};

static INode*
find_scene_obj_node(ReferenceTarget* obj)
{
	// given a scene base object or modifier, look for a referencing node via successive 
	// reference enumerations up through any intervening mod stack entries
	FindSceneObjNodeDEP fsno (obj);
	obj->EnumDependents(&fsno);
	while (fsno.dobj != NULL)
	{
		// found a mod stack, wind up through any derived objs
		fsno.cur_obj = fsno.dobj;
		fsno.dobj = NULL;
		fsno.cur_obj->EnumDependents(&fsno);
	}
	return fsno.node;
}
