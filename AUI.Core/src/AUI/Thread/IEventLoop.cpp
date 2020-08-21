#include "IEventLoop.h"
#include "AThread.h"

IEventLoop::IEventLoop()
{
	std::unique_lock lock(AThread::current()->mEventLoopLock);
	mPrevEventLoop = AThread::current()->mCurrentEventLoop;
	AThread::current()->mCurrentEventLoop = this;
}


IEventLoop::~IEventLoop()
{
	std::unique_lock lock(AThread::current()->mEventLoopLock);
	AThread::current()->mCurrentEventLoop = mPrevEventLoop;
}
