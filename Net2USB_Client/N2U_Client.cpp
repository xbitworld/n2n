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

#include "N2U_Client.h"
#include "../CTMutexSet.h"
#include "../ASIOLib/SerialRW.h"
#include "../SocketLib/c_socket_client.h"

std::string strTMP;

using boost::asio::ip::tcp;

boost::mutex io_mutex;	//mutex for console display
ClassMutexList<CCharArray> inBuf;		//Client Send to Server
ClassMutexList<CCharArray> outBuf;	//Client Read from Server

ClassMutexList<CCharArray> Net2SerialBuffer;		//Buffer for send to serial
ClassMutexList<CCharArray> Serial2NetBuffer;		//Buffer for send to socket

													//Insert data to list, and wait for pop
void PushListData(ClassMutexList<CCharArray> &buf, const char *pData, int iLen)
{
	CCharArray tempData(pData, iLen);
	buf.put(tempData);
}

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

		msg += "\r\n";
		PushListData(Net2SerialBuffer, msg.c_str(), msg.size());
	}
}

static void readCMD(dtCSC::CSocketClient &csc, ClassMutexList<CCharArray> &dataList)
{
	while (true)
	{
		CCharArray tempData = dataList.get_pop();
	}
	csc.close();
}

static void writeCMD(dtCSC::CSocketClient &csc, ClassMutexList<CCharArray> &dataList)
{
	while (true)
	{
		CCharArray tempData = dataList.get_pop();
		int iLen = tempData.getLength();
		char * pchars = new char[tempData.getLength()];
		memcpy_s(pchars, iLen, tempData.getPtr(), iLen);

		std::string strTMP(pchars);

		if (!csc.isSocketOpen())
		{
			csc.ReConnect();
		}
		csc.write(strTMP);

		delete[] pchars;

		//std::cout << "\tCMD strTMP - Length: " << strTMP.length() << ", Data: \n" << strTMP << std::endl;
	}
	csc.close();
}

//Insert data to list, and wait for pop
void PushData(ClassMutexList<CCharArray> &buf, const char *pData, int iLen)
{
	CCharArray tempData(pData, iLen);
	buf.put(tempData);
}

//Get data from Serial List and wait for be sent to Network Port
void PopSerialData(ClassMutexList<CCharArray> &dataList)
{
	while (1)
	{
		CCharArray tempData = dataList.get_pop();
		int iLen = tempData.getLength();
		const char * pDataStr = tempData.getPtr();
	}
}

//Get data from Net List and wait for be sent to Serial Port
void PopNetData(ClassMutexList<CCharArray> &dataList)
{
	while (1)
	{
		CCharArray tempData = dataList.get_pop();
		int iLen = tempData.getLength();
		const char * pDataStr = tempData.getPtr();
	}
}

//Get data from Serial Port, then the function be calledback
void getSerialData(const std::vector<unsigned char> &SerialData)
{
	Serial2NetBuffer.put(CCharArray(SerialData));
	ThreadSafeOutput(std::string(" Serial Data \r\n"));
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
		strTMP = std::string("Address: " + std::string(argv[1]) + ", Port: " + argv[2]);
		ThreadSafeOutput(strTMP.c_str());

		boost::asio::ip::tcp::resolver resolver(io_service);
		auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
		dtCSC::CSocketClient custSocket(io_service, endpoint_iterator, Net2SerialBuffer);

		const boost::shared_ptr<SerialRW> sp(new SerialRW(getSerialData, "COM4", 9600));  // for shared_from_this() to work inside of Reader, Reader must already be managed by a smart pointer

		std::thread readCOMThread([&sp]() {
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
			}
		});

		std::thread InputCMDThread(InputCMD);

		readCOMThread.join();
		writeCOMThread.join();

		InputCMDThread.join();
	}
	catch (std::exception& e)
	{
		strTMP = std::string("Exception: " + std::string(e.what()));
		ThreadSafeOutput(strTMP.c_str());
	}

	return 0;
}
