/**********************************************************************
 *<
	FILE: PaintDeformTest.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "PaintDeformTest.h"
#include "modstack.h"


static PaintDeformTestClassDesc PaintDeformTestDesc;
ClassDesc2* GetPaintDeformTestDesc() { return &PaintDeformTestDesc; }


enum { paintdeformtest_params };


//TODO: Add enums for various parameters
enum { 
	pb_spin,
};



static ParamBlockDesc2 paintdeformtest_param_blk ( paintdeformtest_params, _T("params"),  0, &PaintDeformTestDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params


	end
	);




IObjParam *PaintDeformTest::ip			= NULL;

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



//--- PaintDeformTest -------------------------------------------------------
PaintDeformTest::PaintDeformTest()
{
	PaintDeformTestDesc.MakeAutoParamBlocks(this);
}

PaintDeformTest::~PaintDeformTest()
{
}

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval PaintDeformTest::LocalValidity(TimeValue t)
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
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/


void PaintDeformTest::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	//TODO: Add the code for actually modifying the object
	PaintDefromModData *pmd = NULL;
	if (mc.localData==NULL)
		{
		pmd = new PaintDefromModData();
		mc.localData = (LocalModData *) pmd;
		}
	else pmd = (PaintDefromModData *) mc.localData;

	int ct = os->obj->NumPoints();
	if (pmd->offsetList.Count() != ct)
		{
		
		pmd->offsetList.SetCount(ct);
		for (int i = 0; i < ct; i++)
			pmd->offsetList[i] = Point3(0.0f,0.0f,0.0f);
		}

	for (int i = 0; i < ct; i++)
		{
		Point3 p = os->obj->GetPoint(i);
		p += pmd->offsetList[i];
		os->obj->SetPoint(i,p);
		}

	os->obj->UpdateValidity(GEOM_CHAN_NUM,FOREVER);
}


void PaintDeformTest::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	PaintDeformTestDesc.BeginEditParams(ip, this, flags, prev);
	paintdeformtest_param_blk.SetUserDlgProc(new PaintDeformTestDlgProc(this));

	pPainter = NULL;

	ReferenceTarget *painterRef = (ReferenceTarget *) GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID);
	
//set it to the correct verion
	if (painterRef)
		{
		pPainter = (IPainterInterface_V5 *) painterRef->GetInterface(PAINTERINTERFACE_V5);
		}

	TimeValue t = ip->GetTime();

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	

}

void PaintDeformTest::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	PaintDeformTestDesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	TimeValue t = ip->GetTime();

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
}



Interval PaintDeformTest::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}




RefTargetHandle PaintDeformTest::Clone(RemapDir& remap)
{
	PaintDeformTest* newmod = new PaintDeformTest();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult PaintDeformTest::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}




//just some function to handle UI creatation and destruction
void PaintDeformTest::InitUI(HWND hWnd)
{
	iPaintButton = GetICustButton(GetDlgItem(hWnd,IDC_PAINT));
	iPaintButton->SetType(CBT_CHECK);
	iPaintButton->SetHighlightColor(GREEN_WASH);
}
void PaintDeformTest::DestroyUI(HWND hWnd)
{
	ReleaseICustButton(iPaintButton);
	iPaintButton = NULL;
	if (pPainter)
		{
		if (pPainter->InPaintMode() )
			pPainter->EndPaintSession();
		}
}


PaintDefromModData *PaintDeformTest::GetPMD(INode *pNode)
{
	ModContext *mc = NULL;

	Object* pObj = pNode->GetObjectRef();

	if (!pObj) return NULL;

	
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID && mc == NULL)
	{
		IDerivedObject* pDerObj = (IDerivedObject *)(pObj);
			
		int Idx = 0;

		while (Idx < pDerObj->NumModifiers())
		{
			// Get the modifier. 
			Modifier* mod = pDerObj->GetModifier(Idx);

			
			if (mod->ClassID() == PAINTDEFORMTEST_CLASS_ID)
			{
				// is this the correct Physique Modifier based on index?
				PaintDeformTest *pmod = (PaintDeformTest*)mod;
				if (pmod == this)
					{
					mc = pDerObj->GetModContext(Idx);
					break;
					}
			}

			Idx++;
		}

		pObj = pDerObj->GetObjRef();
	}

	if(!mc) return NULL;

	if ( !mc->localData ) return NULL;

	PaintDefromModData *bmd = (PaintDefromModData *) mc->localData;

	return bmd;

}

//Some helper functions to handle painter UI
void PaintDeformTest::Paint()
{
	if (pPainter) //need to check this since some one could have taken the painterinterface plugin out
		{
		if (!pPainter->InPaintMode())  
			{
			pPainter->InitializeCallback(this); //initialize the callback
			//load up our nodes
			//gather up all our nodes and local data and store em off
			MyEnumProc dep;              
			EnumDependents(&dep);
			Tab<INode *> nodes;
			painterNodeList.ZeroCount();
			for (int  i = 0; i < dep.Nodes.Count(); i++)
				{
				PaintDefromModData *pmd = GetPMD(dep.Nodes[i]);

				if (pmd)
					{
					nodes.Append(1,&dep.Nodes[i]);
					PainterNodeList temp;
					temp.node = dep.Nodes[i];
					temp.pmd = pmd;
					temp.tmToLocalSpace = Inverse(dep.Nodes[i]->GetObjectTM(GetCOREInterface()->GetTime()));
					painterNodeList.Append(1,&temp);
					}
				}

//we use the point gather so we need to tell the system to turn it on
			pPainter->SetEnablePointGather(TRUE);
			pPainter->SetBuildNormalData(TRUE);

//this sends all our dependant nodes to the painter
			pPainter->InitializeNodes(0, nodes);

			BOOL updateMesh = FALSE;
			for (i = 0; i < nodes.Count(); i++)
				{

				ObjectState os = nodes[i]->EvalWorldState(GetCOREInterface()->GetTime());
				
				int ct = os.obj->NumPoints();
//is our local vertex count does not match or the curremt object count we have some
//sort of topo change above us so we need to load a custom point list
//We really need add normals here
//so right now this does not work with patches, nurbs or things that have different  topos at the top
//top of the stack
				if (os.obj->NumPoints() != painterNodeList[i].pmd->offsetList.Count())
					{
					Tab<Point3> pointList;
					pointList.SetCount(ct);
					Matrix3 tm = nodes[i]->GetObjectTM(GetCOREInterface()->GetTime());
					for (int j =0; j < ct; j++)
						{
						pointList[j] = os.obj->GetPoint(j)*tm;
						}
					pPainter->LoadCustomPointGather(ct, pointList.Addr(0), nodes[i]);
					updateMesh = TRUE;
					}

				}
//reinitialize our nodes if we had a custom list
			if (updateMesh)
				pPainter->UpdateMeshes(TRUE);

			pPainter->StartPaintSession(); //start the paint session
			iPaintButton->SetCheck(TRUE);
			}
		else //we are currently in a paint mode so turn it off
			{
			pPainter->EndPaintSession(); //end the paint session
			iPaintButton->SetCheck(FALSE);
			}
		}

}

void PaintDeformTest::PaintOptions()
{
	if (pPainter) //need to check this since some one could have taken the painterinterface plugin out
		{
		pPainter->BringUpOptions();
		}

}


#define POINTCOUNT_CHUNK 0x1000
#define POINT_CHUNK 0x1010

IOResult PaintDeformTest::SaveLocalData(ISave *isave, LocalModData *pld)
{

	ULONG		nb;

	PaintDefromModData *pmd = (PaintDefromModData*)pld;





	int ct = pmd->offsetList.Count();

	isave->BeginChunk(POINTCOUNT_CHUNK);
	isave->Write(&ct, sizeof(ct),&nb);
	isave->EndChunk();

	isave->BeginChunk(POINT_CHUNK);
	isave->Write(pmd->offsetList.Addr(0), sizeof(Point3)*ct,&nb);
	isave->EndChunk();
	return IO_OK;
}

IOResult PaintDeformTest::LoadLocalData(ILoad *iload, LocalModData **pld)

{
	IOResult	res;
	ULONG		nb;

	PaintDefromModData *pmd = new PaintDefromModData();
	*pld = pmd;


	int ct = 0;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case POINTCOUNT_CHUNK: 
				{
				iload->Read(&ct,sizeof(ct), &nb);
				pmd->offsetList.SetCount(ct);
				break;
				}			
			case POINT_CHUNK: 
				{
				iload->Read(pmd->offsetList.Addr(0),sizeof(Point3)*ct, &nb);
				break;
				}
			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}





	return IO_OK;
}


void* PaintDeformTest::GetInterface(ULONG id)
{
	switch(id)
	{
		case PAINTERCANVASINTERFACE_V5 : return (IPainterCanvasInterface_V5 *) this;
			break;
		default: return Modifier::GetInterface(id);
			break;
	}
}
