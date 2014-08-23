#include "PainterTextureSample.h"



////////////////////////////////////////////////////////////////////////////////////////
//
// For ParamMap2 Create an instance of a class derived from ParamMap2UserDlgProc.  This 
// is used to provide special processing of controls in the rollup page.-- NH 
// The original use of this has been replaced by the use of PBAccessor
//
////////////////////////////////////////////////////////////////////////////////////////


BOOL PaintTextureTestDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) 
		{	
		case WM_INITDIALOG:
			{
			tex->InitUI(hWnd);
			break;
			}
		case WM_DESTROY:
			{
			tex->DestroyUI(hWnd);
			break;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				//case IDC_SEL_SPHERE:				
					//mod->flags |= CONTROL_UNIFORM|CONTROL_HOLD;
					//mod->NotifyDependents(FOREVER,SELECT_CHANNEL,REFMSG_CHANGE);
					//break;

				case IDC_PAINT:
					tex->Paint();
					break;
				case IDC_PAINTOPTIONS:
					tex->PaintOptions();
					break;
				}
		}

	return FALSE;
	
}

