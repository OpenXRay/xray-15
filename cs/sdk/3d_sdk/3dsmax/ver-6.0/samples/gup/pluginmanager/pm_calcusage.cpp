/*===========================================================================*\
 | 
 |  FILE:	Plugin Management Tool
 |			3D Studio MAX R4.0
 | 
 |  AUTH:   Harry Denholm
 |			Ravi Karra
 |			All Rights Reserved 
 |
 |  HIST:	Started 19-10-99
 | 
\*===========================================================================*/

#include "plugman.h"


#define SCENE_REF_MTL_EDIT 		0
#define SCENE_REF_MTL_LIB  		1
#define SCENE_REF_SOUNDOBJ 		2
#define SCENE_REF_ROOTNODE 		3 
#define SCENE_REF_REND	   		4
#define SCENE_REF_SELSETS		5
#define SCENE_REF_TVNODE		6
#define SCENE_REF_GRIDREF		7
#define SCENE_REF_RENDEFFECTS	8
#define SCENE_REF_GLOBSHADTYPE	9
#define SCENE_REF_LAYERREF		10
#define NUM_SCENE_REFS			11



void NodeEnum(INode* node, RefEnumProc *proc)
{
	EnumRefHierarchy( node , *proc);
	for (int c = 0; c < node->NumberOfChildren(); c++) {
		NodeEnum(node->GetChildNode(c),proc);
	}
}


void EnumEverything(RefEnumProc &rproc) {
	Interface *ip = GetCOREInterface();
	ReferenceTarget *theScene = ip->GetScenePointer();

    INode *Rootnode = ip->GetRootNode();
	NodeEnum(Rootnode,&rproc);

	EnumRefHierarchy(theScene->GetReference(SCENE_REF_MTL_LIB),rproc);
	EnumRefHierarchy(theScene->GetReference(SCENE_REF_MTL_EDIT),rproc);
	EnumRefHierarchy(theScene->GetReference(SCENE_REF_REND),rproc);
	EnumRefHierarchy(theScene->GetReference(SCENE_REF_SOUNDOBJ),rproc);
	EnumRefHierarchy(theScene->GetReference(SCENE_REF_RENDEFFECTS),rproc);
	}

class ClearAllFlag : public RefEnumProc {
	public:
		void proc(ReferenceMaker *m) { 
			m->ClearAFlag(A_WORK1); 
			}
	};

class CountClassUses: public RefEnumProc {
	public:
		void proc(ReferenceMaker *m);
	};

void CountClassUses::proc(ReferenceMaker *m) { 
	if (m->TestAFlag(A_WORK1)) 
		return;
	ClassEntry *ce = GetCOREInterface()->GetDllDir().ClassDir().FindClassEntry(m->SuperClassID(),m->ClassID());
	if (ce) 
		ce->IncUseCount();
	m->SetAFlag(A_WORK1); 
	}

static void ZeroClassUseCounts(Interface *ip) {
	ClassDirectory &cd = ip->GetDllDir().ClassDir();
	for (int i=0; i<cd.Count(); i++) {
		SubClassList& scl = cd[i];
		for (int j=0; j<scl.Count(ACC_ALL); j++) 
			scl[j].SetUseCount(0);
		}
	}


void ComputeClassUse(Interface *ip) {
	ClearAllFlag cfenum;
	EnumEverything(cfenum);

	ZeroClassUseCounts(ip);

	CountClassUses ccu;
	EnumEverything(ccu);

	EnumEverything(cfenum);
	}