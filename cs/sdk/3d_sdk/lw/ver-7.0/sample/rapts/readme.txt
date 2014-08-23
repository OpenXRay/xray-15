Recycled Additional Procedural Textures for Six  (RAPTS)
--------------------------------------------------------

"Recycle: You'll never see the end of it."


This directory contains the source code for 10 procedural textures that
can be used in version 6.0 of Lightwave 3D. Most of these procedurals
were Lightwave 5.6 shaders found in the Shades project:

http://amber.rc.arizona.edu/lw/shades/


The fractal and noise routines used in these procedurals are presented
and discussed in "Texturing and Modeling: A Procedural Approach" by Ebert,
Musgrave, Peachey, Perlin, and Worley.

http://www.csee.umbc.edu/~ebert/book2e.html


The pseudorandom number generator used by the noise routines is called
Mersenne Twister developed by Makoto Matsumoto and Takuji Nishimura.
Several versions of the code and a scientific paper describing this
freeware random number generator can be found at:

www.math.keio.ac.jp/~matumoto/emt.html



These procedural textures can be used in all of the Lightwave 6 surface
parameters, including the bump channel. And don't forget, they may also
be used as displacement procedurals in an object's Item Properties panel.
The Multifractal procedurals can be used as height displacements on a
subdivided patch, and LW6 will allow the user to specify independent
subdivision levels for rendering and OpenGL display.


Marvin Landis
