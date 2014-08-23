//////////////////////////////////////////////////////////////////////
//
//	Shader UI Utilities
//
#include "shadersPch.h"
#include "shadersRc.h"
#include "shaders.h"
#include "shaderUtil.h"
#include "iColorMan.h"
#include "toneop.h"

static const float Pi = 3.1415926f;
static const float Pi2 = Pi * Pi;
static const float Pi2Ivr = 1.0f / Pi2;
static const float PiIvr = 1.0f / Pi;


Color OrenNayarIllum( Point3& N, Point3& L, Point3& V, float rough, Color& rho, float* pDiffIntensOut, float NL )
{
//	float a = NL >= -1.0f ? float( acos( NL / Len(L) )) : AngleBetween( N, L );
	// use the non-uniform scale corrected NL
	if ( NL < -1.0f )
		NL = Dot( N, L );
//	float a = float( acos( NL / Len(L) )) ;
	float a = 0.0f;
	if( NL < 0.9999f )
		a = acosf( NL );
	a = Bound( a, -Pi*0.49f, Pi*0.49f );
	float NV = Dot( N, V );
	// need this to accomodate double sided materials
	if( NV < 0.0f ){
		NV = -NV;
		V = -V;
	}
	float b = 0.0f;
	if( NV < 0.9999f )
	   b = acosf( NV );
	MinMax( b, a ); // b gets min, a gets max


	//N.V is the length of the projection of v onto n; times N is a vector along N of that length
	// V - that pt gives a tangent vector in the plane of N, for measuring phi
	Point3 tanV = V - N * NV;
	Point3 tanL = L - N * NL;
	float w = Len( tanV ) * Len( tanL );
	float cosDPhi = (Abs(w) < 1e-4) ? 1.0f : Dot( tanV, tanL ) / w;
	cosDPhi = Bound( cosDPhi, -1.0f, 1.0f );
	

//	float bCube = (cosDPhi >= 0.0f) ? 0.0f : Cube( 2.0f * b * PiIvr );
	float bCube = (cosDPhi >= 0.0f) ? Cube( 2.0f * -b * PiIvr ) : Cube( 2.0f * b * PiIvr );

	// these three can be pre-calc'd for speed
	float sigma2 = Sqr( rough );
	float sigma3 = sigma2 / (sigma2 + 0.09f);
	float c1 = 1.0f - 0.5f * (sigma2 / (sigma2 + 0.33f));

	float c2 = 0.45f * sigma3 * (float(sin(a)) - bCube);
	float c3 = 0.125f * sigma3 * Sqr( 4.0f * a * b * Pi2Ivr );
	float tanB = float( tan(b) );
	float tanAB = float( tan( (a+b) * 0.5f ));
	tanB = Bound( tanB, -100.0f, 100.0f );
	tanAB = Bound( tanAB, -100.0f, 100.0f );

	Color o;
	float l1 = ( c1 + c2 * cosDPhi * tanB  + c3 * (1.0f - Abs( cosDPhi )) * tanAB );
	float l2 = 0.17f * (sigma2 / (sigma2 + 0.13f)) * ( 1.0f - cosDPhi * Sqr( 2.0f * b * PiIvr ));
	if( pDiffIntensOut )
		*pDiffIntensOut = l1+l2;
	o.r =  l1 * rho.r + l2 * Sqr( rho.r ); 
	o.g =  l1 * rho.g + l2 * Sqr( rho.g ); 
	o.b =  l1 * rho.b + l2 * Sqr( rho.b ); 
	return UBound( o );

}

// perpendicular to N, in the U (reference) direction
Point3 GetTangent( ShadeContext &sc, int uvChan )
{
//	Point3 basisVecs[ 3 ];
//	sc.DPdUVW( basisVecs, uvChan ); // 0 is vtxclr, 1..n is uv channels, max_meshmaps in mesh.h
//	Point3 U = Normalize( basisVecs[0] );
	
	Point3 U = sc.VectorFrom( Point3( 0.01f, 0.0f, 1.0f ), REF_OBJECT );
//Retry:
	U = Normalize( U );

	Point3 N = sc.Normal();			//assumed normalized

	// the line between the tip of vec[0] and its projection on N is tangent
	float UN = Dot( U, N );
//	if ( Abs(UN) > 0.9999999f ){
//		U = sc.VectorFrom( Point3( 0.01f, 1.0f, 0.0f ), REF_OBJECT ); 
//		goto Retry;
//	}
	Point3 T = U - N * UN;
	T = Normalize( T );
	return T;
}


// specular reflectivity, no colors yet, all vectors assumed normalized 
float GaussHighlight( float gloss, float aniso, float orient,  
					  Point3& N, Point3& V, Point3& L, Point3& T, float* pNL )
{
	float out = 0.0f;

	float asz = (1.0f - gloss) * ALPHA_SZ;
	float ax = ALPHA_MIN + asz;
	float ay = ALPHA_MIN + asz * (1.0f-aniso);
//	DbgAssert( ax >= 0.0f && ay >= 0.0f );
	LBound( ax ); LBound( ay );

	Point3 H = Normalize(L - V); // (L + -V)/2
	float NH = DotProd(N, H);	 
	if (NH > 0.0f) {
		float axy = /* normalizeOn ? ax * ay : */ DEFAULT_GLOSS2;
		float norm = 1.0f / (4.0f * PI * axy );
		float NV = -DotProd(N, V );
		if ( NV <= 0.001f)
			NV = 0.001f;

		float NL = pNL ? *pNL : DotProd( N, L );
		float g = 1.0f / (float)sqrt( NL * NV );
		if ( g > 3.0f ) g = 3.0f;

		// Apply Orientation rotation here
		float or = orient * 180.0f;
		Point3 T1 = T;
		if ( or != 0.0f )
			T1 = RotateVec( T, N, DegToRdn(or));

		// get binormal
		Point3 B = CrossProd( T1, N );

		float x = Dot( H, T1 ) / ax;
		float y = Dot( H, B ) / ay;
		float e = (float)exp( -2.0 * (x*x + y*y) / (1.0+NH) );

		out = norm * g * e;
	}
	return SPEC_MAX * out;	// does not have speclev or light color or kL
}

float GaussHiliteCurve2( float x, float y, float sLevel, float gloss, float aniso )
{
	double axy = DEFAULT_GLOSS2; 
	double asz = (1.0f - gloss) * ALPHA_SZ; 
	double ax = ALPHA_MIN + asz;
	double ay = ALPHA_MIN + asz * (1.0f - aniso) ;
	double ax2 = ax * ax; 
	double ay2 = ay * ay; 
	
	double t, a;
	double l = sqrt( x*x + y*y );
	if ( l == 0.0 ) {
		a = t = 0.0;
	} else {
		x /= float(l);	y /= float(l);
		t = tan( l*PI*0.5 );
		a = x*x/ax2 + y*y/ay2;
	}
	return SPEC_MAX * sLevel*(float)(exp( -(t * t) * a ) / (4.0 * PI * axy));  
}

//////////////////////////////////////////////////////////////////////////////
//
//	Combine Components....Adding & compositing
//
#define COMBINE_ADD(ip) \
	((ip).finalAttenuation * ((ip).ambIllumOut + (ip).diffIllumOut  + (ip).selfIllumOut) \
		+ (ip).specIllumOut + (ip).reflIllumOut + (ip).transIllumOut)

void CombineComponentsAdd( IllumParams& ip )
{
	ip.finalC = COMBINE_ADD(ip); 
}

void CombineComponentsCompShader::ChooseSpecularMethod(TimeValue t, RenderGlobalContext* rgc)
{
	useComposite = true;
	if (rgc == NULL) {
		ToneOperatorInterface* tint = static_cast<ToneOperatorInterface*>(
			GetCOREInterface(TONE_OPERATOR_INTERFACE));
		if (tint != NULL) {
			ToneOperator* top = tint->GetToneOperator();
			if (top != NULL && top->Active(t))
				useComposite = false;
		}
	} else {
		ToneOperator* top = rgc->pToneOp;
		if (top != NULL && top->Active(t))
			useComposite = false;
	}
}

void CombineComponentsCompShader::CombineComponents( ShadeContext &sc, IllumParams& ip )
{
	if (useComposite) {
		Color spec, diff, rem;
		spec = ip.specIllumOut + ip.reflIllumOut;
		rem = 1.0f - spec;
		rem = Bound( rem );
		diff = ip.ambIllumOut + ip.diffIllumOut  + ip.selfIllumOut;
		ip.finalC = spec + ip.finalAttenuation * rem * diff +  rem * ip.transIllumOut; 
	} else {
		ip.finalC = COMBINE_ADD(ip); 
	}
}

BaseInterface* CombineComponentsCompShader::GetInterface(Interface_ID id)
{
	if (id == ISPECULAR_COMPOSITE_SHADER_ID)
		return static_cast<ISpecularCompositeShader*>(this);
	return NULL;
}


//////////////////////////////////////////////////////////////////////////////
//
//	transpColor utility
//
Color transpColor( ULONG type, float opac, Color& filt, Color& diff )
{
	// Compute the color of the transparent filter color
	if ( type == TRANSP_ADD ) { // flags & STDMTL_ADD_TRANSP) {
		float f = 1.0f - opac;
		return Color(f, f, f);   

	} else if ( type == TRANSP_FILTER ) { //flags & STDMTL_FILT_TRANSP ){
		// Transparent Filter color mapping
		if (opac>0.5f) {
			// darken as opac goes ( 0.5--> 1.0)
			// so that max component reaches 0.0f when opac reaches 1.0
			// find max component of filt
			float m = Max(filt);
			float d = 2.0f*(opac-.5f)*m;
			Color fc = filt-d;
			fc = LBound( fc );
			return fc;
		} else {
			// lighten as opac goes ( 0.5--> 0.0)
			// so that min component reaches 1.0f when opac reaches 1.0
			// find min component of filt
			float m = Min(filt);
			float d = (1.0f-2.0f*opac)*(1.0f-m);
			Color fc = filt+d;
			fc = UBound( fc );
			return fc;
		}

	} else {
		// original 3DS transparency 
		Color f = (1.0f-diff);  
		return  (1.0f-opac)*f;
	}

}

//////////////////////////////////////////////////////////////////////////////
//
//	UI utilities
//
static HIMAGELIST hLockButtons = NULL;
extern HINSTANCE hInstance;

// mjm - begin - 5.10.99
class ResourceDelete
{
public:
	ResourceDelete() {}
	~ResourceDelete() { if (hLockButtons) ImageList_Destroy(hLockButtons); }
};

static ResourceDelete theResourceDelete;
// mjm - end

BOOL IsButtonChecked(HWND hWnd,int id)
	{
	ICustButton *iBut;
	BOOL res;
	iBut = GetICustButton(GetDlgItem(hWnd,id));
	res = iBut->IsChecked();
	ReleaseICustButton(iBut);
	return res;
	}

void CheckButton(HWND hWnd,int id, BOOL check) {
	ICustButton *iBut;
	iBut = GetICustButton(GetDlgItem(hWnd,id));
	if( iBut )
		iBut->SetCheck(check);
	ReleaseICustButton(iBut);
	}

void SetupLockButton(HWND hWnd,int id, BOOL check)
	{
	ICustButton *iBut;
	iBut = GetICustButton(GetDlgItem(hWnd,id));
	iBut->SetImage(hLockButtons,0,1,0,1,16,15);
	iBut->SetType(CBT_CHECK);
	ReleaseICustButton(iBut);
	}

void SetupPadLockButton(HWND hWnd,int id, BOOL check) {
	ICustButton *iBut;
	iBut = GetICustButton(GetDlgItem(hWnd,id));
	iBut->SetImage(hLockButtons,2,2,2,2,16,15);
	iBut->SetType(CBT_CHECK);
	ReleaseICustButton(iBut);
	}
 
void LoadStdShaderResources()
	{
	static BOOL loaded=FALSE;
	if (loaded) return;
	loaded = TRUE;	
	HBITMAP hBitmap, hMask;

	hLockButtons = ImageList_Create(16, 15, TRUE, 2, 0);
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DMTL_BUTTONS));
	hMask   = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DMTL_MASKBUTTONS));
	ImageList_Add(hLockButtons,hBitmap,hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	}


//-HiLite Curve Control------------------------------------------------------

static void VertLine(HDC hdc,int x, int ystart, int yend) 
{
	MoveToEx(hdc, x, ystart, NULL); 
	if (ystart <= yend)
		LineTo(hdc, x, yend+1);
	else 
		LineTo(hdc, x, yend-1);
}

void DrawHilite(HDC hdc, Rect& rect, Shader* pShader )
{
int w,h,npts,xcen,ybot,ytop,ylast,i,iy;

	HPEN linePen = (HPEN)GetStockObject(WHITE_PEN);
	HPEN fgPen = CreatePen(PS_SOLID,0,GetCustSysColor(COLOR_BTNFACE));
	HPEN bgPen = CreatePen(PS_SOLID,0,GetCustSysColor(COLOR_BTNSHADOW));

	w = rect.w();
	h = rect.h()-3;
	npts = (w-2)/2;
	xcen = rect.left+npts;
	ybot = rect.top+h;
	ytop = rect.top+2;
	ylast = -1;
	for (i=0; i<npts; i++) {
		float v = pShader->EvalHiliteCurve( (float)i/((float)npts*2.0f) );
		if (v>2.0f) v = 2.0f; // keep iy from wrapping
		iy = ybot-(int)(v*((float)h-2.0f));

		if (iy<ytop) iy = ytop;

		SelectPen(hdc, fgPen);
		VertLine(hdc,xcen+i,ybot,iy);
		VertLine(hdc,xcen-i,ybot,iy);

		if (iy-1>ytop) {
			// Fill in above curve
			SelectPen(hdc,bgPen);
			VertLine(hdc,xcen+i, ytop, iy-1);
			VertLine(hdc,xcen-i, ytop, iy-1);
			}
		if (ylast>=0) {
			SelectPen(hdc,linePen);
			VertLine(hdc,xcen+i-1,iy-1,ylast);
			VertLine(hdc,xcen-i+1,iy-1,ylast);
			}

		ylast = iy;
	}

	SelectObject( hdc, linePen );
	DeleteObject(fgPen);
	DeleteObject(bgPen);
	WhiteRect3D(hdc, rect, 1);
}

LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = GetWindowLongPtr(hwnd,GWL_ID);
	HWND hwParent = GetParent(hwnd);
	ShaderParamDlg *theDlg = (ShaderParamDlg *)GetWindowLongPtr(hwParent, GWLP_USERDATA);
	if (theDlg==NULL) return FALSE;

    switch (msg) {
		case WM_COMMAND: 	
		case WM_MOUSEMOVE: 	
		case WM_LBUTTONUP: 
		case WM_CREATE:
		case WM_DESTROY: 
		break;

		case WM_PAINT: 	
		{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				Shader* pShader = theDlg->GetShader();
				DrawHilite(hdc, rect, pShader );
			}
			EndPaint( hwnd, &ps );
		}													
		break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 

// iso projection of 2 orthogonal highlight curves
void DrawHilite2(HDC hdc, Rect& rect, Shader* pShader, int layer )
{
int w,h,npts,xcen,ybot,ytop,ylast,i,iy, off, vals[200];
float ybr, ybl;

	HPEN linePen = (HPEN)GetStockObject(WHITE_PEN);
	HPEN fgPen = CreatePen(PS_SOLID,0,GetCustSysColor(COLOR_BTNFACE));
	HPEN fg2Pen = CreatePen(PS_SOLID,0,GetCustSysColor(COLOR_BTNHILIGHT));
	HPEN bgPen = CreatePen(PS_SOLID,0,GetCustSysColor(COLOR_BTNSHADOW));

	w = rect.w();
	assert( w/2 < 200 );	// 200 vals saved for visibility
	h = rect.h()-3;
	npts = (w-2)/2;
	off = h / 6;
	float slope = float(h-off-off)/w;
	ybot = rect.top+h;
	ytop = rect.top+2;

	// first the X curve
	ybr = ybl = float(rect.top+h - 2.5* off);
	xcen = rect.left+npts;
	ylast = -1;
	for (i=0; i<npts; i++) {
		float v = pShader->EvalHiliteCurve2( (float)i/((float)npts*2.0f), 0.0f, layer );
		if (v>2.0f) v = 2.0f; // keep iy from wrapping
		iy = (int)(v* 0.6f * ((float)h-2.0f));

		int r = int( ybr + 0.5f );
		if ( r > ybot ) r = ybot;
		int l = int( ybl + 0.5f  );
		if ( l > ybot ) l = ybot;

		int ry = r - iy;
		if (ry<ytop) ry = ytop;
		if (ry>ybot) ry = ybot;
		vals[i] = ry;	// save for visibility

		int ly = l - iy;
		if (ly<ytop) ly = ytop;
		if (ly>ybot) ly = ybot;

		SelectPen(hdc, fgPen);
		VertLine(hdc,xcen+i, r, ry); // start at center & spread out on both sides
		VertLine(hdc,xcen-i, l, ly);

		SelectPen(hdc,bgPen);	  		   // Fill in below baseline
		VertLine(hdc,xcen+i, ybot, r+1);
		VertLine(hdc,xcen-i, ybot, l+1);

		VertLine(hdc,xcen+i, ytop, ry-1);
		VertLine(hdc,xcen-i, ytop, ly-1);	// fill in above curve

//		if (ylast>=0) {
//			SelectPen(hdc,linePen);
//			VertLine(hdc, xcen+i-1, iy-1, ylast); // white dot marks curve
//			VertLine(hdc, xcen-i+1, iy-1, ylast);
//		}

		ylast = iy;
		ybr += slope;
		ybl += -slope;
	}

	// now do the Y curve
	ybr = ybl = float(rect.top+h - 2.5* off);
	xcen = rect.left+npts - 1;
	ylast = -1;
	for (i=0; i < npts; i++) {
		float v = pShader->EvalHiliteCurve2( 0.0f, (float)i/((float)npts*2.0f), layer );
		if (v>2.0f) v = 2.0f; // keep iy from wrapping
		iy = (int)(v* 0.6f * ((float)h-2.0f));

		int r = int( ybr + 0.5f );
		if ( r > ybot ) r = ybot;
		int l = int( ybl + 0.5f  );
		if ( l > ybot ) l = ybot;
		
		int ry = r - iy;
		if (ry<ytop) ry = ytop;
		if (ry>ybot) ry = ybot;

		int ly = l - iy;
		if (ly<ytop) ly = ytop;
		if (ly>ybot) ly = ybot;

		SelectPen(hdc, fg2Pen);
		VertLine(hdc,xcen-i, l, ly);	// left side always visible..in front

		if ( r <= vals[i] )
			VertLine(hdc,xcen+i, r, ry); // start at center & spread out on both sides
		else if ( ry <= vals[i] )
			VertLine(hdc,xcen+i, vals[i]-1, ry); // start at center & spread out on both sides

//		if (ylast>=0) {
//			SelectPen(hdc,linePen);
//			VertLine(hdc, xcen+i-1, iy-1, ylast); // white dot marks curve
//			VertLine(hdc, xcen-i+1, iy-1, ylast);
//		}

		ylast = iy;
		ybr += -slope;
		ybl += slope;
	}

	SelectObject( hdc, linePen );
	DeleteObject(fgPen);
	DeleteObject(fg2Pen);
	DeleteObject(bgPen);
	WhiteRect3D(hdc, rect, 1);
}

LRESULT CALLBACK Hilite2WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = GetWindowLongPtr(hwnd,GWL_ID);
	HWND hwParent = GetParent(hwnd);
	ShaderParamDlg *theDlg = (ShaderParamDlg *)GetWindowLongPtr(hwParent, GWLP_USERDATA);
	if (theDlg==NULL) return FALSE;

    switch (msg) {
		case WM_COMMAND: 	
		case WM_MOUSEMOVE: 	
		case WM_LBUTTONUP: 
		case WM_CREATE:
		case WM_DESTROY: 
		break;

		case WM_PAINT: 	
		{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				Shader* pShader = theDlg->GetShader();
				DrawHilite2(hdc, rect, pShader );
			}
			EndPaint( hwnd, &ps );
		}													
		break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 

LRESULT CALLBACK Hilite2Layer1WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = GetWindowLongPtr(hwnd,GWL_ID);
	HWND hwParent = GetParent(hwnd);
	ShaderParamDlg *theDlg = (ShaderParamDlg *)GetWindowLongPtr(hwParent, GWLP_USERDATA);
	if (theDlg==NULL) return FALSE;

    switch (msg) {
		case WM_PAINT: 	
		{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				Shader* pShader = theDlg->GetShader();
				DrawHilite2(hdc, rect, pShader, 1 );
			}
			EndPaint( hwnd, &ps );
		}													
		break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 

LRESULT CALLBACK Hilite2Layer2WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = GetWindowLongPtr(hwnd,GWL_ID);
	HWND hwParent = GetParent(hwnd);
	ShaderParamDlg *theDlg = (ShaderParamDlg *)GetWindowLongPtr(hwParent, GWLP_USERDATA);
	if (theDlg==NULL) return FALSE;

    switch (msg) {
		case WM_PAINT: 	
		{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				Shader* pShader = theDlg->GetShader();
				DrawHilite2(hdc, rect, pShader, 2 );
			}
			EndPaint( hwnd, &ps );
		}													
		break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 

void UpdateHilite( HWND hwHilite, Shader* pShader, int layer  )
{
Rect r;

		HDC hdc = GetDC( hwHilite );
		GetClientRect( hwHilite, &r );
		DrawHilite( hdc, r, pShader );
		ReleaseDC( hwHilite, hdc );
}


void UpdateHilite2( HWND hwHilite, Shader* pShader, int layer )
{
Rect r;

		HDC hdc = GetDC( hwHilite );
		GetClientRect( hwHilite, &r );
		DrawHilite2( hdc, r, pShader, layer );
		ReleaseDC( hwHilite, hdc );
}


////////////////////////////////////////////////////////////////////////

Color GetMtlColor( int i, Shader* pShader ) 
{
	switch(i) {
		case 0:  return pShader->GetAmbientClr(0,0); 
		case 1:  return pShader->GetDiffuseClr(0,0);
		case 2:  return pShader->GetSpecularClr(0,0);
		case 3:  return pShader->GetSelfIllumClr(0,0);
		default: return Color(0,0,0);
	}
}

TCHAR *GetColorName( int i )
{
	switch(i) {
		case 0:  return GetString(IDS_DS_AMBIENT);	 
		case 1:  return GetString(IDS_DS_DIFFUSE);	 
		case 2:  return GetString(IDS_DS_SPECULAR);	 
		case 3:  return GetString(IDS_KE_SELFILLUM_CLR);	 
		default: return GetString(IDS_KE_NOSUCH_CLR);	 
	}
}

void SetMtlColor(int i, Color c, Shader* pShader, IColorSwatch** cs, TimeValue t)
{
	switch(i) {
		case 0: //ambient
			pShader->SetAmbientClr(c,t); 
			if ( pShader->GetLockAD() ){
				pShader->SetDiffuseClr(c, t);
				cs[1]->SetColor( c );
				if (pShader->GetLockDS() ){
					pShader->SetSpecularClr(c,t);
					cs[2]->SetColor(c);
				}
			}
			break;
		case 1: //diffuse
			pShader->SetDiffuseClr(c,t); 
			if (pShader->GetLockAD() ){
				pShader->SetAmbientClr(c,t);
				cs[0]->SetColor(c);
			}
			if ( pShader->GetLockDS() ){
				pShader->SetSpecularClr(c,t);
				cs[2]->SetColor(c);
				}
			break;
		case 2: // specular
			pShader->SetSpecularClr(c,t); 
			if (pShader->GetLockDS() ){
				pShader->SetDiffuseClr(c,t);
				cs[1]->SetColor(c);
				if (pShader->GetLockAD() ){
					pShader->SetAmbientClr(c,t);
					cs[0]->SetColor(c);
					}
				}
			break;
		case 3: 
			pShader->SetSelfIllumClr(c,t); 
			break;
	}
}

///////////////////////////////////////////////////////////////////////////
// utility math routines
//
Point3 RotateVec( Point3& p, Point3& a, float rdn )
{
	float c = float( cos( rdn ));
	float t = 1 - c;
	float s = float( sin( rdn ));
	float txy = t * a.x * a.y;
	float tyz = t * a.y * a.z;
	float txz = t * a.x * a.z;
	float sx = s * a.x;
	float sy = s * a.y;
	float sz = s * a.z;

	Point3 out;
	out.x = p.x *(t* a.x * a.x + c) + p.y * (txy - sz) + p.z * (txz + sy);
	out.y = p.x *(txy + sz) + p.y * (t* a.y * a.y + c) + p.z * (tyz - sx);
	out.z = p.x *(txz - sy) + p.y * (tyz + sx) + p.z * (t* a.z * a.z + c);

	return out;
}
