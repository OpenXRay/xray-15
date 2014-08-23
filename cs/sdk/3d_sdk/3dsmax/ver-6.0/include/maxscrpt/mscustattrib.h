/*	
 *		MSCustAttrib.h - MAXScript scriptable custom attributes MAX objects
 *
 *			Copyright © Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#ifndef _H_MSCUST_ATTRIB
#define _H_MSCUST_ATTRIB

// ---------- scripter Custom Attribute classes -------------------

#include "CustAttrib.h"

#define I_SCRIPTEDCUSTATTRIB	0x000010C1

// special subclass for holding custom attribute definitions
// these can be applied to any existing object, adding a CustAttrib to it
//   instances of MSCustAttrib (an MSPlugin subclass) refer to CustAttribDefs for metadata
visible_class (MSCustAttribDef)

class MSCustAttribDef : public MSPluginClass
{
public:
	static Tab<MSCustAttribDef*> ms_attrib_defs; // table of existing scripted attribute defs to enable redefinition

	Value*			defData;			// persistent definition data, used by the scripter attribute editor
	TSTR			source;				// extracted definition source, stored persistently with the def & automatically recompiled on 
										// reload

					MSCustAttribDef(Class_ID& attrib_id);
				   ~MSCustAttribDef();

	// definition and redefinition
	static MSCustAttribDef* intern(Class_ID& attrib_id);
	void			init(int local_count, Value** inits, HashTable* local_scope, HashTable* handlers, Array* pblock_defs, Array* rollouts, CharStream* source);

	// MAXScript required
//	BOOL			is_kind_of(ValueMetaClass* c) { return (c == class_tag(MSCustAttribDef)) ? 1 : Value::is_kind_of(c); } // LAM: 2/23/01
	BOOL			is_kind_of(ValueMetaClass* c) { return (c == class_tag(MSCustAttribDef)) ? 1 : MSPluginClass::is_kind_of(c); }
#	define			is_attribute_def(v) ((v)->tag == class_tag(MSCustAttribDef))
	void			collect() { delete this; }
	void			gc_trace();
	void			sprin1(CharStream* s);

	bool			is_custAttribDef() { return true; }
	MSCustAttribDef* unique_clone();

	// from Value 
	Value*			apply(Value** arg_list, int count, CallContext* cc=NULL) { return Value::apply(arg_list, count, cc); }  // CustAttribDef's are not applyable

	// scene I/O
	static IOResult	save_custattrib_defs(ISave* isave);
	static IOResult	load_custattrib_defs(ILoad* iload);

	// ClassDesc delegates
	RefTargetHandle	Create(BOOL loading);

	Value*			get_property(Value** arg_list, int count);
	Value*			set_property(Value** arg_list, int count);
};

// MSCustAttrib - instances contain individual custom attribute blocks 
//   that are added to customized objects.

class MSCustAttrib : public MSPlugin, public CustAttrib, public ISubMap
{
public:
	IObjParam*				cip;		// ip for any currently open command panel dialogs
	static MSAutoMParamDlg* masterMDlg; // master dialog containing all scripted rollout
	IMtlParams*				mip;		// ip for any open mtlEditor panel dlgs

					MSCustAttrib() : cip(NULL), mip(NULL) { }
					MSCustAttrib(MSCustAttribDef* pc, BOOL loading);
				   ~MSCustAttrib() { DeleteAllRefsFromMe(); }

	void			sprin1(CharStream* s);

	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0, int category=ROLLUP_CAT_STANDARD);
	void			DeleteRollupPage(HWND hRollup);
	IRollupWindow*  GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; }  // no delegates in MSCustAttribs 

	// from CustAttrib
	TCHAR*			GetName()	{ return pc->class_name->to_string(); }
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);

	// From Animatable
    
    using CustAttrib::GetInterface;

	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	SClass_ID		SuperClassID() { return pc->sclass_id; }
	void			FreeCaches() { } 		
	int				NumSubs() { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) { return pblocks[i]; }
	TSTR			SubAnimName(int i) { return pblocks[i]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }

	void*			GetInterface(ULONG id);
	void			DeleteThis();
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

	// From ReferenceMaker
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
					{ 
						if (!(pc->mpc_flags & MPC_REDEFINITION))
							return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
						else
							return REF_SUCCEED;
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) { return pblocks[i]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i >= pblocks.Count()) pblocks.SetCount(i+1); pblocks[i] = (IParamBlock2*)rtarg; 
					}
	void			RefDeleted() { MSPlugin::RefDeleted(); }
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	IOResult		Save(ISave *isave) { pc->mpc_flags |= MPC_CAD_FILESAVE; return MSPlugin::Save(isave); } // LAM - 1/24/02 - defect 299822
    IOResult		Load(ILoad *iload) { return MSPlugin::Load(iload); }

	// from ISubMap
	int				NumSubTexmaps();
	Texmap*			GetSubTexmap(int i);
	void			SetSubTexmap(int i, Texmap *m);
	TSTR			GetSubTexmapSlotName(int i);
	int				MapSlotType(int i) { return MAPSLOT_TEXTURE; }
	TSTR			GetSubTexmapTVName(int i) { return GetSubTexmapSlotName(i); }
	ReferenceTarget *GetRefTarget() { return this; }
};

#endif
