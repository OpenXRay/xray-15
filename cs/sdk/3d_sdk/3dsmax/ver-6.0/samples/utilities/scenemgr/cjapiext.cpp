//***************************************************************************
//* SceneAPI - Implementation of Scene Extension API for 3D Studio MAX 2.0
//* 
//* By Christer Janson
//* Kinetix Development
//*
//* November  2, 1996	CCJ Initial coding
//* January   8, 1997	CCJ Added material editor slot access
//* March    15, 1997	CCJ Added scene materials access
//* November 10, 1997	CCJ Ported to MAX Release 2.0
//*
//* This class implements a couple of missing API calls.
//* 


#include "max.h"
#include "cjapiext.h"


class SceneAPIFindSceneProc : public DependentEnumProc {
	public:
		SceneAPIFindSceneProc(ReferenceMaker** anim) {
			scene = anim;
			*scene = NULL;
		}
		int proc(ReferenceMaker *ref) {
			switch (ref->SuperClassID()) {
				case REF_MAKER_CLASS_ID:
					if (ref->ClassID() == Class_ID(THE_SCENE_CLASS_ID, 0)) {
						*scene = ref;
					}
					break;
			}
			return 0;
		}
	private:
		ReferenceMaker** scene;
};


SceneAPI::SceneAPI(Interface* i)
{
	ip = i;

	FindScene();
}

void SceneAPI::FindScene()
{
	scene = NULL;
	SceneAPIFindSceneProc dep(&scene);
	ip->GetRootNode()->EnumDependents(&dep);
}

//////////
// public:
//

MtlBase* SceneAPI::GetMtlSlot(int i)
{
	if (!scene)
		return NULL;

	ReferenceTarget* mtlEdit;
	mtlEdit = scene->GetReference(0);

	if (i >= mtlEdit->NumSubs())
		return NULL;

	MtlBase* mtl = (MtlBase*)mtlEdit->SubAnim(i);

	return mtl;
}

MtlBaseLib* SceneAPI::GetSceneMtls()
{
	if (!scene)
		return NULL;

	return (MtlBaseLib*)scene->GetReference(1);
}
