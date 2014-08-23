/*==============================================================================

  file:	        Enumeration.cpp

  author:       Daniel Levesque
  
  created:	    18 February 2002
  
  description:
        
          Enumeration classes used by this utility.

  modified:	


© 2002 Autodesk
==============================================================================*/
#include "Enumeration.h"

SceneEnumerator::Result __fastcall SceneEnumerator::enumerate(INode* node)
{
	if (node==NULL) 
		return kContinue;
    Result res;
    if (!node->IsRootNode()) {
	    res = callback(node);
        if (res != kContinue)
		    return res;
    }
    int nbChildren = node->NumberOfChildren();
	for (int i = 0; i < nbChildren; ++i) {
		res = enumerate(node->GetChildNode(i));
        if (res != kContinue)
			return res;
    }
	return kContinue;
}