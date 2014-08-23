#ifndef FILETRANSFER
#define FILETRANSFER

#include "associative_vector.h"
#include "..\xrEngine\StatGraph.h"

//this module is an implementation of file transfering ...
namespace file_transfer
{

u32 const	data_max_chunk_size = 4096;	//4Kb on update ~	80		Kb/sec
u32 const	data_min_chunk_size = 128;	//					2.5		Kb/sec

enum sending_status_t			//state for callback
{
	sending_data				= 0x00,
	sending_aborted_by_user		= 0x01,
	sending_rejected_by_peer	= 0x02,
	sending_complete			= 0x03
};
enum receiving_status_t			//state for callback
{
	receiving_data				= 0x00,
	receiving_aborted_by_peer	= 0x01,	
	receiving_aborted_by_user	= 0x02,
	receiving_timeout			= 0x03,
	receiving_complete			= 0x04
};

enum ft_command_t		//command byte to M_FILE_TRANSFER message ...
{
	receive_data		=	0x00,	//means that packet contain new chunk of data
	abort_receive		=	0x01,	//this command send by source site, if he aborts file receiving ..
	receive_rejected	=	0x02	//this command send by dest site, if he doesn't want file..
};

typedef fastdelegate::FastDelegate3<sending_status_t, u32, u32> sending_state_callback_t;
typedef fastdelegate::FastDelegate3<receiving_status_t, u32, u32> receiving_state_callback_t;

void make_reject_packet(NET_Packet& packet, ClientID const & client);
void make_abort_packet(NET_Packet& packet, ClientID const & client);

class filetransfer_node
{
private:
	shared_str			m_file_name;
	u32 				m_chunk_size;
	u32					m_last_peak_throughput;
	u32					m_last_chunksize_update_time;
	u32					m_user_param;
	IReader*			m_reader;
	bool	const		m_is_reader_memory;	//if m_reader in memory (not a physical file)
	
	CMemoryWriter*		m_writer_as_src;
	u32					m_writer_pointer;	// to read ..
	u32		const		m_writer_max_size;

	sending_state_callback_t	m_process_callback;
public:
	filetransfer_node	(shared_str const & file_name, u32 const chunk_size, sending_state_callback_t const & callback);
	filetransfer_node	(u8* data, u32 const data_size, u32 const chunk_size, sending_state_callback_t const & callback, u32 user_param = 0);
	filetransfer_node	(CMemoryWriter* m_src_writer, u32 const max_size, u32 const chunk_size, sending_state_callback_t const & callback, u32 user_param = 0);
	filetransfer_node& operator=(filetransfer_node const & copy) {NODEFAULT;};
	~filetransfer_node	();

	void calculate_chunk_size	(u32 peak_throughput, u32 current_throughput);
	bool make_data_packet		(NET_Packet & packet);	//returns true if this is a last packet ...
	void signal_callback		(sending_status_t status);
	bool is_complete			();
	bool is_ready_to_send		();
	
	//inline	shared_str const &	get_file_name	() { return m_file_name; };
	inline	IReader*			get_reader		() { return m_reader; };
	inline	u32 const			get_chunk_size	() const { return m_chunk_size; };
};


class filereceiver_node
{
private:
	shared_str					m_file_name;
	u32							m_data_size_to_receive;
	u32							m_user_param;
	IWriter*					m_writer;
	bool	const				m_is_writer_memory;	//if true then IWriter is a CMemoryWriter ...
	receiving_state_callback_t	m_process_callback;
	u32							m_last_read_time;
public:
	filereceiver_node	(shared_str const & file_name, receiving_state_callback_t const & callback);
	filereceiver_node	(CMemoryWriter* mem_writer, receiving_state_callback_t const & callback);
	filereceiver_node&	operator=(filereceiver_node const & copy) {NODEFAULT;};
	~filereceiver_node	();

	bool	receive_packet		(NET_Packet & packet);	//returns true if receiving is complete
	bool	is_complete			();
	void	signal_callback		(receiving_status_t status);
	
	inline	u32	const			get_downloaded_size	() { return m_writer->tell(); };
	//inline	shared_str const &	get_file_name		() { return m_file_name; };
	inline	u32 const			get_last_read_time	() { return m_last_read_time; };
	inline	void				set_last_read_time	(u32 const read_time) { m_last_read_time = read_time; };
	inline	IWriter*			get_writer			() { return m_writer; };
	inline	u32					get_user_param		() { return m_user_param; };
};

class server_site
{
	typedef std::pair<ClientID, ClientID> dst_src_pair_t;
	typedef associative_vector<dst_src_pair_t, filetransfer_node*> transfer_sessions_t;
	typedef associative_vector<ClientID, filereceiver_node*> receiving_sessions_t;

	transfer_sessions_t		m_transfers;
	receiving_sessions_t	m_receivers;
	void stop_transfer_sessions		(buffer_vector<dst_src_pair_t> const & tsessions);
	void stop_receiving_sessions	(buffer_vector<ClientID> const & tsessions);
public:
	server_site						();
	~server_site					();

	void update_transfer			();
	void stop_obsolete_receivers	();
	void on_message					(NET_Packet* packet, ClientID const & sender);

	void start_transfer_file		(shared_str const & file_name, 
										ClientID const & to_client,
										ClientID const & from_client,
										sending_state_callback_t & tstate_callback);
	void start_transfer_file		(CMemoryWriter& mem_writer,
										u32 mem_writer_max_size,
										ClientID const & to_client,
										ClientID const & from_client,
										sending_state_callback_t & tstate_callback,
										u32 const user_param);
	void stop_transfer_file			(dst_src_pair_t const & tofrom);

	
	filereceiver_node* start_receive_file	(shared_str const & file_name,
										ClientID const & from_client,
										receiving_state_callback_t & rstate_callback);
	filereceiver_node* start_receive_file	(CMemoryWriter& mem_writer,
										ClientID const & from_client,
										receiving_state_callback_t & rstate_callback);
	void stop_receive_file			(ClientID const & from_client);

	bool is_transfer_active			(ClientID const & to_client, ClientID const & from_client) const;
	bool is_receiving_active		(ClientID const & from_client) const;
};


class client_site
{
	filetransfer_node*		m_transfering;
	typedef associative_vector<ClientID, filereceiver_node*> receiving_sessions_t;
	receiving_sessions_t	m_receivers;
	void stop_receiving_sessions(buffer_vector<ClientID> const & tsessions);
#ifdef DEBUG
	CStatGraph*				m_stat_graph;
	void dbg_init_statgraph		();
	void dbg_update_statgraph	();
	void dbg_deinit_statgraph	();
#endif
public:
	client_site				();
	~client_site			();
	
	void update_transfer		();
	void stop_obsolete_receivers();
	void on_message				(NET_Packet* packet);
	
	void start_transfer_file	(shared_str const & file_name,
											sending_state_callback_t & tstate_callback);
	void start_transfer_file	(u8* data, u32 size,
											sending_state_callback_t & tstate_callback, u32 size_to_allocate = 0);
	void stop_transfer_file		();

	
	filereceiver_node* start_receive_file	(shared_str const & file_name,
											ClientID const & from_client,
											receiving_state_callback_t & rstate_callback);
	filereceiver_node* start_receive_file	(CMemoryWriter& mem_writer,
											ClientID const & from_client,
											receiving_state_callback_t & rstate_callback);
	void stop_receive_file		(ClientID const & from_client);

	inline bool is_transfer_active	() const { return m_transfering ? true : false; };
	inline bool is_receiving_active	(ClientID const & from_client) const { return m_receivers.find(from_client) != m_receivers.end(); };
};

}; //namespace file_transfer

#endif //#ifndef FILETRANSFER