/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623
 * Berlin, Germany. All rights reserved.
 ******************************************************************************
 * Author:      Tim Schroeder
 * Created:     7.9.2004
 * Module:      modules/include
 * Purpose:     Structures and types shared between directx, opengl and hwlib
 *              Structures shared between hwlib and callbacks
 * Exports:
 *****************************************************************************/

#ifndef MI_HWHSHARED_H
#define MI_HWHSHARED_H

// Assert
#ifndef miASSERT
  #ifdef DEBUG
    // Need to define this here, since DEC casts EX to int in assert()
    #define miASSERT(EX) ((EX) ? ((void) 0) : assert(EX))
  #else
    #define miASSERT(EX) ((void) 0)
  #endif // DEBUG
#endif // miASSERT

// Min / Max
#ifndef miMIN2
  #define miMIN2(a, b) ((a) < (b) ? (a) : (b))
#endif // miMIN2
#ifndef miMAX2
  #define miMAX2(a, b) ((a) > (b) ? (a) : (b))
#endif // miMAX2

// Cg include
#ifdef NV_CG
#include <migl/cg.h>
#endif

namespace miHWSI 
{

typedef enum {
    miHW_NO_PARAMS = 0,
    miHW_PARAM_VALS
} miHW_callback_mode;

//
// Hardware Parameter
//
class miHW_param
{
public:
    virtual int          type_get();
    virtual char*	 name_get();
    virtual void         value_get(void *);

};

// Class to gather and provide information about a shader and its parameters, 
// for communication with a callback for the shader. 
class miHW_callback_info
{
public:
    // set the parameter type
    virtual void	type_set(int ptype)				= 0;
    
    // set the parameter value
    virtual void	val_set(miScalar s)				= 0;
    virtual void	val_set(miColor c)				= 0;
    virtual void	val_set(miInteger i)				= 0;
    virtual void	val_set(miCBoolean b)				= 0;
    virtual void	val_set(miTag t)				= 0;
    virtual void	val_set(miVector v)				= 0;
    virtual void	val_set(miMatrix m)				= 0;

    // set a shader to provide the value of a parameter
    virtual bool	shader_connection_set(miTag t)			= 0;
    
    // current shader
    virtual miTag	shader_get()					= 0;
    
    // tag of leaf obj instance with current shader as material
    virtual miTag	obj_inst_get()					= 0;
    
    // tag of leaf light inst if in light shader
    virtual miTag	light_inst_get()				= 0;	
 
    // set the number of elements in the parameter-- 1 for primitives, possibly
    // more for arrays
    virtual void	nb_elements_set(int i)				= 0;
    
    // set whether this parameter should be ignored
    virtual void	parameter_ignore_set(miCBoolean ignore)		= 0;

    // set the name of the parameter
    virtual void	name_set(const char* pname)			= 0;
    
    // Set whether the source of this shader should be echoed
    virtual void	echo_source_set(const miCBoolean echo_on)	= 0;
 
};    
//
// Hardware Shader
//
class miHW_shader
{
public:
    virtual char*	name_get();                 // Name of the shader
    virtual int         nbparam_get();              // Number of parameter
    virtual miHW_param* param_get(int i);           // Get parameter by index
    virtual miHW_param* param_get(const char* n);   // Get parameter by name
};

//
// Return structure for lights
//
// Gives type, position, direction, and other useful light information
//
typedef struct hwLight
{
    miInteger   type;
    miVector    org;
    miVector    dir;
    miColor	energy;
    miScalar    falloff;
    miScalar    spread;
    miScalar    shdmap_bias;
    miMatrix    matrix;		// Global to local
} hwLight;

#ifdef NV_CG

//
// Helper class for Cg functions shaders. 
//
class miCg_render 
{
public:
    // Call the light shader evaluation for a CGparameter
    virtual void        light_eval(
	CGparameter	struct_param,
	const miTag	*pLightTag)					= 0;

    // Get the light information
    virtual bool	light_get( 
	hwLight		*pLight ) const					= 0;

    // Call the drawing of all triangles associate with the
    // current shader
    virtual bool	triangles_draw()				= 0;

    // Get the current camera transformation
    virtual const miMatrix& 
			cam_transfo() const				= 0;

    // Get the current inverse camera transformation
    virtual const miMatrix& 
			cam_inv_transfo() const				= 0;

    // Get the current CGprogram
    virtual CGprogram	program_get()					= 0;

    // Get the current CGcontext
    virtual CGcontext   context_get()					= 0;

    // Get the current light index 
    virtual int		light_index() const				= 0;

    // Get the current object instance tag
    virtual miTag	object_get() const				= 0;

    // Evaluate a parameter of the structure 'struct_param', create the 
    // proper type, and set its value
    virtual void        param_eval(
	CGparameter         struct_param,
	const miHW_param*   param,
	const char*	    name = NULL)				= 0;

};

#endif // NV_CG

} // miHWSI

#endif // MI_HWHSHARED_H
