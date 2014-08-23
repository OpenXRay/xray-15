/*		Listener.h - the Listener class - MAXScript listener windows
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 */

#ifndef _H_LISTENER
#define _H_LISTENER

#include <windowsx.h>
#include "Pipe.h"
#include "Thunks.h"

extern GlobalThunk* listener_result_thunk;

extern ScripterExport BOOL end_keyboard_input;
extern ScripterExport BOOL start_keyboard_input;
extern ScripterExport TCHAR* keyboard_input;
extern ScripterExport Value* keyboard_terminator;
extern ScripterExport Array* keyboard_input_terminators;

// listener flag values
#define LSNR_INPUT_MODE_MASK	0x000F
#define LSNR_KEYINPUT_OFF		0x0000
#define LSNR_KEYINPUT_LINE		0x0001
#define LSNR_KEYINPUT_CHAR		0x0002
#define LSNR_SHOWING			0x0010	// MAXScript is forcing a show, ignore all other ShowWindows
#define LSNR_NO_MACRO_REDRAW	0x0020	// disable drawing in macro-rec box (to get round bug in WM_SETREDRAW)

#define EDIT_BOX_ITEM			1001	// listener edit box dlg item #
#define MACROREC_BOX_ITEM		1002	// listener macro-recorder edit box dlg item #

class ListenerViewWindow;

class Listener : public Value
{
	HANDLE		listener_thread;
	DWORD		thread_id;
public:
	HWND		listener_window;		// main listener window
	HWND		edit_box;				// edit control for main type-in
	HWND		macrorec_box;			// edit control for macro-recorder output
	HWND		mini_listener;			// mini-listener parent window in the MAX status panel
	HWND		mini_edit;				// mini-listener edit control for type_in
	HWND		mini_macrorec;			// mini-listener edit control for macro-recorder output
	WindowStream* edit_stream;			// stream for the main edit box
	WindowStream* macrorec_stream;		// stream for the macro-recorder edit box
	WindowStream* mini_edit_stream;		// stream for the mini edit box
	WindowStream* mini_macrorec_stream;	// stream for the mini macro-recorder edit box
	Pipe*		source_pipe;			// the source pipe for the listener, source written to, compiler reads from
	int			flags;
	ListenerViewWindow* lvw;			// the ViewWindow instance for the listener

				Listener(HINSTANCE mxs_instance, HWND MAX_window);
				~Listener();

	static DWORD run(Listener *l);
	void		create_listener_window(HINSTANCE hInstance, HWND hwnd);

	void		gc_trace();
	void		collect() { delete this; }

	ScripterExport void set_keyinput_mode(int mode) { flags = (flags & ~LSNR_INPUT_MODE_MASK) | mode; }
};

// ViewWindow subclass for putting the listener in a MAX viewport
class ListenerViewWindow : public ViewWindow
{
public:
	TCHAR *GetName();
	HWND CreateViewWindow(HWND hParent, int x, int y, int w, int h);
	void DestroyViewWindow(HWND hWnd);
	BOOL CanCreate();
};

class ListenerMessageData 
{
public:
	WPARAM wParam;
	LPARAM lParam;
	HANDLE message_event;
	ListenerMessageData(WPARAM wp, LPARAM lp, HANDLE me) { wParam = wp; lParam = lp; message_event = me; }
};

#define CLICK_STACK_SIZE	8			// number of clicked-at locations to remember for ctrl-R return
typedef struct edit_window edit_window;
struct edit_window
{
	edit_window*	next;
	edit_window*	prev;
	Value*			file_name;
	bool			dont_flag_save;
	bool			needs_save;
	HWND			window;
	HWND			edit_box;
	int				sr_offset;
	int				click_tos;
	int			    click_at[CLICK_STACK_SIZE];
	bool			editing;
	IUnknown*		pDoc;

	INT_PTR	new_rollout();	
	INT_PTR	edit_rollout();	
};

#endif
