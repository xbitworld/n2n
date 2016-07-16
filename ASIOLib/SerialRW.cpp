
#include "SerialRW.h"

std::string split_head("@$Start$@");
std::string split_end("$@End@$");

int FindSplitPos(const std::vector<unsigned char> &buffer, const std::string &splitStr)
{
	int iSize = buffer.size();
	int iCmpSize = splitStr.size();

	for (int iLoop = 0; iLoop < iSize; iLoop++)
	{
		for (int inerLoop = iLoop;
			inerLoop <= (iSize - iCmpSize);	//保证比较位置不越界
			inerLoop++)
		{
			bool bFind = false;
			for (int iPos = 0; iPos < iCmpSize; iPos++)	//比较是否是标记符
			{
				if (buffer.at(inerLoop + iPos) != splitStr[iPos])
				{
					break;
				}
				bFind = true;
			}

			if (!bFind)
			{//没有找到标记
				continue;
			}

			//找到标记则返回标记的开始位置
			return inerLoop;
		}
	}

	return -1;
}

void SerialRW::OnRead(boost::asio::io_service &, const std::vector<unsigned char> &buffer, size_t bytesRead) 
{
	static  std::vector<unsigned char> vTemp;

	int ioldSize = vTemp.size();
	int iCmpSize = split_end.size();

	vTemp.insert(vTemp.end(), buffer.begin(), buffer.begin() + bytesRead);

	int AllBytes = vTemp.size();
	if (AllBytes > 4096)
	{
		vTemp.erase(vTemp.begin(), vTemp.begin() + AllBytes - 4096);
	}

split:
	int inewSize = vTemp.size();

	for (int iLoop = ((ioldSize > iCmpSize) ? ioldSize - iCmpSize : 0); iLoop < inewSize; iLoop ++)
	{
		for (int inerLoop = iLoop; 
			inerLoop <= (inewSize - iCmpSize);	//保证比较位置不越界
			inerLoop++)
		{
			bool bFind = true;
			for (int iPos = 0; iPos < iCmpSize; iPos++)	//比较是否是结束符
			{
				if (vTemp.at(inerLoop + iPos) != split_end[iPos])
				{
					bFind = false;
					break;
				}
			}

			if (!bFind)
			{//没有找到结束标记
				continue;
			}

			std::vector<unsigned char> dataTemp(vTemp.begin(), vTemp.begin() + inerLoop);
			vTemp.erase(vTemp.begin(), vTemp.begin() + inerLoop + iCmpSize);

			int iStartPos = FindSplitPos(dataTemp, split_head);	//找到开始标记的位置
			if (iStartPos >= 0)
			{
				dataTemp.erase(dataTemp.begin(), dataTemp.begin() + iStartPos + split_head.size());
				_getDataCall(dataTemp, dataTemp.size());
			}
			ioldSize = 0;

			goto split;
		}
	}
}

void SerialRW::Write2Serial(unsigned char *pData, int iLen)
{
	_serialPort->Write((unsigned char *)(split_head.c_str()), split_head.size());
	_serialPort->Write(pData, iLen);
	_serialPort->Write((unsigned char *)(split_end.c_str()), split_end.size());
}