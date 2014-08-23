/*==============================================================================

  file:	        LegendBitmap.cpp

  author:       Jean-Philippe Morel

  created:	    May 27th 2002

  description:
        This object creates a legend in a bitmap. It represents the range of colors
        display in the result of a pseudo color render and also inform the user 
        about the unit used.

  modified:	


© 2002 Autodesk
==============================================================================*/

#include "LegendBitmap.h"
#include "plugin.h"         // For GetString
#include <ILightingUnits.h>


const float CLegendBitmap::PERCENTAGE_USED_BY_GRADIENT_IN_LEGEND = 0.9f;
const int   CLegendBitmap::NUMBER_OF_CARACTERS_PER_LINE          = 100; // Determine the width of caracters
const int   CLegendBitmap::NUMBER_OF_ROW_POSSIBLE                =   4; // Determine the height of caracters
const int   CLegendBitmap::NUMBER_OF_SECTION                     =   6; // If you put it higher the numbers my overlap 


//--------------------------------------------------------------------------------------------------
//  Construction
//---------------------------------------------------------------------------------------------------
CLegendBitmap::CLegendBitmap():
    m_pTheBitmap (NULL),
    m_pPseudoMap (NULL)
{
    m_hDC = CreateCompatibleDC( NULL );
}

//--------------------------------------------------------------------------------------------------
//  Destruction
//---------------------------------------------------------------------------------------------------
CLegendBitmap::~CLegendBitmap()
{
    if(m_pTheBitmap)
    {
        m_pTheBitmap->DeleteThis();
        m_pTheBitmap = NULL;
    }

    ReleaseDC( NULL, m_hDC ) ;
}


//--------------------------------------------------------------------------------------------------
//  Name : BuildLegend
//
//  Input : Height and Width of the legend
//---------------------------------------------------------------------------------------------------
void CLegendBitmap::BuildLegend(int iWidth, int iHeight)
{
    if(m_pTheBitmap)
        m_pTheBitmap->DeleteThis();
    
    BitmapInfo bi;
    
	bi.SetName("");
	bi.SetWidth (iWidth);
	bi.SetHeight(iHeight);
	bi.SetType(BMM_TRUE_32);

	m_pTheBitmap = TheManager->Create(&bi);

    ToneOperatorInterface* toneOpInt = static_cast<ToneOperatorInterface*>( GetCOREInterface(TONE_OPERATOR_INTERFACE) );

    m_pPseudoMap =  (PseudoMap*)toneOpInt->GetToneOperator();

    WriteTitle();
    WriteValues();
    BuildGradient();
    WriteDelimiter();
}


//--------------------------------------------------------------------------------------------------
//  Name : BuildGradient
//---------------------------------------------------------------------------------------------------
void CLegendBitmap::BuildGradient()
{
    int iLeftOffset  = (m_pTheBitmap->Width() - (m_pTheBitmap->Width() * PERCENTAGE_USED_BY_GRADIENT_IN_LEGEND)) / 2;
    int iRightOffset = m_pTheBitmap->Width() - iLeftOffset;

    int GradientWidth  = iRightOffset - iLeftOffset;
    int GradientHeight = m_pTheBitmap->Height() / 2;

    ULONG pseudoColorMap[NB_MAX_PSEUDO_COLORS];
    ZeroMemory(pseudoColorMap, sizeof(ULONG) * NB_MAX_PSEUDO_COLORS);
    
    m_pPseudoMap->GetPseudoColorMap(pseudoColorMap, sizeof(ULONG)*NB_MAX_PSEUDO_COLORS);

    for(int k = iLeftOffset; k < iRightOffset ; k++)
    {
        for(int j = GradientHeight; j < m_pTheBitmap->Height() ; j++)
        {
            BMM_Color_fl color;

            int iIndex = (int)(NB_PSEUDO_COLORS * (k-iLeftOffset) / GradientWidth);
            
            color.r = GetRValue(pseudoColorMap[iIndex]) / 255.0;
            color.g = GetGValue(pseudoColorMap[iIndex]) / 255.0;
            color.b = GetBValue(pseudoColorMap[iIndex]) / 255.0;
            color.a = 0;

            m_pTheBitmap->PutPixels(k,j,1, &color);
        }
    }   
}

    
//--------------------------------------------------------------------------------------------------
//  Name : GetDelimiterValue
//
//  Inputs : - The gradient width in pixels
//           - The exact position in the gradient in pixel
//
//  Outputs : - The new position in pixel
//
//---------------------------------------------------------------------------------------------------
int CLegendBitmap::GetDelimiterValue(int iGradientWidth, int iPosition)
{
    float fColorInterval = (float)iGradientWidth / (float)NB_PSEUDO_COLORS;

    for(int i = 0 ; i < iGradientWidth ; i++)
    {
        if (fColorInterval*i > iPosition) return (int)((fColorInterval*i)-fColorInterval);
    }

    return 0;
}


//--------------------------------------------------------------------------------------------------
//  Name : GetPixelColor
//
//  Input : x, y position of the wanted pixel in the legend
//
//  Output : The RGB color of the pixel
//---------------------------------------------------------------------------------------------------
Color CLegendBitmap::GetPixelColor(int iX, int iY)
{
    BMM_Color_fl color;
    Color ColortoReturn;

    if(m_pTheBitmap)
    {
        m_pTheBitmap->GetPixels(iX, iY, 1, &color);

        ColortoReturn.r = color.r;
        ColortoReturn.g = color.g;
        ColortoReturn.b = color.b;

        return ColortoReturn;
    }
    else
        return Color(0,0,0);           
}


//--------------------------------------------------------------------------------------------------
//  Name : WriteDelimiter
//---------------------------------------------------------------------------------------------------
void CLegendBitmap::WriteDelimiter()
{
    int iDelimiterWidth  = (int)((float)m_pTheBitmap->Width() * 0.003125f);
    int iDelimiterHeight = m_pTheBitmap->Height() / 4;
    
    int iLeftOffset  = (m_pTheBitmap->Width() - (m_pTheBitmap->Width() * PERCENTAGE_USED_BY_GRADIENT_IN_LEGEND)) / 2;
    int iRightOffset = m_pTheBitmap->Width() - iLeftOffset;

    for(int iI = 0; iI < iDelimiterWidth; iI++)
    {
        for(int iJ = 0; iJ < iDelimiterHeight ; iJ++)
        {
            BMM_Color_fl color;
        
            color.r = 1.0;
            color.g = 1.0;
            color.b = 1.0;
            color.a = 0;
      
            m_pTheBitmap->PutPixels(iLeftOffset  + iI,   iJ+(m_pTheBitmap->Height()/2), 1, &color);
            m_pTheBitmap->PutPixels(iRightOffset - iI - 1,   iJ+(m_pTheBitmap->Height()/2), 1, &color);
        }
    }

    float fInterval = (float)(iRightOffset - iLeftOffset)/(float)(NUMBER_OF_SECTION);

    for(int i = 1; i < NUMBER_OF_SECTION ; i++)
        for(int j = 0; j < iDelimiterWidth; j++)
            for(int k = 0; k < iDelimiterHeight ; k++)
            {
                BMM_Color_fl color;
        
                color.r = 1.0;
                color.g = 1.0;
                color.b = 1.0;
                color.a = 0;
     
                m_pTheBitmap->PutPixels(GetDelimiterValue((iRightOffset - iLeftOffset), (fInterval*i))+iLeftOffset+j, k+(m_pTheBitmap->Height()/2), 1, &color);
            }
    
}


//--------------------------------------------------------------------------------------------------
//  Name : WriteText
//
//  Input : - x, y position of the text in the legend
//          - The string that will bee written
//          - The Alignment of the text (DT_RIGHT, DT_CENTER, DT_LEFT(default))
//
//---------------------------------------------------------------------------------------------------
void CLegendBitmap::WriteText(int x, int y, TCHAR *textBuffer, UINT uiAlign)
{
    ////////////////////////////////
    //  Create the bitmap where
    //  will write the text
    //
    BITMAPINFO bmpInfo;
    RGBQUAD*  bits = NULL;

    int fontWidth  = m_pTheBitmap->Width()  / NUMBER_OF_CARACTERS_PER_LINE; 
    int fontHeight = m_pTheBitmap->Height() / NUMBER_OF_ROW_POSSIBLE;
    
    bmpInfo.bmiHeader.biHeight      = fontHeight;
    bmpInfo.bmiHeader.biWidth       = (fontWidth * _tcslen(textBuffer))+fontWidth;
    bmpInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biPlanes      = 1; 
	bmpInfo.bmiHeader.biBitCount    = 32;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biSizeImage   = 0;

    HBITMAP hBitmap = CreateDIBSection(m_hDC, &bmpInfo, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
	
    if(!hBitmap)return;		
   

    ////////////////////////////////////
    // Associate the bitmap to the DC
    //
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(m_hDC, hBitmap);

    
    GdiFlush();
    SetBkColor  (m_hDC, RGB(0,  0,  0  ));
    SetTextColor(m_hDC, RGB(255,255,255));


    ///////////////////////////////////////
    // Create used font
    //
    HFONT hBoldFont = CreateFont(fontHeight, 
                                 fontWidth, 
                                 0, 
                                 0, 
                                 FW_BOLD, //FW_REGULAR, 
                                 0, 0, 0, 
                                 0, 
                                 0, 0, 
                                 0, 
                                 DEFAULT_PITCH|FF_SWISS, 
                                 _T(""));
    
	if(hBoldFont) SelectObject(m_hDC, hBoldFont);

    ///////////////////////////////////////////
    // Draw the text to the bitmap
    //
    RECT rCaption = {0 ,0 , bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight};
	DrawText(m_hDC, textBuffer, _tcslen(textBuffer), &rCaption, DT_TOP|uiAlign);
	if(hBoldFont) DeleteObject(hBoldFont);


    //////////////////////////////////
    // Retrive the bitmap value
    // and put it into the legend
    // at the specified location
    //
    for(int i = 0 ; i < bmpInfo.bmiHeader.biHeight ; i++)
    {
        for(int j = 0; j < bmpInfo.bmiHeader.biWidth; j++)
        {
            BMM_Color_fl color;

            int iIndex = j + (bmpInfo.bmiHeader.biWidth * i);
            
            color.r = bits[iIndex].rgbRed;
            color.g = bits[iIndex].rgbGreen;
            color.b = bits[iIndex].rgbBlue;
            color.a = 0;

            if(uiAlign == DT_LEFT)
            {
                m_pTheBitmap->PutPixels(x+j,y+(bmpInfo.bmiHeader.biHeight - i), 1, &color);
            }
            else if(uiAlign == DT_RIGHT)
            {
                m_pTheBitmap->PutPixels(x-bmpInfo.bmiHeader.biWidth+j,y+(bmpInfo.bmiHeader.biHeight - i), 1, &color);
            }
            else if(uiAlign == DT_CENTER)
            {
                m_pTheBitmap->PutPixels(x-(bmpInfo.bmiHeader.biWidth/2)+j,y+(bmpInfo.bmiHeader.biHeight - i), 1, &color);
            }
            
        }
    }

    ///////////////////////
    // Release objects
    //
    SelectObject(m_hDC, hOldBitmap);
    DeleteObject(hBitmap);
}


//--------------------------------------------------------------------------------------------------
//  Name : WriteTitle
//---------------------------------------------------------------------------------------------------
void CLegendBitmap::WriteTitle()
{
    ILightingUnits* ls = static_cast<ILightingUnits*> (GetCOREInterface(ILIGHT_UNITS_FO_INTERFACE));    

    TCHAR buf[1024];

    if     (m_pPseudoMap->GetUsedQuantity() == PseudoMap::kQuantity_Illuminance)
    {
        _stprintf(buf, "%s (%s)", GetString(IDS_QUANTITY_ILLUMINANCE), ls->GetIlluminanceUnits());
        WriteText(0,0,buf);
    }   
    else if(m_pPseudoMap->GetUsedQuantity() == PseudoMap::kQuantity_Luminance) 
    {
        _stprintf(buf, "%s (%s)", GetString(IDS_QUANTITY_LUMINANCE), ls->GetLuminanceUnits());
        WriteText(0,0,buf);
    }        
}


//--------------------------------------------------------------------------------------------------
//  Name : WriteValues
//--------------------------------------------------------------------------------------------------
void CLegendBitmap::WriteValues()
{
    float fMinimum = 0;
    float fMaximum = 0;

    TCHAR aBuffer[1024];

    int iLeftOffset  = (m_pTheBitmap->Width() - (m_pTheBitmap->Width() * PERCENTAGE_USED_BY_GRADIENT_IN_LEGEND)) / 2;
    int iRightOffset = m_pTheBitmap->Width() - iLeftOffset;

    
    // Write first element
    m_pPseudoMap->GetEnergyRangeFromIndex(0, fMinimum, fMaximum);
    _stprintf(aBuffer, "%.2f", fMinimum);
    WriteText(iLeftOffset, m_pTheBitmap->Height() / 4, aBuffer);


    // Write last element
    m_pPseudoMap->GetEnergyRangeFromIndex(NB_PSEUDO_COLORS-1, fMinimum, fMaximum);
     _stprintf(aBuffer, "%.2f", fMaximum);
    WriteText(iRightOffset, m_pTheBitmap->Height() / 4, aBuffer, DT_RIGHT);

    int iFraction = (m_pTheBitmap->Width() - (iLeftOffset*2)) / NUMBER_OF_SECTION;

    for (int i = 1; i < NUMBER_OF_SECTION ; i++)
    {
         m_pPseudoMap->GetEnergyRangeFromIndex(NB_PSEUDO_COLORS * ((float)i / (float)NUMBER_OF_SECTION), fMinimum, fMaximum);

         _stprintf(aBuffer, "%.2f", fMinimum);
         WriteText((i*iFraction)+iLeftOffset, m_pTheBitmap->Height() / 4, aBuffer, DT_CENTER);
    }
}


