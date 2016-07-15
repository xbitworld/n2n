
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
			inerLoop <= (iSize - iCmpSize);	//��֤�Ƚ�λ�ò�Խ��
			inerLoop++)
		{
			bool bFind = false;
			for (int iPos = 0; iPos < iCmpSize; iPos++)	//�Ƚ��Ƿ��Ǳ�Ƿ�
			{
				if (buffer.at(inerLoop + iPos) != splitStr[iPos])
				{
					break;
				}
				bFind = true;
			}

			if (!bFind)
			{//û���ҵ����
				continue;
			}

			//�ҵ�����򷵻ر�ǵĿ�ʼλ��
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

split:
	int inewSize = vTemp.size();
	if (inewSize > 4096)
	{
		vTemp.clear();
		return;
	}

	for (int iLoop = ((ioldSize > iCmpSize) ? ioldSize - iCmpSize : 0); iLoop < inewSize; iLoop ++)
	{
		for (int inerLoop = iLoop; 
			inerLoop <= (inewSize - iCmpSize);	//��֤�Ƚ�λ�ò�Խ��
			inerLoop++)
		{
			bool bFind = true;
			for (int iPos = 0; iPos < iCmpSize; iPos++)	//�Ƚ��Ƿ��ǽ�����
			{
				if (vTemp.at(inerLoop + iPos) != split_end[iPos])
				{
					bFind = false;
					break;
				}
			}

			if (!bFind)
			{//û���ҵ��������
				continue;
			}

			std::vector<unsigned char> dataTemp(vTemp.begin(), vTemp.begin() + inerLoop);
			vTemp.erase(vTemp.begin(), vTemp.begin() + inerLoop + iCmpSize);

			int iStartPos = FindSplitPos(dataTemp, split_head);	//�ҵ���ʼ��ǵ�λ��
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