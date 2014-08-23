#include "windows.h"
#include "max.h"
#include "bmmlib.h"
#include "guplib.h"
#include "notify.h"
#include "resource.h"


TCHAR *GetString(int id);
ClassDesc* GetSaveVerificationDesc();


class SaveVerificationGUP : public GUP {
	public:
		SaveVerificationGUP();
		~SaveVerificationGUP();
	
		DWORD	Start();
		void	Stop();
		DWORD	Control(DWORD parameter);
		void	DeleteThis();

		void	ValidateFile(TSTR filename);
		bool	ValidateStreams(TSTR filename);
		bool	HasStream(IStorage* storage, TSTR stream);

		static void	PostSaveCallback(void* param, NotifyInfo*);
};