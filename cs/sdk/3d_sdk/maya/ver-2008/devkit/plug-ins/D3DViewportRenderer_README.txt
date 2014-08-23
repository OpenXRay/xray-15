
D3DViewportRenderer Viewport Renderer Plugin
============================================
Last updated: June 12, 2007

This plugin is a simple example of how Direct3D can be used as the renderer for a 
interactive modeling viewport. It is not meant to be a full replacement for 
existing viewport renderers.

The plugin is only an example, and is provided as is to demonstrate the interfaces 
required to be overridden for a MViewportRenderer subclass. MViewportRenderer
is a class introduced with Maya version 8.

1) Features:

It will render the set of visible polygonal surfaces from a given camera in an 
interactive modeling viewport using the fixed-function pipeline. Nurbs and
subdivision surfaces, and the modeling grid will render as wireframe bounding boxes.

Basic shading for materials such as Maya Lambert, Blinn, Phong and PhongE 
is supported. For these materials, the "color" channel can be mapped to a file 
texture. Any file texture supported by Microsoft's file texture reader should work.
Other Maya specific supported formats are not currently supported.

Maya's Ambient, Point, Directional, and SpotLights are also supported. Up to
8 lights are supported (fixed function limit).

Caching for lazy update of geometry, and textures has been added as part of a 
built in resource manager.

A very simple full-screen post-processing mechanism has been included to allow for 
custom HLSL effects to be applied using the original color target output.

New effects can be added by following the simple rules currently used to determine
if the effect is a post effect:

	1) If a filter kernal is required then specify:

	const half|float duKernel;
	const half|float dvKernel;

	The u and v values are basically the 1/width and 1/height of the current render target.

	2) A texture called "textureSourceColor" is required to access a screen space
	2D texture. This is the input color render target.

	3) A technique called "PostProcess" is required, with at least 1 pass and
	corresponding pixel shader.


The selection mechanism is not currently controllable by the renderer. 
Selection is performed internally within Maya. Selection state however can be queried by 
the renderer. This plugin uses this state currently to show wireframe-on-shaded for selected 
objects.

The viewport override is not supported in the Hardware Render Buffer, but is supported
in all modeling viewports. Playblast can be used to save images, or movies to disk.

As Maya uses OpenGL, this plugin renders into an offscreen surface (render target), 
and then copies the rendering image to the final OpenGL target. It is possible
with large viewport sizes that refresh speed may be affected.

The ability to copy back depth or stencil is not currently supported. 
To add this a readback from a depth surface into a system memory surface is required. 
See the MImage class for depth image specifications.

The are other implementation detail caveats listed in the source code for the plugin.

Building the plugin:
--------------------

The provided solution files have been tested for building the 32-bit version 
and 64-bit version of the  plugin using Microsoft VC8 (2005). The DirectX Developer SDK 
compiled with is the June, 2007 release from Microsoft. The include and link library directories will 
need to be modified to point to the location that Maya's SDK has been installed.

The preprocessor directive D3D9_SUPPORTED must be set to 
have the plugin use Direct3D code, otherwise this plugin will basically 
do nothing. The solution files have this directive set.

Using the plugin:
-----------------
1. Installing the plugin:

Copy the plugin to somewhere in your plugin path. e.g. (bin/plug-ins directory)
If the SDK has not been installed then the DirectX End-User Runtimes (June 2007) must be 
installed as the plugin is dependent on these libraries. By default with Maya 2008,
the correct version of the runtime will automatically be installed for you via the Maya
installer.

2. Installing post-effects files: (bin directory)
Copy the .fx files in (shaders) to where the maya executable is located.

3. Installing MEL UI:
Copy the mel files to the scripts/others directory. Replace any existing script of 
the same name.

4. Loading the plugin:

Load the plugin from the Plugin Manager window. Should the plugin fail to load, recheck that 
the correct dependent DLLS have been installed.

Once the plugin is registered, a new "Direct3D Renderer" menu item should appear 
under the "Renderer" menu in each of  the 3d modeling viewports. Selecting that menu item 
will invoke usage of the renderer. 

An option box has also been written to provide an interface to effects options on the 
renderer. Should the options have no effect, check that the .fx files have been installed 
properly.

Floating Point issues:
----------------------

Note that we always attempt to create a D3D9 device with the flag D3DCREATE_FPU_PRESERVE.
This is to avoid any possible floating point computation issues which can come about as
a result of the default behaviour when this option is not used. Please refer to Microsoft
documentation for an explanation of it's usage.

Feedback:
---------
If you plan to make any additions or changes to the plugin, or have any feedback, please send 
this to us so that we can incorporate improvements into the plugin. Thank you.

---









	

