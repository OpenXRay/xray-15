
#include "painterInterface.h"
#include "IpainterInterface.h"


HCTX static NEAR TabletInit(HWND hWnd, PainterInterface *mod)
{
	LOGCONTEXT lcMine;

	if(mod->hWinTabDLL == NULL) return NULL;

	UINT p,rp; 
	rp = mod->PaintWTInfo(WTI_DEVICES, DVC_NPRESSURE, &p );


	/* get default region */
	mod->PaintWTInfo(WTI_DEFCONTEXT, 0, &lcMine);

	/* modify the digitizing region */
	wsprintf(lcMine.lcName, "PrsTest Digitizing %x", hInstance);
	lcMine.lcOptions |= CXO_MESSAGES|CXO_SYSTEM;
	lcMine.lcPktData = PACKETDATA;
	lcMine.lcPktMode = PACKETMODE;
	lcMine.lcMoveMask = PACKETDATA;
	lcMine.lcBtnUpMask = lcMine.lcBtnDnMask;

	/* output in 10000 x 10000 grid */
	lcMine.lcOutOrgX = lcMine.lcOutOrgY = 0;
	lcMine.lcOutExtX = 10000;
	lcMine.lcOutExtY = 10000;

	/* open the region */
	if (mod->hWinTabDLL)
		return mod->PaintWTOpen(hWnd, &lcMine, TRUE);
	else return NULL;

}

static HCTX hTab = NULL;

static INT_PTR CALLBACK TabletFloaterDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

	
	PACKET pkt;
	static UINT prsOld, prsNew;

	PainterInterface *painter = (PainterInterface*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	switch (msg) {
		case WM_INITDIALOG: {
			painter = (PainterInterface*)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);

			
//			hTab = TabletInit(hWnd,painter);

			break;
			}

		case WT_PACKET:
			if (painter->GetPressureEnable())
				{
				if ((painter->hWinTabDLL) && (painter->PaintWTPacket((HCTX)lParam, wParam, &pkt)) )
					{
/*
//					if (painter->inPaint)
						{
						if (HIWORD(pkt.pkButtons)==TBN_DOWN) 
							{
//							mod->strokeState = STARTSTROKE;
							}
						else if (HIWORD(pkt.pkButtons)==TBN_UP) 
							{
//							mod->strokeState = ENDSTROKE;
							painter->fpressure  = 1.0f;
							}

//						mod->pressure = pkt.pkNormalPressure;
*/
						painter->fpressure = (float)(pkt.pkNormalPressure)/1024.0f;
//DebugPrint("%1.2f\n",painter->fpressure);				

//						}
					}
				}
			else painter->fpressure  = 1.0f;
			
			break;


		

		default:
			return FALSE;
		}

	return TRUE;
	}



void PainterInterface::CreateTabletWindow()
	{
	painterTabletPressure = CreateDialogParam(hInstance,
					  MAKEINTRESOURCE(IDD_DIALOG1),
					  GetCOREInterface()->GetMAXHWnd(),
					  TabletFloaterDlgProc,
					  (LPARAM)this);
	InitTablet(GetPressureEnable());
	}
void PainterInterface::DestroyTabletWindow()
	{
	DestroyWindow(painterTabletPressure);
	painterTabletPressure = NULL;
	InitTablet(FALSE);

	}
void PainterInterface::InitTablet(BOOL init)
	{
	
	if ((painterTabletPressure) && (init))
		{
		if (!loadedDLL)
			{
			LoadWinTabDLL();
			loadedDLL = TRUE;
			}
		hTab = TabletInit(painterTabletPressure,this);
		}
	else
		{
//		UnLoadWinTabDLL();
		}
	}
