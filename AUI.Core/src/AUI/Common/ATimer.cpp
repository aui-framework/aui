#include "ATimer.h"

ATimer::ATimer(unsigned msPeriod):
	mMsPeriod(msPeriod)
{
}

ATimer::~ATimer()
{
	stop();
}

void ATimer::restart()
{
	mResetFlag = true;
	mThread->interrupt();
}

void ATimer::start()
{
	mThread = _new<AThread>([&]()
	{
		for (;;) {
			try {
				AThread::sleep(mMsPeriod);
				emit fired();
			}
			catch (...)
			{
				if (mResetFlag)
				{
					mResetFlag = false;
				}
				else
				{
					return;
				}
			}
		}
	});
	mThread->start();
}

void ATimer::stop()
{
	mResetFlag = false;
	if (mThread) {
        mThread->interrupt();
        mThread = nullptr;
    }
}

bool ATimer::isStarted()
{
	return mThread != nullptr;
}
