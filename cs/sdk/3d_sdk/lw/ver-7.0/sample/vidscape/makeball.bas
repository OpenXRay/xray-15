' makeball.bas
' Create a checkered ball object in VideoScape .geo format
' Ernie Wright  30 Mar 00

DEFSNG A-H, O-Z
DEFINT I-N

CONST pi = 3.141592653589793#

nsides = 16                         'number of sides
nsegments = 8                       'number of segments
rx = 1                              'x radius
ry = 1                              'y radius
rz = 1                              'z radius
ic1 = 128 + 15                      'smooth white
ic2 = 128 + 12                      'smooth red
filename$ = "ball.geo"              'output filename

npts = 2 + nsides * (nsegments - 1)

OPEN filename$ FOR OUTPUT AS 1
PRINT #1, "3DG1"
PRINT #1, npts

'----- write point coordinates

FOR j = 0 TO nsegments
   v = j * pi / nsegments - pi / 2
   sv = SIN(v)
   cv = COS(v)
   y = ry * sv

   FOR i = 0 TO nsides - 1
      u = i * 2 * pi / nsides - pi
      su = SIN(u)
      cu = COS(u)
      x = rx * cv * cu
      z = rz * cv * su
      PRINT #1, x; y; z
      IF (j = 0) OR (j = nsegments) THEN EXIT FOR
   NEXT i
NEXT j

'----- write polygons

FOR i = 1 TO nsides
   IF (i MOD 2) = 1 THEN ic = ic1 ELSE ic = ic2
   PRINT #1, "3 0 "; i; (i MOD nsides) + 1; ic
NEXT i

k = 1
FOR j = 1 TO nsegments - 2
   FOR i = 0 TO nsides - 1
      IF ((i + j) MOD 2) = 0 THEN ic = ic1 ELSE ic = ic2
      PRINT #1, "4 ";
      PRINT #1, k + i;
      PRINT #1, k + nsides + i;
      PRINT #1, k + nsides + ((i + 1) MOD nsides);
      PRINT #1, k + ((i + 1) MOD nsides);
      PRINT #1, ic
   NEXT i
   k = k + nsides
NEXT j

k = npts - nsides - 1
FOR i = 0 TO nsides - 1
   IF ((i + nsegments) MOD 2) = 1 THEN ic = ic1 ELSE ic = ic2
   PRINT #1, "3 "; npts - 1; k + ((i + 1) MOD nsides); k + i; ic
NEXT i

CLOSE #1

