#ifndef glr_constants_cacheH
#define glr_constants_cacheH
#pragma once

class	ECORE_API  R_constants
{
private:
	// fp, non-array versions
	ICF void				set(shared_str& name, R_constant_load& L, const Fmatrix& A)		{
		u32 location = glGetUniformLocation(L.program, *name);
		Fvector4	it[4];
		switch (L.cls)
		{
		case RC_2x4:
			it[0].set(A._11, A._21, A._31, A._41);
			it[1].set(A._12, A._22, A._32, A._42);
			CHK_GL(glProgramUniformMatrix2x4fv(L.program, location, 1, GL_FALSE, (float*)&it));
			break;
		case RC_3x4:
			it[0].set(A._11, A._21, A._31, A._41);
			it[1].set(A._12, A._22, A._32, A._42);
			it[2].set(A._13, A._23, A._33, A._43);
			CHK_GL(glProgramUniformMatrix3x4fv(L.program, location, 1, GL_FALSE, (float*)&it));
			break;
		case RC_4x4:
			it[0].set(A._11, A._21, A._31, A._41);
			it[1].set(A._12, A._22, A._32, A._42);
			it[2].set(A._13, A._23, A._33, A._43);
			it[3].set(A._14, A._24, A._34, A._44);
			CHK_GL(glProgramUniformMatrix4fv(L.program, location, 1, GL_FALSE, (float*)&it));
			break;
		default:
#ifdef DEBUG
			xrDebug::Fatal(DEBUG_INFO, "Invalid constant run-time-type for '%s'", *name);
#else
			NODEFAULT;
#endif
		}
	}
	ICF void				set(shared_str& name, R_constant_load& L, const Fvector4& A)		{
		VERIFY(RC_1x4 == L.cls);
		u32 location = glGetUniformLocation(L.program, *name);
		CHK_GL(glProgramUniform4fv(L.program, location, 1, (float*)&A));
	}
	ICF void				set(shared_str& name, R_constant_load& L, float x, float y, float z, float w)	{
		VERIFY(RC_1x4 == L.cls);
		u32 location = glGetUniformLocation(L.program, *name);
		CHK_GL(glProgramUniform4f(L.program, location, x, y, z, w));
	}

	// scalars, non-array versions
	ICF	void				set(shared_str& name, R_constant_load& L, float A)
	{
		VERIFY(RC_1x1 == L.cls);
		u32 location = glGetUniformLocation(L.program, *name);
		CHK_GL(glProgramUniform1f(L.program, location, A));
	}
	ICF	void				set(shared_str& name, R_constant_load& L, int A)
	{
		VERIFY(RC_1x1 == L.cls);
		u32 location = glGetUniformLocation(L.program, *name);
		CHK_GL(glProgramUniform1i(L.program, location, A));
	}

public:
	// fp, non-array versions
	ICF void				set		(R_constant* C, const Fmatrix& A)
	{
		VERIFY(RC_float == C->type);
		if (C->destination&RC_dest_pixel)		{ set(C->name, C->ps, A); }
		if (C->destination&RC_dest_vertex)		{ set(C->name, C->vs, A); }
	}
	ICF void				set		(R_constant* C, const Fvector4& A)
	{
		VERIFY(RC_float == C->type);
		if (C->destination&RC_dest_pixel)		{ set(C->name, C->ps, A); }
		if (C->destination&RC_dest_vertex)		{ set(C->name, C->vs, A); }
	}
	ICF void				set		(R_constant* C, float x, float y, float z, float w)
	{
		VERIFY(RC_float == C->type);
		if (C->destination&RC_dest_pixel)		{ set(C->name, C->ps, x, y, z, w); }
		if (C->destination&RC_dest_vertex)		{ set(C->name, C->vs, x, y, z, w); }
	}

	// scalars, non-array versions
	ICF	void				set(R_constant* C, float A)
	{
		VERIFY(RC_float == C->type);
		if (C->destination&RC_dest_pixel)		{ set(C->name, C->ps, A); }
		if (C->destination&RC_dest_vertex)		{ set(C->name, C->vs, A); }
	}
	ICF	void				set(R_constant* C, int A)
	{
		VERIFY(RC_int == C->type);
		if (C->destination&RC_dest_pixel)		{ set(C->name, C->ps, A); }
		if (C->destination&RC_dest_vertex)		{ set(C->name, C->vs, A); }
	}

	// fp, array versions
	ICF void				seta	(R_constant* C, u32 e, const Fmatrix& A)
	{
		VERIFY(RC_float == C->type);
		string256 str;
		sprintf(str, "%s[%d]", C->name, e);
		shared_str name = str;
		if (C->destination&RC_dest_pixel)		{ set(name, C->ps, A); }
		if (C->destination&RC_dest_vertex)		{ set(name, C->vs, A); }
	}
	ICF void				seta	(R_constant* C, u32 e, const Fvector4& A)
	{
		VERIFY(RC_float == C->type);
		string256 str;
		sprintf(str, "%s[%d]", C->name, e);
		shared_str name = str;
		if (C->destination&RC_dest_pixel)		{ set(name, C->ps, A); }
		if (C->destination&RC_dest_vertex)		{ set(name, C->vs, A); }
	}
	ICF void				seta	(R_constant* C, u32 e, float x, float y, float z, float w)
	{
		VERIFY(RC_float == C->type);
		string256 str;
		sprintf(str, "%s[%d]", C->name, e);
		shared_str name = str;
		if (C->destination&RC_dest_pixel)		{ set(name, C->ps, x, y, z, w); }
		if (C->destination&RC_dest_vertex)		{ set(name, C->vs, x, y, z, w); }

	}

	// TODO: OGL: Implement constant caching through UBOs
	ICF void					flush	()
	{
	}
};
#endif	//	glr_constants_cacheH
