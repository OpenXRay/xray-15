/*	dlxClass.cpp - Sample MAXScript SDK code for adding a new Value class
 *
 *  John Wainwright, 1998
 */
 
#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXObj.h"
#include "LclClass.h"
#include "dlxClass.h"

/* --------------------  local generics and names   --------------- */

#include "lclinsfn.h"
#	include "listpro.h"
#	include "setpro.h"

define_local_generic_class

def_name (head)
def_name (tail)

/* ------------------- ListValue class instance ----------------------- */

local_visible_class_instance (ListValue, "List")

Value*
ListValueClass::apply(Value** arg_list, int count,  CallContext* cc)
{
	// list v1 v2 ... vn
	two_typed_value_locals(ListValue* result, Value* head);
	vl.result = new ListValue ();
	for (int i = 0; i < count; i++)
	{
		vl.head = arg_list[i]->eval()->get_heap_ptr();
		vl.result->append(vl.head);
	}

	return_value(vl.result);
}

/* -------------------- ListValue methods ----------------------------- */

ListValue::ListValue()
{
	tag = class_tag(ListValue);
	head = NULL;
	tail = NULL;
}

ListValue::ListValue(Value* ihead, ListValue* itail)
{
	tag = class_tag(ListValue);
	head = ihead->get_heap_ptr();
	tail = itail;
}

void
ListValue::sprin1(CharStream* s)
{
	int			i=0;
	ListValue*	t;

	s->puts("(");
	for (t = this; t->head != NULL; t = t->tail)
	{
		if (i > 0)
			s->puts(", ");
		if (++i > 20) 
		{
			s->puts("...");
			break;
		}
		t->head->sprin1(s);
	};
	s->puts(")");	
}

void
ListValue::gc_trace()
{
	// mark me & trace sub-objects
	Value::gc_trace();
	if (head->is_not_marked())
		head->gc_trace();
	if (tail->is_not_marked())	// this will recurse which might overflow for big lists, should put a loop here
		tail->gc_trace();
}

Value*
ListValue::car_vf(Value** arg_list, int count)
{
	// car <list>
	return head ? head : &undefined;
}

Value*
ListValue::cdr_vf(Value** arg_list, int count)
{
	// cdr <list>
	return head ? (Value*)tail : (Value*)&undefined;
}

void
ListValue::append(Value* v)
{
	ListValue* t = this;
	while (t->head)
		t = t->tail;
	t->head = v->get_heap_ptr();
	t->tail = new ListValue ();
}

Value*
ListValue::append_vf(Value** arg_list, int count)
{
	// append <list> <val>
	check_gen_arg_count(append, 2, count);
	append(arg_list[0]);
	return this;
}

Value*
ListValue::plus_vf(Value** arg_list, int count)
{
	return append_vf(arg_list, count);
}

Value*
ListValue::isEmpty_vf(Value** arg_list, int count)
{
	// isEmpty <list>
	return head ? &false_value : &true_value;
}

Value*
ListValue::get_count(Value** arg_list, int count)
{
	ListValue* t = this;
	int i = 0;
	while (t->head)
	{
		t = t->tail;
		i++;
	}
	return Integer::intern(i);
}

Value*
ListValue::set_count(Value** arg_list, int count)
{
	throw RuntimeError ("Can't directly set list size", this);
	return arg_list[0];
}

Value*
ListValue::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];
	if (prop == n_head)
		return head ? head : &undefined;
	else if (prop == n_tail)
		return head ? (Value*)tail : (Value*)&undefined;
	else
		return Value::get_property(arg_list, count);
}

Value*
ListValue::set_property(Value** arg_list, int count)
{
	return Value::set_property(arg_list, count);
}

Value*
ListValue::map(node_map& m)
{
	ListValue*	t;
	one_value_local(result);

	// map function over my items
	for (t = this; t->head != NULL; t = t->tail)
	{
		if (m.vfn_ptr != NULL)
			vl.result = (t->head->*(m.vfn_ptr))(m.arg_list, m.count);
		else
		{
			// temporarily replace 1st arg with this node
			Value* arg_save = m.arg_list[0];

			m.arg_list[0] = t->head;
			if (m.flags & NM_MXS_FN)
				vl.result = ((MAXScriptFunction*)m.cfn_ptr)->apply(m.arg_list, m.count);
			else
				vl.result = (*m.cfn_ptr)(m.arg_list, m.count);
			m.arg_list[0] = arg_save;
		}
		if (m.collection != NULL)
			m.collection->append(vl.result);
	}

	pop_value_locals();
	return &ok;
}

/* ------------------- SetValue class instance ----------------------- */

local_visible_class_instance (SetValue, "PSet")

Value*
SetValueClass::apply(Value** arg_list, int count,  CallContext* cc)
{
	// set v1 v2 ... vn
	two_typed_value_locals(SetValue* result, Value* head);
	vl.result = new SetValue ();
	for (int i = 0; i < count; i++)
	{
		vl.head = arg_list[i]->eval()->get_heap_ptr();
		vl.result->append(vl.head);
	}

	return_value(vl.result);
}

/* -------------------- SetValue methods ----------------------------- */

SetValue::SetValue()
{
	tag = class_tag(SetValue);
	head = NULL;
	tail = NULL;
}

void
SetValue::append(Value* v)
{
	ListValue* t = this;
	while (t->head)
	{
		if (t->head == v)
			return;  // don't add duplicates
		t = t->tail;
	}
	t->head = v->get_heap_ptr();
	t->tail = new SetValue ();
}

BOOL
SetValue::is_member(Value* v)
{
	ListValue* t;
	for (t = this; t->head; t = t->tail)
		if (t->head == v)
			return TRUE;
	return FALSE;
}

Value*
SetValue::isMember_vf(Value** arg_list, int count)
{
	// isMember <set> <val>
	check_gen_arg_count(isMember, 2, count);
	return is_member(arg_list[0]) ? &true_value : &false_value;
}

Value*
SetValue::intersection_vf(Value** arg_list, int count)
{
	// intersection <set1> <set2>
	check_gen_arg_count(intersection, 2, count);
	Value* set2 = arg_list[0];
	type_check(set2, SetValue, "intersection()");
	one_typed_value_local(SetValue* result);
	ListValue* t;
	vl.result = new SetValue();
	for (t = (ListValue*)set2; t->head; t = t->tail)
		if (is_member(t->head))
			vl.result->append(t->head);
	return_value(vl.result);
}

Value*
SetValue::union_vf(Value** arg_list, int count)
{
	// union <set1> <set2>
	check_gen_arg_count(union, 2, count);
	Value* set2 = arg_list[0];
	type_check(set2, SetValue, "union()");
	one_typed_value_local(SetValue* result);
	ListValue* t;
	vl.result = new SetValue();
	for (t = this; t->head; t = t->tail)
		vl.result->append(t->head);
	for (t = (ListValue*)set2; t->head; t = t->tail)
		if (!vl.result->is_member(t->head))
			vl.result->append(t->head);
	return_value(vl.result);
}
