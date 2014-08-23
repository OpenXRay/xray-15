
#ifndef __UNDO__H
#define __UNDO__H

class UnwrapMod;
class MeshTopoData;

//*********************************************************
// Undo record for TV posiitons only
//*********************************************************

class TVertRestore : public RestoreObj {
	public:
		UnwrapMod *mod;
		Tab<UVW_TVVertClass> undo, redo;
		BitArray uvsel, rvsel;
		BitArray uesel, resel;
		BitArray ufsel, rfsel;

		BOOL updateView;

		TVertRestore(UnwrapMod *m, BOOL update = TRUE); 
		void Restore(int isUndo);
		void Redo();
		void EndHold();
		TSTR Description();
		
		
	};

//*********************************************************
// Undo record for TV posiitons and face topology
//*********************************************************


class TVertAndTFaceRestore : public RestoreObj {
	public:
		UnwrapMod *mod;
		Tab<UVW_TVVertClass> undo, redo;
		Tab<UVW_TVFaceClass*> fundo, fredo;
		BitArray uvsel, rvsel;
		BitArray ufsel, rfsel;
		BitArray uesel, resel;

		BOOL update;

		TVertAndTFaceRestore(UnwrapMod *m);
		~TVertAndTFaceRestore() ;
		void Restore(int isUndo) ;
		void Redo();
		void EndHold();
		TSTR Description();
	};


class TSelRestore : public RestoreObj {
	public:
		UnwrapMod *mod;
		BitArray undo, redo;
		BitArray fundo, fredo;
		BitArray eundo, eredo;

		TSelRestore(UnwrapMod *m);
		void Restore(int isUndo);
		void Redo();
		void EndHold();
		TSTR Description();
	};	

class ResetRestore : public RestoreObj {
	public:
		UnwrapMod *mod;
		int uchan, rchan;

		Tab<UVW_TVVertClass> undo, redo;
		Tab<UVW_TVFaceClass*> fundo, fredo;
		BitArray uvsel, rvsel;
		BitArray uesel, resel;
		BitArray ufsel, rfsel;
		Tab<Control *> ucont, rcont;


		ResetRestore(UnwrapMod *m);
		~ResetRestore();
		void Restore(int isUndo);
		void Redo();
		void EndHold();

		
		TSTR Description();
	};


// MeshSelRestore --------------------------------------------------
class UnwrapSelRestore : public RestoreObj {
public:
	BitArray usel, rsel;
	BitArray *sel;
	UnwrapMod *mod;
	MeshTopoData *d;
//	int level;

	UnwrapSelRestore(UnwrapMod *m, MeshTopoData *d);
	UnwrapSelRestore(UnwrapMod *m, MeshTopoData *d, int level);
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {
//		d->held=FALSE;
	}
	TSTR Description() { return TSTR(_T(GetString(IDS_PW_SELECTRESTORE))); 
	}
};

class UnwrapPivotRestore : public RestoreObj {
public:
	Point3 upivot, rpivot;
	UnwrapMod *mod;

	UnwrapPivotRestore(UnwrapMod *m);
	
	void Restore(int isUndo);
	void Redo();
	int Size() { return 1; }
	void EndHold() {
//		d->held=FALSE;
	}
	TSTR Description() { return TSTR(_T(GetString(IDS_PW_PIVOTRESTORE))); 
	}
};

#endif // __UWNRAP__H
