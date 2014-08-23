# automatically generated Thu Jul 29 11:45:38 MST 2004
del shader.h geoshader.h mi_version.h
copy shader.34.h shader.h
copy geoshader.34.h geoshader.h
copy shader34x86.lib shader.lib
copy mi_version.34.h mi_version.h
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baseblinn.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basecheck.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basecolor.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basecooktorr.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basedielec.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baseenviron.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basegeo.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basehair.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baseior.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baselambert.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baselens.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baselight.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baselightmap.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basemux.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basephong.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basephotbas.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basepolka.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baseraymarch.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basereflect.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baserefract.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baseshadow.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basetexgen.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basetexgeo.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basetexlook.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basetrans.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baseturb.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basetwoside.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basevolume.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baseward.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basewardderi.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. basewave.cpp
cl -c -DDEBUG /Z7  /MDd /nologo /W3 -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DNV_CG -DHYPERTHREAD -DX86 -I. baseocc.cpp
link /delayload:opengl32.dll /nologo /nodefaultlib:LIBC.LIB /MAP:mapfile /OPT:NOREF /INCREMENTAL:NO /MACHINE:IX86 /STACK:0x200000,0x1000 /DEBUG /pdb:none  ws2_32.lib user32.lib mpr.lib largeint.lib opengl32.lib gdi32.lib delayimp.lib /DLL /OUT:base.dll baseblinn.obj basecheck.obj basecolor.obj basecooktorr.obj basedielec.obj baseenviron.obj basegeo.obj basehair.obj baseior.obj baselambert.obj baselens.obj baselight.obj baselightmap.obj basemux.obj basephong.obj basephotbas.obj basepolka.obj baseraymarch.obj basereflect.obj baserefract.obj baseshadow.obj basetexgen.obj basetexgeo.obj basetexlook.obj basetrans.obj baseturb.obj basetwoside.obj basevolume.obj baseward.obj basewardderi.obj basewave.obj baseocc.obj shader.lib raylib.res
