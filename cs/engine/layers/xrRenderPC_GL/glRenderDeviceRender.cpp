#include "stdafx.h"
#include "glRenderDeviceRender.h"


glRenderDeviceRender::glRenderDeviceRender()
	: m_hWnd(NULL)
	, m_hDC(NULL)
	, m_hRC(NULL)
{
}

bool glRenderDeviceRender::Create(HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2, bool move_window)
{
	m_hWnd = hWnd;
	if (m_hWnd == NULL)
		return false;

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
	return wglMakeCurrent(m_hDC, m_hRC);
}

void glRenderDeviceRender::DestroyHW()
{
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
}
