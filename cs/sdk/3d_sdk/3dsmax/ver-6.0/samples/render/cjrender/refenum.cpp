//***************************************************************************
// CJRender - [refenum.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Implementation of enumeration classes
//
//***************************************************************************

#include "maxincl.h"
#include "cjrender.h"
#include "refenum.h"


//***************************************************************************
// These are enumeration functions for maps, references etc.
//***************************************************************************


//***************************************************************************
// Base class for enumerating references
//***************************************************************************

void EnumRefs(ReferenceMaker *rm, RefEnumProc &proc)
{
	proc.proc(rm);
	int numRefs = rm->NumRefs();
	for (int i=0; i<numRefs; i++) {
		ReferenceMaker *srm = rm->GetReference(i);
		if (srm) {
			EnumRefs(srm,proc);		
		}
	}
}


//***************************************************************************
// Base class of the map enumerator
//***************************************************************************

int EnumMaps(MtlBase *mb, int subMtl,  MtlEnum &tenum)
{
	if (IsTex(mb)) {
		if (!tenum.proc(mb,subMtl)) {
			return 0;
		}
	}
	for (int i=0; i<mb->NumSubTexmaps(); i++) {
		Texmap *st = mb->GetSubTexmap(i); 
		if (st) {
			int subm = (mb->IsMultiMtl()&&subMtl<0)?i:subMtl;
			if (mb->SubTexmapOn(i)) {
				if (!EnumMaps(st,subm,tenum)) {
					return 0;
				}
			}
		}
	}
	if (IsMtl(mb)) {
		Mtl *m = (Mtl *)mb;
		for (i=0; i<m->NumSubMtls(); i++) {
			Mtl *sm = m->GetSubMtl(i);
			if (sm) {
				int subm = (mb->IsMultiMtl()&&subMtl<0)?i:subMtl;
				if (!EnumMaps(sm,subm,tenum)) {
					return 0;
				}
			}
		}
	}
	return 1;
}


//***************************************************************************
// Enumerate a material tree
//***************************************************************************

int EnumMtlTree(MtlBase *mb, int subMtl, MtlEnum &tenum)
{
	for (int i=0; i<mb->NumSubTexmaps(); i++) {
		Texmap *st = mb->GetSubTexmap(i);
		if (st) {
			if (!EnumMtlTree(st,subMtl, tenum)) {
				return 0;
			}
		}
	}
	if (IsTex(mb)) {
		if (!tenum.proc(mb,subMtl)) {
			return 0;
		}
	}
	if (IsMtl(mb)) {
		Mtl *m = (Mtl *)mb;
		for (i=0; i<m->NumSubMtls(); i++) {
			Mtl *sm = m->GetSubMtl(i);
			int subm = (mb->IsMultiMtl()&&subMtl<0)?i:subMtl;
			if (sm) {
				if (!EnumMtlTree(sm,subm,tenum)) {
					return 0;
				}
			}
		}
		if (!tenum.proc(mb,subMtl)) {
			return 0;
		}
	}
	return 1;
}

//***************************************************************************
// Constructor of map enumerator
//***************************************************************************

GetMaps::GetMaps(MtlBaseLib *mbl)
{
	mlib = mbl;
}

//***************************************************************************
// Implementation of the map enumerator
//***************************************************************************

void GetMaps::proc(ReferenceMaker *rm)
{
	if (IsTex((MtlBase*)rm)) {
		mlib->AddMtl((MtlBase *)rm);
	}
}


//***************************************************************************
// Class to manage names of missing maps
//***************************************************************************

CheckFileNames::CheckFileNames(NameTab* n)
{
	missingMaps = n;
}

//***************************************************************************
// Add a name to the list if it's not already there
//***************************************************************************

void CheckFileNames::RecordName(TCHAR *name)
{ 
	if (name) {
		if (name[0]!=0) {
			if (missingMaps->FindName(name)<0) {
			    missingMaps->AddName(name);
			}
		}
	}
}


//***************************************************************************
// Constructor of map loader
// Map loader enum proc
//***************************************************************************

MapLoadEnum::MapLoadEnum(TimeValue time)
{ 
	t = time; 
}

int MapLoadEnum::proc(MtlBase *m, int subMtlNum)
{
	Texmap *tm = (Texmap *)m;
	tm->LoadMapFiles(t);
	return 1;
}

//***************************************************************************
// Constructor of AutoReflect enumerator
// AutoReflect enum proc
// Note:
//***************************************************************************

MapSetupEnum::MapSetupEnum(TimeValue time, CJRenderer* renderer, Instance* inst)
	: rmc(renderer, inst)
{
	t = time; 
	r = renderer;
}

int MapSetupEnum::proc(MtlBase *m, int subMtlNum)
{
	ULONG reqmask = MTLREQ_AUTOREFLECT | MTLREQ_AUTOMIRROR | MTLREQ_PREPRO;
	if (m->LocalRequirements(subMtlNum) & reqmask) {
		rmc.SetSubMtlIndex(subMtlNum);
		m->BuildMaps(t, rmc);
	}
	return 1;
}


//***************************************************************************
// Enumerator to clear the work flag on all objects
//***************************************************************************

void ClearFlags::proc(ReferenceMaker *rm)
{
	if (rm) {
		rm->ClearAFlag(A_WORK1);
	}
}


//***************************************************************************
// Enumerator to call RenderBegin() on all objects
//***************************************************************************

BeginEnum::BeginEnum(TimeValue startTime)
{
	t = startTime;
}

void BeginEnum::proc(ReferenceMaker *rm)
{
	if (rm) {
		if (!rm->TestAFlag(A_WORK1)) {
			rm->SetAFlag(A_WORK1);
			rm->RenderBegin(t);
		}
	}
}

//***************************************************************************
// Enumerator to call RenderEnd() on all objects
//***************************************************************************

EndEnum::EndEnum(TimeValue endTime)
{
	t = endTime;
}

void EndEnum::proc(ReferenceMaker *rm)
{
	if (rm) {
		if (!rm->TestAFlag(A_WORK1)) {
			rm->SetAFlag(A_WORK1);
			rm->RenderEnd(t);
		}
	}
}
