#include "BipedApi.h"
#include "Tracks.h"


void IterateThroughBipSlave(IBipMaster *bipMaster, Control *c,int id,int link);


//the node passed in can be any biped node
void IterateThroughKeys(INode *node)
{
	if (node)
	{
		// Get the node's transform control
		Control *c = node->GetTMController();
		int id,link; //the id and link of the node
		// You can test whether or not this is a biped controller with the following pseudo code:
		if (c&&(c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID))
		{
			IBipMaster *bipMaster = GetBipMasterInterface(c);
			if(bipMaster)
			{
				BOOL doubleCheck =  bipMaster->GetIdLink(node, id,link);
				if(doubleCheck)
					IterateThroughBipSlave(bipMaster,c,id,link);
			}
		}
		else if(c&&c->ClassID() == BIPBODY_CONTROL_CLASS_ID) //this is the COM
		{
			IBipMaster *bipMaster = GetBipMasterInterface(c);
			//the COM has 3 sub controllers for the horz,vert and turn tracks.
			if(bipMaster)
			{
				Control * cCOM;
				cCOM = bipMaster->GetVerticalControl(); //this is the vertical 
				IterateThroughBipSlave(bipMaster,cCOM,KEY_VERTICAL,0);
				cCOM = bipMaster->GetHorizontalControl(); //this is the horizontal 
				IterateThroughBipSlave(bipMaster,cCOM,KEY_HORIZONTAL,0);
				cCOM = bipMaster->GetTurnControl(); //this is the turn 
				IterateThroughBipSlave(bipMaster,cCOM,KEY_TURN,0);
			}
		}
	}
}


void IterateThroughBipSlave(IBipMaster *bipMaster, Control *c,int id,int link)
{
	//NOTE :the link isn't used here but is being passed in for completeness (since an id and link together form a biped node).
	//NOTE : Another thing to check is whether or not the biped part is seperated or not. BOOL IBipMaster::GetSeparateTracks(int id);
	//If it isn't, then we only need to get the TCBs for the first link(id,0), since the keys are shared
	if(bipMaster)
	{
		//get the key based upon the biped body part type
		IBipedKey *key = NULL; //we use the biped key base since that base class holds the  tcb values
		IBipedVertKey vertKey;
		IBipedHorzKey horzKey;
		IBipedTurnKey turnKey;
		IBipedBodyKey bodyKey;
		IBipedHeadKey headKey;
		IBipedPropKey propKey;

		// Now set the key-specific properties
		switch(id)
		{
			case KEY_VERTICAL:
				key = &vertKey;
				break;
			case KEY_HORIZONTAL:
				key = &horzKey;
				break;
			case KEY_TURN:
				key = & turnKey;
				break;
			case KEY_LARM:  case KEY_RARM:		
			case KEY_LHAND: case KEY_RHAND:
			case KEY_LLEG:  case KEY_RLEG:
			case KEY_LFOOT: case KEY_RFOOT:
				key = & bodyKey;
				break;
			case KEY_HEAD:
				key = &headKey;
				break;
			case KEY_PROP1:
			case KEY_PROP2:
			case KEY_PROP3:
				key = &propKey;
				break;
			default: 
				//not good
				break;
		}
		IKeyControl* keyInterface = GetKeyControlInterface(c);
		if (keyInterface != NULL)
		{
			int numKeys = keyInterface->GetNumKeys();
			for(int i =0;i<numKeys;++i)
			{
				keyInterface->GetKey(i, key);
				TimeValue time = c->GetKeyTime(i); 
				float tens = (key->tens-25.0f)/25.0f; //put's value between -1 and 1
				float cont = (key->cont-25.0f)/25.0f;
				float bias = (key->bias-25.0f)/25.0f;
				float easeIn = key->easeIn;
				float easeOut = key->easeOut;
			}
		}
	}
}