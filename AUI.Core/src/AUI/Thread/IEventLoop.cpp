#include "IEventLoop.h"
#include "AThread.h"

IEventLoop::Handle::Handle(IEventLoop* loop) : mCurrentEventLoop(loop) {
	std::unique_lock lock(AThread::current()->mEventLoopLock);
	mPrevEventLoop = AThread::current()->mCurrentEventLoop;
	AThread::current()->mCurrentEventLoop = loop;
}

IEventLoop::Handle::~Handle() {
	std::unique_lock lock(AThread::current()->mEventLoopLock);
	AThread::current()->mCurrentEventLoop = mPrevEventLoop;
}
