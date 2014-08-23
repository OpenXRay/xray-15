/************************************************************************	
 *		ScriptEd.h - wrapper classes for script editor windows			*
 *																		*
 *		Author: Ravi Karra												*
 ************************************************************************/

#ifndef _SCRIPTEDITOR_H
#define _SCRIPTEDITOR_H


#include "MaxScrpt.h"
#include "Listener.h"

// defines for script editor window menu items
#define	IDM_NEW			10
#define IDM_OPEN		11
#define IDM_EVAL_ALL	40026
#define IDM_CLOSE		40024

// wrapper class for script editor windows
class ScriptEditor
{
		TCHAR*			editScript;
		TSTR			title;
	protected:
		WNDPROC			originalWndProc;		
		IntTab			disable_menus;
		edit_window		*ew;
		HWND			hScript;
	
	public:
		ScriptEditor(TCHAR* ititle=NULL) : 
			title(ititle), 
			ew(NULL), 
			hScript(NULL), 
			editScript(NULL) { }

		~ScriptEditor() { if (editScript) delete editScript; editScript = NULL; }
		
		virtual LRESULT APIENTRY proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {	
							return CallWindowProc(originalWndProc, hwnd, uMsg, wParam, lParam); 
							}

		ScripterExport virtual	HWND	DisplayWindow(HWND hParent=NULL/*for future use*/);
		ScripterExport virtual void		CloseWindow(bool notify=false);
		ScripterExport virtual TCHAR*	GetEditScript();
		ScripterExport virtual void		SetEditScript(TCHAR* script);		
		ScripterExport virtual void		SetTitle(TCHAR* t) { title = t; }						
		ScripterExport virtual bool		OnFileOpen(HWND hwnd);
		ScripterExport virtual bool		OnClose(HWND hwnd);
		
		virtual TCHAR*	GetTitle()			{ return title; }
		virtual Value*	GetValueTitle()		{ return (ew) ? ew->file_name : NULL; }
		virtual bool	OnExecute(HWND hwnd){ return false; } // return false to default handling
		virtual	bool	IsDisplayed()		{ return ew!=NULL; }
		virtual	IntTab&	GetDisabledMenuTab(){ return disable_menus; }
};

// open new editor on existing file, pop openfilename dialog if no filename supplied
// if ew is NULL, a new editor window is opened
ScripterExport void open_script(TCHAR* filename=NULL, edit_window *ew=NULL);

#endif //_SCRIPTEDITOR_H