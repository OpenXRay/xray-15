#ifndef glr_constants_cacheH
#define glr_constants_cacheH
#pragma once

class	ECORE_API  R_constants
{
public:
	// fp, non-array versions
	ICF void				set		(R_constant* C, const Fmatrix& A)		{
		VERIFY		(RC_float == C->type);
		Fvector4	it[4];
		switch		(C->program.cls)
		{
		case RC_2x4:
			it[0].set			(A._11, A._21, A._31, A._41);
			it[1].set			(A._12, A._22, A._32, A._42);
			glUniformMatrix2x4fv(C->program.location, 1, GL_FALSE, (float*)&it);
			break;
		case RC_3x4:
			it[0].set			(A._11, A._21, A._31, A._41);
			it[1].set			(A._12, A._22, A._32, A._42);
			it[2].set			(A._13, A._23, A._33, A._43);
			glUniformMatrix3x4fv(C->program.location, 1, GL_FALSE, (float*)&it);
			break;
		case RC_4x4:
			it[0].set			(A._11, A._21, A._31, A._41);
			it[1].set			(A._12, A._22, A._32, A._42);
			it[2].set			(A._13, A._23, A._33, A._43);
			it[3].set			(A._14, A._24, A._34, A._44);
			glUniformMatrix4fv	(C->program.location, 1, GL_FALSE, (float*)&it);
			break;
		default:
#ifdef DEBUG
			xrDebug::Fatal		(DEBUG_INFO,"Invalid constant run-time-type for '%s'",*C->name);
#else
			NODEFAULT;
#endif
		}
	}
	ICF void				set		(R_constant* C, const Fvector4& A)		{
		VERIFY		(RC_float	== C->type);
		VERIFY		(RC_1x4		== C->program.cls);
		glUniform4fv(C->program.location, 1, (float*)&A);
	}
	ICF void				set		(R_constant* C, float x, float y, float z, float w)	{
		VERIFY		(RC_float	== C->type);
		VERIFY		(RC_1x4		== C->program.cls);
		glUniform4f	(C->program.location, x, y, z, w);
	}

	// fp, array versions
	ICF void				seta	(R_constant* C, u32 e, const Fmatrix& A)		{
		VERIFY(!"R_constants array versions not implemented");
	}
	ICF void				seta	(R_constant* C, u32 e, const Fvector4& A)		{
		VERIFY(!"R_constants array versions not implemented");
	}
	ICF void				seta	(R_constant* C, u32 e, float x, float y, float z, float w)	{
		Fvector4 data;		data.set(x,y,z,w);
		seta				(C,e,data);
	}

	// TODO: OGL: Implement constant caching through UBOs
	ICF void					flush	()
	{
	}
};
#endif	//	glr_constants_cacheH
