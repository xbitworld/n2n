#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <memory>
#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <thread>
#include <iomanip> 

#include "../CTMutexSet.h"
#include "N2U_Server.h"
#include "../ASIOLib/SerialRW.h"
#include "../HEXSTRTable.h"

std::string strTMP;

using boost::asio::ip::tcp;

boost::mutex io_mutex;	//mutex for console display

ClassMutexList<CCharArray> Net2SerialBuffer;		//Buffer for send to serial
ClassMutexList<CCharArray> Serial2NetBuffer;		//Buffer for send to socket

//Insert data to list, and wait for pop
void PushListData(ClassMutexList<CCharArray> &buf, const char *pData, int iLen)
{
	CCharArray tempData(pData, iLen);
	buf.put(tempData);
}

static void ThreadSafeOutput(const std::string &info)
{
	boost::mutex::scoped_lock	lock(io_mutex);
	std::cout << info << std::endl;
}

void DisplayHEX(const char * headString, const char *pData, const int iLength)
{//Display the Data
	char * pDataStr = new char[2 * iLength + 1];

	for (int iLoop = 0; iLoop < iLength; iLoop++)
	{
		unsigned char charTMP = pData[iLoop];
		unsigned int charPos = charTMP;
		pDataStr[iLoop * 2] = HEX2STRTable[charPos][0];
		pDataStr[iLoop * 2 + 1] = HEX2STRTable[charPos][1];
	}
	pDataStr[iLength * 2] = '\0';

	ThreadSafeOutput(std::string(headString) + std::string(pDataStr));
}

static void InputCMD(void)
{
	char line[65535 + 1];

	ThreadSafeOutput("Read for Input CMD");
	while (std::cin.getline(line, 65535 + 1))
	{
		std::string msg(line);
		if (msg == "echo")
		{
			ThreadSafeOutput("Echo");
		}
		else if (msg == "exit")
		{
			exit(0);
		}

		msg += "\r\n";
//		PushListData(Net2SerialBuffer, msg.c_str(), msg.size());
		PushListData(Serial2NetBuffer, msg.c_str(), msg.size());
	}
}

class SocketSession
	: public std::enable_shared_from_this<SocketSession>
{
public:
	SocketSession(tcp::socket socket)
		: socket_(std::move(socket))
	{
	}

	void start()
	{
		//do_write("Net Start\r\n", 11);
		do_read();
	}

	void Close()
	{
		socket_.cancel();
		socket_.close();
		ThreadSafeOutput("Disconnected");
	}

	void do_write(const char *pData, std::size_t length)
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(pData, length),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if ((ec.value() == boost::asio::error::eof) || (ec.value() == boost::asio::error::connection_reset))
			{
				char strVal[200] = { 0 };
				sprintf_s(strVal, "Read Error, Code: %d, Message: ", ec.value());
				ThreadSafeOutput(strVal + ec.message());
				self->Close();
			}
		});
	}

private:
	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				Net2SerialBuffer.put(CCharArray(data_, length));
				do_read();
			}
			else if((ec.value() == boost::asio::error::eof) || (ec.value() == boost::asio::error::connection_reset))
			{
				char strVal[200] = { 0 };
				sprintf_s(strVal, "Read Error, Code: %d, Message: ", ec.value());
				ThreadSafeOutput(strVal + ec.message());
				self->Close();
			}
		});
	}

	tcp::socket socket_;
	enum { max_length = 4096 };
	char data_[max_length];
};

class NetServer
{
public:
	NetServer(boost::asio::io_service& io_service, tcp::endpoint endpoint)
		: acceptor_(io_service, endpoint),
		socket_(io_service)
	{
		do_accept();
	}

	void write(const char *pData, std::size_t length)
	{
		_pSocketSession->do_write(pData, length);
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				ThreadSafeOutput("Accept");
				_pSocketSession = std::make_shared<SocketSession>(std::move(socket_));
				_pSocketSession->start();

				std::string notifyConn = "Connect&&The@@Net^^Work";
				Net2SerialBuffer.put(CCharArray(notifyConn.c_str(), notifyConn.length()));
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
	std::shared_ptr<SocketSession> _pSocketSession;
};

//Get data from Serial Port, then the function be calledback
static int getSerialData(const std::vector<unsigned char> &SerialData, int iLen)
{
	CCharArray tmp = CCharArray(SerialData, iLen);
	//DisplayHEX((const char *)("Serial: "), tmp.getPtr(), iLen);

	Serial2NetBuffer.put(tmp);
	//ThreadSafeOutput(std::string("Serial Data \r\n"));

	std::time_t t = std::time(NULL);
	struct tm now;
	char mbstr[100];
	::localtime_s(&now, &t);
	std::strftime(mbstr, sizeof(mbstr), "%T R: ", &now);

	char strLen[20] = { 0 };
	::_itoa_s(tmp.getLength(), strLen, 10);

	strTMP = std::string(mbstr) + std::string(strLen);
	ThreadSafeOutput(strTMP.c_str());

	return 0;
}

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 3)
		{
			std::cerr << "Usage: DelivData <host> <port>\n";
			return 1;
		}
		boost::asio::io_service io_service;
		strTMP = std::string("Serial: " + std::string(argv[1]) + ", Address: " + std::string(argv[2]) + ", Port: " + argv[3]);
		ThreadSafeOutput(strTMP.c_str());

		const boost::shared_ptr<SerialRW> sp(new SerialRW(getSerialData, argv[1], 512000));  // for shared_from_this() to work inside of Reader, Reader must already be managed by a smart pointer

		std::thread readCOMThread([&sp](){
			ASIOLib::Executor e;
			e.OnWorkerThreadError = [](boost::asio::io_service &, boost::system::error_code ec) { ThreadSafeOutput(std::string("SerialRW Read error (asio): ") + boost::lexical_cast<std::string>(ec)); };
			e.OnWorkerThreadException = [](boost::asio::io_service &, const std::exception &ex) { ThreadSafeOutput(std::string("SerialRW Read exception (asio): ") + ex.what()); };

			e.OnRun = boost::bind(&SerialRW::Create, sp, _1);
			e.Run(1);
		});

		std::thread writeCOMThread([&sp]() {
			while (true)
			{
				CCharArray data = Net2SerialBuffer.get_pop();
				sp->Write2Serial((unsigned char *)(data.getPtr()), data.getLength());

				std::time_t t = std::time(NULL);
				struct tm now;
				char mbstr[100];
				::localtime_s(&now, &t);
				std::strftime(mbstr, sizeof(mbstr), "%T W: ", &now);

				char strLen[20] = { 0 };
				::_itoa_s(data.getLength(), strLen, 10);

				strTMP = std::string(mbstr) + std::string(strLen);
				ThreadSafeOutput(strTMP.c_str());
			}
		});

		boost::asio::ip::address address = boost::asio::ip::address::from_string(argv[2]);
		tcp::endpoint endpoint(address, atoi(argv[3]));

		NetServer SocketServer(io_service, endpoint);

		std::thread socketReadThread([&io_service]() { io_service.run(); });
		std::thread socketWriteThread([&SocketServer]() {
			while (true)
			{
				CCharArray data = Serial2NetBuffer.get_pop();
				SocketServer.write(data.getPtr(), data.getLength());
			}
		});

		std::thread InputCMDThread(InputCMD);

		readCOMThread.join();
		writeCOMThread.join();
		socketReadThread.join();
		socketWriteThread.join();

		InputCMDThread.join();
	}
	catch (std::exception& e)
	{
		strTMP = std::string("Exception: " + std::string(e.what()));
		ThreadSafeOutput(strTMP.c_str());
	}

	return 0;
}
