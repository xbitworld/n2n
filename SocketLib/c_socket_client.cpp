#include "c_socket_client.h"
#include "iostream"

namespace dtCSC
{
	void CSocketClient::write(const char *pData, int iLen)
	{
		std::vector<unsigned char> tmpV;

		for (int i = 0; i < iLen; i++)
		{
			tmpV.push_back(*(pData + i));
		}

		if (io_service_.stopped())
		{
			return;
		}

		io_service_.post(
			[this, tmpV]()
		{
			bool write_in_progress = !write_msgs_.empty();
			if (write_in_progress)
			{
				ThreadSafeOutput("write_msgs_ not empty!");
			}
			write_msgs_.push_back(tmpV);
			do_write();
		});
	}

	void CSocketClient::Close()
	{
		io_service_.post([this](){
			socket_->close();
		});
	}

	void CSocketClient::do_connect(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		ThreadSafeOutput("Connecting\r\n");
		boost::asio::async_connect(*socket_, endpoint_iterator,
		[this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
		{
			if (!ec)
			{
				ThreadSafeOutput("Connected\r\n");
				do_read();
			}
			else
			{
				ThreadSafeOutput("Connect failed " + ec.message());
				ReConnect();
			}
		});
	}

	bool CSocketClient::isSocketOpen()
	{
		return socket_->is_open();
	}

	void CSocketClient::ReConnect()
	{
		ThreadSafeOutput("Reconnect");
		
		if (socket_->is_open())
		{
			socket_->close();
		}
			
		boost::asio::async_connect(*socket_, end_iterator,
		[this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
		{
			if (!ec)
			{
				do_read();
			}
		});
	}

	void CSocketClient::do_read()
	{
		char *pStr = xReadData;
		std::memset(pStr, 0, max_length);
		socket_->async_read_some(boost::asio::buffer(pStr, max_length),
			[this, pStr](boost::system::error_code ec, std::size_t length)
		{
			//printf_s("Socket: %0X\n", this);

			if (!ec)
			{
				CCharArray dataTemp(serverHash, (const char *)pStr, length);
				pReadData->put(dataTemp);

				do_read();
			}
			else
			{
				printf_s("Read Exception: %s\n", ec.message().c_str());
			}
		});
	}

	void CSocketClient::do_write()
	{
		boost::asio::async_write(*socket_,
		boost::asio::buffer(write_msgs_.front().data(),
		write_msgs_.front().size()),
		[this](boost::system::error_code ec, std::size_t )
		{
			if (!ec)
			{
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			//else
			//{
			//	ThreadSafeOutput("Write Exception: " + ec.message());
			//}
		});
	}

	size_t CSocketClient::getServerHash()
	{
		return serverHash;
	}

}

