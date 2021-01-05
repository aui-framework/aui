#pragma once

#include <AUI/Core.h>

class API_AUI_CORE IEventLoop
{
public:
	IEventLoop() {}
	virtual ~IEventLoop() {}

	/**
	 * \brief Notifies this IEventLoop that its thread got a new message to process.
	 */
	virtual void notifyProcessMessages() = 0;

	/**
	 * \brief Do message processing loop.
	 */
	virtual void loop() = 0;


    class API_AUI_CORE Handle {
	private:
        IEventLoop* mPrevEventLoop;
	    IEventLoop* mCurrentEventLoop;

	public:
        explicit Handle(IEventLoop* loop);
        ~Handle();
    };
};
