#include "stdafx.h"
#include "xrMessages.h"
#include "file_transfer.h"
#include "Level.h"
#include "xrServer.h"
#include "hudmanager.h"

#define MAX_FT_WAIT_TIME (2000 * 3)	/*3 max pings*/
#define MAX_START_WAIT_TIME (2000 * 14) /* 10 max pings*/

using namespace file_transfer;

filetransfer_node::filetransfer_node(shared_str const & file_name, 
									 u32 const chunk_size, 
									 sending_state_callback_t const & callback) :
	m_writer_as_src(NULL),
	m_file_name(file_name),
	m_is_reader_memory(false),
	m_writer_max_size(0),
	m_chunk_size(chunk_size),
	m_last_peak_throughput(0),
	m_last_chunksize_update_time(0),
	m_user_param(0),
	m_process_callback(callback)
{
	m_reader = FS.r_open(file_name.c_str());
}

filetransfer_node::filetransfer_node(u8* data,
									 u32 const data_size,
									 u32 const chunk_size,
									 sending_state_callback_t const & callback,
									 u32 user_param) :
	m_writer_as_src(NULL),
	m_is_reader_memory(true),
	m_writer_max_size(0),
	m_chunk_size(chunk_size),
	m_last_peak_throughput(0),
	m_last_chunksize_update_time(0),
	m_user_param(user_param),
	m_process_callback(callback)
{
	m_reader = xr_new<IReader>(static_cast<void*>(data), static_cast<int>(data_size));
}
filetransfer_node::filetransfer_node(CMemoryWriter* m_src_writer,
									 u32 const max_size,
									 u32 const chunk_size,
									 sending_state_callback_t const & callback,
									 u32 user_param) :
	m_reader(NULL),
	m_writer_as_src(m_src_writer),
	m_writer_max_size(max_size),
	m_is_reader_memory(true),
	m_chunk_size(chunk_size),
	m_last_peak_throughput(0),
	m_last_chunksize_update_time(0),
	m_user_param(user_param),
	m_process_callback(callback)
{
	m_writer_pointer = 0;
}

filetransfer_node::~filetransfer_node()
{
	if (m_reader)
	{
		if(m_is_reader_memory)
		{
			xr_delete(m_reader);
		} else
		{
			FS.r_close(m_reader);
		}
	}
}

void filetransfer_node::calculate_chunk_size(u32 peak_throughput, u32 current_throughput)
{
	if ((Device.dwTimeGlobal - m_last_chunksize_update_time) < 1000)
		return;

	if (m_last_peak_throughput < peak_throughput)		//peak throughput is increasing, so we can increase upload size :)
	{
		m_chunk_size += data_min_chunk_size;
#ifdef MP_LOGGING
		Msg("* peak throughout is not reached - increasing upload rate : (m_chunk_size: %d)", m_chunk_size);
#endif
	} else //peak is reached
	{
		if (OnServer())
		{
			m_chunk_size = data_max_chunk_size;
			return;
		}
		if ((Device.dwTimeGlobal - m_last_chunksize_update_time) < 3000)
			return;

		m_chunk_size = static_cast<u32>(
			Random.randI(data_min_chunk_size, data_max_chunk_size));
#ifdef MP_LOGGING
		Msg("* peak throughout is reached, (current_throughput: %d), (peak_throughput: %d), (m_chunk_size: %d)",
			current_throughput, peak_throughput, m_chunk_size);
#endif
	}
	clamp(m_chunk_size, data_min_chunk_size, data_max_chunk_size);
	m_last_peak_throughput			= peak_throughput;
	m_last_chunksize_update_time	= Device.dwTimeGlobal;
}

bool filetransfer_node::make_data_packet(NET_Packet & packet)
{
	bool finished = false;
	void* pointer;
	u32 size_to_write;
	if (m_reader)
	{
		size_to_write = (static_cast<u32>(m_reader->elapsed()) >= m_chunk_size) ? 
			m_chunk_size : m_reader->elapsed();
		if (size_to_write)
		{
			pointer = _alloca(size_to_write);
				
			R_ASSERT(size_to_write < (NET_PacketSizeLimit - packet.w_tell()));

			if (!m_reader->tell())
			{
				packet.w_u32(m_reader->length());
				packet.w_u32(m_user_param);
			}
			m_reader->r(pointer, size_to_write);
			packet.w(pointer, size_to_write);
		}
		finished = m_reader->eof() ? true : false;
	} else
	{
		u32 elapsed = m_writer_as_src->size() - m_writer_pointer;
		size_to_write = (elapsed >= m_chunk_size) ? m_chunk_size : elapsed;
		pointer = m_writer_as_src->pointer() + m_writer_pointer;
		
		R_ASSERT(size_to_write < (NET_PacketSizeLimit - packet.w_tell()));

		if (!m_writer_pointer)
		{
			packet.w_u32(m_writer_max_size);
			packet.w_u32(m_user_param);
		}

		m_writer_pointer += size_to_write;
		packet.w(pointer, size_to_write);
		finished = (m_writer_pointer == m_writer_max_size);
	}
	return finished;
}

void filetransfer_node::signal_callback(sending_status_t status)
{
	if (m_reader)
	{
		m_process_callback(status, m_reader->tell(), m_reader->length());
	} else
	{
		m_process_callback(status, m_writer_pointer, m_writer_max_size);
	}
}

bool filetransfer_node::is_complete()
{
	if (m_reader)
		return m_reader->eof() ? true : false;
	if (m_writer_as_src)
		return (m_writer_pointer == m_writer_max_size);

	return false;
}

bool filetransfer_node::is_ready_to_send()
{
	if (is_complete())
		return false;
	
	if (m_writer_as_src)
		return (m_writer_pointer < m_writer_as_src->size());

	return true;
}

filereceiver_node::filereceiver_node(shared_str const & file_name,
									 receiving_state_callback_t const & callback) :
	m_file_name(file_name),
	m_is_writer_memory(false),
	m_process_callback(callback),
	m_last_read_time(0)
{
	m_writer = FS.w_open(file_name.c_str());
}

filereceiver_node::filereceiver_node(CMemoryWriter* mem_writer,
									 receiving_state_callback_t const & callback) :
	m_is_writer_memory(true),
	m_process_callback(callback),
	m_last_read_time(0)
{
	m_writer = static_cast<IWriter*>(mem_writer);
}

filereceiver_node::~filereceiver_node()
{
	if (m_writer && !m_is_writer_memory)
		FS.w_close(m_writer);
}

bool filereceiver_node::receive_packet(NET_Packet & packet)
{
	if (!m_writer->tell())
	{
		packet.r_u32(m_data_size_to_receive);
		packet.r_u32(m_user_param);
	}
	u32 size_to_write = packet.B.count - packet.r_tell();
	void* pointer = static_cast<void*>(packet.B.data + packet.r_tell());
	m_writer->w(pointer, size_to_write);
	m_last_read_time = Device.dwTimeGlobal;
	return (m_writer->tell() == m_data_size_to_receive);
}

void filereceiver_node::signal_callback(receiving_status_t status)
{
	m_process_callback(status, m_writer->tell(), m_data_size_to_receive);
}

bool filereceiver_node::is_complete()
{
	if (m_writer)
		return (m_writer->tell() == m_data_size_to_receive);
	return false;
}

void file_transfer::make_reject_packet(NET_Packet& packet, ClientID const & client)
{
	packet.w_begin(M_FILE_TRANSFER);
	packet.w_u8(static_cast<u8>(receive_rejected));
	packet.w_u32(client.value());
}
void file_transfer::make_abort_packet(NET_Packet& packet, ClientID const & client)
{
	packet.w_begin(M_FILE_TRANSFER);
	packet.w_u8(static_cast<u8>(abort_receive));
	packet.w_u32(client.value());
}


server_site::server_site()
{
}

server_site::~server_site()
{
	transfer_sessions_t::iterator ti = m_transfers.begin();
	while (ti != m_transfers.end())
	{
		stop_transfer_file(ti->first);
		ti = m_transfers.begin();
	}
	receiving_sessions_t::iterator ri = m_receivers.begin();
	while (ri != m_receivers.end())
	{
		stop_receive_file(ri->first);
		ri = m_receivers.begin();
	}
}

void server_site::stop_transfer_sessions	(buffer_vector<dst_src_pair_t> const & tsessions) //notifies sending_rejected_by_peer
{
	for (buffer_vector<dst_src_pair_t>::const_iterator i = tsessions.begin(),
		ie = tsessions.end(); i != ie; ++i)
	{
		stop_transfer_file(*i);
	}

}

void server_site::stop_receiving_sessions	(buffer_vector<ClientID> const & rsessions) //notifies receiving_timeout
{
	for (buffer_vector<ClientID>::const_iterator i = rsessions.begin(),
		ie = rsessions.end(); i != ie; ++i)
	{
		stop_receive_file(*i);
	}
}

void server_site::update_transfer()
{
	if (m_transfers.empty())
		return;

	buffer_vector<dst_src_pair_t>	to_stop_transfers(
		_alloca(m_transfers.size() * sizeof(dst_src_pair_t)),
		m_transfers.size());

	for (transfer_sessions_t::iterator ti = m_transfers.begin(),
			tie = m_transfers.end(); ti != tie; ++ti)
	{
		IClient* tmp_client = Level().Server->GetClientByID(ti->first.first);//dst
		if (!tmp_client)
		{
			Msg("! ERROR: SV: client [%u] not found for transfering file", ti->first);
			to_stop_transfers.push_back(ti->first);
			ti->second->signal_callback(sending_rejected_by_peer);
			continue;
		}
		filetransfer_node* tmp_ftnode	= ti->second;
		if (!tmp_ftnode->is_ready_to_send())
		{
			continue;
		}

		tmp_ftnode->calculate_chunk_size(tmp_client->stats.getPeakBPS(), tmp_client->stats.getBPS());
		NET_Packet tmp_packet;
		tmp_packet.w_begin				(M_FILE_TRANSFER);
		tmp_packet.w_u8					(receive_data);
		tmp_packet.w_u32				(ti->first.second.value());	//src
		bool complete = tmp_ftnode->make_data_packet(tmp_packet);
		Level().Server->SendTo			(tmp_client->ID, tmp_packet, net_flags(TRUE,TRUE,TRUE));
		if (complete)
		{
			tmp_ftnode->signal_callback		(sending_complete);
			to_stop_transfers.push_back(ti->first);
		} else
		{
			tmp_ftnode->signal_callback		(sending_data);
		}
	}
	stop_transfer_sessions(to_stop_transfers);
}

void server_site::on_message(NET_Packet* packet, ClientID const & sender)
{
	ft_command_t command = static_cast<ft_command_t>(packet->r_u8());
	switch (command)
	{
	case receive_data:
		{
			receiving_sessions_t::iterator temp_iter = m_receivers.find(sender);
			if (temp_iter == m_receivers.end())
			{
				NET_Packet reject_packet;
				make_reject_packet(reject_packet, ClientID(0));
				Level().Server->SendTo(sender, reject_packet, net_flags(TRUE,TRUE,TRUE));
				break;
			}
			if (temp_iter->second->receive_packet(*packet))
			{
				temp_iter->second->signal_callback(receiving_complete);
				stop_receive_file(temp_iter->first);
			} else
			{
				temp_iter->second->signal_callback(receiving_data);
			}
		}break;
	case abort_receive:
		{
			//ignoring ClientID(0) source client
			receiving_sessions_t::iterator temp_iter = m_receivers.find(sender);
			if (temp_iter != m_receivers.end())
			{
				temp_iter->second->signal_callback(receiving_aborted_by_peer);
				stop_receive_file(sender);
			}
		}break;
	case receive_rejected:
		{
			transfer_sessions_t::iterator temp_iter = m_transfers.find(
				std::make_pair(sender, ClientID(packet->r_u32()))
			);
			if (temp_iter != m_transfers.end())
			{
				temp_iter->second->signal_callback(sending_rejected_by_peer);
				stop_transfer_file(temp_iter->first);
			}
		}break;
	};
}

void server_site::stop_obsolete_receivers()
{
	u32 current_time = Device.dwTimeGlobal;
	buffer_vector<ClientID>	to_stop_receivers(
		_alloca(m_receivers.size() * sizeof(ClientID)),
		m_receivers.size());
	
	for (receiving_sessions_t::iterator i = m_receivers.begin(),
		ie = m_receivers.end(); i != ie; ++i)
	{
		if (!i->second->get_downloaded_size())
		{
			if (!i->second->get_last_read_time())
			{
				i->second->set_last_read_time(current_time);
			} else if ((current_time - i->second->get_last_read_time()) > MAX_START_WAIT_TIME)
			{
				i->second->signal_callback(receiving_timeout);
				to_stop_receivers.push_back(i->first);
			}
		} else
		{
			if ((current_time - i->second->get_last_read_time()) > MAX_FT_WAIT_TIME)
			{
				i->second->signal_callback(receiving_timeout);
				to_stop_receivers.push_back(i->first);
			}
		}
	}
	stop_receiving_sessions(to_stop_receivers);
}

void server_site::start_transfer_file(shared_str const & file_name, 
										ClientID const & to_client,
										ClientID const & from_client,
										sending_state_callback_t & tstate_callback)
{
	if (is_transfer_active(to_client, from_client))
	{
		Msg("! ERROR: SV: transfering file to client [%d] already active.", to_client);
		return;
	}
	filetransfer_node* ftnode = xr_new<filetransfer_node>(
		file_name, 
		data_max_chunk_size,
		tstate_callback);
	dst_src_pair_t tkey = std::make_pair(to_client, from_client);
	m_transfers.insert(std::make_pair(tkey, ftnode));
	if (!ftnode->get_reader())
	{
		Msg("! ERROR: SV: failed to open file [%s]", file_name.c_str());
		stop_transfer_file(tkey);
	}
}

void server_site::start_transfer_file(CMemoryWriter& mem_writer,
										u32 mem_writer_max_size,
										ClientID const & to_client,
										ClientID const & from_client,
										sending_state_callback_t & tstate_callback,
										u32 const user_param)
{
	if (is_transfer_active(to_client, from_client))
	{
		Msg("! ERROR: SV: transfering file to client [%d] already active.", to_client);
		return;
	}
	filetransfer_node* ftnode = xr_new<filetransfer_node>(
		&mem_writer,
		mem_writer_max_size,
		data_max_chunk_size,
		tstate_callback, user_param);
	m_transfers.insert(
		std::make_pair(
			std::make_pair(to_client, from_client),
			ftnode
		)
	);
}

void server_site::stop_transfer_file(dst_src_pair_t const & tkey)
{
	transfer_sessions_t::iterator temp_iter = m_transfers.find(tkey);
	if (temp_iter == m_transfers.end())
	{
		Msg("! ERROR: SV: no file transfer for client [%d] found from client [%d].", 
			tkey.first, tkey.second);
		return;
	}
	if (!temp_iter->second->is_complete())
	{
		NET_Packet abort_packet;
		make_abort_packet(abort_packet, tkey.second);
		if (Level().Server->GetClientByID(tkey.first))
		{
			Level().Server->SendTo(tkey.first, abort_packet, net_flags(TRUE,TRUE,TRUE));
		}
	}
	xr_delete(temp_iter->second);
	m_transfers.erase(temp_iter);
}

filereceiver_node* server_site::start_receive_file(shared_str const & file_name,
										ClientID const & from_client,
										receiving_state_callback_t & rstate_callback)
{
	receiving_sessions_t::iterator temp_iter = m_receivers.find(from_client);
	if (temp_iter != m_receivers.end())
	{
		Msg("! ERROR: SV: file already receiving from client [%d]", from_client);
		return NULL;
	}
	filereceiver_node* frnode = xr_new<filereceiver_node>(file_name, rstate_callback);
	m_receivers.insert(std::make_pair(from_client, frnode));
	if (!frnode->get_writer())
	{
		Msg("! ERROR: SV: failed to create file [%s]", file_name.c_str());
		stop_receive_file(from_client);
		return NULL;
	}
	return frnode;
}

filereceiver_node* server_site::start_receive_file(CMemoryWriter& mem_writer,
										ClientID const & from_client,
										receiving_state_callback_t & rstate_callback)
{
	receiving_sessions_t::iterator temp_iter = m_receivers.find(from_client);
	if (temp_iter != m_receivers.end())
	{
		Msg("! ERROR: SV: file already receiving from client [%d]", from_client);
		return NULL;
	}
	filereceiver_node* frnode = xr_new<filereceiver_node>(&mem_writer, rstate_callback);
	m_receivers.insert(std::make_pair(from_client, frnode));
	return frnode;
}


void server_site::stop_receive_file(ClientID const & from_client)
{
	receiving_sessions_t::iterator temp_iter = m_receivers.find(from_client);
	if (temp_iter == m_receivers.end())
	{
		Msg("! ERROR: SV: no file receiving from client [%u] found", from_client);
		return;
	}
	if (!temp_iter->second->is_complete())
	{
		NET_Packet reject_packet;
		make_reject_packet(reject_packet, ClientID(0));
		Level().Server->SendTo(from_client, reject_packet, net_flags(TRUE,TRUE,TRUE));
	}
	xr_delete(temp_iter->second);
	m_receivers.erase(temp_iter);
}

bool server_site::is_transfer_active(ClientID const & to_client, ClientID const & from_client) const
{
	transfer_sessions_t::const_iterator temp_iter = m_transfers.find(
		std::make_pair(to_client, from_client)
	);
	if (temp_iter == m_transfers.end())
		return false;
	return true;
}
bool server_site::is_receiving_active(ClientID const & from_client) const
{
	receiving_sessions_t::const_iterator temp_iter = m_receivers.find(from_client);
	if (temp_iter == m_receivers.end())
		return false;
	return true;
}


client_site::client_site() : 
	m_transfering(NULL)
{
#ifdef DEBUG
	m_stat_graph = NULL;
#endif
}

client_site::~client_site()
{
	stop_transfer_file();
	receiving_sessions_t::iterator ri = m_receivers.begin();
	while (ri != m_receivers.end())
	{
		stop_receive_file(ri->first);
		ri = m_receivers.begin();
	}
#ifdef DEBUG
	dbg_deinit_statgraph();
#endif
}

void client_site::update_transfer()
{
	if (is_transfer_active() && m_transfering->is_ready_to_send())
	{
		IClientStatistic& peer_stats = Level().GetStatistic();
		m_transfering->calculate_chunk_size	(peer_stats.getPeakBPS(), peer_stats.getBPS());
#ifdef DEBUG
		if (psDeviceFlags.test(rsStatistic))
		{
			dbg_update_statgraph();
		} else
		{
			dbg_deinit_statgraph();
		}
#endif
		NET_Packet tmp_packet;
		tmp_packet.w_begin					(M_FILE_TRANSFER);
		tmp_packet.w_u8						(receive_data);
		bool complete = m_transfering->make_data_packet(tmp_packet);
		Level().Send(tmp_packet, net_flags	(TRUE, TRUE, TRUE));
		if (complete)
		{
			m_transfering->signal_callback	(sending_complete);
			stop_transfer_file				();
		} else
		{
			m_transfering->signal_callback(sending_data);
		}
	}
}
void client_site::on_message(NET_Packet* packet)
{
	ft_command_t command = static_cast<ft_command_t>(packet->r_u8());
	ClientID from_client(packet->r_u32());
	switch (command)
	{
	case receive_data:
		{
			receiving_sessions_t::iterator tmp_iter = m_receivers.find(from_client);
			if (tmp_iter != m_receivers.end())
			{

				if (tmp_iter->second->receive_packet(*packet))
				{
					tmp_iter->second->signal_callback(receiving_complete);
					stop_receive_file(from_client);
				}
				else
					tmp_iter->second->signal_callback(receiving_data);
			} else
			{
				NET_Packet reject_packet;
				make_reject_packet(reject_packet, from_client);
				Level().Send(reject_packet, net_flags(TRUE,TRUE,TRUE));
			}
		}break;
	case abort_receive:
		{
			receiving_sessions_t::iterator tmp_iter = m_receivers.find(from_client);
			if (tmp_iter != m_receivers.end())
			{
				tmp_iter->second->signal_callback(receiving_aborted_by_peer);
				stop_receive_file(from_client);
			} else
			{
				Msg("! WARNING: CL: server sent unknown abort receive message");
			}
		}break;
	case receive_rejected:
		{
			//ignoring from_client u32
			if (is_transfer_active())
			{
				m_transfering->signal_callback(sending_rejected_by_peer);
				stop_transfer_file();
			} else
			{
				Msg("! WARNING: CL: server sent unknown receive reject message"); 
			}
		}break;
	};
}

	
void client_site::start_transfer_file(shared_str const & file_name,
										sending_state_callback_t & tstate_callback)
{
	if (is_transfer_active())
	{
		Msg("! ERROR: CL: transfering file already active.");
		return;
	}
	m_transfering = xr_new<filetransfer_node>(file_name, data_min_chunk_size, tstate_callback);
	if (!m_transfering->get_reader())
	{
		Msg("! ERROR: CL: failed to open file [%s]", file_name.c_str());
		stop_transfer_file();
	}
}
void client_site::start_transfer_file(u8* data, u32 size,
											sending_state_callback_t & tstate_callback,
											u32 size_to_allocate)
{
	if (is_transfer_active())
	{
		Msg("! ERROR: CL: transfering file already active.");
		return;
	}
	if (!size || !data)
	{
		Msg("! ERROR: CL: no data to transfer ...");
		return;
	}
	m_transfering = xr_new<filetransfer_node>(data,
		size,
		data_min_chunk_size,
		tstate_callback,
		size_to_allocate
	);
}

void client_site::stop_transfer_file()
{
	if (!is_transfer_active())
		return;

	if (!m_transfering->is_complete())
	{
		NET_Packet abort_packet;
		make_abort_packet(abort_packet, ClientID(0));
		Level().Send(abort_packet, net_flags(TRUE,TRUE,TRUE));
	}
	xr_delete(m_transfering);
}

filereceiver_node* client_site::start_receive_file(shared_str const & file_name,
										ClientID const & from_client,
										receiving_state_callback_t & rstate_callback)
{
	if (is_receiving_active(from_client))
	{
		Msg("! ERROR: CL: file already receiving from client [%d]", from_client);
		return NULL;
	}
	filereceiver_node* frnode = xr_new<filereceiver_node>(file_name, rstate_callback);
	m_receivers.insert(std::make_pair(from_client, frnode));
	if (!frnode->get_writer())
	{
		Msg("! ERROR: CL: failed to create file [%s]", file_name.c_str());
		stop_receive_file(from_client);
		return NULL;
	}
	return frnode;
}

filereceiver_node* client_site::start_receive_file(CMemoryWriter& mem_writer,
										ClientID const & from_client,
										receiving_state_callback_t & rstate_callback)
{
	if (is_receiving_active(from_client))
	{
		Msg("! ERROR: CL: file already receiving from client [%d]", from_client);
		return NULL;
	}
	mem_writer.clear();
	filereceiver_node* frnode = xr_new<filereceiver_node>(&mem_writer, rstate_callback);
	m_receivers.insert(std::make_pair(from_client, frnode));
	return frnode;
}


void client_site::stop_receive_file(ClientID const & from_client)
{
	receiving_sessions_t::iterator temp_iter = m_receivers.find(from_client);
	if (temp_iter == m_receivers.end())
	{
		Msg("! ERROR: CL: no file receiving from client [%u] found", from_client);
		return;
	}
	if (!temp_iter->second->is_complete())
	{
		NET_Packet reject_packet;
		make_reject_packet(reject_packet, from_client);
		Level().Send(reject_packet, net_flags	(TRUE, TRUE, TRUE));
	}
	xr_delete(temp_iter->second);
	m_receivers.erase(temp_iter);
}

void client_site::stop_receiving_sessions	(buffer_vector<ClientID> const & rsessions)
{
	for (buffer_vector<ClientID>::const_iterator i = rsessions.begin(),
		ie = rsessions.end(); i != ie; ++i)
	{
		stop_receive_file(*i);
	}
}

void client_site::stop_obsolete_receivers()
{
	u32 current_time = Device.dwTimeGlobal;
	buffer_vector<ClientID>	to_stop_receivers(
		_alloca(m_receivers.size() * sizeof(ClientID)),
		m_receivers.size());
	
	for (receiving_sessions_t::iterator i = m_receivers.begin(),
		ie = m_receivers.end(); i != ie; ++i)
	{
		if (!i->second->get_downloaded_size())
		{
			if (!i->second->get_last_read_time())
			{
				i->second->set_last_read_time(current_time);
			} else if ((current_time - i->second->get_last_read_time()) > MAX_START_WAIT_TIME)
			{
				i->second->signal_callback(receiving_timeout);
				to_stop_receivers.push_back(i->first);
			}
		} else
		{
			if ((current_time - i->second->get_last_read_time()) > MAX_FT_WAIT_TIME)
			{
				i->second->signal_callback(receiving_timeout);
				to_stop_receivers.push_back(i->first);
			}
		}
	}
	stop_receiving_sessions(to_stop_receivers);
}

	
#ifdef DEBUG
void client_site::dbg_init_statgraph()
{
	CGameFont* F = HUD().Font().pFontDI;
	F->SetHeightI(0.015f);
	F->OutSet	(360.f, 700.f);
	F->SetColor	(D3DCOLOR_XRGB(0,255,0));
	F->OutNext("%d", (int)data_max_chunk_size);
	F->OutSet	(360.f, 760.f);
	F->OutNext("%d", (int)data_min_chunk_size);
	m_stat_graph = xr_new<CStatGraph>();
	m_stat_graph->SetRect(400, 700, 200, 68, 0xff000000, 0xff000000);
	m_stat_graph->SetMinMax(float(data_min_chunk_size), float(data_max_chunk_size), 1000);
	m_stat_graph->SetStyle(CStatGraph::stBarLine);
	m_stat_graph->AppendSubGraph(CStatGraph::stBarLine);
}
void client_site::dbg_deinit_statgraph()
{
	if (m_stat_graph)
	{
		xr_delete(m_stat_graph);
	}
}

void client_site::dbg_update_statgraph()
{
	if (!m_stat_graph)
	{
		dbg_init_statgraph();
	}
	if (m_transfering)
	{
		m_stat_graph->AppendItem(float(m_transfering->get_chunk_size()), 0xff00ff00, 0);
	}
}
#endif