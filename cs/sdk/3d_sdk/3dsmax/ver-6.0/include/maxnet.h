//-----------------------------------------------------------------------------
// -------------------
// File ....: maxnet.h
// -------------------
// Author...: Gus J Grubba
// Date ....: February 2000
//
// Descr....: 3D Studio MAX Network Interface
//
// History .: Feb, 07 2000 - Started
//            
//-----------------------------------------------------------------------------

#ifndef _MAXNET_H_
#define _MAXNET_H_

#ifdef WIN32
#ifndef MAXNETEXPORT
#define MAXNETEXPORT __declspec( dllimport )
#endif
#else
#define MAXNETEXPORT
#endif

//-----------------------------------------------------------------------------
//-- MaxNet Errors

typedef enum {
	MAXNET_ERR_NONE = 0,
	MAXNET_ERR_CANCEL,
	MAXNET_ERR_NOMEMORY,
	MAXNET_ERR_FILEIO,
	MAXNET_ERR_BADARGUMENT,
	MAXNET_ERR_NOTCONNECTED,
	MAXNET_ERR_NOTREADY,
	MAXNET_ERR_IOERROR,
	MAXNET_ERR_CMDERROR,
	MAXNET_ERR_HOSTNOTFOUND,
	MAXNET_ERR_BADSOCKETVERSION,
	MAXNET_ERR_WOULDBLOCK,
	MAXNET_ERR_SOCKETLIMIT,
	MAXNET_ERR_CONNECTIONREFUSED,
	MAXNET_ERR_ACCESSDENIED,
	MAXNET_ERR_TIMEOUT,
	MAXNET_ERR_BADADDRESS,
	MAXNET_ERR_HOSTUNREACH,
	MAXNET_ERR_DUPLICATE_JOB_NAME,
	MAXNET_ERR_UNKNOWN
} maxnet_error_t;

//---------------------------------------------------------
//-- Special Types

typedef struct tag_HSERVER {
	BYTE addr[8];
} HSERVER;

#define HBSERVER	(BYTE *)(void *) 
typedef DWORD		HJOB;

//---------------------------------------------------------
//-- Server Work Schedule

#define HIGHPRIORITY		0
#define LOWPRIORITY 		1
#define IDLEPRIORITY		2

typedef struct tag_Schedule {
	DWORD hour;						//-- Bitmap (24 bits = 24 hours of the day)
} Schedule;							//   0 Allowed to work - 1 Not Allowed

typedef struct tag_WeekSchedule {
	Schedule	day[7];
	int			AttendedPriority;
	int			UnattendedPriority;
} WeekSchedule;

//---------------------------------------------------------
//-- Network Status
//

typedef struct tag_NetworkStatus {
	DWORD		dropped_packets;		//-- Packets dropped due to buffer overflow
	DWORD		bad_packets;			//-- Bad formed packets
	DWORD		tcprequests;			//-- Total number of TCP requests (since boot)
	DWORD		udprequests;			//-- Total number of UDP requests (since boot)
	SYSTEMTIME	boot_time;
	char		reserved[32];
} NetworkStatus;

//---------------------------------------------------------
//-- Station Configuration Block
//

typedef struct tag_ConfigurationBlock {
	DWORD	dwTotalPhys;			//-- GlobalMemoryStatus();
	DWORD	dwNumberOfProcessors;	//-- GetSystemInfo();
	DWORD	dwMajorVersion;			//-- GetVersionEx();	
	DWORD	dwMinorVersion;			//-- GetVersionEx();
	DWORD	dwBuildNumber;			//-- GetVersionEx();
	DWORD	dwPlatformId;			//-- GetVersionEx();
	TCHAR	szCSDVersion[128];		//-- GetVersionEx();
	char	user[MAX_PATH];			//-- GetUserName();
	char	tempdir[MAX_PATH];		//-- ExpandEnvironmentStrings()
	char	name[MAX_PATH];			//-- GetComputerName()
	char	workDisk;				//-- Disk used for Server files (incomming jobs, etc. A = 0, B = 1, etc)
	DWORD	disks;					//-- Available disks (bitmap A=0x1, B=0x2, C=0x4, etc)
	DWORD	diskSpace[26];			//-- Space available on disks in MegaBytes (A=diskSpace[0], B=diskSpace[1], etc.)
	BYTE	mac[8];					//-- Computer NIC address (00:00:00:00:00:00) 6 bytes + 2 padding
	char	reserved[32];			//-- Space to grow
} ConfigurationBlock;

//---------------------------------------------------------
//-- Manager Info
//

#define _MANAGER_INFO_VERSION 400

typedef struct tag_ManagerInfo {
	DWORD				size;			//-- Structure Size ( size = sizeof(ManagerInfo) )
	DWORD				version;
	ConfigurationBlock	cfg;
	NetworkStatus		net_status;
	int			  		servers;		//-- Number of registered servers
	int			  		jobs;			//-- Number of jobs
	char				reserved[32];	//-- Space to grow
} ManagerInfo;

//---------------------------------------------------------
//-- Server Info
//

#define _SERVER_INFO_VERSION 400

typedef struct tag_ServerInfo {
	DWORD				size;			//-- Structure Size ( size = sizeof(ServerInfo) )
	DWORD				version;		
	DWORD				total_frames;	//-- Total number of frames rendered
	float				total_time;		//-- Total time spent rendering (in hours)
	float				index;			//-- Performance index
	ConfigurationBlock	cfg;
	NetworkStatus		net_status;
	char				reserved[32];	//-- Space to grow
} ServerInfo;

//---------------------------------------------------------
//-- Client Info
//

#define _CLIENT_INFO_VERSION 400

typedef struct tag_ClientInfo {
	DWORD				size;			//-- Structure Size ( size = sizeof(ClientInfo) )
	DWORD				version;
	ConfigurationBlock	cfg;
	bool				controller;
	short		  		udp_port;
	char				reserved[32];	//-- Space to grow
} ClientInfo;

//-------------------------------------------------------------------
//-- Global Server State
//

#define SERVER_STATE_ABSENT    0
#define SERVER_STATE_IDLE      1
#define SERVER_STATE_BUSY      2
#define SERVER_STATE_ERROR     3
#define SERVER_STATE_SUSPENDED 4

typedef struct tag_ServerList {
	HSERVER		hServer;
	WORD		state;
	ServerInfo	info;
	//-- Current Task
	HJOB  		hJob;			//-- It will be 0 if no current task is defined
	int			frame;			//-- It will be NO_FRAME if loading job (no frames yet assigned)
	SYSTEMTIME	frame_started;	//-- Time frame was assigned
} ServerList;

//---------------------------------------------------------
//-- Server Statistics
//

typedef struct tag_Statistics {
	float		tseconds;
	int			frames;
} Statistics;

//-------------------------------------------------------------------
//-- Servers in Job Queue -------------------------------------------
//
//   Server Information for a given Job

#define JOB_SRV_IDLE		0		//-- Idle
#define JOB_SRV_BUSY		1		//-- Busy
#define JOB_SRV_FAILED		2		//-- Render Error
#define JOB_SRV_ABSENT		3		//-- Absent
#define JOB_SRV_SUSPENDED	4		//-- Out of work schedule
#define JOB_SRV_BUSYOTHER	5		//-- Busy with another job
#define JOB_SRV_ERROR		6		//-- Connection Error
#define JOB_SRV_COOL_OFF	7		//-- In Error Recovery

typedef struct tagJobServer {
	HSERVER		hServer;			//-- Server Handle
	char	  	status;				//-- JOB_SRV_XXX Status Above
	bool		failed;				//-- Internal Use
	bool		active;				//-- This server is active in the job
	int			cur_frame;			//-- Current Rendering Frame
	float		thours;				//-- Total Hours spent rendering
	int			frames;				//-- Total Number of Frames Rendered
} JobServer;

//-----------------------------------------------------------------------------
//-- Job Frame State

#define NO_FRAME				0x0FFFFFFF

#define FRAME_WAITING   		0
#define FRAME_ASSIGNED  		1
#define FRAME_COMPLETE  		2

typedef struct tagJobFrames {
	char	state;				//-- FRAME_XXX above
	int		frame;				//-- Frame Number
	HSERVER	hServer;			//-- The server rendering this frame
	DWORD	elapsed;			//-- Time it took to render this frame (milliseconds)
} JOBFRAMES;

//-----------------------------------------------------------------------------
//-- MaxNet Class (Exception Handler)
//

class MAXNETEXPORT MaxNet {
	protected:
		int	gerror;
		maxnet_error_t	error;
		maxnet_error_t	TranslateError	(int err);
	public:
						MaxNet			();
		maxnet_error_t	GetError		();
		int				GetGError		();
		const TCHAR*	GetErrorText	();
};

#include <maxnet_platform.h>
#include <maxnet_job.h>
#include <maxnet_file.h>
#include <maxnet_archive.h>
#include <maxnet_manager.h>

//-----------------------------------------------------------------------------
//-- Interface

MAXNETEXPORT MaxNetManager*	CreateManager			( );
MAXNETEXPORT void			DestroyManager			(MaxNetManager* mgr);
//-- Initializes a "Job" structure
MAXNETEXPORT bool			jobReadMAXProperties	(char* max_filename, Job* job, CJobText& jobText);
//-- Reads Render Data from a *.max file and fills in a Job structure
MAXNETEXPORT void			jobSetJobDefaults		(Job* job);
//-- Utilities
MAXNETEXPORT void			NumberedFilename		(TCHAR* infile, TCHAR* outfile, int number);
MAXNETEXPORT bool			IsMacNull				(BYTE *addr);
MAXNETEXPORT bool			GetMacAddress			(BYTE* addr);
MAXNETEXPORT bool			MatchMacAddress			(BYTE* addr1, BYTE* addr2);
MAXNETEXPORT void			Mac2String				(BYTE* addr, TCHAR* string );
MAXNETEXPORT void			Mac2StringCondensed		(BYTE* addr, TCHAR* string );
MAXNETEXPORT void			StringCondensed2Mac		(TCHAR* string, BYTE* addr);
MAXNETEXPORT void			InitConfigurationInfo	(ConfigurationBlock &cb, TCHAR workdisk = 0);
MAXNETEXPORT bool			MatchServers			(HSERVER srv1, HSERVER srv2);
MAXNETEXPORT bool			Maz						(TCHAR* archivename, TCHAR* file_list, DWORD* filesize = 0);
MAXNETEXPORT bool			UnMaz					(TCHAR* archivename, TCHAR* output_path);
//-- Localization Resources
MAXNETEXPORT TCHAR*			ResString				(int id, TCHAR* buffer = 0);
//-- Backburner helper
MAXNETEXPORT bool			ConvertOldJobFile		(TCHAR* oldFile, TCHAR* newFile);


#endif

//-- EOF: maxnet.h ------------------------------------------------------------
