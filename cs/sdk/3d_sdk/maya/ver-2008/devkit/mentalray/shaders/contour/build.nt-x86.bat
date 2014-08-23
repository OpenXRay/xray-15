cl -c /O2 /G6 /MD /nologo /W3 /Zc:forScope /EHsc /Ob2 -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DSSE_INTRINSICS -DX86 -I.  contourshade.c
cl -c /O2 /G6 /MD /nologo /W3 /Zc:forScope /EHsc /Ob2 -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DSSE_INTRINSICS -DX86 -I.  outimgshade.c
cl -c /O2 /G6 /MD /nologo /W3 /Zc:forScope /EHsc /Ob2 -DQMC -DMI_MODULE= -DMI_PRODUCT_RAY -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DSSE_INTRINSICS -DX86 -I.  outpsshade.c
link /delayload:opengl32.dll /nologo /nodefaultlib:LIBC.LIB /MAP:mapfile /OPT:NOREF /INCREMENTAL:NO /MACHINE:X86 /LIBPATH:../../lib /STACK:0x200000,0x1000 ws2_32.lib user32.lib mpr.lib largeint.lib opengl32.lib gdi32.lib delayimp.lib /DLL /OUT:contour.dll contourshade.obj outimgshade.obj outpsshade.obj ../../nt-x86/lib/shader.lib

