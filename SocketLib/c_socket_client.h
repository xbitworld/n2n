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
		: public std::enable_shared_from_this<CSocketClient>
	{
	public:
		CSocketClient(size_t Hash,
		boost::asio::io_service& io_service,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		ClassMutexList<CCharArray> &pBuff, void(*pFun)(const std::string &info))
		: serverHash(Hash)
		, io_service_(io_service)
		, socket_(io_service)
		, end_iterator(endpoint_iterator)
		, ThreadSafeOutput(pFun)
		{
			pReadData = &pBuff;
			end_iterator = end_iterator;
			do_connect(end_iterator);
		}

		void Close();

		~CSocketClient()
		{
			//Close();
		}

		void write(const char *pData, int iLen);
		
		bool isSocketOpen();

		void ReConnect();

		size_t getServerHash();

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

		void(*ThreadSafeOutput)(const std::string &info);
		size_t serverHash;
	};

}//namespace dtCSC

#endif // CSOCKETCLIENT_H
