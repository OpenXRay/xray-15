/******************************************************************************
 * Copyright 1986-2006 by mental images GmbH, Fasanenstr. 81, D-10623 Berlin,
 * Germany. All rights reserved.
 ******************************************************************************
 * Author:      Martin-Karl LeFrancois
 * Created:     2003-4-29
 * Module:      mi_opengl
 * Purpose:     Interfaces for shaders callback functions
 *
 * Exports:
 *
 * History
 *
 * Description:
 *****************************************************************************/

#ifndef MI_OPENGL_H
#define MI_OPENGL_H

#if !defined(SHADER_H)
#include <mi_raylib.h>
#endif

#if defined(__cplusplus)


#include <mi_hwshared.h> // Shared HW structures such as hwLight

// OpenGL global error state check
#ifdef DEBUG
#define miOGL_CHECK_ERROR()		    \
    do {				    \
	GLenum errCode = glGetError();	    \
	if (errCode != GL_NO_ERROR) {	    \
	mi_error("(%s, %d) ERROR: %s\n",    \
	    SRCFILE, SRCLINE,		    \
	    gluErrorString(errCode));	    \
	}				    \
    } while (0)
#else	// DEBUG
#define miOGL_CHECK_ERROR() ((void) 0)
#endif	// DEBUG

#ifdef WIN_NT
    #include <windows.h>
    // 'identifier' : identifier was truncated to 'number' characters
    // in the debug information
    #pragma warning (disable : 4786)
    #include <migl/gl.h>
    #include <migl/glu.h>
    #include <migl/wglext.h>
#else
    #include <migl/gl.h>
    #include <migl/glux.h>
    #include <migl/glx.h>
    #include <migl/glxext.h>
#endif // WIN_NT
#include <migl/glext.h>

namespace miHWSI
{

//
// Helper class for OpenGL shaders. It gives methods and information
// about what can or cannot be rendered.
//
class miOgl_render 
{
public:

    // Calls the shader function associated with the tag
    // obj_inst_tag is the tag of the instance with the shader as material, if 
    // appropriate
    virtual bool	shader_eval(miTag fct_tag, miTag obj_inst_tag)	= 0;

    // Calls the light shader associated with the tag
    virtual bool	light_eval(miTag light_inst)			= 0;

    // Get the current light information (to be call from a light shader)
    virtual bool	light_get(hwLight* l) const			= 0;

    // Go through all lights and call their shader. Enable the OpenGL light
    // for each of them and disable the remaning ones.
    virtual bool	lights_enable(
				miHWSI::miOgl_render	*oglCtx,
				int			nbLights,
				const miTag		*light)		= 0;
    
    // Draw the triangles associated with the current material, useful for
    // multi-pass shader
    virtual bool	triangles_draw()				= 0;

    // Get the current camera transformation
    virtual miMatrix&	cam_transfo(void)				= 0;
    virtual const miMatrix& cam_transfo(void) const			= 0;

    // Get the current object instance tag
    virtual miTag	object_get() const				= 0;

    // Get/set the OpenGL light index to be use in the light shader
    virtual int		light_index(void) const				= 0;
    virtual int&	light_index(void)				= 0;

    // Returns the OpenGL texture object.
    virtual GLuint	tex_eval(miTag tag)				= 0;

    // Return the options structure
    virtual const miOptions*
                        get_options() const                             = 0;

    // Evaluation methods for parameters
    virtual miColor     eval(miColor* in_param)				= 0;
    virtual miInteger   eval(miInteger* in_param)			= 0;
    virtual miScalar    eval(miScalar* param)				= 0;
    virtual miBoolean   eval(miCBoolean* param)				= 0;
    virtual miVector    eval(miVector* param)				= 0;
    virtual miMatrix*   eval(miMatrix* param)				= 0;
    virtual miTag*      eval(miTag* param)				= 0;
};

} // miHWSI

#endif /* defined(__cplusplus) */

#endif // MI_OPENGL_H
