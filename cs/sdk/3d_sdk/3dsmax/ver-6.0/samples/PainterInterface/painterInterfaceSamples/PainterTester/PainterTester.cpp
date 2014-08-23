/**********************************************************************
 *<
	FILE: PainterTester.cpp

	DESCRIPTION:	This is a test bed for the painter interface

	CREATED BY:		Peter Watje

	HISTORY: 


 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "PainterTester.h"
#include "modstack.h"
#include "meshadj.h"

static PainterTester thePainterTester;

class PainterTesterClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &thePainterTester; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return PAINTERTESTER_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("PainterTester"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static PainterTesterClassDesc PainterTesterDesc;
ClassDesc2* GetPainterTesterDesc() { return &PainterTesterDesc; }


static BOOL CALLBACK PainterTesterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			thePainterTester.Init(hWnd);
			break;

		case WM_DESTROY:
			thePainterTester.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_RADIO1:
				case IDC_RADIO2:
				case IDC_RADIO3:
				case IDC_RADIO4:
						{
						thePainterTester.sampleInsideBrush = IsDlgButtonChecked(hWnd,IDC_RADIO2);
						thePainterTester.regularSample = IsDlgButtonChecked(hWnd,IDC_RADIO1);
						thePainterTester.testPointGather = IsDlgButtonChecked(hWnd,IDC_RADIO3);
						thePainterTester.testCustomPoints = IsDlgButtonChecked(hWnd,IDC_RADIO4);

						if (thePainterTester.testCustomPoints)
							{

							thePainterTester.pPainter->SetEnablePointGather(TRUE);
							thePainterTester.LoadCustomNodes();
							

							}

						if (thePainterTester.testPointGather)
							{
							if (thePainterTester.pPainter) 
								{
								thePainterTester.pPainter->SetBuildNormalData(TRUE);
								thePainterTester.pPainter->SetEnablePointGather(TRUE);
								}
							else
								{
								thePainterTester.pPainter->SetBuildNormalData(FALSE);
								thePainterTester.pPainter->SetEnablePointGather(FALSE);
								}

							
							}
						break;
					  }
				case IDC_PAINTOPTIONS:
					thePainterTester.PaintOptions(); 
					break;
				case IDC_PAINT:
					thePainterTester.PaintMode(); 
					break;
				case IDC_NODES:
					thePainterTester.SetNodes(); 
					break;
				}

			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			thePainterTester.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}



//--- PainterTester -------------------------------------------------------
PainterTester::PainterTester()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
	iPaintButton = NULL;
}

PainterTester::~PainterTester()
{

}

void PainterTester::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(	hInstance,	MAKEINTRESOURCE(IDD_PANEL),	PainterTesterDlgProc,GetString(IDS_PARAMS),	0);
}
	
void PainterTester::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

/*
void PainterTester::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(	hInstance,	MAKEINTRESOURCE(IDD_PANEL),	PainterTesterDlgProc,GetString(IDS_PARAMS),	0);

}


void PainterTester::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;

}
*/
void PainterTester::Init(HWND hWnd)
{
	iPaintButton = GetICustButton(GetDlgItem(hWnd,IDC_PAINT));
	iPaintButton->SetType(CBT_CHECK);
	iPaintButton->SetHighlightColor(GREEN_WASH);

	ReferenceTarget *painterRef = (ReferenceTarget *) GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID,PAINTERINTERFACE_CLASS_ID);
	
//set it to the correct verion
	if (painterRef)
		{
		pPainter = (IPainterInterface_V5 *) painterRef->GetInterface(PAINTERINTERFACE_V5);
		}
}

void PainterTester::Destroy(HWND hWnd)
{
	ReleaseICustButton(iPaintButton);
	iPaintButton = NULL;
	pPainter->EndPaintSession();
}


void PainterTester::SetNodes()
{
	
	int nodeCount = GetCOREInterface()->GetSelNodeCount();

	nodes.ZeroCount();

	for (int i = 0; i < nodeCount; i++)
		{
		INode *node = GetCOREInterface()->GetSelNode(i);
		nodes.Append(1,&node);
		}
//get a pointer the painter class, note there is only one static instance of the painter 
	
	if (pPainter)
		{
		pPainter->InitializeNodes(0, nodes);
		}

}

void PainterTester::PaintOptions()
	{
//get a pointer the painter class, note there is only one static instance of the painter 
	
	if (pPainter)
		{
		pPainter->BringUpOptions();
		}

	}

void PainterTester::CreateCylinder(BOOL isMirror, Point3 pos, Point3 normal,float frad, float fheight)
	{


	Object *obj = (Object*)GetCOREInterface()->CreateInstance(GEOMOBJECT_CLASS_ID,Class_ID(CYLINDER_CLASS_ID,0));
	assert(obj);

	// Get a hold of the parameter block
	IParamArray *iCylParams = obj->GetParamBlock();
	assert(iCylParams);

	// Set the value of radius, height and segs.
	int rad = obj->GetParamBlockIndex(CYLINDER_RADIUS);
	assert(rad>=0);
	radIndex = rad;
	iCylParams->SetValue(rad,TimeValue(0),frad);
	int height = obj->GetParamBlockIndex(CYLINDER_HEIGHT);
	assert(height>=0);
	heightIndex = height;
	iCylParams->SetValue(height,TimeValue(0),fheight);
	int segs = obj->GetParamBlockIndex(CYLINDER_SEGMENTS);
	assert(segs>=0);
	iCylParams->SetValue(segs,TimeValue(0),1);

	// Create a derived object that references the cylinder
	IDerivedObject *dobj = CreateDerivedObject(obj);

	// Create a node in the scene that references the derived object
	INode *cylinderNode = GetCOREInterface()->CreateObjectNode(dobj);
	// Name the node and make the name unique.
	TSTR name(_T("StrokeNode"));
//	ip->MakeNameUnique(name);
	cylinderNode->SetName(name);
	
	Matrix3 tm;
	MatrixFromNormal(normal, tm);
	tm.SetTrans(pos);
	cylinderNode->SetNodeTM(GetCOREInterface()->GetTime(), tm);
	if (isMirror)
		mirrorNodeList.Append(1,&iCylParams,100);
	else nodeList.Append(1,&iCylParams,100);

	}
void PainterTester::ResizeCylinders(int ct, float *radius, float *str)
	{
	if ((radius) || (str))
		{
		for (int i =0; i < ct; i++)
			{
			if (i < nodeList.Count())
				{
				if (radius)
					nodeList[i]->SetValue(radIndex,TimeValue(0),radius[i]);
				if (str)
					nodeList[i]->SetValue(heightIndex,TimeValue(0),str[i]);

				}
			if (i < mirrorNodeList.Count())
				{
				if (radius)
					mirrorNodeList[i]->SetValue(radIndex,TimeValue(0),radius[i]);
				if (str)
					mirrorNodeList[i]->SetValue(heightIndex,TimeValue(0),str[i]);

				}
			}
		}
	}

PainterTester::LoadCustomNodes()
{
customNodes.ZeroCount();
int nodeCount = GetCOREInterface()->GetSelNodeCount();



for (int i = 0; i < nodeCount; i++)
	{
	INode *node = GetCOREInterface()->GetSelNode(i);
	customNodes.Append(1,&node);
	}
//get a pointer the painter class, note there is only one static instance of the painter 
TimeValue t = GetCOREInterface()->GetTime();
if (pPainter)
	{
	Tab<Point3> pointList;
	pointList.SetCount(nodeCount);
	for (i = 0; i < nodeCount; i++)
		{
		Matrix3 tm;
		
		tm = customNodes[i]->GetNodeTM(t);
		Point3 p = tm.GetRow(3);
		pointList[i] = p;

		}
	pPainter->LoadCustomPointGather(nodeCount, pointList.Addr(0), nodes[0]);
	}

}

// This is called when the user starts begins to start a pen stroke
BOOL  PainterTester::StartStroke()
	{
	DebugPrint("Start Stroke \n");
	mirrorNodeList.ZeroCount();
	nodeList.ZeroCount();


	theHold.Begin();
	return TRUE;
	}

// This is called as the user sytrokes across the mesh or screen
BOOL  PainterTester::PaintStroke(BOOL hit, IPoint2 mousePos, 
						  Point3 worldPoint, Point3 worldNormal,
						  Point3 localPoint, Point3 localNormal,
						  Point3 bary,  int index,
						  BOOL shift, BOOL ctrl, BOOL alt, 
						  float radius, float str,
						  float pressure, INode *node,
						  BOOL mirrorOn,
						  Point3 worldMirrorPoint, Point3 worldMirrorNormal,
						  Point3 localMirrorPoint, Point3 localMirrorNormal) 
	{
//	DebugPrint("mouse pos %d %d\n", mousePos.x, mousePos.y);

//just creates cyliners along the stroke

	// Create a new object through the CreateInstance() API

	if (regularSample)
		{
		CreateCylinder(FALSE, worldPoint, worldNormal,radius,str);
		if (mirrorOn)
			CreateCylinder(TRUE, worldMirrorPoint, worldMirrorNormal,radius,str);
		}
	else if (sampleInsideBrush)
		{
		for (int i = 0; i < 100; i++)
			{
			Point3 sampWorldPoint;
			Point3 sampworldNormal;
			Point3 samplocalPoint;
			Point3 samplocalNormal;
			Point3 sampbary;
			int sampindex;
			float sampstrFalloff;
			INode *sampNode=NULL;
			BOOL sampmirrorOn;
			Point3 sampworldMirrorPoint;
			Point3 sampworldMirrorNormal;
			Point3 samplocalMirrorPoint;
			Point3 samplocalMirrorNormal;

			BOOL hit =  pPainter->RandomHit(sampWorldPoint,sampworldNormal,samplocalPoint,samplocalNormal,
											sampbary,sampindex,
											sampstrFalloff,
											sampNode,
											sampmirrorOn,
											sampworldMirrorPoint,sampworldMirrorNormal,
											samplocalMirrorPoint,samplocalMirrorNormal,
											-1);
			if (hit)
				{
				CreateCylinder(FALSE, sampWorldPoint, sampworldNormal,1.0f,sampstrFalloff);
				if (mirrorOn)
					CreateCylinder(TRUE, sampworldMirrorPoint, sampworldMirrorNormal,1.0f,sampstrFalloff);
				}
			}
		}
	else if (testPointGather)
		{
		}

	else if (testCustomPoints)
		{
		TimeValue t = GetCOREInterface()->GetTime();
		if (pPainter)
			{
			int ct;
			float *weightList;
			float *strList;
			Point3 *points;
			weightList = pPainter->RetrievePointGatherWeights(nodes[0], ct);
			strList = pPainter->RetrievePointGatherStr(nodes[0], ct);
			points = pPainter->RetrievePointGatherPoints(nodes[0], ct);

			
			int nodeCount= customNodes.Count();
			for (int i = 0; i < nodeCount; i++)
				{
				if (weightList[i] > 0.0f)
					{
					Matrix3 tm;
					tm = customNodes[i]->GetNodeTM(t);
					Point3 pos = tm.GetRow(3);
					pos = points[i]+ (worldNormal*strList[i]);
					tm.SetRow(3,pos);
					customNodes[i]->SetNodeTM(t,tm);
					}
				
				


				}
			}

		}

	if (pPainter)
		{
		int ct;
		float *radius=NULL, *str=NULL;
		str = pPainter->GetPredefineStrStrokeData(ct);
		radius = pPainter->GetPredefineSizeStrokeData(ct);
		ResizeCylinders(ct, radius, str);
		}
	return TRUE;
	}

// This is called as the user ends a strokes 
BOOL  PainterTester::EndStroke()
	{
	DebugPrint("End Stroke \n");

	if (testPointGather)
		{
		if (pPainter)
			{
			for (int i = 0; i < nodes.Count(); i++)
				{
				int ct;
				float *weightList;
				float *strList;
				Point3 *points;
				Point3 *norms;
				weightList = pPainter->RetrievePointGatherWeights(nodes[i], ct);
				strList = pPainter->RetrievePointGatherStr(nodes[i], ct);
				points = pPainter->RetrievePointGatherPoints(nodes[i], ct);
				norms= pPainter->RetrievePointGatherNormals(nodes[i], ct);
				for (int j = 0; j <ct; j++)
					{
					if (weightList[j] > 0.0f)
						{	
//DebugPrint("weigth % f str %f\n ",weightList[j],strList);
						CreateCylinder(FALSE, points[j],norms[j],1.0f,strList[j]);
						}
					}
				}
			}
		}
	theHold.Accept(GetString(IDS_STROKE));
	return TRUE;
	}


// This is called as the user ends a strokes when the users has it set to update on mouse up only
// the canvas gets a list of all points, normals etc instead of one at a time

BOOL  PainterTester::EndStroke(int ct, BOOL *hit,IPoint2 *mousePos, 
						  Point3 *worldPoint, Point3 *worldNormal,
						  Point3 *localPoint, Point3 *localNormal,
						  Point3 *bary,  int *index,
						  BOOL *shift, BOOL *ctrl, BOOL *alt, 
						  float *radius, float *str,
						  float *pressure, INode **node,
						  BOOL mirrorOn,
						  Point3 *worldMirrorPoint, Point3 *worldMirrorNormal,
						  Point3 *localMirrorPoint, Point3 *localMirrorNormal	  )
	{
	if (regularSample)
		{
		for (int i =0; i < ct; i++)  // real apps should do some interpolation instead of just picking hit points
			{
			CreateCylinder(FALSE, worldPoint[i], worldNormal[i], radius[i], str[i]);
			if (mirrorOn)
				CreateCylinder(TRUE, worldMirrorPoint[i], worldMirrorNormal[i], radius[i], str[i]);
			}
		}
	else if (sampleInsideBrush)
		{
		for (int k =0; k < ct; k++)  // real apps should do some interpolation instead of just picking hit points
			{
			for (int i = 0; i < 100; i++)
				{
				Point3 sampWorldPoint;
				Point3 sampworldNormal;
				Point3 samplocalPoint;
				Point3 samplocalNormal;
				Point3 sampbary;
				int sampindex;
				float sampstrFalloff;
				INode *sampNode=NULL;
				BOOL sampmirrorOn;
				Point3 sampworldMirrorPoint;
				Point3 sampworldMirrorNormal;
				Point3 samplocalMirrorPoint;
				Point3 samplocalMirrorNormal;

				BOOL hit =  pPainter->RandomHit(sampWorldPoint,sampworldNormal,samplocalPoint,samplocalNormal,
											sampbary,sampindex,
											sampstrFalloff,
											sampNode,
											sampmirrorOn,
											sampworldMirrorPoint,sampworldMirrorNormal,
											samplocalMirrorPoint,samplocalMirrorNormal,
											k);
				if (hit)
					{
					CreateCylinder(FALSE, sampWorldPoint, sampworldNormal,1.0f,sampstrFalloff);
					if (mirrorOn)
						CreateCylinder(TRUE, sampworldMirrorPoint, sampworldMirrorNormal,1.0f,sampstrFalloff);
					}			
				}
			}

		}
	theHold.Accept(GetString(IDS_STROKE));
	return TRUE;
	}

// This is called as the user cancels a stroke by right clicking
BOOL  PainterTester::CancelStroke()	
	{
	DebugPrint("Cancel Stroke \n");
	theHold.Cancel();
	return TRUE;
	}

// This is called as the user cancels a stroke by right clicking
BOOL  PainterTester::SystemEndPaintSession()	
	{
	DebugPrint("End Session \n");
	iPaintButton->SetCheck(FALSE);
//	theHold.Cancel();
	return TRUE;
	}

void PainterTester::PaintMode()
	{
	if (pPainter)
		{
//check to see if we are already in paint mode

		if (pPainter->InPaintMode()) //if so close it 
			{
			pPainter->EndPaintSession();
			iPaintButton->SetCheck(FALSE);
			}
		else //else start up a new one
			{
			pPainter->InitializeCallback((ReferenceTarget *) this);
			pPainter->StartPaintSession();
			iPaintButton->SetCheck(TRUE);
			}
		}
	else
		{
		//error message
		}
	}


void* PainterTester::GetInterface(ULONG id)
{
	switch(id)
	{
		case PAINTERCANVASINTERFACE_V5 : return (IPainterCanvasInterface_V5 *) this;
			break;
		default: return ReferenceTarget::GetInterface(id);
			break;
	}
}
