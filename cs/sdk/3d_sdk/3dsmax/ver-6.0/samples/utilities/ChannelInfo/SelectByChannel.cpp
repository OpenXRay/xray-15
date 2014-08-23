#include "TreeViewUtil.h"




class SelectByChannelClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new SelectByChannel(); }
	const TCHAR *	ClassName() { return GetString(IDS_SELECTBYCHANNELNAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return SELECTBYCHANNEL_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("SelectByChannel"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static SelectByChannelClassDesc SelectByChannelDesc;
ClassDesc2* GetSelectByChannelDesc() { return &SelectByChannelDesc; }


enum { selectbychannel_params };


//TODO: Add enums for various parameters
enum { 
	pb_seltype,
	pb_mapid,pb_mapsubid
	
};



static ParamBlockDesc2 selectbychannel_param_blk ( selectbychannel_params, _T("params"),  0, &SelectByChannelDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_SELECTBYCHANNELPANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	pb_seltype, 			_T("selectionType"), 		TYPE_INT, 	0, 	IDS_SELTYPE, 
		p_default, 		0, 
		p_ui, TYPE_INTLISTBOX, IDC_SELTYPECOMBO, 3, IDS_REPLACE,
												 IDS_ADD,
												 IDS_SUBTRACT,
		end,

	pb_mapid, 		_T("mapID"), 		TYPE_INT, 	0, 	IDS_MAPID, 
//		p_range,	0.0f,255.0f,
		end,

	pb_mapsubid, 		_T("mapSubID"), 		TYPE_INT, 	0, 	IDS_MAPSUBID, 
//		p_range,	0,2,
		end,


	end
	);


int MyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
//7-1-99
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
		Nodes.Append(1, (INode **)&rmaker);  
		count++;
		}


	return 0;
	}


BOOL SelectByChannelDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			mod->hWnd = hWnd;
			mod->FillOutListBox();
			break;
		case WM_COMMAND:
			{
			switch (LOWORD(wParam)) 
				{

				case IDC_SELCHANNELCOMBO:
					{
					if (HIWORD(wParam)==CBN_SELCHANGE)
						{
						int fsel;
						fsel = SendMessage(
							GetDlgItem(hWnd,IDC_SELCHANNELCOMBO),
							CB_GETCURSEL,0,0);	
						if ( (fsel >=0 ) && ( fsel < mod->channelList.Count()) )
							{
							int id,subid;
							id = mod->channelList[fsel].mapID;
							subid = mod->channelList[fsel].subID;
							mod->pblock->SetValue(pb_mapid,0,id);
							mod->pblock->SetValue(pb_mapsubid,0,subid);
							}
						}
					break;
					}

				}
			}

			break;

		}
	return FALSE;
	}


IObjParam *SelectByChannel::ip			= NULL;


//--- SelectByChannel -------------------------------------------------------
SelectByChannel::SelectByChannel()
{
	SelectByChannelDesc.MakeAutoParamBlocks(this);

}

SelectByChannel::~SelectByChannel()
{
}

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval SelectByChannel::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
	return NEVER;
}


/*************************************************************************************************
*
	Between NotifyPreCollapse and NotifyPostCollapse, Modify is
	called by the system.  NotifyPreCollapse can be used to save any plugin dependant data e.g.
	LocalModData
*
\*************************************************************************************************/

void SelectByChannel::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	//TODO:  Perform any Pre Stack Collapse methods here
}



/*************************************************************************************************
*
	NotifyPostCollapse can be used to apply the modifier back onto to the stack, copying over the
	stored data from the temporary storage.  To reapply the modifier the following code can be 
	used

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID)
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());

*
\*************************************************************************************************/

void SelectByChannel::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	//TODO: Perform any Post Stack collapse methods here.

}


/*************************************************************************************************
*
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/


void SelectByChannel::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	//TODO: Add the code for actually modifying the object

	//get th map id

	
//	TriObject *tobj = (TriObject*)os->obj;
//	Mesh &mesh = tobj->GetMesh();		

	Mesh *mesh = NULL;
	MNMesh *mnmesh = NULL;
		
	TriObject *collapsedtobj = NULL;
	if (os->obj->IsSubClassOf(triObjectClassID))
	{
		TriObject *tobj = (TriObject*)os->obj;
		mesh = &tobj->GetMesh();
	}
	else if (os->obj->IsSubClassOf(polyObjectClassID))
	{
		PolyObject *pobj = (PolyObject*)os->obj;
		mnmesh = &pobj->GetMesh();
	}



	int mapID;
	int subID;

	int selType;

	pblock->GetValue(pb_mapid,0,mapID,FOREVER);
	pblock->GetValue(pb_mapsubid,0,subID,FOREVER);

	pblock->GetValue(pb_seltype,0,selType,FOREVER);

	if (subID < 0) subID = 0;
	if (subID > 2) subID = 2;
	
	
	if (mnmesh)
	{
		
		int numMaps = mnmesh->numm;

		mnmesh->dispFlags = MNDISP_VERTTICKS|MNDISP_SELVERTS ;
		mnmesh->selLevel = MNM_SL_VERTEX;

		if (mapID < numMaps)
		
		{
			UVVert *uvw = mnmesh->M(mapID)->v;
			MNMapFace *uvwFace = mnmesh->M(mapID)->f;
			if (uvw && uvwFace)
				
			{
				float *vsw = NULL;

				MNFace *face = mnmesh->f;

					

				vsw = mnmesh->getVSelectionWeights ();

				if (vsw == NULL)
				{
					mnmesh->SupportVSelectionWeights();
					vsw = mnmesh->getVSelectionWeights ();
				}

				BitArray processed;
				processed.SetSize(mnmesh->numv);
				processed.ClearAll();

				if (vsw && uvwFace && uvw)
				{
					if (selType == 0)
					{
						mnmesh->ClearVFlags (MN_SEL);
						for (int i = 0; i < mnmesh->numv; i++)
							vsw[i] = 0.0f;
					}

					for (int i = 0; i < mnmesh->numf; i++)
					{
						int deg = face[i].deg;
						for (int j = 0; j < deg; j++)
						{
							
							int index = uvwFace[i].tv[j];
							int gindex = face[i].vtx[j];
							if (!processed[gindex])
							{
								processed.Set(gindex);
								if (selType == 0)
								{	
									float w = uvw[index][subID];
									if (w >= 1.0f)
										mnmesh->v[gindex].SetFlag (MN_SEL);
//										mesh.vertSel.Set(gindex);
									else vsw[gindex] = uvw[index][subID];
								}
								else if (selType == 1)
								{	
									float w = uvw[index][subID];
									w += vsw[gindex];
									if (w >= 1.0f)
										mnmesh->v[gindex].SetFlag (MN_SEL);
//										mesh.vertSel.Set(gindex);
									else vsw[gindex] = uvw[index][subID];
								}
								else if (selType == 2)
								{	
									float w = uvw[index][subID];
									if (mnmesh->v[gindex].GetFlag (MN_SEL))//(mesh.vertSel[gindex])
										w = 1.0f - w;
									else w = vsw[gindex] - w;;
									if (w < 1.0f)
										mnmesh->v[gindex].ClearFlag (MN_SEL);
//										mesh.vertSel.Set(gindex,FALSE);
									vsw[gindex] = w;
								}
							}

						}
					}
				}
			}

		}
		
	}

	else if (mesh)
	{
		mesh->dispFlags = DISP_VERTTICKS|DISP_SELVERTS;
		mesh->selLevel = MESH_VERTEX;

		if (mesh->mapSupport(mapID))
		{
			UVVert *uvw = mesh->mapVerts(mapID);
			TVFace *uvwFace = mesh->mapFaces(mapID);	
			float *vsw = NULL;

			Face *face = mesh->faces;

				

			vsw = mesh->getVSelectionWeights ();

			if (vsw == NULL)
			{
				mesh->SupportVSelectionWeights();
				vsw = mesh->getVSelectionWeights ();
			}

			BitArray processed;
			processed.SetSize(mesh->numVerts);
			processed.ClearAll();

			if (vsw && uvwFace && uvw)
			{
				if (selType == 0)
				{
					mesh->vertSel.ClearAll();
					for (int i = 0; i < mesh->numVerts; i++)
						vsw[i] = 0.0f;
				}

				for (int i = 0; i < mesh->numFaces; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						
						int index = uvwFace[i].t[j];
						int gindex = face[i].v[j];
						if (!processed[gindex])
						{
							processed.Set(gindex);
							if (selType == 0)
							{	
								float w = uvw[index][subID];
								if (w >= 1.0f)
									mesh->vertSel.Set(gindex);
								else vsw[gindex] = uvw[index][subID];
							}
							else if (selType == 1)
							{	
								float w = uvw[index][subID];
								w += vsw[gindex];
								if (w >= 1.0f)
									mesh->vertSel.Set(gindex);
								else vsw[gindex] = uvw[index][subID];
							}
							else if (selType == 2)
							{	
								float w = uvw[index][subID];
								if (mesh->vertSel[gindex])
									w = 1.0f - w;
								else w = vsw[gindex] - w;;
								if (w < 1.0f)
									mesh->vertSel.Set(gindex,FALSE);
								vsw[gindex] = w;
							}
						}

					}
				}
			}

		}
	}
	


	Interval iv;
	iv = FOREVER;

	os->obj->PointsWereChanged();

	iv = iv & os->obj->ChannelValidity(t,GEOM_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,TOPO_CHAN_NUM);
	iv = iv & os->obj->ChannelValidity(t,SELECT_CHAN_NUM);

	os->obj->UpdateValidity (SELECT_CHAN_NUM, iv);


}


void SelectByChannel::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	SelectByChannelDesc.BeginEditParams(ip, this, flags, prev);

	selectbychannel_param_blk.SetUserDlgProc(selectbychannel_params,new SelectByChannelDlgProc(this));

}

void SelectByChannel::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	SelectByChannelDesc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;


}



Interval SelectByChannel::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle SelectByChannel::Clone(RemapDir& remap)
{
	SelectByChannel* newmod = new SelectByChannel();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult SelectByChannel::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}

/****************************************************************************************
*
 	NotifyInputChanged is called each time the input object is changed in some way
 	We can find out how it was changed by checking partID and message
*
\****************************************************************************************/

void SelectByChannel::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

}



//From Object
BOOL SelectByChannel::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void SelectByChannel::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin internal value to sw				
}

IOResult SelectByChannel::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult SelectByChannel::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	return IO_OK;
	
}



void SelectByChannel::FillOutListBox()
{


	int mapID;
	int subID;

	pblock->GetValue(pb_mapid,0,mapID,FOREVER);
	pblock->GetValue(pb_mapsubid,0,subID,FOREVER);

	MyEnumProc dep;              
	EnumDependents(&dep);

	INode *node = dep.Nodes[0];
	//reset the list box

	SendMessage(GetDlgItem(hWnd,IDC_SELCHANNELCOMBO),
				CB_RESETCONTENT ,0,0);

	channelList.ZeroCount();
	if (dep.Nodes.Count() > 0)
		{
		for (int i = 0; i < 99; i++)
			{
			for (int j = 0; j < 3; j++)
				{
				TSTR key, name;
				key.printf("MapChannel:%d(%d)",i,j);
				if (node->UserPropExists(key))
					{
					node->GetUserPropString(key,name);
					if (name.Length() > 0)
						{
						TSTR lname;
						if (j==0)
							lname.printf("%s %d-Red",name,i);
						if (j==1)
							lname.printf("%s %d-Green",name,i);
						if (j==2)
							lname.printf("%s %d-Blue",name,i);


						SendMessage(GetDlgItem(hWnd,IDC_SELCHANNELCOMBO),
							CB_ADDSTRING,0,(LPARAM)(TCHAR*)lname);					

						ChannelInfo data;
						data.mapID = i;
						data.subID = j;

						channelList.Append(1,&data);
						}
					}
				}
			}
		}

	int sel = -1;
	for (int i = 0; i < channelList.Count(); i++)
		{
		if ((mapID == channelList[i].mapID) && (subID == channelList[i].subID))
			sel = i;
		}

	SendMessage(GetDlgItem(hWnd,IDC_SELCHANNELCOMBO),CB_SETCURSEL  ,sel,0);


}