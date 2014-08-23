#ifndef __BONESDEF_UNDO__H
#define __BONESDEF_UNDO__H

/*************************************************************

This holds all our undo record classes


**************************************************************/

class LocalGizmoData;


//this is the undo record for when a single bone is pasted
class PasteRestore : public RestoreObj 
	{
	public:
		BonesDefMod *mod;
		int whichBone;

		Tab<int> uRefTable,rRefTable;
		BoneDataClass ubone,rbone;

		Tab<RefTargetHandle> urefHandleList;
		Tab<RefTargetHandle> rrefHandleList;


		PasteRestore(BonesDefMod *c) ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold();
		TSTR Description();
	};

//this is the undo record for when a multiple bones are pasted
//it holds the entire list and is used to for other things besides the
//paste all and multiple
class PasteToAllRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		Tab<BoneDataClass *> ubuffer,rbuffer;
		Tab<int> uRefTable,rRefTable;

		Tab<RefTargetHandle> urefHandleList;
		Tab<RefTargetHandle> rrefHandleList;


		PasteToAllRestore(BonesDefMod *c); 
		~PasteToAllRestore() ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};


//this is the hold record for any type of sub object selection
//it holds all the sub object selections
class SelectionRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneModData *bmd;
		int rModeBoneIndex;
		int rModeBoneEnvelopeIndex;
		int rModeBoneEndPoint;
		int rModeBoneEnvelopeSubType;
		BitArray rVertSel;

		int uModeBoneIndex;
		int uModeBoneEnvelopeIndex;
		int uModeBoneEndPoint;
		int uModeBoneEnvelopeSubType;
		BitArray uVertSel;

		SelectionRestore(BonesDefMod *c, BoneModData *md) ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};


//this is the hold record for when a bone is deleted
class DeleteBoneRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneDataClass undo,redo;
		int undoID, redoID;
		Tab<int> undoRefTable,redoRefTable;

		Tab<RefTargetHandle> urefHandleList;
		Tab<RefTargetHandle> rrefHandleList;


		DeleteBoneRestore(BonesDefMod *c, int whichBone);
		void Restore(int isUndo) ;
		void Redo();
		void EndHold();
		TSTR Description();
	};

//this is the hold record for when a bone is added
class AddBoneRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneDataClassList undo;
		BoneDataClassList redo;
		int undoID, redoID;
		Tab<int> undoRefTable,redoRefTable;


		Tab<RefTargetHandle> urefHandleList;
		Tab<RefTargetHandle> rrefHandleList;


		AddBoneRestore(BonesDefMod *c);
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};

//this is the hold record for a vertex weight change
class WeightRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneModData *bmd;
		Tab<VertexListClass*> undoVertexData;
		Tab<VertexListClass*> redoVertexData;
		BOOL updateView;

		WeightRestore(BonesDefMod *bmod, BoneModData *md, BOOL updateView=TRUE);
		~WeightRestore();
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};

//this is the hold record for adding exclusion lists
class ExclusionListRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneModData *bmd;
		Tab<int> uAffectedVerts;
		Tab<int> rAffectedVerts;
		int whichBone;

		ExclusionListRestore(BonesDefMod *bmod, BoneModData *md,int boneID);
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};

//this is the hold record for a gizmo paste operation
class GizmoPasteRestore : public RestoreObj {
	public:
		GizmoClass *giz;
		int whichGizmo;
		
		IGizmoBuffer *uBuffer, *rBuffer;

		GizmoPasteRestore(GizmoClass *c);
		~GizmoPasteRestore();
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};

//this is a helper hold record that updates the UI
class UpdateUIRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		UpdateUIRestore(BonesDefMod *bmod) ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};

//this is a helper hold record that updates the UI
class UpdatePRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		UpdatePRestore(BonesDefMod *bmod) ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold();
		TSTR Description();
	};

//this is a helper hold record that updates the Squash UI
class UpdateSquashUIRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		float uval,rval;
		float index;
		UpdateSquashUIRestore(BonesDefMod *bmod, float val,  int id) ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};




//this is a hold record for an add gizmo operations
class AddGizmoRestore : public RestoreObj {
	public:
		BonesDefMod *mod;

		
		GizmoClass *rGizmo;
		int whichGizmo;

		AddGizmoRestore(BonesDefMod *bmod,GizmoClass *gizmo);
		void Restore(int isUndo);
		void Redo();
		void EndHold();
		TSTR Description();
	};

//this is a hold record for an add gizmo operations
//this holds all the local data for the gizmos
class AddGizmoLocalDataRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneModData *bmd;
		
		LocalGizmoData *rData;
		
		int whichGizmo;

		AddGizmoLocalDataRestore(BonesDefMod *bmod, BoneModData *md); 
		~AddGizmoLocalDataRestore() ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold() ;
		TSTR Description();
	};


//this is a hold record for a remove  gizmo operations
class RemoveGizmoRestore : public RestoreObj, public ReferenceTarget {
	public:
		BonesDefMod *mod;

		
		GizmoClass *uGizmo;
		int whichGizmo;

		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
			   PartID& partID, RefMessage message);
		void DeleteThis();

		RemoveGizmoRestore(BonesDefMod *bmod,GizmoClass *g, int id);
		~RemoveGizmoRestore();
		void Restore(int isUndo) ;
		void Redo();
		void EndHold();
		TSTR Description();
	};

//this is a hold record for an remove gizmo operations
//this holds all the local data for the gizmos
class RemoveGizmoLocalDataRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneModData *bmd;
		
		LocalGizmoData *uData;
		
		int whichGizmo;

		RemoveGizmoLocalDataRestore(BonesDefMod *bmod, BoneModData *md,int gid); 
		~RemoveGizmoLocalDataRestore();
		void Restore(int isUndo) ;
		void Redo();
		void EndHold();
		TSTR Description();
	};

//this is hold record for when a gizmo is selected
class SelectGizmoRestore : public RestoreObj {

	public:
		BonesDefMod *mod;
		
		int uWhichGizmo,rWhichGizmo;

		SelectGizmoRestore(BonesDefMod *bmod);
		void Restore(int isUndo);
		void Redo();
		void EndHold();
		TSTR Description();
	};


#endif

