
#include "painterInterface.h"
#include "IpainterInterface.h"


void PainterInterface::RunCommand( TSTR category, TSTR command)
	{
	TSTR scriptUI;

	scriptUI.printf("macros.run \"%s\" \"%s\" ",category,command);

	init_thread_locals();
	push_alloc_frame();
	one_typed_value_local(StringStream* util_script);

	vl.util_script = new StringStream (scriptUI);

	save_current_frames();
	try 
	{
	   	vl.util_script->execute_vf(NULL, 0);
	}
	catch (MAXScriptException& e)
	{
		restore_current_frames();
		MAXScript_signals = 0;
		error_message_box(e, _T("Unwrap UI Script CloseUnwrapUI Macro not found"));
	}
	catch (...)
	{
		restore_current_frames();
		error_message_box(UnknownSystemException (), _T("Unwrap UI Script  CloseUnwrapUI Macro not found"));
	}
	vl.util_script->close();
	pop_value_locals();
	pop_alloc_frame();

	}

void	PainterInterface::RunMacro(TSTR macro)
	{
	TSTR scriptUI;

//	scriptUI.printf("mcrfile = openFile   \"UI\\MacroScripts\\Macro_UnwrapUI.mcr\" ; execute mcrfile",macro);
	scriptUI.printf("mcrfile = openFile   \"%s\" ; execute mcrfile",macro);

//	scriptCallback = TRUE;

	init_thread_locals();
	push_alloc_frame();
	one_typed_value_local(StringStream* util_script);

	vl.util_script = new StringStream (scriptUI);

	save_current_frames();
	try 
		{
   		vl.util_script->execute_vf(NULL, 0);
		}
	catch (MAXScriptException& e)
		{
		restore_current_frames();
		MAXScript_signals = 0;
		error_message_box(e, _T("Paint macro not found"));
//		scriptCallback = FALSE;
		}
	catch (...)
		{
		restore_current_frames();
		error_message_box(UnknownSystemException (), _T("Paint macro not found"));
//		scriptCallback = FALSE;
		}
	vl.util_script->close();
	pop_value_locals();
	pop_alloc_frame();
	
	
	}

void PainterInterface::RunFunction(Value* fn)
{
if (fn == NULL) return;

	init_thread_locals();
	push_alloc_frame();
	save_current_frames();
	try 
	{
		fn->apply(NULL,0);
	}
	catch (MAXScriptException)// & e)
	{
		restore_current_frames();
		// any error disables the callback
		fn = NULL;
	}
	catch (...)
	{
		restore_current_frames();
		// any error disables the callback
		fn = NULL;
	}

	pop_alloc_frame();
	// callback fn returns #continue -> keep going, false -> stop pickpoint




}


void PainterInterfaceActionsIMP::fnInitializeNodes(int flags,Tab<INode*> *nodeList) 
	{ 
	thePainterInterface.InitializeNodes(flags,*nodeList);
	ScriptPrint("PainterInterface Initializing nodes\n"); 
	}

int  PainterInterfaceActionsIMP::fnGetNumberNodes()
	{
	return thePainterInterface.pblock->Count(painterinterface_nodelist);
	}

INode* PainterInterfaceActionsIMP::fnGetNode(int index)
	{
	index--;
	if ( (index >= 0) && (index < thePainterInterface.pblock->Count(painterinterface_nodelist)))
		return thePainterInterface.pblock->GetINode(painterinterface_nodelist,0,index);
	else return NULL;
	}

void PainterInterfaceActionsIMP::fnUpdateMeshes(BOOL updatePointGather)
	{
	thePainterInterface.UpdateMeshes(updatePointGather);
	}
void PainterInterfaceActionsIMP::fnStartPaintSession() 
	{ 
	thePainterInterface.StartPaintSession();
	ScriptPrint("PainterInterface Starting Paint Session\n"); 
	}

void PainterInterfaceActionsIMP::fnEndPaintSession() 
	{ 
	thePainterInterface.EndPaintSession();
	ScriptPrint("PainterInterface End Paint Session\n"); 
	}

void PainterInterfaceActionsIMP::fnPaintOptions() 
	{ 
	thePainterInterface.BringUpOptions();
	ScriptPrint("PainterInterface Options Dialog\n"); 
	}

BOOL PainterInterfaceActionsIMP::fnInPaintMode()
	{
	return thePainterInterface.InPaintMode();
	}

void PainterInterfaceActionsIMP::fnLoadPaintMacro(TCHAR *macroString,TCHAR *category)
	{ 
//	thePainterInterface.BringUpOptions();
	thePainterInterface.RunMacro(macroString);
	TSTR tempString;
	tempString.printf("%s",category);
	
	if (thePainterInterface.SetCallbackCategory(tempString))
		ScriptPrint("PainterInterface MacroCallback %s Category %s\n",macroString,category); 
	else ScriptPrint("PainterInterface MacroCallback failed\n"); 
	}


BOOL PainterInterfaceActionsIMP::fnGetIsHit(int tabindex)
	{
	tabindex -= 1;
	return thePainterInterface.GetIsHit(tabindex);
	}

void PainterInterfaceActionsIMP::fnGetHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
								  Point3 &hitPointWorld, Point3 &hitNormalWorld,
								  float &radius, float &str, int index)
	{
	index -= 1;
	thePainterInterface.GetHitPointData(hitPointLocal, hitNormalLocal,
										hitPointWorld, hitNormalWorld,
										radius, str,  index);

	}

void PainterInterfaceActionsIMP::fnGetMirrorHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld, int tabindex)
	{
	tabindex -= 1;
	thePainterInterface.GetMirrorHitPointData(hitPointLocal, hitNormalLocal,
										hitPointWorld, hitNormalWorld,
										 tabindex);
	}

void PainterInterfaceActionsIMP::fnGetHitFaceData(Point3 &bary, int &index, INode *node, int tabindex)
	{
	tabindex -= 1;
	thePainterInterface.GetHitFaceData(bary, index, node,tabindex);
	index++;
	}

void PainterInterfaceActionsIMP::fnGetHitPressureData(BOOL &shift, BOOL &ctrl, BOOL &alt, float &pressure , int tabindex)
	{
	tabindex -= 1;
	thePainterInterface.GetHitPressureData(shift, ctrl, alt, pressure , tabindex);

	}

Point2 *PainterInterfaceActionsIMP::fnGetHitMousePos(int tabindex)
	{
	tabindex--;
	int count;
	customHitMousePos = Point2(0.0f,0.0f);
	IPoint2 *mousePos = thePainterInterface.RetrieveMouseHitList(count);
	if ((tabindex >=0) && (tabindex < count))
		{
		customHitMousePos.x = mousePos[tabindex].x;
		customHitMousePos.y = mousePos[tabindex].y;
		}
	return &customHitMousePos;

	}

int PainterInterfaceActionsIMP::fnGetHitTime(int tabindex)
	{
	tabindex--;
	int count;
	int *time = thePainterInterface.RetrieveTimeList(count);
	if ((tabindex >=0) && (tabindex < count))
		return time[tabindex];
	else return time[count-1];
	}
float PainterInterfaceActionsIMP::fnGetHitDist(int tabindex)
	{

	if (tabindex <= 0)
		tabindex =  thePainterInterface.GetHitCount()-1;
	else tabindex--;

	Point3 lpoint,lnorm,wpoint1,wnorm1;
	Point3 wpoint2,wnorm2;
	float radius, str;
	thePainterInterface.GetHitPointData(lpoint, lnorm,
										wpoint1, wnorm1,
										radius, str, tabindex);
	if (tabindex != 0)
		tabindex--;

	thePainterInterface.GetHitPointData(lpoint, lnorm,
										wpoint2, wnorm2,
										radius, str, tabindex);
	float dist = Length(wpoint2-wpoint1);
	return dist;
	}
Point3 *PainterInterfaceActionsIMP::fnGetHitVec(int tabindex)
	{
	if (tabindex <= 0)
		tabindex =  thePainterInterface.GetHitCount()-1;
	else tabindex--;


	Point3 lpoint,lnorm,wpoint1,wnorm1;
	Point3 wpoint2,wnorm2;
	float radius, str;
	thePainterInterface.GetHitPointData(lpoint, lnorm,
										wpoint1, wnorm1,
										radius, str, tabindex);
	if (tabindex != 0)
		tabindex--;

	thePainterInterface.GetHitPointData(lpoint, lnorm,
										wpoint2, wnorm2,
										radius, str, tabindex);
	dirVec = Normalize(wpoint1-wpoint2);
	return &dirVec;
	}

BOOL PainterInterfaceActionsIMP::fnGetRandomHitOnPoint(int tabIndex)
	{

	tabIndex--;

	BOOL hit = thePainterInterface.RandomHit(customHitPointWorld, customHitNormalWorld,
						  customHitPointLocal, customHitNormalLocal,
						  customBary,  customFaceIndex,
						  customStrFalloff, customINode,
						  customMirror,
						  customMirrorHitPointWorld, customMirrorHitNormalWorld,
						  customMirrorHitPointLocal, customMirrorHitNormalLocal,
						  tabIndex);

	return hit;
	}

BOOL PainterInterfaceActionsIMP::fnGetRandomHitAlongStroke(int tabIndex)
	{
	tabIndex--;
	BOOL hit = thePainterInterface.RandomHitAlongStroke(
						  customHitPointWorld, customHitNormalWorld,
						  customHitPointLocal, customHitNormalLocal,
						  customBary,  customFaceIndex,
						  customStrFalloff, customINode,
						  customMirror,
						  customMirrorHitPointWorld, customMirrorHitNormalWorld,
						  customMirrorHitPointLocal, customMirrorHitNormalLocal,tabIndex);

	return hit;
	}
BOOL PainterInterfaceActionsIMP::fnGetTestHit(Point2 p)
	{
	IPoint2 pos((int)p.x,(int)p.y);
	BOOL hit = thePainterInterface.TestHit(
						  pos,
						  customHitPointWorld, customHitNormalWorld,
						  customHitPointLocal, customHitNormalLocal,
						  customBary,  customFaceIndex,
						  customINode,
						  customMirror,
						  customMirrorHitPointWorld, customMirrorHitNormalWorld,
						  customMirrorHitPointLocal, customMirrorHitNormalLocal
						  );

	return hit;

	}

void PainterInterfaceActionsIMP::fnClearStroke() 
	{
	thePainterInterface.ClearStroke();
	}
void PainterInterfaceActionsIMP::fnAddToStroke(Point2 mousePos, BOOL rebuildPointGatherData, BOOL updateViewPort)
	{
	IPoint2 ipos;
	ipos.x = (int) mousePos.x;
	ipos.y = (int) mousePos.y;
	thePainterInterface.AddToStroke(ipos,rebuildPointGatherData,updateViewPort);
	}



void PainterInterfaceActionsIMP::fnGetCustomHitPointData(
								  Point3 &hitPointLocal, Point3 &hitNormalLocal,
								  Point3 &hitPointWorld, Point3 &hitNormalWorld,
								   float &str)
	{
	hitPointLocal = customHitPointLocal;
	hitNormalLocal = customHitNormalLocal;
	hitPointWorld = customHitPointWorld;
	hitNormalWorld = customHitNormalWorld;
	str = customStrFalloff;
	}

void PainterInterfaceActionsIMP::fnGetCustomHitFaceData(Point3 &bary, int &index, INode *node)
	{
	bary = customBary;
	index = customFaceIndex+1;
	node = customINode;

	}


void PainterInterfaceActionsIMP::fnGetCustomMirrorHitPointData(Point3 &hitPointLocal, Point3 &hitNormalLocal,
						Point3 &hitPointWorld, Point3 &hitNormalWorld)
	{
	hitPointWorld = customMirrorHitPointWorld;
	hitNormalWorld= customMirrorHitNormalWorld;
	hitPointLocal = customMirrorHitPointLocal;
	hitNormalLocal= customMirrorHitNormalLocal;
	}


BOOL PainterInterfaceActionsIMP::fnPointInside(Point2 checkPoint, Tab<Point2*> *pointList)
	{ 

	PolyLine poly;
	poly.Init();
	poly.SetNumPts(pointList->Count());

	for (int i=0; i < pointList->Count(); i++)
		{
		Point3 p;
		p.x = (*pointList)[i]->x;
		p.y = (*pointList)[i]->y;
		p.z = 0.0f;
		PolyPt pt(p);
		poly[i] = pt;
		}
	poly.Close();

	BOOL inside = poly.SurroundsPoint(checkPoint);

	return inside;

	}



BOOL PainterInterfaceActionsIMP::GetUpdateOnMouseUp()
	{
	return thePainterInterface.GetUpdateOnMouseUp();
	}

void PainterInterfaceActionsIMP::SetUpdateOnMouseUp(BOOL update)
	{ 
	thePainterInterface.SetUpdateOnMouseUp(update);
	}


int PainterInterfaceActionsIMP::GetLagRate()
	{
	return thePainterInterface.GetLagRate();
	}
void PainterInterfaceActionsIMP::SetLagRate(int rate)
	{ 
	thePainterInterface.SetLagRate(rate);
	}

int PainterInterfaceActionsIMP::GetTreeDepth()
	{
	return thePainterInterface.GetTreeDepth();
	}
 
void PainterInterfaceActionsIMP::SetTreeDepth(int depth)
	{
	thePainterInterface.SetTreeDepth(depth);
	}


float PainterInterfaceActionsIMP::GetMinStr()
	{
	return thePainterInterface.GetMinStr();
	}
void PainterInterfaceActionsIMP::SetMinStr(float str)
	{
	thePainterInterface.SetMinStr(str);
	}
 
float PainterInterfaceActionsIMP::GetMaxStr()
	{
	return thePainterInterface.GetMaxStr();
	}
void PainterInterfaceActionsIMP::SetMaxStr(float str)
	{
	thePainterInterface.SetMaxStr(str);
	} 

float PainterInterfaceActionsIMP::GetMinSize()
	{
	return thePainterInterface.GetMinSize();
	} 
void PainterInterfaceActionsIMP::SetMinSize(float size)
	{
	thePainterInterface.SetMinSize(size);
	}
 
float PainterInterfaceActionsIMP::GetMaxSize()
	{
	return thePainterInterface.GetMaxSize();
	} 
void PainterInterfaceActionsIMP::SetMaxSize(float size) 
	{
	thePainterInterface.SetMaxSize(size);
	}

 
BOOL PainterInterfaceActionsIMP::GetAdditiveMode()
	{
	return thePainterInterface.GetAdditiveMode();
	} 
void PainterInterfaceActionsIMP::SetAdditiveMode(BOOL enable) 
	{
	thePainterInterface.SetAdditiveMode(enable);
	}


BOOL  PainterInterfaceActionsIMP::GetDrawRing()
	{
	return thePainterInterface.GetDrawRing();
	} 
void  PainterInterfaceActionsIMP::SetDrawRing(BOOL draw)
	{
	thePainterInterface.SetDrawRing(draw);
	}

BOOL  PainterInterfaceActionsIMP::GetDrawNormal()
	{
	return thePainterInterface.GetDrawNormal();
	} 
void  PainterInterfaceActionsIMP::SetDrawNormal(BOOL draw)
	{
	thePainterInterface.SetDrawNormal(draw);
	}

BOOL  PainterInterfaceActionsIMP::GetDrawTrace()
	{
	return thePainterInterface.GetDrawTrace();
	} 
void  PainterInterfaceActionsIMP::SetDrawTrace(BOOL draw)
	{
	thePainterInterface.SetDrawTrace(draw);
	}


BOOL  PainterInterfaceActionsIMP::GetEnablePressure()
	{
	return thePainterInterface.GetPressureEnable();
	}
void  PainterInterfaceActionsIMP::SetEnablePressure(BOOL enable)
	{
	thePainterInterface.SetPressureEnable(enable);
	}


int PainterInterfaceActionsIMP::GetPressureAffects()
	{ 
	return thePainterInterface.GetPressureAffects()+1;
	}
void PainterInterfaceActionsIMP::SetPressureAffects(int affects)
	{ 
	affects--;
	thePainterInterface.SetPressureAffects(affects);
	}


BOOL PainterInterfaceActionsIMP::GetPreDefinedStrEnable()
	{
	return thePainterInterface.GetPredefinedStrEnable();
	}
void PainterInterfaceActionsIMP::SetPreDefinedStrEnable(BOOL enable)
	{ 
	thePainterInterface.SetPredefinedStrEnable(enable);
	}

BOOL PainterInterfaceActionsIMP::GetPreDefinedSizeEnable()
	{ 
	return thePainterInterface.GetPredefinedSizeEnable();

	}
void PainterInterfaceActionsIMP::SetPreDefinedSizeEnable(BOOL enable)
	{
	thePainterInterface.SetPredefinedSizeEnable(enable);
	
	}

int PainterInterfaceActionsIMP::fnGetHitCount()
	{ 
	return thePainterInterface.GetHitCount();
	}

BOOL PainterInterfaceActionsIMP::GetMirrorEnable() 
	{
	return thePainterInterface.GetMirrorEnable();
	}
void PainterInterfaceActionsIMP::SetMirrorEnable(BOOL enable)
	{ 
	thePainterInterface.SetMirrorEnable(enable);
	}

int PainterInterfaceActionsIMP::GetMirrorAxis()
	{ 
	return thePainterInterface.GetMirrorAxis()+1;
	}
void PainterInterfaceActionsIMP::SetMirrorAxis(int axis)
	{ 
	axis--;
	thePainterInterface.SetMirrorAxis(axis);
	}

float PainterInterfaceActionsIMP::GetMirrorOffset()
	{ 
	return thePainterInterface.GetMirrorOffset();
	}
void PainterInterfaceActionsIMP::SetMirrorOffset(float offset)
	{ 
	thePainterInterface.SetMirrorOffset(offset);

	}

float PainterInterfaceActionsIMP::GetMirrorGizmoSize()
	{ 
	return thePainterInterface.GetMirrorGizmoSize();
	}
void PainterInterfaceActionsIMP::SetMirrorGizmoSize(float size)
	{ 
	thePainterInterface.SetMirrorGizmoSize(size);
	}


BOOL  PainterInterfaceActionsIMP::GetBuildNormalData()
	{
	return thePainterInterface.GetBuildNormalData();
	}
void  PainterInterfaceActionsIMP::SetBuildNormalData(BOOL enable)
	{
	thePainterInterface.SetBuildNormalData(enable);
	}

BOOL  PainterInterfaceActionsIMP::GetEnablePointGather()
	{
	return thePainterInterface.GetEnablePointGather();
	}

void  PainterInterfaceActionsIMP::SetEnablePointGather(BOOL enable)
	{
	thePainterInterface.SetEnablePointGather(enable);

	}


void PainterInterfaceActionsIMP::fnLoadCustomPointGather(INode *node, Tab<Point3*> *pointList)
	{
	Tab<Point3> worldList;
	int count = pointList->Count();
	worldList.SetCount(count);
	Matrix3 tm = node->GetObjectTM(GetCOREInterface()->GetTime());
	
	for (int i = 0; i < count; i++)
		worldList[i] = (*(*pointList)[i]) * tm;
	thePainterInterface.LoadCustomPointGather(count, worldList.Addr(0),node);
	}

BitArray* PainterInterfaceActionsIMP::fnGetPointGatherHitVerts(INode *node)
	{
	return thePainterInterface.GetPointGatherHitVerts(node);
	}



float  PainterInterfaceActionsIMP::fnGetPointGatherHitWeight(INode *node, int index)
	{
	index--;
	int count;
	float *weight = thePainterInterface.RetrievePointGatherWeights(node,count);
	if ((index >=0) && (index < count))
		return weight[index];
	else return 0.0f;
	}

float  PainterInterfaceActionsIMP::fnGetPointGatherHitStr(INode *node, int index)
	{
	index--;
	int count;
	float *str = thePainterInterface.RetrievePointGatherStr(node,count);
	if ((index >=0) && (index < count))
		return str[index];
	else return 0.0f;
	
	}

Point3 * PainterInterfaceActionsIMP::fnGetPointGatherHitPoint(INode *node, int index)
	{
	index--;
	int count;
	Point3 *points = thePainterInterface.RetrievePointGatherPoints(node,count);
	if ((index >=0) && (index < count))
		return &points[index];
	else return NULL;

	}

Point3 * PainterInterfaceActionsIMP::fnGetPointGatherHitNormal(INode *node, int index)
	{
	index--;
	int count;
	Point3 *norms = thePainterInterface.RetrievePointGatherNormals(node,count);
	if ((index >=0) && (index < count))
		return &norms[index];
	else return NULL;

	}

Point3 *PainterInterfaceActionsIMP::fnGetMirrorCenter()
	{ 
	return &thePainterInterface.mirrorCenter;
	}



float  PainterInterfaceActionsIMP::fnGetNormalScale()
	{
	return thePainterInterface.GetNormalScale();
	}

void  PainterInterfaceActionsIMP::fnSetNormalScale(float scale)
	{
	thePainterInterface.SetNormalScale(scale);
	}

float  PainterInterfaceActionsIMP::fnGetMarker()
	{
	return thePainterInterface.GetMarker();
	}

void  PainterInterfaceActionsIMP::fnSetMarker(float pos)
	{
	thePainterInterface.SetMarker(pos);
	}

BOOL  PainterInterfaceActionsIMP::fnGetMarkerEnable()
	{
	return thePainterInterface.GetMarkerEnable();
	}

void  PainterInterfaceActionsIMP::fnSetMarkerEnable(BOOL enable)
	{
	thePainterInterface.SetMarkerEnable(enable);

	}

int PainterInterfaceActionsIMP::fnGetOffMeshHitType()
{
	return thePainterInterface.GetOffMeshHitType();
}
void PainterInterfaceActionsIMP::fnSetOffMeshHitType(int type)
{
	if (type < 0) type = 0;
	if (type > 2) type = 2;

	thePainterInterface.SetOffMeshHitType(type);
}

float PainterInterfaceActionsIMP::fnGetOffMeshHitZDepth()
{
	return thePainterInterface.GetOffMeshHitZDepth();
}
void PainterInterfaceActionsIMP::fnSetOffMeshHitZDepth(float depth)
{
		thePainterInterface.SetOffMeshHitZDepth(depth);

}

Point3* PainterInterfaceActionsIMP::fnGetOffMeshHitPos()
{
	offMeshHitPos = thePainterInterface.GetOffMeshHitPos();
	return &offMeshHitPos;

}
void PainterInterfaceActionsIMP::fnSetOffMeshHitPos(Point3 pos)
{
	thePainterInterface.SetOffMeshHitPos(pos);

}

void PainterInterfaceActionsIMP::fnScriptFunctions(Value* startStroke,Value* paintStroke,Value* endStroke,Value* cancelStroke,Value* systemEnd)

{
  	thePainterInterface.ScriptFunctions(startStroke,paintStroke,endStroke,cancelStroke,systemEnd);

}


void PainterInterfaceActionsIMP::fnUndoStart()
{
	if (!theHold.Holding())
		theHold.Begin();
}

void PainterInterfaceActionsIMP::fnUndoAccept()
{
	theHold.Accept(GetString(IDS_PW_PAINTERCOLORS));
}

void PainterInterfaceActionsIMP::fnUndoCancel()
{
	theHold.Cancel();
}
