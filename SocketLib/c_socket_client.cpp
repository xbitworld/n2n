#include "c_socket_client.h"
#include "iostream"

namespace dtCSC
{
	void CSocketClient::write(const char *pData, int iLen)
	{
		io_service_.post(
			[this, pData]()
		{
			bool write_in_progress = !write_msgs_.empty();
			//write_msgs_.push_back(pData);
			//			if (!write_in_progress)
			while (write_in_progress)
			{
				write_in_progress = !write_msgs_.empty();
				std::cout << "Write ...... : " << std::endl;
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
		boost::asio::async_connect(socket_, endpoint_iterator,
		[this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
		{
			if (!ec)
			{
				do_read();
			}
		});
	}

	bool CSocketClient::isSocketOpen()
	{
		return socket_.is_open();
	}

	void CSocketClient::ReConnect()
	{
		std::cout << "Reconnect" << std::endl;
		
		if(socket_.is_open())
			close();
			
		boost::asio::async_connect(socket_, end_iterator,
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
		std::string splitStr = "</ipv6testmsg>";
		
		char *pStr = xReadData;
		std::memset(pStr, 0, max_length);
		boost::asio::async_read_until(socket_, streamBUF, splitStr, 
		[this, pStr](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::istream is(&streamBUF);
				int iLen = streamBUF.size();
				is.read(pStr, iLen);

				std::cout << length << ", Data 1: " << pStr << std::endl;
				CCharArray dataTemp((const char *)pStr, length);
				pReadData->put(dataTemp);
				//std::cout << dataTemp.getLength() << ", Data 2: " << pStr << std::endl;
				do_read();
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
				//std::cout << "Now Send Data :" << write_msgs_.front() << std::endl;
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			else
			{
				socket_.close();
			}
		});
	}
}

