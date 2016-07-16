#ifndef CTMUTEXSET_H
#define CTMUTEXSET_H

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <iostream>

class CCharArray
{
	enum { max_length = 4095 };

public:
	CCharArray(int ilen) :iLength(ilen)
	{
		if (iLength > max_length)
		{
			std::cout << "Out of range ! 1-Length: " << iLength << std::endl;
		}
		pArray = new char[iLength];
	}

	CCharArray(const char *pStr, int iLen)
	{
		iLength = iLen;
		if (iLength > max_length)
		{
			std::cout << "Out of range ! 3-Length: " << iLength << std::endl;
		}

		pArray = new char[iLength];
		std::memcpy(pArray, pStr, iLength);
	}

	CCharArray(const std::vector<unsigned char> v, int iLen)
	{
		iLength = iLen;
		if (iLength > max_length)
		{
			std::cout << "Out of range ! 3-Length: " << iLength << std::endl;
		}

		pArray = new char[iLength];

		for (int i = 0; i < iLength; i++)
		{
			*(pArray + i) = v[i];
		}
	}

	CCharArray(const CCharArray &src)
	{
		iLength = src.getLength();
		if (iLength > max_length)
		{
			std::cout << "Out of range ! 2-Length: " << iLength << std::endl;
		}

		char *charTMP = new char[iLength];
		std::memcpy(charTMP, src.pArray, iLength);

		pArray = charTMP;
	}

	CCharArray &operator = (const CCharArray &src)
	{
		//		std::cout << "=" << std::endl;
		if (this == &src)
		{
			return *this;
		}

		iLength = src.getLength();
		char *charTMP = new char[iLength];
		std::memcpy(charTMP, src.pArray, iLength);

		if (pArray != nullptr)
			delete[] pArray;
		pArray = charTMP;

		return *this;
	}

	CCharArray(CCharArray &&src)
	{
		//		std::cout << "Copy Move" << std::endl;
		pArray = src.NullArray(iLength);
	}

	CCharArray &operator =(CCharArray &&src)
	{
		//		std::cout << "= Move" << std::endl;
		if (this == &src)
		{
			return *this;
		}

		if (pArray != nullptr)
			delete[] pArray;

		pArray = src.NullArray(iLength);
	}

	virtual ~CCharArray()
	{//To avoid deconstruct many times
		if (pArray != nullptr)
			delete[] pArray;

		pArray = nullptr;
	}

	char * NullArray(int &iLen)
	{
		char * pTMP = nullptr;

		pTMP = pArray;
		iLen = iLength;

		iLength = 0;
		pArray = nullptr;

		return pTMP;
	}

	void put(const char *pStr, int ilen)
	{
		iLength = ilen;
		if (iLength > max_length)
		{
			std::cout << "Out of range ! 4-Length: " << iLength << std::endl;
		}

		std::memcpy(pArray, pStr, iLength);
	}

	const int getLength() const
	{
		return iLength;
	}

	const char * getPtr() const
	{
		return pArray;
	}

private:
	int iLength;
	char *pArray = nullptr;
};

template <typename T>
class ClassMutexList
{
public:
	typedef boost::mutex::scoped_lock scoped_lock;

	ClassMutexList()
		: full(0), iBUF_SIZE(100)	//Default buffer size is 100
	{
	}

	ClassMutexList(int iBufSize)
		: full(0), iBUF_SIZE(iBufSize)
	{
	}

	void put(const T& m)
	{
		scoped_lock lock(mutex);
		if (full == iBUF_SIZE)
		{
			while (full == iBUF_SIZE)
				cond.wait(lock);
			//			std::cout << "Wait for put!" << std::endl;
		}
		//std::cout << "List Length: " << m.getLength() << std::endl;

		TList.push_back(m);
		++full;
		cond.notify_all();
	}

	T get_last()
	{
		scoped_lock lock(mutex);
		if (full == 0)
		{
			while (full == 0)
				cond.wait(lock);
			//			std::cout << "Wait for get Last!" << std::endl;
		}
		T tmp = TList.front();
		cond.notify_all();

		return tmp;
	}

	T get_pop()
	{
		scoped_lock lock(mutex);
		if (full == 0)
		{
			while (full == 0)
				cond.wait(lock);
			//			std::cout << "Wait for get POP!" << std::endl;
		}

		T tmp = TList.front();
		TList.pop_front();
		--full;
		cond.notify_all();
		return tmp;
	}

	int getCount(void)
	{
		scoped_lock lock(mutex);
		int iCounter = full;
		return iCounter;
	}

	void lock()
	{
		mutex.lock();
	}
	void unlock()
	{
		mutex.unlock();
	}

private:
	boost::mutex mutex;
	boost::condition cond;
	unsigned int full;
	std::list<T> TList;
	int iBUF_SIZE;
};

#endif
