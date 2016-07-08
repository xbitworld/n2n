#ifndef CSOCKETCLIENT_H
#define CSOCKETCLIENT_H
#include <boost/asio.hpp>
#include <deque>
#include <atomic>
#include "../CTMutexSet.h"

namespace dtCSC
{

	enum { max_length = 4096 };

	typedef std::deque<std::vector<unsigned char>> MessageQueue;

	class CSocketClient
	{
	public:
		CSocketClient(boost::asio::io_service& io_service,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		ClassMutexList<CCharArray> &pBuff)
		: io_service_(io_service)
		, socket_(io_service)
		{
			pReadData = &pBuff;
			end_iterator = endpoint_iterator;
			do_connect(endpoint_iterator);
		}

		void close();

		~CSocketClient()
		{
			close();
		}

		void write(const std::string& msg);
		void CSocketClient::write(const char *pData, int iLen);
		
		bool isSocketOpen();

		void ReConnect();

	private:
		void do_connect(boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

		void do_read();

		void do_write();

	private:
		boost::asio::io_service& io_service_;
		boost::asio::ip::tcp::socket socket_;
		char xReadData[max_length];
		MessageQueue write_msgs_;
		ClassMutexList<CCharArray> *pReadData;
		boost::asio::ip::tcp::resolver::iterator end_iterator;
		boost::asio::streambuf streamBUF;
	};

}//namespace dtCSC

#endif // CSOCKETCLIENT_H
