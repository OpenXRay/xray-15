#include "mods.h"
#include "bonesdef.h"

#include "Maxscrpt.h"
#include "Strings.h"
#include "arrays.h"
#include "3DMath.h"
#include "Numbers.h"
#include "definsfn.h"

INT_PTR CALLBACK WeightTableDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddCustomListDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) ;

/************************************************************************
************************************************************************

	MOVE THESE FUNCTIONS TO COMMAND.CPP when we are done

************************************************************************
************************************************************************/
//maxscript setup to bring up the weight table
//skinops.buttonWeightTable $.modifiers[#Skin] 
def_struct_primitive( buttonWeightTable,skinOps, "buttonWeightTable" );

#define get_bonedef_mod()																\
	Modifier *mod = arg_list[0]->to_modifier();										\
	Class_ID id = mod->ClassID();													\
	if ( id != Class_ID(9815843,87654) )	\
		throw RuntimeError(GetString(IDS_PW_NOT_BONESDEF_ERROR), arg_list[0]);			\
	BonesDefMod *bmod = (BonesDefMod*)mod;			


Value*
buttonWeightTable_cf(Value** arg_list, int count)
{
	check_arg_count(buttonWeightTable, 1, count);
	get_bonedef_mod();
//get first mod context not sure how this will work if multiple instanced are selected
//maybe should use COREinterface instead of local interface ????
	if ( !bmod->ip ) throw RuntimeError(GetString(IDS_PW_SKIN_NOT_SELECTED), arg_list[0]);

	ModContextList mcList;		
	INodeTab nodes;

	bmod->ip->GetModContexts(mcList,nodes);
	int objects = mcList.Count();

	if (objects != 0)
		{
		bmod->fnWeightTable();
		}

	return &ok;	
}

void BonesDefMod::fnWeightTable()
	{
	HWND hWnd = hParam;
	hWnd = GetCOREInterface()->GetMAXHWnd();
	RegisterClasses2();
	if (!hWeightTable) 
		{


		GetCOREInterface()->GetMenuManager()->UpdateMenuBar();
		
		hWeightTable = CreateDialogParam(hInstance,MAKEINTRESOURCE(IDD_WEIGHTTABLE_DIALOG),
							hWnd,WeightTableDlgProc,(LPARAM)this);
	// AF (08/01/01) draw the custom menu bar
		if (weightTableWindow.GetShowMenu())
			{
			IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(weightTableWindow.kWeightTableMenuBar);
			assert(pContext);
			pContext->CreateWindowsMenu();
			SetMenu(hWeightTable, pContext->GetCurWindowsMenu());
			DrawMenuBar(hWeightTable);
			pContext->UpdateWindowsMenu();
			}
		} 
	else 
		{
		SetForegroundWindow(hWeightTable);
		ShowWindow(hWeightTable,SW_RESTORE);
		}

	}



static LRESULT CALLBACK NameListLabelProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	static int lastX;
	static int initialX = 0;
	static int initialSize = 0;
	static BOOL resize = FALSE;
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	
	switch (msg) {
		
		case WM_SIZE:			
			{
			if (mod) mod->weightTableWindow.ResizeOffScreenBuffers();
			break;
			}

		case WM_PAINT:
			if (mod) mod->weightTableWindow.PaintNameListLabel();
			break;

		case WM_LBUTTONDOWN:	
			{
						
			WINDOWPLACEMENT winPos;
			GetWindowPlacement(hWnd , &winPos);

//check if inside the move rect for the vert name
			int x = (short)LOWORD(lParam);  // horizontal position of cursor 
			int y = (short)HIWORD(lParam);  // vertical position of cursor 
			initialX = x;
			lastX = x;
			x += winPos.rcNormalPosition.left;
			if (x>=(winPos.rcNormalPosition.right-3)) 
				{
				resize = TRUE;
				if (mod->weightTableWindow.GetFlipFlopUI())
					initialSize = mod->weightTableWindow.buttonWidth;
				else initialSize = mod->weightTableWindow.vertNameWidth;
				SetCapture(hWnd);

				}
			break;
			}
		case WM_LBUTTONUP:	
			{
			if (resize)
				{
				ReleaseCapture();
				if (mod->weightTableWindow.hWnd) 
					{
					InvalidateRect(mod->weightTableWindow.hWnd,NULL,TRUE);
					UpdateWindow(mod->weightTableWindow.hWnd);
					}
				}
			resize = FALSE;
			break;
			}
		case WM_MOUSEMOVE:	
			{
			int x = (short)LOWORD(lParam);  // horizontal position of cursor
			if (resize)
				{
				 
				if (x != lastX)
					{
					lastX = x;
					int offset = x - initialX ;
					if (mod->weightTableWindow.GetFlipFlopUI())
						{
						mod->weightTableWindow.buttonWidth = initialSize + offset;
						if (mod->weightTableWindow.buttonWidth < 50)
							mod->weightTableWindow.buttonWidth = 50;

						}
					else
						{
						mod->weightTableWindow.vertNameWidth = initialSize + offset;
						if (mod->weightTableWindow.vertNameWidth < 50)
							mod->weightTableWindow.vertNameWidth = 50;
						}
					mod->weightTableWindow.ResizeWindowControls();
					mod->weightTableWindow.InvalidateViews();
					}
				}
			else
				{
				WINDOWPLACEMENT winPos;
				GetWindowPlacement(hWnd , &winPos);
				x += winPos.rcNormalPosition.left;
				if (x>=(winPos.rcNormalPosition.right-3)) 
					{
					SetCursor(LoadCursor(NULL,IDC_SIZEWE));
					}
				else SetCursor(LoadCursor(NULL,IDC_ARROW));
				}
			break;
			}



		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}	
	return 0;

	}




static LRESULT CALLBACK NameListProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	
	static BOOL mouseLDown = FALSE;

	static int lastX = 0;
	static int initialX = 0;
	static int initialSelX = 0;
	static int initialSelY = 0;
	static int initialSize = 0;
	static BOOL resize = FALSE;
	static BitArray initialSelection;


	switch (msg) {
		case WM_CREATE:
			break;

		case 0x020A:
			{
			short delta = (short) HIWORD(wParam);
			if (delta>0)
				mod->weightTableWindow.ScrollRowUpOne();
			else mod->weightTableWindow.ScrollRowDownOne();
			//DebugPrint("WHeel\n");
			break;
			}

		case WM_SIZE:			
			{
			if (mod) mod->weightTableWindow.ResizeOffScreenBuffers();
			break;
			}

		case WM_PAINT:
			if (mod) mod->weightTableWindow.PaintNameList();
			break;


		case WM_LBUTTONDOWN:
			{
//check if in name list if so we are in select mode
			int flags = wParam;        // key flags 
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 

			initialSelX = xPos;
			initialSelY = yPos;
			BOOL alt = FALSE;
			BOOL ctrl = flags & MK_CONTROL;
			SHORT iret = GetAsyncKeyState (VK_MENU);
			if (iret==-32767)
				alt = TRUE;
			if ((!alt) && (!ctrl))
				mod->weightTableWindow.ClearAllSelections();
			if (ctrl)
				mod->weightTableWindow.mouseButtonFlags = CTRL_KEY;
			if (alt)
				mod->weightTableWindow.mouseButtonFlags = ALT_KEY;
			mouseLDown = TRUE;

			if (mod->weightTableWindow.GetFlipFlopUI())
				{
				WINDOWPLACEMENT winPos;
				GetWindowPlacement(hWnd , &winPos);

//check if inside the move rect for the vert name
				int x = (short)LOWORD(lParam);  // horizontal position of cursor 
				int y = (short)HIWORD(lParam);  // vertical position of cursor 
				initialX = x+winPos.rcNormalPosition.left;
				lastX = x+winPos.rcNormalPosition.left;

				if ((x%mod->weightTableWindow.vertNameWidth)>=(mod->weightTableWindow.vertNameWidth-3)) 
					{
					resize = TRUE;
					initialSize = mod->weightTableWindow.vertNameWidth;
					}
				}
			if (!resize)
				{
				mod->weightTableWindow.SelectVerts(xPos,yPos);
				mod->weightTableWindow.GetSelectionSet(initialSelection);
				}
			SetCapture(hWnd);

			


			break;
			}
		case WM_LBUTTONDBLCLK:
			{
			break;
			}
		case WM_LBUTTONUP:
			{
			if (resize)
				{
				if (mod->weightTableWindow.hWnd) 
					{
					InvalidateRect(mod->weightTableWindow.hWnd,NULL,TRUE);
					UpdateWindow(mod->weightTableWindow.hWnd);
					}
				}
			if (mouseLDown)
				{
				mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());	
				}


			resize = FALSE;
			ReleaseCapture();

			mouseLDown = FALSE;
			break;
			}
		case WM_MOUSEMOVE:
			{
			int x = (short)LOWORD(lParam);  // horizontal position of cursor 
		 	int y = (short)HIWORD(lParam);  // vertical position of cursor 
			if (resize)
				{

				if (mod->weightTableWindow.GetFlipFlopUI())
					{
					WINDOWPLACEMENT winPos;
					GetWindowPlacement(hWnd , &winPos);
					x += winPos.rcNormalPosition.left;
					if (x != lastX)
						{

						int offset = x - initialX ;
						lastX = x;
						mod->weightTableWindow.vertNameWidth = initialSize + offset;
						if (mod->weightTableWindow.vertNameWidth < 50)
							mod->weightTableWindow.vertNameWidth = 50;

						mod->weightTableWindow.ResizeWindowControls();
	//					mod->weightTableWindow.ComputeNumberRowsColumns();
						mod->weightTableWindow.InvalidateViews();
						
						}
					}

				}
			else if (mouseLDown)  
				{
				WINDOWPLACEMENT winPos;
				GetWindowPlacement(hWnd , &winPos);
				int width = winPos.rcNormalPosition.right - winPos.rcNormalPosition.left;
				int height = winPos.rcNormalPosition.bottom - winPos.rcNormalPosition.top;

				int addY = 0;
				int addX = 0;
				if (!mod->weightTableWindow.GetFlipFlopUI())
					{
					if (y < 0)
						{
						int oldPos = mod->weightTableWindow.firstRow;
						mod->weightTableWindow.ScrollRowUpOne();
						int newPos = mod->weightTableWindow.firstRow;
						if (oldPos != newPos)
							initialSelY += mod->weightTableWindow.textHeight;
						}
					else if (y > height)
						{
						int oldPos = mod->weightTableWindow.firstRow;
						mod->weightTableWindow.ScrollRowDownOne();
						int newPos = mod->weightTableWindow.firstRow;
	
						if (oldPos != newPos)
							{
							initialSelY -= mod->weightTableWindow.textHeight;
							addY = -mod->weightTableWindow.textHeight;
							}
						}
					}
				else
					{
					if (x< 0)
						{
						int oldPos = mod->weightTableWindow.firstColumn;
						mod->weightTableWindow.ScrollColumnUpOne();
						int newPos = mod->weightTableWindow.firstColumn;

						if (oldPos != newPos)
							initialSelX += mod->weightTableWindow.vertNameWidth;
						}
					else if (x > width)
						{
						int oldPos = mod->weightTableWindow.firstColumn;
						mod->weightTableWindow.ScrollColumnDownOne();
						int newPos = mod->weightTableWindow.firstColumn;

						if (oldPos != newPos)
							{
							initialSelX -= mod->weightTableWindow.vertNameWidth;
							addX = -mod->weightTableWindow.vertNameWidth;
							}
						}

					}
				BitArray dragSel;
				mod->weightTableWindow.SelectVertsRange((initialSelX+addX),(initialSelY+addY),x,y,dragSel);

				int flags = wParam;        // key flags 
				BOOL ctrl = flags & MK_CONTROL;

				if (ctrl)
					dragSel |= initialSelection;

				mod->weightTableWindow.SetSelection(dragSel);

				UpdateWindow( hWnd);
				}
			else
				{
				WINDOWPLACEMENT winPos;
				GetWindowPlacement(hWnd , &winPos);
				if ((x%mod->weightTableWindow.vertNameWidth)>=(mod->weightTableWindow.vertNameWidth-3)) 
					{
					SetCursor(LoadCursor(NULL,IDC_SIZEWE));
					}
				else SetCursor(LoadCursor(NULL,IDC_ARROW));
				}
			break;
			}




		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}	
	return 0;

	}


#define NUM_TIPS  5
#define MAX_COLS  1
#define MAX_ROWS  5

HWND DoCreateTooltip(HWND hwndOwner, BonesDefMod *mod) 
	{ 
    HWND hwndTT;    // handle of tooltip 
	int row;
    TOOLINFO ti;    // tool information 
    int id = 0;     // offset to string identifiers 
    static char *szTips[NUM_TIPS] =   
    { 
    "Cut", "Copy", "Paste", "Undo", "Open"
    }; 
	
	static int ttStrings[NUM_TIPS] =   
	{
	IDS_PW_TTSELECTED,IDS_PW_TTMODIFIED,IDS_PW_TTNORMALIZED,IDS_PW_TTRIGID,IDS_PW_TTRIGIDHANDLE
	};

    // Ensure that the common control DLL is loaded, and create 
    // a tooltip control. 
    InitCommonControls(); 
 
    hwndTT = CreateWindow(TOOLTIPS_CLASS, (LPSTR) NULL, TTS_ALWAYSTIP, 
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
        NULL, (HMENU) NULL, hInstance, NULL); 

	int width = 0;
	int height = 0;

	if (mod->weightTableWindow.GetFlipFlopUI())
		{
		width = mod->weightTableWindow.buttonWidth;
		height = mod->weightTableWindow.textHeight;
		}
	else
		{
		width = mod->weightTableWindow.textHeight;
		height = mod->weightTableWindow.textHeight;
		}
 
    if (hwndTT == (HWND) NULL) 
        return (HWND) NULL; 
 
    // Divide the client area into a grid of rectangles, and add each 
    // rectangle to the tooltip. 
    for (row = 0; row < MAX_ROWS ; row++ ) 
		{
            ti.cbSize = sizeof(TOOLINFO); 
            ti.uFlags = 0; 
            ti.hwnd = hwndOwner; 
            ti.hinst = hInstance; 
            ti.uId = (UINT) id; 
//            ti.lpszText = (LPSTR) szTips[id++]; 
            ti.lpszText = (LPSTR) GetString(ttStrings[row]); 
			if (mod->weightTableWindow.GetFlipFlopUI())
				{
				ti.rect.left = 0; 
	            ti.rect.top = row*height; 
		        ti.rect.right = ti.rect.left + width; 
			    ti.rect.bottom = ti.rect.top + height; 
				}
			else
				{
				ti.rect.left = row*width; 
	            ti.rect.top = 0; 
		        ti.rect.right = ti.rect.left + width; 
			    ti.rect.bottom = ti.rect.top + height; 
				}
 
            if (!SendMessage(hwndTT, TTM_ADDTOOL, 0, 
                    (LPARAM) (LPTOOLINFO) &ti)) 
                return NULL; 
        } 
 
    return hwndTT; 
	} 


static LRESULT CALLBACK AttribListLabelProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	
	switch (msg) {

		case WM_CLOSE:
			{
			break;
			}

        case WM_MOUSEMOVE: 
        case WM_LBUTTONDOWN: 
        case WM_LBUTTONUP: 
        case WM_RBUTTONDOWN: 
        case WM_RBUTTONUP: 
            if (mod->weightTableWindow.toolTipHwnd != NULL) { 
                MSG ttmsg; 
 
                ttmsg.lParam = lParam; 
                ttmsg.wParam = wParam; 
                ttmsg.message = msg; 
                ttmsg.hwnd = hWnd; 
                SendMessage(mod->weightTableWindow.toolTipHwnd, TTM_RELAYEVENT, 0, 
                    (LPARAM) (LPMSG) &ttmsg); 
            } 
            break; 

		case WM_SIZE:			
			{
			if (mod) mod->weightTableWindow.ResizeOffScreenBuffers();
			break;
			}

		case WM_PAINT:
			if (mod->weightTableWindow.toolTipHwnd == NULL)
				mod->weightTableWindow.toolTipHwnd = DoCreateTooltip(hWnd, mod); 

			if (mod) mod->weightTableWindow.PaintAttribListLabel();
			break;
		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}	
	return 0;

	}

static LRESULT CALLBACK AttribListGlobalProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	static int currentVert = -1;
	static int currentAttrib = -1;

	switch (msg) {
		case WM_CREATE:
			break;

		case 0x020A:
			DebugPrint("WHeel\n");
			break;

		case WM_SIZE:			
			{
			if (mod) mod->weightTableWindow.ResizeOffScreenBuffers();				
			break;
			}

		case WM_PAINT:
			if (mod) mod->weightTableWindow.PaintAttribListGlobal();
			break;
		case WM_LBUTTONDOWN:
			{
			int flags = wParam;        // key flags 
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			currentAttrib = mod->weightTableWindow.GetCurrentAttrib(xPos,yPos);
			currentVert = mod->weightTableWindow.GetCurrentVert(xPos,yPos);
			SetCapture(hWnd);

			break;
			}
		case WM_LBUTTONDBLCLK:
			{
			break;
			}
		case WM_LBUTTONUP:
			{

			theHold.Begin();
			mod->weightTableWindow.HoldWeights();
			mod->weightTableWindow.HoldSelection();
			theHold.Accept(GetString(IDS_PW_ATTRIBUTESCHANGE));

			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			if ( (mod->weightTableWindow.GetCurrentAttrib(xPos,yPos) == currentAttrib) &&
				 (mod->weightTableWindow.GetCurrentVert(xPos,yPos) == currentVert) )
				{
				mod->weightTableWindow.ToggleGlobalAttribute(xPos,yPos);
				mod->weightTableWindow.InvalidateViews();
				}
			ReleaseCapture();

			break;
			}

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}	
	return 0;

	}



static LRESULT CALLBACK AttribListProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	static int currentVert = -1;
	static int currentAttrib = -1;

	switch (msg) {
		case WM_CREATE:
			break;

		case 0x020A:
			DebugPrint("WHeel\n");
			break;

		case WM_SIZE:			
			{
			if (mod) mod->weightTableWindow.ResizeOffScreenBuffers();
			break;
			}

		case WM_PAINT:
			if (mod) mod->weightTableWindow.PaintAttribList();
			break;
		case WM_LBUTTONDOWN:
			{
			int flags = wParam;        // key flags 
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			currentAttrib = mod->weightTableWindow.GetCurrentAttrib(xPos,yPos);
			currentVert = mod->weightTableWindow.GetCurrentVert(xPos,yPos);
			SetCapture(hWnd);

			break;
			}
		case WM_LBUTTONDBLCLK:
			{
			break;
			}
		case WM_LBUTTONUP:
			{

			theHold.Begin();
			mod->weightTableWindow.HoldWeights();
			mod->weightTableWindow.HoldSelection();
			theHold.Accept(GetString(IDS_PW_ATTRIBUTESCHANGE));

			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			if ( (mod->weightTableWindow.GetCurrentAttrib(xPos,yPos) == currentAttrib) &&
				 (mod->weightTableWindow.GetCurrentVert(xPos,yPos) == currentVert) )
				{
				int vert;
				vert = mod->weightTableWindow.GetCurrentVert(xPos,yPos);

				BOOL selected = FALSE;
				if ((vert >=0) && (vert < mod->weightTableWindow.vertexPtrList.Count()))
					selected = mod->weightTableWindow.vertexPtrList[vert].IsSelected();

				if ( (mod->weightTableWindow.GetJBMethod()) && (selected) )
					mod->weightTableWindow.ToggleGlobalAttribute(xPos,yPos);
				else mod->weightTableWindow.ToggleAttribute(xPos,yPos);

				mod->weightTableWindow.InvalidateViews();
				}
			ReleaseCapture();

			break;
			}


		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}	
	return 0;

	}




	//Windows proc to control the weight table custom control
static LRESULT CALLBACK WeightListProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	static BOOL mouseLDown = FALSE;
	static BOOL mouseRDown = FALSE;
	static BOOL mouseMDown = FALSE;
	static BOOL shiftDown = FALSE;

	static float initialValue = 0.0f;
	static int initialX = 0;
	static int initialY = 0;

	static int initialXR = 0;
	static int initialYR = 0;

	static int currentX = 0;
	static int currentY = 0;

	static int currentVert = 0;
	static int currentBone = 0;
	static BOOL draggin = FALSE;

	switch (msg) {
		case WM_CREATE:
			break;

		case 0x020A:
			{
			short delta = (short) HIWORD(wParam);
			if (delta>0)
				mod->weightTableWindow.ScrollRowUpOne();
			else mod->weightTableWindow.ScrollRowDownOne();
			//DebugPrint("WHeel\n");
			break;
			}

		case WM_SIZE:			
			{
			if (mod) mod->weightTableWindow.ResizeOffScreenBuffers();
			break;
			}

		case WM_PAINT:
			if (mod) 
				{
//				if (mod->weightTableWindow.useSpinners) 
//					return DefWindowProc(hWnd,msg,wParam,lParam);
//				else 
				mod->weightTableWindow.PaintWeightList();
				
				}
			break;


		case WM_LBUTTONDOWN:
			{
//check if in name list if so we are in select mode
			int flags = wParam;        // key flags 
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 

			BOOL alt = FALSE;
			BOOL ctrl = flags & MK_CONTROL;
			SHORT iret = GetAsyncKeyState (VK_MENU);
			shiftDown = FALSE;
			if (flags & MK_SHIFT)
				shiftDown = TRUE;
			if (iret==-32767)
				alt = TRUE;
	
			initialX = xPos;
			initialY = yPos;
	
			mouseLDown = TRUE;
			SetCapture(hWnd);
			draggin = FALSE;

			break;
			}
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
			{

			mouseLDown = FALSE;
			if (!draggin)
				{
				int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
				int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
				currentX = xPos;
				currentY = yPos;
				mod->weightTableWindow.BringDownEditField();
				if (!mod->weightTableWindow.ToggleExclusion(xPos,yPos))
					mod->weightTableWindow.BringUpEditField(xPos,yPos);
				}
			else
				{
				theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
				}
			ReleaseCapture();
			
			draggin = FALSE;
			break;
			}
		case WM_RBUTTONDOWN:
			{

			initialXR = (short)LOWORD(lParam);  // horizontal position of cursor 
			initialYR = (short)HIWORD(lParam);  // vertical position of cursor 


			break;
			}
		case WM_RBUTTONUP:
			{
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			int fwKeys = wParam;        // key flags 
			theHold.Begin();
			mod->weightTableWindow.HoldWeights();

			BOOL selected = FALSE;
			int tVert = mod->weightTableWindow.GetCurrentVert(initialXR,initialYR);
			if ((tVert >=0) && (tVert < mod->weightTableWindow.vertexPtrList.Count()))
				selected = mod->weightTableWindow.vertexPtrList[tVert].IsSelected();

			if ( (mod->weightTableWindow.GetJBMethod()) && (selected) )
				{
				if (fwKeys & MK_SHIFT)
					mod->weightTableWindow.DeleteAllWeights(xPos,yPos);
				else if (fwKeys & MK_CONTROL)
					mod->weightTableWindow.MaxAllWeights(xPos,yPos);
				else mod->weightTableWindow.ClearAllWeights(xPos,yPos);

				}
			else
				{			
				if (fwKeys & MK_SHIFT)
					mod->weightTableWindow.DeleteWeight(xPos,yPos);
				else if (fwKeys & MK_CONTROL)
					mod->weightTableWindow.MaxWeight(xPos,yPos);
				else mod->weightTableWindow.ClearWeight(xPos,yPos);
				}
			mouseRDown = FALSE;
			theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));


			break;
			}
//		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
			{
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			initialX = xPos;
			initialY = yPos;
			mouseMDown = TRUE;
			SetCapture(hWnd);

			break;
			}
//		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONUP:	
			{
			mouseMDown = FALSE;
			ReleaseCapture();
			break;
			}

		case WM_MOUSEMOVE:
			{
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
		 	int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			if (mouseMDown)
				{
				int offsety =0;
				int offsetx =0;

				if (mod->weightTableWindow.GetFlipFlopUI())
					{
					offsety = (initialY - yPos)/mod->weightTableWindow.textHeight;
					offsetx = (initialX - xPos)/mod->weightTableWindow.vertNameWidth;

					}
				else
					{
					offsety = (initialY - yPos)/mod->weightTableWindow.textHeight;
					offsetx = (initialX - xPos)/mod->weightTableWindow.buttonWidth;

					}

				if (offsetx < 0)
					{
					mod->weightTableWindow.ScrollColumnUpOne();
					initialX = xPos;
					}
				else if (offsetx > 0)
					{
					mod->weightTableWindow.ScrollColumnDownOne();
					initialX = xPos;
					}
				if (offsety < 0)
					{
					mod->weightTableWindow.ScrollRowUpOne();
					initialY = yPos;
					}
				else if (offsety > 0)
					{
					mod->weightTableWindow.ScrollRowDownOne();
			 		initialY = yPos;
					}

				UpdateWindow( hWnd);
				}
			else if (mouseLDown)  
				{
				int flags = wParam;        // key flags 
				BOOL alt = FALSE;
				BOOL ctrl = flags & MK_CONTROL;
				SHORT iret = GetAsyncKeyState (VK_MENU);
//				if (iret==-32767)
				if (iret)
					alt = TRUE;
				float speed = 1.0f;
				if (ctrl) speed = 10.0f;
				if (alt) speed = 0.1f;

				if (draggin)
					{
					int r = rand() % 10000;

					if (r < 100)
						mod->weightTableWindow.PrintStatus(STATUS_DRAGMODEALT);
					else if (r < 200)
						mod->weightTableWindow.PrintStatus(STATUS_DRAGMODECTRL);

					theHold.Restore();
					float offset;
					int amount = 0;
					if (mod->weightTableWindow.GetDragLeftMode())
						amount = (xPos - initialX)/3;
					else amount = (initialY- yPos)/3;

					offset = (float) amount * (mod->weightTableWindow.GetPrecision()*speed);
					BOOL selected = FALSE;
					if ((currentVert >=0) && (currentVert < mod->weightTableWindow.vertexPtrList.Count()))
						selected = mod->weightTableWindow.vertexPtrList[currentVert].IsSelected();

					if ( (mod->weightTableWindow.GetJBMethod()) && (selected) )
						{
						mod->weightTableWindow.AddOffsetToAll(initialX,initialY,offset);
						}
					else
						{
						offset = initialValue +(offset);
						if (offset < -1.0f) offset = -1.0f;
						if (offset > 1.0f) offset = 1.0f;

						if (mod->weightTableWindow.GetUpdateOnMouseUp())
							mod->weightTableWindow.SetWeight(currentVert,currentBone,offset,FALSE);
						else mod->weightTableWindow.SetWeight(currentVert,currentBone,offset,TRUE);
						}

					mod->weightTableWindow.InvalidateViews();
					UpdateWindow(hWnd);
					}
				if (draggin == FALSE)
					{
					float amount = 0.0f;
					if (mod->weightTableWindow.GetDragLeftMode())
						amount = (xPos - initialX)/3;
					else amount = (yPos - initialY)/3;
					if (fabs(amount) > 3)
						{
						draggin = TRUE;
						theHold.Begin();
						mod->weightTableWindow.HoldWeights();
						currentVert = mod->weightTableWindow.GetCurrentVert(initialX,initialY);
						currentBone = mod->weightTableWindow.GetCurrentBone(initialX,initialY);
						initialValue = mod->weightTableWindow.GetWeightFromCell(initialX, initialY);

						mod->weightTableWindow.StoreInitialWeights(initialX,initialY);

						int r = rand() % 200;

						if (r < 100)
							mod->weightTableWindow.PrintStatus(STATUS_DRAGMODEALT);
						else 
							mod->weightTableWindow.PrintStatus(STATUS_DRAGMODECTRL);

						}
					}

				}
			else
				{
				int r = rand() % 10000;
				int flags = wParam;        // key flags 

				BOOL ctrl = flags & MK_CONTROL;
				SHORT iret = GetAsyncKeyState (VK_MENU);
				BOOL shiftDown = FALSE;
				if (flags & MK_SHIFT)
					shiftDown = TRUE;

				if ((ctrl) && (mod->weightTableWindow.currentStatusID!=STATUS_RIGHTCLICKCTRL))
					mod->weightTableWindow.PrintStatus(STATUS_RIGHTCLICKCTRL);
				if ((shiftDown) && (mod->weightTableWindow.currentStatusID!=STATUS_RIGHTCLICKSHIFT))
					mod->weightTableWindow.PrintStatus(STATUS_RIGHTCLICKSHIFT);
				else if (r < 100) 
					mod->weightTableWindow.PrintStatus(STATUS_RANDOM);

				}

			break;
			}

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			{
			float v;
			if (mod->weightTableWindow.GetWeightFromEdit(v))
				{
				int bone,vert;
				bone = mod->weightTableWindow.GetCurrentBone(currentX,currentY);
				vert = mod->weightTableWindow.GetCurrentVert(currentX,currentY);

				BOOL selected = FALSE;
				if ((vert >=0) && (vert < mod->weightTableWindow.vertexPtrList.Count()))
					selected = mod->weightTableWindow.vertexPtrList[vert].IsSelected();

				if ( (mod->weightTableWindow.GetJBMethod()) && (selected) )
					{
					
					theHold.Begin();
					mod->weightTableWindow.HoldWeights();
					
					mod->weightTableWindow.SetAllWeights( bone, v, TRUE);
					mod->weightTableWindow.InvalidateViews();
					theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));

					}
				else
					{
					theHold.Begin();
					mod->weightTableWindow.HoldWeights();
				
					mod->weightTableWindow.SetWeight(vert,bone,v);
					theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
					}
				}
			break;
			}
		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}	
	return 0;
	}


	//Windows proc to control the weight table custom control
static LRESULT CALLBACK WeightListGlobalProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	static BOOL mouseLDown = FALSE;
	static BOOL mouseRDown = FALSE;
	static BOOL shiftDown = FALSE;

	static float initialValue = 0.0f;
	static int initialX = 0;
	static int initialY = 0;
	static int currentBone = 0;
	static int currentVert = 0;

	static BOOL draggin = FALSE;

	switch (msg) {
		case WM_CREATE:
			break;

		case 0x020A:
			DebugPrint("WHeel\n");
			break;

		case WM_SIZE:			
			{
			if (mod) mod->weightTableWindow.ResizeWindowControls();
			break;
			}

		case WM_PAINT:
			if (mod) mod->weightTableWindow.PaintWeightListGlobal();
			break;


		case WM_LBUTTONDOWN:
			{
//check if in name list if so we are in select mode
			int flags = wParam;        // key flags 
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 

			BOOL alt = FALSE;
			BOOL ctrl = flags & MK_CONTROL;
			SHORT iret = GetAsyncKeyState (VK_MENU);
			shiftDown = FALSE;
			if (flags & MK_SHIFT)
				shiftDown = TRUE;
			if (iret==-32767)
				alt = TRUE;
			initialX = xPos;
			initialY = yPos;
			mouseLDown = TRUE;
			SetCapture(hWnd);
/*
			if (shiftDown)
				{
				theHold.Begin();
				mod->weightTableWindow.HoldWeights();
				mod->weightTableWindow.StoreInitialWeights(initialX,initialY);
				currentVert = mod->weightTableWindow.GetCurrentVert(xPos,yPos);
				currentBone = mod->weightTableWindow.GetCurrentBone(xPos,yPos);
				}
*/

			break;
			}
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
			{
			mouseLDown = FALSE;
			if (!draggin)
				{
				int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
				int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
				mod->weightTableWindow.BringDownEditField();
				mod->weightTableWindow.BringUpGlobalEditField(xPos,yPos);
				}
			else
				{
				theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
				}
			ReleaseCapture();
			draggin =FALSE;
			break;
			}
		case WM_RBUTTONDOWN:
			{



			break;
			}
		case WM_RBUTTONUP:
			{

			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
			int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			int fwKeys = wParam;        // key flags 
			theHold.Begin();
			mod->weightTableWindow.HoldWeights();
			
			if (fwKeys & MK_SHIFT)
				mod->weightTableWindow.DeleteAllWeights(xPos,yPos);
			else if (fwKeys & MK_CONTROL)
				mod->weightTableWindow.MaxAllWeights(xPos,yPos);
			else mod->weightTableWindow.ClearAllWeights(xPos,yPos);
			mouseRDown = FALSE;
			theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));

			break;
			}

		case WM_MOUSEMOVE:
			{
			int xPos = (short)LOWORD(lParam);  // horizontal position of cursor 
		 	int yPos = (short)HIWORD(lParam);  // vertical position of cursor 
			if (mouseLDown)  
				{
				if (draggin)
					{
					theHold.Restore();
					float offset;

					int flags = wParam;        // key flags 
					BOOL alt = FALSE;
					BOOL ctrl = flags & MK_CONTROL;
					SHORT iret = GetAsyncKeyState (VK_MENU);
					if (iret)
						alt = TRUE;

					float speed = 1.0f;
					if (ctrl) speed = 10.0f;
					if (alt) speed = 0.1f;

					if (mod->weightTableWindow.GetDragLeftMode())
						offset = ((float) xPos - (float) initialX) * (mod->weightTableWindow.GetPrecision()*speed);
					else offset = ((float)initialY - (float)  yPos) * (mod->weightTableWindow.GetPrecision()*speed);
					offset = initialValue +offset;
					if (offset < -1.0f) offset = -1.0f;
					if (offset > 1.0f) offset = 1.0f;
					mod->weightTableWindow.AddOffsetToAll(initialX,initialY,offset);
					mod->weightTableWindow.InvalidateViews();
					UpdateWindow(mod->weightTableWindow.hWeightList);
 					}
				if (draggin == FALSE)
					{
					float offset = 0;
					if (mod->weightTableWindow.GetDragLeftMode())
						offset = fabs(xPos-initialX);
					else offset = fabs(yPos-initialY);
					if ( offset > 3.0f)
						{
						draggin = TRUE;
						theHold.Begin();
						mod->weightTableWindow.HoldWeights();
						mod->weightTableWindow.StoreInitialWeights(initialX,initialY);
						currentVert = mod->weightTableWindow.GetCurrentVert(initialX,initialY);
						currentBone = mod->weightTableWindow.GetCurrentBone(initialX,initialY);						
						}
					}

				}

			break;
			}
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			{

			float v;

			if (mod->weightTableWindow.GetGlobalWeightFromEdit(v))
				{
				int bone;
				theHold.Begin();
				mod->weightTableWindow.HoldWeights();
				bone = mod->weightTableWindow.GetCurrentBone(initialX,initialY);
				mod->weightTableWindow.SetAllWeights( bone, v, TRUE);
				mod->weightTableWindow.InvalidateViews();
				theHold.Accept(GetString(IDS_PW_WEIGHTCHANGE));
				}

			break;

			}

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}	
	return 0;
	}

	//Windows proc to control the weight table custom control
static LRESULT CALLBACK WeightListLabelProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	static int currentVert = 0;
	static int currentBone = 0;

	static int lastX = 0;
	static int initialX = 0;
	static int initialSize = 0;
	static BOOL resize = FALSE;


	switch (msg) {
		case WM_CREATE:
			break;

		case 0x020A:
			DebugPrint("WHeel\n");
			break;

		case WM_SIZE:			
			{
			if (mod) mod->weightTableWindow.ResizeOffScreenBuffers();
			break;
			}

		case WM_PAINT:
			if (mod) mod->weightTableWindow.PaintWeightListLabel();
			break;

		case WM_LBUTTONDOWN:
			{
//check if in name list if so we are in select mode
			int flags = wParam;        // key flags 
			int x = (short)LOWORD(lParam);  // horizontal position of cursor 
			int y = (short)HIWORD(lParam);  // vertical position of cursor 
			if (!mod->weightTableWindow.GetFlipFlopUI())
				{
				WINDOWPLACEMENT winPos;
				GetWindowPlacement(hWnd , &winPos);

//check if inside the move rect for the vert name
				int x = (short)LOWORD(lParam);  // horizontal position of cursor 
				int y = (short)HIWORD(lParam);  // vertical position of cursor 
				initialX = x;
				lastX = x;
				if ((x%mod->weightTableWindow.buttonWidth)>=(mod->weightTableWindow.buttonWidth-3)) 
					{
					resize = TRUE;
					initialSize = mod->weightTableWindow.buttonWidth;
					}
				}
			currentVert = mod->weightTableWindow.GetCurrentVert(x,y);
			currentBone = mod->weightTableWindow.GetCurrentBone(x,y);
			SetCapture(hWnd);
			break;
			}
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
			{
			int x = (short)LOWORD(lParam);  // horizontal position of cursor 
			int y = (short)HIWORD(lParam);  // vertical position of cursor 

			int newVert = mod->weightTableWindow.GetCurrentVert(x,y);
			int newBone = mod->weightTableWindow.GetCurrentBone(x,y);
			if ((newVert == currentVert) && (newBone == currentBone) && (!resize))
				{
				mod->weightTableWindow.SelectBone(x, y);
				mod->weightTableWindow.UpdateWindowControls();
				mod->weightTableWindow.UpdateSpinner();



				mod->weightTableWindow.InvalidateViews();
				}

			resize = FALSE;
			ReleaseCapture();
			break;
			}

		case WM_MOUSEMOVE:	
			{
			if (!mod->weightTableWindow.GetFlipFlopUI())
				{
				int x = (short)LOWORD(lParam);  // horizontal position of cursor
				if (resize)
					{
				 
					if (x != lastX)
						{
						lastX = x;
						int offset = x - initialX ;
						mod->weightTableWindow.buttonWidth = initialSize + offset;
						if (mod->weightTableWindow.buttonWidth < 50)
							mod->weightTableWindow.buttonWidth = 50;

						mod->weightTableWindow.ResizeWindowControls();
						mod->weightTableWindow.InvalidateViews();
						}
					}
				else
					{
					WINDOWPLACEMENT winPos;
					GetWindowPlacement(hWnd , &winPos);
					if ((x%mod->weightTableWindow.buttonWidth)>=(mod->weightTableWindow.buttonWidth-3)) 
						{
						SetCursor(LoadCursor(NULL,IDC_SIZEWE));
						}
					else SetCursor(LoadCursor(NULL,IDC_ARROW));
					}
				}
			break;
			}

		default:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		}	
	return 0;
	}


//Windows proc to control  the Weight Table Window
INT_PTR CALLBACK WeightTableDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

	BonesDefMod *mod = (BonesDefMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
/*
	static int initialSize = 0;
	static int initialX = 0;
	static int lastX = 0;
	static BOOL resizeName = FALSE;
	static BOOL resizeBone = FALSE;
*/
	switch (msg) {

/*
		case WM_LBUTTONDOWN:	
			{
//check if inside the move rect for the vert name
			int x = (short)LOWORD(lParam);  // horizontal position of cursor 
			int y = (short)HIWORD(lParam);  // vertical position of cursor 
			initialX = x;
			lastX = x;
			if (mod->weightTableWindow.flipFlopUI)
				{

				}
			else
				{
				Rect r;
				r.left = mod->weightTableWindow.vertNameWidth+mod->weightTableWindow.leftBorder;
				r.right = mod->weightTableWindow.vertNameWidth+mod->weightTableWindow.leftBorder+2;
//check name
				if ((y>= mod->weightTableWindow.topBorder) && (y<=(mod->weightTableWindow.topBorder+(mod->weightTableWindow.numberOfRows*mod->weightTableWindow.textHeight))) )
					{
					if ((x>=r.left) && (x<=r.right))
						{
						resizeName = TRUE;
						initialSize = mod->weightTableWindow.vertNameWidth;
						SetCapture(hWnd);
						}
					else
						//check if in bone
						{
						
						WINDOWPLACEMENT winPos;
						GetWindowPlacement(mod->weightTableWindow.hWeightList , &winPos);
						x = x - winPos.rcNormalPosition.left;
						int mod1;
						mod1 = x % mod->weightTableWindow.buttonWidth;
						
						if ((mod1 >=0) && (mod1<=2))
							{
							resizeBone = TRUE;
							initialSize = mod->weightTableWindow.buttonWidth;
							SetCapture(hWnd);
							}
						}
					}
				}
			break;
			}
		case WM_LBUTTONUP:	
			{
			if ((resizeName) || (resizeBone))
				ReleaseCapture();
			resizeName = FALSE;
			resizeBone = FALSE;				
			break;
			}
		case WM_MOUSEMOVE:	
			{
			if (resizeName)
				{
				int x = (short)LOWORD(lParam);  // horizontal position of cursor 
				if (x != lastX)
					{
					lastX = x;
					int offset = x - initialX ;
					mod->weightTableWindow.vertNameWidth = initialSize + offset;
					if (mod->weightTableWindow.vertNameWidth < 70)
						mod->weightTableWindow.vertNameWidth = 70;
					mod->weightTableWindow.ResizeWindowControls();
					mod->weightTableWindow.InvalidateViews();
					}
				}
			else if (resizeBone)
				{
				int x = (short)LOWORD(lParam);  // horizontal position of cursor 
				if (x != lastX)
					{
					lastX = x;
					int offset = x - initialX ;
					mod->weightTableWindow.buttonWidth = initialSize + offset;
					if (mod->weightTableWindow.buttonWidth < 70)
						mod->weightTableWindow.buttonWidth = 70;
					mod->weightTableWindow.ResizeWindowControls();
					mod->weightTableWindow.InvalidateViews();
					}
				}
			break;
			}
*/
		case WM_INITDIALOG:
			{
			//initialized custom control GLP_USERDATE to point to the owning skin mod for each custom window
			mod = (BonesDefMod*)lParam;
			mod->weightTableWindow.InitMod(mod);

			SendMessage(hWnd, WM_SETICON, ICON_SMALL, GetClassLongPtr(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM)); // mjm - 3.12.99

			mod->weightTableWindow.hWeightList = GetDlgItem(hWnd,IDC_WEIGHTLIST);
			SetWindowLongPtr(mod->weightTableWindow.hWeightList,GWLP_USERDATA,(LONG_PTR)mod);
			mod->weightTableWindow.hNameList = GetDlgItem(hWnd,IDC_NAMELIST);
			SetWindowLongPtr(mod->weightTableWindow.hNameList,GWLP_USERDATA,(LONG_PTR)mod);
			mod->weightTableWindow.hAttribList = GetDlgItem(hWnd,IDC_ATTRIBLIST);
			SetWindowLongPtr(mod->weightTableWindow.hAttribList,GWLP_USERDATA,(LONG_PTR)mod);


			mod->weightTableWindow.hWeightListGlobal = GetDlgItem(hWnd,IDC_WEIGHTLISTGLOBAL);
			SetWindowLongPtr(mod->weightTableWindow.hWeightListGlobal,GWLP_USERDATA,(LONG_PTR)mod);
																	 
			mod->weightTableWindow.hAttribListGlobal = GetDlgItem(hWnd,IDC_ATTRIBLISTGLOBAL);
			SetWindowLongPtr(mod->weightTableWindow.hAttribListGlobal,GWLP_USERDATA,(LONG_PTR)mod);

			mod->weightTableWindow.hWeightListLabel = GetDlgItem(hWnd,IDC_WEIGHTLISTLABEL);
			SetWindowLongPtr(mod->weightTableWindow.hWeightListLabel,GWLP_USERDATA,(LONG_PTR)mod);
			mod->weightTableWindow.hNameListLabel = GetDlgItem(hWnd,IDC_NAMELISTLABEL);
			SetWindowLongPtr(mod->weightTableWindow.hNameListLabel,GWLP_USERDATA,(LONG_PTR)mod);
			mod->weightTableWindow.hAttribListLabel = GetDlgItem(hWnd,IDC_ATTRIBLISTLABEL);
			SetWindowLongPtr(mod->weightTableWindow.hAttribListLabel,GWLP_USERDATA,(LONG_PTR)mod);


			//initialize offscreen buffers
			mod->weightTableWindow.BuildOffScreenBuffers();



			mod->weightTableWindow.hWnd = hWnd;

			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);


			GetCOREInterface()->RegisterDlgWnd(hWnd);


			TSTR windowsName;
			windowsName.printf("%s",mod->weightTableWindow.GetName());
			SetWindowText(hWnd, windowsName  );


			mod->weightTableWindow.UpdateUI();
			
			//fill out all our data
			mod->weightTableWindow.BuildModDataList();
			mod->weightTableWindow.InitWindowControl();
			mod->weightTableWindow.CreateGDIData();

			mod->weightTableWindow.SetFontSize();
			

			if (!mod->weightTableWindow.isDocked)
				mod->weightTableWindow.SetWindowState();
			mod->weightTableWindow.UpdatePasteButton();
			mod->weightTableWindow.UpdateDeleteButton();

			mod->weightTableWindow.PrintStatus(STATUS_RANDOM);
			break;
			}
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_INACTIVE) 
				mod->weightTableWindow.DeActivateActionTable();			
			else mod->weightTableWindow.ActivateActionTable();			

			break;


		case WM_DESTROY:
			if (!mod->weightTableWindow.isDocked)
				mod->weightTableWindow.SaveWindowState();
//			mod->weightTableWindow.DeActivateActionTable();
			mod->weightTableWindow.hWnd = NULL;
			mod->weightTableWindow.FreeTempData();
			mod->hWeightTable = NULL;

			mod->weightTableWindow.toolTipHwnd = NULL;

			GetCOREInterface()->UnRegisterDlgWnd(hWnd);
			EndDialog(hWnd, 0);
			break;
		case WM_CLOSE:

			DestroyWindow(hWnd);
			break;


		case WM_SIZE:
			if (mod) mod->weightTableWindow.ResizeWindowControls();
			break;

		case WM_CUSTEDIT_ENTER:
			{
			ICustEdit *custEdit = GetICustEdit((HWND)lParam );
			if (custEdit)
				{
				if ((wParam) == IDC_FONTSIZE2)
					{
					ISpinnerControl *spin = (ISpinnerControl *) lParam ;
					mod->weightTableWindow.SetFontSize(custEdit->GetInt());
					if (hWnd) 
						{
						InvalidateRect(hWnd,NULL,TRUE);
						}
					mod->weightTableWindow.ResetFont();
					mod->weightTableWindow.SetFontSize();
					mod->weightTableWindow.BringDownEditField();
					mod->weightTableWindow.ResizeWindowControls();
					}
				else if (LOWORD(wParam) == IDC_PRECISION2)
					mod->weightTableWindow.SetPrecision(custEdit->GetFloat());
					
				ReleaseICustEdit(custEdit);
				}
			break;
			}
		case CC_SPINNER_BUTTONUP:
			{
			if (HIWORD(wParam))
				{
				ISpinnerControl *spin = (ISpinnerControl *) lParam ;
				if (LOWORD(wParam) == IDC_FONTSIZE_SPIN2)
					{
					mod->weightTableWindow.SetFontSize(spin->GetIVal());
					if (hWnd) 
						{
						InvalidateRect(hWnd,NULL,TRUE);
						}
					mod->weightTableWindow.ResetFont();
					mod->weightTableWindow.SetFontSize();
					mod->weightTableWindow.BringDownEditField();
					mod->weightTableWindow.ResizeWindowControls();
					}
				else if (LOWORD(wParam) == IDC_PRECISION_SPIN2)
					mod->weightTableWindow.SetPrecision(spin->GetFVal());
				}

			break;
			}

		case 0x020A:
			{
			short delta = (short) HIWORD(wParam);
			if (delta>0)
				mod->weightTableWindow.ScrollRowUpOne();
			else mod->weightTableWindow.ScrollRowDownOne();
			//DebugPrint("WHeel\n");
			break;
			}

		case WM_VSCROLL:
			{
			int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
			short int nPos = (short int) HIWORD(wParam);   // scroll box position 
			HWND hwndScrollBar = (HWND) lParam;       
			if ( (nScrollCode == SB_THUMBTRACK) || (nScrollCode == SB_THUMBPOSITION))
				{
				if (mod)
					mod->weightTableWindow.firstRow = nPos;

				}	
			else if (nScrollCode == SB_LINELEFT) 
				{
				if (mod)
					mod->weightTableWindow.firstRow--;
				}	
			else if (nScrollCode == SB_LINERIGHT) 
				{
				if (mod)
					mod->weightTableWindow.firstRow++;
				}	
			else if (nScrollCode == SB_PAGELEFT) 
				{
				if (mod)
					{
					mod->weightTableWindow.firstRow-=mod->weightTableWindow.numberOfRows;
					}
				}	
			else if (nScrollCode == SB_PAGERIGHT) 
				{
				if (mod)
					mod->weightTableWindow.firstRow+=mod->weightTableWindow.numberOfRows;
				
				}
			if (mod)
				{
				if (mod->weightTableWindow.firstRow<0) mod->weightTableWindow.firstRow = 0;
				int lastRow;
				if (mod->weightTableWindow.GetFlipFlopUI())
					lastRow = mod->weightTableWindow.activeBones.Count() - mod->weightTableWindow.numberOfRows;
				else lastRow = mod->weightTableWindow.vertexPtrList.Count() - mod->weightTableWindow.numberOfRows;
				if (mod->weightTableWindow.firstRow>lastRow) mod->weightTableWindow.firstRow = lastRow ;
				nPos = mod->weightTableWindow.firstRow;
				SetScrollPos( hwndScrollBar,SB_CTL, nPos, TRUE);
				mod->weightTableWindow.BringDownEditField();
				mod->weightTableWindow.InvalidateViews();
				}

			break;
			}

		case WM_HSCROLL:
			{
			int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
			short int nPos = (short int) HIWORD(wParam);   // scroll box position 
			HWND hwndScrollBar = (HWND) lParam;       
			if ( (nScrollCode == SB_THUMBTRACK) || (nScrollCode == SB_THUMBPOSITION))
				{
				if (mod)
					mod->weightTableWindow.firstColumn = nPos;

				}	
			else if (nScrollCode == SB_LINELEFT) 
				{
				if (mod)
					mod->weightTableWindow.firstColumn--;
				}	
			else if (nScrollCode == SB_LINERIGHT) 
				{
				if (mod)
					mod->weightTableWindow.firstColumn++;
				}	
			else if (nScrollCode == SB_PAGELEFT) 
				{
				if (mod)
					mod->weightTableWindow.firstColumn-=mod->weightTableWindow.numberOfColumns;
				}	
			else if (nScrollCode == SB_PAGERIGHT) 
				{
				if (mod)
					mod->weightTableWindow.firstColumn+=mod->weightTableWindow.numberOfColumns;
				
				}
			if (mod)
				{
				if (mod->weightTableWindow.firstColumn<0) mod->weightTableWindow.firstColumn = 0;
				int lastColumn;
				if (mod->weightTableWindow.GetFlipFlopUI())
					lastColumn =  mod->weightTableWindow.vertexPtrList.Count()- mod->weightTableWindow.numberOfColumns;
				else lastColumn = mod->weightTableWindow.activeBones.Count() - mod->weightTableWindow.numberOfColumns;

				if (mod->weightTableWindow.firstColumn>lastColumn) mod->weightTableWindow.firstColumn = lastColumn;
				mod->weightTableWindow.ResizeWindowControls();
				nPos = mod->weightTableWindow.firstColumn;
				SetScrollPos( hwndScrollBar,SB_CTL, nPos, TRUE);
				mod->weightTableWindow.BringDownEditField();
				mod->weightTableWindow.InvalidateViews();
				}

			
			break;
			}


		case WM_NCRBUTTONDOWN:
			{
			if (mod->weightTableWindow.GetShowMenu())
				{

				POINT pt;
				pt.x = (short)LOWORD(lParam);
				pt.y = 5;
				if (mod->weightTableWindow.isDocked)
					GetCOREInterface()->PutUpViewMenu(hWnd, pt);

				}
			break;
			}
		case WM_RBUTTONDOWN:
			{
			POINT pt;
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);

			if (mod->weightTableWindow.isDocked)
				GetCOREInterface()->PutUpViewMenu(hWnd, pt);
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_NAMELISTGLOBAL_DROP:
					{
					if (HIWORD(wParam)==CBN_SELCHANGE) 
						{
						if (SendMessage(GetDlgItem(hWnd,IDC_NAMELISTGLOBAL_DROP),CB_GETCURSEL,0,0) != mod->weightTableWindow.GetAffectSelectedOnly())
							mod->weightTableWindow.WtExecute(LOWORD(wParam));
//							mod->weightTableWindow.SetAffectSelectedOnly(FALSE);	
//						else mod->weightTableWindow.SetAffectSelectedOnly(TRUE);	
						}
					break;
					}
					
				case IDC_SETS:
					{
					if (HIWORD(wParam)==CBN_SELCHANGE) 
						{
						mod->weightTableWindow.SetActiveSet(SendMessage(GetDlgItem(hWnd,IDC_SETS),CB_GETCURSEL,0,0));	
						}
					break;
					}

				case IDC_CREATE:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//			DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_WEIGHTTABLE_CUSTOMLISTNAME),hWnd,
//				AddCustomListDlgProc, (LPARAM)&mod->weightTableWindow);
					break;
					}
				case IDC_DELETE:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//					if (mod->weightTableWindow.activeSet >2)
//						mod->weightTableWindow.DeleteCustomList(mod->weightTableWindow.activeSet);
					break;
					}
				case IDC_COPY:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//			mod->weightTableWindow.SetCopyBuffer();
//			mod->weightTableWindow.InvalidateViews();
					break;
					}

				case IDC_PASTE:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));					
//					mod->weightTableWindow.PasteCopyBuffer();
//					mod->weightTableWindow.InvalidateViews();
					break;
					}

//Option check boxes
				case IDC_AFFECTEDBONES_CHECK:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//					mod->weightTableWindow.SetAffectedBonesOnly(IsDlgButtonChecked(hWnd,IDC_AFFECTEDBONES_CHECK));
//					mod->weightTableWindow.ResizeWindowControls();
					break;
					}
				case IDC_UPDATEONMOUSEUP_CHECK2:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//					mod->weightTableWindow.SetUpdateOnMouseUp(IsDlgButtonChecked(hWnd,IDC_AFFECTEDBONES_CHECK));
					break;
					}
				case IDC_FLIPFLOPUI_CHECK2:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//					mod->weightTableWindow.SetFlipFlopUI(IsDlgButtonChecked(hWnd,IDC_FLIPFLOPUI_CHECK2));
//					mod->weightTableWindow.ResetScrollBars();
//					mod->weightTableWindow.BringDownEditField();
//					mod->weightTableWindow.ResizeWindowControls();
					break;
					}	
				case IDC_ATTRIBUTE_CHECK2:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//			mod->weightTableWindow.SetShowAttributes(IsDlgButtonChecked(hWnd,IDC_ATTRIBUTE_CHECK2));
//			mod->weightTableWindow.BringDownEditField();
//			mod->weightTableWindow.ResizeWindowControls();
					break;
					}					
				case IDC_GLOBAL_CHECK2:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//					mod->weightTableWindow.SetShowGlobals(IsDlgButtonChecked(hWnd,IDC_GLOBAL_CHECK2));
//					mod->weightTableWindow.BringDownEditField();
//					mod->weightTableWindow.ResizeWindowControls();
					break;
					}	
				case IDC_REDUCELABELS_CHECK2:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//					mod->weightTableWindow.SetReduceLabels(IsDlgButtonChecked(hWnd,IDC_REDUCELABELS_CHECK2));
//					mod->weightTableWindow.BringDownEditField();
//					mod->weightTableWindow.ResizeWindowControls();
					break;
					}	
				case IDC_SHOWEXCLUSION_CHECK:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//					mod->weightTableWindow.SetShowExclusion(IsDlgButtonChecked(hWnd,IDC_SHOWEXCLUSION_CHECK));
//					mod->weightTableWindow.BringDownEditField();
//					mod->weightTableWindow.ResizeWindowControls();
					break;
					}
				case IDC_SHOWLOCK_CHECK:
					{
					mod->weightTableWindow.WtExecute(LOWORD(wParam));
//					mod->weightTableWindow.SetShowLock(IsDlgButtonChecked(hWnd,IDC_SHOWLOCK_CHECK));
//					mod->weightTableWindow.BringDownEditField();
//					mod->weightTableWindow.ResizeWindowControls();
					break;
					}
				case IDOK:
				case IDCANCEL:
					{
					break;
					}

				default:
					IMenuBarContext* pContext = (IMenuBarContext*) GetCOREInterface()->GetMenuManager()->GetContext(mod->weightTableWindow.kWeightTableMenuBar);
					if (pContext)
						{
						int id = LOWORD(wParam);
						int hid = HIWORD(wParam);
#ifdef DEBUGMODE 
						if (mod->GetDebugMode()) ScriptPrint("Executing Action %d lid %d\n",hid, id); 
#endif
						if (hid == 0)
							pContext->ExecuteAction(id);

#ifdef DEBUGMODE 
						if (mod->GetDebugMode()) ScriptPrint("Done Execution\n"); 
#endif

						}
					return TRUE;
			}
			break;


		default:
			return FALSE;
		}
	return TRUE;
	}




INT_PTR CALLBACK AddCustomListDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
	{
	static WeightTableWindow *mod ;


	switch (msg) 
		{
	case WM_INITDIALOG:
		{
		mod = (WeightTableWindow*)lParam;

		ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_EDIT));
		TSTR name;

		name.printf("%s",GetString(IDS_PW_CUSTOMLIST));

		iName->SetText( name);
		ReleaseICustEdit(iName);

	//	CenterWindow(hWnd,GetParent(hWnd));
		break;
		}
		


	case WM_COMMAND:
		switch (LOWORD(wParam)) 
			{
			case IDOK:
				{
				ICustEdit *iName = GetICustEdit(GetDlgItem(hWnd,IDC_EDIT));
				TSTR name;



				TCHAR tname[200];
				iName->GetText(tname,200);
				ReleaseICustEdit(iName);
				name.printf("%s",tname);
				mod->AddCustomList(name);
				EndDialog(hWnd,1);
				break;
				}
			case IDCANCEL:
				EndDialog(hWnd,0);
				break;
			}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}



//This registers our 2 custom windows the weight and attrib list 
void BonesDefMod::RegisterClasses2()
	{
	static BOOL registered = FALSE;
	if (!registered) {
		registered = TRUE;
		WNDCLASS  wc;



		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL; 
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = WeightListProc;
		wc.lpszClassName = _T("weightList");
		RegisterClass(&wc);

		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = WeightListGlobalProc;
		wc.lpszClassName = _T("weightListGlobal");
		RegisterClass(&wc);

		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL; 
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = WeightListLabelProc;
		wc.lpszClassName = _T("weightListLabel");
		RegisterClass(&wc);


		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = NameListProc;
		wc.lpszClassName = _T("nameList");
		RegisterClass(&wc);


		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = NameListLabelProc;
		wc.lpszClassName = _T("nameListLabel");
		RegisterClass(&wc);


		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = AttribListProc;
		wc.lpszClassName = _T("attribList");
		RegisterClass(&wc);

		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = AttribListGlobalProc;
		wc.lpszClassName = _T("attribListGlobal");
		RegisterClass(&wc);

		wc.style         = 0;
		wc.hInstance     = hInstance;
		wc.hIcon         = NULL;
		wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpfnWndProc   = AttribListLabelProc;
		wc.lpszClassName = _T("attribListLabel");
		RegisterClass(&wc);




		}
	}


void WeightTableWindow::UpdateUI()
	{
	HWND hGlobalDrop = GetDlgItem(hWnd,IDC_NAMELISTGLOBAL_DROP);
	TSTR name;

	SendMessage(hGlobalDrop, CB_RESETCONTENT, 0, 0);	

	
	name.printf("%s",GetString(IDS_PW_AFFECTALLVERTS2));
	SendMessage(hGlobalDrop, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) name.data());			
	
	name.printf("%s",GetString(IDS_PW_AFFECTSEL));
	SendMessage(hGlobalDrop, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR) name.data());
	SendMessage(hGlobalDrop, CB_SETCURSEL , (WPARAM) GetAffectSelectedOnly(), 0);			
	if (GetAffectedBonesOnly())
		CheckDlgButton(hWnd,IDC_AFFECTEDBONES_CHECK,BST_CHECKED);
	else CheckDlgButton(hWnd,IDC_AFFECTEDBONES_CHECK,BST_UNCHECKED);

	
	HWND hSets = GetDlgItem(hWnd,IDC_SETS);
	SendMessage(hSets, CB_SETCURSEL , (WPARAM) GetActiveSet(), 0);	

	if (GetUpdateOnMouseUp())
		CheckDlgButton(hWnd,IDC_UPDATEONMOUSEUP_CHECK2,BST_CHECKED);
	else CheckDlgButton(hWnd,IDC_UPDATEONMOUSEUP_CHECK2,BST_UNCHECKED);

	if (GetFlipFlopUI())
		CheckDlgButton(hWnd,IDC_FLIPFLOPUI_CHECK2,BST_CHECKED);
	else CheckDlgButton(hWnd,IDC_FLIPFLOPUI_CHECK2,BST_UNCHECKED);

	if (GetShowAttributes())
		CheckDlgButton(hWnd,IDC_ATTRIBUTE_CHECK2,BST_CHECKED);
	else CheckDlgButton(hWnd,IDC_ATTRIBUTE_CHECK2,BST_UNCHECKED);

	if (GetShowGlobals())
		CheckDlgButton(hWnd,IDC_GLOBAL_CHECK2,BST_CHECKED);
	else CheckDlgButton(hWnd,IDC_GLOBAL_CHECK2,BST_UNCHECKED);

	if (GetReduceLabels())
		CheckDlgButton(hWnd,IDC_REDUCELABELS_CHECK2,BST_CHECKED);
	else CheckDlgButton(hWnd,IDC_REDUCELABELS_CHECK2,BST_UNCHECKED);



	if (GetShowExclusion())
		CheckDlgButton(hWnd,IDC_SHOWEXCLUSION_CHECK,BST_CHECKED);
	else CheckDlgButton(hWnd,IDC_SHOWEXCLUSION_CHECK,BST_UNCHECKED);

	if (GetShowLock())
		CheckDlgButton(hWnd,IDC_SHOWLOCK_CHECK,BST_CHECKED);
	else CheckDlgButton(hWnd,IDC_SHOWLOCK_CHECK,BST_UNCHECKED);

//setup name size spinner
	ISpinnerControl* spin = NULL;

//setup precision spinner
	float fval = GetPrecision();
	spin = SetupFloatSpinner(hWnd, IDC_PRECISION_SPIN2, IDC_PRECISION2, 0.0f, 5.0f, fval, .1f);
	ReleaseISpinner(spin);

//setup font spinner
	int ival = GetFontSize();
	spin = SetupIntSpinner(hWnd, IDC_FONTSIZE_SPIN2, IDC_FONTSIZE2, 8, 24, ival);
	ReleaseISpinner(spin);

	}

void WeightTableWindow::PrintStatus(int statusID)
	{
	if (statusID == STATUS_RANDOM)
		statusID = (rand() % 10)+1;
	if ((statusID == STATUS_DRAGMODELR) && (!mod->weightTableWindow.GetDragLeftMode()))
		statusID =STATUS_DRAGMODETB;
	if ((statusID == STATUS_DRAGMODETB) && (mod->weightTableWindow.GetDragLeftMode()))
		statusID =STATUS_DRAGMODELR;

	TSTR name(GetString(statusIDs[statusID-1]));

	HWND hStatus = GetDlgItem(hWnd,IDC_STATUS);

	currentStatusID = statusID;
	for (int i = name.Length()-1; i >= 0; i--)
		{
		TCHAR *t = &name[i];
		SetWindowText(  hStatus , (LPCTSTR) t);
 		}

	
/*
#define		STATUS_RANDOM				0
#define		STATUS_DRAGMODE				1
#define		STATUS_LEFTCLICK			2
#define		STATUS_RIGHTCLICK			3
#define		STATUS_RIGHTCLICKALT		4
#define		STATUS_RIGHTCLICKCTRL		5
#define		STATUS_MIDDLEMOUSEBUTTON	6
#define		STATUS_MIDDLEMOUSESCROLL	7
*/
	}