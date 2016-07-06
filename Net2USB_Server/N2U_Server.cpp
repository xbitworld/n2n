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

#include "N2U_Server.h"
#include "../CTMutexSet.h"
#include "../ASIOLib/Executor.h"
#include "../ASIOLib/SerialPort.h"

std::string strTMP;

using boost::asio::ip::tcp;

boost::mutex io_mutex;	//mutex for console display
ClassMutexList<CMTCharArray> inBuf;		//Client Send to Server
ClassMutexList<CMTCharArray> outBuf;	//Client Read from Server

static void ThreadSafeOutput(const void * pChar)
{
	std::string outputString = std::string((const char *)pChar);
	boost::mutex::scoped_lock	lock(io_mutex);
	std::cout << outputString << std::endl;
}

static void ThreadSafeOutput(const std::string &info)
{
	boost::mutex::scoped_lock	lock(io_mutex);
	std::cout << info << std::endl;
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
		do_read();
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
				strTMP = "Data: " + std::string(data_);
				ThreadSafeOutput(strTMP.c_str());
				do_write(length);
			}
		});
	}

	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				do_read();
			}
		});
	}

	tcp::socket socket_;
	enum { max_length = 1024 };
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

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				ThreadSafeOutput("Accept");
				std::make_shared<SocketSession>(std::move(socket_))->start();
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

//Insert data to list, and wait for pop
void PushData(ClassMutexList<CMTCharArray> &buf, const char *pData, int iLen)
{
	CMTCharArray tempData(pData, iLen);
	buf.put(tempData);
}

//Get data from Serial List and wait for be sent to Network Port
void PopSerialData(ClassMutexList<CMTCharArray> &dataList)
{
	while (1)
	{
		CMTCharArray tempData = dataList.get_pop();
		int iLen = tempData.getLength();
		const char * pDataStr = tempData.getPtr();
	}
}

//Get data from Net List and wait for be sent to Serial Port
void PopNetData(ClassMutexList<CMTCharArray> &dataList)
{
	while (1)
	{
		CMTCharArray tempData = dataList.get_pop();
		int iLen = tempData.getLength();
		const char * pDataStr = tempData.getPtr();
	}
}

typedef boost::tuple<boost::posix_time::time_duration::tick_type, std::vector<unsigned char>> WriteBufferElement;

class SerialReader : private boost::noncopyable, public boost::enable_shared_from_this<SerialReader> {
	boost::shared_ptr<ASIOLib::SerialPort> _serialPort;
	std::string _portName;
	unsigned int _baudRate;
	boost::posix_time::ptime _lastRead;

	std::vector<WriteBufferElement> _writeBuffer;
	bool _noTimeOffsets;

	void OnRead(boost::asio::io_service &ios, const std::vector<unsigned char> &buffer, size_t bytesRead);

public:
	SerialReader(const std::string &portName, int baudRate, const std::vector<WriteBufferElement> &writeBuffer, bool noTimeOffsets) :
		_portName(portName), _baudRate(baudRate), _writeBuffer(writeBuffer), _noTimeOffsets(noTimeOffsets) {}
	void Create(boost::asio::io_service &ios) {
		try {
			_serialPort.reset(new ASIOLib::SerialPort(ios, _portName));
			_serialPort->Open(boost::bind(&SerialReader::OnRead, shared_from_this(), _1, _2, _3), _baudRate);
		}
		catch (const std::exception &e) {
			std::cout << e.what() << std::endl;
		}

		// post the creation, so Run() executes immediately - threads can start to work on what the func is posting, rather than waiting until all work is queued
		ios.post([=, &ios] {
			uint64_t startTime = 0;
			std::for_each(_writeBuffer.begin(), _writeBuffer.end(),
				[&](const WriteBufferElement &e) {
				// if noTimeOffsets, just write the buffer, otherwise create a timer to write the buffer in the future
				if (_noTimeOffsets)
					_serialPort->Write(e.get<1>());
				else {
					startTime += e.get<0>();
					const boost::shared_ptr<boost::asio::deadline_timer> timer(new boost::asio::deadline_timer(ios));
					timer->expires_from_now(boost::posix_time::milliseconds(startTime));
					timer->async_wait([=](const boost::system::error_code &ec) {
						boost::shared_ptr<boost::asio::deadline_timer> t(timer); // need this to keep the timer object alive
						_serialPort->Write(e.get<1>());
					});
				}
			}
			);
		});
	}
};


void SerialReader::OnRead(boost::asio::io_service &, const std::vector<unsigned char> &buffer, size_t bytesRead) {
	const boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();

	if (_lastRead == boost::posix_time::not_a_date_time)
		_lastRead = now;

	const std::vector<unsigned char> v(buffer.begin(), buffer.begin() + bytesRead);

	const uint64_t offset = (now - _lastRead).total_milliseconds();
	//*_oa << offset << v;

	_lastRead = now;

	std::copy(v.begin(), v.end(), std::ostream_iterator<unsigned char>(std::cout, ""));
}

class SerialWriter : private boost::noncopyable, public boost::enable_shared_from_this<SerialWriter> {
	boost::shared_ptr<ASIOLib::SerialPort> _serialPort;
	std::string _portName;
	unsigned int _baudRate;
	std::vector<WriteBufferElement> _writeBuffer;
	bool _noTimeOffsets;
public:
	SerialWriter(const std::string &portName, int baudRate, const std::vector<WriteBufferElement> &writeBuffer, bool noTimeOffsets) :
		_portName(portName), _baudRate(baudRate), _writeBuffer(writeBuffer), _noTimeOffsets(noTimeOffsets) {}

	void Create(boost::asio::io_service &ios) {
		_serialPort.reset(new ASIOLib::SerialPort(ios, _portName));
		_serialPort->Open(0, _baudRate);

		// post the creation, so Run() executes immediately - threads can start to work on what the func is posting, rather than waiting until all work is queued
		ios.post([=, &ios] {
			uint64_t startTime = 0;
			std::for_each(_writeBuffer.begin(), _writeBuffer.end(),
				[&](const WriteBufferElement &e) {
				// if noTimeOffsets, just write the buffer, otherwise create a timer to write the buffer in the future
				if (_noTimeOffsets)
					_serialPort->Write(e.get<1>());
				else {
					startTime += e.get<0>();
					const boost::shared_ptr<boost::asio::deadline_timer> timer(new boost::asio::deadline_timer(ios));
					timer->expires_from_now(boost::posix_time::milliseconds(startTime));
					timer->async_wait([=](const boost::system::error_code &ec) {
						boost::shared_ptr<boost::asio::deadline_timer> t(timer); // need this to keep the timer object alive
						_serialPort->Write(e.get<1>());
					});
				}
			}
			);
		});
	}
};

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
		strTMP = std::string("Address: " + std::string(argv[1]) + ", Port: " + argv[2]);
		ThreadSafeOutput(strTMP.c_str());
		boost::asio::ip::address address = boost::asio::ip::address::from_string(argv[1]);
		tcp::endpoint endpoint(address, atoi(argv[2]));

		NetServer s(io_service, endpoint);

		std::thread socketNETThread([&io_service]() { io_service.run(); });

		std::thread readCOMThread([](){
			ASIOLib::Executor e;
			e.OnWorkerThreadError = [](boost::asio::io_service &, boost::system::error_code ec) { ThreadSafeOutput(std::string("SerialReader error (asio): ") + boost::lexical_cast<std::string>(ec)); };
			e.OnWorkerThreadException = [](boost::asio::io_service &, const std::exception &ex) { ThreadSafeOutput(std::string("SerialReader exception (asio): ") + ex.what()); };

			std::vector<WriteBufferElement> writeBuffer;
			bool noTimeOffsets = false;

			std::string message = "Send Test";
			if (!message.empty()) {
				std::vector<unsigned char> v;
				std::copy(message.begin(), message.end(), std::back_insert_iterator<std::vector<unsigned char>>(v));
				writeBuffer.push_back(boost::make_tuple(0, v));
			}

			const boost::shared_ptr<SerialReader> sp(new SerialReader("COM4", 9600, writeBuffer, noTimeOffsets));  // for shared_from_this() to work inside of Reader, Reader must already be managed by a smart pointer
			e.OnRun = boost::bind(&SerialReader::Create, sp, _1);
			e.Run(1);
		});

		std::thread InputCMDThread(InputCMD);

		socketNETThread.join();
		readCOMThread.join();
		InputCMDThread.join();
	}
	catch (std::exception& e)
	{
		strTMP = std::string("Exception: " + std::string(e.what()));
		ThreadSafeOutput(strTMP.c_str());
	}

	return 0;
}
