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

ClassMutexList<CCharArray> Net2SerialBuffer;		//Buffer for send to serial
ClassMutexList<CCharArray> Serial2NetBuffer;		//Buffer for send to socket

std::string connectFlag("&*CONN*&");
std::string disconnectFlag("*&DISC&*");

class CommPair
{
public:
	CommPair() {}

	std::shared_ptr<std::thread> pTH;
	size_t serverHash;
	dtCSC::CSocketClient* pClientSocket;
};

class CHashMark
{
public:
	CHashMark() {}
	CHashMark(size_t h, bool ar) :hash(h), add_rmv(ar) {}

	size_t hash;
	bool add_rmv;	//true for add, otherwise remove
};

ClassMutexList<CHashMark> HashList;

boost::mutex cv_mutex;	//mutex for commVector
std::vector<CommPair> commVector;	//Need to lock in multi thread
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

static void writeNetData(ClassMutexList<CCharArray> &dataList)
{
	while (true)
	{
		CCharArray tempData = dataList.get_pop();

		boost::mutex::scoped_lock	lock(cv_mutex);

		bool bFind = false;

		for each (auto commTMP in commVector)
		{
			if (commTMP.serverHash == tempData.getHash())
			{
				commTMP.pClientSocket->write(tempData.getPtr(), tempData.getLength());
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			ThreadSafeOutput("Didn't find CommObj");
		}
	}
}

void socketConnect()
{//Need to modify, dutao@2016-08-05
	boost::asio::io_service io_service;
	boost::asio::io_service::work worker(io_service);

	while (true)
	{
		size_t addHash = 0;
		size_t rmvHash = 0;

		CHashMark hashObj = HashList.get_pop();

		if(hashObj.add_rmv)
		{//Add thread
			boost::asio::ip::tcp::resolver resolver(io_service);
			auto endpoint_iterator = resolver.resolve({ strDestIP, strDestPort });

			CommPair commObj;
			commObj.serverHash = hashObj.hash;

			commObj.pClientSocket = new dtCSC::CSocketClient(commObj.serverHash, io_service, endpoint_iterator, std::ref(Net2SerialBuffer), ThreadSafeOutput);

			boost::mutex::scoped_lock	lock(cv_mutex);
			if (commVector.size() > 0)
			{
				commObj.pTH = commVector[0].pTH;
				if (io_service.stopped())
				{
					ThreadSafeOutput("Reset io_service");
				}
				io_service.reset();
			}
			else
			{
				commObj.pTH = std::make_shared<std::thread>(std::thread([&io_service]() { io_service.run(); }));
			}
			commVector.push_back(commObj);

			char strEvent[100];
			sprintf_s(strEvent, "Current: %zd", commVector.size());
			ThreadSafeOutput("Add commVector" + std::string(strEvent));
		}
		else
		{//Remove thread
			int iCount = 0;
			bool bFind = false;
			CommPair commObj;

			{//Avoid lock commVector too long
				boost::mutex::scoped_lock	lock(cv_mutex);
				for each (auto tmpObj in commVector)
				{
					if (tmpObj.serverHash == hashObj.hash)
					{
						commObj = tmpObj;
						bFind = true;
						break;
					}
					iCount++;
				}
			}

			if (bFind)
			{
				//commObj.pClientSocket->Close();
				io_service.stop();
				commObj.pTH->join();
				delete commObj.pClientSocket;
				commObj.pTH.reset();

				commVector.erase(commVector.begin() + iCount);

				char strEvent[100];
				sprintf_s(strEvent, "Current: %zd", commVector.size());
				ThreadSafeOutput("Erase commVector, " + std::string(strEvent));
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

		CHashMark hashObj(Hash, true);
		HashList.put(hashObj);

		iEvent = 1;
	}
	else if (strncmp(disconnectFlag.c_str(), tmp.getPtr(), disconnectFlag.size()) == 0)
	{
		ThreadSafeOutput("to Disconnect");

		CHashMark hashObj(Hash, false);
		HashList.put(hashObj);

		iEvent = 2;

	}
	//DisplayHEX((const char *)("Serial: "), tmp.getPtr(), iLen);
	std::string lstrTMP;

	std::time_t t = std::time(NULL);
	struct tm now;
	char mbstr[100];
	::localtime_s(&now, &t);
	std::strftime(mbstr, sizeof(mbstr), "%T", &now);

	if (iEvent != 0)
	{
		char strEvent[100];
		sprintf_s(strEvent, "%s, Event: %d, Current: %zd", mbstr, iEvent, commVector.size());
		lstrTMP = std::string(strEvent);
	}
	else
	{
		Serial2NetBuffer.put(tmp);
		//ThreadSafeOutput(std::string("Serial Data\r\n"));// +std::string(tmp.getPtr()));

		char strLen[100] = { 0 };
		::_itoa_s(tmp.getLength(), strLen, 10);

		lstrTMP = std::string(mbstr) + " R: " + std::string(strLen);
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

		const boost::shared_ptr<SerialRW> sp(new SerialRW(getSerialData, argv[1], 115200));  // for shared_from_this() to work inside of Reader, Reader must already be managed by a smart pointer

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
		std::thread socketMannager(socketConnect);

		std::thread InputCMDThread(InputCMD);

		readCOMThread.join();
		writeCOMThread.join();
		writeNetThread.join();
		socketMannager.join();
		InputCMDThread.join();
	}
	catch (std::exception& e)
	{
		strTMP = std::string("Exception: " + std::string(e.what()));
		ThreadSafeOutput(strTMP.c_str());
	}

	return 0;
}
