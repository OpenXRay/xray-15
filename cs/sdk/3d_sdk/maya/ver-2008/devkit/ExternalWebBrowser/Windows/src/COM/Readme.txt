MayaCmdCOM Project - Maya(R) MEL Command Engine Interface
=========================================================
(c) 2006 Autodesk, Inc. All rights reserved.

This Microsoft Visual Studio .NET 2003 Project builds the MayaCmdCOM.dll
shared library, which contains the:

- type library for the IMELCommand interface and the MELCommand class
- implementation of the MELCommand class
- registry entries and implementation of the mel: URL protocol handler

Performing a Release or Debug build will register the DLL for both
the type library and the mel: URL protocol handler.

The IMELCommand.htm file documents the IMELCommand Automation interface.

Both the mel: URL handler and the MELCommand class use "commandportDefault"
as the default command port name. Be sure to open a command port with this
name from Maya, either through the preferences (in Forge or later), or by
entering:

  commandPort -name "commandportDefault";

in either the Maya Script Editor or Command Shell window.

Try the various tests in the Tests folder. Note that the Test.htm file
is designed to work only with Internet Explorer, although the mel: URLs
should work in any web browser.

The Register.bat and Unregister.bat batch files are provided to
register and deregister the Release build of MayaCmdCOM.dll.
