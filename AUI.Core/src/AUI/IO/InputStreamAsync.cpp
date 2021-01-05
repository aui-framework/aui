#include "InputStreamAsync.h"

InputStreamAsync::InputStreamAsync(_<IInputStream> inputStream):
	mReadThread(_new<AThread>([&, inputStream]()
	{
		try {
			for (;;)
			{
				auto buffer = _new<AByteBuffer>();
				inputStream->read(*buffer);
				emit read(buffer);
			}
		} catch (...)
		{
			
		}
		emit finished();
	}))
{
	auto t = mReadThread;
	AThread::current() << [t] {
		t->start();
	};
}