/*==============================================================================

  file:	        LegendBitmap.h

  author:       Jean-Philippe Morel

  created:	    May 27th 2002

  description:
        This object creates a legend in a bitmap. It represents the range of colors
        display in the result of a pseudo color render and also inform the user 
        about the unit used.

  modified:	


© 2002 Autodesk
==============================================================================*/
#ifndef _LEGENDBITMAP_H_
#define _LEGENDBITMAP_H_

#include <max.h>
#include "pseudoMap.h" 
#include "resource.h"
#include "bmmlib.h"   // For the legend bitmap
#include "bitmap.h"   // For the legend bitmap



//--------------------------------------------------------------------------------------------------
//  Class: CLegendBitmap
//--------------------------------------------------------------------------------------------------
class CLegendBitmap 
{
public:

    CLegendBitmap();
    ~CLegendBitmap();
        
    void  BuildLegend   (int iWidth, int iHeight);
    Color GetPixelColor (int iX, int iY);

private:   

    /////////////////////
    // Constants
    //
    static const float PERCENTAGE_USED_BY_GRADIENT_IN_LEGEND ;
    static const int   NUMBER_OF_CARACTERS_PER_LINE         ; // Determine the width of caracters
    static const int   NUMBER_OF_ROW_POSSIBLE               ; // Determine the height of caracters
    static const int   NUMBER_OF_SECTION                    ; 


    Bitmap    * m_pTheBitmap;  // Contain the bitmap legend
    PseudoMap * m_pPseudoMap; 
    
    HDC m_hDC;

	void  BuildGradient         ();
    int   GetDelimiterValue    (int iGradientWidth, int iPosition);
    void  WriteDelimiter       ();		
	void  WriteText            (int x, int y, TCHAR * textBuffer, UINT uiAlign = DT_LEFT);
    void  WriteTitle           ();
    void  WriteValues          ();   
};


#endif // _LEGENDBITMAP_H_