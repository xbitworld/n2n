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

#include "N2U_Client.h"
#include "../CTMutexSet.h"
#include "../ASIOLib/SerialRW.h"
#include "../SocketLib/c_socket_client.h"
#include "../HEXSTRTable.h"

using boost::asio::ip::tcp;
using namespace std;

std::string strTMP;

std::string strDestIP, strDestPort;	//The target IP & Port

boost::mutex io_mutex;	//mutex for console display
ClassMutexList<CCharArray> inBuf;		//Client Send to Server
ClassMutexList<CCharArray> outBuf;	//Client Read from Server

ClassMutexList<CCharArray> Net2SerialBuffer;		//Buffer for send to serial
ClassMutexList<CCharArray> Serial2NetBuffer;		//Buffer for send to socket

std::string connectFlag("&*CONN*&");
std::string disconnectFlag("*&DISC&*");

class CommPair
{
public:
	CommPair() {}

	boost::thread *pTH;
	size_t serverHash;
	dtCSC::CSocketClient *pClientSocket;
};

std::vector<CommPair> commVector;
//ClassMutexList<std::shared_ptr<dtCSC::CSocketClient>> socketList;

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

static void ThreadSafeOutput(const void * pChar)
{
	std::string outputString = std::string((const char *)pChar);
	boost::mutex::scoped_lock	lock(io_mutex);
	std::cout << outputString << std::endl;
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
	}
}

void newConnect(CommPair &commObj)
{
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::resolver resolver(io_service);
	auto endpoint_iterator = resolver.resolve({ strDestIP, strDestPort });

	commObj.pClientSocket = new dtCSC::CSocketClient(commObj.serverHash, io_service, endpoint_iterator, std::ref(Net2SerialBuffer), ThreadSafeOutput);
	commObj.pTH = new boost::thread([&io_service]() { io_service.run(); });

	//commObj.pTH->join();
}

static void writeNetData(ClassMutexList<CCharArray> &dataList)
{
	while (true)
	{
		CCharArray tempData = dataList.get_pop();

		for each (auto commTMP in ((std::vector<CommPair>)(commVector)))
		{
			if (commTMP.serverHash == tempData.getHash())
			{
				commTMP.pClientSocket->write(tempData.getPtr(), tempData.getLength());
			}
		}
	}
}

//Get data from Serial Port, then the function be calledback
static int getSerialData(size_t Hash, const std::vector<unsigned char> &SerialData, int iLen)
{
	int iEvent = 0;
	CCharArray tmp = CCharArray(Hash, SerialData, iLen);

	if(strncmp(connectFlag.c_str(), tmp.getPtr(), connectFlag.size()) == 0)
	{
		ThreadSafeOutput("to Connect");

		CommPair commObj;
		commObj.serverHash = Hash;
		newConnect(commObj);

		//std::thread(bind(newConnect, _1, commObj));

		((std::vector<CommPair>)(commVector)).push_back(commObj);

		iEvent = 1;
	}
	else if (strncmp(disconnectFlag.c_str(), tmp.getPtr(), disconnectFlag.size()) == 0)
	{
		ThreadSafeOutput("to Disconnect");

		int iCount = 0;
		for each (auto commTMP in ((std::vector<CommPair>)(commVector)))
		{
			if (commTMP.serverHash == Hash)
			{
				commTMP.pTH->interrupt();
				commTMP.pClientSocket->Close();

				delete commTMP.pTH;
				delete commTMP.pClientSocket;

				((std::vector<CommPair>)(commVector)).erase(((std::vector<CommPair>)(commVector)).begin() + iCount);
				iEvent = 2;
			}
			iCount++;
		}

		iEvent = 3;
	}
	//DisplayHEX((const char *)("Serial: "), tmp.getPtr(), iLen);
	std::string lstrTMP;

	std::time_t t = std::time(NULL);
	struct tm now;
	char mbstr[100];
	::localtime_s(&now, &t);
	std::strftime(mbstr, sizeof(mbstr), "%T ", &now);

	if (iEvent != 0)
	{
		char strEvent[100];
		sprintf_s(strEvent, "Event: %d", iEvent);
		lstrTMP = std::string(mbstr)+ std::string(strEvent);
	}
	else
	{
		Serial2NetBuffer.put(tmp);
		//ThreadSafeOutput(std::string("Serial Data\r\n"));// +std::string(tmp.getPtr()));

		char strLen[100] = { 0 };
		::_itoa_s(tmp.getLength(), strLen, 10);

		lstrTMP = std::string(mbstr) + std::string(strLen);
	}

	ThreadSafeOutput(lstrTMP);

	return 0;
}

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 3)
		{
			std::cerr << "Usage: DelivData <serial> <host> <port>\n";
			return 1;
		}

		strDestIP = argv[2];
		strDestPort = argv[3];

		strTMP = std::string("Serial: " + std::string(argv[1]) + ", Address: " + strDestIP + ", Port: " + strDestPort);
		ThreadSafeOutput(strTMP.c_str());

		const boost::shared_ptr<SerialRW> sp(new SerialRW(getSerialData, argv[1], 512000));  // for shared_from_this() to work inside of Reader, Reader must already be managed by a smart pointer

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
				//DisplayHEX((const char *)("Netdata: "), data.getPtr(), data.getLength());
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
		
		std::thread writeNetThread(writeNetData, std::ref(Serial2NetBuffer));

		std::thread InputCMDThread(InputCMD);

		readCOMThread.join();
		writeCOMThread.join();
		writeNetThread.join();

		InputCMDThread.join();
	}
	catch (std::exception& e)
	{
		strTMP = std::string("Exception: " + std::string(e.what()));
		ThreadSafeOutput(strTMP.c_str());
	}

	return 0;
}
