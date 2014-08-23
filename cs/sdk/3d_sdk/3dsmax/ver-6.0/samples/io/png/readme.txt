This plugin implements support for the PNG bitmap format.
It relies on two libraries, libpng and zlib, the source
for which can be located at their respective home pages.

LibPNG:
http://www.wco.com/~png/
We've integrated version 0.96
A complete set of PNG test images is also located here

ZLib:
http://www.cdrom.com/pub/infozip/zlib/
We've integrated version 1.0.4


Note:  the libpng source has been altered slightly from
the original as it is distributed for 0.96, in order to
eliminate compiler warnings.  Specifically, this meant
a type change for a variable in libpng\pngwtran.c. Other
than this change, the source is as distributed for both libraries.
===================== pngwtran.c
255c255
<             int c;
---
>             png_uint_32 c;
281c281
<             int c;
---
>             png_uint_32 c;


