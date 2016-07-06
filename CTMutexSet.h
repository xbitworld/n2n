#ifndef CTMUTEXSET_H
#define CTMUTEXSET_H

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <iostream>

class CMTCharArray
{
	enum { max_length = 4095 };

public:
	typedef boost::mutex::scoped_lock scoped_lock;

	CMTCharArray(int ilen) :iLength(ilen)
	{
		scoped_lock lock(mutex);

		if (iLength > max_length)
		{
			std::cout << "Out of range ! 1-Length: " << iLength << std::endl;
		}

		pArray = new char[iLength + 1];
	}

	CMTCharArray(const char *pStr, int iLen)
	{
		scoped_lock lock(mutex);

		iLength = iLen;
		if (iLength > max_length)
		{
			std::cout << "Out of range ! 3-Length: " << iLength << std::endl;
		}

		pArray = new char[iLength + 1];
		std::memcpy(pArray, pStr, iLength);
		pArray[iLength] = '\0';
	}

	CMTCharArray(const CMTCharArray &src)
	{
		//		std::cout << "Copy" << std::endl;

		scoped_lock lock(mutex);

		//		src.lock();
		iLength = src.getLength();
		if (iLength > max_length)
		{
			std::cout << "Out of range ! 2-Length: " << iLength << std::endl;
		}

		char *charTMP = new char[iLength + 1];
		std::memcpy(charTMP, src.pArray, iLength);

		pArray = charTMP;

		//		src.unlock();

		pArray[iLength] = '\0';
	}

	CMTCharArray &operator = (const CMTCharArray &src)
	{
		//		std::cout << "=" << std::endl;
		if (this == &src)
		{
			return *this;
		}

		scoped_lock lock(mutex);
		//		src.lock();

		iLength = src.getLength();
		char *charTMP = new char[iLength + 1];
		std::memcpy(charTMP, src.pArray, iLength);

		if (pArray != nullptr)
			delete[] pArray;
		pArray = charTMP;

		//		src.unlock();

		pArray[iLength] = '\0';

		return *this;
	}

	CMTCharArray(CMTCharArray &&src)
	{
		//		std::cout << "Copy Move" << std::endl;
		scoped_lock lock(mutex);

		src.lock();

		pArray = src.NullArray(iLength);

		src.unlock();
	}

	CMTCharArray &operator =(CMTCharArray &&src)
	{
		//		std::cout << "= Move" << std::endl;
		if (this == &src)
		{
			return *this;
		}

		src.lock();

		if (pArray != nullptr)
			delete[] pArray;

		pArray = src.NullArray(iLength);

		src.unlock();
	}

	virtual ~CMTCharArray()
	{//To avoid deconstruct many times

		scoped_lock lock(mutex);

		if (pArray != nullptr)
			delete[] pArray;

		pArray = nullptr;
	}

	char * NullArray(int &iLen)
	{
		char * pTMP = nullptr;
		scoped_lock lock(mutex);

		pTMP = pArray;
		iLen = iLength;

		iLength = 0;
		pArray = nullptr;

		return pTMP;
	}

	void put(const char *pStr, int ilen)
	{
		scoped_lock lock(mutex);

		iLength = ilen;
		if (iLength > max_length)
		{
			std::cout << "Out of range ! 4-Length: " << iLength << std::endl;
		}

		std::memcpy(pArray, pStr, iLength);
		pArray[iLength] = '\0';
	}

	const int getLength() const
	{
		return iLength;
	}

	const char * getPtr() const
	{
		return pArray;
	}

	void lock()
	{
		exOPMutex.lock();
	}

	void unlock()
	{
		exOPMutex.unlock();
	}

private:
	boost::mutex mutex;
	boost::mutex exOPMutex;
	int iLength;

	//Init without Static Var
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
