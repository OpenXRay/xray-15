//-------------------------------------------------------------
// Access to the Camera Map modifier
//
#include "iFnPub.h"

/***************************************************************
Function Publishing System stuff   
****************************************************************/

#define CLUSTNODE_MOD_INTERFACE Interface_ID(0x2f6d169b, 0x7c211e3f)
#define GetIClustNodeModInterface(cd) \
			(IClustNodeMod *)(cd)->GetInterface(CLUSTNODE_MOD_INTERFACE)

//****************************************************************


class IClustNodeMod : public OSModifier, public FPMixinInterface {
	public:
		
		//Function Publishing System
		enum {  get_control_node, set_control_node,  
				};
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			PROP_TFNS(get_control_node, getControlNode, set_control_node, setControlNode, TYPE_INODE);

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//**************************************************

		virtual INode*	getControlNode(TimeValue t)=0;
		virtual void	setControlNode(INode* cam, TimeValue t)=0;
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
