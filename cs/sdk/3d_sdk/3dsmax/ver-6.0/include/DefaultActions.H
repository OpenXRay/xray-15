//-----------------------------------------------------------------------------
// ----------------
// File ....: DefaultActions.h
// ----------------
// Author...: Larry Minton
// Date ....: June 2003
//
// History .: June, 19 2003 - Started
//
//-----------------------------------------------------------------------------
		
#ifndef DEFAULTACTION_H_DEFINED
#define DEFAULTACTION_H_DEFINED

#define DEFAULTACTIONS_DEFAULT_MSG_LOG_MAXSIZE 500

// Actions - set as bits in a DWORD
#define	DEFAULTACTIONS_LOGTOFILE		0x00000001
#define	DEFAULTACTIONS_LOGMSG			0x00000002
#define	DEFAULTACTIONS_ABORT			0x00000004
#define	DEFAULTACTIONS_RESERVED_4		0x00000008
#define	DEFAULTACTIONS_RESERVED_5		0x00000010
#define	DEFAULTACTIONS_RESERVED_6		0x00000020
#define	DEFAULTACTIONS_RESERVED_7		0x00000040
#define	DEFAULTACTIONS_RESERVED_8		0x00000080

// Event IDs. 0 through 65535 reserved for internal use.
#define TYPE_MISSING_EXTERNAL_FILES		1
#define TYPE_MISSING_DLL_FILES			2
#define TYPE_MISSING_XREF_FILES			3
#define TYPE_MISSING_UVWS				4


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//

class DefaultActionSys {

 public:

	/*	Provides the default action to take for a specified event instead of displaying a dialog.  
		Returns TRUE if a default action was specified for the event, false otherwise. 
		The eventID argument is a random value signifying a specific event, with values 0 through 65535 reserved 
		for internal use. 
		If a default action was specified for the event it is returned through argument action.  
		Bits 0, 1, and 2 are reserved to mean Write to Log File, Write to MsgLog, and Abort, respectively. 
		Bits 3 to 7 are reserved for future internal use
		The remaining bits are event specific. 
		The default return value for events which have not had an action value set is 1 (Write to Log, No Abort). 
	*/
	virtual	BOOL	GetAction (DWORD eventID, DWORD &action) = 0;

	/*	Sets the default action and descriptive title for the event. 
		Returns TRUE if a default action was previously specified for the event, and if oldAction is not NULL the 
		old action value is stored to it.
	*/
	virtual	BOOL	SetAction ( DWORD eventID, DWORD action, TCHAR * = NULL, DWORD *oldAction = NULL) = 0;

	// Deletes the event. Returns TRUE of eventID was located and deleted
	virtual	BOOL	DeleteAction (DWORD eventID) = 0;

	// Returns the number of event IDs registered.
	virtual	int		GetActionCount() = 0;

	// Returns the event’s title, null string if eventID not found
	virtual	TSTR	GetActionTitle(DWORD eventID) = 0;

	// Returns the indexed event’s title.
	virtual	TSTR	GetActionTitleByIndex(int index) = 0;

	// Returns the indexed event’s ID.
	virtual	DWORD	GetActionIDByIndex(int index) = 0;

	// Logs the event ID and message. Returns TRUE if successfully logged.
	virtual BOOL	LogEntry(DWORD eventID, TCHAR *message) = 0;

	// Clears the logged messages. 
	// If eventID is 0, all messages are deleted, otherwise only messages for the specific event ID are deleted.
	virtual	void	MsgLogClear(DWORD eventID) = 0;

	// Returns the number of logged messages. 
	// If eventID is 0, all messages are counted, otherwise only messages for the specific event ID are counted.
	virtual	int		GetMsgLogCount(DWORD eventID) = 0;

	// Returns the specified logged message. 
	// If eventID is 0, all messages are counted, otherwise only messages for the specific event ID are counted.
	virtual	TSTR	GetMsgLogMsg(DWORD eventID, int index) = 0;

	// Returns the event ID of specified logged message.
	virtual	DWORD	GetMsgLogID(int index) = 0;

	// Sets the maximum number of messages that can be stored. Returns the previous limit. 
	// If eventID == -1, the count is not changed, and the current limit is returned.
	virtual	int		SetMsgLogMaxCount(int count) = 0;
};

#endif

//-- EOF: DefaultActions.h ---------------------------------------------------------------
