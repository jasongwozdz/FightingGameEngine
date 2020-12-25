#pragma once
#include <assert.h>
template<class T>
class Singleton
{
private:
	Singleton(const Singleton<T>&);

	Singleton& operator=(const Singleton<T>&);

protected:

	static T* msSingleton;
public:
	Singleton() {
		assert(!msSingleton);
		msSingleton = reinterpret_cast<T*>(this);
	};

	~Singleton() 
	{
		assert(msSingleton);
		msSingleton = nullptr;
	};

	static T& getSingleton() {
		assert(msSingleton);
		return (*msSingleton);
	};

	static T* getSingletonPtr()
	{
		return msSingleton;
	};
};

