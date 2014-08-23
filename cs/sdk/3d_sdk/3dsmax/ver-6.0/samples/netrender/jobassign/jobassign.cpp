//-----------------------------------------------------------------------------
// listjobs.cpp
//
//  MaxNet API Sample Program
//
//  listjobs [managername [port]]
//

//-- OS Includes
#include <windows.h>
//-- MaxNet API Includes
#include <maxnet.h>

static char work_path[MAX_PATH];

static bool file_size(char* filename, DWORD* size) {
	HANDLE findhandle;
	WIN32_FIND_DATA file;
	findhandle = FindFirstFile(filename,&file);
	FindClose(findhandle);
	if	(findhandle	==	INVALID_HANDLE_VALUE)
		return false;
	if (size)
		*size = file.nFileSizeLow;
	return true;
}

static bool assign_job (MaxNetManager* manager, char *filename) {
	Job			job;
	CJobText	jobText;
	//-- Fill in job structure using information found in the
	//   scene file (filename)
	if (jobReadMAXProperties(filename,&job,jobText)) {
		//-- At this point, the job structure is complete for submission.
		//   Any changes you want to make could be done here (such as a
		//   different output resolution, etc.
		//   You would just change job.width, job.height, etc.
		//-- File list (just one file in this case. By setting the entire
		//   char array to zeroes, it is guaranteed the array ends with two
		//   nulls.
		char filelist[MAX_PATH + 2];
		memset(filelist,0,MAX_PATH + 2);
		strcpy(filelist,filename);
		//-- Archive the secene file
		char archive[MAX_PATH];
		strcpy(archive,work_path);
		strcat(archive,job.name);
		strcat(archive,".maz");
		if (Maz(archive,filelist,&job.filesizeextracted)) {
			//-- Archive size
			if (!file_size(archive,&job.filesize))
				return false;
			//-- Submit job
			manager->AssignJob(&job,archive,0,jobText);
			DeleteFile(archive);
			return true;
		}
	}
	//-- Something went wrong
	//-- Note that if an error occurs while submitting the job, the
	//   error would go right through all this and be caught by the
	//   exception handler below. Returning false here means something
	//   went wrong at this level.
	return false;
}

int main (int argc, char* argv[]) {

	if (argc < 2) {
		printf("Usage:\n");
		printf("%s max_filename [manager [port]]\n",argv[0]);
		return 1;
	}

	//-- Come up with a temporary path for creating archives
	if (!ExpandEnvironmentStrings("%TEMP%",work_path,MAX_PATH)) {
		if (!ExpandEnvironmentStrings("%TMP%",work_path,MAX_PATH)) {
			if (!ExpandEnvironmentStrings("%HOMEPATH%",work_path,MAX_PATH))
				strcpy(work_path,"c:\\");
		}
	}

	if (work_path[strlen(work_path) - 1] != '\\')
		strcat(work_path,"\\");

	//-- Creates new manager access module instance
	MaxNetManager* manager = CreateManager();
	if (manager) {
		try {
			//-- Setup default manager port
			short port = DF_MGRPORT;
			//-- Placeholder for manager name
			char managername[MAX_PATH];
			//-- If we have a port defined, use it
			if (argc > 3)
				port = atoi(argv[3]);
			//-- If we have a host name (or ip address) for the manager, use it
			if (argc > 2)
				strcpy(managername,argv[2]);
			else {
				//-- Otherwise, find out who is the manager (if any)
				//   The API will broadcast a message looking for a
				//   manager using the given port. If a manager is
				//   found, its name/ip address will be placed in 
				//   "managername" below.
				if (!manager->FindManager(port,managername)) {
					printf("Could not find a Manager\n");
					DestroyManager(manager);
					return 1;
				}
			}
			//-- Connect to the manager
			manager->Connect(port,managername);
			//-- We're connected. Sumbmit Job(s)...
			WIN32_FIND_DATA find;
			HANDLE findhandle = FindFirstFile(argv[1],&find);
			if (findhandle != INVALID_HANDLE_VALUE) {
				char opath[MAX_PATH],npath[MAX_PATH];
				_splitpath(argv[1],opath,npath,0,0);
				strcat(opath,npath);
				if (opath[strlen(opath) - 1] != '\\')
					strcat(opath,"\\");
				do {
					strcpy(npath,opath);
					strcat(npath,find.cFileName);
					printf("Submitting %s...",find.cFileName);
					if (!assign_job(manager,npath))
						break;
					printf("\r%s Submitted successfully...\n",find.cFileName);
				} while (FindNextFile(findhandle,&find));
				FindClose(findhandle);
			}
			manager->Disconnect();
		//-- Exception Handler
		} catch (MaxNet* maxerr) {
			printf("\n%s (0x%08X)\n",maxerr->GetErrorText(),maxerr->GetError());
		}
		//-- Clean up manager instance
		DestroyManager(manager);
	}
	return 0;
}
