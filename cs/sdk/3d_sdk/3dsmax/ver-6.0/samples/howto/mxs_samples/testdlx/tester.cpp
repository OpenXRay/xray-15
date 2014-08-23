/*	Sample MAXScript SDK code
 *
 *	Adds 3 new primitives, a new system global and a new MAX geometry
 *  object class property descriptor.  The property descriptor includes
 *  ParamBlock-hosted and accessor function-based properties
 *
 *  John Wainwright, 1997
 */
 
#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXclses.h"
#include "Streams.h"
#include "MSTime.h"
#include "MAXObj.h"
#include "Parser.h"
#include "LclClass.h"
#include "resource.h"

// define the new primitives using macros from SDK
#include "definsfn.h"
	def_visible_primitive( my_copy_file,	"myCopyFile");
	def_visible_primitive( my_file_in,		"myFileIn");

Value*
my_copy_file_cf(Value** arg_list, int count)
{
	// myCopyFile "old_file_name" "new_file_name"
	check_arg_count(myCopyFile, 2, count);
	BOOL result = CopyFile(arg_list[0]->to_string(), arg_list[1]->to_string(), TRUE);
	return result ? &true_value : &false_value;
}

// establish a new system global variable, first are the getter & stter fns
Value*
get_slider_time()
{
	return MSTime::intern(MAXScript_interface->GetTime());
}

Value*
set_slider_time(Value* val)
{
	MAXScript_interface->SetTime(val->to_timevalue());
	return val;
}

// stream-based function for the fileIn primitive
static Value* file_in(CharStream* source, int quiet)
{
	init_thread_locals();
	push_alloc_frame();
	three_typed_value_locals(Parser* parser, Value* code, Value* result);

	// make a new compiler instance
	CharStream* out = thread_local(current_stdout);
	vl.parser = new Parser (out);
	if (!quiet)
		source->log_to(out);

	// loop through stream compiling & evaluating all expressions
	try
	{
		source->flush_whitespace();
		while (!source->at_eos())
		{
			vl.code = vl.parser->compile(source);
			vl.result = vl.code->eval();
			if (!quiet)
				vl.result->print();
			source->flush_whitespace();
		}
		source->close();
	}
	catch (...)
	{
		// catch any errors and tell what file we are in if any
		out->puts("Error occurred during fileIn: ");
		source->sprin1(out);
		out->puts("\n");
		pop_alloc_frame();
		throw;
	}

	// if there was one, return last expression in stream as result
	if (vl.result == NULL)
		vl.result = &ok;
	else
		vl.result = vl.result->get_heap_ptr();
	pop_alloc_frame();
	return_value(vl.result);
}

Value*
my_file_in_cf(Value** arg_list, int count)
{
	// myFileIn "filename" [quiet:true]

	// pick up arguments
	check_arg_count_with_keys(myFileIn, 1, count);
	two_typed_value_locals(FileStream* file, Value* result);
	Value* quiet = key_arg_or_default(quiet, &true_value);
	type_check(quiet, Boolean, "myFileIn quiet:");

	// open a fileStream instance on the file
	vl.file = (new FileStream)->open(arg_list[0]->to_string(), "rt");
	if (vl.file == (FileStream*)&undefined)
		throw RuntimeError ("fileIn cannot open file: ", arg_list[0]);

	// pass it to the stream-based fileIn utility function
	try
	{
		vl.result = file_in(vl.file, (quiet == &true_value));
	}
	catch (...)
	{
		// catch any errors and close the temp filestream
		vl.file->close();
		throw;
	}

	// pop value locals & return fileIn result
	return_value(vl.result);
}

// define properties for the new Capsule object

#define CAPS_RADIUS		0	// these are the ParamBlock indexes
#define CAPS_HEIGHT		1
#define CAPS_CENTERS	2
#define CAPS_SIDES		3
#define CAPS_HSEGS		4
#define CAPS_SMOOTHON	5
#define CAPS_SLICEON	6
#define CAPS_SLICEFROM	7
#define CAPS_SLICETO	8
#define CAPS_GENUVS		9

static Class_ID CAPS_CLASS_ID(0x6d3d77ac, 0x79c939a9);

// property accessor functions for a new property 'aspect'
Value* 
get_caps_aspect(ReferenceTarget* obj, Value* prop, TimeValue t, Interval& valid)
{
	// compute & return capsule aspect
	IParamBlock *pb = ((Object*)obj)->GetParamBlock()->GetParamBlock();
	float h, r;
	pb->GetValue(CAPS_HEIGHT, t, h, valid);
	pb->GetValue(CAPS_RADIUS, t, r, valid);
	return Float::intern(r * 2.0f / h);
}
void
set_caps_aspect(ReferenceTarget* obj, Value* prop, TimeValue t, Value* val)
{
	throw RuntimeError ("Sorry, cannot directly set capsule aspect");
}

// MAX object class property descriptor
MAXClass capsule
	("Capsule", CAPS_CLASS_ID, GEOMOBJECT_CLASS_ID, &geom_class, md_direct_index,
		accessors,
			paramblock,
				"radius",		CAPS_RADIUS,		TYPE_FLOAT,		15.0,
				"height",		CAPS_HEIGHT,		TYPE_FLOAT,		25.0,
				"heighttype",	CAPS_CENTERS,		TYPE_INT,		1,
				"sides",		CAPS_SIDES,			TYPE_INT,		24,
				"heightsegs",	CAPS_HSEGS,			TYPE_INT,		1,
				"smooth",		CAPS_SMOOTHON,		TYPE_BOOL,		TRUE,
				"sliceon",		CAPS_SLICEON,		TYPE_BOOL,		FALSE,
				"slicefrom",	CAPS_SLICEFROM,		TYPE_FLOAT,		0.0,
				"sliceto",		CAPS_SLICETO,		TYPE_FLOAT,		0.0,
				"mapcoords",	CAPS_GENUVS,		TYPE_BOOL,		FALSE,
				end,
			fns,
				"aspect",		get_caps_aspect,	set_caps_aspect,	TYPE_UNSPECIFIED,
				end,
			end,
		end
	);

// test new rollout

#include "3DMath.h"

TCHAR *GetString(int id)
{
	static TCHAR buf1[128], buf2[128];
	static TCHAR* bp = buf1;
	TCHAR* result = NULL;
	if (hInstance)
		result = LoadString(hInstance, id, bp, sizeof(buf1)) ? bp : NULL;
	bp = (bp == buf1) ? buf2 : buf1;   // twiddle between two buffers to help multi-getstring users
	return result;
}

/* -------------------- FooControl  ------------------- */

local_visible_class (FooControl)

class FooControl : public RolloutControl
{
public:
	float		value;
	float		max, min;
	float		scale;
	EditSpinnerType spin_type;

				FooControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
					: RolloutControl(name, caption, keyparms, keyparm_count)  { tag = class_tag(FooControl); }

    static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
							{ return new FooControl (name, caption, keyparms, keyparm_count); }

				classof_methods (FooControl, RolloutControl);
	void		collect() { delete this; }
	void		sprin1(CharStream* s) { s->printf(_T("FooControl:%s"), name->to_string()); }

	void		add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCTSTR		get_control_class() { return SPINNERWINDOWCLASS; }
	void		compute_layout(Rollout *ro, layout_data* pos) { }
	BOOL		handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	void		set_enable();
};


/* -------------------- FooControl  ------------------- */

local_visible_class_instance (FooControl, "FooControl")

void
FooControl::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
	HWND	label, edit_box, spinner;
	int		left, top, width, height;
	SIZE	size;
	TCHAR*	text = caption->eval()->to_string();	

	/* add 3 controls for a spinner: the label static, value custeditbox, & spinner itself,
	 * label & edit box are given IDs that are the spinner id * control_count & that + 1, to make
	 * sure they are unique and decodable */

	parent_rollout = ro;
	control_ID = next_id();
	WORD label_id = next_id();
	WORD edit_box_id = next_id();	

	// compute bounding box, apply layout params
	layout_data pos;
	setup_layout(ro, &pos, current_y);
	Value* fw = control_param(fieldwidth);
	int spin_width = (fw == &unsupplied) ? ro->current_width / 2 : fw->to_int() + 10;
	GetTextExtentPoint32(ro->rollout_dc, text, strlen(text), &size); 	
	int lbl_width = size.cx;
	int orig_width = lbl_width + spin_width;

	pos.width = orig_width + 2;
	pos.left = pos.left + ro->current_width - pos.width; 
	pos.height = ro->text_height + 3;
	process_layout_params(ro, &pos, current_y);

	// place spinner elements
	int cex = (fw == &unsupplied) ? pos.width * lbl_width / orig_width : pos.width - spin_width;
	cex = min(cex, pos.width);
	width = lbl_width; height = ro->text_height;
	left = pos.left + cex - width - 1; top = pos.top; 
	label = CreateWindow(_T("STATIC"),
							text,
							WS_VISIBLE | WS_CHILD | WS_GROUP,
							left, top, width, height,    
							parent, (HMENU)label_id, hInstance, NULL);

	width = pos.width - cex - 13; height = ro->text_height + 3;
	left = pos.left + cex + 1; 
	edit_box = CreateWindowEx(0,
							CUSTEDITWINDOWCLASS,
							_T(""),
							WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_GROUP,
							left, top, width, height,    
							parent, (HMENU)edit_box_id, hInstance, NULL);

	left += width; width = 10; 
	spinner = CreateWindowEx(0,
							SPINNERWINDOWCLASS,
							_T(""),
							WS_VISIBLE | WS_CHILD,
							left, top, width, height,    
							parent, (HMENU)control_ID, hInstance, NULL);

    SendDlgItemMessage(parent, label_id, WM_SETFONT, (WPARAM)ro->font, 0L);
    SendDlgItemMessage(parent, edit_box_id, WM_SETFONT, (WPARAM)ro->font, 0L);

	/* setup the spinner control */

	ISpinnerControl* spin = GetISpinner(spinner);
	Value*			 range = control_param(range);
	Value*			 type = control_param(type);
	Value*			 scaleval = control_param(scale);

	if (type == n_integer)
		spin_type = EDITTYPE_INT;
	else if (type == n_worldUnits)
		spin_type = EDITTYPE_UNIVERSE;
	else if (type == n_float || type == &unsupplied)
		spin_type = EDITTYPE_FLOAT;
	else
		throw TypeError (GetString(IDS_SPINNER_TYPE_MUST_BE_INTEGER_OR_FLOAT), type);
	
	if (ro->init_values)
	{
		if (range == &unsupplied)
		{
			min = 0.0f; max = 100.0f; value = 0.0f;
		}
		else if (is_point3(range))
		{
			Point3 p = range->to_point3();
			min = p.x; max = p.y; value = p.z;
		}
		else
			throw TypeError (GetString(IDS_SPINNER_RANGE_MUST_BE_A_VECTOR), range);

		if (scaleval == &unsupplied)
		{
				scale = (spin_type == EDITTYPE_INT) ? 1.0f : 0.1f;
		}
		else
			scale = scaleval->to_float();
	}
	
    spin->LinkToEdit(edit_box, spin_type);
	spin->SetScale(scale);
    if (spin_type == EDITTYPE_INT)
	{
		spin->SetLimits((int)min, (int)max, FALSE);
		spin->SetValue((int)value, FALSE);
	}
	else if (spin_type == EDITTYPE_UNIVERSE)
	{
		spin->SetLimits(min, max, FALSE);
		spin->SetValue(value, FALSE);
	}
	else
	{
		spin->SetLimits(min, max, FALSE);
		spin->SetValue(value, FALSE);
	}
	ReleaseISpinner(spin);
}

BOOL
FooControl::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == CC_SPINNER_CHANGE)
	{
		one_value_local(arg);
		/* handle CC_SPINNER_CHANGE message received for this control - call my _T('changed') event handler with
		 * with the current spinner value as the argument */
		if (spin_type == EDITTYPE_INT)
		{
			value = (float)((ISpinnerControl *)lParam)->GetIVal();
			vl.arg = Integer::intern((int)value);
		}
		else
			vl.arg = Float::intern(value = ((ISpinnerControl *)lParam)->GetFVal()); 
		try
		{
			call_event_handler(ro, n_changed, &vl.arg, 1);
		}
		catch (...)
		{
   			SendMessage(GetDlgItem(ro->page, control_ID), WM_LBUTTONUP, 0, 0); // on error, force a buttonup to release the spinner
			throw;
		}
		pop_value_locals();
		return TRUE;
	}
	return FALSE;
}

Value*
FooControl::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if (prop == n_value)
		return (spin_type == EDITTYPE_INT) ? Integer::intern((int)value) : Float::intern(value);
	else if (prop == n_enabled)
		return enabled ? &true_value : &false_value;
	else if (prop == n_range)
		return new Point3Value (min, max, value);
	else
		return RolloutControl::get_property(arg_list, count);
}

Value*
FooControl::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if (prop == n_value)
	{
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			value = val->to_float();
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				ISpinnerControl* spin = GetISpinner(GetDlgItem(parent_rollout->page, control_ID));
				if (spin_type == EDITTYPE_INT)
					spin->SetValue((int)value, FALSE);
				else
					spin->SetValue(value, FALSE);
				ReleaseISpinner(spin);
			}
		}
	}
	else if (prop == n_range)
	{
		Point3 p;
		if (is_point3(val))
			p = val->to_point3();
		else
			throw TypeError (GetString(IDS_SPINNER_RANGE_MUST_BE_A_VECTOR), val);
		min = p.x; max = p.y; value = p.z;

		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			ISpinnerControl* spin = GetISpinner(GetDlgItem(parent_rollout->page, control_ID));
			if (spin_type == EDITTYPE_INT)
			{
				spin->SetLimits((int)p.x, (int)p.y, FALSE);
				spin->SetScale(1.0F);
				spin->SetValue((int)p.z, FALSE);
			}
			else
			{
				spin->SetLimits(p.x, p.y, FALSE);
				spin->SetScale(0.1F);
				spin->SetValue(p.z, FALSE);
			}
			ReleaseISpinner(spin);
		}
	}
	else
		return RolloutControl::set_property(arg_list, count);

	return val;
}

void
FooControl::set_enable()
{
	if (parent_rollout != NULL && parent_rollout->page != NULL)
	{
		// set spinner enable
		ISpinnerControl *spin = GetISpinner(GetDlgItem(parent_rollout->page, control_ID));
		if (enabled) spin->Enable();
		else spin->Disable();
		ReleaseISpinner(spin);
		// set editbox enable
		ICustEdit *edit = GetICustEdit(GetDlgItem(parent_rollout->page, control_ID+2));
		if (enabled) edit->Enable();
		else edit->Disable();
		ReleaseICustEdit(edit);
		// set caption enable
		HWND ctrl = GetDlgItem(parent_rollout->page, control_ID+1);
		if (ctrl)
			EnableWindow(ctrl, enabled);
	}
}

/* --------------------- plug-in init --------------------------------- */
// this is called by the dlx initializer, register the global var here
void tester_init()
{
	CharStream* out = thread_local(current_stdout);
	out->puts(_T("--* Testdlx.dlx 4.00 loaded *--\n"));
	define_system_global("mySlider", get_slider_time, set_slider_time);
	install_rollout_control(Name::intern("foo"), FooControl::create);
}

