
README - Hardware Plugin Shaders 

REQUIREMENTS
------------
For all projects, you will need:
1. Maya4.0.1 or later.
2. Microsoft VC++ 6.0 (also tested with Intel Compiler).

For the nVidia-specific demo shaders (shaders ending with the _NV20 suffix), 
you will need:

1. A video card that supports the required extensions. At the time of writing this document,
   only the nVidia Geforce3 and GeforceDCC supported them.
2. nVidia SDK (freely available at www.nvidia.com)

For the ATI-specific demo shaders (pnTriangles), you will need:

1. A video card that supports the required extensions. At the time of writing this document,
   only the ATI Radeon 8500 and higher cards supported them.
2. There is no ATI SDK required. The required extensions and binding mechanism are
   provide as part of the example shader.

COMPILATION
-----------
1. Open the projects using VC++.
2. If required, update the include path to point to the nVidia SDK.
   In VC++, go to Tools->Options->Directories->Include and add an entry for the nVidia SDK GLH folder.
  
	(typically: C:\PROGRAM FILES\NVIDIA CORPORATION\OPENGL SDK\INCLUDE\GLH)

3. Compile. The corresponding .mll files will get saved in the devkit\plugins directory.

USAGE
-------
1. Start Maya v4.5 
2. Load the plug-ins you compiled earlier, using the Plug-In Manager.
3. You can try loading of some of the sample scenes provided in the gift section. 
   Scenes whose filename is terminated by NV20 will not display properly if your video card 
   doesn't support the nVidia-specific extensions.
4. Read the "Maya Hardware Shader white paper" for more information. This document is available
   at http://www.alias.com/en/Community/Games/includes/MayaHWSWhitePaper.zip


OVERVIEW OF THE SAMPLE PROJECTS
-------------------------------
hwUnlitShader: Simple hardware shader which uses regular OpenGL texturing and disables 
               lighting. Worth looking at before jumping to more complex projects.

    This sample demonstrates how to:
      - Specify a color attribute on your HW shader plug-in 
        so that end-users can attach a file texture to it.
      - Traverse what's upstream of the color attribute, including 
        going through shading switches if required.
      - Using the MImage API class to load and uncompress a file 
        texture in any Maya-supported file format.
      - Setting up the graphics pipeline to display the 
        texture correctly.

hwDecalBumpShader_NV20: NV20-specific (Geforce3) sample shader.
                        This shader can simultaneously display both decal 
                        (base color) and bump textures.

hwReflectBumpShader_NV20: NV20-specific (Geforce3) sample shader.
                          This shader can simultaneously display both  
                          reflection and bump.

hwAnisotropicShader_NV20: NV20-specific (Geforce) sample shader.
						  This shader computes anisotropic shading, and
						  supports direction, and point lights with or
						  without decay.

hwToonShader_NV20 : NV20-specific (Geforce) sample shader.
				  This shader computes a cartoon shading effect using
				  an user definable lookup table for it's light model.

    These the latter shaders build on the foundation demonstrated in the 
    hwUnlitShader.

pnTriangles : ATI Radeon (8500..) sample shader
			This shader demonstrates the usage of hardware tessellation
			using PN-triangles, as well as simple vertex programmability.

    Additionally, these sample demonstrate how to:
      - Use vendor-specific extensions, namely vertex programs,
        texture shaders and register combiners, to achieve
        effects that are impossible in standard OpenGL.
      - Convert height field bump format (used by Maya) into
        a normal map format, for real-time rendering.

    Some parameters are easily customizable:
      - The MNormalMapConverter::convertToNormalMap_InPlace()
        bumpScale parameter is currently constant. You can change
        it to a different value to increase or decrease the 
        bumpiness.


MTexture: A simple class that abstracts OpenGL texture objects, that can
          load a texture image from disk and generate mipmaps.

MTextureCache: A simple texture cache, that include a dirtying mechanism.

MNormalMapConverter: Demonstrates how to convert a Maya height-field bump map
                     to a normal map suitable for HW display.

NodeMonitor: Provides an interface to abstract dirtying callbacks.

ShadingConnection: Provides an interface to abstract shading network traversal.

ATTRIBUTE EDITORS
-----------------
Under the Gifts directory there is a subdirectory called hwShaders which
contains two sample scenes which contain sample attribute editors 
for each of the three shader types. They are called:

- AEhwDecalBumpShader_NV20Template.mel
- AEhwReflectBumpShader_NV20Template.mel
- AEhwUnLitShaderTemplate.mel

To use these attribute editor templaes change the
MAYA_SCRIPT_PATH environment variable to include
a path to these MEL scripts.

SAMPLE SCENES 
-------------
Under the Gifts directory there is a subdirectory called hwShaders which
contains two sample scenes which use the sample plugin shaders.

1. sampleBumpDecal_NV20.ma

This scene contains a single polygonal object (a shark) which uses the 
hwDecalBumpShader_NV20.

There is a single color texture used for the decal texture, and a greyscale
height field texture used as the bump texture. These same textures
have been connected up a software shader (Blinn shader) for software rendering.
As loaded, the hardware shader is assigned to the body of the shark object.
To view the software rendering, the shader assignment needs to be changed
to use the software shader.

The hwDecalBumpShader_NV20 uses only the directional light in the
scene. The "rotation" attribute has been connected the "light" attribute on
the hwDecalBumpShader_NV20. This allows for the shader to know about
changes in light orientation. Additional light parameters of "shininess" and
"light color" are user modifiable.

There is IK animation on the shark.

2. sampleBumpReflect_NV20.ma

This scene contains four polygonal planes which form a cube, and a polygonal
"Maya" logo. 

Each plane of the cube uses a separate hwUnLitShader shader, with each
shader assigned to a different file texture. The cube is used
to display a "baked" environment.

The "Maya" logo has a hwReflectBumpShader_NV20 assigned to it.
The the color channel is assigned a environment cubemap node. The same textures
used for the "baked" environment cube is used for the cubemap node.
A "chrome" like bump map is used to bump the normals before the cubemap
lookup.

For software rendering the assignment to the "Maya" logo needs
to be changed to use the sofware shader which reuses the
environment cubemap node, and the bump map. The hwUnlitShaders
have had the texture mapped directly to the "outColor" attribute
on each shader respectively for software rendering. There
is no requirement for separate software shaders in this case.

There is keyframe animation which will cause the "Maya" logo
to revolve.

DISCLAIMER
---------
This code is provided as is, and is provided only as an example of
how to take advantage of the hardware shading interface. These plugins
are not fully supported.
