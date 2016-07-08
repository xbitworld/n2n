#include "SerialRW.h"

void SerialRW::OnRead(boost::asio::io_service &, const std::vector<unsigned char> &buffer, size_t bytesRead) {
	const boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();

	if (_lastRead == boost::posix_time::not_a_date_time)
		_lastRead = now;

	const std::vector<unsigned char> v(buffer.begin(), buffer.begin() + bytesRead);

	const uint64_t offset = (now - _lastRead).total_milliseconds();
	//*_oa << offset << v;

	_lastRead = now;

	std::copy(v.begin(), v.end(), std::ostream_iterator<unsigned char>(std::cout, ""));

	_getDataCall(v);
}

void SerialRW::Write2Serial(unsigned char *pData, int iLen)
{
	_serialPort->Write(pData, iLen);
}