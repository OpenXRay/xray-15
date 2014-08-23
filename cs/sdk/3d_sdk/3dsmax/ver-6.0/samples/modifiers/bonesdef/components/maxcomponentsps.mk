
MAXComponentsps.dll: dlldata.obj MAXComponents_p.obj MAXComponents_i.obj
	link /dll /out:MAXComponentsps.dll /def:MAXComponentsps.def /entry:DllMain dlldata.obj MAXComponents_p.obj MAXComponents_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del MAXComponentsps.dll
	@del MAXComponentsps.lib
	@del MAXComponentsps.exp
	@del dlldata.obj
	@del MAXComponents_p.obj
	@del MAXComponents_i.obj
