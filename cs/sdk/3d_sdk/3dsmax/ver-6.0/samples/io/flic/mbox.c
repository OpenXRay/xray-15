#include <windows.h>
 
void
DoMessageBox(HWND w,const char *text,const char *heading,int buttons) {
	MessageBox(w,text,heading,buttons);
	}

void DoOutputDebugString(char *string) {
	OutputDebugString(string);
	}
