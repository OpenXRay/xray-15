/**********************************************************************
 *<
	FILE: StdDualVSImp.h

	DESCRIPTION: Standard Dual VertexShader helper class definition

	CREATED BY: Nikolai Sander, Discreet

	HISTORY: created 10/11/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "max.h"
#include "StdDualVSImp.h"
#include "ICustAttribContainer.h"
#include "CustAttrib.h"
#include <d3dx8.h>
#include "IViewportManager.h"

static StdDualVSImpClassDesc StdDualVSDesc;


ClassDesc2* GetStdDualVSImpDesc() {return &StdDualVSDesc;}


StdDualVSImp::~StdDualVSImp()
{
	DeleteAllRefsFromMe();
	caches.SetCount(0);
}

HRESULT StdDualVSImp::Initialize(Mesh *mesh, INode *node)
{
	HRESULT hr;
	hr = S_OK;

	if(!node)
		return E_FAIL;
	
	int index = FindNodeIndex(node);
	
	if( index < 0)
	{
		caches.SetCount(caches.Count()+1);
		caches[caches.Count()-1] = NULL;
		MakeRefByID(FOREVER,caches.Count()-1,node);

		index = caches.Count()-1;
	}
	
	if(!caches[index]->GetValid())
	{
		caches[index]->SetValid(true);
		hr = callb->InitValid(mesh, node);
	}
	return hr;
}

HRESULT StdDualVSImp::Initialize(MNMesh *mnmesh, INode *node)
{
	HRESULT hr;
	hr = S_OK;

	if(!node)
		return E_FAIL;
	
	int index = FindNodeIndex(node);
	
	if( index < 0)
	{
		caches.SetCount(caches.Count()+1);
		caches[caches.Count()-1] = NULL;
		MakeRefByID(FOREVER,caches.Count()-1,node);
		index = caches.Count()-1;
	}
	
	if(!caches[index]->GetValid())
	{
		caches[index]->SetValid(true);
		hr = callb->InitValid(mnmesh, node);
	}
	return hr;
}

RefResult StdDualVSImp::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message)
{
	switch (message) 
	{
		case REFMSG_TARGET_DELETED:
			{
				int i = FindNodeIndex((INode*)hTarget);
				assert(i >= 0);
				delete caches[i];
				caches[i] = NULL;
				// NH 07|11|02 The cache is no longer dynamic - it will only grow but not shrink, 
				// so there could be null entries.  This was causing a problem in SetReference
//				caches.Delete(i,1);
//				caches.Shrink();
			}
			break;
		case REFMSG_NODE_MATERIAL_CHANGED:
			{
				int i = FindNodeIndex((INode*)hTarget);
				assert(i >= 0);
				DeleteReference(i);
			}
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			{
				int i = FindNodeIndex((INode*)hTarget);
				//NH:06|11|02  I removed this, as in an undo situation the node reference may not be restored yet
				// as the viewport shader has not been initialised and thus not set the node.  In this case -1 is valid
//				assert(i >= 0);
				if(i >= 0)
					caches[i]->SetValid(false);
			}
			break;
		case REFMSG_SUBANIM_STRUCTURE_CHANGED :
			{
				BOOL del = TRUE;

				int i = FindNodeIndex((INode*)hTarget);
				INode * n = (INode*)hTarget;
				Mtl * m = n->GetMtl();
				if(m)
				{
					// NH|06|11|02 check for the Viewport Manager - if the Effect is the same as the callback then keep it
					ICustAttribContainer *cont = m->GetCustAttribContainer();
					if (cont && cont->GetNumCustAttribs()) {
						IDXShaderManagerInterface *sm = GetDXShaderManager();
						if (!sm) 
						{
							break;
						}

						for (int kk = 0; kk < cont->GetNumCustAttribs(); kk++) {
							CustAttrib *ca = cont->GetCustAttrib(kk);
							IViewportShaderManager *manager = (IViewportShaderManager*)ca->GetInterface(VIEWPORT_SHADER_MANAGER_INTERFACE);
							if (manager) {

								ReferenceTarget *rt = manager->GetActiveEffect();
								if(rt == callb->GetRefTarg())
									del = FALSE;

							}
						}
					}
				}
				// Check, if there's still a reference path from the node to the MtlBase
				if(!DependsOn(hTarget,callb->GetRefTarg()) && del)
				{
					DeleteReference(i);
				}
			}
	}
	return REF_SUCCEED;
}

void StdDualVSImp::SetReference(int i, RefTargetHandle rtarg)
{
	//NH|06|11|02 - Need to be very careful of undo - as the cache array is dynamic
	// on undo we could actually overwrite data then we are in big trouble.

	if(i < caches.Count() && i >= 0)
	{
		if(!caches[i])
			caches[i] = callb->CreateVertexShaderCache();
		
		if(rtarg == NULL)
		{
			delete caches[i];
			caches[i] = NULL;
			//NH|06|11|02 - Need to be careful here, as the NumRefs returns caches.Count()
			// so decresing it here will mean some reference are not removed - must do it externally
//			caches.Delete(i,1);
		}
		else
			caches[i]->SetNode((INode *) rtarg);
	}
	else
	{
		//NH: 06|11|02
		// this as far as I can tell will only happen in an undo situation.  We need to patch the data back in
		// so mulitple references are not made
		caches.SetCount(caches.Count()+1);
		caches[i] = callb->CreateVertexShaderCache();
		caches[i]->SetNode((INode *) rtarg);

	}

}

RefTargetHandle StdDualVSImp::GetReference(int i)
{
	if(i < caches.Count() && i >= 0 && caches[i])
		return caches[i]->GetNode();
	else
		return NULL;
}

int StdDualVSImp::FindNodeIndex(INode *node)
{
	int num = caches.Count();
	for(int i = 0 ; i < num; i++)
	{
		if(caches[i] && caches[i]->GetNode() == node)
			return i;
	}
	return -1;
}