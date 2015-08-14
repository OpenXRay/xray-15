#include "stdafx.h"
#pragma hdrstop

#include "../xrRender/r_constants.h"

void cl_sampler::setup(R_constant* C)
{
	CHK_GL(glUniform1i(C->samp.location, C->samp.index));
}

IC bool	p_sort(ref_constant C1, ref_constant C2)
{
	return xr_strcmp(C1->name, C2->name)<0;
}

// TODO: OGL: Use constant buffers like DX10.
BOOL	R_constant_table::parse(void* _desc, u16 destination)
{
	GLuint program = *(GLuint*)_desc;

	// Get the maximum length of the constant name and allocate a buffer for it
	GLint maxLength;
	CHK_GL(glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength));
	GLchar* name = xr_alloc<GLchar>(maxLength + 1); // Null terminator

	// Iterate all uniforms and parse the entries for the constant table.
	GLint uniformCount;
	CHK_GL(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount));

	for (GLint i = 0; i < uniformCount; i++) {
		GLint size;
		GLenum type;
		CHK_GL(glGetActiveUniform(program, i, maxLength, NULL, &size, &type, name));

		if (GL_BOOL == type ||
			GL_BOOL_VEC2 == type ||
			GL_BOOL_VEC3 == type ||
			GL_BOOL_VEC4 == type)
			type = RC_bool;
		if (GL_INT == type ||
			GL_INT_VEC2 == type ||
			GL_INT_VEC3 == type ||
			GL_INT_VEC4 == type)
			type = RC_int;

		// Rindex,Rcount,Rlocation
		u16		r_index = i;
		u16		r_type = u16(-1);
		GLuint	r_location = glGetUniformLocation(program, name);

		// TypeInfo + class
		BOOL bSkip = FALSE;
		switch (type)
		{
		case GL_FLOAT:
		case GL_BOOL:
		case GL_INT:
			r_type = RC_1x1;
			break;
		case GL_FLOAT_VEC2:
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
			r_type = RC_1x2;
			break;
		case GL_FLOAT_VEC3:
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
			r_type = RC_1x3;
			break;
		case GL_FLOAT_VEC4:
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
			r_type = RC_1x4;
			break;
		case GL_FLOAT_MAT2:
		case GL_FLOAT_MAT3:
			fatal("GL_FLOAT_MAT: unsupported number of dimensions");
			break;
		case GL_FLOAT_MAT2x4:
			r_type = RC_2x4;
			break;
		case GL_FLOAT_MAT3x4:
			r_type = RC_3x4;
			break;
		case GL_FLOAT_MAT4:
			r_type = RC_4x4;
			break;
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
			{
				// ***Register sampler***
				// We have determined all valuable info, search if constant already created
				ref_constant	C = get(name);
				if (!C)	{
					C = new R_constant();//.g_constant_allocator.create();
					C->name = name;
					C->destination = RC_dest_sampler;
					C->type = RC_sampler;
					C->handler = &sampler_binder;
					R_constant_load& L = C->samp;
					L.index = r_index;
					L.cls = RC_sampler;
					L.location = r_location;
					table.push_back(C);
				}
				else {
					R_ASSERT(C->destination == RC_dest_sampler);
					R_ASSERT(C->type == RC_sampler);
					R_ASSERT(C->handler == &sampler_binder);
					R_constant_load& L = C->samp;
					R_ASSERT(L.index == r_index);
					R_ASSERT(L.cls == RC_sampler);
					R_ASSERT(L.location == r_location);
				}
			}
			bSkip = TRUE;
			break;
		default:
			bSkip = TRUE;
			break;
		}
		if (bSkip)			continue;

		// We have determined all valuable info, search if constant already created
		ref_constant	C		=	get	(name);
		if (!C)	{
			C					= new R_constant();//.g_constant_allocator.create();
			C->name				=	name;
			C->destination		=	destination;
			C->type				=	type;
			R_constant_load& L	=	C->program;
			L.index				=	r_index;
			L.cls				=	r_type;
			L.location			=	r_location;
			table.push_back		(C);
		} else {
			C->destination		|=	destination;
			VERIFY	(C->type	==	type);
			R_constant_load& L	=	C->program;
			L.index				=	r_index;
			L.cls				=	r_type;
			L.location			=	r_location;
		}
	}
	std::sort(table.begin(), table.end(), p_sort);
	xr_free(name);
	return		TRUE;
}
