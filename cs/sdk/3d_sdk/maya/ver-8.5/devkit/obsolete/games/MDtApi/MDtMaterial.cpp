#include <stdio.h>	
#include <string.h>
#include <stdlib.h>
#include <maya/MIOStream.h>

// Need to work around some file problems on the NT platform with Maya 1.0

#ifdef WIN32

#include <math.h>

// The include file flib.h needs to be modified on the NT
// platform so that HAWExport.h and NTDependencies.h not included.
//
// i.e. make changes such that flib looks like:
//
// #ifndef FCHECK
// //#include <HAWExport.h>
// #else
// #define FND_EXPORT
// #endif
// //#include <"NTDependencies.h"
// #endif
//

// The following include and typedef are sufficient to enable the 
// translator to be compiled.

typedef unsigned int uint;

#include <maya/MTypes.h>

#ifndef __uint32_t
typedef __int32 __uint32_t;
#endif

// End of the NT specific modifications (Maya NT 1.0)

#endif


// For NT version lets not redefine "getenv" and lets use it

#ifdef WIN32
#define NO_ENV_REDEF

#include <process.h>

#endif

// Maya header files

#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnReflectShader.h>
#include <maya/MColor.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlugArray.h>
#include <maya/MPlug.h>

#include <maya/MAnimControl.h>
#include <maya/MTime.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyGraph.h>

#include <stdlib.h>
#ifdef AW_NEW_IOSTREAMS
#include <iostream>
#else
#include <iostream.h>
#endif

// IFF image reading routines

#include <maya/ilib.h>
#include <iffreader.h>

#ifdef WIN32
#pragma warning(disable: 4244)
#endif // WIN32


// found in another file MDtShape.cpp

extern const char * objectName( MObject object );
extern const char * objectType( MObject object );

// Forward definitions

int
generate_textureImages( MString &shaderName,
						MString &texNode,
                        MString &texType,
                        MStringArray& texMap,
                        MDagPath &dagPath );

static unsigned char *readTextureFile( MString textureFile, MString transFile,
					int useTransparency, int &xres, int &yres, int &zres);

// Maya Games Data Translator header files

#include <MDt.h>
#include <MDtExt.h>

// Defines, macros, and magic numbers:
//
#define MT_BLOCK_SIZE		16
#define DT_BLOCK_SIZE		16
#define DT_VALID_BIT_MASK 0x00ffffff

typedef struct
{
	double	r;
	double	g;
	double	b;
	double	a;
} ShaderColor;

typedef struct
{
	float	r;
	float	g;
	float	b;
	float	a;
} MtlColor;

typedef struct
{
	float		shininess;
	float		transparency;
	MtlColor	diffuse;
	MtlColor	emissive;
	MtlColor	color;
	MtlColor	specular;
	int			lightsource;
} Material;


// The material data structure:
//
typedef struct
{
	char* 		name;			// the material name
	Material*	mtl;			// pointer to the material data
	MObject     shader;			// Maya shader
	char* 		texture_name;	// name of texture if it has one
	char*		texture_filename; // name of the texture filename if it has one.
	char*		ambient_name;
	char*		diffuse_name;
	char*		translucence_name;
	char*		incandescence_name;
	char*		bump_name;
	char*		shininess_name;
	char*		specular_name;
	char*		reflectivity_name;
	char*		reflected_name;
	int			hasAlpha;		// Flag if material has Alpha texture map
	int			isRflMap;		// Flag indicating if material is a reflection map.
} MtlStruct;



// The texture data structure:
//
typedef struct
{
	char*			name;		// the texture name
	char*           texture;	// Maya texture node name
	char*			filename;
	char*			nodeName;
	char*			placeName;
	float**		 	trans;		// the texture transform node
	int				filetexture;
	unsigned char*	image;		// the image of the file texture
	int				xres;
	int				yres;
	int				zres;
} TexStruct;


// The object structure:
//
typedef struct
{
	int			shape_ct;	// count of nodes in the shapes cache
	int			mtl_ct;		// count of materials in mtl array
	int			mtl_sz;		// size of material array
	MtlStruct*	mtl;		// array of material structures
	int			tex_ct;		// count of textures in the tex array
	int			tex_sz;		// size of texture array
	TexStruct*	tex;		// array of texture structures
} DtPrivate;


// private function prototypes
//
void  				mtlCreateCaches 	(void);
static void			MtAddMtl			(DtPrivate* local, MObject mtl);
static MtlStruct*	MtFindMtl			(DtPrivate* local,char* name);
static int			MtAddTex			(DtPrivate* local,char* texture, char *filename,
							 			 int filetexture,unsigned char* image,
										 int xres,int yres,int zres);
static TexStruct*	MtFindTex			(DtPrivate* local,char* name);
Material*			shader_material_multi(MObject alShader,Material* material,int index);
int		shader_texture2(MObject alShader, MObject object, MDagPath &dagPath);
int     shader_multiTextures(MObject alShader, MObject object, MDagPath &dagPath);

// private data
//
static DtPrivate* local = NULL;


// =================================================================================
// ------------------------------  PUBLIC  FUNCTIONS  ------------------------------
// =================================================================================

//  ========== DtExt_MtlGetShader ==========
//
//  SYNOPSIS
//	Return the MObject for the material
//

int DtExt_MtlGetShader( int mtlID, MObject &obj )
{
	if ( local &&
		(mtlID >= 0) &&
		(mtlID < local->mtl_ct) )
	{
		obj = local->mtl[mtlID].shader;
		return 1;
	}
	return 0;

}

//  ========== DtMtlGetName ==========
//
//  SYNOPSIS
//	Return the material name used by the group.
//

int DtMtlGetName(int shapeID, int groupID, char** name)
{
	MObject group;
	int		ret = 0;

	if( DtShapeGetCount() == 0 ) 
	{
		return(0);
	}

    // Find the material group node by group ID.
    //

	ret = DtExt_ShapeGetShader( shapeID, groupID, group );

	if( (1 == ret) && (! group.isNull() ) ) 
	{
		*name = (char*)objectName( group );
	}
	
	return 1;

}  // DtMtlGetName //


//  ========== DtMtlGetNameByID ==========
//
//  SYNOPSIS
//	Return the material name used by the group.
//

int DtMtlGetNameByID( int LID, char** LName )
{
	*LName = NULL;

	if ( local )
		*LName = local->mtl[LID].name;

	return 1;

}  // DtMtlGetNameByID //


//  ============== DtMtlGetSceneCount ==============
//
//  SYNOPSIS
//	Return the number of materials in the scene.
//
int DtMtlGetSceneCount( int* LMatNum )
{
	*LMatNum = 0;
	
	if ( local )
		*LMatNum = local->mtl_ct;

	return 1;

}  // DtMtlGetSceneCount //


//  ========== DtMtlGetID ==========
//
//  SYNOPSIS
//	Return the material id used by the group.
//

int DtMtlGetID( int shapeID, int groupID, int* mtl_id )
{
	MObject 	group;
	int			ret = 0;
	MtlStruct	*mtl = NULL;
	int			i = 0;

	*mtl_id = -1;
	if( DtShapeGetCount() == 0 ) 
	{	
		return(0);
	}

	if( NULL == local ) 
	{
		return 0;
	}

    // Find the material group node by group ID.
    //
	ret = DtExt_ShapeGetShader( shapeID, groupID, group );
	if( (1 == ret) && (! group.isNull()) )
	{
       if( NULL == objectName( group ) )
            return 0;

		for( i = 0; i < local->mtl_ct; i++ )
		{
			mtl = &local->mtl[i];
			
			if( mtl && (strcmp( mtl->name, objectName( group ) ) == 0) ) 
			{
				*mtl_id = i;
				return 1;
			}
		}
	}

	return 1;
}  // DtMtlGetID //


//  ========== DtPolygonMtlGetName ==========
//
//  SYNOPSIS
//	Return the material name used by the given polygon 
//      in the group.
//
int  DtPolygonMtlGetName( int shapeID, int groupID, int /* index */,char** name)
{
    DtMtlGetName( shapeID, groupID, name );

    return 1;

}  // DtPolygonMtlGetName //


//  ========== DtMtlGetAmbientClr ==========
//
//  SYNOPSIS
//	Return the ambient color for the material.
//

int  DtMtlGetAmbientClr( char* name, int /* index */, float *red, float *green, float *blue )
{
    MtlStruct	*material = NULL;

    // Initialize return values.
    //
    *red   = 0.0;
    *green = 0.0;
    *blue  = 0.0;

    // Search for the material by name.
    //
    if((material = MtFindMtl( local, name )) == NULL) 
	{
		return( 0 );
	}

    if ( material->mtl == NULL )
	    return( 0 );

    // return values
    //
    *red   = material->mtl->color.r;
    *green = material->mtl->color.g;
    *blue  = material->mtl->color.b;

    return( 1 );
}  // DtMtlGetAmbientClr //


//  ========== DtMtlGetAllbyID ==========
//
//  SYNOPSIS
//	Return lot's of stuff for the material.
//
int  DtMtlGetAllClrbyID(int mtl_id, int /* index */, 
						float *ared, float *agreen, float *ablue,
						float *dred, float *dgreen, float *dblue,
						float *sred, float *sgreen, float *sblue,
						float *ered, float *egreen, float *eblue,
						float *shininess, float *transparency)
{
	MtlStruct	*mtl = NULL;

    // Initialize return values.
    //
    *ared = 0.0;
	*dred = 0.0;
	*sred = 0.0;
	*ered = 0.0;
    *agreen = 0.0;
	*dgreen = 0.0;
	*sgreen = 0.0;
	*egreen = 0.0;
    *ablue = 0.0;
	*dblue = 0.0;
	*sblue = 0.0;
	*eblue  =0.0;
    *transparency = 0.0;
	*shininess = 0.0;

    if( NULL == local ) 
	{
		return(0);
	}
    if( mtl_id >= local->mtl_ct )
	{
		DtExt_Err(" Material #%d. was not found, bad material ID ?\n",mtl_id);
		return(0);
	}

    mtl = &local->mtl[mtl_id];
	
	// Lets check that this is a valid material
	
	if ( !mtl || !mtl->mtl )
	{
		DtExt_Err(" Didn't find a valid material pointer for Material %d\n",
									mtl_id );
		return( 0 );
	}

    *ared   = mtl->mtl->color.r;
    *agreen = mtl->mtl->color.g;
    *ablue  = mtl->mtl->color.b;
    
	*dred   = mtl->mtl->diffuse.r;
    *dgreen = mtl->mtl->diffuse.g;
    *dblue  = mtl->mtl->diffuse.b;
	
    *sred   = mtl->mtl->specular.r;
    *sgreen = mtl->mtl->specular.g;
    *sblue  = mtl->mtl->specular.b;
    
	*ered   = mtl->mtl->emissive.r;
    *egreen = mtl->mtl->emissive.g;
    *eblue  = mtl->mtl->emissive.b;
    
	*shininess    = mtl->mtl->shininess;
	
    *transparency = mtl->mtl->transparency;
	
	return(1);

}  // DtMtlGetAmbientClrbyID //


//  ========== DtMtlGetDiffuseClr ==========
//
//  SYNOPSIS
//	Return the diffues color of the material.
//

int  DtMtlGetDiffuseClr( char* name, int /* index */, float *red, float *green, float *blue )
{
    MtlStruct	*material = NULL;

    // Initialize return values
    //
    *red   = 0.0;
    *green = 0.0;
    *blue  = 0.0;

    // Search for the material by name
    //
    if((material = MtFindMtl( local, name )) == NULL) 
	{
		return( 0 );
	}

	if ( material->mtl == NULL )
		return( 0 );

    // Return values.
    //
    *red   = material->mtl->diffuse.r;
    *green = material->mtl->diffuse.g;
    *blue  = material->mtl->diffuse.b;

    return( 1 );

}  // DtMtlGetDiffuseClr //



//  ========== DtMtlGetSpecularClr ==========
//
//  SYNOPSIS
//	Return the specular color of the material.
//

int  DtMtlGetSpecularClr( char* name, int /* index */, float *red, float *green, float *blue )
{
    MtlStruct	*material = NULL;

    // Initialize return values.
    //
    *red   = 0.0;
    *green = 0.0;
    *blue  = 0.0;

    // Search for the material by name.
    //
    if((material = MtFindMtl( local, name )) == NULL) 
	{
		return( 0 );
	}

	if ( material->mtl == NULL )
		return( 0 );
		
    // Return values.
    //
    *red =   material->mtl->specular.r;
    *green = material->mtl->specular.g;
    *blue =  material->mtl->specular.b;

    return( 1 );

}  // DtMtlGetSpecularClr //



//  ========== DtMtlGetEmissiveClr ==========
//
//  SYNOPSIS
//	Return the emissive color of the material.
//
int  DtMtlGetEmissiveClr( char* name, int /* index */, float *red, float *green, float *blue )
{
    MtlStruct	*material = NULL;

    // Initialize return values.
    //
    *red   = 0.0;
    *green = 0.0;
    *blue  = 0.0;

    // Search for the material by name.
    //
    if((material = MtFindMtl( local, name )) == NULL) 
	{
		return( 0 );
	}

    if ( material->mtl == NULL )
	    return( 0 );

    // Return values.
    //
    *red =   material->mtl->emissive.r;
    *green = material->mtl->emissive.g;
    *blue =  material->mtl->emissive.b;

    return( 1 );

}  // DtMtlGetEmissiveClr //


//  ========== DtMtlGetShininess ==========
//
//  SYNOPSIS
//	Return the shininess of the material.
//
int  DtMtlGetShininess( char* name, int /* index */, float *shininess )
{
    MtlStruct	*material = NULL;

    // Initialize return value.
    //
    *shininess = 0.0;

    // Search for the material by name.
    //
    if((material = MtFindMtl( local, name )) == NULL) return(0);

    if ( material->mtl == NULL )
	     return( 0 );

    // return values
    //
    *shininess = material->mtl->shininess;

    return( 1 );

}  // DtMtlGetShininess //


//  ========== DtMtlGetTransparency ==========
//
//  SYNOPSIS
//	Return the transparency of the material.
//

int  DtMtlGetTransparency( char* name, int /* index */, float *transparency )
{
    MtlStruct	*material = NULL;

    // initialize return value
    //
    *transparency = 0.0;

    // search for the material by name
    //
    if((material = MtFindMtl( local, name )) == NULL) return( 0 );

    if ( material->mtl == NULL )
	    return( 0 );

    // return values
    //
    *transparency =  material->mtl->transparency;

    return( 1 );

}  // DtMtlGetTransparency //


//  ========== DtMtlGetLightSource ==========
//
//  SYNOPSIS
//  Return the shininess of the material.
//
int  DtMtlGetLightSource( char* name, int /* index */, int *lightsource )
{
    MtlStruct   *material = NULL;

    // initialize return value
    //
    *lightsource = 0;

    // search for the material by name
    //
    if((material = MtFindMtl( local, name )) == NULL) return(0);

    if ( material->mtl == NULL )
	    return( 0 );

    // return values
    //
    *lightsource = material->mtl->lightsource;

    return( 1 );

}  // DtMtlGetLightSource //


//  ========== DtMtlIsRflMap ==========
//
//  SYNOPSIS
//	Returns true if the material is a reflection
//	map False otherwise.
//

int  DtMtlIsRflMap( char* name )
{
    MtlStruct *    material = MtFindMtl( local, name );

    if( !material) 
	{
		return( 0 ); 
	}
	else 
	{
		return( material->isRflMap );
	}
}  // DtMtlRflMap //

//  ========== DtMtlHasAlpha ==========
//
//  SYNOPSIS
//  Returns true if the material is a reflection
//  map False otherwise.
//

int  DtMtlHasAlpha( char* name )
{
    MtlStruct *    material = MtFindMtl( local, name );

    if( !material)
    {
        return( 0 );
    }
    else
    {
        return( material->hasAlpha );
    }

}  // DtMtlHasAlpha //

//  ========== DtMtHasAlphaByID ==========
//
//  SYNOPSIS
//  Return the material hasAlpha texture map by the group.
//

int DtMtlHasAlphaByID( int mtl_id )
{
    MtlStruct   *mtl = NULL;
        
    if( NULL == local )
    {
        return(0);
    }   
    if( mtl_id >= local->mtl_ct )
    {
        DtExt_Err(" Material #%d. was not found, bad material ID ?\n",mtl_id);
        return(0);
    }   
    
    mtl = &local->mtl[mtl_id];
    
    // Lets check that this is a valid material
    
    if ( !mtl )
    {
        DtExt_Err(" Didn't find a valid material pointer for Material %d\n",
                                    mtl_id );
        return( 0 );                
    }   

	return ( mtl->hasAlpha );

    
}  // DtMtlHasAlphaByID //


//  ========== DtMtlIsValid ==========
//
//  SYNOPSIS
//	Return 1 if none of the attributes of the material have changed.
//

int  DtMtlIsValid(char* name, int valid_bit)
{
    int		ret = 1;

    // search for the material by name
    //
    if ( MtFindMtl(local, name) == NULL ) 
		return(0);

    // return valid state
    //
    return(ret);

}  // DtMtlIsValid //


//  ========== DtMtlsUpdate ==========
//
//  SYNOPSIS
//  Update materials
//
void DtMtlsUpdate( void )
{
	unsigned int   LCnt,LNum;
	MtlStruct* LMtl;

	for( LCnt = 0, LNum = local->mtl_ct, LMtl = local->mtl;
			LCnt < LNum;
			LCnt++, LMtl++ )
	{
		shader_material_multi( LMtl->shader, LMtl->mtl, 0 );
	}

}

//  ========== DtTextureGetSceneCount ==========
//
//  SYNOPSIS
//	Return the count of textures in the entire scene.
//

int  DtTextureGetSceneCount(int *count)
{
	if( NULL != local )
	{
		*count = local->tex_ct;
	}

    return(1);

}  // DtTextureGetSceneCount //


//  ========== DtTextureGetCount ==========
//
//  SYNOPSIS
//	Return the number of textures for the given shape.
//

int  DtTextureGetCount(int shapeID, int *count)
{
	int		texCnt = 0;

    // initialize return value
    //
    *count = 0;

	if ( local == NULL )
		return( 0 );


    // check for error
    //
    if((shapeID < 0) || (shapeID >= local->shape_ct)) return(0);

    // return number of textures found
    //
	DtExt_ShapeGetTexCnt(shapeID,&texCnt);

	*count=texCnt;

    return(1);

}  // DtTextureGetCount //

//  ========== DtTextureGetFileName ==========
//
//  SYNOPSIS
//  Return the texture file name of the material.
//

int  DtTextureGetFileName(char *mtl_name, char **name)
{

    // initialize return value
    //
    *name = NULL;


    // look up the material by name
    //

    MtlStruct  *mtl = MtFindMtl(local, mtl_name);
    if (mtl == NULL)
    {
        return(0);  // error, bad material name
    }


    // return texture name
    //
    *name = mtl->texture_filename;
    return(1);

}  // DtTextureGetFileName //


//  ========== DtTextureGetName ==========
//
//  SYNOPSIS
//	Return the texture name of the material.
//

int  DtTextureGetName(char* mtl_name,char** name)
{
    // initialize return value
    //
    *name = NULL;

    // look up the material by name
    //
    MtlStruct  *mtl = MtFindMtl(local, mtl_name);
    if(mtl == NULL) 
	{
		DtExt_Err(" \"%s\" was not found, bad material name ?\n", mtl_name );
		return(0);  // error, bad material name
	}

    // return texture name
    //
    *name = mtl->texture_name;
    return(1);

}  // DtTextureGetName //


//  ========== DtTextureGetNameMulti ==========
//
//  SYNOPSIS
//  Return the texture name of the material.
//

int  DtTextureGetNameMulti(char* mtl_name, char *texType, char** name)
{
    // initialize return value
    //
    *name = NULL;
    
    // look up the material by name
    //
    MtlStruct  *mtl = MtFindMtl(local, mtl_name);
    if(mtl == NULL) 
    {
        DtExt_Err(" \"%s\" was not found, bad material name ?\n", mtl_name );
        return(0);  // error, bad material name
    }   
    
    // return texture name
    //

	if ( !strcmp( texType, "color" ) )
		*name = mtl->texture_name;
    else if ( !strcmp( texType, "opacity" ) )
	    *name = mtl->texture_name;
	else if ( !strcmp( texType, "ambient" ) )
		*name = mtl->ambient_name;
    else if ( !strcmp( texType, "diffuse" ) )
	    *name = mtl->diffuse_name;
    else if ( !strcmp( texType, "translucence" ) )
        *name = mtl->translucence_name;
	else if ( !strcmp( texType, "incandescence" ) )
		*name = mtl->incandescence_name;
	else if ( !strcmp( texType, "bump" ) )
        *name = mtl->bump_name;
    else if ( !strcmp( texType, "shininess" ) )
	    *name = mtl->shininess_name;
    else if ( !strcmp( texType, "specular" ) )
        *name = mtl->specular_name;
    else if ( !strcmp( texType, "reflectivity" ) )
        *name = mtl->reflectivity_name;
    else if ( !strcmp( texType, "reflected" ) )
        *name = mtl->reflected_name;
 
    return(1);
    
}  // DtTextureGetName //

//  ========== DtTextureGetID ==========
//
//  SYNOPSIS
//	Return the texture id of the material.
//
int  DtTextureGetID(int mtl_id,int* txt_id)
{
	int		i = 0;
	TexStruct	*tex = NULL;
	MtlStruct	*mtl = NULL;

    // initialize return value
    //
    *txt_id = -1;

    if( NULL == local ) 
	{
		return 0;
	}
    if(mtl_id >= local->mtl_ct)
	{
		DtExt_Err(" Material #%d. was not found, bad material ID ?\n",mtl_id);
		return(0);
	}

    mtl = &local->mtl[mtl_id];
    if(mtl && mtl->texture_name!=NULL)
    {
		for (i = 0; i < local->tex_ct; i++)
		{
			tex = &local->tex[i];
			if(tex && (strcmp(tex->name, mtl->texture_name) == 0) ) 
			{
				*txt_id = i;
				return(1);
			}
		}
    }
    return(1);

}  // DtTextureGetID //


//  ========== DtTextureGetIDMulti ==========
//
//  SYNOPSIS
//  Return the texture id of the material.
//
int  DtTextureGetIDMulti(int mtl_id,char *texType, int* txt_id)
{
    int     i = 0;
    TexStruct   *tex = NULL;
    MtlStruct   *mtl = NULL;
	char		*name = NULL;
	
    // initialize return value
    //
    *txt_id = -1;

    if( NULL == local )
    {
        return 0;
    }
    if(mtl_id >= local->mtl_ct)
    {
        DtExt_Err(" Material #%d. was not found, bad material ID ?\n",mtl_id);
        return(0);
    }

    mtl = &local->mtl[mtl_id];

    if ( !strcmp( texType, "color" ) )
        name = mtl->texture_name; 
    else if ( !strcmp( texType, "opacity" ) )
	    name = mtl->texture_name;
    else if ( !strcmp( texType, "ambient" ) )
        name = mtl->ambient_name;
    else if ( !strcmp( texType, "diffuse" ) )
        name = mtl->diffuse_name;
    else if ( !strcmp( texType, "translucence" ) )
        name = mtl->translucence_name;
    else if ( !strcmp( texType, "incandescence" ) )
        name = mtl->incandescence_name;
    else if ( !strcmp( texType, "bump" ) )
		name = mtl->bump_name;
    else if ( !strcmp( texType, "shininess" ) )
	    name = mtl->shininess_name;
	else if ( !strcmp( texType, "specular" ) )
        name = mtl->specular_name;
    else if ( !strcmp( texType, "reflectivity" ) )
        name = mtl->reflectivity_name;
    else if ( !strcmp( texType, "reflected" ) )
        name = mtl->reflected_name;

    if ( mtl && name != NULL)
    {
        for (i = 0; i < local->tex_ct; i++)
        {
            tex = &local->tex[i];
            if(tex && (strcmp(tex->name, name) == 0) )
            {
                *txt_id = i;
                return(1);
            }
        }
    }

    return(1);

}  // DtTextureGetID //

//  ========== DtTextureGetNameID ==========
//
//  SYNOPSIS
//	Return the texture name by texture ID. The name is
//	stored in an internal buffer and should not be modified
//	by the caller.
//

int  DtTextureGetNameID(int textureID,char** name)
{
    // check for error
    //
    if(textureID >= local->tex_ct)
    {
		*name = NULL;
		return(0);
    }

    // return a pointer to the name
    //
    *name = local->tex[textureID].name;
    return(1);

}  // DtTextureNameID //

//  ========== DtTextureGetNameID ==========
//
//  SYNOPSIS
//  Return the texture name by texture ID. The name is
//  stored in an internal buffer and should not be modified
//  by the caller.
//

int  DtTextureGetFileNameID(int textureID, char **name)
{

    // check for error
    //

    if (textureID >= local->tex_ct)
    {
    *name = NULL;
    return(0);
    }

    // return a pointer to the name
    //
    *name = local->tex[textureID].filename;
    return(1);

}  // DtTextureFileNameID //


//  ========== DtTextureGetWrap ==========
//
//  SYNOPSIS
//	Return the wrap type of the texture.
//

int  DtTextureGetWrap(char* name, int *horizontal, int *vertical)
{
    // double wraps, wrapt;

    // initialize return value
    //
    *horizontal = DT_REPEAT;
    *vertical = DT_REPEAT;

    // search for the texture by name
    //
    TexStruct  *tex = MtFindTex(local, name);

    if ( tex == NULL ) return(0);
    if ( tex->placeName == NULL ) return ( 0 );

    // get pointer to texture node
    //
    MStatus status;
    MString command;
    MIntArray wrapU, wrapV;

    command = MString("getAttr ") + MString( tex->placeName ) +
                                        MString(".wrapU");

    status = MGlobal::executeCommand( command, wrapU );

    if ( status != MS::kSuccess )
    {
           printf( "bad return from \"%s\"\n", command.asChar() );
           return 0;
    }

    command = MString("getAttr ") + MString( tex->placeName ) +
                                        MString(".wrapV");
                                        
    status = MGlobal::executeCommand( command, wrapV );
    
    if ( status != MS::kSuccess )
    {
           printf( "bad return from \"%s\"\n", command.asChar() );
           return 0;
    }      

    // return the horizontal wrap type
    //
    *horizontal = wrapU[0] ? DT_REPEAT : DT_CLAMP;

    // return the vertical wrap type
    //
	*vertical = wrapV[0] ? DT_REPEAT : DT_CLAMP;

    return(1);

}  // DtTextureGetWrap //


//  ========== DtTextureGetMode ==========
//
//  SYNOPSIS
//	Return the texture mode of the texture.
//

int  DtTextureGetMode(char* name, int *mode)

{
    // initialize return value
    //
    *mode = DT_DECAL;
#if 0
    // search for the texture by name
    //
    TexStruct  *tex = MtFindTex(local, name);
    if(tex == NULL) return(0);

    // return the model type
    //
	*mode = DT_DECAL;
#endif
    return(1);

}  // DtTextureGetMode //


//  ========== DtTextureGetBlendClr ==========
//
//  SYNOPSIS
//	Return the texture blend color.
//

int  DtTextureGetBlendClr(char* name, float *red, float *green, float *blue)
{

    // initialize return value
    //
    *red   = 1.0;
    *green = 1.0;
    *blue  = 1.0;

    // search for the texture by name
    //
    TexStruct  *tex = MtFindTex(local, name);
    if(tex == NULL) return(0);


    MStatus status;
    MString command;
    MDoubleArray colorGainR, colorGainG, colorGainB;

    command = MString("getAttr ") + MString( tex->nodeName ) +
                                        MString(".colorGainR");
    status = MGlobal::executeCommand( command, colorGainR );
    
    if ( status != MS::kSuccess )
    {
           printf( "bad return from \"%s\"\n", command.asChar() );
           return 0;
    }      

    command = MString("getAttr ") + MString( tex->nodeName ) +
                                        MString(".colorGainG");
                                        
    status = MGlobal::executeCommand( command, colorGainG );
    
    if ( status != MS::kSuccess )
    {
           printf( "bad return from \"%s\"\n", command.asChar() );
           return 0;
    }      

    command = MString("getAttr ") + MString( tex->nodeName ) +
                                        MString(".colorGainB");
                                        
    status = MGlobal::executeCommand( command, colorGainB );
    
    if ( status != MS::kSuccess )
    {
           printf( "bad return from \"%s\"\n", command.asChar() );
           return 0;
    }      

    if( tex->filetexture )
    {
        // return the blend color
        //
        *red = colorGainR[0];
        *green = colorGainG[0];
        *blue = colorGainB[0];
    }

    return(1);

}  // DtTextureGetBlendClr //


//  ========== DtTextureGetImageSizeByID ==========
//
//  SYNOPSIS
//	Return the size and number of components in the texture image.
//
int DtTextureGetImageSizeByID(int LID,int* LXSize,int* LYSize,int* LComponents)
{
	*LXSize = *LYSize = *LComponents = 0;

	TexStruct*	LTex=&(local->tex[LID]);
	
	if ( LTex == NULL ) 
		return(0);

	*LXSize = LTex->xres;
	*LYSize = LTex->yres;
	*LComponents = LTex->zres;
	
	return(1);

}  // DtTextureGetImageSizeByID //



//  ========== DtTextureGetImageSize ==========
//
//  SYNOPSIS
//	Return the size and number of components in the texture image.
//
int DtTextureGetImageSize(char* name,int* width,int* height,int* components)
{
    // initialize return value
    //
	*width=0;
	*height=0;
	*components=0;

    // search for the texture by name
    //
	TexStruct*	tex=MtFindTex(local,name);
	if ( tex == NULL ) 
		return(0);

    // return values
    //
	*width = tex->xres;
	*height = tex->yres;
	*components = tex->zres;

	return(1);

}  // DtTextureGetImageSize //


//  ========== DtTextureGetImageByID ==========
//
//  SYNOPSIS
//	Return the texture image.
//
int DtTextureGetImageByID(int LID,unsigned char** LImage)
{
	TexStruct*	LTex=&(local->tex[LID]);

	*LImage = (unsigned char*)LTex->image;

	return(1);

} // DtTextureGetImageByID //


//  ========== DtTextureGetImage ==========
//
//  SYNOPSIS
//	Return the texture image.
//
int  DtTextureGetImage(char* name, unsigned char** img)
{
    // initialize return value
    //
    *img = NULL;

    // search for the texture by name
    //

    TexStruct*	tex=MtFindTex(local,name);
    if ( tex == NULL ) 
	{
		DtExt_Err(" \"%s\" Texture was Not Found\n",name);
		return(0);
	}

    // return value
    //
    *img = (unsigned char* )tex->image;

    return(1);

}  // DtTextureGetImage //

//  ========== DtTextureGetTranslation ==========
//
//  SYNOPSIS
//	Return the texture image translation.
//

int  DtTextureGetTranslation(char* name, float *s, float *t)
{
	double	uoffset = 0.0, voffset = 0.0;

    // initialize return values
    //
    *s = 0.0;
    *t = 0.0;

    // search for the texture by name
    //
    TexStruct  *tex = MtFindTex(local, name);
    if ( tex == NULL ) return(0);
    if ( tex->placeName == NULL ) return ( 0 );


    // get pointer to texture 
    //

    MStatus status;
    MString command;
    MDoubleArray translate;

    if ( getenv("MDT_USE_TEXTURE_TRANSFORMS" ) )
    {
        command = MString("getAttr ") + MString(tex->placeName) +
        			                    MString(".translateFrame");
    } else {

    	command = MString("getAttr ") + MString( tex->placeName ) +
                                        MString(".offset");
	}

    status = MGlobal::executeCommand( command, translate );

    if ( status != MS::kSuccess )
    {
           printf( "bad return from \"%s\"\n", command.asChar() );
           return 0;
    }


    // get pointer to texture transform node
    //

	uoffset = translate[0];
	voffset = translate[1];

    // return translation values
    //
    *s = (float)uoffset;
    *t = (float)voffset;

    return(1);

}  // DtTextureGetTranslation //


//  ========== DtTextureGetRotation ==========
//
//  SYNOPSIS
//	Return the texture image rotation.
//

int  DtTextureGetRotation(char* name, float *angle)
{
	double	rotation = 0.0;

    // initialize return values
    //
    *angle = 0.0;

    // search for the texture by name
    //
    TexStruct  *tex = MtFindTex(local, name);
    if ( tex == NULL ) return(0);
    if ( tex->placeName == NULL ) return ( 0 );


    // If we are doing sampling then the angle is really 0 and we should
    // return that value
    
    if ( DtExt_softTextures() )
    {
        return 1;
    }   


    MStatus status;
    MString command;
    MDoubleArray rotateFrame;

    if ( getenv("MDT_USE_TEXTURE_TRANSFORMS" ) )
    {
        command = MString("getAttr ") + MString(tex->placeName) +
                            MString(".rotateFrame");
	} else {
	    command = MString("getAttr ") + MString(tex->placeName) +
                            MString(".rotateUV");
    }

    status = MGlobal::executeCommand( command, rotateFrame );
    
    if ( status != MS::kSuccess ) 
	{
           cerr << "bad return from " << command.asChar() << endl;
           return 0;
    }                           
    
    // get pointer to texture transform node
    //

	rotation = rotateFrame[0];

    // return rotation values
    //
    *angle = (float)rotation;

    return(1);

}  // DtTextureGetRotation //


//  ========== DtTextureGetScale ==========
//
//  SYNOPSIS
//	Return the scale factors of the image.
//

int  DtTextureGetScale(char* name, float *s, float *t)
{
	double	uscale = 1.0, vscale = 1.0;

    // initialize return values
    //
    *s = 1.0;
    *t = 1.0;

    // search for the texture by name
    //
    TexStruct  *tex = MtFindTex(local, name);
    if ( tex == NULL ) return(0);
	if ( tex->placeName == NULL ) return ( 0 );


	// If we are doing sampling then the scale is really 1 and we should
	// return that value

	if ( DtExt_softTextures() )
	{
		return 1;
	}


    // get pointer to texture 
    //

	MStatus status;
	MString command;
	MDoubleArray repeatUV;

    command = MString("getAttr ") + MString( tex->placeName ) +
                                        MString(".repeatUV");

    status = MGlobal::executeCommand( command, repeatUV );

    if ( status != MS::kSuccess ) 
	{
           printf( "bad return from \"%s\"\n", command.asChar() );
           return 0;
    }

    // return translation values
    //
	uscale = repeatUV[0];
	vscale = repeatUV[1];
    *s = (float)uscale;
    *t = (float)vscale;

    return(1);

}  // DtTextureGetScale //


//  ========== DtTextureGetCenter ==========
//
//  SYNOPSIS
//	Return the center of rotation and scale transformations.
//

int  DtTextureGetCenter(char* /*name*/, float *x, float *y)
{

    // initialize return values
    //
    *x = 0.0;
    *y = 0.0;

#if 0
    // search for the texture by name
    //
    TexStruct  *tex = MtFindTex(local, name);
    if(tex == NULL) return(0);


    // get pointer to texture transform node
    //
    AlTexture  *alTex = tex->texture;


    // return translation values
    //
    *x = 0.0;
    *y = 0.0;
#endif

    return(1);

}  // DtTextureGetCenter //


//  ========== DtTextureIsValid ==========
//
//  SYNOPSIS
//	Returns 1 if none of the texture attributes have
//	changed for the current frame.
//

int  DtTextureIsValid(char* name, int valid_bit)
{
    int		ret = 1;

    // search for the texture by name
    //
    TexStruct  *tex = MtFindTex(local, name);
    if ( tex == NULL ) return(0);

    // return valid state
    //
    return(ret);

}  // DtTextureIsValid //



//  ========= addElemnt  ===============
//
//	Helper function to ensure unique values in the keyframe array
//

static bool addElement( MIntArray  *intArray, int newElem )
{
    unsigned int currIndex;

    for ( currIndex = 0; currIndex < intArray->length(); currIndex++ )
    {
        if ( newElem == (*intArray)[currIndex] ) // Don't add if it's there already
            return false;

        if ( newElem < (*intArray)[currIndex] )
        {
            intArray->insert( newElem, currIndex );
            return true;
        }
    }

    // If we made it here it should go at the end...
    intArray->append( newElem );
    return true;
}

//  ========== DtMtlGetAnimKeys ==========
//
//  SYNOPSIS
//  Returns an array of keyframes found in the given mtlID 
//
//  User passes in a pointer to a MIntArray, user needs to delete the
//  array when done with it.
//  this looks for animCurves on the shader object

int DtMtlGetAnimKeys( int mtlID, MIntArray *keyFrames )
{
    MStatus         status;
    MObject         mtlNode;
    
    MObject         anim;
    MFnDependencyNode dgNode;
    MDagPath        dagPath;
    
    int             currKey,
                    numKeys,
                    keyTime,
                    stat;
                    
    MItDependencyGraph::Direction direction = MItDependencyGraph::kUpstream;
    MItDependencyGraph::Traversal traversalType = MItDependencyGraph::kBreadthFirst;
    MItDependencyGraph::Level level = MItDependencyGraph::kNodeLevel;
    
    MFn::Type filter = MFn::kAnimCurve;
    
    // A quick check to see if the user has actually given us a valid
    // pointer.
    
    if ( !keyFrames )
    {
        return 0;
    }   
    
    stat = DtExt_MtlGetShader( mtlID, mtlNode );
    if ( 1 != stat )
    {
        cerr << "DtExt_MtlGetShader returned error" << endl;
        return 0;
    }   
    
    MItDependencyGraph dgIter( mtlNode, filter, direction,
                            traversalType, level, &status );
                            
    for ( ; !dgIter.isDone(); dgIter.next() )
    {        
        anim = dgIter.thisNode( &status );
        MFnAnimCurve animCurve( anim, &status );
        if ( MS::kSuccess == status ) 
        {
            numKeys = animCurve.numKeyframes( &status );
            for ( currKey = 0; currKey < numKeys; currKey++ )
            {
                // Truncating values here...
                keyTime = (int) animCurve.time( currKey, &status ).value();
                addElement( keyFrames, keyTime );
            }   
        }   
    }   
    
    return 1;
}   


// =================================================================================
// ------------------------------  CLASS   FUNCTIONS  ------------------------------
// =================================================================================




// =================================================================================
// ------------------------------      CALLBACKS      ------------------------------
// =================================================================================




// =================================================================================
// ------------------------------  PRIVATE FUNCTIONS  ------------------------------
// =================================================================================


/*
 *  ========== mtlNew ==========
 *
 *  SYNOPSIS
 *	A private function. Used to reset all internal states.
 */

void  mtlNew(void)
{
    int	i = 0;

    // create the object instance structure
    //
    if( NULL == local ) 
	{
		local = (DtPrivate *)calloc( 1, sizeof( DtPrivate ) );
	}
	
	local->shape_ct = 0;

    // Free data within each material structure.
    //
    for( i = 0; i < local->mtl_ct; i++ )
    {
		if( NULL != local->mtl[i].name ) 
		{
			free( local->mtl[i].name );
		}
		if( NULL != local->mtl[i].texture_name ) 
		{
			free( local->mtl[i].texture_name );
		}
    }

    // Free the material structure array.
    //
    if( NULL != local->mtl )
    {
		free( local->mtl );
		local->mtl = NULL;
		local->mtl_ct = 0;
		local->mtl_sz = 0;
    }

    // Free the data within each texture structure.
    //
    for( i = 0; i < local->tex_ct; i++ )
    {
		free( local->tex[i].name );
    }

    // Free the texture structure array.
    //
    if( NULL != local->tex )
    {
		free(local->tex);
		local->tex = NULL;
		local->tex_ct = 0;
		local->tex_sz = 0;
    }

	// Now call the routine to setup the cache for the shader values.
	//
	mtlCreateCaches();
}  // mtlNew //



//  ========== mtlCreateCaches ==========
//
//  SYNOPSIS
//	Create a node cache of the root nodes of all the
//	shapes. Create a list of materials. Create a list
//	of textures.
//

void  mtlCreateCaches( void )
{
	int			 i = 0;
	int 		 j = 0;
	int			 group_ct = 0;
	MObject      shader;
	MObject      object;
	MDagPath	 dagPath;
	MtlStruct	 *mtl = NULL;

	local->shape_ct = DtShapeGetCount();
	
	int count = 0;
	DtTextureGetSceneCount( &count );

	for( i = 0; i < local->shape_ct; i++ )
	{
		group_ct = DtGroupGetCount( i );

		DtExt_ShapeGetDagPath( i, dagPath );
		
		for( j = 0; j < group_ct; j++ )
		{
			if( DtExt_ShapeGetShader( i, j, shader ) ) 
			{	
				MtAddMtl(local, shader);

				DtExt_ShapeGetOriginal( i, j, object );
				
				// Assume no texture on shaders at this point.	
				// Check for shader/transparency textures

				if( shader_texture2( shader, object, dagPath ) )
				{
					DtExt_ShapeIncTexCnt( i );
				}

				// Now if the user wants the other possible textures
				// to be sampled then this flag needs to be set.
				// Off by default, due to the extra processing time
				// that not all translators need this done.
				
				if ( DtExt_MultiTexture() )
				{
					if ( shader_multiTextures( shader, object, dagPath ) )
					{
						DtExt_ShapeIncTexCnt( i );
					}	
				}
				
				// See if this material is being used as a reflection map.
				//
				mtl = MtFindMtl( local, (char* )objectName( shader ) );
				mtl->isRflMap = false;
			}
		}
	}
}  // mtlCreateCaches //


void
DtExt_MaterialDelete()
{
    int i = 0;
    MtlStruct* mtl = NULL;
    TexStruct* tex = NULL;

    if( local == NULL ) 
	{
		return;
	}
    if( local->mtl != NULL )
    {
        for ( i = 0; i < local->mtl_ct; i++ )
        {
            mtl = &local->mtl[i];
            free( mtl->name );
            free( mtl->mtl );
            if( mtl->texture_name )
                free( mtl->texture_name );
			if( mtl->ambient_name )
				free( mtl->ambient_name );
            if( mtl->diffuse_name )
                free( mtl->diffuse_name );
            if( mtl->translucence_name )
                free( mtl->translucence_name );
            if( mtl->incandescence_name )
                free( mtl->incandescence_name );
            if( mtl->bump_name )
                free( mtl->bump_name );
			if( mtl->shininess_name )
				free( mtl->shininess_name );
            if( mtl->specular_name )
                free( mtl->specular_name );
            if( mtl->reflectivity_name )
                free( mtl->reflectivity_name );
            if( mtl->reflected_name )
                free( mtl->reflected_name );
        }
        free( local->mtl );
    }
    if( local->tex != NULL )
    {
        for ( i = 0; i < local->tex_ct; i++ )
        {
            tex = &local->tex[i];
            if( tex->image )
                free( tex->image );
            if( tex->name )
                free( tex->name );
			if( tex->placeName )
				free( tex->placeName );
			if( tex->nodeName )
				free( tex->nodeName );
			if( tex->filename )
				free( tex->filename );

            // if( AlIsValid( tex->texture ) )
			// if( tex->texture )
			//  delete tex->texture;
        }
        free( local->tex );
    }
    free( local );
    local = NULL;
}

//  ========== MtAddMtl ==========
//
//  SYNOPSIS
//	Add a material node to the material list.
//	First search the list to make sure the node is 
//	not already in the list.
//
static void  MtAddMtl( DtPrivate *local_2, MObject mtl_node )
{
    MtlStruct	*mtl = NULL;
	Material	*mtl_data = NULL;

    // Simply return if node is NULL.
    //
    if( mtl_node.isNull() ) 
	{
		return;
	}

    // Do nothing if material already in list.
    //
	if( NULL != MtFindMtl(local_2, (char* )objectName( mtl_node ) ) ) 
	{
		DtExt_Msg("MtAddMtl: \"%s\" is already in list\n", objectName( mtl_node ) );
		return;
	}

    // Increase array if needed.
    //
    if( local_2->mtl_ct == local_2->mtl_sz )
    {
		//long current_sz = local_2->mtl_sz * sizeof(MtlStruct);
		local_2->mtl_sz += MT_BLOCK_SIZE;
		local_2->mtl = (MtlStruct *)realloc(local_2->mtl, local_2->mtl_sz * sizeof(MtlStruct));
		memset( &local_2->mtl[local_2->mtl_ct], 0, MT_BLOCK_SIZE*sizeof(MtlStruct) );
	}

    // Add material to list.
    //
    mtl = &local_2->mtl[local_2->mtl_ct++];
    mtl->name = strdup( objectName( mtl_node ) );

	mtl_data = (Material *)calloc( 1, sizeof( Material ) ); 
    mtl->mtl = shader_material_multi( mtl_node, mtl_data, 0 );
    mtl->texture_name = NULL;
	mtl->shader = mtl_node;
	mtl->isRflMap = 0;
}  // MtAddMtl //


//  ========== MtFindMtl ==========
//
//  SYNOPSIS
//	Search for a material by name. This function searches
//	the material list for the material of the given name.
//	If found, a pointer to that material structure is returned.
//	Otherwise, NULL is returned.
//
static MtlStruct  *MtFindMtl(DtPrivate *local, char* name)
{
    MtlStruct	*mtl;
    int		    i;

	if( NULL == local ) 
	{
		return( NULL );
	}
    // Search the material array.
    //
    for( i = 0; i < local->mtl_ct; i++)
    {
		mtl = &local->mtl[i];
		if( 0 == strcmp(mtl->name, name) ) 
		{
			return( mtl );
		}
    }

    // Material not found.
    //
    return( NULL );

}  // MtFindMtl //


//  ========== MtAddTex ==========
//
//  SYNOPSIS
//	Add a texture (and texture transform) to the texture list.
//	First search the list to make sure the texture is not 
//	already in the list.
//
static int MtAddTex(DtPrivate* local,char *texture, char *filename,
					int filetexture,unsigned char* image, 
					int xres, int yres, int zres )
{
    // simply return if nodes are NULL
    //
    if( texture == NULL ) return 0;

    // do nothing if texture is already in the list
    //
    if(MtFindTex(local, texture) != NULL) 
	{
		DtExt_Msg("MtAddTex: \"%s\" is already in list\n", texture );
		return 0;
	}

    // increase array if needed
    //
    if(local->tex_ct == local->tex_sz)
    {
		local->tex_sz += MT_BLOCK_SIZE;
		local->tex = (TexStruct *)realloc(local->tex, local->tex_sz * sizeof(TexStruct));
    }

    // add texture to list
    //
    TexStruct  *tex = &local->tex[local->tex_ct++];

	if ( texture ) 
	{
    	tex->name    = strdup( texture );
		tex->nodeName = strdup( texture );
	}

	tex->texture = NULL;

	if ( filename )
		tex->filename = strdup( filename );

	// Lets now add reference to the Placement Node
	// We will check both 2d and 3d for now.

	tex->placeName = NULL;

	MString command = "listConnections -type place2dTexture " + 
								MString( texture );
	MStatus status;
	MStringArray result;
	
	status = MGlobal::executeCommand( command, result );

    if ( status == MS::kSuccess ) 
	{
    	if ( result.length() > 0 ) 
		{
        	tex->placeName = strdup( result[0].asChar() );
		}
	}

#if 0
	// 3D placement nodes don't have the attributes we're expecting
	// so we're going to leave them alone until we figure out 
	// what to do with them - JP - Oct99
	if ( !tex->placeName )
	{
		command = "listConnections -type place3dTexture " + MString( texture );

	    status = MGlobal::executeCommand( command, result );
    
    	if ( status == MS::kSuccess ) 
    	{
    	    if ( result.length() > 0 ) 
    	    {
    	        tex->placeName = strdup( result[0].asChar() );
    	    }   
    	}   
	}
#endif

	tex->trans   = NULL;			// This will need to be added

	tex->filetexture = filetexture;
	tex->image	 = image;
	tex->xres    = xres;
	tex->yres    = yres;
	tex->zres    = zres;

	return 1;

}  // MtAddTex //

//  ========== MtFindTex ==========
//
//  SYNOPSIS
//	Search for a material by name. This function searches
//	the material list for the material of the given name.
//	If found, a pointer to that material structure is returned.
//	Otherwise, NULL is returned.
//

static TexStruct  *MtFindTex(DtPrivate *local, char* name)
{
    TexStruct	*tex = NULL;
    int		i = 0;

    // search the texture array
    //
    for (i = 0; i < local->tex_ct; i++)
    {
		tex = &local->tex[i];
		if(strcmp(tex->name, name) == 0) return(tex);
    }

    // texture not found
    //
    return(NULL);

}  // MtFindTex //

// static double uoffset, voffset, rotation, uscale, vscale;
// static double tuoffset, tvoffset, trotation, tuscale, tvscale;

static MDoubleArray Save_rotateFrame, Save_translateFrame, Save_repeatUV;
static MDoubleArray Save_rotateUV, Save_offset;

void
saveTransforms( MString textureName )
{

    if( textureName.length() == 0 )
		return; 

	int		placeFound = false;

	MString placeName;
    
   	MString command = "listConnections -type place2dTexture " + textureName;
   	MStatus status;
   	MStringArray result;
    
    status = MGlobal::executeCommand( command, result );
    
    if ( status == MS::kSuccess )
    {   
        if ( result.length() > 0 )
        {   
            placeName = result[0];
			placeFound = true;
        }
    }
    
#if 0
	// 3D placement nodes don't have the attributes we're expecting
	// so we're going to leave them alone until we figure out 
	// what to do with them - JP - Oct99
    if ( !placeFound )
    {   
        command = "listConnections -type place3dTexture " + textureName;
        
        status = MGlobal::executeCommand( command, result );
        
        if ( status == MS::kSuccess )
        {   
            if ( result.length() > 0 )
            {   
                placeName = result[0];
				placeFound = true;
            }
        }
    }
#endif

	if ( placeFound )
	{
		MIntArray	wrapU, wrapV;
		
    	command = MString("getAttr ") + placeName + MString(".translateFrame");
    	status = MGlobal::executeCommand( command, Save_translateFrame );
        command = MString("getAttr ") + placeName + MString(".rotateFrame");
        status = MGlobal::executeCommand( command, Save_rotateFrame );

        command = MString("getAttr ") + placeName + MString(".offset");
        status = MGlobal::executeCommand( command, Save_offset );
        command = MString("getAttr ") + placeName + MString(".rotateUV");
        status = MGlobal::executeCommand( command, Save_rotateUV );

        command = MString("getAttr ") + placeName + MString(".repeatUV");
        status = MGlobal::executeCommand( command, Save_repeatUV );
        command = MString("getAttr ") + placeName + MString(".wrapU");
		status = MGlobal::executeCommand( command, wrapU );
	    command = MString("getAttr ") + placeName + MString(".wrapV");
		status = MGlobal::executeCommand( command, wrapV );


		if( !DtExt_softTextures() )
		{

			if ( getenv("MDT_USE_TEXTURE_TRANSFORMS" ) )
			{
	        	command = MString("setAttr ") + placeName 
												+ MString(".rotateFrame 0");
    	    	status = MGlobal::executeCommand( command );
            	command = MString("setAttr ") + placeName + 
												MString(".translateFrame 0 0");
				status = MGlobal::executeCommand( command );
			} else {
                command = MString("setAttr ") + placeName
                                                + MString(".rotateUV 0");
                status = MGlobal::executeCommand( command );
                command = MString("setAttr ") + placeName + 
                                                MString(".offset 0 0");
                status = MGlobal::executeCommand( command );
			}

			if ( wrapU[0] )
			{
            	command = MString("setAttr ") + placeName + 
												MString(".repeatU 1");
            	status = MGlobal::executeCommand( command );
			}
            if ( wrapV[0] )
            {
                command = MString("setAttr ") + placeName +
                                                MString(".repeatV 1");
                status = MGlobal::executeCommand( command );
            }

		}
    }
}

void
restoreTransforms( MString textureName )
{
    if( textureName.length() == 0 )
        return;
    
	int		placeFound = false;

    MString placeName;
    
    MString command = "listConnections -type place2dTexture " + textureName;
    MStatus status;
    MStringArray result;
    
    status = MGlobal::executeCommand( command, result );
    
    if ( status == MS::kSuccess )
    {   
        if ( result.length() > 0 )
        {   
            placeName = result[0];
			placeFound = true;
        }
    }
    
#if 0
	// 3D placement nodes don't have the attributes we're expecting
	// so we're going to leave them alone until we figure out 
	// what to do with them - JP - Oct99
    if ( !placeFound )
    {   
        command = "listConnections -type place3dTexture " + textureName;
        
        status = MGlobal::executeCommand( command, result );
        
        if ( status == MS::kSuccess )
        {   
            if ( result.length() > 0 )
            {   
                placeName = result[0];
				placeFound = true;
            }
        }
    }
#endif
    
    if ( placeFound && !DtExt_softTextures() )
    {   
		char cmdStr[1024];

		sprintf( cmdStr, "setAttr %s.rotateFrame %f", 
								placeName.asChar(), Save_rotateFrame[0] );
		status = MGlobal::executeCommand( cmdStr );

        sprintf( cmdStr, "setAttr %s.translateFrame %f %f", 
                     	placeName.asChar(), Save_translateFrame[0], 
					 						Save_translateFrame[1] );
        status = MGlobal::executeCommand( cmdStr );

        sprintf( cmdStr, "setAttr %s.rotateUV %f",
                                placeName.asChar(), Save_rotateUV[0] );
        status = MGlobal::executeCommand( cmdStr );

        sprintf( cmdStr, "setAttr %s.offset %f %f",
                        placeName.asChar(), Save_offset[0],
                                            Save_offset[1] );
        status = MGlobal::executeCommand( cmdStr );

        sprintf( cmdStr, "setAttr %s.repeatUV %f %f", 
                     placeName.asChar(), Save_repeatUV[0], Save_repeatUV[1] );
        status = MGlobal::executeCommand( cmdStr );
    }

}

Material *
shader_material_multi( MObject alShader, Material *material, int index )
{
    if( alShader.isNull() )
	{
        return NULL;
	}

    float      	factor = 1.0;
	float       specintense = 0.0;
	float       rolloff = 0.0;
    float	    shiny=0.0;
	float 		transparency[3];
	float 		diffuse[3];
	float 		emmission[3];
	float 		color[3];
	float 		specular[3];
	// float 	diffuseAlpha = 0.0;
	// float 	emmissionAlpha = 0.0;

	if( // alShader.hasFn( MFn::kPhong )  ||
		// alShader.hasFn( MFn::kLambert )  ||
		// alShader.hasFn( MFn::kBlinn)  )
		( 0 == strcmp( "phong", objectType( alShader ) ) ) ||
		( 0 == strcmp( "phongE", objectType( alShader ) ) ) ||
		( 0 == strcmp( "blinn", objectType( alShader ) ) ) ||
		( 0 == strcmp( "lambert", objectType( alShader ) ) ) ||
		( 0 == strcmp( "layeredShader", objectType( alShader ) ) )  ||
		( 0 == strcmp( "anisotropic", objectType( alShader ) ) ) )
	{
		if( DtExt_Debug() )
		{	
			cerr << objectName( alShader ) << " is indeed a shader of type "
					<< objectType( alShader ) << endl;
		}
	}
	else
	{
		if( DtExt_Debug() )
		{
			cerr << objectName( alShader ) <<  " is not a recognized shader " 
					<< objectType( alShader) << endl;
		}

		// But for now lets return a default material setting
		
        material->color.r = 0.5;
        material->color.g = 0.5;
        material->color.b = 0.5;

        material->diffuse.r = 0.5;
		material->diffuse.g = 0.5;
	    material->diffuse.b = 0.5;

		material->transparency = 0.0;

		return material;

	}


	MStatus stat = MS::kSuccess;

    if( 0 == strcmp( "lambert", objectType( alShader )) ||
		0 == strcmp( "anisotropic", objectType( alShader )) ) 	
	{
		MFnLambertShader fnShader;

    	stat = fnShader.setObject( alShader );
    	MColor mcolor;
    	mcolor = fnShader.color( &stat );
    	mcolor.get( color );
    	
		// Diffuse in Maya becomes diffuse coefficient... confirm...
    	//
    	factor = fnShader.diffuseCoeff( &stat );
    	
    	// Calculate diffuse value
    	//
    	for( int i = 0; i < 3; i++ )
    	{
    	    diffuse[i] = factor * color[i];
    	}

    	MColor mincan;
    	mincan = fnShader.incandescence( &stat );
    	mincan.get( emmission );
    	
    	MColor mtrans;
    	mtrans = fnShader.transparency( &stat );
    	mtrans.get( transparency );
    	
	    // emmissionAlpha = diffuseAlpha = 255.0 *
	    // (1.0 - ((transparency[0] + transparency[1] + transparency[2])/3.0) );

	    material->color.r = color[0];
	    material->color.g = color[1];
	    material->color.b = color[2];
       
	    material->diffuse.r = diffuse[0];
	    material->diffuse.g = diffuse[1];
	    material->diffuse.b = diffuse[2];

        material->emissive.r = emmission[0];
        material->emissive.g = emmission[1];
        material->emissive.b = emmission[2];

		material->shininess = 0;

		if ( 0 == strcmp( "lambert", objectType( alShader )) )
		{
        	material->specular.r =  0;
        	material->specular.g =  0;
        	material->specular.b =  0;

		} else {

			MFnReflectShader fnAniso;
	        stat = fnAniso.setObject( alShader );

	        MColor mspecular;
	        mspecular = fnAniso.specularColor( &stat );
	        mspecular.get( specular );

       		// reflectivity
        	//
        	specintense = fnAniso.reflectivity( &stat );

        	material->specular.r = specular[0] * specintense;
        	material->specular.g = specular[1] * specintense;
        	material->specular.b = specular[2] * specintense;

			// use the "roughness" attribute for shininess

    		MFnDependencyNode curSNode( alShader, &stat );
    		MPlug vPlug = curSNode.findPlug( "roughness", &stat );
			double roughness;

		    if ( stat == MS::kSuccess )
    		{
        		vPlug.getValue( roughness );
				material->shininess = roughness;
			}
		}

        material->lightsource = false;
    }
    else if( (0 == strcmp( "phong",  objectType( alShader ) ) )  || 	
	         (0 == strcmp( "phongE", objectType( alShader ) ) ) ) 

	{
		MFnReflectShader fnPhong;
		stat = fnPhong.setObject( alShader );

    	MColor mcolor;
    	mcolor = fnPhong.color( &stat );
    	mcolor.get( color );
    
    	// Diffuse in Maya becomes diffuse coefficient... confirm...
    	//
    	factor = fnPhong.diffuseCoeff( &stat );
    	
		// Calculate diffuse value
    	//
    	for( int i = 0; i < 3; i++ )
    	{
    	    diffuse[i] = factor * color[i];
    	}

    	MColor mincan;
    	mincan = fnPhong.incandescence( &stat );
    	mincan.get( emmission );
    	
		MColor mtrans;
    	mtrans = fnPhong.transparency( &stat );
    	mtrans.get( transparency );
    
    	// emmissionAlpha = diffuseAlpha = 255.0 *
    	// (1.0 - ((transparency[0] + transparency[1] + transparency[2])/3.0) );

    	material->color.r = color[0];
    	material->color.g = color[1];
    	material->color.b = color[2];
	
	    material->diffuse.r = diffuse[0];
	    material->diffuse.g = diffuse[1];
    	material->diffuse.b = diffuse[2];

		MColor mspecular;
		mspecular = fnPhong.specularColor( &stat );
		mspecular.get( specular );

		// reflectivity
		//
		specintense = fnPhong.reflectivity( &stat );

        material->emissive.r = emmission[0];
        material->emissive.g = emmission[1];
        material->emissive.b = emmission[2];

        material->specular.r = specular[0] * specintense;
        material->specular.g = specular[1] * specintense;
        material->specular.b = specular[2] * specintense;

	    // Fill in the shininess depending on type

	    if( (0 == strcmp( "phongE",  objectType( alShader ) ) )  )
	    {
            // use the "roughness" attribute for shininess
           
            MFnDependencyNode curSNode( alShader, &stat );
            MPlug vPlug = curSNode.findPlug( "roughness", &stat );
            double roughness;
            
            if ( stat == MS::kSuccess )
            {
                vPlug.getValue( roughness );
                material->shininess = roughness;
            }   
		}
		else
		{
	        MFnPhongShader rfnPhong;
    	    stat = rfnPhong.setObject( alShader );

    	    // Shinyness becomes cosinePower in Maya:
    	    //
    	    factor = rfnPhong.cosPower( &stat );

    	    //shiny = sqrt(sqrt((float)(factor)/210.0)) * 300.0 - 90.0;
    	    material->shininess = factor;

		}

        material->lightsource = false;
    }
    else if( 0 == strcmp( "blinn", objectType( alShader )  ) ) 
	{	
		MFnBlinnShader fnBlinn;
		stat = fnBlinn.setObject( alShader );

    	MColor mcolor;
    	mcolor = fnBlinn.color( &stat );
    	mcolor.get( color );
    	
		// Diffuse in Maya becomes diffuse coefficient... confirm...
    	//
    	factor = fnBlinn.diffuseCoeff( &stat );
    	
		// Calculate diffuse value
    	//
    	for( int i = 0; i < 3; i++ )
    	{
    	    diffuse[i] = factor * color[i];
    	}

    	MColor mincan;
    	mincan = fnBlinn.incandescence( &stat );
    	mincan.get( emmission );
    	
		MColor mtrans;
    	mtrans = fnBlinn.transparency( &stat );
    	mtrans.get( transparency );
    	
		// emmissionAlpha = diffuseAlpha = 255.0 *
    	// (1.0 - ((transparency[0] + transparency[1] + transparency[2])/3.0) );

    	material->color.r = color[0];
    	material->color.g = color[1];
    	material->color.b = color[2];

    	material->diffuse.r = diffuse[0];
    	material->diffuse.g = diffuse[1];
    	material->diffuse.b = diffuse[2];

		MColor mspecular;
		mspecular = fnBlinn.specularColor( &stat );
		mspecular.get( specular );
		
		// reflectivity
		//
		specintense = fnBlinn.reflectivity( &stat );
	
		rolloff = fnBlinn.specularRollOff( &stat );
		
        specintense *= (1.0 - (rolloff * 0.8) );

		factor = fnBlinn.eccentricity( &stat );
		
        //shiny = 1.15/(factor + 0.1);
        //shiny = (shiny * shiny * shiny) - 0.5;
		
		shiny = factor;
		
        material->emissive.r = emmission[0];
        material->emissive.g = emmission[1];
        material->emissive.b = emmission[2];

        material->specular.r = specular[0] * specintense;
        material->specular.g = specular[1] * specintense;
        material->specular.b = specular[2] * specintense;

        material->shininess = shiny;

        material->lightsource = false;

    } 
	else if( 0 == strcmp( "layeredShader", objectType( alShader )  ) )
    {   
        material->color.r = 0.5;
        material->color.g = 0.5;
        material->color.b = 0.5;
        
        material->diffuse.r = 0.5;
        material->diffuse.g = 0.5;
        material->diffuse.b = 0.5;
        
        transparency[0] = 0.0;
        transparency[1] = 0.0;
        transparency[2] = 0.0;
    
    }
	else 
	{
		DtExt_Err("shading model not recognized: \"%s\"\n", 
				 objectType( alShader ) );
	}


	// Fill in the missing info for phong shader

	if( (0 == strcmp( "phong",  objectType( alShader ) ) )  )
	{
		MFnPhongShader fnPhong;
		stat = fnPhong.setObject( alShader );

        // Shinyness becomes cosinePower in Maya:
        //
        factor = fnPhong.cosPower( &stat );

        //shiny = sqrt(sqrt((float)(factor)/210.0)) * 300.0 - 90.0;
		shiny = factor;

        material->shininess = shiny;
	}
	
    material->transparency = (transparency[0] +
                              transparency[1] +
                              transparency[2]) / 3.0;
    return material;
}


//	Routine to walk thru the possible texture mapped fields and generate
//	texture images for those that are mapped

int
shader_multiTextures(MObject alShader, MObject object, MDagPath &dagPath )
{
    MString   shaderName;
    MString   textureName;
    MString   textureFile = "";
    MString   projectionName = "";
    MString   convertName;
    MString   command;
    MStringArray result;
    MStringArray arrayResult;

    MStatus   status;

    MStringArray    Tex_mapped;

	int	texCnt = 0;


    MFnDependencyNode dgNode;
    status = dgNode.setObject( alShader );
    shaderName = dgNode.name( &status );

    result.clear();

    // Lets return without truing if we get a null shader name

    if ( shaderName.length() == 0 )
    {
        return 0;
    }


	// If this is a layered shader, we are going to skip this for now.
	// will need to run thru the layered shaders each by themselves ?
	// Leave for later.

	if ( 0 == strcmp( "layeredShader", objectType( alShader ) ) )
    {
		return 0;
    }


	// We are not going to do the color and transparency thing here

#if 0
    command = MString("listConnections ") + shaderName + MString(".color;");
    status = MGlobal::executeCommand( command, result );

    command = MString("listConnections ") + shaderName
                                          + MString(".transparency;");

    statusT = MGlobal::executeCommand( command, resultT );
#endif

	// We will be doing the other textures here

    command = MString("listConnections ") + shaderName
                                          + MString(".ambientColor;");
	if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
    {
    	MString texType( "ambient" );
		MString texNode( ".ambientColor" );
        texCnt += generate_textureImages( shaderName, texNode, texType, 
													Tex_mapped, dagPath);
    }

    command = MString("listConnections ") + shaderName
                                          + MString(".diffuse;");
    if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
    {
        MString texType( "diffuse" );
		MString texNode( ".diffuse" );
        texCnt += generate_textureImages( shaderName, texNode, texType, 
													Tex_mapped, dagPath);
    }

    command = MString("listConnections ") + shaderName
                                          + MString(".translucence;");
    if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
    {
        MString texType( "translucence" );
		MString texNode( ".translucence");
        texCnt += generate_textureImages( shaderName, texNode, texType, Tex_mapped, dagPath);
    }

    command = MString("listConnections ") + shaderName
                                          + MString(".incandescence;");
    if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
	{
        MString texType( "incandescence" );
		MString texNode( ".incandescence");
        texCnt += generate_textureImages( shaderName, texNode, texType, 
													Tex_mapped, dagPath);
    }


    command = MString("listConnections ") + shaderName
                                          + MString(".normalCamera;");
    if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
	{
        MString texType( "bump" );
		MString texNode( ".normalCamera");
        texCnt += generate_textureImages( shaderName, texNode, texType, 
													Tex_mapped, dagPath);
    } 

    command = MString("listConnections ") + shaderName
                                          + MString(".cosinePower;");
    if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
    {
        MString texType( "shininess" );        
		MString texNode( ".cosinePower");
        texCnt += generate_textureImages( shaderName, texNode, texType, 
													Tex_mapped, dagPath);
    }   

    command = MString("listConnections ") + shaderName
                                          + MString(".specularColor;");
    if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
    {
        MString texType( "specular" );
		MString texNode( ".specularColor");
        texCnt += generate_textureImages( shaderName, texNode, texType, 
													Tex_mapped, dagPath);
    }   

    command = MString("listConnections ") + shaderName
                                          + MString(".reflectivity;");
    if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
    {
        MString texType( "reflectivity" );
		MString texNode( ".reflectivity");
        texCnt += generate_textureImages( shaderName, texNode, texType, 
													Tex_mapped, dagPath);
    }

    command = MString("listConnections ") + shaderName
                                          + MString(".reflectedColor;");
    if ( MS::kSuccess == MGlobal::executeCommand( command, Tex_mapped ) )
    {
        MString texType( "reflected" );
		MString texNode( ".reflectedColor");
        texCnt += generate_textureImages( shaderName, texNode, texType, 
													Tex_mapped, dagPath);
    }   


	return texCnt; 

	// Other textures that could be added include:
	//
	// Common:
	//
	// diffuse
	// translucence
	//
	// phong: 
	//
	// reflectivity
	//
	// phongE:
	//
	// roughness
	// highlightSize
	// whiteness
	// 
	// blinn:
	//
	// eccentricity
	// specularRollOff
	//

}

int
getTextureFileSize( bool fileTexture, MString &textureName, 
									  MString &textureFile, 
									  int &xres, int &yres, int &zres )
{
    // If we are doing file texture lets see how large the file is
    // else lets use our default sizes.

    xres = DtExt_xTextureRes();
    yres = DtExt_yTextureRes();
    zres = 4;

    if ( fileTexture )
    {
#if 1
		// Lets try getting the size of the textures this way.

		MStatus		status;
		MString		command;
		MIntArray	sizes;

		command = MString( "getAttr " ) + textureName + MString( ".outSize;" );
		status = MGlobal::executeCommand( command, sizes );
		if ( MS::kSuccess == status )
		{
			xres = sizes[0];
			yres = sizes[1];
		}

#else
	
		// Old method 
        IFFimageReader reader;
        MStatus Rstat;

        Rstat = reader.open( textureFile );
        if ( Rstat != MS::kSuccess )
        {
            cerr << "Error reading file " << textureFile.asChar() << " "
                 << reader.errorString() << endl;
            return 0;
        }

        int bpp = reader.getBytesPerChannel();
        int w,h;

        Rstat = reader.getSize (w, h);
        if ( Rstat != MS::kSuccess )
        {
            reader.close();
            return 0;
        }

        xres = w;
        yres = h;
       	zres = 1;

        if (reader.isRGB () || reader.isGrayscale ())
        {
            if (reader.isRGB ())
            {
                zres = 3;
                if (reader.hasAlpha ())
                    zres = 4;
            }
        }

        reader.close();
#endif

	}

	return 1;
}



int
generate_textureImages(	MString &shaderName, 
						MString &texNode,
						MString &texType, 
						MStringArray& texMap,
						MDagPath &dagPath )
{
	MString			command;
	MString			textureName;
	MString			convertName;
	MString			textureFile;

	MString			bumpTexture;

	MStatus			status;

	MStringArray	result;
    MStringArray 	arrayResult;

	
	MtlStruct		*mtl;
	
	int				xres, yres, zres;

	bool			fileTexture;

	// Now check to see if something was there

    if ( (texMap.length() && texMap[0].length()) ) 
	{
        command = MString("ls -st ") + texMap[0];
        status = MGlobal::executeCommand( command, arrayResult );

        if ( status != MS::kSuccess ) {
            return 0;
        }

		// Now see if it is a file texture in order to get the size of
		// the image to use

		fileTexture = false;

        if ( (arrayResult.length() ==2) && arrayResult[1].length() &&
            (arrayResult[1] == MString("file") )  ) {

            textureName = arrayResult[0];
            command = MString("getAttr ") + textureName +
                                        MString(".fileTextureName");

            status = MGlobal::executeCommand( command, result );

            if ( status != MS::kSuccess ) {
                return 0;
            }

            textureFile = result[0];
            fileTexture = true;

            if( DtExt_Debug() )
                cerr << "file: texture is: " << textureFile << endl;

		} else if ( (arrayResult.length() ==2) && arrayResult[1].length() &&
		            (arrayResult[1] == MString("bump2d") )  ) {

			command = MString("listConnections ") + texMap[0]
                                       + MString(".bumpValue;");
			status = MGlobal::executeCommand( command, result );

		    if ( (result.length() && result[0].length()) )
    		{   

				bumpTexture = result[0];

        		command = MString("ls -st ") + result[0];
        		status = MGlobal::executeCommand( command, arrayResult );
        
        		if ( status != MS::kSuccess ) {
        		    return 0;
        		}
        
        		// Now see if it is a file texture in order to get the size of
        		// the image to use
        
        		fileTexture = false;
        
        		if ( (arrayResult.length() ==2) && arrayResult[1].length() &&
            		(arrayResult[1] == MString("file") )  ) {
            
            		textureName = arrayResult[0]; 
            		command = MString("getAttr ") + textureName +
                	                        MString(".fileTextureName");
            
            		status = MGlobal::executeCommand( command, result );
            
            		if ( status != MS::kSuccess ) {
           		 	    return 0;
       				}

           		 	textureFile = result[0];
            		fileTexture = true;
            
            		if( DtExt_Debug() )
            		    cerr << "file: texture is: " << textureFile << endl;
        		}
			}
		}
				

		// There is a texture, I don't care what kind it is, going to use
		// convertSolidTx to generate a file texture

        textureName = texMap[0];

		// Do nothing if texture is already in the list.
		//
		if( MtFindTex(local, (char *) textureName.asChar() ) != NULL)
		{
			// add texture name to material
			//
			mtl = MtFindMtl(local, (char *) shaderName.asChar() );

			if ( texType == "ambient" )
			{
				if ( mtl->ambient_name )
					free( mtl->ambient_name );
				mtl->ambient_name = strdup( textureName.asChar() );
			}
            else if ( texType == "diffuse" )
            {
                if ( mtl->diffuse_name )
                    free( mtl->diffuse_name );
                mtl->diffuse_name = strdup( textureName.asChar() );
            }

            else if ( texType == "translucence" )
            {
                if ( mtl->translucence_name )
                    free( mtl->translucence_name );
                mtl->translucence_name = strdup( textureName.asChar() );
            }

			else if ( texType == "incandescence" ) 
			{
				if ( mtl->incandescence_name )
					free( mtl->incandescence_name );
				mtl->incandescence_name = strdup( textureName.asChar() );
			} 
			else if ( texType == "bump" )
			{
				if ( mtl->bump_name )
					free( mtl->bump_name );
				mtl->bump_name = strdup( textureName.asChar() );
			} 
            else if ( texType == "shininess" )
            {
                if ( mtl->shininess_name )
                    free( mtl->shininess_name );
                mtl->shininess_name = strdup( textureName.asChar() );
            }
			else if ( texType == "specular" )
			{
				if ( mtl->specular_name )
					free( mtl->specular_name );
				mtl->specular_name = strdup( textureName.asChar() );
			} 

            else if ( texType == "reflectivity" )
            {
                if ( mtl->reflectivity_name )
                    free( mtl->reflectivity_name );
                mtl->reflectivity_name = strdup( textureName.asChar() );
            }

			else if ( texType == "reflected" )
			{
				if ( mtl->reflected_name )
					free( mtl->reflected_name );
				mtl->reflected_name = strdup( textureName.asChar() );
			}

			return( 1 );
		}
		
		// Will use the convertSolidTx command to generate the image to use.
		//
	
		unsigned char *image = NULL;
           
		char	cmdStr[2048];

		saveTransforms( textureName );

		// need to have different temp location for SGI and NT

		char tmpFile[2048];
		sprintf( tmpFile, "%s/mdt%d.tmp", getenv("TMPDIR"), getpid() );

#ifdef WIN32
		// Check for '\' characters and replace with '/'
		// Makes passing strings to convertSolidTx easier
		
		for ( unsigned int i=0; i < strlen(tmpFile); i++ )
		{
			if ( tmpFile[i] == '\\' ) 
				tmpFile[i] = '/';
		}
#endif

		// 
		// Check to see if a projection was found and if so then use it
		// as the base object to do the convert solid texture command on
		//

		// Need to check for the bump mapping.
		// as there is a utitity node in front of the actual texture
		// that is to be converted.


		MString forSizeName;

		if ( texType == "bump" )
		{
			// convertName = textureName + MString(".bumpValue");
			convertName = bumpTexture + MString( ".outColor" );
			forSizeName = bumpTexture;
		} else {
#if 0 
			convertName = shaderName + texNode;
			forSizeName = shaderName + texNode;
#else
			convertName = textureName + MString( ".outColor" );
			forSizeName = textureName;
#endif
		}


        // Use the supplied x/y size to generate the texture files

        getTextureFileSize( fileTexture, forSizeName, textureFile, 
													xres, yres, zres );


		// Here we are only going to do all of this work if we really want/need
		// to generate new images.  Allow the user to specify this with the
		// DtExt_setOriginalTexture( mode ) function.

		if ( DtExt_OriginalTexture() )
		{
			// We will use a temp file
			restoreTransforms( textureName );

			// allocate a little memory to make other things happy

			image = (unsigned char *)strdup("originalFile");

		} else {
			sprintf( cmdStr,
                "convertSolidTx -rx %d -ry %d -n \"%s\" -fin \"%s\" -uvr 0.0 1.0 0.0 1.0 %s %s",
                    	xres, yres, 
						"mdt_tmp_texture_name",
						tmpFile,
						convertName.asChar(), 
						dagPath.fullPathName().asChar() );
			if( DtExt_Debug() )
				cerr << "command to execute: " << cmdStr << endl;

			status = MGlobal::executeCommand( cmdStr );

			restoreTransforms( textureName );

			if ( status != MS::kSuccess )
			{
				cerr << "error from convertSolidTx " << 
								 status.errorString() << endl;
			    return 0;
			}   

			// Read in the image file generated 

			image = readTextureFile( MString( tmpFile ), MString( " " ),
									0,  xres, yres, zres );

			// Need to delete the node that was generated by the convertSolidTx
			// command.

			MGlobal::executeCommand( "delete mdt_tmp_texture_name" );
			
			// Now lets remove the temp file that we generated

			unlink ( tmpFile );

		}

		// If we have an image in memory, then we add it to our local table
		// references.


		// Need to find file name for texture if possible.

		if ( image )
		{	
   			if( !MtAddTex(local, (char *)textureName.asChar(), 
				 	(char *)textureFile.asChar(), fileTexture, 
				 		image, xres, yres, 4 ) )
			{
				DtExt_Msg("\"%s\" already in list delete this copy of image\n",
						(char *)textureName.asChar() );
				free( image );
			}

			// add texture name to material
			//
			mtl = MtFindMtl(local, (char* )shaderName.asChar() );
		
			// color and opacity are not done here as in the 
			// normal texture generation
	
			if ( texType == "ambient" )
			{
				if ( mtl->ambient_name )
					free( mtl->ambient_name );
				mtl->ambient_name = strdup( textureName.asChar() );
			}
            else if ( texType == "diffuse" )
            {   
                if ( mtl->diffuse_name )
                    free( mtl->diffuse_name );
                mtl->diffuse_name = strdup( textureName.asChar() );
            } 
            else if ( texType == "translucence" )
            {   
                if ( mtl->translucence_name )
                    free( mtl->translucence_name );
                mtl->translucence_name = strdup( textureName.asChar() );
            } 

			else if ( texType == "incandescence" ) 
			{
				if ( mtl->incandescence_name )
					free( mtl->incandescence_name );
				mtl->incandescence_name = strdup( textureName.asChar() );
			} 
			else if ( texType == "bump" )
			{
				if ( mtl->bump_name )
					free( mtl->bump_name );
				mtl->bump_name = strdup( textureName.asChar() );
			} 
            else if ( texType == "shininess" )
            {
                if ( mtl->shininess_name )
                    free( mtl->shininess_name );
                mtl->shininess_name = strdup( textureName.asChar() );
            }   
			else if ( texType == "specular" )
			{
				if ( mtl->specular_name )
					free( mtl->specular_name );
				mtl->specular_name = strdup( textureName.asChar() );
			} 
            else if ( texType == "reflectivity" )
            {   
                if ( mtl->reflectivity_name )
                    free( mtl->reflectivity_name );
                mtl->reflectivity_name = strdup( textureName.asChar() );
            } 
			else if ( texType == "reflected" )
			{
				if ( mtl->reflected_name )
					free( mtl->reflected_name );
				mtl->reflected_name = strdup( textureName.asChar() );
			}

			
			return 1;
		}
	}

	return 0;
}

int
shader_texture2( MObject alShader, MObject object, MDagPath &dagPath )
{

    MtlStruct	  *mtl;

	MString   shaderName;
	MString   textureName;
	MString   textureFile = "";
	MString	  projectionName = "";
	MString	  convertName;
	MString   command;
	MStringArray result;
	MStringArray resultT;
	MStringArray arrayResult;

	MStatus   status;
	MStatus   statusT;
	
	bool		projectionFound = false;
	bool		layeredFound = false;

    int xres, yres, zres;
    int xres_use, yres_use;
	
    bool fileTexture = false;

	// The following variables are not used yet.

    //int colorFound = false;
    int transFound = false;
    //unsigned char *image = NULL;
    //ShaderColor color={0.,0.,0.,0.};
    //ShaderColor transparency={0.,0.,0.,0.};


    MFnDependencyNode dgNode;
    status = dgNode.setObject( alShader );
	shaderName = dgNode.name( &status );

	if ( DtExt_Debug() )
	{
		cerr << "checking shader: " << shaderName.asChar() << endl;
		cerr << "2nd Check: name = " << objectName( alShader ) 
				<< "type = " << objectType(alShader ) << endl;
	}

    result.clear();
	resultT.clear();

	// Lets return without truing if we get a null shader name

	if ( shaderName.length() == 0 )
	{
		return 0;
	}
	

    if ( 0 == strcmp( "layeredShader", objectType( alShader ) ) )
    {
        layeredFound = true;
    }

    if ( !layeredFound )
    {
		command = MString("listConnections ") + shaderName + MString(".color;");

		status = MGlobal::executeCommand( command, result ); 

		command = MString("listConnections ") + shaderName 
												+ MString(".transparency;");

		statusT = MGlobal::executeCommand( command, resultT );
	
		if ( status != MS::kSuccess ) {
			return 0;
		}
	}


    if ( layeredFound || (result.length() && result[0].length()) ) {

        if ( layeredFound )
        {
            textureName = shaderName;
            fileTexture = false;
            arrayResult.clear();
            arrayResult[0] = shaderName;
        }   
        else
        {
			command = MString("ls -st ") + result[0];

			status = MGlobal::executeCommand( command, arrayResult );

			if ( status != MS::kSuccess ) {
				return 0;
			}
		
			if( DtExt_Debug() )
			{
				cerr << "ls -st result length " << arrayResult.length() << endl;
				cerr << "ls -st results " << arrayResult[0] << arrayResult[1] 
																	<< endl;
			}
		}

		if ( (arrayResult.length() ==2) && arrayResult[1].length() &&
			(arrayResult[1] == MString("file") )  ) {

			textureName = arrayResult[0];
			command = MString("getAttr ") + textureName +
					  					MString(".fileTextureName");

			status = MGlobal::executeCommand( command, result );

			if ( status != MS::kSuccess ) {
				return 0;
			}

			textureFile = result[0];
			fileTexture = true;
			
        	if( DtExt_Debug() )
				cerr << "file: texture is: " << textureFile << endl;

		} else if ( (arrayResult.length() ==2) && arrayResult[1].length() &&
		            (arrayResult[1] == MString("projection") )  ) {

            projectionName = arrayResult[0];
			
			projectionFound = true;
			
	        if( DtExt_Debug() )
				cerr << "found projection " << projectionName << endl;

		    command = MString("listConnections ") + projectionName + 
														MString(".image;");
    
		    status = MGlobal::executeCommand( command, result );
		    
    		if ( status != MS::kSuccess ) {
        
    		    return 0;
    		}

    		if ( result.length() && result[0].length() ) {
    
        		command = MString("ls -st ") + result[0];
        
        
        		status = MGlobal::executeCommand( command, arrayResult );
        
        
        		if ( status != MS::kSuccess ) {
        		    return 0;
        		}   
       
			    if( DtExt_Debug() )
					cerr << "proj types: " << arrayResult[0] << " " <<
													arrayResult[1] << endl;
				
        		if ( (arrayResult.length() ==2) && arrayResult[1].length() &&
            		(arrayResult[1] == MString("file") )  ) {
            
            
            		textureName = arrayResult[0];
            		fileTexture = true;
					
            		command = MString("getAttr ") + textureName + 
											MString(".fileTextureName");
            
            		status = MGlobal::executeCommand( command, result );
            
            		if ( status != MS::kSuccess ) {
            		    return 0;
            		}   
            
            		textureFile = result[0];

        			if( DtExt_Debug() )
						cerr << "use texture file: " << textureFile << endl;

				}
			}

		} else {

			// There is a texture, but it is not a file, or projection, but rather a procedural

            textureName = arrayResult[0];

	        if( DtExt_Debug() )
				cerr << "is a texture, but not file, name is: " 
						<< textureName << endl;
		}

		// Do nothing if texture is already in the list.
		//
		if( MtFindTex(local, (char *) textureName.asChar() ) != NULL)
		{
			// add texture name to material
			//
			mtl = MtFindMtl(local, (char *) shaderName.asChar() );

			if ( mtl->texture_name )
				free( mtl->texture_name );

			mtl->texture_name = strdup( textureName.asChar() );

			if ( mtl->texture_filename )
				free( mtl->texture_filename );

			if ( textureFile.asChar() )
				mtl->texture_filename = strdup( textureFile.asChar() );

			return( 1 );
		}
		

        // If we are doing file texture lets see how large the file is
      	// else lets use our default sizes.  If we can't open the file
		// we will use the default size.

		getTextureFileSize( fileTexture, textureName, textureFile, 
													xres, yres, zres );

		// Lets check that we are not going over the limit for some of  the
		// translators.  Lets try to rescale the evaluation here.

        if ( xres > DtExt_MaxXTextureRes() )
            xres_use = DtExt_MaxXTextureRes();
        else
            xres_use = xres;
            
        if ( yres > DtExt_MaxYTextureRes() )
            yres_use = DtExt_MaxYTextureRes();
        else
            yres_use = yres;
            



		// This portion to be completed for transparency
		//
		// Will use the convertSolidTx command to generate the image to use.
		//
		// Also to check for transparency if the convertSolidTx command 
		// doesn't do it for me.


		// If we are just reading in file textures do it that way.
	
		unsigned char *image = NULL;

		if ( !DtExt_inlineTextures() && fileTexture )
		{
			image = readTextureFile( MString(textureFile), MString( " " ), 
													0, xres, yres, zres );

            // Here we are not just reading in file textures, but will
            // call the convertSolidTx command to do the evaluation for us
            
		} else {
           
			char	cmdStr[2048];

			saveTransforms( textureName );

			// need to have different temp location for SGI and NT

			char tmpFile[2048];
			char tmpFileT[2048];

			sprintf( tmpFile, "%s/mdt%d.tmp", getenv("TMPDIR"), getpid() );

#ifdef WIN32
	        // Check for '\' characters and replace with '/'
	        // Makes passing strings to convertSolidTx easier
        
	        for ( unsigned int i=0; i < strlen(tmpFile); i++ )
	        {
	            if ( tmpFile[i] == '\\' ) 
	                tmpFile[i] = '/';
	        }
#endif

			// 
			// Check to see if a projection was found and if so then use it
			// as the base object to do the convert solid texture command on
			//

			if ( projectionFound )
			{
				convertName = projectionName;
			} else {
				convertName = textureName;
			}


			if ( DtExt_OriginalTexture() )
			{
				restoreTransforms( textureName );

				image = (unsigned char *)strdup( "originalFile" );

			} else {

				sprintf( cmdStr,
					"convertSolidTx -rx %d -ry %d -n \"%s\" -fin \"%s\" -uvr 0.0 1.0 0.0 1.0 %s %s",
                    	xres_use, yres_use, 
						"mdt_tmp_texture_name",
						tmpFile,
						convertName.asChar(), 
						dagPath.fullPathName().asChar() );
			
				if( DtExt_Debug() )
					cerr << "command to execute: " << cmdStr << endl;

				status = MGlobal::executeCommand( cmdStr );

				restoreTransforms( textureName );

				if ( status != MS::kSuccess )
				{
					cerr << "error from convertSolidTx " << 
									 status.errorString() << endl;
				    return 0;
				}   


				//
				// Lets now see about transparency settings
				//

				transFound = false;

				if ( !layeredFound && (resultT.length() && resultT[0].length()) ) 
				{
					transFound = true;

					sprintf( tmpFileT, "%s/mdtT%d.tmp", 
									getenv("TMPDIR"), getpid() );
#ifdef WIN32
			        // Check for '\' characters and replace with '/'
        			// Makes passing strings to convertSolidTx easier

        			for ( unsigned int ii=0; ii < strlen(tmpFileT); ii++ )
        			{
        			    if ( tmpFileT[ii] == '\\' )
        			        tmpFileT[ii] = '/';
        			}
#endif
            		sprintf( cmdStr,
					"convertSolidTx -rx %d -ry %d -n \"%s\" -fin \"%s\" -uvr 0.0 1.0 0.0 1.0 %s.transparency %s",
						    xres_use, yres_use, 
						    "mdt_tmp_textureT_name",
						    tmpFileT,
						    shaderName.asChar(),
						    dagPath.fullPathName().asChar() );

					if( DtExt_Debug() )
    				    cerr << "command to execute: " << cmdStr << endl;
            
    				status = MGlobal::executeCommand( cmdStr );
            
            		if ( status != MS::kSuccess )
            		{   
						cerr << "error from convertSolidTx " << 
					                     status.errorString() << endl;
						return 0;
            		}


				}


				// Read in the image file generated 

				image = readTextureFile( MString( tmpFile ), MString( tmpFileT ),
									transFound,  xres, yres, zres );

				// Need to delete the node that was generated by the convertSolidTx
				// command.

				MGlobal::executeCommand( "delete mdt_tmp_texture_name" );
			
				// Now lets remove the temp file that we generated

				unlink ( tmpFile );

				if ( transFound )
				{
				    // delete the transparency node if used
            
           			MGlobal::executeCommand( "delete mdt_tmp_textureT_name" );
            
					// Now lets remove the temp file that we generated
            
					unlink ( tmpFileT );
				}
			}
		}

		// If we have an image in memory, then we add it to our local table
		// references.

		if ( image )
		{	
   			if( !MtAddTex(local, (char *)textureName.asChar(), 
					(char *)textureFile.asChar(), fileTexture, 
						image, xres, yres, 4 ) )
			{
				DtExt_Msg("\"%s\" already in list delete this copy of image\n",
						(char *)textureName.asChar() );
				free( image );
			}

			// add texture name to material
			//
			mtl = MtFindMtl(local, (char* )shaderName.asChar() );
			mtl->texture_name = strdup( textureName.asChar() );
			mtl->texture_filename = strdup( textureFile.asChar() );
			mtl->hasAlpha = transFound;

			return 1;

		} else {

			return 0;

		}


	}

	return 0;
}


//
//	Be careful of Textures now that we may not be generating them all of the time.
//


static
unsigned char *readTextureFile( MString textureFile, MString transFile,
						int useTransparency, int &xres, int &yres, int &zres)
{
    IFFimageReader reader;
    MStatus Rstat;
            
    Rstat = reader.open( textureFile );
    if ( Rstat != MS::kSuccess )
    {   
    	cerr << "Error reading file " << textureFile.asChar() << " "
                << reader.errorString() << endl;
        return 0;
    }
            
    int imageWidth,imageHeight,bytesPerChannel;
	
	//int imageComp;
	
    Rstat = reader.getSize (imageWidth,imageHeight);
    if ( Rstat != MS::kSuccess )
    {   
    	reader.close();
        return 0;
    }

	bytesPerChannel = reader.getBytesPerChannel ();

#if 0
	// This really should always return a 4 component image, as that
	// is what we defined in the iffreader class we wanted.

    imageComp = 1;
    if (reader.isRGB () || reader.isGrayscale ())
    {   
    	if (reader.isRGB ())
        {   
           imageComp = 3;
           if (reader.hasAlpha ())
              imageComp = 4;
        }
    }
#endif       
            
    unsigned char *image = (unsigned char *)malloc(imageWidth*imageHeight*4);
    unsigned char *imagePtr = image;
            
    Rstat = reader.readImage ();
    if ( Rstat != MS::kSuccess )
    {
        reader.close();
		free( image );
        return 0;
    }

    // Now we can read the image into our area.  We only deal with 8 bit colours
	// so if it is a 16bit file, we will truncate down to 8 bits.

    int x, y;
    const byte *bitmap = reader.getPixelMap ();
    const byte *pixel = bitmap;
    int bytesPerPixel = (bytesPerChannel == 1) ? 4 : 8;
           

	// Need to see if we actually read in an image file

	if ( pixel == 0 )
	{
		reader.close();
		free( image );
		return 0;
	}
		
	//printf(" widht %d, height %d, bpp %d \n", 
	//			imageWidth, imageHeight, bytesPerPixel );
	
    for ( y = 0; y < imageHeight; y++ ) {
           
		for ( x = 0; x < imageWidth; x++, pixel += bytesPerPixel ) {
                
			if ( bytesPerChannel == 1 )
        	{
            	imagePtr[0] = pixel[0];
            	imagePtr[1] = pixel[1];
            	imagePtr[2] = pixel[2];
            	imagePtr[3] = pixel[3];
          	} else {
            	imagePtr[0] = pixel[0];
            	imagePtr[1] = pixel[2];
            	imagePtr[2] = pixel[4];
            	imagePtr[3] = pixel[6];
        	}   

			imagePtr += 4;
                    
    	}   
                
	}   
            
    Rstat = reader.close();

	xres = imageWidth;
	yres = imageHeight;
	zres = 4;

	//
	// Now most of this is repeated to read in the transparency mask
	//

	if ( useTransparency )
	{

    	Rstat = reader.open( transFile );
    	if ( Rstat != MS::kSuccess )
    	{    
    	    cerr << "Error reading transparency file " 
				<< transFile.asChar() << " "
                << reader.errorString() << endl;
        	return image;
    	}   
        
    
    	Rstat = reader.getSize (imageWidth,imageHeight);
    	if ( Rstat != MS::kSuccess )
    	{    
    	    reader.close();
    	    return image;
    	}   
    
    	bytesPerChannel = reader.getBytesPerChannel ();

#if 0   
    	// This really should always return a 4 component image, as that
    	// is what we defined in the iffreader class we wanted.
    
    	imageComp = 1;
    	if (reader.isRGB () || reader.isGrayscale ())
    	{   
    	    if (reader.isRGB ())
    	    {   
    	       imageComp = 3;
    	       if (reader.hasAlpha ())
    	          imageComp = 4;
    	    }     
    	}   
#endif   
        
    	imagePtr = image; 
             
    	Rstat = reader.readImage ();
    	if ( Rstat != MS::kSuccess )
    	{
    	    reader.close();
        	return image;
    	}   
    
    	// Now we can read the image into our area.  
		// We only deal with 8 bit colours    
		// so if it is a 16bit file, we will truncate down to 8 bits.
    
    	bitmap = reader.getPixelMap ();
    	pixel = bitmap;
    	bytesPerPixel = (bytesPerChannel == 1) ? 4 : 8;

	    // Need to see if we actually read in an image file
    
	    if ( pixel == 0 )
	    {   
	        reader.close();
	        return image;
	    }   

    	//printf(" widht %d, height %d, bpp %d \n", 
    	//          imageWidth, imageHeight, bytesPerPixel );

    	for ( y = 0; y < imageHeight; y++ ) {
        	for ( x = 0; x < imageWidth; x++, pixel += bytesPerPixel ) {

#if 0
				// Lets keep this here for use later.  maybe useful
				if ( y == 6 && x <= 64 )
				printf( "color 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x  trans  0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
					imagePtr[0], imagePtr[1],imagePtr[2],imagePtr[3],
					pixel[0], pixel[1], pixel[2], pixel[3] );
#endif

				//
				// For now we check to see which platform we are on and 
				// generate the alpha based on the ordering.  Seems to be
				// different on the different platforms.  Maybe taked care 
				// of in the IFF library.
				//

            	if ( bytesPerChannel == 1 )
            	{
#if		defined(WIN32) || defined(LINUX)
					imagePtr[3] = static_cast<unsigned char>
						(255 - (pixel[0] + pixel[1] + pixel[2])/3.);
#else
					imagePtr[0] = static_cast<unsigned char>(255 - (pixel[1] + pixel[2] + pixel[3])/3.0);
#endif
				} else {
#if		defined(WIN32) || defined(LINUX)
					imagePtr[6] = static_cast<unsigned char>
						(255 - (pixel[0] + pixel[2] + pixel[4])/3.);
#else
					imagePtr[0] = static_cast<unsigned char>(255 - (pixel[0] + pixel[2] + pixel[4])/3.0);
#endif
				}

				imagePtr += 4;

			}
    	}

    	Rstat = reader.close();
	}

	return image;
}

//-
// ==========================================================================
// Copyright (C) 1995 - 2006 Autodesk, Inc. and/or its licensors.  All 
// rights reserved.
//
// The coded instructions, statements, computer programs, and/or related 
// material (collectively the "Data") in these files contain unpublished 
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its 
// licensors, which is protected by U.S. and Canadian federal copyright 
// law and by international treaties.
//
// The Data is provided for use exclusively by You. You have the right 
// to use, modify, and incorporate this Data into other products for 
// purposes authorized by the Autodesk software license agreement, 
// without fee.
//
// The copyright notices in the Software and this entire statement, 
// including the above license grant, this restriction and the 
// following disclaimer, must be included in all copies of the 
// Software, in whole or in part, and all derivative works of 
// the Software, unless such copies or derivative works are solely 
// in the form of machine-executable object code generated by a 
// source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. 
// AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED 
// WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF 
// NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
// PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE, OR 
// TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS LICENSORS 
// BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL, 
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK 
// AND/OR ITS LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY 
// OR PROBABILITY OF SUCH DAMAGES.
//
// ==========================================================================
//+

