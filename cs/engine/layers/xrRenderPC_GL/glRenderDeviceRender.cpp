#include "stdafx.h"
#include "glRenderDeviceRender.h"

#include <stdio.h>

#ifndef _EDITOR
void	fill_vid_mode_list();
void	free_vid_mode_list();
#else
void	fill_vid_mode_list()	{}
void	free_vid_mode_list()	{}
#endif

void CALLBACK glRenderDeviceRender::OnDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
		Log(message, id);
}

glRenderDeviceRender::glRenderDeviceRender()
	: m_hWnd(NULL)
	, m_hDC(NULL)
	, m_hRC(NULL)
	, Resources(nullptr)
{
}

void glRenderDeviceRender::OnDeviceCreate(LPCSTR shName)
{
	// Signal everyone - device created
	RCache.OnDeviceCreate();
	Resources->OnDeviceCreate(shName);
	::Render->create();
	Device.Statistic->OnDeviceCreate();

	//#ifndef DEDICATED_SERVER
	if (!g_dedicated_server)
	{
		m_WireShader.create("editor\\wire");
		m_SelectionShader.create("editor\\selection");

		//DUImpl.OnDeviceCreate();
	}
	//#endif
}

bool glRenderDeviceRender::Create(HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2, bool move_window)
{
	m_hWnd = hWnd;
	if (m_hWnd == NULL)
		return false;

	m_move_window = move_window;

#ifndef _EDITOR
	updateWindowProps();
	fill_vid_mode_list();
#endif

	RECT rClient = { 0 };

	if (!GetClientRect(m_hWnd, &rClient))
		return false;

	dwWidth = (rClient.right - rClient.left);
	dwHeight = (rClient.bottom - rClient.top);
	fWidth_2 = float(dwWidth / 2);
	fHeight_2 = float(dwHeight / 2);

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
			PFD_SUPPORT_OPENGL |
			PFD_DOUBLEBUFFER,	// Flags
		PFD_TYPE_RGBA,			// The kind of framebuffer. RGBA or palette.
		32,						// Color depth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,						// Number of bits for the depthbuffer
		8,						// Number of bits for the stencilbuffer
		0,						// Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	// Get the device context
	m_hDC = GetDC(hWnd);
	if (m_hDC == NULL)
	{
		Msg("Could not get device context.");
		return false;
	}

	// Choose the closest pixel format
	int iPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	if (iPixelFormat == 0)
	{
		Msg("No pixel format found.");
		return false;
	}

	// Apply the pixel format to the device context
	if (!SetPixelFormat(m_hDC, iPixelFormat, &pfd))
	{
		Msg("Could not set pixel format.");
		return false;
	}

	// Create the context
	m_hRC = wglCreateContext(m_hDC);
	if (m_hRC == NULL)
	{
		Msg("Could not create drawing context.");
		return false;
	}

	// Make the new context the current context for this thread
	// NOTE: This assumes the thread calling Create() is the only
	// thread that will use the context.
	if (!wglMakeCurrent(m_hDC, m_hRC))
	{
		Msg("Could not make context current.");
		return false;
	}

	// Initialize OpenGL Extension Wrangler
	if (glewInit() != GLEW_OK)
	{
		Msg("Could not initialize glew.");
		return false;
	}

#ifdef DEBUG
	CHK_GL(glEnable(GL_DEBUG_OUTPUT));
	CHK_GL(glDebugMessageCallback((GLDEBUGPROC)OnDebugCallback, nullptr));
#endif // DEBUG

	// TODO: OGL: Remap dpeth values to the -1..1 range.
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	Resources = new CResourceManager();

	return true;
}

void glRenderDeviceRender::DestroyHW()
{
	xr_delete(Resources);

	if (m_hRC)
	{
		if (!wglMakeCurrent(nullptr, nullptr))
			Msg("Could not release drawing context.");

		if (!wglDeleteContext(m_hRC))
			Msg("Could not delete context.");

		m_hRC = nullptr;
	}

	if (m_hDC)
	{
		if (!ReleaseDC(m_hWnd, m_hDC))
			Msg("Could not release device context.");

		m_hDC = nullptr;
	}

#ifndef _EDITOR
	free_vid_mode_list();
#endif
}

void glRenderDeviceRender::SetupStates()
{
	HW.Caps.Update();

	//	TODO: OGL: Implement Resetting of render states into default mode
	//VERIFY(!"glRenderDeviceRender::SetupStates not implemented.");
}

void glRenderDeviceRender::DeferredLoad(BOOL E)
{
	Resources->DeferredLoad(E);
}

void glRenderDeviceRender::ResourcesDeferredUpload()
{
	Resources->DeferredUpload();
}

void glRenderDeviceRender::ResourcesGetMemoryUsage(u32& m_base, u32& c_base, u32& m_lmaps, u32& c_lmaps)
{
	if (Resources)
		Resources->_GetMemoryUsage(m_base, c_base, m_lmaps, c_lmaps);
}

void glRenderDeviceRender::ResourcesDestroyNecessaryTextures()
{
	Resources->DestroyNecessaryTextures();
}

void glRenderDeviceRender::ResourcesStoreNecessaryTextures()
{
	glRenderDeviceRender::Instance().Resources->StoreNecessaryTextures();
}

void glRenderDeviceRender::ResourcesDumpMemoryUsage()
{
	glRenderDeviceRender::Instance().Resources->_DumpMemoryUsage();
}

void glRenderDeviceRender::ClearTarget()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	CHK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void glRenderDeviceRender::SetCacheXform(Fmatrix &mView, Fmatrix &mProject)
{
	RCache.set_xform_view(mView);
	RCache.set_xform_project(mProject);
}

void glRenderDeviceRender::Begin()
{
	RCache.OnFrameBegin();
	RCache.set_CullMode(CULL_CW);
	RCache.set_CullMode(CULL_CCW);
}

void glRenderDeviceRender::End()
{
	RCache.OnFrameEnd();
	Memory.dbg_check();
	SwapBuffers(m_hDC);
}

void  glRenderDeviceRender::Reset(HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2)
{
	// We should still be rendering to the same window.
	R_ASSERT(m_hWnd == hWnd);
	Resources->reset_begin();
	Memory.mem_compact();

	dwWidth = psCurrentVidMode[0];
	dwHeight = psCurrentVidMode[1];
	updateWindowProps();

	fWidth_2 = float(dwWidth / 2);
	fHeight_2 = float(dwHeight / 2);
	Resources->reset_end();
}

void  glRenderDeviceRender::OnAssetsChanged()
{
	Resources->m_textures_description.UnLoad();
	Resources->m_textures_description.Load();
}

void glRenderDeviceRender::updateWindowProps()
{
	//	BOOL	bWindowed				= strstr(Core.Params,"-dedicated") ? TRUE : !psDeviceFlags.is	(rsFullscreen);
	BOOL	bWindowed = !psDeviceFlags.is(rsFullscreen);

	u32		dwWindowStyle = 0;
	// Set window properties depending on what mode were in.
	if (bWindowed)		{
		if (m_move_window) {
			if (strstr(Core.Params, "-no_dialog_header"))
				SetWindowLong(m_hWnd, GWL_STYLE, dwWindowStyle = (WS_BORDER | WS_VISIBLE));
			else
				SetWindowLong(m_hWnd, GWL_STYLE, dwWindowStyle = (WS_BORDER | WS_DLGFRAME | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX));
			// When moving from fullscreen to windowed mode, it is important to
			// adjust the window size after recreating the device rather than
			// beforehand to ensure that you get the window size you want.  For
			// example, when switching from 640x480 fullscreen to windowed with
			// a 1000x600 window on a 1024x768 desktop, it is impossible to set
			// the window size to 1000x600 until after the display mode has
			// changed to 1024x768, because windows cannot be larger than the
			// desktop.

			RECT			m_rcWindowBounds;
			BOOL			bCenter = FALSE;
			if (strstr(Core.Params, "-center_screen"))	bCenter = TRUE;

			if (bCenter) {
				RECT				DesktopRect;

				GetClientRect(GetDesktopWindow(), &DesktopRect);

				SetRect(&m_rcWindowBounds,
					(DesktopRect.right - psCurrentVidMode[0]) / 2,
					(DesktopRect.bottom - psCurrentVidMode[1]) / 2,
					(DesktopRect.right + psCurrentVidMode[0]) / 2,
					(DesktopRect.bottom + psCurrentVidMode[1]) / 2);
			}
			else{
				SetRect(&m_rcWindowBounds,
					0,
					0,
					psCurrentVidMode[0],
					psCurrentVidMode[1]);
			};

			AdjustWindowRect(&m_rcWindowBounds, dwWindowStyle, FALSE);

			SetWindowPos(m_hWnd,
				HWND_TOP,
				m_rcWindowBounds.left,
				m_rcWindowBounds.top,
				(m_rcWindowBounds.right - m_rcWindowBounds.left),
				(m_rcWindowBounds.bottom - m_rcWindowBounds.top),
				SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_DRAWFRAME);
		}
	}
	else
	{
		SetWindowLong(m_hWnd, GWL_STYLE, dwWindowStyle = (WS_POPUP | WS_VISIBLE));
	}

	ShowCursor(FALSE);
	SetForegroundWindow(m_hWnd);
}

struct _uniq_mode
{
	_uniq_mode(LPCSTR v) :_val(v){}
	LPCSTR _val;
	bool operator() (LPCSTR _other) { return !stricmp(_val, _other); }
};

void free_vid_mode_list()
{
	for (int i = 0; vid_mode_token[i].name; i++)
	{
		xr_free(vid_mode_token[i].name);
	}
	xr_free(vid_mode_token);
	vid_mode_token = NULL;
}

void fill_vid_mode_list()
{
	if (vid_mode_token != NULL)		return;
	xr_vector<LPCSTR>	_tmp;

	DWORD iModeNum = 0;
	DEVMODE dmi;
	ZeroMemory(&dmi, sizeof(dmi));
	dmi.dmSize = sizeof(dmi);

	while (EnumDisplaySettings(nullptr, iModeNum++, &dmi) != 0)
	{
		string32 str;

		if (dmi.dmPelsWidth < 800)
			continue;

		sprintf_s(str, sizeof(str), "%dx%d", dmi.dmPelsWidth, dmi.dmPelsHeight);

		if (_tmp.end() != std::find_if(_tmp.begin(), _tmp.end(), _uniq_mode(str)))
			continue;

		_tmp.push_back(NULL);
		_tmp.back() = xr_strdup(str);
	}

	u32 _cnt = _tmp.size() + 1;

	vid_mode_token = xr_alloc<xr_token>(_cnt);

	vid_mode_token[_cnt - 1].id = -1;
	vid_mode_token[_cnt - 1].name = NULL;

#ifdef DEBUG
	Msg("Available video modes[%d]:", _tmp.size());
#endif // DEBUG
	for (u32 i = 0; i<_tmp.size(); ++i)
	{
		vid_mode_token[i].id = i;
		vid_mode_token[i].name = _tmp[i];
#ifdef DEBUG
		Msg("[%s]", _tmp[i]);
#endif // DEBUG
	}
}
