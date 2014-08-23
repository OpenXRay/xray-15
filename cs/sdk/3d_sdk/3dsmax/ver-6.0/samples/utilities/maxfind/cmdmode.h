/****************************************************************************
 MAX File Finder
 Christer Janson
 September 20, 1998
 CmdMode.h - Header for Virtual TrackBall
 ***************************************************************************/
class ICommandMode {
public:
	virtual BOOL	Init(HWND hWnd, int nWidth, int nHeight) = 0;
	virtual BOOL	Resize(int nWidth, int nHeight) = 0;

	virtual BOOL	OnLMouseDown(short x, short y) = 0;
	virtual BOOL	OnLMouseUp(short x, short y) = 0;
	virtual BOOL	OnMMouseDown(short x, short y) = 0;
	virtual BOOL	OnMMouseUp(short x, short y) = 0;
	virtual BOOL	OnRMouseDown(short x, short y) = 0;
	virtual BOOL	OnRMouseUp(short x, short y) = 0;
	virtual BOOL	OnMouseMove(DWORD keyFlags, short x, short y) = 0;

	virtual BOOL	HasMotion() = 0;
};
