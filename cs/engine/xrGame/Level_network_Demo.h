private:
	BOOL						m_DemoPlay;
	BOOL						m_DemoPlayStarted;
	BOOL						m_DemoPlayStoped;
	BOOL						m_DemoSave;
	BOOL						m_DemoSaveStarted;
	u32							m_StartGlobalTime;
	CObject*					m_current_spectator;	//in real, this is CurrentControlEntity 
	message_filter*				m_msg_filter;
public:
	void						SetDemoSpectator		(CObject* spectator);
	inline CObject*				GetDemoSpectator		()	{ return m_current_spectator ? smart_cast<CGameObject*>(m_current_spectator) : NULL; };
	
	void						PrepareToSaveDemo		();
	bool						PrepareToPlayDemo		(shared_str const & file_name);
	
	void						StartPlayDemo			();
	void						StopPlayDemo			();
	void						RestartPlayDemo			();

	float						GetDemoPlayPos				() const;
	//void						SetDemoPlayPos				(float const pos);
	float						GetDemoPlaySpeed			() const;					//Device.time_factor()
	void						SetDemoPlaySpeed			(float const time_factor);	//Device.time_factor(
	message_filter*				GetMessageFilter			();
	


	//virtual	NET_Packet*		net_msg_Retreive		();
	BOOL						IsDemoPlay				()	{return (!m_DemoSave && m_DemoPlay);};
	BOOL						IsDemoSave				()	{return ( m_DemoSave && !m_DemoPlay);};
	inline	BOOL				IsDemoPlayStarted		()	{return (IsDemoPlay() && m_DemoPlayStarted); };
	inline	BOOL				IsDemoPlayFinished		()	{return m_DemoPlayStoped; };
	inline	BOOL				IsDemoSaveStarted		()	{return (IsDemoSave() && m_DemoSaveStarted); };

private:

	void						StartSaveDemo			(shared_str const & server_options);
	void						StopSaveDemo			();

	void						SpawnDemoSpectator		();

	//saving
#pragma pack(push, 1)
	struct	DemoHeader
	{
		string4096				m_server_options;
		u32						m_time_global;
		u32						m_time_server;
		s32						m_time_delta;
		s32						m_time_delta_user;
	};
	struct	DemoPacket
	{
		u32						m_time_global_delta;
		u32						m_timeReceive;
		u32						m_packet_size;
		//here will be body of NET_Packet ...						
	};
#pragma pack(pop)

	void						SaveDemoHeader	(shared_str const & server_options);
	void						SavePacket		(NET_Packet& packet);

	bool						LoadDemoHeader		();
	bool						LoadPacket			(NET_Packet & dest_packet, u32 global_time_delta);
	void						SimulateServerUpdate();
	
	DemoHeader					m_demo_header;
	IWriter*					m_writer;
	CStreamReader*				m_reader;
	

