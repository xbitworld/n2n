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

std::hash<int> hash_socket;

std::string disconnectFlag("*&DISC&*");
std::string notifyConn("&*CONN*&");

//Insert data to list, and wait for pop
void PushListData(ClassMutexList<CCharArray> &buf, const CCharArray &tempData)
{
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

		//msg += "\r\n";
		//const CCharArray msgA(0, msg.c_str(), msg.size());
		//PushListData(Serial2NetBuffer, msgA);
	}
}

class SocketSession
	: public std::enable_shared_from_this<SocketSession>
{
public:
	SocketSession(boost::asio::io_service& io_service)
		: socket_(io_service)
	{
	}

	void start()
	{
		Net2SerialBuffer.put(CCharArray(hash_socket(socket_.remote_endpoint().port()), notifyConn.c_str(), notifyConn.length()));
		//do_write("Net Start\r\n", 11);
		do_read();
	}

	tcp::socket& getSocket()
	{
		return socket_;
	}

	size_t getSocketHash()
	{
		return hash_socket(socket_.remote_endpoint().port());
	}

	void Close()
	{
		char charTMP[200];

		boost::system::error_code ec;
		boost::asio::ip::address v4address;
		socket_.remote_endpoint(ec).address(v4address);
		size_t socketHash = hash_socket(socket_.remote_endpoint().port());
		if (!ec)
		{
			sprintf_s(charTMP, "IP:%s, Port:%d", v4address.to_string().c_str(), socket_.remote_endpoint().port());
			socket_.cancel();
			socket_.close();
			ThreadSafeOutput(std::string(charTMP) + std::string(", Disconnected"));
		}
		else
		{
			sprintf_s(charTMP, "Close Error, Code: %d, Message: ", ec.value());
			ThreadSafeOutput(charTMP + ec.message());
		}

		PushListData(Net2SerialBuffer, CCharArray(socketHash, disconnectFlag.c_str(), (int)disconnectFlag.size()));
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
				sprintf_s(strVal, "Write Error, Code: %d, Message: ", ec.value());
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
				Net2SerialBuffer.put(CCharArray(hash_socket(socket_.remote_endpoint().port()), data_, length));
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

std::vector<std::shared_ptr<SocketSession>> sessionVector;

class NetServer
{
public:
	NetServer(boost::asio::io_service& io_service, tcp::endpoint endpoint)
		: acceptor_(io_service, endpoint),
		io_service_(io_service)
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
		_pSocketSession = std::make_shared<SocketSession>(io_service_);
		acceptor_.async_accept(_pSocketSession->getSocket(),
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				boost::asio::ip::address v4address;
				_pSocketSession->getSocket().remote_endpoint().address(v4address);
				size_t socketHash = hash_socket(_pSocketSession->getSocket().remote_endpoint().port());

				char charTMP[200];
				sprintf_s(charTMP, "Accepted, IP:%s, Port:%d", v4address.to_string().c_str(), _pSocketSession->getSocket().remote_endpoint().port());
				ThreadSafeOutput(std::string(charTMP));
				
				_pSocketSession->start();
				sessionVector.push_back(_pSocketSession);
			}

			do_accept();
		});
	}

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	std::shared_ptr<SocketSession> _pSocketSession;
};

//Get data from Serial Port, then the function be calledback
static int getSerialData(size_t Hash, const std::vector<unsigned char> &SerialData, int iLen)
{
	CCharArray tmp = CCharArray(Hash, SerialData, iLen);
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

				sp->Write2Serial(data.getHash(), (unsigned char *)(data.getPtr()), data.getLength());

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
		std::thread socketWriteThread([] {
			while (true)
			{
				CCharArray data = Serial2NetBuffer.get_pop();

				//May be lost the chance for writing when sessionList be changed in the loop
				//getCount every time to avoid out of rang
				for (int iLoop = 0; iLoop < sessionVector.size(); iLoop ++)
				{
					if (sessionVector[iLoop]->getSocketHash() == data.getHash())
					{
						sessionVector[iLoop]->do_write(data.getPtr(), data.getLength());
						break;
					}
				}
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
