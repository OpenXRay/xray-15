This directory tree contains the complete SDK for character studio 4.2.

Note that the SDK has changed since CS 4.0/4.1.  The changes are simply new functions. No existing functions where changed so all that should be required is a recompile.  

In the include directory, are the complete header files for the biped,crowd and physique sdk. This differs from the previous verions where biped and physique headers were in the in the root directory and the crowd headers were in a seperate directory.

In the Crowd directory there is documentation for the crowd sdk plus two sample SDK behaviors, pursuit and formation.
In the Doc directory are various documention on the biped and physique SDK.
In the Sample directory are code snippets and maxscript examples.
In the Lib directory are the biped export symbols and library files.

NEW IN CS 4.1

There is a new mixer SDK, found in imixer.h, new documentation for the new mixer maxscript in the Doc directory and a samples using mixer maxscript and SDK calls in the Sample Directory.

NEW IN CS 4.2

There are new functions/maxscript calls for setting keys(planted, free and sliding),a new function for resetting IK keys,which is useful if you have limbs with scale subanims, and for collapsing and baking move all mode.  In addition  a couple other functions that where previously only in the SDK have been made accessible to maxscript for completeness. All of this is documented in the Doc directory in the CS4.2SDK.doc file.

