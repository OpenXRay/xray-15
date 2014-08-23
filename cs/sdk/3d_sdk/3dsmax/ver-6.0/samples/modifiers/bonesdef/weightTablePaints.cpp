#include "mods.h"
#include "bonesdef.h"

#include "Maxscrpt.h"
#include "Strings.h"
#include "arrays.h"
#include "3DMath.h"
#include "Numbers.h"
#include "definsfn.h"


void BonesDefMod::PaintAttribList()
	{
	if (weightTableWindow.hWnd)
		weightTableWindow.UpdateWindowControls();
	}

void WeightTableWindow::ResizeOffScreenBuffers()
	{
	if (iNameListLabelBuf)	iNameListLabelBuf->Resize();
	if (iNameListBuf) iNameListBuf->Resize();

	if (iAttribListLabelBuf) iAttribListLabelBuf->Resize();
	if (iAttribListGlobalBuf) iAttribListGlobalBuf->Resize();
	if (iAttribListBuf) iAttribListBuf->Resize();

	if (iWeightListBuf) iWeightListBuf->Resize();
	if (iWeightListGlobalBuf) iWeightListGlobalBuf->Resize();
	if (iWeightListLabelBuf) iWeightListLabelBuf->Resize();

	}

void WeightTableWindow::BuildOffScreenBuffers()
	{	
	iWeightListBuf = CreateIOffScreenBuf(hWeightList );
	iNameListBuf = CreateIOffScreenBuf(hNameList );
	iAttribListBuf = CreateIOffScreenBuf(hAttribList );

	iWeightListGlobalBuf = CreateIOffScreenBuf(hWeightListGlobal );
	iAttribListGlobalBuf = CreateIOffScreenBuf(hAttribListGlobal );

	iWeightListLabelBuf = CreateIOffScreenBuf(hWeightListLabel );
	iNameListLabelBuf = CreateIOffScreenBuf(hNameListLabel );
	iAttribListLabelBuf = CreateIOffScreenBuf(hAttribListLabel );

	COLORREF bkColor = ColorMan()->GetColor(kWindow ) ;
	iWeightListBuf->SetBkColor(bkColor);
	iNameListBuf->SetBkColor(bkColor);
	iAttribListBuf->SetBkColor(bkColor);

	iWeightListGlobalBuf->SetBkColor(bkColor);
	iAttribListGlobalBuf->SetBkColor(bkColor);

	iWeightListLabelBuf->SetBkColor(bkColor);
	iNameListLabelBuf->SetBkColor(bkColor);
	iAttribListLabelBuf->SetBkColor(bkColor);

	}

void WeightTableWindow::BlackBorder(HDC hdc, HWND hWnd)
	{
	SelectObject(hdc,pTextPen);
	WINDOWPLACEMENT winPos;
 	GetWindowPlacement(hWnd , &winPos);
	
	winPos.rcNormalPosition.right -= (winPos.rcNormalPosition.left+1);
	winPos.rcNormalPosition.bottom -= (winPos.rcNormalPosition.top+1);

	winPos.rcNormalPosition.left = 0;
	winPos.rcNormalPosition.top = 0;

	MoveToEx(hdc, winPos.rcNormalPosition.left,winPos.rcNormalPosition.top , NULL);
	LineTo(hdc,winPos.rcNormalPosition.right,winPos.rcNormalPosition.top );
	LineTo(hdc,winPos.rcNormalPosition.right,winPos.rcNormalPosition.bottom );
	LineTo(hdc,winPos.rcNormalPosition.left,winPos.rcNormalPosition.bottom );
	LineTo(hdc,winPos.rcNormalPosition.left,winPos.rcNormalPosition.top );

	}


//5.1.01
void WeightTableWindow::PaintCellName(HDC hdc, int x, int y,int width, BOOL sel, int justify, TSTR name)
	{
	HBRUSH textBackground;
	DWORD textColor;
	DWORD textBKColor;
	
	if (sel)
		{
		
		textBackground = GetSysColorBrush(COLOR_HIGHLIGHT);
		textColor = GetSysColor(COLOR_HIGHLIGHTTEXT) ;
		textBKColor = GetSysColor(COLOR_HIGHLIGHT) ;
		SelectObject(hdc, hFixedFontBold);
 		}
	else
		{
		textBackground = ColorMan()->GetBrush(kWindow );
		textColor = ColorMan()->GetColor(kWindowText ) ;
		textBKColor = ColorMan()->GetColor(kWindow ) ;
		SelectObject(hdc, hFixedFont);
		}
	SetTextColor(hdc, textColor);
	SelectObject(hdc, textBackground);
	SetBkColor(hdc,  textBKColor);
	SelectObject(hdc,pBackPen);

	int offset = 0;

	  //5.1.01
	if (justify == TEXT_CENTER_JUSTIFIED)
		{
		SIZE strSize;
		GetTextExtentPoint32(hdc, (LPCTSTR) name, name.Length(), (LPSIZE) &strSize ); 

		offset = width/2 - strSize.cx/2;
		}
	else if (justify == TEXT_RIGHT_JUSTIFIED)
		{
		SIZE strSize;
		GetTextExtentPoint32(hdc, (LPCTSTR) name, name.Length(), (LPSIZE) &strSize ); 

		offset = width - strSize.cx - 4;
		}

	Rectangle(hdc, x,  y,x+width+1,y+textHeight+1);
	TextOut(hdc, x+3+offset,y+1,name,name.Length());   //5.1.01

	}


void WeightTableWindow::PaintCellWeight(HDC hdc, int x, int y,int width, BOOL sel, BOOL centered, 
										float value,BOOL indeterminant, BOOL locked, BOOL excluded)

	{
	HBRUSH textBackground;
	DWORD textColor;
	DWORD textBKColor;


	
	if (sel)
		{
		
		textBackground = GetSysColorBrush(COLOR_HIGHLIGHT);
		textColor = GetSysColor(COLOR_HIGHLIGHTTEXT) ;
		textBKColor = GetSysColor(COLOR_HIGHLIGHT) ;
		SelectObject(hdc, hFixedFontBold);
 		}
	else
		{
		textBackground = ColorMan()->GetBrush(kWindow );
		textColor = ColorMan()->GetColor(kWindowText ) ;
		textBKColor = ColorMan()->GetColor(kWindow ) ;
		SelectObject(hdc, hFixedFont);
		}

	SetTextColor(hdc, textColor);
	SelectObject(hdc, textBackground);
	SetBkColor(hdc,  textBKColor);
	SelectObject(hdc,pBackPen);

	int offset = 0;
	TSTR weightStr;

	if (indeterminant)
		weightStr.printf("-");
	else weightStr.printf("%0.3f",value);

	SIZE strSize;
	GetTextExtentPoint32(hdc,  (LPCTSTR) weightStr,    weightStr.Length(),  (LPSIZE) &strSize ); 

	int xPos;
	
	if (indeterminant)
		xPos = width/2 - strSize.cx/2;
	else xPos = 3;

	if (centered)
		{

		GetTextExtentPoint32(hdc, (LPCTSTR) weightStr, weightStr.Length(), (LPSIZE) &strSize ); 

		xPos = width/2 - strSize.cx/2;
		}

	Rectangle(hdc, x,  y,x+width+1,y+textHeight+1);
	TextOut(hdc, x+xPos,y+1,weightStr,weightStr.Length());

/*	if ((locked) || (excluded))
		{
		textBackground = ColorMan()->GetBrush(kWindow );
		textColor = GetSysColor(COLOR_GRAYTEXT ) ;
		textBKColor = ColorMan()->GetColor(kWindow ) ;
		}
*/
	if (this->GetShowExclusion())
		{
		SelectObject(hdc,pBackPen);
		Rectangle(hdc, x+width-9,  y,x+width+1,y+9+1);
		if (excluded)
			{
			SelectObject(hdc,pSelPen);
			MoveToEx(hdc,x+width-9, y, NULL);
			LineTo(hdc,x+width+1,y+9+1);

			MoveToEx(hdc,x+width, y, NULL);
			LineTo(hdc,x+width-9,y+9);
			}
		}

	if (GetShowLock())
		{
		SelectObject(hdc,pBackPen);
		Rectangle(hdc, x+width-18,  y,x+width-9+1,y+9+1);
		if (locked)
			{
			SelectObject(hdc,pSelPen);

			MoveToEx(hdc,x+width-16, y+2, NULL);
			LineTo(hdc,x+width-16,y+7);
			LineTo(hdc,x+width-11,y+7);
			}
		}
	
	}


void WeightTableWindow::PaintCellNameVertically(HDC hdc, int x, int y, int height, BOOL sel, TSTR name)
	{
	HBRUSH textBackground;
	DWORD textColor;
	DWORD textBKColor;
	
	if (sel)
		{
		
		textBackground = GetSysColorBrush(COLOR_HIGHLIGHT);
		textColor = GetSysColor(COLOR_HIGHLIGHTTEXT) ;
		textBKColor = GetSysColor(COLOR_HIGHLIGHT) ;
		SelectObject(hdc, hFixedFontBold);
 		}
	else
		{
		textBackground = ColorMan()->GetBrush(kWindow );
		textColor = ColorMan()->GetColor(kWindowText ) ;
		textBKColor = ColorMan()->GetColor(kWindow ) ;
		SelectObject(hdc, hFixedFont);
		}
	SetTextColor(hdc, textColor);
	SelectObject(hdc, textBackground);
	SetBkColor(hdc,  textBKColor);
	SelectObject(hdc,pBackPen);
	Rectangle(hdc, x,  y,x+textHeight+1,y+height+1);
	int th = textHeight -4;
	if ((th * name.Length()) <= height)
		{
		y = height - textHeight+1;

		for	(int i= (name.Length()-1); i >= 0 ; i--)
			{
			TCHAR *t = &name[i];
			int offset = textHeight/4;
			TextOut(hdc, x+offset,y,t,1);
			y -= th;
			}
		}
	else
		{
		y = 1;

		for	(int i= 0 ; i < name.Length() ; i++)
			{
			TCHAR *t = &name[i];
			int offset = textHeight/4;
			TextOut(hdc, x+offset,y,t,1);
			y += th;
			}

		}

	}

void WeightTableWindow::PaintCellAttribute(HDC hdc, int x, int y, BOOL sel, int state)
	{
	HBRUSH textBackground;
	DWORD textColor;
	DWORD textBKColor;
	SIZE strSize;

	if (sel)
		{
		
		textBackground = GetSysColorBrush(COLOR_HIGHLIGHT);
		textColor = GetSysColor(COLOR_HIGHLIGHTTEXT) ;
		textBKColor = GetSysColor(COLOR_HIGHLIGHT) ;
		SelectObject(hdc, hFixedFontBold);
 		}
	else
		{
		textBackground = ColorMan()->GetBrush(kWindow );
		textColor = ColorMan()->GetColor(kWindowText ) ;
		textBKColor = ColorMan()->GetColor(kWindow ) ;
		SelectObject(hdc, hFixedFont);
		}
	SetTextColor(hdc, textColor);
	SelectObject(hdc, textBackground);
	SetBkColor(hdc,  textBKColor);
	SelectObject(hdc,pBackPen);

	TSTR stateStr;
	if (state == STATE_CHECKED)
		stateStr.printf("X");
	else if (state == STATE_INDETERMIANT)
		stateStr.printf("-");
	else stateStr.printf(" ");

	int offset;
	GetTextExtentPoint32(hdc, (LPCTSTR) stateStr, stateStr.Length(), (LPSIZE) &strSize ); 
	if (GetFlipFlopUI())
		offset = vertNameWidth/2 - strSize.cx/2;
	else offset = textHeight/2 - strSize.cx/2;
	if (GetFlipFlopUI())
		Rectangle(hdc, x,  y,x+vertNameWidth+1,y+textHeight+1);
	else Rectangle(hdc, x,  y,x+textHeight+1,y+textHeight+1);

	TextOut(hdc, x+offset,y+1,stateStr,stateStr.Length());

	}

void WeightTableWindow::PaintNameListLabel()
	{
	HDC hdc;
	PAINTSTRUCT		ps;
 
	BeginPaint(hNameListLabel,&ps);
	EndPaint(hNameListLabel,&ps);

	iNameListLabelBuf->Erase();
	hdc = iNameListLabelBuf->GetDC();
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);
	SelectObject(hdc,pTextPen);

	int x = 0;
	int y = 0;
	x = 0;
	y = 0;
	TSTR vertName;
	if (GetFlipFlopUI())
		{
		vertName.printf("%s",GetString(IDS_PW_BONEID));
		PaintCellName(hdc,x,y,buttonWidth,FALSE,TEXT_LEFT_JUSTIFIED,vertName);
		}
	else
		{
		vertName.printf("%s",GetString(IDS_PW_VERTEXID));
		PaintCellName(hdc,x,y,vertNameWidth,FALSE,TEXT_LEFT_JUSTIFIED,vertName);
		}

	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc, hOldFont);
	iNameListLabelBuf->Blit();

	}




void WeightTableWindow::PaintNameList()
	{
	HDC hdc;
	PAINTSTRUCT		ps;
	BeginPaint(hNameList,&ps);
	EndPaint(hNameList,&ps);

	iNameListBuf->Erase();
	hdc = iNameListBuf->GetDC();
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);
	
	WINDOWPLACEMENT winPos;
	GetWindowPlacement(hNameList , &winPos);
	int width = winPos.rcNormalPosition.right - winPos.rcNormalPosition.left;



	TSTR vertName;
	if (GetFlipFlopUI())
		{
		int y = 0;
		int x = 0;
//paint cells
		int ct = firstColumn;
		for (int i = 0; i < numberOfColumns; i++)
			{
			if ((ct >=0) && (ct < vertexPtrList.Count()))
				{
				if (modDataList.Count() == 1)
					{
					if (GetReduceLabels())
						vertName.printf("#%d",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
					else vertName.printf("#%d Vert",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
					}
				else
					{
					if (GetReduceLabels())
						vertName.printf("#%d(%s)",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
					else vertName.printf("#%d Vert(%s)",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
					}
				
				PaintCellName(hdc,x,y,vertNameWidth,vertexPtrList[ct].IsSelected(), TEXT_LEFT_JUSTIFIED ,vertName);

				}
			ct++;
			x+= vertNameWidth;
			}

		}
	else
		{
		int x = 0;
		int y = 0;
		

//paint vert names
		int ct = firstRow;
		for (int i = 0; i < numberOfRows; i++)
			{
			if ((ct >=0) && (ct < vertexPtrList.Count()))
				{
				vertName.printf("#%d Vert(%s)",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
				if (modDataList.Count() == 1)
					{
					if (GetReduceLabels())
						vertName.printf("#%d",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
					else vertName.printf("#%d Vert",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
					}
				else
					{
					if (GetReduceLabels())
						vertName.printf("#%d(%s)",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
					else vertName.printf("#%d Vert(%s)",vertexPtrList[ct].index,vertexPtrList[ct].node->GetName());
					}
				PaintCellName(hdc,x,y,vertNameWidth,vertexPtrList[ct].IsSelected(),TEXT_LEFT_JUSTIFIED, vertName);
				}
			ct++;
			y+= textHeight;
			}

		}
	BlackBorder(hdc, hNameList);

//make sure to select in a stock object befire deletes otherwise it will leak a resource
	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc, hOldFont);

	iNameListBuf->Blit();


	}

void WeightTableWindow::PaintAttribListLabel()
	{
	HDC hdc;
	PAINTSTRUCT		ps;
	BeginPaint(hAttribListLabel,&ps);
	EndPaint(hAttribListLabel,&ps);

	iAttribListLabelBuf->Erase();
	hdc = iAttribListLabelBuf->GetDC();

	HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);
	SelectObject(hdc,pTextPen);
	SetTextColor(hdc, textColor);

	WINDOWPLACEMENT winPos;
 	GetWindowPlacement(hAttribListLabel , &winPos);
	int x = 0;
	int y = 0;

	if (GetFlipFlopUI())
		{
		x = 0;
		y = 0;
		TSTR selectedStr;
		selectedStr.printf("%s",GetString(IDS_SELECTED));

		PaintCellName(hdc,  x, y, buttonWidth, FALSE, TEXT_LEFT_JUSTIFIED, selectedStr);

		y += textHeight;
		selectedStr.printf("%s",GetString(IDS_MODIFIED));
		PaintCellName(hdc,  x, y, buttonWidth, FALSE, TEXT_LEFT_JUSTIFIED, selectedStr);

		y += textHeight;
		selectedStr.printf("%s",GetString(IDS_PW_NORMALIZED));
		PaintCellName(hdc,  x, y, buttonWidth, FALSE, TEXT_LEFT_JUSTIFIED, selectedStr);

		y += textHeight;
		selectedStr.printf("%s",GetString(IDS_PW_RIGID));
		PaintCellName(hdc,  x, y, buttonWidth, FALSE, TEXT_LEFT_JUSTIFIED, selectedStr);

		y += textHeight;
		selectedStr.printf("%s",GetString(IDS_PW_RIGIDHANDLE));
		PaintCellName(hdc,  x, y, buttonWidth, FALSE, TEXT_LEFT_JUSTIFIED, selectedStr);

/*		y += textHeight;
		selectedStr.printf(" ");
		PaintCellName(hdc,  x, y, buttonWidth, FALSE, FALSE, selectedStr);

		y += textHeight;
		selectedStr.printf(" ");
		PaintCellName(hdc,  x, y, buttonWidth, FALSE, FALSE, selectedStr);

		y += textHeight;
		selectedStr.printf(" ");
		PaintCellName(hdc,  x, y, buttonWidth, FALSE, FALSE, selectedStr);

*/
		}
	else
		{
		int height = winPos.rcNormalPosition.bottom-winPos.rcNormalPosition.top;
		int startY = height;
		y = startY;

		BOOL shortenLabel = FALSE;
		if (height < (textHeight+textHeight/2))
			shortenLabel = TRUE;



		x = 0;
		TSTR selectedStr;
		if (shortenLabel)
			selectedStr.printf("%s",GetString(IDS_PW_SELECTEDSHORT));
		else selectedStr.printf("%s",GetString(IDS_SELECTED));

		PaintCellNameVertically(hdc, x, 0, height, TEXT_LEFT_JUSTIFIED, selectedStr);

		x += textHeight;
		if (shortenLabel)
			selectedStr.printf("%s",GetString(IDS_PW_MODIFIEDSHORT));
		else selectedStr.printf("%s",GetString(IDS_MODIFIED));
		PaintCellNameVertically(hdc, x, 0, height, TEXT_LEFT_JUSTIFIED, selectedStr);

		x += textHeight;
		if (shortenLabel)
			selectedStr.printf("%s",GetString(IDS_PW_NORMALIZEDSHORT));
		else selectedStr.printf("%s",GetString(IDS_PW_NORMALIZED));
		PaintCellNameVertically(hdc, x, 0, height, TEXT_LEFT_JUSTIFIED, selectedStr);

		x += textHeight;
		if (shortenLabel)
			selectedStr.printf("%s",GetString(IDS_PW_RIGIDSHORT));
		else selectedStr.printf("%s",GetString(IDS_PW_RIGID));
		PaintCellNameVertically(hdc, x, 0, height, TEXT_LEFT_JUSTIFIED, selectedStr);

		x += textHeight;
		if (shortenLabel)
			selectedStr.printf("%s",GetString(IDS_PW_RIGIDHANDLESHORT));
		else selectedStr.printf("%s",GetString(IDS_PW_RIGIDHANDLE));
		PaintCellNameVertically(hdc, x, 0, height, TEXT_LEFT_JUSTIFIED, selectedStr);

/*
		x += textHeight;
		selectedStr.printf(" ");
		PaintCellNameVertically(hdc, x, 0, height, FALSE, selectedStr);

		x += textHeight;
		selectedStr.printf(" ");
		PaintCellNameVertically(hdc, x, 0, height, FALSE, selectedStr);

		x += textHeight;
		selectedStr.printf(" ");
		PaintCellNameVertically(hdc, x, 0, height, FALSE, selectedStr);
*/

		}
	BlackBorder(hdc, hAttribListLabel);

	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc, hOldFont);


	iAttribListLabelBuf->Blit();
	}

void WeightTableWindow::PaintAttribListGlobal()
	{
	HDC hdc;
	PAINTSTRUCT		ps;
	BeginPaint(hAttribListGlobal,&ps);
	EndPaint(hAttribListGlobal,&ps);

	iAttribListGlobalBuf->Erase();
	hdc = iAttribListGlobalBuf->GetDC();


	HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);
	SelectObject(hdc,pTextPen);

	SetTextColor(hdc, textColor);


	BuildGlobalAttribList();

	int x,y;
	x = 0;
	y = 0;
	if (GetFlipFlopUI())
		{
		x = 0;
		y = 0;
		for(int i =0; i < numAttributes; i++)
			{
			PaintCellAttribute(hdc, x, y,FALSE, globalAttribList[i] );
			y += textHeight;
			}


		}
	else
		{

		x = 0;
		y = 0;
		for(int i =0; i < numAttributes; i++)
			{
			PaintCellAttribute(hdc, x, y,FALSE, globalAttribList[i] );
			x += textHeight;
			}
		}

	BlackBorder(hdc, hAttribListGlobal);

	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc, hOldFont);




	iAttribListGlobalBuf->Blit();
	}

void WeightTableWindow::PaintAttribList()
	{
	HDC hdc;
	PAINTSTRUCT		ps;
	BeginPaint(hAttribList,&ps);
	EndPaint(hAttribList,&ps);

	iAttribListBuf->Erase();
	hdc = iAttribListBuf->GetDC();

	COLORREF darkGreyColor = RGB(80,80,80);
	COLORREF blackColor = RGB(0,0,0);

//	COLORREF selColor = selectionColor;

 	SelectObject(hdc,pTextPen);
//draw labels

	WINDOWPLACEMENT winPos;
 	GetWindowPlacement(hAttribList , &winPos);
	
	

	HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);
//paint attribute names here

	SIZE strSize;
	TSTR hString("H");
	GetTextExtentPoint32(hdc,  (LPCTSTR) hString,    hString.Length(),  (LPSIZE) &strSize ); 
	int th = strSize.cy-4;

	

	if (GetFlipFlopUI())
		{
//paint names
		int y = 0;
		int x = 0;

//		int x1 = winPos.rcNormalPosition.right;

		
	//now draw regular attibributes
		int start = firstColumn;
		for (int i=0; i < numberOfColumns; i++)
			{
			VertexListClass *vd;
			y = 0;
		
			if ((start >=0) && (start < vertexPtrList.Count()))
				{
				vd = vertexPtrList[start].vertexData;

				if (vertexPtrList[start].bmd->selected[vertexPtrList[start].index])
					PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );
//next row modified
				y += textHeight;
				if (vd->IsModified())
					PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );
//next row normalized
				y += textHeight;
				if (!vd->IsUnNormalized())
					PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );
//next row rigid
				y += textHeight;
				if (vd->IsRigid())
					PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );

//next row rigidHandle
				y += textHeight;
				if (vd->IsRigidHandle())
					PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );


				//after this newer attributes
/*
				for (int k = 2; k < numAttributes; k++)
					{
					y += textHeight;
					PaintCellAttribute(hdc, x, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );
					}
*/
				}
			start++;
			x += vertNameWidth;
			}

		}
	else
		{
		int x,y;
		x = 0;
		y = 0;
		//draw regular attibutes
		y = 0;
		int start = firstRow;
		for (int i=0; i < numberOfRows; i++)
			{
			VertexListClass *vd;
			int x1 = 0;
		
			if ((start >=0) && (start < vertexPtrList.Count()))
				{
				vd = vertexPtrList[start].vertexData;

				if (vertexPtrList[start].bmd->selected[vertexPtrList[start].index])
					PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );
//next row modified
				x1 += textHeight;
				if (vd->IsModified())
					PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );

//next row normalized
				x1 += textHeight;
				if (!vd->IsUnNormalized())
					PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );

//next row rigid
				x1 += textHeight;
				if (vd->IsRigid())
					PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );
				
//next row rigid
				x1 += textHeight;
				if (vd->IsRigidHandle())
					PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_CHECKED );
				else PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );
				
				for (int k = 2; k < numAttributes; k++)
					{
					x1 += textHeight;
					PaintCellAttribute(hdc, x1, y,vertexPtrList[start].IsSelected(), STATE_UNCHECKED );
					}
//after this newer attributes
				}
			start++;
			y += textHeight;
			}
		}

	BlackBorder(hdc, hAttribList);

	SelectObject(hdc, hOldFont);

	SelectObject(hdc,GetStockObject(BLACK_PEN));

	iAttribListBuf->Blit();

	}

void WeightTableWindow::PaintWeightListLabel()
	{
	HDC hdc;
	PAINTSTRUCT		ps;
	BeginPaint(hWeightListLabel,&ps);
	EndPaint(hWeightListLabel,&ps);

	iWeightListLabelBuf->Erase();
	hdc = iWeightListLabelBuf->GetDC();

	WINDOWPLACEMENT winPos;
 	GetWindowPlacement(hWeightListLabel , &winPos);

	HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);

//5.1.01
	int  justify = TEXT_LEFT_JUSTIFIED;
	if (GetRightJustify()) justify = TEXT_RIGHT_JUSTIFIED;

	if (GetFlipFlopUI())
		{
//paint in names
	//draw bone name labels
	//paint in bone names
		int y = 0;
 		int x = 0;
		SetTextColor(hdc,textColor);
		for (int j = 0; j < numberOfRows; j++)
			{
//if not affacted 
//if not get weight
			TSTR boneName("-");
			if ((j+firstRow) < activeBones.Count())
				{
				int currentBone = activeBones[j+firstRow];
				if ((mod) && (currentBone < mod->BoneData.Count()))
					{
					if (mod->BoneData[currentBone].Node)
						{
						boneName.printf("%s",mod->BoneData[currentBone].Node->GetName());

						PaintCellName(hdc, x, y, buttonWidth, (mod->ModeBoneIndex == currentBone), justify,boneName);
						y+= textHeight;
						
						}	
					}
				}

			}

		}
	else
		{
		int y = 0;
		int x = 0;

	//5.1.01
		if (justify == TEXT_LEFT_JUSTIFIED)
			{
			for (int j = 0; j < numberOfColumns; j++)
				{
	//if not affacted 
	//if not get weight
				TSTR boneName("-");
				if ((j+firstColumn) < activeBones.Count())
					{
					int currentBone = activeBones[j+firstColumn];
					if ((mod) && (currentBone < mod->BoneData.Count()))
						{
						if (mod->BoneData[currentBone].Node)
							{
							boneName.printf("%s",mod->BoneData[currentBone].Node->GetName());

	//5.1.01
							PaintCellName(hdc, x, y,buttonWidth, (mod->ModeBoneIndex == currentBone), justify,boneName);
							x+= buttonWidth;
							}	
						}
					}
				}
			}
		else 	//5.1.01
			{
			int ct = 0;
			for (int j = 0; j < numberOfColumns; j++)
				{
				if ((j+firstColumn) < activeBones.Count())
					{
					x += buttonWidth ;
					ct++;
					}
				}

			x -=buttonWidth;
			
			for (j = (numberOfColumns-1); j >= 0; j--)
				{
	//if not affacted 
	//if not get weight
				TSTR boneName("-");
				if ((j+firstColumn) < activeBones.Count())
					{
					int currentBone = activeBones[j+firstColumn];
					if ((mod) && (currentBone < mod->BoneData.Count()))
						{
						if (mod->BoneData[currentBone].Node)
							{
							boneName.printf("%s",mod->BoneData[currentBone].Node->GetName());

	//5.1.01
							PaintCellName(hdc, x, y,buttonWidth, (mod->ModeBoneIndex == currentBone), justify,boneName);
							x-= buttonWidth;
							}	
						}
					}
				}
			}


		}


	BlackBorder(hdc, hWeightListLabel);

	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc, hOldFont);
	iWeightListLabelBuf->Blit();
	}


void WeightTableWindow::PaintWeightListGlobal()
	{
	HDC hdc;
	PAINTSTRUCT		ps;
	BeginPaint(hWeightListGlobal,&ps);
	EndPaint(hWeightListGlobal,&ps);

	iWeightListGlobalBuf->Erase();
	hdc = iWeightListGlobalBuf->GetDC();

	WINDOWPLACEMENT winPos;
 	GetWindowPlacement(hWeightListGlobal , &winPos);
	

	HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);

	if (GetFlipFlopUI())
		{
//draw hor. line
		int x = 0;
		int y = 0;
		for (int j = 0; j < numberOfRows; j++)
			{
			TSTR blank;
			blank.printf(" ");
			PaintCellName(hdc, x, y,vertNameWidth, FALSE, TEXT_LEFT_JUSTIFIED,blank);
			y += textHeight;
			}

		}
	else
		{
		int x = 0;
		int y = 0;
		for (int j = 0; j < numberOfColumns; j++)
			{
			TSTR blank;
			blank.printf(" ");
			PaintCellName(hdc, x, y,buttonWidth, FALSE, TEXT_LEFT_JUSTIFIED,blank);
			x += buttonWidth;
			}
		}	


	BlackBorder(hdc, hWeightListGlobal);

	SelectObject(hdc,GetStockObject(BLACK_PEN));
	SelectObject(hdc, hOldFont);
	iWeightListGlobalBuf->Blit();


	if (hGlobalWeight)
		{

		BringWindowToTop(hGlobalWeight);
		
		InvalidateRect(hGlobalWeight, NULL,TRUE);
  
		UpdateWindow( hGlobalWeight);
		}

	}

void WeightTableWindow::PaintWeightList()
	{

	if (firstRow < 0 ) firstRow = 0;
	

	HDC hdc;
	PAINTSTRUCT		ps;
	BeginPaint(hWeightList,&ps);
	EndPaint(hWeightList,&ps);



	iWeightListBuf->Erase();
	hdc = iWeightListBuf->GetDC();
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFixedFont);

	WINDOWPLACEMENT winPos;
	GetWindowPlacement(hWeightList , &winPos);

//draw bone name labels
	BlackBorder(hdc, hWeightList);



	COLORREF redColor = selectionColor;

	SelectObject(hdc,pTextPen);
	SetTextColor(hdc, textColor);
	if (GetFlipFlopUI())
		{
//paint in names
	//draw bone name labels
	//paint in bone names
		int y = 1;
 		int x = 3;
	//draw in weights
		int start = firstColumn;
		y = 0;
		int currentSpinner = 0;
		x = 0;
		for (int i = 0; i < numberOfColumns; i++)
			{
			y = 0;
			for (int j = 0; j < numberOfRows; j++)
				{
//if not affacted 
//if not get weight
				if ( (start >=0) && (start < vertexPtrList.Count()) && ((j+firstRow)<activeBones.Count()))
					{
					int currentBone = activeBones[j+firstRow];
					BOOL influenced = FALSE;
					float w = 0.0f;
					for (int k = 0; k < vertexPtrList[start].vertexData->d.Count(); k++)
						{
						if (vertexPtrList[start].vertexData->d[k].Bones == currentBone)
							{
						
							if (vertexPtrList[start].vertexData->IsModified())
								w = vertexPtrList[start].vertexData->d[k].Influences;
							else w = vertexPtrList[start].vertexData->d[k].normalizedInfluences;
							influenced = TRUE;
							}
						}
					BOOL indet = TRUE;
					if (influenced) indet = FALSE;
					BOOL excluded = FALSE;
					int exCount = vertexPtrList[start].bmd->exclusionList.Count();
					int index =  vertexPtrList[start].index;
					if  ( (currentBone <  exCount) && (vertexPtrList[start].bmd->exclusionList[currentBone]) )
						excluded = vertexPtrList[start].bmd->isExcluded(currentBone, index);
					PaintCellWeight(hdc, x, y,vertNameWidth, vertexPtrList[start].IsSelected(), FALSE, 
										w,indet, FALSE, excluded);

					y+= textHeight;
					}
			
				}
			x += vertNameWidth;
			start++;
			}
		}
	else
		{
		int y = 0;
		int x = 0;
	//paint in weights now
//get the number of verts visible
//get the verts height
		int start = firstRow;
//		y = 1;
		
		for (int i = 0; i < numberOfRows; i++)
			{
			x = 0;
			for (int j = 0; j < numberOfColumns; j++)
				{
//if not affacted 
//if not get weight
				if ( (start >=0) && (start < vertexPtrList.Count()) && ((j+firstColumn)<activeBones.Count()))
					{
				
					int currentBone = activeBones[j+firstColumn];
					BOOL influenced = FALSE;
					float w = 0.0f;
					for (int k = 0; k < vertexPtrList[start].vertexData->d.Count(); k++)
						{
						if (vertexPtrList[start].vertexData->d[k].Bones == currentBone)
							{
						
							if (vertexPtrList[start].vertexData->IsModified())
								w = vertexPtrList[start].vertexData->d[k].Influences;
							else w = vertexPtrList[start].vertexData->d[k].normalizedInfluences;
//							weightStr.printf("%0.3f",w);
							influenced = TRUE;
							}
						}
					BOOL indet = TRUE;
					if (influenced) indet = FALSE;
					BOOL excluded = FALSE;
					int exCount = vertexPtrList[start].bmd->exclusionList.Count();
					int index =  vertexPtrList[start].index;
					if  ( (currentBone <  exCount) && (vertexPtrList[start].bmd->exclusionList[currentBone]) )
						excluded = vertexPtrList[start].bmd->isExcluded(currentBone, index);
					PaintCellWeight(hdc, x, y,buttonWidth, vertexPtrList[start].IsSelected(), FALSE, 
										w,indet, FALSE, excluded);
					x+= buttonWidth;
					}
			
				}
			y += textHeight;
			start++;
			}
		
		}

	iWeightListBuf->Blit();
	if (hWeight)
		{
/*
		if (useSpinners)
			{
			for (i = 0; i < spinList.Count(); i++)
				{
//				BringWindowToTop(spinList[i].edit);
//				BringWindowToTop(spinList[i].spinner);
//				InvalidateRect(spinList[i].edit, NULL,TRUE);
//				InvalidateRect(spinList[i].spinner, NULL,TRUE);
				}

			}
		else 
*/
		BringWindowToTop(hWeight);
		
		InvalidateRect(hWeight, NULL,TRUE);
  
		UpdateWindow( hWeight);
		}


	}



