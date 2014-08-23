/****************************************************************************
 MAX File Finder
 Christer Janson
 September 19, 1998
 About.cpp - 3D AboutBox Implementation
 ***************************************************************************/
#include "pch.h"

#include "app.h"
#include "..\..\..\include\buildver.h"
#include "resource.h"
#include "resourceOverride.h"

#define COOL_ABOUTBOX
#ifdef COOL_ABOUTBOX

#include <gl/gl.h>
#include <gl/glaux.h>

#include <stdlib.h>
#include <math.h>
#include "gtypes.h"
#include "vtrackbl.h"

#include "geometry.h"

INT_PTR CALLBACK AboutDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK oglWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void InitOpenGL(HWND hWnd);
void DrawGrid();
void SetView();
void SetupLights();
void RenderColor(Point3 color);
void Render();
void RenderGeometry();

#define WM_INITOPENGL	(WM_USER+42)
#define WM_CHECKSPIN	(WM_USER+43)

static HWND			hGLWnd;
static HDC			hdc;
static HGLRC		hRC;
static HINSTANCE	hInstance;
static VirtualTrackball Trackball;
static float distance;
static float aspect;
static float col[3];
float  rcount;

void App::DoAboutBox()
	{
	hInstance = GetInstance();

	DialogBoxParam(
		GetInstance(),
        MAKEINTRESOURCE(IDD_ABOUTBOX),
        GetHWnd(),
        (DLGPROC)AboutDlgProc,
        (LPARAM)this);
	}

INT_PTR CALLBACK AboutDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
	switch (message) {
		case WM_INITDIALOG:
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			CenterWindow(hWnd, GetParent(hWnd));
			PostMessage(hWnd, WM_INITOPENGL, 0, 0);
			return 1;
		case WM_INITOPENGL:
			InitOpenGL(GetDlgItem(hWnd, IDC_IMAGE));
			break;

		case WM_DESTROY:
			wglDeleteContext(hRC);
			ReleaseDC(hGLWnd, hdc);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd,1);
					break;
				}
			return 1;
		}
	return 0;
	}

void InitOpenGL(HWND hWnd)
	{
	WNDCLASSEX	wc;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = oglWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIconSm		 = NULL; //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINWND));
	wc.hIcon         = NULL; //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINWND));
	wc.hCursor       = LoadCursor(NULL, IDC_SIZEALL);
	wc.hbrBackground = NULL; //(HBRUSH)(COLOR_DESKTOP+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "CJOGLFRAME";

	RegisterClassEx(&wc);

	RECT rect;
	GetClientRect(hWnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	hGLWnd = CreateWindow("CJOGLFRAME",
						"",
						WS_CHILDWINDOW | WS_VISIBLE,
						rect.left, rect.top, rect.right, rect.bottom,
						hWnd, NULL, hInstance, NULL);

	hdc = GetDC(hGLWnd);

	PIXELFORMATDESCRIPTOR pfd = { 
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
		1,                                // version number 
		PFD_DRAW_TO_WINDOW |              // support window 
		PFD_SUPPORT_OPENGL |              // support OpenGL 
		PFD_DOUBLEBUFFER,                 // double buffered 
		PFD_TYPE_RGBA,                    // RGBA type 
		24,                               // 24-bit color depth 
		0, 0, 0, 0, 0, 0,                 // color bits ignored 
		0,                                // no alpha buffer 
		0,                                // shift bit ignored 
		0,                                // no accumulation buffer 
		0, 0, 0, 0,                       // accum bits ignored 
		32,                               // 32-bit z-buffer     
		0,                                // no stencil buffer 
		0,                                // no auxiliary buffer 
		PFD_MAIN_PLANE,                   // main layer 
		0,                                // reserved 
		0, 0, 0                           // layer masks ignored 
	}; 
	int  iPixelFormat; 

	// get the device context's best, available pixel format match 
	if ((iPixelFormat = ChoosePixelFormat(hdc, &pfd)) == 0) {
		return;
	}

	// make that match the device context's current pixel format 
	if (SetPixelFormat(hdc, iPixelFormat, &pfd) == FALSE) {
        return;
	}

	glDepthFunc(GL_LESS);
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	hRC = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hRC);

	Trackball.Init(hWnd, width, height);
	distance = 2.0f;
	aspect = (float)width/(float)height;

	rcount = 0.0f;
	col[0] = (float)sin(rcount+0.00f)/2.0f+0.5f;
	col[1] = (float)sin(rcount+2.09f)/2.0f+0.5f;
	col[2] = (float)sin(rcount+4.18f)/2.0f+0.5f;
	}

void Render()
	{
	glClearColor(0.7f, 0.7f, 0.7f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	SetView();
	DrawGrid();

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);

	glEnable(GL_CULL_FACE);
	glEnable(GL_CCW);
	glEnable(GL_LIGHTING);

	RenderGeometry();

    glDisable(GL_LIGHTING);

	SwapBuffers(hdc);
	}

void RenderGeometry()
	{
	RenderColor(Point3(col[0], col[1], col[2]));

	rcount+=0.01f;
	col[0] = (float)sin(rcount+0.00f)/2.0f+0.5f;
	col[1] = (float)sin(rcount+2.09f)/2.0f+0.5f;
	col[2] = (float)sin(rcount+4.18f)/2.0f+0.5f;

	int numfloats = sizeof(objectDef)/sizeof(float);

	//auxSolidIcosahedron(0.5);
	glBegin(GL_TRIANGLES);

	for (int i=0; i<numfloats; i+=6) {
		glNormal3f(objectDef[i], objectDef[i+1], objectDef[i+2]);
		glVertex3f(objectDef[i+3], objectDef[i+4], objectDef[i+5]);
		}

	glEnd();

	}

void SetView()
	{
    float matRot[4][4];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, aspect, 0.1f, 2.0 * distance);
    glTranslated( 0, 0, -distance);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    SetupLights();
    Trackball.CalcRotMatrix(matRot);
    glMultMatrixf( &(matRot[0][0]) );
	}

void SetupLights()
	{
	static float ambient[]		= {   0.1f,    0.1f,    0.1f, 1.0f};
    static float diffuse[]		= {   0.8f,    0.8f,    0.8f, 1.0f};
    static float specular[]		= {   1.0f,    1.0f,    1.0f, 1.0f};
    static float position[]		= { 100.0f,    0.0f, -100.0f, 0.0f};
    static float position2[]	= {-100.0f,    0.0f,  100.0f, 0.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT1, GL_POSITION, position2);
    glEnable(GL_LIGHT1);
	}

void RenderColor(Point3 color)
{
	static float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	static float specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static float diffuse[4];

    diffuse[0] = color.x;
	diffuse[1] = color.y;
	diffuse[2] = color.z;
	diffuse[3] = 1.0f;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,	 ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,	 diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 128);
}

void DrawGrid()
	{
    float gx, gy;
    float dx, dy;
    float axisLen;

    Point3 gridStart    = Point3(-1, -1, 0);
    Point3 gridEnd      = Point3( 1,  1, 0);

    float maxVal = (float)__max(fabs(gridStart.x), __max(fabs(gridStart.y),
                    __max(fabs(gridEnd.x), fabs(gridEnd.y))));

    gridStart.x = -maxVal;
    gridStart.y = -maxVal;
    gridEnd.x   = maxVal;
    gridEnd.y   = maxVal;

    gridStart.z = 0.0f;
    gridEnd.z   = 0.0f;

    dx = (gridEnd.x - gridStart.x) / 20.0f;
    dy = (gridEnd.y - gridStart.y) / 20.0f;

    axisLen = (gridEnd.y - gridStart.y) / 4.0f;

/*
	glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(axisLen, 0.0f, 0.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, axisLen, 0.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, axisLen);
	glEnd();
*/

	// Grid lines
	glColor3f(0.5f, 0.5f, 0.5f);

	glBegin(GL_LINES);
		for (gy = gridStart.y; gy<(gridEnd.y+dy); gy+=dy) {
			glVertex2f(gridStart.x, gy);
			glVertex2f(gridEnd.x, gy);
			}
		for (gx = gridStart.x; gx<(gridEnd.x+dx); gx+=dx) {
			glVertex2f(gx, gridStart.y);
			glVertex2f(gx, gridEnd.y);
			}

		glColor3f(0.3f, 0.3f, 0.3f);
		glVertex2f(gridStart.x, 0.0f);
		glVertex2f(gridEnd.x, 0.0f);

		glVertex2f(0.0f, gridStart.y);
		glVertex2f(0.0f, gridEnd.y);

	glEnd();
	}


LRESULT CALLBACK oglWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	PAINTSTRUCT	ps;
	static		int rbY = 0;

    switch (message) {
		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			if (Trackball.OnLMouseDown((short)LOWORD(lParam), (short)HIWORD(lParam)))
				InvalidateRect(hWnd, NULL, TRUE);
			break;
		case WM_MBUTTONDOWN:
			SetCapture(hWnd);
			if (Trackball.OnMMouseDown((short)LOWORD(lParam), (short)HIWORD(lParam)))
				InvalidateRect(hWnd, NULL, TRUE);
			break;
		case WM_RBUTTONDOWN:
			SetCapture(hWnd);
			if (Trackball.OnRMouseDown((short)LOWORD(lParam), (short)HIWORD(lParam))) {
				InvalidateRect(hWnd, NULL, TRUE);
			}
			else {
				rbY = (short)HIWORD(lParam);
			}
			break;
		case WM_LBUTTONUP:
			ReleaseCapture();
			if (Trackball.OnLMouseUp((short)LOWORD(lParam), (short)HIWORD(lParam)))
				InvalidateRect(hWnd, NULL, TRUE);
			PostMessage(hWnd, WM_CHECKSPIN, 0, 0);

			break;
		case WM_MBUTTONUP:
			ReleaseCapture();
			if (Trackball.OnMMouseUp((short)LOWORD(lParam), (short)HIWORD(lParam)))
				InvalidateRect(hWnd, NULL, TRUE);
			PostMessage(hWnd, WM_CHECKSPIN, 0, 0);

			break;
		case WM_RBUTTONUP:
			ReleaseCapture();
			if (Trackball.OnRMouseUp((short)LOWORD(lParam), (short)HIWORD(lParam)))
				InvalidateRect(hWnd, NULL, TRUE);
			PostMessage(hWnd, WM_CHECKSPIN, 0, 0);

			break;

		case WM_MOUSEMOVE:
			if (Trackball.OnMouseMove(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam))) {
				InvalidateRect(hWnd, NULL, TRUE);
			}
			else if (wParam & MK_RBUTTON) {
				// Right button down
				distance += ((short)HIWORD(lParam) - rbY)/40.0f;
				rbY = (short)HIWORD(lParam);
				InvalidateRect(hWnd, NULL, TRUE);
			}

			break;

		case WM_TIMER:
			if (Trackball.HasMotion()) {
				InvalidateRect(hWnd, NULL, TRUE);
				}
			break;

		case WM_PAINT:
			BeginPaint(hWnd, &ps);
			Render();
			EndPaint(hWnd, &ps);

			if (Trackball.HasMotion()) {
				SetTimer(hWnd, 1, 10, NULL);
				//PostMessage(hWnd, WM_CHECKSPIN, 0, 0);
				}

			break;

		default:
			break;
		}

    return (DefWindowProc(hWnd, message, wParam, lParam));

	}

#else

INT_PTR CALLBACK AboutDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

void App::DoAboutBox(void)
	{
	DialogBoxParam(
		GetInstance(),
        MAKEINTRESOURCE(IDD_ABOUTBOX_TINY),
        GetHWnd(),
        (DLGPROC)AboutDlgProc,
        (LPARAM)this);
	}

INT_PTR CALLBACK AboutDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
	switch (message) {
		case WM_INITDIALOG:
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			CenterWindow(hWnd, GetParent(hWnd));
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd,1);
					break;
				}
			return 1;
		}
	return 0;
	}
#endif