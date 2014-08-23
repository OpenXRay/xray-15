/**********************************************************************
 *<
	FILE: ffdui.h

	DESCRIPTION:

	CREATED BY: Ravi Karra

	HISTORY: created 1/11/99

 *>	Copyright (c) 1996 Rolf Berteig, All Rights Reserved.
 **********************************************************************/

#ifndef __FFDUI__H
#define __FFDUI__H

#include "ffdmod.h"
#include "ActionTable.h"

// The reference versions
#define ES_REF_VER_0	0	// Pre-r3 (vertex controllers are only refs)
#define ES_REF_VER_1	1	// MAXr3 (ref 0 is master vertex controller)

// Reference indices
#define TM_REF		0
#define PBLOCK_REF	1
#define MASTER_REF	2

// Selection levels
#define SEL_OBJECT		0
#define SEL_POINTS		1
#define SEL_LATTICE		2
#define SEL_SETVOLUME	3

// Right Click ID's
enum { RC_ANIMATE_ALL = SEL_SETVOLUME+1, RC_ALLX, RC_ALLY, RC_ALLZ };

// Keyboard Shortcuts stuff
const ActionTableId kFFDActions = 0x7ed73ca2;
const ActionContextId kFFDContext = 0x7ed73ca2;

#define NumElements(array) (sizeof(array) / sizeof(array[0]))

ActionTable* GetActions();

template <class T>
class FFDActionCB : public ActionCallback
{
	public:
		T*		ffd;
				FFDActionCB(T *ffd) { this->ffd = ffd; }
		BOOL	ExecuteAction(int id); 
};

// Right-click menu stuff
template <class T>
class FFDRightMenu : public RightClickMenu {
	public:
		T		*ffd;
				FFDRightMenu(T *ffd) { this->ffd = ffd; }
		void	Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m);
		void	Selected(UINT id);
		void	AddMenu(RightClickMenuManager* pManager, int commandId, 
					int stringId, DWORD flags = MF_STRING, TCHAR* pName = NULL);
};

class sMyEnumProc : public DependentEnumProc 
	{
      public :
		INodeTab Nodes;              
		virtual int proc(ReferenceMaker *rmaker); 
	};


template <class T>
BOOL FFDActionCB<T>::ExecuteAction(int id) {
	switch (id) {
		case ID_SUBOBJ_TOP:
			ffd->ip->SetSubObjectLevel(SEL_OBJECT);
			ffd->ip->RedrawViews(ffd->ip->GetTime());
			return TRUE;
		case ID_SUBOBJ_CP:
			ffd->ip->SetSubObjectLevel(SEL_POINTS);
			return TRUE;
		case ID_SUBOBJ_LATTICE:
			ffd->ip->SetSubObjectLevel(SEL_LATTICE);
			return TRUE;
		case ID_SUBOBJ_SETVOLUME:
			ffd->ip->SetSubObjectLevel(SEL_SETVOLUME);
			return TRUE;
	}
	return FALSE;
	
}

template <class T>
void FFDRightMenu<T>::AddMenu(RightClickMenuManager* manager,
                   int commandId, int stringId, DWORD flags,
                   TCHAR* pName)
{
    TCHAR buf[64];
    TCHAR buf2[512];
    TCHAR *pOpName; 
    if (stringId)
        pOpName = GetString(stringId);
    else
        pOpName = pName;
            
    if (ffd->ip->GetActionManager()->GetShortcutString(kFFDActions, commandId, buf)) {
        sprintf(buf2, _T("%s\t%s"), pOpName, buf);
    } else
        _tcscpy(buf2, pOpName);
    
    manager->AddMenu(this, flags, commandId, buf2);
}

template <class T>
void FFDRightMenu<T>::Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m) {
	manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
	AddMenu(manager, ID_SUBOBJ_TOP,			IDS_RK_TOP,			(ffd->selLevel==SEL_OBJECT) ? MF_CHECKED : MF_UNCHECKED);
	AddMenu(manager, ID_SUBOBJ_CP,			IDS_RB_CONTPOINTS,	(ffd->selLevel==SEL_POINTS) ? MF_CHECKED : MF_UNCHECKED);
	if(ffd->SuperClassID()==OSM_CLASS_ID)
	{
		AddMenu(manager, ID_SUBOBJ_LATTICE,		IDS_RB_LATTICE,		(ffd->selLevel==SEL_LATTICE) ? MF_CHECKED : MF_UNCHECKED);
		AddMenu(manager, ID_SUBOBJ_SETVOLUME,	IDS_RK_SETVOLUME,	(ffd->selLevel==SEL_SETVOLUME) ? MF_CHECKED : MF_UNCHECKED);
	}
	manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
	AddMenu(manager, RC_ANIMATE_ALL,		IDS_RK_ANIMATEALL,	MF_STRING);	
#ifdef INC_CONSTRAINTS
	AddMenu(manager, RC_ALLX,				IDS_RK_ALLX,		(ffd->allX)?MF_CHECKED:MF_UNCHECKED);
	AddMenu(manager, RC_ALLY,				IDS_RK_ALLY,		(ffd->allY)?MF_CHECKED:MF_UNCHECKED);
	AddMenu(manager, RC_ALLZ,				IDS_RK_ALLZ,		(ffd->allZ)?MF_CHECKED:MF_UNCHECKED);
#endif
}

template <class T>
void FFDRightMenu<T>::Selected(UINT id) {	
	if (ffd->ffdActionCB->ExecuteAction(id))		
		return;		
	switch(id) {				
		case RC_ANIMATE_ALL:
			ffd->AnimateAll();
			break;
#ifdef INC_CONSTRAINTS
		case RC_ALLX: {
			ffd->allX = !ffd->allX;
			ICustButton *but = GetICustButton(GetDlgItem(ffd->pblock->GetMap()->GetHWnd(),IDC_FFD_ALLX));
			but->SetCheck(ffd->allX);
			ReleaseICustButton(but);			
			break;
			}
		case RC_ALLY: {
			ffd->allY = !ffd->allY;
			ICustButton *but = GetICustButton(GetDlgItem(ffd->pblock->GetMap()->GetHWnd(),IDC_FFD_ALLY));
			but->SetCheck(ffd->allY);
			ReleaseICustButton(but);						
			break;
			}
		case RC_ALLZ: {
			ffd->allZ = !ffd->allZ;
			ICustButton *but = GetICustButton(GetDlgItem(ffd->pblock->GetMap()->GetHWnd(),IDC_FFD_ALLZ));
			but->SetCheck(ffd->allZ);
			ReleaseICustButton(but);			
			break;
			}
#endif
		}
	}

#endif //__FFDUI__H
