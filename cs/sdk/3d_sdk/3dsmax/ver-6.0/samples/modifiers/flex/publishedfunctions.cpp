
#include "lag.h"

INT_PTR CALLBACK OptionDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Function Publishing descriptor for Mixin interface
//*****************************************************
static FPInterfaceDesc lag_interface(
    LAG_INTERFACE, _T("flexOps"), 0, &lagDesc, FP_MIXIN,
		lag_paint, _T("paint"), 0, TYPE_VOID, 0, 0,
		lag_setreference, _T("setReference"), 0, TYPE_VOID, 0, 0,
		lag_reset, _T("reset"), 0, TYPE_VOID, 0, 0,
		lag_addforce, _T("addForce"), 0, TYPE_VOID, 0, 1,
			_T("force"), 0, TYPE_INODE,
		lag_removeforce, _T("removeForce"), 0, TYPE_VOID, 0, 1,
			_T("force"), 0, TYPE_INT,

		lag_numbervertices, _T("numberVertices"), 0, TYPE_INT, 0, 0,

		lag_selectvertices, _T("selectVertices"), 0, TYPE_VOID, 0, 2,
			_T("sel"), 0, TYPE_BITARRAY,
			_T("update"), 0, TYPE_BOOL,

		lag_getselectedvertices, _T("getSelectedVertices"), 0, TYPE_BITARRAY, 0, 0,


		lag_getvertexweight, _T("getVertexWeight"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INT,
		lag_setvertexweight, _T("setVertexWeight"), 0, TYPE_VOID, 0, 2,
			_T("indices"), 0, TYPE_INT_TAB,
			_T("values"), 0, TYPE_FLOAT_TAB,

 		lag_setedgelist, _T("setEdgeList"), 0, TYPE_VOID, 0, 2,
			_T("sel"), 0, TYPE_BITARRAY,
			_T("update"), 0, TYPE_BOOL,

		lag_getedgelist, _T("getEdgeList"), 0, TYPE_BITARRAY, 0, 0,

		lag_addspringselection, _T("addSpringFromSelection"), 0, TYPE_VOID, 0, 2,
			_T("flag"), 0, TYPE_INT,
			_T("addDuplicates"), 0, TYPE_BOOL,

		lag_addspring, _T("addSpring"), 0, TYPE_VOID, 0, 4,
			_T("indexA"), 0, TYPE_INT,
			_T("indexB"), 0, TYPE_INT,
			_T("flag"), 0, TYPE_INT,
			_T("addDuplicates"), 0, TYPE_BOOL,
		lag_removeallsprings, _T("removeAllSprings"), 0, TYPE_VOID, 0, 0,

		lag_addspring_button, _T("addSpringButton"), 0, TYPE_VOID, 0, 0,
		lag_removespring_button, _T("removeSpringButton"), 0, TYPE_VOID, 0, 0,
		lag_option_button, _T("optionsButton"), 0, TYPE_VOID, 0, 0,
		lag_simplesoft_button, _T("createSimpleSoftButton"), 0, TYPE_VOID, 0, 0,

		lag_removespring_by_end,_T("removeSpringByEnd"), 0, TYPE_VOID, 0, 1,
			_T("a"), 0, TYPE_INT,
		lag_removespring_by_both_ends,_T("removeSpringByEnds"), 0, TYPE_VOID, 0, 2,
			_T("a"), 0, TYPE_INT,
			_T("b"), 0, TYPE_INT,
		lag_removespringbyindex,_T("removeSpringByIndex"), 0, TYPE_VOID, 0, 1,
			_T("index"), 0, TYPE_INT,
		lag_numbersprings,_T("numberSprings"), 0, TYPE_INT, 0, 0,

		lag_getspringgroup,_T("getSpringGroup"), 0, TYPE_INT, 0, 1,
			_T("index"), 0, TYPE_INT,
		lag_setspringgroup,_T("setSpringGroup"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("group"), 0, TYPE_INT,

		lag_getspringlength,_T("getSpringLength"), 0, TYPE_FLOAT, 0, 1,
			_T("index"), 0, TYPE_INT,
		lag_setspringlength,_T("setSpringLength"), 0, TYPE_VOID, 0, 2,
			_T("index"), 0, TYPE_INT,
			_T("length"), 0, TYPE_FLOAT,

		lag_getindex,_T("getIndex"), 0, TYPE_INT, 0, 2,
			_T("a"), 0, TYPE_INT,
			_T("b"), 0, TYPE_INT,

      end
      );



void *LagClassDesc::Create(BOOL loading)
		{
		AddInterface(&lag_interface);
		return new LagMod;
		}

FPInterfaceDesc* ILagMod::GetDesc()
	{
	 return &lag_interface;
	}


void LagMod::fnPaint()
	{
	SendMessage(hPaint,WM_COMMAND,IDC_PAINT,0);
	}

void LagMod::fnSetReference()
	{
	SendMessage(hAdvance,WM_COMMAND,IDC_SETREFERENCE_BUTTON,0);
	}

void LagMod::fnReset()
	{
	SendMessage(hAdvance,WM_COMMAND,IDC_RESET_BUTTON,0);
	}

void LagMod::fnAddForce(INode *node)
	{
	PB2Value v;
	v.r = node;
	if (validator.Validate(v))
		pblock2->Append(lag_force_node,1,&node);
	}

void LagMod::fnRemoveForce(int whichNode)
	{
	if (whichNode == -1)
		{
		whichNode = SendMessage(GetDlgItem(hParams,IDC_LIST1),LB_GETCURSEL,0,0);	
		}
	else whichNode--;

	int ct = pblock2->Count(lag_force_node);

	pblock2->Delete(lag_force_node,whichNode,1);

	}

int	LagMod::fnNumberVertices()
	{

	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;
		ct = lmd->SpringList.Count();
		}
	return ct;	

	}

void LagMod::fnSelectVertices(BitArray *selList, BOOL updateViews)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;
		
		for (int i =  0; i < lmd->wsel.Count(); i++)
			lmd->wsel[i] = 0;
		for (int index = 0; index < selList->GetSize(); index++)
			{
			if (((*selList)[index]))
				{
				if ((index <0) || (index >= lmd->wsel.Count()))
					{
					}
				else lmd->wsel[index] = 1;
				}
			}
		if (updateViews)
			{
			NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			}
		}

	}
BitArray* LagMod::fnGetSelectedVertices()
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		tempBitArray.SetSize(lmd->wsel.Count());
		for (int i =  0; i < lmd->wsel.Count(); i++)
			tempBitArray.Set(i,lmd->wsel[i]);

		}
	return &tempBitArray;
	}

float LagMod::fnGetVertexWeight(int index)
	{
	float weight = 0.0f;
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;
		index--;
		if ((index >=0) && (index<lmd->SpringList.Count()))
			{
			weight = lmd->SpringList[index].InheritVel;
			}
		}

	return weight;

	}
void LagMod::fnSetVertexWeight(Tab<int> *indexList, Tab<float> *values)
	{
	int indexCount = indexList->Count();
	int valCount = values->Count();
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;
		for (int i =0; i < indexCount; i++)
			{
			int id = (*indexList)[i];
			if (i < valCount)
				{
				float val = (*values)[i];
				id--;
				if ((id >=0) && (id < lmd->SpringList.Count()))
					{
					lmd->SpringList[id].InheritVel = val;
					lmd->SpringList[id].modified = TRUE;					
					}

				}
			}
		}

	}

void	LagMod::fnSetEdgeList(BitArray *selList, BOOL updateViews)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;
		
		for (int i =  0; i < lmd->esel.Count(); i++)
			lmd->esel[i] = 0;
		for (int index = 0; index < selList->GetSize(); index++)
			{
			if (((*selList)[index]))
				{
				if ((index <0) || (index >= lmd->esel.Count()))
					{
					}
				else lmd->esel[index] = 1;
				}
			}
		if (updateViews)
			{
			NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			}
		}


	}
BitArray* LagMod::fnGetEdgeList()
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		tempBitArray.SetSize(lmd->esel.Count());
		for (int i =  0; i < lmd->esel.Count(); i++)
			tempBitArray.Set(i,lmd->esel[i]);

		}
	return &tempBitArray;

	}



/************************* OLD for legacy support of old scripter access stuff *****************************************/

#define get_lagdef_mod()																\
	Modifier *mod = arg_list[0]->to_modifier();										\
	Class_ID id = mod->ClassID();													\
	if ( id != Class_ID(LAZYID) )	\
		throw RuntimeError(GetString(IDS_PW_NOT_LAGDEF_ERROR), arg_list[0]);			\
	LagMod *lmod = (LagMod*)mod;			


// Maxscript stuff


//just returns the number of vertices in the system
def_struct_primitive( flexOps_getNumberVertices,flexOps, "GetNumberVertices" );
//SelectVertices number/array/bitarray
//selects the vertices specified
def_struct_primitive (flexOps_selectVerts, flexOps,			"SelectVertices" );
//GetVertexWeightCount vertexid
//returns the inlfuence of that vertex
def_struct_primitive (flexOps_getVertexWeight,flexOps,		"GetVertexWeight" );
//SetVertexWeights VerticesID  Weights
//assigns an array of vertices to an array of weights
//VerticesID and Weights can be arrays or just numbers but if they are arrays they need to be the same length
def_struct_primitive (flexOps_setVertWeights,flexOps,		"SetVertexWeights" );

//isVertexEdge vertid
//return is a vertex is an edge edge 
def_struct_primitive (flexOps_isVertEdge,flexOps,		"isEdgeVertex" );
//SetVertexEdges vertidlist
//sets the vertices to edge vertices
def_struct_primitive (flexOps_setVertEdges,flexOps,	"SetEdgeVertices" );
//ClearVertexEdges vertidlist
//clears the vertices to edge vertices
def_struct_primitive (flexOps_clearVertEdges,flexOps,	"ClearEdgeVertices" );




Value*
flexOps_getNumberVertices_cf(Value** arg_list, int count)
{
	check_arg_count(getNumberVertices, 1, count);
	get_lagdef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !lmod->ip ) throw RuntimeError(GetString(IDS_PW_LAG_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	lmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList[0]->localData;
		ct = lmd->SpringList.Count();
		}
	return Integer::intern(ct);	
}


Value*
flexOps_selectVerts_cf(Value** arg_list, int count)
{
	check_arg_count(selectVerts, 2, count);
	get_lagdef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !lmod->ip ) throw RuntimeError(GetString(IDS_PW_LAG_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	lmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	Value* ival = arg_list[1];

//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList[0]->localData;
		for (int i = 0; i < lmd->wsel.Count(); i++)
			lmd->wsel[i] = 0;

		int index;
		if (is_number(ival))   // single index
			{
			index = ival->to_int()-1;
			if ((index <0) || (index >= lmd->wsel.Count()) )
				throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
			else lmd->wsel[index] = 1;
			}

		else if (is_array(ival))   // array of indexes
			{
			Array* aval = (Array*)ival;
			for (int i = 0; i < aval->size; i++)
				{
				ival = aval->data[i];
				if (is_number(ival))   // single index
					{
					index = ival->to_int()-1;
					if ((index <0) || (index >= lmd->wsel.Count()))
						throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
					else lmd->wsel[index] = 1;

					}
				}

			}
		else if (is_BitArrayValue(ival))   // array of indexes
			{
			BitArrayValue *list = (BitArrayValue *) ival;
			for (int index = 0; index < list->bits.GetSize(); index++)
				{
				if (list->bits[index])
					{
					if ((index <0) || (index >= lmd->wsel.Count()))
						throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
					else lmd->wsel[index] = 1;
					}
				}

			}



		}

	lmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	lmod->ip->RedrawViews(lmod->ip->GetTime());


	return &ok;	
}


Value*
flexOps_getVertexWeight_cf(Value** arg_list, int count)
{
	check_arg_count(getVertexWeight, 2, count);
	get_lagdef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !lmod->ip ) throw RuntimeError(GetString(IDS_PW_LAG_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	lmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	float w = 0;
	Value* ival = arg_list[1];

	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList[0]->localData;
		int index;
		if (is_number(ival))   // single index
			{
			index = ival->to_int()-1;
			if ((index <0) || (index >= lmd->SpringList.Count()) )
				throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
			else w =1.0f - lmd->SpringList[index].InheritVel;
			}

		}
	return Float::intern(w);	





}


Value*
flexOps_setVertWeights_cf(Value** arg_list, int count)
{
	check_arg_count(setVertWeights, 3, count);
	get_lagdef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !lmod->ip ) throw RuntimeError(GetString(IDS_PW_LAG_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	lmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;

	Value* vertsval = arg_list[1];
	Value* weightsval = arg_list[2];



//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList[0]->localData;
//		if (vertID >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);

		if (is_number(weightsval) && is_number(vertsval))  // single index
			{
			int vertID = vertsval->to_int()-1;
			float weight = weightsval->to_float();
			weight = 1.0f-weight;
			if (weight < 0.0f ) weight = 0.0f;
			if (weight > 1.0f ) weight = 1.0f;
			if (vertID >= lmd->SpringList.Count()) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
			lmd->SpringList[vertID].InheritVel = weight;
			lmd->SpringList[vertID].modified = TRUE;
			}

		else if (is_array(weightsval) && is_array(vertsval))   // array of indexes
			{

			Array* wval = (Array*)weightsval;
			Array* vval = (Array*)vertsval;
			if (wval->size != vval->size) throw RuntimeError(GetString(IDS_PW_ERROR_COUNT), arg_list[0]);

			for (int i = 0; i < wval->size; i++)
				{
				Value *vertval = vval->data[i];
				Value *weightval = wval->data[i];
				if ( (is_number(vertval)) && (is_number(weightval)))  // single index
					{
					int vertID = vertval->to_int()-1;
					float weight = weightval->to_float();
					weight = 1.0f - weight;
					if (weight < 0.0f ) weight = 0.0f;
					if (weight > 1.0f ) weight = 1.0f;

					if (vertID >= lmd->SpringList.Count()) throw RuntimeError(GetString(IDS_PW_ERROR_COUNT), arg_list[0]);
					lmd->SpringList[vertID].InheritVel = weight;
					lmd->SpringList[vertID].modified = TRUE;

					}
				}


			}



		}
	lmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	lmod->ip->RedrawViews(lmod->ip->GetTime());


	return &ok;	
}


Value*
flexOps_isVertEdge_cf(Value** arg_list, int count)
{
	check_arg_count(isVertEdge, 2, count);
	get_lagdef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !lmod->ip ) throw RuntimeError(GetString(IDS_PW_LAG_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	lmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;
	Value* vertsval = arg_list[1];

	if (objects != 0)
		{

		LagModData *lmd = (LagModData*)mcList[0]->localData;
		if (is_number(vertsval))  // single index
			{
			int vertID = vertsval->to_int()-1;
			ct = lmd->esel[vertID];
			}
		}
	return Integer::intern(ct);	
}


Value*
flexOps_setVertEdges_cf(Value** arg_list, int count)
{
	check_arg_count(setVertEdges, 2, count);
	get_lagdef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !lmod->ip ) throw RuntimeError(GetString(IDS_PW_LAG_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	lmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;

	Value* vertsval = arg_list[1];



//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList[0]->localData;
//		if (vertID >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);

		if (is_number(vertsval))  // single index
			{
			int vertID = vertsval->to_int()-1;
			if (vertID >= lmd->SpringList.Count()) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
			lmd->esel[vertID] = 1;
			}

		else if (is_array(vertsval))   // array of indexes
			{

			Array* vval = (Array*)vertsval;

			for (int i = 0; i < vval->size; i++)
				{
				Value *vertval = vval->data[i];
				if ( (is_number(vertval)) )  // single index
					{
					int vertID = vertval->to_int()-1;
					if (vertID >= lmd->SpringList.Count()) throw RuntimeError(GetString(IDS_PW_ERROR_COUNT), arg_list[0]);
					lmd->esel[vertID] = 1;

					}
				}


			}



		}
	lmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	lmod->ip->RedrawViews(lmod->ip->GetTime());


	return &ok;	
}

Value*
flexOps_clearVertEdges_cf(Value** arg_list, int count)
{
	check_arg_count(clearVertEdges, 2, count);
	get_lagdef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !lmod->ip ) throw RuntimeError(GetString(IDS_PW_LAG_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	lmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();
	int ct = 0;

	Value* vertsval = arg_list[1];



//	BitArray selList = arg_list[1]->to_bitarray();
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList[0]->localData;
//		if (vertID >= bmd->VertexData.Count() ) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);

		if (is_number(vertsval))  // single index
			{
			int vertID = vertsval->to_int()-1;
			if (vertID >= lmd->SpringList.Count()) throw RuntimeError(GetString(IDS_PW_EXCEEDED_VERTEX_COUNT), arg_list[0]);
			lmd->esel[vertID] = 0;
			}

		else if (is_array(vertsval))   // array of indexes
			{

			Array* vval = (Array*)vertsval;

			for (int i = 0; i < vval->size; i++)
				{
				Value *vertval = vval->data[i];
				if ( (is_number(vertval)) )  // single index
					{
					int vertID = vertval->to_int()-1;
					if (vertID >= lmd->SpringList.Count()) throw RuntimeError(GetString(IDS_PW_ERROR_COUNT), arg_list[0]);
					lmd->esel[vertID] = 0;

					}
				}


			}



		}
	lmod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	lmod->ip->RedrawViews(lmod->ip->GetTime());


	return &ok;	
}

BOOL LagMod::IsDupe(LagModData *lmd,int a, int b)
	{
	if (occuppiedList)
		{
		if (a>b)
			{
			int temp;
			temp = a;
			a = b;
			b = temp;
			}
//		BitArray p = occuppiedList[a];
//		int ct = p.GetSize();
//		if (p[b])
//			return TRUE;
		int newB = b-a;
		int ct = occuppiedList[a].GetSize();
		if (occuppiedList[a][newB])
			return TRUE;
		occuppiedList[a].Set(newB,TRUE);
		return FALSE;
		}
	else
		{
		int ct = lmd->edgeSprings.Count();
		for (int i = 0; i < ct; i++)
			{
			if (  ((a==lmd->edgeSprings[i].v1) && (b==lmd->edgeSprings[i].v2) ) ||
				  ((b==lmd->edgeSprings[i].v1) && (a==lmd->edgeSprings[i].v2) ) )
				  return TRUE;

			}
		return FALSE;
		}
	}


void LagMod::fnAddSingleSpringFromSelection(int flag, BOOL addDupes)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		AddSingleSpringFromSelection(lmd, flag,addDupes);
		}

	}

void LagMod::AddSingleSpringFromSelection(LagModData *lmd, int flag, BOOL addDupes)
	{
	int ct = lmd->wsel.Count();

	int ids[2];
	int index =0;
	for (int i = 0; i < ct; i++)
		{
		if (lmd->wsel[i] == 1)
			{
			if (index <2)
				{
				ids[index] = i;
				}
			index++;
			if (index > 2) i = ct;
			}
		}
	if (!addDupes)
		{
		if (IsDupe(lmd,ids[0], ids[1]))
			return;
		}
	if (index == 2)
		{	
		EdgeBondage t;
		t.v1 = ids[0];
		t.v2 = ids[1];
		t.flags = flag;
		Point3 a,b;
		a = lmd->SpringList[ids[0]].LocalPt;
		b = lmd->SpringList[ids[1]].LocalPt;
		t.dist = Length(a-b);
		lmd->edgeSprings.Append(1,&t,1);
		}
	}
void LagMod::fnAddSpring(int a, int b, int flag, BOOL addDupes)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);
	a--;
	b--;

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		AddSpring(lmd, a, b, flag,addDupes);
		}

	}

void LagMod::AddSpring(LagModData *lmd, int a, int b, int flag, BOOL addDupes)
	{
	if (!addDupes)
		{
		if (IsDupe(lmd,a, b))
			return;
		}
	if (a==b) return;

	EdgeBondage t;
	t.v1 = a;
	t.v2 = b;
	t.flags = flag;
	Point3 pa,pb;
	pa = lmd->SpringList[a].LocalPt;
	pb = lmd->SpringList[b].LocalPt;
	t.dist = Length(pa-pb);
	lmd->edgeSprings.Append(1,&t,1);
	}
/*
void LagMod::fnDeleteSpring(int a, int b)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	a--;
	b--;
	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		DeleteSpring(lmd, a, b);
		}

	}
void LagMod::fnDeleteSpring(int index)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		DeleteSpring(lmd, index);
		}

	}
void LagMod::DeleteSpring(LagModData *lmd, int a, int b)
	{
	for (int i = 0; i < lmd->edgeSprings.Count();i++)
		{
		if ( ((a ==  lmd->edgeSprings[i].v1) && (b ==  lmd->edgeSprings[i].v2)) ||
			 ((b ==  lmd->edgeSprings[i].v1) && (a ==  lmd->edgeSprings[i].v2)) )
			{
			lmd->edgeSprings.Delete(i,1);
			}
		}

	}

void LagMod::DeleteSpring(LagModData *lmd, int index)
	{
	if ((index >=0) && (index<lmd->edgeSprings.Count()))
		lmd->edgeSprings.Delete(index,1);
	}


void LagMod::fnSetSpringFlag(int index, int flag)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		SetSpringFlag(lmd, index, flag);
		}
	
	}
void LagMod::SetSpringFlag(LagModData *lmd,int index, int flag)
	{
	if ((index >=0) && (index<lmd->edgeSprings.Count()))
		{
		lmd->edgeSprings[index].flags = flag;
		}

	}

int	LagMod::fnGetSpringFlag(int index)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		return GetSpringFlag(lmd, index);
		}
	
	return 0;
	}
int	LagMod::GetSpringFlag(LagModData *lmd,int index)
	{
	if ((index >=0) && (index<lmd->edgeSprings.Count()))
		{
		return lmd->edgeSprings[index].flags;
		}
	return 0;

	}

*/
void LagMod::fnRemoveAllSprings()
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		RemoveAllSprings(lmd);
		}

	}
void LagMod::RemoveAllSprings(LagModData *lmd)
	{
	lmd->edgeSprings.ZeroCount();
	}



void LagMod::fnAddSpringButton()
	{
	LagModEnumProc lmdproc(this);
	EnumModContexts(&lmdproc);

	for (int i =0; i < lmdData.Count(); i++)
		lmdData[i]->addSprings = TRUE;
    NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	}
void LagMod::fnRemoveSpringButton()
	{
	LagModEnumProc lmdproc(this);
	EnumModContexts(&lmdproc);

	for (int i =0; i < lmdData.Count(); i++)
		lmdData[i]->removeSprings = TRUE;
    NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
	}
void LagMod::fnOptionButton()
	{
	LagModEnumProc lmdproc(this);
	EnumModContexts(&lmdproc);

	for (int i =0; i < lmdData.Count(); i++)
		lmdData[i]->computeEdges = TRUE;
					
	int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_SPRINGOPTION_DIALOG),
				hAdvanceSprings,OptionDlgProc,(LPARAM)this);	
	}

int radioList[] = {IDC_RADIO1,IDC_RADIO2,IDC_RADIO3,IDC_RADIO4,IDC_RADIO5};

static INT_PTR CALLBACK OptionDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

	static ISpinnerControl *spinRadius;

	LagMod *mod = (LagMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
		    mod = (LagMod *)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
//			CheckRadioButton(  hWnd,          // handle to dialog box
//								IDC_RADIO1, // identifier of first radio button in group
//								IDC_RADIO5,  // identifier of last radio button in group
//								mod->addMode);  // identifier of radio button to select 
			CheckDlgButton(  hWnd,radioList[mod->addMode],BST_CHECKED);
 
			spinRadius = SetupFloatSpinner(
				hWnd,IDC_LAG_HOLDSHAPE_OUTER_SPIN,IDC_LAG_HOLDSHAPE_OUTER,
				0.0f,10000.0f,mod->holdRadius);			

			TSTR maxStr;
			if (mod->edgeLength>0)
				{
				maxStr.printf(" %s %0.1f",GetString(IDS_AVERAGE),mod->edgeLength);
				SendMessage(GetDlgItem(hWnd, IDC_EDGEDATA), WM_SETTEXT, 0, (LPARAM)(char *)maxStr);

				maxStr.printf(" %s %0.1f\n",GetString(IDS_MAX),mod->maxEdgeLength);
				SendMessage(GetDlgItem(hWnd, IDC_EDGEDATA1), WM_SETTEXT, 0, (LPARAM)(char *)maxStr);

				maxStr.printf(" %s %0.1f\n",GetString(IDS_MIN),mod->minEdgeLength);
				SendMessage(GetDlgItem(hWnd, IDC_EDGEDATA2), WM_SETTEXT, 0, (LPARAM)(char *)maxStr);

				}

			break;



			break;

			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDOK:
					{
//get radius and addmode
					if (IsDlgButtonChecked( hWnd,IDC_RADIO1) == BST_CHECKED )
						mod->addMode = 0;
					else if (IsDlgButtonChecked( hWnd,IDC_RADIO2) == BST_CHECKED )
						mod->addMode = 1;
					else if (IsDlgButtonChecked( hWnd,IDC_RADIO3) == BST_CHECKED )
						mod->addMode = 2;
					else if (IsDlgButtonChecked( hWnd,IDC_RADIO4) == BST_CHECKED )
						mod->addMode = 3;
					else mod->addMode = 4;


					mod->holdRadius = spinRadius->GetFVal();
					ReleaseISpinner(spinRadius);

					mod->pblock2->SetValue(lag_add_mode,0,mod->addMode);
					mod->pblock2->SetValue(lag_hold_radius,0,mod->holdRadius);

					EndDialog(hWnd,1);
					break;
					}
				case IDCANCEL:
					ReleaseISpinner(spinRadius);
					EndDialog(hWnd,0);
					break;
				}
			break;

		
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			break;

	
		default:
			return FALSE;
		}
	return TRUE;
	}


void LagMod::fnSimpleSoftButton()
	{
	LagModEnumProc lmdproc(this);
	EnumModContexts(&lmdproc);

	for (int i =0; i < lmdData.Count(); i++)
		lmdData[i]->simpleSoft = TRUE;
    NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

	}




void LagMod::fnRemoveSpring(int a)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	a--;
	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		RemoveSpring(lmd, a);
		}

	}
void LagMod::RemoveSpring(LagModData *lmd,int a)
	{
	for (int i = 0; i < lmd->edgeSprings.Count();i++)
		{
		if ( (a ==  lmd->edgeSprings[i].v1) || (a ==  lmd->edgeSprings[i].v2))
		
			{
			lmd->edgeSprings.Delete(i,1);
			i--;
			}
		}

	}
void LagMod::fnRemoveSpring(int a,int b)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	a--;
	b--;
	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		RemoveSpring(lmd, a, b);
		}

	}
void LagMod::RemoveSpring(LagModData *lmd,int a,int b)
	{
	for (int i = 0; i < lmd->edgeSprings.Count();i++)
		{
		if ( ((a ==  lmd->edgeSprings[i].v1) && (b ==  lmd->edgeSprings[i].v2)) ||
			 ((b ==  lmd->edgeSprings[i].v1) && (a ==  lmd->edgeSprings[i].v2)) )
			{
			lmd->edgeSprings.Delete(i,1);
			}
		}

	}
void LagMod::fnRemoveSpringByIndex(int index)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);
	index--;

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		RemoveSpringByIndex(lmd, index);
		}

	}
void LagMod::RemoveSpringByIndex(LagModData *lmd,int index)
	{
	if ((index >=0) && (index<lmd->edgeSprings.Count()))
		lmd->edgeSprings.Delete(index,1);

	}


int LagMod::fnNumberSprings()
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		return NumberSprings(lmd);
		}
	return 0;
	}
int LagMod::NumberSprings(LagModData *lmd)
	{
	return lmd->edgeSprings.Count();

	}

int	LagMod::fnGetSpringGroup(int index)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);
	index--;

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		return GetSpringGroup(lmd, index);
		}
	
	return 0;

	}

int	LagMod::GetSpringGroup(LagModData *lmd,int index)
	{
	if ((index >=0) && (index<lmd->edgeSprings.Count()))
		{
		return lmd->edgeSprings[index].flags+1;
		}
	return 0;

	}

void LagMod::fnSetSpringGroup(int index, int group)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);
	index--;
	group--;
	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		SetSpringGroup(lmd, index, group);
		}

	}
void LagMod::SetSpringGroup(LagModData *lmd,int index, int group)
	{
	if ((index >=0) && (index<lmd->edgeSprings.Count()))
		{
		lmd->edgeSprings[index].flags= group;
		}

	}


float LagMod::fnGetSpringLength(int index)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);
	index--;

	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		return GetSpringLength(lmd, index);
		}
	
	return 0.0f;

	}

float LagMod::GetSpringLength(LagModData *lmd,int index)
	{
	if ((index >=0) && (index<lmd->edgeSprings.Count()))
		{
		return lmd->edgeSprings[index].dist;
		}
	return 0;

	}

void LagMod::fnSetSpringLength(int index, float len)
	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);
	index--;
	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		SetSpringLength(lmd, index, len);
		}

	}
void LagMod::SetSpringLength(LagModData *lmd,int index, float len)
	{
	if ((index >=0) && (index<lmd->edgeSprings.Count()))
		{
		lmd->edgeSprings[index].dist = len;
		}

	}

int	LagMod::fnGetIndex(int a, int b)

	{
	LagModContextEnumProc mcList;
	mcList.modList.ZeroCount();
	EnumModContexts(&mcList);

	a--;
	b--;
	int objects = mcList.modList.Count();
	int ct = 0;
	if (objects != 0)
		{
		LagModData *lmd = (LagModData*)mcList.modList[0]->localData;

		return GetIndex(lmd, a, b);
		}

	return 0;	
	}
int	LagMod::GetIndex(LagModData *lmd,int a, int b)
	{
	for (int i = 0; i < lmd->edgeSprings.Count();i++)
		{
		if ( ((a ==  lmd->edgeSprings[i].v1) && (b ==  lmd->edgeSprings[i].v2)) ||
			 ((b ==  lmd->edgeSprings[i].v1) && (a ==  lmd->edgeSprings[i].v2)) )
			{
			return i+1;
			}
		}
	return 0;
	}
