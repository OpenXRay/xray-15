/*	
 *		OLEAutomation.h - OLE Automation services for MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_MAX_OLE_AUTOMATION
#define _H_MAX_OLE_AUTOMATION

#include "Arrays.h"
#include "classIDs.h"
#include "Funcs.h"

/* error scodes */

#define MS_E_EXCEPTION					MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0200)  
#define MS_E_ILLEGAL_RETURN_VALUE       MS_E_EXCEPTION + 0x001  

/* ------- the MAXScript OLE object class factory ---------- */

class MSClassFactory : public IClassFactory
{
public:
    static IClassFactory* Create();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID iid, void** ppv);
    STDMETHOD(LockServer)(BOOL fLock);

private:
    MSClassFactory();

    unsigned long m_refs;
};

/* ---------- the MAXScript OLE object class -------------- */

class MSOLEObject : public IDispatch
{
public:
    static MSOLEObject* Create();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    /* IDispatch methods */
    STDMETHOD(GetTypeInfoCount)(unsigned int* pcTypeInfo);
    STDMETHOD(GetTypeInfo)(unsigned int iTypeInfo, LCID lcid, ITypeInfo** ppTypeInfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR** rgszNames, unsigned int cNames, LCID lcid, DISPID* rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, unsigned short wFlags,
					  DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, unsigned int* puArgErr);

	/* MSOLEObject stuff */

    unsigned long m_refs;
	static Array* exposed_fns;		// array of exposed MAXScript functions, DISPID is 1-based index in array

    MSOLEObject();

	static void install_fns(Array* fns);
};

/* ---------------- client-side classes -------------------- */

visible_class (OLEObject)

class OLEObject : public Value
{
public:
    Value*		progID;		// user-supplied progID string
	CLSID		clsid;		// CLSID of ActiveX object.
    LPDISPATCH	pdisp;		// IDispatch of ActiveX object.

				OLEObject(Value* progID, CLSID cslid, LPDISPATCH pdisp);
				OLEObject(Value* progID, LPDISPATCH pdisp);
			   ~OLEObject();

				classof_methods (OLEObject, Value);
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void		sprin1(CharStream* s);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	Value*		get_fn_property(Value* prop);
};

visible_class (OLEMethod)

class OLEMethod : public Function
{
public:
	OLEObject*	ole_obj;	// my OLE object
    DISPID		dispid;		// method dispatch ID

				OLEMethod() { }
				OLEMethod(TCHAR* name, OLEObject* ole_obj, DISPID mth_id);

				classof_methods (OLEMethod, Function);
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

BOOL init_MAXScript_OLE();
void uninit_OLE();

#define UNUSED(X) (X)

#endif