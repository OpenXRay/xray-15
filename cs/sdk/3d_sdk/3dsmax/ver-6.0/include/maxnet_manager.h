//-----------------------------------------------------------------------------
// ---------------------------
// File ....: maxnet_manager.h
// ---------------------------
// Author...: Gus J Grubba
// Date ....: February 2000
// O.S. ....: Windows 2000
//
// History .: Feb, 15 2000 - Created
//
// 3D Studio Max Network Rendering Classes
// 
//-----------------------------------------------------------------------------

#ifndef _MAXNET_MANAGER_H_
#define _MAXNET_MANAGER_H_

//-------------------------------------------------------------------
//-- Global Server State
//

#define JOB_STATE_COMPLETE	0
#define JOB_STATE_WAITING	1
#define JOB_STATE_BUSY		2
#define JOB_STATE_ERROR		3
#define JOB_STATE_SUSPENDED	4

typedef struct tag_JobList {
	Job			job;
	HJOB		hJob;
	WORD		state;
} JobList;

//-----------------------------------------------------------------------------
//-- MaxNetCallBack
//
//	Note: Return as soon as possible from these calls. They block the API thread
//  and nothing will happen until you return. If you have to do some processing,
//  post a message to your own code and return immediately. Also note that these
//  calls may come from a separate thread than your main process thread. 
//

class MAXNETEXPORT MaxNetCallBack {
	public:
		//-- Return "true" to cancel, "false" to continue
		virtual bool Progress		(int total, int current){return false;}
		virtual void ProgressMsg	(const TCHAR *message){;}
		//-- Notifies the Manager Went Down
		virtual void ManagerDown	( ){;}
		//-- Notifies something has changed (new job, new server, new frame, etc.)
		virtual void Update			( ){;}
		//-- Notifies someone wants control of the queue; Send grant control msg to manager;
		virtual void QueryControl	( TCHAR* station ){;}
		//-- Notifies someone has taken control of the queue (Another QueueManager for instance)
		virtual void QueueControl	( ){;}
};

//-----------------------------------------------------------------------------
//-- Manager Session Class
//

class MAXNETEXPORT MaxNetManager : public MaxNet {

	public:

		//-- Optional Call Back
		
		virtual void	SetCallBack				( MaxNetCallBack* cb )=0;

		//-- Session

		virtual bool	FindManager				( short port, char* manager, char* netmask = "255.255.255.0" )=0;
		virtual void	Connect					( short port, char* manager = NULL, bool enable_callback = false )=0;
		virtual void	Disconnect				( )=0;
		virtual void	GetManagerInfo			( ManagerInfo* info )=0;
		virtual bool	KillManager				( )=0;
		virtual void	EnableUpdate			( bool enable = true )=0;
		
		virtual bool	QueryManagerControl		( bool wait )=0;
		virtual bool	TakeManagerControl		( )=0;
		virtual void	GrantManagerControl		( bool grant )=0;
		virtual bool	LockControl				( bool lock  )=0;
		virtual int		GetClientCount			( )=0;
		virtual int		ListClients				( int start, int end, ClientInfo* clientList )=0;

		//-- Jobs
		
		virtual int		GetJobCount				( )=0;
		virtual int		ListJobs				( int start, int end, JobList* jobList )=0;
		virtual void	GetJob					( HJOB hJob, JobList* jobList )=0;
		virtual void	GetJob					( HJOB hJob, Job* job )=0;
		virtual void	GetJobText				( HJOB hJob, CJobText& jobText, int count )=0;
		virtual void	SetJob					( HJOB hJob, Job* job, CJobText& jobText, bool reset )=0;
		virtual int		GetJobPriority			( HJOB hJob )=0;
		virtual bool	SetJobPriority			( HJOB hJob, int priority )=0;
		virtual void	SetJobOrder				( HJOB* hJob, DWORD count )=0;
		virtual void	DeleteJob				( HJOB hJob )=0;
		virtual void	SuspendJob				( HJOB hJob )=0;
		virtual void	ActivateJob				( HJOB hJob )=0;
		virtual int		GetJobServersCount		( HJOB hJob )=0;
		virtual int		GetJobServers			( int start, int end, HJOB hJob, JobServer* servers )=0;
		virtual void	GetJobServerStatus		( HJOB hJob, HSERVER hServer, TCHAR* status_text )=0;
		virtual void	SuspendJobServer		( HJOB hJob, HSERVER hServer )=0;
		virtual void	AssignJobServer			( HJOB hJob, HSERVER hServer )=0;
		virtual int		GetJobFramesCount		( HJOB hJob )=0;
		virtual int		GetJobFrames			( int start, int end, HJOB hJob, JOBFRAMES* frames )=0;
		virtual int		GetJobLog				( int start, int count, HJOB hJob, TCHAR** buffer )=0;
		
		virtual bool	CheckOutputVisibility	( TCHAR* output, TCHAR* err )=0;
		virtual void	AssignJob				( Job* job, TCHAR* archive, HSERVER* servers, CJobText& jobtext, DWORD blocksize = 0 )=0;
		

		//-- Servers (Global)

		virtual int		GetServerCount			( )=0;
		virtual int		ListServers				( int start, int end, ServerList* serverList )=0;
		virtual void	GetServer				( HSERVER hServer, ServerList* serverList )=0;
		virtual bool	DeleteServer			( HSERVER hServer )=0;
		virtual bool	ResetServerIndex		( HSERVER hServer )=0;
		virtual void	GetWeekSchedule			( HSERVER hServer, WeekSchedule* schedule )=0;
		virtual void	SetWeekSchedule			( HSERVER hServer, WeekSchedule* schedule )=0;
		virtual void	GetServerNetStat		( HSERVER hServer, NetworkStatus* net_stat )=0;
		
		virtual int		GetServerGroupCount		( )=0;
		virtual int		GetServerGroupXCount	( int group )=0;
		virtual int		GetServerGroup			( int group, int count, HSERVER* grplist, TCHAR* name )=0;
		virtual void	NewServerGroup			( int count, HSERVER* grplist, TCHAR* name )=0;
		virtual void	DeleteServerGroup		( int group )=0;
		

};

#endif

//-- EOF: maxnet_manager.h ----------------------------------------------------
