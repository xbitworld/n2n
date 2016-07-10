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

		io_service_.post(
			[this, tmpV]()
		{
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(tmpV);
			//			if (!write_in_progress)
			while (write_in_progress)
			{
				write_in_progress = !write_msgs_.empty();
				ThreadSafeOutput("Write ...... : ");
			}
			{
				do_write();
			}
		});
	}

	void CSocketClient::close()
	{
		io_service_.post([this]() { socket_.close(); });
	}

	void CSocketClient::do_connect(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
	{
		ThreadSafeOutput("Connect\r\n");
		boost::asio::async_connect(socket_, endpoint_iterator,
		[this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
		{
			if (!ec)
			{
				do_read();
			}
			else
			{
				::Sleep(1000);
				ReConnect();
			}
		});
	}

	bool CSocketClient::isSocketOpen()
	{
		return socket_.is_open();
	}

	void CSocketClient::ReConnect()
	{
		ThreadSafeOutput("Reconnect");
		
		if(socket_.is_open())
			close();
			
		boost::asio::async_connect(socket_, end_iterator,
		[this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
		{
			if (!ec)
			{
				do_read();
			}
			else
			{
				::Sleep(1000);
				ReConnect();
			}
		});
	}

	void CSocketClient::do_read()
	{
		char *pStr = xReadData;
		std::memset(pStr, 0, max_length);
		socket_.async_read_some(boost::asio::buffer(pStr, max_length),
			[this, pStr](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				CCharArray dataTemp((const char *)pStr, length);
				pReadData->put(dataTemp);
				do_read();
			}
			else
			{
				::Sleep(1000);
				ReConnect();
			}
		});
	}

	void CSocketClient::do_write()
	{
		boost::asio::async_write(socket_,
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
			else
			{
				::Sleep(1000);
				ReConnect();
			}
		});
	}
}

