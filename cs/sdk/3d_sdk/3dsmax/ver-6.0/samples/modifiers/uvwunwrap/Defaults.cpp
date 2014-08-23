#include "unwrap.h"
#include <locale.h>

#define UNWRAPCONFIGNAME _T("unwrapuvw.ini")
#define WSDSECTION    _T("Default State")
#define GETFACESELECTIONFROMSTACK		_T("GetFaceSelectionFromStack")
#define FALLOFFSTR						_T("FalloffStr")
#define WINDOWPOSX1						_T("WindposX1")
#define WINDOWPOSY1						_T("WindposY1")
#define WINDOWPOSX2						_T("WindposX2")
#define WINDOWPOSY2						_T("WindposY2")
#define LOCKASPECT						_T("LockAspect")
#define SHOWMAP							_T("ShowMap")
#define FORCEUPDATE						_T("ForceUpdate")
#define RENDERW							_T("RenderW")
#define RENDERH							_T("RenderH")
#define SHOWVERTS						_T("ShowVerts")
#define USEBITMAPRES					_T("UseBitmapRes")
#define PIXELSNAP						_T("PixelSnap")


#define CHANNEL							_T("Channel")
#define FALLOFF							_T("Falloff")
#define FALLOFFSPACE					_T("FalloffSpace")

#define NORMALSPACING					_T("NormalMapSpacing")
#define NORMALNORMALIZE					_T("NormalMapNormalize")
#define NORMALROTATE					_T("NormalMapRotate")
#define NORMALALIGNWIDTH				_T("NormalMapAlignWidth")
#define NORMALMETHOD					_T("NormalMapMethod")

#define FLATTENANGLE					_T("FlattenMapAngle")
#define FLATTENSPACING					_T("FlattenMapSpacing")
#define FLATTENNORMALIZE				_T("FlattenMapNormalize")
#define FLATTENROTATE					_T("FlattenMapRotate")
#define FLATTENCOLLAPSE					_T("FlattenMapCollapse")

#define UNFOLDNORMALIZE					_T("UnfoldMapNormalize")
#define UNFOLDMETHOD					_T("UnfoldMapMethod")

#define STITCHBIAS						_T("StitchBias")
#define STITCHALIGN						_T("StitchAlign")

#define TILEON							_T("TileOn")
#define TILEBRIGHTNESS					_T("TileBrightness")
#define TILELIMIT						_T("TileLimit")

#define SOFTSELLIMIT					_T("SoftSelLimit")
#define SOFTSELRANGE					_T("SoftSelRange")

#define GEOMELEMENTMODE					_T("GeomElementMode")
#define PLANARTHRESHOLD					_T("PlanarThreshold")
#define PLANARMODE						_T("PlanarMode")
#define IGNOREBACKFACECULL				_T("IgnoreBackFaceCull")
#define OLDSELMETHOD					_T("OldSelectionMethod")

	
#define TVELEMENTMODE					_T("TVElementMode")
#define ALWAYSEDIT						_T("AlwaysEdit")

#define PACKMETHOD						_T("PackMethod")
#define PACKSPACING						_T("Packpacing")
#define PACKNORMALIZE					_T("PackNormalize")
#define PACKROTATE						_T("PackRotate")
#define PACKCOLLAPSE					_T("PackCollapse")

#define FILLMODE						_T("FaceFillMode")
#define DISPLAYOPENEDGES				_T("DisplayOpenEdges")
#define UVEDGEMMODE						_T("UVEdgeMode")
#define OPENEDGEMMODE					_T("OpenEdgeMode")
#define DISPLAYHIDDENEDGES				_T("DisplayHiddenEdges")

#define SKETCHSELMODE					_T("SketchSelMode")
#define SKETCHTYPE						_T("SketchType")
#define SKETCHINTERACTIVE				_T("SketchInteactiveMode")
#define SKETCHDISPLAYPOINTS				_T("SketchDisplayPoints")

#define HITSIZE							_T("HitSize")

#define RESETSELONPIVOT					_T("ResetSelOnPivot")
#define POLYMODE						_T("PolygonMode")
#define ALLOWSELECTIONINSIDEGIZMO		_T("AllowSelectionInsideGizmo")

#define WELDTHRESHOLD					_T("WeldThreshold")
#define CONSTANTUPDATE					_T("ConstantUpdate")
#define MIDPIXELSNAP					_T("MidPixelSnap")

#define LINECOLOR						_T("LineColor")
#define SELCOLOR						_T("SelColor")
#define OPENEDGECOLOR					_T("OpenEdgeColor")
#define HANDLECOLOR						_T("HandleColor")
#define TRANSFORMGIZMOCOLOR				_T("TransformGizmoColor")

#define SHOWSHARED						_T("ShowShared")
#define SHAREDCOLOR						_T("SharedColor")

#define ICONLIST						_T("DisplayIconList")

#define SYNCSELECTION					_T("SyncSelection")

#define BRIGHTCENTERTILE				_T("BrightnessAffectsCenterTile")
#define BLENDTILE						_T("BlendTilesToBackground")
#define PAINTSIZE						_T("PaintSelectSize")

#define TICKSIZE						_T("TickSize")

//new
#define GRIDSIZE						_T("GridSize")
#define GRIDSNAP						_T("GridSnap")
#define GRIDVISIBLE						_T("GridVisible")
#define GRIDSTR							_T("GridStr")
#define AUTOMAP							_T("AutoBackground")

#define ENABLESOFTSELECTION				_T("EnableSoftSelection")


//5.1.05
#define AUTOBACKGROUND					_T("EnableAutoBackground")

//5.1.06
#define RELAXAMOUNT						_T("RelaxAmount")
#define RELAXITERATIONS					_T("RelaxIterations")
#define RELAXBOUNDARY					_T("RelaxBoundary")
#define RELAXCORNER						_T("RelaxCorner")


// GetCfgFilename()
void UnwrapMod::GetCfgFilename( TCHAR *filename ) 
	{
	_tcscpy(filename,TheManager->GetDir(APP_PLUGCFG_DIR));
	int len = _tcslen(filename);
	if (len) 
		{
	   if (_tcscmp(&filename[len-1],_T("\\")))
		 _tcscat(filename,_T("\\"));

		}
	_tcscat(filename,UNWRAPCONFIGNAME);
	}
	
void	UnwrapMod::fnSetAsDefaults()
{
	windowPos.length = sizeof(WINDOWPLACEMENT); 
	GetWindowPlacement(hWnd,&windowPos);

	TCHAR filename[MAX_PATH];
	TSTR str;

	GetCfgFilename(filename);
	
	char* saved_lc = NULL;
	char* lc = ::setlocale(LC_NUMERIC, NULL); // query  
	if (lc)  
 		saved_lc = ::strdup(lc);  
	lc = ::setlocale(LC_NUMERIC, "C");  
   
   

	str.printf("%d",getFaceSelectionFromStack);
	WritePrivateProfileString(WSDSECTION,GETFACESELECTIONFROMSTACK,str,filename);

	str.printf("%f",falloffStr);
	WritePrivateProfileString(WSDSECTION,FALLOFFSTR,str,filename);

	
	str.printf("%d",windowPos.rcNormalPosition.left);
	WritePrivateProfileString(WSDSECTION,WINDOWPOSX1,str,filename);
	str.printf("%d",windowPos.rcNormalPosition.bottom);
	WritePrivateProfileString(WSDSECTION,WINDOWPOSY1,str,filename);
	str.printf("%d",windowPos.rcNormalPosition.right);
	WritePrivateProfileString(WSDSECTION,WINDOWPOSX2,str,filename);
	str.printf("%d",windowPos.rcNormalPosition.top);
	WritePrivateProfileString(WSDSECTION,WINDOWPOSY2,str,filename);


	str.printf("%d",forceUpdate);
	WritePrivateProfileString(WSDSECTION,FORCEUPDATE,str,filename);

	str.printf("%d",lockAspect);
	WritePrivateProfileString(WSDSECTION,LOCKASPECT,str,filename);

	str.printf("%d",showMap);
	WritePrivateProfileString(WSDSECTION,SHOWMAP,str,filename);

	str.printf("%d",showVerts);
	WritePrivateProfileString(WSDSECTION,SHOWVERTS,str,filename);

	str.printf("%d",rendW);
	WritePrivateProfileString(WSDSECTION,RENDERW,str,filename);
	str.printf("%d",rendH);
	WritePrivateProfileString(WSDSECTION,RENDERH,str,filename);
	str.printf("%d",useBitmapRes);
	WritePrivateProfileString(WSDSECTION,USEBITMAPRES,str,filename);

	str.printf("%d",pixelSnap);
	WritePrivateProfileString(WSDSECTION,PIXELSNAP,str,filename);

	str.printf("%d",channel);
	WritePrivateProfileString(WSDSECTION,CHANNEL,str,filename);

	str.printf("%d",falloff);
	WritePrivateProfileString(WSDSECTION,FALLOFF,str,filename);
	str.printf("%d",falloffSpace);
	WritePrivateProfileString(WSDSECTION,FALLOFFSPACE,str,filename);

	str.printf("%f",normalSpacing);
	WritePrivateProfileString(WSDSECTION,NORMALSPACING,str,filename);
	str.printf("%d",normalNormalize);
	WritePrivateProfileString(WSDSECTION,NORMALNORMALIZE,str,filename);
	str.printf("%d",normalRotate);
	WritePrivateProfileString(WSDSECTION,NORMALROTATE,str,filename);
	str.printf("%d",normalAlignWidth);
	WritePrivateProfileString(WSDSECTION,NORMALALIGNWIDTH,str,filename);
	str.printf("%d",normalMethod);
	WritePrivateProfileString(WSDSECTION,NORMALMETHOD,str,filename);


	str.printf("%f",flattenAngleThreshold);
	WritePrivateProfileString(WSDSECTION,FLATTENANGLE,str,filename);
	str.printf("%f",flattenSpacing);
	WritePrivateProfileString(WSDSECTION,FLATTENSPACING,str,filename);
	str.printf("%d",flattenNormalize);
	WritePrivateProfileString(WSDSECTION,FLATTENNORMALIZE,str,filename);
	str.printf("%d",flattenRotate);
	WritePrivateProfileString(WSDSECTION,FLATTENROTATE,str,filename);
	str.printf("%d",flattenCollapse);
	WritePrivateProfileString(WSDSECTION,FLATTENCOLLAPSE,str,filename);

	str.printf("%d",unfoldNormalize);
	WritePrivateProfileString(WSDSECTION,UNFOLDNORMALIZE,str,filename);
	str.printf("%d",unfoldMethod);
	WritePrivateProfileString(WSDSECTION,UNFOLDMETHOD,str,filename);

	str.printf("%d",bStitchAlign);
	WritePrivateProfileString(WSDSECTION,STITCHBIAS,str,filename);
	str.printf("%f",fStitchBias);
	WritePrivateProfileString(WSDSECTION,STITCHALIGN,str,filename);

	str.printf("%d",bTile);
	WritePrivateProfileString(WSDSECTION,TILEON,str,filename);
	str.printf("%f",fContrast);
	WritePrivateProfileString(WSDSECTION,TILEBRIGHTNESS,str,filename);
	str.printf("%d",iTileLimit);
	WritePrivateProfileString(WSDSECTION,TILELIMIT,str,filename);

	str.printf("%d",limitSoftSel);
	WritePrivateProfileString(WSDSECTION,SOFTSELLIMIT,str,filename);
	str.printf("%d",limitSoftSelRange);
	WritePrivateProfileString(WSDSECTION,SOFTSELRANGE,str,filename);

	str.printf("%d",geomElemMode);
	WritePrivateProfileString(WSDSECTION,GEOMELEMENTMODE,str,filename);
	str.printf("%f",planarThreshold);
	WritePrivateProfileString(WSDSECTION,PLANARTHRESHOLD,str,filename);
	str.printf("%d",planarMode);
	WritePrivateProfileString(WSDSECTION,PLANARMODE,str,filename);
	str.printf("%d",ignoreBackFaceCull);
	WritePrivateProfileString(WSDSECTION,IGNOREBACKFACECULL,str,filename);
	str.printf("%d",oldSelMethod);
	WritePrivateProfileString(WSDSECTION,OLDSELMETHOD,str,filename);

	str.printf("%d",tvElementMode);
	WritePrivateProfileString(WSDSECTION,TVELEMENTMODE,str,filename);
	str.printf("%d",alwaysEdit);
	WritePrivateProfileString(WSDSECTION,ALWAYSEDIT,str,filename);

	str.printf("%d",packMethod);
	WritePrivateProfileString(WSDSECTION,PACKMETHOD,str,filename);
	str.printf("%f",packSpacing);
	WritePrivateProfileString(WSDSECTION,PACKSPACING,str,filename);
	str.printf("%d",packNormalize);
	WritePrivateProfileString(WSDSECTION,PACKNORMALIZE,str,filename);
	str.printf("%d",packRotate);
	WritePrivateProfileString(WSDSECTION,PACKROTATE,str,filename);
	str.printf("%d",packFillHoles);
	WritePrivateProfileString(WSDSECTION,PACKCOLLAPSE,str,filename);

	str.printf("%d",fillMode);
	WritePrivateProfileString(WSDSECTION,FILLMODE,str,filename);
	str.printf("%d",displayOpenEdges);
	WritePrivateProfileString(WSDSECTION,DISPLAYOPENEDGES,str,filename);
	str.printf("%d",uvEdgeMode);
	WritePrivateProfileString(WSDSECTION,UVEDGEMMODE,str,filename);
	str.printf("%d",openEdgeMode);
	WritePrivateProfileString(WSDSECTION,OPENEDGEMMODE,str,filename);
	str.printf("%d",displayHiddenEdges);
	WritePrivateProfileString(WSDSECTION,DISPLAYHIDDENEDGES,str,filename);

	str.printf("%d",sketchSelMode);
	WritePrivateProfileString(WSDSECTION,SKETCHSELMODE,str,filename);
	str.printf("%d",sketchType);
	WritePrivateProfileString(WSDSECTION,SKETCHTYPE,str,filename);
	str.printf("%d",sketchInteractiveMode);
	WritePrivateProfileString(WSDSECTION,SKETCHINTERACTIVE,str,filename);
	str.printf("%d",sketchDisplayPoints);
	WritePrivateProfileString(WSDSECTION,SKETCHDISPLAYPOINTS,str,filename);


	str.printf("%d",hitSize);
	WritePrivateProfileString(WSDSECTION,HITSIZE,str,filename);

	str.printf("%d",resetPivotOnSel);
	WritePrivateProfileString(WSDSECTION,RESETSELONPIVOT,str,filename);

	str.printf("%d",polyMode);
	WritePrivateProfileString(WSDSECTION,POLYMODE,str,filename);

	str.printf("%d",allowSelectionInsideGizmo);
	WritePrivateProfileString(WSDSECTION,ALLOWSELECTIONINSIDEGIZMO,str,filename);

	str.printf("%f",weldThreshold);
	WritePrivateProfileString(WSDSECTION,WELDTHRESHOLD,str,filename);

	str.printf("%d",update);
	WritePrivateProfileString(WSDSECTION,CONSTANTUPDATE,str,filename);

	str.printf("%d",midPixelSnap);
	WritePrivateProfileString(WSDSECTION,MIDPIXELSNAP,str,filename);

/*
	str.printf("%d %d %d ",(int) GetRValue(lineColor),(int) GetGValue(lineColor),(int) GetBValue(lineColor));
	WritePrivateProfileString(WSDSECTION,LINECOLOR,str,filename);

	str.printf("%d %d %d ",(int) GetRValue(selColor),(int) GetGValue(selColor),(int) GetBValue(selColor));
	WritePrivateProfileString(WSDSECTION,SELCOLOR,str,filename);

	str.printf("%d %d %d ",(int) GetRValue(openEdgeColor),(int) GetGValue(openEdgeColor),(int) GetBValue(openEdgeColor));
	WritePrivateProfileString(WSDSECTION,OPENEDGECOLOR,str,filename);

	str.printf("%d %d %d ",(int) GetRValue(handleColor),(int) GetGValue(handleColor),(int) GetBValue(handleColor));
	WritePrivateProfileString(WSDSECTION,HANDLECOLOR,str,filename);

	str.printf("%d %d %d ",(int) GetRValue(freeFormColor),(int) GetGValue(freeFormColor),(int) GetBValue(freeFormColor));
	WritePrivateProfileString(WSDSECTION,TRANSFORMGIZMOCOLOR,str,filename);

	str.printf("%d %d %d ",(int) GetRValue(sharedColor),(int) GetGValue(sharedColor),(int) GetBValue(sharedColor));
	WritePrivateProfileString(WSDSECTION,SHAREDCOLOR,str,filename);
*/

	str.printf("%d",showShared );
	WritePrivateProfileString(WSDSECTION,SHOWSHARED,str,filename);

	str.printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
				showIconList[1],showIconList[2],showIconList[3],showIconList[4],showIconList[5],showIconList[6],showIconList[7],showIconList[8],showIconList[9],showIconList[10],
				showIconList[11],showIconList[12],showIconList[13],showIconList[14],showIconList[15],showIconList[16],showIconList[17],showIconList[18],showIconList[19],showIconList[20],
				showIconList[21],showIconList[22],showIconList[23],showIconList[24],showIconList[25],showIconList[26],showIconList[27],showIconList[28],showIconList[29],showIconList[30],
				showIconList[31]
				);
	WritePrivateProfileString(WSDSECTION,ICONLIST,str,filename);


	str.printf("%d",syncSelection );
	WritePrivateProfileString(WSDSECTION,SYNCSELECTION,str,filename);

	str.printf("%d",brightCenterTile );
	WritePrivateProfileString(WSDSECTION,BRIGHTCENTERTILE,str,filename);

	str.printf("%d",blendTileToBackGround );
	WritePrivateProfileString(WSDSECTION,BLENDTILE,str,filename);

	str.printf("%d",paintSize );
	WritePrivateProfileString(WSDSECTION,PAINTSIZE,str,filename);

	str.printf("%d",tickSize );
	WritePrivateProfileString(WSDSECTION,TICKSIZE,str,filename);

//new	
	str.printf("%f",gridSize );
	WritePrivateProfileString(WSDSECTION,GRIDSIZE,str,filename);

	str.printf("%d",gridSnap );
	WritePrivateProfileString(WSDSECTION,GRIDSNAP,str,filename);

	str.printf("%d",gridVisible );
	WritePrivateProfileString(WSDSECTION,GRIDVISIBLE,str,filename);

	str.printf("%f",gridStr );
	WritePrivateProfileString(WSDSECTION,GRIDSTR,str,filename);

	str.printf("%d",autoMap );
	WritePrivateProfileString(WSDSECTION,AUTOMAP,str,filename);

	str.printf("%d",enableSoftSelection );
	WritePrivateProfileString(WSDSECTION,ENABLESOFTSELECTION,str,filename);

	//5.1.05
	str.printf("%d",this->autoBackground);
	WritePrivateProfileString(WSDSECTION,AUTOBACKGROUND,str,filename);

//5.1.06
	str.printf("%f",this->relaxAmount);
	WritePrivateProfileString(WSDSECTION,RELAXAMOUNT,str,filename);

	str.printf("%d",this->relaxIteration);
	WritePrivateProfileString(WSDSECTION,RELAXITERATIONS,str,filename);

	str.printf("%d",this->relaxBoundary);
	WritePrivateProfileString(WSDSECTION,RELAXBOUNDARY,str,filename);

	str.printf("%d",this->relaxSaddle);
	WritePrivateProfileString(WSDSECTION,RELAXCORNER,str,filename);


	if (saved_lc)  
 	{  
 		lc = ::setlocale(LC_NUMERIC, saved_lc);  
 		free(saved_lc);  
 		saved_lc = NULL;  
	}  	
	

}

void	UnwrapMod::fnLoadDefaults()
{
	TCHAR filename[MAX_PATH];
	GetCfgFilename(filename);
	TCHAR str[MAX_PATH];
	TSTR def("DISCARD");


	char* saved_lc = NULL;
	char* lc = ::setlocale(LC_NUMERIC, NULL); // query  
	if (lc)  
 		saved_lc = ::strdup(lc);  
	lc = ::setlocale(LC_NUMERIC, "C");  
	

	int res = GetPrivateProfileString(WSDSECTION,GETFACESELECTIONFROMSTACK,def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&getFaceSelectionFromStack);
		
	res = GetPrivateProfileString(WSDSECTION,FALLOFFSTR,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&falloffStr);

	int full = 0;
	res = GetPrivateProfileString(WSDSECTION,WINDOWPOSX1,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		sscanf(str,"%d",&windowPos.rcNormalPosition.left);
		full++;
		}
	res = GetPrivateProfileString(WSDSECTION,WINDOWPOSY1,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		sscanf(str,"%d",&windowPos.rcNormalPosition.bottom);
		full++;
		}
	res = GetPrivateProfileString(WSDSECTION,WINDOWPOSX2,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{	
		sscanf(str,"%d",&windowPos.rcNormalPosition.right);
		full++;
		}
	res = GetPrivateProfileString(WSDSECTION,WINDOWPOSY2,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		sscanf(str,"%d",&windowPos.rcNormalPosition.top);
		full++;
		}
	if (full == 4) windowPos.length = sizeof(WINDOWPLACEMENT); 



	res = GetPrivateProfileString(WSDSECTION,FORCEUPDATE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&forceUpdate);

	res = GetPrivateProfileString(WSDSECTION,LOCKASPECT,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&lockAspect);

	res = GetPrivateProfileString(WSDSECTION,SHOWMAP,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&showMap);
	res = GetPrivateProfileString(WSDSECTION,SHOWVERTS,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&showVerts);
	res = GetPrivateProfileString(WSDSECTION,RENDERW,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&rendW);
	res = GetPrivateProfileString(WSDSECTION,RENDERH,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&rendH);
	res = GetPrivateProfileString(WSDSECTION,USEBITMAPRES,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&useBitmapRes);
	res = GetPrivateProfileString(WSDSECTION,PIXELSNAP,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&pixelSnap);

	res = GetPrivateProfileString(WSDSECTION,CHANNEL,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&channel);

	res = GetPrivateProfileString(WSDSECTION,FALLOFF,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&falloff);
	res = GetPrivateProfileString(WSDSECTION,FALLOFFSPACE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&falloffSpace);

	res = GetPrivateProfileString(WSDSECTION,NORMALSPACING,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&normalSpacing);
	res = GetPrivateProfileString(WSDSECTION,NORMALNORMALIZE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&normalNormalize);
	res = GetPrivateProfileString(WSDSECTION,NORMALROTATE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&normalRotate);
	res = GetPrivateProfileString(WSDSECTION,NORMALALIGNWIDTH,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&normalAlignWidth);
	res = GetPrivateProfileString(WSDSECTION,NORMALMETHOD,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&normalMethod);


	res = GetPrivateProfileString(WSDSECTION,FLATTENANGLE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&flattenAngleThreshold);
	res = GetPrivateProfileString(WSDSECTION,FLATTENSPACING,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&flattenSpacing);
	res = GetPrivateProfileString(WSDSECTION,FLATTENNORMALIZE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&flattenNormalize);
	res = GetPrivateProfileString(WSDSECTION,FLATTENROTATE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&flattenRotate);
	res = GetPrivateProfileString(WSDSECTION,FLATTENCOLLAPSE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&flattenCollapse);

	res = GetPrivateProfileString(WSDSECTION,UNFOLDNORMALIZE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&unfoldNormalize);
	res = GetPrivateProfileString(WSDSECTION,UNFOLDMETHOD,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&unfoldMethod);


	res = GetPrivateProfileString(WSDSECTION,STITCHBIAS,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&bStitchAlign);
	res = GetPrivateProfileString(WSDSECTION,STITCHALIGN,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&fStitchBias);

	res = GetPrivateProfileString(WSDSECTION,TILEON,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&bTile);
	res = GetPrivateProfileString(WSDSECTION,TILEBRIGHTNESS,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&fContrast);
	res = GetPrivateProfileString(WSDSECTION,TILELIMIT,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&iTileLimit);


	res = GetPrivateProfileString(WSDSECTION,SOFTSELLIMIT,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&limitSoftSel);
	res = GetPrivateProfileString(WSDSECTION,SOFTSELRANGE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&limitSoftSelRange);


	res = GetPrivateProfileString(WSDSECTION,GEOMELEMENTMODE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&geomElemMode);
	res = GetPrivateProfileString(WSDSECTION,PLANARTHRESHOLD,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&planarThreshold);
	res = GetPrivateProfileString(WSDSECTION,PLANARMODE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&planarMode);
	res = GetPrivateProfileString(WSDSECTION,IGNOREBACKFACECULL,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&ignoreBackFaceCull);
	res = GetPrivateProfileString(WSDSECTION,OLDSELMETHOD,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&oldSelMethod);

	res = GetPrivateProfileString(WSDSECTION,TVELEMENTMODE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&tvElementMode);
	res = GetPrivateProfileString(WSDSECTION,ALWAYSEDIT,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&alwaysEdit);


	res = GetPrivateProfileString(WSDSECTION,PACKMETHOD,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&packMethod);
	res = GetPrivateProfileString(WSDSECTION,PACKSPACING,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&packSpacing);
	res = GetPrivateProfileString(WSDSECTION,PACKNORMALIZE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&packNormalize);
	res = GetPrivateProfileString(WSDSECTION,PACKROTATE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&packRotate);
	res = GetPrivateProfileString(WSDSECTION,PACKCOLLAPSE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&packFillHoles);


	res = GetPrivateProfileString(WSDSECTION,FILLMODE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&fillMode);
	res = GetPrivateProfileString(WSDSECTION,DISPLAYOPENEDGES,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&displayOpenEdges);
	res = GetPrivateProfileString(WSDSECTION,UVEDGEMMODE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&uvEdgeMode);
	res = GetPrivateProfileString(WSDSECTION,OPENEDGEMMODE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&openEdgeMode);
	res = GetPrivateProfileString(WSDSECTION,DISPLAYHIDDENEDGES,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&displayHiddenEdges);


	res = GetPrivateProfileString(WSDSECTION,SKETCHSELMODE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&sketchSelMode);
	res = GetPrivateProfileString(WSDSECTION,SKETCHTYPE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&sketchType);
	res = GetPrivateProfileString(WSDSECTION,SKETCHINTERACTIVE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&sketchInteractiveMode);
	res = GetPrivateProfileString(WSDSECTION,SKETCHDISPLAYPOINTS,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&sketchDisplayPoints);

	res = GetPrivateProfileString(WSDSECTION,HITSIZE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&hitSize);

	res = GetPrivateProfileString(WSDSECTION,RESETSELONPIVOT,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&resetPivotOnSel);

	res = GetPrivateProfileString(WSDSECTION,POLYMODE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&polyMode);

	res = GetPrivateProfileString(WSDSECTION,ALLOWSELECTIONINSIDEGIZMO,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&allowSelectionInsideGizmo);

	res = GetPrivateProfileString(WSDSECTION,WELDTHRESHOLD,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&weldThreshold);

	res = GetPrivateProfileString(WSDSECTION,CONSTANTUPDATE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&update);

	res = GetPrivateProfileString(WSDSECTION,MIDPIXELSNAP,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&midPixelSnap);

/*
	res = GetPrivateProfileString(WSDSECTION,LINECOLOR,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		int r,g,b;
		sscanf(str,"%d %d %d",&r,&g,&b);
		lineColor = RGB(r,g,b);
		}

	res = GetPrivateProfileString(WSDSECTION,SELCOLOR,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		int r,g,b;
		sscanf(str,"%d %d %d",&r,&g,&b);
		selColor = RGB(r,g,b);
		}

	res = GetPrivateProfileString(WSDSECTION,OPENEDGECOLOR,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		int r,g,b;
		sscanf(str,"%d %d %d",&r,&g,&b);
		openEdgeColor = RGB(r,g,b);
		}

	res = GetPrivateProfileString(WSDSECTION,HANDLECOLOR,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		int r,g,b;
		sscanf(str,"%d %d %d",&r,&g,&b);
		handleColor = RGB(r,g,b);
		}

	res = GetPrivateProfileString(WSDSECTION,TRANSFORMGIZMOCOLOR,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		int r,g,b;
		sscanf(str,"%d %d %d",&r,&g,&b);
		freeFormColor = RGB(r,g,b);
		}

	res = GetPrivateProfileString(WSDSECTION,SHAREDCOLOR,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		int r,g,b;
		sscanf(str,"%d %d %d",&r,&g,&b);
		sharedColor = RGB(r,g,b);
		}
*/

	res = GetPrivateProfileString(WSDSECTION,SHOWSHARED,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&showShared);

	res = GetPrivateProfileString(WSDSECTION,ICONLIST,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) 
		{
		int sIconList[32];
		sscanf(str,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
				&sIconList[1],&sIconList[2],&sIconList[3],&sIconList[4],&sIconList[5],&sIconList[6],&sIconList[7],&sIconList[8],&sIconList[9],&sIconList[10],
				&sIconList[11],&sIconList[12],&sIconList[13],&sIconList[14],&sIconList[15],&sIconList[16],&sIconList[17],&sIconList[18],&sIconList[19],&sIconList[20],
				&sIconList[21],&sIconList[22],&sIconList[23],&sIconList[24],&sIconList[25],&sIconList[26],&sIconList[27],&sIconList[28],&sIconList[29],&sIconList[30],
				&sIconList[31]
				);
		for (int i = 1; i < 32; i++)
			showIconList.Set(i,sIconList[i]);
		}


	res = GetPrivateProfileString(WSDSECTION,SYNCSELECTION,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&syncSelection);

	res = GetPrivateProfileString(WSDSECTION,BRIGHTCENTERTILE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&brightCenterTile);

	res = GetPrivateProfileString(WSDSECTION,BLENDTILE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&blendTileToBackGround);

	res = GetPrivateProfileString(WSDSECTION,PAINTSIZE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&paintSize);

	res = GetPrivateProfileString(WSDSECTION,TICKSIZE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&tickSize);


//new	
	res = GetPrivateProfileString(WSDSECTION,GRIDSIZE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&gridSize);

	res = GetPrivateProfileString(WSDSECTION,GRIDSNAP,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&gridSnap);

	res = GetPrivateProfileString(WSDSECTION,GRIDVISIBLE,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&gridVisible);

	res = GetPrivateProfileString(WSDSECTION,GRIDSTR,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&gridStr);

	res = GetPrivateProfileString(WSDSECTION,AUTOMAP,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&autoMap);

	res = GetPrivateProfileString(WSDSECTION,ENABLESOFTSELECTION,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&enableSoftSelection);

//5.1.04
	res = GetPrivateProfileString(WSDSECTION,AUTOBACKGROUND,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&autoBackground);

//5.1.06
	res = GetPrivateProfileString(WSDSECTION,RELAXAMOUNT,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%f",&relaxAmount);

	res = GetPrivateProfileString(WSDSECTION,RELAXITERATIONS,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&relaxIteration);

	res = GetPrivateProfileString(WSDSECTION,RELAXBOUNDARY,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&relaxBoundary);

	res = GetPrivateProfileString(WSDSECTION,RELAXCORNER,  def,str,MAX_PATH,filename);
	if ((res) && (_tcscmp(str,def))) sscanf(str,"%d",&relaxSaddle);


	if (saved_lc)  
 	{  
 		lc = ::setlocale(LC_NUMERIC, saved_lc);  
 		free(saved_lc);  
 		saved_lc = NULL;  
	}  	


	InvalidateView();

}



