/**********************************************************************
 *<
	FILE: SkinTools.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 




  	//add holds to the skin weights


	
	//add vertex selection exposure
	//add vertex property exposure
	//add bone selection exposure


 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "SkinTools.h"
#include "..\RayMeshIntersect\IRayMeshGridIntersect.h"
#define SKINTOOLS_CLASS_ID	Class_ID(0x5135cd12, 0x5a8c1cd9)






//--- Publish bone creation -------------------------------------------------------------------

#define SKINTOOLS_FP_INTERFACE_ID Interface_ID(0x549a3355, 0xef977711)

class SkinToolsFunctionPublish : public FPStaticInterface {
	public:
		DECLARE_DESCRIPTOR(SkinToolsFunctionPublish);

		enum OpID {
			kExtractData,
			kPasteData,
			kGetBoneBindTM,
			kSetBoneBindTM,
			kGetMeshBindTM,
			kSetMeshBindTM,

			kGetBoneStretchTM,
			kSetBoneStretchTM,
			kGrowSel,kShrinkSel,
			kLoopSel,kRingSel
			
			};
		
		BEGIN_FUNCTION_MAP			

			VFN_1(kExtractData,  ExtractData, TYPE_INODE)
			VFN_2(kPasteData,  PasteData, TYPE_INODE,TYPE_INODE)

			FN_2(kGetBoneBindTM,  TYPE_MATRIX3_BV, GetBoneBindTM, TYPE_INODE,TYPE_INODE)
			VFN_3(kSetBoneBindTM,  SetBoneBindTM, TYPE_INODE,TYPE_INODE,TYPE_MATRIX3_BV)

			FN_1(kGetMeshBindTM,  TYPE_MATRIX3_BV, GetMeshBindTM, TYPE_INODE)
			VFN_2(kSetMeshBindTM,  SetMeshBindTM, TYPE_INODE,TYPE_MATRIX3_BV)

			FN_2(kGetBoneStretchTM,  TYPE_MATRIX3_BV, GetBoneStretchTM, TYPE_INODE,TYPE_INODE)
			VFN_3(kSetBoneStretchTM,  SetBoneStretchTM, TYPE_INODE,TYPE_INODE,TYPE_MATRIX3_BV)
			
			VFN_1(kGrowSel,  GrowSel, TYPE_INODE)
			VFN_1(kShrinkSel,  ShrinkSel, TYPE_INODE)

			VFN_1(kLoopSel,  LoopSel, TYPE_INODE)
			VFN_1(kRingSel,  RingSel, TYPE_INODE)
			
		END_FUNCTION_MAP

		
		void ExtractData(INode *node);
		void PasteData(INode *tnode,INode *snode);
		
		Matrix3 GetBoneBindTM(INode *skinNode, INode *boneNode);
		void SetBoneBindTM(INode *skinNode, INode *boneNode, Matrix3 tm);

		Matrix3 GetMeshBindTM(INode *skinNode );
		void SetMeshBindTM(INode *skinNode, Matrix3 tm);
		
		Matrix3 GetBoneStretchTM(INode *skinNode, INode *boneNode);
		void SetBoneStretchTM(INode *skinNode, INode *boneNode, Matrix3 tm);
		
		void GrowSel(INode *skinNode);
		void ShrinkSel(INode *skinNode);

		void LoopSel(INode *skinNode);
		void RingSel(INode *skinNode);

	};

static SkinToolsFunctionPublish theSkinToolsFunctionPublish(
	SKINTOOLS_FP_INTERFACE_ID, _T("SkinUtils"), -1, 0, FP_CORE,
	// The first operation, boneCreate:
	SkinToolsFunctionPublish::kExtractData, _T("ExtractSkinData"), -1, TYPE_VOID, 0, 1,
		_T("node"), -1, TYPE_INODE,
	SkinToolsFunctionPublish::kPasteData, _T("ImportSkinData"), -1, TYPE_VOID, 0, 2,
		_T("targetNode"), -1, TYPE_INODE,
		_T("sourceNode"), -1, TYPE_INODE,

	SkinToolsFunctionPublish::kGetBoneBindTM, _T("GetBoneBindTM"), -1, TYPE_MATRIX3_BV, 0, 2,
		_T("skinNode"), -1, TYPE_INODE,
		_T("boneNode"), -1, TYPE_INODE,
	SkinToolsFunctionPublish::kSetBoneBindTM, _T("SetBoneBindTM"), -1, TYPE_VOID, 0, 3,
		_T("skinNode"), -1, TYPE_INODE,
		_T("boneNode"), -1, TYPE_INODE,
		_T("tm"), -1, TYPE_MATRIX3_BV,

	SkinToolsFunctionPublish::kGetMeshBindTM, _T("GetMeshBindTM"), -1, TYPE_MATRIX3_BV, 0, 1,
		_T("skinNode"), -1, TYPE_INODE,
	SkinToolsFunctionPublish::kSetMeshBindTM, _T("SetMeshBindTM"), -1, TYPE_VOID, 0, 2,
		_T("skinNode"), -1, TYPE_INODE,
		_T("tm"), -1, TYPE_MATRIX3_BV,


	SkinToolsFunctionPublish::kGetBoneStretchTM, _T("GetBoneStretchTM"), -1, TYPE_MATRIX3_BV, 0, 2,
		_T("skinNode"), -1, TYPE_INODE,
		_T("boneNode"), -1, TYPE_INODE,
	SkinToolsFunctionPublish::kSetBoneStretchTM, _T("SetBoneStretchTM"), -1, TYPE_VOID, 0, 3,
		_T("skinNode"), -1, TYPE_INODE,
		_T("boneNode"), -1, TYPE_INODE,
		_T("tm"), -1, TYPE_MATRIX3_BV,

	SkinToolsFunctionPublish::kGrowSel, _T("GrowSelection"), -1, TYPE_VOID, 0, 1,
		_T("skinNode"), -1, TYPE_INODE,

	SkinToolsFunctionPublish::kShrinkSel, _T("ShrinkSelection"), -1, TYPE_VOID, 0, 1,
		_T("skinNode"), -1, TYPE_INODE,

	SkinToolsFunctionPublish::kLoopSel, _T("LoopSelection"), -1, TYPE_VOID, 0, 1,
		_T("skinNode"), -1, TYPE_INODE,

	SkinToolsFunctionPublish::kRingSel, _T("RingSelection"), -1, TYPE_VOID, 0, 1,
		_T("skinNode"), -1, TYPE_INODE,

	end);



static SkinTools theSkinTools;

class SkinToolsClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theSkinTools; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return SKINTOOLS_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("SkinTools"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static SkinToolsClassDesc SkinToolsDesc;
ClassDesc2* GetSkinToolsDesc() { return &SkinToolsDesc; }



void SkinToolsFunctionPublish::ExtractData(INode *node)
{
	theSkinTools.ExtractData(node);
}

void SkinToolsFunctionPublish::PasteData(INode *tnode,INode *snode)
{
	theSkinTools.PasteData (tnode,snode);
}

Matrix3 SkinToolsFunctionPublish::GetBoneBindTM(INode *skinNode, INode *boneNode)
{
	Matrix3 tm(1);
	
	Interface *ip = GetCOREInterface();

	if (skinNode == NULL) return tm;
	if (boneNode == NULL) return tm;

	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(skinNode);
	
	ISkin *skin = (ISkin *) mod->GetInterface(I_SKIN);
	if (skin)
		{
		if (boneNode)
			{
			
			skin->GetBoneInitTM(boneNode, tm );
			}
		}		
	return tm;
}
void SkinToolsFunctionPublish::SetBoneBindTM(INode *skinNode, INode *boneNode, Matrix3 tm)
{

	Interface *ip = GetCOREInterface();


	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(skinNode);
	
	ISkinImportData *skin = (ISkinImportData *) mod->GetInterface(I_SKINIMPORTDATA);
	if (skin)
		{
		if (skinNode && boneNode)
			skin->SetBoneTm(boneNode, tm,tm );
		}	

}

Matrix3 SkinToolsFunctionPublish::GetMeshBindTM(INode *skinNode )
{
	Matrix3 tm(1);
	
	Interface *ip = GetCOREInterface();

	if (skinNode == NULL) return tm;

	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(skinNode);
	
	ISkin *skin = (ISkin *) mod->GetInterface(I_SKIN);
	if (skin)
		{
			
		skin->GetSkinInitTM(skinNode, tm );
		}		
	return tm;

}
void SkinToolsFunctionPublish::SetMeshBindTM(INode *skinNode, Matrix3 tm)
{
	
	Interface *ip = GetCOREInterface();


	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(skinNode);
	
	ISkinImportData *skin = (ISkinImportData *) mod->GetInterface(I_SKINIMPORTDATA);
	if (skin)
		{
			
		skin->SetSkinTm(skinNode, tm,tm );
		}		
}





Matrix3 SkinToolsFunctionPublish::GetBoneStretchTM(INode *skinNode, INode *boneNode)
{
	Matrix3 tm(1);
	
	Interface *ip = GetCOREInterface();

	if (skinNode == NULL) return tm;
	if (boneNode == NULL) return tm;

	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(skinNode);
	
	ISkin2 *skin = (ISkin2 *) mod->GetInterface(I_SKIN2);
	if (skin)
		{
		if (boneNode)
			{
			
			tm = skin->GetBoneStretchTm(boneNode);
			}
		}		
	return tm;
}
void SkinToolsFunctionPublish::SetBoneStretchTM(INode *skinNode, INode *boneNode, Matrix3 tm)
{

	Interface *ip = GetCOREInterface();


	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(skinNode);
	
	ISkin2 *skin = (ISkin2 *) mod->GetInterface(I_SKIN2);
	if (skin)
		{
		if (skinNode && boneNode)
			skin->SetBoneStretchTm(boneNode, tm );
		}	

}


#define LOOP_SEL 1
#define RING_SEL 2
#define GROW_SEL 3
#define SHRINK_SEL 4

void EdgeSel(BitArray &sel, INode *node, int mode)
{
	//get mesh
	ObjectState sos = node->EvalWorldState(GetCOREInterface()->GetTime());
	int numberVerts = sel.GetSize();
		

//get the mesh
	//check if mesh or poly
	if (sos.obj->IsSubClassOf(triObjectClassID))
		{

		TriObject *tobj = (TriObject*)sos.obj;
		Mesh *tmsh = &tobj->GetMesh();

		Object *obj = sos.obj->ConvertToType(GetCOREInterface()->GetTime(), polyObjectClassID);
		PolyObject *pobj = (PolyObject*)obj;
		MNMesh *msh = &pobj->GetMesh();

		if (tmsh->numVerts != numberVerts) return;


		Tab<int> vertIndexIntoPolyMesh;
		vertIndexIntoPolyMesh.SetCount(msh->numv);


		for (int i = 0; i < msh->numv; i++)
			{
			Point3 mnP = msh->v[i].p;
			for (int j = 0; j < tmsh->numVerts; j++)
				{
				Point3 p = tmsh->verts[j];
				if (p == mnP)
					{
					vertIndexIntoPolyMesh[i] = j;
					}
				}
			}

		//convert vert sel to edge sel
		BitArray vsel, esel;

		vsel.SetSize(numberVerts);
		vsel.ClearAll();
	//loop through the selection
		for ( i =0; i < numberVerts; i++)
			{
			if (sel[i])
				vsel.Set(i);
			}

		esel.SetSize(msh->nume);
		esel.ClearAll();

		for (i = 0; i < msh->nume; i++)
			{
			int a,b;
			a = msh->e[i].v1;
			b = msh->e[i].v2;
			a = vertIndexIntoPolyMesh[a];
			b = vertIndexIntoPolyMesh[b];
			if (vsel[a] && vsel[b])
				esel.Set(i);
			}


		//do loop
		if (mode == LOOP_SEL)
			msh->SelectEdgeLoop (esel);
		else if (mode == RING_SEL)
			msh->SelectEdgeRing (esel);


		//convert back to vert sel
		
		//pass that back
		sel.ClearAll();
		for (i = 0; i < msh->nume; i++)
			{
			if (esel[i])
				{
				int a,b;
				a = msh->e[i].v1;
				b = msh->e[i].v2;

				a = vertIndexIntoPolyMesh[a];
				b = vertIndexIntoPolyMesh[b];

				sel.Set(a);
				sel.Set(b);
				}
			}
		obj->DeleteThis();

		}
	//if poly object we can use that
	else if (sos.obj->IsSubClassOf(polyObjectClassID))
		{
		PolyObject *pobj = (PolyObject*)sos.obj;
		MNMesh *msh = &pobj->GetMesh();

		//convert vert sel to edge sel
		BitArray vsel, esel;

		if (msh->numv != numberVerts) return;

		vsel.SetSize(numberVerts);
		vsel.ClearAll();
	//loop through the selection
		for (int i =0; i < numberVerts; i++)
			{
			if (sel[i])
				vsel.Set(i);
			}

		esel.SetSize(msh->nume);
		esel.ClearAll();

		for (i = 0; i < msh->nume; i++)
			{
			int a,b;
			a = msh->e[i].v1;
			b = msh->e[i].v2;
			if (vsel[a] && vsel[b])
				esel.Set(i);
			}


		//do loop
		if (mode == LOOP_SEL)
			msh->SelectEdgeLoop (esel);
		else if (mode == RING_SEL)
			msh->SelectEdgeRing (esel);

		//convert back to vert sel
		
		//pass that back
		sel.ClearAll();
		for (i = 0; i < msh->nume; i++)
			{
			if (esel[i])
				{
				int a,b;
				a = msh->e[i].v1;
				b = msh->e[i].v2;

				sel.Set(a);
				sel.Set(b);
				}
			}


		}
	
}


void GrowVertSel(BitArray &sel, INode *node, int mode)
{
	//get mesh
	ObjectState sos = node->EvalWorldState(GetCOREInterface()->GetTime());
	int numberVerts = sel.GetSize();
		

//get the mesh
	//check if mesh or poly
	if (sos.obj->IsSubClassOf(triObjectClassID))
	{

		TriObject *tobj = (TriObject*)sos.obj;
		Mesh *tmsh = &tobj->GetMesh();

		if (tmsh->numVerts != numberVerts) return;

		BitArray origSel(sel);
		
		for (int i = 0; i < tmsh->numFaces; i++)
		{
			int deg = 3;
			int hit = 0;
			for (int j = 0; j < deg; j++)
			{	
				int a = tmsh->faces[i].v[j];
				if (origSel[a])
					hit ++;
			}
			if ((hit>0) && (mode == GROW_SEL))
			{
				for (int j = 0; j < deg; j++)
				{	
					int a = tmsh->faces[i].v[j];
					
					sel.Set(a,TRUE);	
										
				}			
			}
			else if ((hit!= deg) && (mode == SHRINK_SEL))
			{
				for (int j = 0; j < deg; j++)
				{	
					int a = tmsh->faces[i].v[j];
					
					sel.Set(a,FALSE);						
				}			
			}			

		}
	}
	//if poly object we can use that
	else if (sos.obj->IsSubClassOf(polyObjectClassID))
		{
		PolyObject *pobj = (PolyObject*)sos.obj;
		MNMesh *msh = &pobj->GetMesh();

		//convert vert sel to edge sel
		BitArray vsel, esel;

		if (msh->numv != numberVerts) return;

		BitArray origSel(sel);
		
		for (int i = 0; i < msh->numf; i++)
		{
			int deg = msh->f[i].deg;
			int hit = 0;
			for (int j = 0; j < deg; j++)
			{	
				int a = msh->f[i].vtx[j];
				if (origSel[a])
					hit++;
			}
			if ((hit>0) && (mode == GROW_SEL))
			{
				for (int j = 0; j < deg; j++)
				{	
					int a = msh->f[i].vtx[j];
					
					sel.Set(a,TRUE);
						
				}			
			}
			else if ((hit!= deg) && (mode == SHRINK_SEL))
			{
				for (int j = 0; j < deg; j++)
				{	
					int a = msh->f[i].vtx[j];
					
					sel.Set(a,FALSE);						
				}			
			}				


		}
		}
}

void SkinToolsFunctionPublish::GrowSel(INode *node)
{
	Interface *ip = GetCOREInterface();

	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(node);
	
	ISkin2 *skin = (ISkin2 *) mod->GetInterface(I_SKIN2);
	if (skin)
	{
		BitArray sel;
		skin->GetVertexSelection(node,sel);
		GrowVertSel(sel,node,GROW_SEL );
		skin->SetVertexSelection(node,sel);
	}

}

void SkinToolsFunctionPublish::ShrinkSel(INode *node)
{
	Interface *ip = GetCOREInterface();

	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(node);
	
	ISkin2 *skin = (ISkin2 *) mod->GetInterface(I_SKIN2);
	if (skin)
	{
		BitArray sel;
		skin->GetVertexSelection(node,sel);
		GrowVertSel(sel,node,SHRINK_SEL );
		skin->SetVertexSelection(node,sel);
	}

}

void SkinToolsFunctionPublish::RingSel(INode *node)
{
	Interface *ip = GetCOREInterface();

	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(node);
	
	ISkin2 *skin = (ISkin2 *) mod->GetInterface(I_SKIN2);
	if (skin)
	{
		BitArray sel;
		skin->GetVertexSelection(node,sel);
		EdgeSel(sel,node,RING_SEL );
		skin->SetVertexSelection(node,sel);
	}

}

void SkinToolsFunctionPublish::LoopSel(INode *node)
{
	Interface *ip = GetCOREInterface();

	//get the skin modifier
	Modifier *mod = theSkinTools.GetSkin(node);
	
	ISkin2 *skin = (ISkin2 *) mod->GetInterface(I_SKIN2);
	if (skin)
	{
		BitArray sel;
		skin->GetVertexSelection(node,sel);
		EdgeSel(sel,node,LOOP_SEL );
		skin->SetVertexSelection(node,sel);
	}

}



static BOOL CALLBACK SkinToolsDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theSkinTools.Init(hWnd);
			break;

		case WM_DESTROY:
			theSkinTools.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_EXTRACTDATABUTTON:
					theSkinTools.ExtractData();

					break;
				case IDC_PASTEDATABUTTON:
					theSkinTools.PasteData();

					break;

				}
			return 1;

			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theSkinTools.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}



//--- SkinTools -------------------------------------------------------
SkinTools::SkinTools()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

SkinTools::~SkinTools()
{

}

void SkinTools::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		SkinToolsDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void SkinTools::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void SkinTools::Init(HWND hWnd)
{


}

void SkinTools::Destroy(HWND hWnd)
{

}


Modifier *SkinTools::GetSkin(INode *node)
{

	Object* pObj = node->GetObjectRef();

	if (!pObj) return NULL;

	
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject *)(pObj);
			
		int Idx = 0;

		while (Idx < pDerObj->NumModifiers())
		{
			// Get the modifier. 
			Modifier* mod = pDerObj->GetModifier(Idx);

			
			if (mod->ClassID() == SKIN_CLASSID)
			{
				// is this the correct Physique Modifier based on index?
				return mod;
			}

			Idx++;
		}

		pObj = pDerObj->GetObjRef();
	}

	return NULL;


}

void SkinTools::ExtractData()
{
	Interface *ip = GetCOREInterface();

	int selCount = ip->GetSelNodeCount();
//loop through selection

	for (int i = 0; i < selCount; i++)
	{
	//extract data from each node
		INode *node = ip->GetSelNode(i);
		ExtractData(node);

	}


}

void SkinTools::ExtractData(INode *node)

{
	Interface *ip = GetCOREInterface();

	if (node == NULL) return;

	//get the skin modifier
	Modifier *mod = GetSkin(node);

	for (int i = 0; i < boneList.Count(); i++)
	{
		if (boneList[i])
			delete boneList[i];
	}
	boneList.ZeroCount();

	if (mod)
	{
	//get the skin interface
		ISkin *skin = (ISkin *) mod->GetInterface(I_SKIN);
	//loop though bones
		if (skin)
		{
			int boneCount = skin->GetNumBones();
			int ct = 0;
			for (int i = 0; i < boneCount; i++)
			{
				INode *node = skin->GetBone(i);

				BoneData *data = new BoneData();
				data->node = node;
				TSTR name;
				name.printf("%s",node->GetName());
				data->SetOriginalName(name);

				data->mapID = ct++;
				data->subID = -50;


				boneList.Append(1,&data);
			}

			//loop through the vertices
			ISkinContextData *skinData = skin->GetContextInterface(node);

			if (skinData)
			{
				int numberOfPoints;
				numberOfPoints = skinData->GetNumPoints();
				for (int i = 0; i < numberOfPoints; i++)
				{
					int numOfWeights;
					numOfWeights = skinData->GetNumAssignedBones(i);
					for (int j = 0; j < numOfWeights; j++)
					{
						int boneIndex = skinData->GetAssignedBone(i,j);
						float boneWeight = skinData->GetBoneWeight(i,j);
						if ((boneIndex >= 0) && (boneIndex < boneList.Count()))
						{
							WeightData data;
							data.vertIndex = i;
							data.vertWeight = boneWeight;

							boneList[boneIndex]->weights.Append(1,&data,100);
						}
					}
				}
				//now clone our node

				Object *obj = NULL;
				ObjectState os;
				
				TimeValue t = ip->GetTime();
	

				os = node->EvalWorldState(ip->GetTime());	

/*				if (os.obj->CanConvertToType(triObjectClassID)) 
				{	
					Object *oldObj = os.obj;
					TriObject *tobj;
					tobj = (TriObject*)os.obj->ConvertToType(ip->GetTime(),triObjectClassID);
					if (oldObj==tobj) 
					{
						obj = (Object*)tobj->Clone();
					} 
					else 
					{
						obj = tobj;
					}						
				} 
				else 
				{*/
					obj = (Object*)os.obj->Clone();
//				}
				
				obj = obj->CollapseObject();

				INode *cloneNode = ip->CreateObjectNode(obj);

	// Name the node and make the name unique.
				TSTR name;
				name.printf("SkinData_%s",node->GetName());

				cloneNode->SetName(name);

//clone tm
				Matrix3 tm   = node->GetNodeTM(t);
				cloneNode->SetObjOffsetPos(node->GetObjOffsetPos());
				cloneNode->SetObjOffsetRot(node->GetObjOffsetRot());
				cloneNode->SetObjOffsetScale(node->GetObjOffsetScale());
				cloneNode->SetNodeTM(t,tm);
//name the channels

				for (i = 0; i < boneList.Count(); i++)
				{
					TSTR key;
					TSTR tname;
					tname.printf("SkinWeight:%s",boneList[i]->GetOriginalName());
					key.printf("MapChannel:%d(%d)",boneList[i]->mapID/3,boneList[i]->mapID%3);
					cloneNode->SetUserPropString(key,tname);
				}


				//add our modifier
				PasteSkinWeights* mod;
				mod = (PasteSkinWeights*)CreateInstance(OSM_CLASS_ID, PASTESKINWEIGHTS_CLASS_ID);

				if (mod)
				{
					//load our data
					mod->boneList.SetCount(boneList.Count());

					for (i = 0; i < boneList.Count(); i++)
					{
						mod->boneList[i] = new BoneData();
						*mod->boneList[i] = *boneList[i];
					}
        
					IDerivedObject* dobj = CreateDerivedObject(obj);
					dobj->AddModifier(mod);


					cloneNode->SetObjectRef(dobj);


					mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					mod->NotifyDependents(FOREVER,PART_ALL,REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
					cloneNode->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
					ip->ForceCompleteRedraw(ip->GetTime());



					int selCount= ip->GetSelNodeCount();

					BOOL select = FALSE;
					for (int i = 0; i < selCount; i++)
						{
						INode *selNode = ip->GetSelNode(i);
						if (selNode == node)
							select = TRUE;
						}
					if (select)
						ip->SelectNode(node);

					
					Object *oldObj = cloneNode->GetObjectRef();


					ObjectState os = oldObj->Eval(ip->GetTime());

					Object *obj = (Object*)os.obj->CollapseObject();
					if(obj == os.obj)
						obj = (Object*)obj->Clone();
		
					oldObj->SetAFlag(A_LOCK_TARGET);
					cloneNode->SetObjectRef(obj);		
					oldObj->ClearAFlag(A_LOCK_TARGET)	;
					oldObj->MaybeAutoDelete();
					

				}
			}


		}





	}


	for (i = 0; i < boneList.Count(); i++)
	{
		if (boneList[i])
			delete boneList[i];
	}
	boneList.ZeroCount();

}


static INT_PTR CALLBACK PasteFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: 
			{

			theSkinTools.floaterHWND = hWnd;


			theSkinTools.FilloutListBox();
			theSkinTools.InitDialog();
			
			break;
			}

		


		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_ADDTARGBUTTON:
					theSkinTools.AddTargetBones();

					break;
				case IDC_REMOVETARGBUTTON:
					theSkinTools.RemoveTargetBones();
					break;

				case IDC_ADDSOURCEBUTTON:
					theSkinTools.AddSourceBones();

					break;
				case IDC_REMOVESOURCEBUTTON:
					theSkinTools.RemoveSourceBones();
					break;


				case IDC_TOTARGBUTTON:
					theSkinTools.SourceToTarget();
					break;


				case IDC_TOSOURCEBUTTON:
					theSkinTools.TargetToSource();
					break;

				
				case IDC_MATCHBYNAMEBUTTON:
					theSkinTools.MatchByName();
					break;

				case IDC_REMOVEPREFFIXTARG_CHECK:
				case IDC_REMOVESUFFIXTARG_CHECK:
					theSkinTools.RemoveTargPrefix(IsDlgButtonChecked(hWnd,IDC_REMOVEPREFFIXTARG_CHECK));
					theSkinTools.RemoveTargSufix(IsDlgButtonChecked(hWnd,IDC_REMOVESUFFIXTARG_CHECK));
					break;

				case IDC_REMOVEPREFFIXSOURCE_CHECK:
				case IDC_REMOVESUFFIXSOURCE_CHECK:
					theSkinTools.RemoveSourcePrefix(IsDlgButtonChecked(hWnd,IDC_REMOVEPREFFIXSOURCE_CHECK));
					theSkinTools.RemoveSourceSufix(IsDlgButtonChecked(hWnd,IDC_REMOVESUFFIXSOURCE_CHECK));
					break;



				case IDOK:
					theSkinTools.DestroyDialog(TRUE);
					EndDialog(hWnd,1);
					break;
				case IDCANCEL:
					theSkinTools.DestroyDialog(FALSE);
					theSkinTools.floaterHWND = NULL;

					EndDialog(hWnd,0);
				break;
			}
			break;

		default:
			return FALSE;
		}

	return TRUE;
	}


void SkinTools::PasteData()
{
	Interface *ip = GetCOREInterface();

	int selCount = ip->GetSelNodeCount();
//loop through selection

	INode *snode = NULL;
	INode *tnode = NULL;

	for (int i = 0; i < selCount; i++)
	{
	//extract data from each node
		INode *node = ip->GetSelNode(i);
		if (snode == NULL)
			snode = node;
		else if (tnode == NULL)
			tnode = node;
	}
	
	if (snode && tnode)
		PasteData(snode,tnode);

}


void SkinTools::MatchByPoints(Mesh *msh,ISkinImportData *skinImp, INode *snode, INode *tnode)
{
	Interface *ip = GetCOREInterface();
	TimeValue t = ip->GetTime();

					Tab<Point3> sourcePoints;
					Tab<Point3> targetPoints;
					Tab<int> matchingSource;
					

					//get our target mesh
					ObjectState os = tnode->EvalWorldState(t);

					Object *obj = os.obj;

					//get the mesh
				//get the object
				// get the mesh
					Mesh *tmsh = NULL;
					if (os.obj->IsSubClassOf(triObjectClassID))
					{
						TriObject *tobj = (TriObject*)os.obj;
						tmsh = &tobj->GetMesh();
//msh is out source
						Matrix3 stm,ttm;
						stm = snode->GetObjectTM(t);
						ttm = tnode->GetObjectTM(t);

						matchingSource.SetCount(tmsh->numVerts);
		//get our target points
						targetPoints.SetCount(tmsh->numVerts);
						for (int i = 0; i < tmsh->numVerts; i++)
						{
							targetPoints[i] = tmsh->verts[i] * ttm;
							matchingSource[i] = -1;
						}

		//get our source points
						sourcePoints.SetCount(msh->numVerts);
						for (i = 0; i < msh->numVerts; i++)
						{
							sourcePoints[i] = msh->verts[i] * stm;
						}
		//find matches
						for (i = 0; i < tmsh->numVerts; i++)
						{
							Point3 tp = targetPoints[i];
							Box3 box;
							box.Init();
							box += tp;
							box.EnlargeBy(threshold);
							int index = -1;
							float closestDist = -1.0f;
							for (int j = 0; j < msh->numVerts; j++)
							{
								Point3 sp = sourcePoints[j];
								if (box.Contains(sp))
								{
									float dist = LengthSquared(sp-tp);
									if ((dist < closestDist) || (index == -1))
									{
										index = j;
										closestDist = dist;
									}
								}

							}
							matchingSource[i] = index;
						}

						//now build the weights
						for (i = 0; i < boneList.Count(); i++)
						{
							int mapID = boneList[i]->mapID;
							int subID = boneList[i]->subID;

							boneList[i]->weights.SetCount(msh->numVerts);

							for (int j = 0; j < boneList[i]->weights.Count(); j++)
							{
								boneList[i]->weights[j].vertWeight = 0.0f;
							}
							//get the tvfaces
							//get the tvpoints
							TVFace *tvFace = msh->mapFaces(mapID);
							UVVert *tvVerts = msh->mapVerts(mapID);
							if (tvFace && tvVerts)
							{
								for (int j = 0; j < msh->numFaces; j++)
								{
									int index = tvFace[j].t[0];
									int gindex = msh->faces[j].v[0];

									float w =tvVerts[index][subID];
									boneList[i]->weights[gindex].vertWeight = w;

									index = tvFace[j].t[1];
									gindex = msh->faces[j].v[1];
									w =tvVerts[index][subID];
									boneList[i]->weights[gindex].vertWeight = w;

									index = tvFace[j].t[2];
									gindex = msh->faces[j].v[2];
									w =tvVerts[index][subID];
									boneList[i]->weights[gindex].vertWeight = w;
								}
							}

								
						}

/*
						for (i = 0; i < msh->numVerts; i++)
						{
							DebugPrint("vert %d ", i);
							for (int j = 0; j < boneList.Count(); j++)
							{
								float w = boneList[j]->weights[i].vertWeight;
								if (w != 0.0f)
								{
									DebugPrint(" %s:%f,", boneList[j]->name,w);
								}

							}
							DebugPrint("\n");
						}
*/
						for (i = 0; i < tmsh->numVerts; i++)
						{
//	if (i==6323)
//		DebugPrint("Test\n");
							int sourceVert = matchingSource[i];
							if (sourceVert != -1)
							{
								Tab<INode*> nodeData;
								Tab<float> weights;
								for (int j = 0; j < targBoneList.Count(); j++)
								{
									float w = 0.0f;
									
									for (int k = 0; k < targBoneList[j]->matchingBones.Count(); k++)
									{
										int sourceBone = targBoneList[j]->matchingBones[k];
										w += boneList[sourceBone]->weights[sourceVert].vertWeight;
									}
									if (w != 0.0f)
									{
										weights.Append(1,&w);
										INode *node = targBoneList[j]->node;
										nodeData.Append(1,&node);
									}
								}
								//drop that weight onto skin
							
								skinImp->AddWeights(tnode, i, nodeData, weights);
							}
						}

						

					}






}


void SkinTools::MatchByPoints(MNMesh *msh,ISkinImportData *skinImp, INode *snode, INode *tnode)
{
	Interface *ip = GetCOREInterface();
	TimeValue t = ip->GetTime();

					Tab<Point3> sourcePoints;
					Tab<Point3> targetPoints;
					Tab<int> matchingSource;
					

					//get our target mesh
					ObjectState os = tnode->EvalWorldState(t);

					Object *obj = os.obj;

					//get the mesh
				//get the object
				// get the mesh
					MNMesh *tmsh = NULL;
					if (os.obj->IsSubClassOf(polyObjectClassID))
					{
						PolyObject *tobj = (PolyObject*)os.obj;
						tmsh = &tobj->GetMesh();
//msh is out source
						Matrix3 stm,ttm;
						stm = snode->GetObjectTM(t);
						ttm = tnode->GetObjectTM(t);

						matchingSource.SetCount(tmsh->numv);
		//get our target points
						targetPoints.SetCount(tmsh->numv);
						for (int i = 0; i < tmsh->numv; i++)
						{
							targetPoints[i] = tmsh->v[i].p * ttm;
							matchingSource[i] = -1;
						}

		//get our source points
						sourcePoints.SetCount(msh->numv);
						for (i = 0; i < msh->numv; i++)
						{
							sourcePoints[i] = msh->v[i].p * stm;
						}
		//find matches
						for (i = 0; i < tmsh->numv; i++)
						{
							Point3 tp = targetPoints[i];
							Box3 box;
							box.Init();
							box += tp;
							box.EnlargeBy(threshold);
							int index = -1;
							float closestDist = -1.0f;
							for (int j = 0; j < msh->numv; j++)
							{
								Point3 sp = sourcePoints[j];
								if (box.Contains(sp))
								{
									float dist = LengthSquared(sp-tp);
									if ((dist < closestDist) || (index == -1))
									{
										index = j;
										closestDist = dist;
									}
								}

							}
							matchingSource[i] = index;
						}

						//now build the weights
						for (i = 0; i < boneList.Count(); i++)
						{
							int mapID = boneList[i]->mapID;
							int subID = boneList[i]->subID;

							boneList[i]->weights.SetCount(msh->numv);

							for (int j = 0; j < boneList[i]->weights.Count(); j++)
							{
								boneList[i]->weights[j].vertWeight = 0.0f;
							}
							//get the tvfaces
							//get the tvpoints
//							TVFace *tvFace = msh->mapFaces(mapID);
							MNMapFace *tvFace = msh->M(mapID)->f;
							UVVert *tvVerts = msh->M(mapID)->v;//msh->mapVerts(mapID);
							if (tvFace && tvVerts)
							{
								for (int j = 0; j < msh->numf; j++)
								{
									int deg = msh->f[j].deg;


									for (int k = 0; k < deg; k++)
									{
										int index = tvFace[j].tv[k];
										int gindex = msh->f[j].vtx[k];

										float w =tvVerts[index][subID];
										boneList[i]->weights[gindex].vertWeight = w;
									}
								}
							}

								
						}

						for (i = 0; i < tmsh->numv; i++)
						{
//	if (i==6323)
//		DebugPrint("Test\n");
							int sourceVert = matchingSource[i];
							if (sourceVert != -1)
							{
								Tab<INode*> nodeData;
								Tab<float> weights;
								for (int j = 0; j < targBoneList.Count(); j++)
								{
									float w = 0.0f;
									
									for (int k = 0; k < targBoneList[j]->matchingBones.Count(); k++)
									{
										int sourceBone = targBoneList[j]->matchingBones[k];
										w += boneList[sourceBone]->weights[sourceVert].vertWeight;
									}
									if (w != 0.0f)
									{
										weights.Append(1,&w);
										INode *node = targBoneList[j]->node;
										nodeData.Append(1,&node);
									}
								}
								//drop that weight onto skin
							
								skinImp->AddWeights(tnode, i, nodeData, weights);
							}
						}

						

					}






}



void SkinTools::MatchByPoints(PatchMesh *msh,ISkinImportData *skinImp, INode *snode, INode *tnode)
{
	Interface *ip = GetCOREInterface();
	TimeValue t = ip->GetTime();
	int numv = msh->numVerts+msh->numVecs;

					Tab<Point3> sourcePoints;
					Tab<Point3> targetPoints;
					Tab<int> matchingSource;
					

					//get our target mesh
					ObjectState os = tnode->EvalWorldState(t);

					Object *obj = os.obj;

					//get the mesh
				//get the object
				// get the mesh
					PatchMesh *tmsh = NULL;
					if (os.obj->IsSubClassOf(patchObjectClassID))
					{
						PatchObject *tobj = (PatchObject*)os.obj;
						tmsh = &tobj->patch;
//msh is out source
						Matrix3 stm,ttm;
						stm = snode->GetObjectTM(t);
						ttm = tnode->GetObjectTM(t);

						int tnumv = tmsh->numVerts+tmsh->numVecs;

						matchingSource.SetCount(tnumv);
		//get our target points
						targetPoints.SetCount(tnumv);
						int ct = 0;
						for (int i = 0; i < tmsh->numVerts; i++)
						{
							targetPoints[ct] = tmsh->verts[i].p * ttm;
							matchingSource[ct] = -1;
							ct++;
						}
						for (i = 0; i < tmsh->numVecs; i++)
						{
							targetPoints[ct] = tmsh->vecs[i].p * ttm;
							matchingSource[ct] = -1;
							ct++;
						}


		//get our source points
						sourcePoints.SetCount(numv);
						ct = 0;
						for (i = 0; i < msh->numVerts; i++)
						{
							sourcePoints[ct] = msh->verts[i].p * stm;
							ct++;
						}
						for (i = 0; i < msh->numVecs; i++)
						{
							sourcePoints[ct] = msh->vecs[i].p * stm;
							ct++;
						}

		//find matches
						for (i = 0; i < tnumv; i++)
						{
							Point3 tp = targetPoints[i];
							Box3 box;
							box.Init();
							box += tp;
							box.EnlargeBy(threshold);
							int index = -1;
							float closestDist = -1.0f;
							for (int j = 0; j < numv; j++)
							{
								Point3 sp = sourcePoints[j];
								if (box.Contains(sp))
								{
									float dist = LengthSquared(sp-tp);
									if ((dist < closestDist) || (index == -1))
									{
										index = j;
										closestDist = dist;
									}
								}

							}
							matchingSource[i] = index;
						}

						//now build the weights
						for (i = 0; i < boneList.Count(); i++)
						{
							int mapID = boneList[i]->mapID;
							int subID = boneList[i]->subID;

							boneList[i]->weights.SetCount(numv);

							for (int j = 0; j < boneList[i]->weights.Count(); j++)
							{
								boneList[i]->weights[j].vertWeight = 0.0f;
							}
							//get the tvfaces
							//get the tvpoints
//							TVFace *tvFace = msh->mapFaces(mapID);
//							MNMapFace *tvFace = msh->M(mapID)->f;
//							UVVert *tvVerts = msh->M(mapID)->v;//msh->mapVerts(mapID);

							TVPatch *tvFace = msh->tvPatches[mapID];;//TVFace *tvFace = pmesh->mapFaces(mapID);
							PatchTVert *tvVerts = msh->mapVerts(mapID);

							if (tvFace && tvVerts)
							{
								for (int j = 0; j < msh->numPatches; j++)
								{
									int deg = 3;
									if (msh->patches[j].type == PATCH_QUAD)
										deg = 4;	

									for (int k = 0; k < deg; k++)
									{
										int index = tvFace[j].tv[k];
										int gindex = msh->patches[j].v[k];

										float w =tvVerts[index].p[subID];
										boneList[i]->weights[gindex].vertWeight = w;

										index = tvFace[j].handles[k*2];
										gindex = msh->patches[j].vec[k*2] + msh->numVerts;
										if ((index != -1) && (gindex != -1))
										{
											w =tvVerts[index].p[subID];
											boneList[i]->weights[gindex].vertWeight = w;
										}

										index = tvFace[j].handles[k*2+1];
										gindex = msh->patches[j].vec[k*2+1] + msh->numVerts;
										if ((index != -1) && (gindex != -1))
										{
											w =tvVerts[index].p[subID];
											boneList[i]->weights[gindex].vertWeight = w;
										}


										index = tvFace[j].interiors[k];
										gindex = msh->patches[j].interior[k] + msh->numVerts;
										if ((index != -1) && (gindex != -1))
										{
											w =tvVerts[index].p[subID];
											boneList[i]->weights[gindex].vertWeight = w;
										}




									}
								}
							}

								
						}

						for (i = 0; i < tnumv; i++)
						{

							int sourceVert = matchingSource[i];

							if (sourceVert != -1)
							{
								Tab<INode*> nodeData;
								Tab<float> weights;
								for (int j = 0; j < targBoneList.Count(); j++)
								{
									float w = 0.0f;
									
									for (int k = 0; k < targBoneList[j]->matchingBones.Count(); k++)
									{
										int sourceBone = targBoneList[j]->matchingBones[k];
										w += boneList[sourceBone]->weights[sourceVert].vertWeight;
									}
									if (w != 0.0f)
									{
										weights.Append(1,&w);
										INode *node = targBoneList[j]->node;
										nodeData.Append(1,&node);
									}
								}
								//drop that weight onto skin
							
								skinImp->AddWeights(tnode, i, nodeData, weights);
							}
						}

						

					}






}



void SkinTools::MatchByFace(Mesh *sourceMsh, ISkinImportData *skinImp, INode *snode, INode *tnode)
{

//get the engine
	ReferenceTarget *ref = (ReferenceTarget *) CreateInstance(REF_TARGET_CLASS_ID, RAYMESHGRIDINTERSECT_CLASS_ID);
	if (ref == NULL) return;


	IRayMeshGridIntersect_InterfaceV1 *rt =  (IRayMeshGridIntersect_InterfaceV1 *) ref->GetInterface(RAYMESHGRIDINTERSECT_V1_INTERFACE);

//if no engine bail
	if (rt == NULL) return;

//initial engine with mesh
	rt->fnFree();
	rt->fnInitialize(50);
//add node
	rt->fnAddNode(snode);
	rt->fnBuildGrid();
	//build grid


	//loop through all our points and find the closest face

	Interface *ip = GetCOREInterface();
	TimeValue t = ip->GetTime();

	ObjectState os = tnode->EvalWorldState(t);
	Object *obj = os.obj;

	int pointCount = obj->NumPoints();

	Matrix3 baseTM = tnode->GetObjectTM(t);

	Tab<int> faceList;
	Tab<Point3> baryList;
	Tab<float> offsetDist;
	faceList.SetCount(pointCount);
	baryList.SetCount(pointCount);
	offsetDist.SetCount(pointCount);
	

	for (int i = 0; i < pointCount; i++)
	{
		Point3 p = obj->GetPoint(i);
		p = p * baseTM;
//if (1770 == i)
//DebugPrint("Text\n");

		rt->fnClosestFace(p);
		int index =  rt->fnGetClosestHit();
		int faceIndex =  rt->fnGetHitFace(index)-1;
		Point3 bary = rt->fnGetHitBary(index);

//get bary, face index and  perp dist
		baryList[i] = bary;
		faceList[i] = faceIndex;
		offsetDist[i] = rt->fnGetHitDist(index);

		//get face indices
	}
	//gather up all the weights now
	for ( i = 0; i < pointCount; i++)
	{
		int faceIndex = faceList[i];
		Point3 bary = baryList[i];
		float offset = offsetDist[i];

//if (1770 == i)
//DebugPrint("Text\n");

		if (fabs(offset) < threshold)
		{
			Tab<INode*> bones;
			Tab<float> weights;
			
			for (int j = 0; j < targBoneList.Count(); j++)
			{
				float w = 0.0f;
									
				for (int k = 0; k < targBoneList[j]->matchingBones.Count(); k++)
				{
					int sourceBone = targBoneList[j]->matchingBones[k];

					int mapID = boneList[sourceBone]->mapID;
					int subID = boneList[sourceBone]->subID;

					//get the tvfaces
					//get the tvpoints
					TVFace *tvFace = sourceMsh->mapFaces(mapID);
					UVVert *tvVerts = sourceMsh->mapVerts(mapID);

					if (tvFace && tvVerts && (faceIndex >= 0))
					{
						int a = tvFace[faceIndex].t[0];
						w += tvVerts[a][subID] * bary[0];
						a = tvFace[faceIndex].t[1];
						w += tvVerts[a][subID] * bary[1];
						a = tvFace[faceIndex].t[2];
						w += tvVerts[a][subID] * bary[2];
					}

					
				}
				if (w != 0.0f)
				{
					bones.Append(1,&targBoneList[j]->node,0);
					weights.Append(1,&w,0);
				}
			}
			if (bones.Count())
			{
				skinImp->AddWeights(tnode, i, bones, weights);
			}

		}
	}


	rt->fnPrintStats();


	//delete the engine

	theHold.Suspend();
	ref->DeleteMe();
	theHold.Resume();

	//get our point target point list
	//load up the engine with the source mesh
	//loop through through the points finding closest face and gets the bary
}

void SkinTools::PasteData (INode *snode,INode *tnode)
{
	Interface *ip = GetCOREInterface();
	TimeValue t = ip->GetTime();
//get our modifier
	//get the skin modifier
	Modifier *mod = GetSkin(tnode);

	if (mod == NULL)
	{
		INode *temp = snode;
		snode = tnode;
		tnode = temp;
		mod = GetSkin(tnode);
	}

	if (mod != NULL)
	{
//get our target bone list
	//get the skin interface
		ISkin *skin = (ISkin *) mod->GetInterface(I_SKIN);
		ISkinImportData *skinImp = (ISkinImportData *) mod->GetInterface(I_SKINIMPORTDATA);
	//loop though bones
		if (skin && skinImp)
		{
			int boneCount = skin->GetNumBones();
			int ct = 0;
			for (int i = 0; i < targBoneList.Count(); i++)
			{	
				if (targBoneList[i]) delete targBoneList[i];
			}
			
			targBoneList.ZeroCount();
			targBoneList.SetCount(boneCount);

			for (i = 0; i < boneCount; i++)
			{
				INode *node = skin->GetBone(i);

				TargBones *data = new TargBones();
				data->boneIndex = i;
				data->node = node;
				TSTR name;
				name.printf("%s",node->GetName());
				data->SetOriginalName(name);

				targBoneList[i] = data;
			}


	//get our source bone list
			//get the node

			ObjectState os = snode->EvalWorldState(t);

			Object *obj = os.obj;

			//get the mesh
		//get the object
		// get the mesh
			Mesh *msh = NULL;
			MNMesh *mnMsh = NULL;
			PatchMesh *pMsh = NULL;
			if (os.obj->IsSubClassOf(triObjectClassID))
			{
				TriObject *tobj = (TriObject*)os.obj;
				msh = &tobj->GetMesh();
			}
			else if (os.obj->IsSubClassOf(polyObjectClassID))
			{
				PolyObject *tobj = (PolyObject*)os.obj;
				mnMsh = &tobj->GetMesh();
			}
			else if (os.obj->IsSubClassOf(patchObjectClassID))
			{
				PatchObject *tobj = (PatchObject*)os.obj;
				pMsh = &tobj->patch;
			}
			if (msh == NULL) nukeMatchType = TRUE;
			else nukeMatchType = FALSE;
			if ((msh) || (mnMsh) || (pMsh))
			{

				TSTR match("SkinWeight:*");

				for (int i = 0; i < boneList.Count(); i++)
				{
					if (boneList[i])
						delete boneList[i];
				}
				boneList.ZeroCount();

		//loop through the maps looking for skin weights
				for (i = 0; i < 99; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						TSTR key;
						TSTR name;
						key.printf("MapChannel:%d(%d)",i,j);
						if (snode->UserPropExists(key))
						{
							snode->GetUserPropString(key,name);
							BOOL isWeightChannel = MatchPattern(name, match);
							if (isWeightChannel)
							{
								BoneData* data = new BoneData();
								data->mapID = i;
								data->subID = j;
								data->SetOriginalName(name.remove(0,11));
								boneList.Append(1,&data);
//DebugPrint("match %s\n",name);							
							}
	
							//see if it is a skin weight key
						}

					}
		//loop through the keys
				}

				pasteData.Clear();

				pasteData.liveSourceBones.SetCount(boneList.Count());
				for (i = 0; i < boneList.Count(); i++)
					pasteData.liveSourceBones[i] = i;

				pasteData.liveTargetBones.SetCount(targBoneList.Count());
				for (i = 0; i < targBoneList.Count(); i++)
					pasteData.liveTargetBones[i] = i;

				if (DialogBoxParam(
					hInstance,
					MAKEINTRESOURCE(IDD_PASTEDIALOG),
					GetCOREInterface()->GetMAXHWnd(),
					PasteFloaterDlgProc,NULL))
				{
					if (matchType==0)
					{	
						if (msh)
							MatchByPoints(msh,skinImp,snode,tnode);
						else if (mnMsh)
							MatchByPoints(mnMsh,skinImp,snode,tnode);
						else if (pMsh)
							MatchByPoints(pMsh,skinImp,snode,tnode);
					}
					else MatchByFace(msh,skinImp,snode,tnode);;

				}



				for (i = 0; i < targBoneList.Count(); i++)
				{	
					if (targBoneList[i]) delete targBoneList[i];
				}
			
				targBoneList.ZeroCount();
			
				for (i = 0; i < boneList.Count(); i++)
				{
					if (boneList[i])
						delete boneList[i];
				}
				boneList.ZeroCount();

			}
		
		}
	}
}

void SkinTools::FilloutListBox()
{
	//reset the list boxes
	


	SendMessage(GetDlgItem(floaterHWND,IDC_TARGETLIST), LB_RESETCONTENT ,0,0);
	SendMessage(GetDlgItem(floaterHWND,IDC_SOURCELIST), LB_RESETCONTENT ,0,0);

	SendMessage(GetDlgItem(floaterHWND,IDC_DEADTARGLIST), LB_RESETCONTENT ,0,0);
	SendMessage(GetDlgItem(floaterHWND,IDC_DEADSOURCELIST), LB_RESETCONTENT ,0,0);


	for (int i = 0; i < pasteData.liveTargetBones.Count(); i++)
	{
		int index = pasteData.liveTargetBones[i];
		TSTR name;
		name.printf("%s : ",targBoneList[index]->GetDisplayName());

		TSTR comma(", ");

		TargBones* targ = targBoneList[index];

		for (int j = 0; j < targ->matchingBones.Count(); j++)
		{
			int sourceIndex = targ->matchingBones[j];

			name += boneList[sourceIndex]->GetDisplayName();
			if (j != (targ->matchingBones.Count()-1))
				name += comma;
		}

		SendMessage(GetDlgItem(floaterHWND,IDC_TARGETLIST),LB_ADDSTRING,0,(LPARAM)(TCHAR*)name);
	}

	for (i = 0; i < pasteData.liveSourceBones.Count(); i++)
	{
		int index = pasteData.liveSourceBones[i];
		SendMessage(GetDlgItem(floaterHWND,IDC_SOURCELIST),LB_ADDSTRING,0,(LPARAM)(TCHAR*)boneList[index]->GetDisplayName());
	}


	for (i = 0; i < pasteData.deadTargetBones.Count(); i++)
	{
		int index = pasteData.deadTargetBones[i];
		SendMessage(GetDlgItem(floaterHWND,IDC_DEADTARGLIST),LB_ADDSTRING,0,(LPARAM)(TCHAR*)targBoneList[index]->GetDisplayName());
	}


	for (i = 0; i < pasteData.deadSourceBones.Count(); i++)
	{
		int index = pasteData.deadSourceBones[i];
		SendMessage(GetDlgItem(floaterHWND,IDC_DEADSOURCELIST),LB_ADDSTRING,0,(LPARAM)(TCHAR*)boneList[index]->GetDisplayName());
	}


//	SendMessage(GetDlgItem(floaterHWND,IDC_LIST1),LB_ADDSTRING,0,(LPARAM)(TCHAR*)title);
	//add the targ bones

}


void SkinTools::RemoveTargetBones()
{
	HWND hWnd = GetDlgItem(floaterHWND,IDC_TARGETLIST);
//get our list selection
	int listCt = SendMessage(hWnd,LB_GETCOUNT,0,0);
	int selCt =  SendMessage(hWnd,LB_GETSELCOUNT ,0,0);
	int *selList;
	selList = new int[selCt];

	SendMessage(hWnd,	LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);


	BitArray delList;
	delList.SetSize(pasteData.liveTargetBones.Count());
	delList.ClearAll();
	for (int i=0; i < selCt; i++)
	{
		int index = selList[i];
		int bindex = pasteData.liveTargetBones[index];
		pasteData.deadTargetBones.Append(1,&bindex);
		delList.Set(index);
	}
	
	for (i=pasteData.liveTargetBones.Count(); i >=0; i--)
	{
		if (delList[i])
			pasteData.liveTargetBones.Delete(i,1);
			
	}	


	delete [] selList;
	FilloutListBox();
}


void SkinTools::AddTargetBones()
{
	HWND hWnd = GetDlgItem(floaterHWND,IDC_DEADTARGLIST);
//get our list selection
	int listCt = SendMessage(hWnd,LB_GETCOUNT,0,0);
	int selCt =  SendMessage(hWnd,LB_GETSELCOUNT ,0,0);
	int *selList;
	selList = new int[selCt];

	SendMessage(hWnd,	LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);


	BitArray delList;
	delList.SetSize(pasteData.deadTargetBones.Count());
	delList.ClearAll();
	for (int i=0; i < selCt; i++)
	{
		int index = selList[i];
		int bindex = pasteData.deadTargetBones[index];
		pasteData.liveTargetBones.Append(1,&bindex);
		delList.Set(index);
	}
	
	for (i=pasteData.deadTargetBones.Count(); i >=0; i--)
	{
		if (delList[i])
			pasteData.deadTargetBones.Delete(i,1);
			
	}	


	delete [] selList;
	FilloutListBox();
}


void SkinTools::RemoveSourceBones()
{

	HWND hWnd = GetDlgItem(floaterHWND,IDC_SOURCELIST);
//get our list selection
	int listCt = SendMessage(hWnd,LB_GETCOUNT,0,0);
	int selCt =  SendMessage(hWnd,LB_GETSELCOUNT ,0,0);
	int *selList;
	selList = new int[selCt];

	SendMessage(hWnd,	LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);


	BitArray delList;
	delList.SetSize(pasteData.liveSourceBones.Count());
	delList.ClearAll();
	for (int i=0; i < selCt; i++)
	{
		int index = selList[i];
		int bindex = pasteData.liveSourceBones[index];
		pasteData.deadSourceBones.Append(1,&bindex);
		delList.Set(index);
	}
	
	for (i=pasteData.liveSourceBones.Count(); i >=0; i--)
	{
		if (delList[i])
			pasteData.liveSourceBones.Delete(i,1);
			
	}	


	delete [] selList;
	FilloutListBox();
}


void SkinTools::AddSourceBones()
{
	HWND hWnd = GetDlgItem(floaterHWND,IDC_DEADSOURCELIST);
//get our list selection
	int listCt = SendMessage(hWnd,LB_GETCOUNT,0,0);
	int selCt =  SendMessage(hWnd,LB_GETSELCOUNT ,0,0);
	int *selList;
	selList = new int[selCt];

	SendMessage(hWnd,	LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);


	BitArray delList;
	delList.SetSize(pasteData.deadSourceBones.Count());
	delList.ClearAll();
	for (int i=0; i < selCt; i++)
	{
		int index = selList[i];
		int bindex = pasteData.deadSourceBones[index];
		pasteData.liveSourceBones.Append(1,&bindex);
		delList.Set(index);
	}
	
	for (i=pasteData.deadSourceBones.Count(); i >=0; i--)
	{
		if (delList[i])
			pasteData.deadSourceBones.Delete(i,1);
			
	}	


	delete [] selList;
	FilloutListBox();
}


void SkinTools::SourceToTarget()
{
//get our source sel

	HWND hSWnd = GetDlgItem(floaterHWND,IDC_SOURCELIST);
//get our list selection
	int sourceListCt = SendMessage(hSWnd,LB_GETCOUNT,0,0);
	int sourceSelCt =  SendMessage(hSWnd,LB_GETSELCOUNT ,0,0);
	int *sourceSelList;
	sourceSelList = new int[sourceSelCt];

	SendMessage(hSWnd,	LB_GETSELITEMS  ,(WPARAM) sourceSelCt,(LPARAM) sourceSelList);

//get out target sel
	HWND hTWnd = GetDlgItem(floaterHWND,IDC_TARGETLIST);
//get our list selection
	int targetListCt = SendMessage(hTWnd,LB_GETCOUNT,0,0);
	int targetSelCt =  SendMessage(hTWnd,LB_GETSELCOUNT ,0,0);
	int *targetSelList;
	targetSelList = new int[targetSelCt];

	SendMessage(hTWnd,	LB_GETSELITEMS  ,(WPARAM) targetSelCt,(LPARAM) targetSelList);


	BitArray delList;
	delList.SetSize(pasteData.liveSourceBones.Count());
	delList.ClearAll();

//if we just have one target sel
	if (targetSelCt == 1)
	{
		int targIndex = targetSelList[0];
		targIndex = pasteData.liveTargetBones[targIndex];
		//copy all our source in 
		TargBones* targ = targBoneList[targIndex];
		for (int i=0; i < sourceSelCt; i++)
		{
			int index = sourceSelList[i];
			int bindex = pasteData.liveSourceBones[index];

			
			targ->matchingBones.Append(1,&bindex);
			delList.Set(index);
		}
	}
	else
	{
		for (int i=0; i < sourceSelCt; i++)
		{
			if (i < targetSelCt)
			{
			int index = sourceSelList[i];
			int sindex = pasteData.liveSourceBones[index];

			int tind = targetSelList[i];
			int tindex = pasteData.liveTargetBones[tind];

			TargBones* targ = targBoneList[tindex];

			targ->matchingBones.Append(1,&sindex);
			delList.Set(index);
			}
		}
	}
	
	for (int i=pasteData.liveSourceBones.Count(); i >=0; i--)
	{
		if (delList[i])
			pasteData.liveSourceBones.Delete(i,1);
			
	}	




//else loop through targ and match one to one
	delete [] targetSelList;
	delete [] sourceSelList;
	FilloutListBox();
}


void SkinTools::TargetToSource()
{
	HWND hWnd = GetDlgItem(floaterHWND,IDC_TARGETLIST);
//get our list selection
	int listCt = SendMessage(hWnd,LB_GETCOUNT,0,0);
	int selCt =  SendMessage(hWnd,LB_GETSELCOUNT ,0,0);
	int *selList;
	selList = new int[selCt];

	SendMessage(hWnd,	LB_GETSELITEMS  ,(WPARAM) selCt,(LPARAM) selList);


	for (int i=0; i < selCt; i++)
	{
		int index = selList[i];
		int bindex = pasteData.liveTargetBones[index];
		
		TargBones* targ = targBoneList[bindex];

		for (int j = 0; j < targ->matchingBones.Count(); j++)
		{
			int index = targ->matchingBones[j];
			pasteData.liveSourceBones.Append(1,&index);
		}
		targ->matchingBones.ZeroCount();

		
	
	}
	


	delete [] selList;
	FilloutListBox();

}


void SkinTools::MatchByName()
{
//loop through the source
	for (int i = 0; i < pasteData.liveSourceBones.Count(); i++)
	{
		int sIndex = pasteData.liveSourceBones[i];
		TSTR sname = boneList[sIndex]->matchName;
//get the source name

	//loop through targets
		for (int j = 0; j < pasteData.liveTargetBones.Count(); j++)
		{
	//get the target target name
			int tIndex = pasteData.liveTargetBones[j];
			TSTR tname = targBoneList[tIndex]->matchName;
	//if they are equal copy them over
			if (tname == sname)
			{
				TargBones* targ = targBoneList[tIndex];
				targ->matchingBones.Append(1,&sIndex);
				pasteData.liveSourceBones.Delete(i,1);
				j = pasteData.liveTargetBones.Count();
				i--;
			}
		}
	}
	FilloutListBox();

}


void SkinTools::RemoveTargPrefix(BOOL remove)
{
	for (int i = 0; i < targBoneList.Count(); i++)
	{
		TargBones* targ = targBoneList[i];
		targ->matchName = targ->GetOriginalName();
	}


	if (remove)
	{
	//int number to remove
		if (targBoneList.Count() == 0) return;
		int ct = 0;
		BOOL done = FALSE;
	//loop through the targets

		while (!done)
		{
			TargBones* targ = targBoneList[0];
			TSTR firstName = targ->matchName;
			BOOL same = TRUE;
		//get first char
		//see if all names have it

			for (int j = 1; j < targBoneList.Count(); j++)
			{
				TargBones* targ = targBoneList[j];
				TSTR cname = targ->matchName;
				 
				if ( (ct >= cname.Length()) ||(cname[ct] != firstName[ct]) )
				{
					same = FALSE;
					j = targBoneList.Count();
				}
			}
			if (!same)
			{
				done = TRUE;
			}
			else ct++;

		}
		for (int i = 0; i < targBoneList.Count(); i++)
		{
			TargBones* targ = targBoneList[i];
			targ->matchName = targ->matchName;
			targ->matchName = targ->matchName.remove(0,ct);
		}

	FilloutListBox();
	}
	else
	///put back the old names
	{
//		for (int i = 0; i < targBoneList.Count(); i++)
//		{
//			TargBones* targ = targBoneList[i];
//			targ->matchName = targ->GetOriginalName();
//		}
	}
//	FilloutListBox();

}


void SkinTools::RemoveTargSufix(BOOL remove)
{

	if (remove)
	{
	//int number to remove
		if (targBoneList.Count() == 0) return;
		int ct = 0;
		BOOL done = FALSE;
	//loop through the targets

		while (!done)
		{
			TargBones* targ = targBoneList[0];
			TSTR firstName = targ->matchName;
			BOOL same = TRUE;
		//get first char
		//see if all names have it

			for (int j = 1; j < targBoneList.Count(); j++)
			{
				TargBones* targ = targBoneList[j];
				TSTR cname = targ->matchName;
				 
				if ( (ct >= cname.Length()) ||(cname[cname.Length() - ct -1] != firstName[firstName.Length() - ct -1]) )
				{
					same = FALSE;
					j = targBoneList.Count();
				}
			}
			if (!same)
			{
				done = TRUE;
			}
			else ct++;

		}
		for (int i = 0; i < targBoneList.Count(); i++)
		{
			TargBones* targ = targBoneList[i];
			targ->matchName = targ->matchName;
			targ->matchName = targ->matchName.remove(targ->matchName.Length()-ct,ct);
		}

	
	}
	else
	///put back the old names
	{
//		for (int i = 0; i < targBoneList.Count(); i++)
//		{
//			TargBones* targ = targBoneList[i];
//			targ->matchName = targ->GetOriginalName();
//		}
	}
	FilloutListBox();

}


void SkinTools::RemoveSourcePrefix(BOOL remove)
{

	for (int i = 0; i < targBoneList.Count(); i++)
	{
		BoneData* targ = boneList[i];
		targ->matchName = targ->GetOriginalName();
	}

	if (remove)
	{
	//int number to remove
		if (boneList.Count() == 0) return;
		int ct = 0;
		BOOL done = FALSE;
	//loop through the targets

		while (!done)
		{
			BoneData* targ = boneList[0];
			TSTR firstName = targ->matchName;
			BOOL same = TRUE;
		//get first char
		//see if all names have it

			for (int j = 1; j < boneList.Count(); j++)
			{
				BoneData* targ = boneList[j];
				TSTR cname = targ->matchName;
				 
				if ( (ct >= cname.Length()) ||(cname[ct] != firstName[ct]) )
				{
					same = FALSE;
					j = boneList.Count();
				}
			}
			if (!same)
			{
				done = TRUE;
			}
			else ct++;

		}
		for (int i = 0; i < boneList.Count(); i++)
		{
			BoneData* targ = boneList[i];
			targ->matchName = targ->matchName;
			targ->matchName = targ->matchName.remove(0,ct);
		}

	FilloutListBox();
	}
	else
	///put back the old names
	{
//		for (int i = 0; i < targBoneList.Count(); i++)
//		{
//			BoneData* targ = boneList[i];
//			targ->matchName = targ->GetOriginalName();
//		}
	}
//	FilloutListBox();

}


void SkinTools::RemoveSourceSufix(BOOL remove)
{

	if (remove)
	{
	//int number to remove
		if (boneList.Count() == 0) return;
		int ct = 0;
		BOOL done = FALSE;
	//loop through the targets

		while (!done)
		{
			BoneData* targ = boneList[0];
			TSTR firstName = targ->matchName;
			BOOL same = TRUE;
		//get first char
		//see if all names have it

			for (int j = 1; j < boneList.Count(); j++)
			{
				BoneData* targ = boneList[j];
				TSTR cname = targ->matchName;
				 
				if ( (ct >= cname.Length()) ||(cname[cname.Length() - ct -1] != firstName[firstName.Length() - ct -1]) )
				{
					same = FALSE;
					j = boneList.Count();
				}
			}
			if (!same)
			{
				done = TRUE;
			}
			else ct++;

		}
		for (int i = 0; i < boneList.Count(); i++)
		{
			BoneData* targ = boneList[i];
			targ->matchName = targ->matchName;
			targ->matchName = targ->matchName.remove(targ->matchName.Length()-ct,ct);
		}

	
	}
	else
	///put back the old names
	{
//		for (int i = 0; i < boneList.Count(); i++)
//		{
//			BoneData* targ = boneList[i];
//			targ->matchName = targ->GetOriginalName();
//		}
	}
	FilloutListBox();

}




void SkinTools::InitDialog()
{

	if (nukeMatchType)
		EnableWindow(GetDlgItem(floaterHWND,IDC_INTERPCOMBO),FALSE);

	SendMessage(GetDlgItem(floaterHWND,IDC_INTERPCOMBO),CB_ADDSTRING,0,(LPARAM)(TCHAR*)GetString(IDS_BYVERTEX));
	SendMessage(GetDlgItem(floaterHWND,IDC_INTERPCOMBO),CB_ADDSTRING,0,(LPARAM)(TCHAR*)GetString(IDS_BYFACE));
	SendMessage(GetDlgItem(floaterHWND,IDC_INTERPCOMBO),CB_SETCURSEL,0,0);

	iThreshold = SetupFloatSpinner(	floaterHWND,IDC_THRESHOLD_SPIN,IDC_THRESHOLD,
									0.0f,50.0f,1.0f);	

}

void SkinTools::DestroyDialog(BOOL ok)
{
//	float threshold;
	threshold = iThreshold->GetFVal();
	ReleaseISpinner(iThreshold);
	matchType = SendMessage(GetDlgItem(floaterHWND,IDC_INTERPCOMBO),CB_GETCURSEL,0,0);



}


