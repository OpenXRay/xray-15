/**********************************************************************
 *<
	FILE: RefCheck.h

	DESCRIPTION:	Template Utility

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#ifndef __REFCHECK__H
#define __REFCHECK__H

#include "Max.h"
#include "resource.h"
#include "utilapi.h"
#include "istdplug.h"
#include "stdmat.h"
#include "bmmlib.h"
#include <io.h>

#include "iparamb2.h"
#include "iparamm2.h"
#include "IDxMaterial.h"

#define REFCHECK_CLASS_ID	Class_ID(0xa7d423ed, 0x64de98f9)

extern ClassDesc*	GetRefCheckDesc();
extern HINSTANCE	hInstance;
extern TCHAR*		GetString(int id);

#define COPYWARN_YES		0x00
#define COPYWARN_YESTOALL	0x01
#define COPYWARN_NO			0x02
#define COPYWARN_NOTOALL	0x03


/**
 * The NameEnumCallBack used to find all Light Dist. files.
 */

class EnumLightDistFileCallBack : public NameEnumCallback
{
public:

	NameTab fileList;
	int size;

	EnumLightDistFileCallBack() { size = 0; }

	void RecordName(TCHAR *name)
	{
		//convert to lower case
		TSTR buf(name);
		int i = 0;
		while(buf[i] && i <100)	{
			buf[i] = _totlower(buf[i]);
			i++;
		}
		// check for distribution types before appending.
		if(_tcsstr(buf, ".ies") || 
			_tcsstr(buf, ".cibse") ||
			_tcsstr(buf, ".ltli"))		{
			if(fileList.FindName(name) < 0)	{
				fileList.AddName(name);
				size++;
			}
		}
			
	}
};

class DxMaterialBaseFileCallback : public NameEnumCallback
{
public:

	NameTab fileList;
	int size;

	DxMaterialBaseFileCallback() { size = 0; }

	void RecordName(TCHAR *name)
	{
		//convert to lower case
		TSTR buf(name);
		int i = 0;
		while(buf[i] && i <100)	{
			buf[i] = _totlower(buf[i]);
			i++;
		}
		if(fileList.FindName(name) < 0)	
		{
			fileList.AddName(name);
			size++;
		}
		
	}
};



class RefCheck : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		HWND hDialog;
		TSTR bitmapName;
		

		RefCheck();
		~RefCheck();

		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		// Main dialog resize functions
		void ResizeWindow(int x, int y);
		void SetMinDialogSize(int w, int h)	{ minDlgWidth = w; minDlgHeight = h; }
		int	 GetMinDialogWidth()			{ return minDlgWidth; }
		int  GetMinDialogHeight()			{ return minDlgHeight; }

		void CheckDependencies();
		void Update();
		void StripSelected();
		void StripAll();
		void DoDialog();
		void DoSelection();
		void SetPath(TCHAR* path, BitmapTex* map);
		void SetDxPath(MtlBase * mtl, TCHAR * path, TCHAR * newPath, TCHAR *ext);

		void StripMapName(TCHAR* path);
		void BrowseDirectory();
		BOOL ChooseDir(TCHAR *title, TCHAR *dir);
		void EnableEntry(HWND hWnd, BOOL bEnable, int numSel);
		void ShowInfo();
		void HandleInfoDlg(HWND dlg);

		BOOL GetIncludeMatLib();
		BOOL GetIncludeMedit();

		int  CopyWarningPrompt(HWND hParent, TCHAR* filename);
		void CopyMaps();
		void SetActualPath();
		void SelectMissing();

		BOOL FindMap(TCHAR* mapName, TCHAR* newName);

		BOOL GetCopyQuitFlag()	{ return bCopyQuitFlag; }
		void SetCopyQuitFlag(BOOL bStatus)	{ bCopyQuitFlag = bStatus; }
		void SetInfoTex(BitmapTex* b) { infoTex = b; }
		BitmapTex* GetInfoTex() { return infoTex; }
		void EnumerateNodes(INode *root);

	private:
		BOOL		bCopyQuitFlag;
		BitmapTex*	infoTex;
		int			minDlgWidth;
		int			minDlgHeight;
		EnumLightDistFileCallBack distributionLister;	
				
		//PBITMAPINFO	pDib;
};

class FindNodesProc : public DependentEnumProc {
public:
	FindNodesProc(INodeTab* tab) {
		nodetab = tab;
	}
	int proc(ReferenceMaker *ref) {
		switch (ref->SuperClassID()) {
				case BASENODE_CLASS_ID:
					INode* n = (INode*)ref;
					nodetab->Append(1, &n, 5);
					break;
		}
		return 0;
	}
private:
	INodeTab* nodetab;
};
#endif // __REFCHECK__H
