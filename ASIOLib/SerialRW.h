#pragma once
#include <boost/bind.hpp>
#include "Executor.h"
#include "SerialPort.h"

class SerialRW : private boost::noncopyable, public boost::enable_shared_from_this<SerialRW> {
	boost::shared_ptr<ASIOLib::SerialPort> _serialPort;
	std::string _portName;
	unsigned int _baudRate;
	boost::posix_time::ptime _lastRead;

	void OnRead(boost::asio::io_service &ios, const std::vector<unsigned char> &buffer, size_t bytesRead);

	void(*_getDataCall)(const std::vector<unsigned char> &buffer, int iLen);

public:
	SerialRW(void(*getDataCall)(const std::vector<unsigned char> &, int), const std::string &portName, int baudRate) :
		_getDataCall(getDataCall), _portName(portName), _baudRate(baudRate) {}

	void Create(boost::asio::io_service &ios) {
		try {
			_serialPort.reset(new ASIOLib::SerialPort(ios, _portName));
			_serialPort->Open(boost::bind(&SerialRW::OnRead, shared_from_this(), _1, _2, _3), _baudRate);
		}
		catch (const std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	}

	void Write2Serial(unsigned char *pData, int iLen);
};
