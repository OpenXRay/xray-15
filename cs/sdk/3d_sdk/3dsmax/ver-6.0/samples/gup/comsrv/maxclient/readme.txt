In order to compile this project the following line found in MaxClientDlg.h
needs to reflect your current path structure.


#import "..\..\..\..\..\exe\stdplugs\comsrv.gup

If when you compile you receive an error saying comsrv.gup can't be found then
this is the cause.