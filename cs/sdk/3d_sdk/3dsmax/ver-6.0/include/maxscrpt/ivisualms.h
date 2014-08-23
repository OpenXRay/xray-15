/*	
 *		IVisualMS.h - public interfaces to VisualMS
 *
 *		  IVisualMSMgr  -  core interface to the VisualMS manager
 *		  IVisualMSForm -  interface to an existing form
 *
 *			Copyright © Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#ifndef _H_IVISUALMS
#define _H_IVISUALMS

#include "max.h"
#include "iFnPub.h"

class IVisualMSMgr;
class IVisualMSForm;
class IVisualMSItem;
class IVisualMSCallback;

// -------- Core interface to the VisualMS manager -----------
class IVisualMSMgr : public FPStaticInterface 
{
public:
	virtual IVisualMSForm* CreateForm()=0;						// create a new form
	virtual IVisualMSForm* CreateFormFromFile(TCHAR* fname)=0;	// create form from a .vms file

	// function IDs 
	enum { createForm,
		   createFormFromFile,
		}; 
}; 

#define VISUALMS_MGR_INTERFACE   Interface_ID(0x423d2cf2, 0x526706b5)
inline IVisualMSMgr* GetVisualMSMgr() { return (IVisualMSMgr*)GetCOREInterface(VISUALMS_MGR_INTERFACE); }

// ------- interface to individual VisiualMS forms ------------
class IVisualMSForm : public FPMixinInterface 
{
public:
	virtual void			Open(IVisualMSCallback* cb=NULL, TCHAR* source=NULL)=0; // open the form editor on this form
	virtual void			Close()=0;									// close the form editor

	virtual void			InitForm(TCHAR* formType, TCHAR* formName, TCHAR* caption)=0;
	virtual void			SetWidth(int w)=0;
	virtual void			SetHeight(int h)=0;
	virtual IVisualMSItem*	AddItem(TCHAR* itemType, TCHAR* itemName, TCHAR* text, int src_from=-1, int src_to=-1)=0;
	virtual IVisualMSItem*  AddCode(TCHAR* code, int src_from=-1, int src_to=-1)=0;
	virtual IVisualMSItem*  FindItem(TCHAR* itemName)=0;
	virtual BOOL			GenScript(TSTR& script, TCHAR* indent=NULL)=0;

	virtual BOOL			HasSourceBounds(int& from, int& to)=0;
	virtual void			SetSourceBounds(int from, int to)=0;

	FPInterfaceDesc*		GetDesc();

	// function IDs 
	enum { open,
		   close,
		   genScript,
		}; 

	// dispatch map
	BEGIN_FUNCTION_MAP
		VFN_0(open,					Open); 
		VFN_0(close,				Close); 
		FN_1( genScript, TYPE_BOOL, GenScript, TYPE_TSTR_BR); 
	END_FUNCTION_MAP 

};

#define VISUALMS_FORM_INTERFACE   Interface_ID(0x446b6824, 0x39502f75)

// ------- interface to individual VisiualMS form items ------------
class IVisualMSItem : public FPMixinInterface
{
public:
	virtual	void	SetPropery(TCHAR* propName, float f)=0;
	virtual	void	SetPropery(TCHAR* propName, int i)=0;
	virtual	void	SetPropery(TCHAR* propName, bool b)=0;
	virtual	void	SetPropery(TCHAR* propName, TCHAR* s)=0;
	virtual	void	SetProperyLit(TCHAR* propName, TCHAR* s)=0;
	virtual	void	SetPropery(TCHAR* propName, Point2 pt)=0;
	virtual	void	SetPropery(TCHAR* propName, Point3 pt)=0;
	virtual	void	SetPropery(TCHAR* propName, Tab<TCHAR*>* sa)=0;

	virtual void	SetHandler(TCHAR* eventName, TCHAR* source, int arg_start, int arg_end, int body_start, int body_end)=0;

	virtual void    GetShape(int& left, int& top, int& width, int& height)=0;
	virtual void    SetShape(int left, int top, int width, int height)=0;

	virtual void    AddComment(TCHAR* code, int src_from, int src_to)=0;

	FPInterfaceDesc* GetDesc();
};

#define VISUALMS_ITEM_INTERFACE   Interface_ID(0x13a403e4, 0x5f920eed)

// ------- base class for VMS edit callbacks ------------
//   instances supplied to VMSMgr::CreateForm() and called-back
//   when edit events happen

#define VISUALMS_CALLBACK_INTERFACE   Interface_ID(0x612b70df, 0xef77884)

class IVisualMSCallback : public FPMixinInterface 
{
public:
	virtual void			Save() { }				// save changes
	virtual void			Close() { }				// form editor close (already saved)

	virtual void			DeleteThis() { }

	virtual HWND			GetEditBox() { return NULL; }

	FPInterfaceDesc*		GetDesc() { return (FPInterfaceDesc*)GetCOREInterface(VISUALMS_CALLBACK_INTERFACE); }

	// function IDs 
	enum { save,
		   close,
		}; 

	// dispatch map
	BEGIN_FUNCTION_MAP
		VFN_0(save,	  Save); 
		VFN_0(close,  Close); 
	END_FUNCTION_MAP 

};

#endif
