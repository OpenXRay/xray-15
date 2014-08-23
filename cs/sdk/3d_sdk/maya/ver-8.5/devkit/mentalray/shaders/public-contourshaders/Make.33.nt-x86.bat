# automatically generated Thu Jul 29 12:09:24 MST 2004
del shader.h geoshader.h mi_version.h
copy shader.33.h shader.h
copy geoshader.33.h geoshader.h
copy shader33x86.lib shader.lib
copy mi_version.33.h mi_version.h
cl -c /O2 /G6  /MD /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. contourshade.c
cl -c /O2 /G6  /MD /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. outimgshade.c
cl -c /O2 /G6  /MD /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. outpsshade.c
link /delayload:opengl32.dll /nologo /nodefaultlib:LIBC.LIB /MAP:mapfile /OPT:NOREF /INCREMENTAL:NO /MACHINE:IX86 /STACK:0x200000,0x1000  ws2_32.lib user32.lib mpr.lib largeint.lib opengl32.lib gdi32.lib delayimp.lib /DLL /OUT:contour.dll contourshade.obj outimgshade.obj outpsshade.obj shader.lib raylib.res
